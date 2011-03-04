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
#include "adamant_config.h"
#include "adamant_runtime.h"
#include "adamant_branch_prediction.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static gboolean counter_msb(/* 2-bit counter */ guint64 cnt)
{ return (cnt >= 2); }
static guint64 counter_inc(/* 2-bit counter */ guint64 cnt)
{ if (cnt != 3) ++cnt; return cnt; }
static guint64 counter_dec(/* 2-bit counter */ guint64 cnt)
{ if (cnt != 0) --cnt; return cnt; }

gboolean get_prediction(TmtStaticOper *soper, AdamantRuntime *runtime)
{
  gboolean prediction = FALSE;
  guint64 pc = soper->ip;
  guint64 index;
  guint cnt;

  index = ((pc ^ (runtime->bhr)) & PHT_INDEX_MASK);
  cnt = g_array_index(runtime->pht,guint8,index);
  prediction = counter_msb(cnt);
  return prediction;   // true for taken, false for not taken
}

// static inline gboolean
static gboolean
update_bp_accuracy(AdamantRuntime *runtime, 
		   TmtStaticOper *soper, gboolean correct, gboolean taken)
{
  BranchHistory *history = g_hash_table_lookup(runtime->branch_pred_accuracy,
  					       soper);
  gboolean is_indirect;
  guint64 pc = soper->ip;
  guint64 index;
  guint8 cnt;

  is_indirect = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_INDIRECT);

  if(!is_indirect) {
    index = ((pc ^ (runtime->bhr)) & PHT_INDEX_MASK);
    cnt = g_array_index(runtime->pht,guint8,index);
    if (taken)
      cnt = counter_inc(cnt);
    else
      cnt = counter_dec(cnt);
    //update PHT
    g_array_index(runtime->pht,guint8,index) = cnt;
    //update BHR
    runtime->bhr >>= 1;
    if (taken) {
      runtime->bhr |= BHR_MSB;
    }
  }

  if(history == NULL) {
    history = g_new0(BranchHistory, 1);
    g_hash_table_insert(runtime->branch_pred_accuracy, soper, history);
  }

  if(correct)
    history->correct++;
  history->total++;

  return correct;
}

static gboolean
print_accuracy(gpointer key,
	       gpointer value,
	       gpointer user_data)
{
  guint64 correct = ((BranchHistory*)value)->correct;
  guint64 total = ((BranchHistory*)value)->total;

  printf("0x%016llx : %10lld/%10lld (%6.2f%%)\n", 
	 ((TmtStaticOper*)key)->ip,
	 correct,
	 total,
	 ((float)correct)/total * 100);
  return FALSE;
}


void
adamant_report_bp_accuracy(AdamantRuntime *runtime)
{
  printf("Branch Prediction Accuracy:\n");
  g_hash_table_foreach_steal(runtime->branch_pred_accuracy, 
			     print_accuracy, NULL);
}

gboolean
adamant_predict_branch(AdamantRuntime *runtime, 
		       TmtOper *oper, TmtOper *next_oper)
{
  guint64 *target;
  TmtStaticOper *soper;
  gboolean is_branch,is_call,is_return,is_indirect;

  soper = oper->soper;

  // if instruction is not a branch, call, or return, then the next 
  // instruction is automatically predicted correctly
  is_branch = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH);
  if (!is_branch) return (TRUE);

  // if next instruction not valid, then obviously we can't predict it!
  if(next_oper == NULL) return update_bp_accuracy(runtime, soper, FALSE, FALSE);

  // lookup branch target in target table
  target = g_hash_table_lookup(runtime->branch_target_dist, &soper->ip);

  // if target not found, then branch has no bias info.  Assume correct
  // prediction, so we can simulate certain branches as being perfect
  if(target == NULL) return update_bp_accuracy(runtime, soper, TRUE, TRUE); 

  is_call   = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_CALL);
  is_return = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_RETURN);
  is_indirect = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_INDIRECT);
  
  if(is_indirect) {
    return update_bp_accuracy(runtime, soper,((*target) == next_oper->soper->ip),oper->taken);
  } else {
    if (is_call || is_return) {
      // target must be zero (not-taken) or one (taken)
      g_assert((*target) == 0 || (*target) == 1);

      if (runtime->config->x86_translate) {
        // calls and returns automatically taken in x86, 
        // so always predicted correctly
        return update_bp_accuracy(runtime, soper, TRUE, TRUE);
      } else if ((!get_prediction(soper, runtime)) && (!(oper->taken))) {
        // predicted not-taken and actually not-taken
        return update_bp_accuracy(runtime, soper, TRUE, (oper->taken));
      } else if ((get_prediction(soper, runtime)) && (oper->taken)) {
        // predicted taken and actually taken
        return update_bp_accuracy(runtime, soper, TRUE, oper->taken);
      } else {
        // mispredicted
        return update_bp_accuracy(runtime, soper, FALSE, oper->taken);
      }
    } else {
      // conditional branches
      return update_bp_accuracy(runtime, soper, 
    			      ((get_prediction(soper, runtime)) == (oper->taken)), oper->taken);    
    }
  }
}

void
adamant_bp_read_distribution(AdamantConfig *config, AdamantRuntime *runtime)
{
  GIOChannel *bpinfo;
  GError *error = NULL;
  GString *line;
  GIOStatus status;

  /* Read in the branch target distribution information */
  bpinfo = g_io_channel_new_file(config->bp_annotations, "r", &error);
  if(bpinfo == NULL) {
    printf("Unable to open branch target distribution "
	   "annotations file %s: %s\n", config->bp_annotations,
	   error->message);
    exit(-1);
  }
	 
  runtime->branch_target_dist = 
    g_hash_table_new(g_int_hash,EqualUINT64);

  line = g_string_new("");
  while((status = 
	 g_io_channel_read_line_string(bpinfo, line, NULL, &error)) == 
	G_IO_STATUS_NORMAL) {
    guint64 *op, *target;
    int i;
    gchar **pieces;
    gchar *end;
    guint64 ip, frequency, max;

    op = g_new(guint64, 1);
    target = g_new(guint64, 1);

    pieces = g_strsplit_set(line->str, " \t|", -1);
    *op = g_ascii_strtoull(pieces[0], &end, 0);
    if(*end != '\0') {
      printf("Non-numeric value in branch target "
	     "distribution file: \"%s\"\n", pieces[0]);
      exit(-1);
    }

    max = 0;
    for(i = 1; pieces[i] != NULL; i++) {
      g_strstrip(pieces[i]);

      /* Skip empty pieces */
      if(*pieces[i] == '\0') continue;

      /* Convert the data into a uint64 */
      ip = g_ascii_strtoull(pieces[i], &end, 0);
      if(*end != ':') {
	printf("Non-numeric value or missing ':' in branch target "
	       "distribution file: \"%s\"\n", pieces[i]);
	exit(-1);
      }

      frequency = g_ascii_strtoull(end+1, &end, 0);
      if(*end != '\0') {
	printf("Non-numeric value in branch target "
	       "distribution file: \"%s\"\n", pieces[i]);
	exit(-1);
      }

      if(frequency >= max) {
	*target = ip;
	max = frequency;
      }
    }

    g_hash_table_insert(runtime->branch_target_dist, op, target);
    g_strfreev(pieces);
  }
  g_string_free(line, TRUE);

  if(status != G_IO_STATUS_EOF) {
    printf("Error while reading branch target distributions"
	   "file %s: %s\n", config->bp_annotations,
	   error->message);
    exit(-1);
  }
}

