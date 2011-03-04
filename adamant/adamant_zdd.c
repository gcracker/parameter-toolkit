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


#include "adamant_zdd.h"
#include "adamant_zdd_test.h"
#include "adamant_dd_hot.h"
#include "adamant.h"
#include "adamant_branch_prediction.h"
#include "adamant_plot.h"
#include "adamant_window.h"

#include <stdio.h>
#include <sys/resource.h>
#include <gmp.h>
#include <sys/time.h>
#include <time.h>
#include <cudd.h>
#include <cuddInt.h>

#define TESTZDDSTORE
#define BUILDZDD
#define matchkeywd(str,key) (strncmp(str,key,strlen(key))==0)
#define DELIMITERS    " \r\n\t!@#$%^&*()_+-={}|\\:\"'?¿/.,<>’¡º×÷‘"

//#define ZDDNUM 128
#define THREADCNT 2

/*! External Functions */

/*! Local Global Variables */
int first_zdd_tick = 1;
double oldzddshuffletime = 0;
int zdd_split_point = 0;
DdNode * local_temp_zdd_node = NULL;
DdNode * dinrdy_temp_zdd_node = NULL;
DdNode * dinsin_temp_zdd_node = NULL;
DdNode * dindin_temp_zdd_node = NULL;
guint64  ZDDMEMORYCHECK = (1073741824/10);
unsigned long ZDDMEMORYCHECKDELAY = 100000;
unsigned long ZDDMEMORYCHECKDELTA = (1073741824/100);

unsigned long zdd_delaycount = 0;
DdManager * zdd_local_manager;
guint64 g_dinstart = 0;

// !! DEBUG !!
unsigned long zdd_debug_count = 0;
guint64 zdd_dead_count = 0;
// !!!!!

unsigned int zdd_first_time = 0;
int * zdd_ordr = NULL;
int * g_zdd_var_order;
AdamantConfig * zdd_global_config = NULL;
adamantHotManager * hotManager = NULL;


/* these variables are used for some fairly dumb
   counting and should be removed soon
*/
struct timeval timecount;
int no_zdd_var_swap = 0;

//! \warning TEST VARIABLES
thread_args * threadArgs;

guint64 adamant_zdd_freemem(void)
{
  char *cmd = "echo $($_CMD free -b | grep Mem: | awk '{ print $4 }')";
  FILE *cmdfile = popen(cmd,"r");
  char * tresult = g_new0(char, 256);
  guint64 iResult = 0;

  if((NULL != cmdfile) && 
     (NULL != fgets(tresult, 256, cmdfile)))
    {
      gchar * result = g_strchomp (tresult);
      
      iResult = g_ascii_strtoull(result, NULL, 10);
    }
  pclose(cmdfile);
  g_free(tresult);

  g_print("Detected Free Memory: %"G_GUINT64_FORMAT"\n", iResult);
  return (iResult);
}


void adamant_initialize_zddstats(AdamantRuntime *runtime)
{
  AdamantConfig *config = runtime->config;
  int u = 0;

  //! set up a global config file
  zdd_global_config = runtime->config;


#ifdef ZDDGRP
  //!! DEBUG !!
  testDINS_hash = NULL;
#endif


  if (config->dd_make.doit == TRUE)
    {

      /* initialize the CUDD ZDD manager */
      int k = 0;
      zdd_ordr = calloc(ZDDNUM, sizeof(int)); /* variable never freed, but should not leak */
      guint64 freeMem = adamant_zdd_freemem();
      //! initialize the CUDD ZDD managers
      runtime->stats->dd_stats.manager = Cudd_Init(ZDDNUM, ZDDNUM,
      						    (config->dd_slots_multi * CUDD_UNIQUE_SLOTS),
      						    CUDD_CACHE_SLOTS, freeMem);
      zdd_local_manager = Cudd_Init(ZDDNUM, ZDDNUM, (config->dd_slots_multi * CUDD_UNIQUE_SLOTS),
      				    CUDD_CACHE_SLOTS, freeMem);
      /* runtime->stats->dd_stats.manager = Cudd_Init(ZDDNUM, ZDDNUM, */
      /* 						    (config->dd_slots_multi * CUDD_UNIQUE_SLOTS), */
      /* 						    config->dd_mem_init, config->dd_mem_cap); */
      /* zdd_local_manager = Cudd_Init(ZDDNUM, ZDDNUM, (config->dd_slots_multi * CUDD_UNIQUE_SLOTS), */
      /* 				    config->dd_mem_init, config->dd_mem_cap); */

      //      CUDD_CACHE_SLOTS

      // Read and set the LooseUp paramemter of the manager
      guint32 tempReadLoose = Cudd_ReadLooseUpTo(runtime->stats->dd_stats.manager);
      Cudd_SetLooseUpTo(runtime->stats->dd_stats.manager,
			(tempReadLoose * config->dd_looseup_multi));

      tempReadLoose = Cudd_ReadLooseUpTo(zdd_local_manager);

      Cudd_SetLooseUpTo(zdd_local_manager,
			(tempReadLoose * config->dd_looseup_multi));

      if (config->dd_order != NULL)
	{
	  FILE * forder = NULL;
	  int tpos = 0, tvar = 0;
	  forder = fopen(config->dd_order,"r+");
	  if(forder != NULL)
	    {
	      while((!feof(forder)) && (tpos < ZDDNUM))
		{
		  fscanf(forder, "%d", &zdd_ordr[tpos]);
		  tpos++;
		}
	      fclose(forder);
	    }
	  else
	    {
	      g_print("Error opening ZDD order file\n");
	    }
	}
      else
	{

	  //! if there is no variable order file
	  //! default to the interlaced MSB to LSB
	  //! so that it is ParaMeter compatible
	  for(k = 0; k < 64; k++)
	    {
	      zdd_ordr[127-2*k] = k;
	      zdd_ordr[127-(2*k+1)] = (64+k);
	    }
	}

      Cudd_zddShuffleHeap(runtime->stats->dd_stats.manager, zdd_ordr);
      Cudd_ShuffleHeap(runtime->stats->dd_stats.manager, zdd_ordr);

      //! turn off automatic garbage collection
      if(config->dd_garbage_collect != 0)
	{
	  Cudd_DisableGarbageCollection(runtime->stats->dd_stats.manager);
	}

      //! initialize ZDDs and support information
      runtime->stats->dd_stats.din_vs_ready =
	runtime->stats->dd_stats.din_vs_sin =
	runtime->stats->dd_stats.din_vs_din = NULL;

      if (config->dd_din_vs_ready.doit)
	{
	  runtime->stats->dd_stats.din_vs_ready = Cudd_ReadZero(runtime->stats->dd_stats.manager);
	  Cudd_Ref(runtime->stats->dd_stats.din_vs_ready);
	}
      if (config->dd_din_vs_sin.doit)
	{
	  runtime->stats->dd_stats.din_vs_sin = Cudd_ReadZero(runtime->stats->dd_stats.manager);
	  Cudd_Ref(runtime->stats->dd_stats.din_vs_sin);
	}
      if (config->dd_din_vs_din.doit)
	{
	  runtime->stats->dd_stats.din_vs_din = Cudd_ReadZero(runtime->stats->dd_stats.manager);
	  Cudd_Ref(runtime->stats->dd_stats.din_vs_din);
	}
      if (config->dd_din_vs_sys.doit)
	{
	  runtime->stats->dd_stats.din_vs_sys = Cudd_ReadZero(runtime->stats->dd_stats.manager);
	  Cudd_Ref(runtime->stats->dd_stats.din_vs_sys);
	}
      if (config->dd_din_vs_hot.doit)
	{
	  hotManager = adamant_hot_init();
	  adamant_hot_bufferInit(hotManager);
	  runtime->stats->dd_stats.din_vs_hot = Cudd_ReadZero(runtime->stats->dd_stats.manager);
	  Cudd_Ref(runtime->stats->dd_stats.din_vs_hot);
	}

      //! read in information from a DD restart file
      //! the "restart file" is just a prior dd dump
      if(config->dd_restart.outfilename != NULL)
	{
	  FILE * newfile;

	  newfile = fopen(config->dd_restart.outfilename, "r");

	  if(newfile != NULL)
	    {
	      //! check the headerinfo for information
	      adamant_zdd_headerinit(newfile, runtime );
	      fclose(newfile);
	    }
	}

      //! set a manual garbage collection size based on the set memory cap
      if(config->dd_mem_cap != 0)
	{
	  ZDDMEMORYCHECK = config->dd_mem_cap;
	}

      //! grab the time when we startup
      gettimeofday(&timecount, NULL);
      zdd_first_time = (unsigned int)(timecount.tv_sec);
    }
}


void
adamant_zdd_ready_time(AdamantRuntime *runtime,
                       TmtOper *oper,
                       guint64 ready,
                       guint64 din,
                       guint thread_id,
                       guint num_din_srcs)
{
  AdamantConfig *config = runtime->config;
  AdamantStats *stats = runtime->stats;

  if(config->dd_din_vs_ready.doit)
    {
      if(din >= config->g_dd_start_din)
	{

	  DdNode * tmp = adamant_zdd_build_tuple(runtime->stats->dd_stats.manager,
						 runtime->stats->dd_stats.din_vs_ready,
						 din, ready);

	  Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
				 runtime->stats->dd_stats.din_vs_ready);

	  if(tmp == NULL)
	    {
	      //! if this happens, the program is exploding

	      /* Kill Adamantium Gracefully */
	      runtime->done = 1;

	      config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit =
		config->dd_din_vs_din.doit = config->dd_din_vs_sys.doit= 0;

	      g_print("ZDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);

	      /* print out general statistics */
	      Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);

	      return;
	    }

	  runtime->stats->dd_stats.din_vs_ready = tmp;
	}
    }
  if(config->dd_din_vs_sys.doit)
    {
      if((din >= config->g_dd_start_din) && ( true == oper->is_syscall))
	{
	  DdNode * tmp = adamant_zdd_build_tuple(runtime->stats->dd_stats.manager,
						 runtime->stats->dd_stats.din_vs_sys,
						 din, oper->sysno);

	  Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
				 runtime->stats->dd_stats.din_vs_sys);

	  // START DEBUG
	  //	  g_print("DIN: %"G_GUINT64_FORMAT" SYS: %"G_GUINT64_FORMAT"\n", 
	  //		  din, oper->sysno);
	  // END DEBUG

	  if(tmp == NULL)
	    {
	      //! if this happens, the program is exploding

	      /* Kill Adamantium Gracefully */
	      runtime->done = 1;

	      config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit =
		config->dd_din_vs_din.doit = config->dd_din_vs_sys.doit= 0;

	      g_print("ZDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);

	      /* print out general statistics */
	      Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);

	      return;
	    }

	  runtime->stats->dd_stats.din_vs_sys = tmp;
	}
    }

  if(config->dd_din_vs_sin.doit)
    {
      if(din >= config->g_dd_start_din)
	{
	  DdNode *tmp;
	  tmp=adamant_zdd_build_tuple(runtime->stats->dd_stats.manager,
				      runtime->stats->dd_stats.din_vs_sin,
				      din, oper->soper->ip);
	  if(tmp == NULL)
	    {
	      //! if this happens, the program is exploding
	      runtime->done = 1;

	      config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit =
		config->dd_din_vs_din.doit = 0;

	      g_print("ZDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);

	      /* print out general statistics */
	      Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);
	      return;
	    }
#ifdef BUILDZDD
	  // This is check is a hack
	  if(runtime->stats->dd_stats.din_vs_sin != NULL)
	    {
	      Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
				     runtime->stats->dd_stats.din_vs_sin);
	    }

	  runtime->stats->dd_stats.din_vs_sin = tmp;
#else
	  Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
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

	    tmp = adamant_zdd_build_tuple(runtime->stats->dd_stats.manager,
					  runtime->stats->dd_stats.din_vs_din,
					  din, si->din_src);
	    if(tmp == NULL)
	      {
		//! if this happens, the program is exploding
		runtime->done = 1;

		config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit =
		  config->dd_din_vs_din.doit = 0;

		g_print("ZDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);

		/* print out general statistics */
		Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);
		return;
	      }
#ifdef BUILDZDD
	    // This is check is a hack
	    if(runtime->stats->dd_stats.din_vs_din != NULL)
	      {
		Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
				       runtime->stats->dd_stats.din_vs_din);
	      }

	    runtime->stats->dd_stats.din_vs_din = tmp;
