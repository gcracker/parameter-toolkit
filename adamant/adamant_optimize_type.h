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

#ifndef ADAMANT_OPTIMIZE_TYPE_H
#define ADAMANT_OPTIMIZE_TYPE_H

#include <glib.h>
#include <pagetable64.h>
#include "adamant_meminfo_pagetable.h"

G_BEGIN_DECLS

#define ADAMANT_IVE_GC_INTERVAL 10000000

typedef struct {
  gboolean ive; /* Perform induction variable expansion */
  guint unroll_factor;
} AdamantOptimizeConfig;

typedef struct {
  GMemChunk *IVE_node_allocator;
  GArray *induction_vars;
  PageTable64 *memory_induction_vars; /* Induction vars spilled to memory */
  guint IVE_gc_interval;
} AdamantOptimizeRuntime;

typedef struct _AdamantIVENode AdamantIVENode;
struct _AdamantIVENode {
  AdamantIVENode *parent; /* Previous instruction in the dependence chain */
  guint ref_count;        /* How many people point to me */
  SchedInfo sched_info;   /* When did I *FINISH* executing */
};

G_END_DECLS

#endif /* ADAMANT_OPTIMIZE_TYPE_H */

