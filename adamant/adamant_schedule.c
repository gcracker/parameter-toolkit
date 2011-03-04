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

#include <stdio.h>
#include <gmp.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

#include "adamant.h"
#include "adamant_schedule.h"
#include "adamant_latency.h"
#include "adamant_runtime.h"
#include "adamant_cd.h"
#include "adamant_cd_impl.h"
#include "adamant_branch_prediction.h"
#include "adamant_optimize.h"

#undef BDDZDDFINALTEST


/*
 * Local function declarations
 */
static guint64
adamant_schedule_oper(AdamantRuntime *runtime, TmtOper *oper, 
                      TmtOper *next_oper, guint64 opcount);

static void
find_src_regs(AdamantRuntime * runtime, TmtOper * oper,
              GArray *src_sched_info, guint * src_index);

static void
update_dst_regs_ready(AdamantRuntime * runtime, TmtOper * oper,
                      guint64 ready, guint64 opcount, guint thread_id);

static void
find_src_mems(AdamantRuntime * runtime, TmtOper * oper,
              GArray *src_sched_info, guint * src_index);

static void
update_dst_mems_ready(AdamantRuntime * runtime, TmtOper * oper,
                      guint64 ready, guint64 opcount, guint thread_id);

static mpz_t two_to_32;
static mpz_t mpz_tmp;

void adamant_schedule_program(AdamantRuntime * runtime)
{
    AdamantConfig *config = runtime->config;
    gint tmterr;
    TmtOper oper1, oper2;
    TmtOper *oper = &oper1, *next_oper = &oper2, *tmp;
    guint64 opcount = 0;
    guint64 tick_time = 0;
    guint64 bdd_tick_time = 0;
    gboolean tick_enabled = config->tick_enabled;
    guint64 tick_interval = config->tick_interval;
    guint64 bdd_tick_interval = config->dd_tick_interval;
    guint i;
    
    /* Some ugliness for variance calculations */
    mpz_init(mpz_tmp);
    mpz_init(two_to_32);
    mpz_ui_pow_ui(two_to_32, 2,32);
    
    
    tmt_readcontext_read(&tmterr, runtime->tmtrc, next_oper);
    while (tmterr == TMTIO_OK) {
        guint64 ready;
        
        /* Swap the pointers oper and next oper */
        tmp = oper;
        oper = next_oper;
        next_oper = tmp;
        tmt_readcontext_read(&tmterr, runtime->tmtrc, next_oper);
        
        for(i = 0; i < 2; i++) 
	  {
            TmtTypedStaticOper *copy;
            if(config->split_post_increment && 
               (oper->soper->num_mem_src > 0 ||
                oper->soper->num_mem_dst > 0) &&
               ((copy = g_hash_table_lookup(runtime->split_sopers, 
                                            &oper->soper->ip)) != NULL)) 
	      {
                /* We want to let the instruction (or more specifically its
                   destinations) be scheduled at two seperate times, but we
                   only want it to count as a single operation */
                if(i == 0) opcount--;
                if(i == 1) oper->soper = (TmtStaticOper*)copy;
	      } 
	    else 
	      {
                /* Force the loop to iterate only once */
                i = 1;
	      } 
            
            /* We need to do this here since stack unwinds are necessary even
               for operations with no effect */
            if(runtime->cdstack) {
                preupdate_cdstack(runtime, oper);
            }
            
            /* Remember not to count instructions with no effect */
            if(instruction_has_effect_when_qp0(runtime->config->isa,oper) &&
               !instruction_is_nop(runtime->config->isa,oper)) 
	      { 
                ready = adamant_schedule_oper(runtime, 
                                              oper, 
                                              tmterr == TMTIO_OK ? next_oper : NULL,
                                              opcount);
                if (runtime->config->ia64_translate) 
		  {
                    ia64_update_renameinfo(runtime->ia64_stfp, &runtime->ia64_reg_stack, oper);
		  }
                
                ++opcount;
                tick_time++;
                bdd_tick_time++;
                if (tick_enabled && (tick_time == tick_interval)) 
                    {
                        adamant_stats_do_tick(config, runtime, opcount);
                        if(!(config->use_zdds))
                            {

                                adamant_bdd_tick(config, runtime, opcount);
                            }
                        else
                            {

			      adamant_zdd_tick(runtime, opcount);
                            }

                        tick_time = 0;
                    }
                
                if (bdd_tick_time == bdd_tick_interval) 
                    {
                        if(!(config->use_zdds))
                            {
                                adamant_bdd_tickoutput(config, runtime, opcount);
                            }
                        else
                            {
                                adamant_zdd_tickoutput(runtime, opcount);
                            }

                        bdd_tick_time = 0;                        
                    }
            }
        }
        
        /* maxopcount of 0 indicates run to finish,
           Notice that opcount will never be 0 here. */
        //! runtime->done is used to stop BDD creation also
        if ((opcount == runtime->config->maxopcount) || (runtime->done != 0))
            {
                break;
            }
    }

    adamant_stats_finalize(runtime);

    //! finalize the DD creation
    if(!(config->use_zdds))
        {
            adamant_bdd_finalize(runtime, opcount);
        }
    else
        {
            adamant_zdd_finalize(runtime, opcount);
        }

    
    g_print("Maximum cycle to finish the program: %" G_GUINT64_FORMAT "\n",
            runtime->finish_time);
    g_print("Total ops: %" G_GUINT64_FORMAT "\n", opcount);
    g_print("Average IPC: %0.2f\n",
            (double) opcount / (double) runtime->finish_time);
    
    if (tmterr == TMTIO_ERROR) {
        printf("**ERROR**: Trace file ended abnormally, opcount = %"G_GUINT64_FORMAT"\n",
               opcount);
        exit(-1);
    }
}

