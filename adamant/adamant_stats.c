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

#include "adamant_stats.h"
#include "adamant_plot.h"
#include "adamant_branch_prediction.h"
#include "adamant_window.h"
#include <gmp.h>
#include <sys/time.h>
#include <time.h>

// ! DEBUG
GQueue * exeQueue = NULL;
//! END DEBUG

AdamantStats *adamant_new_stats()
{
  AdamantStats *stats = g_new(AdamantStats, 1);
  stats->histogram = NULL;
  return stats;
}

void adamant_free_stats(AdamantStats *stats)
{
  int count;
  if (stats != NULL) {
    if (stats->histogram!=NULL) g_array_free(stats->histogram, TRUE);
    if(stats->din_vs_ready.arr) {
      g_array_free(stats->din_vs_ready.arr,TRUE);
    }
    if(stats->din_vs_src.arr) {
      g_array_free(stats->din_vs_src.arr,TRUE);
    }
    if(stats->sin_vs_ready.pt) {
      adamant_plot_sin_vs_ready_free(&stats->sin_vs_ready);
    }
    if(stats->sin_vs_src.pt) {
      adamant_plot_sin_vs_src_free(&stats->sin_vs_src);
    }

    g_free(stats);
  }
}

void adamant_initialize_stats(AdamantRuntime *runtime)
{
    AdamantStats *stats = runtime->stats;
    AdamantConfig *config = runtime->config;

    //! DEBUG
    /* exeQueue = g_queue_new (); */
    /* if(config->dd_print_sin_bfd != NULL) */
    /*   { */
    /* 	adamant_bfd_init(config->dd_print_sin_bfd); */
    /*   } */
    //! END DEBUG

    if (config->hist_enabled)
        stats->histogram = g_array_new(TRUE, TRUE, sizeof(guint64));
    
    if (config->din_vs_ready.doit) {
        adamant_plot_din_vs_ready_init(runtime);
    } else {
        runtime->stats->din_vs_ready.arr=NULL;
    }
    
    if (config->din_vs_src.doit) {
        svt_plot_din_vs_src_init(&(config->din_vs_src),&(stats->din_vs_src));
    } else {
        runtime->stats->din_vs_src.arr=NULL;
    }
    
    if (config->sin_vs_ready.doit) {
        adamant_plot_sin_vs_ready_init(runtime);
    } else {
        runtime->stats->sin_vs_ready.pt=NULL;
    }
    
    if (config->sin_vs_src.doit) {
        adamant_plot_sin_vs_src_init(runtime);
    } else {
        runtime->stats->sin_vs_src.pt=NULL;
    }
    
    if (config->dep_graph_config.doit) {
        svt_plot_dep_graph_init(&(config->dep_graph_config),
                                &(stats->dep_graph));
    }
    
    if(config->variance_sampling_period != 0) {
        runtime->stats->variance = 
            g_array_new(FALSE, FALSE, sizeof(AdamantWindowVarianceInfo));
    }
    
    if (config->win_hist.doit) {
        guint i;
        
        if(config->win.w_num == 0) {
            g_error("Cannot use window histogram"
                    "(--window-hist) without windowing"); 
        }
        
        runtime->stats->win_hist.fp=fopen(config->win_hist.filname,"w");
        if(!runtime->stats->win_hist.fp) {
            g_error("Unable to open file %s for window histogram", 
                    config->win_hist.filname);
        }
        
        for(i=0;i<runtime->tmtrc->oper_table->len;i++) {
            g_array_index(runtime->tmtrc->oper_table, 
                          TmtTypedStaticOper, i).window_count=
                g_new0(guint64, config->win.w_num);
        }
    }
}

