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

// zddwork.cpp - Implementation of various ZDD functions for graphing
// 
// More about this class 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <list>
#include "cuddInt.h"
#include "zddwork.h"
#include "bddwork.h"
#include "imagework.h"
#include "extra.h"

#define RES_ADD 10
#define DEFAULT_RES 13
#define ZDDNUM 128
#define lsb(n)		((n)&1 ? one : zero)
#define lsbNot(n)	((n)&1 ? zero : one)
#define VIEW_BUFFER

using namespace std;

extern bool g_render;
extern bool dieondone;
extern int x_res;
extern int y_res;
extern int depth_clip;
extern guint64 x_scale;
extern guint64 y_scale;
extern guint64 x_start;
extern guint64 x_stop;
extern guint64 x_cap;
extern guint64 y_start;
extern guint64 y_stop;
extern guint64 g_start_sin;
extern float min_res_time;
extern float max_res_time;
extern bool use_res_time;

static HC_KEY tempkey = -1;

//! debug
// int dc_count = 0;
// struct timeval dcstime, dcetime;
// double totaldctime = 0;


ZDDWork::ZDDWork(HBaseView * t_view, ImageWork * temp_imagework)  :
  dd_dinrdy(NULL),
  dd_dinrdy_original(NULL),
  dd_dinrdy_filtered(NULL),
  dd_dindin(NULL),
  dd_dinsin(NULL),
  dd_dinhot(NULL),
  dd_dinsys(NULL),
  dd_dinrdy_overlay(NULL),
  dd_dinrdy_filename(NULL),
  dd_dindin_filename(NULL),
  dd_dinsin_filename(NULL),
  dd_dinhot_filename(NULL),
  dd_dinsys_filename(NULL),
  dd_dinrdy_overlay_filename(NULL),
  dd_manager(NULL),
  doDeadReadyFilter(false),
  doUniqStaticFilter(false),
  loadedDD(false),
  zoomFactor(1),
  yFullCubeNode(NULL),  
  xFullCubeNode(NULL)  
{ 
  //! initialize the HOOPS base view pointer
  localView = t_view;

  //! no zdd managers to start
  dd_managers.clear();

  //! if the user has not set a depth_clip, use the #define as default
  if(depth_clip != 0)
    {
      g_maxDepth = depth_clip;
    }
  else
    {
      g_maxDepth = DEFAULT_RES;
    }

  //! time-based resolution variables
  if(max_res_time == 0.0)
    {
      maxRefreshTime = 0.35;
    }
  else
    {
      maxRefreshTime = max_res_time;
    }
  if(min_res_time == 0.0)
    {
      minRefreshTime = 0.30;
    }

  else
    {
      minRefreshTime = min_res_time;
    }
 

  useTimeResolution = use_res_time;

  //! a safety net for resoltion tim
  if((useTimeResolution == true) && (maxRefreshTime < minRefreshTime))
    {
      minRefreshTime = maxRefreshTime;
    }

  //! initilize the factor used in computing depth clipping
  maxDepthClip = depth_clip+RES_ADD;
  minDepthClip = 0;
    

  //! initialize some fairly random variables
  DDPackageInit(&sliceDD);
  useRange = true;
  useDepth = true;

  //! setting up the x and y offsets
  HC_Open_Segment_By_Key(localView->GetSceneKey());

  //! grab the camera field offset
  HC_Show_Net_Camera_Field(&x_offset, &y_offset);
  x_offset /= 2;
  y_offset /= 2;

  //! initialize variables for the two axis ranging
  bottomY = -1;    
  topY = -1;    
  topX = -1;    
  bottomX = -1;

  if(x_start != 0)
    {
      bottomX = (int)ceil(log2(x_start));    
    }
  if(x_stop != 0)
    {
      topX = (int)ceil(log2(x_stop));       
    }
   
  HC_Close_Segment();

  //! initialize the analysis results ZDD
  analysisDD.node = NULL;
  analysisDD.manager = NULL; 
    
  //! initialize the color data for each pixel
  g_color_data = new quint8[temp_imagework->buffer_pixel_size];

  g_color_data[temp_imagework->redPos] = 0x00;
  g_color_data[temp_imagework->greenPos] = 0x00;
  g_color_data[temp_imagework->bluePos] = 0x00;

  if(temp_imagework->alphaPos < temp_imagework->buffer_pixel_size)
    {
      g_color_data[temp_imagework->alphaPos] = 0xFF;
    }
    
  //! **** initialize view buffer work ****
    
  //! make a new image work class
  imwork = temp_imagework;
}   


//! Destructor  
ZDDWork::~ZDDWork() 
{ 

  delete [] g_color_data;

  //! Clean up any imported ZDDs    
  int zdd_manager_size = (int) dd_managers.size();

  for(int i = 0; i < zdd_manager_size; i++)
    {
      Cudd_Quit( dd_managers[i].manager );
    }    
} 
  
//! Copy Constructor
ZDDWork::ZDDWork(const ZDDWork &zddCopyRef)  
{ 

  loadedDD = zddCopyRef.loadedDD;

  //! initialize the HOOPS base view pointer
  localView = zddCopyRef.localView;

  //! no zdd managers to start
  dd_managers = zddCopyRef.dd_managers;

  maxRefreshTime = zddCopyRef.maxRefreshTime;
}

//! Assignement Operator
ZDDWork & ZDDWork::operator=(const ZDDWork & rhs)
{
  loadedDD = rhs.loadedDD;

  //! initialize the HOOPS base view pointer
  localView = rhs.localView;

  //! no zdd managers to start
  dd_managers = rhs.dd_managers;

  maxRefreshTime = rhs.maxRefreshTime;

  return(*this);
}

guint64 ZDDWork::freemem(void)
{
  QString cmd = QString("echo $($_CMD free -b | grep Mem: | awk '{ print $4 }')");
  FILE *cmdfile = popen((cmd.toAscii()).constData(),"r");
  char tresult[256] = {0x0};
  guint64 iResult = 0;

  if(fgets(tresult,sizeof(tresult),cmdfile) != NULL)
    {
      gchar * result = g_strchomp (tresult);
      
      iResult = g_ascii_strtoull(result, NULL, 10);
    }
  pclose(cmdfile);

  g_print("Detected Free Memory: %"G_GUINT64_FORMAT"\n", iResult);
  return (iResult);
}


/*!
  This function will return an array of guint64 DIN numbers
  that correspond to a given RDY time value
*/
guint64 ZDDWork::range_traversal(DdManager *manager, DdNode * e, 
                                 void (ZDDWork::*ptr2ddcollect)(collectFuncStruct*))
{
  guint64 num_dins = 0;
  recurseFuncStructZdd recurStruct;
    
  recurStruct.manager = manager;
  recurStruct.myNode = e;
  recurStruct.setNumber = 0;
  recurStruct.setNumberNot = ~recurStruct.setNumber;
  recurStruct.collectedNumber = 0;
  recurStruct.ptr2ddcollect = ptr2ddcollect;
  recurStruct.hotDD = NULL;

  //! perform the recursive DIN number finder	
  num_dins = range_recurs_builder(recurStruct);

  //     //! DEBUG
  //     printf("DC Expand:%d\n", dc_count);  
  //     printf("DC Expand time:%f\n", (float)(totaldctime)); 
  //     totaldctime = 0;
  //     dc_count = 0;

  return(num_dins);	
}


/*!
  This function will return an array of guint64 DIN numbers
  that correspond to a given RDY time value
*/
guint64 ZDDWork::range_traversal(DdManager *manager, DdNode * e, DdNode * hotDD,
                                 void (ZDDWork::*ptr2ddcollect)(collectFuncStruct*))
{
  guint64 num_dins = 0;
  recurseFuncStructZdd recurStruct;
    
  recurStruct.manager = manager;
  recurStruct.myNode = e;
  recurStruct.setNumber = 0;
  recurStruct.setNumberNot = ~recurStruct.setNumber;
  recurStruct.collectedNumber = 0;
  recurStruct.ptr2ddcollect = ptr2ddcollect;
  recurStruct.hotDD = hotDD;

  //! perform the recursive DIN number finder	
  num_dins = range_recurs_builder(recurStruct);

  return(num_dins);	
}


/*!
 * This function will extract all of the DINs for a given
 * Ready time using a recursive procedure
 */
guint64 ZDDWork::range_recurs_builder(recurseFuncStructZdd recurStruct)
{
  guint64 return_num = 0;	//! return number of collected numbers
  DdNode * nextNode;
  int level = 0, varIndex = 0;
  guint64 saved_collected_num = recurStruct.collectedNumber;
  guint64 saved_set_num = recurStruct.setNumber;

  //! this should have a value
  assert(variableAssign != NULL);

  //! check to see if this node is a constant
  if(Cudd_IsConstant(recurStruct.myNode))
    {
      if(recurStruct.myNode != Cudd_ReadZero(recurStruct.manager))
	{
	  //! increment the count of collected numbers
	  return_num++;
	  
	  //! call the function that will handle the collected
	  //! number
	  collectFuncStruct t_col_struct;
	  t_col_struct.manager = recurStruct.manager;
	  t_col_struct.newNum = recurStruct.collectedNumber;
	  t_col_struct.presetNum = recurStruct.setNumber;
	  t_col_struct.hotcodeDD = recurStruct.hotDD;
	  (*this.*recurStruct.ptr2ddcollect)(&t_col_struct);
	}
    }
	
  else if(MaxNodeDepth(recurStruct.manager, recurStruct.myNode) == 1)
    {                
             
      //! increment the count of collected numbers
      return_num++;                    
                            
      //! call the function that will handle the collected
      //! number
      collectFuncStruct t_col_struct;
      t_col_struct.manager = recurStruct.manager;
      t_col_struct.newNum = recurStruct.collectedNumber;
      t_col_struct.presetNum = recurStruct.setNumber;
      t_col_struct.hotcodeDD = recurStruct.hotDD;

      (*this.*recurStruct.ptr2ddcollect)(&t_col_struct);

    }
  else
    {		            
      //! Determine the variable that this node represents
      varIndex = Cudd_NodeReadIndex(recurStruct.myNode);
            
      //! Determine the permuation at which this variable occurs
      level = Cudd_ReadPermZdd(recurStruct.manager, varIndex);
            
      assert(varIndex != 65535);

        
      //!***********************************
      //! ** first handle the "Then" node **
      //!***********************************
            
      nextNode = Cudd_T(recurStruct.myNode);	// grab the then node
      if(NodeCheckRange(1, &recurStruct) == 1)//! check to see if we are in bounds
	{
                    
	  DdNode * save_node = recurStruct.myNode;
	  recurStruct.myNode = nextNode;

	  // g_print("Diving down THEN branch:%"G_GUINT64_FORMAT"\n", myNode);
                            
                            
	  //! if we have a true value, OR in a 1 into the correct spot in the out_num
	  if(variableAssign[level] == DIN)
	    {
	      recurStruct.collectedNumber = 
		(recurStruct.collectedNumber | mask((guint64)(varIndex - collection_bottom)));
	    }
	  if(variableAssign[level] == RDY)
	    {
	      recurStruct.setNumber = 
		(recurStruct.setNumber | mask((guint64)(varIndex - rdybottom)));
	    }
  
                    
	  //! Here is the recursive call
	  return_num += range_recurs_builder(recurStruct);		
                            
	  //! reset the local copy struct members
	  recurStruct.collectedNumber = saved_collected_num;
	  recurStruct.setNumber = saved_set_num;
	  recurStruct.myNode = save_node;      
	}            

             
      //!********************************
      //! ** now handle the else case **
      //!********************************            
 
      nextNode = Cudd_E(recurStruct.myNode);
      if(NodeCheckRange(0, &recurStruct) == 1)// ! check bounds
	{
#if 0
	  // TEST
	  if(debug_pred || true)
	    {
	      g_print("Doing ELSE for:%"G_GUINT64_FORMAT", level:%d\n", recurStruct.myNode, level);			
	    }
#endif
                    
	  DdNode * save_node = recurStruct.myNode;
	  recurStruct.myNode = nextNode;
                            
	  //! Here is the else recursive call
	  return_num += range_recurs_builder(recurStruct);

	  recurStruct.myNode = save_node;//!restore the current node 
	}
    }
                       
        
    
#if ZDD_DEBUG
  g_print("Returning\n");
#endif
	
  return (return_num);
}