guint64 adamant_schedule_oper(AdamantRuntime * runtime, 
                              TmtOper * oper, 
                              TmtOper * next_oper, 
                              guint64 opcount)
{
    guint64 ready;
    guint64 windows;
    AdamantConfig *config = runtime->config;

    AdamantWindow * thewindow = NULL;
    guint w_id = 0;
    guint num_din_srcs = 0;
    guint total_mem_src_size;
    guint i;
    SchedInfo *cdinfo = NULL;

    // START DEBUG
    //    if((oper->is_syscall))
    //      {
    //	g_print("SIN: %"G_GUINT64_FORMAT" SYS: %"G_GUINT64_FORMAT"\n", 
    //		oper->soper->ip, oper->sysno);
    //      }
    // END DEBUG
    
    // Grow the source schedule info array if necessary
    total_mem_src_size = 1; // 1 for the control dependence
    for(i = 0; i < oper->soper->num_mem_src; i++) {
        total_mem_src_size += oper->mem_src_size[i];
    }
    if(config->ia64_translate) {
        /* Must add 64 since the PR register could be accessed bloating
           the number of accessed registers by 64 */
        total_mem_src_size += 64;
    }
    if (runtime->src_sched_info_array->len < 
        (oper->soper->num_reg_src + total_mem_src_size)) {
        g_array_set_size(runtime->src_sched_info_array,
                         (oper->soper->num_reg_src + total_mem_src_size));
    }

    // Find the ready time of register and memory operands
    find_src_regs(runtime, oper, runtime->src_sched_info_array, &num_din_srcs);

    //    if(!oper->is_syscall )
    //      {
	find_src_mems(runtime, oper, runtime->src_sched_info_array, &num_din_srcs);
	//      }

    // Find the time due to control dependences
    if(runtime->cdstack) {
        cdinfo = check_control_deps(runtime, oper);
        if(cdinfo != NULL) {
            g_array_index(runtime->src_sched_info_array, 
                          SchedInfo *, num_din_srcs) = cdinfo;
            num_din_srcs++;
        }
    }

    // Find the window time constraints
    if (runtime->wrt) {
        w_id = 
            adamant_window_runtime_choose(runtime->wrt,
                                          runtime->src_sched_info_array,
                                          num_din_srcs,
                                          config->communication_latency);
        thewindow = adamant_window_runtime_index(runtime->wrt, w_id);
        windows = adamant_window_head(thewindow);

        /* We can compute the following max even when not doing in order
           branches since the last_mispredicted_branch stuff will only be
           updated if we are doing in order branches */
        windows = MAX(windows, thewindow->last_mispredicted_branch.ready);
    } else {
        /* Otherwise, find the earliest time that this op can be inserted
           into a window */
        w_id = 0;
        windows = 0;   
        /* We can compute the following max even when not doing in order
           branches since the last_mispredicted_branch stuff will only be
           updated if we are doing in order branches */
        windows = MAX(runtime->last_mispredicted_branch.ready, windows);
    }

    /*
      g_print("IP: %llx, opc=%s, readytime=%lld, windowtime=%lld\n",
      oper->soper->ip, oper->soper->opcode, ready, windows);
    */


    /* After finding the ready times for all the different pieces, pick
       the biggest one */
    ready = windows;
    for(i = 0; i < num_din_srcs; i++) {
        SchedInfo *sched_info = g_array_index(runtime->src_sched_info_array, 
                                              SchedInfo*, i);
        guint64 local_ready = sched_info->ready;
        if(w_id != sched_info->thread_id) {
            local_ready += config->communication_latency; 
        }
        ready = MAX(ready, local_ready);
    }

    ready += adamant_compute_latency(runtime, oper);

    /* ====================================================================== */

    adamant_stats_ready_time(runtime, oper, ready, opcount, w_id, num_din_srcs);

    /* ====================================================================== */

    if(!(config->use_zdds))
        {
            //! ready time for BDD creation
            adamant_bdd_ready_time(runtime, oper, ready, opcount, w_id, num_din_srcs);
        }
    else
        {
            //! ready time for ZDD creation
            adamant_zdd_ready_time(runtime, oper, ready, opcount, w_id, num_din_srcs);
        }

    // START DEBUG
    /* if((oper->is_syscall)) */
    /*   { */
    /* 	g_print("SIN: %"G_GUINT64_FORMAT" SYS: %"G_GUINT64_FORMAT" RDY: %"G_GUINT64_FORMAT"\n",  */
    /* 		oper->soper->ip, oper->sysno, ready); */
    /*   } */
    // END DEBUG
    
    update_dst_regs_ready(runtime, oper, ready, opcount, w_id);
    
    //    if(!oper->is_syscall)
    //      {
    update_dst_mems_ready(runtime, oper, ready, opcount, w_id);
	//      }

    if (runtime->wrt) {
        // update chosen window with commit time of instruction
        adamant_window_insert(thewindow, ready);
        // update histogram of which ROB each static instruction has gone to
        // adamant_stats_window_histogram(runtime, oper, adamant_rob_id(therob));
        if((config->variance_sampling_period != 0)) {
            if((ready % config->variance_sampling_period) == 1) {
                AdamantWindowVarianceInfo *variance;

                guint index = ready/config->variance_sampling_period;
                if(index >= runtime->stats->variance->len) {
                    guint start = runtime->stats->variance->len;
                    g_array_set_size(runtime->stats->variance, index+1);
                    for(; start < index+1; start++) {
                        variance = &g_array_index(runtime->stats->variance, 
                                                  AdamantWindowVarianceInfo, start);
                        variance->count = 0;
                        variance->sum = 0;
                        mpz_init_set_ui(variance->sum_squared, 0);
                    }
                }
                variance = &g_array_index(runtime->stats->variance, 
                                          AdamantWindowVarianceInfo, index);

                /* Increment the sum */
                variance->sum += opcount;

                /* Incrument the sum squared */
                mpz_set_ui(mpz_tmp, opcount >> 32);
                mpz_mul(mpz_tmp, mpz_tmp, two_to_32);
                mpz_add_ui(mpz_tmp, mpz_tmp, opcount & (0xFFFFFFFF));
                mpz_addmul(variance->sum_squared, mpz_tmp, mpz_tmp);

                /* Count how many things are in each bin */
                variance->count++;
            }
        }

        if (runtime->config->win_hist.doit) {
            ((TmtTypedStaticOper *)(oper->soper))->window_count[w_id]++;
        }
    }

    if(runtime->cdstack) {
        gboolean prediction = FALSE;
        if(config->predict_branches) {
            prediction = adamant_predict_branch(runtime, oper, next_oper);
        }
        update_cdinfo(runtime, oper, opcount, w_id, 
                      ready, 
                      cdinfo == NULL ? 0 : cdinfo->ready, 
                      cdinfo == NULL ? 0 : cdinfo->din_src,
                      prediction);
    }

    if (ready > runtime->finish_time)
        runtime->finish_time = ready;

    return ready;
}

