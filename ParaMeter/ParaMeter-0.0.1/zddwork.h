
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

// zddwork.h
  
#ifndef ZDDWORK_H 
#define ZDDWORK_H 

#include <glib.h>

// STL things
#include <vector>
#include <list>
#include <map>

// Qt Includes 
#include <QLabel> 
#include <QWidget> 
#include <QMenu> 
#include <QSlider> 

// HOOPS/Qt Includes 
#include "HQWidget.h" 

// CUDD Includes
#include <cudd.h>
//#include "cuddInt.h"
#include "dddmp.h"  
#include <math.h>

// DD Type Include
#include "ddtypes.h"

using namespace std;


//! forward class declarations
class ZDDWork;
class BDDWork;
class ImageWork;


typedef struct recurseFuncStructZdd {
  DdManager *manager;
  DdNode * myNode;
  DdNode * hotDD;
  guint64 setNumber;
  guint64 setNumberNot;
  guint64 collectedNumber;
  guint64 collectedNumberNot;
  int numComplement;
  void (ZDDWork::*ptr2ddcollect)(collectFuncStruct*);
};

class ZDDWork 
{ 
    
 public: 

  //! Constructors and Destructors
  ZDDWork(HBaseView * t_View, ImageWork * temp_imwork); 
  ZDDWork(const ZDDWork &zddWorkRef); 
  ~ZDDWork();

  //! ** Operator Overloads
  ZDDWork & operator=(const ZDDWork & rhs);


  //! ** function prototypes **

  //! what to do with the numbers as they are collected
  void GraphCollectedNums(collectFuncStruct* collStruct);
  void CollectedNumStats(collectFuncStruct* collStruct);
  void StoreCollectedNums(collectFuncStruct* col_struct);
  void HotCodeRecurse(collectFuncStruct * collStruct);
  guint64 range_traversal(DdManager *manager, DdNode * e, 
			  void (ZDDWork::*ptr2ddcollect)(collectFuncStruct*));
  guint64 range_traversal(DdManager *manager, DdNode * e, DdNode * hotDD,
			  void (ZDDWork::*ptr2ddcollect)(collectFuncStruct*));
  guint64 range_recurs_builder(recurseFuncStructZdd recurStruct);

  int MaxNodeDepth(DdManager *manager, DdNode * node);
  int NodeCheckRange(int then, recurseFuncStructZdd * recus_struct);
  dd_package GetPConlyDD(dd_package dinPack);
  dd_package GetDepDINDD(dd_package dinPack);
  int GraphFinalSlice(dd_package depPack);
  guint64 get_ready_time(DdManager *manager, DdNode * e, guint64 rdytime, 
			 void (ZDDWork::*ptr2ddcollect)(collectFuncStruct*));

  DdNode * GraphSelect(DdManager * manager, guint64 x_min, guint64 x_max,
		       guint64 y_min, guint64 y_max);

  int RedoDepthClip(float z);
  void ClearDependents(void);
  void DdSize(DdManager * manager, DdNode * node);
  void DdSize(DdManager * manager, DdNode * node, guint64 &topX, 
	      guint64 &bottomX, guint64 &topY, guint64 &bottomY);


  //! utility functions
  guint64 mask(guint64 v);
  map<guint64, guint64> * CleanPC(list<dd_tuple> * in_list);
  void DDPackageInit(dd_package * t_package);
  void CalculateResolutionFactor(void);
  void SetMaxDepthClip(int newMaxClip);
  int GetMaxDepthClip(void);
  void SetMinDepthClip(int newMinClip);
  int GetMinDepthClip(void);
  void SetupDepthClipMask(void);
  guint64 hot_recurs_builder(recurseFuncStructZdd recurStruct);
  int SetHotScale(DdManager * manager, DdNode * hotCodeDD);
  int GetTupleTop(DdManager * manager, DdNode * node, 
		  guint64 &ptopX, guint64 &ptopY);
  int GetTupleTop2(DdManager * manager, DdNode * node, 
		   guint64 &ptopX, guint64 &ptopY);
  int GetTupleBottom(DdManager * manager, DdNode * node, 
		  guint64 &pbottomX, guint64 &pbottomY);
  int GetTupleBottom2(DdManager * manager, DdNode * node, 
		  guint64 &pbottomX, guint64 &pbottomY);

  void GetTopReadyTime(DdNode * dinDinSliceDD, 
			      guint64 &topDin,
		       guint64 &topReady);
  guint64 freemem(void);

  //! point translation functions
  int Pixel2Pixel(HPoint * pixel);
  HPoint Plot2World(guint64 x, guint64 y);
  int World2Plot(HPoint p, guint64 * x, guint64 * y);
  int SetWindow(void);
  int SetScale(quint64 highX, quint64 highY);

  //! view buffer functions
  void ModifyPixel (quint8 * buffer, quint64 x, 
		    quint64 y, quint8 * data);
  void UpdateViewBuffer(void);
  void UpdateView(void);

