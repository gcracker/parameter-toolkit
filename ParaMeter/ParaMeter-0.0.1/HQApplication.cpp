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

// HQApplication.cpp - Implementation of the HQApplication class 
  
#include <stdlib.h>


// qt includes
#include <QApplication>
#include <QCoreApplication>
#include <QPoint>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QPixmap>
#include <QBitmap>
#include <QToolBar>
#include <QToolButton>
#include <QMainWindow>
#include <QMessageBox> 
#include <QSplitter>
#include <QLayout> 
#include <QHBoxLayout> 
#include <QWorkspace>
#include <QFrame>
#include <QLayout>

// location of the image files for menubar icons 
#include "images/tsa.xpm" 
#include "images/cone.xpm" 
#include "images/cylinder.xpm" 
#include "images/runmycode.xpm" 
#include "images/sphere.xpm" 
#include "images/fittowindow.xpm" 
#include "images/folder.xpm" 
#include "images/orbit.xpm" 
#include "images/pan.xpm" 
#include "images/zoominter.xpm" 
#include "images/zoomtowindow.xpm" 
#include "images/print.xpm" 
#include "images/page.xpm" 
#include "images/select.xpm"

// HOOPS/Qt includes 
#include "HQApplication.h" 
#include "SimpleHQWidget.h" 
#include "HQDeleter.h" 
  
#include "vhash.h"

#include "hversion.h" //HOOPS 3dgs' version header

// Global HQDeleter object; defined in main.cpp 
extern HQDeleter * deleter; 


//this is used for the global quit
QApplication * HQApplication::myparent;
int HQApplication::count;

QPixmap * HQApplication::tsaIcon = 0;
QPixmap * HQApplication::pageIcon = 0;
QPixmap * HQApplication::openIcon = 0;
QPixmap * HQApplication::printIcon = 0;
QPixmap * HQApplication::zoomextentsIcon = 0;
QPixmap * HQApplication::zoomwindowIcon = 0;
QPixmap * HQApplication::orbitIcon = 0;
QPixmap * HQApplication::zoomIcon = 0;
QPixmap * HQApplication::panIcon = 0;
QPixmap * HQApplication::sphereIcon = 0;
QPixmap * HQApplication::coneIcon = 0;
QPixmap * HQApplication::cylinderIcon = 0;
QPixmap * HQApplication::addddIcon = 0;
QPixmap * HQApplication::selectIcon = 0;
//QPixmap * HQApplication::selectIcon = 0;

// this should be called only once
HQApplication::HQApplication(QApplication * p, const char * filename) : QMainWindow()
{
	myparent = p;
	count=1;
	
#if USE_MDI
	wh = new_vhash(16,malloc,free);
#endif

    CreatePixmaps();

	this->setWindowIcon(*tsaIcon);
	this->setWindowIconText("ParaMeter");
	this->setWindowTitle(QString("ParaMeter"));
	this->load(filename);
}

HQApplication::HQApplication(
	const char * filename) : QMainWindow()
{

	count++;
	this->setWindowIcon(*tsaIcon);
	this->setWindowIconText("Simple");
	this->setWindowTitle(QString("ParaMeter"));
	this->load(filename);


}

HQApplication::~HQApplication()
{

#if USE_MDI
	delete_vhash(wh);
#endif

	count--;
	
	if (count==0) {			

		QApplication::exit(0);
	}
}


void HQApplication::myclose() 
{ 
	// This method uses the HQDelete object to schedule the deletion of  
	// the HQApplication "this" pointer for after the Qt Signal/Slot  
	// processor has completed unwinding. 

	this->hide(); 

	deleter->deleteLater(this); 
} 


void HQApplication::closeEvent( QCloseEvent * ) 
{ 
	// catch the X button 
	myclose(); 
} 


