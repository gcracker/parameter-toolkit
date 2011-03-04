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

#include "adamant_bdd.h"
#include "adamant_plot.h"
#include "adamant_branch_prediction.h"
#include "adamant_window.h"
#include <gmp.h>
#include <sys/time.h>
#include <time.h>
#include <cudd.h>
#include <cuddInt.h>


#define BUILDBDD
#define matchkeywd(str,key) (strncmp(str,key,strlen(key))==0)
#define DELIMITERS    " \r\n\t!@#$%^&*()_+-={}|\\:\"'?¿/.,<>’¡º×÷‘"

#define BDDNUM 128
#define THREADCNT 2
#undef LAYERS

//! External Functions
extern int adamantbdd_layer_close(DdManager * manager, 
                                  DdNode ** set);
extern int adamantbdd_layer_init(void);
extern DdNode * adamantbdd_layer_addtuple(DdManager * manager, 
                                          DdNode * set, guint64 x, guint64 y);


/*! Local Global Variables */
guint64  MEMORYCHECK = (1073741824/10);
unsigned long MEMORYCHECKDELAY = 100000;
unsigned long MEMORYCHECKDELTA = (1073741824/100);
unsigned long delaycount = 0;
DdManager * local_manager;
DdNode * local_temp_node = NULL;
DdNode * dinrdy_temp_node = NULL;
DdNode * dinsin_temp_node = NULL;
DdNode * dindin_temp_node = NULL;
double oldshuffletime = 0;
int first_tick = 1;
unsigned int first_time = 0;
int split_point = 0;
int oldreordernum = -1;
int * ordr = NULL;
int * g_var_order;
AdamantConfig * global_config = NULL; 

int THREADED = 0;


/* these variables are used for some fairly dumb
   counting and should be removed soon
*/
guint64 dindintuples = 0;
struct timeval timecount;
int no_var_swap = 0;

//! \warning TEST VARIABLES
guint64 dintest = 10, readytest = 39;
pthread_t * bddThreads;
thread_args * threadArgs;

//!!! ** DEBUG VARIABLES **
guint64 bdd_dead_count = 0;

void adamant_initialize_bddstats(AdamantRuntime *runtime)
{
    AdamantConfig *config = runtime->config;
    int u = 0;

    //! set up a global config file
    global_config = runtime->config;

    //! initialize the reorder variable
    bdd_reordering = FALSE;

    
    if (config->dd_din_vs_ready.doit ||
        config->dd_din_vs_sin.doit ||
        config->dd_din_vs_din.doit) {
        
        int tempUniqueSlots = CUDD_UNIQUE_SLOTS * 100;


        /* initialize the CUDD BDD manager */
        int k = 0;
        ordr = calloc(BDDNUM, sizeof(int)); /* variable never freed, but should not leak */

        bddThreads = calloc(THREADCNT, sizeof(pthread_t));
        threadArgs = calloc(THREADCNT, sizeof(thread_args));
                
        //! initialize the CUDD BDD managers 
        runtime->stats->dd_stats.manager = Cudd_Init(BDDNUM, 0, 
                                                      (config->dd_slots_multi * CUDD_UNIQUE_SLOTS),
                                                      config->dd_mem_init, config->dd_mem_cap);
        local_manager = Cudd_Init(BDDNUM, 0,(config->dd_slots_multi * CUDD_UNIQUE_SLOTS),
                                  config->dd_mem_init, config->dd_mem_cap);

        guint32 tempReadLoose = Cudd_ReadLooseUpTo(runtime->stats->dd_stats.manager);
        Cudd_SetLooseUpTo(runtime->stats->dd_stats.manager, 
                          (tempReadLoose * config->dd_looseup_multi));

        tempReadLoose = Cudd_ReadLooseUpTo(local_manager);
        Cudd_SetLooseUpTo(local_manager, 
                          (tempReadLoose * config->dd_looseup_multi));

        //! setup the managers for multi-threaded work
        if (THREADED == 1)
            {
                //! multi threaded work
                for (u = 0; u < THREADCNT; u++)
                    {
                        threadArgs[u].threadManager = Cudd_Init(BDDNUM, 0, tempUniqueSlots, 
                                                                config->dd_mem_init, config->dd_mem_cap);

                    }
                
            }

            
        if (config->dd_order != NULL)
            {
                FILE * forder = NULL;
                int tpos = 0, tvar = 0;
                forder = fopen(config->dd_order,"r+");
                if(forder != NULL)
                    {
                        while((!feof(forder)) && (tpos < BDDNUM))
                            {
                                fscanf(forder, "%d", &ordr[tpos]);
                                tpos++;
                            }
                        fclose(forder);
                    }
                else
                    {
                        g_print("Error opening BDD order file\n");
                    }
            }
        else
            {

                //! if there is no variable order file
                //! default to the interlaced MSB to LSB
                //! so that it is ParaMeter compatible
                for(k = 0; k < 64; k++)
                    {
                        ordr[127-2*k] = k;
                        ordr[127-(2*k+1)] = (64+k);
                    }
            }
        
#if 0
        
        // create the ordering we want to use
        for(k = 0; k < 64; k++){
#define BREAKP 30
#define BREAKPI (64-BREAKP)
#if 0
            ordr[127-2*k] = k;
            ordr[127-(2*k+1)] = (64+k);
#else
#    if 1
            ordr[2*k] = k;
            ordr[2*k+1] = (64+k);
#    else
            if(k < BREAKP){
                ordr[(2*BREAKP-1)-2*k] = k;
                ordr[(2*BREAKP-1)-(2*k+1)] = (64+k);
            }
            else{
                ordr[(2*BREAKPI-1)-2*(k-BREAKP)/*+2*BREAKP*/] = k;
                ordr[(2*BREAKPI-1)-(2*(k-BREAKP)+1)/*+2*BREAKP*/] = (64+k);
            }
#    endif
#endif
        }
        
#endif
	
        
        Cudd_ShuffleHeap(runtime->stats->dd_stats.manager, ordr);


/*         //! print out the variable positions for debugging */
/*         for(k = 0; k < 128; k++) */
/*             { */
/*                 g_print("position %d has variable %d\n", k, */
/*                         Cudd_ReadInvPerm(runtime->stats->dd_stats.manager,k)); */
/*             } */
                

        if (THREADED == 1)
            {
                //! multi threaded work
                for (u = 0; u < THREADCNT; u++)
                    {
                        Cudd_ShuffleHeap(threadArgs[u].threadManager, ordr);
                    }
                
            }
        
        //	for(k = 0; k < 128; k = k + 2){
        //	  Cudd_MakeTreeNode(runtime->stats->dd_stats.manager, k, 2, MTR_DEFAULT); /* or MTR_FIXED */
        //	}
        
        /* turn on dynamic reordering */
        if(config->dd_print_varmotion == TRUE)
            {
                Cudd_AutodynEnable(runtime->stats->dd_stats.manager, CUDD_REORDER_SIFT);
        //	Cudd_AddHook(runtime->stats->dd_stats.manager, &DynReorderHook ,CUDD_POST_REORDERING_HOOK);
                bdd_reordering = TRUE;
            }
        //! turn off automatic garbage collection
        if(config->dd_garbage_collect != 0)
            {
                Cudd_DisableGarbageCollection(runtime->stats->dd_stats.manager);
            }

        //! multi threaded work
        if (THREADED == 1)
            {
                for (u = 0; u < THREADCNT; u++)
                    {
                        Cudd_DisableGarbageCollection(threadArgs[u].threadManager);
                    }
            }
        
        //! initialize the BDDs
        if (config->dd_din_vs_ready.doit) {
            runtime->stats->dd_stats.din_vs_ready = Cudd_ReadLogicZero(runtime->stats->dd_stats.manager);
            Cudd_Ref(runtime->stats->dd_stats.din_vs_ready);
        }    
        if (config->dd_din_vs_sin.doit) {
            runtime->stats->dd_stats.din_vs_sin = Cudd_ReadLogicZero(runtime->stats->dd_stats.manager);
            Cudd_Ref(runtime->stats->dd_stats.din_vs_sin);
        }
        if (config->dd_din_vs_din.doit) {
            runtime->stats->dd_stats.din_vs_din = Cudd_ReadLogicZero(runtime->stats->dd_stats.manager);
            Cudd_Ref(runtime->stats->dd_stats.din_vs_din);
        }
      

        /* if the outfile contains a BDD, see if it contains any information */        
        if(config->dd_din_vs_ready.outfilename != NULL)
            {
                FILE * newfile;
                 
                newfile = fopen(config->dd_din_vs_ready.outfilename, "r");
                
                if(newfile != NULL)
                    {
                        //! check the headerinfo for information
                        adamant_bdd_headerinit(newfile, runtime );
                        fclose(newfile);	
                    }


            }
        if(config->dd_din_vs_sin.outfilename != NULL)
            {
                FILE * newfile;  

                newfile = fopen(config->dd_din_vs_sin.outfilename, "r");
            
                if(newfile != NULL)
                    {
                        //! check the headerinfo for information
                        adamant_bdd_headerinit(newfile, runtime );
                        fclose(newfile);	
                    }
            }

        if(config->dd_din_vs_din.outfilename != NULL)
            {
                FILE * newfile;

                newfile = fopen(config->dd_din_vs_din.outfilename, "r");
                
                if(newfile != NULL)
                    {

                        //! check the headerinfo for information
                        adamant_bdd_headerinit(newfile, runtime );
                        fclose(newfile);	
                    }
            }

        //! set a manual garbage collection size based on the set memory cap        
        if(config->dd_mem_cap != 0)
            {
                MEMORYCHECK = config->dd_mem_cap;
            }

        //! this section spawns the threads
        if(THREADED == 1)
            {
                //! multi threaded work
                for (u = 0; u < THREADCNT; u++)
                    {
                        //! initialize thread arguments
                        threadArgs[u].x = calloc(1, sizeof(int));
                        *(threadArgs[u].x) = 0;
                        threadArgs[u].y = calloc(1, sizeof(int));
                        *(threadArgs[u].y) = 0;  
                        threadArgs[u].stayalive = calloc(1, sizeof(int));
                        *(threadArgs[u].stayalive) = 1;  
                        threadArgs[u].threadMutex = calloc(1, sizeof(pthread_mutex_t));
                        threadArgs[u].holdMutex = calloc(1, sizeof(pthread_mutex_t));

                        threadArgs[u].threadNode = Cudd_ReadLogicZero(threadArgs[u].threadManager);
                        pthread_mutex_init(threadArgs[u].threadMutex, NULL);
                        pthread_mutex_init(threadArgs[u].holdMutex, NULL);

                        //! set up the mutexes in the correct state
                        pthread_mutex_lock(threadArgs[u].holdMutex);
                        pthread_mutex_unlock(threadArgs[u].threadMutex);

                        pthread_create(&bddThreads[u], NULL, adamant_bdd_thread, (void*)(&threadArgs[u]));
                    }
            }
 
        //! grab the time when we startup
        gettimeofday(&timecount, NULL); 
        first_time = (unsigned int)(timecount.tv_sec);        
    }
    
#ifdef LAYERS
    //! initialize the BDD layering system
    adamantbdd_layer_init();
#endif

}


