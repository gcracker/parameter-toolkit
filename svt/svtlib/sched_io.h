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
#ifndef _SCHED_IO_H_
#define _SCHED_IO_H_

#include <stdio.h>
#include <bzlib.h>
#include <glib.h>

typedef struct {
  gboolean init; /* Should we (re)init the TCgen'er */
  guint count;   /* How much data have we read/written */

  FILE *fp;
  BZFILE *bzfp;

  unsigned char buf[BUFSIZ];
  guint index;
  guint end_index;

  gboolean eof;

  guint32 chunk;
  GArray *offset_table;

  guint64 reset_interval;

  struct {
    guint64 sin[3];
    guint64 din[3];
    guint64 cycle[2];
  } TCgen_performance;
} SVTContext;

SVTContext *svt_context_write_create(char *filename, guint64 interval);
void svt_context_write_destroy(SVTContext *ctxt);
void svt_sched_write(SVTContext *ctxt, guint64 din, guint64 sin, guint64 cycle);

SVTContext *svt_context_read_create(char *filename);
void svt_context_read_destroy(SVTContext *ctxt);
void svt_sched_read(SVTContext *ctxt, guint64 *din, guint64 *sin, guint64 *cycle);

gboolean svt_schedule_finished(SVTContext *ctxt);
void svt_sched_scan(SVTContext *ctxt, guint64 pos);

#endif /* _SCHED_IO_H_ */
#ifdef __cplusplus
}
#endif

