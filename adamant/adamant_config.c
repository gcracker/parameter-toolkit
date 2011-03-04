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

#include "adamant_config.h"

#include <glib.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <cudd.h>

/*
 * Local function declarations
 */
static void print_help();
static void adamant_process_config_file(const gchar *file,
                                        AdamantConfig *config);

/*
 * Global Variable declarations
 */
extern guint64 g_dinstart;

AdamantConfig *
adamant_new_config()
{
    AdamantConfig *config = g_new(AdamantConfig, 1);

    config->echo = FALSE;

    config->program = NULL;
    config->trace = NULL;

    config->tick_enabled = FALSE;
    config->tick_interval = 1000000;

    config->hist_enabled = FALSE;
    config->hist_bin_size = 100000;

    config->ignore_regs = g_array_new(FALSE, FALSE, sizeof(guint64));
    config->maxopcount=0;

    config->ia64_translate=0;
    config->ia64_sidetrace=NULL;
    config->ppc64_translate=0;
    config->x86_translate=0;

    config->din_vs_ready.doit=0;
    config->din_vs_ready.maxopcount=0;
    config->din_vs_ready.minopcount=0;
    config->din_vs_ready.mincycle=0;
    config->din_vs_ready.maxcycle=G_MAXUINT64;
    config->din_vs_ready.outfilename=NULL;

    config->din_vs_src.doit=0;
    config->din_vs_src.maxopcount=0;
    config->din_vs_src.minopcount=0;
    config->din_vs_src.outfilename=NULL;

    config->sin_vs_ready.doit=0;
    config->sin_vs_ready.maxopcount=0;
    config->sin_vs_ready.minopcount=0;
    config->sin_vs_ready.mincycle=0;
    config->sin_vs_ready.maxcycle=G_MAXUINT64;
    config->sin_vs_ready.outfilename=NULL;

    config->dd_make.doit=FALSE;
    config->dd_din_vs_ready.doit=FALSE;
    config->dd_din_vs_ready.outfilename=NULL;
    config->dd_din_vs_sin.doit=FALSE;
    config->dd_din_vs_sin.outfilename=NULL;
    config->dd_din_vs_din.doit=FALSE;
    config->dd_din_vs_din.outfilename=NULL;
    config->dd_din_vs_hot.doit=FALSE;
    config->dd_din_vs_hot.outfilename=NULL;
    config->dd_din_vs_sys.doit=FALSE;
    config->dd_din_vs_sys.outfilename=NULL;
    config->dd_restart.doit=FALSE;
    config->dd_restart.outfilename=NULL;
    config->dd_order = NULL;
    config->g_dd_start_din = 0;
    config->dd_tick_interval = 0;
    config->dd_output_file = NULL;
    config->dd_mem_cap = 0;
    config->dd_mem_init = CUDD_CACHE_SLOTS;
    config->dd_slots_multi = 1;
    config->dd_looseup_multi = 1;
    config->dd_garbage_collect = 0;
    config->dd_print_varmotion = FALSE;
    config->use_zdds = FALSE;
    config->dd_print_sin = 0;
    config->dd_print_sin_bfd = NULL;
    config->dd_slice = NULL;

    config->sin_vs_src.doit=0;
    config->sin_vs_src.maxopcount=0;
    config->sin_vs_src.minopcount=0;
    config->sin_vs_src.outfilename=NULL;

    // initialize dependence graph config
    svt_plot_dep_graph_config_init(&(config->dep_graph_config));

    config->opti.ive=0;

    config->setjmp_addr = 0;

    config->split_post_increment = 0;

    config->communication_latency = 0;
    config->inorder_branches = FALSE;

    config->win.w_num = 0;
    config->win.w_ready_weight = 1;
    config->win.w_head_weight = 1;
    config->win.w_tail_weight = 1;

    config->store_schedule = FALSE;
    config->ignore_stack = FALSE;

    config->respect_control = FALSE;
    config->predict_branches = FALSE;

    config->variance_sampling_period = 0;
    config->variance_filename = NULL;

    config->win_hist.doit = FALSE;
    config->win_hist.filname = FALSE;

    return config;
}

