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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <tmt.h>

//
// Command line parsing variables
//
static gchar * program_file = NULL;
static gchar * trace_file = NULL;
static gboolean x86translate = FALSE;

static GOptionEntry entries[] = { 
  { "program", 'p', 0, G_OPTION_ARG_FILENAME, &program_file,
    "Program file", "P" },
  { "trace", 't', 0, G_OPTION_ARG_FILENAME, &trace_file,
    "Trace file", "T" },
  { "x86translate", 'x', 0, G_OPTION_ARG_NONE, &x86translate,
    "Process the trace through x86 translation filter", NULL },
  { NULL }
};

void count(TmtReadContext *tmtrc);

int main(int argc, char *argv[])
{
  TmtReadContext *tmtrc;
  GOptionContext * context;
  GError *error = NULL;

  // setup and parse command line
  context = g_option_context_new ("- Determine branch bias of trace");
  g_option_context_add_main_entries (context, entries, NULL);
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    fprintf(stderr, "Error parsing command line arguments: %s\n",
	    error->message);
    exit(1);
  }

  // check that command line is good
  if (!program_file || !trace_file) {
    fprintf(stderr, "You must specify a program file and a trace file\n");
    exit(1);
  }

  tmtrc = tmt_readcontext_new(trace_file, program_file);
  if(tmtrc == NULL) {
    g_error("Unable to create read context from trace file %s "
	    "and program file %s", trace_file, program_file);
  }
  count(tmtrc);
  tmt_readcontext_free(tmtrc);
  return 0;
}

typedef struct {
  guint64 ip;
  guint64 count;
} target_info_t;

gboolean print_deps(gpointer key,
		    gpointer value,
		    gpointer user_data)
{
  int i;
  TmtStaticOper *soper = (TmtStaticOper*)key;
  GArray *targets = (GArray*)value;

  printf("0x%016llx |", soper->ip);
  for(i = 0; i < targets->len; i++) {
    target_info_t *info = &g_array_index(targets, target_info_t, i);
    printf(" 0x%016llx:%llu", info->ip, info->count);
  }
  printf("\n");
  return FALSE;
}

void update_targets(GHashTable *branches, TmtStaticOper *soper, guint64 target)
{
  target_info_t *target_info_p, target_info;
  int i;
  GArray *targets = g_hash_table_lookup(branches, soper);

  if(targets == NULL) {
    targets = g_array_new(FALSE,FALSE,sizeof(target_info_t));
    g_hash_table_insert(branches, soper, targets);
  }

  for(i = 0; i < targets->len; i++) {
    target_info_p = &g_array_index(targets, target_info_t, i);
    if(target_info_p->ip == target) {
      target_info_p->count++;
      return;
    }
  }

  target_info.ip = target;
  target_info.count = 1;
  g_array_append_val(targets, target_info);
}


void count(TmtReadContext *tmtrc)
{
    gint tmterr;
    gboolean is_branch, is_call, is_return;
    TmtOper oper1, oper2;
    TmtOper *oper = &oper1, *next_oper = &oper2, *tmp;
    TmtStaticOper *soper;
    guint64 opcount = 0;

    GHashTable *static_branches;
    static_branches = g_hash_table_new(g_int_hash,g_direct_equal);

    tmt_readcontext_read(&tmterr, tmtrc, next_oper);
    while (tmterr == TMTIO_OK) {
        tmp = oper;
	oper = next_oper;
	next_oper = tmp;

	tmt_readcontext_read(&tmterr, tmtrc, next_oper);

	soper = oper->soper;
	is_branch = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH);
	is_call   = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_CALL);
	is_return = tmt_flag_get_value(soper->instr_attr, TMT_BRANCH_RETURN);

	if(tmterr == TMTIO_OK) {
	  if(is_branch && (is_call || is_return)) {
	    /* Target is irrelevant, we just want taken and not-taken */
	    if (x86translate) {
	      /* If x86translate is specified, we assume all calls, returns,
		 and unconditional jumps are automatically taken */
	      update_targets(static_branches, soper, TRUE);
	    } else {
	      update_targets(static_branches, soper, oper->taken);
	    }
	  } else if(is_branch) {
	    update_targets(static_branches, soper, next_oper->soper->ip);
	  }
	}
    }

    g_hash_table_foreach_steal(static_branches, print_deps, NULL);

    if (tmterr == TMTIO_ERROR) {
	printf("**ERROR**: Trace file ended abnormally, opcount = %lld\n",
	       opcount);
	exit(-1);
    }
}


