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

// SimpleHQWidget.cpp - Implementation of the HOOPS/Qt class SimpleHQWidget
// 
// More about this class 
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <list>
#include <stdint.h>

// qt includes
#include <QImage>
#include <QImageWriter>
#include <QCoreApplication>
#include <QPaintEngine>
#include <QPainter>
#include <QPaintDevice>
#include <QPrinter>
#include <QFileDialog>
#include <QLabel> 
#include <QMessageBox>
#include <QMenu> 
#include <QCursor> 
#include <QSlider>
#include <QLayout>
#include <QLineEdit>
#include <QTimer>
#include <QColorDialog>
#include <QColor>
#include <QTextStream>

// hoops_mvo includes
#include "HDB.h"
#include "HQApplication.h"
#include "HBaseModel.h"
#include "HBaseView.h"
#include "HSelectionSet.h"
#include "HSelectionItem.h"

#include "HModelInfo.h"
#include "HEventInfo.h"
#include "HOpCameraOrbit.h"
#include "HOpCameraZoom.h"
#include "PMZoom.h"
#include "HOpCameraZoomBox.h"
#include "PMPan.h"
#include "HOpCreateSphere.h"
#include "HOpCreateCone.h"
#include "HOpCreateCylinder.h"
#include "HOpSelectAperture.h"
#include "PMSelectArea.h"
#include "HUtility.h"
#include "HStream.h"
#include "HStreamFileToolkit.h"

//#include "HSSelectionSet.h"
#undef null

// the qt/hoops base class
#include "SimpleHQWidget.h"
#include "bddwork.h"
#include "zddwork.h"
#include "bfdwork.h"
#include "imagework.h"

// hoops include
#include "hc.h"


// this is setup in main
extern HDB * m_pHDB;
extern bool g_render;
extern bool dieondone;
extern bool invst;
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

//extern QApplication * HQApplication::myparent;
QWidget * my_parent;


#define Debug_USE_QGL_CONTEXT           0x00000080
#define DDNUM 128
#define VIEW_INTVS 1
//#define DD_DEBUG
#undef DD_DEBUG
//#define DD_DEBUG
#undef DD_DEBUG2
#define VIEW_BUFFER
#define USE_ZDDS

bool debug_pred = false; //! \warning JUST FOR TEMP DEBUGGING


using namespace std;

SimpleHQWidget::SimpleHQWidget(QWidget* parent, const char* name , const char * filename) 
    : HQWidget( parent, name ) 
{ 
    my_parent = parent;
 
	// Create and initialize HOOPS/MVO Model and View objects 
	m_pHBaseModel = new HBaseModel(); 
	m_pHBaseModel->Init(); 

	// Initialize View object to null ; gets created in SimpleHQWidget::Init 
	m_pHView = 0; 

	// if called with a file name we load it  
	// otherwise open an empty view 
	if(filename) 
        m_pHBaseModel->Read(filename); 

	// enable MouseMoveEvents  
	setMouseTracking(true); 

	// enable key events 
	setEnabled(true); 
	setFocusPolicy(Qt::StrongFocus); 

    //! random init
    fileOpenLoc = g_string_new("/home/pricegd/demo/");

    //! initialize the view timer
    view_timer = new QTimer(this);    
    connect(view_timer, SIGNAL(timeout()), this, SLOT(ViewTimer()));
    view_timer_time = 800;
    view_timer_intervals = VIEW_INTVS;
} 
  

SimpleHQWidget::~SimpleHQWidget() 
{ 
 // Destructor  

  // Clean up any imported DDs  
  for(unsigned int i = 0; i < ddTools->dd_managers.size(); i++)
	{
	  Cudd_Quit( ddTools->dd_managers[i].manager );
	}

  g_string_free(fileOpenLoc, TRUE);

  // Clean up memory 
  if(m_pHView)        delete m_pHView; 
  if(m_pHBaseModel)   delete m_pHBaseModel; 

} 
  

void SimpleHQWidget::SetupView() 
{ 

	// set initial HOOPS/3DGS segment tree attributes for the  
	// HOOPS/MVO View Object 

	m_pHView->FitWorld();  // fit the camera to the scene extents 
	m_pHView->RenderGouraud();  // set the render mode to gouraud 
	m_pHView->SetProjMode(ProjOrthographic);

	// configure view segment  
	HC_Open_Segment_By_Key(m_pHView->GetViewKey()); 
    //HC_Set_Color_By_Index("windows", 0); 
    HC_Set_Color_By_Value("windows","RGB", 1,1,1);
	HC_Set_Selectability("everything = off"); 
	HC_Close_Segment(); 

	// Configure scene/model segment 
	HC_Open_Segment_By_Key(m_pHView->GetSceneKey()); 
	HC_Set_Color_By_Index("faces", 2); 
	HC_Set_Color_By_Index("lights", 1); 
	HC_Set_Color_By_Index("edges, lines", 1); 
    HC_Set_Color("text = black");

    // HC_Set_Rendering_Options ("no color interpolation, no color index interpolation, lod=on, lodo=(algorithm=fast)"); 
    HC_Set_Rendering_Options ("no color interpolation, no color index interpolation, lod=on, lodo=(algorithm=fast)"); 
	HC_Set_Visibility ("lights = (faces = off, edges = off, markers = off), faces=on, edges=off, lines=on, text = on, markers = on, images=on"); 

	HC_Set_Selectability("everything = off, geometry = off, markers = on"); 
	HC_Set_Text_Font("transforms = off"); 

    //! set up makers
    HC_Set_Color_By_Value("markers","RGB", 0.5, 0.5, 0.5);

    //	HC_Set_Color("markers = black"); 
	HC_Set_Marker_Symbol("."); 
	HC_Set_Marker_Size(0.01);
	HC_Set_Heuristics("quick moves, no hidden surfaces"); 

	HC_Close_Segment(); 

	// configure segment for temporary construction geometry 
	HC_Open_Segment_By_Key (m_pHView->GetConstructionKey()); 
	HC_Set_Heuristics("quick moves, no hidden surfaces"); 
	HC_Set_Visibility("faces = off, edges = on, lines = on"); 
	HC_Close_Segment(); 

	// configure windowspace segment for quickmoves 
	HC_Open_Segment_By_Key(m_pHView->GetWindowspaceKey()); 
	HC_Set_Color_By_Index ("geometry", 3); 
	HC_Set_Color_By_Index ("window contrast", 1); 
	HC_Set_Color_By_Index ("windows", 1); 

	HC_Set_Selectability("off"); 

	HC_Close_Segment();  

    //! these variables tweak the depth clipping and ranging DD algorithm
    y_scaler = 1;
    x_scaler = 1;
    res_scaler = 0.5;
} 
  

void SimpleHQWidget::Init() 
{ 
	// setup our HOOPS/MVO Base View, Selection object and current operator 
	// This must be called after SimpleHQWidget's constructor has executed 

	// create and initialize MVO HBaseView object 
#if defined(IS_OSX) && defined(USE_MDI)
	m_pHView = new HBaseView(m_pHBaseModel, NULL, NULL, NULL, GetWindowId(), GetColorMap(), GetClipOverride(), GetQGLContext()); 
#else
	m_pHView = new HBaseView(m_pHBaseModel, NULL, NULL, NULL, GetWindowId(), GetColorMap(), GetClipOverride()); 
#endif
	m_pHView->Init(); 

	// create our Selection object
	HSelectionSet * tempsel = new HSelectionSet(m_pHView);
	tempsel->Init();
	m_pHView->SetSelection(tempsel);

	// Set up the HOOPS/MVO View's HOOPS/3DGS Attributes 
	SetupView(); 

	// Set View's current Operator 
    //	m_pHView->SetCurrentOperator(new HOpCameraPan(m_pHView)); 
    OnPan();

	//DEBUG_STARTUP_CLEAR_BLACK = 0x00004000 clear ogl to black on init update
	HC_Open_Segment_By_Key(m_pHView->GetViewKey());
		HC_Set_Driver_Options("debug = 0x00004000");

#if defined(IS_OSX) && defined(USE_MDI)
		char debug_opts[256];
		sprintf(debug_opts, "set debug = %d", Debug_USE_QGL_CONTEXT);
		HC_Set_Driver_Options(debug_opts);
#endif
	HC_Close_Segment();

	// Call the Views Update Method - initiates HOOPS/3DGS Update_Display  
	m_pHView->Update();


    //! **** DD/Other stuff to be torn out later ****

    //! initialize the start sin from the global
    //! passed in from the command line
    start_sin = g_start_sin;

    //! initialize image work class
    imageWork = new ImageWork();

    //! startup the DD tools
#ifdef USE_ZDDS
    ddTools = new ZDDWork(m_pHView, imageWork);
#else
    ddTools = new BDDWork(m_pHView, imageWork);
#endif

    DDWorkInit();

    //! initialize the BFD toolkit
    bfdTools = new BFDWork();    

    //! end of stuff to be torn out
} 
  