void
adamant_free_config(AdamantConfig *config)
{
    if (config != NULL) {
        g_free(config->program);
        g_free(config->trace);
        g_array_free(config->ignore_regs, TRUE);

        if(config->din_vs_ready.outfilename)
            g_free(config->din_vs_ready.outfilename);
        if(config->din_vs_src.outfilename)
            g_free(config->din_vs_src.outfilename);
        if(config->sin_vs_ready.outfilename)
            g_free(config->sin_vs_ready.outfilename);
        if(config->sin_vs_src.outfilename)
            g_free(config->sin_vs_src.outfilename);
        if (config->dep_graph_config.doit) {
            svt_plot_dep_graph_config_free(&(config->dep_graph_config));
        }

        g_free(config);
    }
}

/* numeric values for long-only options */
enum {
    OPTION_OTHERS = 0x100,
    OPTION_ECHO,
    OPTION_TICK,
    OPTION_NOTICK,
    OPTION_HIST,
    OPTION_NOHIST,
    OPTION_IGNORE_REGS,
    OPTION_MAXOPCOUNT,
    OPTION_IA64TRANSLATE,
    OPTION_PPC64TRANSLATE,
    OPTION_X86TRANSLATE,
    OPTION_PLOTDINVSRDY,
    OPTION_PLOTDINVSSRC,
    OPTION_PLOTSINVSRDY,
    OPTION_PLOTSINVSSRC,
    OPTION_PLOTDEPGRAPH,
    OPTION_RESPECTCD,
    OPTION_PREDICT_BRANCHES,
    OPTION_INDUCTION_VARIABLE_EXPANSION,
    OPTION_IGNORE_STACK,
    OPTION_WIN_HIST,
    OPTION_SETJMP_ADDR,
    OPTION_SPLIT_POST_INCREMENT,
    OPTION_STORE_SCHEDULE,
    OPTION_COMMUNICATION_LATENCY,
    OPTION_INORDER_BRANCHES,
    OPTION_WINDOWS,
    OPTION_WINDOW_WEIGHTS,
    OPTION_COLLECT_DIN_VARIANCE,
    OPTION_DD_DIN_VS_READY,
    OPTION_DD_DIN_VS_SIN,
    OPTION_DD_DIN_VS_DIN,
    OPTION_DD_DIN_VS_HOT,
    OPTION_DD_DIN_VS_SYS,
    OPTION_DD_ORDER,
    OPTION_DD_START_DIN,
    OPTION_DD_TICK,
    OPTION_DD_OUT_FILE,
    OPTION_DD_MEM_CAP,
    OPTION_DD_MEM_INIT,
    OPTION_DD_SLOTS_MULTI,
    OPTION_DD_LOOSEUP_MULTI,
    OPTION_DD_GARBAGE_COLLECT,
    OPTION_DD_PRINT_VARMOTION,
    OPTION_DD_RESTART_FILENAME,
    OPTION_USE_ZDDS,
    OPTION_DD_PRINT_SIN,
    OPTION_DD_SLICE,
    OPTION_UNKNOWN
};

