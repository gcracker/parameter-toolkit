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

// imagework.cpp - Implementation of various graphing functions
// 
// More about this class 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <QMatrix>
#include <QTextStream>
#include "imagework.h"

#define RES_ADD 10
#define DEFAULT_RES 13
#define BDDNUM 128
#define lsb(n)		((n)&1 ? one : zero)
#define lsbNot(n)	((n)&1 ? zero : one)
#define VIEW_BUFFER

extern guint64 x_scale;
extern guint64 y_scale;
extern guint64 x_start;
extern guint64 x_stop;
extern guint64 y_start;

using namespace std;

ImageWork::ImageWork():
  pixel_group_size(0),
  zoomFudgeFactor(1),
  currentBufferWidth(600),
  currentBufferHeight(600),
  current_imagekey(-1),
  buffer_pixel_size(4),
  cur_vbuffer(0),
  cur_vbuffer_mod(2),
  redPos(0),
  greenPos(1),
  bluePos(2),
  alphaPos(3)
{ 
    //! iterate and setup view buffers
    for(quint32 i = 0; i < cur_vbuffer_mod; i++){
            
        vbuffer[i] = new quint8[currentBufferWidth * currentBufferHeight * buffer_pixel_size];
       
        init_view_buffer(vbuffer[i]);
    }

    //! set the starting view buffer
    view_buffer = vbuffer[cur_vbuffer];

    //! initialize the translation matrix
    //! (m11 = hor scale, m12 = hor shear, m21 = vert shear, m22 = ver scale, dx,dy)
    transMatrix.setMatrix(1,0,0,1,0,0);

    //! initialize selection variables
    clearSelection();
}   


// Destructor  
ImageWork::~ImageWork()
{ 

    //! **** destroy view buffer work ****
    for(quint32 i = 0; i < cur_vbuffer_mod; i++){
        delete [] vbuffer[i];
    }
}
/*
  getNextViewBuffer - returns a pointer to the next buffer
  
  This function resets the width and height of the view buffers.

  Called with:
                
  Returns: void

  Side Effects:

 */
quint8 * ImageWork::getNextViewBuffer()
{
  //! increment the view buffer
  cur_vbuffer = (cur_vbuffer + 1) % cur_vbuffer_mod;
  view_buffer = vbuffer[cur_vbuffer];
  
  //! initialize the buffer
  init_view_buffer(view_buffer);

  return(view_buffer);
}



/*
  SetBufferSize - This function changes the view buffer size
  
  This function resets the width and height of the view buffers.

  Called with: quint32 width - the new width
                quint32 height - the new height
                
  Returns: void

  Side Effects: Resizes and clears all known view buffers

 */
void ImageWork::SetBufferSize(quint32 width, quint32 height){

    //! sets the new height and width
    currentBufferHeight = height;
    currentBufferWidth = width;

    for (unsigned int i = 0; i < cur_vbuffer_mod; i++){

        //! delete the old view buffer
        delete [] vbuffer[i];
        
        //! allocate memory for the new view buffer
        vbuffer[i] = new quint8[currentBufferWidth * currentBufferHeight * buffer_pixel_size];

        //! initialize the new view buffer
        init_view_buffer(vbuffer[i]);
    }
}


/*!
 * initialize the view buffer
 */
void ImageWork::init_view_buffer(quint8 * temp_vbuffer)
{
    //! set all of the buffer's data to white
    for(quint32 i = 0; 
        i < (currentBufferHeight * currentBufferWidth * buffer_pixel_size); 
        i++)

        {
//             //! DEBUG
//             if( (i % 50) == 0 ){
//                 temp_vbuffer[i] = (quint8)(0x00); 
//             }
//             else{
//                 temp_vbuffer[i] = (quint8)(0xff); 
//             }
            

            temp_vbuffer[i] = (quint8)(0xff);           
        }
}


/*
  Function: PanView - pans the 2D view

  This function alters the point translation 
  matrix to include pan movement.  Note that
  this is the CHANGE in x and y, not the 
  absolute positions.

  Called With: qreal x - the x translation
               qreal y - the y translation
               
  Returns: nada
  Side Effects: modifys the class's translation matrix.
  
 */
void ImageWork::PanView(qreal x, qreal y){

    //! translate the matrix
    transMatrix.translate(x, y);
}


/*
  Function: ZoomView - zooms the 2D view

  This function alters the point translation 
  matrix to include zoom movement. This 
  is performed with a relative scaling.

  Called With: qreal x - the x scaling
               qreal y - the y scaling
               
  Returns: nada
  Side Effects: modifies the class's translation matrix.
  
 */
