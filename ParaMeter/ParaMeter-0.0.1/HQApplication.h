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

// HQApplication.h - Public interface for HOOPS/Qt class HQApplication 
// 
// About this class 
#ifndef HQAPPLICATION_H 
#define HQAPPLICATION_H 

// Qt Headers 
#include <QMainWindow> 
#include <QEvent> 

class QToolButton;
class QWorkspace;

class HQApplication : public QMainWindow 
{ 
 Q_OBJECT 

public: 

    HQApplication(QApplication * p, const char * filename = 0);
	HQApplication(const char * filename); 
	~HQApplication();  

private slots: 

	void myclose();  
	void new_window();  
	void load(); 
	void about();

	void MDIOnPrint();
	void MDIOnSaveFileAs();
	void MDIOnZoomToExtents();
	void MDIOnZoomToWindow();
	void MDIOnOrbit();
	void MDIOnZoom();
	void MDIOnPan();
	void MDIOnCreateSphere();
	void MDIOnCreateCone();
	void MDIOnCreateCylinder();	
	void MDIOnAddDD();

private: 

	void load(const char * filename); 
	QToolBar * tools; 

	//used with MDI only
	QWorkspace * qws;
	struct vhash_s * wh; 

protected: 

	static int count;
    static QApplication * myparent;
    void closeEvent( QCloseEvent * );

    void CreatePixmaps();

	static QPixmap * tsaIcon;
    static QPixmap * pageIcon;
    static QPixmap * openIcon;
    static QPixmap * printIcon;
    static QPixmap * zoomextentsIcon;
    static QPixmap * zoomwindowIcon;
    static QPixmap * orbitIcon;
    static QPixmap * zoomIcon;
    static QPixmap * panIcon;
    static QPixmap * sphereIcon;
    static QPixmap * coneIcon;
    static QPixmap * cylinderIcon;
    static QPixmap * addddIcon;
	static QPixmap * selectIcon;


	QAction * orbitAct;
	QAction * zoomAct;
	QAction * panAct;
	QAction * zExtentsAct;
	QAction * zWindowAct;
	QAction * sphereAct;
	QAction * cylAct;
	QAction * coneAct;
	QAction * addddAct;
	QAction * newAct;
	QAction * openAct;
	QAction * printAct;
	QAction * selectedSINAct;	
	QAction * selectAct;
	QAction * DDRefAct;
}; 

#endif  


