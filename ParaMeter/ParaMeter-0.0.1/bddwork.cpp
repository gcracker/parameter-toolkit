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

// bddwork.cpp - Implementation of various BDD functions for graphing
// 
// More about this class 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <list>
#include "bddwork.h"
#include "imagework.h"

#define RES_ADD 10
#define DEFAULT_RES 13
#define BDDNUM 128
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


BDDWork::BDDWork(HBaseView * t_view, ImageWork * temp_imagework)  
{ 

	loadedDD = false;

    //! initialize the HOOPS base view pointer
    localView = t_view;

    //! no bdd managers to start
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
    zoomFactor = 1;
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

    //! initialize the analysis results BDD
    analysisDD.node = NULL;
    analysisDD.manager = NULL; 
    
    //! initialize the color data for each pixel
    g_color_data = new quint8[temp_imagework->buffer_pixel_size];

    //! if a pixel is not 3 bytes, this will break!!
    assert(temp_imagework->buffer_pixel_size == 3);
    g_color_data[0] = 0x00;
    g_color_data[1] = 0x00;
    g_color_data[2] = 0x00;
    
    //! **** initialize view buffer work ****
    
    //! make a new image work class
    imwork = temp_imagework;
}   


//! Destructor  
BDDWork::~BDDWork() 
{ 

    delete [] g_color_data;

    //! Clean up any imported BDDs    
    int bdd_manager_size = (int) dd_managers.size();

    for(int i = 0; i < bdd_manager_size; i++)
        {
            Cudd_Quit( dd_managers[i].manager );
        }    
} 
  

void BDDWork::Init() 
{ 

} 


