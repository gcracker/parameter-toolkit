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

/*****************************************************************************
 * adamant_runtime.c
 ****************************************************************************/
#include "adamant_runtime.h"

#include <stdlib.h>
#include <tmtoper_x86.h>
#include <tmtoper_ia64.h>
#include <tmtoper_ppc64.h>

#include "adamant_branch_prediction.h"
#include "adamant_optimize.h"

/*
 * Local function declarations
 */
static gint u64CmpFcn(foint a, foint b);

AdamantRuntime *adamant_new_runtime()
{
  AdamantRuntime *runtime = g_new(AdamantRuntime, 1);

  runtime->config = adamant_new_config();
  runtime->stats = adamant_new_stats();
  runtime->tmtrc = NULL;
  runtime->reg_info = NULL;
  runtime->max_regs = 0;
  runtime->meminfo_pgtbl = NULL;
  runtime->wrt = NULL;
  runtime->ignore_regs = NULL;
  runtime->finish_time = 0;
  runtime->cdstack = NULL;
  runtime->control_dep_times = NULL;
  runtime->branch_target_dist = NULL;
  runtime->branch_pred_accuracy = NULL;
  runtime->bhr=0;
  runtime->pht = NULL;
  runtime->opti = NULL;
  runtime->split_sopers = NULL;
  runtime->schedule_store_context = NULL;
  runtime->src_sched_info_array = NULL;

  runtime->last_mispredicted_branch.din_src = 0;
  runtime->last_mispredicted_branch.ready = 0;
  runtime->last_mispredicted_branch.thread_id = 0;

  runtime->done = 0; /* Set to one to cleanly end adamantium */

  return (runtime);
}

void adamant_free_runtime(AdamantRuntime *runtime)
{
  if (runtime != NULL) {
    adamant_free_stats(runtime->stats);
    adamant_free_config(runtime->config);

    if(!(runtime->config->use_zdds))
        {
            //! cleanup BDD work
            adamant_bdd_quit(runtime->stats);
        }
    else
        {
            //! cleanup ZDD work
            adamant_zdd_quit(runtime->stats);
        }

    if (runtime->tmtrc != NULL)
      tmt_readcontext_free(runtime->tmtrc);

    // free the register array
    g_free(runtime->reg_info);

    // free the page table
    MemInfoPageTable_destroy(runtime->meminfo_pgtbl);

    // itanium-specific
    if (runtime->ia64_stfp != NULL)
      tmt_ia64sidetrace_read_close(NULL, runtime->ia64_stfp);

    // free array of ignored registers
    g_free(runtime->ignore_regs);

    // free the pattern history table array
    if(runtime->pht)
      g_array_free(runtime->pht, TRUE);
    
    // free the schedule storing context
    if(runtime->schedule_store_context)
      svt_context_write_destroy(runtime->schedule_store_context);

    // free the src sched info array
    g_array_free(runtime->src_sched_info_array, TRUE);

    g_free(runtime);
  }
}

void adamant_initialize_runtime(AdamantRuntime *runtime)
{
  AdamantConfig *config = runtime->config;

  if (config->program == NULL) {
    g_printerr("Must specify a program file\n");
    g_printerr("Use -h or --help for options\n");
    exit(EXIT_FAILURE);
  }

  if (config->trace == NULL) {
    g_printerr("Must specify a trace file\n");
    g_printerr("Use -h or --help for options\n");
    exit(EXIT_FAILURE);
  }

  runtime->tmtrc = tmt_readcontext_new(config->trace, config->program);
  if(runtime->tmtrc == NULL) {
    g_error("Unable to create read context from trace file %s "
	    "and program file %s", config->trace, config->program);
  }

  // setup register array
  // todo: reg_info array should be a properly determined size
  runtime->reg_info = g_new0(SchedInfo, 0x10000);
  runtime->max_regs = 0x10000;

  // setup memory pagetable
  runtime->meminfo_pgtbl = MemInfoPageTable_new();

  {
    gboolean *ignore_regs = g_new0(gboolean,runtime->max_regs);
    guint64 *data = (guint64*)config->ignore_regs->data;
    guint len = config->ignore_regs->len;
    for (;len;) {
      guint val = (guint)data[--len];
      g_assert(val < runtime->max_regs);
      ignore_regs[val]=1;
    }
    runtime->ignore_regs = ignore_regs;
  }

  adamant_initialize_stats(runtime);

  if(!(config->use_zdds))
      {
          //!initialize BDD work
          adamant_initialize_bddstats(runtime);
      }
  else
      {
          //!initialize ZDD work
          adamant_initialize_zddstats(runtime);
      }

  if(config->respect_control) {
    runtime->cdstack = g_new(AdamantCDStack,1);	  
    runtime->cdstack->stack = NULL;
    runtime->cdstack->dopop = 0;
    adamant_cd_read_deps(config, runtime);
  }

  if(config->predict_branches) {
    guint i;
    adamant_bp_read_distribution(config, runtime);
    runtime->branch_pred_accuracy = g_hash_table_new(g_direct_hash,
						     g_direct_equal);
    runtime->bhr = 0;
    runtime->pht = g_array_sized_new(FALSE, TRUE, sizeof(guint8),PHT_SIZE);
    for(i=0;i<PHT_SIZE;++i) {
      g_array_index(runtime->pht,guint8,i) = PHT_INIT;
    }

    runtime->bp_random_state[0] = 0;
    runtime->bp_random_state[1] = 0;
    runtime->bp_random_state[2] = 0;
  }

  adamant_optimize_create_runtime(runtime);
  
  if(config->split_post_increment)
    adamant_split_opers(runtime);

  if(config->store_schedule) {
    runtime->schedule_store_context = 
      svt_context_write_create(config->schedule_file,
			       config->schedule_block_size);
  }

  // array of source schedule info -- accomodates variable number of sources
  runtime->src_sched_info_array = g_array_new(0, 0, sizeof(SchedInfo*));

  // Itanium-specific initialization
  if(config->ia64_translate) {
    runtime->ia64_stfp =
      tmt_ia64sidetrace_read_open(config->ia64_sidetrace);
    if(!runtime->ia64_stfp) {
      printf("Unable to open sidetrace file %s\n", 
	     config->ia64_sidetrace);
      exit(-1);
    }
    tmt_ia64_readcontext_type_opers(runtime->tmtrc);
  } else {
    runtime->ia64_stfp = NULL;
  }
  ia64_init_regstackinfo(&runtime->ia64_reg_stack);

  // PowerPC-specific initialization
  if(config->ppc64_translate) {
    tmt_ppc64_readcontext_type_opers(runtime->tmtrc);
  }

  // X86-specific initialization
  if(config->x86_translate) {
    tmt_x86_readcontext_type_opers(runtime->tmtrc);
  }

  // Multi window initializtaion
  if(config->win.w_num > 0) {
    runtime->wrt = g_new0(AdamantWindowRuntime, 1);
    adamant_window_runtime_init(runtime->wrt, &config->win);
  }
}

gint u64CmpFcn(foint a, foint b)
{
  return ((a.u64<b.u64) ? -1 : ((a.u64==b.u64)?0:1));
}

void HeapTypePrint(foint a)
{   
  printf("%"G_GUINT64_FORMAT" ", a.u64); 
}