  //! Slicing functions
  DdNode * VarSwap(DdManager * manager, DdNode * node);
  DdNode * abstractY(DdManager *manager, DdNode * sliceNode);
  DdNode * abstractX(DdManager *manager, DdNode * sliceNode);
  DdNode * xDC(DdManager *manager, DdNode * node);
  DdNode * yDC(DdManager *manager, DdNode * node);
  DdNode * BuildIterDinDinForwardSlice(DdNode * sliceNode, 
				       DdNode * local_dindinDD, 
				       guint64 stopCount);
  DdNode * DinDinForwardSlice(DdNode * sliceDD, DdNode * targetDD);
  DdNode * BuildIterDinDinReverseSlice(DdNode * sliceNode, 
				       DdNode * local_dindinDD, 
				       guint64 depthRes,
				       guint64 stopCount);
  DdNode * DinDinReverseSlice(DdNode * sliceDD, DdNode * targetDD);
  DdNode * ReverseSlice(DdManager * manager, 
			DdNode * sliceNode, 
			DdNode * targetNode);
  DdNode * BuildReverseSlice(DdManager * manager, DdNode * sliceNode, DdNode * targetNode);
  DdNode * BuildIterReverseSlice(DdManager * manager, DdNode * sliceNode, 
			       DdNode * targetNode);
  DdNode * BuildIterReverseSlice(DdManager * manager, DdNode * sliceNode, 
			       DdNode * targetNode, guint64 stopCount);
  DdNode * LongReverseSlice(DdManager * manager, DdNode * targetNode);
  DdNode * build_fullcube(DdManager *manager, guint64 x, guint64 y);
  DdNode * DeadReadyFilter(DdManager * manager, 
			   DdNode * dinrdyDD, DdNode * dindinDD);
  DdNode * QuickDeadFilter(DdNode * dinSelDD);
  guint64 * getTopDenseVars(DdManager * manager, DdNode * node);
  //! ZDD tuple creation function
  DdNode * build_tuple(DdManager * manager, DdNode *set, 
		       guint64 x, guint64 y);

  //! Filtering functions
  DdNode * uniqueStaticSliceFilterRecur(DdManager * manager, 
					DdNode * univsinDD,
					DdNode * dindinDD,
					DdNode * dinsinDD,
					DdNode * leftDD, 
					DdNode * rightDD);

  DdNode * UniqueStaticSliceFilter(DdManager * manager, 
				   DdNode * dinrdyDD, 
				   DdNode * dindinDD);
  DdNode * DeadReadyFilterSelection(DdNode * dinRdySelDD);
  DdNode * HotDeadFilterSelection(DdNode * dinRdySelDD);
  bool toggleUniqueStaticFilter();
  bool toggleDeadReadyFilter();

  
  //! ** variables **

  vector<manager_group> dd_managers;	
  list <dd_tuple> g_CollectedNums;
  DdNode * dd_dinrdy;
  DdNode * dd_dinrdy_original;
  DdNode * dd_dinrdy_filtered;
  DdNode * dd_dindin;
  DdNode * dd_dinsin;
  DdNode * dd_dinhot;
  DdNode * dd_dinsys;
  DdNode * dd_dinrdy_overlay;
  gchar * dd_dinrdy_filename;
  gchar * dd_dindin_filename;  
  gchar * dd_dinsin_filename;  
  gchar * dd_dinhot_filename;
  gchar * dd_dinsys_filename;
  gchar * dd_dinrdy_overlay_filename;

  DdManager * dd_manager;
  bool doDeadReadyFilter;
  bool doUniqStaticFilter;
  bool loadedDD;
  bool useRange;
  bool useDepth;
  float start_x;
  float start_y;
  float start_z;
  float x_offset;
  float y_offset;
  int topY;
  int bottomY;
  int topX;
  int bottomX;
  int g_lowOne;
  guint64 hotTop_;
  guint64 hotBottom_;
  guint64 hotSplit_;
  unsigned int collection_top;
  unsigned int collection_bottom;
  guint64 highColNumber;
  guint64 lowColNumber;
  guint64 highSetNumber;
  guint64 lowSetNumber;
  int g_maxDepth;
  short * variableAssign;
  int rdybottom;
  int tempLowOne;
  short int * testers;
  int rdytop;
  struct timeval sliceStartTime;
  struct timeval sliceEndTime;
  int g_allRange;
  float zoomFactor;
  dd_package sliceDD;
  dd_package analysisDD;
  dd_package hotcodeDD;
  quint8 * g_color_data;
  guint64 g_depth_clip_mask;
  DdNode * yFullCubeNode;
  DdNode * xFullCubeNode;
    
  //! time-adjusted resolution levels
  float viewRefreshTime;
  float maxRefreshTime;
  float minRefreshTime;
  bool useTimeResolution;

 protected: 

 private: 
  HBaseView * localView; //! HOOPS crap
  int maxDepthClip;
  int minDepthClip;

  ImageWork * imwork;    
};

#endif 