#else
	    Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
				   tmp);
#endif
	  } // end for(k = 0...
	}// end if din > config...
    } // if(config->dd_din_vs_din...

  if(config->dd_din_vs_hot.doit)
    {
      // Increment the sin count
      adamant_hot_sinInc(hotManager, oper->soper->ip);

      // Output the DINxSIN buffer
      adamant_hot_buffer_writetuple(hotManager, din, oper->soper->ip);
    }

  if(config->dd_make.doit == FALSE)
    { /* Add other zdd based stats checks here */

      zdd_delaycount++;
      if(zdd_delaycount > ZDDMEMORYCHECKDELAY){
	zdd_delaycount = 0;

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

	    //! this section checks to see if the ZDD size,after garbage
	    //! collection, has exceded a preset limit
	    if((config->dd_mem_cap != 0) &&
	       (current_memory_use >= (config->dd_mem_cap)))
	      {

		/* Kill Adamantium Gracefully */
		g_print("\nAdamantium Stopped Prematurely\n");

		runtime->done = 1;

		config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit =
		  config->dd_din_vs_din.doit = 0;

		g_print("ZDD Creation Memory Limit Hit\n");
		g_print("ZDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);

		/* print out general statistics */
		Cudd_PrintInfo(stats->dd_stats.manager, stdout);

	      }
	  }
      }
    }
}

static DdNode *build_xstar(DdManager *manager, guint64 x)
{
  int i, j;
  DdNode * tmp, * itenode, * logicZero;

  logicZero = Cudd_ReadLogicZero(manager);
  Cudd_Ref(logicZero);

  //	printf("Adding %llx\n", x);
  /* make a new zdd for this tuple */
  itenode = Cudd_ReadZddOne(manager, 0);
  Cudd_Ref(itenode);

  /* iterate through the 64 bit values for both X and Y */
  for (i = 0; i < (sizeof(guint64) * 8 * 2); i++) {
    guint64 bitset;
    int v;

    // This code only works for a total of 128 ZDD vars, beware
    assert(Cudd_ReadSize(manager) <= ZDDNUM);
    v= Cudd_ReadInvPerm(manager,127-i);

    tmp = itenode;
    if(v <= 63) {
      bitset = (x & mask((guint64)(v)));
      //printf("Adding variable %d to the tuple with value %d\n",v,(bitset != 0));
      if(bitset) {
	itenode = Cudd_zddIte(manager, Cudd_zddIthVar(manager,v), tmp,
			      logicZero);
	Cudd_Ref(itenode);
      } else {
	itenode = Cudd_zddIte(manager, Cudd_zddIthVar(manager, v),
			      logicZero, tmp);
	Cudd_Ref(itenode);
      }
    } else {
      itenode = Cudd_zddIte(manager, Cudd_zddIthVar(manager, v),
			    tmp,tmp);
    }
    Cudd_RecursiveDerefZdd(manager, tmp);
  }

  Cudd_RecursiveDerefZdd(manager, logicZero);

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


DdNode *adamant_zdd_range(AdamantRuntime * runtime, guint64 high, guint64 low)
{
  int i;
  DdManager *manager;
  DdNode * retDD, * highDD[ZDDNUM/2], * lowDD[ZDDNUM/2];
  struct timeval stime, etime;

  manager = runtime->stats->dd_stats.manager;

  gettimeofday(&stime,NULL);

  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Need to redo/check the make variable function
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  for(i=0; i < 64; i++)
    {
      //	  highDD[i] = adamant_zdd_make_var(manager, high);
    }

  for(i=0; i < 64; i++)
    {
      //	  lowDD[i] = adamant_zdd_make_var(manager, low);
    }

  retDD = Cudd_Xgty(manager, ZDDNUM, NULL, highDD, lowDD);
  Cudd_Ref(retDD);

  // CLEANUP
  for(i = 0; i < (ZDDNUM/2); i++)
    {
      Cudd_RecursiveDerefZdd(manager, highDD[i]);
      Cudd_RecursiveDerefZdd(manager, lowDD[i]);
    }

  gettimeofday(&etime,NULL);
  g_print("Time for range search is %g\n",
	  (etime.tv_sec + (etime.tv_usec/1000000.0)) -
	  (stime.tv_sec + (stime.tv_usec/1000000.0)));


  return (retDD);
}


void adamant_zdd_dinhot(AdamantRuntime *runtime, adamantHotManager *hotManager)
{
  AdamantConfig *config = runtime->config;
  AdamantStats *stats = runtime->stats;

  if(config->dd_din_vs_hot.doit)
    {
      adamantHotBuffer hotBufMem;

      g_print("Creating DINxHOT DD\n");
      adamant_hot_buffer_write2read(hotManager);

      while (adamant_hot_buffer_readbuffer(hotManager, &hotBufMem) != 0)
	{
	  guint64 din = hotBufMem.din_;
	  guint64 sin_count = adamant_hot_sinLookup(hotManager, hotBufMem.sin_);

	  if((din >= config->g_dd_start_din) && (sin_count > 0))
	    {
	      DdNode *tmp;
	      tmp = adamant_zdd_build_tuple(runtime->stats->dd_stats.manager,
					    runtime->stats->dd_stats.din_vs_hot,
					    din, sin_count);
	      if(tmp == NULL)
		{
		  //! if this happens, the program is exploding
		  runtime->done = 1;

		  config->dd_din_vs_ready.doit = config->dd_din_vs_sin.doit = 0;
		  config->dd_din_vs_din.doit = config->dd_din_vs_hot.doit = 0;

		  g_print("ZDD Op Count: %"G_GUINT64_FORMAT"\n\n", din);

		  /* print out general statistics */
		  Cudd_PrintInfo(runtime->stats->dd_stats.manager, stdout);
		  return;
		}

#ifdef BUILDZDD
	      // This is check is a hack
	      if(runtime->stats->dd_stats.din_vs_hot != NULL)
		{
		  Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
					 runtime->stats->dd_stats.din_vs_hot);
		}

	      runtime->stats->dd_stats.din_vs_hot = tmp;
#else
	      Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
				     tmp);
#endif
	    }
	}

      //! Close up the buffer
      adamant_hot_buffer_close(hotManager);
    }
}


void adamant_zdd_finalize(AdamantRuntime *runtime, guint64 opcount)
{
  AdamantConfig *config = runtime->config;
  AdamantStats *stats = runtime->stats;

  if(config->dd_make.doit == TRUE)
    {

      DdNode * temp_zdd_node = NULL;

#ifdef DO_SLICE_ANAL
      if(config->dd_din_vs_din.doit) {
	do_slice_anal(runtime);
      }
#endif

      //! grab the current time and print
      gettimeofday(&timecount, NULL);
      g_print("DDTIME:%u\n",((unsigned int)timecount.tv_sec - zdd_first_time));

      cuddGarbageCollect(stats->dd_stats.manager, 1);

      /* print out general statistics */
      Cudd_PrintInfo(stats->dd_stats.manager, stdout);

      //! Finalize Hot Code
      adamant_zdd_dinhot_finalize(runtime, opcount);

      //! Output the DD file
      int tConfigTick = config->dd_tick_interval;
      config->dd_tick_interval = 1;
      adamant_zdd_tickoutput(runtime, opcount);
      config->dd_tick_interval = tConfigTick;
    }
}


