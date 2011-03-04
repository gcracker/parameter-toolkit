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
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <svt.h>

typedef struct {
  guint32 graylevel;
} PlotIntensity;

void
svt_plot_dep_graph_config_init(SvtDepGraphConfig * dgconfig)
{
  dgconfig->doit = FALSE;
  dgconfig->filename = NULL;
  dgconfig->range_start = NULL;
  dgconfig->range_end = NULL;
  dgconfig->range_len = 0;
}

void
svt_plot_dep_graph_config_setup(SvtDepGraphConfig * dgconfig,
				gchar **args, guint nargs)
{
  guint i;

  if(nargs < 3) {
    g_error("Must pass at least 3 arguments to --plotdepgraph\n"
	    "usage: --plotdepgraph <file>,<beg,end>,<beg,end>,...>\n");
  }
  
  dgconfig->doit=1;
  dgconfig->filename = g_strdup(args[0]);
  dgconfig->range_start = g_new0(guint64, nargs-1);
  dgconfig->range_end = g_new0(guint64, nargs-1);
  dgconfig->range_len = (nargs-1)/2;

  for (i=1; i<nargs; i++) {
    if ((i%2) == 1)
      dgconfig->range_start[i/2] = g_ascii_strtoull(args[i], NULL, 0);
    else
      dgconfig->range_end[(i-1)/2] = g_ascii_strtoull(args[i], NULL, 0);
  }
}

void
svt_plot_dep_graph_config_free(SvtDepGraphConfig * dgconfig)
{
  g_free(dgconfig->range_start);
  g_free(dgconfig->range_end);
  g_free(dgconfig->filename);
}

void
svt_plot_dep_graph_init(SvtDepGraphConfig * dgconfig,
			SvtDepGraph * dg)
{
  gchar * din_src_filename;
  gchar * din_rdy_filename;
  gchar range_string[128];
  guint i;

  dg->din_src_fp = NULL;
  dg->din_rdy_fp = NULL;

  if (dgconfig->range_len == 0) return;

  // allocate FILE pointers
  dg->din_src_fp = g_new0(FILE*, dgconfig->range_len);
  dg->din_rdy_fp = g_new0(FILE*, dgconfig->range_len);

  // setup file for each range
  for (i=0; i<dgconfig->range_len; i++) {
    sprintf(range_string, "%lld-%lld", 
	    dgconfig->range_start[i],
	    dgconfig->range_end[i]);

    din_src_filename = g_strconcat(dgconfig->filename, 
				   "_",
				   range_string,
				   ".edge",
				   NULL);
    din_rdy_filename = g_strconcat(dgconfig->filename,
				   "_",
				   range_string,
				   ".node",
				   NULL);

    dg->din_src_fp[i] = fopen(din_src_filename, "w");
    dg->din_rdy_fp[i] = fopen(din_rdy_filename, "w");

    g_free(din_src_filename);
    g_free(din_rdy_filename);
  }
}

void
svt_plot_dep_graph_finalize(SvtDepGraph * dg,
			    SvtDepGraphConfig * dgconfig)
{
  guint i;

  for (i=0; i<dgconfig->range_len; i++) {
    fclose(dg->din_src_fp[i]);
    fclose(dg->din_rdy_fp[i]);
  }
}

void
svt_plot_dep_graph(guint64 din,
		   guint64 ready,
		   guint64 thread_id,
		   GArray * src_sched_info_array,
		   guint src_sched_info_len,
		   SvtDepGraph * dg,
		   SvtDepGraphConfig * dgconfig)
{
  guint i,j;

  for (i=0; i<dgconfig->range_len; i++) {

    // look for ready time in current range
    if ((ready >= dgconfig->range_start[i]) &&
	(ready < dgconfig->range_end[i])) {

      // print current din information
      fprintf(dg->din_rdy_fp[i], "  %" G_GUINT64_FORMAT " %" G_GUINT64_FORMAT
	      " %" G_GUINT64_FORMAT "\n", din, ready, thread_id);

      for (j=0; j<src_sched_info_len; j++) {
	SchedInfo * si = g_array_index(src_sched_info_array, SchedInfo*, j);

	if (si->din_src != 0) {
	  if (si->ready < dgconfig->range_start[i]) {
	    // din outside range
	    continue;
	    /*
	    fprintf(dg->din_rdy_fp[i], "  %" G_GUINT64_FORMAT
		    " %" G_GUINT64_FORMAT " %" G_GINT32_FORMAT "\n",
		    si->din_src, si->ready, -1);
	    */
	  } 
	  // print edges
	  fprintf(dg->din_src_fp[i], "  %" G_GUINT64_FORMAT " -> %"
		  G_GUINT64_FORMAT ";\n", si->din_src, din);
	}
      }
    }
  }
}

