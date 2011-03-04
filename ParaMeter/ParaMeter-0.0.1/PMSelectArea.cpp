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


#include <math.h>
#include <string.h>

#include "HTools.h"
#include "HBaseModel.h"
#include "HBaseView.h"
#include "HBaseOperator.h"
#include "HOpConstructRectangle.h"
#include "PMSelectArea.h"
#include "HEventInfo.h"
#include "HSelectionSet.h"
#include "HUtility.h"
#include "HEventManager.h"
#include "SimpleHQWidget.h"
#include "imagework.h"


/////////////////////////////////////////////////////////////////////////////
// PMSelectArea
//
//! Derived from the HOpSelectArea Class
// Operator for performing a hit test on the scene using a screen-space window
// and placing hit objects in the view's selection list
// Mouse down - computes the first point of the selection window
// Mouse down and move - rubberbands a box to denote current selection window
// Mouse up - computes the area selection and adds items to the selection list
// No mouse down and move - performs an "O-snap" quickmoves highlight of closest marker edge or line segment

 
PMSelectArea::PMSelectArea(HBaseView* view, int DoRepeat, int DoCapture) 
		: HOpConstructRectangle(view, DoRepeat, DoCapture, false)
{
    simple_widget = NULL;
}

PMSelectArea::~PMSelectArea()
{
}

HBaseOperator * PMSelectArea::Clone()
{
	return (HBaseOperator *)new PMSelectArea(m_pView);
}



const char * PMSelectArea::GetName() { return "PMSelectArea"; }


int PMSelectArea::OnLButtonDown(HEventInfo &event)
{	
  if(simple_widget == NULL){
    return 0;
  }
  
  int status = HOpConstructRectangle::OnLButtonDown(event);
  HRenderMode mode = m_pView->GetRenderMode();
  
  HC_Open_Segment_By_Key(m_pView->GetWindowspaceKey());
  //HOpConstructRectangle handles the save and restore for visibility and color

  if (mode != HRenderHiddenLine && 
      mode != HRenderHiddenLineFast && 
      mode != HRenderHiddenLineHOOPS &&
      m_pView->GetTransparentSelectionBoxMode() &&
      m_pView->GetQuickMovesMethod() != HQuickMoves_XOR) {
    HC_Set_Visibility ("faces=on");
    HC_Set_Color ("faces = transmission = light grey");
  }
  HC_Set_Color ("geometry = blue");
  HC_Close_Segment();
  
  //! manually do some selection work
  simple_widget->selectionRect[0] = event.GetMousePixelPos();
  simple_widget->selectionRect[1].x = 0;
  simple_widget->selectionRect[1].y = 0;
  simple_widget->selectionRect[1].z = 0;

  return (status);
}


int PMSelectArea::OnLButtonUp(HEventInfo &event)
{
  quint64 plotBRx, plotBRy, plotTLx, plotTLy;
  
  if (!m_bOpStarted) 
    return HBaseOperator::OnLButtonDownAndMove(event);
  
  
  HOpConstructRectangle::OnLButtonUp(event);
  
  if(simple_widget == NULL){
    return 0;
    }
  
  //! manually do some selection work
  simple_widget->selectionRect[1] = event.GetMousePixelPos();
  
  //! convert those world pixels to image pixels
  simple_widget->ddTools->Pixel2Pixel( &(simple_widget->selectionRect[0]));
  simple_widget->ddTools->Pixel2Pixel( &(simple_widget->selectionRect[1]));
  
  //! catch invalid values
  if(simple_widget->selectionRect[0].x > simple_widget->imageWork->currentBufferWidth)
    {
      simple_widget->selectionRect[0].x = simple_widget->imageWork->currentBufferWidth;
    }   
  if(simple_widget->selectionRect[0].y > simple_widget->imageWork->currentBufferHeight)
    {
      simple_widget->selectionRect[0].y = simple_widget->imageWork->currentBufferHeight;
    }
  if(simple_widget->selectionRect[0].x < 0)
    {
      simple_widget->selectionRect[0].x = 0;
    }   
  if(simple_widget->selectionRect[0].y < 0)
    {
      simple_widget->selectionRect[0].y = 0;
    }    
  if(simple_widget->selectionRect[1].x > simple_widget->imageWork->currentBufferWidth)
    {
      simple_widget->selectionRect[1].x = simple_widget->imageWork->currentBufferWidth;
    }   
  if(simple_widget->selectionRect[1].y > simple_widget->imageWork->currentBufferHeight)
    {
      simple_widget->selectionRect[1].y = simple_widget->imageWork->currentBufferHeight;
    }
  if(simple_widget->selectionRect[1].x < 0)
    {
      simple_widget->selectionRect[1].x = 0;
    }   
  if(simple_widget->selectionRect[1].y < 0)
    {
      simple_widget->selectionRect[1].y = 0;
    }
  
  //! convert those local pixel points to RDYxDIN plot points
  simple_widget->imageWork->Pixel2Plot((quint64) (simple_widget->selectionRect[0].x), 
				       (quint64) (simple_widget->selectionRect[0].y),
				       &plotBRx, &plotBRy);
  simple_widget->imageWork->Pixel2Plot((quint64) (simple_widget->selectionRect[1].x), 
				       (quint64) (simple_widget->selectionRect[1].y),
				       &plotTLx, &plotTLy);
  
  //! convert those points into QT valid points
  QPointF plotBR = QPointF((qreal)plotBRx, (qreal)plotBRy);
  QPointF plotTL = QPointF((qreal)plotTLx, (qreal)plotTLy);
  
  //! straighten out the points
  simple_widget->imageWork->unmangleSelectedPoints(&plotTL, &plotBR);
  
  //! save off the selected plot points into the
  //! imagework class
  simple_widget->imageWork->addSelection(plotTL, plotBR);   
  
  //! update the view created by the DD graph
  simple_widget->OnViewChange();
  
  return(HOP_READY);
}


int PMSelectArea::setWidget(SimpleHQWidget * temp_widget)
{
    simple_widget = temp_widget;

    return (0);
}

int PMSelectArea::RemoveRectangle(void){

    //	HOpConstructRectangle::OnLButtonUp(l_button_up_event);
    return (0);
}