/*!
  This function will output tick information related to ZDD creation and, if requested,
  it will save off the current ZDD

  \date 09/28/07
  \author Graham Price
*/
void adamant_zdd_tickoutput(AdamantRuntime *runtime, guint64 opcount)
{
  AdamantConfig *config = runtime->config;

  if(config->dd_tick_interval != 0)
    {
      AdamantConfig *config = runtime->config;
      DdNode * tempZDD = NULL;
      FILE * newfile = NULL;
      gchar * fileName = NULL;
      Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP)*/
      Dddmp_MoreDDHeaderInfo headerInfo;

      headerInfo.extraTraceInfo = g_new(char, 512);

      //! get the information needed to save off the tick ZDD
      if( config->dd_din_vs_ready.doit )
	{

	  newfile = fopen(config->dd_din_vs_ready.outfilename, "w+");

	  fileName = g_strdup(config->dd_din_vs_ready.outfilename);

	  //! add extra information to this ZDD dump
	  g_snprintf(headerInfo.extraTraceInfo,512,
		     "type:dinrdy,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		     g_dinstart, opcount);

	  //! set up this ZDD node to be written out
	  tempZDD = runtime->stats->dd_stats.din_vs_ready;
	}
      if( config->dd_din_vs_sys.doit )
	{
	  newfile = fopen(config->dd_din_vs_sys.outfilename, "w+");
	  fileName = g_strdup(config->dd_din_vs_sys.outfilename);

	  g_snprintf(headerInfo.extraTraceInfo,512,
		     "type:dinsys,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		     g_dinstart, opcount);

	  tempZDD = runtime->stats->dd_stats.din_vs_sys;
	}
      
      if( config->dd_din_vs_sin.doit )
	{
	  newfile = fopen(config->dd_din_vs_sin.outfilename, "w+");
	  fileName = g_strdup(config->dd_din_vs_sin.outfilename);

	  g_snprintf(headerInfo.extraTraceInfo,512,
		     "type:dinsin,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		     g_dinstart, opcount);

	  tempZDD = runtime->stats->dd_stats.din_vs_sin;
	}
      if( config->dd_din_vs_din.doit )
	{
	  newfile = fopen(config->dd_din_vs_din.outfilename, "w+");

	  fileName = g_strdup(config->dd_din_vs_din.outfilename);
	  g_snprintf(headerInfo.extraTraceInfo,512,
		     "type:dindin,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		     g_dinstart, opcount);

	  tempZDD = runtime->stats->dd_stats.din_vs_din;
	}

      //! now save off the tick zdd
      if (newfile != NULL)
	{
	  if(zdd_ordr == NULL)
	    {
	      assert(tempZDD != NULL);

	      Dddmp_cuddZddStore(runtime->stats->dd_stats.manager, NULL,
				 tempZDD, NULL, NULL,
				 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
				 fileName, newfile);
	    }
	  else
	    {
	      assert(tempZDD != NULL);

	      Dddmp_cuddZddStore(runtime->stats->dd_stats.manager, NULL,
				 tempZDD, NULL, zdd_ordr,
				 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
				 fileName, newfile);
	    }

	  fclose(newfile);
	}
      if(fileName != NULL)
	{
	  g_free(fileName);
	}

      //! CLEANUP
      free(headerInfo.extraTraceInfo);
    }
}

void adamant_zdd_tick(AdamantRuntime *runtime, guint64 opcount)
{
  AdamantConfig *config = runtime->config;

  if(config->dd_din_vs_ready.doit ||
     config->dd_din_vs_sin.doit ||
     config->dd_din_vs_din.doit ||
     config->dd_din_vs_hot.doit ||
     config->dd_din_vs_sys.doit ||
     FALSE) { // Add other zdd based stats checks here

    DdManager * manager = runtime->stats->dd_stats.manager;

    //! ** print out information just for the opcount,tuple,memory usage info **

    //! grab the current time
    gettimeofday(&timecount, NULL);

    if(first_zdd_tick == 1)
      {
	//! set up the header info and the first time
	first_zdd_tick = 0;

	g_print("*Tick Output*\n");
	g_print("*opcount, GC Time, GC Count,Cache Collisions, memory, time*\n");
      }

    g_print("%"G_GUINT64_FORMAT",%ld,%d,%.0f,%lu,%u\n",
	    opcount,
	    Cudd_ReadGarbageCollectionTime(manager),
	    Cudd_ReadGarbageCollections(manager),
	    (manager->cachecollisions),
	    ((manager->keysZ - manager->deadZ) * sizeof(DdNode)),
	    ((unsigned int)timecount.tv_sec - zdd_first_time));
  }
}


/*!

  This function is should cleanup ZDD work
  such as managers, nodes, etc

  \author Graham Price
  \date 11/14/07

*/
void adamant_zdd_quit(AdamantStats *stats)
{
  int count = 0;

  if(stats->dd_stats.manager) {
    /* if(stats->dd_stats.din_vs_ready) { */
    /*   Cudd_RecursiveDerefZdd(stats->dd_stats.manager, stats->dd_stats.din_vs_ready); */
    /* } */
    /* if(stats->dd_stats.din_vs_sin) { */
    /*   Cudd_RecursiveDerefZdd(stats->dd_stats.manager, stats->dd_stats.din_vs_sin); */
    /* } */
    /* if(stats->dd_stats.din_vs_din) { */
    /*   Cudd_RecursiveDerefZdd(stats->dd_stats.manager, stats->dd_stats.din_vs_din); */
    /* } */
    /* if(stats->dd_stats.din_vs_hot) { */
    /*   Cudd_RecursiveDerefZdd(stats->dd_stats.manager, stats->dd_stats.din_vs_hot); */
    /* } */

    /* if((count=Cudd_CheckZeroRef(stats->dd_stats.manager))) { */
    /*   fprintf(stderr,"ZDD Manager returned final count = %d\n",count); */
    /* } */
    Cudd_Quit(stats->dd_stats.manager);
  }
}


//! this will initialize the ZDD node and manager
//! given header information
//! this does destroy the passed char string
int adamant_zdd_headerinit(FILE * zddfile, AdamantRuntime *runtime )
{
  int return_val = 0;
  guint64 temp_dinstart = 0;
  char * headerInfo = NULL;
  DdNode * tempBddNode = NULL;
  DdNode * tempZddNode = NULL;
  char * fileinfo = calloc(512, sizeof(char));
  DdManager * manager = runtime->stats->dd_stats.manager;


  tempZddNode = Dddmp_cuddZddLoad(runtime->stats->dd_stats.manager,
				  DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				  DDDMP_MODE_BINARY, &headerInfo,
				  fileinfo, zddfile);
  Cudd_Ref(tempZddNode);

  if((tempZddNode != NULL) && (headerInfo != NULL))
    {
      char* token = strtok(headerInfo, ":");
      while(token != NULL)
	{
	  if(matchkeywd(token,"type"))
	    {
	      //! save the type of ZDD this is
	      token = strtok(NULL,",");

	      if(matchkeywd(token,"dindin"))
		{
		  //! replace any existing ZDDs of this type
		  if(runtime->stats->dd_stats.din_vs_din != NULL)
		    {
		      Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
					     runtime->stats->dd_stats.din_vs_din);
		    }

		  runtime->stats->dd_stats.din_vs_din = tempZddNode;
		}
	      else if(matchkeywd(token,"dinrdy"))
		{
		  //! replace any existing ZDDs of this type
		  if(runtime->stats->dd_stats.din_vs_ready != NULL)
		    {
		      Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
					     runtime->stats->dd_stats.din_vs_ready);
		    }

		  runtime->stats->dd_stats.din_vs_ready = tempZddNode;
		}
	      else if(matchkeywd(token,"dinsin"))
		{
		  //! replace any existing ZDDs of this type
		  if(runtime->stats->dd_stats.din_vs_sin != NULL)
		    {
		      Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
					     runtime->stats->dd_stats.din_vs_sin);
		    }

		  runtime->stats->dd_stats.din_vs_sin = tempZddNode;
		}
	      else if(matchkeywd(token,"dinhot"))
		{
		  //! replace any existing ZDDs of this type
		  if(runtime->stats->dd_stats.din_vs_hot != NULL)
		    {
		      Cudd_RecursiveDerefZdd(runtime->stats->dd_stats.manager,
					     runtime->stats->dd_stats.din_vs_hot);
		    }

		  runtime->stats->dd_stats.din_vs_hot = tempZddNode;
		}
	    }
	  else if(matchkeywd(token,"dinstop"))
	    {
	      //! glue new instructions to the start
	      //! of this ZDD
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

  This function was taken from adamant_bdd.c

*/
DdNode *adamant_zdd_bdd_build_tuple(DdManager *manager, guint64 x, guint64 y)
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


/*

  This function was taken from adamant_bdd.c

*/
DdNode * adamant_zdd_build_tuple(DdManager *manager, DdNode *set,
				 guint64 x, guint64 y)
{
  int i = 0, j = 0;
  DdNode * tmp = NULL, * retnode = NULL, * newNode = NULL, * zero = NULL, * one = NULL;

  //! NOTE: normally ref'ing one and zero is not needed

  //! grab the ZDD One
  //    one = Cudd_ReadZddOne(manager, 0);
  one = DD_ONE(manager);
  Cudd_Ref(one);

  //! grab zero
  zero = Cudd_ReadZero(manager);
  Cudd_Ref(zero);

  //! setup the itenode
  newNode = one;

  /* iterate through the 64 bit values for both X and Y */
  for (i = 0; i < (sizeof(guint64) * 8 * 2); i++)
    {
      guint64 bitset;
      int v = 0;

      // This code only works for a total of 128 BDD vars, beware
      assert(Cudd_ReadSize(manager) <= 128);
      v = Cudd_ReadInvPermZdd(manager, 127-i);

      //! reset the tmp node
      tmp = newNode;

      bitset = (v < 64)?(x & mask((guint64)(v))):(y & mask((guint64)(v-64)));

      //  printf("Adding variable %d to the tuple with value %d\n",v,bitset);
      if(bitset)
	{
	  do{
	    newNode = cuddUniqueInterZdd(manager, v, tmp, zero);
//        newNode = cuddZddGetNode(manager, v, tmp, zero);

//	    cuddSatInc(newNode->ref);

	    Cudd_Ref(newNode);

	    // The standard tmp derefrence
	    Cudd_RecursiveDerefZdd(manager, tmp);

	    //                        newNode = cuddZddGetNode(manager, v, tmp, zero);
	  } while (manager->reordered == 1);
	}
    }

  //! union the tuple zdd to the trace zdd
  //! NOTE: The caller must clean up the original trace ZDD
  tmp = Cudd_zddUnion(manager, newNode, set);
  Cudd_Ref(tmp);

  // kill the old tuple zdd
  Cudd_RecursiveDerefZdd(manager, newNode);

  return (tmp);
}

/*

  This function was taken from adamant_bdd.c

*/
DdNode * adamant_zdd_build_fullcube(DdManager *manager, guint64 x, guint64 y)
{
  int i = 0, j = 0;
  DdNode * tmp = NULL, * retnode = NULL, * newNode = NULL, * zero = NULL, * one = NULL;

  //! NOTE: normally ref'ing one and zero is not needed
  //! grab the ZDD One
  one = DD_ONE(manager);
  Cudd_Ref(one);

  //! grab zero
  zero = Cudd_ReadZero(manager);
  Cudd_Ref(zero);

  //! setup the initial node value
  newNode = one;

  /* iterate through the 64 bit values for both X and Y */
  for (i = 0; i < (sizeof(guint64) * 8 * 2); i++)
    {
      guint64 bitset;
      int v = 0;

      // This code only works for a total of 128 BDD vars, beware
      assert(Cudd_ReadSize(manager) <= 128);
      v = Cudd_ReadInvPermZdd(manager, 127-i);

      //! reset the tmp node
      tmp = newNode;

      bitset = (v < 64)?(x & mask((guint64)(v))):(y & mask((guint64)(v-64)));

      if(bitset)
	{
	  do{
	    newNode = cuddUniqueInterZdd(manager, v, tmp, tmp);

	    Cudd_Ref(newNode);

	    // The standard tmp derefrence
	    Cudd_RecursiveDerefZdd(manager, tmp);

	  } while (manager->reordered == 1);
	}
    }

  Cudd_RecursiveDerefZdd(manager, zero);

  return (newNode);
}


int DynZddReorderHook(DdManager * dd, const char * str, void * data)
{
  int k = 0;

  g_print ("*** Start Variable Reorder Hook*** \n");

  //! print out the variable positions for debugging
  for(k = 0; k < 128; k++)
    {
      g_print("position %d has variable %d\n", k,
	      Cudd_ReadInvPerm(dd,k));
    }
  g_print ("*** End Variable Reorder Hook*** \n\n");

  return 1;
}


DdNode * adamant_zdd_evaluate(DdManager *manager)
{
  DdNode * return_node = NULL;

  return(return_node);
}

/*
  Special output for size data
*/
void adamant_zdd_sizeout(DdManager * manager, guint64 opcount)
{
  //! grab the current time
  gettimeofday(&timecount, NULL);

  if(first_zdd_tick == 1)
    {
      //! set up the header info and the first time
      first_zdd_tick = 0;
      g_print("*Tick Output*\n");
      g_print("*Opcount, Slots, Keys, Dead, Time*\n");
    }
  g_print("%"G_GUINT64_FORMAT",%d,%d,%d,%u\n",
	  opcount,
	  manager->slots,
	  manager->keysZ,
	  manager->deadZ,
	  ((unsigned int)timecount.tv_sec - zdd_first_time));

}

/*
  Special output for subtable size data
*/
void adamant_zdd_subTableSizeOut(DdManager * manager, guint64 opcount)
{
  //! grab the current time
  gettimeofday(&timecount, NULL);

  //! clean up some garbage
  cuddGarbageCollect(manager, 1);

  if(first_zdd_tick == 1)
    {
      //! set up the header info and the first time
      first_zdd_tick = 0;
      g_print("*Tick Output*\n");
      g_print("*Opcount, Slots, Keys, Dead, Time*\n");
    }
  int i = 0;

  g_print("Opcount:%"G_GUINT64_FORMAT",%d,%d,%d,%u\n",
	  opcount,
	  manager->slots,
	  manager->keysZ,
	  manager->deadZ,
	  ((unsigned int)timecount.tv_sec - zdd_first_time));

  for(i = 0; i < manager->sizeZ; i++)
    {
      g_print("%d", manager->subtableZ[i].keys);

      if((i+1) < manager->sizeZ)
	{
	  g_print(",");
	}
    }
  g_print("\n");
}


void adamant_zdd_dinhot_finalize(AdamantRuntime *runtime, guint64 opcount)
{

  AdamantConfig *config = runtime->config;
  AdamantStats *stats = runtime->stats;

  if(config->dd_make.doit == TRUE)
    {
      DdNode * temp_zdd_node = NULL;

      //! Make the {DIN,HOT} set
      adamant_zdd_dinhot(runtime, hotManager);

      if(config->dd_din_vs_hot.outfilename != NULL)
	{
	  //! grab the current time and print
	  gettimeofday(&timecount, NULL);
	  g_print("HOT DD TIME:%u\n",((unsigned int)timecount.tv_sec - zdd_first_time));

	  FILE * newfile;
	  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
	  Dddmp_MoreDDHeaderInfo headerInfo;

	  //! Convert the buffer to read

	  g_print("TOPHOT:%u:%u\n",
		  adamant_hot_getTopValue(hotManager),
		  adamant_hot_getTopSin(hotManager));
	  g_print("BOTTOMHOT:%u:%u\n",
		  adamant_hot_getBottomValue(hotManager),
		  adamant_hot_getBottomSin(hotManager));

	  newfile = fopen(config->dd_din_vs_hot.outfilename, "w+");
	  if(newfile != NULL)
	    {
	      headerInfo.extraTraceInfo = calloc(512,sizeof(char));

	      //! add extra information to this ZDD dump

	      g_snprintf(headerInfo.extraTraceInfo,512,
			 "type:dinhot,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
			 g_dinstart, opcount);

	      if(zdd_ordr == NULL)
		{
		  Dddmp_cuddZddStore(stats->dd_stats.manager, NULL,
				     stats->dd_stats.din_vs_hot, NULL, NULL,
				     DDDMP_MODE_BINARY, extrainfo, &headerInfo,
				     config->dd_din_vs_hot.outfilename, newfile);
		}
	      else
		{
		  Dddmp_cuddZddStore(stats->dd_stats.manager, NULL,
				     stats->dd_stats.din_vs_hot, NULL, zdd_ordr,
				     DDDMP_MODE_BINARY, extrainfo, &headerInfo,
				     config->dd_din_vs_hot.outfilename, newfile);

		}

	      fclose(newfile);
	      free(headerInfo.extraTraceInfo);
	    }
	}
    }
}

DdNode * adamant_zdd_varswap(DdManager * manager, DdNode * node)
{
  int zdd_permut[ZDDNUM];
  int i = 0;
  int k = 0;

  //! create new vector order
  for(i=0; i < (ZDDNUM/2); i++)
    {
      k = (i+(ZDDNUM/2));

      zdd_permut[k] = i;
      zdd_permut[i] = k;
    }

  //! rearrange the variables in this zdd
  DdNode * ret = mg_Extra_zddPermute(manager, node, zdd_permut);

  Cudd_Ref(ret);

  return (ret);
}

DdNode * adamant_zdd_abstractY(DdManager *manager, DdNode * sliceNode)
{
  // Make the zero zdd
  DdNode * zeroNode = Cudd_ReadZero(manager);
  Cudd_Ref(zeroNode);

  // Build positive cube for existential abstraction
  DdNode * yCubeNode = adamant_zdd_build_tuple(manager, zeroNode, 0, 0xffffffffffffffffLL);
  Cudd_RecursiveDerefZdd(manager, zeroNode);

  // Create an abstraction of the slice ZDD
  DdNode * abstractNode = mg_Extra_zddExistAbstract(manager, sliceNode, yCubeNode);
  Cudd_Ref(abstractNode);

  Cudd_RecursiveDerefZdd(manager, yCubeNode);

  return (abstractNode);
}

DdNode * adamant_zdd_abstractX(DdManager *manager, DdNode * sliceNode)
{
  // Make the zero zdd
  DdNode * zeroNode = Cudd_ReadZero(manager);
  Cudd_Ref(zeroNode);
  
  // Build positive cube for existential abstraction
  DdNode * xCubeNode = adamant_zdd_build_tuple(manager, zeroNode, 0xffffffffffffffffLL, 0);
  Cudd_RecursiveDerefZdd(manager, zeroNode);

  // Create an abstraction of the slice ZDD
  DdNode * abstractNode = mg_Extra_zddExistAbstract(manager, sliceNode, xCubeNode);
  Cudd_Ref(abstractNode);

  return (abstractNode);
}

DdNode * adamant_zdd_xDC(DdManager *manager, DdNode * node)
{
  // Build positive and negitive cubes
  DdNode * xFullCubeNode = adamant_zdd_build_fullcube(manager, 0xffffffffffffffffLL, 0);

  // Replace the missing nodes in the X position with positive Don't Care nodes
  DdNode * productNode = Cudd_zddUnateProduct( manager, node, xFullCubeNode);
  Cudd_Ref(productNode);
  Cudd_RecursiveDerefZdd(manager, xFullCubeNode);

  return (productNode);
}

DdNode * adamant_zdd_yDC(DdManager *manager, DdNode * node)
{
  // Build positive and negitive cubes
  DdNode * yFullCubeNode = adamant_zdd_build_fullcube(manager, 0, 0xffffffffffffffffLL);

  // Replace the missing nodes in the Y position with positive Don't Care nodes
  DdNode * productNode = Cudd_zddUnateProduct( manager, node, yFullCubeNode);
  // DdNode * productNode = Extra_zddDotProduct( manager, node, yFullCubeNode);
  Cudd_Ref(productNode);
  Cudd_RecursiveDerefZdd(manager, yFullCubeNode);

  return (productNode);
}


DdNode * adamant_zdd_dindin_forward_slice(DdManager * manager, DdNode * sliceDD, DdNode * dindinDD)
{
  // Form a {DIN, blank} set
  DdNode * dinblnkDD = adamant_zdd_abstractY(manager, sliceDD);

  // Swap the variable positions to form a {blank, DIN} set
  DdNode * swappedDinNode = adamant_zdd_varswap( manager, dinblnkDD);
  Cudd_RecursiveDerefZdd(manager, dinblnkDD);

  // Form a {univ, DIN} set
  DdNode * univDinNode = adamant_zdd_xDC(manager, swappedDinNode);
  Cudd_RecursiveDerefZdd(manager, swappedDinNode);

  // Get a new {DIN, {DIN}} set
  DdNode * newDinDinDD = Cudd_zddIntersect(manager, univDinNode, dindinDD);
  Cudd_Ref(newDinDinDD);
  Cudd_RecursiveDerefZdd(manager, univDinNode);

  // Union with the original slice {DIN, {DIN}} set
  DdNode * newRevSliceNode = Cudd_zddUnion( manager, sliceDD, newDinDinDD);
  Cudd_Ref(newRevSliceNode);

  return (newRevSliceNode);
}

DdNode * adamant_zdd_reverse_sliceUP(DdManager * manager, DdNode * sliceNode, DdNode * targetNode)
{
  // Form Y don't care values
  // DdNode * dcNode = adamant_zdd_yDC(manager, sliceNode);
  //  DdNode * targetY = adamant_zdd_abstractX(manager, targetNode);
  //  DdNode * dcNode = Cudd_zddUnateProduct(manager, sliceNode, targetY);
  //  Cudd_Ref(dcNode);
  //  Cudd_RecursiveDerefZdd(manager, targetY);


  // Logic AND the target ZDD node with the correctly-formed slice node
  //DdNode * productNode = Cudd_zddIte(manager, targetNode,
  //  dcNode, Cudd_ReadZero(manager));
  DdNode * productNode = Cudd_zddUnateProduct(manager, targetNode, sliceNode);
  //  DdNode * productNode = Extra_zddDotProduct(manager, targetNode, sliceNode);
  // DdNode * productNode = Cudd_zddIntersect(manager, targetNode, dcNode);
  Cudd_Ref(productNode);
  //  Cudd_RecursiveDerefZdd(manager, dcNode);

  // Remove the X variables
  DdNode * newXnode = adamant_zdd_abstractX( manager, productNode);
  Cudd_RecursiveDerefZdd(manager, productNode);

  /*
    NOTE!!
    Switching and unioning could be done quicker with vector composition
   */
  DdNode * swappedNode = adamant_zdd_varswap( manager, newXnode);
  Cudd_RecursiveDerefZdd(manager, newXnode);

  DdNode * newRevSliceNode = Cudd_zddUnion( manager, sliceNode, swappedNode);
  Cudd_Ref(newRevSliceNode);

  return (newRevSliceNode);
}

DdNode * adamant_zdd_reverse_sliceITE(DdManager * manager, DdNode * sliceNode, DdNode * targetNode)
{
  // Form Y don't care values
  DdNode * dcNode = adamant_zdd_yDC(manager, sliceNode);
  //  DdNode * targetY = adamant_zdd_abstractX(manager, targetNode);
  //  DdNode * dcNode = Cudd_zddUnateProduct(manager, sliceNode, targetY);
  //  Cudd_Ref(dcNode);
  //  Cudd_RecursiveDerefZdd(manager, targetY);


  // Logic AND the target ZDD node with the correctly-formed slice node
  DdNode * productNode = Cudd_zddIte(manager, targetNode,
				     dcNode, Cudd_ReadZero(manager));
  // DdNode * productNode = Cudd_zddUnateProduct(manager, targetNode, sliceNode);
  //  DdNode * productNode = Extra_zddDotProduct(manager, targetNode, sliceNode);
  // DdNode * productNode = Cudd_zddIntersect(manager, targetNode, dcNode);
  Cudd_Ref(productNode);
  Cudd_RecursiveDerefZdd(manager, dcNode);

  // Remove the X variables
  DdNode * newXnode = adamant_zdd_abstractX( manager, productNode);
  Cudd_RecursiveDerefZdd(manager, productNode);

  /*
    NOTE!!
    Switching and unioning could be done quicker with vector composition
   */
  DdNode * swappedNode = adamant_zdd_varswap( manager, newXnode);
  Cudd_RecursiveDerefZdd(manager, newXnode);

  DdNode * newRevSliceNode = Cudd_zddUnion( manager, sliceNode, swappedNode);
  Cudd_Ref(newRevSliceNode);

  return (newRevSliceNode);
}

DdNode * adamant_zdd_reverse_slice_nounion(DdManager * manager, DdNode * sliceNode, DdNode * targetNode)
{
  // Form Y don't care values
  DdNode * dcNode = adamant_zdd_yDC(manager, sliceNode);
  //  DdNode * targetY = adamant_zdd_abstractX(manager, targetNode);
  //  DdNode * dcNode = Cudd_zddUnateProduct(manager, sliceNode, targetY);
  //  Cudd_Ref(dcNode);
  //  Cudd_RecursiveDerefZdd(manager, targetY);


  // Logic AND the target ZDD node with the correctly-formed slice node
  //DdNode * productNode = Cudd_zddIte(manager, targetNode,
  //  dcNode, Cudd_ReadZero(manager));
  //DdNode * productNode = Cudd_zddUnateProduct(manager, targetNode, sliceNode);
  //  DdNode * productNode = Extra_zddDotProduct(manager, targetNode, sliceNode);
  DdNode * productNode = Cudd_zddIntersect(manager, targetNode, dcNode);
  Cudd_Ref(productNode);
  Cudd_RecursiveDerefZdd(manager, dcNode);

  // Remove the X variables
  DdNode * newXnode = adamant_zdd_abstractX( manager, productNode);
  Cudd_RecursiveDerefZdd(manager, productNode);

  /*
    NOTE!!
    Switching and unioning could be done quicker with vector composition
   */
  DdNode * swappedNode = adamant_zdd_varswap( manager, newXnode);
  Cudd_RecursiveDerefZdd(manager, newXnode);

  return (swappedNode);
}

DdNode * adamant_zdd_reverse_slice(DdManager * manager, DdNode * sliceNode, DdNode * targetNode)
{
  // Form Y don't care values
  DdNode * dcNode = adamant_zdd_yDC(manager, sliceNode);
  //  DdNode * targetY = adamant_zdd_abstractX(manager, targetNode);
  //  DdNode * dcNode = Cudd_zddUnateProduct(manager, sliceNode, targetY);
  //  Cudd_Ref(dcNode);
  //  Cudd_RecursiveDerefZdd(manager, targetY);


  // Logic AND the target ZDD node with the correctly-formed slice node
  //DdNode * productNode = Cudd_zddIte(manager, targetNode,
  //  dcNode, Cudd_ReadZero(manager));
  //DdNode * productNode = Cudd_zddUnateProduct(manager, targetNode, sliceNode);
  //  DdNode * productNode = Extra_zddDotProduct(manager, targetNode, sliceNode);
  DdNode * productNode = Cudd_zddIntersect(manager, targetNode, dcNode);
  Cudd_Ref(productNode);
  Cudd_RecursiveDerefZdd(manager, dcNode);

  // Remove the X variables
  DdNode * newXnode = adamant_zdd_abstractX( manager, productNode);
  Cudd_RecursiveDerefZdd(manager, productNode);

  /*
    NOTE!!
    Switching and unioning could be done quicker with vector composition
   */
  DdNode * swappedNode = adamant_zdd_varswap( manager, newXnode);
  Cudd_RecursiveDerefZdd(manager, newXnode);

  DdNode * newRevSliceNode = Cudd_zddUnion( manager, sliceNode, swappedNode);
  Cudd_Ref(newRevSliceNode);

  return (newRevSliceNode);
}

int adamant_zdd_memory(DdManager *manager, DdNode * node, AdamantRuntime *runtime)
{
  int returnval = 0;
  int who = RUSAGE_SELF;
  struct rusage usage;
  int ret;

  ret = getrusage(who, &usage);
}


int adamant_zdd_ddStore(DdManager *manager, DdNode * node,
			AdamantRuntime *runtime, guint64 opcount)
{
  AdamantConfig *config = runtime->config;
  DdNode * tempZDD = NULL;
  FILE * newfile = NULL;
  gchar * fileName = NULL;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP)*/
  Dddmp_MoreDDHeaderInfo headerInfo;

  headerInfo.extraTraceInfo = g_new(char, 512);

  //! get the information needed to save off the tick ZDD
  if( config->dd_din_vs_ready.doit )
    {

      newfile = fopen(config->dd_din_vs_ready.outfilename, "w+");

      fileName = g_strdup(config->dd_din_vs_ready.outfilename);

      //! add extra information to this ZDD dump
      g_snprintf(headerInfo.extraTraceInfo,512,
		 "type:dinrdy,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		 g_dinstart, opcount);

      //! set up this ZDD node to be written out
      tempZDD = runtime->stats->dd_stats.din_vs_ready;
    }
  if( config->dd_din_vs_sin.doit )
    {
      newfile = fopen(config->dd_din_vs_sin.outfilename, "w+");
      fileName = g_strdup(config->dd_din_vs_sin.outfilename);

      g_snprintf(headerInfo.extraTraceInfo,512,
		 "type:dinsin,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		 g_dinstart, opcount);

      tempZDD = runtime->stats->dd_stats.din_vs_sin;
    }
  if( config->dd_din_vs_din.doit )
    {
      newfile = fopen(config->dd_din_vs_din.outfilename, "w+");

      fileName = g_strdup(config->dd_din_vs_din.outfilename);
      g_snprintf(headerInfo.extraTraceInfo,512,
		 "type:dindin,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		 g_dinstart, opcount);

      tempZDD = runtime->stats->dd_stats.din_vs_din;
    }
 if( config->dd_din_vs_sys.doit )
    {
      newfile = fopen(config->dd_din_vs_din.outfilename, "w+");

      fileName = g_strdup(config->dd_din_vs_din.outfilename);
      g_snprintf(headerInfo.extraTraceInfo,512,
		 "type:dinsys,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT",",
		 g_dinstart, opcount);

      tempZDD = runtime->stats->dd_stats.din_vs_din;
    }
  //! now save off the zdd
  if (newfile != NULL)
    {
      if(zdd_ordr == NULL)
	{
	  assert(tempZDD != NULL);

	  Dddmp_cuddZddStore(runtime->stats->dd_stats.manager, NULL,
			     tempZDD, NULL, NULL,
			     DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			     fileName, newfile);
	}
      else
	{
	  assert(tempZDD != NULL);

	  Dddmp_cuddZddStore(runtime->stats->dd_stats.manager, NULL,
			     tempZDD, NULL, zdd_ordr,
			     DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			     fileName, newfile);
	}

      fclose(newfile);
    }
  if(fileName != NULL)
    {
      g_free(fileName);
    }

  //! CLEANUP
  free(headerInfo.extraTraceInfo);

  return(0);
}


int adamant_zdd_GetTupleTop2(DdManager * manager, DdNode * node,
				  guint64 * ptopX, guint64 * ptopY)
{

  DdNode * nextNode = node;
  *ptopY = 0;
  *ptopX = 0;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      if(varIndex > 64)
	{
	  *ptopY =  *ptopY | mask((guint64)(varIndex - 64));
	}
      else
	{
	  *ptopX =  *ptopX | mask((guint64)(varIndex));

	}

      nextNode = Cudd_T(nextNode);
    }
  return (0);
}


int adamant_zdd_GetTupleBottom2(DdManager * manager, DdNode * node,
			    guint64 * pbottomX, guint64 * pbottomY)
{
  *pbottomX = 0;
  *pbottomY = 0;

  DdNode * nextNode = node;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);
      DdNode * tNextNode = Cudd_E(nextNode);

      if (tNextNode == Cudd_ReadZero(manager))
	{
	  if(varIndex > 64)
	    {
	      *pbottomY =  *pbottomY | mask((guint64)(varIndex - 64));
	    }
	  else
	    {
	      *pbottomX =  *pbottomX | mask((guint64)(varIndex));
	    }
	  tNextNode = Cudd_T(nextNode);
	}
      nextNode = tNextNode;
    }

  return (0);
}


int adamant_zdd_GetTupleTop(DdManager * manager, DdNode * node,
				  guint64 * ptopX, guint64 * ptopY)
{
  *ptopY = 0;
  *ptopX = 0;

  //! Pull out the X-only node
  DdNode * xOnly = adamant_zdd_abstractY(manager, node);

  DdNode * nextNode = xOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      *ptopX =  *ptopX | mask((guint64)(varIndex));

      nextNode = Cudd_T(nextNode);
    }

  Cudd_RecursiveDerefZdd(manager, xOnly);  

  //! Pull out the Y-only node
  DdNode * yOnly = adamant_zdd_abstractX(manager, node);

   nextNode = yOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      *ptopY =  *ptopY | mask((guint64)(varIndex - 64));

      nextNode = Cudd_T(nextNode);
    }

  Cudd_RecursiveDerefZdd(manager, yOnly);

  return (0);
}

/*!
 *
 */
int adamant_zdd_GetTupleBottom(DdManager * manager, DdNode * node,
			    guint64 * pbottomX, guint64 * pbottomY)
{
  *pbottomX = 0;
  *pbottomY = 0;

  //! Pull out the X-only node
  DdNode * xOnly = adamant_zdd_abstractY(manager, node);

  DdNode * nextNode = xOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      DdNode * tNextNode = Cudd_E(nextNode);
      if (tNextNode == Cudd_ReadZero(manager))
	{
	  *pbottomX =  *pbottomX | mask((guint64)(varIndex));
	  tNextNode = Cudd_T(nextNode);
	}
      nextNode = tNextNode;
    }
  Cudd_RecursiveDerefZdd(manager, xOnly);

  //! Pull out the Y-only node
  DdNode * yOnly = adamant_zdd_abstractX(manager, node);

  nextNode = yOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      DdNode * tNextNode = Cudd_E(nextNode);
      if ( tNextNode == Cudd_ReadZero(manager))
	{
	  *pbottomY =  *pbottomY | mask((guint64)(varIndex - 64));
	  tNextNode = Cudd_T(nextNode);
	}
      nextNode = tNextNode;
    }

  Cudd_RecursiveDerefZdd(manager, yOnly);

  return (0);
}