void
adamant_bdd_ready_time(AdamantRuntime *runtime,
                       TmtOper *oper,
                       guint64 ready,
                       guint64 din,
                       guint thread_id,
                       guint num_din_srcs)
{
    AdamantConfig *config = runtime->config;
    AdamantStats *stats = runtime->stats;
 
#if 1        
    if(config->dd_din_vs_ready.doit) 
        {
            if(din >= config->g_dd_start_din)
                {
                    if(THREADED == 1)
                        {
                            int tuple_done = 0;
                            int k = 0;
                            do{
                                for( k = 0; ((k < THREADCNT) && (tuple_done == 0)); k++)
                                    {
                                        //! see if this thread is busy
                                        if(pthread_mutex_trylock(threadArgs[k].threadMutex) == 0)
                                            {
                                                //! set up some variables
                                                *(threadArgs[k].x) = din;
                                                *(threadArgs[k].y) = ready;
                                                
                                                //! release the thread hold mutex
                                                pthread_mutex_unlock(threadArgs[k].holdMutex);

                                                //! and we are done with this tuple
                                                tuple_done = 1;
                                            }
                                    }

                            } while(tuple_done == 0);
                        }
                    else
                        {
                            DdNode *tmp;

                            //!!! DEBUG and TESTING CODE
#ifdef LAYERS
                            tmp = adamantbdd_layer_addtuple(runtime->stats->dd_stats.manager, 
                                                            runtime->stats->dd_stats.din_vs_ready, 
                                                            din, ready);

                            //                            adamantbdd_layer_addtuple_wb(runtime->stats->dd_stats.manager, 
                            //                                                               runtime->stats->dd_stats.din_vs_ready, 
                            //                                                               din, ready);
                            
                            // g_printf("Memory used:%u\n",
                            //     Cudd_ReadMemoryInUse(runtime->stats->dd_stats.manager));
                            
#else

                            tmp=adamant_bdd_add_tuple(runtime->stats->dd_stats.manager, 
                                                      runtime->stats->dd_stats.din_vs_ready, 
                                                      din, ready);

                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                runtime->stats->dd_stats.din_vs_ready);

#endif
                            
                            //!!! END DEBUGING STUFF


                            if(tmp == NULL)
                                {
                                    //! if this happens, the program is exploding

                                    /* Kill Adamantium Gracefully */
                                    runtime->done = 1;
                                    
                                    config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit = 
                                        config->dd_din_vs_din.doit = 0;	  	
                                    
                                    g_print("BDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);
                                    
                                    /* print out general statistics */
                                    Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);

                                    return;                            
                                }                            

#ifdef BUILDBDD

                            runtime->stats->dd_stats.din_vs_ready = tmp;
#else
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                tmp);
#endif
                        }
                }
        }
#else
    if(config->dd_din_vs_ready.doit) 
        {
            if(din >= config->g_dd_start_din)
                {
                    
                    DdNode *tmp;

                    //                    tmp=adamant_bdd_add_tuple(runtime->stats->dd_stats.manager, 
                    //                                              runtime->stats->dd_stats.din_vs_ready, 
                    //                                              din, ready);

                    tmp = adamantbdd_layer_addtuple(runtime->stats->dd_stats.manager, 
                                              runtime->stats->dd_stats.din_vs_ready, 
                                              din, ready);

                    if(tmp == NULL)
                        {
                            //! if this happens, the program is exploding
                            runtime->done = 1;
                            
                            config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit = 
                                config->dd_din_vs_din.doit = 0;	  	
                            
                            g_print("BDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);
                            
                            /* print out general statistics */
                            Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);
                            return;                            
                        }
#ifdef BUILDBDD
                    //                    Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                    //                                        runtime->stats->dd_stats.din_vs_ready);

                    runtime->stats->dd_stats.din_vs_ready = tmp;
#else
                    Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                        tmp);
#endif
                }
        }
#endif

    if(config->dd_din_vs_sin.doit) 
        {
            if(din >= config->g_dd_start_din)
                {
                    DdNode *tmp;
                    tmp=adamant_bdd_add_tuple(runtime->stats->dd_stats.manager, 
                                              runtime->stats->dd_stats.din_vs_sin, 
                                              din, oper->soper->ip);
                    if(tmp == NULL)
                        {
                            //! if this happens, the program is exploding
                            runtime->done = 1;
                            
                            config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit = 
                                config->dd_din_vs_din.doit = 0;	  	

                            g_print("BDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);
                            
                            /* print out general statistics */
                            Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);
                            return;                            
                        }
#ifdef BUILDBDD
                    Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                        runtime->stats->dd_stats.din_vs_sin);
                    runtime->stats->dd_stats.din_vs_sin = tmp;
#else
                    Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                        tmp);