void ImageWork::ZoomView(qreal x, qreal y){



    //! DEBUG
    //    printf("Zoom X:%f\n", 1 + x);
    //    printf("Zoom Y:%f\n", 1 + y);

    //! scale the matrix
    transMatrix.scale( 1 + x, 1 + y);
}


/*
  Function: Translate - maps points to ImageWork coordiantes

  This function translates points according to
  the in-house translation matrix.

  Called With: qreal xO - the old x value
               qreal yO - the old y value
               qreal * xN - a pointer to the new X value
               qreal * yN - a pointer to the new X value
               
  Returns: nada
  Side Effects: none
  
 */
void ImageWork::Translate(qreal xO, qreal yO, qreal * nX, qreal * nY){

    //! translate the matrix
    transMatrix.map(xO, yO, nX, nY);
}


//! version for quint64 bit numbers
void ImageWork::Translate(quint64 xO, quint64 yO, quint64 * nX, quint64 * nY){

    qreal temp_nX, temp_nY;

    //! translate the matrix
    transMatrix.map((qreal)xO, (qreal)yO, &temp_nX, &temp_nY);

    //! return the translated values
    *nX = (quint64)(temp_nX);
    *nY = (quint64)(temp_nY);

}


//! version for using a destroyable set of numbers
void ImageWork::Translate(quint64 * X, quint64 * Y){

    qreal temp_nX, temp_nY;

    //! translate the matrix
    transMatrix.map((qreal)(*X), (qreal)(*Y), &temp_nX, &temp_nY);

    //! return the translated values
    *X = (quint64)(temp_nX);
    *Y = (quint64)(temp_nY);
}


//! version for using a destroyable set of reals
void ImageWork::Translate(qreal * X, qreal * Y){

    qreal temp_nX, temp_nY;

    //! translate the matrix
    transMatrix.map(*X, *Y, &temp_nX, &temp_nY);

   //! return the translated values
    *X = temp_nX;
    *Y = temp_nY;
}


//! version for using a destroyable set of reals
bool ImageWork::InvTranslate(qreal * X, qreal * Y){

    qreal temp_nX, temp_nY;
    bool invertable = false;
    
    //! see if we can invert this matrix
    QMatrix temp_inv = transMatrix.inverted (&invertable);
    if(invertable == true){
        
        //! translate the matrix
        temp_inv.map(*X, *Y, &temp_nX, &temp_nY);
        
        //! return the translated values
        *X = temp_nX;
        *Y = temp_nY;
    }

    return(invertable);
}


/* 
   Function: addSelection - Adds a selection that is graphed

   Called with: QPointF top_left - top left selected point
                QPointF bottom_right - bottom right selected point

   Returns:

 */
void ImageWork::addSelection(QPointF top_left, QPointF bottom_right)
{
  selectedRegions_.append(QRectF(top_left, bottom_right));
}


/* 
   Function: GetSelection - Sets the selection that is graphed

   Called with: QPointF top_left - top left selected point
                QPointF bottom_right - bottom right selected point

   Returns: bool - return status   

 */
int ImageWork::getSelection(int selNum, QPointF * top_left, QPointF * bottom_right)
{

  if(selectedRegions_.count() <= selNum)
    {
      return(1);
    }
  
  QRectF tSelRect = selectedRegions_.at(selNum);
  
  *top_left = tSelRect.topLeft();
  *bottom_right = tSelRect.bottomRight();
    
  return(0);
}



/* 
   Function: clearSelection

   Called with: void

   Returns: void

 */
void ImageWork::clearSelection(void)
{
    //! clear the selected points
    selectedRegions_.clear();
}

/* 
   Function: getSelectionCount

   Called with: void

   Returns: int - return count   

 */
int ImageWork::getSelectionCount(void)
{
    return(selectedRegions_.count());
}


/* 
   Function: isSelectionEmpty

   Called with: void

   Returns: int - return count   

 */
bool ImageWork::isSelectionEmpty(void)
{

    return(selectedRegions_.empty());
}


/*
  Function: insertImageSelections - color selected pixels

  If a selected region exists this function will
  highlight any points in the selected region

  Called with: quint8 * image - the image to modify
  
  Returns: return code

  Side effects: none
 */
void ImageWork::insertImageSelections(quint8 * image)
{
  if(NULL == image)
    {
      return;
    }

  for (int i = 0; i < selectedRegions_.size(); ++i)
    {
      QRectF tSelectedRect = selectedRegions_.at(i);
      makeSelectedImagePoints(image, tSelectedRect.topLeft(), tSelectedRect.bottomRight());
    }
}

/*
  Function: saveSelected - color selected pixels

  If a selected region exists this function will
  highlight any points in the selected region

  Called with: 
  
  Returns: return code

  Side effects: none
 */
