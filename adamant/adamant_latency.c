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

#include <glib.h>

#include "adamant.h"
#include "adamant_latency.h"
#include "adamant_runtime.h"
#include "adamant_config.h"

guint64 adamant_compute_latency(AdamantRuntime *runtime, TmtOper *oper)
{
  /* Assume all operations with memory sources or memory destinations
     are loads or stores */
  if(runtime->config->ignore_stack) {
    if(oper->soper->num_mem_src > 0 ||
       oper->soper->num_mem_dst > 0) {
      
      /* If all the addresses referenced are on the stack, then this
	 is a stack operation and we assign it zero latency */
      guint i;
      gboolean is_stack = 1;
      for(i = 0; is_stack && i < oper->soper->num_mem_src; i++) {
	guint32 size = oper->mem_src_size[i];
	guint64 ea = oper->mem_src[i];
	if(!(ea >= runtime->config->stack_start && 
	     (ea + size - 1) <= runtime->config->stack_end)) {
	  is_stack = 0;
	}
      }

      for(i = 0; is_stack && i < oper->soper->num_mem_dst; i++) {
	guint32 size = oper->mem_dst_size[i];
	guint64 ea = oper->mem_dst[i];
	if(!(ea >= runtime->config->stack_start && 
	     (ea + size - 1) <= runtime->config->stack_end)) {
	  is_stack = 0;
	}
      }

      if(is_stack) {
#if 0
	printf("Instruction %016llx is a stack access\n", oper->soper->ip);
#endif
	return 0;
      }
    }
  }

  return 1; /* TODO: calculate better latency */
}

