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

#ifndef ADAMANT_OPTIMIZE_H
#define ADAMANT_OPTIMIZE_H

#include <glib.h>
#include "adamant_optimize_type.h"
#include "adamant_runtime.h"

/* IVE stands for induction variable expansion */

static void
adamant_optimize_create_runtime(AdamantRuntime *runtime)
{
  AdamantOptimizeRuntime *oruntime = NULL;

  if(runtime->config->opti.ive) {
    oruntime = g_new(AdamantOptimizeRuntime, 1);
    runtime->opti = oruntime;
  } else {
    runtime->opti = NULL;
  }

  if(runtime->config->opti.ive) {
    oruntime->IVE_node_allocator = 
      g_mem_chunk_create(AdamantIVENode, 32, G_ALLOC_AND_FREE);
    oruntime->induction_vars = 
      g_array_new(FALSE, TRUE, sizeof(AdamantIVENode*));
    oruntime->memory_induction_vars = PageTable64_new();
    oruntime->IVE_gc_interval = ADAMANT_IVE_GC_INTERVAL;
  }/* else {
      oruntime->IVE_node_allocator = NULL;
      oruntime->induction_vars = NULL;
  }
   */
}

static inline AdamantIVENode *
adamant_IVE_alloc_node(AdamantRuntime *runtime)
{
  return g_chunk_new0(AdamantIVENode, runtime->opti->IVE_node_allocator);
}

static inline void *
adamant_IVE_unref(AdamantRuntime *runtime, AdamantIVENode *node)
{
  if(node == NULL) return NULL;

  node->ref_count--;
  if(node->ref_count == 0) {
    adamant_IVE_unref(runtime, node->parent);
    g_chunk_free(node, runtime->opti->IVE_node_allocator);
  }
  return NULL;
}

static inline void
adamant_IVE_ref(AdamantRuntime *runtime, AdamantIVENode *node)
{
  if(node == NULL) return;
  node->ref_count++;
}

static inline void
adamant_IVE_reset(AdamantRuntime *runtime, guint reg, 
		  guint64 time, guint64 din, guint thread)
{
  AdamantIVENode *node;

  if(reg >= runtime->opti->induction_vars->len) {
    g_array_set_size(runtime->opti->induction_vars, reg + 1);
    node = NULL;
  } else {
    node = g_array_index(runtime->opti->induction_vars, AdamantIVENode*, reg);
  }

  /* If the node is only referenced from the registers directly, then
     we can reuse the node, otherwise, we should allocate a new one */
  if(node != NULL) {
    if(node->ref_count == 1) {
      node->parent = adamant_IVE_unref(runtime, node->parent);
    } else {
      node = adamant_IVE_unref(runtime, node);
    }
  }

  if(node == NULL) {
    node = adamant_IVE_alloc_node(runtime);
    adamant_IVE_ref(runtime, node);
    g_array_index(runtime->opti->induction_vars, AdamantIVENode*, reg) = node;
  }

  node->sched_info.ready = time;
  node->sched_info.din_src = din;
  node->sched_info.thread_id = thread;
}

static inline void 
adamant_IVE_garbage_collect_chain(AdamantRuntime *runtime, 
				  AdamantIVENode *node)
{
  guint unroll_factor = runtime->config->opti.unroll_factor;
  guint chain_length = 0;

  for(; node; node = node->parent) {
    chain_length++;

    /* If there is a side entrance to the chain at this point, we
       can't be certain that we are on the shortest chain, so just
       reset our chain length */
    if(node->ref_count != 1) 
      chain_length = 1;
    
    /* If items beyond this point can never be referenced, we might
       as well unlink ourselves */
    if(chain_length == unroll_factor) {
      node->parent = adamant_IVE_unref(runtime, node->parent);
      break;
    }
  }
}