void find_src_regs(AdamantRuntime * runtime, TmtOper * oper,
                   GArray *src_sched_info, guint * src_index)
{
    SchedInfo *reg_ready_time = NULL;
    SchedInfo *reg_info = runtime->reg_info;
    gboolean *ignore_regs = runtime->ignore_regs;
    guint8 i;

    // only checking RAW dependencies
    for (i = 0; i < oper->soper->num_reg_src; ++i) {
        guint reg = (guint) oper->soper->reg_src[i];
        // Bump up max_regs if this is triggered
        g_assert(reg < runtime->max_regs);

        if (!ignore_regs[reg]) {
            if (runtime->config->ia64_translate) {
                // Itanium-specific translation
                if(reg == REG_PR) { 
                    // Do this before rotation since we're going to effect every 
                    // non-constant PR
                    int i; 
                    for(i=0;i<64;i++) {
                        g_array_index(src_sched_info, SchedInfo *, *src_index) = &(reg_info[REG_PBASE+i]);
                        (*src_index)++;
                    } 
                } else {
                    reg = ia64_translate_regs(&runtime->ia64_reg_stack, reg);
                    g_array_index(src_sched_info, SchedInfo *, *src_index) = &(reg_info[reg]);
                    (*src_index)++;
                }
            } else {
                gboolean handled = 0;

                if(runtime->config->opti.ive) {
                    if(runtime->config->ppc64_translate) {
                        switch(((TmtTypedStaticOper *)oper->soper)->type) {
                        case PPC64_LOAD_ARITHMETIC_UPDATE:
                        case PPC64_LOAD_ZERO_UPDATE:
                        case PPC64_LOAD_UPDATE:
                        case PPC64_STORE_UPDATE:
                            if(oper->soper->num_mem_dst == 0 && 
                               oper->soper->num_mem_src == 0) {
                                // We have the address computation part of the update instrs
                                reg_ready_time = 
                                    adamant_IVE_read(runtime, reg, 
                                                     runtime->config->opti.unroll_factor);
                                handled = 1;
                            }
                            break;
                        case PPC64_MOV:
                        case PPC64_EXTEND:
                        case PPC64_ADDI:
                            reg_ready_time = 
                                adamant_IVE_read(runtime, reg, 
                                                 runtime->config->opti.unroll_factor);
                            handled = 1;
                            break;
                        }
                    }
                }
                if(!handled) {
                    reg_ready_time = &reg_info[reg];
                }
            }
	  
            // store pointer to sched info of this reg
            g_array_index(src_sched_info, SchedInfo *, *src_index) = reg_ready_time;
            (*src_index)++;
        }
    }
}

