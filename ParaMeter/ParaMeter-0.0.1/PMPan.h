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
// $Header: /files/homes/master/cvs/hoops_master/hoops_mvo/source/HOpCameraPan.h,v 1.34 2006-08-07 20:38:45 stage Exp $
//

// PMPan.h : interface of the PMPan class
// pans the camera of current view

#ifndef _PMPAN_H
#define _PMPAN_H  

#include "HTools.h"
#include "HBaseOperator.h"
#include <QPointF>

//! forward declarations
class ImageWork;
class SimpleHQWidget;
class BDDWork;


//! The PMPan class pans the camera about a current view.
/*! 
  PMPan implements three mouse event methods defined on the base class and maps the event information
  to HOOPS camera routines. This operation consists of the following steps:
  <ol>
  <li>Left Button Down:				pan initiated
  <li>Left Button Down and Drag:	camera pan
  <li>Left Button Up:				operation ended
  </ol>
  More Detailed Description: see event methods
*/
class MVO_API PMPan : public HBaseOperator
{
public:
	/*! constructor */
    PMPan(HBaseView* view, int DoRepeat=0, int DoCapture=1);
	~PMPan();

	/*!
		Returns a pointer to a character string denoting the name of the operator  'Realtime 2D Camera Pan'
	*/
	virtual const char * GetName();  

	/*!
		OnLButtonDown records the initial mouse position. 
		\param hevent An HEventInfo object containing information about the current event.
		\return A value indicating the result of the event handling.
	*/
    int OnLButtonDown (HEventInfo &hevent);

	/*!
		OnLButtonDownAndMove records points as the mouse is moved and calculates the values used for panning the camera 
		by modifying the camera's target and position.  It also updates the position of the default light, depending on the 
		state of SetLightFollowsCamera. 
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
		SetLightFollowsCamera sets the lights to follow the camera or to stay in one place during pan.  This has
		the effect of either lighting the scene constantly or allowing the camera to pan into shadow.  Default
		is off (follow=0).
		\param follow A Boolean value that switches the follow mechanism on or off.
	*/
	void SetLightFollowsCamera(bool follow){m_LightFollowsCamera = follow;};
	/*!
		Returns the state of the camera follow mechanism, either on or off.
	*/
	bool GetLightFollowsCamera(){return m_LightFollowsCamera;};

	/*
	  Sets a pointer to the current SimpleHQWidget class
	*/
	void setWidget(SimpleHQWidget * temp_widget);

	HBaseOperator * Clone();  /*! returns a pointer to a copy of the operator */

private:
	bool	m_LightFollowsCamera;
	SimpleHQWidget * simple_widget;
    QPointF mousePosition;
};

#endif