DdNode * adamant_zdd_iterReverse_slice(DdManager * manager, DdNode * sliceNode,
					DdNode * targetNode, guint64 stopCount)
{
  guint64 count = 1;
  DdNode * oldSliceTotal = NULL;
  struct timeval startSliceTime, currentSliceTime;
  DdNode * newSlice = adamant_zdd_reverse_slice_nounion(manager, sliceNode, targetNode);
  DdNode * newSliceTotal = Cudd_zddUnion(manager, sliceNode, newSlice);
  Cudd_Ref(newSliceTotal);
  gettimeofday(&startSliceTime,NULL); // DEBUG !!!

  while((newSliceTotal != oldSliceTotal) && ((stopCount == 0) || (count < stopCount)))
    {
      oldSliceTotal = newSliceTotal;

      DdNode * tmpSlice = adamant_zdd_reverse_slice_nounion( manager,  newSlice,  targetNode);
      Cudd_RecursiveDerefZdd(manager, newSlice);
      newSlice = tmpSlice;
      
      DdNode * tmpSliceTotal = Cudd_zddUnion(manager, newSliceTotal, newSlice);
      Cudd_Ref(tmpSliceTotal);
      Cudd_RecursiveDerefZdd(manager, newSliceTotal);
      newSliceTotal = tmpSliceTotal;

      ++count;

      //! DEBUG !
      if ((count % 10000) == 0)
        {
          gettimeofday(&currentSliceTime,NULL); // DEBUG !!!

          double timeForSlice = (((currentSliceTime.tv_sec +
                       (currentSliceTime.tv_usec/1000000.0)) -
                      (startSliceTime.tv_sec +
                       (startSliceTime.tv_usec/1000000.0))));

          g_print("Time for %d slices:%f\n", count, timeForSlice);
       }
    }

  //! DEBUG !
  gettimeofday(&currentSliceTime,NULL); // DEBUG !!!
  
  double timeForSlice = (((currentSliceTime.tv_sec +
			   (currentSliceTime.tv_usec/1000000.0)) -
			  (startSliceTime.tv_sec +
			   (startSliceTime.tv_usec/1000000.0))));
  
  g_print("Slicing Complete. Time for %d slices:%f\n", count, timeForSlice);

  return(newSliceTotal);
}