// load a file into our view
void SimpleHQWidget::load(const char * filename)
{
	m_pHView->Flush(true);

	HC_Open_Segment_By_Key(m_pHView->GetSceneKey());
    HC_Flush_Contents (".", "geometry");
	HC_Close_Segment();

	char * extension = (char*)malloc(strlen(filename)+1);

	HUtility::FindFileNameExtension(filename, extension);

	HFileInputResult result;

	if (strstr("hsf", extension)) {

	    m_pHView->GetModel()->GetStreamFileTK()->Restart();

#define HSF_BUFFER_CHUNK 8192
	    char buffer[HSF_BUFFER_CHUNK];
	    int length;
	    FILE *fp;

	    fp = fopen(filename, "rb");
		
 	    HC_Open_Segment_By_Key(m_pHView->GetModel()->GetModelKey());

		while (!feof(fp))
            {
                length = fread(buffer,1,HSF_BUFFER_CHUNK,fp);
                m_pHView->InsertHSFData("file", buffer, length, true);
            }

	    HC_Close_Segment();
	    
	    result = InputOK;
	    m_pHView->GetModel()->GetStreamFileTK()->Restart();

	}
	else 
        {
            result = m_pHBaseModel->Read(filename, m_pHView);
        }

	free(extension);

	if(result == InputVersionMismatch)
		QMessageBox::information( this, "Simple\n",
                                  "This file was created with a newer version of the HOOPS/Stream Toolkit.\n"
                                  "To view it, this application will need to be updated.\n");
	else if(result == InputBadFileName)
		QMessageBox::information( this, "Simple\n",
                                  "A problem occured with the file name provided.\n"
                                  "perhaps you don't have read permision or it is missing.\n");
	else if(result == InputNotHandled)
		QMessageBox::information( this, "Simple\n",
                                  "This program doesn't have a handler for this file operation.\n");
	else if(result != InputOK)
		QMessageBox::information( this, "Simple\n",
                                  "A problem occured during this file operation.\n");
		
	SetupView();

	if (m_pHView->GetCurrentOperator())
		delete m_pHView->GetCurrentOperator();

	m_pHView->SetCurrentOperator(new HOpCameraOrbit(m_pHView));


	m_pHView->Update();
}
  
// print
void SimpleHQWidget::OnPrint()
{ 

	QMessageBox::information(this, "Simple\n",
			"Printing not enabled in qt_simple.\n");

}

void SimpleHQWidget::OnSaveFileAs()
{ 
		

	QFileDialog* qfd = new QFileDialog(this, "qfd");


    // setup selection filter
    QStringList demFilters;
    demFilters.append("HOOPS Metafiles (*.hmf)");
    demFilters.append("HOOPS Binary Stream (*.hsf)");
    demFilters.append("Tagged Image Format (*.tif)");
    demFilters.append("Postscript (*.ps)");
    demFilters.append("HPGL2 (*.hp)");
    demFilters.append("CGM (*.cgm)");
    demFilters.append("HOOPS 3D Stream Control (*.html)");
    qfd->setFilters(demFilters);

    // first get new filename
    qfd->setFileMode(QFileDialog::AnyFile);
	qfd->setAcceptMode(QFileDialog::AcceptSave);

    if (qfd->exec() == QDialog::Accepted){

		QString fn = (qfd->selectedFiles()).front();

		if (!fn.isEmpty()){
		
			int width = 0;
			int height = 0;
			char ext[4096]; 
			char file_name[4096];
			
			strcpy(file_name, fn.toLatin1());

			HUtility::FindFileNameExtension(file_name, ext);

			//TODO add this to a dialog
			if (streq(ext, "tif")){
				width=640;
				height=480;
			}

			if(streq(ext, "html")){
				
				//TODO what is this for?
				char file_tmp[4096];
				HC_Parse_String(file_name, "/", -1, file_tmp);
				HC_Parse_String(file_tmp, "\\", -1, file_name);
			}

			HFileOutputResult result = m_pHBaseModel->Write(file_name, m_pHView, width, height);

			if(result == OutputBadFileName)
				QMessageBox::information( this, "Simple\n",
				"A problem occured with the file name provided.\n"
				"perhaps you don't have read permision or it is missing.\n");
			else if(result == OutputNotHandled)
				QMessageBox::information( this, "Simple\n",
				"This program doesn't have a handler for this file operation.\n");
			else if(result != OutputOK)
				QMessageBox::information( this, "Simple\n",
				"A problem occured during this file operation.\n");
		}
	}

	delete qfd;
}


// open a load file dialog
void SimpleHQWidget::OnLoad()
{

/*
	QString fn = QFileDialog::getOpenFileName(0,"HOOPS Metafiles (*.hmf);;HOOPS Binary Stream Files (*.hsf)",this);
	if (!fn.isEmpty())
		load(fn);
*/
	QFileDialog* qfd = new QFileDialog(this, "qfd");

    // setup selection filter
    QStringList demFilters;
    demFilters.append("HOOPS Binary Stream Files (*.hsf)");
    demFilters.append("HOOPS Metafiles (*.hmf)");
    demFilters.append("VRML (*.wrl)");
    demFilters.append("Gif (*.gif)");
    demFilters.append("Stereolithography files (*.stl)");
    demFilters.append("Alias Wavefront (*.obj)");


    qfd->setFilters(demFilters);

    // first get new filename
    qfd->setFileMode(QFileDialog::ExistingFile);

    if (qfd->exec() == QDialog::Accepted){

		QString fname = (qfd->selectedFiles()).front();
		if (!fname.isEmpty())
			this->load(fname.toLatin1());
	}

	delete qfd;
}
  

void SimpleHQWidget::OnOrbit()  
{ 
	// Set MVO View Object current Operator to HOpCameraOrbit  
    /*
	if (m_pHView->GetCurrentOperator()) 
	delete m_pHView->GetCurrentOperator(); 

	m_pHView->SetCurrentOperator(new HOpCameraOrbit(m_pHView));  
    */
} 
  
/* Function: OnZoom
   
   This function changes the current view operator
   to perform zoom operations.

   Called with: void
   Returns: void
   Side effects: destroys old view operator and 
                 replaces it with a new zoom operator
 */
void SimpleHQWidget::OnZoom() 
{
	if (m_pHView->GetCurrentOperator())
	    delete m_pHView->GetCurrentOperator();

	PMZoom * temp_zoom = new PMZoom(m_pHView);

	// do some pre-operator setting operations
    temp_zoom->setWidget(this);

    m_pHView->SetCurrentOperator(temp_zoom);
}


/* Function: OnSelectedRegion
   
   This function changes the current view operator
   to perform region selection operations

   Called with: void
   Returns: void
   Side effects: destroys old view operator and 
                 replaces it with a new region
                 selection operator
 */
void SimpleHQWidget::OnSelectRegion() 
{
  if (m_pHView->GetCurrentOperator())
    delete m_pHView->GetCurrentOperator();
  
  PMSelectArea * temp_selArea = new PMSelectArea(m_pHView);
  
  // do some pre-operator setting operations
  temp_selArea->setWidget(this);
  
  m_pHView->SetCurrentOperator(temp_selArea);
}

/* Function: OnSelectedRegion
   
   This function changes the current view operator
   to perform region selection operations

   Called with: void
   Returns: void
   Side effects: destroys old view operator and 
                 replaces it with a new region
                 selection operator
 */
void SimpleHQWidget::OnClearSelectedRegions() 
{
  //! clear selected regions
  ClearSelected();

  //! perform an image refresh
  OnViewChange();

}

// !!! A Dead Function - DEADWOOD
// window zoomer
void SimpleHQWidget::OnZoomToWindow() 
{
	if (m_pHView->GetCurrentOperator())
	    delete m_pHView->GetCurrentOperator();

    m_pHView->SetCurrentOperator(new HOpCameraZoomBox(m_pHView));
}

// resets the camera to view the world space extents of the model
void SimpleHQWidget::OnZoomToExtents() 
{
    m_pHView->ZoomToExtents();
}

//!!!! END DEADWOOD
  
/* Function: OnPan
   
   This function changes the current view operator
   to perform region selection operations

   Called with: void
   Returns: void
   Side effects: destroys old view operator and 
                 replaces it with a new Pan
                 operator
 */
void SimpleHQWidget::OnPan() 
{

    if (m_pHView->GetCurrentOperator())
	    delete m_pHView->GetCurrentOperator();

	PMPan * temp_pan = new PMPan(m_pHView);

	// do some pre-operator setting operations
	temp_pan->setWidget(this);

    m_pHView->SetCurrentOperator(temp_pan);		

}

//!!!! DEADWOOD
// create sphere
void SimpleHQWidget::OnCreateSphere() 
{

    if (m_pHView->GetCurrentOperator())
	    delete m_pHView->GetCurrentOperator();

    m_pHView->SetCurrentOperator(new HOpCreateSphere(m_pHView));		


}

// create cone
void SimpleHQWidget::OnCreateCone() 
{
    if (m_pHView->GetCurrentOperator())
		delete m_pHView->GetCurrentOperator();

    m_pHView->SetCurrentOperator(new HOpCreateCone(m_pHView));		


}



// create cylinder
void SimpleHQWidget::OnCreateCylinder() 
{

    if (m_pHView->GetCurrentOperator())
	    delete m_pHView->GetCurrentOperator();


    m_pHView->SetCurrentOperator(new HOpCreateCylinder(m_pHView));		

}


void SimpleHQWidget::OnApertureSelect()
{
	if (m_pHView->GetCurrentOperator())
		delete m_pHView->GetCurrentOperator();

	m_pHView->SetCurrentOperator(new HOpSelectAperture(m_pHView));
}
//!!!! END DEADWOOD


