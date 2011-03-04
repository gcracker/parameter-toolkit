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

// Handles area selection

#ifndef _PMSELECTAREA_H
#define _PMSELECTAREA_H

#include "HTools.h"
#include "HOpConstructRectangle.h" 

//! forward class declarations
class ImageWork;
class SimpleHQWidget;

//! Direved from the HOpSelectArea class
//! The PMSelectArea class computes a selection list for objects inside a rectangular area.
/*!
  PMSelectArea employs all of the drawing functionality of HOpConstructRectangle to define a temporary, overlayed rectangular selection 
  area, then maps the rectangle information to the HOOPS routine HC_Compute_Selection_By_Area.  This provides the basic
  functionality for window selection.
  The operation consists of the following steps:
  <ol>
  <li>Left Button Down:				first point of rectangle
  <li>Left Button Down and Drag:	rubberband rectangle to desired dimensions
  <li>Left Button Up:				rectangled completed and flushed from scene, selection list computed, objects highlighted, operation ended
  </ol>
  More Detailed Description: see event methods 
*/
class MVO_API PMSelectArea : public HOpConstructRectangle
{
public:
	/*! constructor */
    PMSelectArea(HBaseView* view, int DoRepeat=0, int DoCapture=1);
	~PMSelectArea();

	/*!
		Returns a pointer to a character string denoting the name of the operator  'PMSelectArea'
	*/
	virtual const char * GetName();  

	/*!
		OnLButtonDown calls through to HOpConstructRectangle then sets a few 
		attributes (color and visibility) that are specific to selection by area
		\param hevent An HEventInfo object containing information about the current event.
		\return A value indicating the result of the event handling.
	*/
	virtual int OnLButtonDown(HEventInfo &hevent);

	/*!
		OnLButtonUp maps rectangle information from HOpConstructRectangle into the HOOPS routine 
		HC_Compute_Selection_By_Area.  This function also deselects any selected items that are not present in the current
		selection list.
		\param hevent An HEventInfo object containing information about the current event.
		\return A value indicating the result of the event handling.
	*/
    virtual int OnLButtonUp(HEventInfo &hevent);  
	
	HBaseOperator * Clone(); /*! returns a pointer to a copy of the operator */
    int setWidget(SimpleHQWidget * temp_widget); 
    int RemoveRectangle(void);

 private:
    SimpleHQWidget * simple_widget;
    //    HEventInfo l_button_up_event;

};

#endif