/**Function********************************************************************

   Synopsis [Filters out code from a select region]

   Description [Filters out code from a select region that does not contribute
   to the final graph results]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * adamant_zdd_QuickDeadFilter(DdManager * dd_manager, 
				     DdNode * dinSelDD, 
				     DdNode * dd_dindin)
{
  guint64 xTop = 0;
  guint64 yTop = 0;

  if ((dd_manager == NULL) || (dd_dindin == NULL))
    {
      return (NULL);
    }

  printf("Taking one slice step forward...\n");

  // Create a {DIN_i,empty} set from our selection
  DdNode * dinEmptySel = adamant_zdd_abstractY(dd_manager, dinSelDD);

  // Swap the Vars {empty, DIN_d} 
  DdNode * emptyDinSel = adamant_zdd_varswap(dd_manager, dinEmptySel);

  // Create a {univ, DIN_d} set from our {empty, DIN_d} set
  DdNode * univDinSel = adamant_zdd_xDC(dd_manager, emptyDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, emptyDinSel);
    
  printf("Final forward slice step...\n");

  // Create a {DIN_i, DIN_d} forward step selection set
  DdNode * dinDinForwardDD = Cudd_zddIntersect(dd_manager, dd_dindin, univDinSel);
  Cudd_Ref(dinDinForwardDD);
  Cudd_RecursiveDerefZdd(dd_manager, univDinSel);

  // Now take a step back
  printf("Taking one slice step back...\n");

  // Create a {DIN_i,empty} set from our selection
  DdNode * dinEmptyRevDD = adamant_zdd_abstractY(dd_manager, dinDinForwardDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinForwardDD);
  
  // Create a {DIN_i, univ_d} set from our {DIN_i, empty} set
  DdNode * dinUnivRevDD = adamant_zdd_yDC(dd_manager, dinEmptyRevDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinEmptyRevDD);

   // Create a {DIN_i, DIN_d} forward step selection set
  DdNode * dinDinRevDD = Cudd_zddIntersect(dd_manager, dd_dindin, dinUnivRevDD);
  Cudd_Ref(dinDinRevDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnivRevDD);

  // Create a {empty,DIN_d} set from our selection
  DdNode * emptyDinRevDD = adamant_zdd_abstractX(dd_manager, dinDinRevDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinRevDD );

  // Swap the Vars {DIN,empty} 
  DdNode * dinEmptyRevFinalDD = adamant_zdd_varswap(dd_manager, emptyDinRevDD);
  Cudd_RecursiveDerefZdd(dd_manager,  emptyDinRevDD);

  // Create a {DIN, univ} set from our rev {DIN, empty} set
  DdNode * dinUnivRevFinalDD = adamant_zdd_yDC(dd_manager, dinEmptyRevFinalDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinEmptyRevDD);
  
  printf("Final reverse slice step...\n");

  // Create our final live DIN set
  DdNode * dinSelLiveDD = Cudd_zddIntersect(dd_manager, dinSelDD, dinUnivRevFinalDD);
  Cudd_Ref(dinSelLiveDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnivRevFinalDD);
  
  return(dinSelLiveDD);
}

/**Function********************************************************************

   Synopsis [Filters out code from a select region]

   Description [Filters out code from a select region that does not contribute
   to the final graph results]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * adamant_zdd_DeadReadyFilterSelection(DdManager * dd_manager, 
					      DdNode * dd_dinrdy, 
					      DdNode * dd_dindin,
					      DdNode * dinSelDD)
{
  guint64 xTop = 0;
  guint64 yTop = 0;

  if ((dd_manager == NULL) || (dd_dindin == NULL) || (dd_dinrdy == NULL))
    {
      return (NULL);
    }

  // Create a {DIN,empty} set from our selection
  DdNode * dinEmptySel = adamant_zdd_abstractY(dd_manager, dinSelDD);

  // Create a {DIN, Univ} set from our {DIN, empty} set
  DdNode * dinUnvSel = adamant_zdd_yDC(dd_manager, dinEmptySel);
  DdNode * emptyDinSel = adamant_zdd_varswap(dd_manager, dinEmptySel);
  Cudd_RecursiveDerefZdd(dd_manager, dinEmptySel);

  // Create a {DIN,RDY} set
  DdNode * dinRdySelDD = Cudd_zddIntersect(dd_manager, dd_dinrdy, dinUnvSel);
  Cudd_Ref(dinRdySelDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnvSel);

  // Create a {univ, DIN} set from our {empty, DIN} set
  DdNode * univDinSel = adamant_zdd_xDC(dd_manager, emptyDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, emptyDinSel);

  // Create a {DIN, {DIN}} selection set
  DdNode * dinDinSel = Cudd_zddIntersect(dd_manager, dd_dindin, univDinSel);
  Cudd_Ref(dinDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, univDinSel);

  // Find the smallest slice values
  guint64 smallX = 0;
  guint64 smallY = 0;
  guint64 zddNumHalf = ZDDNUM/2;

  int xlist [ZDDNUM/2];
  int ylist [ZDDNUM/2];
    
  //! gather a list of ZDD node variables
  unsigned int i = 0;
  for (i = 0; i < zddNumHalf; i+=1)
    {
      xlist[(zddNumHalf - 1) - i] = i;
      ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
    }

  adamant_zdd_GetTupleBottom2(dd_manager, dinDinSel, &smallX, &smallY);

  //! find the ZDD for the selected area
  DdNode * xLb =  mg_Cudd_zddLb(dd_manager, zddNumHalf, xlist, (uint64_t)smallX);
  Cudd_Ref(xLb);

  //! find the ZDD for the selected area
  DdNode * yLb =  mg_Cudd_zddLb(dd_manager, zddNumHalf, ylist, (uint64_t)smallY);
  Cudd_Ref(yLb);
  
  DdNode * lB = Cudd_zddUnateProduct(dd_manager, xLb, yLb);
  Cudd_Ref(lB);
  Cudd_RecursiveDerefZdd(dd_manager, xLb);
  Cudd_RecursiveDerefZdd(dd_manager, yLb);

  // Shrink our {DIN, {DIN}} selection set using the lower bound
  DdNode * dinDinTarget = Cudd_zddIntersect(dd_manager, lB, dd_dindin);
  Cudd_Ref(dinDinTarget);
  Cudd_RecursiveDerefZdd(dd_manager, lB);

  printf("Building forward slice...\n");

  // Forward slice from this selection set
  DdNode * dinDinSlice = adamant_zdd_BuildIterDinDinForwardSlice(dd_manager, dinDinSel, dinDinTarget, 0);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinSel);

  printf("Forward slice building complete\n");

  // START DEBUG
  // Remove the X variables
  DdNode * dinBlankDDd = adamant_zdd_abstractY( dd_manager, dinDinSlice);

  // add in the universal Y
  DdNode * dinUnivDDd = adamant_zdd_yDC(dd_manager, dinBlankDDd);
  Cudd_RecursiveDerefZdd(dd_manager, dinBlankDDd);
  
  DdNode * newDinRdyDDd = Cudd_zddIntersect(dd_manager, dinUnivDDd, dd_dinrdy);
  Cudd_Ref(newDinRdyDDd);

  adamant_zdd_GetTupleTop(dd_manager, newDinRdyDDd, &xTop, &yTop);  
  Cudd_RecursiveDerefZdd(dd_manager, newDinRdyDDd);

  printf("Top Slice Din:%d, Rdy:%d\n", xTop, yTop);
  // END DEBUG

  adamant_zdd_GetTupleTop(dd_manager, dd_dinrdy, &xTop, &yTop);

  // START DEBUG
  printf("Top Din:%d, Rdy:%d\n", xTop, yTop);
  // END DEBUG
  
  DdNode * zero = Cudd_ReadZero(dd_manager);
  Cudd_Ref(zero);
  
  // Build a tuple that represents our top ready time 
  DdNode * rdyNode = adamant_zdd_build_tuple(dd_manager, zero, 0, yTop); 
  Cudd_RecursiveDerefZdd(dd_manager, zero); 

  // Add in the universal set of X values
  DdNode * tmpSliceNode = adamant_zdd_xDC(dd_manager, rdyNode);
  Cudd_RecursiveDerefZdd(dd_manager, rdyNode);
  
  // Intersect our {univ,rdy} with {din,rdy}
  DdNode * intersectNode = Cudd_zddIntersect(dd_manager, dd_dinrdy, tmpSliceNode);
  Cudd_Ref(intersectNode);
  Cudd_RecursiveDerefZdd(dd_manager, tmpSliceNode);

  // Abstract away the rdy
  DdNode * dinOnlyDD = adamant_zdd_abstractY(dd_manager, intersectNode);
  Cudd_RecursiveDerefZdd(dd_manager, intersectNode);

  // Create a {DIN, Univ} set from our {DIN, empty} set
  dinUnvSel = adamant_zdd_yDC(dd_manager, dinOnlyDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinOnlyDD);

  // Create a {DIN, {DIN}} reverse slice set
  dinDinSel = Cudd_zddIntersect(dd_manager, dd_dindin, dinUnvSel);
  Cudd_Ref(dinDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnvSel);

  printf("Building the reverse slice...\n");

  // Build a slice from this {DIN}
  DdNode * revDinDinSlice = adamant_zdd_BuildIterDinDinReverseSlice(dd_manager, dinDinSel, dinDinSlice, 0, 0);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinSlice);

  // Form a {blank, DIN} set
  DdNode * blankDinDD = adamant_zdd_abstractX(dd_manager, revDinDinSlice);

  // Swap the variable positions to form a {DIN, blank} set
  DdNode * dinBlankDD = adamant_zdd_varswap(dd_manager, blankDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, blankDinDD);

  printf("Reverse slice building complete\n");

  // Build {DIN, univ}
  DdNode * rdySliceNode = adamant_zdd_yDC(dd_manager, dinBlankDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinBlankDD);
  
  DdNode * dinRdyIntersectNode = Cudd_zddIntersect(dd_manager, dinRdySelDD, rdySliceNode);
  Cudd_Ref(dinRdyIntersectNode);
  Cudd_RecursiveDerefZdd(dd_manager, rdySliceNode);

  return(dinRdyIntersectNode);
}

DdNode * adamant_zdd_BuildIterDinDinReverseSlice(DdManager * dd_manager,
						 DdNode * sliceNode, 
						 DdNode * local_dindinDD,
						 guint64 resDepth,
						 guint64 stopCount)
{
  struct timeval startSliceTime, currentSliceTime;
  gettimeofday(&startSliceTime,NULL); // DEBUG !!!
 
  guint64 count = 0;
  DdNode * newDinDinDD = NULL;

  // Do one iteration of the slice to avoid messing with the ref count of sliceNode 
  DdNode * dinDinSubSetDD = Cudd_zddDiff(dd_manager, local_dindinDD, sliceNode);
  Cudd_Ref(dinDinSubSetDD);

  DdNode * newSlice = adamant_zdd_DinDinReverseSlice (dd_manager, sliceNode, dinDinSubSetDD);

  // Accumulate the new dindin set off to the side
  if(0 != resDepth)
    {
      guint64 depthVal = 1 << resDepth;
      depthVal -= 1;

      DdNode * univDepthDD = adamant_zdd_build_fullcube(dd_manager, depthVal, depthVal);
      DdNode * tmpNewDinDinDD = Cudd_zddUnion( dd_manager, univDepthDD, sliceNode);
      Cudd_RecursiveDerefZdd(dd_manager, univDepthDD);
      Cudd_Ref(tmpNewDinDinDD);
      newDinDinDD = Cudd_zddUnion( dd_manager, tmpNewDinDinDD , newSlice);
      Cudd_Ref(newDinDinDD);
      Cudd_RecursiveDerefZdd(dd_manager, tmpNewDinDinDD);
    }
  else
    {
      newDinDinDD = Cudd_zddUnion( dd_manager, newSlice, sliceNode);
      Cudd_Ref(newDinDinDD);
    }

  gboolean sliceEqual = (gboolean)(newDinDinDD == sliceNode);

  while((sliceEqual == FALSE) && ((stopCount == 0) || (count < stopCount)))
    {
      // Create a new subset DinDin graph set
      DdNode * tmpDinDinSubSetDD = Cudd_zddDiff(dd_manager, dinDinSubSetDD, newSlice);
      Cudd_Ref(tmpDinDinSubSetDD);
      Cudd_RecursiveDerefZdd(dd_manager, dinDinSubSetDD);
      dinDinSubSetDD = tmpDinDinSubSetDD;

      DdNode * tmpSlice = adamant_zdd_DinDinReverseSlice (dd_manager, newSlice, dinDinSubSetDD);
      Cudd_RecursiveDerefZdd(dd_manager, newSlice);  
      newSlice = tmpSlice;

      // Union with the new slice {DIN, {DIN}} set
      DdNode * tmpDinDinDD = Cudd_zddUnion( dd_manager, newSlice, newDinDinDD);
      Cudd_Ref(tmpDinDinDD);   

      sliceEqual = (gboolean)(tmpDinDinDD == newDinDinDD);

      if(sliceEqual == FALSE)
	{
	  Cudd_RecursiveDerefZdd(dd_manager, newDinDinDD);
	}
      // else
      // 	{
      // 	  printf("slice diff is true\n");
      // 	  Cudd_CheckKeys(dd_manager);
      // 	}
      newDinDinDD = tmpDinDinDD;
      ++count;

      // //! DEBUG !
      // if ((count % 1000) == 0)
      //   {
      //     gettimeofday(&currentSliceTime,NULL); // DEBUG !!!

      //     double timeForSlice = (((currentSliceTime.tv_sec +
      //                  (currentSliceTime.tv_usec/1000000.0)) -
      //                 (startSliceTime.tv_sec +
      //                  (startSliceTime.tv_usec/1000000.0))));

      //     g_print("Time for %d slices:%f\n", count, timeForSlice);
      //  }
    }

  //! DEBUG !
  gettimeofday(&currentSliceTime,NULL); // DEBUG !!!
  
  double timeForSlice = (((currentSliceTime.tv_sec +
			   (currentSliceTime.tv_usec/1000000.0)) -
			  (startSliceTime.tv_sec +
			   (startSliceTime.tv_usec/1000000.0))));
  
  g_print("Slicing Complete. Time for %d slices:%f\n", count, timeForSlice);

  return(newDinDinDD);
}


DdNode * adamant_zdd_BuildIterDinDinForwardSlice(DdManager * dd_manager,
						 DdNode * sliceNode, 
						 DdNode * local_dindinDD, 
						 guint64 stopCount)
{
  struct timeval startSliceTime, currentSliceTime;
  gettimeofday(&startSliceTime,NULL); // DEBUG !!!
 
  guint64 count = 0;  

  // Do one iteration of the slice to avoid messing with the ref count of sliceNode 
  DdNode * dinDinSubSetDD = Cudd_zddDiff(dd_manager, local_dindinDD, sliceNode);
  Cudd_Ref(dinDinSubSetDD);

  DdNode * newSlice = adamant_zdd_DinDinForwardSlice (dd_manager, sliceNode, dinDinSubSetDD);

  // Accumulate the new dindin set off to the side
  DdNode * newDinDinDD = Cudd_zddUnion( dd_manager, newSlice, sliceNode);
  Cudd_Ref(newDinDinDD);

  gboolean sliceEqual = (gboolean)(newDinDinDD == sliceNode);

  while((sliceEqual == FALSE) && ((stopCount == 0) || (count < stopCount)))
    {
      // Create a new subset DinDin graph set
      DdNode * tmpDinDinSubSetDD = Cudd_zddDiff(dd_manager, dinDinSubSetDD, newSlice);
      Cudd_Ref(tmpDinDinSubSetDD);
      Cudd_RecursiveDerefZdd(dd_manager, dinDinSubSetDD);
      dinDinSubSetDD = tmpDinDinSubSetDD;

      DdNode * tmpSlice = adamant_zdd_DinDinForwardSlice (dd_manager, newSlice, dinDinSubSetDD);
      Cudd_RecursiveDerefZdd(dd_manager, newSlice);  
      newSlice = tmpSlice;

      // Union with the new slice {DIN, {DIN}} set
      DdNode * tmpDinDinDD = Cudd_zddUnion( dd_manager, newSlice, newDinDinDD);
      Cudd_Ref(tmpDinDinDD);   

      sliceEqual = (gboolean)(tmpDinDinDD == newDinDinDD);

      Cudd_RecursiveDerefZdd(dd_manager, newDinDinDD);
      newDinDinDD = tmpDinDinDD;
      ++count;

      //! DEBUG !
      if ((count % 10000) == 0)
        {
          gettimeofday(&currentSliceTime,NULL); // DEBUG !!!

          double timeForSlice = (((currentSliceTime.tv_sec +
                       (currentSliceTime.tv_usec/1000000.0)) -
                      (startSliceTime.tv_sec +
                       (startSliceTime.tv_usec/1000000.0))));

          g_print("Time for %d slices:%f\n", count, timeForSlice);
       }
    }

  //! DEBUG !
  gettimeofday(&currentSliceTime,NULL); // DEBUG !!!
  
  double timeForSlice = (((currentSliceTime.tv_sec +
			   (currentSliceTime.tv_usec/1000000.0)) -
			  (startSliceTime.tv_sec +
			   (startSliceTime.tv_usec/1000000.0))));
  
  g_print("Slicing Complete. Time for %d slices:%f\n", count, timeForSlice);

  return(newDinDinDD);
}


DdNode * adamant_zdd_DinDinReverseSlice(DdManager * dd_manager, DdNode * sliceDD, DdNode * targetDD)
{
  // Form a {blank, DIN} set
  DdNode * blankDinDD = adamant_zdd_abstractX(dd_manager, sliceDD);

  // Swap the variable positions to form a {DIN, blank} set
  DdNode * dinBlankDD = adamant_zdd_varswap(dd_manager, blankDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, blankDinDD);

  // Form a {DIN, univ} set
  DdNode * dinUnivDD = adamant_zdd_yDC(dd_manager, dinBlankDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinBlankDD);

  // Get a new {DIN, {DIN}} set
  DdNode * newDinDinDD = Cudd_zddIntersect(dd_manager, dinUnivDD, targetDD);
  Cudd_Ref(newDinDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnivDD);

  return (newDinDinDD);
}


DdNode * adamant_zdd_DinDinForwardSlice(DdManager * dd_manager,
					DdNode * sliceDD, 
					DdNode * targetDD)
{
 
  // Form a {DIN, blank} set
  DdNode * dinblnkDD = adamant_zdd_abstractY(dd_manager, sliceDD);

  // Swap the variable positions to form a {blank, DIN} set
  DdNode * swappedDinNode = adamant_zdd_varswap(dd_manager, dinblnkDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinblnkDD);

  // Form a {univ, DIN} set
  DdNode * univDinNode = adamant_zdd_xDC(dd_manager, swappedDinNode);
  Cudd_RecursiveDerefZdd(dd_manager, swappedDinNode);

  // Get a new {DIN, {DIN}} set
  DdNode * newDinDinDD = Cudd_zddIntersect(dd_manager, univDinNode, targetDD);
  Cudd_Ref(newDinDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, univDinNode);

  return (newDinDinDD);
}


/**Function********************************************************************

   Synopsis [Filters out code from a select region]

   Description [Filters out code from a select region that does not contribute
   to the final graph results]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * adamant_zdd_DeadSlicer(DdManager * dd_manager, 
				DdNode * dd_dindin_sel,
				DdNode * dd_din_add_back,
				int iterations)
{
  guint64 xTop = 0;
  guint64 yTop = 0;
  int count = 0;
  DdNode * dindinAddBackDD = NULL;

  if ((dd_manager == NULL) || (dd_dindin_sel == NULL))
    {
      return (NULL);
    }
  
  DdNode * dd_dindin_sel_old = NULL;

  // Construct an add back set
  if(NULL != dd_din_add_back)
    {
      DdNode * dinUnvAbDD = adamant_zdd_yDC(dd_manager, dd_din_add_back);
      Cudd_RecursiveDerefZdd(dd_manager, dd_din_add_back);
      dindinAddBackDD = Cudd_zddIntersect(dd_manager, dd_dindin_sel, dinUnvAbDD);
      Cudd_Ref(dindinAddBackDD);
      Cudd_RecursiveDerefZdd(dd_manager, dinUnvAbDD);
    }

  while(((iterations > count) || (0 == iterations))
	&& (dd_dindin_sel_old != dd_dindin_sel))
    {
      // Set up to monitor for convergence
      dd_dindin_sel_old = dd_dindin_sel;

        // Create a {empty,DIN_d} set from our selection
      DdNode * emptyDinSelD = adamant_zdd_abstractX(dd_manager, dd_dindin_sel);

      // Swap the Vars {Din_d_i, empty} 
      DdNode * dinEmptySelD = adamant_zdd_varswap(dd_manager, emptyDinSelD);
      Cudd_RecursiveDerefZdd(dd_manager, emptyDinSelD);
      
      // Create a {DIN, univ} set from our rev {DIN, empty} set
      DdNode * dinUnivSelDD = adamant_zdd_yDC(dd_manager, dinEmptySelD);
      Cudd_RecursiveDerefZdd(dd_manager, dinEmptySelD);

      // Find out which instructions are not in the dependence set
      DdNode * dindinSelDD = Cudd_zddIntersect(dd_manager, dd_dindin_sel, dinUnivSelDD);
      Cudd_Ref(dindinSelDD);
      Cudd_RecursiveDerefZdd(dd_manager, dinUnivSelDD);

      // Add back any dins that should not be removed
      if(NULL != dindinAddBackDD)
	{
	  DdNode * tmpAddBackDD = Cudd_zddUnion(dd_manager,  
						dindinAddBackDD, 
						dindinSelDD);
	  Cudd_Ref(tmpAddBackDD);
	  Cudd_RecursiveDerefZdd(dd_manager, dindinSelDD);
	  dindinSelDD = tmpAddBackDD;
	}

      // Report the number of dindin values removed
      DdNode * dindinDiff = Cudd_zddDiff(dd_manager, dd_dindin_sel, dindinSelDD);
      Cudd_Ref(dindinDiff);
      
      g_print ("DdDiff:%u\n", 
	       Cudd_zddCount(dd_manager, dindinDiff));
     
      Cudd_RecursiveDerefZdd(dd_manager, dindinDiff);
      
      Cudd_RecursiveDerefZdd(dd_manager, dd_dindin_sel);

      // Setup the selection
      dd_dindin_sel = dindinSelDD;

      ++count;
    }
  if(NULL != dindinAddBackDD)
    {
      Cudd_RecursiveDerefZdd(dd_manager, dindinAddBackDD);
    }

  return(dd_dindin_sel);
}



/*
  Function: 

  Called with: 
  
  Returns: return code

  Side effects: none
 */