#endif
                }
        }

    if(config->dd_din_vs_din.doit) 
        {
            if(din >= config->g_dd_start_din)
                {
                    DdNode *tmp;
                    int k;
                    
                    /* Loop for every din for this din */
                    for(k = 0; k < num_din_srcs; k++){
                        
                        // SchedInfo contains ready, din_src, thread_id (all guint64)
                        SchedInfo * si = g_array_index(runtime->src_sched_info_array, SchedInfo*, k);
                        
                        tmp = adamant_bdd_add_tuple(runtime->stats->dd_stats.manager, 
                                                    runtime->stats->dd_stats.din_vs_din, 
                                                    din, si->din_src);
                        if(tmp == NULL)
                            {
                                //! if this happens, the program is explodin
                                runtime->done = 1;
                            
                                config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit = 
                                    config->dd_din_vs_din.doit = 0;	  	

                                g_print("BDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);
                            
                                /* print out general statistics */
                                Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);
                                return;                            
                            }
#ifdef BUILDBDD                        
                        Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                            runtime->stats->dd_stats.din_vs_din);
                        
                        runtime->stats->dd_stats.din_vs_din = tmp;	
#else
                        Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                            tmp);
#endif                        
                        dindintuples = dindintuples + 1;
                    } // end for(k = 0...
                }// end if din > config...
        } // if(config->dd_din_vs_din...

    
    if(config->dd_din_vs_ready.doit ||
       config->dd_din_vs_sin.doit ||
       config->dd_din_vs_din.doit ||
       FALSE) { /* Add other bdd based stats checks here */
        
        
        delaycount++;	
        if(delaycount > MEMORYCHECKDELAY){
            delaycount = 0;

            if(config->dd_garbage_collect != 0 
               || config->dd_mem_cap != 0)
                {
                    
                    guint64 current_memory_use 
                        = Cudd_ReadKeys(runtime->stats->dd_stats.manager)*sizeof(DdNode);
                    
                    //! use this for more manual garbage collection
                    if((config->dd_garbage_collect != 0) &&
                       (current_memory_use >= (config->dd_garbage_collect)))
                        {
                            cuddGarbageCollect(runtime->stats->dd_stats.manager, 1);
                            //! update the memory use variable
                            current_memory_use = Cudd_ReadKeys(runtime->stats->dd_stats.manager)*sizeof(DdNode);
                        }
                    
                    //! this section checks to see if the BDD size,after garbage
                    //! collection, has exceded a preset limit
                    if((config->dd_mem_cap != 0) && 
                       (current_memory_use >= (config->dd_mem_cap)))
                        {
                            
                            /* Kill Adamantium Gracefully */
                            g_print("\nAdamantium Stopped Prematurely\n");
                            
                            runtime->done = 1;
                            
                            config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit = 
                                config->dd_din_vs_din.doit = 0;	  	
                            
                            g_print("BDD Creation Memory Limit Hit\n");
                            g_print("Din vs Din tuples: %"G_GUINT64_FORMAT"\n", dindintuples);
                            g_print("BDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);
                            
                            /* print out general statistics */
                            Cudd_PrintInfo(stats->dd_stats.manager, stdout);
                            
                        }
                }
#if 1
            /*!
              check memory usage and perform tasks based on that memory
              usage
            */
            
#if 0
            //! * this section will test memory use and create a partition
            //! * of the BDD
            // use this for more frequent garbage collection
            if((Cudd_ReadKeys(runtime->stats->dd_stats.manager)*sizeof(DdNode)) >= (MEMORYCHECK))
                {
                    // do manual garbage collection
                    cuddGarbageCollect(runtime->stats->dd_stats.manager, 1);
                    
                    if(config->dd_din_vs_ready.doit)
                        {
                            //! initialize this node if needed
                            if(dinrdy_temp_node == NULL)
                                {
                                    dinrdy_temp_node =
                                        Cudd_ReadLogicZero(runtime->stats->dd_stats.manager); 
                                    
                                    Cudd_Ref(dinrdy_temp_node);
                                }
                            
                            DdNode * temp_node = NULL;
                            
                            temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager,
                                                   dinrdy_temp_node, runtime->stats->dd_stats.din_vs_ready);
                            Cudd_Ref(temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                dinrdy_temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                runtime->stats->dd_stats.din_vs_ready);
                            
                            //! reset the BDD variables
                            dinrdy_temp_node = temp_node;
                            runtime->stats->dd_stats.din_vs_ready = 
                                Cudd_ReadLogicZero(runtime->stats->dd_stats.manager);
                        }
                    
                    if(config->dd_din_vs_din.doit)
                        {
                            //! initialize this node if needed
                            if(dindin_temp_node == NULL)
                                {
                                    dindin_temp_node =
                                        Cudd_ReadLogicZero(runtime->stats->dd_stats.manager); 
                                    
                                    Cudd_Ref(dindin_temp_node);
                                }
                            
                            DdNode * temp_node = NULL;
                            
                            temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager,
                                                   dindin_temp_node, runtime->stats->dd_stats.din_vs_din);
                            Cudd_Ref(temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                dindin_temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                runtime->stats->dd_stats.din_vs_din);
                            
                            //! reset the BDD variables
                            dindin_temp_node = temp_node;
                            runtime->stats->dd_stats.din_vs_din = 
                                Cudd_ReadLogicZero(runtime->stats->dd_stats.manager);
                        }
                    
                    if(config->dd_din_vs_sin.doit)
                        {
                            //! initialize this node if needed
                            if(dinsin_temp_node == NULL)
                                {
                                    dinsin_temp_node =
                                        Cudd_ReadLogicZero(runtime->stats->dd_stats.manager); 
                                    
                                    Cudd_Ref(dinsin_temp_node);
                                }
                            
                            DdNode * temp_node = NULL;
                            
                            temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager,
                                                   dinsin_temp_node, runtime->stats->dd_stats.din_vs_sin);
                            Cudd_Ref(temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                dinsin_temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                runtime->stats->dd_stats.din_vs_sin);
                            
                            //! reset the BDD variables
                            dinsin_temp_node = temp_node;
                            runtime->stats->dd_stats.din_vs_sin = 
                                Cudd_ReadLogicZero(runtime->stats->dd_stats.manager);
                            dinsin_temp_node = Cudd_bddOr(local_manager, dinsin_temp_node, 
                                                          runtime->stats->dd_stats.din_vs_sin);
                        }
                }
#endif	 
            
            
#if 0
            //! * this section will try out a simple dynamic BDD variable movement *
            
            if(((Cudd_ReadKeys(runtime->stats->dd_stats.manager)*sizeof(DdNode)) >= (MEMORYCHECK))
               && (no_var_swap == 0))          
                {
                    // do manual garbage collection
                    cuddGarbageCollect(runtime->stats->dd_stats.manager, 1);     
                    
                    g_print("Pre-swap memory: %"G_GUINT64_FORMAT"\n", 
                            (Cudd_ReadKeys(runtime->stats->dd_stats.manager)*sizeof(DdNode)));
                    
                    //! do variable reordering
                    no_var_swap = adamant_bdd_var_swap(runtime, MEMORYCHECK);
                    
                    //! \warning For debugging
                    if(no_var_swap == 1)
                        {
                            g_printf("Variable Swap Stopped\n");
                        }
                    
                    //! do manual garbage collection
                    cuddGarbageCollect(runtime->stats->dd_stats.manager, 1);     
                    
                    g_print("Post-swap memory: %"G_GUINT64_FORMAT"\n", 
                            (Cudd_ReadKeys(runtime->stats->dd_stats.manager)*sizeof(DdNode)));
                    
                    MEMORYCHECK = (Cudd_ReadKeys(runtime->stats->dd_stats.manager)*sizeof(DdNode))
                        + MEMORYCHECKDELTA;
                    
                    MEMORYCHECKDELTA += MEMORYCHECKDELTA;
                }
#endif
        }
        
        /*
          if(Cudd_ReadReorderings(runtime->stats->dd_stats.manager) > 2){
          Cudd_AutodynDisable(runtime->stats->dd_stats.manager);
          }
        */
#endif
        
    }
    
    /* End BDD Work */
    
}