void ImageWork::saveViewBuffer(QString fileName)
{
  QImage lImage = QImage(view_buffer, currentBufferWidth, currentBufferHeight, 
			 QImage::Format_ARGB32);
  QImage bgraImage = lImage.rgbSwapped();
  bgraImage.save(fileName);
}

/*
  Function: saveSelectedRegions - 

  Called with: 
  
  Returns: return code

  Side effects: none
 */
void ImageWork::saveSelectedRegions(const char * fileName)
{
  FILE * fh = fopen(fileName, "w");
  fprintf(fh, "VERSION: %d\n", 1);
  fprintf(fh, "%s\n", fileName);
  
  int selectionCount = getSelectionCount();
  
  while (0 < selectionCount )
    {
      QPointF top_left;
      QPointF bottom_right;
      getSelection((selectionCount - 1), &top_left, &bottom_right);
      fprintf(fh, "REGSTART: %d\n",
		 (selectionCount - 1));      
      fprintf(fh, "TL: %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT"\n",
		 (guint64)top_left.x(),
		 (guint64)top_left.y());
      fprintf(fh, "BR: %"G_GUINT64_FORMAT" %"G_GUINT64_FORMAT"\n",
		(guint64)bottom_right.x(),
		(guint64)bottom_right.y());
      fprintf(fh, "REGEND: %d\n",
		(selectionCount - 1));
     --selectionCount;
    }
  
  fclose(fh);
}

/*
  Function: saveSelectedRegions - 

  Called with: 
  
  Returns: return code

  Side effects: none
 */
void ImageWork::addSelectedRegions(const char * fileName)
{
  
  FILE * fh = fopen(fileName, "r");
  QTextStream inStream (fh);
  QPointF top_left;
  QPointF bottom_right;

  while (!inStream.atEnd())
    {
      QString line = inStream.readLine();

      if(line.contains("VERSION",
		       Qt::CaseInsensitive))
	{

	}
      else if(line.contains("REGSTART",
		       Qt::CaseInsensitive))
	{
	  top_left.setX(0);
	  top_left.setY(0);
	  bottom_right.setX(0);
	  bottom_right.setY(0);
	}
      else if(line.contains("REGEND",
		       Qt::CaseInsensitive))
	{
	  
	  addSelection(top_left, bottom_right);

	  top_left.setX(0);
	  top_left.setY(0);
	  bottom_right.setX(0);
	  bottom_right.setY(0);
	}
      else if(line.contains("TL",
			    Qt::CaseInsensitive))
	{
	  QStringList lineParts = line.split(" ");
	  
	  if(3 == lineParts.count())
	    {
	      top_left.setX(lineParts[1].toDouble());
	      top_left.setY(lineParts[2].toDouble());
	    }
	}      
      else if(line.contains("BR",
			    Qt::CaseInsensitive))
	{
	  QStringList lineParts = line.split(" ");
	  
	  if(3 == lineParts.count())
	    {
	      bottom_right.setX(lineParts[1].toDouble());
	      bottom_right.setY(lineParts[2].toDouble());
	    }
	}
    }

  // QPointF top_left;
  // QPointF bottom_right;
  // fprintf(fh, "TL: %f %f\n",
  // 	  top_left.x(),
  // 	  top_left.y());
  // fprintf(fh, "BR: %f %f\n",
  // 	  bottom_right.x(),
  // 	  bottom_right.y());
  
  fclose(fh);
}

/*
  Function: MakeSelectedImage - color selected pixels

  If a selected region exists this function will
  highlight any points in the selected region

  Called with: quint8 * image - the image to modify
  
  Returns: return code

  Side effects: none
 */
void ImageWork::makeSelectedImagePoints(quint8 * image, QPointF topLeftSel, QPointF bottomRightSel)
{
    qint64 TLx = 0, TLy = 0, BRx = 0, BRy = 0;

    //! convert the points to pixels
    Plot2Pixel((quint64)topLeftSel.x(), (quint64)topLeftSel.y(),
               &TLx, &TLy);
    Plot2Pixel((quint64)bottomRightSel.x(), (quint64) bottomRightSel.y(),
               &BRx, &BRy);

    //! check to make sure we do not select out of bounds
    if(BRy > currentBufferHeight){
        BRy = currentBufferHeight;
    }
    if(BRx > currentBufferWidth){
        BRx = currentBufferWidth;
    }
    if(TLy < 0){
       TLy = 0;
    }
    if(TLx < 0){
        TLx = 0;
    }

    //! scan through that region in the image
    for (quint32 ynum = TLy; ynum < BRy; ynum++)
      {
        for (quint32 xnum = TLx; xnum < BRx; xnum++)
	  {           
            //! calculate the pixel position in the array
            quint32 position = (ynum*currentBufferWidth*buffer_pixel_size)+
	      (xnum*buffer_pixel_size);

	    
	    if((0 == image[position + redPos]) ||
	       (0 == image[position + bluePos]) ||
	       (0 == image[position + greenPos]))
	      {	    	 
		image[position + redPos] = 0x00;
		image[position + greenPos] = 0x00;
		image[position + bluePos] = 0xff;
	      }
	  }
      }
}