void svt_plot_din_vs_ready_init(SVTInvsreadyConfig *invsreadyconfig,
				SVTInvsready *invsready) 
{
  guint32 din_bin_size=invsreadyconfig->din_bin_size;
  guint32 ready_bin_size=invsreadyconfig->ready_bin_size;
  guint32 bufelem;
  int fd;

  invsready->pt = NULL;
  invsready->arr = g_array_new(FALSE,TRUE,
			       sizeof(PlotIntensity));
  invsready->ready_bin_size = ready_bin_size;
  invsready->din_bin_size = din_bin_size;
  invsready->cur_din_bin = 0;
  invsready->end = FALSE;
  fd=invsready->fd = 
    open(invsreadyconfig->outfilename, 
	 O_WRONLY | O_CREAT | O_TRUNC, 
	 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  bufelem=GUINT32_TO_LE(din_bin_size);
  write(fd,&bufelem,sizeof(guint32));
  bufelem=GUINT32_TO_LE(ready_bin_size);
  write(fd,&bufelem,sizeof(guint32));

  //  printf("\tDIN bin size=%d, Ready bin size=%d\n",
  //	 din_bin_size, ready_bin_size);
}

void svt_plot_din_vs_src_init(SVTInvsSrcConfig *invssrcconfig,
				SVTInvsSrc *invssrc) 
{
  guint32 din_bin_size=invssrcconfig->din_bin_size;
  guint32 bufelem;
  int fd;

  invssrc->pt = NULL;
  invssrc->arr = g_array_new(FALSE,TRUE,
			       sizeof(PlotIntensity));
  invssrc->din_bin_size = din_bin_size;
  invssrc->cur_din_bin = 0;
  invssrc->end = FALSE;
  fd=invssrc->fd = 
    open(invssrcconfig->outfilename, 
	 O_WRONLY | O_CREAT | O_TRUNC, 
	 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  bufelem=GUINT32_TO_LE(din_bin_size);
  write(fd,&bufelem,sizeof(guint32));
  bufelem=GUINT32_TO_LE(din_bin_size);
  write(fd,&bufelem,sizeof(guint32));
}

void svt_plot_din_vs_ready(guint64 din, 
			   guint64 ready, 
			   SVTInvsready *invsready, 
			   SVTInvsreadyConfig *invsreadyconfig)
{
  guint32 x,y;
  PlotIntensity *intensity;
  guint32 cur_din_bin = invsready->cur_din_bin;
  GArray *arr;
  int fd;

  fd=invsready->fd; 
  arr=invsready->arr;

  if(invsready->end) return;
  if(ready < invsreadyconfig->mincycle) return;
  if(ready > invsreadyconfig->maxcycle) return;
  if(din < invsreadyconfig->minopcount) return;
  if(invsreadyconfig->maxopcount &&
     (din > invsreadyconfig->maxopcount)) {
    invsready->end = TRUE;
    svt_plot_din_vs_ready_finalize(invsready);
  }
  
  x = (ready - invsreadyconfig->mincycle) / invsready->ready_bin_size;
  y = din / invsready->din_bin_size;
  g_assert(y >= cur_din_bin);

  /* BIN ended, write out and reinitialize array */
  if(y > cur_din_bin) {
    guint32 bufelem;
    int i;

    cur_din_bin++;
    invsready->cur_din_bin=cur_din_bin;

    bufelem=GUINT32_TO_LE(arr->len);
    write(fd,&bufelem,sizeof(guint32));
    for(i=0;i<arr->len;i++) {
      intensity = &g_array_index(arr,PlotIntensity,i);
      intensity->graylevel = GUINT32_TO_LE(intensity->graylevel);
    }
    write(fd,arr->data, arr->len*sizeof(PlotIntensity));

    for(i=0;i<arr->len;i++) {
      intensity = &g_array_index(arr,PlotIntensity,i);
      intensity->graylevel = 0;
    }
  }
  if(invsready->end) return;

  if(x >= arr->len) g_array_set_size(arr,x+1);
  intensity =  &g_array_index(invsready->arr, 
			      PlotIntensity, x); 
  if(intensity->graylevel != G_MAXUINT32)
    intensity->graylevel++;
  
  return;
}

void svt_plot_din_vs_src(guint64 din,
                         GArray * src_sched_info_array,
                         guint src_sched_info_len,
                         SVTInvsSrc *invssrc,
                         SVTInvsSrcConfig *invssrcconfig)
{
  guint32 y;     //y coordinate; 
  PlotIntensity *intensity;
  guint32 cur_din_bin = invssrc->cur_din_bin;
  GArray *arr;
  int fd;
  int i;

  fd=invssrc->fd; 
  arr=invssrc->arr;

  if(invssrc->end) return;
  if(din < invssrcconfig->minopcount) return;
  if(invssrcconfig->maxopcount &&
     (din > invssrcconfig->maxopcount)) {
    invssrc->end = TRUE;
    svt_plot_din_vs_src_finalize(invssrc);
  }

  y = din / invssrc->din_bin_size;
  g_assert(y >= cur_din_bin);

  /* BIN ended, write out and reinitialize array */
  if(y > cur_din_bin) {
    guint32 bufelem;
    int i;

    cur_din_bin++;
    invssrc->cur_din_bin=cur_din_bin;

    bufelem=GUINT32_TO_LE(arr->len);
    write(fd,&bufelem,sizeof(guint32));
    for(i=0;i<arr->len;i++) {
      intensity = &g_array_index(arr,PlotIntensity,i);
      intensity->graylevel = GUINT32_TO_LE(intensity->graylevel);
    }
    write(fd,arr->data, arr->len*sizeof(PlotIntensity));

    for(i=0;i<arr->len;i++) {
      intensity = &g_array_index(arr,PlotIntensity,i);
      intensity->graylevel = 0;
    }
  }
  if(invssrc->end) return;

  for (i=0; i<src_sched_info_len; i++) {
    SchedInfo * si = g_array_index(src_sched_info_array, SchedInfo*, i); //x-coordinates

    if (si->din_src != 0) {
      if((si->din_src/invssrc->din_bin_size) >=arr->len) g_array_set_size(arr,((si->din_src/invssrc->din_bin_size)+1));
        intensity = &g_array_index(invssrc->arr,PlotIntensity,(si->din_src/invssrc->din_bin_size));
      if(intensity->graylevel != G_MAXUINT32)
        intensity->graylevel++;
    }
  }
  return;
}

void svt_plot_din_vs_ready_finalize(SVTInvsready *invsready) 
{
  int fd;
  guint32 i;
  PlotIntensity *intensity;
  GArray *arr;
  guint32 bufelem;

  // If end is set, this was called earlier by plot
  if(invsready->end) return;

  fd=invsready->fd;
  arr=invsready->arr;

  bufelem=GUINT32_TO_LE(arr->len);
  write(fd,&bufelem,sizeof(guint32));
  for(i=0;i<arr->len;i++) {
    intensity = &g_array_index(arr,PlotIntensity,i);
    intensity->graylevel = GUINT32_TO_LE(intensity->graylevel);
  }
  write(fd,arr->data, arr->len*sizeof(PlotIntensity));

  close(fd);
  return;
}

void svt_plot_din_vs_src_finalize(SVTInvsSrc *invssrc) 
{
  int fd;
  guint32 i;
  PlotIntensity *intensity;
  GArray *arr;
  guint32 bufelem;

  // If end is set, this was called earlier by plot
  if(invssrc->end) return;

  fd=invssrc->fd;
  arr=invssrc->arr;

  bufelem=GUINT32_TO_LE(arr->len);
  write(fd,&bufelem,sizeof(guint32));
  for(i=0;i<arr->len;i++) {
    intensity = &g_array_index(arr,PlotIntensity,i);
    intensity->graylevel = GUINT32_TO_LE(intensity->graylevel);
  }
  write(fd,arr->data, arr->len*sizeof(PlotIntensity));

  close(fd);
  return;
}

void svt_plot_sin_vs_ready_init(SVTInvsreadyConfig *invsreadyconfig,
				SVTInvsready *invsready)
{
  guint32 sin_bin_size=invsreadyconfig->din_bin_size; //really sin_bin_size
  guint32 ready_bin_size=invsreadyconfig->ready_bin_size;
  guint32 bufelem;
  int fd;

  invsready->arr = NULL;
  invsready->pt = PageTable64_new();
  invsready->ready_bin_size = ready_bin_size;
  invsready->din_bin_size = sin_bin_size;
  invsready->cur_din_bin = 0; // Not used for sinvsready
  invsready->end = FALSE; // Not used for sinvsready
  fd=invsready->fd = 
    open(invsreadyconfig->outfilename, 
	 O_WRONLY | O_CREAT | O_TRUNC, 
	 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  bufelem=GUINT32_TO_LE(sin_bin_size);
  write(fd,&bufelem,sizeof(guint32));
  bufelem=GUINT32_TO_LE(ready_bin_size);
  write(fd,&bufelem,sizeof(guint32));
  
  //printf("readybinsize=%d\n",ready_bin_size);
}

void svt_plot_sin_vs_src_init(SVTInvsSrcConfig *invssrcconfig,
				SVTInvsSrc *invssrc) 
{
  guint32 sin_bin_size=invssrcconfig->sin_bin_size; //really sin_bin_size
  guint32 din_bin_size=invssrcconfig->din_bin_size;
  guint32 bufelem;
  int fd;
                                                                                                                                                             
  invssrc->arr = NULL;
  invssrc->pt = PageTable64_new();
  invssrc->sin_bin_size = sin_bin_size;
  invssrc->din_bin_size = din_bin_size;
  invssrc->cur_din_bin = 0; // Not used for sinvssrc
  invssrc->end = FALSE; // Not used for sinvssrc
  fd=invssrc->fd =
    open(invssrcconfig->outfilename,
         O_WRONLY | O_CREAT | O_TRUNC,
         S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                                                                                                                                                             
  bufelem=GUINT32_TO_LE(sin_bin_size);
  write(fd,&bufelem,sizeof(guint32));
  bufelem=GUINT32_TO_LE(din_bin_size);
  write(fd,&bufelem,sizeof(guint32));
}

void svt_plot_sin_vs_ready(guint64 din, 
			   guint64 sin, 
			   guint64 ready,
			   SVTInvsready *invsready,
			   SVTInvsreadyConfig *invsreadyconfig)
{
  guint32 x,y;
  guint64 aligned_y, index_y;
  PlotIntensity *intensity;
  GArray *subarr;
  PageTableEntry *entry;

  if(ready < invsreadyconfig->mincycle) return;
  if(ready > invsreadyconfig->maxcycle) return;
  if(din < invsreadyconfig->minopcount) return;
  if(invsreadyconfig->maxopcount && din >= invsreadyconfig->maxopcount) return;

  x = (ready - invsreadyconfig->mincycle) / invsready->ready_bin_size;
  y = sin / invsready->din_bin_size; /* Really sin_bin_size */

  aligned_y = y & ENTRY_KEY_LINE_MASK;
  index_y = y & ENTRY_KEY_INDEX_MASK;
  entry = PageTable64_lookup(invsready->pt, aligned_y);
  if(entry == NULL) {
    entry = g_new0(PageTableEntry,1);
    entry->key = aligned_y;
    PageTable64_add(invsready->pt, aligned_y, entry);
  }

  if(entry->data[index_y] == NULL) {
    entry->data[index_y]=g_array_new(FALSE,TRUE,sizeof(PlotIntensity));
  }
  subarr = (GArray*)entry->data[index_y];

  if(x >= subarr->len) g_array_set_size(subarr,x+1);
  intensity =  &g_array_index(subarr, PlotIntensity, x);
  if(intensity->graylevel != G_MAXUINT32)
    intensity->graylevel++;
  
  return;
}

void svt_plot_sin_vs_src(guint64 din,
                         guint64 sin,
			 GArray * src_sched_info_array,
                         guint src_sched_info_len,
			 SVTInvsSrc *invssrc,
			 SVTInvsSrcConfig *invssrcconfig)
{
  guint32 y;
  guint64 aligned_y, index_y;
  PlotIntensity *intensity;
  GArray *subarr;
  PageTableEntry *entry;
  guint i;      

  if(din < invssrcconfig->minopcount) return;
  if(invssrcconfig->maxopcount && din >= invssrcconfig->maxopcount) return;
                                                                                                                                                             
  y = sin / invssrc->sin_bin_size; /* Really sin_bin_size */
                                                                                                                                                             
  aligned_y = y & ENTRY_KEY_LINE_MASK;
  index_y = y & ENTRY_KEY_INDEX_MASK;
  entry = PageTable64_lookup(invssrc->pt, aligned_y);
  if(entry == NULL) {
    entry = g_new0(PageTableEntry,1);
    entry->key = aligned_y;
    PageTable64_add(invssrc->pt, aligned_y, entry);
  }
  if(entry->data[index_y] == NULL) {
    entry->data[index_y]=g_array_new(FALSE,TRUE,sizeof(PlotIntensity));
  }
  subarr = (GArray *)entry->data[index_y];
                                                                                                                                                             
  for (i=0; i<src_sched_info_len; i++) {
    SchedInfo * si = g_array_index(src_sched_info_array, SchedInfo*, i); //x-coordinates

    if (si->din_src != 0) {
      if((si->din_src/invssrc->din_bin_size) >=subarr->len) g_array_set_size(subarr,((si->din_src/invssrc->din_bin_size)+1));
        intensity = &g_array_index(subarr,PlotIntensity,(si->din_src/invssrc->din_bin_size));
      if(intensity->graylevel != G_MAXUINT32)
        intensity->graylevel++;
    }
  }
  return;
}

static guint64 ipow(guint64 x, guint64 y)
{
  guint64 result = 1;
  while(y) {
    result *= x;
    y--;
  }
  return result;
}

static guint32 plot_sin_vs_ready_finalize_walk(SVTInvsready *invsready,
					       guint64 address,
					       void *node, 
					       int max_depth, int depth, 
					       gboolean *plot, 
					       guint32 backlog,
					       int fd,
					       guint64 *start_address)
{
  guint32 i;
  guint32 j;

  if(depth < max_depth) {
    PageTable64InteriorNode *int_node = (PageTable64InteriorNode *)node;
    for(i = 0; i < PT64_INTERIOR_PAGE_LEN; i++) {
      if(int_node->nodes[i] == NULL) {
	if(*plot) {
	  backlog += ipow(PT64_INTERIOR_PAGE_LEN, max_depth-depth-1) *
	    ENTRY_LINE_SIZE;
	}
      } else {
	guint64 child_address = 
	  address + (i << (PT64_INTERIOR_PAGE_BITS * (max_depth - depth - 1) + 
			   ENTRY_LINE_BITS));

	backlog = plot_sin_vs_ready_finalize_walk(invsready,
						  child_address,
						  int_node->nodes[i],
						  max_depth, 
						  depth + 1,
						  plot, backlog, fd,
						  start_address);
      }
    }
  } else {
    PageTableEntry *entry = (PageTableEntry *)node;
    for(i = 0; i < ENTRY_LINE_SIZE; i++) {
      GArray *subarr = (GArray *)entry->data[i];
      if(subarr == NULL) {
	if(*plot)
	  backlog++;
      } else {
	guint32 bufelem;

	if(*plot) {
	  /* No need to convert 0 to little-endian */
	  guint32 *zeros = g_new0(guint32, backlog);
	  write(fd, zeros, sizeof(guint32) * backlog);
	  backlog = 0;
	} else {
	  *start_address = (address + i) * invsready->din_bin_size;
	}

	*plot = 1;

	bufelem=GUINT32_TO_LE(subarr->len);
	write(fd,&bufelem,sizeof(guint32));
	for(j=0;j<subarr->len;j++) {
	  PlotIntensity *intensity;
	  intensity = &g_array_index(subarr,PlotIntensity,j);
	  intensity->graylevel = GUINT32_TO_LE(intensity->graylevel);
	}
	write(fd,subarr->data, subarr->len*sizeof(PlotIntensity));
      }
    }
  }

  return backlog;
}

static guint32 plot_sin_vs_src_finalize_walk(SVTInvsSrc *invssrc,
					       guint64 address,
					       void *node, 
					       int max_depth, int depth, 
					       gboolean *plot, 
					       guint32 backlog,
					       int fd,
					       guint64 *start_address)
{
  guint32 i;
  guint32 j;

  if(depth < max_depth) {
    PageTable64InteriorNode *int_node = (PageTable64InteriorNode *)node;
    for(i = 0; i < PT64_INTERIOR_PAGE_LEN; i++) {
      if(int_node->nodes[i] == NULL) {
	if(*plot) {
	  backlog += ipow(PT64_INTERIOR_PAGE_LEN, max_depth-depth-1) *
	    ENTRY_LINE_SIZE;
	}
      } else {
	guint64 child_address = 
	  address + (i << (PT64_INTERIOR_PAGE_BITS * (max_depth - depth - 1) + 
			   ENTRY_LINE_BITS));

	backlog = plot_sin_vs_src_finalize_walk(invssrc,
						child_address,
						int_node->nodes[i],
						max_depth, 
						depth + 1,
						plot, backlog, fd,
						start_address);
      }
    }
  } else {
    PageTableEntry *entry = (PageTableEntry *)node;
    for(i = 0; i < ENTRY_LINE_SIZE; i++) {
      GArray *subarr = (GArray *)entry->data[i];
      if(subarr == NULL) {
	if(*plot)
	  backlog++;
      } else {
	guint32 bufelem;

	if(*plot) {
	  /* No need to convert 0 to little-endian */
	  guint32 *zeros = g_new0(guint32, backlog);
	  write(fd, zeros, sizeof(guint32) * backlog);
	  backlog = 0;
	} else {
	  *start_address = (address + i) * invssrc->sin_bin_size;
	}

	*plot = 1;

	bufelem=GUINT32_TO_LE(subarr->len);
	write(fd,&bufelem,sizeof(guint32));
	for(j=0;j<subarr->len;j++) {
	  PlotIntensity *intensity;
	  intensity = &g_array_index(subarr,PlotIntensity,j);
	  intensity->graylevel = GUINT32_TO_LE(intensity->graylevel);
	}
	write(fd,subarr->data, subarr->len*sizeof(PlotIntensity));
      }
    }
  }

  return backlog;
}

void svt_plot_sin_vs_ready_finalize(SVTInvsready *invsready) 
{
  gboolean plot = 0;
  guint64 start_address;
  
  plot_sin_vs_ready_finalize_walk(invsready,
				  0,
				  &invsready->pt->root,
				  invsready->pt->depth, 0,
				  &plot,
				  0,
				  invsready->fd,
				  &start_address);

  printf("Starting Static Instruction Address: 0x%llx\n", start_address);

  close(invsready->fd);
  return;
}

void svt_plot_sin_vs_src_finalize(SVTInvsSrc *invssrc) 
{
  gboolean plot = 0;
  guint64 start_address;
  
  plot_sin_vs_src_finalize_walk(invssrc,
				0,
				&invssrc->pt->root,
				invssrc->pt->depth, 0,
				&plot,
				0,
				invssrc->fd,
				&start_address);

  printf("Starting Static Instruction Address: 0x%llx\n", start_address);

  close(invssrc->fd);
  return;
} 
#ifdef __cplusplus
}
#endif

