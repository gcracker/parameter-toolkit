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



// PMZoom.cpp : implementation of the HOpCameraZoom class
//

#include <math.h>
#include <string.h>

#include "PMZoom.h"
#include "HTools.h"
#include "HBaseModel.h"
#include "HBaseView.h"
#include "HBaseOperator.h"
#include "HEventInfo.h"
#include "HOpCameraZoom.h"
#include "HBhvBehaviorManager.h"
#include "SimpleHQWidget.h"
#include "imagework.h"
#include "bddwork.h"
#include "zddwork.h"


/////////////////////////////////////////////////////////////////////////////
// PMZoom
//
// Operator for zooming the scene's camera based on the user mouse input
// based on the demo HOpCameraZoom 
//
// Left button down - find first position and bounds object with box
//
// Mouse motion while down - zooms camera based on directon of movement
// mouse movement to right zooms in, movement to left zooms out
//


PMZoom::PMZoom(HBaseView* view, int DoRepeat,int DoCapture) : HBaseOperator(view, DoRepeat, DoCapture)
{
  m_LightFollowsCamera = false;
  simple_widget = NULL;
}

PMZoom::~PMZoom()
{
  simple_widget = NULL;
}


HBaseOperator * PMZoom::Clone()
{
	return (HBaseOperator *)new PMZoom(m_pView);
}



const char * PMZoom::GetName() { return "PMZoom"; }

/////////////////////////////////////////////////////////////////////////////
// PMZoom message handlers



int PMZoom::OnLButtonDown(HEventInfo &event)
{
	HPoint target;

    if(simple_widget == NULL){
        return 0;
    }


// 	if (m_pView->GetModel()->GetBhvBehaviorManager()->IsPlaying() && m_pView->GetModel()->GetBhvBehaviorManager()->GetCameraUpdated())
// 		return (HOP_OK);
	
 	if (!m_bOpStarted) 
 		m_bOpStarted = true;

	m_ptNew = event.GetMouseWindowPos();

    //! set the image tools mouse position
    simple_widget->imageWork->mousePosition.setX(m_ptNew.x);
	simple_widget->imageWork->mousePosition.setY(m_ptNew.y);

    // 	HC_Open_Segment_By_Key(m_pView->GetSceneKey());
       
// 		HC_Show_Net_Camera_Target(&m_ptCamera.x,&m_ptCamera.y,&m_ptCamera.z);
// 		HC_Show_Net_Camera_Position(&target.x,&target.y,&target.z);	
// 		m_ptDist.Set(target.x-m_ptCamera.x, target.y - m_ptCamera.y, target.z - m_ptCamera.z);
// 		m_fLength = (float)HC_Compute_Vector_Length(&m_ptDist);
// 		HC_Compute_Normalized_Vector(&m_ptDist,&m_ptDist);
// 		HC_Show_Net_Camera_Field(&m_Width, &m_Height);

       //  //! calculate the current window size in pixels
//         HPoint pixel_pos = event.GetMousePixelPos();
//         HPoint camera_field_obj, camera_field_pixels;

//         camera_field_obj.x = (0);
//         camera_field_obj.y = (0);        
        
//         //! calculate the size of the current window
//         HC_Compute_Coordinates(".", "object", &camera_field_obj, "local pixels", &camera_field_pixels);

//         //! translate the pixel position
//         pixel_pos.x -= (imageWork->currentViewWidth / 2);
//         pixel_pos.y -= (imageWork->currentViewHeight / 2);
//         imageWork->Translate( (qreal*)(&pixel_pos.x), (qreal*)(&pixel_pos.y));
//         pixel_pos.x += (imageWork->currentViewWidth / 2);
//         pixel_pos.y += (imageWork->currentViewHeight / 2);
  
//         //! if the size of the display is less than the size of the 
//         //! image, we need to shift the pixel coordinates
//         pixel_pos.x += (((imageWork->currentViewWidth / 2) - camera_field_pixels.x)) ;
//         pixel_pos.y += (((imageWork->currentViewHeight / 2) - camera_field_pixels.y));
        
//         printf("pixel:%f,%f\n", pixel_pos.x, pixel_pos.y);

// 	HC_Close_Segment();

    //	m_pView->PrepareForCameraChange();


 	return (HOP_OK);
}



