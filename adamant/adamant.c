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
#include <stdlib.h>

#include "adamant.h"
#include "adamant_config.h"
#include "adamant_zdd_test.h"
#include "adamant_runtime.h"
#include "adamant_schedule.h"
#include "adamant_zdd.h"

#undef ZDDUNITTESTS

gint main(gint argc, gchar *argv[])
{

#ifdef ZDDUNITTESTS
  adamant_zddtest_deathSliceTest();
  //  adamant_zddtest_interval();
  //  adamant_zddtest_sliceiter();
  //  adamant_zddtest_zddslice();
  //  adamant_zddtest_zddstore();
  //  adamant_zddtest_varswap();
  return EXIT_SUCCESS;
#endif

  AdamantRuntime *runtime;

  runtime = adamant_new_runtime();
  adamant_process_options(argc, argv, runtime->config);

  // BEGIN DEBUG
  if(runtime->config->dd_slice != NULL)
    {
      int count = 0;
      while (NULL != runtime->config->dd_slice->str[count])
	{
	  if(('\t' == runtime->config->dd_slice->str[count]) ||
	     ('\n' == runtime->config->dd_slice->str[count]) ||
	     ('"' == runtime->config->dd_slice->str[count]))
	    {
	      runtime->config->dd_slice->str[count] = ' ';	      
	    }
	  ++count;
	}
      
      
      g_strstrip(runtime->config->dd_slice->str);
      gchar ** fileVec = g_strsplit(runtime->config->dd_slice->str,
				    ":" , 2);
      if(NULL != fileVec[1])
	{
	  GString * inputFiles = g_string_new(fileVec[1]);
	  
	  if (NULL != g_strrstr(fileVec[0], "perf"))
	    {
	      g_print("Performance Tests\n");
	      adamant_zddtest_performance(runtime->config->dd_slots_multi, 
					  inputFiles);
	    }
	  else if (NULL != g_strrstr(fileVec[0], "optimalPerf"))
	    {
	      g_print("Optimal Performance Tests\n");
	      adamant_zddtest_naivePerformance(runtime->config->dd_slots_multi, 
					       inputFiles);
	    }
	  else if (NULL != g_strrstr(fileVec[0], "death"))
	    {
	      g_print("Dead Code Iter Test\n");
	      adamant_zddtest_deathSlice(runtime->config->dd_slots_multi, 
					 inputFiles);
	    }
	  else if (NULL != g_strrstr(fileVec[0], "sinInfo"))
	    {
	      g_print("Printing SIN Information\n");
	      adamant_zddtest_sinStats(runtime->config->dd_slots_multi,
				       inputFiles);
	    }
	  else if (NULL != g_strrstr(fileVec[0], "sinRegion"))
	    {
	      adamant_zddtest_sinRegion(runtime->config->dd_slots_multi,
				       inputFiles);
	    }
	  else if (NULL != g_strrstr(fileVec[0], "deadCodeRemove"))
	    {
	      adamant_zddtest_deadCodeRemoval(runtime->config->dd_slots_multi,
					      inputFiles);
	    }
	  //    adamant_zddtest_dinsinslice(runtime->config->dd_slice);
	  //      adamant_zddtest_rdydinslice(runtime->config->dd_slice);
	  //      adamant_zddtest_quickdead(runtime->config->dd_slice);
	  //      adamant_zddtest_tophot(runtime->config->dd_slots_multi, 
	  //			     runtime->config->dd_slice);
	  //    adamant_zddtest_reverseSliceMethodTest(runtime->config->dd_slots_multi,
	  //						 runtime->config->dd_slice);

	  g_string_free(inputFiles, TRUE);
	}
      else
	{
	  adamant_zddtest_deathSlice(runtime->config->dd_slots_multi, 
				     runtime->config->dd_slice);
	}
      g_strfreev(fileVec);
    }
  else
    {
      // END DEBUG
      
      adamant_print_config(runtime->config);
      adamant_initialize_runtime(runtime);
      adamant_schedule_program(runtime);
      adamant_free_runtime(runtime);
    }

  return EXIT_SUCCESS;
}

gint CompareUINT64(gconstpointer a, gconstpointer b)
{
  const guint64 x = *(const guint64 *)a;
  const guint64 y = *(const guint64 *)b;
  //return ((x<y)? -1 : ((x==y)? 0 : 1));
  return (x-y)>>32 | ((x-y) & ~(0x80000000));
}

gboolean
EqualUINT64(gconstpointer v1, gconstpointer v2)
{
  return *((const guint64*) v1) == *((const guint64*) v2);
}

