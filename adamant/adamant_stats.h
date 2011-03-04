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

#ifndef __ADAMANT_STATS_H__
#define __ADAMANT_STATS_H__

/*
 * Header dependences
 */
#include <glib.h>
#include <tmt.h>
#include <svt.h>
#include <stdio.h>

#include <cudd.h>
#include <cuddInt.h>
#include "dddmp.h"


/* Forward declarations */
typedef struct _AdamantStats AdamantStats;

/* Circular dependences */
#include "adamant_runtime.h"
#include "adamant_bdd.h"

G_BEGIN_DECLS

/*
 * Type definitions
 */
typedef struct {
    DdManager *manager;   
    DdNode *din_vs_ready;
    DdNode *din_vs_sin;
    DdNode *din_vs_din;
    DdNode *din_vs_hot;
    DdNode *din_vs_sys;
} DDStats;

struct _AdamantStats {
    GArray *histogram;
    SVTInvsready din_vs_ready;
    SVTInvsSrc din_vs_src;
    SVTInvsready sin_vs_ready;
    SVTInvsSrc sin_vs_src;
    SvtDepGraph dep_graph;    
    DDStats dd_stats;
    GArray *variance;
    struct {
        FILE *fp;
    } win_hist;
};

/*
 * Functions declarations
 */

AdamantStats * 
adamant_new_stats();

void 
adamant_free_stats(AdamantStats *stats);

void
adamant_initialize_stats(AdamantRuntime *runtime);

void 
adamant_stats_finalize(AdamantRuntime *runtime);

void 
adamant_stats_ready_time(AdamantRuntime *runtime, 
                         TmtOper *oper, 
                         guint64 ready, 
                         guint64 din,
                         guint thread_id,
                         guint num_din_srcs);

void adamant_stats_do_tick(AdamantConfig *, 
                           AdamantRuntime *,
                           guint64);
G_END_DECLS

#endif

