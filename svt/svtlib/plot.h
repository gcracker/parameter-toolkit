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
#ifndef _PLOT_H_
#define _PLOT_H_

#include <glib.h>
#include "svt_sched.h"

typedef struct {
  GArray *arr;
  PageTable64 *pt;
  int fd;
  guint32 cur_din_bin;
  guint32 ready_bin_size;
  guint32 din_bin_size;
  gboolean end;
} SVTInvsready;

typedef struct {
  GArray *arr;
  PageTable64 *pt;
  int fd;
  guint32 cur_din_bin;
  guint32 sin_bin_size;
  guint32 din_bin_size;
  gboolean end;
} SVTInvsSrc;

typedef struct {
  gboolean doit;
  gchar * outfilename;
  guint32 din_bin_size;
  guint32 ready_bin_size;
  guint64 minopcount;
  guint64 maxopcount;
  guint64 mincycle;
  guint64 maxcycle;
} SVTInvsreadyConfig;

typedef struct {
  gboolean doit;
  gchar * outfilename;
  guint32 sin_bin_size;
  guint32 din_bin_size;
  guint64 minopcount;
  guint64 maxopcount;
} SVTInvsSrcConfig;


typedef struct _SvtDepGraph SvtDepGraph;
struct _SvtDepGraph
{
  FILE ** din_src_fp;
  FILE ** din_rdy_fp;
};

typedef struct _SvtDepGraphConfig SvtDepGraphConfig;
struct _SvtDepGraphConfig
{
  gboolean doit;
  gchar * filename;
  guint64 * range_start;
  guint64 * range_end;
  guint range_len;
};

void
svt_plot_dep_graph_config_init(SvtDepGraphConfig * dgconfig);
void
svt_plot_dep_graph_config_setup(SvtDepGraphConfig * dgconfig,
				gchar **args, guint nargs);
void
svt_plot_dep_graph_config_free(SvtDepGraphConfig * dgconfig);

void
svt_plot_dep_graph_init(SvtDepGraphConfig * dgconfig,
			SvtDepGraph * dg );

void
svt_plot_dep_graph(guint64 din,
		   guint64 ready,
		   guint64 thread_id,
		   GArray * src_sched_info_array,
		   guint src_sched_info_len,
		   SvtDepGraph * dg,
		   SvtDepGraphConfig * dgconfig);

void
svt_plot_dep_graph_finalize(SvtDepGraph * dg,
			    SvtDepGraphConfig * dgconfig);

void svt_plot_din_vs_ready_init(SVTInvsreadyConfig *invsreadyconfig,
				SVTInvsready *invsready);
void svt_plot_din_vs_src_init(SVTInvsSrcConfig *invssrcconfig,
			      SVTInvsSrc *invssrc);

void svt_plot_din_vs_ready(guint64 din, 
			   guint64 ready,
			   SVTInvsready *invsready,
			   SVTInvsreadyConfig *invsreadyconfig);
void svt_plot_din_vs_src(guint64 din, 
			 GArray * src_sched_info_array,
                         guint src_sched_info_len,
			 SVTInvsSrc *invssrc,
			 SVTInvsSrcConfig *invssrcconfig);

void svt_plot_din_vs_ready_finalize(SVTInvsready *invsready);
void svt_plot_din_vs_src_finalize(SVTInvsSrc *invssrc);

void svt_plot_sin_vs_ready_init(SVTInvsreadyConfig *invsreadyconfig,
				SVTInvsready *invsready);
void svt_plot_sin_vs_src_init(SVTInvsSrcConfig *invssrcconfig,
			      SVTInvsSrc *invssrc);

void svt_plot_sin_vs_ready(guint64 din, 
			   guint64 sin, 
			   guint64 ready,
			   SVTInvsready *invsready,
			   SVTInvsreadyConfig *invsreadyconfig);
void svt_plot_sin_vs_src(guint64 din,
                         guint64 sin,
			 GArray * src_sched_info_array,
                         guint src_sched_info_len,
			 SVTInvsSrc *invssrc,
			 SVTInvsSrcConfig *invssrcconfig);

void svt_plot_sin_vs_ready_finalize(SVTInvsready *invsready);
void svt_plot_sin_vs_src_finalize(SVTInvsSrc *invssrc);

#endif /* _PLOT_H_ */
#ifdef __cplusplus
}
#endif