#undef DO_SLICE_ANAL
#ifdef DO_SLICE_ANAL
static DdNode *build_xstar(DdManager *manager, guint64 x)
{
    int i, j;
	DdNode * tmp, * itenode, * logicZero;
	
	logicZero = Cudd_ReadLogicZero(manager);
	Cudd_Ref(logicZero);

	//	printf("Adding %llx\n", x);

    /* make a new bdd for this tuple */
	itenode = Cudd_ReadOne(manager);
	Cudd_Ref(itenode);

    /* iterate through the 64 bit values for both X and Y */
    for (i = 0; i < (sizeof(guint64) * 8 * 2); i++) {
	  guint64 bitset;
	  int v;

	  // This code only works for a total of 128 BDD vars, beware
      assert(Cudd_ReadSize(manager) <= BDDNUM);
	  v= Cudd_ReadInvPerm(manager,127-i);

	  tmp = itenode;	  
	  if(v <= 63) {
          bitset = (x & mask((guint64)(v)));
		//printf("Adding variable %d to the tuple with value %d\n",v,(bitset != 0));
		if(bitset) {
		  itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager,v), tmp,
								logicZero);
		  Cudd_Ref(itenode);
		} else {
		  itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager, v),
								logicZero, tmp);
		  Cudd_Ref(itenode);
		}
	  } else {
		itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager, v),
							  tmp,tmp);
	  }
	  Cudd_RecursiveDeref(manager, tmp);
    }

	Cudd_RecursiveDeref(manager, logicZero);

    return itenode;
}

DdNode *bdd_build_tuple(DdManager *manager, guint64 x, guint64 y)
{
    int i, j;
	DdNode * tmp, * retnode, * itenode, * logicZero;
	
	logicZero = Cudd_ReadLogicZero(manager);
	Cudd_Ref(logicZero);

    /* make a new bdd for this tuple */
	itenode = Cudd_ReadOne(manager);
	Cudd_Ref(itenode);

    /* iterate through the 64 bit values for both X and Y */
    for (i = 0; i < (sizeof(guint64) * 8 * 2); i++) {
	  guint64 bitset;
	  int v;

	  // This code only works for a total of 128 BDD vars, beware
      assert(Cudd_ReadSize(manager) <= 128);
	  v= Cudd_ReadInvPerm(manager,127-i);
	  tmp = itenode;	  

	  bitset = (v < 64)?(x & mask((guint64)(v))):(y & mask((guint64)(v-64)));
	  //  printf("Adding variable %d to the tuple with value %d\n",v,bitset);
      if(bitset) {
		itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager,v), tmp,
							  logicZero);
		Cudd_Ref(itenode);
	  } else {
		itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager, v),
							  logicZero, tmp);
		Cudd_Ref(itenode);
	  }
	  
	  Cudd_RecursiveDeref(manager, tmp);
    }

	Cudd_RecursiveDeref(manager, logicZero);

    return itenode;

}

static DdNode *build_d1cube(DdManager *manager) {
  DdNode *ret;
  DdNode *tmp[1];
  FILE *f;

  ret = build_xstar(manager,0xffffffffffffffffLL);

  tmp[0]=ret;
  f=fopen("/u/pricegd/tmpfil.dot","w+");
  Cudd_DumpDot(manager, 1, tmp, NULL, NULL, f);
  fclose(f);

  return ret;
}

static void do_slice_anal(AdamantRuntime *runtime) {
  DdManager *manager;
  DdNode *e,*ep;
  DdNode *s,*stmp;
  DdNode *d1cube;
  DdNode *vector[BDDNUM];
  int count, i;
  struct timeval stime, etime;

  manager = runtime->stats->dd_stats.manager;
  gettimeofday(&stime,NULL);

  for(i=0;i<64;i++) {
	vector[i] = Cudd_bddIthVar(manager,i);
  }
  for(i=64;i<128;i++) {
	vector[i] = Cudd_bddIthVar(manager,i-64);
  }

  count=0;
  d1cube = build_d1cube(manager);
  s = build_xstar(manager, 2000000);
  e = runtime->stats->dd_stats.din_vs_din;
  do {
	ep = Cudd_bddAnd(manager, e, s);
	Cudd_RecursiveDeref(manager,s);

	s = Cudd_bddExistAbstract(manager, ep, d1cube);
	Cudd_RecursiveDeref(manager,ep);

	stmp = s;
	s = Cudd_bddVectorCompose(manager, stmp, vector);
	Cudd_RecursiveDeref(manager,stmp);

	count++;
  } while(count < 100);

  gettimeofday(&etime,NULL);
  g_print("Time for 1000 depth slice is %g\n", 
		 (etime.tv_sec + (etime.tv_usec/1000000.0)) - 
		 (stime.tv_sec + (stime.tv_usec/1000000.0)));
}
#endif


DdNode *adamant_bdd_range(AdamantRuntime * runtime, guint64 high, guint64 low)
{
  int i;
  DdManager *manager;
  DdNode * retDD, * highDD[BDDNUM/2], * lowDD[BDDNUM/2];
  struct timeval stime, etime;

  manager = runtime->stats->dd_stats.manager;

  gettimeofday(&stime,NULL);

  for(i=0; i < 64; i++)
	{
	  highDD[i] = adamant_bdd_make_var(manager, high);
	}

  for(i=0; i < 64; i++)
	{
	  lowDD[i] = adamant_bdd_make_var(manager, low);
	}

  retDD = Cudd_Xgty(manager, BDDNUM, NULL, highDD, lowDD);
  Cudd_Ref(retDD);

  // CLEANUP
  for(i = 0; i < (BDDNUM/2); i++)
	{
	  Cudd_RecursiveDeref(manager, highDD[i]);
	  Cudd_RecursiveDeref(manager, lowDD[i]);
	}

  gettimeofday(&etime,NULL);
  g_print("Time for range search is %g\n", 
		 (etime.tv_sec + (etime.tv_usec/1000000.0)) - 
		 (stime.tv_sec + (stime.tv_usec/1000000.0)));


  return (retDD);
}

DdNode * adamant_bdd_make_var(DdManager *manager, guint64 x)
{
    int i;
	DdNode * tmp, * itenode, * logicZero;
	
	logicZero = Cudd_ReadLogicZero(manager);
	Cudd_Ref(logicZero);

    /* make a new bdd for this tuple */
	itenode = Cudd_ReadOne(manager);
	Cudd_Ref(itenode);

    /* iterate through the 64 bit values for both X and Y */
    for (i = 0; i < (sizeof(guint64) * 8); i++) {
	  guint64 bitset;
	  tmp = itenode;	  

	  // grab the state of this variable bit
	  bitset = (x & mask((guint64)(i)));

	  //  printf("Adding variable %d to the tuple with value %d\n",v,bitset);
      if(bitset) {
		itenode = Cudd_bddIte(manager, itenode, tmp, logicZero);
		Cudd_Ref(itenode);
	  } else {
		itenode = Cudd_bddIte(manager, itenode, logicZero, tmp);
		Cudd_Ref(itenode);
	  }
	  
	  Cudd_RecursiveDeref(manager, tmp);
    }

	Cudd_RecursiveDeref(manager, logicZero);

    Cudd_Ref(tmp);
	Cudd_RecursiveDeref(manager, itenode);

    return tmp;
}