static inline void 
adamant_IVE_garbage_collect_walk_mem(AdamantRuntime *runtime, 
				     int depth, int max_depth, void *node)
{
  guint32 i;

  if(depth < max_depth) {
    PageTable64InteriorNode *int_node = (PageTable64InteriorNode *)node;
    for(i = 0; i < PT64_INTERIOR_PAGE_LEN; i++) {
      if(int_node->nodes[i] != NULL) {
	adamant_IVE_garbage_collect_walk_mem(runtime,
					     depth + 1,
					     max_depth,
					     int_node->nodes[i]);
      }
    }
  } else {
    PageTableEntry *ptentry = (PageTableEntry *)node;
    for(i = 0; i < ENTRY_LINE_SIZE; i++) {
      AdamantIVENode *ddnode = (AdamantIVENode *)ptentry->data[i];
      adamant_IVE_garbage_collect_chain(runtime, ddnode);
    }
  }
}

static inline void
adamant_IVE_garbage_collect(AdamantRuntime *runtime)
{
  guint i;
  GArray *regs = runtime->opti->induction_vars;

  for(i = 0; i < regs->len; i++) {
    AdamantIVENode *node = g_array_index(regs, AdamantIVENode*, i);
    adamant_IVE_garbage_collect_chain(runtime, node);
  }

  adamant_IVE_garbage_collect_walk_mem(runtime, 0,
				       runtime->opti->memory_induction_vars->depth,
				       &runtime->opti->memory_induction_vars->root);

  g_mem_chunk_clean(runtime->opti->IVE_node_allocator);
  g_mem_chunk_print(runtime->opti->IVE_node_allocator);
}


static inline void 
adamant_IVE_chain(AdamantRuntime *runtime, guint dest_reg, 
		  guint src_reg, 
		  guint64 time, guint64 din, guint thread)
{
  AdamantIVENode *node, *parent;

  if(src_reg >= runtime->opti->induction_vars->len) {
    parent = NULL;
  } else {
    parent = g_array_index(runtime->opti->induction_vars, 
			   AdamantIVENode*, src_reg);
  }

  node = adamant_IVE_alloc_node(runtime);
  node->parent = parent;
  node->sched_info.ready = time;
  node->sched_info.din_src = din;
  node->sched_info.thread_id = thread;
  adamant_IVE_ref(runtime, node);
  adamant_IVE_ref(runtime, parent);

  /* If the array is too small, make it bigger.  If its not too small,
     make sure we unreference the node already in the array */
  if(dest_reg >= runtime->opti->induction_vars->len) {
    g_array_set_size(runtime->opti->induction_vars, dest_reg + 1);
  } else {
    AdamantIVENode *old;
    old = g_array_index(runtime->opti->induction_vars, AdamantIVENode*,
			dest_reg);
    old = adamant_IVE_unref(runtime, old);
  }

  g_array_index(runtime->opti->induction_vars, AdamantIVENode*, 
		dest_reg) = node;

  /* Every ADAMANT_IVE_GC_INTERVAL chain requests, lets calculate
     depths and clean up memory */
  if(runtime->opti->IVE_gc_interval == 0) {
    adamant_IVE_garbage_collect(runtime);
    runtime->opti->IVE_gc_interval = ADAMANT_IVE_GC_INTERVAL;
  } else {
    runtime->opti->IVE_gc_interval--;
  }
  
}

static inline void 
adamant_IVE_link(AdamantRuntime *runtime, guint dest_reg, guint src_reg)
{
  AdamantIVENode *parent;

  if(src_reg >= runtime->opti->induction_vars->len) {
    parent = NULL;
  } else {
    parent = g_array_index(runtime->opti->induction_vars,
			   AdamantIVENode*, src_reg);
  }

  adamant_IVE_ref(runtime, parent);

  /* If the array is too small, make it bigger.  If its not too small,
     make sure we unreference the node already in the array */
  if(dest_reg >= runtime->opti->induction_vars->len) {
    g_array_set_size(runtime->opti->induction_vars, dest_reg + 1);
  } else {
    AdamantIVENode *old;
    old = g_array_index(runtime->opti->induction_vars, AdamantIVENode*, 
			dest_reg);
    old = adamant_IVE_unref(runtime, old);
  }

  g_array_index(runtime->opti->induction_vars, AdamantIVENode*, 
		dest_reg) = parent;
}

