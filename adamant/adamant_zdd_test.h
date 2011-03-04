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

#ifndef __ADAMANT_ZDD_TEST_H__
#define __ADAMANT_ZDD_TEST_H__

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
enum SliceMethod{
  SLINTERSECT,
  SLITE,
  SLUP,
  SLEND
};

/*
 * Functions declarations
 */

int adamant_zddtest_interval();
DdNode * adamant_zddtest_initnode(DdManager * manager);
DdNode * adamant_zddtest_initInvNode(DdManager * manager);
int adamant_zddtest_zddstore();
DdNode * adamant_zddtest_initnode(DdManager * manager);
DdManager * adamant_zddtest_initmanager(int * zdd_order);
DdManager * adamant_zddtest_initmanagerSlots(int * zdd_ordr, 
					     guint64 slotsMulti,
					     guint64 freeMemory);
int adamant_zddtest_deadCodeRemoval(guint64 slotsMulti, GString * inputs);
int adamant_zddtest_zddslice();
int adamant_zddtest_varswap();
int adamant_zddtest_error(const char * error_str);
int adamant_zddtest_printorder(DdManager * manager);
DdNode * adamant_zddtest_initSliceNode(DdManager * manager);
int adamant_zddtest_printdd(DdManager * manager, DdNode * node, const char * filename);
int adamant_zddtest_rdydinslice(GString * dinrdyFiles);
int adamant_zddtest_dinsinslice(GString * dinsinFiles);
int adamant_zddtest_dintop(GString * dinrdyFiles);
int adamant_zddtest_quickdead(GString * ddFiles);
int adamant_zddtest_tophot(guint64 slotsMulti, GString * ddFiles);
int adamant_zddtest_deathSlice();
int adamant_zddtest_deathSlice(guint64 slotsMulti, GString * ddfiles);
int adamant_zddtest_reverseSliceMethodTest(guint64 slotsMulti, GString * inputs);
double adamant_zddtest_reverseSliceMethod(int method, 
					  guint32 randSeed,					  
					  guint64 slotsMulti,  
					  guint64 freeMemory, 
					  gchar ** inputArray);
int adamant_zddtest_performance(guint64 slotsMulti, GString * inputs);
int adamant_zddtest_parallelAdd(DdManager * manager, DdNode * node, GQueue * queue);
int adamant_zddtest_sinStats(guint64 slotsMulti, GString * ddFiles);
int adamant_zddtest_dumpRegionInfo(GQueue * regionQueue);
int adamant_zddtest_naivePerformance(guint64 slotsMulti, GString * inputs);
int adamant_zddtest_sinRegion(guint64 slotsMulti, GString * ddFiles);

G_END_DECLS

#endif
