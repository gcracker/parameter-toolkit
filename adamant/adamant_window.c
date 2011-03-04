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
 * adamant_window.c
 ****************************************************************************/
#include <string.h>

#include "adamant_window.h"
#include "adamant_meminfo_pagetable.h" // SchedInfo
#include "misc.h"

#define ADAMANT_WINDOW_OOO(window) ((AdamantWindowOoO *) (window))
#define ADAMANT_WINDOW_ROB(window) ((AdamantWindowRob *) (window))

/**
 * Static variables
 */ 
static guint adamant_window_next_id = 0;

/**
 * Local function declarations
 */
static gint u64CmpFcn(foint a, foint b);

static void
adamant_window_ooo_insert(AdamantWindowOoO * ooo, guint64 ready);

static void
adamant_window_rob_insert(AdamantWindowRob * rob, guint64 ready);

/* ========================================================================
 *             adamant window base class implementation
 * ========================================================================
 */
static void
adamant_window_init(AdamantWindow * window, 
		    AdamantWindowType windowtype,
		    guint windowsize)
{
  window->type = windowtype;
  window->id = adamant_window_next_id++;
  window->size = windowsize;
  window->head = 0;
  window->tail = 0;
  window->sum = 0;
  window->last_mispredicted_branch.ready = 0;
  window->last_mispredicted_branch.din_src = 0;
  window->last_mispredicted_branch.thread_id = 0;
}

void
adamant_window_insert(AdamantWindow * window, guint64 ready)
{
  switch(window->type) {
  case ADAMANT_WINDOW_TYPE_OOO:
    adamant_window_ooo_insert((AdamantWindowOoO*) window, ready);
    break;
  case ADAMANT_WINDOW_TYPE_ROB:
    adamant_window_rob_insert((AdamantWindowRob*) window, ready);
    break;
  }  
}

/* ========================================================================
 *             adamant out-of-order window implementation
 * ========================================================================
 */
void
adamant_window_ooo_init(AdamantWindowOoO * ooo, guint windowsize)
{
  foint val;

  // init base
  adamant_window_init(&ooo->w_base, ADAMANT_WINDOW_TYPE_OOO, windowsize);

  // init self
  if (ooo->ooo_heap)
    HeapFree(ooo->ooo_heap);
  ooo->ooo_heap = HeapAlloc(windowsize, &u64CmpFcn);

  val.u64 = 0;
  do { HeapInsert(ooo->ooo_heap, val); } while (--windowsize);
}

void
adamant_window_ooo_destroy(AdamantWindowOoO * ooo)
{
  if (ooo->ooo_heap)
    HeapFree(ooo->ooo_heap);
}

void
adamant_window_ooo_insert(AdamantWindowOoO * ooo, guint64 ready)
{
  foint val;
  // value at head is lost
  val = HeapNext(ooo->ooo_heap);
  // insert new value
  val.u64 = ready;
  HeapInsert(ooo->ooo_heap, val);
  // set new head and tail
  ooo->w_base.head = HeapPeek(ooo->ooo_heap).u64;
  if ( val.u64 > ooo->w_base.tail ) {
    ooo->w_base.tail = val.u64;
  }
}

gint u64CmpFcn(foint a, foint b)
{
  return ((a.u64<b.u64) ? -1 : ((a.u64==b.u64)?0:1));
}

/* ========================================================================
 *             adamant reorder buffer (ROB) implementation
 * ========================================================================
 */
void
adamant_window_rob_init(AdamantWindowRob * rob, guint windowsize)
{
  // init base
  adamant_window_init(&rob->w_base, ADAMANT_WINDOW_TYPE_ROB, windowsize);

  // init self
  rob->ptr = 0;
  rob->rob = g_new0(guint64, windowsize);
}

void
adamant_window_rob_destroy(AdamantWindowRob * rob)
{
  g_free(rob->rob);
}

void
adamant_window_rob_insert(AdamantWindowRob * rob, guint64 ready)
{
  // insert at head (head now becomes tail in circular buffer)
  rob->rob[rob->ptr] = MAX(ready, 
			   rob->rob[MOD_MINUS(rob->ptr, 1, rob->w_base.size)]);
  rob->w_base.tail = rob->rob[rob->ptr];

  // increment head
  rob->ptr = MOD_PLUS(rob->ptr, 1, rob->w_base.size);
  rob->w_base.head = rob->rob[rob->ptr];
}

/* ========================================================================
 *             adamant window runtime implementation
 * ========================================================================
 */