/*!
 * 
 */
int ZDDWork::GetTupleTop(DdManager * manager, DdNode * node, 
				  guint64 &ptopX, guint64 &ptopY)
{

  //! Pull out the X-only node
  DdNode * xOnly = abstractY(manager, node);

  DdNode * nextNode = xOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);            

      ptopX =  ptopX | mask((guint64)(varIndex - collection_bottom));

      nextNode = Cudd_T(nextNode);
    }

  Cudd_RecursiveDerefZdd(manager, xOnly);

  //! Pull out the Y-only node
  DdNode * yOnly = abstractX(manager, node);

   nextNode = yOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      ptopY =  ptopY | mask((guint64)(varIndex - rdybottom));

      nextNode = Cudd_T(nextNode);
    }

  Cudd_RecursiveDerefZdd(manager, yOnly);

  return (0);
}

/*!
 * 
 */
int ZDDWork::GetTupleBottom(DdManager * manager, DdNode * node, 
			    guint64 &pbottomX, guint64 &pbottomY)
{

  //! Pull out the X-only node
  DdNode * xOnly = abstractY(manager, node);

  DdNode * nextNode = xOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      DdNode * tNextNode = Cudd_E(nextNode);
      if (tNextNode == Cudd_ReadZero(manager))
	{
	  pbottomX =  pbottomX | mask((guint64)(varIndex - collection_bottom));
	  tNextNode = Cudd_T(nextNode);
	}
      nextNode = tNextNode;
    }
  Cudd_RecursiveDerefZdd(manager, xOnly);

  //! Pull out the Y-only node
  DdNode * yOnly = abstractX(manager, node);

  nextNode = yOnly;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      DdNode * tNextNode = Cudd_E(nextNode);
      if ( tNextNode == Cudd_ReadZero(manager))
	{
	  pbottomY =  pbottomY | mask((guint64)(varIndex - rdybottom));
	  tNextNode = Cudd_T(nextNode);
	}
      nextNode = tNextNode;
    }

  Cudd_RecursiveDerefZdd(manager, yOnly);

  return (0);
}


/*!
 * This function will extract all of the DINs for a given
 * Ready time using a recursive procedure
 */
guint64 ZDDWork::hot_recurs_builder(recurseFuncStructZdd recurStruct)
{
  guint64 return_hot = 0;	//! return hot num
  DdNode * nextNode;
  int level = 0, varIndex = 0;
  guint64 saved_collected_num = recurStruct.collectedNumber;
  guint64 saved_set_num = recurStruct.setNumber;
  collectFuncStruct t_col_struct;


  //! this should have a value
  assert(variableAssign != NULL);

  //! check to see if this node is a constant
  if(Cudd_IsConstant(recurStruct.myNode))
    {
      if(recurStruct.myNode != Cudd_ReadZero(recurStruct.manager))
	{
	  return_hot = recurStruct.setNumber;
	}
    }
	
  else if(MaxNodeDepth(recurStruct.manager, recurStruct.myNode) == 1)
    {          
      return_hot = recurStruct.setNumber;
    }
  else
    {		            
      //! Determine the variable that this node represents
      varIndex = Cudd_NodeReadIndex(recurStruct.myNode);
            
      //! Determine the permuation at which this variable occurs
      level = Cudd_ReadPermZdd(recurStruct.manager, varIndex);
            
      assert(varIndex != 65535);

      //! if we have a true value, OR in a 1 into the correct spot in the out_num
      if(variableAssign[level] == DIN)
	{
	  if((recurStruct.collectedNumber & mask((guint64)(varIndex - collection_bottom))))
	    {           
	      nextNode = Cudd_T(recurStruct.myNode);	// grab the then node
	      if(NodeCheckRange(1, &recurStruct) == 1)//! check to see if we are in bounds
		{
		  recurStruct.myNode = nextNode;

		  //! Here is the recursive call
		  return_hot = hot_recurs_builder(recurStruct);	
		}
	    }      
	  else
	    {
	      nextNode = Cudd_E(recurStruct.myNode);
	      if(NodeCheckRange(0, &recurStruct) == 1)// ! check bounds
		{                    
		  recurStruct.myNode = nextNode;
                  
		  //! Here is the else recursive call
		  return_hot = hot_recurs_builder(recurStruct);
		}
	    }
	}
      if(variableAssign[level] == RDY)
	{
        
	  //!***********************************
	  //! ** first handle the "Then" node **
	  //!***********************************
            
	  nextNode = Cudd_T(recurStruct.myNode);	// grab the then node
	  if(NodeCheckRange(1, &recurStruct) == 1)//! check to see if we are in bounds
	    {
                    
	      DdNode * save_node = recurStruct.myNode;
	      recurStruct.myNode = nextNode;

	      // g_print("Diving down THEN branch:%"G_GUINT64_FORMAT"\n", myNode);
                                        
	      //! if we have a true value, OR in a 1 into the correct spot in the out_num
	      recurStruct.setNumber = 
		(recurStruct.setNumber | mask((guint64)(varIndex - rdybottom)));
     
	      //! Here is the recursive call
	      return_hot = hot_recurs_builder(recurStruct);		
                            
	      //! reset the local copy struct members
	      recurStruct.collectedNumber = saved_collected_num;
	      recurStruct.setNumber = saved_set_num;
	      recurStruct.myNode = save_node;      
	    }            

             
	  //!********************************
	  //! ** now handle the else case **
	  //!********************************            
	  if(return_hot == 0)
	    {
	      nextNode = Cudd_E(recurStruct.myNode);
	      if(NodeCheckRange(0, &recurStruct) == 1)// ! check bounds
		{                    
		  DdNode * save_node = recurStruct.myNode;
		  recurStruct.myNode = nextNode;
		  
		  //! Here is the else recursive call
		  return_hot = hot_recurs_builder(recurStruct);
		  
		  recurStruct.myNode = save_node;//!restore the current node 
		}
	    }
	}
    }
  
  return (return_hot);
}

bool ZDDWork::toggleUniqueStaticFilter()
{
  if(TRUE == doUniqStaticFilter)
    {
      doUniqStaticFilter = FALSE;
    }
  else
    {
      doUniqStaticFilter = TRUE ;
    }

  return(doUniqStaticFilter);
}

bool ZDDWork::toggleDeadReadyFilter()
{
 if(TRUE == doDeadReadyFilter)
    {
      doDeadReadyFilter = FALSE;
    }
 else
   {
      doDeadReadyFilter = TRUE;
   }
  return(doDeadReadyFilter);
}


/*!
  This function recalculates the maximum depth clip
  according to a new Z axis position
*/
int ZDDWork::RedoDepthClip(float newZ)
{
  int return_value = 0;


  if(useTimeResolution == true)
    {
      if(viewRefreshTime > maxRefreshTime)
	{
	  g_maxDepth++;
                    
	}
      else if (viewRefreshTime < minRefreshTime)
	{
	  g_maxDepth--;
                    
	}
    }
  else
    {
      g_maxDepth = (int)ceil(newZ*zoomFactor);
    
      //! make sure we do not exceed the max clip
      if(g_maxDepth > maxDepthClip)
	{
	  g_maxDepth = maxDepthClip;
	}


      //! make sure we do not go under the min clip
      if(g_maxDepth < minDepthClip)
	{
	  g_maxDepth = minDepthClip;
	}
    }

  return(return_value);
}


/*!
  Set the maxium depth clipping
*/
void ZDDWork::SetMaxDepthClip(int newMaxClip)
{
  maxDepthClip = newMaxClip;
}


/*!
  Get the maxium depth clipping
*/
int ZDDWork::GetMaxDepthClip(void)
{
  return(maxDepthClip);
}


/*!
  Set the minimum depth clipping
*/
void ZDDWork::SetMinDepthClip(int newMinClip)
{
  minDepthClip = newMinClip;
}


/*!
  Get the minimum depth clipping
*/
int ZDDWork::GetMinDepthClip(void)
{
  return(minDepthClip);
}


/*!
 * This function checks the node depth to see if it is beyond the 
 * set limit
 *
 */
int ZDDWork::MaxNodeDepth(DdManager *manager, DdNode * node)
{
  int returnValue = 0;
  int varIndex = 0;

  if(useDepth == false)
    {
      returnValue = 1;
    }
  else
    {
      //! Determine the variable that this node represents
      varIndex = Cudd_NodeReadIndex(node);

      //! Normalize this variable for a 64 bit checks
      if(varIndex > 63)
	{
	  varIndex = varIndex - 64;
	}

      if((g_maxDepth != 0) && (varIndex < g_maxDepth))
	{
	  returnValue = 1;
	}
      else
	{
	  returnValue = 0;
	}
    }

  return(returnValue);
} 


/*!
  \brief Calculates the needed zdd values from a resolution
  \author Graham Price
  This function consists only of side effects, which include
  the following variables:
  -zoomFactor
*/
void ZDDWork::CalculateResolutionFactor(void)
{  
  zoomFactor = (g_maxDepth / (start_z));
}


/*!
  This function checks the node to determine if the search
  is still in bounds
*/
int ZDDWork::NodeCheckRange(int then, recurseFuncStructZdd * recurs_struct)
{
  int returnValue = 1;
  int varIndex = 0;
  int level = 0;

  if(useRange == false)
    {
      returnValue = 1;
    }
  else            
    {
      if(!Cudd_IsConstant(recurs_struct->myNode))
	{

	  //! Determine the variable that this node represents
	  varIndex = Cudd_NodeReadIndex(recurs_struct->myNode);
        
	  //! Determine the permuation at which this variable occurs
	  level = Cudd_ReadPermZdd(recurs_struct->manager, varIndex); 

	  //! Normalize this variable for a 64 bit checks
	  if(varIndex > 63)
	    {
	      varIndex = varIndex - 64;
	    }
            
	  if(variableAssign[level] == RDY)
	    {
	      //! check the X axis bottom
	      if ((bottomX != -1) && (then == 0)
		  && (varIndex == bottomX) &&
		  (recurs_struct->setNumber == 0))
		{
		  returnValue = 0;
		}
                
	      //! check the X axis top
	      else if ((topX != -1) && (then == 1) && (varIndex > topX))
		{
		  returnValue = 0;
		}
	    }
	  else
	    {
	      //! check the Y axis bottom
	      if ((bottomY != -1) && (then == 0) && 
		  (varIndex == bottomY) && 
		  (recurs_struct->collectedNumber == 0))
		{
		  returnValue = 0;
		}
                
	      //! check the Y axis top
	      else if ((topY != -1) && (then == 1) &&  (varIndex > topY))
		{
		  returnValue = 0;
		}
	    }
	}
    }
  return(returnValue);
}