/*
  Function: OnDDRefresh
  
  This function refreshes the current display
  by performing a full DD traversal, but does
  so without performing new size or scaling 
  calculations.  This is mostly used to 
  get quick timing information.
  
  Called with: void
  Returns: void
  Side effects: refreshes the image painted to
                the screen.
 */
void SimpleHQWidget::OnDDRefresh() 
{
    HC_Open_Segment_By_Key(m_pHView->GetSceneKey());
    
    HPoint target, position, diff; 
    struct timeval stime, etime;
  
    gettimeofday(&stime,NULL);  
    
    // refresh the DD view display
    OnViewChange(); 
    
    // close all Hoops segments
    HC_Close_Segment();

    gettimeofday(&etime,NULL);  

    g_print("Time for screen refresh is %f seconds, resolution:%d\n", 
            (etime.tv_sec + (etime.tv_usec/1000000.0)) - 
            (stime.tv_sec + (stime.tv_usec/1000000.0)),
            ddTools->g_maxDepth);

    //! DEBUG
    ddTools->g_maxDepth--;
}


/*
  Function: PlotDINRDY
  
  This function does the first paint and
  traversal of a DINxRDY DD.  This function
  is a wrapper for OnViewChange, but 
  also performs some default value
  setup.
  
  Called with: void
  Returns: void
  Side effects: refreshes the image painted to
                the screen.
 */
void SimpleHQWidget::PlotDINRDY()
{
    char * temp_str = new char[255]; // yummy hard-coded limit.  Good for buffer-overrun attacks.
    DdManager * local_manager = NULL;
    DdNode * local_node = NULL;
    HPoint line_center;
    HPoint line_pos, line_up;
    QString s;
    QStringList strlist;
    guint64 num_DINs = 0;     
    ddTools->lowColNumber = ddTools->lowSetNumber = (guint64)(99999999999999);
    ddTools->highColNumber = ddTools->highSetNumber = 0;
    struct timeval stime, etime;
     
    //! grab the time for time measurement
    gettimeofday(&stime,NULL);  

    
    /**** Draw stuff to the screen *****/
    
    if(NULL != ddTools->dd_dinrdy)
        {

            //! grab the time for tracking
            gettimeofday(&stime,NULL);              
            local_node = ddTools->dd_dinrdy;
	    local_manager = ddTools->dd_manager;

            // //! try to find and get the DINvsRDY DD
            // for (unsigned int i = 0; (i < ddTools->dd_managers.size()) && 
            //          (local_node == NULL); i++)
            //     {
            //         manager_group t_manager;
            //         t_manager = ddTools->dd_managers[i]; 
            //         if(t_manager.type == DINVSRDY)
            //             {
            //                 local_node = t_manager.node;
            //                 local_manager = t_manager.manager;
            //             }
            //     }

            //! construct the array of shorts that tell the dd traversal function what
            //! nodes it needs to care about
            ddTools->testers = new short int [DDNUM];
            num_testers = DDNUM;
            ddTools->variableAssign = new short int [DDNUM];            

            //! ** PRECLEAN **
            ddTools->g_lowOne = -1;
            
            //! initialize these variables that define where the numbers
            //! exist in the encoded tuple
            //! **flavored with crispy hard-coded flakes**
            ddTools->rdytop = 127;
            ddTools->rdybottom = 64;
            
            //! specify the top and bottom variable indices for the numbers we are collecting
            ddTools->collection_top = 63;
            ddTools->collection_bottom = 0;            
            
            //! make sure we found a DINxRDY manager and DD node
            if((local_node != NULL) && (local_manager != NULL))
                {
		    guint64 t_maxDepth = ddTools->g_maxDepth;
		    int t_topx = ddTools->topX;
		    int t_bottomx = ddTools->bottomX;
		    int t_topy = ddTools->topY;
		    int t_bottomy = ddTools->bottomY;

		    ddTools->g_maxDepth = 0;
		    ddTools->topX = ddTools->bottomX = ddTools->topY = ddTools->bottomY = -1;
                    
#ifdef USE_ZDDS
                    // set the variable permutation assignments
                    for(int i = 0; i < DDNUM; i++)
                        {		  
                            if ((i >= ddTools->rdybottom) && (i <= ddTools->rdytop))
                                { 
                                    ddTools->variableAssign[Cudd_ReadPermZdd(local_manager, i)] 
                                        = RDY;
                                }
                            else
                                {
                                    ddTools->variableAssign[Cudd_ReadPermZdd(local_manager, i)] 
                                        = DIN;
                                }
                        }
		    if((NULL != ddTools->dd_dinhot) && (NULL != ddTools->dd_manager))
		      {
			ddTools->SetHotScale( ddTools->dd_manager, ddTools->dd_dinhot);
		      }
		    
//                     //! perform the DD traversal, but only collect information
//                     //! about the DINxRDY plot so we can do correct size and scale
//                     num_DINs = ddTools->range_traversal(local_manager, local_node, 
// 							&ZDDWork::CollectedNumStats);

		    guint64 tempCol = 0;
		    guint64 tempSet = 0;
		    ddTools->GetTupleTop2(local_manager, local_node,
					  tempCol, tempSet);
		    ddTools->highColNumber = tempCol;
		    ddTools->highSetNumber = tempSet;

		    ddTools->lowSetNumber = 0;
		    ddTools->lowColNumber = 0;
		    num_DINs = 0;
		    
#else
                    // set the variable permutation assignments
                    for(int i = 0; i < DDNUM; i++)
                        {		  
                            if ((i >= ddTools->rdybottom) && (i <= ddTools->rdytop))
                                { 
                                    ddTools->variableAssign[Cudd_ReadPerm(local_manager, i)] 
                                        = RDY;
                                }
                            else
                                {
                                    ddTools->variableAssign[Cudd_ReadPerm(local_manager, i)] 
                                        = DIN;
                                }
                        }

                    //! perform the DD traversal, but only collect information
                    //! about the DINxRDY plot so we can do correct size and scale
                    num_DINs = ddTools->range_traversal(local_manager, local_node, 
							&BDDWork::CollectedNumStats); 
#endif

		    //! restoring the limiting values
		    ddTools->g_maxDepth = t_maxDepth;
		    ddTools->topX = t_topx;
		    ddTools->bottomX = t_bottomx;
		    ddTools->topY = t_topy;
		    ddTools->bottomY = t_bottomy;  
                }
	    
            //! reset the scaling factor
            ddTools->SetScale((quint64)ddTools->highSetNumber, (quint64)ddTools->highColNumber); 

            //! set the plot window
            ddTools->SetWindow();

            //! perform a scale and resolution correct image
            OnViewChange();

            gettimeofday(&etime,NULL);
            
            g_print("Time for search is %f seconds\n", 
                    (etime.tv_sec + (etime.tv_usec/1000000.0)) - 
                    (stime.tv_sec + (stime.tv_usec/1000000.0)));
            
            g_print("Largest DIN value found is: %"G_GUINT64_FORMAT"\n", 
                    ddTools->highColNumber); 
            g_print("Smallest DIN value found is: %"G_GUINT64_FORMAT"\n", 
                    ddTools->lowColNumber); 
            g_print("Largest RDY value found is: %"G_GUINT64_FORMAT"\n", 
                    ddTools->highSetNumber); 
            g_print("Smallest RDY value found is: %"G_GUINT64_FORMAT"\n", 
                    ddTools->lowSetNumber); 

            g_print("Total DINs found is: %"G_GUINT64_FORMAT"\n", num_DINs); 
            
            // CLEANUP
            delete temp_str;
            delete ddTools->testers;
        }
}