void adamant_zdd_addRegions(const char * fileName)
{
  gchar * line  = NULL;
  GIOChannel * fc = g_io_channel_new_file (fileName, "r",  NULL);
  ddpoint top_left;
  ddpoint bottom_right;
  gint64 region_id = -1;

  if(NULL == fc)
    {
      g_vfprintf (stderr, 
		  "Unable to read regions from file: %s\n",
		  fileName);
      return;
    }

  if (NULL == ddRegions)
    {
       ddRegions = g_queue_new ();
    }

  while (G_IO_STATUS_EOF != g_io_channel_read_line (fc, &line, NULL, NULL, NULL))
    {
      gchar * upLine = g_ascii_strup(line, -1);
      g_free(line);
      line = NULL;

      if(NULL != g_strrstr(upLine, "VERSION"))
	{

	}
      else if(NULL != g_strrstr(upLine, "REGSTART"))
	{
	  gchar ** lineParts = g_strsplit(upLine, ":", 2);
	  
	  if(NULL != lineParts[1])
	    {
	      lineParts[1] = g_strstrip(lineParts[1]);
	      region_id = g_ascii_strtoll(lineParts[1], NULL, 10);
	    }
	  
	  top_left.X = 0;
	  top_left.Y = 0;
	  bottom_right.X = 0;
	  bottom_right.Y = 0;
	}
      else if(NULL != g_strrstr(upLine, "REGEND"))
	{
	  ddregion * newRegion = g_new0(ddregion, 1);

	  newRegion->topLeft = top_left;
	  newRegion->bottomRight = bottom_right;
	  newRegion->regionID = region_id;
	  newRegion->parallelCount = 0;
	  g_queue_push_head(ddRegions, newRegion);
	  
	  top_left.X = 0;
	  top_left.Y = 0;
	  bottom_right.X = 0;
	  bottom_right.Y = 0;
	  region_id = -1;
	}
      else if(NULL != g_strrstr(upLine, "TL"))
	{
	  gchar ** lineParts = g_strsplit(upLine, " ", 3);

	  if((NULL != lineParts[1]) &&
	     (NULL != lineParts[2]))
	    {
	      top_left.X = g_ascii_strtoull(lineParts[1], NULL, 10); 
	      top_left.Y = g_ascii_strtoull(lineParts[2], NULL, 10); 
	    }
	  g_strfreev(lineParts);
	}
      else if(NULL != g_strrstr(upLine, "BR"))
	{
	  gchar ** lineParts = g_strsplit(upLine, " ", 3);
	  
	  if((NULL != lineParts[1]) &&
	     (NULL != lineParts[2]))
	    {
	      bottom_right.X = g_ascii_strtoull(lineParts[1], NULL, 10); 
	      bottom_right.Y = g_ascii_strtoull(lineParts[2], NULL, 10); 
	    }
	  g_strfreev(lineParts);
	}

      g_free(upLine);
    }

  // QPointF top_left;
  // QPointF bottom_right;
  // fprintf(fh, "TL: %f %f\n",
  // 	  top_left.x(),
  // 	  top_left.y());
  // fprintf(fh, "BR: %f %f\n",
  // 	  bottom_right.x(),
  // 	  bottom_right.y());
  g_io_channel_shutdown (fc, TRUE, NULL);
}