/*!
  
  Function: CollectedNumStats

  This function will get stats
  on the numbers collected by the
  ZDD traversal.

*/
void ZDDWork::CollectedNumStats(collectFuncStruct * collStruct)
{

  //! record the largest collected value
  if ( collStruct->newNum > highColNumber)
    {
      highColNumber = collStruct->newNum;
    }
                                    
  //! record the smallest collected value
  if ( collStruct->newNum < lowColNumber)
    {
      lowColNumber = collStruct->newNum;
    }
                                    
  //! record the largest set value
  if ( collStruct->presetNum > highSetNumber)
    {
      highSetNumber = collStruct->presetNum;
    }
                                    
  //! record the smallest set value
  if ( collStruct->presetNum < lowSetNumber)
    {
      lowSetNumber = collStruct->presetNum;
    }
}


/*!
  This function will graph the number
  collected by the recursive number collector

  \author Graham Price
  \date 0920/2007
*/
void ZDDWork::GraphCollectedNums(collectFuncStruct * collStruct)
{
  if(g_render)
    {
            
      //! convert the plot coordinates to a pixel
      //! X and pixel Y location
      qint64 pixX, pixY;
      imwork->Plot2Pixel(collStruct->presetNum, collStruct->newNum,
			 &pixX, &pixY);

      //! safety catch
      if((pixX < 0) || (pixY < 0))
	{
	  return;
	}

      //! modify the pixel to not be white 
      //! in the current view buffer
      //! this function should be inlined
      ModifyPixel(imwork->view_buffer, pixX, pixY, g_color_data);
    }    
}


/*!
  This function will collect the number
  collected by the recursive number collector

  \author Graham Price
  \date 0920/2007
 */
void ZDDWork::StoreCollectedNums(collectFuncStruct * collStruct)
{
    dd_tuple temp_tuple;
    
    //! add this number to a global list of numbers
    //! NOTE: the DINs are usually on the Y axis, and the RDYs are on the X
    //! but by the notation set here, that would be flipped
    temp_tuple.x = collStruct->newNum;
    temp_tuple.y = collStruct->presetNum;
    g_CollectedNums.push_back(temp_tuple);    
}




/*!
  This function will collect the number
  collected by the recursive number collector

  \author Graham Price
*/
void ZDDWork::HotCodeRecurse(collectFuncStruct * collStruct)
{

  if(g_render)
    {
      recurseFuncStructZdd recursHot;
      recursHot.manager = collStruct->manager;
      recursHot.myNode = collStruct->hotcodeDD;
      recursHot.setNumber = 0;
      recursHot.setNumberNot = ~recursHot.setNumber;
      recursHot.collectedNumber = collStruct->newNum;
      recursHot.collectedNumberNot = ~recursHot.collectedNumber;
      recursHot.numComplement = 0;

      guint64 t_maxDepth = g_maxDepth;
      int t_topx = topX;
      int t_bottomx = bottomX;
      int t_topy = topY;
      int t_bottomy = bottomY;

      //      g_maxDepth = (guint64)(floor(log2(floor(hotSplit_ / 20))));
      g_maxDepth = 0;
      // if(8 < hotSplitShift)
      // 	{
      // 	  g_maxDepth = hotSplitShift - 7;
      // 	}
      // else
      // 	{
      // 	  g_maxDepth = 0;
      // 	}
      
      //DEBUG
      //      g_print("Hot Code Depth:%"G_GUINT64_FORMAT"\n", g_maxDepth);
      

      topX = bottomX = topY = bottomY = -1;

      guint64 hotval = hot_recurs_builder(recursHot);

      //! restoring the limiting values
      g_maxDepth = t_maxDepth;
      topX = t_topx;
      bottomX = t_bottomx;
      topY = t_topy;
      bottomY = t_bottomy;
 
      //! convert the plot coordinates to a pixel
      //! X and pixel Y location
      qint64 pixX, pixY;
      imwork->Plot2Pixel(collStruct->presetNum, collStruct->newNum,
			 &pixX, &pixY);

      //! safety catch
      if((pixX < 0) || (pixY < 0))
	{
	  return;
	}

      //! initialize the color data for each pixel
      quint8 t_color_data[imwork->buffer_pixel_size];
      t_color_data[imwork->greenPos] = 0x00;
      t_color_data[imwork->bluePos] = 0x00;
      t_color_data[imwork->redPos] = 0x00;

      if(hotval > hotSplit_)
      	{	  
      	  t_color_data[imwork->redPos] = 0xFF;
	  
      	  if(imwork->alphaPos < imwork->buffer_pixel_size)
      	    {
      	      t_color_data[imwork->alphaPos] = 0xFF;
      	    }
      	}
      else if(hotval > (guint64)(hotSplit_ / 20))
      	{
      	  t_color_data[imwork->redPos] = 0x32;
      	  t_color_data[imwork->greenPos] = 0xCD;
      	  t_color_data[imwork->bluePos] = 0x32;
	  
      	  if(imwork->alphaPos < imwork->buffer_pixel_size)
      	    {
	      //      	      t_color_data[imwork->alphaPos] = 0x20;
      	      t_color_data[imwork->alphaPos] = 0xFF;
      	    }
      	}
      else if(hotval > (guint64)(hotSplit_ / 40))
      	{
      	  t_color_data[imwork->redPos] = 0x20;
      	  t_color_data[imwork->greenPos] = 0xB2;
      	  t_color_data[imwork->bluePos] = 0xAA;
	  
      	  if(imwork->alphaPos < imwork->buffer_pixel_size)
      	    {
	      //      	      t_color_data[imwork->alphaPos] = 0x20;
      	      t_color_data[imwork->alphaPos] = 0xFF;
      	    }
      	}
      else
      	{
      	  t_color_data[imwork->redPos] = 0x00;
      	  t_color_data[imwork->greenPos] = 0xBF;
      	  t_color_data[imwork->bluePos] = 0xFF;

      	  if(imwork->alphaPos < imwork->buffer_pixel_size)
      	    {
	      t_color_data[imwork->alphaPos] = 0x20;
	      //      	      t_color_data[imwork->alphaPos] = 0x10;
      	    }
      	}

      ModifyPixel(imwork->view_buffer, pixX, pixY, t_color_data);
    }
}

guint64 * ZDDWork::getTopDenseVars(DdManager * manager, DdNode * node)
{
  guint64 * returnTops = g_new0(guint64, 2);

  int * varCountArray = mg_Extra_zddLitCount(manager, node, 12);

  if(NULL == varCountArray)
    {
      g_free(returnTops);
      return(NULL);
    }

  // for (int v = 0; v < manager->sizeZ; ++v)
  //   {
  //     g_print("%d->%d ", v, varCountArray[v]);

  //     if(v == 63)
  // 	{
  // 	  g_print("\n");
  // 	}
  //   }
  //  g_print("\n");
  
   const double growthFactor = 1.2;
   const guint64 bJump = 10;
   guint64 * firstV = g_new0(guint64, 2);
   guint64 * firstVb = g_new0(guint64, 2);
   for (int v = (manager->sizeZ - 1); 0 < v; --v)
     {
       if((v < 64) && (0 == returnTops[0]))
	 {
	   if(0 == firstV[0])
	     {
	       firstV[0] = varCountArray[v];
	       firstVb[0] = v;
	     }
	   else if(varCountArray[v] > (guint64)((double)firstV[0] * growthFactor))
	      {
		if(varCountArray[v] > (firstV[0] * bJump))
		  {
		    returnTops[0] = v;
		  }
		else
		  {
		    returnTops[0] = firstVb[0];
		  }
	      }
	 }
       else if(0 == returnTops[1])
	 {
	   if(0 == firstV[1])
	     {
	       firstV[1] = varCountArray[v];
	       firstVb[1] = v - 64;
	     }
	   else if(varCountArray[v] > (guint64)((double)firstV[1] * growthFactor))
	     {
	       if(varCountArray[v] > (firstV[1] * bJump))
		 {
		   returnTops[1] = v - 64;
		 }
	       else
		 {
		   returnTops[1] = firstVb[1];
		 }
	      }
	 }
     }
   g_free(firstV);
   g_free(firstVb);

   // //START DEBUG
   g_print("t0: %d, t1: %d\n", returnTops[0], returnTops[1]);
   // // END DEBUG
   
  // // Calculate the mean variable density
  // guint64 * sum  = g_new0(guint64, 2);
  // guint64 * vCount  = g_new0(guint64, 2);
  // for (int v = 0; v < manager->sizeZ; ++v)
  //   {
  //     if(v < 64)
  // 	{
  // 	  sum[0] += varCountArray[v];
  // 	  vCount[0] += 1;//(int)(0 < varCountArray[v]);
  // 	}
  //     else
  // 	{
  // 	  sum[1] += varCountArray[v];
  // 	  vCount[1] += 1;//(int)(0 < varCountArray[v]);
  // 	}
  //   }
  // guint64 * aMean = g_new0(guint64, 2); 
  // aMean[0] = (guint64)(sum[0] / vCount[0]);
  // aMean[1] = (guint64)(sum[1] / vCount[1]);
  
  // //START DEBUG
  // g_print("a0: %d, a1: %d\n", aMean[0], aMean[1]);
  // // END DEBUG

  // g_free(sum);
  // g_free(vCount);

  // for (int v = (manager->sizeZ - 1); v >= 0; --v)
  //   {
  //     if((v < 64) && (0 == returnTops[0]))
  // 	{
  // 	  if(varCountArray[v] > aMean[0])
  // 	    {
  // 	      returnTops[0] = v;
  // 	    }
  // 	}
  //     else if (0 == returnTops[1])
  // 	{
  // 	  if(varCountArray[v] > aMean[1])
  // 	    {
  // 	      returnTops[1] = v - 64;
  // 	    }
  // 	}
  //   }
  // g_free(varCountArray);
  // g_free(aMean);

  return(returnTops);
}


/*!
 * This function takes a ZDD full of DIN values and returns
 * a ZDD with the corrisponding dependence DIN ZDD
 */
dd_package ZDDWork::GetDepDINDD(dd_package dinPack)
{
  dd_package return_pack;
  DdNode * depZDD = NULL;
  DdNode * dindinZDD = NULL;
  DdManager * dindinMan = NULL;

  DDPackageInit(&return_pack);

  assert(dinPack.node != NULL);

  //! try to find all the needed ZDDs
  for (unsigned int i = 0; (i < dd_managers.size()); i++)
    {
      manager_group t_manager;
      t_manager =  dd_managers[i]; 
      if(t_manager.type == DINVSDIN)
	{
	  dindinZDD = t_manager.node;
	  dindinMan = t_manager.manager;
	}
    }

  if((dindinMan != NULL) && (dindinZDD != NULL))
    {

      struct timeval startSliceTime, currentSliceTime;
      gettimeofday(&startSliceTime,NULL); // DEBUG !!!

      depZDD = BuildIterReverseSlice(dindinMan, dinPack.node, 
				     dindinZDD, 1000);
      //      depZDD = ReverseSlice(dindinMan, dinPack.node, 
      //      			     dindinZDD);
      //depZDD = LongReverseSlice(dindinMan, dindinZDD);
            
      //! DEBUG !!!
      // gettimeofday(&currentSliceTime,NULL); // DEBUG !!!
      // double timeForSlice = (((currentSliceTime.tv_sec + 
      // 			       (currentSliceTime.tv_usec/1000000.0)) - 
      // 			      (startSliceTime.tv_sec + 
      // 			       (startSliceTime.tv_usec/1000000.0))));
      
      // g_print("Time for 1 slices:%f\n", timeForSlice);
    }
    
  return_pack.node = depZDD;
  return_pack.manager = dindinMan;
    
  return(return_pack); 
}


