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
 * adamant_window.h
 ****************************************************************************/
#ifndef __ADAMANT_WINDOW_H__
#define __ADAMANT_WINDOW_H__

#include <gmp.h>
#include <glib.h>
#include "heap.h"
#include "adamant_meminfo_pagetable.h"

G_BEGIN_DECLS

typedef enum
{
  ADAMANT_WINDOW_TYPE_OOO,
  ADAMANT_WINDOW_TYPE_ROB
} AdamantWindowType;


typedef struct _AdamantWindow AdamantWindow;
struct _AdamantWindow
{
  AdamantWindowType type;
  guint id;
  guint size;
  guint64 head;
  guint64 tail;
  guint64 sum;
  SchedInfo last_mispredicted_branch;
};

/**
 * Get ready time at head of window
 */
#define adamant_window_head(window) ((window)->head)

/**
 * Get ready time at tail of window
 */
#define adamant_window_tail(window) ((window)->tail)

/**
 * Get window id
 */ 
#define adamant_window_id(window) ((window)->id)

/**
 * Get window size
 */ 
#define adamant_window_size(window) ((window)->size)

/**
 * Insert into window (causes value at head to be consumed)
 */
void
adamant_window_insert(AdamantWindow * window, guint64 ready);

/**
 * Reorder buffer (ROB).  Simulates a reorder buffer implemented as a 
 * circular buffer FIFO queue.
 */
typedef struct _AdamantWindowRob AdamantWindowRob;
struct _AdamantWindowRob
{
  AdamantWindow w_base;
  guint ptr;
  guint64 * rob;
};

/**
 * Adamantium out-of-order instruction window.  Simulates an instruction 
 * issue window by keeping track of the ready times of all the instructions 
 * in the window.  Inserting a new instruction into the window replaces the 
 * instruction with the smallest ready time.
 *
 * The instruction window is implemented using a binary heap, or 
 * priority queue.  The top element of the heap is always the smallest,
 * and so removal done in O(1) time.  Inserting into the heap is a 
 * O(log(N)) time function.  Other, faster heap implementations exist and
 * should be considered for use in future versions of the instruction
 * window.
 */
typedef struct _AdamantWindowOoO AdamantWindowOoO;
struct _AdamantWindowOoO
{
  AdamantWindow w_base;
  HEAP * ooo_heap;
};

/**
 * Adamantium instruction window configuration parameters.  Currently models
 * only symmetric windows
 */
typedef struct _AdamantWindowConfig AdamantWindowConfig;
struct _AdamantWindowConfig
{
  AdamantWindowType w_type;
  guint w_num;
  guint w_size;
  guint w_ready_weight;
  guint w_head_weight;
  guint w_tail_weight;
};

/**
 * Adamantium window runtime.  Partitions instructions among 
 * multiple instruction windows.
 */
typedef struct _AdamantWindowRuntime AdamantWindowRuntime;
struct _AdamantWindowRuntime 
{
  AdamantWindowConfig config;
  AdamantWindow * windows;
};

/**
 * Initialize a window runtime with the specified config
 */ 
void
adamant_window_runtime_init(AdamantWindowRuntime * wrt, 
			    AdamantWindowConfig * w_config);

/**
 * Destroy window runtime
 */
void
adamant_window_runtime_destroy(AdamantWindowRuntime * wrt);

/**
 * Get window at index
 */
static inline AdamantWindow*
adamant_window_runtime_index(AdamantWindowRuntime *wrt, guint index) 
{
    switch(wrt->config.w_type)
        {
        case ADAMANT_WINDOW_TYPE_ROB:
            return  (AdamantWindow *)(&((AdamantWindowRob*)wrt->windows)[index]);
        case ADAMANT_WINDOW_TYPE_OOO:
            return (AdamantWindow *)(&((AdamantWindowOoO*)wrt->windows)[index]);
        default:
            return NULL;
        }
} 

/**
 * Insert into window (causes value at head to be consumed)
 */
void
adamant_window_runtime_insert(AdamantWindowRuntime * wrt, guint w_id, 
			      guint64 ready);

/**
 * Choose a window given an instruction and its sources
 * @return an index to the chosen window
 */
guint
adamant_window_runtime_choose(AdamantWindowRuntime * wrt, 
			      GArray * sched_info, guint sched_info_len,
			      guint communication_latency);

typedef struct {
  guint64 sum;
  mpz_t sum_squared;
  guint64 count;
} AdamantWindowVarianceInfo;

G_END_DECLS

#endif /* __ADAMANT_WINDOW_H__ */