void adamant_zdd_DdRegionsSeperate(DdManager * manager, DdNode * dindinDD, DdNode ** regionA, DdNode ** regionB)
{
  DdNode * regionsIntersect = Cudd_zddIntersect(manager, *regionA, *regionB);
  Cudd_Ref(regionsIntersect);

  // return if regions are seperate
  if(Cudd_ReadZero(manager) == regionsIntersect)
    {
      Cudd_RecursiveDerefZdd(manager, regionsIntersect);
      return;
    }

  DdNode * regionAwoOverlap = Cudd_zddDiff(manager, *regionA, regionsIntersect);
  Cudd_Ref(regionAwoOverlap);

  DdNode * regionBwoOverlap = Cudd_zddDiff(manager, *regionB, regionsIntersect);
  Cudd_Ref(regionBwoOverlap); 

  // Forward Slice Tests
  DdNode * dinEmptyOvlp = adamant_zdd_abstractY(manager, regionsIntersect);

  DdNode * emptyDinOvlp = adamant_zdd_varswap(manager, dinEmptyOvlp);

  DdNode * dinUnvOvlp = adamant_zdd_yDC(manager, dinEmptyOvlp);
  Cudd_RecursiveDerefZdd(manager, dinEmptyOvlp);

  DdNode * unvDinOvlp = adamant_zdd_xDC(manager, emptyDinOvlp);
  Cudd_RecursiveDerefZdd(manager, emptyDinOvlp);
  
  DdNode * dinEmptyA = adamant_zdd_abstractY(manager, regionAwoOverlap);
  DdNode * dinUnvA = adamant_zdd_yDC(manager, dinEmptyA);
  Cudd_RecursiveDerefZdd(manager, dinEmptyA);

  DdNode * dinEmptyB = adamant_zdd_abstractY(manager, regionBwoOverlap);
  DdNode * dinUnvB = adamant_zdd_yDC(manager, dinEmptyB);
  Cudd_RecursiveDerefZdd(manager, dinEmptyB);  

  DdNode * dinDinARev = Cudd_zddIntersect(manager, dindinDD, dinUnvA);
  Cudd_Ref(dinDinARev);
  DdNode * dinDinARevOvlp = Cudd_zddIntersect(manager, dinDinARev, unvDinOvlp);
  Cudd_Ref(dinDinARevOvlp);
  Cudd_RecursiveDerefZdd(manager, dinDinARev);

  DdNode * dinDinBRev = Cudd_zddIntersect(manager, dindinDD, dinUnvB);
  Cudd_Ref(dinDinBRev);
  DdNode * dinDinBRevOvlp = Cudd_zddIntersect(manager, dinDinBRev, unvDinOvlp);
  Cudd_Ref(dinDinARevOvlp);
  Cudd_RecursiveDerefZdd(manager, dinDinARev);

}