void update_dst_regs_ready(AdamantRuntime * runtime, TmtOper * oper,
                           guint64 ready, guint64 opcount, guint thread_id)
{
    SchedInfo *reg_info = runtime->reg_info;
    gboolean *ignore_regs = runtime->ignore_regs;
    guint8 i;
    guint count = 0;
    guint src_reg = 0;
    guint ld_reg_dst = 0;
    guint64 ea = 0;
    guint32 ld_size = 0;
  
    if(runtime->config->opti.ive) {
        if(runtime->config->ppc64_translate) {
            switch(((TmtTypedStaticOper *)oper->soper)->type) {
            case PPC64_EXTEND:
                count = 0; 
                for(i = 0; i < oper->soper->num_reg_src; i++) {
                    if(PPC64_is_gp(oper->soper->reg_src[i])) {
                        count++;
                        src_reg = oper->soper->reg_src[i];
                    }
                }
                g_assert(count == 1);
                break;
            case PPC64_MOV:
                count = 0; 
                for(i = 0; i < oper->soper->num_reg_src; i++) {
                    if(PPC64_is_gp(oper->soper->reg_src[i])) {
                        if(count == 0) src_reg = oper->soper->reg_src[i];
                        else g_assert(src_reg == oper->soper->reg_src[i]);
                        count++;
                    }
                }
                g_assert(count == 2);
                break;
            case PPC64_ADDI:
                count = 0; 
                for(i = 0; i < oper->soper->num_reg_src; i++) {
                    if(PPC64_is_gp(oper->soper->reg_src[i])) {
                        count++;
                        src_reg = oper->soper->reg_src[i];
                    }
                }
                g_assert(count == 0 || count == 1);
                break;

            case PPC64_LOAD_ARITHMETIC:
            case PPC64_LOAD_ARITHMETIC_INDEXED:
            case PPC64_LOAD_ARITHMETIC_UPDATE:
            case PPC64_LOAD_ARITHMETIC_INDEXED_UPDATE:
            case PPC64_LOAD_ZERO:
            case PPC64_LOAD_ZERO_INDEXED:
            case PPC64_LOAD_ZERO_UPDATE:
            case PPC64_LOAD_ZERO_INDEXED_UPDATE:
            case PPC64_LOAD:
            case PPC64_LOAD_INDEXED:
            case PPC64_LOAD_UPDATE:
            case PPC64_LOAD_INDEXED_UPDATE:
                if(oper->soper->num_mem_src == 1) {
                    ea = oper->mem_src[0];
                    ld_size = oper->mem_src_size[0];
                }
                ld_reg_dst = oper->soper->reg_dst[0];
                break;
            }
        }
    }

    for (i = 0; i < oper->soper->num_reg_dst; ++i) {
        guint reg = (guint) oper->soper->reg_dst[i];
        g_assert(reg < runtime->max_regs);
        if (!ignore_regs[reg]) {

            if (runtime->config->ia64_translate) {
                if(reg == REG_PR) { /* Do this before rotation since we're
                                       going to effect every non-constant
                                       PR */
                    int i; 
                    for(i=1;i<64;i++) { /* Recall PRO is constant 1 */
                        reg_info[REG_PBASE+i].ready = ready;
                        reg_info[REG_PBASE+i].din_src = opcount;
                        reg_info[REG_PBASE+i].thread_id = thread_id;
                    } 
                    continue;
                } else if(((reg-REG_PBASE)==0) ||
                          ((reg-REG_GBASE) == 0) ||
                          (((reg-REG_FBASE) & (~0x1)) ==0)) {
                    continue;
                }
                reg = ia64_translate_regs(&runtime->ia64_reg_stack, reg);
            }
            /* Don't update dependence for instructions that don't write
               their destination */
            if(instruction_has_effect_when_qp0(runtime->config->isa, oper) ||
               oper->qp) {
                reg_info[reg].ready = ready;
                reg_info[reg].din_src = opcount;
                reg_info[reg].thread_id = thread_id;

                if(runtime->config->opti.ive) {
                    if(runtime->config->ppc64_translate) {
                        switch(((TmtTypedStaticOper *)oper->soper)->type) {
                        case PPC64_MOV:
                        case PPC64_EXTEND:
                            if(!PPC64_is_gp(reg)) goto reset;
#if 0
                            if(reg == PPC64_R31) printf("Moving to r31 -- %08llx @ %lld\n", oper->soper->ip, ready);
#endif
                            adamant_IVE_link(runtime, reg, src_reg);
                            break;
	      
                        case PPC64_ADDI:
                            if(!PPC64_is_gp(reg)) goto reset;
                            if(count != 1) goto reset;
#if 0
                            if(src_reg == PPC64_R31) printf("Adding from r31 -- %08llx @ %lld\n", oper->soper->ip, ready);
#endif
                            adamant_IVE_chain(runtime, reg, src_reg, 
                                              ready, opcount, thread_id);
                            break;

                        case PPC64_STORE_UPDATE:
                            if(!PPC64_is_gp(reg)) goto reset;
                            if(oper->soper->num_mem_dst != 0) goto reset;
#if 0
                            if(src_reg == PPC64_R31) printf("Adding from r31 -- %08llx @ %lld\n", oper->soper->ip, ready);
#endif
                            adamant_IVE_chain(runtime, reg, oper->soper->reg_src[0], 
                                              ready, opcount, thread_id);
                            break;

                        case PPC64_LOAD_ARITHMETIC_UPDATE:
                        case PPC64_LOAD_ZERO_UPDATE:
                        case PPC64_LOAD_UPDATE:
                            if(PPC64_is_gp(reg) && oper->soper->num_mem_src == 0) {
#if 0
                                if(src_reg == PPC64_R31) printf("Adding from r31 -- %08llx @ %lld\n", oper->soper->ip, ready);
#endif
                                adamant_IVE_chain(runtime, reg, oper->soper->reg_src[0], 
                                                  ready, opcount, thread_id);
                                break;
                            }

                        case PPC64_LOAD_ARITHMETIC:
                        case PPC64_LOAD_ARITHMETIC_INDEXED:
                        case PPC64_LOAD_ARITHMETIC_INDEXED_UPDATE:
                        case PPC64_LOAD_ZERO:
                        case PPC64_LOAD_ZERO_INDEXED:
                        case PPC64_LOAD_ZERO_INDEXED_UPDATE:
                        case PPC64_LOAD:
                        case PPC64_LOAD_INDEXED:
                        case PPC64_LOAD_INDEXED_UPDATE:
                            if(!runtime->config->ignore_stack) goto reset;
                            if(!(ea >= runtime->config->stack_start && 
                                 (ea + ld_size - 1) <= runtime->config->stack_end)) goto reset;
                            if(reg != ld_reg_dst) goto reset;
#if 0
                            printf("Loading from stack -- r%-2d = [%llx] -- %08llx @ %lld -- ", ld_reg_dst - PPC64_R0, ea, oper->soper->ip, ready);
#endif
                            adamant_IVE_link_from_mem(runtime, ld_reg_dst, ea, ld_size);

                            break;
	      
                        default:
                        reset:
#if 0
                            if(reg == PPC64_R31) printf("Resetting r31 -- %08llx\n", oper->soper->ip);
#endif
                            adamant_IVE_reset(runtime, reg, ready, opcount, thread_id);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void find_src_mems(AdamantRuntime * runtime, TmtOper * oper,
                   GArray* src_sched_info, guint * src_index)
{
    guint8 i;
    MemInfoPageTableEntry *meminfo;

    for (i=0; i<oper->soper->num_mem_src; ++i) {
        guint32 size = oper->mem_src_size[i];
        guint64 ea = oper->mem_src[i];
        guint64 ea_index = ea & MIPT_INDEX_MASK;
        guint64 ea_line = ea & MIPT_LINE_MASK;
    
        while (size) {
            if ((meminfo = MemInfoPageTable_lookup(runtime->meminfo_pgtbl, 
                                                   ea_line))) { 
                do {
                    // store pointer to sched info of this mem address
                    g_array_index(src_sched_info, SchedInfo *, *src_index) = 
                        &(meminfo->data[ea_index]);
                    (*src_index)++;

                    --size; ++ea_index;
                    if (ea_index >= MIPT_LINE_SIZE) {
                        ea_index = 0;
                        ea_line += MIPT_LINE_SIZE;
                        break;
                    }
                } while (size);
            } else {
                if ((ea_index + size) > MIPT_LINE_SIZE) {
                    size -= MIPT_LINE_SIZE - ea_index;
                    ea_index = 0;
                    ea_line += MIPT_LINE_SIZE;
                } else {
                    size = 0;
                }
            }
        }
    }
}

void update_dst_mems_ready(AdamantRuntime * runtime, TmtOper * oper,
                           guint64 ready, guint64 opcount, guint thread_id)
{
    guint8 i;
    MemInfoPageTable *meminfo_pgtbl = runtime->meminfo_pgtbl;
    MemInfoPageTableEntry *meminfo;
  
    for (i=0; i<oper->soper->num_mem_dst; ++i) {
        guint32 size = oper->mem_dst_size[i];
        guint64 ea = oper->mem_dst[i];
        guint64 ea_index = ea & MIPT_INDEX_MASK;
        guint64 ea_line = ea & MIPT_LINE_MASK;
    
        while (size) {
            if (!(meminfo=MemInfoPageTable_lookup(meminfo_pgtbl, ea_line))) {
                MemInfoPageTableEntry *ami = 
                    g_new0(MemInfoPageTableEntry, 1);
                ami->ea = ea_line;
                MemInfoPageTable_add(meminfo_pgtbl, ea_line, ami);
                meminfo = ami;
            }
            do {
                meminfo->data[ea_index].ready = ready;
                meminfo->data[ea_index].din_src = opcount;
                meminfo->data[ea_index].thread_id = thread_id;
                --size; ++ea_index;
                if (ea_index >= MIPT_LINE_SIZE) {
                    ea_index = 0;
                    ea_line += MIPT_LINE_SIZE;
                    break;
                }
            } while (size);
        }

        if(runtime->config->opti.ive) {
            if(runtime->config->ppc64_translate) {
                switch(((TmtTypedStaticOper *)oper->soper)->type) {
                case PPC64_STORE:
                case PPC64_STORE_INDEXED:
                case PPC64_STORE_UPDATE:
                case PPC64_STORE_INDEXED_UPDATE:
                    if(!runtime->config->ignore_stack) break;
                    if(!(ea >= runtime->config->stack_start && 
                         (ea + size - 1) <= runtime->config->stack_end)) break;
#if 0
                    printf("Storing to stack -- [%llx] = r%-2d -- %08llx @ %lld -- ", ea, oper->soper->reg_src[0] - PPC64_R0, oper->soper->ip, ready);	  
#endif
                    adamant_IVE_link_to_mem(runtime, ea, oper->mem_dst_size[i], oper->soper->reg_src[0]);
	  
                    break;
                }
            }
        }
    }
}

