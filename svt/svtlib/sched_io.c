/*#*STARTLICENCE*#
Copyright (c) 2005-2009, Regents of the University of Colorado

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of the University of Colorado nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
#*ENDLICENCE*#*/

#ifdef __cplusplus
extern "C" {
#endif
/* TODO: fix endianness issues */

#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include "TCgen_sched.h"
#include <glib.h>
#include <bzlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define BZ_BLOCKSIZE100K_MIN 1
#define BZ_BLOCKSIZE100K_MAX 9
#define BZ_BLOCKSIZE100K 5

#define BZ_VERBOSITY_SILENT 0
#define BZ_WORKFACTOR_DEFAULT 30

#define BZ_READ_SLOW 1
#define BZ_READ_FAST 0

#include <svt.h>

static inline void 
write_data(SVTContext *ctxt, void *buf, size_t count)
{
  int bzerror;

  g_assert(count < BUFSIZ);

  if(count > (BUFSIZ - ctxt->index)) {
    BZ2_bzWrite(&bzerror, ctxt->bzfp, ctxt->buf, ctxt->index);
    if(bzerror != BZ_OK) {
      g_error("SVT write_data failed");
    }
    ctxt->index = 0;
  }

  memcpy(&ctxt->buf[ctxt->index], buf, count);
  ctxt->index += count;
}

static inline void 
write_context_reset(SVTContext *ctxt)
{
  off64_t offset;
  int bzerror;
  unsigned int in, out;

  g_assert(ctxt->bzfp != NULL || ctxt->index == 0);

  if(ctxt->bzfp) {
    BZ2_bzWrite(&bzerror, ctxt->bzfp, ctxt->buf, ctxt->index);
    if(bzerror != BZ_OK) {
      g_error("SVT write_context_reset write failed");
    }
    ctxt->index = 0;
    
    BZ2_bzWriteClose(&bzerror, ctxt->bzfp, 0, &in, &out);
    if(bzerror != BZ_OK) {
      g_error("SVT write_context_reset close failed");
    }

    ctxt->chunk++;
    offset = ftello64(ctxt->fp);
    g_array_append_val(ctxt->offset_table, offset);
  }
  
  ctxt->bzfp = BZ2_bzWriteOpen(&bzerror, ctxt->fp, BZ_BLOCKSIZE100K,
			       BZ_VERBOSITY_SILENT, BZ_WORKFACTOR_DEFAULT);
  if(bzerror != BZ_OK) {
    g_error("SVT write_context_reset could not open bzip2 stream");
  }
}


SVTContext *
svt_context_write_create(char *filename, guint64 interval)
{
  off64_t offset;
  SVTContext *ctxt;
  ctxt = g_new0(SVTContext, 1);

  ctxt->fp = fopen64(filename, "w");
  if(ctxt->fp == NULL) {
    g_error("Could not open %s for writing\n", filename);
  }
 
  ctxt->count = 0;
  ctxt->init = TRUE;
  ctxt->index = 0;
  ctxt->bzfp = NULL;

  ctxt->reset_interval = interval;

  ctxt->chunk = 1;
  ctxt->offset_table = g_array_new(FALSE, FALSE, sizeof(off64_t));
  offset = ftello64(ctxt->fp);
  g_array_append_val(ctxt->offset_table, offset);

  return ctxt;
}

void
svt_context_write_destroy(SVTContext *ctxt)
{
  int bzerror;
  guint i;
  unsigned int in, out;

  BZ2_bzWrite(&bzerror, ctxt->bzfp, ctxt->buf, ctxt->index);
  if(bzerror != BZ_OK) {
    g_error("SVT svt_context_write_destroy write failed");
  }
  ctxt->index = 0;

  BZ2_bzWriteClose(&bzerror, ctxt->bzfp, 0, &in, &out);
  if(bzerror != BZ_OK) {
    g_error("SVT svt_context_write_destroy close failed");
  }

  g_assert(ctxt->chunk == ctxt->offset_table->len);
  for(i = 0; i < ctxt->chunk; i++) {
    off64_t offset = g_array_index(ctxt->offset_table, off64_t, i);
    fwrite(&offset, sizeof(off64_t), 1, ctxt->fp);
  }
  fwrite(&ctxt->chunk, sizeof(guint32), 1, ctxt->fp);
  fwrite(&ctxt->reset_interval, sizeof(guint64), 1, ctxt->fp);

  fclose(ctxt->fp);

  g_free(ctxt);
}


void 
svt_sched_write(SVTContext *ctxt, guint64 din, guint64 sin, guint64 cycle)
{
  unsigned char code;

  if(ctxt->init) {
    TCgen_sched_Init();
    write_context_reset(ctxt);
    ctxt->init = FALSE;
  }

  code = TCgen_sched_encode_sin(sin);
  write_data(ctxt, &code, sizeof(code));
  if(code == 2) {
    write_data(ctxt, &sin, sizeof(sin));
  }

  code = TCgen_sched_encode_din(din);
  write_data(ctxt, &code, sizeof(code));
  if(code == 2) {
    write_data(ctxt, &din, sizeof(din));
  }

  code = TCgen_sched_encode_cycle(sin, cycle);
  write_data(ctxt, &code, sizeof(code));
  if(code == 1) {
    write_data(ctxt, &cycle, sizeof(cycle));
  }

  ctxt->count++;
  if(ctxt->count == ctxt->reset_interval) {
    ctxt->init = TRUE;
    ctxt->count = 0;
  }  
}


static inline void 
read_data(SVTContext *ctxt, void *buf, size_t count)
{
  int bzerror;

  g_assert(count < BUFSIZ);

  if(count > (ctxt->end_index - ctxt->index)) {
    memcpy(buf, &ctxt->buf[ctxt->index], ctxt->end_index - ctxt->index);
    buf += (ctxt->end_index - ctxt->index);
    count -= (ctxt->end_index - ctxt->index);

    g_assert(!ctxt->eof);
    ctxt->end_index = BZ2_bzRead(&bzerror, ctxt->bzfp, ctxt->buf, BUFSIZ);

    if(bzerror == BZ_STREAM_END) {
      ctxt->eof = TRUE;
    } else if(bzerror != BZ_OK) {
      g_error("SVT read_data failed");
    }

    ctxt->index = 0;
  }
  
  g_assert(count <= (ctxt->end_index - ctxt->index));

  memcpy(buf, &ctxt->buf[ctxt->index], count);
  ctxt->index += count;
}

static inline void
read_context_reset(SVTContext *ctxt)
{
  int bzerror; 

  if(ctxt->bzfp != NULL) {
    BZ2_bzReadClose(&bzerror, ctxt->bzfp);
    if(bzerror != BZ_OK) {
      g_error("SVT read_context_reset close failed");
    }

    ctxt->chunk++;
  }
  fseeko64(ctxt->fp, g_array_index(ctxt->offset_table, off64_t, ctxt->chunk),
	   SEEK_SET);

  ctxt->bzfp = BZ2_bzReadOpen(&bzerror, ctxt->fp, BZ_READ_FAST, 
			      BZ_VERBOSITY_SILENT, NULL, 0);
  if(bzerror != BZ_OK) {
    g_error("SVT read_context_reset could not open bzip2 stream");
  }

  ctxt->index = ctxt->end_index = 0;
  ctxt->eof = FALSE;
}

SVTContext *
svt_context_read_create(char *filename)
{
  guint i;

  SVTContext *ctxt;
  ctxt = g_new0(SVTContext, 1);

  ctxt->fp = fopen64(filename, "r");
  if(ctxt->fp == NULL) {
    g_error("Could not open %s for reading\n", filename);
  }

  ctxt->offset_table = g_array_new(FALSE, FALSE, sizeof(off64_t));
  fseeko64(ctxt->fp, -(off64_t)(sizeof(guint32)+sizeof(guint64)), SEEK_END);
  fread(&ctxt->chunk, sizeof(guint32), 1, ctxt->fp);
  fread(&ctxt->reset_interval, sizeof(guint64), 1, ctxt->fp);
  fseeko64(ctxt->fp, 
	   -(off64_t)(sizeof(off64_t) * ctxt->chunk + 
		      sizeof(guint32) + sizeof(guint64)), 
	   SEEK_END);
  g_array_set_size(ctxt->offset_table, ctxt->chunk);
  for(i = 0; i < ctxt->chunk; i++) {
    fread(&g_array_index(ctxt->offset_table, off64_t, i),
	  sizeof(off64_t), 1, ctxt->fp);
  }
  ctxt->chunk = 0;
  fseeko64(ctxt->fp, 0, SEEK_SET);
 
  ctxt->count = 0;
  ctxt->init = TRUE;
  ctxt->index = 0;
  ctxt->end_index = 0;
  ctxt->bzfp = NULL;

  return ctxt;
}

void
svt_context_read_destroy(SVTContext *ctxt)
{
  int bzerror;

  BZ2_bzReadClose(&bzerror, ctxt->bzfp);
  if(bzerror != BZ_OK) {
    g_error("SVT svt_context_read_destroy close failed");
  }

  fclose(ctxt->fp);

#ifdef DEBUG_TCGEN
 {
   int i;
   fprintf(stderr, "SIN: ");
   for(i = 0; i < 3; i++) {
     fprintf(stderr, "%d ", ctxt->TCgen_performance.sin[i]);
   }
   fprintf(stderr, "\n");

   fprintf(stderr, "DIN: ");
   for(i = 0; i < 3; i++) {
     fprintf(stderr, "%d ", ctxt->TCgen_performance.din[i]);
   }
   fprintf(stderr, "\n");

   fprintf(stderr, "CYCLE: ");
   for(i = 0; i < 2; i++) {
     fprintf(stderr, "%d ", ctxt->TCgen_performance.cycle[i]);
   }
   fprintf(stderr, "\n");
 }
#endif

  g_free(ctxt);
}

void 
svt_sched_read(SVTContext *ctxt, guint64 *din, guint64 *sin, guint64 *cycle)
{
  unsigned char code;

  if(ctxt->init) {
    TCgen_sched_Init();
    read_context_reset(ctxt);
    ctxt->init = FALSE;
  }

  read_data(ctxt, &code, sizeof(code));
  if (code==2) {
    read_data(ctxt, sin, sizeof(*sin));
  }
  *sin = TCgen_sched_decode_sin(code, *sin);
#ifdef DEBUG_TCGEN
  ctxt->TCgen_performance.sin[code]++;
#endif

  read_data(ctxt, &code, sizeof(code));
  if (code==2) {
    read_data(ctxt, din, sizeof(*din));
  }
  *din = TCgen_sched_decode_din(code, *din);
#ifdef DEBUG_TCGEN
  ctxt->TCgen_performance.din[code]++;
#endif

  read_data(ctxt, &code, sizeof(code));
  if (code==1) {
    read_data(ctxt, cycle, sizeof(*cycle));
  }
  *cycle = TCgen_sched_decode_cycle(code, *sin, *cycle);
#ifdef DEBUG_TCGEN
  ctxt->TCgen_performance.cycle[code]++;
#endif

  ctxt->count++;
  if(ctxt->count == ctxt->reset_interval) {
    ctxt->init = TRUE;
    ctxt->count = 0;
  }  
}

gboolean
svt_schedule_finished(SVTContext *ctxt)
{
  return (ctxt->chunk == (ctxt->offset_table->len-1) &&
	  ctxt->eof && ctxt->index == ctxt->end_index);
}

void 
svt_sched_scan(SVTContext *ctxt, guint64 pos)
{
  int index;
  guint64 i;

  guint64 din, sin, cycle;

  /* Find the entry in the offset table closest to pos without exceeding it */
  index = pos / ctxt->reset_interval;

  pos = pos - (index * ctxt->reset_interval);
  ctxt->init = TRUE;
  ctxt->count = 0;
  ctxt->chunk = ctxt->bzfp ? index - 1 : index; /* Reset will increment chunk if there is already an open file */

  for(i = 0; i < pos; i++) {
    svt_sched_read(ctxt, &din, &sin, &cycle);
  }
}
#ifdef __cplusplus
}
#endif