void HQApplication::new_window() 
{ 
	// create a new HQApplication object without loading a file 
#ifdef USE_MDI
	QFrame * qframe = new QFrame(qws);
	QHBoxLayout * qlayout = new QHBoxLayout(qframe);
	qlayout->setMargin(0);
	SimpleHQWidget * my_widget = new SimpleHQWidget( qframe, "SimpleHQWidget", (const char *)0);
	
	qlayout->addWidget(my_widget);
	my_widget->setMinimumSize( 150, 150 ); 
	vhash_insert_item(wh, (void*)qframe,(void*)my_widget);
	qws->addWindow(qframe);
	qframe->show();
#else
	new HQApplication((const char *)0); 
#endif

} 


// open a file dialog and create a new window to for it
void HQApplication::load()
{
	QFileDialog* qfd = new QFileDialog(this, QString::null);

    // setup selection filter
    QStringList demFilters;

    demFilters.append("HOOPS Stream Files (*.hsf)");
    demFilters.append("HOOPS Metafiles (*.hmf)");

    qfd->setFilters(demFilters);

    // first get new filename
    qfd->setFileMode(QFileDialog::ExistingFile);

    if (qfd->exec() == QDialog::Accepted){

                QStringList fnames = qfd->selectedFiles();
				QString fname = QString(fnames.first());
                if (!fname.isEmpty())
#ifdef USE_MDI
					{
						QFrame * qframe = new QFrame(qws);
						qframe->setWindowTitle(QString(fname));
						QHBoxLayout * qlayout = new QHBoxLayout(qframe);
						qlayout->setMargin(0);
						SimpleHQWidget * my_widget = new SimpleHQWidget( qframe, "SimpleHQWidget", (const char *)fname.toAscii());
						
						qlayout->addWidget(my_widget);
						my_widget->setMinimumSize( 150, 150 ); 
						qframe->show();
						vhash_insert_item(wh, (void*)qframe,(void*)my_widget);
						qws->addWindow(qframe);
					}
#else
					new HQApplication((const char *)(fname.toAscii()));
#endif
	}

	delete qfd;
}




// about dialog
void HQApplication::about()
{
	char tempstr[4096];
        int major, minor;

	major = HOOPS_VERSION / 100;
	minor = HOOPS_VERSION % 100;
	sprintf( tempstr, 
		"ParaMeter Viewer %d.%d\n"
		"Univeristy of Colorado at Boulder\n"
		"Built against version %s of Qt, from TrollTech\n",
			major,
			minor,
			QT_VERSION_STR );

	QMessageBox::information( this, "about ParaMeter", tempstr );
}