void
adamant_process_options(gint argc, gchar *argv[], AdamantConfig *config)
{
    gint c;
    const gchar *shortopts = "c:p:t:h";
    SVTInvsreadyConfig *invsready=NULL;
    SVTInvsSrcConfig *invssrc=NULL;

    const struct option longopts[] = {
        { "config",           required_argument, NULL, 'c'                                 },
        { "program",          required_argument, NULL, 'p'                                 },
        { "trace",            required_argument, NULL, 't'                                 },
        { "help",             no_argument,       NULL, 'h'                                 },
        { "echo",             no_argument,       NULL, OPTION_ECHO                         },
        { "tick",             required_argument, NULL, OPTION_TICK                         },
        { "notick",           no_argument,       NULL, OPTION_NOTICK                       },
        { "hist",             optional_argument, NULL, OPTION_HIST                         },
        { "nohist",           no_argument,       NULL, OPTION_NOHIST                       },
        { "ignore-regs",      required_argument, NULL, OPTION_IGNORE_REGS                  },
        { "maxopcount",       required_argument, NULL, OPTION_MAXOPCOUNT                   },
        { "ia64translate",    required_argument, NULL, OPTION_IA64TRANSLATE                },
        { "ppc64translate",   no_argument,       NULL, OPTION_PPC64TRANSLATE               },
        { "x86translate",     no_argument,       NULL, OPTION_X86TRANSLATE                 },
        { "plotdinvsrdy",     required_argument, NULL, OPTION_PLOTDINVSRDY                 },
        { "plotdinvssrc",     required_argument, NULL, OPTION_PLOTDINVSSRC                 },
        { "plotsinvsrdy",     required_argument, NULL, OPTION_PLOTSINVSRDY                 },
        { "plotsinvssrc",     required_argument, NULL, OPTION_PLOTSINVSSRC                 },
        { "plotdepgraph",     required_argument, NULL, OPTION_PLOTDEPGRAPH                 },
        { "respect-cd",       required_argument, NULL, OPTION_RESPECTCD                    },
        { "predict-branches", required_argument, NULL, OPTION_PREDICT_BRANCHES             },
        { "ignore-stack",     required_argument, NULL, OPTION_IGNORE_STACK                 },
        { "ive",              required_argument, NULL, OPTION_INDUCTION_VARIABLE_EXPANSION },
        { "setjmp",           required_argument, NULL, OPTION_SETJMP_ADDR                  },
        { "split-post-incr",  no_argument,       NULL, OPTION_SPLIT_POST_INCREMENT         },
        { "store-schedule",   required_argument, NULL, OPTION_STORE_SCHEDULE               },
        { "comm-latency",     required_argument, NULL, OPTION_COMMUNICATION_LATENCY        },
        { "inorder-branches", no_argument,       NULL, OPTION_INORDER_BRANCHES             },
        { "windows",          required_argument, NULL, OPTION_WINDOWS                      },
        { "window-weights",   required_argument, NULL, OPTION_WINDOW_WEIGHTS               },
        { "window-variance",  required_argument, NULL, OPTION_COLLECT_DIN_VARIANCE         },
        { "window-hist",   required_argument, NULL, OPTION_WIN_HIST},
        { "dddinvsrdy", required_argument, NULL, OPTION_DD_DIN_VS_READY},
        { "dddinvssin", required_argument, NULL, OPTION_DD_DIN_VS_SIN},
        { "dddinvsdin",required_argument, NULL, OPTION_DD_DIN_VS_DIN},
        { "dddinvshot",required_argument, NULL, OPTION_DD_DIN_VS_HOT},
        { "dddinvssys",required_argument, NULL, OPTION_DD_DIN_VS_SYS},
        { "ddorder", required_argument, NULL, OPTION_DD_ORDER},
        { "ddrestart", required_argument, NULL, OPTION_DD_RESTART_FILENAME},
        { "ddtick", required_argument, NULL, OPTION_DD_TICK},
        { "ddstartdin", required_argument, NULL, OPTION_DD_START_DIN},
        { "ddmemcap", required_argument, NULL, OPTION_DD_MEM_CAP},
        { "ddmeminit", required_argument, NULL, OPTION_DD_MEM_INIT},
        { "ddgc", required_argument, NULL, OPTION_DD_GARBAGE_COLLECT},
        { "ddslotsmulti", required_argument, NULL, OPTION_DD_SLOTS_MULTI},
        { "ddlooseupmulti", required_argument, NULL, OPTION_DD_LOOSEUP_MULTI},
        { "ddvarmotion", no_argument, NULL, OPTION_DD_PRINT_VARMOTION},
        { "usezdds", no_argument, NULL,  OPTION_USE_ZDDS},
        { "ddprintsin", required_argument, NULL, OPTION_DD_PRINT_SIN},
        { "ddslice", required_argument, NULL, OPTION_DD_SLICE},
        { NULL, 0, NULL, 0 }
    };

    do {

        invsready=NULL;
        invssrc=NULL;
        c = getopt_long(argc, argv, shortopts, longopts, NULL);
        switch (c) {
        case 'c':
            adamant_process_config_file(optarg, config);
            break;
        case 'p':
            if (config->program != NULL) g_free(config->program);
            config->program = g_strdup(optarg);
            break;
        case 't':
            if (config->trace != NULL) g_free(config->trace);
            config->trace = g_strdup(optarg);
            break;
        case 'h':
            print_help();
            exit(EXIT_SUCCESS);
            break;
        case OPTION_ECHO:
            config->echo = TRUE;
            break;
        case OPTION_TICK:
            config->tick_enabled = TRUE;
            if (optarg!=NULL) {
                config->tick_interval = g_ascii_strtoull(optarg, NULL, 10);
            }
            break;
        case OPTION_NOTICK:
            config->tick_enabled = FALSE;
            break;
        case OPTION_HIST:
            config->hist_enabled = TRUE;
            if (optarg!=NULL)
                config->hist_bin_size = g_ascii_strtoull(optarg, NULL, 10);
            break;
        case OPTION_NOHIST:
            config->hist_enabled = FALSE;
            break;
        case OPTION_IGNORE_REGS:
            {
                GArray *ignore_regs = config->ignore_regs;
                gchar *end = optarg;
                gchar *arg;
                do {
                    guint64 val;
                    arg = end;
                    val = g_ascii_strtoull(arg, &end, 10);
                    if (val>0) g_array_append_val(ignore_regs, val);
                    if (*end == ',') ++end;
                } while (*end && arg!=end);
            }
            break;
        case OPTION_IGNORE_STACK:
            {
                gchar *end = optarg;
                gchar *arg = NULL;
                guint num_args = 0;
                while(*end != '\0' && arg != end) {
                    guint64 val;
                    arg = end;
                    val = g_ascii_strtoull(arg, &end, 0);
                    if(num_args == 0) config->stack_start = val;
                    if(num_args == 1) config->stack_end = val;
                    if (*end == ',') ++end;
                    num_args++;
                }
                if(num_args != 2 || *end != '\0')
                    g_error("You must pass two comma seperated arguments (stack_start,stack_end) to --ignore-stack");

                if(config->stack_end < config->stack_start) {
                    guint64 tmp;
                    tmp = config->stack_start;
                    config->stack_start = config->stack_end;
                    config->stack_end = tmp;
                }
                config->ignore_stack = 1;
            }
            break;
        case OPTION_MAXOPCOUNT:
            config->maxopcount = g_ascii_strtoull(optarg,
                                                  NULL, 10);
            break;
        case OPTION_IA64TRANSLATE:
            config->ia64_translate=1;
            config->ia64_sidetrace=g_strdup(optarg);
            config->isa = IA64_isa;
            break;
        case OPTION_PPC64TRANSLATE:
            config->ppc64_translate=1;
            config->isa = PPC_isa;
            break;
        case OPTION_X86TRANSLATE:
            config->x86_translate=1;
            config->isa = x86_isa;
            break;
        case OPTION_PLOTDINVSRDY:
            invsready=&config->din_vs_ready;
        case OPTION_PLOTSINVSRDY:
            if(!invsready) {
                invsready=&config->sin_vs_ready;
            }
            invsready->doit=1;
            {
                gchar **args;
                int i;
                args=g_strsplit(optarg, ",", 0);
                for(i=0;args[i];i++); /* Count args */
                if((i < 3)  || (i > 5)) {
                    g_error("Must pass at least 3 arguments to"
                            " --plot%cinvsrdy.  filename, ready_bin_size, din_bin_size\n"
                            "You may optionally pass a 4th maxopcount argument\n"
                            "Or a 4th minopcount argument and 5th maxopcount argument\n",
                            (c==OPTION_PLOTDINVSRDY)?'d':'s');
                }
                invsready->outfilename=
                    g_strdup(args[0]);
                invsready->ready_bin_size=
                    g_ascii_strtoull(args[1],NULL,0);
                invsready->din_bin_size=
                    g_ascii_strtoull(args[2],NULL,0);
                if(i == 4) {
                    invsready->maxopcount=
                        g_ascii_strtoull(args[3],NULL,0);
                } else if(i==5) {
                    invsready->minopcount=
                        g_ascii_strtoull(args[3],NULL,0);
                    invsready->maxopcount=
                        g_ascii_strtoull(args[4],NULL,0);
                    if(invsready->maxopcount) {
                        if(invsready->minopcount >
                           invsready->maxopcount) {
                            g_error("--plot%cvsrdy argument minopcount must be "
                                    "<= maxopcount", (c==OPTION_PLOTDINVSRDY)?'d':'s');
                        }
                    }
                }
                g_strfreev(args);
            }
            break;
        case OPTION_PLOTDINVSSRC:
            invssrc=&config->din_vs_src;
            invssrc->doit=1;
            {
                gchar **args;
                int i;
                args=g_strsplit(optarg, ",", 0);
                for(i=0;args[i];i++); /* Count args */
                if((i < 2)  || (i > 4)) {
                    g_error("Must pass at least 2 arguments to"
                            " --plotdinvssrc.  filename, din_bin_size\n"
                            "You may optionally pass a 3rd maxopcount argument\n"
                            "Or a 3rd minopcount argument and 4th maxopcount argument\n");
                }
                invssrc->outfilename=
                    g_strdup(args[0]);
                invssrc->din_bin_size=
                    g_ascii_strtoull(args[1],NULL,0);
                if(i == 3) {
                    invssrc->maxopcount=
                        g_ascii_strtoull(args[2],NULL,0);
                } else if(i==4) {
                    invssrc->minopcount=
                        g_ascii_strtoull(args[2],NULL,0);
                    invssrc->maxopcount=
                        g_ascii_strtoull(args[3],NULL,0);
                    if(invssrc->maxopcount) {
                        if(invssrc->minopcount >
                           invssrc->maxopcount) {
                            g_error("--plot%cvssrc argument minopcount must be "
                                    "<= maxopcount", (c==OPTION_PLOTDINVSSRC)?'d':'s');
                        }
                    }
                }
                g_strfreev(args);
            }
            break;
        case OPTION_PLOTSINVSSRC:
            if(!invssrc) {
                invssrc=&config->sin_vs_src;
            }
            invssrc->doit=1;
            {
                gchar **args;
                int i;
                args=g_strsplit(optarg, ",", 0);
                for(i=0;args[i];i++); /* Count args */
                if((i < 3)  || (i > 5)) {
                    g_error("Must pass at least 3 arguments to"
                            " --plotsinvsrdy.  filename, din_bin_size, sin_bin_size\n"
                            "You may optionally pass a 4th maxopcount argument\n"
                            "Or a 4th minopcount argument and 5th maxopcount argument\n");
                }
                invssrc->outfilename=
                    g_strdup(args[0]);
                invssrc->din_bin_size=
                    g_ascii_strtoull(args[1],NULL,0);
                invssrc->sin_bin_size=
                    g_ascii_strtoull(args[2],NULL,0);
                if(i == 4) {
                    invssrc->maxopcount=
                        g_ascii_strtoull(args[3],NULL,0);
                } else if(i==5) {
                    invssrc->minopcount=
                        g_ascii_strtoull(args[3],NULL,0);
                    invssrc->maxopcount=
                        g_ascii_strtoull(args[4],NULL,0);
                    if(invssrc->maxopcount) {
                        if(invssrc->minopcount >
                           invssrc->maxopcount) {
                            g_error("--plotsinvsrdy argument minopcount must be "
                                    "<= maxopcount");
                        }
                    }
                }
                g_strfreev(args);
            }
            break;
        case OPTION_PLOTDEPGRAPH:
            {
                SvtDepGraphConfig * dgconfig = &(config->dep_graph_config);
                gchar **args;
                guint nargs;

                args=g_strsplit(optarg, ",", 0);
                for(nargs=0;args[nargs];nargs++);

                svt_plot_dep_graph_config_setup(dgconfig, args, nargs);

                g_strfreev(args);
            }
            break;

            /* cases handing DD generation */
        case OPTION_DD_DIN_VS_READY:
            if ((optarg != NULL) &&
                (strcmp(optarg, "/dev/null") != 0))
                {
                    config->dd_din_vs_ready.outfilename = g_strdup(optarg);
                }
            config->dd_din_vs_ready.doit=TRUE;
	    config->dd_make.doit=TRUE;
            break;
        case OPTION_DD_DIN_VS_SIN:
            if ((optarg != NULL) &&
                (strcmp(optarg, "/dev/null") != 0))
                {
                    config->dd_din_vs_sin.outfilename = g_strdup(optarg);
                }
            config->dd_din_vs_sin.doit=TRUE;
	    config->dd_make.doit=TRUE;
            break;
        case OPTION_DD_DIN_VS_DIN:
            if ((optarg != NULL) &&
                (strcmp(optarg, "/dev/null") != 0))
                {
                    config->dd_din_vs_din.outfilename = g_strdup(optarg);
                }
            config->dd_din_vs_din.doit=TRUE;
	    config->dd_make.doit=TRUE;
            break;
        case OPTION_DD_DIN_VS_SYS:
            if ((optarg != NULL) &&
                (strcmp(optarg, "/dev/null") != 0))
                {
                    config->dd_din_vs_sys.outfilename = g_strdup(optarg);
                }
            config->dd_din_vs_sys.doit=TRUE;
	    config->dd_make.doit=TRUE;
            break;
       case OPTION_DD_RESTART_FILENAME:
            if ((optarg != NULL) &&
                (strcmp(optarg, "/dev/null") != 0))
                {
                    config->dd_restart.outfilename = g_strdup(optarg);
                }
            config->dd_restart.doit=TRUE;
	    config->dd_make.doit=TRUE;
            break;

       case OPTION_DD_DIN_VS_HOT:
            if ((optarg != NULL) &&
                (strcmp(optarg, "/dev/null") != 0))
                {
                    config->dd_din_vs_hot.outfilename = g_strdup(optarg);
                }
            config->dd_din_vs_hot.doit=TRUE;
	    config->dd_make.doit=TRUE;
            break;


        case OPTION_DD_ORDER:
            if (optarg != NULL)
                {
                    config->dd_order = g_strdup(optarg);
                }
            break;

	case OPTION_DD_PRINT_SIN:
	  if (optarg != NULL)
	    {
	      gchar ** printsin = g_strsplit(optarg,",",2);
	      if(printsin[0] != NULL)
		{
		  config->dd_print_sin = g_ascii_strtoull(printsin[0], NULL, 0);
		}
	      if(printsin[1] != NULL)
		{
		  config->dd_print_sin_bfd = g_strdup(printsin[1]);
		}
	      g_strfreev(printsin);
	    }
	  break;

        case OPTION_DD_TICK:
            if (optarg != NULL)
                {
                    config->dd_tick_interval = g_ascii_strtoull(optarg, NULL, 0);
                }
            break;

        case OPTION_DD_START_DIN:
            if (optarg != NULL)
                {
                    config->g_dd_start_din = g_ascii_strtoull(optarg, NULL, 0);
		    g_dinstart = config->g_dd_start_din;
                }
            break;

        case OPTION_DD_OUT_FILE:
            if (optarg != NULL)
                {
                    config->dd_output_file = g_strdup(optarg);
                }
            break;

            //! sets a memory cap to the DD creation
        case OPTION_DD_MEM_CAP:
            if (optarg != NULL)
                {
                    config->dd_mem_cap = g_ascii_strtoull(optarg, NULL, 0);
                }
            break;

            //! sets a memory cap to the DD creation
        case OPTION_DD_SLOTS_MULTI:
            if (optarg != NULL)
                {
                    config->dd_slots_multi = g_ascii_strtoull(optarg, NULL, 0);
                }
            break;

           //! sets a memory cap to the DD creation
        case OPTION_DD_LOOSEUP_MULTI:
            if (optarg != NULL)
                {
                    config->dd_looseup_multi = g_ascii_strtoull(optarg, NULL, 0);
                }
            break;

            //! sets a memory initialization size for CUDD
        case OPTION_DD_MEM_INIT:
            if (optarg != NULL)
                {
                    config->dd_mem_init = g_ascii_strtoull(optarg, NULL, 0);
                }
            break;

            //! used to set a memory trigger to manual garbage collection
        case OPTION_DD_GARBAGE_COLLECT:
            if (optarg != NULL)
                {
                    config->dd_garbage_collect = g_ascii_strtoull(optarg, NULL, 0);
                }
            break;
        case OPTION_DD_PRINT_VARMOTION:
            config->dd_print_varmotion = TRUE;
            break;
        case OPTION_DD_SLICE:
            config->dd_slice = g_string_new(optarg);
            break;
        case OPTION_USE_ZDDS:
            config->use_zdds = TRUE;
        break;

        /************************************/

        case OPTION_RESPECTCD:
            config->respect_control=1;
            config->cd_annotations=g_strdup(optarg);
            break;
        case OPTION_PREDICT_BRANCHES:
            config->predict_branches=1;
            config->bp_annotations=g_strdup(optarg);
            break;
        case OPTION_SETJMP_ADDR:
            config->setjmp_addr=g_ascii_strtoull(optarg, NULL, 0);
            break;
        case OPTION_INDUCTION_VARIABLE_EXPANSION:
            config->opti.ive=1;
            config->opti.unroll_factor=g_ascii_strtoull(optarg, NULL, 0);
            break;
        case OPTION_SPLIT_POST_INCREMENT:
            config->split_post_increment = 1;
            break;
        case OPTION_STORE_SCHEDULE:
            config->store_schedule = 1;
            {
                gchar *comma;
                comma = strrchr(optarg, ',');
                if(comma == NULL) {
                    g_error("Must specify two arguments to --store-schedule."
                            "  The first argument is the filename to store the schedule, "
                            "  followed by a comma and the schedule block size.");
                }
                config->schedule_file = g_strdup(optarg);
                config->schedule_file[comma - optarg] = '\0';
                config->schedule_block_size = g_ascii_strtoull(comma+1, NULL, 0);
            }
            break;
        case OPTION_INORDER_BRANCHES:
            config->inorder_branches = TRUE;
            break;
        case OPTION_COMMUNICATION_LATENCY:
            config->communication_latency = g_ascii_strtoull(optarg, NULL, 0);
            break;
        case OPTION_WINDOWS:
            {
                gchar **args;
                int i;
                args=g_strsplit(optarg, ",", 0);
                for(i=0;args[i];i++); /* Count args */
                if(i != 3) {
                    g_error("Must pass 3 arguments to"
                            " --windows.  # of windows,type=[ooo|rob],size\n");
                }
                config->win.w_num = g_ascii_strtoull(args[0], NULL, 0);
                if(!g_strcasecmp(args[1], "rob"))
                    config->win.w_type = ADAMANT_WINDOW_TYPE_ROB;
                else if(!g_strcasecmp(args[1], "ooo"))
                    config->win.w_type = ADAMANT_WINDOW_TYPE_OOO;
                else
                    g_error("Type argument to --windows must be rob or ooo\n");
                config->win.w_size = g_ascii_strtoull(args[2], NULL, 0);
                g_strfreev(args);
            }
            break;
        case OPTION_WINDOW_WEIGHTS:
            {
                gchar **args;
                int i;
                args=g_strsplit(optarg, ",", 0);
                for(i=0;args[i];i++); /* Count args */
                if(i != 3) {
                    g_error("Must pass 3 arguments to"
                            " --window-weights.  ready weight,head weight,tail weight\n");
                }
                config->win.w_ready_weight = g_ascii_strtoull(args[0], NULL, 0);
                config->win.w_head_weight = g_ascii_strtoull(args[1], NULL, 0);
                config->win.w_tail_weight = g_ascii_strtoull(args[2], NULL, 0);
                g_strfreev(args);
            }
            break;
        case OPTION_COLLECT_DIN_VARIANCE:
            {
                gchar *comma;
                comma = strrchr(optarg, ',');
                if(comma == NULL) {
                    g_error("Must specify two arguments to --window-variance."
                            "  The first argument is the filename to store the schedule, "
                            "  followed by a comma and the sampling period.");
                }
                config->variance_filename = g_strdup(optarg);
                config->variance_filename[comma - optarg] = '\0';
                config->variance_sampling_period = g_ascii_strtoull(comma+1, NULL, 0);
            }
            break;
        case OPTION_WIN_HIST:
            {
                config->win_hist.doit=TRUE;
                config->win_hist.filname=g_strdup(optarg);
            }
            break;
        case '?':
            g_printerr("Use -h or --help for options\n");
            exit(EXIT_FAILURE);
            break;
        case -1:
            break;
        default:
            g_print("?? getopt returned 0x%X ??\n", c);
            exit(EXIT_FAILURE);
            break;
        }
    } while (c >= 0);
}

