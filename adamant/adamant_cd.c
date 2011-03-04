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
#include "adamant_cd.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static CDSchedInfo *
add_branch(GHashTable *branches, guint64 address)
{
  CDSchedInfo *info = 
    g_hash_table_lookup(branches, &address);

  if(info == NULL) {
    guint64 *address_p;

    address_p = g_new(guint64, 1);
    info = g_new(CDSchedInfo, 1);

    *address_p = address;
    info->cd_din_src = 0;
    info->info.ready = 0;
    info->info.din_src = 0;
    info->info.thread_id = 0;
    
    g_hash_table_insert(branches, address_p, info);
  }

  return info;
}

static void build_cd_times(gpointer key, gpointer value, gpointer user_data)
{
  guint64 *ip = key;
  GArray *cd_times = value;
  AdamantRuntime *runtime = user_data;

  guint64 aligned_addr, index_addr;
  PageTableEntry *entry;

  aligned_addr = (*ip) & ENTRY_KEY_LINE_MASK;
  index_addr = (*ip) & ENTRY_KEY_INDEX_MASK;
  entry = PageTable64_lookup(runtime->control_deps, aligned_addr);
  if(entry == NULL) {
    entry = g_new0(PageTableEntry,1);
    entry->key = aligned_addr;
    PageTable64_add(runtime->control_deps, aligned_addr, entry);
  }

  entry->data[index_addr]=cd_times->data;
  return;
}

static void 
destroy_array(GArray *array)
{
  g_array_free(array, FALSE);
}

void
adamant_cd_read_deps(AdamantConfig *config, AdamantRuntime *runtime)
{
  GIOChannel *cdinfo;
  GError *error = NULL;
  GString *line;
  GIOStatus status;
  GHashTable *control_deps;

  /* Read in the control dependence information */
  cdinfo = g_io_channel_new_file(config->cd_annotations, "r", &error);
  if(cdinfo == NULL) {
    printf("Unable to open control dependence "
	   "annotations file %s: %s\n", config->cd_annotations,
	   error->message);
    exit(-1);
  }
	 
  control_deps =
    g_hash_table_new_full(g_int_hash, EqualUINT64, g_free, 
			  (GDestroyNotify)destroy_array);
  runtime->control_dep_times = 
    g_hash_table_new_full(g_int_hash, EqualUINT64, g_free, NULL);

  runtime->control_deps = PageTable64_new();

  line = g_string_new("");
  while((status = 
	 g_io_channel_read_line_string(cdinfo, line, NULL, &error)) == 
	G_IO_STATUS_NORMAL) {
    guint64 *op, dep;
    int i;
    gchar **pieces;
    gchar *end;
    GArray *deps;

    op = g_new(guint64, 1);
    pieces = g_strsplit_set(line->str, " \t|", -1);
    *op = g_ascii_strtoull(pieces[0], &end, 0);
    if(*end != '\0') {
      printf("Non-numeric value found in control dependence file: \"%s\"\n",
	     pieces[0]);
      exit(-1);
    }

    deps = g_array_new(TRUE, FALSE, sizeof(CDSchedInfo*));
    for(i = 1; pieces[i] != NULL; i++) {
      CDSchedInfo *info;

      g_strstrip(pieces[i]);

      /* Skip empty pieces */
      if(*pieces[i] == '\0') continue;

      /* Convert the data into a uint64 */
      dep = g_ascii_strtoull(pieces[i], &end, 0);
      if(*end != '\0') {
	printf("Non-numeric value found in control dependence file: \"%s\"\n",
	       pieces[i]);
	exit(-1);
      }
      info = add_branch(runtime->control_dep_times, dep);
      g_array_append_val(deps, info);
    }
    g_hash_table_insert(control_deps, op, deps);
    g_strfreev(pieces);
  }

  g_hash_table_foreach(control_deps, build_cd_times, runtime);

  g_string_free(line, TRUE);
  g_hash_table_destroy(control_deps);

  if(status != G_IO_STATUS_EOF) {
    printf("Error while reading control dependence "
	   "annotations file %s: %s\n", config->cd_annotations,
	   error->message);
    exit(-1);
  }
}