void adamant_bdd_finalize(AdamantRuntime *runtime, guint64 opcount)
{
    AdamantConfig *config = runtime->config;
    AdamantStats *stats = runtime->stats;
    
    if(config->dd_din_vs_ready.doit || 
       config->dd_din_vs_sin.doit ||
       config->dd_din_vs_din.doit) {

        DdNode * temp_node = NULL;		
        
#ifdef DO_SLICE_ANAL
        if(config->dd_din_vs_din.doit) {
            do_slice_anal(runtime);
        }
#endif
        if (THREADED == 1)
            {
                //! merge the BDDs from a multi-threaded run into the main BDD
                merge_threads(runtime);
            }


        cuddGarbageCollect(stats->dd_stats.manager, 1);     	
	   

        //! if these variables are not null, then we have
        //! been using a partitioned BDD system
        if(dinrdy_temp_node != NULL)
            {
                temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager,
                                       dinrdy_temp_node, runtime->stats->dd_stats.din_vs_ready);
                Cudd_Ref(temp_node);
                Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                    dinrdy_temp_node);
                Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                    runtime->stats->dd_stats.din_vs_ready);
                
                //! reset the BDD variables
                runtime->stats->dd_stats.din_vs_ready = temp_node;
            }
        if(dindin_temp_node != NULL)
            {
                temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager,
                                       dindin_temp_node, runtime->stats->dd_stats.din_vs_din);
                Cudd_Ref(temp_node);
                Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                    dindin_temp_node);
                Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                    runtime->stats->dd_stats.din_vs_din);
                
                //! reset the BDD variables
                runtime->stats->dd_stats.din_vs_din = temp_node;

            }
        if(dinsin_temp_node != NULL)
            {
                temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager,
                                       dindin_temp_node, runtime->stats->dd_stats.din_vs_sin);
                Cudd_Ref(temp_node);
                Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                    dinsin_temp_node);
                Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                    runtime->stats->dd_stats.din_vs_sin);
                
                //! reset the BDD variables
                runtime->stats->dd_stats.din_vs_sin = temp_node;
            }
    
#if 0
        FILE * f;
        DdNode *tmp[1];
        
        tmp[0] = runtime->stats->dd_stats.din_vs_din;
        
        f=fopen("/u/pricegd/tmpfil.dot","w+");
        Cudd_DumpDot(stats->dd_stats.manager, 1, tmp, NULL, NULL, f);
        fclose(f);
#endif

        //! perform any last minute BDD operations
        if(config->dd_din_vs_ready.doit)
            {

#ifdef LAYERS
                //! empty out any BDD buffers
                adamantbdd_layer_close(stats->dd_stats.manager,
                                       &runtime->stats->dd_stats.din_vs_ready);
#endif
            }
        if(config->dd_din_vs_sin.doit)
            {
#ifdef LAYERS
                //! empty out any BDD buffers
                adamantbdd_layer_close(stats->dd_stats.manager,
                                       &runtime->stats->dd_stats.din_vs_sin);
#endif
            }
        if(config->dd_din_vs_din.doit)
            {

#ifdef LAYERS
                //! empty out any BDD buffers
                adamantbdd_layer_close(stats->dd_stats.manager,
                                       &runtime->stats->dd_stats.din_vs_din);
#endif
            }       

        //! grab the current time and print
        gettimeofday(&timecount, NULL);
        g_print("DDTIME:%u\n",((unsigned int)timecount.tv_sec - first_time));

        //! Do a final Garbage sweep
        cuddGarbageCollect(stats->dd_stats.manager, 1); 

        // Print out general statistics
        Cudd_PrintInfo(stats->dd_stats.manager, stdout);

        //! if requested, output a dump file of the bdd      
        if(config->dd_din_vs_ready.outfilename != NULL){
            FILE * newfile;
            Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
            Dddmp_MoreDDHeaderInfo headerInfo;

            newfile = fopen(config->dd_din_vs_ready.outfilename, "w+");

            headerInfo.extraTraceInfo = calloc(512,sizeof(char));

            //! add extra information to this BDD dump
            g_snprintf(headerInfo.extraTraceInfo,512,
                       "type:dinrdy,dinstart:0,dinstop:%"G_GUINT64_FORMAT",",
                       opcount);
            
            if(ordr == NULL){
                Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                   runtime->stats->dd_stats.din_vs_ready, NULL, NULL,
                                   DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                   config->dd_din_vs_ready.outfilename, newfile);
            }
            else{
                Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                   runtime->stats->dd_stats.din_vs_ready, NULL, ordr,
                                   DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                   config->dd_din_vs_ready.outfilename, newfile);
                
            }
            fclose(newfile);	
            free(headerInfo.extraTraceInfo);
        }
        if(config->dd_din_vs_sin.outfilename != NULL){
            FILE * newfile;
            Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
            Dddmp_MoreDDHeaderInfo headerInfo;
            
            newfile = fopen(config->dd_din_vs_sin.outfilename, "w+");
            headerInfo.extraTraceInfo = calloc(512,sizeof(char));
            
            //! add extra information to this BDD dump
            g_snprintf(headerInfo.extraTraceInfo,512,
                       "type:dinsin,dinstart:0,dinstop:%"G_GUINT64_FORMAT",",
                       opcount);
            
            if(ordr == NULL){
                
                Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                   runtime->stats->dd_stats.din_vs_sin, NULL, NULL,
                                   DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                   config->dd_din_vs_sin.outfilename, newfile);
            }
            else{
                Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                   runtime->stats->dd_stats.din_vs_sin, NULL, ordr,
                                   DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                   config->dd_din_vs_sin.outfilename, newfile);
                
            }
            
            
            fclose(newfile);	
            free(headerInfo.extraTraceInfo);
        }
        if(config->dd_din_vs_din.outfilename != NULL){
            FILE * newfile;
            Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
            Dddmp_MoreDDHeaderInfo headerInfo;
            
            newfile = fopen(config->dd_din_vs_din.outfilename, "w+");

            headerInfo.extraTraceInfo = calloc(512,sizeof(char));

            //! add extra information to this BDD dump
            g_snprintf(headerInfo.extraTraceInfo,512,
                       "type:dindin,dinstart:0,dinstop:%"G_GUINT64_FORMAT",",
                       opcount);
            
            if(ordr == NULL){
                
                Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                   runtime->stats->dd_stats.din_vs_din, NULL, NULL,
                                   DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                   config->dd_din_vs_din.outfilename, newfile);
            } 
            else {
                Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                   runtime->stats->dd_stats.din_vs_din, NULL, ordr,
                                   DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                   config->dd_din_vs_din.outfilename, newfile);
            }
            
            fclose(newfile);
            free(headerInfo.extraTraceInfo);            
        }		
    }
}


/*!
  This function will output tick information related to BDD creation and, if requested,
  it will save off the current BDD
  
  \date 09/28/07
  \author Graham Price
*/
void adamant_bdd_tickoutput(AdamantConfig *config, AdamantRuntime *runtime,
			       guint64 opcount) 
{
    if(config->dd_tick_interval != 0)
        {

            DdNode * tempBDD = NULL;
            FILE * newfile = NULL;
            Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP)*/
            Dddmp_MoreDDHeaderInfo headerInfo;
            
            
            headerInfo.extraTraceInfo = calloc(512, sizeof(char));
            
            
            if (THREADED == 1)
                {
                    //! merge the BDDs from a multi-threaded run into the main BDD
                    merge_threads(runtime);
                }
            
            //! get the information needed to save off the tick BDD
            if( config->dd_din_vs_ready.doit )
                {
                    
                    newfile = fopen(config->dd_din_vs_ready.outfilename, "w+");
                    
                    //! add extra information to this BDD dump
                    g_snprintf(headerInfo.extraTraceInfo,512,
                               "type:dinrdy,dinstart:0,dinstop:%"G_GUINT64_FORMAT",",
                               opcount);
                    
                    //! set up this BDD node to be written out
                    tempBDD = runtime->stats->dd_stats.din_vs_ready;
                }
            if( config->dd_din_vs_sin.doit )
                {
                    newfile = fopen(config->dd_din_vs_sin.outfilename, "w+");
                    g_snprintf(headerInfo.extraTraceInfo,512,
                               "type:dinsin,dinstart:0,dinstop:%"G_GUINT64_FORMAT",",
                               opcount);
                    
                    tempBDD = runtime->stats->dd_stats.din_vs_sin;
                }
            if( config->dd_din_vs_din.doit )
                {
                    newfile = fopen(config->dd_din_vs_din.outfilename, "w+");
                    g_snprintf(headerInfo.extraTraceInfo,512,
                               "type:dindin,dinstart:0,dinstop:%"G_GUINT64_FORMAT",",
                               opcount);
                    
                    tempBDD = runtime->stats->dd_stats.din_vs_din;
                }
            
            //! now save off the tick bdd
            if (newfile != NULL)
                {
                    if(ordr == NULL)
                        {
                            assert(tempBDD != NULL);
                            
                            Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                               tempBDD, NULL, NULL,
                                               DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                               config->dd_output_file, newfile);
                        } 
                    else 
                        {
                            assert(tempBDD != NULL);
                            
                            Dddmp_cuddBddStore(runtime->stats->dd_stats.manager, NULL,
                                               tempBDD, NULL, ordr,
                                               DDDMP_MODE_BINARY, extrainfo, &headerInfo,
                                               config->dd_output_file, newfile);
                        }
                    
                    fclose(newfile);
                }	

            //! CLEANUP
            free(headerInfo.extraTraceInfo);
        }
}