void
adamant_window_runtime_init(AdamantWindowRuntime * wrt,
			    AdamantWindowConfig * config)
{
  // copy the configuration
  memcpy(&wrt->config, config, sizeof(AdamantWindowConfig));
  wrt->windows = NULL;

  // allocate memory for specified type of window
  switch(wrt->config.w_type) {
  case ADAMANT_WINDOW_TYPE_OOO:
    wrt->windows = 
      (AdamantWindow *) g_new0(AdamantWindowOoO, wrt->config.w_num);
    break;
  case ADAMANT_WINDOW_TYPE_ROB:
    wrt->windows =
      (AdamantWindow *) g_new0(AdamantWindowRob, wrt->config.w_num);
    break;
  }

  // initialize windows
  switch(wrt->config.w_type) {
    guint i;
  case ADAMANT_WINDOW_TYPE_OOO:
    for (i=0; i<wrt->config.w_num; i++) {
      adamant_window_ooo_init(ADAMANT_WINDOW_OOO(adamant_window_runtime_index(wrt, i)),
			      wrt->config.w_size);
    }
    break;
  case ADAMANT_WINDOW_TYPE_ROB:
    for (i=0; i<wrt->config.w_num; i++) {
      adamant_window_rob_init(ADAMANT_WINDOW_ROB(adamant_window_runtime_index(wrt, i)),
			      wrt->config.w_size);
    }
    break;
  }
}

void
adamant_window_runtime_destroy(AdamantWindowRuntime * wrt)
{
  switch(wrt->config.w_type) {
    guint i;
  case ADAMANT_WINDOW_TYPE_OOO:
    for (i=0; i<wrt->config.w_num; i++) {
      adamant_window_ooo_destroy(ADAMANT_WINDOW_OOO(adamant_window_runtime_index(wrt, i)));
    }
    break;
  case ADAMANT_WINDOW_TYPE_ROB:
    for (i=0; i<wrt->config.w_num; i++) {
      adamant_window_rob_destroy(ADAMANT_WINDOW_ROB(adamant_window_runtime_index(wrt, i)));
    }
    break;
  }
}

guint
adamant_window_runtime_choose(AdamantWindowRuntime * wrt,
			      GArray *src_sched_info, 
			      guint src_sched_info_len,
			      guint communication_latency)
{
  guint i,j;
  guint64 cost;
  guint64 min_cost = G_MAXUINT64;
  guint best_w_id = 0;

  guint64 *ready_cost;
  //guint64 *head_cost;
  //guint64 *tail_cost;
  guint64 min_ready;

  /* Optimize the case of one window */
  if(wrt->config.w_num == 1) return 0;

  ready_cost = alloca(sizeof(guint64) * wrt->config.w_num);
  //head_cost = alloca(sizeof(guint64) * wrt->config.w_num);
  //tail_cost = alloca(sizeof(guint64) * wrt->config.w_num);  

  for(j = 0; j < wrt->config.w_num; j++) {
    AdamantWindow * w = adamant_window_runtime_index(wrt, j);

    // Initialize ready_cost, more processing later
    ready_cost[j] = MAX(adamant_window_head(w), 
			w->last_mispredicted_branch.ready);    

    // FIXME: Calculate head cost
    // head_cost[j] = rob->rob[MOD_PLUS(rob->ptr, 1, rob->size)] - 
    //   adamant_window_rob_head(rob);
    // head_cost[j] = 0;
  }

  // Compute the ready time of this instr for all ROBs
  for(i = 0; i < src_sched_info_len; i++) {
    SchedInfo * si = g_array_index(src_sched_info, SchedInfo*, i);
    guint64 local_ready = si->ready;

    for(j = 0; j < wrt->config.w_num; j++) {
      guint64 w_local_ready = local_ready;
      AdamantWindow * w = adamant_window_runtime_index(wrt, j);

      if (adamant_window_id(w) != si->thread_id) {
	w_local_ready += communication_latency;
      }
      ready_cost[j] = MAX(ready_cost[j], w_local_ready);
    }
  }

  // Compute the tail cost (uses ready times just computed)
  /*
  for(j = 0; j < wrt->config.w_num; j++) {
    AdamantWindow * w = adamant_window_runtime_index(wrt, j);

    if (adamant_window_tail(w) > ready_cost[j])
      tail_cost[j] = 0;
    else
      tail_cost[j] = ready_cost[j] - adamant_window_tail(w);
  }
  */

  // Find the minimum ready time across all ROBs
  min_ready = ready_cost[0];
  for(j = 1; j < wrt->config.w_num; j++)
    min_ready = MIN(min_ready, ready_cost[j]);

  // Subtract this minimum from all ROB readys to get a ready_cost
  for(j = 0; j < wrt->config.w_num; j++)
    ready_cost[j] -= min_ready;

  // Calculate total cost using weights
  for(j = 0; j < wrt->config.w_num; j++) {
    cost = ready_cost[j];
    /*
    cost =
      wrt->config.w_ready_weight * ready_cost[j] + 
      wrt->config.w_head_weight * head_cost[j] + 
      wrt->config.w_tail_weight * tail_cost[j];
    */

    if(cost < min_cost) {
      best_w_id = j;
      min_cost = cost;
    }
  }
  
  return (best_w_id);
}