/*!
 * This function, when given a ZDD containing the final
 * slice of DIN dependence info, should AND that with
 * the resident DINxRDY ZDD and graph the result
 *
 */
int ZDDWork::GraphFinalSlice(dd_package depPack)
{
  DdNode * depZDD = depPack.node;
  DdNode * finZDD = NULL;
  DdNode * dinrdyZDD = NULL;
  DdManager * dinrdyMan = NULL;

  assert(depZDD != NULL);

  //! try to find all the needed ZDDs
  for (unsigned int i = 0; (i < dd_managers.size()); i++)
    {
      manager_group t_manager;
      t_manager = dd_managers[i]; 
      if(t_manager.type == DINVSRDY)
	{
	  dinrdyZDD = t_manager.node;
	  dinrdyMan = t_manager.manager;
	}
    }

  if((dinrdyZDD != NULL) && (dinrdyMan != NULL))
    {
      DdNode * tmp = yDC(dinrdyMan, depZDD);
      depZDD = tmp;
  
      // Logic AND the target ZDD node with the slice node
      finZDD = Cudd_zddIntersect(dinrdyMan, dinrdyZDD, depZDD);
      Cudd_RecursiveDerefZdd(dinrdyMan, depZDD);
      Cudd_Ref(finZDD); 

      //! setup the analysis ZDD
      if(analysisDD.node != NULL){
	Cudd_RecursiveDerefZdd(analysisDD.manager, analysisDD.node);                
      }
      analysisDD.node = finZDD;
      analysisDD.manager = dinrdyMan;
    }

  return (0);
}


DdNode * ZDDWork::VarSwap(DdManager * manager, DdNode * node)
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

DdNode * ZDDWork::abstractY(DdManager *manager, DdNode * sliceNode)
{
  // Make the zero zdd
  DdNode * zeroNode = Cudd_ReadZero(manager);
  Cudd_Ref(zeroNode);

  // Build positive cube for existential abstraction
  DdNode * yCubeNode = build_tuple(manager, zeroNode, 0, 0xffffffffffffffffLL);
  Cudd_RecursiveDerefZdd(manager, zeroNode);

  // Create an abstraction of the slice ZDD
  DdNode * abstractNode = mg_Extra_zddExistAbstract(manager, sliceNode, yCubeNode);
  Cudd_Ref(abstractNode);  
  Cudd_RecursiveDerefZdd(manager, yCubeNode);

  return (abstractNode);
}

DdNode * ZDDWork::build_fullcube(DdManager *manager, guint64 x, guint64 y)
{
  int i = 0;
  DdNode * tmp = NULL, * newNode = NULL, * zero = NULL, * one = NULL;

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

  return (newNode);
}

DdNode * ZDDWork::abstractX(DdManager *manager, DdNode * sliceNode)
{
  // Make the zero zdd
  DdNode * zeroNode = Cudd_ReadZero(manager);
  Cudd_Ref(zeroNode);

  // Build positive cube for existential abstraction
  DdNode * xCubeNode = build_tuple(manager, zeroNode, 0xffffffffffffffffLL, 0);
  Cudd_RecursiveDerefZdd(manager, zeroNode);

  // Create an abstraction of the slice ZDD
  DdNode * abstractNode = mg_Extra_zddExistAbstract(manager, sliceNode, xCubeNode);
  Cudd_Ref(abstractNode);  
  Cudd_RecursiveDerefZdd(manager, xCubeNode);

  return (abstractNode);
}

DdNode * ZDDWork::xDC(DdManager *manager, DdNode * node)
{
  // Build positive and negitive cubes
  if (xFullCubeNode == NULL)
    {
     xFullCubeNode = build_fullcube(manager, 0xffffffffffffffffLL, 0);
    }

  // Replace the missing nodes in the X position with positive Don't Care nodes
  DdNode * productNode = Cudd_zddUnateProduct( manager, node, xFullCubeNode);
  Cudd_Ref(productNode);
  //  Cudd_RecursiveDerefZdd(manager, xFullCubeNode);

  return (productNode);
}

DdNode * ZDDWork::yDC(DdManager *manager, DdNode * node)
{
  // Build positive and negitive cubes
  if (yFullCubeNode == NULL)
    {
      yFullCubeNode = build_fullcube(manager, 0, 0xffffffffffffffffLL);
    }

  // Replace the missing nodes in the Y position with positive Don't Care nodes
  DdNode * productNode = Cudd_zddUnateProduct( manager, node, yFullCubeNode);
  Cudd_Ref(productNode);
  //  Cudd_RecursiveDerefZdd(manager, yFullCubeNode);

  return (productNode);
}


DdNode * ZDDWork::BuildIterReverseSlice(DdManager * manager, DdNode * sliceNode, 
					DdNode * targetNode)
{
  DdNode * newSlice = BuildIterReverseSlice(manager, sliceNode, targetNode, 0);

  return(newSlice);
}