void
adamant_stats_ready_time(AdamantRuntime *runtime,
                         TmtOper *oper,
                         guint64 ready,
                         guint64 din,
                         guint thread_id,
                         guint num_din_srcs)
{
    AdamantConfig *config = runtime->config;
    AdamantStats *stats = runtime->stats;

    //! BEGIN DEBUG
    /* if(config->dd_print_sin > 0) */
    /*   { */
    /* 	GString * sinInfo = adamant_bfd_sinInfo(oper->soper->ip); */
    /* 	gchar * stringInfo = g_strdup_printf("%llx,%s\n",  */
    /* 					     oper->soper->ip, */
    /* 					     sinInfo->str ); */
    /* 	g_free(sinInfo); */
    /* 	g_queue_push_head( exeQueue,  stringInfo); */
    /* 	if (config->dd_print_sin <= g_queue_get_length(exeQueue)) */
    /* 	  { */
    /* 	    gpointer tempPointer = g_queue_pop_tail(exeQueue); */
    /* 	    if(NULL != tempPointer) */
    /* 	      { */
    /* 		g_free(tempPointer); */
    /* 	      } */
    /* 	  } */
    /*   } */
    //! END DEBUG

    if (config->hist_enabled) {
        GArray *histogram = stats->histogram;
        guint64 hist_bin_size = config->hist_bin_size;

        if (ready/hist_bin_size >= histogram->len)
            g_array_set_size(histogram, ready/hist_bin_size + 1);
        ++g_array_index(histogram, guint64, ready/hist_bin_size);
    }
    if(config->din_vs_ready.doit) {
        adamant_plot_din_vs_ready(runtime, oper, din, ready);
    }
    if(config->din_vs_src.doit) {
        svt_plot_din_vs_src(din, 
                            runtime->src_sched_info_array, 
                            num_din_srcs, 
                            &(stats->din_vs_src), 
                            &(config->din_vs_src));
    }

    if(config->sin_vs_ready.doit) {
        adamant_plot_sin_vs_ready(runtime, oper, din, oper->soper->ip, ready);
    }
    if(config->sin_vs_src.doit) {
        svt_plot_sin_vs_src(din, 
                            oper->soper->ip, 
                            runtime->src_sched_info_array, 
                            num_din_srcs, 
                            &(stats->sin_vs_src), 
                            &(config->sin_vs_src));
    }

    if (config->dep_graph_config.doit) {
        svt_plot_dep_graph(din, ready, thread_id,
                           runtime->src_sched_info_array,
                           num_din_srcs,
                           &(stats->dep_graph),
                           &(config->dep_graph_config));
    }

    if(config->store_schedule) {
        svt_sched_write(runtime->schedule_store_context, din, 
                        oper->soper->ip, ready);
    }

}