void HQApplication::load(const char * filename)
{

#ifdef USE_MDI
	qws = new QWorkspace( this);
	this->setCentralWidget( qws ); 

	QFrame * qframe = new QFrame(qws);
	QHBoxLayout * qlayout = new QHBoxLayout(qframe);
	qlayout->setMargin(0);
	SimpleHQWidget * my_widget = new SimpleHQWidget( qframe, "SimpleHQWidget", filename);
	
	qlayout->addWidget(my_widget);
	my_widget->setMinimumSize( 150, 150 ); 
	qframe->show();
	vhash_insert_item(wh, (void*)qframe,(void*)my_widget);
	qws->addWindow(qframe)->setWindowTitle(QString(filename));

#else

	// Create an HOOPS widget
	SimpleHQWidget * my_widget = new SimpleHQWidget( this, "SimpleHQWidget", filename);
	my_widget->setMinimumSize( 50, 50 ); 

	this->setCentralWidget( my_widget ); 

#endif
    

	/* Create a tool bar*/
	tools = new QToolBar( "ops", this );


	newAct = new QAction(*pageIcon, tr("&New Window"), this);
	connect(newAct, SIGNAL(triggered()), this, SLOT(new_window()));
	tools->addAction(newAct);

	openAct = new QAction(*openIcon, tr("&Open File"), this);
	connect(openAct, SIGNAL(triggered()), this, SLOT(load()));
	tools->addAction(openAct);

#ifdef USE_MDI
	printAct = new QAction(*printIcon, tr("&Print"), this);
	connect(printAct, SIGNAL(triggered()), this, SLOT(MDIOnPrint()));
	tools->addAction(printAct);
#else
	printAct = new QAction(*printIcon, tr("&Print"), my_widget);
	connect(printAct, SIGNAL(triggered()), my_widget, SLOT(OnPrint()));
	tools->addAction(printAct);
#endif


	tools->addSeparator ();

#ifdef USE_MDI
	zExtentsAct = new QAction(*zoomextentsIcon, tr("&Zoom to Extents"), this);
	connect(zExtentsAct, SIGNAL(triggered()), this, SLOT(MDIOnZoomToExtents()));
	tools->addAction(zExtentsAct);

	zWindowAct = new QAction(*zoomwindowIcon, tr("&Zoom to Window"), this);
	connect(zWindowAct, SIGNAL(triggered()), this, SLOT(MDIOnZoomToWindow()));
	tools->addAction(zWindowAct);
#else
	zExtentsAct = new QAction(*zoomextentsIcon, tr("&Zoom to Extents"), my_widget);
	connect(zExtentsAct, SIGNAL(triggered()), my_widget, SLOT(OnZoomToExtents()));
	tools->addAction(zExtentsAct);

	zWindowAct = new QAction(*zoomwindowIcon, tr("&Zoom to Window"), my_widget);
	connect(zWindowAct, SIGNAL(triggered()), my_widget, SLOT(OnZoomToWindow()));
	tools->addAction(zWindowAct);
#endif


 
	tools->addSeparator ();

#ifdef USE_MDI
	orbitAct = new QAction(*orbitIcon, tr("&Orbit"), this);
	connect(orbitAct, SIGNAL(triggered()), this, SLOT(MDIOnOrbit()));
	tools->addAction(orbitAct);

	zoomAct = new QAction(*zoomIcon, tr("&Zoom"), this);
	connect(zoomAct, SIGNAL(triggered()), this, SLOT(MDIOnZoom()));
	tools->addAction(zoomAct);

	panAct = new QAction(*panIcon, tr("&Pan"), this);
	connect(panAct, SIGNAL(triggered()), this, SLOT(MDIOnPan()));
	tools->addAction(panAct);
#else

	selectAct = new QAction(*selectIcon, tr("&Select by Single Click"), this);
	selectAct->setCheckable(true);
	connect(selectAct, SIGNAL(triggered()), my_widget, SLOT(OnApertureSelect()));
	tools->addAction(selectAct);

	tools->addSeparator ();

    /*
	orbitAct = new QAction(*orbitIcon, tr("&Orbit"), my_widget);
	connect(orbitAct, SIGNAL(triggered()), my_widget, SLOT(OnOrbit()));
	tools->addAction(orbitAct);
    */

	zoomAct = new QAction(*zoomIcon, tr("&Zoom"), my_widget);
	connect(zoomAct, SIGNAL(triggered()), my_widget, SLOT(OnZoom()));
	tools->addAction(zoomAct);

	panAct = new QAction(*panIcon, tr("&Pan"), my_widget);
	connect(panAct, SIGNAL(triggered()), my_widget, SLOT(OnPan()));
	tools->addAction(panAct);

#endif
	


	tools->addSeparator ();


#ifdef USE_MDI
   
	sphereAct = new QAction(*sphereIcon, tr("&Create Sphere"), this);
	connect(sphereAct, SIGNAL(triggered()), this, SLOT(MDIOnCreateSphere()));
	tools->addAction(sphereAct);

	coneAct = new QAction(*coneIcon, tr("&Create Cone"), this);
	connect(coneAct, SIGNAL(triggered()), this, SLOT(MDIOnCreateCone()));
	tools->addAction(coneAct);

	cylAct = new QAction(*cylinderIcon, tr("&Create Cylinder"), this);
	connect(cylAct, SIGNAL(triggered()), this, SLOT(MDIOnCreateCylinder()));
	tools->addAction(cylAct);

	tools->addSeparator ();

	addddAct = new QAction(*adddIcon, tr("&Add DD"), this);
	connect(addddAct, SIGNAL(triggered()), this, SLOT(MDIOnRunMyCode()));
	tools->addAction(addddAct);
#else

	DDRefAct = new QAction(*sphereIcon, tr("&Refresh DD View"), my_widget);
	connect(DDRefAct, SIGNAL(triggered()), my_widget, SLOT(OnDDRefresh()));
	tools->addAction(DDRefAct);

    /*
	sphereAct = new QAction(*sphereIcon, tr("&Create Sphere"), my_widget);
	connect(sphereAct, SIGNAL(triggered()), my_widget, SLOT(OnCreateSphere()));
	tools->addAction(sphereAct);

	coneAct = new QAction(*coneIcon, tr("&Create Cone"), my_widget);
	connect(coneAct, SIGNAL(triggered()), my_widget, SLOT(OnCreateCone()));
	tools->addAction(coneAct);

	cylAct = new QAction(*cylinderIcon, tr("&Create Cylinder"), my_widget);
	connect(cylAct, SIGNAL(triggered()), my_widget, SLOT(OnCreateCylinder()));
	tools->addAction(cylAct);
    */

	tools->addSeparator ();

	addddAct = new QAction(*addddIcon, tr("&Add DD"), my_widget);
	connect(addddAct, SIGNAL(triggered()), my_widget, SLOT(OnAddDD()));
	tools->addAction(addddAct);

#endif
	
	this->addToolBar(tools);


	// File menu
	QMenu *file = this->menuBar()->addMenu(tr("&File"));
	file->addAction( "New", this, SLOT(new_window()));
	file->addAction( "Open", this, SLOT(load()));
	file->addSeparator();

	file->addAction( "Print", my_widget, SLOT(OnPrint())); 
	file->addAction( "Save", my_widget, SLOT(OnSaveFileAs())); 
	file->addAction( "Save Image", my_widget, SLOT(OnSaveImage()));
	file->addAction( "Save Selected Regions", my_widget, SLOT(OnSaveRegions()));
	file->addAction( "Add Selected Regions", my_widget, SLOT(OnAddSelRegions()));

	file->addSeparator();
	file->addAction( "Close", this, SLOT(myclose()), Qt::CTRL+Qt::Key_C ); 
	file->addSeparator();
	file->addAction( "Exit", myparent, SLOT(quit()), Qt::CTRL+Qt::Key_Q );


	//! Trace Analysis menu
	QMenu *ddtrace = this->menuBar()->addMenu(tr("&Trace"));
	ddtrace->addAction( "DINs->PCs", my_widget, SLOT(OnSelectedDINs()));
	ddtrace->addAction( "BFD File",my_widget, SLOT(GetBFDFile()));
	ddtrace->addAction( "Select Region",my_widget, SLOT(OnSelectRegion()));
	ddtrace->addAction( "Clear Selected Regions",my_widget, SLOT(OnClearSelectedRegions()));
	ddtrace->addAction( "Find Dep",my_widget, SLOT(OnFindDep()));
	ddtrace->addAction( "Dead Ready Filter",my_widget, SLOT(OnDeadRdyFilter ()));
	ddtrace->addAction( "Unique Static Slice Filter",my_widget, SLOT(OnUniqStaticFilter ()));
    

#ifndef USE_MDI
	// Manipulate menu
	
	QMenu *manipulate = this->menuBar()->addMenu(tr("&Manipulate"));
    //	manipulate->addAction( orbitAct);
	manipulate->addAction(zoomAct);
	manipulate->addAction(panAct);
	manipulate->addAction(zWindowAct);
	manipulate->addAction(zExtentsAct);
	manipulate->connect(manipulate, SIGNAL(aboutToShow()), this, SLOT(UpdateMenus()));

	
	// Create menu
	QMenu *create = this->menuBar()->addMenu(tr("&Create"));
    //	create->addAction(sphereAct);
    //	create->addAction(cylAct);
    //	create->addAction(coneAct);
	create->addAction(addddAct);
	create->connect(create, SIGNAL(aboutToShow()), this, SLOT(UpdateMenus()));
	
#endif
	
	
	


	// Help menu
	QMenu *help = this->menuBar()->addMenu(tr("&Help"));
    help->addAction( "About", this , SLOT(about()) );

	this->resize(640,400);
	this->show();
}