void
print_help()
{
    g_print("\n");
    g_print("Usage:  adamantium {--program|-p} <program> {--trace|-t} <trace>");
    g_print("[options]\n");
    g_print("        adamantium {--help|-h}\n");
    g_print("\n");
    g_print("        <program>  bz2 file containing the program data\n");
    g_print("        <trace>    bz2 file containing the trace data\n");
    g_print("\n");
    g_print("  General options:\n");
    g_print("  ---------------------------------------------------\n");
    g_print("    --config <file> Parse 'file' for more options\n");
    g_print("    --tick\n");
    g_print("    --notick        Enable/disable the status ticking\n");
    g_print("    --echo          Print configuration options\n");
    g_print("    --hist\n");
    g_print("    --nohist        Enable/disable IPC histogram\n");
    g_print("    --maxopcount <count> Stop scheduling after 'count' ops\n");
    g_print("\n");
    g_print("  Choose only one of the following windowing options:\n");
    g_print("  ---------------------------------------------------\n");
    g_print("    --windows <N>,<rob|ooo>,<size>\n");
    g_print("\n");
    g_print("  Additional options:\n");
    g_print("  ---------------------------------------------------\n");
    g_print("    --branch-window <size> Branch window limit\n");
    g_print("    --respect-cd <file> Respect control deps listed in 'file'\n");
    g_print("    --predict-branches <file> Predict branches using 'file'\n");
    g_print("    --ignore-regs <reg>,[reg,reg,...] List of regs to ignore\n");
    g_print("\n");
    g_print("  Plotting options:\n");
    g_print("  ---------------------------------------------------\n");
    g_print("    --plotdinvsrdy <args> Generate a DIN vs. ready-time plot\n");
    g_print("    --plotdinvssrc <args> Generate a DIN vs. DIN-source plot\n");
    g_print("    --plotsinvsrdy <args> Generate a SIN vs. ready-time plot\n");
    g_print("    --plotsinvssrc <args> Generate a SIN vs. SIN-source plot\n");
    g_print("    --plotdepgraph <args> Generate a dependence graph plot\n");
    g_print("\n");
    g_print("  DD options:\n");
    g_print("  ---------------------------------------------------\n");
    g_print("    --dddinvsrdy Build DD for DIN vs. ready data\n");
    g_print("    --dddinvssin Build DD for DIN vs. SIN data\n");
    g_print("    --dddinvsdin Build DD for DIN vs. DIN data\n");
    g_print("    --dddinvshot Build DD for DIN vs. HOT data\n");
    g_print("    --dddinvssys Build DD for DIN vs. SYS data\n");
    g_print("    --ddrestart Specify a DD file to restart DD creation\n");
    g_print("    --ddoutfile Specify a DD output file\n");
    g_print("    --ddtick Specify a tick number for DDs\n");
    g_print("    --ddstartdin Specify a DIN to start DD creation\n");
    g_print("    --ddmemcap Specify a memory limit for the DD\n");
    g_print("    --ddmeminit Specify a memory target for DD creation\n");
    g_print("    --ddgc Specify a memory trigger for manual garbage collection\n");
    g_print("    --ddvarmotion Print out variable motion in DD creation\n");
    g_print("    --ddslotsmulti Specify a multiplier for the initial number of slots");
    g_print("    --ddlooseupmulti Specify a multiplier for the looseup slot value");
    g_print("\n");
    g_print("  Architecture-specific options:\n");
    g_print("  ---------------------------------------------------\n");
    g_print("    --ppc64translate <file> Use 'file' to schedule for PPC64\n");
    g_print("    --ia64translate <file> Use 'file' to schedule for IA64\n");
    g_print("    --x86translate <file> Use 'file' to schedule for X86\n");
}

