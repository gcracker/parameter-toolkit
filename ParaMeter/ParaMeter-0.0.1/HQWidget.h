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

//
// Copyright (c) 2000 by Tech Soft 3D, LLC.
// The information contained herein is confidential and proprietary to
// Tech Soft 3D, LLC., and considered a trade secret as defined under
// civil and criminal statutes.  Tech Soft 3D shall pursue its civil
// and criminal remedies in the event of unauthorized use or misappropriation
// of its trade secrets.  Use of this information by anyone other than
// authorized employees of Tech Soft 3D, LLC. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
// $Header: /files/homes/master/cvs/hoops_master/qt_simple_4/HQWidget.h,v 1.32 2006-12-15 08:45:47 david Exp $
//


#ifndef HQWIDGET_H
#define HQWIDGET_H


#if defined(IS_OSX) && defined(USE_MDI)
#include <QGLWidget>
#else
#include <QWidget>
#endif

#if defined(ACIS) || defined(PARASOLID)
#include "HSolidModel.h"
#include "HSolidView.h"
#else
#include "HBaseModel.h"
#include "HBaseView.h"
#endif


class HNetClient;

/*!
	The HQWidget class provides a HOOPS-specific implementation of the QWidget object. 	It serves as a base HOOPS/Qt widget 
	class and should be sub-classed to create an application specific derivation.

	HQWidget creates and manages the connection of a HOOPS/3dGS driver instance to a QWidget object.  The custom widget:

	- optimally configures associated windowing system resources, such as X11 Visuals & colormaps or 
	  MS Windows color palettes

	- ensures correct handling of "Paint/Expose" and "Resize" events

	- overrides the QWidget's Mouse and Key events and pass the events to the appropriate HOOPS/MVO View's current Operator methods 
*/
#if defined(IS_OSX) && defined(USE_MDI)
class HQWidget: public QGLWidget
#else
class HQWidget: public QWidget
#endif
{
	Q_OBJECT
public:
	HQWidget( QWidget* parent=0, const char* name=0, Qt::WFlags f=0, HNetClient * net_client=0);
	~HQWidget();

	/*! \return A pointer to the HOOPS/MVO HBaseView object associated with the widget */
	HBaseView* GetHoopsView() { return (HBaseView*) m_pHView; }

	/*! \return A pointer to the HOOPS/MVO HBaseModel object associated with the widget */
	HBaseModel* GetHoopsModel() { return (HBaseModel*) m_pHBaseModel; }

	/*! \return A pointer to the HOOPS/Net HNetClient object associated with the widget */
	HNetClient* GetHNetClient() { return m_pHNetClient; }

	static bool GetKeyState(unsigned int key, int &flags);

protected:

	/*! \return A pointer to the colormap associated with the widget */
	void * GetColorMap();

	/*! \return A pointer to the window id associated with the widget */
	void * GetWindowId();

#if defined(IS_OSX) && defined(USE_MDI)
	/*! \return A pointer to the QGLContext if derived from a QGLWidget (OSX MDI only) */
	void * GetQGLContext();
#endif

	/*! \return A pointer to the clip override structure; currently only applies to OS/X */
	void * GetClipOverride();

	/*! Performs HOOPS/QT specific initialization */
    virtual void Init();	

	/*! return null */
	virtual QPaintEngine* paintEngine() const;

	/*! 
		Dispatches the OnLeftButtonDown event to all event listeners that are registered for the OnLeftButtonDown event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnLeftButtonDown(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnLeftButtonUp event to all event listeners that are registered for the OnLeftButtonUp event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnLeftButtonUp(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnLeftButtonDblClk event to all event listeners that are registered for the OnLeftButtonDblClk event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnLeftButtonDblClk(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnMidButtonDown event to all event listeners that are registered for the OnMidButtonDown event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnMidButtonDown(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnMidButtonUp event to all event listeners that are registered for the OnMidButtonUp event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnMidButtonUp(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnMidButtonDblClk event to all event listeners that are registered for the OnMidButtonDblClk event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnMidButtonDblClk(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnRightButtonDown event to all event listeners that are registered for the OnRightButtonDown event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnRightButtonDown(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnRightButtonUp event to all event listeners that are registered for the OnRightButtonUp event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnRightButtonUp(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnRightButtonDblClk event to all event listeners that are registered for the OnRightButtonDblClk event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void OnRightButtonDblClk(QMouseEvent * e = 0);

	/*! 
		Dispatches the OnRightButtonDblClk event to all event listeners that are registered for the OnMouseWheel event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	virtual void wheelEvent(QWheelEvent * e = 0);

	/*! 
		Overloaded to automatically perform a HOOPS/3dGS update
	*/
	void paintEvent( QPaintEvent* e);

	/*! 
		Overloaded to automatically perform a HOOPS/3dGS update
	*/
	void resizeEvent( QResizeEvent* e);

	void focusInEvent(QFocusEvent* e);

	void focusOutEvent(QFocusEvent* e);


	/*! 
		Calls the OnKeyDown method of the view's current HOOPS/MVO HBaseOperator object. 
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	void keyPressEvent ( QKeyEvent * e );

	/*! 
		Calls the OnKeyUp method of the view's current HOOPS/MVO HBaseOperator object. 
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	void keyReleaseEvent ( QKeyEvent * e );

	/*! 
		Looks at the state of the left/mid/right mouse buttons and calls the appropriate On[type]ButtonDown method of the widget
	*/
	void mousePressEvent( QMouseEvent * mev);

	/*! 
		Looks at the state of the left/mid/right mouse buttons and calls the appropriate On[type]ButtonDblClk method of the widget
	*/
	void mouseDoubleClickEvent( QMouseEvent * mev);

	/*! 
		Looks at the state of the left/mid/right mouse buttons and calls the appropriate On[type]ButtonUp method of the widget
	*/
	void mouseReleaseEvent( QMouseEvent * mev);


	/*! 
		Dispatches the OnMouseMove event to all event listeners that are registered for the OnMouseMove event type.
		The GUI specific flags are mapped to HOOPS/MVO abstracted flags
	*/
	void mouseMoveEvent( QMouseEvent * mev);

	/*! Adjusts the relative size of the Axis window so that it always has the same size if the outer window is resized */
	void AdjustAxisWindow();

	/*!
		Used to stop events from getting passed to the object
	*/
	bool eventFilter(QObject *obj, QEvent *ev);

#ifdef PARASOLID

	/*! Pointer to the HSolidView object associated with this widget */
	HSolidView * m_pHView;

	/*! Pointer to the HSolidModel object associated with this widget */
	HSolidModel * m_pHBaseModel;
#else
	/*! Pointer to the HBaseView object associated with this widget */
	HBaseView * m_pHView;

	/*! Pointer to the HBaseModel object associated with this widget */
	HBaseModel * m_pHBaseModel;
#endif

	/*! Pointer to the HNetClient object associated with this widget */
	HNetClient * m_pHNetClient;

private:

	bool initDone;

	void setup_window(bool use_gl);

	unsigned long MapFlags(unsigned long state);

	void * my_colormap;	
	void * my_windowid;
	void * my_clip_override;
	bool my_have_gl_visual;

#if IS_OSX
	int use_clip_override[8];
#if USE_MDI
	void * my_qgl_context;
#endif
#endif

};


#endif 