void adamant_bdd_tick(AdamantConfig *config, AdamantRuntime *runtime,
                      guint64 opcount) 
{
    if(config->dd_din_vs_ready.doit ||
       config->dd_din_vs_sin.doit ||
       config->dd_din_vs_din.doit ||
       FALSE) { // Add other bdd based stats checks here

        DdManager * manager = runtime->stats->dd_stats.manager;

        //! ** print out information just for the opcount,tuple,memory usage info **

        //! grab the current time
        gettimeofday(&timecount, NULL); 

        //! if configured to do so, print out the variable motion
        if(config->dd_print_varmotion == TRUE)
            {
                if(runtime->stats->dd_stats.din_vs_ready != NULL)
                    {                        
                        print_variable_motion(runtime->stats->dd_stats.din_vs_ready,
                                              manager,
                                              g_var_order);
                    }
                if(runtime->stats->dd_stats.din_vs_din != NULL)
                    {                        
                        print_variable_motion(runtime->stats->dd_stats.din_vs_din,
                                              manager,
                                              g_var_order);
                    }
                if(runtime->stats->dd_stats.din_vs_sin != NULL)
                    {                        
                        print_variable_motion(runtime->stats->dd_stats.din_vs_sin,
                                              manager,
                                              g_var_order);
                    }
            }
        
#if 0
        if(first_tick == 1)
            {
                //! set up the header info and the first time
                first_tick = 0;

                g_print("*Tick Output*\n");
                g_print("*opcount, GC Time, GC Count,Cache Collisions, memory, time*\n");
            }
        g_print("%"G_GUINT64_FORMAT",%ld,%d,%.0f,%lu,%u\n",
                opcount,
                Cudd_ReadGarbageCollectionTime(manager),
                Cudd_ReadGarbageCollections(manager),
                (manager->cachecollisions),
                ((manager->keys - manager->dead) * sizeof(DdNode)),
                ((unsigned int)timecount.tv_sec - first_time));
#endif

        adamant_bdd_subTableSizeOut(manager, opcount);        
    }
}

/*
  This Function adds a tuple with 128 total bits
  to a BDD
*/
DdNode *adamant_bdd_add_tuple(DdManager *manager, DdNode *set, guint64 x, guint64 y)
{
    int i, j, comple;
	unsigned int topf, topg, toph, v;
	DdNode * tmp, * itenode, * logicZero;
   
	logicZero = Cudd_ReadLogicZero(manager);
	
	Cudd_Ref(logicZero);
    
	/* make a new bdd for this tuple */
	itenode = Cudd_ReadOne(manager);
	Cudd_Ref(itenode);

    /* iterate through the 64 bit values for both X and Y */
    for (i = 0; i < (sizeof(guint64) * 8 * 2); i++) {
        guint64 bitset;
        int v;

        // This code only works for a total of 128 BDD vars, beware
        assert(Cudd_ReadSize(manager) <= 128);
        v = Cudd_ReadInvPerm(manager,127-i);
        tmp = itenode;	
	  
        bitset = (v < 64)?(x & mask((guint64)(v))):(y & mask((guint64)(v-64)));
        //  printf("Adding variable %d to the tuple with value %d\n",v,bitset);
        if(bitset) {

            if((bdd_reordering == TRUE))
                {
                    itenode = adamant_tight_BddIte(manager, Cudd_bddIthVar(manager,v), tmp, logicZero);
                }
            else
                {
                    itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager,v), tmp,
                                          logicZero);
                }
            if(itenode == NULL)
                {


                    return (NULL);

                }

            Cudd_Ref(itenode);

        } 
        else {

            if((bdd_reordering == TRUE))
                {
                    itenode = adamant_tight_BddIte(manager, Cudd_bddIthVar(manager, v), logicZero, tmp);
                }
            else 
                {
                  
                    itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager, v),
                                          logicZero, tmp);
                }

            if(itenode == NULL)
                {
                    g_print("**** ITE failed to add a tuple ****\n");

                    return (NULL);

                }          

            Cudd_Ref(itenode);
        }
	  
	  
        Cudd_RecursiveDeref(manager, tmp);
	}

	Cudd_RecursiveDeref(manager, logicZero);

    tmp = Cudd_bddOr(manager, itenode, set);
	
	Cudd_Ref(tmp);
	
	//cuddSatInc(Cudd_Regular(tmp)->ref);

	
	// this should be on, but for some reason the cuddUniqueInter function toys with the ref counts
	// and if this is on it will blow up during garbage collection !!!
	Cudd_RecursiveDeref(manager, itenode);
	

#if 0  // Turn on for check versus super-slow version
    { DdNode *tmp2;
        tmp2=adamant_bdd_add_tuple_serial(manager,set,x,y);
        if(tmp2 != tmp) {
            DdNode *arr[2] = {tmp, tmp2};
            char *names[2] = {"tmp","tmp2"};
            fprintf(stdout,"Uh-oh, wrong code, tmp=%p, tmp2=%p\n",tmp,tmp2);
            Cudd_DumpDot(manager,2,arr,NULL,names,stderr);
            exit(-1);
        }
	}
#endif 

    return tmp;
}


DdNode *adamant_bdd_add_tuple_serial(DdManager *manager, DdNode *set, guint64 x, guint64 y)
{
    int i = 0;
    DdNode *var, *tmp, *n_tuple;

    /* make a new bdd for this tuple */
    n_tuple = Cudd_ReadOne(manager);
    Cudd_Ref(n_tuple);

    /* iterate through the 64 bit values for both X and Y */
    for (i = 0; i < sizeof(unsigned long long) * 8 * 2; i++) {
	  tmp = NULL;
	  
	  /* grab the BDD variable */
	  var = Cudd_bddIthVar(manager, i);
	  
	  /* current encoding system puts 0 -> 63 as X, 64 -> 127 as Y */
	  if (i < 64) {
	    if ((x >> i) & 1) {
		  tmp = Cudd_bddAnd(manager, var, n_tuple);
	    } else {
		  tmp = Cudd_bddAnd(manager, Cudd_Not(var), n_tuple);
	    }
	  }
	  
	  else {
	    if ((y >> (i - 64)) & 1) {
		  tmp = Cudd_bddAnd(manager, var, n_tuple);
	    } else {
		  tmp = Cudd_bddAnd(manager, Cudd_Not(var), n_tuple);
	    }
	  }
	  
	  Cudd_Ref(tmp);
	  Cudd_RecursiveDeref(manager, n_tuple);
	  n_tuple = tmp;
    }

    /* Or in the new tuple */
    tmp = Cudd_bddOr(manager, n_tuple, set);
	
    /* prevent garbage collection on the new BDD */
    Cudd_Ref(tmp);
	
    /* tell the garbage collector to collect the old BDDs */
    Cudd_RecursiveDeref(manager, n_tuple);
	
    return tmp;
}


/*! 
  This function will move the split point of the variable order 

  \author Graham Price
 */