static void
adamant_process_config_file(const gchar *file,
                            AdamantConfig *config)
{
    gchar *contents;
    GError *error;
    gint argc;
    gchar **argv;

    if (!g_file_get_contents(file, &contents, NULL, &error)) {
        /* TODO report the error */
        g_printerr("Error reading file: %s\n", file);
        return;
    }
    if (!g_shell_parse_argv(contents, &argc, &argv, &error)) {
        /* TODO report the error */
        g_printerr("Error parsing arguments from file: %s\n", file);
        return;
    }
    {
        char *b_optarg=optarg;
        int b_optind=optind, b_opterr=opterr, b_optopt=optopt;
        optarg=NULL; optind=0; opterr=0; optopt=0;
        adamant_process_options(argc, argv, config);
        optarg=b_optarg; optind=b_optind; opterr=b_opterr; optopt=b_optopt;
    }
    g_strfreev(argv);
    g_free(contents);
}

void
adamant_print_config(AdamantConfig *config)
{
    if (config->echo) {
        g_print("Adamant Configuration:\n");
        if (config->program != NULL) {
            g_print("        Program: %s\n", config->program);
        } else {
            g_print("        Program: not specified!\n");
        }
        if (config->trace != NULL) {
            g_print("          Trace: %s\n", config->trace);
        } else {
            g_print("          Trace: not specified!\n");
        }
        if (config->tick_enabled) {
            g_print("           Tick: %"G_GUINT64_FORMAT" instructions\n",
                    config->tick_interval);
        } else {
            g_print("           Tick: disabled\n");
        }
        if (config->hist_enabled) {
            g_print("      Histogram: %"G_GUINT64_FORMAT" cycles/bin\n",
                    config->hist_bin_size);
        } else {
            g_print("      Histogram: disabled\n");
        }
        if (config->ignore_regs->len==0) {
            g_print("   Ignored Regs: none\n");
        } else {
            guint i;
            guint len = config->ignore_regs->len;
            guint64 *data = (guint64 *)config->ignore_regs->data;
            g_print("   Ignored Regs: %"G_GUINT64_FORMAT, data[0]);
            for (i=1; i<len; ++i)
                g_print(", %"G_GUINT64_FORMAT, data[i]);
            g_print("\n");
        }
        g_print("\n");
    }
}