/*!
  This function will return an array of guint64 DIN numbers
  that correspond to a given RDY time value
*/
guint64 BDDWork::range_traversal(DdManager *manager, DdNode * e, 
                                 void (BDDWork::*ptr2ddcollect)(collectFuncStruct*))
{
	guint64 num_dins = 0;
    recurseFuncStruct recurStruct;
    
    recurStruct.manager = manager;
    recurStruct.myNode = e;
    recurStruct.setNumber = 0;
    recurStruct.setNumberNot = ~recurStruct.setNumber;
    recurStruct.collectedNumber = 0;
    recurStruct.collectedNumberNot = ~recurStruct.collectedNumber;
    recurStruct.numComplement = 0;
    recurStruct.ptr2ddcollect = ptr2ddcollect;

    //! clear this global out
    g_CollectedNums.clear();

    //! setup the depth clip mask
    SetupDepthClipMask();

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
 * This function will extract all of the DINs for a given
 * Ready time using a recursive procedure
 */
guint64 BDDWork::range_recurs_builder(recurseFuncStruct recurStruct)
{
	guint64 return_num = 0;	//! return number of collected numbers
	DdNode * nextNode;
    int level = 0, varIndex = 0;
    guint64 saved_collected_num = recurStruct.collectedNumber;
    guint64 saved_set_num = recurStruct.setNumber;
    collectFuncStruct t_col_struct;


    //! this should have a value
    assert(variableAssign != NULL);
    
	//! check to see if this node is complemented
	if((Cudd_IsComplement(recurStruct.myNode) == 1))
        {
#if 0
            g_print("%"G_GUINT64_FORMAT": is COMPLEMENTED\n", recurStruct.myNode);
            
#endif
            recurStruct.numComplement++;	
        }		
    
	
	//! check to see if this node is a constant
	if(Cudd_IsConstant(Cudd_Regular(recurStruct.myNode)))
        {
            //! determine if this pointer value is not complemented an odd
            //! number of times, and the constant value is One
            if(((recurStruct.numComplement & 0x1) != 0x1))
                {   
      
                    //! check for "don't care" values
                    if(((recurStruct.collectedNumber == recurStruct.collectedNumberNot)
                        &&(recurStruct.setNumber == recurStruct.setNumberNot)))
                        {
                            //! increment the count of collected numbers
                            return_num++;
                            
                            //! call the function that will handle the collected
                            //! number
                            t_col_struct.manager = recurStruct.manager;
                            t_col_struct.newNum = recurStruct.collectedNumber;
                            t_col_struct.presetNum = recurStruct.setNumber;
                            (*this.*recurStruct.ptr2ddcollect)(&t_col_struct);
                        }
                    else
                        {                    
                              
                            //! expand any "Don't Care" values which show up 
                            //! as skipped levels in the BDD traversal.
                            DCExpand_recur(recurStruct, ((sizeof(guint64)*8)-1), 
                                           recurStruct.setNumber, recurStruct.collectedNumber);
                           
                        }
                }	
        }
	
	else if(MaxNodeDepth(recurStruct.manager, Cudd_Regular(recurStruct.myNode)) == 1)
        {                
            guint64 t_collectedNumberNot = recurStruct.collectedNumberNot;
            guint64 t_setNumberNot = recurStruct.setNumberNot;

            //! really, really dumb way to cheat this
            t_collectedNumberNot = t_collectedNumberNot & g_depth_clip_mask;
            t_setNumberNot = t_setNumberNot & g_depth_clip_mask;

            //! if there is no need to expand any numbers
            //! this will shortcut to the graphing
            //! function
            if(((recurStruct.collectedNumber == t_collectedNumberNot)
                &&(recurStruct.setNumber == t_setNumberNot)))
                {                    
                    //! increment the count of collected numbers
                    return_num++;                    
                            
                    //! call the function that will handle the collected
                    //! number
                    t_col_struct.manager = recurStruct.manager;
                    t_col_struct.newNum = recurStruct.collectedNumber;
                    t_col_struct.presetNum = recurStruct.setNumber;

                    //                    GraphCollectedNums(&t_col_struct);
                    (*this.*recurStruct.ptr2ddcollect)(&t_col_struct);
                }
            else
                {
//                     recurStruct.collectedNumberNot = t_collectedNumberNot;
//                     recurStruct.setNumberNot = t_setNumberNot;             
                    

//                    //! DEBUG
                    //                    dc_count++; 
 
                    //! grab the time for tracking
                    //                    gettimeofday(&dcstime,NULL); 
                    
                    DCExpand_recur(recurStruct, 63, 
                                   recurStruct.setNumber, recurStruct.collectedNumber);

 //                    gettimeofday(&dcetime,NULL); 

//                     totaldctime += ((dcetime.tv_sec + (dcetime.tv_usec/1000000.0)) - 
//                                     (dcstime.tv_sec + (dcstime.tv_usec/1000000.0)));
                }
        }
    else
        {		
            
            //! Determine the variable that this node represents
            varIndex = Cudd_NodeReadIndex(recurStruct.myNode);
            
            //! Determine the permuation at which this variable occurs
            level = Cudd_ReadPerm(recurStruct.manager, varIndex);
            
            assert(varIndex != 65535);

        
            //!***********************************
            //! ** first handle the "Then" node **
            //!***********************************
            
            nextNode = Cudd_T(Cudd_Regular(recurStruct.myNode));	// grab the then node
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
 
            nextNode = Cudd_E(Cudd_Regular(recurStruct.myNode));
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

                    if(variableAssign[level] == DIN)
                        {
                            //! if we have a true value, AND in a 0 into the correct spot in the out_num
                            recurStruct.collectedNumberNot = 
                                (recurStruct.collectedNumberNot & ~mask((guint64)(varIndex - collection_bottom)));
                        }                            
                    if(variableAssign[level] == RDY)
                        {
                            //! if we have a true value, AND in a 0 into the correct spot in the out_num
                            recurStruct.setNumberNot = 
                                (recurStruct.setNumberNot & ~mask((guint64)(varIndex - rdybottom)));
                        }
                            
                    //			g_print("Diving down ELSE branch:%"G_GUINT64_FORMAT"\n", myNode);
                            
                    //! Here is the else recursive call
                    return_num += range_recurs_builder(recurStruct);

                    recurStruct.myNode = save_node;//!restore the current node 
                }
        }
                       
        
    
#if BDD_DEBUG
	g_print("Returning\n");
#endif
	
	return (return_num);
}

/*!
  This function recalculates the maximum depth clip
  according to a new Z axis position
 */
int BDDWork::RedoDepthClip(float newZ)
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
void BDDWork::SetMaxDepthClip(int newMaxClip)
{
    maxDepthClip = newMaxClip;
}


/*!
  Get the maxium depth clipping
 */
int BDDWork::GetMaxDepthClip(void)
{
    return(maxDepthClip);
}


/*!
  Set the minimum depth clipping
 */
void BDDWork::SetMinDepthClip(int newMinClip)
{
    minDepthClip = newMinClip;
}


/*!
  Get the minimum depth clipping
 */
int BDDWork::GetMinDepthClip(void)
{
    return(minDepthClip);
}


/*!
 * This function checks the node depth to see if it is beyond the 
 * set limit
 *
 */
