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
// $Header: /files/homes/master/cvs/hoops_master/hoops_mvo/source/PMPan.cpp,v 1.53 2006-08-07 20:38:45 stage Exp $
//

/*
  Large portions of this file were taken from the Tech Soft 3D
  HOpCameraPan class 
*/

// PMPan.cpp : implementation of the PMPan class
//

#include <string.h>
#include <math.h>

#include "HTools.h"
#include "HBaseModel.h"
#include "HBaseView.h"
#include "HBaseOperator.h"
#include "HEventInfo.h"
#include "PMPan.h"
#include "HBhvBehaviorManager.h"
#include "SimpleHQWidget.h"
#include "imagework.h"
#include "bddwork.h"
#include "zddwork.h"


/////////////////////////////////////////////////////////////////////////////
// PMPan
//
// Operator for panning the scene's camera based on the user dragging the mouse
// with the left button down
//
// Left button down - find first position and bounds object with box
//
// Mouse motion while down - pans camera based on difference between previous and 
// current world space point
//



PMPan::PMPan(HBaseView* view, int DoRepeat,int DoCapture) : HBaseOperator(view, DoRepeat, DoCapture)
{
	m_LightFollowsCamera = false;
	simple_widget = NULL;
}

PMPan::~PMPan()
{
  simple_widget = NULL;
}


HBaseOperator * PMPan::Clone()
{
	return (HBaseOperator *)new PMPan(m_pView);
}



const char * PMPan::GetName() { return "PMPan"; }



int PMPan::OnLButtonDown(HEventInfo &event)
{
    HPoint temp_mouse_pos;
    QPointF test;
// 	if (m_pView->GetModel()->GetBhvBehaviorManager()->IsPlaying() && m_pView->GetModel()->GetBhvBehaviorManager()->GetCameraUpdated())
// 		return (HOP_OK);

 	if (!m_bOpStarted) 
 		m_bOpStarted = true;

    temp_mouse_pos = event.GetMouseWorldPos(); 


    //! set the image tools mouse position
    mousePosition.setX(temp_mouse_pos.x);
	mousePosition.setY(temp_mouse_pos.y);

    //	m_pView->PrepareForCameraChange();
 
    return (HOP_OK);
}



int PMPan::OnLButtonDownAndMove(HEventInfo &event)
{
    HPoint delta, camera, target;
    qreal delta_scale = 100;
    float w,h;
	
    if (!m_bOpStarted) 
        return HBaseOperator::OnLButtonDownAndMove(event);

    if(simple_widget == NULL){
        return HBaseOperator::OnLButtonDownAndMove(event);
    }


    m_ptNew = event.GetMouseWorldPos();  
										
    delta.x = (m_ptNew.x - mousePosition.x()) * delta_scale;
    delta.y = (m_ptNew.y - mousePosition.y()) * delta_scale;
    delta.z = 0;

    //! reset the image tools mouse position
    mousePosition.setX(m_ptNew.x);
	mousePosition.setY(m_ptNew.y);

    HC_Open_Segment_By_Key (m_pView->GetSceneKey());
		
		HC_Show_Net_Camera_Target (&target.x, &target.y, &target.z);
		HC_Show_Net_Camera_Position (&camera.x, &camera.y, &camera.z);
	
        //		HC_Set_Camera_Target (target.x - delta.x, target.y - delta.y, target.z - delta.z);
        //		HC_Set_Camera_Position (camera.x - delta.x, camera.y - delta.y, camera.z - delta.z);
        HC_Show_Net_Camera_Field(&w, &h);
        
        //! pan the image
        simple_widget->imageWork->PanView(delta.x, -delta.y);

        //! update the BDD window
        simple_widget->ddTools->SetWindow();
        
        //! refresh the BDD window
        simple_widget->OnViewChangeWTimer();
		  
        
	HC_Close_Segment();

    //    m_pView->CameraPositionChanged();

	m_pView->Update();
	return (HOP_OK);
}




int PMPan::OnLButtonUp(HEventInfo &event)
{
	if(!m_bOpStarted) 
	    return HBaseOperator::OnLButtonDownAndMove(event);

    if(simple_widget == NULL){
        return HBaseOperator::OnLButtonDownAndMove(event);
    }

    //! set the image tools mouse position
    mousePosition.setX(0);
	mousePosition.setY(0);

 	m_bOpStarted = false;
    //    m_pView->CameraPositionChanged(true, true);

    return(HOP_READY);
}



void PMPan::setWidget(SimpleHQWidget * temp_widget)
{
    simple_widget = temp_widget;
}



