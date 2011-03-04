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
  CAUSED AND ON ANY THEORY OF LIABILITY, HETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  #*ENDLICENCE*#*/


#include "adamant_zdd_test.h"
#include "adamant_dd_hot.h"
#include "adamant_zdd.h"
#include "adamant.h"
#include "adamant_branch_prediction.h"
#include "adamant_plot.h"
#include "adamant_window.h"
#include "extra.h"

#include <gmp.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <cudd.h>
#include <cuddInt.h>

const guint32 rdyMinus = 100;
extern GQueue * ddRegions;

int adamant_zddtest_deadCodeRemoval(guint64 slotsMulti, GString * inputs)
{  
  DdNode * dinrdyDD = NULL;
  DdNode * dinUnivDD = NULL;
  int zdd_ordr[ZDDNUM];
  
  DdManager * manager = adamant_zddtest_initmanagerSlots(zdd_ordr, slotsMulti, 0);
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
  headerInfo.extraTraceInfo = g_new(char, 512);
  gchar ** fileVec = g_strsplit(inputs->str, " " , 5);
  gchar * overlayDDFileName = NULL;
  int i = 0;

  while (NULL != fileVec[i])
    {      
      if ((NULL != g_strrstr(fileVec[i], ".dd.working")))
	{
	  g_print("Using Dead DINxDIN from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  DdNode * dindinDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  Cudd_Ref(dindinDD);
	  fclose(fh);
	
	  DdNode * blankDinDD =  adamant_zdd_abstractX(manager, dindinDD);
	  DdNode * dinBlankXDD = adamant_zdd_varswap(manager, blankDinDD);
	  Cudd_RecursiveDerefZdd(manager, blankDinDD);
	  DdNode * dinBlankYDD = adamant_zdd_abstractY(manager, dindinDD);
	  Cudd_RecursiveDerefZdd(manager, dindinDD);
	  DdNode * dinBlankDD = Cudd_zddUnion(manager, dinBlankYDD, dinBlankXDD);
	  Cudd_Ref(dinBlankDD);
	  Cudd_RecursiveDerefZdd(manager, dinBlankYDD);
	  Cudd_RecursiveDerefZdd(manager, dinBlankXDD);
	  dinUnivDD = adamant_zdd_yDC(manager, dinBlankDD);
	  Cudd_RecursiveDerefZdd(manager, dinBlankDD);
	}
      else if (NULL != g_strrstr(fileVec[i], "output"))
	{
	  g_print("Saving new DINxRDY DD to file:%s\n", fileVec[i]);
	  overlayDDFileName = g_strdup(fileVec[i]);	  
	}
      else if (NULL != g_strrstr(fileVec[i], "dinvsrdy")||
	       (NULL != g_strrstr(fileVec[i], "dinrdy")))
	{
	  g_print("Using {DIN,RDY} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  dinrdyDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  Cudd_Ref(dinrdyDD);
	  fclose(fh);	  
	}
     
      ++i;
    }
  g_strfreev(fileVec);

  if((NULL == dinUnivDD) ||
     (NULL == dinrdyDD))
    {
      return(1);
    }

  DdNode * newDinRdyDD = Cudd_zddIntersect(manager, dinUnivDD, dinrdyDD);
  Cudd_Ref(newDinRdyDD);
  Cudd_RecursiveDerefZdd(manager, dinrdyDD);
  Cudd_RecursiveDerefZdd(manager, dinUnivDD);  

  if(NULL != overlayDDFileName)
    {
      g_print("Saving to file:%s\n", overlayDDFileName);
      
      DdNode * finalDD = newDinRdyDD;
      
      FILE * outFh = fopen(overlayDDFileName, "w+");
      //! add extra information to this ZDD dump
      
      guint64 xTop = 0;
      guint64 yTop = 0;
      guint64 xBottom = 0;
      guint64 yBottom = 0;
      
      adamant_zdd_GetTupleTop(manager, finalDD, &xTop, &yTop);
      adamant_zdd_GetTupleBottom(manager, finalDD, &xBottom, &yBottom);    
      
      headerInfo.extraTraceInfo = g_strdup_printf("type:dinrdy,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT, 
						  xBottom,
						  xTop);
      
      Dddmp_cuddZddStore(manager, NULL,
			 finalDD, NULL, &zdd_ordr,
			 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			 overlayDDFileName, outFh);	     
      fclose(outFh);
      g_free(headerInfo.extraTraceInfo);
    }
}

int adamant_zddtest_ddchop(guint64 slotsMulti, GString * inputs)
{
  DdNode * dinsysDD = NULL;
  DdNode * dinrdyDD = NULL;
  DdNode * dindinDD = NULL;
  DdNode * dinhotDD = NULL;
  gchar * regionFileName = NULL;
  gchar * overlayDDFileName = NULL;
  guint64 topHotPercent = 0;
  int zdd_ordr[ZDDNUM];
  
  DdManager * manager = adamant_zddtest_initmanagerSlots(zdd_ordr, slotsMulti, 0);
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
  headerInfo.extraTraceInfo = g_new(char, 512);
  gchar ** fileVec = g_strsplit(inputs->str, " " , 5);


  g_strfreev(fileVec);
}

int adamant_zddtest_performance(guint64 slotsMulti, GString * inputs)
{
  DdNode * dinsysDD = NULL;
  DdNode * dinrdyDD = NULL;
  DdNode * dindinDD = NULL;
  DdNode * dinhotDD = NULL;
  gchar * regionFileName = NULL;
  gchar * overlayDDFileName = NULL;
  guint64 topHotPercent = 0;
  int zdd_ordr[ZDDNUM];
  
  DdManager * manager = adamant_zddtest_initmanagerSlots(zdd_ordr, slotsMulti, 0);
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
  headerInfo.extraTraceInfo = g_new(char, 512);
  gchar ** fileVec = g_strsplit(inputs->str, " " , 10);
  guint64 * naiveRegions = NULL;
  guint64 * interferRegions = NULL;
  int i = 0;
  while (NULL != fileVec[i])
    {      
      if ((NULL != g_strrstr(fileVec[i], ".slr")))
	{
	  g_print("Using regions from file:%s\n", fileVec[i]);
	  regionFileName = g_strdup(fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  
	  adamant_zdd_addRegions(regionFileName);
	  naiveRegions = g_new0(guint64, g_queue_get_length(ddRegions));
	  interferRegions = g_new0(guint64, g_queue_get_length(ddRegions));
	}
      else if (NULL != g_strrstr(fileVec[i], "output"))
	{
	  g_print("Saving parallel overlay DD to file:%s\n", fileVec[i]);
	  overlayDDFileName = g_strdup(fileVec[i]);	  
	}
      else if (NULL != g_strrstr(fileVec[i], "dinvssys"))
	{
	  g_print("Using {DIN,SYS} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  dinsysDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  Cudd_Ref(dinsysDD);
	  fclose(fh);
	}      
      else if ((NULL != g_strrstr(fileVec[i], "dindin")) || 
	       (NULL != g_strrstr(fileVec[i], "dinvsdin")))
	{
	  g_print("Using {DIN,DIN} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  dindinDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  Cudd_Ref(dindinDD);
	  fclose(fh);	  
	}
      else if (NULL != g_strrstr(fileVec[i], "dinvsrdy")||
	       (NULL != g_strrstr(fileVec[i], "dinrdy")))
	{
	  g_print("Using {DIN,RDY} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  dinrdyDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  Cudd_Ref(dinrdyDD);
	  fclose(fh);	  
	}
     
      ++i;
    }
  g_strfreev(fileVec);

  if((NULL == dindinDD) ||
     (NULL == dinrdyDD))
    {
      return(1);
    }

  // START DEBUG
  //  g_print("Ref Check 1\n");
  //  Cudd_CheckKeys(manager);
  // END DEBUG

  GQueue * doneRegions = g_queue_new ();

  // Variables needed for upper and lower bounds
  guint64 zddNumHalf = ZDDNUM/2;
  int xlist [ZDDNUM/2];
  int ylist [ZDDNUM/2];      
  //! gather a list of ZDD node variables
  for (i = 0; i < zddNumHalf; i+=1)
    {
      xlist[(zddNumHalf - 1) - i] = i;
      ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
    }

  //! performance tracking variables
  guint64 executionSize = Cudd_zddCount(manager, dinrdyDD);
  guint64 naiveSize = 0;
  guint64 simpleInterferSize = 0;
  DdNode * dinRdyOptimalDD = Cudd_ReadZero(manager);
  DdNode * dinRdySimpleInterferDD = Cudd_ReadZero(manager);

  while (!g_queue_is_empty (ddRegions))
    {
      ddregion * regionA =  g_queue_pop_head(ddRegions);
      
      guint queueCount = g_queue_get_length(ddRegions);

      while (0 < queueCount)
	{
	  ddregion * regionB = g_queue_pop_head(ddRegions);

	  // START DEBUG
	  //	  g_print("regionA TL X:%u Y:%u, BR X:%u Y:%u\n", 
	  //		  regionA->topLeft.X, regionA->topLeft.Y, 
	  //		  regionA->bottomRight.X, regionA->bottomRight.Y);	  
	  // END DEBUG

	  /***********************************************************/
	  /* Find optimal performance gain */
	  /***********************************************************/

	  ddregion * overlap = adamant_zdd_regionsOverlap(regionA, regionB);

	  // START DEBUG
	  g_print("\nRegions:%u->%u,Overlap:TL X:%u Y:%u, BR X:%u Y:%u\n", 
		  regionA->regionID, regionB->regionID, 
		  overlap->topLeft.X, overlap->topLeft.Y, 
		  overlap->bottomRight.X, overlap->bottomRight.Y);
	  // END DEBUG
	  
	  if(!((0 == overlap->topLeft.X) &&
	       (0 == overlap->bottomRight.X)))
	    {
	      // create a regionA shrunk by the x overlap
	      ddregion * xShrunkA = g_new0(ddregion, 1);
	      xShrunkA->bottomRight.Y = overlap->topLeft.X;
	      xShrunkA->bottomRight.X = regionA->topLeft.Y;
	      xShrunkA->topLeft.Y = overlap->bottomRight.X;
	      xShrunkA->topLeft.X = regionA->bottomRight.Y;
	      xShrunkA->regionID = regionA->regionID;

	      // create a regionB shrunk by the x overlap
	      ddregion * xShrunkB = g_new0(ddregion, 1);
	      xShrunkB->bottomRight.Y = overlap->topLeft.X;
	      xShrunkB->bottomRight.X = regionB->topLeft.Y;
	      xShrunkB->topLeft.Y = overlap->bottomRight.X;
	      xShrunkB->topLeft.X = regionB->bottomRight.Y;
	      xShrunkB->regionID = regionB->regionID;

	      // Get the DD for the region A
	      DdNode * tmpRegADD = adamant_zdd_DdRegion(manager, dinrdyDD, xShrunkA);
	      DdNode * regADD = Cudd_zddDiff(manager, tmpRegADD, dinRdyOptimalDD);
	      Cudd_Ref(regADD);
	      
	      guint64 regionCountA = Cudd_zddCount(manager, regADD);
 
	      // Get the DD for the region B
	      DdNode * tmpRegBDD = adamant_zdd_DdRegion(manager, dinrdyDD, xShrunkB);
	      DdNode * regBDD = Cudd_zddDiff(manager, tmpRegBDD, dinRdyOptimalDD);
	      Cudd_Ref(regBDD);

	      guint64 regionCountB = Cudd_zddCount(manager, regBDD);

	      guint64 localNaiveSize = 0;
	      DdNode * localNaiveDD = NULL;

	      if(regionCountB < regionCountA)
		{
		  localNaiveDD = Cudd_zddDiff(manager, regBDD, regADD);
		  Cudd_Ref(localNaiveDD);
		}
	      else
		{
		  localNaiveDD = Cudd_zddDiff(manager, regADD, regBDD);
		  Cudd_Ref(localNaiveDD);
		}

	      localNaiveSize = Cudd_zddCount(manager, localNaiveDD);
	      naiveSize += localNaiveSize;
	  		
	      if(0 < localNaiveSize)
		{
		  naiveRegions[xShrunkA->regionID] = naiveRegions[xShrunkA->regionID] + 1;
		  naiveRegions[xShrunkB->regionID] = naiveRegions[xShrunkB->regionID] + 1;

		  DdNode * tempDinRdy = Cudd_zddUnion(manager, dinRdyOptimalDD, localNaiveDD);
		  Cudd_Ref(tempDinRdy);
		  Cudd_RecursiveDerefZdd(manager, dinRdyOptimalDD);
		  Cudd_RecursiveDerefZdd(manager, localNaiveDD);
		  dinRdyOptimalDD = tempDinRdy;
		}	
	      Cudd_RecursiveDerefZdd(manager, regADD);
	      Cudd_RecursiveDerefZdd(manager, regBDD);

	      // START DEBUG
	      g_print("Shrunk Regions:%"G_GINT64_FORMAT"->%"G_GINT64_FORMAT",Naive Size:%u\n", 
		      xShrunkA->regionID,
		      xShrunkB->regionID,
		      localNaiveSize);
	      g_print("\n");
	      // END DEBUG

	      /***********************************************************/
	      /* Find simple dependence interference */
	      /***********************************************************/

	      // Get the DD for the region A in our Simple Interfer DINxRDY DD		  
	      regADD = Cudd_zddDiff(manager, tmpRegADD, dinRdySimpleInterferDD);
	      Cudd_Ref(regADD);
	      Cudd_RecursiveDerefZdd(manager, tmpRegADD);

	      // Get the DD for the region B
	      regBDD = Cudd_zddDiff(manager, tmpRegBDD, dinRdySimpleInterferDD);
	      Cudd_Ref(regBDD);
	      Cudd_RecursiveDerefZdd(manager, tmpRegBDD);
 
	      guint64 aBottomDIN = 0;
	      guint64 aBottomRDY = 0;
	      adamant_zdd_GetTupleBottom(manager, regADD,
					 &aBottomDIN, &aBottomRDY);

	      guint64 bBottomDIN = 0;
	      guint64 bBottomRDY = 0;
	      adamant_zdd_GetTupleBottom(manager, regBDD,
					 &bBottomDIN, &bBottomRDY);	      		

	      DdNode * forwardDinRdyDD = NULL;
	      gint64 forwardDinRdyID = -1;
	      DdNode * reverseDinRdyDD = NULL;
	      gint64 reverseDinRdyID = -1;
	      
	      if(aBottomDIN < bBottomDIN)
		{
		  forwardDinRdyDD = regADD;
		  forwardDinRdyID = xShrunkA->regionID;
		  reverseDinRdyDD = regBDD;
		  reverseDinRdyID = xShrunkB->regionID;
		}
	      else
		{
		  forwardDinRdyDD = regBDD;
		  forwardDinRdyID = xShrunkB->regionID;
		  reverseDinRdyDD = regADD;
		  reverseDinRdyID = xShrunkA->regionID;
		}

	      // Create an upper bound of our {DIN,RDY}	      
	      DdNode * rdyUb = mg_Cudd_zddUb(manager, zddNumHalf, ylist, 
					     (uint64_t)overlap->bottomRight.X);
	      Cudd_Ref(rdyUb);

	      DdNode * rdyUbUnvDD = adamant_zdd_xDC(manager, rdyUb);
	      Cudd_RecursiveDerefZdd(manager, rdyUb);

	      DdNode * dinRdyUbDD = Cudd_zddIntersect(manager, rdyUbUnvDD, dinrdyDD);
	      Cudd_Ref(dinRdyUbDD);	      
	      Cudd_RecursiveDerefZdd(manager, rdyUbUnvDD);

	      DdNode * dinBlankUbDD = adamant_zdd_abstractY(manager, dinRdyUbDD);
	      Cudd_RecursiveDerefZdd(manager, dinRdyUbDD);

	      DdNode * dinUnvUbDD = adamant_zdd_yDC(manager, dinBlankUbDD);
	      Cudd_RecursiveDerefZdd(manager, dinBlankUbDD);

	      DdNode * dindinUbDD = Cudd_zddIntersect(manager, dindinDD, dinUnvUbDD);
	      Cudd_Ref(dindinUbDD);
	      Cudd_RecursiveDerefZdd(manager, dinUnvUbDD);
	      
	      DdNode * forwardDinBlankDD = adamant_zdd_abstractY(manager, forwardDinRdyDD);
	      DdNode * forwardBlankDinDD = adamant_zdd_varswap(manager, forwardDinBlankDD);
	      Cudd_RecursiveDerefZdd(manager, forwardDinBlankDD);
	      DdNode * forwardUnvDinDD = adamant_zdd_xDC(manager, forwardBlankDinDD);
	      Cudd_RecursiveDerefZdd(manager, forwardBlankDinDD);
	      DdNode * forwardDinDinDD = Cudd_zddIntersect(manager, dindinUbDD, forwardUnvDinDD);
	      Cudd_Ref(forwardDinDinDD);
	      Cudd_RecursiveDerefZdd(manager, forwardUnvDinDD);

	      DdNode * dinDinSlice = adamant_zdd_BuildIterDinDinForwardSlice(manager, forwardDinDinDD, dindinUbDD, 0);
	      Cudd_RecursiveDerefZdd(manager, forwardDinDinDD);
	      Cudd_RecursiveDerefZdd(manager, dindinUbDD);
	      
	      // START DEBUG
	      g_print("Forward Slice Size:%u\n", Cudd_zddCount(manager, dinDinSlice));
	      // END DEBUG

	      // Now make the reverse slice set smaller
	      DdNode * forwardDinBlankSliceDD = adamant_zdd_abstractY(manager, dinDinSlice);
	      DdNode * forwardBlankDinSliceDD = adamant_zdd_abstractX(manager, dinDinSlice);

	      DdNode * forwardDinBlankSliceDepDD = adamant_zdd_varswap(manager, forwardBlankDinSliceDD);
	      Cudd_RecursiveDerefZdd(manager, forwardBlankDinSliceDD);
	      DdNode * forwardDinBlankSliceTotalDD = Cudd_zddUnion(manager, forwardDinBlankSliceDD, forwardDinBlankSliceDepDD);
	      Cudd_Ref(forwardDinBlankSliceTotalDD);
	      Cudd_RecursiveDerefZdd(manager, forwardDinBlankSliceDD);
	      Cudd_RecursiveDerefZdd(manager, forwardDinBlankSliceDepDD);
	      DdNode * forwardDinUnvSliceDD = adamant_zdd_yDC(manager, forwardDinBlankSliceTotalDD);
	      Cudd_RecursiveDerefZdd(manager, forwardDinBlankSliceTotalDD);

	      // Here is the {DIN,RDY} for the reverse slice set with dependences removed
	      DdNode * simpleReverseSetInterferDD = Cudd_zddIntersect(manager, reverseDinRdyDD, forwardDinUnvSliceDD);
	      Cudd_Ref(simpleReverseSetInterferDD);	      
	      Cudd_RecursiveDerefZdd(manager, forwardDinUnvSliceDD);

	      // START DEBUG	    
	      //	      g_print("Simple Reverse Slice Interfer Size:%u\n", Cudd_zddCount(manager, simpleReverseSetInterferDD));
	      // END DEBUG
	      
	      DdNode * smallRevDinRdyDD = Cudd_zddDiff(manager, reverseDinRdyDD, simpleReverseSetInterferDD);
	      Cudd_Ref(smallRevDinRdyDD);

	      guint64 simpleInterfRevCount = Cudd_zddCount(manager, smallRevDinRdyDD);

	      // START DEBUG
	      g_print("Interference Removal Reverse Slice Size:%u\n", simpleInterfRevCount);
	      // END DEBUG	      

	      if(0 < simpleInterfRevCount)
		{	      
		  // Build the initial reverse slice
		  DdNode * reverseDinBlankInterferDD = adamant_zdd_abstractY(manager, simpleReverseSetInterferDD);
		  DdNode * reverseDinUnvInterferDD = adamant_zdd_yDC(manager, reverseDinBlankInterferDD);
		  Cudd_RecursiveDerefZdd(manager, reverseDinBlankInterferDD);
		  DdNode * reverseDinDinInterferDD = Cudd_zddIntersect(manager, reverseDinUnvInterferDD, dinDinSlice);
		  Cudd_Ref(reverseDinDinInterferDD);
		  Cudd_RecursiveDerefZdd(manager, reverseDinUnvInterferDD);
		  
		  // Perform the reverse slice to complete the chop
		  DdNode * revDinDinSliceDD = adamant_zdd_BuildIterDinDinReverseSlice(manager, reverseDinDinInterferDD, dinDinSlice, 0, 0);
		  Cudd_RecursiveDerefZdd(manager, reverseDinDinInterferDD);
		  Cudd_RecursiveDerefZdd(manager, dinDinSlice);
	      
		  // Now make the forward slice set smaller
		  DdNode * revDinBlankSliceDD = adamant_zdd_abstractY(manager, revDinDinSliceDD);
		  DdNode * revBlankDinSliceDD = adamant_zdd_abstractX(manager, revDinDinSliceDD);
		  Cudd_RecursiveDerefZdd(manager, revDinDinSliceDD);

		  DdNode * revDinBlankSliceDepDD = adamant_zdd_varswap(manager, revBlankDinSliceDD);
		  Cudd_RecursiveDerefZdd(manager, revBlankDinSliceDD);
		  DdNode * revDinBlankSliceTotalDD = Cudd_zddUnion(manager, revDinBlankSliceDD, revDinBlankSliceDepDD);
		  Cudd_Ref(revDinBlankSliceTotalDD);
		  Cudd_RecursiveDerefZdd(manager, revDinBlankSliceDD);
		  Cudd_RecursiveDerefZdd(manager, revDinBlankSliceDepDD);
		  DdNode * revDinUnvSliceDD = adamant_zdd_yDC(manager, revDinBlankSliceTotalDD);
		  Cudd_RecursiveDerefZdd(manager, revDinBlankSliceTotalDD);
		  
		  // Build the forward interference set
		  DdNode * simpleForwardSetInterferDD = Cudd_zddIntersect(manager, forwardDinRdyDD, revDinUnvSliceDD);
		  Cudd_Ref(simpleForwardSetInterferDD);
		  Cudd_RecursiveDerefZdd(manager, revDinUnvSliceDD);

		  // START DEBUG	    
		  //	      g_print("Simple Forward Slice Interfer Set:%u\n", Cudd_zddCount(manager, simpleForwardSetInterferDD));
		  // END DEBUG

		  // Here is the {DIN,RDY} for the forward slice set with dependences removed
		  DdNode * smallForwardDinRdyDD = Cudd_zddDiff(manager, forwardDinRdyDD, simpleForwardSetInterferDD);
		  Cudd_Ref(smallForwardDinRdyDD);
		  guint64 simpleInterfForwardCount = Cudd_zddCount(manager, smallForwardDinRdyDD);

		  // START DEBUG
		  g_print("Interference Removal Forward Slice Set:%u\n", simpleInterfForwardCount);
		  // END DEBUG

		  // Get performace numbers with simple dependence interference
		  guint64 newSimpleInterferSize = 0;
		  DdNode * tempSimpleDinRdyDD = NULL;
		  if (simpleInterfForwardCount < simpleInterfRevCount)
		    {
		      newSimpleInterferSize = simpleInterfForwardCount;
		      tempSimpleDinRdyDD = smallForwardDinRdyDD;
		    }
		  else
		    {
		      newSimpleInterferSize = simpleInterfRevCount;
		      tempSimpleDinRdyDD = smallRevDinRdyDD;
		    }

		  simpleInterferSize += newSimpleInterferSize;
	      
		  if(0 < newSimpleInterferSize)
		    {

		      interferRegions[xShrunkA->regionID] = interferRegions[xShrunkA->regionID] + 1;
		      interferRegions[xShrunkB->regionID] = interferRegions[xShrunkB->regionID] + 1;
		      
		      DdNode * tmpDinRdy = Cudd_zddDiff(manager, dinrdyDD, tempSimpleDinRdyDD);
		      Cudd_Ref(tmpDinRdy);
		      Cudd_RecursiveDerefZdd(manager, dinrdyDD);
		      dinrdyDD = tmpDinRdy;

		      DdNode * tmpSIDinRdyDD = Cudd_zddUnion(manager, dinRdySimpleInterferDD, tempSimpleDinRdyDD);
		      Cudd_Ref(tmpSIDinRdyDD);
		      Cudd_RecursiveDerefZdd(manager, dinRdySimpleInterferDD);
		      dinRdySimpleInterferDD = tmpSIDinRdyDD;
		      
		      if(NULL != overlayDDFileName)
			{
			  g_print("Saving to file:%s\n", overlayDDFileName);
			  
			  DdNode * finalDD = dinRdySimpleInterferDD;
			  
			  FILE * outFh = fopen(overlayDDFileName, "w+");
			  //! add extra information to this ZDD dump
			  
			  guint64 xTop = 0;
			  guint64 yTop = 0;
			  guint64 xBottom = 0;
			  guint64 yBottom = 0;
			  
			  adamant_zdd_GetTupleTop(manager, finalDD, &xTop, &yTop);
			  adamant_zdd_GetTupleBottom(manager, finalDD, &xBottom, &yBottom);    
			  
			  
			  headerInfo.extraTraceInfo = g_strdup_printf("type:dinrdyoverlay,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT, 
								      xBottom,
								      xTop);
			  
			  Dddmp_cuddZddStore(manager, NULL,
					     finalDD, NULL, &zdd_ordr,
					     DDDMP_MODE_BINARY, extrainfo, &headerInfo,
					     overlayDDFileName, outFh);	     
			  fclose(outFh);
			  g_free(headerInfo.extraTraceInfo);
			}
		    }

		  Cudd_RecursiveDerefZdd(manager, smallForwardDinRdyDD);
		  Cudd_RecursiveDerefZdd(manager, simpleForwardSetInterferDD);
		}

	      Cudd_RecursiveDerefZdd(manager, smallRevDinRdyDD);
	      Cudd_RecursiveDerefZdd(manager, simpleReverseSetInterferDD);

	      g_free(xShrunkA);
	      g_free(xShrunkB);			      
	    }

	  g_free(overlap);
	  g_queue_push_tail(ddRegions, regionB);
	  --queueCount;
	}

      g_queue_push_tail(doneRegions, regionA);
    }

  // Restore the regions queue
  while (!g_queue_is_empty (doneRegions))
    {
      g_queue_push_tail(ddRegions, 
			g_queue_pop_head(doneRegions));
    }

  g_print("#RegionID,Naive\n");
  for (i = 0; i < g_queue_get_length(ddRegions); ++i)
    {
      g_print("%d,%d\n", i, naiveRegions[i]);      
    }
  g_free(naiveRegions);
  g_print("\n#RegionID,Interfer\n");
  for (i = 0; i < g_queue_get_length(ddRegions); ++i)
    {
      g_print("%d,%d\n", i, interferRegions[i]);      
    }
  g_free(interferRegions);
  
  g_queue_free(doneRegions);

  // Save out the results
  if(NULL != overlayDDFileName)
    {
      g_print("Saving to file:%s\n", overlayDDFileName);
      
      DdNode * finalDD = dinRdySimpleInterferDD;
      
      FILE * outFh = fopen(overlayDDFileName, "w+");
      //! add extra information to this ZDD dump
      
      guint64 xTop = 0;
      guint64 yTop = 0;
      guint64 xBottom = 0;
      guint64 yBottom = 0;
      
      adamant_zdd_GetTupleTop(manager, finalDD, &xTop, &yTop);
      adamant_zdd_GetTupleBottom(manager, finalDD, &xBottom, &yBottom);    
      
      
      headerInfo.extraTraceInfo = g_strdup_printf("type:dinrdyoverlay,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT, 
						  xBottom,
						  xTop);
      
      Dddmp_cuddZddStore(manager, NULL,
			 finalDD, NULL, &zdd_ordr,
			 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			 overlayDDFileName, outFh);	     
      fclose(outFh);
      g_free(headerInfo.extraTraceInfo);
    }

  //! Report performance gains
  gdouble nPerGain = (1 - ((gdouble)(executionSize - naiveSize) / executionSize)) * 100;
  gdouble sPerGain = (1 - ((gdouble)(executionSize - simpleInterferSize) / executionSize)) * 100;
  g_print("\nExecution Count: %"G_GUINT64_FORMAT"\n", executionSize);
  g_print("Naive Count: %"G_GUINT64_FORMAT"\n", naiveSize);
  g_print("Naive Performance Gain: %f\%\n", nPerGain);
  g_print("Simple Interference Count: %"G_GUINT64_FORMAT"\n", simpleInterferSize);
  g_print("Simple Interference Performance Gain: %f\%\n", sPerGain); 
}

int adamant_zddtest_naivePerformance(guint64 slotsMulti, GString * inputs)
{
  DdNode * dinrdyDD = NULL;
  gchar * regionFileName = NULL;
  int zdd_ordr[ZDDNUM];
  
  DdManager * manager = adamant_zddtest_initmanagerSlots(zdd_ordr, slotsMulti, 0);
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
  headerInfo.extraTraceInfo = g_new(char, 512);
  gchar ** fileVec = g_strsplit(inputs->str, " " , 5);
  guint64 * naiveRegions = NULL;
  int i = 0;

  while (NULL != fileVec[i])
    {      
      if ((NULL != g_strrstr(fileVec[i], ".slr")))
	{
	  g_print("Using regions from file:%s\n", fileVec[i]);
	  regionFileName = g_strdup(fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  
	  adamant_zdd_addRegions(regionFileName);
	  naiveRegions = g_new0(guint64, g_queue_get_length(ddRegions));
	}
      else if (NULL != g_strrstr(fileVec[i], "dinvsrdy")||
	       (NULL != g_strrstr(fileVec[i], "dinrdy")))
	{
	  g_print("Using {DIN,RDY} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  dinrdyDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  Cudd_Ref(dinrdyDD);
	  fclose(fh);	  
	}
     
      ++i;
    }
  g_strfreev(fileVec);

  if(NULL == dinrdyDD)
    {
      return(1);
    }

  GQueue * doneRegions = g_queue_new ();

  // Variables needed for upper and lower bounds
  guint64 zddNumHalf = ZDDNUM/2;
  int xlist [ZDDNUM/2];
  int ylist [ZDDNUM/2];      
  //! gather a list of ZDD node variables
  for (i = 0; i < zddNumHalf; i+=1)
    {
      xlist[(zddNumHalf - 1) - i] = i;
      ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
    }

  //! performance tracking variables
  guint64 executionSize = Cudd_zddCount(manager, dinrdyDD);
  guint64 naiveSize = 0;

  while (!g_queue_is_empty (ddRegions))
    {
      ddregion * regionA =  g_queue_pop_head(ddRegions);
      
      guint queueCount = g_queue_get_length(ddRegions);

      while (0 < queueCount)
	{
	  ddregion * regionB = g_queue_pop_head(ddRegions);

	  /***********************************************************/
	  /* Find optimal performance gain */
	  /***********************************************************/

	  ddregion * overlap = adamant_zdd_regionsOverlap(regionA, regionB);

	  // START DEBUG
	  g_print("\nRegions:%u->%u,Overlap:TL X:%u Y:%u, BR X:%u Y:%u\n", 
		  regionA->regionID, regionB->regionID, 
		  overlap->topLeft.X, overlap->topLeft.Y, 
		  overlap->bottomRight.X, overlap->bottomRight.Y);
	  // END DEBUG
	  
	  if(!((0 == overlap->topLeft.X) &&
	       (0 == overlap->bottomRight.X)))
	    {
	      // create a regionA shrunk by the x overlap
	      ddregion * xShrunkA = g_new0(ddregion, 1);
	      xShrunkA->bottomRight.Y = overlap->topLeft.X;
	      xShrunkA->bottomRight.X = regionA->topLeft.Y;
	      xShrunkA->topLeft.Y = overlap->bottomRight.X;
	      xShrunkA->topLeft.X = regionA->bottomRight.Y;
	      xShrunkA->regionID = regionA->regionID;

	      // create a regionB shrunk by the x overlap
	      ddregion * xShrunkB = g_new0(ddregion, 1);
	      xShrunkB->bottomRight.Y = overlap->topLeft.X;
	      xShrunkB->bottomRight.X = regionB->topLeft.Y;
	      xShrunkB->topLeft.Y = overlap->bottomRight.X;
	      xShrunkB->topLeft.X = regionB->bottomRight.Y;
	      xShrunkB->regionID = regionB->regionID;

	      // Get the DD for the region A
	      DdNode * regADD = adamant_zdd_DdRegion(manager, dinrdyDD, xShrunkA);
	      Cudd_Ref(regADD);
	      
	      guint64 regionCountA = Cudd_zddCount(manager, regADD);
 
	      // Get the DD for the region B
	      DdNode * regBDD = adamant_zdd_DdRegion(manager, dinrdyDD, xShrunkB);
	      Cudd_Ref(regBDD);

	      guint64 regionCountB = Cudd_zddCount(manager, regBDD);

	      guint64 localNaiveSize = 0;
	      DdNode * localNaiveDD = NULL;

	      if(regionCountB < regionCountA)
		{
		  localNaiveDD = Cudd_zddDiff(manager, regBDD, regADD);
		  Cudd_Ref(localNaiveDD);
		}
	      else
		{
		  localNaiveDD = Cudd_zddDiff(manager, regADD, regBDD);
		  Cudd_Ref(localNaiveDD);
		}

	      localNaiveSize = Cudd_zddCount(manager, localNaiveDD);

	      naiveSize += localNaiveSize;
	  		
	      if(0 < localNaiveSize)
		{
		  naiveRegions[xShrunkA->regionID] = naiveRegions[xShrunkA->regionID] + 1;
		  naiveRegions[xShrunkB->regionID] = naiveRegions[xShrunkB->regionID] + 1;

		  DdNode * tempDinRdy = Cudd_zddDiff(manager, dinrdyDD, localNaiveDD);
		  Cudd_Ref(tempDinRdy);
		  Cudd_RecursiveDerefZdd(manager, dinrdyDD);
		  dinrdyDD = tempDinRdy;
		}	

	      Cudd_RecursiveDerefZdd(manager, regADD);
	      Cudd_RecursiveDerefZdd(manager, regBDD);
	      Cudd_RecursiveDerefZdd(manager, localNaiveDD);

	      // START DEBUG
	      g_print("Shrunk Regions:%"G_GINT64_FORMAT"->%"G_GINT64_FORMAT",Naive Size:%u\n", 
		      xShrunkA->regionID,
		      xShrunkB->regionID,
		      localNaiveSize);
	      g_print("\n");
	      // END DEBUG


	      g_free(xShrunkA);
	      g_free(xShrunkB);
	    }				  
	  
	  g_free(overlap);
	  g_queue_push_tail(ddRegions, regionB);
	  --queueCount;
	}

      g_queue_push_tail(doneRegions, regionA);
    }

  // Restore the regions queue
  while (!g_queue_is_empty (doneRegions))
    {
      g_queue_push_tail(ddRegions, 
			g_queue_pop_head(doneRegions));
    }

  g_print("#RegionID,Naive\n");
  for (i = 0; i < g_queue_get_length(ddRegions); ++i)
    {
      g_print("%d,%d\n", i, naiveRegions[i]);      
    }
  g_free(naiveRegions);

  //! Report performance gains
  gdouble nPerGain = (1 - ((gdouble)(executionSize - naiveSize) / executionSize)) * 100;
  g_print("\nExecution Count: %"G_GUINT64_FORMAT"\n", executionSize);
  g_print("Naive Count: %"G_GUINT64_FORMAT"\n", naiveSize);
  g_print("Naive Performance Gain: %f\%\n", nPerGain);
  
  return(0);
}


int adamant_zddtest_parallelAdd(DdManager * manager, DdNode * node, GQueue * queue)
{
  ddpoint top_left;
  ddpoint bottom_right;
  
  guint queueCount = g_queue_get_length(queue);
  top_left.X = 0;
  top_left.Y = 0;
  bottom_right.X = 0;
  bottom_right.Y = 0;

  adamant_zdd_GetTupleTop(manager, node, &(top_left.Y), &(bottom_right.X));
  adamant_zdd_GetTupleBottom(manager, node, &(bottom_right.Y), &(top_left.X));

  ddregion * newRegion = g_new0(ddregion, 1);  
  newRegion->topLeft = top_left;
  newRegion->bottomRight = bottom_right;
  newRegion->regionID = (queueCount + 1);
  newRegion->parallelCount = 0; 

  int i = 0;
  for (i = 0; i < queueCount;++i)
    {
      ddregion * queueRegion = g_queue_pop_head(queue);
      ddregion * overlap = adamant_zdd_regionsOverlap(newRegion,queueRegion);
	  
      if(!((0 == overlap->topLeft.X) && (0 == overlap->bottomRight.X)) &&
	 ((newRegion->topLeft.Y > queueRegion->topLeft.Y) ||
	  (newRegion->bottomRight.Y < queueRegion->bottomRight.Y))
	 )       
	{
	  // START DEBUG
	  g_print("Parallel Add TL: %"G_GUINT64_FORMAT",%"G_GUINT64_FORMAT" BR: %"G_GUINT64_FORMAT",%"G_GUINT64_FORMAT"\n",
		  newRegion->topLeft.X, newRegion->topLeft.Y,
		  newRegion->bottomRight.X, newRegion->bottomRight.Y);
	  // END DEBUG

	  queueRegion->parallelCount = queueRegion->parallelCount + 1;
	  newRegion->parallelCount = newRegion->parallelCount + 1;
	}      

      g_queue_push_tail(queue, queueRegion);
    }


  g_queue_push_head(queue, newRegion);
  
  return(0);
}

int adamant_zddtest_dumpRegionInfo(GQueue * regionQueue)
{
  int i = 0;
  guint queueCount = g_queue_get_length(regionQueue);

  // RegionID,Start,End,ParallelCount
  for (i = 0; i < queueCount; ++i)
    {
      ddregion * queueRegion = g_queue_pop_head(regionQueue);
      
      g_print("%u,%"G_GUINT64_FORMAT",%"G_GUINT64_FORMAT",%"G_GUINT64_FORMAT"\n",
	      queueRegion->regionID,
	      queueRegion->topLeft.X,
	      queueRegion->bottomRight.X,
	      queueRegion->parallelCount);

      g_queue_push_tail(regionQueue, queueRegion);
    }

  return(0);
}

int adamant_zddtest_sinRegion(guint64 slotsMulti, GString * ddFiles)
{
  DdNode * dinsinDD = NULL;
  DdNode * dinrdyDD = NULL;
  DdNode * dinhotDD = NULL;
  int zdd_ordr[ZDDNUM];
  gchar * regionFileName = NULL;
  
  DdManager * manager = adamant_zddtest_initmanagerSlots(zdd_ordr, slotsMulti, 0);
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
  headerInfo.extraTraceInfo = g_new(char, 512);
  gchar ** fileVec = g_strsplit(ddFiles->str, " " , 50);
  GArray * inputFiles = g_array_new (FALSE, FALSE, sizeof (gchar*));
  GArray * dinrdyRegionDDs = g_array_new (FALSE, FALSE, sizeof (DdNode*));
  int i = 0;
  
  i = 0;
  while (NULL != fileVec[i])
    {
      g_array_append_val(inputFiles, fileVec[i]);
      ++i;
    }
  
  i = 0;
  while ((i < inputFiles->len) && 
	 (NULL == regionFileName))
    {
      gchar * filename = g_array_index (inputFiles, gchar*, i);
      if ((NULL != g_strrstr(filename, ".slr")))
	{
	  g_print("Using regions from file:%s\n", filename);
	  regionFileName = g_strdup(filename);
	  FILE * fh = fopen(filename, "r+");
	  
	  adamant_zdd_addRegions(regionFileName);
	  inputFiles = g_array_remove_index(inputFiles,i);
	}     
      ++i;
    }

  i = 0;
  GQueue * tempQueue = g_queue_new ();
  while (i < inputFiles->len)
    {
      gchar * regionStr = g_array_index (inputFiles, gchar*, i);
      gboolean isRegion = true;
      int c = 0;
      while((TRUE == isRegion) && 
	    (NULL != regionStr[c]))
	{
	  isRegion = isRegion & g_ascii_isdigit(regionStr[c]);
	  ++c;
	}
      if(TRUE == isRegion)
	{
	  guint64 regionNum = g_ascii_strtoull (regionStr,
						NULL,
						10);

	  guint queueCount = g_queue_get_length(ddRegions);
	  int regCount = 0;
	  while (regCount < queueCount)
	    {
	      ddregion * region = g_queue_pop_head(ddRegions);
	      if(regionNum == region->regionID)
		{
		  g_queue_push_tail(tempQueue, region);
		}
	      g_queue_push_tail(ddRegions, region);
	      ++regCount;
	    }
	}
      
      ++i;
    }
  g_queue_free (ddRegions);
  ddRegions = tempQueue;

  i = 0;
  while ((i < inputFiles->len) && 
	 (NULL == dinrdyDD))
    {
      gchar * filename = g_array_index (inputFiles, gchar*, i);
	if (NULL != g_strrstr(filename, "dinvsrdy")||
	    (NULL != g_strrstr(filename, "dinrdy")))
	  {
	    g_print("Using {DIN,RDY} set from file:%s\n", filename);
	    FILE * fh = fopen(filename, "r+");
	    dinrdyDD = Dddmp_cuddZddLoad(manager,
					 DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					 DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					 filename, fh);
	    Cudd_Ref(dinrdyDD);

	    if(NULL != regionFileName)
	      {
		guint queueCount = g_queue_get_length(ddRegions);
		int regCount = 0;
		while (regCount < queueCount)
		  {
		    ddregion * region = g_queue_pop_head(ddRegions);

		    ddregion * swizzleRegion = g_new0(ddregion, 1);
		    swizzleRegion->bottomRight.Y = region->topLeft.X;
		    swizzleRegion->bottomRight.X = region->topLeft.Y;
		    swizzleRegion->topLeft.Y = region->bottomRight.X;
		    swizzleRegion->topLeft.X = region->bottomRight.Y;
		    swizzleRegion->regionID = region->regionID;
		    swizzleRegion->parallelCount = region->parallelCount;

		    DdNode * tmpDinRdyRegionDD = adamant_zdd_DdRegion(manager, dinrdyDD, swizzleRegion);
		    g_free(swizzleRegion);
		    DdNode * tmpDinBlankRegionDD = adamant_zdd_abstractY(manager, tmpDinRdyRegionDD);
		    Cudd_RecursiveDerefZdd(manager, tmpDinRdyRegionDD);
		    DdNode * tmpDinUnvRegionDD = adamant_zdd_yDC(manager, tmpDinBlankRegionDD);
		    Cudd_RecursiveDerefZdd(manager, tmpDinBlankRegionDD);
		    g_array_append_val(dinrdyRegionDDs, tmpDinUnvRegionDD);		    
		    g_queue_push_tail(ddRegions, region);
		    ++regCount;
		  }
		Cudd_RecursiveDerefZdd(manager, dinrdyDD);
	      }

	    fclose(fh);	
	    inputFiles = g_array_remove_index(inputFiles,i);
	  }      
      ++i;
    }

  i = 0;
  while ((i < inputFiles->len) && 
	 (NULL == dinsinDD))
    {
      gchar * filename = g_array_index (inputFiles, gchar*, i);
      if (NULL != g_strrstr(filename, "dinvssin")||
	       (NULL != g_strrstr(filename, "dinsin")))
	{
	  g_print("Using {DIN,SIN} set from file:%s\n", filename);
	  FILE * fh = fopen(filename, "r+");
	  dinsinDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       filename, fh);
	  Cudd_Ref(dinsinDD);
	  fclose(fh);
	  inputFiles = g_array_remove_index(inputFiles,i);
	}
      ++i;
    } 
 
  g_array_free (inputFiles, FALSE);
  g_strfreev(fileVec);

  if((NULL == dinsinDD)||
     (0 == dinrdyRegionDDs->len))
    {
      return(0);
    }

  for (i = 0; i < dinrdyRegionDDs->len; ++i)
    {
      int j = 0;
      int * dinSinPath[ZDDNUM];
      for (j = 0; j < ZDDNUM; ++j)
	{
	  dinSinPath[j] = 0;
	}
      DdNode * dinUnvRegionDD = g_array_index (dinrdyRegionDDs, DdNode*, i);
      DdNode * dinSinRegionDD = Cudd_zddIntersect(manager, dinsinDD, dinUnvRegionDD);
      Cudd_Ref(dinSinRegionDD);
      Cudd_RecursiveDerefZdd(manager, dinUnvRegionDD);

      adamant_zdd_SinPrint(manager, dinSinRegionDD, 0, 0);
   
      Cudd_RecursiveDerefZdd(manager,dinSinRegionDD);
    }
  g_array_free (dinrdyRegionDDs, FALSE);
  return(0);
}


int adamant_zddtest_sinStats(guint64 slotsMulti, GString * ddFiles)
{
  DdNode * dinsinDD = NULL;
  DdNode * dinrdyDD = NULL;
  DdNode * dinhotDD = NULL;
  int zdd_ordr[ZDDNUM];
  gchar * regionFileName = NULL;
  
  DdManager * manager = adamant_zddtest_initmanagerSlots(zdd_ordr, slotsMulti, 0);
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
  headerInfo.extraTraceInfo = g_new(char, 512);
  gchar ** fileVec = g_strsplit(ddFiles->str, " " , 5);
  GArray * inputFiles = g_array_new (FALSE, FALSE, sizeof (gchar*));
  GArray * dinrdyRegionDDs = g_array_new (FALSE, FALSE, sizeof (DdNode*));
  int i = 0;
  
  i = 0;
  while (NULL != fileVec[i])
    {
      g_array_append_val(inputFiles, fileVec[i]);
      ++i;
    }

  
  i = 0;
  while ((i < inputFiles->len) && 
	 (NULL == regionFileName))
    {
      gchar * filename = g_array_index (inputFiles, gchar*, i);
      if ((NULL != g_strrstr(filename, ".slr")))
	{
	  g_print("Using regions from file:%s\n", filename);
	  regionFileName = g_strdup(filename);
	  FILE * fh = fopen(filename, "r+");
	  
	  adamant_zdd_addRegions(regionFileName);
	  inputFiles = g_array_remove_index(inputFiles,i);
	}     
      ++i;
    }

  i = 0;
  while ((i < inputFiles->len) && 
	 (NULL == dinrdyDD))
    {
      gchar * filename = g_array_index (inputFiles, gchar*, i);
	if (NULL != g_strrstr(filename, "dinvsrdy")||
	    (NULL != g_strrstr(filename, "dinrdy")))
	  {
	    g_print("Using {DIN,RDY} set from file:%s\n", filename);
	    FILE * fh = fopen(filename, "r+");
	    dinrdyDD = Dddmp_cuddZddLoad(manager,
					 DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					 DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					 filename, fh);
	    Cudd_Ref(dinrdyDD);

	    if(NULL != regionFileName)
	      {
		guint queueCount = g_queue_get_length(ddRegions);
		int regCount = 0;
		while (regCount < queueCount)
		  {
		    ddregion * region = g_queue_pop_head(ddRegions);

		    ddregion * swizzleRegion = g_new0(ddregion, 1);
		    swizzleRegion->bottomRight.Y = region->topLeft.X;
		    swizzleRegion->bottomRight.X = region->topLeft.Y;
		    swizzleRegion->topLeft.Y = region->bottomRight.X;
		    swizzleRegion->topLeft.X = region->bottomRight.Y;
		    swizzleRegion->regionID = region->regionID;
		    swizzleRegion->parallelCount = region->parallelCount;

		    DdNode * tmpDinRdyRegionDD = adamant_zdd_DdRegion(manager, dinrdyDD, swizzleRegion);
		    g_free(swizzleRegion);
		    g_print("REG:%d Size:%u\n",regCount, Cudd_zddCount(manager, tmpDinRdyRegionDD));
		    DdNode * tmpDinBlankRegionDD = adamant_zdd_abstractY(manager, tmpDinRdyRegionDD);
		    Cudd_RecursiveDerefZdd(manager, tmpDinRdyRegionDD);
		    DdNode * tmpDinUnvRegionDD = adamant_zdd_yDC(manager, tmpDinBlankRegionDD);
		    Cudd_RecursiveDerefZdd(manager, tmpDinBlankRegionDD);
		    g_array_append_val(dinrdyRegionDDs, tmpDinUnvRegionDD);		    
		    g_queue_push_tail(ddRegions, region);
		    ++regCount;
		  }
		Cudd_RecursiveDerefZdd(manager, dinrdyDD);
	      }

	    fclose(fh);	  
	    inputFiles = g_array_remove_index(inputFiles,i);
	  }      
      ++i;
    }

  i = 0;
  while ((i < inputFiles->len) && 
	 (NULL == dinsinDD))
    {
      gchar * filename = g_array_index (inputFiles, gchar*, i);
      if (NULL != g_strrstr(filename, "dinvssin")||
	       (NULL != g_strrstr(filename, "dinsin")))
	{
	  g_print("Using {DIN,SIN} set from file:%s\n", filename);
	  FILE * fh = fopen(filename, "r+");
	  dinsinDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       filename, fh);
	  Cudd_Ref(dinsinDD);
	  fclose(fh);	  
	  inputFiles = g_array_remove_index(inputFiles,i);
	}
      ++i;
    } 
  i = 0;
  while ((i < inputFiles->len) && 
	 (NULL == dinhotDD))
    {
      gchar * filename = g_array_index (inputFiles, gchar*, i);
      if (NULL != g_strrstr(filename, "dinvshot")||
	       (NULL != g_strrstr(filename, "dinhot")))
	{
	  g_print("Using {DIN,HOT} set from file:%s\n", filename);
	  FILE * fh = fopen(filename, "r+");
	  dinhotDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       filename, fh);
	  Cudd_Ref(dinhotDD);
	  fclose(fh);	  
	  inputFiles = g_array_remove_index(inputFiles,i);
	}
      ++i;
    } 
  g_array_free (inputFiles, FALSE);
  g_strfreev(fileVec);

  if((NULL == dinsinDD)||
     (0 == dinrdyRegionDDs->len))
    {
      return(0);
    }

  for (i = 0; i < dinrdyRegionDDs->len; ++i)
    {
      int j = 0;
      int * dinSinPath[ZDDNUM];
      for (j = 0; j < ZDDNUM; ++j)
	{
	  dinSinPath[j] = 0;
	}
      DdNode * dinUnvRegionDD = g_array_index (dinrdyRegionDDs, DdNode*, i);
      DdNode * dinSinRegionDD = Cudd_zddIntersect(manager, dinsinDD, dinUnvRegionDD);
      Cudd_Ref(dinSinRegionDD);
      DdNode * dinHotRegionDD = Cudd_zddIntersect(manager, dinhotDD, dinUnvRegionDD);
      Cudd_Ref(dinHotRegionDD);
      Cudd_RecursiveDerefZdd(manager, dinUnvRegionDD);
      g_print("REG:%d Size:%u\n", i, Cudd_zddCount(manager, dinSinRegionDD));
      g_print("REG:%d,DIN,SIN\n", i);

      adamant_zdd_DinSinHot(manager, dinSinRegionDD, dinHotRegionDD, 0, 0);
   
      Cudd_RecursiveDerefZdd(manager,dinSinRegionDD);
    }
  g_array_free (dinrdyRegionDDs, FALSE);
  return(0);
}

int adamant_zddtest_tophot(guint64 slotsMulti, GString * ddFiles)
{
    DdNode * dinrdyDD = NULL;
    DdNode * dindinDD = NULL;
    DdNode * dinhotDD = NULL;
    guint64 topHotPercent = 0;
    int zdd_ordr[ZDDNUM];

    DdManager * manager = NULL;
    Dddmp_MoreDDHeaderInfo headerInfo;
    Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
    headerInfo.extraTraceInfo = g_new(char, 512);
    gchar ** fileVec = g_strsplit(ddFiles->str, " " , 5);

    manager = Cudd_Init(0, 0, (CUDD_UNIQUE_SLOTS * slotsMulti), CUDD_CACHE_SLOTS,  adamant_zdd_freemem());

    if(NULL != fileVec[4])
      {
	topHotPercent = g_ascii_strtoull(fileVec[4], NULL, 10);
	g_print("Top Hot:%"G_GUINT64_FORMAT"\n", topHotPercent); 
      }

    if(0 == topHotPercent)
      {
	topHotPercent = 25;
      }

    if (fileVec[0] != NULL)
      {
        g_print("Using {DIN,HOT} set from file:%s\n", fileVec[0]);
        FILE * fh = fopen(fileVec[0], "r+");
        dinhotDD = Dddmp_cuddZddLoad(manager,
				     DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				     DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				     fileVec[0], fh);

        Cudd_Ref(dinhotDD);
        fclose(fh);

	// Get the top section of hot code
	guint64 topX, topY;
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

	adamant_zdd_GetTupleTop2(manager,dinhotDD, &topX, &topY);
      
	guint64 topHotSel = (guint64) (topY - (guint64)((double)topY * (double)((double)topHotPercent) / 100));

	printf("Using the value %d as an upper bound\n", (int)topHotSel);

	DdNode * yLb =  mg_Cudd_zddUb(manager, zddNumHalf, ylist, (uint64_t)topHotSel);
	Cudd_Ref(yLb);

	DdNode * hotLbUnvDD = adamant_zdd_xDC(manager, yLb);

	DdNode * topHotDD = Cudd_zddIntersect(manager, hotLbUnvDD, dinhotDD);
	Cudd_Ref(topHotDD);
	Cudd_RecursiveDerefZdd(manager, hotLbUnvDD);
	Cudd_RecursiveDerefZdd(manager, dinhotDD);

	// Remove the Hot Values
	DdNode * dinEmptyHotDD = adamant_zdd_abstractY(manager, topHotDD);
	Cudd_RecursiveDerefZdd(manager, topHotDD);
	DdNode * dinUnvHotDD = adamant_zdd_yDC(manager, dinEmptyHotDD);
	Cudd_RecursiveDerefZdd(manager, dinEmptyHotDD);

	// Read in the {DIN,RDY} tuple set
	if (fileVec[1] != NULL)
	  {
	    g_print("Using {DIN,RDY} file:%s\n", fileVec[1]);

	    FILE * fh = fopen(fileVec[1], "r+");
	    dinrdyDD = Dddmp_cuddZddLoad(manager, DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					 DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					 fileVec[1], fh);
	    Cudd_Ref(dinrdyDD);
	    fclose(fh);

	    if (fileVec[2] != NULL)
	      {
		g_print("Using {DIN,DIN} file:%s\n", fileVec[2]);

		FILE * fh = fopen(fileVec[2], "r+");
		dindinDD = Dddmp_cuddZddLoad(manager, DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					     DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					     fileVec[2], fh);
		Cudd_Ref(dindinDD);
		fclose(fh);

		DdNode * dinRdyHotDD = Cudd_zddIntersect(manager, dinUnvHotDD, dinrdyDD);
		Cudd_Ref(dinRdyHotDD);
		Cudd_RecursiveDerefZdd(manager, dinUnvHotDD);

		// Remove the dead code from hot DINxRDY DD
		printf("Old DINxRDY Size:%u\n", Cudd_zddCount(manager,dinRdyHotDD));
		DdNode * dinHotAliveDD = adamant_zdd_DeadReadyFilterSelection(manager,
									      dinrdyDD,
									      dindinDD,
									      dinRdyHotDD);
		Cudd_RecursiveDerefZdd(manager, dinRdyHotDD);
		printf("New DINxRDY Size:%u\n", Cudd_zddCount(manager, dinHotAliveDD));

		DdNode * finalDD = dinHotAliveDD;
      
		// Save out the results
		if(fileVec[3] != NULL)
		  {
		    g_print("Saving to file:%s\n", fileVec[3]);
		
		    FILE * outFh = fopen(fileVec[3], "w+");
		    //! add extra information to this ZDD dump
		    g_snprintf(headerInfo.extraTraceInfo,512, "type:dinrdy,dinstart:0,dinstop:0");
		
		    Dddmp_cuddZddStore(manager, NULL,
				       finalDD, NULL, &zdd_ordr,
				       DDDMP_MODE_BINARY, extrainfo, &headerInfo,
				       fileVec[3], outFh);
		    fclose(outFh);
		  }
	      }
	  }
      }
    
    g_free(headerInfo.extraTraceInfo);
    g_string_free(ddFiles, TRUE);
    g_strfreev(fileVec);

    return (0);
}

int adamant_zddtest_quickdead(GString * ddFiles)
{
    DdNode * dinrdyDD = NULL;
    DdNode * dindinDD = NULL;
    int zdd_ordr[ZDDNUM];

    DdManager * manager = NULL;
    Dddmp_MoreDDHeaderInfo headerInfo;
    Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
    headerInfo.extraTraceInfo = g_new(char, 512);
    gchar ** fileVec = g_strsplit(ddFiles->str, " " , 4);

    manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS,  adamant_zdd_freemem());

    if (fileVec[0] != NULL)
    {
        g_print("Using {DIN,RDY} set from file:%s\n", fileVec[0]);
        FILE * fh = fopen(fileVec[0], "r+");
        dinrdyDD = Dddmp_cuddZddLoad(manager,
					     DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					     DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					     fileVec[0], fh);

        Cudd_Ref(dinrdyDD);
        fclose(fh);
    }

    // Read in the {DIN,{DIN}} tuple set
    if (fileVec[1] != NULL)
    {
        g_print("Using {DIN_i,DIN_d} file:%s\n", fileVec[1]);

        FILE * fh = fopen(fileVec[1], "r+");
        dindinDD = Dddmp_cuddZddLoad(manager, DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					      DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					      fileVec[1], fh);
	Cudd_Ref(dindinDD);
        fclose(fh);
    }

    if((dindinDD != NULL) && (dinrdyDD != NULL))
    {
      // Get the top RDY time set
      guint64 xTop = 0;
      guint64 yTop = 0;

      adamant_zdd_GetTupleTop(manager, dinrdyDD, &xTop, &yTop);

      // DEBUG
      printf("Top Din:%d, Rdy:%d\n", xTop, yTop);

      DdNode * zero = Cudd_ReadZero(manager);
      Cudd_Ref(zero);
  
      // Build a tuple that represents our top ready time 
      DdNode * topRdyDD = adamant_zdd_build_tuple(manager, zero, 0, yTop); 
      Cudd_RecursiveDerefZdd(manager, zero); 

      // Add in the universal set of X values
      DdNode * topUnivRdyDD = adamant_zdd_xDC(manager, topRdyDD);
      Cudd_RecursiveDerefZdd(manager, topRdyDD);
  
      // Intersect our {univ,rdy} with {din,rdy}
      DdNode * topDinRdyDD = Cudd_zddIntersect(manager, dinrdyDD, topUnivRdyDD);
      Cudd_Ref(topDinRdyDD);
      Cudd_RecursiveDerefZdd(manager, topUnivRdyDD);      

      // Create a {DIN,empty} set
      printf("Old {DIN,RDY} Size:%u\n", Cudd_zddCount(manager, dinrdyDD));
      DdNode * tmpDinRdyDD = adamant_zdd_QuickDeadFilter(manager, dinrdyDD, dindinDD);

      // Add the top {DIN,RDY} set back in, because they do not really count as dead
      DdNode * newDinRdyDD = Cudd_zddUnion(manager, tmpDinRdyDD, topDinRdyDD);
      Cudd_Ref(newDinRdyDD);
      Cudd_RecursiveDerefZdd(manager, tmpDinRdyDD); 
      Cudd_RecursiveDerefZdd(manager, topDinRdyDD); 

      printf("New {DIN,RDY} Size:%u\n", Cudd_zddCount(manager, newDinRdyDD));

      DdNode * finalDD = newDinRdyDD;
      
      // Save out the results
      

      if(fileVec[2] != NULL)
	{
	  g_print("Saving to file:%s\n", fileVec[2]);
	  
	  FILE * outFh = fopen(fileVec[2], "w+");
	  //! add extra information to this ZDD dump
	  g_snprintf(headerInfo.extraTraceInfo,512, "type:dinrdy,dinstart:0,dinstop:0");
	  
	  Dddmp_cuddZddStore(manager, NULL,
			     finalDD, NULL, &zdd_ordr,
			     DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			     fileVec[2], outFh);
	  fclose(outFh);
	}
    }
    
    g_free(headerInfo.extraTraceInfo);
    g_string_free(ddFiles, TRUE);
    g_strfreev(fileVec);

    return (0);
}

int adamant_zddtest_dinsinslice(GString * dinsinFiles)
{
    DdNode * dinsinDD = NULL;
    DdNode * dindinDD = NULL;
    int zdd_ordr[ZDDNUM];
    guint64 * topDin = g_new(guint64, 1);
    guint64 * topDinDin = g_new(guint64, 1);
    guint64 * topSin = g_new(guint64, 1);
    *topDin = 0;
    *topSin = 0;
    *topDinDin = 0;
    guint64 * bottomDin = g_new(guint64, 1);
    guint64 * bottomSin = g_new(guint64, 1);
    *bottomDin = 0;
    *bottomSin = 0;

    Dddmp_MoreDDHeaderInfo headerInfo;
    Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
    headerInfo.extraTraceInfo = g_new(char, 512);

    //! Setup the initial manager
    DdManager * manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    gchar ** fileVec = g_strsplit(dinsinFiles->str, "," , 3);

    if (fileVec[0] != NULL)
    {
        g_print("Using {DIN,SIN} set from file:%s\n", fileVec[0]);
        FILE * fh = fopen(fileVec[0], "r+");
        dinsinDD = Dddmp_cuddZddLoad(manager,
					     DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					     DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					     fileVec[0], fh);

        Cudd_Ref(dinsinDD);
        fclose(fh);
    }

    // Read in the {DIN,{DIN}} tuple set
    if (fileVec[1] != NULL)
    {
        g_print("Using {DIN,{DIN}} file:%s\n", fileVec[1]);

        FILE * fh = fopen(fileVec[1], "r+");
        dindinDD = Dddmp_cuddZddLoad(manager, DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					      DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					      fileVec[1], fh);
	Cudd_Ref(dindinDD);
        fclose(fh);
    }

    if((dindinDD != NULL) && (dinsinDD != NULL))
    {
      
      adamant_zdd_GetTupleTop(manager, dindinDD, topDin, topDinDin);
      g_print("{DIN,{DIN}} top DIN:%u, top {DIN}:%u\n", *topDin, *topDinDin);

      guint64 dinSliceNum = 0;

      DdNode * zero = Cudd_ReadZero(manager);
      Cudd_Ref(zero);
      DdNode * sinOnlyDD = zero;

      while (dinSliceNum < (*topDinDin))
	{
	  // Build a tuple that represents our din value time
	  DdNode * dinonlyDD = adamant_zdd_build_tuple(manager, zero, dinSliceNum, 0);
	  
	  // Get the full reverse slice of that din in the {DIN,{DIN}} set
	  DdNode * dinSliceDD = adamant_zdd_iterReverse_slice(manager, dinonlyDD,
						  dindinDD, 0);
	  Cudd_RecursiveDerefZdd(manager, dinonlyDD);
	  
	  // Create a {DIN, univ} set
	  DdNode * tmpSliceNode = adamant_zdd_yDC(manager, dinSliceDD);
	  Cudd_RecursiveDerefZdd(manager, dinSliceDD);

	  // Intersect our {DIN, univ} with {din,sin}
	  DdNode * dinIntersectNode = Cudd_zddIntersect(manager, dinsinDD, tmpSliceNode);
	  Cudd_Ref(dinIntersectNode);
	  Cudd_RecursiveDerefZdd(manager, tmpSliceNode);
	  
	  // Get rid of the DINs
	  DdNode * tmpSinOnlyDD = adamant_zdd_abstractX(manager, dinIntersectNode);
	  Cudd_RecursiveDerefZdd(manager, dinIntersectNode);

	  // Difference our {DIN, SIN} with our running {DIN,SIN} tally set
	  DdNode * sinIntersectNode = Cudd_zddIntersect(manager, sinOnlyDD, tmpSinOnlyDD);
	  Cudd_Ref(sinIntersectNode); 

	  int sinSize = Cudd_zddCount(manager, tmpSinOnlyDD);
	  int intersectSize = Cudd_zddCount(manager, sinIntersectNode);
	  Cudd_RecursiveDerefZdd(manager, sinIntersectNode);
	  
	  g_print("%d,%d,%d\n", dinSliceNum, sinSize, intersectSize);

	  DdNode * tmpUnion = Cudd_zddUnion(manager, tmpSinOnlyDD, sinOnlyDD);
	  Cudd_Ref(tmpUnion);
	  Cudd_RecursiveDerefZdd(manager, tmpSinOnlyDD);
	  Cudd_RecursiveDerefZdd(manager, sinOnlyDD);
	  sinOnlyDD = tmpUnion;	 
	  dinSliceNum = dinSliceNum + 1024; 
	}

      Cudd_RecursiveDerefZdd(manager, zero); 
    }

    /* if(fileVec[2] != NULL) */
    /* { */
    /*     g_print("Saving slice to file:%s\n", fileVec[2]); */

    /*     FILE * outFh = fopen(fileVec[2], "w+"); */
    /*     //! add extra information to this ZDD dump */
    /*     g_snprintf(headerInfo.extraTraceInfo,512, "type:dindin,dinstart:0,dinstop:0"); */

    /*     Dddmp_cuddZddStore(manager, NULL, */
    /* 			   sliceDD, NULL, &zdd_ordr, */
    /* 			   DDDMP_MODE_BINARY, extrainfo, &headerInfo, */
    /* 			   fileVec[2], outFh); */
    /*     fclose(outFh); */
    /* } */

    //    g_free(headerInfo.extraTraceInfo);
    g_string_free(dinsinFiles, TRUE);
    g_strfreev(fileVec);
    g_free(topDin);
    g_free(topDinDin);
    g_free(topSin);
    g_free(bottomDin);
    g_free(bottomSin);

    return (0);
}

int adamant_zddtest_dintop(GString * dinrdyFiles)
{
    int zdd_ordr[ZDDNUM];
    guint64 * topDin = g_new(guint64, 1);
    guint64 * topRdy = g_new(guint64, 1);
    *topDin = 0;
    *topRdy = 0;
    guint64 * bottomDin = g_new(guint64, 1);
    guint64 * bottomRdy = g_new(guint64, 1);
    *bottomDin = 0;
    *bottomRdy = 0;

    Dddmp_MoreDDHeaderInfo headerInfo;
    Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
    headerInfo.extraTraceInfo = g_new(char, 512);

    //! Setup the initial manager
    DdManager * manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
    gchar ** fileVec = g_strsplit(dinrdyFiles->str, "," , 3);

    if (fileVec[0] != NULL)
    {
        g_print("Using {DIN,X} set from file:%s\n", fileVec[0]);
        FILE * fh = fopen(fileVec[0], "r+");
        DdNode * dinXxDD = Dddmp_cuddZddLoad(manager,
					     DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					     DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					     fileVec[0], fh);
	
        Cudd_Ref(dinXxDD);
        fclose(fh);
	
	const guint64 zddNumHalf = ZDDNUM/2;
	guint i;
	int xlist [ZDDNUM/2];
	int ylist [ZDDNUM/2];
	
	//! gather a list of ZDD node variables
	for (i = 0; i < zddNumHalf; i+=1)
	  {
	    xlist[(zddNumHalf - 1) - i] = i;
	    ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
	  }
	
	/* //! find the ZDD for the selected area */
	DdNode * dinBound =  mg_Cudd_zddUb(manager, zddNumHalf, xlist, (uint64_t)(250000000));
	Cudd_Ref(dinBound);
	
	// Add in the universal set of Y values
	DdNode * tmpSliceNode = adamant_zdd_yDC(manager, dinBound);
	Cudd_RecursiveDerefZdd(manager, dinBound);
	
	// Intersect our {din,univ} with {din,X}
	DdNode * intersectNode = Cudd_zddIntersect(manager, dinXxDD, tmpSliceNode);
	Cudd_Ref(intersectNode);
	Cudd_RecursiveDerefZdd(manager, tmpSliceNode); 
	Cudd_RecursiveDerefZdd(manager, dinXxDD);
	
	if(fileVec[1] != NULL)
	  {
	    g_print("Saving to file:%s\n", fileVec[1]);
	    
	    FILE * outFh = fopen(fileVec[1], "w+");
	    //! add extra information to this ZDD dump
	    g_snprintf(headerInfo.extraTraceInfo,512, "type:dinsin,dinstart:0,dinstop:250000000,");
	    
	    Dddmp_cuddZddStore(manager, NULL,
			       intersectNode, NULL, &zdd_ordr,
			       DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			       fileVec[1], outFh);
	    fclose(outFh);
	  }
    }

    g_free(headerInfo.extraTraceInfo);
    g_string_free(dinrdyFiles, TRUE);
    g_strfreev(fileVec);
    g_free(topDin);
    g_free(topRdy);
    g_free(bottomDin);
    g_free(bottomRdy);

    return (0);
}

int adamant_zddtest_rdydinslice(GString * dinrdyFiles)
{
    DdNode * dinrdyDD = NULL;
    DdNode * dindinDD = NULL;
    DdNode * sliceDD = NULL;
    DdNode * dinonlyDD = NULL;
    int zdd_ordr[ZDDNUM];
    guint64 * topDin = g_new(guint64, 1);
    guint64 * topRdy = g_new(guint64, 1);
    *topDin = 0;
    *topRdy = 0;
    guint64 * bottomDin = g_new(guint64, 1);
    guint64 * bottomRdy = g_new(guint64, 1);
    *bottomDin = 0;
    *bottomRdy = 0;

    Dddmp_MoreDDHeaderInfo headerInfo;
    Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
    headerInfo.extraTraceInfo = g_new(char, 512);

    //! Setup the initial manager
    DdManager * manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS * 300, CUDD_CACHE_SLOTS, 0);
    gchar ** fileVec = g_strsplit(dinrdyFiles->str, "," , 3);

    if (fileVec[0] != NULL)
    {
        g_print("Using {DIN,RDY} set from file:%s\n", fileVec[0]);
        FILE * fh = fopen(fileVec[0], "r+");
        dinrdyDD = Dddmp_cuddZddLoad(manager,
					     DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					     DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					     fileVec[0], fh);

        Cudd_Ref(dinrdyDD);
        fclose(fh);
    }

    if(dinrdyDD != NULL)
    {
      // First get the last rdy value
      adamant_zdd_GetTupleTop(manager, dinrdyDD, topDin, topRdy);
      g_print("{DIN,RDY} top DIN:%u, top RDY:%u\n", *topDin, *topRdy);

      adamant_zdd_GetTupleBottom(manager, dinrdyDD, bottomDin, bottomRdy);
      g_print("{DIN,RDY} bottom DIN:%u, bottom RDY:%u\n", *bottomDin, *bottomRdy);

      //      adamant_zdd_GetTupleTop2(manager, dinrdyDD, topDin, topRdy);
      //      g_print("Using rdy time2:%u\n", *topRdy);

      const guint64 zddNumHalf = ZDDNUM/2;
      guint i;
      int xlist [ZDDNUM/2];
      int ylist [ZDDNUM/2];
    
      //! gather a list of ZDD node variables
      for (i = 0; i < zddNumHalf; i+=1)
	{
	  xlist[(zddNumHalf - 1) - i] = i;
	  ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
	}
      
      /* //! find the ZDD for the selected area */
      /* DdNode * xUb =  mg_Cudd_zddUb(manager, zddNumHalf, xlist, (uint64_t)x_max); */
      /* Cudd_Ref(xUb); */
      /* //  DdSize(manager, xUb); */
      /* DdNode * xLb =  mg_Cudd_zddLb(manager, zddNumHalf, xlist, (uint64_t)x_min); */
      /* Cudd_Ref(xLb); */

      //! find the ZDD for the selected area
      /* DdNode * yUb =  mg_Cudd_zddUb(manager, zddNumHalf, ylist, (uint64_t)y_max); */
      /* Cudd_Ref(yUb); */
      /* //  DdSize(manager, yUb); */
      //      DdNode * rdyNode =  mg_Cudd_zddLb(manager, zddNumHalf, ylist, (uint64_t)((*topRdy) - 
      //                                             rdyMinus));
//      Cudd_Ref(rdyNode);
      
      DdNode * zero = Cudd_ReadZero(manager);
      Cudd_Ref(zero);

      // Build a tuple that represents our ready time 
      DdNode * rdyNode = adamant_zdd_build_tuple(manager, zero, 0, *topRdy);
      Cudd_RecursiveDerefZdd(manager, zero);

      // Add in the universal set of X values
      DdNode * tmpSliceNode = adamant_zdd_xDC(manager, rdyNode);
      Cudd_RecursiveDerefZdd(manager, rdyNode);

      // Intersect our {univ,rdy} with {din,rdy}
      DdNode * intersectNode = Cudd_zddIntersect(manager, dinrdyDD, tmpSliceNode);
      Cudd_Ref(intersectNode);
      Cudd_RecursiveDerefZdd(manager, tmpSliceNode);

      // Now we have the DINs of interest
      dinonlyDD = adamant_zdd_abstractY(manager, intersectNode);
      Cudd_RecursiveDerefZdd(manager, intersectNode);

      //      // Ditch the original {DIN,RDY} set
      //      Cudd_RecursiveDerefZdd(manager, dinrdyDD);
    }

    // Read in the {DIN,{DIN}} tuple set
    if (fileVec[1] != NULL)
    {
        g_print("Using {DIN,{DIN}} file:%s\n", fileVec[1]);

        FILE * fh = fopen(fileVec[1], "r+");
        dindinDD = Dddmp_cuddZddLoad(manager, DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					      DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					      fileVec[1], fh);
	Cudd_Ref(dindinDD);
        fclose(fh);
    }

    if((dindinDD != NULL) && (dinonlyDD != NULL))
    {
      g_print("Starting slice...\n");
      sliceDD = adamant_zdd_iterReverse_slice(manager, dinonlyDD, dindinDD, 0);
      g_print("Slice Complete\n");
    }

    if(fileVec[2] != NULL)
      {
	// Add in the universal set of y values
	DdNode * tmpDinUnvNode = adamant_zdd_yDC(manager, sliceDD);
	Cudd_RecursiveDerefZdd(manager, sliceDD);
	
	// Intersect our {din,univ} with {din,rdy}
	DdNode * newDinRdyDD = Cudd_zddIntersect(manager, dinrdyDD, tmpDinUnvNode);
	Cudd_Ref(newDinRdyDD);
	Cudd_RecursiveDerefZdd(manager, tmpDinUnvNode);

	// First get the last rdy value
	adamant_zdd_GetTupleTop(manager, newDinRdyDD, topDin, topRdy);
	g_print("New {DIN,RDY} top DIN:%"G_GUINT64_FORMAT", top RDY:%"G_GUINT64_FORMAT"\n", *topDin, *topRdy);
	
	adamant_zdd_GetTupleBottom(manager, newDinRdyDD, bottomDin, bottomRdy);
	g_print("New {DIN,RDY} bottom DIN:%"G_GUINT64_FORMAT", bottom RDY:%"G_GUINT64_FORMAT"\n", *bottomDin, *bottomRdy);
	
        g_print("Saving slice to file:%s\n", fileVec[2]);

        FILE * outFh = fopen(fileVec[2], "w+");

        //! add extra information to this ZDD dump
        g_snprintf(headerInfo.extraTraceInfo,512, 
		   "type:dinrdy,dinstart:%"G_GUINT64_FORMAT",dinstop:%"G_GUINT64_FORMAT"", *bottomDin, *topDin);

        Dddmp_cuddZddStore(manager, NULL,
			   newDinRdyDD, NULL, &zdd_ordr,
			   DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			   fileVec[2], outFh);
        fclose(outFh);
    }

    g_free(headerInfo.extraTraceInfo);
    g_string_free(dinrdyFiles, TRUE);
    g_strfreev(fileVec);
    g_free(topDin);
    g_free(topRdy);
    g_free(bottomDin);
    g_free(bottomRdy);

    return (0);
}

int adamant_zddtest_interval()
{

  int zdd_ordr[ZDDNUM];
  int xlist[ZDDNUM/2];
  int ylist[ZDDNUM/2];
  unsigned int i;

  //! gather a list of BDD node variables
  for (i = 0; i < (ZDDNUM/2); i+=1)
    {
      xlist[( (ZDDNUM/2) - 1) - i] = i;
      ylist[( (ZDDNUM/2) - 1) - i] = i + (ZDDNUM/2);
    }

  //! Setup the initial manager and node
  DdManager * manager = adamant_zddtest_initmanager(zdd_ordr);

  // Make the test zdd
  DdNode * testNode = Cudd_ReadZero(manager);
  Cudd_Ref(testNode);

  DdNode * tmpNode = adamant_zdd_build_tuple(manager, testNode, 200, 2);
  Cudd_RecursiveDerefZdd(manager,testNode);
  testNode = tmpNode;

  tmpNode = adamant_zdd_build_tuple(manager, testNode, 120, 5);
  Cudd_RecursiveDerefZdd(manager,testNode);
  testNode = tmpNode;

  tmpNode = adamant_zdd_build_tuple(manager, testNode, 250, 7);
  Cudd_RecursiveDerefZdd(manager,testNode);
  testNode = tmpNode;

  DdNode * mainNode = testNode;

  adamant_zddtest_printdd(manager, mainNode, "/tmp/ddtestmainNode.dot");
  DdNode * sliceNode = mainNode;

  // Remove the X variables
  DdNode * tmp = adamant_zdd_abstractX( manager, mainNode);
  // mainNode = tmp;

  DdNode * intervalX = mg_Cudd_zddInterval(manager, (ZDDNUM/2), xlist, (guint64) 199, (guint64) 201);
  Cudd_Ref(intervalX);
  adamant_zddtest_printdd(manager, intervalX, "/tmp/ddtestIntervalX.dot");

  DdNode * intervalY = mg_Cudd_zddInterval(manager, (ZDDNUM/2), ylist, (guint64) 4, (guint64) 25);
  Cudd_Ref(intervalY);
  //  tmp = adamant_zdd_xDC( manager, intervalY);
  //  intervalY = tmp;
  adamant_zddtest_printdd(manager, intervalY, "/tmp/ddtestIntervalY.dot");

  DdNode * intervalProd = Cudd_zddProduct(manager, intervalX, intervalY);
  Cudd_Ref(intervalProd);
  adamant_zddtest_printdd(manager, intervalProd, "/tmp/ddtestIntervalProd.dot");

  tmp = Cudd_zddIntersect(manager, intervalProd, sliceNode);
  Cudd_Ref(tmp);
  //  Cudd_RecursiveDerefZdd(manager,mainNode);
  mainNode = tmp;
  adamant_zddtest_printdd(manager, mainNode, "/tmp/ddtestInterFinal.dot");

  return(0);
}


int adamant_zddtest_sliceiter()
{
  int zdd_ordr[ZDDNUM];
  struct timeval timecount;
  const int slice_count = 1000;

  //! Setup the initial manager and node
  DdManager * manager = adamant_zddtest_initmanager(zdd_ordr);
  //  DdNode * mainNode = adamant_zddtest_initSliceNode(manager);
  //  adamant_zddtest_printdd(manager, mainNode, "/tmp/ddtestsliceNode.dot");

  // Make the zero zdd
  DdNode * zeroNode = Cudd_ReadZero(manager);
  Cudd_Ref(zeroNode);

  // Build slice test tuple (5,4)
  DdNode * sliceNode = adamant_zdd_build_tuple(manager, zeroNode, 5, 4);
  adamant_zddtest_printdd(manager, sliceNode, "/tmp/ddtestsliceTuple.dot");

  // Build the target tuple

  DdNode * tmpNode = zeroNode;
  DdNode * targetNode = zeroNode;
  DdNode * cmpNode = adamant_zdd_build_tuple(manager, zeroNode, 4, 0);

  int y = 4, i = 0;

  srand(time(NULL));

  for (i = 0; i < slice_count; i++)
    {
      int x = y;
      y = rand();


      tmpNode = adamant_zdd_build_tuple(manager, targetNode, x, y);
      Cudd_RecursiveDerefZdd(manager,targetNode);
      targetNode = tmpNode;


      tmpNode = adamant_zdd_build_tuple(manager, cmpNode, y, 0);
      Cudd_RecursiveDerefZdd(manager,cmpNode);
      cmpNode = tmpNode;
    }

  // Remove the X variables
  DdNode * tmp = adamant_zdd_abstractX( manager, sliceNode);
  Cudd_RecursiveDerefZdd(manager,sliceNode);
  sliceNode = tmp;

  adamant_zddtest_printdd(manager, sliceNode, "/tmp/ddtestsliceInitAb.dot");

  // Swap node variables
  tmp = adamant_zdd_varswap( manager, sliceNode);
  Cudd_RecursiveDerefZdd(manager,sliceNode);
  sliceNode = tmp;
  adamant_zddtest_printdd(manager, sliceNode, "/tmp/ddtestsliceInitSwap.dot");

  g_print("Begin Slice\n");

  // Begin the slice
  int count = 0;
  DdNode * oldNode = NULL;

  //! grab the time when we startup
  gettimeofday(&timecount, NULL);
  unsigned int zdd_first_time = (guint64)(timecount.tv_sec);

  while ((count < (slice_count * 10)) && (sliceNode != oldNode))
    {
      oldNode = sliceNode;

      tmp = adamant_zdd_reverse_sliceITE(manager, sliceNode, targetNode);
      Cudd_RecursiveDerefZdd(manager,sliceNode);
      sliceNode = tmp;

      ++count;
    }

  if(cmpNode != sliceNode)
    {
      DdNode * diffNode = Cudd_zddDiff(manager, cmpNode, sliceNode);
      adamant_zddtest_printdd(manager, sliceNode, "/tmp/ddtestSliceDiff.dot");
      adamant_zddtest_error("Failure\n");
    }
  else
    {
      g_print("Successful\n");

      //! grab the current time and print
      gettimeofday(&timecount, NULL);
      g_print("SliceTime:%u\n",((unsigned int)timecount.tv_sec - zdd_first_time));
    }

  adamant_zddtest_printdd(manager, sliceNode, "/tmp/ddtestsliceFinal.dot");

  // Cleanup the DdManager
  Cudd_Quit(manager);

  return (0);
}

double adamant_zddtest_reverseSliceMethod(int method, 
					  guint32 randSeed,
					  guint64 slotsMulti, 
					  guint64 freeMemory,
					  gchar ** inputArray)
{
  int zdd_ordr[ZDDNUM];
  int slice_count = 0;
  
  //! Setup the initial manager and node
  DdManager * manager = adamant_zddtest_initmanagerSlots(zdd_ordr, 
							 slotsMulti, 
							 freeMemory);

  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo;
  headerInfo.extraTraceInfo = g_new(char, 512);
  DdNode * dindinDD = NULL;
  DdNode * oldNode = NULL;
  DdNode * sliceNode = Cudd_ReadZero(manager);
  DdNode * tmp = NULL;

  guint inputStringCount = g_strv_length(inputArray);

  GTimer * sliceTimer = g_timer_new();
  GTimer * sliceIterTimer = g_timer_new();
  guint64 startSetCount = 0;
  gchar * outFileNameBase = NULL;
  gulong microseconds;
  guint64 count = 0;

  if( 0 < inputStringCount)
    {
      g_print("Using {DIN,DIN} set from file:%s\n", inputArray[0]);
      FILE * fh = fopen(inputArray[0], "r+");
      dindinDD = Dddmp_cuddZddLoad(manager,
				   DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				   DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				   inputArray[0], fh);
      
      Cudd_Ref(dindinDD);
      fclose(fh);	
    }
  if( 1 < inputStringCount )
    {
      startSetCount = g_ascii_strtoull(inputArray[1], NULL, 10);

      g_print("Building start DIN set with %u instructions\n", 
	      (unsigned int) startSetCount);

      GRand * randGen = g_rand_new_with_seed (randSeed);
      guint64 setI = 0;
      for( setI = 0; setI < startSetCount; ++setI)
	{
	  gint32 randDIN = g_rand_int_range (randGen,
					     0,
					     1000000000);
	  
	  DdNode * tempNode = adamant_zdd_build_tuple(manager, 
						      sliceNode, 
						      randDIN, 
						      0);
	  Cudd_RecursiveDerefZdd(manager, sliceNode);
	  sliceNode = tempNode;
	}
    }

  if( 2 < inputStringCount )
    {
      slice_count = g_ascii_strtoull(inputArray[2], NULL, 10);
    }
 
  if( 3 < inputStringCount )
    {
      outFileNameBase = g_strdup(inputArray[3]);
    }

  if(SLINTERSECT == method)
    {
      g_print("Starting reverse slice with Intersect");
    }
  else if(SLITE == method)
    {
      g_print("Starting reverse slice with ITE");
    }
  else if(SLUP == method)
    {
      g_print("Starting reverse slice with UP");
    }
  g_print(": Max Count:%u\n", 
	  (unsigned int) slice_count);

  g_timer_start(sliceTimer);
  g_timer_start(sliceIterTimer);
  g_timer_stop(sliceTimer);
  g_timer_stop(sliceIterTimer);
  g_timer_reset(sliceTimer);
  g_timer_reset(sliceIterTimer);  

  while ((sliceNode != oldNode) && 
	 ((0 == slice_count) || (slice_count > count)))
    {
      oldNode = sliceNode;
      
      g_timer_continue(sliceTimer);
      g_timer_continue(sliceIterTimer);

      if(0 == method)
	{
	  tmp = adamant_zdd_reverse_slice(manager, sliceNode, dindinDD);
	}
      else if(1 == method)
	{
	  tmp = adamant_zdd_reverse_sliceITE(manager, sliceNode, dindinDD);
	}
      else if(2 == method)
	{
	  tmp = adamant_zdd_reverse_sliceUP(manager, sliceNode, dindinDD);
	}

      Cudd_RecursiveDerefZdd(manager, sliceNode);
      sliceNode = tmp;
      
      g_timer_stop(sliceTimer);
      g_timer_stop(sliceIterTimer);

      ++count;
      
      if ((count % 2) == 0)
        {
	  g_print("%u,%u,%f\n", count, 
		  Cudd_zddCount(manager, sliceNode), 
		  g_timer_elapsed (sliceIterTimer, &microseconds));
	  g_timer_reset(sliceIterTimer);
       }
    }

  if(0 == method)
    {
      g_print("Intersect Slice:");
    }
  else if(1 == method)
    {
      g_print("ITE Slice:");
    }
  else if(2 == method)
    {
      g_print("UP Slice:\n");
    }
  
  double timeForSlice = g_timer_elapsed (sliceTimer, &microseconds);
  
  g_print("%u,%u,%f\n", 
	  count,
	  Cudd_zddCount(manager, sliceNode), 
	  timeForSlice);

  Cudd_Quit(manager);
  g_timer_destroy(sliceTimer);
  g_timer_destroy(sliceIterTimer);

  return (timeForSlice);
}

int adamant_zddtest_reverseSliceMethodTest(guint64 slotsMulti, GString * inputs)
{
  gchar ** orderArr = NULL;
  gchar ** inputArr = g_strsplit(inputs->str, " " , -1);
  
  if(NULL != inputArr[0])
    {
      orderArr = g_strsplit(inputArr[0], "," , -1);      
    }
  
  guint32 randSeed = g_random_int();
  guint64 freeMemory = adamant_zdd_freemem();
  g_print("RandSeed:%u\n", randSeed);

  if (NULL != orderArr)
    {
      int i = 0;
      for (i = 0; i < g_strv_length(orderArr); ++i)
	{
	  guint64 sliceType = g_ascii_strtoull(orderArr[i], NULL, 10);
	  adamant_zddtest_reverseSliceMethod((int)sliceType, randSeed, 
					     slotsMulti, freeMemory, &inputArr[1]);
	}
    }
  else
    {
      adamant_zddtest_reverseSliceMethod(0, randSeed, slotsMulti, 
					 freeMemory, &inputArr[1]);
      adamant_zddtest_reverseSliceMethod(1, randSeed, slotsMulti, 
					 freeMemory, &inputArr[1]);
    }
  

  return(0);
}

int adamant_zddtest_deathSlice(guint64 slotsMulti, GString * ddFiles)
{
  DdNode * dinrdyAddBackDD = NULL;
  DdNode * dindinDD = NULL;
  DdNode * dinsysDD = NULL;
  DdNode * addBackDD = NULL;
  int zdd_ordr[ZDDNUM];
  
  DdManager * manager = NULL;
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */
  headerInfo.extraTraceInfo = g_new(char, 512);
  gchar * finalDdFileName = NULL; 
  gchar * dinrdyFileName = NULL;
  gchar ** fileVec = g_strsplit(ddFiles->str, " " , 5);
  guint64 freemem = adamant_zdd_freemem();
  freemem = freemem - (freemem / 10);
  manager = adamant_zddtest_initmanagerSlots(zdd_ordr, slotsMulti, 0);
  
  int i = 0;
  while (NULL != fileVec[i])
    {
      if (NULL != g_strrstr(fileVec[i], "dinvssys"))
	{
	  g_print("Using {DIN,SYS} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  dinsysDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  
	  Cudd_Ref(dinsysDD);
	  fclose(fh);
	}
      else if (NULL != g_strrstr(fileVec[i], "dinvsdin_output"))
	{
	  finalDdFileName = g_strdup(fileVec[i]);
	  g_print("Saving DD results to file:%s\n", finalDdFileName);
	} 
      else if ((NULL != g_strrstr(fileVec[i], "dindin")) || 
	       (NULL != g_strrstr(fileVec[i], "dinvsdin")))
	{
	  g_print("Using {DIN,DIN} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  dindinDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);
	  
	  Cudd_Ref(dindinDD);
	  fclose(fh);	  
	}
      else if ((NULL != g_strrstr(fileVec[i], "dinvsrdy")) || 
	       (NULL != g_strrstr(fileVec[i], "dinrdy")))
	{
	  g_print("Using {DIN,RDY} set from file:%s\n", fileVec[i]);
	  FILE * fh = fopen(fileVec[i], "r+");
	  DdNode * dinrdyDD = Dddmp_cuddZddLoad(manager,
				       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
				       DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
				       fileVec[i], fh);

	  Cudd_Ref(dinrdyDD);
	  fclose(fh);

	  guint64 xTop = 0;
	  guint64 yTop = 0;

	  adamant_zdd_GetTupleTop(manager, dinrdyDD, &xTop, &yTop);

	  // START DEBUG
	  printf("Top Din:%d, Rdy:%d\n", xTop, yTop);
	  // END DEBUG
	  
	  DdNode * zero = Cudd_ReadZero(manager);
	  Cudd_Ref(zero);
	  
	  // Build a tuple that represents our top ready time 
	  DdNode * rdyNode = adamant_zdd_build_tuple(manager, zero, 0, yTop); 
	  Cudd_RecursiveDerefZdd(manager, zero); 
	  
	  // Add in the universal set of X values
	  DdNode * tmpSliceNode = adamant_zdd_xDC(manager, rdyNode);
	  Cudd_RecursiveDerefZdd(manager, rdyNode);
  
	  // Intersect our {univ,rdy} with {din,rdy}
	  DdNode * intersectNode = Cudd_zddIntersect(manager, dinrdyDD, tmpSliceNode);
	  Cudd_Ref(intersectNode);
	  Cudd_RecursiveDerefZdd(manager, tmpSliceNode);
	  Cudd_RecursiveDerefZdd(manager, dinrdyDD);
	  
	  // Abstract away the rdy
	  dinrdyAddBackDD = adamant_zdd_abstractY(manager, intersectNode);
	  Cudd_RecursiveDerefZdd(manager, intersectNode);

	  // Free up the {DIN,RDY} memory
	  cuddGarbageCollect(manager, 1);

	  // START DEBUG
	  //	  g_print("Ref Check 3\n");
	  //	  Cudd_CheckKeys(manager);
	  // END DEBUG
	}

      // START DEBUG
      //      g_print("Ref Check 1\n");
      //      Cudd_CheckKeys(manager);
      // END DEBUG

      ++i;
    }

  g_free(headerInfo.extraTraceInfo);
  
  if(NULL != dinsysDD)
    {
      DdNode * tempDD = NULL;

      // write syscall is 1 in trace
      addBackDD = adamant_zdd_build_tuple(manager, Cudd_ReadZero(manager), 0, 1);

      // pwrite64 syscall is 18 in trace
      tempDD = adamant_zdd_build_tuple(manager, addBackDD, 0, 18);
      Cudd_RecursiveDerefZdd(manager, addBackDD);
      addBackDD = tempDD;

      // sys_msgsnd syscall is 69 in trace
      tempDD = adamant_zdd_build_tuple(manager, addBackDD, 0, 69);
      Cudd_RecursiveDerefZdd(manager, addBackDD);
      addBackDD = tempDD;

      // Final add back DD addition
      tempDD = adamant_zdd_abstractX(manager, addBackDD);
      Cudd_RecursiveDerefZdd(manager, addBackDD);
      addBackDD = tempDD;
      tempDD = adamant_zdd_xDC(manager, addBackDD);
      Cudd_RecursiveDerefZdd(manager, addBackDD);
      addBackDD = tempDD;
      tempDD = Cudd_zddIntersect(manager, dinsysDD, addBackDD);
      Cudd_Ref(tempDD);
      Cudd_RecursiveDerefZdd(manager, addBackDD);
      addBackDD = tempDD;
      tempDD = adamant_zdd_abstractY(manager, addBackDD);
      Cudd_RecursiveDerefZdd(manager, addBackDD);
      addBackDD = tempDD;

      // Output some DEBUG information
      g_print("Add Back DD Size:%u\n", 
	      Cudd_zddCount(manager, addBackDD));
    }



  if(NULL != dinrdyAddBackDD)
    {
      if(NULL == addBackDD)
	{
	  addBackDD = Cudd_ReadZero(manager);
	  Cudd_Ref(addBackDD);
	}

      DdNode * tempDD = Cudd_zddUnion(manager, dinrdyAddBackDD, addBackDD);
      Cudd_Ref(tempDD);
      Cudd_RecursiveDerefZdd(manager, addBackDD);
      Cudd_RecursiveDerefZdd(manager, dinrdyAddBackDD);
      addBackDD = tempDD;

      // Output some DEBUG information      
      g_print("Add Back DD Size with RDY:%u\n", 
	      Cudd_zddCount(manager, addBackDD));
    }

  
  if(NULL != dindinDD)
    {
      // Clean up before slicing
      cuddGarbageCollect(manager, 1);
      
      //

      //      g_print("DdInitialSize:%u\n", Cudd_zddCount(manager, dindinDD));
      
      DdNode * olddindinDD = NULL;
      const int topCount = 1;
      int iterCount = 0;
      while ((olddindinDD != dindinDD) &&
	     (topCount > iterCount))
	{      
	  olddindinDD = dindinDD;


	  // START DEBUG
	  //	  g_print("Ref Check 2\n");
	  //	  Cudd_CheckKeys(manager);
	  // END DEBUG

	  // Perform the Ninja Slice of Death!
	  dindinDD = adamant_zdd_DeadSlicer(manager,
					    olddindinDD,
					    addBackDD,
					    (100 * topCount));

	  // NOTE: The DeadSlicer dereferences the dindinDD, so we don't need too

	  // Clean up before saving
	  cuddGarbageCollect(manager, 1);
      
	  // Save out the results
	  if(NULL != finalDdFileName)
	    {
	      g_print("Saving to file:%s\n", finalDdFileName);
	      
	      DdNode * finalDD = dindinDD;
	      
	      FILE * outFh = fopen(finalDdFileName, "w+");
	      //! add extra information to this ZDD dump

	      guint64 xTop = 0;
	      guint64 yTop = 0;
	      
	      adamant_zdd_GetTupleTop(manager, dindinDD, &xTop, &yTop);

	      headerInfo.extraTraceInfo = g_strdup_printf("type:dindin,dinstart:0,dinstop:%"G_GUINT64_FORMAT, xTop);
	      
	      Dddmp_cuddZddStore(manager, NULL,
				 finalDD, NULL, &zdd_ordr,
				 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
				 finalDdFileName, outFh);	     
	      fclose(outFh);
	      g_free(headerInfo.extraTraceInfo);
	      ++iterCount;
	    }
	}	      
      g_print("DdFinalSize:%u\n", Cudd_zddCount(manager, dindinDD));
    }
  
  // Cleanup the DdManager
  Cudd_Quit(manager);

  g_string_free(ddFiles, TRUE);
  g_strfreev(fileVec);
  
  return (0);
}

int adamant_zddtest_deathSliceTest()
{
  int zdd_ordr[ZDDNUM];

  //! Setup the initial manager and node
  DdManager * manager = adamant_zddtest_initmanager(zdd_ordr);
  DdNode * mainNode = adamant_zddtest_initSliceNode(manager);
  adamant_zddtest_printdd(manager, mainNode, "/tmp/ddtestsliceNode.dot");

  DdNode * addBackDD = adamant_zdd_build_tuple(manager, Cudd_ReadZero(manager), 3, 0);
  DdNode * tempDD = adamant_zdd_abstractY(manager, addBackDD);
  Cudd_RecursiveDerefZdd(manager, addBackDD);
  addBackDD = tempDD;
      
  DdNode * deadNode = adamant_zdd_DeadSlicer(manager, 
					     mainNode,
					     addBackDD,
					     0);
  adamant_zddtest_printdd(manager, deadNode, "/tmp/ddDeathSliceConverge.dot");

  // Cleanup the DdManager
  Cudd_Quit(manager);
  
  return (0);
}

int adamant_zddtest_zddslice()
{
  int zdd_ordr[ZDDNUM];

  //! Setup the initial manager and node
  DdManager * manager = adamant_zddtest_initmanager(zdd_ordr);
  DdNode * mainNode = adamant_zddtest_initSliceNode(manager);
  adamant_zddtest_printdd(manager, mainNode, "/tmp/ddtestsliceNode.dot");

  // Make the zero zdd
  DdNode * zeroNode = Cudd_ReadZero(manager);
  Cudd_Ref(zeroNode);

  // Build slice test tuple (4,3)
  DdNode * sliceNode = adamant_zdd_build_tuple(manager, zeroNode, 4, 3);
  adamant_zddtest_printdd(manager, sliceNode, "/tmp/ddtestsliceTuple.dot");

  // Build slice cubes
  DdNode * yCubeNode = adamant_zdd_build_tuple(manager, zeroNode, 0, 0xffffffffffffffffLL);
  adamant_zddtest_printdd(manager, yCubeNode, "/tmp/ddtestsliceyCube.dot");

  DdNode * xFullCubeNode = adamant_zdd_build_fullcube(manager, 0xffffffffffffffffLL, 0);
  adamant_zddtest_printdd(manager, xFullCubeNode, "/tmp/ddtestslicexFullCube.dot");

  DdNode * xCubeNode = adamant_zdd_build_tuple(manager, zeroNode, 0xffffffffffffffffLL, 0);

  // Create an abstraction of this BDD that removes the 1
  DdNode * abstractNode = Extra_zddExistAbstract(manager,sliceNode,yCubeNode);
  Cudd_Ref(abstractNode);
  adamant_zddtest_printdd(manager, abstractNode, "/tmp/ddtestsliceAbstract.dot");

  // Swap the vars
  DdNode * swappedNode = adamant_zdd_varswap( manager, abstractNode);
  adamant_zddtest_printdd(manager, swappedNode, "/tmp/ddtestsliceSwapped.dot");

  // Make an node compatible with intersection
  DdNode * unionNode = Cudd_zddUnion( manager, swappedNode, xFullCubeNode);
  Cudd_Ref(unionNode);
  adamant_zddtest_printdd(manager, unionNode, "/tmp/ddtestsliceUnion.dot");

  DdNode * productNode = Cudd_zddUnateProduct( manager, swappedNode, xFullCubeNode);
  Cudd_Ref(productNode);
  adamant_zddtest_printdd(manager, productNode, "/tmp/ddtestsliceProduct.dot");

  // Intersect the slice tuple and the test tuple abstraction
  DdNode * intersectNode = Cudd_zddIntersect(manager, mainNode, productNode);
  Cudd_Ref(intersectNode);
  adamant_zddtest_printdd(manager, intersectNode, "/tmp/ddtestsliceInter.dot");

  // Cleanup the DdManager
  Cudd_Quit(manager);

  return (0);
}

DdManager * adamant_zddtest_initmanager(int * zdd_ordr)
{
  int k = 0;

  //! initialize the CUDD ZDD manager
  DdManager * manager =  Cudd_Init(0, ZDDNUM, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

  for(k = 0; k < 64; k++)
    {
      zdd_ordr[127-2*k] = k;
      zdd_ordr[127-(2*k+1)] = (64+k);
    }

  Cudd_zddShuffleHeap(manager, zdd_ordr);

  return (manager);
}

DdManager * adamant_zddtest_initmanagerSlots(int * zdd_ordr, guint64 slotsMulti, guint64 freeMemory)
{
  int k = 0;

  //! initialize the CUDD ZDD manager
 DdManager * manager = Cudd_Init(0, 0, (CUDD_UNIQUE_SLOTS * slotsMulti), 
				 CUDD_CACHE_SLOTS, freeMemory);

  for(k = 0; k < 64; k++)
    {
      zdd_ordr[127-2*k] = k;
      zdd_ordr[127-(2*k+1)] = (64+k);
    }

  Cudd_zddShuffleHeap(manager, zdd_ordr);

  return (manager);
}


DdNode * adamant_zddtest_initnode(DdManager * manager)
{
  // Make the test zdd
  DdNode * test_StoreNode = Cudd_ReadZero(manager);
  Cudd_Ref(test_StoreNode);

  // Add in the first test tuple (1,3)
  DdNode * tmpNode = adamant_zdd_build_tuple(manager, test_StoreNode, 1, 3);
  Cudd_RecursiveDerefZdd(manager,test_StoreNode);
  test_StoreNode = tmpNode;

  // Add in the second test tuple (7,4)
  tmpNode = adamant_zdd_build_tuple(manager, test_StoreNode, 7, 4);
  Cudd_RecursiveDerefZdd(manager,test_StoreNode);
  test_StoreNode = tmpNode;

  return (test_StoreNode);
}

DdNode * adamant_zddtest_initSliceNode(DdManager * manager)
{
  // Make the test zdd
  DdNode * testNode = Cudd_ReadZero(manager);
  Cudd_Ref(testNode);

  // Add in the test tuple (4, 3)
  DdNode * tmpNode = adamant_zdd_build_tuple(manager, testNode, 4, 3);
  Cudd_RecursiveDerefZdd(manager,testNode);
  testNode = tmpNode;

  // Add in the test tuple (4,2)
  tmpNode = adamant_zdd_build_tuple(manager, testNode, 4, 2);
  Cudd_RecursiveDerefZdd(manager,testNode);
  testNode = tmpNode;

  // Add in the test tuple (3,2)
  tmpNode = adamant_zdd_build_tuple(manager, testNode, 3, 2);
  Cudd_RecursiveDerefZdd(manager,testNode);
  testNode = tmpNode;

   // Add in the test tuple (2,1)
  tmpNode = adamant_zdd_build_tuple(manager, testNode, 2, 1);
  Cudd_RecursiveDerefZdd(manager,testNode);
  testNode = tmpNode;

  return (testNode);
}


DdNode * adamant_zddtest_initInvNode(DdManager * manager)
{
  // Make the test zdd
  DdNode * test_StoreNode = Cudd_ReadZero(manager);
  Cudd_Ref(test_StoreNode);

  // Add in the first test tuple (3,1)
  DdNode * tmpNode = adamant_zdd_build_tuple(manager, test_StoreNode, 3, 1);
  Cudd_RecursiveDerefZdd(manager,test_StoreNode);
  test_StoreNode = tmpNode;

  // Add in the second test tuple (4,7)
  tmpNode = adamant_zdd_build_tuple(manager, test_StoreNode, 4, 7);
  Cudd_RecursiveDerefZdd(manager,test_StoreNode);
  test_StoreNode = tmpNode;

  return (test_StoreNode);
}

int adamant_zddtest_zddstore()
{

  int zdd_ordr[ZDDNUM];
  DdManager * manager = adamant_zddtest_initmanager(&zdd_ordr);
  DdNode *ret;
  DdNode *tmp[1];
  FILE *f = NULL;
  int k = 0;
  char testFile[] = "/tmp/testZddStore.dd";
  FILE * testFH = fopen(testFile, "w+");
  Dddmp_MoreDDHeaderInfo headerInfo;
  Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */

  headerInfo.extraTraceInfo = calloc(512,sizeof(char));

  //! add extra information to this ZDD dump
  g_snprintf(headerInfo.extraTraceInfo,512, "type:dinrdy,dinstart:0,dinstop:3");

  DdNode * test_StoreNode = adamant_zddtest_initnode(manager);

  // Print out a dot file of this ZDD before store
  tmp[0]=test_StoreNode;
  f=fopen("/tmp/ddtest01.dot","w+");
  Cudd_zddDumpDot(manager, 1, tmp, NULL, NULL, f);
  fclose(f);

  Dddmp_cuddZddStore(manager, NULL,
		     test_StoreNode, NULL, &zdd_ordr,
		     DDDMP_MODE_BINARY, extrainfo, &headerInfo,
		     testFile, testFH);
  fclose(testFH);
  testFH = fopen(testFile, "r+");
  DdNode * test_LoadNode = Dddmp_cuddZddLoad(manager,
					     DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL,
					     DDDMP_MODE_BINARY, &headerInfo.extraTraceInfo,
					     testFile, testFH);

  // Print out a dot file of this post store
  tmp[0]=test_LoadNode;
  f=fopen("/tmp/ddtest02.dot","w+");
  Cudd_zddDumpDot(manager, 1, tmp, NULL, NULL, f);
  fclose(f);
  free(headerInfo.extraTraceInfo);

  // Compare nodes
  if (test_LoadNode != test_StoreNode)
    {
      adamant_zddtest_error("UNIT TEST ERROR: ZDD load DD does not match store DD");
    }

  // Cleanup the DdManager
  Cudd_Quit(manager);

  return (0);
}

int adamant_zddtest_varswap()
{
  int zdd_ordr[ZDDNUM];
  int zdd_permut[ZDDNUM];
  DdManager * manager = adamant_zddtest_initmanager(zdd_ordr);
  DdNode *ret;
  DdNode *tmp[1];
  FILE *f = NULL;
  int i = 0;
  int k = 0;
  int j = 0;
  DdNode * node_vec1[ZDDNUM/2];
  DdNode * node_vec2[ZDDNUM/2];


  DdNode * test_InitialNode = adamant_zddtest_initnode(manager);
  Cudd_Ref(test_InitialNode);

  // Print out a dot file of this ZDD
  tmp[0]=test_InitialNode;
  f=fopen("/tmp/ddtestvarswapInit.dot","w+");
  Cudd_zddDumpDot(manager, 1, tmp, NULL, NULL, f);
  fclose(f);

  DdNode * test_InitialInvNode = adamant_zddtest_initInvNode(manager);
  Cudd_Ref(test_InitialInvNode);
  tmp[0]=test_InitialInvNode;
  f=fopen("/tmp/ddtestvarswapInitInv.dot","w+");
  Cudd_zddDumpDot(manager, 1, tmp, NULL, NULL, f);
  fclose(f);

  //! generate the node vectors that will be swapped
  /*  for(i=0; i < (ZDDNUM/2); i++)
    {
      node_vec1[i] = mg_Cudd_zddIthVar(manager,i+(ZDDNUM/2));
      Cudd_Ref(node_vec1[i]);
    }
  for(i=0; i < (ZDDNUM/2); i++)
    {
      node_vec2[i] = mg_Cudd_zddIthVar(manager, i);
      Cudd_Ref(node_vec2[i]);
    }
  */
  for(i=0; i < (ZDDNUM/2); i++)
    {
      //      j = Cudd_ReadInvPermZdd(manager, i+(ZDDNUM/2));
      //      k = Cudd_ReadInvPermZdd(manager, i);
      j =(i+(ZDDNUM/2));
      k = i;
      zdd_permut[j] = k;
      zdd_permut[k] = j;
    }

  adamant_zddtest_printorder(manager);

  //! rearrange the variables in this zdd
  DdNode * test_postZDD = Extra_zddPermute(manager, test_InitialNode, zdd_permut);

  //  DdNode * test_postZDD = mg_Extra_zddSwapVariables(manager, test_InitialNode,
  //						    node_vec1, node_vec2, (ZDDNUM/2));

  Cudd_Ref(test_postZDD);

  adamant_zddtest_printorder(manager);

  // Print out a dot file of this post store
  tmp[0]=test_postZDD;
  f=fopen("/tmp/ddtestvarswapPost.dot","w+");
  Cudd_zddDumpDot(manager, 1, tmp, NULL, NULL, f);
  fclose(f);

  // Cleanup the DdManager
  Cudd_Quit(manager);

  return (0);
}

int adamant_zddtest_printorder(DdManager * manager)
{
  int k = 0;
  for(k = 0; k < ZDDNUM; k++)
    {
      g_print("position %d has variable %d\n", k,
              Cudd_ReadInvPermZdd(manager,k));
    }

  return (0);
}

int adamant_zddtest_error(const char * error_str)
{
  g_fprintf(stderr,error_str);
  return (0);
}

int adamant_zddtest_printdd(DdManager * manager, DdNode * node,
			    const char * filename)
{
  DdNode *tmp[1];
  FILE *f = NULL;

  // Print out a dot file of this post store
  tmp[0] = node;
  f=fopen(filename,"w+");
  Cudd_zddDumpDot(manager, 1, tmp, NULL, NULL, f);
  fclose(f);

  return (0);
}