int BDDWork::MaxNodeDepth(DdManager *manager, DdNode * node)
{
    int returnValue = 0;
    int varIndex = 0;
    int level = 0;

    if(useDepth == false)
        {

            returnValue = 1;
        }
    else
        {

            //! Determine the variable that this node represents
            varIndex = Cudd_NodeReadIndex(node);

            //! Normalize this variable for a 64 bit checks
            if(varIndex > 64)
                {
                    varIndex = varIndex - 64;
                }
    
            //! Determine the permuation at which this variable occurs
            //!    level = Cudd_ReadPerm(manager, varIndex);   

            if((variableAssign[level] == RDY) && (g_maxDepth != 0) && (varIndex < g_maxDepth))//! g_maxDepth is an ugly global
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
  \brief Calculates the needed bdd values from a resolution
  \author Graham Price
  This function consists only of side effects, which include
  the following variables:
  -zoomFactor
 */
void BDDWork::CalculateResolutionFactor(void)
{  
    zoomFactor = (g_maxDepth / (start_z));
}


/*!
  This function checks the node to determine if the search
  is still in bounds
 */
int BDDWork::NodeCheckRange(int then, recurseFuncStruct * recurs_struct)
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
                    level = Cudd_ReadPerm(recurs_struct->manager, varIndex); 

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

  This function will return a list of numbers that 
  take into account "don't care" values from BDDs
  and expand them into multiple numbers

 */
guint64 BDDWork::DCExpand(list<guint64> * dcList, guint64 dc_number, guint64 dc_notNumber)
{
    list<guint64>::iterator list_iter;
    list<guint64> temp_list;
    guint64 temp_mask = 0;
    guint64 return_num = 0;

    (*dcList).push_back(dc_number);

    //! short circut
    if(dc_number == dc_notNumber)
        {
            return_num = dc_number;
        }
    else
        {
            //! add additional do not care members
            for(int i = 0; i < 64; i++)
                {
                    temp_mask = mask(i);

                    if((dc_number & temp_mask) != (dc_notNumber & temp_mask))
                        {
                            temp_list = *dcList;
                            guint64 temp_num = 0;

                            for(list_iter = (*dcList).begin(); list_iter != (*dcList).end();list_iter++)
                                {
                                    //! add the opposite value into the temp list
                                    temp_num = (guint64)(*list_iter);
                                    if((temp_num | ~temp_mask) == ~temp_mask)
                                        {
                                            temp_list.push_back(temp_num | temp_mask);
                                        }
                                    else
                                        {
                                            temp_list.push_back(temp_num & ~temp_mask);
                                        }
                                } 
                            *dcList = temp_list;
                            temp_list.clear();
                        }
                }  
        }
    
    return(return_num);
}


/*
  Function: DCExpand_recur: Expand skipped levels in the BDD
  
  Called with: recurseFuncStruct - the recursive

*/
void BDDWork::DCExpand_recur(recurseFuncStruct recurs_struct, int bit, guint64 new_X, guint64 new_Y)
{

    guint64 temp_nX = 0, temp_nY = 0, temp_mask = 0;

    //! print out the new X and Y
    collectFuncStruct t_col_struct;
                           
                                    
    //! call the function that will handle the collected
    //! number
    t_col_struct.manager = recurs_struct.manager;
    t_col_struct.newNum = new_Y;
    t_col_struct.presetNum = new_X;
    (*this.*recurs_struct.ptr2ddcollect)(&t_col_struct);
    //    GraphCollectedNums(&t_col_struct);
   
    for (int i = bit; i > g_maxDepth; i--)
        {

            temp_mask = mask(i);

            //! first expand the X values
            if((new_X & temp_mask) != ( recurs_struct.setNumberNot & temp_mask))
                {
    
                    //! invert this bit
                    if((new_X | ~temp_mask) == ~temp_mask)
                        {
                            temp_nX = (new_X | temp_mask);
                        }
                    else
                        {
                            temp_nX = (new_X & ~temp_mask);
                        }   

                    //! recursive call to DCExpand
                    DCExpand_recur(recurs_struct, i,temp_nX, new_Y);
 
                }

            //! now expand the Y values
            if((new_Y & temp_mask) != (recurs_struct.collectedNumberNot & temp_mask))
                {
                    //! invert this bit
                    if((new_Y | ~temp_mask) == ~temp_mask)
                        {
                            temp_nY = (new_Y | temp_mask);
                        }
                    else
                        {
                            temp_nY = (new_Y & ~temp_mask);
                        }   

                    DCExpand_recur(recurs_struct, i, new_X, temp_nY);

                    //! if both X and Y have expanded on this bit
                    //! then perform the recursion on 
                    if(temp_nX != 0)
                        {
                            DCExpand_recur(recurs_struct, i, temp_nX, temp_nY);
                        }                
                }
        }
}


/*!
  
  Function: CollectedNumStats

  This function will get stats
  on the numbers collected by the
  BDD traversal.

 */
void BDDWork::CollectedNumStats(collectFuncStruct * collStruct)
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
void BDDWork::GraphCollectedNums(collectFuncStruct * collStruct)
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
void BDDWork::StoreCollectedNums(collectFuncStruct * collStruct)
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
 * This function takes a BDD full of DIN values and returns
 * a BDD with the corrisponding dependence DIN BDD
 */
dd_package BDDWork::GetDepDINDD(dd_package dinPack)
{
    dd_package return_pack;
    DdNode * depBDD = NULL;
    DdNode * depBDDtemp = NULL;
    DdNode * cubeBDD = NULL;
    DdNode * dindinBDD = NULL;
    DdManager * dindinMan = NULL;
    //_vec[BDDNUM];
    DdNode * node_vec1[BDDNUM/2];
    DdNode * node_vec2[BDDNUM/2];
    int vector[BDDNUM];
    int i = 0;
    struct timeval startSliceTime, currentSliceTime;

    DDPackageInit(&return_pack);

    assert(dinPack.node != NULL);

    //! try to find all the needed BDDs
    for (unsigned int i = 0; (i < dd_managers.size()); i++)
        {
            manager_group t_manager;
            t_manager =  dd_managers[i]; 
            if(t_manager.type == DINVSDIN)
                {
                    dindinBDD = t_manager.node;
                    dindinMan = t_manager.manager;
                }
        }

    if((dindinMan != NULL) && (dindinBDD != NULL))
        {          
	  
	  gettimeofday(&startSliceTime,NULL); // DEBUG !!!
	  
	  //! build a cube for the extraction
            cubeBDD = build_d1cube(dindinMan, collection_bottom, collection_top);
            Cudd_Ref(cubeBDD);
            
            //! now perform the AND and the extraction
            depBDDtemp = Cudd_bddAndAbstract(dindinMan, dindinBDD, dinPack.node, cubeBDD);
            Cudd_Ref(depBDDtemp);

            // tempBDD = Cudd_bddAnd(dindinMan, dindinBDD, dinPack.node);
            //             Cudd_Ref(tempBDD);

            //             depBDDtemp = Cudd_bddExistAbstract(dindinMan,tempBDD,cubeBDD);
            //             Cudd_Ref(depBDDtemp);
 
            //! generate the node vectors that will be swapped
            for(i=0; i < (BDDNUM/2); i++)
                {
                    node_vec1[i] = Cudd_bddIthVar(dindinMan, i+(BDDNUM/2));
                    Cudd_Ref(node_vec1[i]);
                }
            for(i=0; i < (BDDNUM/2); i++)
                {
                    node_vec2[i] = Cudd_bddIthVar(dindinMan, i);
                    Cudd_Ref(node_vec2[i]);
                }
            
            //! rearrange the variables in this bdd
            //            transBDD = Cudd_bddVectorCompose(temp_manager, transBDDtemp, node_vec);

            //            transBDD = Cudd_bddPermute(temp_manager, transBDDtemp, vector);
            depBDD = Cudd_bddSwapVariables(dindinMan, depBDDtemp, node_vec1, node_vec2, (BDDNUM/2));
            Cudd_Ref(depBDD);
                     
            //            depBDD = Cudd_bddVectorCompose(dindinMan, depBDDtemp, vector);
            //            Cudd_Ref(depBDD);
            //            depBDD = depBDDtemp;
            
            //! BDD CLEANUP
            for (i = 0; i < (BDDNUM/2); i++)
                {
		  Cudd_IterDerefBdd(dindinMan, node_vec1[i]);
		  Cudd_IterDerefBdd(dindinMan, node_vec2[i]);
                }
            Cudd_IterDerefBdd(dindinMan, depBDDtemp);
            Cudd_IterDerefBdd(dindinMan, cubeBDD);
            //            Cudd_IterDerefBdd(dindinMan, tempBDD);
            Cudd_IterDerefBdd(dinPack.manager, dinPack.node);

	    //! DEBUG !!!
	    gettimeofday(&currentSliceTime,NULL); // DEBUG !!!
	    double timeForSlice = (((currentSliceTime.tv_sec + 
				     (currentSliceTime.tv_usec/1000000.0)) - 
				    (startSliceTime.tv_sec + 
				     (startSliceTime.tv_usec/1000000.0))));
	  
	  g_print("Time for 1 slices:%f\n", timeForSlice);
        }
    
    return_pack.node = depBDD;
    return_pack.manager = dindinMan;
    
    return(return_pack);
}


/*!
 * This function, when given a BDD containing the final
 * slice of DIN dependence info, should AND that with
 * the resident DINxRDY BDD and graph the result
 *
 */
int BDDWork::GraphFinalSlice(dd_package depPack)
{
    DdNode * depBDD = depPack.node;
    DdNode * finBDD = NULL;
    DdNode * dinrdyBDD = NULL;
    DdManager * dinrdyMan = NULL;

    assert(depBDD != NULL);

    //! try to find all the needed BDDs
    for (unsigned int i = 0; (i < dd_managers.size()); i++)
        {
            manager_group t_manager;
            t_manager = dd_managers[i]; 
            if(t_manager.type == DINVSRDY)
                {
                    dinrdyBDD = t_manager.node;
                    dinrdyMan = t_manager.manager;
                }
        }

    if((dinrdyBDD != NULL) && (dinrdyMan != NULL))
        {
            //! get the ANDed BDD
            finBDD = Cudd_bddAnd(dinrdyMan, dinrdyBDD, depBDD);
            Cudd_Ref(finBDD);

            if(finBDD != NULL)
                {
                    //! clean out this old BDD first
                    if(sliceDD.node != NULL)
                        {
                            Cudd_IterDerefBdd(sliceDD.manager, sliceDD.node);
                        }

                    //! * This section pulls out the din values from this *
                    //! * final BDD for future slicing iterations *
                    DdNode * cubeBDD = NULL;

                    //! build a cube for the extraction
                    cubeBDD = build_d1cube(dinrdyMan, rdybottom, rdytop);
                    Cudd_Ref(cubeBDD);

                    sliceDD.node = Cudd_bddExistAbstract(dinrdyMan, finBDD, cubeBDD);
                    Cudd_Ref(sliceDD.node);
                    sliceDD.manager = dinrdyMan;

                    //! BDD CLEANUP
                    Cudd_IterDerefBdd(dinrdyMan, cubeBDD);
                }
            

            //! setup the analysis BDD
            if(analysisDD.node != NULL){
                Cudd_IterDerefBdd(analysisDD.manager, analysisDD.node);                
            }
            analysisDD.node = finBDD;
            analysisDD.manager = dinrdyMan;            
            
            //! BDD CLEANUP
                //            Cudd_IterDerefBdd(dinrdyMan, finBDD);
            Cudd_IterDerefBdd(dinrdyMan, depBDD);

        }


    return (0);
}


/*!
  
  This function returns the PC BDD for a given DIN BDD
  Requires a DINxSIN BDD be loaded into the manager arrays
  Returns a single SIN
  \author Graham Price
  \date 07/31/2007

*/
dd_package BDDWork::GetPConlyDD(dd_package dinPack)
{
    dd_package return_pack;
    DdNode * pcBDD = NULL;
    DdNode * cubeBDD = NULL;
    DdNode * tempBDD = NULL;
    DdNode * dinpcBDD = NULL;
    DdManager * dinpcMan = NULL;
    guint64 num_dins = 0;

    DDPackageInit(&return_pack);

    assert(dinPack.node != NULL);

    //! try to find all the needed BDDs
    for (unsigned int i = 0; (i < dd_managers.size()); i++)
        {
            manager_group t_manager;
            t_manager =  dd_managers[i]; 
            if(t_manager.type == DINVSSIN)
                {
                    dinpcBDD = t_manager.node;
                    dinpcMan = t_manager.manager;
                }
        }

    if((dinpcMan != NULL) && (dinpcBDD != NULL))
        {
            //! build a cube for the extraction
            cubeBDD = build_d1cube(dinpcMan, collection_bottom, collection_top);

            Cudd_Ref(cubeBDD);
            
            //! now perform the AND and the extraction
            //            depBDD = Cudd_bddAndAbstract(dindinMan, dinBDD, dindinBDD, cubeBDD);
            tempBDD = Cudd_bddAnd(dinpcMan, dinPack.node, dinpcBDD);
            Cudd_Ref(tempBDD);

            //! before we remove the rdy time values, we should collect
            //! the pc values from this BDD
             guint64 t_maxDepth = g_maxDepth;
             int t_topx = topX;
             int t_bottomx = bottomX;
             int t_topy = topY;
             int t_bottomy = bottomY;
             
            //! this is ugly; I am saving the recursive search
            //! limiting values, then removing them for this call

             g_maxDepth = 0;
             topX = bottomX = topY = bottomY = -1;
//             useRange = false;
//             useDepth = false;

            num_dins = range_traversal(dinpcMan, tempBDD, 
                                       &BDDWork::StoreCollectedNums);
//             useRange = true;
//             useDepth = true;

//            g_print("Collected %d\n",g_CollectedNums.size()); //!DEBUG

            //! restoring the limiting values
                       g_maxDepth = t_maxDepth;
                       topX = t_topx;
                       bottomX = t_bottomx;
                       topY = t_topy;
                       bottomY = t_bottomy;

            pcBDD = Cudd_bddExistAbstract(dinpcMan,tempBDD,cubeBDD);
            Cudd_Ref(pcBDD);
            
            //! BDD CLEANUP
            Cudd_IterDerefBdd(dinpcMan, cubeBDD);
            Cudd_IterDerefBdd(dinpcMan, tempBDD);
            Cudd_IterDerefBdd(dinPack.manager, dinPack.node);
        }

    return_pack.node = pcBDD;
    return_pack.manager = dinpcMan;

    return(return_pack); 
}


/*!
  This function returns a BDD that contains the area selected
  if the BDD contains (X,Y), 
*/
DdNode * BDDWork::GraphSelect(DdManager * manager, guint64 x_min, guint64 x_max,
                              guint64 y_min, guint64 y_max)
{
    guint64 bddNumHalf = BDDNUM/2;

    DdNode ** xlist = new DdNode * [BDDNUM/2];
    DdNode ** ylist = new DdNode * [BDDNUM/2];
    DdNode * tempNodeX = NULL;
    DdNode * tempNodeY = NULL;
    DdNode * returnBDD = NULL;


    for (unsigned int i = 0; i < bddNumHalf; i++) 
        {
            xlist[i] = NULL;
            ylist[i] = NULL;
        }
    
    //! gather a list of BDD node variables
    for (unsigned int i = 0; i < bddNumHalf; i+=1)
        {
            xlist[(bddNumHalf - 1) - i] = Cudd_ReadVars(manager,i);
            ylist[(bddNumHalf - 1) - i] = Cudd_ReadVars(manager,i + bddNumHalf);
            
            if((Cudd_ReadVars(manager,i + bddNumHalf) == NULL) || 
               (Cudd_ReadVars(manager,i) == NULL))
                {
                    //                    g_print("Null Variable\n");
                    assert(false);
                }
        }

    //! find the BDD for the selected area
    tempNodeX =  Cudd_bddInterval(manager, bddNumHalf, xlist,(guint64)x_min, (guint64)x_max);
    Cudd_Ref(tempNodeX);

    //! find the BDD for the selected area
    tempNodeY =  Cudd_bddInterval(manager, bddNumHalf, ylist,(guint64)y_min, (guint64)y_max);
    Cudd_Ref(tempNodeY);

    //! now perform the AND
    returnBDD = Cudd_bddAnd(manager, tempNodeX, tempNodeY);
    Cudd_Ref(returnBDD);
    
    //! CLEANUP
    Cudd_IterDerefBdd(manager, tempNodeX);
    Cudd_IterDerefBdd(manager, tempNodeY);
    delete xlist;
    delete ylist;

    return returnBDD;
}


/*!

  This function will return an array of guint64 DIN numbers
  that correspond to a given RDY time value

  THIS IS DEADWOOD

*/
guint64 BDDWork::get_ready_time(DdManager *manager, DdNode * e, 
                                guint64 rdytime,
                                void (BDDWork::*ptr2ddcollect)(collectFuncStruct*))
{
    guint64 num_dins = 0;
    recurseFuncStruct recurStruct;
    
    recurStruct.manager = manager;
    recurStruct.myNode = e;
    recurStruct.setNumber = rdytime;
    recurStruct.collectedNumber = 0;
    recurStruct.collectedNumberNot = ~recurStruct.collectedNumber;
    recurStruct.numComplement = 0;
    recurStruct.ptr2ddcollect = ptr2ddcollect;

    //! clear this global out
    g_CollectedNums.clear();

    //! perform the recursive DIN number finder	
    //    num_dins = recurs_number_builder(recurStruct); ** !!! ** NEED TO REPLACE
    

    //	g_print("found ready:%"G_GUINT64_FORMAT",DINS:%d\n", rdytime, num_dins);

    //! \warning Optimization for DIN vs RDY
    //!
    //! if the new lowest number from the recent dive
    //! is larger than the previous, set g_lowOne
    if(tempLowOne > g_lowOne)
        {
            g_lowOne = tempLowOne;
        }
    
    return(num_dins);	
}


/*! 
 * Simple mask function
 */
guint64 BDDWork::mask(guint64 v) {
    return ((guint64)(1) << v);
}

DdNode * BDDWork::build_d1cube(DdManager *manager, int bottom, int top) {
    DdNode *ret;
    //  DdNode *tmp[1];
    //  FILE *f;
    int cubeArray[BDDNUM];
    
    for(int i = 0; i < BDDNUM; i++)
        {
            if((i >= bottom) && (i <= top))
                {
                    cubeArray[i] = 1;                    
                }
            else
                {
                    cubeArray[i] = 2;
                }

        }
    
    ret = Cudd_CubeArrayToBdd(manager, cubeArray);
    


    // ret = build_xstar_range(manager,0xffffffffffffffffLL, bottom, top);

    Cudd_Ref(ret);
    /*reorder
      tmp[0]=ret;
      f=fopen("/u/pricegd/tmpfil.dot","w+");
      Cudd_DumpDot(manager, 1, tmp, NULL, NULL, f);
      fclose(f);
    */

    return ret;
}

DdNode * BDDWork::build_xstar_range(DdManager *manager, guint64 x, int bottomR, int topR)
{
    int i;
    DdNode * tmp, * itenode, * logicZero;
	
    logicZero = Cudd_ReadLogicZero(manager);
    Cudd_Ref(logicZero);

    //	printf("Adding %llx\n", x);

    /* make a new bdd for this tuple */
    itenode = Cudd_ReadOne(manager);
    Cudd_Ref(itenode);

    /* iterate through the 64 bit values for both X and Y */
    for (i = 0; i < (int)(sizeof(guint64) * 8 * 2); i++) {
        guint64 bitset;
        int v;

        // This code only works for a total of 128 BDD vars, beware
        assert(Cudd_ReadSize(manager) <= BDDNUM);
        v= Cudd_ReadInvPerm(manager,127-i);

        tmp = itenode;	  
        if((v >= bottomR) && (v <= topR)) {
            bitset = (x & mask((guint64)(v-bottomR)));
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
        } 
        else {
		
            itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager, v),
                                  tmp,tmp);

        }
        Cudd_RecursiveDeref(manager, tmp);
    }

    Cudd_RecursiveDeref(manager, logicZero);

    return itenode;
}