int adamant_bdd_var_swap(AdamantRuntime *runtime, unsigned long maxMem)
{

    int i = 0, k = 0, return_num = 0;
  clock_t startclock, endclock;
  double clocktime = 0.0;
  int result = 0;
  int num_swaps = 2;

  AdamantConfig *config = runtime->config;
  AdamantStats *stats = runtime->stats;

  startclock = clock();
    
  g_print("Beginning swap\n");
  /*
  result = mg_Cudd_ShuffleHeap(stats->dd_stats.manager,
                               split_point, num_swaps);
  */
  result = mg_Cudd_ShuffleVars(stats->dd_stats.manager, num_swaps);
  g_print("\nSwapped:%d\n", result);
  
  split_point += num_swaps;
  
  endclock = clock();
  
  clocktime =  (double) ((endclock - startclock)/CLOCKS_PER_SEC);
  
  printf("Shuffled the Heap. %f time spent shuffling\n",
		 clocktime);         
  
  //! if the shuffle time is growing super fast, kill the job 
  //! gracefully
  if(oldshuffletime == 0) //! comparing a float...not so good
      {
          oldshuffletime = clocktime;
      }
  else
      {
          //! this cutoff is arbitrary
          if(clocktime > (oldshuffletime * 3))
              {
                  return_num = 1;//! report that the shuffle is taking too much time
              }   
          else
              {
                  oldshuffletime = clocktime;
              }
      }
  
  
  /* grab the current order of the variables */
  for(i = 0; i < 128; i++)
	{
	  printf("position %d has variable %d\n", i, 
             Cudd_ReadInvPerm(stats->dd_stats.manager, i));
	}
  
  return (return_num);
}

int DynReorderHook(DdManager * dd, const char * str, void * data){
  
  printf("post reorder\n");
  
  return 1;
}


/**Function********************************************************************

  Synopsis    [Implements a smaller version of Cudd_bddIte.]

  Description [Returns a
  pointer to the resulting BDD. NULL if the intermediate result blows
  up or if reordering occurs.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
inline DdNode * adamant_tight_BddIte(
							  DdManager * dd,
							  DdNode * f,
							  DdNode * g,
							  DdNode * h)
{
  DdNode	 *one, *zero, *res;
  DdNode	 *r, *Fv, *Fnv, *Gv, *Gnv, *H, *Hv, *Hnv, *t, *e;
  //  unsigned int topf, topg, toph, v;
  int		 index;
  int		 comple;
  
  statLine(dd);
  
  comple = adamant_Canonical(dd, &f, &g, &h);
  
  r = cuddUniqueInter(dd, (int) f->index, g, h);

  return(Cudd_NotCond(r,comple && r != NULL));

} /* end of adamant_tight_BddIte */
 

/* This was pulled straight out of the CUDD code */
int adamant_Canonical( DdManager * dd, DdNode ** fp, DdNode ** gp, DdNode ** hp )
{
    register DdNode		*r, *f, *g, *h;
    int				comple, change;

	change = 0;
    comple = 0;

	f = *fp;
	g = *gp;
	h = *hp;

    if (Cudd_IsComplement(g)) {	/* ITE(F,!G,H) = !ITE(F,G,!H) */
	  g = Cudd_Not(g);
	  h = Cudd_Not(h);
	  change = 1;
	  comple = 1;
    }

    if (change) {
	  *fp = f;
	  *gp = g;
	  *hp = h;
    }

    return(comple);

} /* end of adamant_Canonical */



/*!

  This function is should cleanup BDD work
  such as managers, nodes, etc

  \author Graham Price
  \date 11/14/07

 */
void adamant_bdd_quit(AdamantStats *stats)
{
    int count = 0;

	if(stats->dd_stats.manager) {
		if(stats->dd_stats.din_vs_ready) {
			Cudd_RecursiveDeref(stats->dd_stats.manager, stats->dd_stats.din_vs_ready);
		}
		if(stats->dd_stats.din_vs_sin) {
			Cudd_RecursiveDeref(stats->dd_stats.manager, stats->dd_stats.din_vs_sin);
		}
		if(stats->dd_stats.din_vs_din) {
			Cudd_RecursiveDeref(stats->dd_stats.manager, stats->dd_stats.din_vs_din);
		}
		
        if((count=Cudd_CheckZeroRef(stats->dd_stats.manager))) {
		  fprintf(stderr,"BDD Manager returned final count = %d\n",count);
		}
		Cudd_Quit(stats->dd_stats.manager);

        if(THREADED == 1)
            {
                int k = 0;
                for (k = 0; k < THREADCNT; k++)
                    {
                
                        Cudd_Quit(threadArgs[k].threadManager);
                        
                        //! kill the thread
                        pthread_exit(&bddThreads[k]);
                        
                        //! free arguments
                        free(threadArgs[k].x);
                        threadArgs[k].x = NULL;
                        free(threadArgs[k].y);
                        threadArgs[k].y = NULL;
                        free(threadArgs[k].stayalive);
                        threadArgs[k].stayalive = NULL;
                        
                        //! destroy the mutexes...muticies..
                        pthread_mutex_destroy(threadArgs[k].holdMutex);
                        pthread_mutex_destroy(threadArgs[k].threadMutex);
                    }
                free(threadArgs);
                free(bddThreads);
            }
	}
}

/*!
  Multi threading work?
  \author Graham Price
 */
void * adamant_bdd_thread(void * temp_args)
{
    int valid = 0;

    thread_args * local_args = (thread_args*)(temp_args);

    if((local_args->stayalive == NULL)||
       (local_args->x == NULL) ||
       (local_args->y == NULL))
        {
            valid = 0;            
        }
    else
        {
            valid = 1;
        }

    while((valid == 1) && (*(local_args->stayalive) == 1))
        {
            //! wait until the hold is released
            pthread_mutex_lock(local_args->holdMutex);

            if((*(local_args->x) != 0) || (*(local_args->y) != 0))
                {
                    //! add the new tuple to the BDD
                    DdNode *tmp;
                    tmp=adamant_bdd_add_tuple(local_args->threadManager, 
                                              local_args->threadNode, 
                                              *(local_args->x), *(local_args->y));
                    if(tmp == NULL)
                        {
                            //! if this happens, the program is exploding
                            return(0);                            
                        }

                    Cudd_RecursiveDeref(local_args->threadManager, local_args->threadNode);
                    local_args->threadNode = tmp;
                    
                    
                    //! reset the tuple members to be safe
                    *(local_args->x) = 0;
                    *(local_args->y) = 0;
                }
            
            //! unlock the thread
            pthread_mutex_unlock(local_args->threadMutex);

            //! make sure these are still valid
            if((local_args->stayalive == NULL)||
               (local_args->x == NULL) ||
               (local_args->y == NULL))
                {
                    valid = 0;            
                }
        }

    return (0);
}

/*!
  This function looks for any difference in variable order
  from a previous iteration and prints out the variable order
  information
  
  \author Graham Price
  \date 11/23/07
*/
int print_variable_motion(DdNode * node, DdManager * manager, 
                          int * previous_order)
{
    int i = 0, changed = 0;
    int * current_order = calloc(BDDNUM, sizeof(int));

    if(previous_order == NULL)
        {

            //! grab the current variable order
            for(i = 0; i < BDDNUM; i++)
                {
                    current_order[i] = Cudd_ReadInvPerm(manager,i);
                }

            //! flop the array pointers
            previous_order = current_order;
            
            //! let everyone know the order is new
            changed = 1;
        }
    else
        {
            for (i = 0; ((i < BDDNUM)&&(changed = 0)); i++)
                {
                    current_order[i] = Cudd_ReadInvPerm(manager,i);

                    if(current_order[i] != previous_order[i])
                        {
                            //! kill the old order pointer
                            free (previous_order);

                            //! flop the array pointers
                            previous_order = current_order;
                            
                            //! let everyone know the order is new
                            changed = 1;
                        }                       
                }
        }

    if(changed == 1)
        {

            g_print("\n New Variable Order (permutation,index)\n");            

            for(i = 0; i < BDDNUM; i++)
                {
                    g_print("(%d,%d)\n", i, current_order[i]);
                }
        }
    return (0);
}


/*!
  This function will merge the BDDs from a multithreaded
  execution into the main BDD

  \author Graham Price
  \date 11/27/2007
 */