DdNode * ZDDWork::BuildIterDinDinReverseSlice(DdNode * sliceNode, 
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

  DdNode * newSlice = DinDinReverseSlice (sliceNode, dinDinSubSetDD);

  // Accumulate the new dindin set off to the side
  if(0 != resDepth)
    {
      guint64 depthVal = 1 << resDepth;
      depthVal -= 1;

      DdNode * univDepthDD = build_fullcube(dd_manager, depthVal, depthVal);
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

      DdNode * tmpSlice = DinDinReverseSlice (newSlice, dinDinSubSetDD);
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


DdNode * ZDDWork::BuildIterDinDinForwardSlice(DdNode * sliceNode, 
					      DdNode * local_dindinDD, 
					      guint64 stopCount)
{
  struct timeval startSliceTime, currentSliceTime;
  gettimeofday(&startSliceTime,NULL); // DEBUG !!!
 
  guint64 count = 0;  

  // Do one iteration of the slice to avoid messing with the ref count of sliceNode 
  DdNode * dinDinSubSetDD = Cudd_zddDiff(dd_manager, local_dindinDD, sliceNode);
  Cudd_Ref(dinDinSubSetDD);

  DdNode * newSlice = DinDinForwardSlice (sliceNode, dinDinSubSetDD);

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

      DdNode * tmpSlice = DinDinForwardSlice (newSlice, dinDinSubSetDD);
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

      //! DEBUG !
      if ((count % 1000) == 0)
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


DdNode * ZDDWork::BuildIterReverseSlice(DdManager * manager, DdNode * sliceNode, 
					DdNode * targetNode, guint64 stopCount)
{
  struct timeval startSliceTime, currentSliceTime;  
  gettimeofday(&startSliceTime,NULL); // DEBUG !!!

  guint64 count = 0;

//   DdNode * localTargetDD = Cudd_zddDiff(manager, targetNode, sliceNode);
//   Cudd_Ref(localTargetDD);

  DdNode * localTargetDD = targetNode;

  DdNode * newSlice = ReverseSlice( manager, sliceNode, localTargetDD); 
  DdNode * newAcSlice = Cudd_zddUnion(manager, sliceNode, newSlice);
  Cudd_Ref(newAcSlice);

  gboolean sliceEqual = (gboolean)(sliceNode == newAcSlice);
   
  while((sliceEqual == FALSE) && ((stopCount == 0) || (count < stopCount)))
    {
//       DdNode * tmpLocalTargetDD = Cudd_zddDiff(manager, localTargetDD, newSlice);
//       Cudd_Ref(tmpLocalTargetDD);
//       Cudd_RecursiveDerefZdd(manager, localTargetDD);
//       localTargetDD = tmpLocalTargetDD;

      DdNode * tmpSlice = ReverseSlice( manager,  newSlice,  localTargetDD);
      Cudd_RecursiveDerefZdd(manager, newSlice);
      newSlice = tmpSlice;

      DdNode * tmpNewAcSlice = Cudd_zddUnion(manager, newSlice, newAcSlice);
      Cudd_Ref(tmpNewAcSlice);

      sliceEqual = (gboolean)(tmpNewAcSlice == newAcSlice);
      Cudd_RecursiveDerefZdd(manager, newAcSlice);
      newAcSlice = tmpNewAcSlice;
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

  return(newAcSlice);
}



DdNode * ZDDWork::BuildReverseSlice(DdManager * manager, DdNode * sliceNode, DdNode * targetNode)
{
  DdNode * tmpSlice = ReverseSlice( manager,  sliceNode,  targetNode);
  DdNode * newSlice = Cudd_zddUnion(manager, tmpSlice, sliceNode);
  Cudd_Ref(newSlice);
  Cudd_RecursiveDerefZdd(manager, tmpSlice);

  return(newSlice);
}

DdNode * ZDDWork::LongReverseSlice(DdManager * manager, DdNode * targetNode)
{
  guint64 stopCount = 0, count = 0;
  DdNode * oldSlice = NULL;
  struct timeval startSliceTime, currentSliceTime, deltaSliceTime;

  //! find the ZDD for the selected area
  guint64 thighXNumber = 0;
  guint64 thighYNumber = 0;

  GetTupleTop(manager, targetNode, thighXNumber, thighYNumber);
  DdNode * zero = Cudd_ReadZero(manager);
  Cudd_Ref(zero);
  DdNode * longSliceNode = zero;
  guint64 longSliceNum = thighXNumber - 100;
  guint64 longSliceCount = 0;
  guint64 longSliceX = 0;
  
  while (longSliceNum < thighXNumber)
    { 
      g_print("\nSlice starting at %d\n", thighXNumber);
      DdNode * sliceNode = build_tuple(manager, zero, 
				       thighXNumber, 0);

      DdNode * newSlice = BuildReverseSlice(manager, sliceNode, targetNode);
      
      gettimeofday(&startSliceTime,NULL); // DEBUG !!!
      deltaSliceTime = startSliceTime;
      

      while((newSlice != oldSlice) && ((stopCount == 0) || (count < stopCount)))
	{
	  oldSlice = newSlice;
      
	  DdNode * tmpSlice = BuildReverseSlice( manager,  newSlice,  targetNode);
	  if (!cuddIsConstant(newSlice))
	    {
	      Cudd_RecursiveDerefZdd(manager, newSlice);
	    }
	  newSlice = tmpSlice;

	  ++count;
      
	  // //! DEBUG !
	  // if ((count % 1000) == 0)
	  //   {
	  //     gettimeofday(&currentSliceTime,NULL); // DEBUG !!!
	  
	  //     double timeForSlice = (((currentSliceTime.tv_sec + 
	  // 			       (currentSliceTime.tv_usec/1000000.0)) - 
	  // 			      (startSliceTime.tv_sec + 
	  // 			       (startSliceTime.tv_usec/1000000.0))));
	  //     double deltaTimeForSlice = (((currentSliceTime.tv_sec + 
	  // 				    (currentSliceTime.tv_usec/1000000.0)) - 
	  // 				   (deltaSliceTime.tv_sec + 
	  // 				    (deltaSliceTime.tv_usec/1000000.0))));
	  //     deltaSliceTime = currentSliceTime;
	  
	  //     g_print("Slices:%d, Total Time:%f, Delta Time:%f\n", count, timeForSlice, deltaTimeForSlice);
	  //   }
	} 

      if (count > longSliceCount)
	{
	  if (!cuddIsConstant(longSliceNode))
	    {
	      Cudd_RecursiveDerefZdd(manager, longSliceNode);
	    }

	  longSliceNode = newSlice;
	  longSliceCount = count;
	  longSliceX = thighXNumber;
	}
      else
	{
	  if (!cuddIsConstant(newSlice))
	    {
	      Cudd_RecursiveDerefZdd(manager, newSlice );
	    }
	}
      if (!cuddIsConstant(sliceNode))
	{
	  Cudd_RecursiveDerefZdd(manager, sliceNode);
	}
      
      --thighXNumber;
      count = 0;
    }

  //! DEBUG
  g_print("Longest slice of %d for DIN:%d\n", longSliceX, longSliceCount);

  return(longSliceNode);
}


DdNode * ZDDWork::DinDinReverseSlice(DdNode * sliceDD, DdNode * targetDD)
{
  // Form a {blank, DIN} set
  DdNode * blankDinDD = abstractX(dd_manager, sliceDD);

  // Swap the variable positions to form a {DIN, blank} set
  DdNode * dinBlankDD = VarSwap(dd_manager, blankDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, blankDinDD);

  // Form a {DIN, univ} set
  DdNode * dinUnivDD = yDC(dd_manager, dinBlankDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinBlankDD);

  // Get a new {DIN, {DIN}} set
  DdNode * newDinDinDD = Cudd_zddIntersect(dd_manager, dinUnivDD, targetDD);
  Cudd_Ref(newDinDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnivDD);

  return (newDinDinDD);
}



DdNode * ZDDWork::ReverseSlice(DdManager * manager, DdNode * sliceNode, DdNode * targetNode)
{
  //! DEBUG !!!
  //  DdSize(manager, sliceNode);

  DdNode * tmpSliceNode = yDC(manager, sliceNode);
  //  DdNode * targetY = abstractX(manager, targetNode);
  //DdNode * tmpSliceNode = Cudd_zddUnateProduct(manager, sliceNode, targetY);

  // Logic AND the target ZDD node with the slice node
  //  DdNode * intersectNode = Cudd_zddIte(manager, targetNode, tmpSliceNode, Cudd_ReadZero(manager));
  DdNode * intersectNode = Cudd_zddIntersect(manager, targetNode, tmpSliceNode);
  //  DdNode * intersectNode = Cudd_zddUnateProduct(manager, targetNode, sliceNode);
  Cudd_Ref(intersectNode);
  Cudd_RecursiveDerefZdd(manager, tmpSliceNode);
   
  //! DEBUG !!!
  //  DdSize(manager, intersectNode);

  // Remove the X variables
  DdNode * newXnode = abstractX( manager, intersectNode);
  Cudd_RecursiveDerefZdd(manager, intersectNode);
    
  // Swap the X and Y variables
  DdNode * newRevSliceNode = VarSwap( manager, newXnode);
  Cudd_RecursiveDerefZdd(manager, newXnode);
    
  //! DEBUG !!!
  //  DdSize(manager, newRevSliceNode);

  return (newRevSliceNode);
}


DdNode * ZDDWork::build_tuple(DdManager * manager, DdNode *set, 
				 guint64 x, guint64 y)
{
  int i = 0;
  DdNode * tmp = NULL, * newNode = NULL, * zero = NULL, * one = NULL;

  //! grab the ZDD One
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
	    newNode = cuddZddGetNode(manager, v, tmp, zero);
	    Cudd_Ref(newNode);
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



/*!
  
  This function returns the PC ZDD for a given DIN ZDD
  Requires a DINxSIN ZDD be loaded into the manager arrays
  Returns a single SIN
  \author Graham Price
  \date 07/31/2007

*/
dd_package ZDDWork::GetPConlyDD(dd_package dinPack)
{
  dd_package return_pack;
  DdNode * pcZDD = NULL;
  DdNode * dinpcZDD = NULL;
  DdManager * dinpcMan = NULL;
  guint64 num_dins = 0;

  DDPackageInit(&return_pack);

  assert(dinPack.node != NULL);

  //! try to find all the needed ZDDs
 dinpcZDD = dd_dinsin;
 dinpcMan = dd_manager;

  if((dinpcMan != NULL) && (dinpcZDD != NULL))
    {            
      //! Build a {DIN, univ} set
      DdNode * dinUnivDD = yDC(dinPack.manager, dinPack.node);
      Cudd_RecursiveDerefZdd(dinPack.manager, dinPack.node);

      //! Intersect with our {DIN,SIN} set
      DdNode * dinPcIntDD = Cudd_zddIntersect(dinpcMan, dinUnivDD, dinpcZDD);
      Cudd_Ref(dinPcIntDD);
      Cudd_RecursiveDerefZdd(dinpcMan, dinUnivDD);

      //! before we remove the rdy time values, we should collect
      //! the pc values from this ZDD
      guint64 t_maxDepth = g_maxDepth;
      int t_topx = topX;
      int t_bottomx = bottomX;
      int t_topy = topY;
      int t_bottomy = bottomY;
             
      g_maxDepth = 0;
      topX = bottomX = topY = bottomY = -1;

      num_dins = range_traversal(dinpcMan, dinPcIntDD, 
				 &ZDDWork::StoreCollectedNums);

      //            g_print("Collected %d\n",g_CollectedNums.size()); //!DEBUG

      //! restoring the limiting values
      g_maxDepth = t_maxDepth;
      topX = t_topx;
      bottomX = t_bottomx;
      topY = t_topy;
      bottomY = t_bottomy;
      
      //! Get the SIN only DD
      pcZDD = abstractX(dinpcMan, dinPcIntDD);
      Cudd_RecursiveDerefZdd(dinpcMan, dinPcIntDD);
    }

  return_pack.node = pcZDD;
  return_pack.manager = dinpcMan;

  return(return_pack); 
}


/*!
  This function returns a ZDD that contains the area selected
  if the ZDD contains (X,Y), 
*/
DdNode * ZDDWork::GraphSelect(DdManager * manager, guint64 x_min, guint64 x_max,
                              guint64 y_min, guint64 y_max)
{
  guint64 zddNumHalf = ZDDNUM/2;

  int xlist [ZDDNUM/2];
  int ylist [ZDDNUM/2];
  DdNode * returnZDD = NULL;
    
  //! gather a list of ZDD node variables
  for (unsigned int i = 0; i < zddNumHalf; i+=1)
    {
      xlist[(zddNumHalf - 1) - i] = i;
      ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
    }

  //! find the ZDD for the selected area
  DdNode * xUb =  mg_Cudd_zddUb(manager, zddNumHalf, xlist, (uint64_t)x_max);
  Cudd_Ref(xUb);
  //  DdSize(manager, xUb);
  DdNode * xLb =  mg_Cudd_zddLb(manager, zddNumHalf, xlist, (uint64_t)x_min);
  Cudd_Ref(xLb);
  //  DdSize(manager, xLb);
  //  tempNodeX =  mg_Cudd_zddInterval(manager, zddNumHalf, xlist,(uint64_t)x_min, (uint64_t)x_max);
  //  Cudd_Ref(tempNodeX);

  //  DdSize(manager, tempNodeX);

  //  tmp = yDC(manager, tempNodeX);
  //  Cudd_RecursiveDerefZdd(manager, tempNodeX);
  //  tempNodeX = tmp;

  //! find the ZDD for the selected area
  DdNode * yUb =  mg_Cudd_zddUb(manager, zddNumHalf, ylist, (uint64_t)y_max);
  Cudd_Ref(yUb);
  //  DdSize(manager, yUb);
  DdNode * yLb =  mg_Cudd_zddLb(manager, zddNumHalf, ylist, (uint64_t)y_min);
  Cudd_Ref(yLb);
  //  DdSize(manager, yLb);
  //  tempNodeY = mg_Cudd_zddInterval(manager, zddNumHalf, ylist,(uint64_t)y_min, (uint64_t)y_max);
  //  Cudd_Ref(tempNodeY);

  //  DdSize(manager, tempNodeY);

  DdNode * uB = Cudd_zddUnateProduct(manager, xUb, yUb);
  Cudd_Ref(uB);
  Cudd_RecursiveDerefZdd(manager, xUb);
  Cudd_RecursiveDerefZdd(manager, yUb);
  //  DdSize(manager, uB);
  
  DdNode * lB = Cudd_zddUnateProduct(manager, xLb, yLb);
  Cudd_Ref(lB);
  Cudd_RecursiveDerefZdd(manager, xLb);
  Cudd_RecursiveDerefZdd(manager, yLb);
  //  DdSize(manager, lB);


  //! now perform the AND
  //  returnZDD = Cudd_zddUnateProduct(manager, tempNodeX, tempNodeY);
  returnZDD = Cudd_zddIntersect(manager, uB, lB);
  Cudd_Ref(returnZDD);
  Cudd_RecursiveDerefZdd(manager, lB);
  Cudd_RecursiveDerefZdd(manager, uB);
  //  DdSize(manager, returnZDD);

  return returnZDD;
}


/*! 
 * Simple mask function
 */
guint64 ZDDWork::mask(guint64 v) {
  return ((guint64)(1) << v);
}

HPoint ZDDWork::Plot2World(guint64 x, guint64 y)
{
  HPoint center;
 
  //! compute the center of the sphere used to mark this DIN
  center.Set(((float)x - (float)x_start) / (float)x_scale,
	     ((float)y - (float)y_start) / (float)y_scale,
	     0);

  //! add in the offset
  center.x = center.x - x_offset;
  center.y = center.y - y_offset;      

  return (center);
}

int ZDDWork::World2Plot( HPoint p, guint64 * x, guint64 * y)
{ 
  int return_val = 0;
  HPoint center;

  if((x != NULL) && (y != NULL))
    {
      //! add in the offset
      p.x = p.x + x_offset;
      p.y = p.y + y_offset;

      *x = (guint64)((p.x + x_start) * x_scale);
      *y = (guint64)((p.y + y_start) * y_scale);
     
    }
  else
    {
      return_val = 1;
    }
  return (return_val);
}


//! this function takes a list of DINxPC values
//! and returns a map of PCxCount
//! NOTE:Map must be destroyed by the caller
map<guint64, guint64> * ZDDWork::CleanPC(list<dd_tuple> * in_list)
{
  list<dd_tuple>::iterator temp_Liter;
  map<guint64, guint64>::iterator temp_Miter;
  map<guint64, guint64> * return_map = new map<guint64, guint64>; //! sometimes I dream of Java.  Just sometimes.

  //! now move through the list of DINxPC values
  for(temp_Liter = in_list->begin(); temp_Liter != in_list->end(); temp_Liter++)
    {
      (*return_map)[temp_Liter->y]++;
    }
    

  return (return_map);
} 


/*!
  Initialize a ZDD Package
*/
void ZDDWork::DDPackageInit(dd_package * t_package)
{
  t_package->node = NULL;
  t_package->manager = NULL;
}


void ZDDWork::ClearDependents(void)
{    
  HC_Open_Segment_By_Key(localView->GetModel()->GetModelKey());
    
  // clear out the segment
  HC_Open_Segment("dependents");

  HC_Flush_Contents (".", "");

  HC_Close_Segment();
  HC_Close_Segment();  

  // update display
  localView->SetGeometryChanged();
  localView->Update(); 
}


/***************** View Buffer Work  *********************/


/*
  ModfiyPixel - modifies the pixel data in a buffer

  This function modifys the buffer in what will become an
  image.  This assumes each pixel is a simple, single byte, color map.

*/
inline
void ZDDWork::ModifyPixel (quint8 * buffer, quint64 x, 
                           quint64 y, quint8 * data){
    
  //! if the pixel is out of view, discard
  if((y >= imwork->currentBufferHeight) || (x >= imwork->currentBufferWidth)){
    return;
  }

  //! account for the row with wrap around
  quint32 position = (y * imwork->currentBufferWidth * imwork->buffer_pixel_size);
        
  //! account for the column position
  position += (x * imwork->buffer_pixel_size);
        
  //! set the new pixel data
  //    for(int i = 0; i < imwork->buffer_pixel_size; i++){
  //        buffer[position + i] = data[i];    
  //    }

  buffer[position + imwork->redPos] = data[imwork->redPos];
  buffer[position + imwork->greenPos] = data[imwork->greenPos];
  buffer[position + imwork->bluePos] = data[imwork->bluePos];

  if(imwork->alphaPos < imwork->buffer_pixel_size)
    {
      buffer[position + imwork->alphaPos] = data[imwork->alphaPos];
    }

}

/*  
    Function:UpdateViewBuffer - performs an update on the view
  
    This function updates the view window with the data in the
    current view buffer.
  
*/
void ZDDWork::UpdateViewBuffer(void)
{
  DdNode * local_dinhot = dd_dinhot;
  DdNode * local_node = dd_dinrdy;
  DdManager * local_manager = dd_manager;

  if((local_node != NULL) && (local_manager != NULL))
    {                 
      quint8 * viewBuffer = imwork->getNextViewBuffer();

      if(local_dinhot != NULL)
	{
	  //! now find the new points           
	  range_traversal(local_manager, local_node, local_dinhot,
				     &ZDDWork::HotCodeRecurse);
		      
	}
      else
	{	      
	  //! now find the new points           
	  range_traversal(local_manager, local_node, 
				     &ZDDWork::GraphCollectedNums);
	}

      //! check for any painted analysis results
      if(NULL != dd_dinrdy_overlay)
	{
	  //! save off the old pixel color
	  quint8 * pixel_color_save = g_color_data;
		      
	  //! create and set up the new pixel color holder
	  quint8 new_pixel_color[imwork->buffer_pixel_size];
		      
	  //! set the color as hot pink
	  new_pixel_color[imwork->redPos] = 0xff;
	  new_pixel_color[imwork->greenPos] = 0x69;
	  new_pixel_color[imwork->bluePos] = 0xb4;
	    
	  if(imwork->alphaPos < imwork->buffer_pixel_size)
	    {
	      new_pixel_color[imwork->alphaPos] = 0xFF;
	    }
	    
	  //! set the bdd color data structure to our new
	  //! pixel color
	  g_color_data = new_pixel_color;
		      
	  //! perform the plotting DD traversal
	  range_traversal(dd_manager,
			  dd_dinrdy_overlay, 
			  &ZDDWork::GraphCollectedNums);
		      
	  //! restore the old pixel color
	  g_color_data = pixel_color_save;
	}
		  
		  
      //! check for any painted analysis results
      if(analysisDD.node != NULL)
	{
	  //! save off the old pixel color
	  quint8 * pixel_color_save = g_color_data;
		      
	  //! create and set up the new pixel color holder
	  quint8 new_pixel_color[imwork->buffer_pixel_size];
		      
	  //! set the color as red
	  new_pixel_color[imwork->redPos] = 0xff;
	  new_pixel_color[imwork->greenPos] = 0x00;
	  new_pixel_color[imwork->bluePos] = 0x00;
	    
	  if(imwork->alphaPos < imwork->buffer_pixel_size)
	    {
	      new_pixel_color[imwork->alphaPos] = 0xFF;
	    }
	    
	  //! set the bdd color data structure to our new
	  //! pixel color
	  g_color_data = new_pixel_color;
		      
	  //! perform the plotting DD traversal
	  range_traversal(analysisDD.manager, 
			  analysisDD.node, 
			  &ZDDWork::GraphCollectedNums);
		      
	  //! restore the old pixel color
	  g_color_data = pixel_color_save;
	}
		  
      //! check for any selected regions
      imwork->insertImageSelections(viewBuffer);
    }
}


/*  
    Function:UpdateViewBuffer - performs an update on the view
  
    This function updates the view window with the data in the
    current view buffer.
  
*/
void ZDDWork::UpdateView(void)
{   
  // First update the image buffer
  UpdateViewBuffer();  

  //! if there is a view buffer available
  //! use it to insert a marker into the 
  if(NULL != imwork->view_buffer)
    {
      //! open the model segment
      HC_Open_Segment_By_Key(localView->GetModel()->GetModelKey());
            
      //! delete the last image
      HC_Flush_Contents(".", "image");
      
      if(imwork->alphaPos < imwork->buffer_pixel_size)
	{
	  tempkey = HC_KInsert_Image_By_Ref(0, 
					    0,
					    0, "RGBA", 
					    imwork->currentBufferWidth, 
					    imwork->currentBufferHeight, 
					    imwork->view_buffer);
	}
      else
	{
	  tempkey = HC_KInsert_Image_By_Ref(0, 
					    0,
					    0, "RGB", 
					    imwork->currentBufferWidth, 
					    imwork->currentBufferHeight, 
					    imwork->view_buffer);
	  
	}
       
      HC_Close_Segment();             
    }
}


/**Function********************************************************************

   Synopsis [Filters out code that does not contribute to the final {DIN,RDY}]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * ZDDWork::UniqueStaticSliceFilter(DdManager * manager, DdNode * dinrdyDD, DdNode * dindinDD)
{
  quint64 topRightx, topRighty;
  quint64 topRightxPlot, topRightyPlot;

  //! initial points setup
  topRightx = imwork->currentBufferWidth;
  topRighty = 0;

  //! convert those points to ZDD 
  //! plot coordinates (DINxRDY usually)
  imwork->Pixel2Plot(topRightx, topRighty, &topRightxPlot, &topRightyPlot);

  DdNode * zero = Cudd_ReadZero(manager);
  Cudd_Ref(zero);

  // Build a tuple that represents our ready time 
  DdNode * rdyNode = build_tuple(manager, zero, 0, topRightxPlot); 
  Cudd_RecursiveDerefZdd(manager, zero); 

  // Add in the universal set of X values
  DdNode * tmpSliceNode = xDC(manager, rdyNode);
  Cudd_RecursiveDerefZdd(manager, rdyNode);
  
  // Intersect our {univ,rdy} with {din,rdy}
  DdNode * intersectNode = Cudd_zddIntersect(manager, dinrdyDD, tmpSliceNode);
  Cudd_Ref(intersectNode);
  Cudd_RecursiveDerefZdd(manager, tmpSliceNode);

  // Abstract away the rdy
  DdNode * dinOnlyDD = abstractY(manager, intersectNode);
  Cudd_RecursiveDerefZdd(manager, intersectNode);

  
  

  
  // Build a slice from this {DIN}
  DdNode * dinSlice = BuildIterReverseSlice(manager, dinOnlyDD, dindinDD, 0);
  Cudd_RecursiveDerefZdd(manager, dinOnlyDD);

  // Build {DIN, univ}
  DdNode * rdySliceNode = yDC(manager, dinSlice);
  Cudd_RecursiveDerefZdd(manager, dinSlice);
  
  DdNode * rdyIntersectNode = Cudd_zddIntersect(manager, dinrdyDD, rdySliceNode);
  Cudd_Ref(rdyIntersectNode);
  Cudd_RecursiveDerefZdd(manager, rdySliceNode);

  return(rdyIntersectNode);
}


/**Function********************************************************************

   Synopsis [Filters out code that does not contribute to the final {DIN,RDY}]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * ZDDWork::uniqueStaticSliceFilterRecur(DdManager * manager, 
					       DdNode * univsinDD,
					       DdNode * dindinDD,
					       DdNode * dinsinDD,
					       DdNode * leftDD, 
					       DdNode * rightDD)
{
  DdNode * dinSliceDD = NULL;
  
  // // Abstract away the {{DIN}} sets
  // DdNode * leftDinOnlyDD = abstractY(manager, leftDD);
  // Cudd_RecursiveDerefZdd(manager, leftDD);
  // DdNode * rightDinOnlyDD = abstractY(manager, rightDD);
  // Cudd_RecursiveDerefZdd(manager, rightDD);
   
  // Build the slice sets
  DdNode * leftDinSlice = BuildIterReverseSlice(manager, leftDD, dindinDD, 0);
  //  Cudd_RecursiveDerefZdd(manager, leftDinOnlyDD);
  DdNode * rightDinSlice = BuildIterReverseSlice(manager, rightDD, dindinDD, 0);
  //  Cudd_RecursiveDerefZdd(manager, rightDinOnlyDD);

  // Create a {DIN, univ} set
  DdNode * tmpLeftSliceNode = yDC(manager, leftDinSlice);
  Cudd_RecursiveDerefZdd(manager, leftDinSlice);
  DdNode * tmpRightSliceNode = yDC(manager, rightDinSlice);
  Cudd_RecursiveDerefZdd(manager, rightDinSlice);

  // Intersect our {DIN, univ} with {din,sin}
  DdNode * leftDinSinDD = Cudd_zddIntersect(manager, dinsinDD, tmpLeftSliceNode);
  Cudd_Ref(leftDinSinDD);
  Cudd_RecursiveDerefZdd(manager, tmpLeftSliceNode);
  DdNode * rightDinSinDD = Cudd_zddIntersect(manager, dinsinDD, tmpRightSliceNode);
  Cudd_Ref(rightDinSinDD);
  Cudd_RecursiveDerefZdd(manager, tmpRightSliceNode);
	  
  // Get rid of the DINs
  DdNode * leftSinOnlyDD = abstractX(manager, leftDinSinDD);
  Cudd_RecursiveDerefZdd(manager, leftDinSinDD);
  DdNode * rightSinOnlyDD = abstractX(manager, rightDinSinDD);
  Cudd_RecursiveDerefZdd(manager, rightDinSinDD);

  // Equal DDs, recur left
  if (leftSinOnlyDD == rightSinOnlyDD)
    {
      guint64 topX = 0;
      guint64 topY = 0;
      guint64 bottomX = 0;
      guint64 bottomY = 0;
      
      GetTupleTop2( manager, leftDD, topX, topY);
      
      GetTupleBottom( manager, leftDD, bottomX, bottomY);    

      int xlist [ZDDNUM/2];
      int ylist [ZDDNUM/2];
      guint64 zddNumHalf = ZDDNUM/2;

      
      //! gather a list of ZDD node variables
      for (unsigned int i = 0; i < zddNumHalf; i+=1)
	{
	  xlist[(zddNumHalf - 1) - i] = i;
	  ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
	}

      //! find the ZDD for the selected area
      DdNode * xUb =  mg_Cudd_zddUb(manager, zddNumHalf, ylist, (uint64_t)topY);
      Cudd_Ref(xUb);
      DdNode * xLb =  mg_Cudd_zddLb(manager, zddNumHalf, ylist, (uint64_t)topY);
      Cudd_Ref(xLb);

      dinSliceDD = uniqueStaticSliceFilterRecur(manager,univsinDD, dindinDD, 
						dinsinDD, leftDD, rightDD);
    }
  else
    {

    }

  return(dinSliceDD);
}


/**Function********************************************************************

   Synopsis []

   Description  ]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
void ZDDWork::GetTopReadyTime(DdNode * dinDinSliceDD, 
			      guint64 &topDin,
			      guint64 &topReady)
{
  // Remove the X variables
  DdNode * dinBlankDD = abstractY( dd_manager, dinDinSliceDD);
    
//   // Swap the X and Y variables
//   DdNode * dinBlankDD = VarSwap( dd_manager, blankDinDD);
//   Cudd_RecursiveDerefZdd(dd_manager, blankDinDD);

  // add in the universal Y
  DdNode * dinUnivDD = yDC(dd_manager, dinBlankDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinBlankDD);
  
  DdNode * newDinRdyDD = Cudd_zddIntersect(dd_manager, dinUnivDD, dd_dinrdy);
  Cudd_Ref(newDinRdyDD);

  GetTupleTop(dd_manager, newDinRdyDD, topDin, topReady);
  Cudd_RecursiveDerefZdd(dd_manager, newDinRdyDD);
}

/**Function********************************************************************

   Synopsis [Filters out code from a select region]

   Description [Filters out code from a select region that does not contribute
   to the final graph results]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * ZDDWork::HotDeadFilterSelection(DdNode * dinSelDD)
{
  if ((dd_manager == NULL) || (dd_dindin == NULL)
      || (dd_dinrdy == NULL)|| (dd_dinhot == NULL))
    {
      return (NULL);
    }

  // Get the top quarter of hot code
  guint64 topX=0, topY=0;
  guint64 bottomX=0, bottomY=0;
  guint64 topPercent = 25;
  guint64 zddNumHalf = ZDDNUM/2;
  int xlist [ZDDNUM/2];
  int ylist [ZDDNUM/2];

  //! gather a list of ZDD node variables                                                                          
  for (unsigned int i = 0; i < zddNumHalf; i+=1)
    {
      xlist[(zddNumHalf - 1) - i] = i;
      ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
    }

  DdNode * dinBlankSelDD = abstractY(dd_manager, dinSelDD);
  DdNode * dinUnivSelDD = yDC(dd_manager, dinBlankSelDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinBlankSelDD);

  DdNode * dinHotSelDD = Cudd_zddIntersect(dd_manager, dinUnivSelDD, dd_dinhot);
  Cudd_Ref(dinHotSelDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnivSelDD);
  printf("Initial DINxHOT Selection Size:%u\n", Cudd_zddCount(dd_manager,dinHotSelDD));

  GetTupleTop(dd_manager, dinHotSelDD, topX, topY);
  printf("DINxHOT Top DIN:%u, HOT:%u\n", (unsigned int)topX,  (unsigned int)topY);
  GetTupleBottom(dd_manager, dinHotSelDD, bottomX, bottomY);
  printf("DINxHOT Bottom DIN:%u, HOT:%u\n", (unsigned int)bottomX,  (unsigned int)bottomY);

  guint64 countHot = topY - bottomY;
  guint64 countHotSub = (guint64)((double)countHot * (double)((double)topPercent) / 100);
  guint64 topHotSel = topY - countHotSub;

  printf("Using the value %u as a lower bound\n", (unsigned int)topHotSel);

  DdNode * yLb =  mg_Cudd_zddLb(dd_manager, zddNumHalf, ylist, (uint64_t)topHotSel);
  Cudd_Ref(yLb);

  DdNode * hotLbUnvDD = xDC(dd_manager, yLb);

  DdNode * topHotDD = Cudd_zddIntersect(dd_manager, hotLbUnvDD, dinHotSelDD);
  Cudd_Ref(topHotDD);
  Cudd_RecursiveDerefZdd(dd_manager, hotLbUnvDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinHotSelDD);
  printf("DINxHOT Size w/LB:%u\n", Cudd_zddCount(dd_manager,topHotDD));

  // Remove the Hot Values
  DdNode * dinEmptyHotDD = abstractY(dd_manager, topHotDD);
  Cudd_RecursiveDerefZdd(dd_manager, topHotDD);
  DdNode * dinUnvHotDD = yDC(dd_manager, dinEmptyHotDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinEmptyHotDD);
  DdNode * dinRdyHotDD = Cudd_zddIntersect(dd_manager, dinUnvHotDD, dd_dinrdy);
  Cudd_Ref(dinRdyHotDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnvHotDD);

  // Remove the dead code from hot DINxRDY DD
  printf("Old DINxRDY Size:%u\n", Cudd_zddCount(dd_manager,dinRdyHotDD));
  DdNode * dinHotAliveDD = DeadReadyFilterSelection(dinRdyHotDD);
  //  Cudd_RecursiveDerefZdd(dd_manager, dinRdyHotDD);
  printf("New DINxRDY Size:%u\n", Cudd_zddCount(dd_manager, dinHotAliveDD));

  return (dinHotAliveDD);
}


/**Function********************************************************************

   Synopsis [Filters out code from a select region]

   Description [Filters out code from a select region that does not contribute
   to the final graph results]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * ZDDWork::QuickDeadFilter(DdNode * dinSelDD)
{
  if ((dd_manager == NULL) || (dd_dindin == NULL))
    {
      return (NULL);
    }

  printf("Taking one slice step forward...\n");

  // Create a {DIN_i,empty} set from our selection
  DdNode * dinEmptySel = abstractY(dd_manager, dinSelDD);

  // Swap the Vars {empty, DIN_d} 
  DdNode * emptyDinSel = VarSwap(dd_manager, dinEmptySel);

  // Create a {univ, DIN_d} set from our {empty, DIN_d} set
  DdNode * univDinSel = xDC(dd_manager, emptyDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, emptyDinSel);
    
  printf("Final forward slice step...\n");

  // Create a {DIN_i, DIN_d} forward step selection set
  DdNode * dinDinForwardDD = Cudd_zddIntersect(dd_manager, dd_dindin, univDinSel);
  Cudd_Ref(dinDinForwardDD);
  Cudd_RecursiveDerefZdd(dd_manager, univDinSel);

  // Now take a step back
  printf("Taking one slice step back...\n");

  // Create a {DIN_i,empty} set from our selection
  DdNode * dinEmptyRevDD = abstractY(dd_manager, dinDinForwardDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinForwardDD);
  
  // Create a {DIN_i, univ_d} set from our {DIN_i, empty} set
  DdNode * dinUnivRevDD = yDC(dd_manager, dinEmptyRevDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinEmptyRevDD);

   // Create a {DIN_i, DIN_d} forward step selection set
  DdNode * dinDinRevDD = Cudd_zddIntersect(dd_manager, dd_dindin, dinUnivRevDD);
  Cudd_Ref(dinDinRevDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnivRevDD);

  // Create a {empty,DIN_d} set from our selection
  DdNode * emptyDinRevDD = abstractX(dd_manager, dinDinRevDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinRevDD );

  // Swap the Vars {DIN,empty} 
  DdNode * dinEmptyRevFinalDD = VarSwap(dd_manager, emptyDinRevDD);
  Cudd_RecursiveDerefZdd(dd_manager,  emptyDinRevDD);

  // Create a {DIN, univ} set from our rev {DIN, empty} set
  DdNode * dinUnivRevFinalDD = yDC(dd_manager, dinEmptyRevFinalDD);
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
DdNode * ZDDWork::DeadReadyFilterSelection(DdNode * dinSelDD)
{
  guint64 xTop = 0;
  guint64 yTop = 0;

  if ((dd_manager == NULL) || (dd_dindin == NULL) || (dd_dinrdy == NULL))
    {
      return (NULL);
    }

  // Create a {DIN,empty} set from our selection
  DdNode * dinEmptySel = abstractY(dd_manager, dinSelDD);

  // Create a {DIN, Univ} set from our {DIN, empty} set
  DdNode * dinUnvSel = yDC(dd_manager, dinEmptySel);
  DdNode * emptyDinSel = VarSwap(dd_manager, dinEmptySel);
  Cudd_RecursiveDerefZdd(dd_manager, dinEmptySel);

  // Create a {DIN,RDY} set
  DdNode * dinRdySelDD = Cudd_zddIntersect(dd_manager, dd_dinrdy, dinUnvSel);
  Cudd_Ref(dinRdySelDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnvSel);

  // Create a {univ, DIN} set from our {empty, DIN} set
  DdNode * univDinSel = xDC(dd_manager, emptyDinSel);
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
  for (unsigned int i = 0; i < zddNumHalf; i+=1)
    {
      xlist[(zddNumHalf - 1) - i] = i;
      ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
    }

  GetTupleBottom2(dd_manager, dinDinSel, smallX, smallY);

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
  DdNode * dinDinSlice = BuildIterDinDinForwardSlice(dinDinSel, dinDinTarget, 0);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinSel);

  printf("Forward slice building complete\n");

  // DEBUG
  GetTopReadyTime(dinDinSlice, xTop, yTop);
  printf("Top Slice Din:%d, Rdy:%d\n", xTop, yTop);

  GetTupleTop(dd_manager, dd_dinrdy, xTop, yTop);

  // DEBUG
  printf("Top Din:%d, Rdy:%d\n", xTop, yTop);

  //  DdNode * rdyNode =  mg_Cudd_zddLb(dd_manager, zddNumHalf, ylist, (uint64_t)yTop);
  //  Cudd_Ref(rdyNode);

  DdNode * zero = Cudd_ReadZero(dd_manager);
  Cudd_Ref(zero);
  
  // Build a tuple that represents our top ready time 
  DdNode * rdyNode = build_tuple(dd_manager, zero, 0, yTop); 
  Cudd_RecursiveDerefZdd(dd_manager, zero); 

  // Add in the universal set of X values
  DdNode * tmpSliceNode = xDC(dd_manager, rdyNode);
  Cudd_RecursiveDerefZdd(dd_manager, rdyNode);
  
  // Intersect our {univ,rdy} with {din,rdy}
  DdNode * intersectNode = Cudd_zddIntersect(dd_manager, dd_dinrdy, tmpSliceNode);
  Cudd_Ref(intersectNode);
  Cudd_RecursiveDerefZdd(dd_manager, tmpSliceNode);

  // Abstract away the rdy
  DdNode * dinOnlyDD = abstractY(dd_manager, intersectNode);
  Cudd_RecursiveDerefZdd(dd_manager, intersectNode);

  // Create a {DIN, Univ} set from our {DIN, empty} set
  dinUnvSel = yDC(dd_manager, dinOnlyDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinOnlyDD);

  // Create a {DIN, {DIN}} reverse slice set
  dinDinSel = Cudd_zddIntersect(dd_manager, dd_dindin, dinUnvSel);
  Cudd_Ref(dinDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, dinUnvSel);

  printf("Building the reverse slice...\n");

  // Build a slice from this {DIN}
  DdNode * revDinDinSlice = BuildIterDinDinReverseSlice(dinDinSel, dinDinSlice, 0, 0);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinSel);
  Cudd_RecursiveDerefZdd(dd_manager, dinDinSlice);

  // Form a {blank, DIN} set
  DdNode * blankDinDD = abstractX(dd_manager, revDinDinSlice);

  // Swap the variable positions to form a {DIN, blank} set
  DdNode * dinBlankDD = VarSwap(dd_manager, blankDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, blankDinDD);

  printf("Reverse slice building complete\n");

  // Build {DIN, univ}
  DdNode * rdySliceNode = yDC(dd_manager, dinBlankDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinBlankDD);
  
  DdNode * dinRdyIntersectNode = Cudd_zddIntersect(dd_manager, dinRdySelDD, rdySliceNode);
  Cudd_Ref(dinRdyIntersectNode);
  Cudd_RecursiveDerefZdd(dd_manager, rdySliceNode);

  return(dinRdyIntersectNode);
}


/**Function********************************************************************

   Synopsis [Filters out code that does not contribute to the final {DIN,RDY}]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode * ZDDWork::DeadReadyFilter(DdManager * manager, DdNode * dinrdyDD, DdNode * dindinDD)
{
  quint64 topRightx, topRighty;
  quint64 topRightxPlot, topRightyPlot;

  //! initial points setup
  topRightx = imwork->currentBufferWidth;
  topRighty = 0;

  //! convert those points to ZDD 
  //! plot coordinates (DINxRDY usually)
  imwork->Pixel2Plot(topRightx, topRighty, &topRightxPlot, &topRightyPlot);

  DdNode * zero = Cudd_ReadZero(manager);
  Cudd_Ref(zero);

  // Build a tuple that represents our ready time 
  DdNode * rdyNode = build_tuple(manager, zero, 0, topRightxPlot); 
  Cudd_RecursiveDerefZdd(manager, zero); 

  // Add in the universal set of X values
  DdNode * tmpSliceNode = xDC(manager, rdyNode);
  Cudd_RecursiveDerefZdd(manager, rdyNode);
  
  // Intersect our {univ,rdy} with {din,rdy}
  DdNode * intersectNode = Cudd_zddIntersect(manager, dinrdyDD, tmpSliceNode);
  Cudd_Ref(intersectNode);
  Cudd_RecursiveDerefZdd(manager, tmpSliceNode);

  // Abstract away the rdy
  DdNode * dinOnlyDD = abstractY(manager, intersectNode);
  Cudd_RecursiveDerefZdd(manager, intersectNode);
  
  // Build a slice from this {DIN}
  DdNode * dinSlice = BuildIterReverseSlice(manager, dinOnlyDD, dindinDD, 0);
  Cudd_RecursiveDerefZdd(manager, dinOnlyDD);

  // Build {DIN, univ}
  DdNode * rdySliceNode = yDC(manager, dinSlice);
  Cudd_RecursiveDerefZdd(manager, dinSlice);
  
  DdNode * rdyIntersectNode = Cudd_zddIntersect(manager, dinrdyDD, rdySliceNode);
  Cudd_Ref(rdyIntersectNode);
  Cudd_RecursiveDerefZdd(manager, rdySliceNode);

  return(rdyIntersectNode);
}



/*
  Function: SetWindow
  
  This function will set the view clipping window
  used during the ZDD traversal.

  Called with: void
  

  Returns: int - return status

  Side effects: modifies - bottomY 
  bottomX
  topY
  topX

*/
int ZDDWork::SetWindow(void){

  int return_value = 1;
  quint64 topRightx, topRighty, bottomLeftx, bottomLefty;
  quint64 topRightxPlot, topRightyPlot, bottomLeftxPlot, bottomLeftyPlot;
  int topx_fudge = -1, topy_fudge = -1, bottomx_fudge = 0, bottomy_fudge = 0;
  int res_fudge = 0;


  //! initial points setup
  topRightx = imwork->currentBufferWidth;
  topRighty = 0;
    
  bottomLeftx = 0;
  bottomLefty = imwork->currentBufferHeight;

  //! convert those points to ZDD 
  //! plot coordinates (DINxRDY usually)
  imwork->Pixel2Plot(topRightx, topRighty, &topRightxPlot, &topRightyPlot);
  imwork->Pixel2Plot(bottomLeftx, bottomLefty, &bottomLeftxPlot, &bottomLeftyPlot);

  //! calculate the least power of 2 that contains the new screen
  bottomY = (gint64)(floor(log2(bottomLeftyPlot)));
  bottomX = (gint64)(floor(log2(bottomLeftxPlot)));
  topY = (gint64)(ceil(log2(topRightyPlot)));
  topX = (gint64)(ceil(log2(topRightxPlot)));

  //! always add some fudge
  bottomY += bottomy_fudge;
  bottomX += bottomx_fudge;
  topY += topy_fudge;
  topX += topx_fudge;            

  //! some safety catches
  if(topX < bottomX)
    {
      bottomX = 0;
    }
  if(topY < bottomY)
    {
      bottomY = 0;
    }

  //! calculate the depth clip
  qint64 y_resolution = ceil(log2((topRightyPlot - bottomLeftyPlot) 
				  / imwork->currentBufferHeight));
                               
  qint64 x_resolution = ceil(log2((topRightxPlot - bottomLeftxPlot) 
				  / imwork->currentBufferWidth));

  //    g_maxDepth = (int)(((qreal)y_resolution + (qreal)(x_resolution))/2);
    
  if(y_resolution < x_resolution){
    g_maxDepth = y_resolution;
        
  }
  else{
    g_maxDepth = x_resolution;
  }
    
  g_maxDepth += res_fudge;
                               
  return(return_value);
}


/*
  Function: SetHotScale
  
  This function will set the translation
  bits for hot code graphing

  Returns: int - return status

  Side effects:

*/
int ZDDWork::SetHotScale(DdManager * manager, DdNode * hotCodeDD)
{

  guint64 topDin = 0;
  guint64 bottomDin = 0;
  hotTop_ = 0;
  hotBottom_ = 0;

  GetTupleTop(manager, hotCodeDD, topDin, hotTop_);
  GetTupleBottom2(manager, hotCodeDD, bottomDin, hotBottom_);

  //  hotSplit_ = (guint64)((double)(hotTop_ + hotBottom_) / 1.5);

  guint64 * hotTopDen = getTopDenseVars(manager, hotCodeDD);
  if(NULL == hotTopDen)
    {
      hotSplit_ = (guint64)((double)(hotTop_ + hotBottom_) / 1.5);
    }
  else
    {
      hotSplit_ = hotTop_;
      int tHotCount = 0;
      while (hotSplit_ >= hotTop_)
	{
	  hotSplit_ = (guint64)ceil((double)((double)hotTop_ + 
					     (double)pow(2, hotTopDen[1] - tHotCount)) / 1.50);
	  ++tHotCount;
	}

      // START DEBUG
      g_print("HotSplitPow2: %"G_GUINT64_FORMAT"\n", (hotTopDen[1] - tHotCount));
      // END DEBUG
    }
  // START DEBUG
  g_print("Hot:%"G_GUINT64_FORMAT"->%"G_GUINT64_FORMAT"->%"G_GUINT64_FORMAT"->%"G_GUINT64_FORMAT"\n", 
	  hotTop_, hotSplit_, (guint64)(hotSplit_ / 20), hotBottom_);
  // END DEBUG

  //  g_print("HotTop: %d, hotsplit:%d, x:%d, y:%d\n", hotTop_, hotSplit_, hotTopDen[0], hotTopDen[1]);

  g_free(hotTopDen);

  return (0);
}


int ZDDWork::GetTupleTop2(DdManager * manager, DdNode * node,
				  guint64 &ptopX, guint64 &ptopY)
{

  DdNode * nextNode = node;
  ptopY = 0;
  ptopX = 0;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);

      if(varIndex > 63)
	{
	  ptopY =  ptopY | mask((guint64)(varIndex - 64));
	}
      else
	{
	  ptopX =  ptopX | mask((guint64)(varIndex));
	}

      nextNode = Cudd_T(nextNode);
    }
  return (0);
}


int ZDDWork::GetTupleBottom2(DdManager * manager, DdNode * node,
			    guint64  &pbottomX, guint64 &pbottomY)
{
  pbottomX = 0;
  pbottomY = 0;

  DdNode * nextNode = node;

  while (!Cudd_IsConstant(nextNode))
    {
      //! Determine the variable index this node represents
      int varIndex = Cudd_NodeReadIndex(nextNode);
      DdNode * tNextNode = Cudd_E(nextNode);

      if (tNextNode == Cudd_ReadZero(manager))
	{
	  if(varIndex > 63)
	    {
	      pbottomY =  pbottomY | mask((guint64)(varIndex - 64));
	    }
	  else
	    {
	      pbottomX =  pbottomX | mask((guint64)(varIndex));
	    }
	  tNextNode = Cudd_T(nextNode);
	}
      nextNode = tNextNode;
    }

  return (0);
}


DdNode * ZDDWork::DinDinForwardSlice(DdNode * sliceDD, DdNode * targetDD)
{
 
  // Form a {DIN, blank} set
  DdNode * dinblnkDD = abstractY(dd_manager, sliceDD);

  // Swap the variable positions to form a {blank, DIN} set
  DdNode * swappedDinNode = VarSwap(dd_manager, dinblnkDD);
  Cudd_RecursiveDerefZdd(dd_manager, dinblnkDD);

  //  printf("Point A\n");
  //  Cudd_CheckKeys(dd_manager);

  // Form a {univ, DIN} set
  DdNode * univDinNode = xDC(dd_manager, swappedDinNode);
  Cudd_RecursiveDerefZdd(dd_manager, swappedDinNode);

  // Get a new {DIN, {DIN}} set
  DdNode * newDinDinDD = Cudd_zddIntersect(dd_manager, univDinNode, targetDD);
  Cudd_Ref(newDinDinDD);
  Cudd_RecursiveDerefZdd(dd_manager, univDinNode);

  //  printf("Point B\n");
  //  Cudd_CheckKeys(dd_manager);

  return (newDinDinDD);
}


/*
  Function: SetScale
  
  This function will attempt
  to calculate the correct scaling values.

  Called with: quint64 highX - the largest X value
  quint64 highY - the largest Y value
  

  Returns: int - return status

  Side effects: modifies - x_scale
  y_scale

*/
int ZDDWork::SetScale(quint64 highX, quint64 highY){
   
  x_scale = (guint64)((qreal)highX / (qreal)(imwork->currentBufferWidth));
  y_scale = (guint64)((qreal)highY / (qreal)(imwork->currentBufferHeight));

  return 0;
}


/*
  Function: Pixel2Pixel

*/
int ZDDWork::Pixel2Pixel(HPoint * pixel)
{
  //! calculate the current window size in pixels
  HPoint camera_field_obj, camera_field_pixels;

  camera_field_obj.x = (0);
  camera_field_obj.y = (0);
        
  HC_Open_Segment_By_Key(localView->GetSceneKey());

  //! calculate the size of the buffer window
  HC_Compute_Coordinates(".", "object", &camera_field_obj, "local pixels", &camera_field_pixels);

  HC_Close_Segment();

  //! if the size of the display is less than the size of the 
  //! image, we need to shift the pixel coordinates
  pixel->x += (((imwork->currentBufferWidth / 2) - camera_field_pixels.x));
  pixel->y += (((imwork->currentBufferHeight / 2) - camera_field_pixels.y)); 

  return (0);   
}


/*
  Function: SetupDepthClipMask
  
  This function sets up the global depth clip mask.  Every
  position in the mask that is 1 is a good value, every position
  that is a 0 is a value not modified due to the depth clip.
  
*/
void ZDDWork::SetupDepthClipMask (void)
{

  guint64 temp_mask = 0;
  g_depth_clip_mask = ~temp_mask;

  for(int u = 0; u < g_maxDepth; u++)
    {
      g_depth_clip_mask = g_depth_clip_mask & ~mask(u);
    } 

}

void ZDDWork::DdSize(DdManager * manager, DdNode * node)
{
  guint64 thighXNumber = 0;
  guint64 tlowXNumber = 0;
  guint64 thighYNumber = 0;
  guint64 tlowYNumber = 0;

  GetTupleTop(manager, node, thighXNumber, thighYNumber);
  GetTupleBottom(manager, node, tlowXNumber, tlowYNumber);

  g_print("Largest X value found is: %"G_GUINT64_FORMAT"\n", 
	  thighXNumber); 
  g_print("Smallest X value found is: %"G_GUINT64_FORMAT"\n", 
	  tlowXNumber); 
  g_print("Largest Y value found is: %"G_GUINT64_FORMAT"\n", 
	  thighYNumber); 
  g_print("Smallest Y value found is: %"G_GUINT64_FORMAT"\n", 
	  tlowYNumber);
}

void ZDDWork::DdSize(DdManager * manager, DdNode * node, guint64 &topX, 
		     guint64 &bottomX, guint64 &topY, guint64 &bottomY)
{
  GetTupleTop(manager, node, topX, topY);
  GetTupleBottom(manager, node, bottomX, bottomY);
}