static inline void 
adamant_IVE_link_to_mem(AdamantRuntime *runtime, 
			guint64 ea, guint32 size, guint src_reg)
{
  guint32 i;
  AdamantIVENode *parent;
  PageTableEntry *ptentry;
  guint64 ea_line = ea & ENTRY_KEY_LINE_MASK;
  guint64 ea_index = ea & ENTRY_KEY_INDEX_MASK;

  if(src_reg >= runtime->opti->induction_vars->len) {
    parent = NULL;
  } else {
    parent = g_array_index(runtime->opti->induction_vars,
			   AdamantIVENode*, src_reg);
  }

  for(i = 0; i < size; i++)
    adamant_IVE_ref(runtime, parent);

  /* Check to see if we already have this thing in the PageTable.  If
     not, add it.  If its already there, make sure we unreference the
     node already in the pagetable */

  while (size) {
    if (!(ptentry=PageTable64_lookup(runtime->opti->memory_induction_vars, 
				     ea_line))) {
      PageTableEntry *tmp=g_new0(PageTableEntry,1); 
      tmp->key = ea_line;
      PageTable64_add(runtime->opti->memory_induction_vars, ea_line, tmp);
      ptentry = tmp;
    }
    do {
      if(ptentry->data[ea_index] != NULL) 
	adamant_IVE_unref(runtime, (AdamantIVENode*)ptentry->data[ea_index]);
      ptentry->data[ea_index] = parent;

      --size; ++ea_index;
      if (ea_index >= ENTRY_LINE_SIZE) {
	ea_index = 0;
	ea_line += ENTRY_LINE_SIZE;
	break;
      }
    } while (size);
  }
}

static inline void
adamant_IVE_link_from_mem(AdamantRuntime *runtime, guint dest_reg, 
			  guint64 ea, guint32 size)
{
  guint32 i;
  AdamantIVENode *parent;
  PageTableEntry *ptentry;
  guint64 ea_line = ea & ENTRY_KEY_LINE_MASK;
  guint64 ea_index = ea & ENTRY_KEY_INDEX_MASK;

  if (!(ptentry=PageTable64_lookup(runtime->opti->memory_induction_vars, 
				   ea_line))) {
    parent = NULL;
  } else {
    parent = (AdamantIVENode*)ptentry->data[ea_index];
  }
  adamant_IVE_ref(runtime, parent);

  /* If the array is too small, make it bigger.  If its not too small,
     make sure we unreference the node already in the array */
  if(dest_reg >= runtime->opti->induction_vars->len) {
    g_array_set_size(runtime->opti->induction_vars, dest_reg + 1);
  } else {
    AdamantIVENode *old;
    old = g_array_index(runtime->opti->induction_vars, 
			AdamantIVENode*, dest_reg);
    old = adamant_IVE_unref(runtime, old);
  }

  g_array_index(runtime->opti->induction_vars, 
		AdamantIVENode*, dest_reg) = parent;
}


static inline SchedInfo*
adamant_IVE_read(AdamantRuntime *runtime, guint reg, guint distance)
{
  /* The basic idea here is to follow the dependence chain for reg
     back runtime->config->opti.unroll_factor many links.  That node
     will act as our input dependence rather than the tail of the
     dependence chain (since all intervening nodes could be folded). */
  AdamantIVENode *node;
  guint i;

  if(reg > runtime->opti->induction_vars->len)
    adamant_IVE_reset(runtime, reg, 0, 0, 0);

  g_assert(reg < runtime->opti->induction_vars->len);

  node = g_array_index(runtime->opti->induction_vars, AdamantIVENode*, reg);
  if(node == NULL) {
    adamant_IVE_reset(runtime, reg, 0, 0, 0);
    node = g_array_index(runtime->opti->induction_vars, AdamantIVENode*, reg);
    g_assert(node != NULL);
  }

  for(i = 1; node->parent != NULL && i < distance; i++, node = node->parent);
  return &node->sched_info;
}

#endif /* ADAMANT_OPTIMIZE_H */