/*
  Function: OnAddDD
  
  This function reads in a DD from
  a file and adds the DD to our current
  working set of DDs.
  
  NOTE: Currently all of the DDs are added
  to the same DD manager.  It may be good to
  keep each DD seperate and only transfer the DDs
  to the same manager when needed.

  Called with: void
  Returns: void
  Side effects: modifys the DD manager and node
                lists
*/
void SimpleHQWidget::OnAddDD() 
{
    FILE * newfile;
    char * temp_str = new char[255]; // yummy hard-coded limit.  Good for buffer-overrun attacks.
    DdManager * local_manager = NULL;
    DdNode * local_node = NULL;
    char * fileinfo = new char [512]; // not good programming practice
    char * headerInfo = NULL;
    HPoint line_center;
    HPoint line_pos, line_up;
    quint8 added_dinrdy = false;

    QString s;
    QStringList strlist;

    ddTools->lowColNumber = (guint64)99999999999999;
    ddTools->highColNumber = 0;
    struct timeval stime, etime;

    guint64 r = 1;
    bool endp = false;
    

    //! if X start has been set, use 1
    if(x_start != 0)
        {
            r = x_start;
        }

    //! check for a command line given DD file
    if(NULL == DDstartupFile)
        {        
            /* This section grabs a DD file */
            strlist = QFileDialog::getOpenFileNames(this, "Select a DD", fileOpenLoc->str, "DDs (*.dd)");   
        }	
    else
        {
	  QString tempString = QString(DDstartupFile);
	  strlist = tempString.split(QRegExp("(\\s+|:|,)", Qt::CaseSensitive), QString::SkipEmptyParts);
        }

    //! if the list is empty, don't do any graphing
    endp = strlist.isEmpty();
    
    //! grab the time for tracking
    gettimeofday(&stime,NULL);

    //! initialize a new manager for this DD
    if(ddTools->dd_manager == NULL)
      {
	//	ddTools->dd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS * 200, CUDD_CACHE_SLOTS, ddTools->freemem());
	ddTools->dd_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS * 200, CUDD_CACHE_SLOTS, 0);
      }
    local_manager = ddTools->dd_manager;

    gchar * ddDinDinDeadCode = NULL;

    //! open each DD into a seperate manager
    while (strlist.size() > 0)
        {
            s = strlist.takeFirst();
         
	    if(s.contains(".slr", Qt::CaseInsensitive) == true)
	      {
		g_print("Loading Regions File:%s\n", (const char *)s.toAscii()); 
		imageWork->addSelectedRegions(s.toAscii().constData()); 
	      }
	    else if(s.contains(".dd.working", Qt::CaseInsensitive) == true)
	      {
		g_print("Found DINxDIN Dead Code File:%s\n", (const char *)s.toAscii()); 
		ddDinDinDeadCode = g_strdup((const char *)s.toAscii());          
	      }
	    else if(s.contains(".dd", Qt::CaseInsensitive) == true)
	      { 

		g_print("Loading DD File:%s\n", (const char *)s.toAscii()); 
		
		strcpy(fileinfo, (const char *)s.toAscii());           

		/* Now we import this DD into our CUDD manager */
		newfile = fopen((const char *)s.toAscii(), "r+");

#ifdef USE_ZDDS
		//! grab the new DD from the dddump file
		local_node = Dddmp_cuddZddLoad(local_manager, 
					       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL, 
					       DDDMP_MODE_BINARY, &headerInfo, 
					       fileinfo, newfile);
#else
		//! grab the new DD from the dddump file
		local_node = Dddmp_cuddBddLoad(local_manager, 
					       DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL, 
					       DDDMP_MODE_BINARY, &headerInfo, 
					       fileinfo, newfile);
#endif
	    
		//! add the manager  and the new DD node
		//! to the global vector of DD managers
		if((local_node != NULL) && (local_manager != NULL))
		  {

                    QString t_headerinfo(headerInfo);
                    manager_group l_manager;
                    l_manager.type = NONE;

                    l_manager.manager = local_manager;
                    l_manager.node = local_node;

                    //! this section of code tries to identify
                    //! the type of DD that was loaded
                    if(t_headerinfo.contains("dinrdyoverlay", Qt::CaseInsensitive) == true)
		      {
			l_manager.type = DINVSRDY;
			
			//! add the new DD manager to the vector
			//! of DD managers
			ddTools->dd_managers.push_back(l_manager);
#ifdef USE_ZDDS
			ddTools->dd_dinrdy_overlay = l_manager.node;
			ddTools->dd_manager = l_manager.manager;
			ddTools->dd_dinrdy_overlay_filename = g_strdup(fileinfo);
#endif
			
		      }
                    else if(t_headerinfo.contains("dinrdy", Qt::CaseInsensitive) == true)
		      {
			l_manager.type = DINVSRDY;
			added_dinrdy = 1;
		
			//! add the new DD manager to the vector
			//! of DD managers
			ddTools->dd_managers.push_back(l_manager);
#ifdef USE_ZDDS
			ddTools->dd_dinrdy = l_manager.node;
			ddTools->dd_manager = l_manager.manager;
			ddTools->dd_dinrdy_filename = g_strdup(fileinfo);
#endif
			
		      }
                    else if(t_headerinfo.contains("dinsin", Qt::CaseInsensitive) == true)
		      {
			l_manager.type = DINVSSIN;   
			
			//! add the new DD manager to the vector
			//! of DD managers
			ddTools->dd_managers.push_back(l_manager);

#ifdef USE_ZDDS
			ddTools->dd_dinsin = l_manager.node;
			ddTools->dd_manager = l_manager.manager;
			ddTools->dd_dinsin_filename = g_strdup(fileinfo);
#endif
			
		      }
		    else if(t_headerinfo.contains("dinhot", Qt::CaseInsensitive) == true)
		      {
			l_manager.type = DINVSHOT;
			
			//! add the new DD manager to the vector
			//! of DD managers
			ddTools->dd_managers.push_back(l_manager);
#ifdef USE_ZDDS
			ddTools->dd_dinhot = l_manager.node;
			ddTools->dd_manager = l_manager.manager;
			ddTools->dd_dinhot_filename = g_strdup(fileinfo);
#endif

		      }
		    else if(t_headerinfo.contains("dindin", Qt::CaseInsensitive) == true)
		      {
			l_manager.type = DINVSDIN;

			//! add the new DD manager to the vector
			//! of DD managers
			ddTools->dd_managers.push_back(l_manager);
#ifdef USE_ZDDS
			ddTools->dd_dindin = l_manager.node;
			ddTools->dd_manager = l_manager.manager;
			ddTools->dd_dindin_filename = g_strdup(fileinfo);
#endif


		      }           
 		    else if(t_headerinfo.contains("dinsys", Qt::CaseInsensitive) == true)
		      {
			l_manager.type = DINVSSYS;

			//! add the new DD manager to the vector
			//! of DD managers
			ddTools->dd_managers.push_back(l_manager);
#ifdef USE_ZDDS
			ddTools->dd_dinsys = l_manager.node;
			ddTools->dd_manager = l_manager.manager;
			ddTools->dd_dinsys_filename = g_strdup(fileinfo);
#endif		      
		      }  
			
		    //! if we could not determine the type of DD
                    //! alert the user...perhaps ask for a type
		    else
		      {
			QMessageBox::information( this, "ParMeter\n",
						  "This DD does not seem to match a DD type");
		      }
		  }
            
		// make sure we cleanup
		if(NULL != newfile)
		  {
		    fclose(newfile);
		  }
	      }
	    else
	      {
		g_print("Loading BFD File:%s\n", (const char *)s.toAscii()); 

		//! initialize BFD with the new string name
		bfdTools->InitBFD(s.toStdString()); 
	      }
        }

    if((NULL != ddTools->dd_dinrdy) &&
       (NULL != ddDinDinDeadCode))
      {
	/* If we have the DINxDIN dead code file, use it to reduce the DINxRDY */
	FILE * deadfile = fopen(ddDinDinDeadCode, "r+");
	
	DdNode * deadDinDin = Dddmp_cuddZddLoad(local_manager, 
						DDDMP_VAR_USEPERMIDS, NULL, NULL, NULL, 
						DDDMP_MODE_BINARY, &headerInfo, 
						ddDinDinDeadCode, deadfile);
	fclose(deadfile);
	g_free(ddDinDinDeadCode);
	
	DdNode * blankDinDD =  ddTools->abstractX(local_manager, deadDinDin);
	DdNode * dinBlankXDD = ddTools->VarSwap(local_manager, blankDinDD);
	Cudd_RecursiveDerefZdd(local_manager, blankDinDD);
	DdNode * dinBlankYDD = ddTools->abstractY(local_manager, deadDinDin);
	DdNode * dinBlankDD = Cudd_zddUnion(local_manager, dinBlankYDD, dinBlankXDD);
	Cudd_Ref(dinBlankDD);
	Cudd_RecursiveDerefZdd(local_manager, dinBlankYDD);
	Cudd_RecursiveDerefZdd(local_manager, dinBlankXDD);
	DdNode * dinUnvDD = ddTools->yDC(local_manager, dinBlankDD);
	Cudd_RecursiveDerefZdd(local_manager, dinBlankDD);

	DdNode * newDinRdyDD = Cudd_zddIntersect(local_manager, dinUnvDD, ddTools->dd_dinrdy);
	Cudd_Ref(newDinRdyDD);
	Cudd_RecursiveDerefZdd(local_manager, ddTools->dd_dinrdy);
	Cudd_RecursiveDerefZdd(local_manager, dinUnvDD);
	ddTools->dd_dinrdy = newDinRdyDD;
      }
    
    // set the flag true to let the rest of the system
    // know that a DD (or DDs) has been loaded
    if(ddTools->dd_managers.empty() == false)
        {
            ddTools->loadedDD = true;
        }

   
    gettimeofday(&etime,NULL);
    
    g_print("Done Loading DD file(s):%f\n",
            (etime.tv_sec + (etime.tv_usec/1000000.0)) - 
            (stime.tv_sec + (stime.tv_usec/1000000.0)));

    g_print("DD Manager list size:%d\n", (int)ddTools->dd_managers.size());

    //! if this is just a research run
    //! then we may not want to even plot
    if(invst)
      {
	//! initialize these variables that define where the numbers
	//! exist in the encoded tuple
	//! **flavored with crispy hard-coded flakes**
	ddTools->rdytop = 127;
	ddTools->rdybottom = 64;
	
	//! specify the top and bottom variable indices for the numbers we are collecting
	ddTools->collection_top = 63;
	ddTools->collection_bottom = 0;   
	InitPicture1();
      }
    else
      {
	//! if we have added a new din vs rdy bdd
	//! refresh the screen
	if(added_dinrdy == 1){
	  
	  //! go ahead and plot the DINxRDY plot
	  PlotDINRDY();
	  
	  // update display
	  m_pHView->SetGeometryChanged();
	  m_pHView->Update(); 
	}
      }
    
    // CLEANUP
    delete fileinfo;
    delete temp_str;

    //! for some scripting applications
    //! we just want to generate the DINxRDY
    //! plot and then die
    if(dieondone)
      {
	InitPicture1();
	my_parent->close();
      }   
}


