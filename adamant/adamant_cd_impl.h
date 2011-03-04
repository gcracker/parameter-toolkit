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

#ifndef ADAMANT_CDSTACK_IMPL_H
#define ADAMANT_CDSTACK_IMPL_H

#include "adamant.h"
#include "adamant_runtime.h"
#include "tmt.h"
#include "pagetable64.h"

static inline SchedInfo*
cdstack_peek(AdamantRuntime *runtime)
{
  if(runtime->cdstack->stack)
    return &runtime->cdstack->stack->info.info;
  else
    return NULL;
}

static inline void 
cdstack_pop(AdamantRuntime *runtime, TmtOper *oper)
{
  AdamantCDItem *item;
  guint64 ip;
  
  while(runtime->cdstack->stack) {
    item = runtime->cdstack->stack;
    ip = item->ip;
    runtime->cdstack->stack = item->prev;
    g_free(item);
    
    if(ip == oper->soper->ip) {
      return;
    }
  }

  g_error("Popping continued until stack was empty");
}

static inline void 
cdstack_push(AdamantRuntime *runtime, TmtOper *oper, 
	     guint64 op_number, guint64 cd_din_src,
	     guint window, guint64 time)
{
  AdamantCDItem *item;

  item = g_new(AdamantCDItem, 1);
  item->info.cd_din_src = op_number;
  item->info.info.ready = time;
  item->info.info.thread_id = window;
  item->info.info.din_src = cd_din_src;
  item->ip = oper->soper->ip + oper->soper->instr_size;
  item->prev = runtime->cdstack->stack;

  runtime->cdstack->stack = item;
}

static inline void
cdstack_duplicate_top(AdamantRuntime *runtime)
{
  AdamantCDItem *item;

  printf("Duplicating %llx\n", runtime->cdstack->stack->ip);

  item = g_new(AdamantCDItem, 1);
  item->info = runtime->cdstack->stack->info;
  item->ip = runtime->cdstack->stack->ip;
  item->prev = runtime->cdstack->stack;

  runtime->cdstack->stack = item;
}

static inline void
preupdate_cdstack(AdamantRuntime *runtime, TmtOper *oper)
{
  if(oper->soper->ip == runtime->config->setjmp_addr) {
    g_assert(runtime->cdstack->dopop == 0);
    cdstack_duplicate_top(runtime);
  }

  if(runtime->cdstack->dopop) {
    runtime->cdstack->dopop = 0;
    cdstack_pop(runtime, oper);
  }
}

static inline void
update_cdstack(AdamantRuntime *runtime, TmtOper *oper, 
	       guint64 op_number, guint64 cd_din_src,
	       guint window, guint64 time)
{
  TmtStaticOper *soper;
  gboolean is_call, is_return, is_branch;

  soper = oper->soper;

  is_branch   = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH);
  is_call   = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_CALL);
  is_return = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_RETURN);

  if (is_branch && is_call && 
      (oper->taken || runtime->config->x86_translate)) {
    cdstack_push(runtime, oper, op_number, cd_din_src, window, time);
  } else if(is_branch && is_return && 
	    (oper->taken || runtime->config->x86_translate)) {
    runtime->cdstack->dopop = 1;
  }
}

static inline CDSchedInfo **
lookup(AdamantRuntime *runtime, guint64 ip) 
{
  guint64 aligned_addr, index_addr;
  PageTableEntry *entry;

  aligned_addr = ip & ENTRY_KEY_LINE_MASK;
  index_addr = ip & ENTRY_KEY_INDEX_MASK;
  entry = PageTable64_lookup(runtime->control_deps, aligned_addr);

  if(entry == NULL) {
    return NULL;
  }

  return (CDSchedInfo**)entry->data[index_addr];
}

static inline SchedInfo*
check_control_deps(AdamantRuntime *runtime, TmtOper *oper)
{
  CDSchedInfo **cds;
  CDSchedInfo *time;
  SchedInfo *stack_time;
  guint64 max_op_number = 0;

  cds = lookup(runtime, oper->soper->ip);

  time = NULL;
  if(cds != NULL) {
    int i;
    guint64 dep;
    CDSchedInfo *info;

    i = 0;
    while(cds[i]) {
      info = cds[i];
      if(info->cd_din_src > max_op_number) {
	time = info;
	max_op_number = info->cd_din_src;
      }
      i++;
    }
  }

  stack_time = cdstack_peek(runtime);
  if(stack_time != NULL && 
     (time == NULL || stack_time->ready > time->info.ready)) 
    return stack_time;
  else
    return &time->info;
}

static inline void
update_cdinfo(AdamantRuntime *runtime, TmtOper *oper, 
	      guint64 op_number, guint window,
	      guint64 ready_time, guint64 cd_time,
	      guint64 cd_din, gboolean predicted_correctly)
{
  TmtStaticOper *soper;
  gboolean is_branch;
  guint64 time;
  CDSchedInfo *info;

  guint64 mispred_din_src;

  soper = oper->soper;

  is_branch = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH);

  if(!is_branch) return;
  if(predicted_correctly) {
    time = cd_time;
    mispred_din_src = cd_din;
  } else {
    if(runtime->config->inorder_branches) {
      SchedInfo mispred_info;
      mispred_info.ready = ready_time;
      mispred_info.din_src = op_number;
      mispred_info.thread_id = window;
      
      if(runtime->wrt) {
	AdamantWindow * w = 
	  adamant_window_runtime_index(runtime->wrt, window);
	w->last_mispredicted_branch = mispred_info;
      } else
	runtime->last_mispredicted_branch = mispred_info;
    }

    time = ready_time;
    mispred_din_src = op_number;
  }
  
  info = g_hash_table_lookup(runtime->control_dep_times, &soper->ip);
  if(info != NULL) {
    info->cd_din_src = op_number;
    info->info.ready = time;
    info->info.din_src = mispred_din_src;
    info->info.thread_id = window;
  }
  
  update_cdstack(runtime, oper, op_number, mispred_din_src, window, time);
}

#endif /* ADAMANT_CDSTACK_IMPL_H */