int merge_threads(AdamantRuntime *runtime)
{
    AdamantConfig *config = runtime->config;
    AdamantStats *stats = runtime->stats;

    if (THREADED == 1)
        {

            DdNode * temp_node = NULL;
            
            int k = 0;
            for (k = 0; k < THREADCNT; k++)
                {
                    //! garbage collect for this manager
                    cuddGarbageCollect(threadArgs[k].threadManager, 1);     	
                    
                    //! print out general statistics
                    //                    Cudd_PrintInfo(threadArgs[k].threadManager, stdout);
                    
                    //! move this BDD into the main manager
                    Cudd_bddTransfer(threadArgs[k].threadManager,
                                     runtime->stats->dd_stats.manager,
                                     threadArgs[k].threadNode);
                    
                    //! merge the BDD into the main BDD
                    if( config->dd_din_vs_ready.doit ) 
                        {
                            temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager, threadArgs[k].threadNode, runtime->stats->dd_stats.din_vs_ready);
                            Cudd_Ref(temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                threadArgs[k].threadNode);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                runtime->stats->dd_stats.din_vs_ready);
                            
                            runtime->stats->dd_stats.din_vs_ready = temp_node;
                        }
                    
                    if( config->dd_din_vs_sin.doit )
                        {
                            temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager, threadArgs[k].threadNode, runtime->stats->dd_stats.din_vs_sin);
                            Cudd_Ref(temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                threadArgs[k].threadNode);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                runtime->stats->dd_stats.din_vs_sin);
                            
                            runtime->stats->dd_stats.din_vs_sin = temp_node;
                        }
                    if( config->dd_din_vs_din.doit)
                        {
                            temp_node = Cudd_bddOr(runtime->stats->dd_stats.manager, threadArgs[k].threadNode, runtime->stats->dd_stats.din_vs_din);
                            Cudd_Ref(temp_node);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                threadArgs[k].threadNode);
                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                runtime->stats->dd_stats.din_vs_din);
                            
                            runtime->stats->dd_stats.din_vs_din = temp_node;
                            
                        }
                }
        }

    return (0);
}


//! this will initialize the BDD node and manager
//! given header information
//! this does destroy the passed char string
int adamant_bdd_headerinit(FILE * bddfile, AdamantRuntime *runtime )
{
    int return_val = 0;
    guint64 temp_dinstart = 0;
    char * headerInfo = NULL;
    DdNode * tempnode = NULL;
    char * fileinfo = calloc(512, sizeof(char));


    tempnode = Dddmp_cuddBddLoad(runtime->stats->dd_stats.manager, 
                                 DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL, 
                                 DDDMP_MODE_BINARY, &headerInfo, 
                                 fileinfo, bddfile);

    if((tempnode != NULL) && (headerInfo != NULL))
        {
            char* token = strtok(headerInfo, ":");
            while(token != NULL)
                {    
                    if(matchkeywd(token,"type"))
                        {
                            //! save the type of BDD this is
                            token = strtok(NULL,",");   
                    
                            if(matchkeywd(token,"dindin"))
                                {
                                    //! replace any existing BDDs of this type
                                    if(runtime->stats->dd_stats.din_vs_din != NULL)
                                        {
                                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                                runtime->stats->dd_stats.din_vs_din);
                                        }
                            
                                    runtime->stats->dd_stats.din_vs_din = tempnode;
                                }
                            else if(matchkeywd(token,"dinrdy"))
                                {
                                    //! replace any existing BDDs of this type
                                    if(runtime->stats->dd_stats.din_vs_ready != NULL)
                                        {
                                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                                runtime->stats->dd_stats.din_vs_ready);
                                        }
                            
                                    runtime->stats->dd_stats.din_vs_ready = tempnode;
                                }
                            else if(matchkeywd(token,"dinsin"))
                                {
                                    //! replace any existing BDDs of this type
                                    if(runtime->stats->dd_stats.din_vs_sin != NULL)
                                        {
                                            Cudd_RecursiveDeref(runtime->stats->dd_stats.manager,
                                                                runtime->stats->dd_stats.din_vs_sin);
                                        }
                            
                                    runtime->stats->dd_stats.din_vs_sin = tempnode;
                                }

                        }                                    
                                        
                    else if(matchkeywd(token,"dinstop"))
                        {
                            //! glue new instructions to the start
                            //! of this BDD
                            token = strtok(NULL,","); 
  
                            temp_dinstart = g_ascii_strtoull(token, NULL, 10);
                    
                            //! check to make sure we default start as early as needed
                            if((runtime->config->g_dd_start_din == 0) ||
                               (temp_dinstart < runtime->config->g_dd_start_din))
                                {
                                    runtime->config->g_dd_start_din = g_ascii_strtoull(token, NULL, 10);
                                }
                        } 
                    else 
                        {
                            //! this should keep things clean
                            token = strtok(NULL,",");   
                        }
                    token = strtok(NULL, ":");
                }

            //! CLEANUP
            free(headerInfo);

            return_val = 0;
        }
    else
        {
            return_val = 1;
        }

    //! CLEANUP
    free (fileinfo);

    return(return_val);

}


/*
  Function: adamant_bdd_manager
  
  This function generates a new BDD manager
  with a given variable order.

  Called with: void
  
  Returns: DdManager - the new BDD manager
  
  Side Effects: none

 */
DdManager * adamant_bdd_manager(void)
{
    DdManager * temp_manager = NULL;

    if(global_config != NULL)
        {
            
            //! initialize the CUDD manager
            temp_manager = Cudd_Init(BDDNUM, 0, CUDD_UNIQUE_SLOTS, 
                                     global_config->dd_mem_init, global_config->dd_mem_cap);
  
            //! read in the variable order from a file
            //! specified on the command line
            if (global_config->dd_order != NULL)
                {
                    FILE * forder = NULL;
                    int tpos = 0, tvar = 0;
                    forder = fopen(global_config->dd_order,"r+");
                    if(forder != NULL)
                        {
                            while((!feof(forder)) && (tpos < BDDNUM))
                                {
                                    fscanf(forder, "%d", &ordr[tpos]);
                                    tpos++;
                                }
                            fclose(forder);
                        }
                    else
                        {
                            g_print("Error opening BDD order file\n");
                        }
                }
            else                
                {
                    int k = 0;

                    // create a default MSB to LSB order
                    for(k = 0; k < 64; k++)
                        {
                            ordr[127-2*k] = k;
                            ordr[127-(2*k+1)] = (64+k);
                        }
                }

            //! perform an initial shuffle (and set) of the new BDD
            //! variable order
            Cudd_ShuffleHeap(temp_manager, ordr);

            //! make sure automatic dynamic variable reordering
            //! is turned off for this manager
            Cudd_AutodynDisable(temp_manager);
        }

    return(temp_manager);
}


/*
  Special output for size data
*/
void adamant_bdd_sizeout(DdManager * manager, guint64 opcount)
{
    //! grab the current time
    gettimeofday(&timecount, NULL); 
 
    if(first_tick == 1)
        {
            //! set up the header info and the first time
            first_tick = 0;
            g_print("*Tick Output*\n");
            g_print("*Opcount, Slots, Keys, Dead, Time*\n");
        }
    g_print("%"G_GUINT64_FORMAT",%d,%d,%d,%u\n",
            opcount,
            manager->slots,
            manager->keys,
            manager->dead,
            ((unsigned int)timecount.tv_sec - first_time));
}

/*
  Special output for subtable size data
*/
void adamant_bdd_subTableSizeOut(DdManager * manager, guint64 opcount)
{
    //! grab the current time
    gettimeofday(&timecount, NULL); 
 
    //! clean up some garbage
    cuddGarbageCollect(manager, 1);

    if(first_tick == 1)
        {
            //! set up the header info and the first time
            first_tick = 0;
            g_print("*Tick Output*\n");
            g_print("*Opcount, Slots, Keys, Dead, Time*\n");
        }
    int i = 0;
    
    g_print("Opcount:%"G_GUINT64_FORMAT",%d,%d,%d,%u\n",
            opcount,
            manager->slots,
            manager->keys,
            manager->dead,
            ((unsigned int)timecount.tv_sec - first_time));

    for(i = 0; i < manager->size; i++)
        {
            g_print("%d", manager->subtables[i].keys);

            if((i+1) < manager->size)
                {
                    g_print(",");
                }
        }
    g_print("\n");
}