/**Function********************************************************************
   
   Synopsis    [Generates a BDD for the function x &gt; y.]
   
   Description [This function generates a BDD for the function x &gt; y.
   Both x and y are N-bit numbers, x\[0\] x\[1\] ... x\[N-1\], with 0 the
   most significant bit (important!).
   The BDD is built bottom-up.
   It has 3*N-1 internal nodes, if the variables are ordered as follows: 
   x\[0\] x\[1\] ... x\[N-1\].]
   
   SideEffects [None]
   
   SeeAlso     [Cudd_Xgty]
   
******************************************************************************/
DdNode * BDDWork::Cudd_bddInterval(
                                   DdManager * dd /* DD manager */,
                                   int  N /* number of x and y variables */,
                                   DdNode ** x /* array of x variables */,
                                   guint64 lowerB /* lower bound */,
                                   guint64 upperB /* upper bound */)
{
    DdNode *one = NULL, *zero = NULL;
    DdNode *r = NULL, *rl = NULL, *ru = NULL;
    DdNode *vl = NULL, *vu = NULL;
    int     i;


    if (N == 0) { /* x is 0 */
        return(lsb(lowerB == 0));
    }

    //    one = DD_ONE(dd);
    one = Cudd_ReadOne(dd);
    zero = Cudd_Not(one);

    /* Build bottom part of the two BDDs outside loop. */
    rl = Cudd_bddOr(dd, x[N-1], lsbNot(lowerB));
    if (rl == NULL) return(NULL);
    Cudd_Ref(rl);
    ru = Cudd_bddOr(dd, Cudd_Not(x[N-1]), lsb(upperB));
    if (ru == NULL) {
        Cudd_IterDerefBdd(dd, rl);
        return(NULL);
    }
    Cudd_Ref(ru);
    lowerB >>= 1;
    upperB >>= 1;

    /* Loop to build the rest of the BDDs. */
    for (i = N-2; i >= 0; i--) {
        vl = Cudd_bddIte(dd, x[i],
                         lowerB&1 ? rl : one,
                         lowerB&1 ? zero : rl);
        if (vl == NULL) {
            Cudd_IterDerefBdd(dd, rl);
            Cudd_IterDerefBdd(dd, ru);
            return(NULL);
        }
        Cudd_Ref(vl);
        Cudd_IterDerefBdd(dd, rl);
        rl = vl;
        vu = Cudd_bddIte(dd, x[i],
                         upperB&1 ? ru : zero,
                         upperB&1 ? one : ru);
        if (vu == NULL) {
            Cudd_IterDerefBdd(dd, ru);
            Cudd_IterDerefBdd(dd, vl);
            return(NULL);
        }
        Cudd_Ref(vu);
        Cudd_IterDerefBdd(dd, ru);
        ru = vu;
        lowerB >>= 1;
        upperB >>= 1;
    }

    /* Conjoin the two bounds. */
    r = Cudd_bddAnd(dd, vl, vu);
    if (r == NULL) {
        Cudd_IterDerefBdd(dd, rl);
        Cudd_IterDerefBdd(dd, ru);
        return(NULL);
    }
    Cudd_Ref(r);
    Cudd_IterDerefBdd(dd, rl);
    Cudd_IterDerefBdd(dd, ru);
    Cudd_Deref(r);
 
    return(r);

} /* end of Cudd_bddInterval */