int PMZoom::OnLButtonDownAndMove(HEventInfo &event)
{
	if (!m_bOpStarted) 
		return HBaseOperator::OnLButtonDownAndMove(event);

    if(simple_widget == NULL){
        return HBaseOperator::OnLButtonDownAndMove(event);
    }

	m_ptNew = event.GetMouseWindowPos();

	HC_Open_Segment_By_Key(m_pView->GetSceneKey());
		
    float w, h, zoom_factor = m_ptNew.y - simple_widget->imageWork->mousePosition.y();

    //! redo the mouse position
    simple_widget->imageWork->mousePosition.setX(m_ptNew.x);
	simple_widget->imageWork->mousePosition.setY(m_ptNew.y);

    HPoint target, position, diff, newPos; 
    char proj[64];
 
    HC_Show_Net_Camera_Position(&position.x, &position.y, &position.z);
    HC_Show_Net_Camera_Target(&target.x, &target.y, &target.z);
    HC_Show_Net_Camera_Field(&w, &h);
    HC_Show_Net_Camera_Projection(proj);

    //! zoom the image matrix
    simple_widget->imageWork->ZoomView(zoom_factor, zoom_factor);

    //! update the BDD window
    simple_widget->ddTools->SetWindow();
    
    // if needed, change the view created by the BDD graph
    simple_widget->OnViewChangeWTimer();
 

 //    // we only want to zoom in if we are still further out than
//     // the maximum zoom level and we are actually zooming in
//     float maxZoomLevel = m_pView->GetZoomLimit();
//     if ((w > maxZoomLevel) || (h > maxZoomLevel) || (zoom_factor<0.0f))
//         {
		  

//             diff.Set(position.x - target.x, 
//                      position.y - target.y, 
//                      position.z - target.z);
		  
//             // if zoom_factor greater than 1 than we will zoom past the camera target.  Extra check to avoid unneccessary 
//             // zoom in when the camera is close to target (sign of zoom factor indicates whether trying to zoom in/out) 
//             // #5241, sometimes ACIS users DO want to make zooms that small
//             // if ((HC_Compute_Vector_Length(&diff) > 0.0000001))
//             {
//                 if (zoom_factor < 0.99)
//                     {
//                         newPos.x = m_ptCamera.x + m_ptDist.x * (m_fLength-(m_fLength*zoom_factor));
//                         newPos.y = m_ptCamera.y + m_ptDist.y * (m_fLength-(m_fLength*zoom_factor));
//                         newPos.z = m_ptCamera.z + m_ptDist.z * (m_fLength-(m_fLength*zoom_factor));
                        
//                         HC_Set_Camera_Position(newPos.x, newPos.y, newPos.z);                
//                     }
//                 else
//                     {
//                         newPos.x = position.x;
//                         newPos.y = position.y;
//                         newPos.z = position.z;                        
//                     }
//             }
		  
//             if (streq(proj, "orthographic"))
//                 {
//                     if (!(zoom_factor > 0.99))
//                         {
//                             HC_Set_Camera_Field(m_Width - (m_Width * zoom_factor), 
//                                                 m_Height - (m_Height * zoom_factor));
                            
//                             //! zoom the image
//                             imageWork->ZoomView(zoom_factor, zoom_factor);

//                             if(simple_widget != NULL)
//                                 {
//                                     // if needed, change the view created by the BDD graph
//                                     simple_widget->OnViewChange(newPos.x, newPos.y, newPos.z,
//                                                                 m_Width - (m_Width * zoom_factor),
//                                                                 m_Height - (m_Height * zoom_factor));
//                                 }	
                
//                         }
//                 }
//             else
//                 {

                
//                     if(simple_widget != NULL)
//                         {
//                             // if needed, change the view created by the BDD graph
//                             simple_widget->OnViewChange(newPos.x, newPos.y, newPos.z,
//                                                         w, h);
//                         }	   
//                 }

// 		}

	HC_Close_Segment();

	// tell the view that the camera position has changed
    //	m_pView->CameraPositionChanged();

 	m_pView->Update();
	return (HOP_OK);
}


int PMZoom::OnLButtonUp(HEventInfo &event)
{
	if(!m_bOpStarted) 
		return HBaseOperator::OnLButtonDownAndMove(event);

    //! set the image tools mouse position
    simple_widget->imageWork->mousePosition.setX(0);
	simple_widget->imageWork->mousePosition.setY(0);
    
 	m_bOpStarted = false;
    //	m_pView->CameraPositionChanged(true, true);

    return(HOP_READY);
}


void PMZoom::setWidget(SimpleHQWidget * temp_widget)
{
    simple_widget = temp_widget;
}




