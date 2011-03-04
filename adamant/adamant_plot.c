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

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "adamant_plot.h"

void adamant_plot_din_vs_ready_init(AdamantRuntime *runtime) {
  svt_plot_din_vs_ready_init(&runtime->config->din_vs_ready,
			    &runtime->stats->din_vs_ready);
}

void adamant_plot_din_vs_src_init(AdamantRuntime *runtime) {
  svt_plot_din_vs_src_init(&runtime->config->din_vs_src,
                            &runtime->stats->din_vs_src);
}

void adamant_plot_din_vs_ready(AdamantRuntime *runtime, 
			       TmtOper *oper, guint64 din, 
			       guint64 ready) 
{
  svt_plot_din_vs_ready(din, ready,
			&runtime->stats->din_vs_ready,
			&runtime->config->din_vs_ready);
  
}
/*
void adamant_plot_din_vs_src(AdamantRuntime *runtime,
                             TmtOper *oper, guint64 din,
                             guint64 num_src,
                             guint64 *sources)
{
  svt_plot_din_vs_src(din, num_src, sources,
                      &runtime->stats->din_vs_src,
                      &runtime->config->din_vs_src);
}
*/
void adamant_plot_din_vs_ready_finalize(AdamantRuntime *runtime) {
  svt_plot_din_vs_ready_finalize(&runtime->stats->din_vs_ready); 
}

void adamant_plot_din_vs_src_finalize(AdamantRuntime *runtime) {
  svt_plot_din_vs_src_finalize(&runtime->stats->din_vs_src);
}

void adamant_plot_sin_vs_ready_init(AdamantRuntime *runtime) {
  svt_plot_sin_vs_ready_init(&runtime->config->sin_vs_ready,
			     &runtime->stats->sin_vs_ready);
}

void adamant_plot_sin_vs_src_init(AdamantRuntime *runtime) {
  svt_plot_sin_vs_src_init(&runtime->config->sin_vs_src,
                           &runtime->stats->sin_vs_src);
}

void adamant_plot_sin_vs_ready(AdamantRuntime *runtime, 
			       TmtOper *oper, guint64 din, 
			       guint64 sin, 
			       guint64 ready) 
{
  svt_plot_sin_vs_ready(din, sin, ready, 
			&runtime->stats->sin_vs_ready,
			&runtime->config->sin_vs_ready);
}
/*
void adamant_plot_sin_vs_src(AdamantRuntime *runtime,
                             TmtOper *oper, guint64 din,
                             guint64 sin,
                             guint64 num_src, guint64 *sources)
{
  svt_plot_sin_vs_src(din, sin, num_src, sources,
                        &runtime->stats->sin_vs_src,
                        &runtime->config->sin_vs_src);
}
*/
void adamant_plot_sin_vs_ready_finalize(AdamantRuntime *runtime) {
  svt_plot_sin_vs_ready_finalize(&runtime->stats->sin_vs_ready);
}

void adamant_plot_sin_vs_src_finalize(AdamantRuntime *runtime) {
  svt_plot_sin_vs_src_finalize(&runtime->stats->sin_vs_src);
}

void adamant_plot_sin_vs_ready_free(SVTInvsready *invsready) {
  /* Do nothing because there is no page table destruction routine.
     If you do fill this code in, be sure to free all the GArray's
     inside the PT */
}

void adamant_plot_sin_vs_src_free(SVTInvsSrc *invssrc) {
  /* Do nothing because there is no page table destruction routine.
     If you do fill this code in, be sure to free all the GArray's
     inside the PT */
}

