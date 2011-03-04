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

// bddwork.h
// 
// More about this class 
  
#ifndef BDDWORK_H 
#define BDDWORK_H 

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
class BDDWork;
class ImageWork;


typedef struct recurseFuncStruct {
    DdManager *manager;
    DdNode * myNode;
    guint64 setNumber;
    guint64 setNumberNot;
    guint64 collectedNumber;
    guint64 collectedNumberNot;
    int numComplement;
    void (BDDWork::*ptr2ddcollect)(collectFuncStruct*);
};


class BDDWork 
{ 
    
 public: 

	BDDWork(HBaseView * t_View, ImageWork * temp_imwork); 
    ~BDDWork(); 


    //! ** function prototypes **

    //! what to do with the numbers as they are collected
    void GraphCollectedNums(collectFuncStruct* collStruct);
    void CollectedNumStats(collectFuncStruct* collStruct);
    void StoreCollectedNums(collectFuncStruct* col_struct);
    guint64 DCExpand(list<guint64> * dcList, guint64 dc_number, 
                     guint64 dc_notNumber);
    void DCExpand_recur(recurseFuncStruct recurs_struct, int bit,
                        guint64 new_din, guint64 new_ready);
        
    guint64 range_traversal(DdManager *manager, DdNode * e, 
                            void (BDDWork::*ptr2ddcollect)(collectFuncStruct*));
    guint64 range_recurs_builder(recurseFuncStruct recurStruct);
    int MaxNodeDepth(DdManager *manager, DdNode * node);
    int NodeCheckRange(int then, recurseFuncStruct * recus_struct);
    dd_package GetPConlyDD(dd_package dinPack);
    dd_package GetDepDINDD(dd_package dinPack);
    dd_package hotcodeDD;
    int GraphFinalSlice(dd_package depPack);
	guint64 get_ready_time(DdManager *manager, DdNode * e, guint64 rdytime, 
                           void (BDDWork::*ptr2ddcollect)(collectFuncStruct*));

    DdNode * GraphSelect(DdManager * manager, guint64 x_min, guint64 x_max,
                         guint64 y_min, guint64 y_max);

    DdNode * Cudd_bddInterval( DdManager * dd /* DD manager */,
                               int  N /* number of x and y variables */,
                               DdNode ** x /* array of x variables */,
                               guint64 lowerB /* lower bound */,
                               guint64 upperB /* upper bound */);
    int RedoDepthClip(float z);
    void ClearDependents(void);

    //! utility functions
    guint64 mask(guint64 v);
	DdNode * build_d1cube(DdManager *manager, int bottom, int top);	
	DdNode * build_xstar_range(DdManager *manager, guint64 x, int bottomR, int topR);
    map<guint64, guint64> * CleanPC(list<dd_tuple> * in_list);
    void DDPackageInit(dd_package * t_package);
    void CalculateResolutionFactor(void);
    void SetMaxDepthClip(int newMaxClip);
    int GetMaxDepthClip(void);
    void SetMinDepthClip(int newMinClip);
    int GetMinDepthClip(void);
    void SetupDepthClipMask(void);

    //! point translation functions
    int Pixel2Pixel(HPoint * pixel);
    HPoint Plot2World(guint64 x, guint64 y);
    int World2Plot(HPoint p, guint64 * x, guint64 * y);
    int SetWindow(void);
    int SetScale(quint64 highX, quint64 highY);

    //! view buffer functions
    void ModifyPixel (quint8 * buffer, quint64 x, 
                      quint64 y, quint8 * data);
    int UpdateViewBuffer(void);


    //! ** variables **

	vector<manager_group> dd_managers;	
    list <dd_tuple> g_CollectedNums;
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
    quint8 * g_color_data;
    guint64 g_depth_clip_mask;
    
    //! time-adjusted resolution levels
    float viewRefreshTime;
    float maxRefreshTime;
    float minRefreshTime;
    bool useTimeResolution;

 protected: 
	void Init(); 

 private: 
    HBaseView * localView; //! HOOPS crap
    int maxDepthClip;
    int minDepthClip;

    ImageWork * imwork;    
};

#endif 