/* 
   Function: unmangleSelectedPoints - corrects selected points

   Called with: QPointF top_right - top right selected point
                QPointF bottom_left - bottom left selected point

   Returns: int - return status
   Side effects - none  

 */
int ImageWork::unmangleSelectedPoints(QPointF * top_left, QPointF * bottom_right)
{
    //! first check the x values
    if(top_left->x() > bottom_right->x()){

        QPointF temp_topleft(*top_left);
        
        // swap x values
        top_left->setX(bottom_right->x());
        bottom_right->setX(temp_topleft.x());        
    }

    //! check the y values
    if(top_left->y() < bottom_right->y()){

        QPointF temp_topleft(*top_left);
        
        // swap x values
        top_left->setY(bottom_right->y());
        bottom_right->setY(temp_topleft.y());        
    }

    return(0);
}


/*
  Function: Plot2Pixel - Convert from plot coord to pixel coord

  This function will convert from plot coordinates (the size of the
  entire plot) to pixel coordinates (the pixel to be rendered in the
  view buffer).

  Called with: guint64 plotX - the plot x coordinates
               guint64 plotY - the plot y coordinates
  Returns:     guint64 * pixX - the pixel x coordinates
               guint64 * pixY - the pixel y coordinates
               int - 1 if successful, 0 if not

 */
int ImageWork::Plot2Pixel(quint64 plotX, quint64 plotY, 
                        qint64 * pixX, qint64 * pixY)
{
    int return_value = 1;   
    
    qreal tempX, tempY;

    //! make sure the numbers are initialized
    if((pixX == NULL) || (pixY == NULL)){
        return (0);        
    }    

    //! compute the center of the mark for this RDY
    tempX = ((qreal)plotX - (qreal)x_start) / ((qreal)x_scale);

    //! compute the center of the mark for this DIN
    tempY = (((qreal)plotY - (qreal)y_start) / ((qreal)y_scale));

    //! invert the Y axis
    tempY = (qreal)currentBufferHeight - tempY;
    
    //! move quadrants
    tempX -= (currentBufferWidth / 2);
    tempY -= (currentBufferHeight / 2);
 
    //! apply the ImageWork translation matrix
    Translate(&tempX, &tempY);

    //! move back to the original quadrants
    tempX += (currentBufferWidth / 2);
    tempY += (currentBufferHeight / 2);

    //! now round off to an int
    *pixX = (qint64)qRound64(tempX);
    *pixY = (qint64)qRound64(tempY);

    return (return_value);
}


/*
  Function: Pixel2Plot - Convert from pixel coord to plot coord

  This function will convert from pixel coordinates to plot coordinates.

  Called with: guint64 pixX - the plot x coordinates
               guint64 pixY - the plot y coordinates
  Returns:     guint64 * plotX - the pixel x coordinates
               guint64 * plotY - the pixel y coordinates
               int - 1 if successful, 0 if not

 */
int ImageWork::Pixel2Plot(quint64 pixX, quint64 pixY, 
                        quint64 * plotX, quint64 * plotY)
{
    int return_value = 1;   
    
    qreal tempX = (qreal)pixX;
    qreal tempY = (qreal)pixY;

    //! make sure the numbers are initialized
    if((plotX == NULL) || (plotY == NULL)){
        return (0);        
    }    

    //! move quadrants
    tempX -= (currentBufferWidth / 2);
    tempY -= (currentBufferHeight / 2);
 
    //! apply the ImageWork translation matrix
    InvTranslate(&tempX, &tempY);

    //! move back to the original quadrants
    tempX += (currentBufferWidth / 2);
    tempY += (currentBufferHeight / 2);

    //! compute the center of the mark for this RDY
    tempX = (tempX * (qreal)x_scale) + (qreal)x_start;

    //! invert this DIN, scale, add offset
    tempY = ((((qreal)currentBufferHeight - tempY) * 
              (qreal)y_scale) + (qreal)y_start);


    //! now round off to an int
    *plotX = (quint64)qRound64(tempX);
    *plotY = (quint64)qRound64(tempY);

    return (return_value);
}


