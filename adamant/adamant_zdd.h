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

#ifndef __ADAMANT_ZDD_H__
#define __ADAMANT_ZDD_H__

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
 * Type definitions
 */


/*
 * Variables
 */

thread_args t_arg;
gboolean zdd_reordering;
GHashTable * zdd_layer_hash;
GQueue * ddRegions;

/*
 * Functions declarations
 */

int DynZddReorderHook(DdManager * dd, const char * str, void * data);
DdManager * adamant_zdd_manager(void);

void * adamant_zdd_thread(void *);

void adamant_initialize_zddstats(AdamantRuntime *runtime);

void
adamant_zdd_finalize(AdamantRuntime *runtime, guint64 opcount);

void
adamant_zdd_ready_time(AdamantRuntime *runtime,
                         TmtOper *oper,
                         guint64 ready,
                         guint64 din,
                         guint thread_id,
                         guint num_din_srcs);

void adamant_zdd_tick( AdamantRuntime *runtime, guint64 opcount);


void adamant_zdd_tickoutput(AdamantRuntime *, guint64 opcount);


DdNode *adamant_zdd_add_tuple(DdManager *manager, DdNode *set, guint64 x, guint64 y);

DdNode *adamant_zdd_add_tuple_classic(DdManager *manager, DdNode *set, guint64 x, guint64 y);

int adamant_zdd_var_swap(AdamantRuntime *runtime, unsigned long maxMem);

DdNode * adamant_zdd_make_var(DdManager *manager, guint64 x);

DdNode *adamant_zdd_range(AdamantRuntime * runtime, guint64 high, guint64 low);

void adamant_zdd_quit(AdamantStats *stats);

int adamant_zdd_headerinit(FILE * zddfile, AdamantRuntime *runtime );

DdNode *adamant_zdd_bdd_build_tuple(DdManager *manager, guint64 x, guint64 y);

DdNode *adamant_zdd_build_tuple(DdManager *manager, DdNode *set, guint64 x, guint64 y);
DdNode * adamant_zdd_build_fullcube(DdManager *manager, guint64 x, guint64 y);
static guint64 mask(guint64 v);
guint64 adamant_zdd_freemem(void);
void adamant_zdd_sizeout(DdManager * manager, guint64 opcount);
void adamant_zdd_subTableSizeOut(DdManager * manager,
                                 guint64 opcount);
int adamant_zdd_GetTupleTop2(DdManager * manager, DdNode * node,
				  guint64 * ptopX, guint64 * ptopY);
int adamant_zdd_GetTupleTop(DdManager * manager, DdNode * node,
				  guint64 * ptopX, guint64 * ptopY);
int adamant_zdd_GetTupleBottom(DdManager * manager, DdNode * node,
			    guint64 * pbottomX, guint64 * pbottomY);
int adamant_zdd_GetTupleBottom2(DdManager * manager, DdNode * node,
				guint64 * pbottomX, guint64 * pbottomY);

//! ZDD Hot Code Function Prototypes
void adamant_zdd_dinhot_finalize(AdamantRuntime *runtime, guint64 opcount);
void adamant_zdd_dinhot(AdamantRuntime *runtime, adamantHotManager *hotManager);

//! ZDD Slicing Function Prototypes
DdNode * adamant_zdd_varswap(DdManager * manager, DdNode * node);
DdNode * adamant_zdd_reverse_slice(DdManager * manager, DdNode * sliceNode, DdNode * targetNode);
DdNode * adamant_zdd_reverse_slice_nounion(DdManager * manager, DdNode * sliceNode, DdNode * targetNode);
DdNode * adamant_zdd_reverse_sliceUP(DdManager * manager, DdNode * sliceNode, DdNode * targetNode);
DdNode * adamant_zdd_reverse_sliceITE(DdManager * manager, DdNode * sliceNode, DdNode * targetNode);

DdNode * adamant_zdd_yDC(DdManager *manager, DdNode * node);
DdNode * adamant_zdd_xDC(DdManager *manager, DdNode * node);
DdNode * adamant_zdd_abstractX(DdManager *manager, DdNode * sliceNode);
DdNode * adamant_zdd_abstractY(DdManager *manager, DdNode * sliceNode);
DdNode * adamant_zdd_iterReverse_slice(DdManager * manager, DdNode * sliceNode,
				       DdNode * targetNode, guint64 stopCount);
DdNode * adamant_zdd_dindin_forward_slice(DdManager * manager, DdNode * sliceDD,
					  DdNode * dindinDD);
DdNode * adamant_zdd_QuickDeadFilter(DdManager * dd_manager, 
				     DdNode * dinSelDD, 
				     DdNode * dd_dindin);

DdNode * adamant_zdd_BuildIterDinDinReverseSlice(DdManager * dd_manager,
						 DdNode * sliceNode, 
						 DdNode * local_dindinDD,
						 guint64 resDepth,
						 guint64 stopCount);

DdNode * adamant_zdd_DeadReadyFilterSelection(DdManager * dd_manager, 
					      DdNode * dd_dinrdy, 
					      DdNode * dd_dindin,
					      DdNode * dinSelDD);

DdNode * adamant_zdd_BuildIterDinDinForwardSlice(DdManager * dd_manager,
						 DdNode * sliceNode, 
						 DdNode * local_dindinDD, 
						 guint64 stopCount);
DdNode * adamant_zdd_DinDinForwardSlice(DdManager * dd_manager,DdNode * sliceDD, DdNode * targetDD);
DdNode * adamant_zdd_DinDinReverseSlice(DdManager * manager, DdNode * sliceDD, DdNode * targetDD);

DdNode * adamant_zdd_DeadSlicer(DdManager * dd_manager,
				DdNode * dd_dindin_sel,
				DdNode * dd_din_add_back,
				int iterations);

void adamant_zdd_addRegions(const char * fileName);

ddregion * adamant_zdd_regionsOverlap(ddregion * regionA, 
				      ddregion * regionB);

DdNode * adamant_zdd_DdRegion(DdManager * manager, 
			      DdNode * ddnode, 
			      ddregion * region);
void adamant_zdd_DinSinHot(DdManager * manager, DdNode * ddnode, 
			   DdNode * dinHotDD, guint64 din, guint64 sin);

void adamant_zdd_DinHot(DdManager * manager, DdNode * dinHotDD, 
			guint64 din, guint64 sin, guint64 hot);

void adamant_zdd_SinPrint(DdManager * manager, DdNode * ddnode, 
			  guint64 din, guint64 sin);
G_END_DECLS

#endif
