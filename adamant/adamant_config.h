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

#ifndef __ADAMANT_CONFIG_H__
#define __ADAMANT_CONFIG_H__

/*
 * Header dependences
 */
#include <glib.h>
#include <svt.h>
#include "adamant_optimize_type.h"
#include "adamant_window.h"
#include "tmt.h"


/*
 * Forward declarations
 */
typedef struct _AdamantConfig  AdamantConfig;

G_BEGIN_DECLS

/*
 * Type definitions
 */
typedef struct {
    gboolean doit;
    gchar *outfilename;
} DdMake;

typedef struct {
    gboolean doit;
    gchar *outfilename;
} DdConfig;

struct _AdamantConfig {
    gboolean echo;

    gchar *program;
    gchar *trace;

    gboolean tick_enabled;
    guint64 tick_interval;

    gboolean hist_enabled;
    guint64 hist_bin_size;

    GArray *ignore_regs;
    guint64 maxopcount;

    gboolean respect_control;
    gchar *cd_annotations;
    guint64 setjmp_addr;

    gboolean predict_branches;
    gchar *bp_annotations;

    AdamantWindowConfig win;

    SVTInvsreadyConfig din_vs_ready;
    SVTInvsSrcConfig din_vs_src;
    SVTInvsreadyConfig sin_vs_ready;
    SVTInvsSrcConfig sin_vs_src;
    SvtDepGraphConfig dep_graph_config;

    /* DD Generation */
    DdMake dd_make;
    DdConfig dd_din_vs_hot;
    DdConfig dd_din_vs_ready;
    DdConfig dd_din_vs_sin;
    DdConfig dd_din_vs_din;
    DdConfig dd_din_vs_sys;
    DdConfig dd_restart;
    gchar * dd_order;
    guint64 g_dd_start_din;
    guint64 dd_tick_interval;
    gchar * dd_output_file;
    guint64 dd_mem_cap;
    guint64 dd_mem_init;
    guint64 dd_garbage_collect;
    gboolean dd_print_varmotion;
    gboolean use_zdds;
    guint dd_print_sin;
    GString * dd_print_sin_bfd;
    GString * dd_slice;
    guint32 dd_slots_multi;
    guint32 dd_looseup_multi;

    AdamantOptimizeConfig opti;

    gboolean ignore_stack;
    guint64 stack_start;
    guint64 stack_end;

    gboolean split_post_increment;

    gboolean store_schedule;
    gchar *schedule_file;
    guint64 schedule_block_size;

    gboolean ia64_translate;
    gchar *ia64_sidetrace;
    gboolean ppc64_translate;
    gboolean x86_translate;

    guint communication_latency;
    gboolean inorder_branches;

    guint64 variance_sampling_period;
    char *variance_filename;

    struct {
        gboolean doit;
        char *filname;
    } win_hist;

    TMT_ISA isa;
};

/*
 * Function declarations
 */
AdamantConfig *
adamant_new_config();

void
adamant_free_config(AdamantConfig *config);

void
adamant_process_options(gint argc, gchar *argv[], AdamantConfig *config);

void
adamant_print_config(AdamantConfig *config);

G_END_DECLS

#endif

