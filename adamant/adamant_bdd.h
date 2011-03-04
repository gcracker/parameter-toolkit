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

#ifndef __ADAMANT_BDD_H__
#define __ADAMANT_BDD_H__

/*
 * Header dependences
 */
#include <glib.h>
#include <tmt.h>
#include <svt.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cudd.h"
#include "dddmp.h"
#include "adamant_stats.h"
#include "adamant_dd_types.h"


G_BEGIN_DECLS

/*
 * Variables
 */

thread_args t_arg;
gboolean bdd_reordering;


/*
 * Functions declarations
 */
DdManager * adamant_bdd_manager(void);

void * adamant_bdd_thread(void *);

void adamant_initialize_bddstats(AdamantRuntime *runtime);

void 
adamant_bdd_finalize(AdamantRuntime *runtime, guint64 opcount);

void 
adamant_bdd_ready_time(AdamantRuntime *runtime, 
                         TmtOper *oper, 
                         guint64 ready, 
                         guint64 din,
                         guint thread_id,
                         guint num_din_srcs);

void adamant_bdd_tick(AdamantConfig *, 
                           AdamantRuntime *,
                           guint64);

void adamant_bdd_tickoutput(AdamantConfig *, 
                           AdamantRuntime *,
                           guint64);

DdNode *adamant_bdd_add_tuple(DdManager *manager, DdNode *set, guint64 x, guint64 y);

DdNode *adamant_bdd_add_tuple_serial(DdManager *manager, DdNode *set, guint64 x, guint64 y);

int adamant_bdd_var_swap(AdamantRuntime *runtime, unsigned long maxMem);

int DynReorderHook(DdManager * dd, const char * str, void * data);

DdNode * adamant_bdd_make_var(DdManager *manager, guint64 x);

DdNode *adamant_bdd_range(AdamantRuntime * runtime, guint64 high, guint64 low);

inline DdNode * adamant_tight_BddIte( DdManager * dd, DdNode * f, DdNode * g, DdNode * h);

int adamant_Canonical( DdManager * dd, DdNode ** fp, DdNode ** gp, DdNode ** hp );

void adamant_bdd_quit(AdamantStats *stats);

int print_variable_motion(DdNode * node, DdManager * manager, int * previous_order);

int merge_threads(AdamantRuntime *runtime);
int adamant_bdd_headerinit(FILE * bddfile, AdamantRuntime *runtime );

DdNode * bdd_build_tuple(DdManager *manager, guint64 x, guint64 y);
void adamant_bdd_sizeout(DdManager * manager, guint64 opcount);
void adamant_bdd_subTableSizeOut(DdManager * manager, 
                                 guint64 opcount);
static guint64 mask(guint64 v);

G_END_DECLS

#endif