//! increases the resolution after a period of no view updates
void SimpleHQWidget::ViewTimer()
{

    if(view_timer_intervals <= 0)
        {
            view_timer->stop();
            view_timer_intervals = VIEW_INTVS;
            //            ddTools->g_maxDepth += VIEW_INTVS;
        }
    else
        {
            view_timer_intervals--;

            ddTools->g_maxDepth--;

            OnViewChange();                
        }
}


/*

  A wrapper for OnViewChange that uses the timer

 */
int SimpleHQWidget::OnViewChangeWTimer(void)
{
    //! restart the view timer
    //    view_timer->start(view_timer_time);

    //! return the value from OnViewChange
    return(OnViewChange());
}


/*!
 *
 * This function gets called by the Zoom operator and will rebuild the DD graph
 * depending on zoom level and view position
 * The DD(s) must be loaded before the function was called.
 *
 * \author  Graham Price
 * \date 04/18/07
 *
 */
int SimpleHQWidget::OnViewChange(void)
{
    bool endp = true;
    guint64 num_DINs = 0;
    
    struct timeval startViewChange, stopViewChange;

    //! make sure we are the only one
    //! rendering
    if (sem_trywait(&now_rendering) != 0){
        return(0);
    }
    
    gettimeofday(&startViewChange,NULL);
		  
    ddTools->UpdateView();
    HC_Update_Display();
    
    gettimeofday(&stopViewChange,NULL);
    
    ddTools->viewRefreshTime = (stopViewChange.tv_sec + (stopViewChange.tv_usec/1000000.0)) - 
      (startViewChange.tv_sec + (startViewChange.tv_usec/1000000.0));
    
    //! hopefully prevent some bad parallel cases
    sem_post(&now_rendering);
    
    return 0;
}