void HQApplication::CreatePixmaps()
{

    selectIcon = new QPixmap( (const char**)select_xpm );
	selectIcon->setMask( selectIcon->createHeuristicMask() );

	tsaIcon = new QPixmap( (const char**)tsa_xpm );
	tsaIcon->setMask( tsaIcon->createHeuristicMask() );
	
    pageIcon = new QPixmap( (const char**)page_xpm );
	pageIcon->setMask( pageIcon->createHeuristicMask() );
	
    openIcon = new QPixmap( (const char**)folder_xpm );
	openIcon->setMask( openIcon->createHeuristicMask() );
	
    printIcon = new QPixmap( (const char**)print_xpm );
	printIcon->setMask( printIcon->createHeuristicMask() );
	
    zoomextentsIcon = new QPixmap( (const char**) fittowindow_xpm );
	zoomextentsIcon->setMask( zoomextentsIcon->createHeuristicMask() );

    zoomwindowIcon = new QPixmap( (const char**)zoomtowindow_xpm);
	zoomwindowIcon->setMask( zoomwindowIcon->createHeuristicMask() );

    orbitIcon = new QPixmap( (const char**)orbit_xpm );
	orbitIcon->setMask( orbitIcon->createHeuristicMask() );

    zoomIcon = new QPixmap( (const char**)zoominter_xpm );
	zoomIcon->setMask( zoomIcon->createHeuristicMask() );

    panIcon = new QPixmap( (const char**)pan_xpm );
	panIcon->setMask( panIcon->createHeuristicMask() );

    sphereIcon = new QPixmap( (const char**)sphere_xpm );
	sphereIcon->setMask( sphereIcon->createHeuristicMask() );

    coneIcon = new QPixmap( (const char**)cone_xpm );
	coneIcon->setMask( coneIcon->createHeuristicMask() );

    cylinderIcon = new QPixmap( (const char**)cylinder_xpm );
	cylinderIcon->setMask( cylinderIcon->createHeuristicMask() );

    addddIcon = new QPixmap( (const char**)runmycode_xpm );
	addddIcon->setMask( addddIcon->createHeuristicMask() );

}

void HQApplication::MDIOnPrint()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnPrint();
	}
#endif
}

void HQApplication::MDIOnSaveFileAs()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnSaveFileAs();
	}
#endif
}

void HQApplication::MDIOnZoomToExtents()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnZoomToExtents();
	}
#endif
}

void HQApplication::MDIOnZoomToWindow()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnZoomToWindow();
	}
#endif
}

void HQApplication::MDIOnOrbit()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnOrbit();
	}
#endif
}

void HQApplication::MDIOnZoom()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnZoom();
	}
#endif
}

void HQApplication::MDIOnPan()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnPan();
	}
#endif
}

void HQApplication::MDIOnCreateSphere()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnCreateSphere();
	}
#endif
}

void HQApplication::MDIOnCreateCone()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnCreateCone();
	}
#endif
}

void HQApplication::MDIOnCreateCylinder()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnCreateCylinder();
	}
#endif
}

void HQApplication::MDIOnAddDD()
{
#if USE_MDI
	QFrame* f=(QFrame*)qws->activeWindow();
	if(f) {
		SimpleHQWidget* w;
		vhash_lookup_item(wh, (void*)f, (void**)&w); 
		if(w) w->OnRunMyCode();
	}
#endif
}



