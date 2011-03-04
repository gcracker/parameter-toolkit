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

#include "adamant.h"
#include "adamant_runtime.h"
#include "adamant_config.h"
#include <glib.h>
#include <tmtoper_ppc64.h>

static TmtTypedStaticOper *split_PPC_oper(TmtTypedStaticOper *soper)
{
  TmtTypedStaticOper *soper_copy = NULL;

  switch(soper->type) {
  case PPC64_STORE_UPDATE:
    g_assert(soper->soper.num_reg_dst == 1);
    g_assert(soper->soper.num_reg_src == 2);
    
    soper_copy = g_memdup(soper, sizeof(TmtTypedStaticOper));
    soper->soper.num_reg_dst = 0;
    
    soper_copy->soper.num_mem_dst = 0;
    soper_copy->soper.num_reg_src = 1;
    soper_copy->soper.reg_src[0] = soper_copy->soper.reg_src[1];
    break;

  case PPC64_STORE_INDEXED_UPDATE:
    g_assert(soper->soper.num_reg_dst == 1);
    g_assert(soper->soper.num_reg_src == 3);
      
    soper_copy = g_memdup(soper, sizeof(TmtTypedStaticOper));
    soper->soper.num_reg_dst = 0;

    soper_copy->soper.num_mem_dst = 0;
    soper_copy->soper.num_reg_src = 2;
    soper_copy->soper.reg_src[0] = soper_copy->soper.reg_src[1];
    soper_copy->soper.reg_src[1] = soper_copy->soper.reg_src[2];
    break;

  case PPC64_LOAD_ARITHMETIC_UPDATE:
  case PPC64_LOAD_ZERO_UPDATE:
  case PPC64_LOAD_UPDATE:
    g_assert(soper->soper.num_reg_dst == 2);
    g_assert(soper->soper.num_reg_src == 1);
      
    soper_copy = g_memdup(soper, sizeof(TmtTypedStaticOper));
    soper->soper.num_reg_dst = 1;

    soper_copy->soper.num_mem_src = 0;
    soper_copy->soper.num_reg_dst = 1;
    soper_copy->soper.reg_dst[0] = soper_copy->soper.reg_dst[1];
    break;
      
  case PPC64_LOAD_ARITHMETIC_INDEXED_UPDATE:
  case PPC64_LOAD_ZERO_INDEXED_UPDATE:
  case PPC64_LOAD_INDEXED_UPDATE:
    g_assert(soper->soper.num_reg_dst == 2);
    g_assert(soper->soper.num_reg_src == 2);
      
    soper_copy = g_memdup(soper, sizeof(TmtTypedStaticOper));
    soper->soper.num_reg_dst = 1;

    soper_copy->soper.num_mem_src = 0;
    soper_copy->soper.num_reg_dst = 1;
    soper_copy->soper.reg_dst[0] = soper_copy->soper.reg_dst[1];
    break;
  }
  
  return soper_copy;
}

void adamant_split_opers(AdamantRuntime *runtime)
{
  guint i;
  GArray *sopers = runtime->tmtrc->oper_table;

  runtime->split_sopers = g_hash_table_new(g_int_hash, EqualUINT64);

  for(i = 0; i < sopers->len; i++) {
    TmtTypedStaticOper *soper = &g_array_index(sopers, TmtTypedStaticOper, i);
    TmtTypedStaticOper *copy = NULL;
    if(runtime->config->ppc64_translate)
      copy = split_PPC_oper(soper);

    if(copy != NULL)
      g_hash_table_insert(runtime->split_sopers, &soper->soper.ip, copy);
  }
}