DdNode * SimpleHQWidget::build_tuple(DdManager *manager, guint64 x, guint64 y)
{
    unsigned int i;
    DdNode * tmp, * itenode, * logicZero;
    
    logicZero = Cudd_ReadLogicZero(manager);
    Cudd_Ref(logicZero);

    itenode = Cudd_ReadOne(manager);
    Cudd_Ref(itenode);

    /* iterate through the 64 bit values for both X and Y */
    for (i = 0; i < (sizeof(guint64) * 8 * 2); i++) 
        {
            guint64 bitset;
            int v;

            // This code only works for a total of 128 DD vars, beware
            assert(Cudd_ReadSize(manager) <= 128);
            v= Cudd_ReadInvPerm(manager,127-i);
            tmp = itenode;	  

            bitset = (v < 64)?(x & ddTools->mask((guint64)(v))):(y & ddTools->mask((guint64)(v-64)));
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
  This function should return some pertinant information
  about the DINs that are currently selected in the 
  Parameter graph
*/
void SimpleHQWidget::PrintSinInfo(DdNode * dinRdySelDD)
{

    map<guint64, guint64>::iterator t_iter;
    map<guint64,guint64> * pcMap = NULL;
    dd_package t_selection;
    ddTools->DDPackageInit(&t_selection);
    
    t_selection.manager = ddTools->dd_manager;
    t_selection.node = dinRdySelDD;
      
    if (t_selection.node != NULL)
        {
	  dd_package t_pconly = ddTools->GetPConlyDD(t_selection);
	  //            g_print("rdy:%s,din:%s\n", temp_str2, temp_str1);       
	  //! grab the PC DD for this set of selected DINs
	  t_pconly = ddTools->GetPConlyDD(t_selection);

            if(t_pconly.node != NULL)
                {

                    //! clean up the list of values from the search
                    pcMap = ddTools->CleanPC(&(ddTools->g_CollectedNums));

                    //! if we have loaded the program file, return the
                    //! further information regarding the sin
                    if(bfdTools->g_BFD != NULL)
                        {                            
                            //! now iterate through the map
                            for(t_iter = (*pcMap).begin(); t_iter != (*pcMap).end(); t_iter++)
                                {
                                    //                                    g_print( "addr:0x%lx, occurs:%"G_GUINT64_FORMAT"\n", 
                                    //                                             t_iter->first, t_iter->second);         

                                    //! output any info for this sin
                                    GString * temp_string = bfdTools->BfdSinInfo(t_iter->first);

                                    //! if temp_string is not NULL, we found some info
                                    if(temp_string != NULL)
                                        {
                                            
                                            g_print( "addr:0x%lx, occurs:%"G_GUINT64_FORMAT"\n", 
                                                     t_iter->first, t_iter->second);     

                                            //! print out the info
                                            g_print(temp_string->str);

                                            //! free the string variable
                                            g_string_free(temp_string, TRUE);
                                        }

                                } 

                            //! CLEANUP
                            if(pcMap != NULL)
                                {
                                    delete pcMap;
                                }
                        } 
                    else
                        {                          
                            //! now iterate through the map
                            for(t_iter = (*pcMap).begin(); t_iter != (*pcMap).end(); t_iter++)
                                {
                                    g_print( "addr:0x%lx, occurs:%"G_GUINT64_FORMAT"\n", 
                                             t_iter->first, t_iter->second);        
                                } 

                            //! CLEANUP
                            if(pcMap != NULL)
                                {
                                    delete pcMap;
                                }
                        }
                }
        
            else
                {
                    g_print("No PC values located for the selection\n");
                    
                }
        }
    else
        {
            g_print("No items found in selection\n");
        }
}

/*
  This function should return some pertinant information
  about the DINs that are currently selected in the 
  Parameter graph
*/
void SimpleHQWidget::OnSelectedDINs()
{

    dd_package t_selection;
    dd_package t_pconly; 

    ddTools->DDPackageInit(&t_selection);

    map<guint64, guint64>::iterator t_iter;
    map<guint64,guint64> * pcMap = NULL;
    struct timeval stime, etime;
  
    gettimeofday(&stime,NULL);  
    
    t_selection = GetSelectedDD();
    
    if (t_selection.node != NULL)
        {

            //            g_print("rdy:%s,din:%s\n", temp_str2, temp_str1);       
            //! grab the PC DD for this set of selected DINs
            t_pconly = ddTools->GetPConlyDD(t_selection);

            if(t_pconly.node != NULL)
                {

                    //! clean up the list of values from the search
                    pcMap = ddTools->CleanPC(&(ddTools->g_CollectedNums));

                    //! if we have loaded the program file, return the
                    //! further information regarding the sin
                    if(bfdTools->g_BFD != NULL)
                        {

                            
                            //! now iterate through the map
                            for(t_iter = (*pcMap).begin(); t_iter != (*pcMap).end(); t_iter++)
                                {
                                    //                                    g_print( "addr:0x%lx, occurs:%"G_GUINT64_FORMAT"\n", 
                                    //                                             t_iter->first, t_iter->second);         

                                    //! output any info for this sin
                                    GString * temp_string = bfdTools->BfdSinInfo(t_iter->first);

                                    //! if temp_string is not NULL, we found some info
                                    if(temp_string != NULL)
                                        {
                                            
                                            g_print( "addr:0x%lx, occurs:%"G_GUINT64_FORMAT"\n", 
                                                     t_iter->first, t_iter->second);     

                                            //! print out the info
                                            g_print(temp_string->str);

                                            //! free the string variable
                                            g_string_free(temp_string, TRUE);
                                        }

                                } 

                            //! CLEANUP
                            if(pcMap != NULL)
                                {
                                    delete pcMap;
                                }
                        } 
                    else
                        {                          
                            //! now iterate through the map
                            for(t_iter = (*pcMap).begin(); t_iter != (*pcMap).end(); t_iter++)
                                {
                                    g_print( "addr:0x%lx, occurs:%"G_GUINT64_FORMAT"\n", 
                                             t_iter->first, t_iter->second);        
                                } 

                            //! CLEANUP
                            if(pcMap != NULL)
                                {
                                    delete pcMap;
                                }
                        }
                }
        
            else
                {
                    g_print("No PC values located for the selection\n");
                    
                }
        }
    else
        {
            g_print("No items found in selection\n");
        }

    gettimeofday(&etime,NULL);  

    g_print("Time for SIN lookup is %f seconds", 
            (etime.tv_sec + (etime.tv_usec/1000000.0)) - 
            (stime.tv_sec + (stime.tv_usec/1000000.0)));

}


void SimpleHQWidget::OnDeadRdyFilter()
{

  if(ddTools->dd_dindin == NULL)
    {
      printf("Please load a DINxDIN DD file\n");
      return;
    }
  if(ddTools->dd_dindin == NULL)
    {
      printf("Please load a DINxRDY DD file\n");
      return;
    }

  if (false == ddTools->toggleDeadReadyFilter())
    {
      if(NULL != ddTools->dd_dinrdy_original)
	{
	  ddTools->dd_dinrdy_filtered = ddTools->dd_dinrdy;
	  ddTools->dd_dinrdy = ddTools->dd_dinrdy_original;
	}
    }
  else
    {
      ddTools->dd_dinrdy_original = ddTools->dd_dinrdy;

      dd_package t_selected;
    
      //! initialize the bdd packages
      ddTools->DDPackageInit(&t_selected);

      //!!! DEBUG !!!
      t_selected = GetSelectedDD();
    
      //! if we have nothing selected, perhaps pull the selected DD from
      //! a previous iteration of dependence slicing
      if(t_selected.node == NULL)
	{
	  if(ddTools->dd_dinrdy_filtered != NULL)
	    {
	      t_selected.node = ddTools->dd_dinrdy_filtered;
	      t_selected.manager = ddTools->dd_manager;
	    }
	}
    
      if(t_selected.node != NULL)
        {
	  DdNode * selDinUniv = ddTools->yDC(ddTools->dd_manager, t_selected.node);

	  DdNode * selDinRdy = Cudd_zddIntersect(ddTools->dd_manager,
						 ddTools->dd_dinrdy,
						 selDinUniv);
	  Cudd_Ref(selDinRdy);
	  Cudd_RecursiveDerefZdd(ddTools->dd_manager, selDinUniv);

	  printf("Selection Size:%u\n", Cudd_zddCount(ddTools->dd_manager, selDinRdy));


	  //	  DdNode * dinRdySelFilterDD = ddTools->DeadReadyFilterSelection(selDinRdy);

	  // DEBUG
	  DdNode * dinRdySelFilterDD = ddTools->HotDeadFilterSelection(selDinRdy);
	  printf("Selection Filter Size:%u\n", Cudd_zddCount(ddTools->dd_manager, dinRdySelFilterDD));

	  DdNode * remDinRdyDD = Cudd_zddDiff(ddTools->dd_manager, 
						      selDinRdy, 
						      dinRdySelFilterDD);
	  Cudd_Ref(remDinRdyDD);

	  printf("Diff of Selection and Filter Size:%u\n", Cudd_zddCount(ddTools->dd_manager, remDinRdyDD));

	  Cudd_RecursiveDerefZdd(ddTools->dd_manager, dinRdySelFilterDD);

	  // If we have a DINxSIN set, and a BFD, lookup the dead nodes
	  if((NULL != bfdTools->g_BFD) && (NULL != ddTools->dd_dinsin))
	    {
	      printf("Source code for dead DINxRDY points:\n");

	      DdNode * remDinBlankDD = ddTools->abstractY(ddTools->dd_manager, remDinRdyDD);
	      Cudd_Ref(remDinBlankDD);
	      
	      PrintSinInfo(remDinBlankDD);	     
	      Cudd_RecursiveDerefZdd(ddTools->dd_manager, remDinBlankDD);
	    }

	  DdNode * invRemDinRdyDD = Cudd_zddDiff(ddTools->dd_manager,
						 Cudd_ReadZddOne(ddTools->dd_manager, 0),
						 remDinRdyDD);
	  Cudd_Ref(invRemDinRdyDD);
	  Cudd_RecursiveDerefZdd(ddTools->dd_manager, remDinRdyDD);

	  printf("Inverted diff Size:%u\n", Cudd_zddCount(ddTools->dd_manager,invRemDinRdyDD));


	  DdNode * newDinRdyDD = Cudd_zddIntersect(ddTools->dd_manager,
						   ddTools->dd_dinrdy,
						   invRemDinRdyDD);

	  Cudd_Ref(newDinRdyDD);
	  Cudd_RecursiveDerefZdd(ddTools->dd_manager, invRemDinRdyDD);	  
	  
	  //! DEBUG
	  printf("New DINxRDY Size:%u\n", Cudd_zddCount(ddTools->dd_manager, newDinRdyDD));
	  printf("Old DINxRDY Size:%u\n",  Cudd_zddCount(ddTools->dd_manager, ddTools->dd_dinrdy));

	  ddTools->dd_dinrdy = newDinRdyDD;

	  if(ddTools->dd_dinrdy_filtered != NULL)
	    {
	      Cudd_RecursiveDerefZdd(ddTools->dd_manager, ddTools->dd_dinrdy_filtered);
	    }

	  ddTools->dd_dinrdy_filtered = newDinRdyDD;
	  
	  //! update the view
	  OnViewChange();
          
	  //! that seemed to work, clear the selected region
	  ClearSelected();
        }

      // update display
      m_pHView->SetGeometryChanged();
      m_pHView->Update(); 
    }

  //! update the view
  OnViewChange();
  
  // update display
  m_pHView->SetGeometryChanged();
  m_pHView->Update(); 
}


void SimpleHQWidget::OnUniqStaticFilter()
{
  if (false == ddTools->toggleUniqueStaticFilter())
    {
      ddTools->toggleUniqueStaticFilter();
    }
}


/*
  This function should find the reverse dependence slice 
  of all selected DINxRDY points
*/
void SimpleHQWidget::OnFindDep()
{
    dd_package t_selected;
    dd_package t_depPack;
    
    //! initialize the bdd packages
    ddTools->DDPackageInit(&t_selected);
    ddTools->DDPackageInit(&t_depPack);

    //!!! DEBUG !!!
    t_selected = GetSelectedDD();

    /*    
    guint64 t_highX = ddTools->highColNumber;
    guint64 t_highY = ddTools->highSetNumber;

    t_selected.node = ddTools->build_tuple(t_selected.manager, 
					   Cudd_ReadZero(t_selected.manager), t_highX, 0);
    */
    
    //! if we have nothing selected, perhaps pull the selected DD from
    //! a previous iteration of dependence slicing
    if(t_selected.node == NULL)
      {
	if(ddTools->sliceDD.node != NULL)
	  {
	    t_selected = ddTools->sliceDD;
	  }
      }
    
    if(t_selected.node != NULL)
        {

// 	  DdNode * dinRdyFilterDD = ddTools->DeadReadyFilterSelection(t_selected.node);
// 	  t_depPack.manager = t_selected.manager;
// 	  t_depPack.node = dinRdyFilterDD;

	  t_depPack = ddTools->GetDepDINDD(t_selected);
        }
    if(t_depPack.node != NULL)
        {

            //! clear out the display of any previous slice
            ddTools->ClearDependents();   
            
            //! setup the final DINxRDY analysis DD
            ddTools->GraphFinalSlice(t_depPack);
            
            //! update the view
            OnViewChange();
            
            //! that seemed to work, clear the selected region
            ClearSelected();
        }


    // update display
    m_pHView->SetGeometryChanged();
    m_pHView->Update(); 
 
}


//! \brief Gets the selction for the BFD
//! \author Graham Price
//! \date 08/29/2007
void SimpleHQWidget::GetBFDFile()
{
    QString filename;
        QStringList strlist;

        /*! This section grabs an object file */
        strlist = QFileDialog::getOpenFileNames(this, "Select an valid file", fileOpenLoc->str, "(*)");


        //! while limited, we should just take the first file given
        //! for now
        filename = strlist.takeFirst();

    
        //! initialize BFD with the new string name
        bfdTools->InitBFD(filename.toStdString());    
    
}


/*!
  This function creates a DD that contains the DINs selected region
  \author Graham Price
  \date 11/14/07
*/
dd_package SimpleHQWidget::GetSelectedDD()
{
    dd_package return_package;

    guint64 largest_din = 0;
    guint64 smallest_din = 99999999999;
    guint64 largest_rdy = 0;
    guint64 smallest_rdy = 99999999999;

    DdNode *dinonlyDD = NULL, *dinrdyDD = NULL;
    DdNode *selDD = NULL, *zero = NULL;
    
    DdManager * dinrdyMan = NULL;

    //! try to find all the needed DDs
    for (unsigned int i = 0; (i < ddTools->dd_managers.size()); i++)
        {
            manager_group t_manager;
            t_manager = ddTools->dd_managers[i]; 
            if(t_manager.type == DINVSRDY)
                {
                    dinrdyDD = t_manager.node;
                    dinrdyMan = t_manager.manager;
               }
        }

    QPointF top_left, bottom_right;
    
    for ( int i = 0; i < imageWork->getSelectionCount(); ++i)
      {
	imageWork->getSelection(i, &top_left, &bottom_right);

	if(top_left.x() > bottom_right.x())
	  {
	    largest_rdy = (guint64)top_left.x();
	    smallest_rdy = (guint64)bottom_right.x();
	  }
	else
	  {
	    largest_rdy = (guint64)bottom_right.x();
	    smallest_rdy =(guint64)top_left.x();
	  } 

	if(top_left.y() > bottom_right.y())
	  {
	    largest_din = (guint64)top_left.y();
	    smallest_din =(guint64)bottom_right.y();
	  }
	else
	  {
	    largest_din =(guint64)bottom_right.y();
	    smallest_din =(guint64)top_left.y();
	  } 


	//!! DEBUG
	//             g_print("Largest DIN:%"G_GUINT64_FORMAT",Smallest DIN:%"G_GUINT64_FORMAT"\n",
	//                     largest_din, smallest_din);
    
	//! Start the Slice Timer!
	gettimeofday(&(ddTools->sliceStartTime),NULL);

	//! this function forms a DD from the selected region
	selDD = ddTools->GraphSelect(dinrdyMan, (uint64_t)smallest_din, (uint64_t)largest_din,
				     (uint64_t)smallest_rdy, (uint64_t)largest_rdy);

	//	    DdNode *tmp[1];
	//	    FILE *f = NULL;
	    
	// // Print out a dot file of this post store
	//  tmp[0] = selDD;
	//  f=fopen("graphselect.dot","w+");
	//  Cudd_zddDumpDot(dinrdyMan, 1, tmp, NULL, NULL, f);
	//  fclose(f);

#ifdef USE_ZDDS

	//! This needs to be an set intersection, not a logic product
	//! because of the zdd returned by GraphSelect
	DdNode * tempSel = Cudd_zddIntersect(dinrdyMan, dinrdyDD, selDD);
	Cudd_Ref(tempSel);

	//! DEBUG
	//	    ddTools->DdSize(dinrdyMan, tempSel);

	// tmp[0] = tempSel;
	// f=fopen("drselection.dot","w+");
	// Cudd_zddDumpDot(dinrdyMan, 1, tmp, NULL, NULL, f);
	// fclose(f);

	//! Abstract the Y variables from a (X,Y) tuple
	dinonlyDD = ddTools->abstractY(dinrdyMan, tempSel);
	Cudd_RecursiveDerefZdd(dinrdyMan, tempSel);
#else
	//! build a cube for the extraction
	DdNode * cubeDD = ddTools->build_d1cube(dinrdyMan, ddTools->rdybottom, ddTools->rdytop);
	Cudd_Ref(cubeDD);

	dinonlyDD = Cudd_bddAndAbstract(dinrdyMan, dinrdyDD, selDD, cubeDD);
	Cudd_Ref(dinonlyDD);

	//! DD CLEANUP
	Cudd_IterDerefBdd(dinrdyMan, cubeDD);
	Cudd_IterDerefBdd(dinrdyMan, selDD);
#endif
  
      }

    //! check to see if we have not found any 
    //! DD for our selection
    if((dinonlyDD != NULL) && (dinonlyDD == zero))
      {
	dinonlyDD = NULL;
      }

    return_package.node = dinonlyDD;
    return_package.manager = dinrdyMan;

    return(return_package);
}


/*
  Function: ClearSelected - clear any manually selected items

  This function should clear out any selection rectangle.

  Called with: void
  returns: void
  side effects: modifies the selectionRect rectangle
*/
void SimpleHQWidget::ClearSelected(void)
{
  //! clear the imagework selection
  imageWork->clearSelection();
}


void SimpleHQWidget::OnSaveRegions(void)
{  
  /*! This section grabs an object file */
  if(NULL != ddTools->dd_dinrdy_filename)
    {
      gchar * imageFileName = g_strdup_printf ("%s%s", 
					       ddTools->dd_dinrdy_filename,
					       ".slr");
      
      imageWork->saveSelectedRegions(imageFileName);
      
      g_print("Saved Regions: %s\n", imageFileName);
      g_free(imageFileName);
    }
}


void SimpleHQWidget::OnAddSelRegions(void)
{  

  QStringList strlist = 
    QFileDialog::getOpenFileNames(this, "Select a DD", fileOpenLoc->str, "Regions (*.slr)"); 

  while (strlist.size() > 0)
    {
      QString s = strlist.takeFirst();
      imageWork->addSelectedRegions(s.toAscii().constData());      
    }
  OnViewChange();
}

void SimpleHQWidget::OnSaveImage(void)
{
  // setup selection filter
  QList<QByteArray> formatList = QImageWriter::supportedImageFormats();
  QStringList tStringFormatList = QStringList();
  QString imageFilterTypes = QString("Images (");
  for (int i = 0; i < formatList.size(); ++i)
    {
      tStringFormatList.append((QString(formatList.at(i))));
      imageFilterTypes += "*.";
      imageFilterTypes += formatList.at(i);
      imageFilterTypes += " ";
    }
  imageFilterTypes = imageFilterTypes.trimmed();
  imageFilterTypes +=")";
  
  /*! This section grabs an object file */
  QString imageFileName = QFileDialog::getSaveFileName(this, "Save Image file", ".", imageFilterTypes);
  
  imageWork->saveViewBuffer(imageFileName);
}

/*
  Function: DDWorkInit - This function initializes DD work

  This function should initialize all DD stuff

  Called with: void
  returns: void
  side effects: destroys and resets a number of variables
*/
void SimpleHQWidget::DDWorkInit(void)
{
  //! initialize the view rendering semaphore
  sem_init(&now_rendering, 0, 1);
  
  //! clear out any possible old DD stuff
  DDClear();
  
  // set the starting view position
  // used for adjusting the DD number extration
  HC_Open_Segment_By_Key(m_pHView->GetSceneKey());
  HC_Show_Net_Camera_Position(&(ddTools->start_x), 
			      &(ddTools->start_y),
			      &(ddTools->start_z));
  HC_Close_Segment();
	
  //! initialize some resolution items
  view_scaler = 5000;
  if(x_res == 0)
    {
      x_res = 10000;
    }
  if(y_res == 0)
    {
      y_res = 10000;
    }
  
  //! Command line auto-run
  if (DDstartupFile != NULL)
    {
      OnAddDD();
    }
}


/*
  Function: DDClear - This function should clear the DD work
  
  This function will clear the display, DD managers, and other
  items to start a new DD display.

  Called with: void
  returns: void
  side effects: destroys and resets a number of variables
*/
void SimpleHQWidget::DDClear(void){

    //! delete the old bdd tools instance
    delete ddTools;
    
#ifdef USE_ZDDS
    //! create a new bdd tools instance
    ddTools = new ZDDWork(m_pHView, imageWork);
#else
    //! create a new bdd tools instance
    ddTools = new BDDWork(m_pHView, imageWork);
#endif

}

//! Slicing test
void SimpleHQWidget::Invst1(void)
{
  
  DdNode * dindinDD = NULL;
  DdManager * dindinMan = NULL;
  DdNode * dinrdyDD = NULL;
  DdManager * dinrdyMan = NULL;
    
  //! try to find all the needed DDs
  for (unsigned int i = 0; (i < ddTools->dd_managers.size()); i++)
    {
      manager_group t_manager;
      t_manager = ddTools->dd_managers[i]; 
      if(t_manager.type == DINVSDIN)
	{
	  dindinDD = t_manager.node;
	  dindinMan = t_manager.manager;
	}
      else if(t_manager.type == DINVSRDY)
	{
	  dinrdyDD = t_manager.node;
	  dinrdyMan = t_manager.manager;
	}
    }
  
  if((dindinDD != NULL) && (dinrdyDD != NULL))
    {
      // First get the last rdy value
      guint64 topDin = 0, topRdy = 0;
      ddTools->GetTupleTop(dinrdyMan, dinrdyDD, topDin, topRdy);

      // Build a tuple that represents our ready time
      DdNode * rdyNode = ddTools->build_tuple(dinrdyMan, Cudd_ReadZero(dinrdyMan), 0, topRdy);
      
      // Add in the universal set of X values
      DdNode * tmpSliceNode = ddTools->xDC(dinrdyMan, rdyNode);
      Cudd_RecursiveDerefZdd(dinrdyMan, rdyNode);
      
      // Intersect our {univ,rdy} with {din,rdy}
      DdNode * intersectNode = Cudd_zddIntersect(dinrdyMan, dinrdyDD, tmpSliceNode);
      Cudd_Ref(intersectNode);
      Cudd_RecursiveDerefZdd(dinrdyMan, tmpSliceNode);

      g_print("Initial {DIN,RDY} slice set:\n");
      ddTools->DdSize(dinrdyMan, intersectNode);

      // Now we have the DINs of interest.  Ditch the ready time.
      DdNode * dinonlyDD = ddTools->abstractY(dinrdyMan, intersectNode);
      Cudd_RecursiveDerefZdd(dinrdyMan, intersectNode);

      // Ditch the original {DIN,RDY} set
      Cudd_RecursiveDerefZdd(dinrdyMan, dinrdyDD);
      
      DdNode * longRevSlice = ddTools->BuildIterReverseSlice(dindinMan, dinonlyDD, 
							     dindinDD, 0);
      
      // Save out the results
      struct timeval timecount;
      int * zdd_ordr = g_new(int, 128);
      gettimeofday(&timecount, NULL); 
      gchar * fileName = g_strdup_printf("%s_%d.dd", "invst1", (int)timecount.tv_sec);
      FILE * fh = fopen(fileName, "w+");
      Dddmp_MoreDDHeaderInfo headerInfo;
      Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */

      for(int k = 0; k < 64; k++)
	{
	  zdd_ordr[127-2*k] = k;
	  zdd_ordr[127-(2*k+1)] = (64+k);
	}

      headerInfo.extraTraceInfo = g_new(char, 512);
  
      //! add extra information to this ZDD dump
      g_snprintf(headerInfo.extraTraceInfo, 512, "type:dindin,dinstart:0,dinstop:0");

      Dddmp_cuddZddStore(dindinMan, NULL,
			 longRevSlice, NULL, zdd_ordr,
			 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			 fileName, fh);
      fclose(fh);
      g_free(fileName);
      g_free(headerInfo.extraTraceInfo);
      g_free(zdd_ordr);
    }
}

//! Slicing test 2
void SimpleHQWidget::Invst2(void)
{
  
  DdNode * dindinDD = NULL;
  DdManager * dindinMan = NULL;
    
  //! try to find all the needed DDs
  for (unsigned int i = 0; (i < ddTools->dd_managers.size()); i++)
    {
      manager_group t_manager;
      t_manager = ddTools->dd_managers[i]; 
      if(t_manager.type == DINVSDIN)
	{
	  dindinDD = t_manager.node;
	  dindinMan = t_manager.manager;
	}
    }
  
  if((dindinDD != NULL))
    {
      DdNode *  longRevSlice = ddTools->LongReverseSlice(dindinMan, dindinDD);
      
      // Save out the results
      struct timeval timecount;
      int * zdd_ordr = g_new(int, 128);
      gettimeofday(&timecount, NULL); 
      gchar * fileName = g_strdup_printf("%s_%d.dd", "invst1", (int)timecount.tv_sec);
      FILE * fh = fopen(fileName, "w+");
      Dddmp_MoreDDHeaderInfo headerInfo;
      Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */

      for(int k = 0; k < 64; k++)
	{
	  zdd_ordr[127-2*k] = k;
	  zdd_ordr[127-(2*k+1)] = (64+k);
	}

      headerInfo.extraTraceInfo = g_new(char, 512);
  
      //! add extra information to this ZDD dump
      g_snprintf(headerInfo.extraTraceInfo, 512, "type:dindin,dinstart:0,dinstop:0");

      Dddmp_cuddZddStore(dindinMan, NULL,
			 longRevSlice, NULL, zdd_ordr,
			 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			 fileName, fh);
      fclose(fh);
      g_free(fileName);
      g_free(headerInfo.extraTraceInfo);
      g_free(zdd_ordr);
    }
}

//! Slicing test 3
void SimpleHQWidget::Invst3(void)
{
  
  if((NULL != ddTools->dd_dindin) 
     && (NULL != ddTools->dd_dinhot)
     && (NULL != ddTools->dd_dinrdy))
    {
      // Get the top quarter of hot code
      guint64 topX, topY;
      guint64 topPercent = 50;
      guint64 zddNumHalf = ZDDNUM/2;
      int xlist [ZDDNUM/2];
      int ylist [ZDDNUM/2];

      //! gather a list of ZDD node variables                                                                          
      for (unsigned int i = 0; i < zddNumHalf; i+=1)
	{
	  xlist[(zddNumHalf - 1) - i] = i;
	  ylist[(zddNumHalf - 1) - i] = i + zddNumHalf;
	}

      ddTools->GetTupleTop2(ddTools->dd_manager, ddTools->dd_dinhot, topX, topY);
      
      guint64 topHotSel = (guint64) (topY - (guint64)((double)topY * (double)((double)topPercent) / 100));

      printf("Using the value %d as a lower bound\n", (int)topHotSel);

      DdNode * yLb =  mg_Cudd_zddLb(ddTools->dd_manager, zddNumHalf, ylist, (uint64_t)topHotSel);
      Cudd_Ref(yLb);

      DdNode * hotLbUnvDD = ddTools->xDC(ddTools->dd_manager, yLb);

      DdNode * topHotDD = Cudd_zddIntersect(ddTools->dd_manager, hotLbUnvDD, ddTools->dd_dinhot);
      Cudd_Ref(topHotDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, hotLbUnvDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, ddTools->dd_dinhot);

      // Remove the Hot Values
      DdNode * dinEmptyHotDD = ddTools->abstractY(ddTools->dd_manager, topHotDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, topHotDD);
      DdNode * dinUnvHotDD = ddTools->yDC(ddTools->dd_manager, dinEmptyHotDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, dinEmptyHotDD);
      DdNode * dinRdyHotDD = Cudd_zddIntersect(ddTools->dd_manager, dinUnvHotDD, ddTools->dd_dinrdy);
      Cudd_Ref(dinRdyHotDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, dinUnvHotDD);

      // Remove the dead code from hot DINxRDY DD
      printf("Old DINxRDY Size:%u\n", Cudd_zddCount(ddTools->dd_manager,dinRdyHotDD));
      DdNode * dinHotAliveDD = ddTools->DeadReadyFilterSelection(dinRdyHotDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, dinRdyHotDD);
      printf("New DINxRDY Size:%u\n", Cudd_zddCount(ddTools->dd_manager, dinHotAliveDD));

      DdNode * finalDD = dinHotAliveDD;
      
      // Save out the results
      struct timeval timecount;
      int * zdd_ordr = g_new(int, 128);
      gettimeofday(&timecount, NULL); 
      gchar * fileName = g_strdup_printf("%s_%d.dd", "invst3", (int)timecount.tv_sec);

      printf("New DINxRDY fileName: %s\n", fileName);

      FILE * fh = fopen(fileName, "w+");
      Dddmp_MoreDDHeaderInfo headerInfo;
      Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */

      for(int k = 0; k < 64; k++)
	{
	  zdd_ordr[127-2*k] = k;
	  zdd_ordr[127-(2*k+1)] = (64+k);
	}

      headerInfo.extraTraceInfo = g_new(char, 512);
  
      //! add extra information to this ZDD dump
      g_snprintf(headerInfo.extraTraceInfo, 512, "type:dinrdy,dinstart:0,dinstop:0");

      Dddmp_cuddZddStore(ddTools->dd_manager, NULL,
			 finalDD, NULL, zdd_ordr,
			 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
			 fileName, fh);
      fclose(fh);
      g_free(fileName);
      g_free(headerInfo.extraTraceInfo);
      g_free(zdd_ordr);
    }
  else if((NULL != ddTools->dd_dindin)
	  && (NULL != ddTools->dd_dinrdy))
    {
      // Get the top RDY time set
      guint64 xTop = 0;
      guint64 yTop = 0;

      ddTools->GetTupleTop(ddTools->dd_manager, ddTools->dd_dinrdy, xTop, yTop);

      // DEBUG
      printf("Top Din:%d, Rdy:%d\n", xTop, yTop);

      DdNode * zero = Cudd_ReadZero(ddTools->dd_manager);
      Cudd_Ref(zero);
  
      // Build a tuple that represents our top ready time 
      DdNode * topRdyDD = ddTools->build_tuple(ddTools->dd_manager, zero, 0, yTop); 
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, zero); 

      // Add in the universal set of X values
      DdNode * topUnivRdyDD = ddTools->xDC(ddTools->dd_manager, topRdyDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, topRdyDD);
  
      // Intersect our {univ,rdy} with {din,rdy}
      DdNode * topDinRdyDD = Cudd_zddIntersect(ddTools->dd_manager, ddTools->dd_dinrdy, topUnivRdyDD);
      Cudd_Ref(topDinRdyDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, topUnivRdyDD);      

      // Create a {DIN,empty} set
      printf("Old {DIN,RDY} Size:%u\n", Cudd_zddCount(ddTools->dd_manager, ddTools->dd_dinrdy));
      DdNode * tmpDinRdyDD = ddTools->QuickDeadFilter(ddTools->dd_dinrdy);

      // Add the top {DIN,RDY} set back in, because they do not really count as dead
      DdNode * newDinRdyDD = Cudd_zddUnion(ddTools->dd_manager, tmpDinRdyDD, topDinRdyDD);
      Cudd_Ref(newDinRdyDD);
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, tmpDinRdyDD); 
      Cudd_RecursiveDerefZdd(ddTools->dd_manager, topDinRdyDD); 

      printf("New {DIN,RDY} Size:%u\n", Cudd_zddCount(ddTools->dd_manager, newDinRdyDD));

      DdNode * finalDD = newDinRdyDD;
      
      // Save out the results
      struct timeval timecount;
      int * zdd_ordr = g_new(int, 128);
      gettimeofday(&timecount, NULL); 
      gchar * fileName = g_strdup_printf("%s_%d.dd", "invst_dinrdy", (int)timecount.tv_sec);
      
      printf("New DINxRDY fileName: %s\n", fileName);
      
      FILE * fh = fopen(fileName, "w+");
      Dddmp_MoreDDHeaderInfo headerInfo;
      Dddmp_VarInfoType extrainfo; /* not used in binary mode, I think (GDP) */

      for(int k = 0; k < 64; k++)
      	{
      	  zdd_ordr[127-2*k] = k;
      	  zdd_ordr[127-(2*k+1)] = (64+k);
      	}

      headerInfo.extraTraceInfo = g_new(char, 512);
  
      //! add extra information to this ZDD dump
      g_snprintf(headerInfo.extraTraceInfo, 512, "type:dinrdy,dinstart:0,dinstop:0");

      Dddmp_cuddZddStore(ddTools->dd_manager, NULL,
      			 finalDD, NULL, zdd_ordr,
      			 DDDMP_MODE_BINARY, extrainfo, &headerInfo,
      			 fileName, fh);
      fclose(fh);
      g_free(fileName);
      g_free(headerInfo.extraTraceInfo);
      g_free(zdd_ordr);
    }
}

//! Take a quick picture of the plot
void SimpleHQWidget::InitPicture1(void)
{
  struct timeval timecount;
  gettimeofday(&timecount, NULL); 
  gchar * fileName = NULL;
  
  //! check for a command line given DD file
  if(NULL != DDstartupFile)
    {
      QString tempString = QString(DDstartupFile);
      QStringList strlist = tempString.split(QRegExp("(\\s+|:|,)", Qt::CaseSensitive), QString::SkipEmptyParts);

      QString fileString = strlist.at(0);
      fileString.remove(".dd");
      
      fileName = g_strdup_printf("%s_%s_%d.png", fileString.toAscii().constData(), "image", (int)timecount.tv_sec);
    }
  else
    {
      fileName = g_strdup_printf("%s_%d.png", "dinrdy_image", (int)timecount.tv_sec);
    }

  g_print("Image filename: %s\n", fileName);

  imageWork->saveViewBuffer(fileName);
}
