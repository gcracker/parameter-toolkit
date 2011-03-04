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

// SimpleHQWidget.h - public interface for the HOOPS/Qt class SimpleHQWidget 
// 
// More about this class 
  
#ifndef SIMPLEHQWIDGET_H 
#define SIMPLEHQWIDGET_H 

#include <glib.h>

// STL things
#include <vector>
#include <list>

// Qt Includes 
#include <QLabel> 
#include <QWidget> 
#include <QMenu> 
#include <QSlider> 

// HOOPS/Qt Includes 
#include "HQWidget.h" 

// CUDD Includes
#include <cudd.h>
#include "util.h"
//#include "cuddInt.h"

#include "dddmp.h"  

//! Other Includes
#include <semaphore.h>

#include "ddtypes.h"
#include "PMBDDCollect.h"
#include "PMDinRdyGraph.h"

#define USE_ZDDS

#ifdef USE_ZDDS
#include "zddwork.h"
#else
#include "bddwork.h"
#endif

using namespace std;

// these are globals needed from non class based main function
extern char * DDstartupFile;
extern int x_res;
extern int y_res;

class ImageWork;
class BFDWork;

class SimpleHQWidget : public HQWidget 
{ 

 Q_OBJECT 

public: 

	SimpleHQWidget(QWidget* parent, const char* name ,  
	  const char * FileName = 0); 
	~SimpleHQWidget(); 

	int OnViewChange(void);
	int OnViewChangeWTimer(void);
    dd_package GetSelectedDD();

    //! Variables
    HPoint selectionRect[2];
    GString * fileOpenLoc;

#ifdef USE_ZDDS
    ZDDWork * ddTools;
#else
    BDDWork * ddTools;
#endif

    BFDWork * bfdTools;
    ImageWork * imageWork;

public slots: 

    void OnLoad();
	void OnSaveFileAs();
	void OnPrint();
	void OnSaveImage();
	void OnSaveRegions();
	void OnAddSelRegions();

	void OnZoomToExtents();
	void OnZoomToWindow();

	void OnZoom();
	void OnOrbit();
	void OnPan();

	void OnCreateSphere(); 
	void OnCreateCone(); 
	void OnCreateCylinder();
	void OnSelectRegion();
	void OnClearSelectedRegions(); 
	void OnAddDD(); 
	void OnSelectedDINs();
	void OnApertureSelect();
	void OnFindDep();
	void OnDeadRdyFilter();
	void OnUniqStaticFilter();

	void OnDDRefresh();
    void GetBFDFile();
    void ViewTimer();


protected: 
    
    //! * function prototypes *
	void SetupView();  
	void Init(); 

    //! * variables *
 
    guint64 start_sin;

private: 

	//! ** Functions **
	void load(const char * filename); 
	DdNode * build_tuple(DdManager *manager, guint64 x, guint64 y);
	void do_slice_anal(DdManager *manager, DdNode * e); 

    //	guint64 recurs_number_builder(recurseFuncStruct recurStruct);

	void calculate_resolution(guint ddresolution,
							  vector <bool> * dd_list);
    void ClearSelected(void);


    //    void PaintDinRdy(guint64 din_num);
    //    void FindDepForDin(guint64 din_num);
    void PlotDINRDY();
    void PrintSinInfo(DdNode * dinSelDD);
    //! function prototypes for ranging
    unsigned short checkHighOnes(int variableIndex);
    unsigned short checkLowOnes(int variableIndex);

    //! DD graphing related functions
    void DDWorkInit(void);
    void DDClear(void);

    // Research related functions
    void Invst1(void);
    void Invst2(void);
    void Invst3(void);
    void InitPicture1(void);
    
	//! ** Variables **

	int num_testers; 

	guint64 largest_right;
	guint view_scaler;
	guint x_scaler;
    guint y_scaler;
    float res_scaler;
    QTimer *view_timer;
    int view_timer_time;
    int view_timer_intervals;
    sem_t now_rendering;
 
}; 
  

//! * Gather ye inline functions here! *

//! This function checks the list of high one values 
//! to deterimine if we equal or exceed the high value
//! If so, the traversal should only take the else edges
//! from this point forward
inline 
unsigned short SimpleHQWidget::checkHighOnes(int variableIndex)
{
    unsigned short return_val = 0;

    //! first check to see if there are any Ones in the
    //! HighOnes vector
    if( false )
        {
            //! check to see which encoded value range we are in
            
        }

    return (return_val);
}


//! This function checks the list of low one values
//! to make sure the traversal only takes the then
//! edge if the current level is equal to the lowOne
//! value.  Except in the case where we have already
//! gaurentted to be above the lowOne range value
inline 
unsigned short SimpleHQWidget::checkLowOnes(int variableIndex)
{
    unsigned short return_val = 0;

    //! first check to see if the lowOnes vector is empty
    if( false )
        {


        }       

    return (return_val);
}


#endif 