DdNode * adamant_zdd_DdRegion(DdManager * manager, DdNode * ddnode, ddregion * region)
{
  guint64 zddNumHalf = ZDDNUM/2;
  int xlist [ZDDNUM/2];
  int ylist [ZDDNUM/2];
    
  //! gather a list of ZDD node variables
  unsigned int i = 0;
  for (i = 0; i < zddNumHalf; i+=1)
    {
      xlist[(zddNumHalf - 1) - i] = i;
      ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
    }

  //! find the ZDD for the selected area X
  DdNode * xUb =  mg_Cudd_zddUb(manager, zddNumHalf, xlist, (uint64_t)region->bottomRight.X);
  Cudd_Ref(xUb);
  DdNode * xLb =  mg_Cudd_zddLb(manager, zddNumHalf, xlist, (uint64_t)region->topLeft.X);
  Cudd_Ref(xLb);
  
  // START DEBUG
  //  g_print("Bottom X:%u Y:%u, Top X:%u Y:%u\n", region->topLeft.X, region->bottomRight.Y, region->bottomRight.X, region->topLeft.Y);
  // END DEBUG  

  //! find the ZDD for the selected area Y
  DdNode * yUb =  mg_Cudd_zddUb(manager, zddNumHalf, ylist, (uint64_t)region->topLeft.Y);
  Cudd_Ref(yUb);
  DdNode * yLb =  mg_Cudd_zddLb(manager, zddNumHalf, ylist, (uint64_t)region->bottomRight.Y);
  Cudd_Ref(yLb);

  DdNode * uB = Cudd_zddUnateProduct(manager, xUb, yUb);
  Cudd_Ref(uB);
  Cudd_RecursiveDerefZdd(manager, xUb);
  Cudd_RecursiveDerefZdd(manager, yUb);
  
  DdNode * lB = Cudd_zddUnateProduct(manager, xLb, yLb);
  Cudd_Ref(lB);
  Cudd_RecursiveDerefZdd(manager, xLb);
  Cudd_RecursiveDerefZdd(manager, yLb);

  //! now perform the AND
  DdNode * regionDD = Cudd_zddIntersect(manager, uB, lB);
  Cudd_Ref(regionDD);
  Cudd_RecursiveDerefZdd(manager, lB);
  Cudd_RecursiveDerefZdd(manager, uB);

  DdNode * returnDD = Cudd_zddIntersect(manager, regionDD, ddnode); 
  Cudd_Ref(returnDD);

  Cudd_RecursiveDerefZdd(manager, regionDD);

  return (returnDD);
}

ddregion * adamant_zdd_regionsOverlap(ddregion * regionA, ddregion * regionB)
{
  ddregion * newRegion = g_new0(ddregion, 1);

  if(NULL != newRegion)
    { 
      ddpoint topL;
      ddpoint bottomR;

      // Find overlap on X axis
      if(regionA->topLeft.X > regionB->topLeft.X)
	{
	  topL.X = regionA->topLeft.X;
	}
      else
	{
	  topL.X = regionB->topLeft.X;	  
	}

      if(regionA->bottomRight.X < regionB->bottomRight.X)
	{
	  bottomR.X = regionA->bottomRight.X;
	}
      else
	{
	  bottomR.X = regionB->bottomRight.X;	  
	}

      if(topL.X > bottomR.X)
	{
	  bottomR.X = 0;
	  topL.X = 0;
	}

      // Find overlap on Y axis
      if(regionA->topLeft.Y > regionB->topLeft.Y)
	{
	  topL.Y = regionA->topLeft.Y;
	}
      else
	{
	  topL.Y = regionB->topLeft.Y;	  
	}

      if(regionA->bottomRight.Y < regionB->bottomRight.Y)
	{
	  bottomR.Y = regionA->bottomRight.Y;
	}
      else
	{
	  bottomR.Y = regionB->bottomRight.Y;	  
	}

      if(topL.Y < bottomR.Y)
	{
	  bottomR.Y = 0;
	  topL.Y = 0;
	}
      
      newRegion->topLeft = topL;
      newRegion->bottomRight = bottomR;
    }

  return(newRegion);
}

void adamant_zdd_DinSinHot(DdManager * manager, DdNode * ddnode, 
			   DdNode * dinHotDD, guint64 din, guint64 sin)
{
  //! check to see if this node is a constant
  if(Cudd_IsConstant(ddnode))
    {
      if(ddnode != Cudd_ReadZero(manager))
	{
	  g_print("%"G_GUINT64_FORMAT",%"G_GUINT64_FORMAT"\n",
		  din,sin);
	  //	  adamant_zdd_DinHot(manager, dinHotDD, 
	  //			     din, sin, 0);
	}
      return;
    }
  
  int varIndex = Cudd_NodeReadIndex(ddnode);

  guint64 thenSin = sin;
  guint64 thenDin = din;
  if(64 <= varIndex)
    {
      thenSin = thenSin | mask((guint64)(varIndex - 64));
    }
  else
    {
      thenDin = thenDin | mask((guint64)(varIndex));
    }
  
  //! Handle the "then" branch
  adamant_zdd_DinSinHot(manager, Cudd_T(ddnode), dinHotDD, thenDin, thenSin);

  //! Handle the "else" branch
  adamant_zdd_DinSinHot(manager, Cudd_E(ddnode), dinHotDD, din, sin);
}

void adamant_zdd_SinPrint(DdManager * manager, DdNode * ddnode, 
			  guint64 din, guint64 sin)
{
  //! check to see if this node is a constant
  if(Cudd_IsConstant(ddnode))
    {
      if(ddnode != Cudd_ReadZero(manager))
	{
	  g_print("0x%x\n", sin);
	}
      return;
    }
  
  int varIndex = Cudd_NodeReadIndex(ddnode);

  guint64 thenSin = sin;
  guint64 thenDin = din;
  if(64 <= varIndex)
    {
      thenSin = thenSin | mask((guint64)(varIndex - 64));
    }
  else
    {
      thenDin = thenDin | mask((guint64)(varIndex));
    }
  
  //! Handle the "then" branch
  adamant_zdd_SinPrint(manager, Cudd_T(ddnode), thenDin, thenSin);

  //! Handle the "else" branch
  adamant_zdd_SinPrint(manager, Cudd_E(ddnode), din, sin);
}


void adamant_zdd_DinHot(DdManager * manager, DdNode * dinHotDD, 
			guint64 din, guint64 sin, guint64 hot)
{
  //! check to see if this node is a constant
  if(Cudd_IsConstant(dinHotDD))
    {
      if(dinHotDD != Cudd_ReadZero(manager))
	{
	  g_print("%"G_GUINT64_FORMAT",%"G_GUINT64_FORMAT",%"G_GUINT64_FORMAT"\n",
		  din,sin,hot);
	}
      return;
    }
  
  int varIndex = Cudd_NodeReadIndex(dinHotDD);
  if(64 <= varIndex)
    {
      guint64 thenHot = hot | mask((guint64)(varIndex - 64));

      //! Handle the "then" branch
      adamant_zdd_DinHot(manager, Cudd_T(dinHotDD), din, sin, thenHot);
      
      //! Handle the "else" branch
      adamant_zdd_DinHot(manager, Cudd_E(dinHotDD), din, sin, hot);
    }
  else
    {
      if(din & mask((guint64)(varIndex)))
	{
	  //! Handle the "then" branch
	  adamant_zdd_DinHot(manager, Cudd_T(dinHotDD), din, sin, hot);
	}
      else
	{
	  //! Handle the "else" branch
	  adamant_zdd_DinHot(manager, Cudd_E(dinHotDD), din, sin, hot);
	}
    }
}