HPoint BDDWork::Plot2World(guint64 x, guint64 y)
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

int BDDWork::World2Plot( HPoint p, guint64 * x, guint64 * y)
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
map<guint64, guint64> * BDDWork::CleanPC(list<dd_tuple> * in_list)
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
  Initialize a BDD Package
 */
void BDDWork::DDPackageInit(dd_package * t_package)
{
    t_package->node = NULL;
    t_package->manager = NULL;
}


void BDDWork::ClearDependents(void)
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
void BDDWork::ModifyPixel (quint8 * buffer, quint64 x, 
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

    //! NOTE: This is basically manual loop unrolling
    buffer[position] = data[0];
    buffer[position+1] = data[1];
    buffer[position+2] = data[2];
}


/*  
  Function:UpdateViewBuffer - performs an update on the view
  
  This function updates the view window with the data in the
  current view buffer.
  
 */
int BDDWork::UpdateViewBuffer(void){
    
    int return_value = 0;

    //! if there is a view buffer available
    //! use it to insert a marker into the 
    if(imwork->view_buffer != NULL)
        {

            //! return a 1
            return_value = 1;

            //! open the model segment
            HC_Open_Segment_By_Key(localView->GetModel()->GetModelKey());
            
            //! delete the last image
            HC_Flush_Contents(".", "image");

            tempkey = HC_KInsert_Image_By_Ref(0, 
                                              0,
                                              0, "RGB", 
                                              imwork->currentBufferWidth, 
                                              imwork->currentBufferHeight, 
                                              imwork->view_buffer);

            //! increment the buffers
            //            imwork->cur_vbuffer = (imwork->cur_vbuffer + 1) % imwork->cur_vbuffer_mod;
            //            imwork->view_buffer = imwork->vbuffer[imwork->cur_vbuffer];

            //!! DEBUG
            //            imwork->view_buffer = imwork->vbuffer[1];
            
            //! initialize the buffer
            //            imwork->init_view_buffer(imwork->view_buffer);
            
            HC_Close_Segment();             
        }

    return (return_value);
}


