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

// HPanCamera.h : interface of the HPanCamera class
// zooms the camera of current view in realtime

#ifndef _PMCAMERAZOOM_H
#define _PMCAMERAZOOM_H

#include "HTools.h"
#include "HBaseOperator.h"

//! forward declarations
class SimpleHQWidget;
class ImageWork;
class BDDWork;

//! The PMZoom class zooms the camera toward a target.
/*! 
  PMZoom implements three of the mouse button handlers defined on the base class and maps the event
  information to HOOPS camera routines.  This provides the basic functionality for 
  interactively zooming in or out from a model in realtime.  The operation consists of the following steps:
  <ol>
  <li>Left Button Down:				zoom initiated
  <li>Left Button Down and Drag:	camera zoom to default target
  <li>Left Button Up:				operation ended
  </ol>
  More Detailed Description: see event methods
*/
class MVO_API PMZoom : public HBaseOperator
{
public:
	/*! constructor */
    PMZoom (HBaseView* view, int DoRepeat=0, int DoCapture=1); 
	~PMZoom();

	/*!
		Returns a pointer to a character string denoting the name of the operator  'Realtime Zoom'
	*/
	virtual const char * GetName();  

	/*!
		OnLButtonDown draws a bounding box around the scene; records the first mouse position; and calculates the current
		camera target based on the first point. 
		\param hevent An HEventInfo object containing information about the current event.
		\return A value indicating the result of the event handling.
	*/
    int OnLButtonDown (HEventInfo &hevent);

	/*!
		OnLButtonDownAndMove tests for left button down; records points as the mouse is moved; recalculates the current
		target for each point; and zooms the camera towards that current target. The function also checks to avoid 
		unneccessary zoom when the camera is close to the target. Depending on the state of SetLightFollowsCamera, 
		lighting targets will also be recalculated and lights reoriented.
		\param hevent An HEventInfo object containing information about the current event.
		\return A value indicating the result of the event handling.
	*/
	int OnLButtonDownAndMove(HEventInfo &hevent);

	/*!
		OnLButtonUp updates the scene with current camera and lighting information and cleans up. 
		\param hevent An HEventInfo object containing information about the current event.
		\return A value indicating the result of the event handling.
	*/
    int OnLButtonUp (HEventInfo &hevent);

	/*!
		SetLightFollowsCamera sets the lights to follow the camera or to stay in one place during zoom.  This has
		the effect of either lighting the scene constantly or allowing the camera to zoom into shadow.  Default
		is off (follow=0).
		\param follow A Boolean value that switches the follow mechanism on or off.
	*/
	void SetLightFollowsCamera(bool follow){m_LightFollowsCamera = follow;};

	/*!
		Returns the state of the camera follow mechanism, either on or off.
	*/
	bool GetLightFollowsCamera(){return m_LightFollowsCamera;};

	/*
	  use this widget as the parent
	*/
    void setWidget(SimpleHQWidget * temp_widget);


	HBaseOperator * Clone();  /*! returns a pointer to a copy of the operator */

private:
	bool	m_LightFollowsCamera;	
	HPoint	m_ptDist, m_ptCamera;
	float	m_Width, m_Height, m_fLength;
	SimpleHQWidget * simple_widget;
};

#endif