void adamant_stats_finalize(AdamantRuntime *runtime)
{
    AdamantConfig *config = runtime->config;
    AdamantStats *stats = runtime->stats;

    // START DEBUG
    /* gchar * exeString = (gchar *) g_queue_pop_head(exeQueue); */
    /* while (NULL != exeString) */
    /*   { */
    /* 	g_print(exeString); */
    /* 	g_free(exeString); */
    /* 	exeString = (gchar *) g_queue_pop_head(exeQueue); */
    /*   } */
    /* g_queue_free(exeQueue); */
    // END DEBUG
    
    /* TODO: do something more interesting than just printing to screen */
    if (config->hist_enabled) {
        guint i;
        GArray *histogram = stats->histogram;
        guint64 hist_bin_size = config->hist_bin_size;
        guint length = histogram->len;
        guint64 *d = (guint64 *)histogram->data;
        for (i=0; i<length; ++i)
            g_print("histogram[%"G_GUINT64_FORMAT"] = %"G_GUINT64_FORMAT"\n",
                    i*hist_bin_size, d[i]);
    }
    if (config->din_vs_ready.doit) {
        adamant_plot_din_vs_ready_finalize(runtime);
    }
    if (config->din_vs_src.doit) {
        svt_plot_din_vs_src_finalize(&(stats->din_vs_src));
    }
    
    if (config->sin_vs_ready.doit) {
        adamant_plot_sin_vs_ready_finalize(runtime);
    }
    if (config->sin_vs_src.doit) {
        adamant_plot_sin_vs_src_finalize(runtime);
    }
    
    if (config->dep_graph_config.doit) {
        svt_plot_dep_graph_finalize(&(stats->dep_graph),
                                    &(config->dep_graph_config));
    }
    
    if (config->predict_branches) {
        adamant_report_bp_accuracy(runtime);
    }
    
    if (config->store_schedule) {
        svt_context_write_destroy(runtime->schedule_store_context);
        runtime->schedule_store_context = NULL;
    }
    
    if(config->variance_sampling_period != 0) {
        FILE *out;
        AdamantWindowVarianceInfo *variance;
        int i;
        mpz_t mpz_tmp, two_to_32;
        mpf_t mpf_count, mpf_mean, sigma;
        guint64 mean;
        
        out = fopen(config->variance_filename, "w");
        
        mpz_init(mpz_tmp);
        mpz_init(two_to_32);
        
        mpf_init(sigma);
        mpf_init(mpf_count);
        mpf_init(mpf_mean);
        
        mpz_ui_pow_ui(two_to_32, 2,32);
    
        for(i = 0; i < runtime->stats->variance->len; i++) {
            variance = &g_array_index(runtime->stats->variance, AdamantWindowVarianceInfo, i);
            
            if(variance->count > 0) {
                mean = variance->sum / variance->count;
                
                mpz_set_ui(mpz_tmp, variance->count >> 32); // count
                mpz_mul(mpz_tmp, mpz_tmp, two_to_32);
                mpz_add_ui(mpz_tmp, mpz_tmp, variance->count & (0xFFFFFFFF));
                
                mpf_set_z(mpf_count, mpz_tmp);
                mpf_set_z(sigma, variance->sum_squared);
                
                mpf_div(sigma, sigma, mpf_count); // E[X^2]
                
                mpz_set_ui(mpz_tmp, variance->sum >> 32);  // sum
                mpz_mul(mpz_tmp, mpz_tmp, two_to_32);
                mpz_add_ui(mpz_tmp, mpz_tmp, variance->sum & (0xFFFFFFFF));
                
                mpf_set_z(mpf_mean, mpz_tmp);
                mpf_div(mpf_mean, mpf_mean, mpf_count); // E[X] 
                
                mpf_pow_ui(mpf_mean, mpf_mean, 2);  // E[X]^2
                
                mpf_sub(sigma, sigma, mpf_mean); // E[X^2] - E[X]^2
            } else {
                mean = 0;
                mpf_set_ui(sigma, 0);
            }

            fprintf(out, "%"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT" ", i*config->variance_sampling_period+1, variance->count, mean);
            gmp_fprintf(out, "%Ff\n", sigma);
        }
        
        fclose(out);
    }
    
    if (config->win_hist.doit) {
        int i;
        int j;
        for(i=0;i<runtime->tmtrc->oper_table->len;i++) {
            TmtTypedStaticOper *soper=
                &g_array_index(runtime->tmtrc->oper_table,TmtTypedStaticOper,i);
            fprintf(runtime->stats->win_hist.fp,"0x%llx:",soper->soper.ip);
            for(j=0;j<config->win.w_num;j++) {
                fprintf(runtime->stats->win_hist.fp,"%ld ",
                        soper->window_count[j]);
            }
            fprintf(runtime->stats->win_hist.fp,"\n");
        }
        fclose(runtime->stats->win_hist.fp);
    }
}

void adamant_stats_do_tick(AdamantConfig *config, AdamantRuntime *runtime,
                           guint64 opcount) 
{
    int position = 0, variable = 0;

    //!!! NOTE: I do not need this output right now
    //!! but it may be useful in future tests.
    //!! 
#if 0
    g_print("opcount: %" G_GUINT64_FORMAT
            "    maxcycle: %" G_GUINT64_FORMAT
            "    meminfos: %d \n",
            opcount, runtime->finish_time,
            runtime->meminfo_pgtbl->nummeminfos);
#endif

}