/*
  Function: SetWindow
  
  This function will set the view clipping window
  used during the BDD traversal.

  Called with: void
  

  Returns: int - return status

  Side effects: modifies - bottomY 
                           bottomX
                           topY
                           topX

*/
int BDDWork::SetWindow(void){

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

    //! convert those points to BDD 
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
  Function: SetScale
  
  This function will attempt
  to calculate the correct scaling values.

  Called with: quint64 highX - the largest X value
               quint64 highY - the largest Y value
  

  Returns: int - return status

  Side effects: modifies - x_scale
                           y_scale

*/
int BDDWork::SetScale(quint64 highX, quint64 highY){
   
    x_scale = (guint64)((qreal)highX / (qreal)(imwork->currentBufferWidth));
    y_scale = (guint64)((qreal)highY / (qreal)(imwork->currentBufferHeight));

    return 0;
}


/*
  Function: Pixel2Pixel

 */
int BDDWork::Pixel2Pixel(HPoint * pixel)
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
void BDDWork::SetupDepthClipMask (void)
{

    guint64 temp_mask = 0;
    g_depth_clip_mask = ~temp_mask;

    for(int u = 0; u < g_maxDepth; u++)
        {
            g_depth_clip_mask = g_depth_clip_mask & ~mask(u);
        } 

}

