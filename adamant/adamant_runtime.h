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

#ifndef __ADAMANT_RUNTIME_H__
#define __ADAMANT_RUNTIME_H__

/*
 * Header dependences
 */
#include <glib.h>
#include <svt.h>
#include <tmt.h>

#include "adamant_meminfo_pagetable.h"
#include "adamant_config.h"
#include "heap.h"

/*
 * Forward declarations
 */
typedef struct _AdamantRuntime AdamantRuntime;

/*
 * Header circular dependences
 */
#include "adamant_stats.h"
#include "adamant_cd.h"
#include "adamant_optimize_type.h"
#include "adamant_window.h"
#include "adamant_bdd.h"

G_BEGIN_DECLS

/*
 * Type definitions
 */
struct _AdamantRuntime {
  AdamantConfig  *config;
  AdamantStats   *stats;

  TmtReadContext *tmtrc;

  SchedInfo * reg_info;
  guint32 max_regs;

  MemInfoPageTable * meminfo_pgtbl;

  AdamantWindowRuntime * wrt;

  gboolean *ignore_regs;
  guint64 finish_time;
      
  AdamantCDStack *cdstack;
  PageTable64 *control_deps;
  GHashTable *control_dep_times;

  GHashTable *branch_target_dist;
  GHashTable *branch_pred_accuracy;
  guint64 bhr;//BRANCH HISTORY REGISTER
  GArray *pht;//PATTERN HISTORY TABLE
  unsigned short bp_random_state[3];

  AdamantOptimizeRuntime *opti;

  GHashTable *split_sopers;

  SVTContext *schedule_store_context;

  GArray * src_sched_info_array;

  // Itanium-specific fields
  IA64RegStackInfo ia64_reg_stack;
  TMTFILE *ia64_stfp; /* File pointer for the IA64 side trace */

  SchedInfo last_mispredicted_branch; /* This field is superceded by the ones in window if windows are being used */
  int done;
};

/*
 * Function declarations
 */
AdamantRuntime *adamant_new_runtime();
void adamant_free_runtime(AdamantRuntime *runtime);
void adamant_initialize_runtime(AdamantRuntime *runtime);

void adamant_split_opers(AdamantRuntime *runtime);


G_END_DECLS

#endif


