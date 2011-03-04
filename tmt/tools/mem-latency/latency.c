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
#include <glib-object.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tmt.h>
#include <atm.h>

//
// Command line parsing variables
//
static gchar * program_file = NULL;
static gchar * trace_file = NULL;
static gchar * cache_file = NULL;

static GOptionEntry entries[] = { 
  { "program", 'p', 0, G_OPTION_ARG_FILENAME, &program_file,
    "Program file", "P" },
  { "trace", 't', 0, G_OPTION_ARG_FILENAME, &trace_file,
    "Trace file", "T" },
  { "cache", 'c', 0, G_OPTION_ARG_FILENAME, &cache_file,
    "Specify cache xml configuration file", NULL },
  { NULL }
};

typedef struct {
  guint64 total_references;
  guint64 total_latency;
} load_info_t;

void avg_load_latency(TmtReadContext * tmtrc, AtmCache * cache);

int main(int argc, char *argv[])
{
  AtmCacheHierarchy * cachehierarchy = NULL;
  AtmXmlCacheHierarchy * xmlcachehierarchy = NULL;
  AtmCache * cache = NULL;

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
  if (!program_file || !trace_file || !cache_file) {
    fprintf(stderr, "You must specify program file, trace file,"
	    " and cache configuration file\n");
    exit(1);
  }

  // create tmt context
  tmtrc = tmt_readcontext_new(trace_file, program_file);
  if(tmtrc == NULL) {
    g_error("Unable to create read context from trace file %s "
	    "and program file %s", trace_file, program_file);
  }

  // create cache simulator
  xmlcachehierarchy = 
    atm_xmlcachehierarchy_new_from_xml_file(cache_file);
  g_assert( xmlcachehierarchy );
  atm_xmlcachehierarchy_debug_print( xmlcachehierarchy, stdout );

  // create the cache hierarchy from the xml file
  cachehierarchy = atm_cachehierarchy_new( xmlcachehierarchy );
  atm_xmlcachehierarchy_free( xmlcachehierarchy );
  cache = atm_cachehierarchy_lookup_cache( cachehierarchy, "l1d" );
  g_assert(cache);

  // compute avg load latency for each load
  avg_load_latency(tmtrc);

  tmt_readcontext_free(tmtrc);

  return 0;
}

gboolean print_load_latency(gpointer key,
			    gpointer value,
			    gpointer user_data)
{
  int i;
  TmtStaticOper *soper = (TmtStaticOper*)key;
  load_info_p * = (load_info_t*)value;

  printf("0x%016llx |", soper->ip);
  printf(" %d (%f)\n", 
	 load_info_p->total_latency/load_info_p->total_references,
	 ((float)load_info_p->total_latency)/load_info_p->total_references);
  return FALSE;
}

void avg_load_latency(TmtReadContext * tmtrc, AtmCache * cache)
{
  gint tmterr;
  TmtOper oper1;
  TmtOper *oper = &oper1;
  TmtStaticOper *soper;
  guint64 opcount = 0;
  guint i;
  guint latency;
  load_info_t *load_info_p, load_info;

  GHashTable *static_loads;
  static_loads = g_hash_table_new(g_int_hash,g_direct_equal);

  tmt_readcontext_read(&tmterr, tmtrc, oper);
  while (tmterr == TMTIO_OK) {
    soper = oper->soper;
    latency = 0;

    // if this has memory sources
    for (i=0; i<soper->num_mem_src; i++) {
      latency += atm_cache_request(cache, soper->ip, ATM_CACHE_READ);
    }
    // todo: do memory destinations

    // update load in table
    load_info_p = g_hash_table_lookup(static_loads, soper);
    if (load_info_p == NULL) {
      load_info_p = g_new0(load_info_t,1);
      g_hash_table_insert(static_loads, soper, load_info_p);
    }
    load_info_p->total_latency += latency;
    load_info_p->total_references += 1;

    opcount++;
    tmt_readcontext_read(&tmterr, tmtrc, oper);
  }

  g_hash_table_foreach_steal(static_loads, print_load_latency, NULL);

  if (tmterr == TMTIO_ERROR) {
    printf("**ERROR**: Trace file ended abnormally, opcount = %lld\n",
	   opcount);
    exit(-1);
  }
}


