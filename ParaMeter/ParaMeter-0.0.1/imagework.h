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

// imagework.h
// 
// More about this class 
  
#ifndef IMAGEWORK_H 
#define IMAGEWORK_H 

#include <glib.h>

// STL things
#include <vector>
#include <list>
#include <map>

// HOOPS/Qt Includes 
#include "HQWidget.h" 
#include <qpoint.h>

// CUDD Includes
#include <cudd.h>
//#include "cuddInt.h"
#include "dddmp.h"  
#include <math.h>

using namespace std;

//! forward class declarations
class ImageWork;
class QMatrix;


/*!
  The ImageWork Class
 */
class ImageWork {

 public:

    //  *************************
    //! ** function prototypes **
    //  *************************

    //! constructor(s)
    ImageWork();
    
    //! destructor
    ~ImageWork();

    //! Set the view buffer size
    void SetBufferSize(quint32 width, quint32 height);

    //! Initialize the view buffer
    void init_view_buffer(quint8 * temp_vbuffer);

    //! Get the next view buffer
    quint8 * getNextViewBuffer();

    //! view translation functions
    void PanView(qreal x, qreal y);    
    void ZoomView(qreal x, qreal y);    
    void Translate(qreal xO, qreal yO, qreal * nX, qreal * nY);
    void Translate(quint64 xO, quint64 yO, quint64 * nX, quint64 * nY);
    void Translate(quint64 * X, quint64 * Y);
    void Translate(qreal * X, qreal * Y);
    bool InvTranslate(qreal * X, qreal * Y);

    //! point translation functions
    int Plot2Pixel(quint64 plotX, quint64 plotY, 
                   qint64 * pixX, qint64 * pixY);
    int Pixel2Plot(quint64 pixX, quint64 pixY, 
                   quint64 * plotX, quint64 * plotY);
    
    //! region selection functions
    void addSelection(QPointF top_left, QPointF bottom_right);
    void clearSelection();    
    int MakeSelectedImage(quint8 * image);
    int unmangleSelectedPoints(QPointF * top_left, QPointF * bottom_right);
    int getSelection(int selectionNumber, QPointF * top_left, QPointF * bottom_right);
    void insertImageSelections(quint8 * image);
    void makeSelectedImagePoints(quint8 * image, 
				 QPointF topLeftSel, 
				 QPointF topRightSel); 
    bool isSelectionEmpty(void);
    int getSelectionCount(void);

    void saveViewBuffer(QString fileName);
    void saveSelectedRegions(const char * fileName);
    void addSelectedRegions(const char * fileName);

    //  *****************
    //! *** variables ***
    //  *****************
    int pixel_group_size;

    //! keep track of the current position
    QPointF currentViewPosition;
    QPointF mousePosition;
    
    //! keep track of the current Z position
    qreal currentZPosition; 
    qreal zoomFudgeFactor;
    qreal zoomFactor;

    //! keep track of coordinate space
    //! sizes and translations
    qreal startViewWidth;
    qreal startViewHeight;
    quint32 currentBufferWidth;
    quint32 currentBufferHeight;
    //    qreal currentViewWidth;
    //    qreal currentViewHeight;
    QMatrix transMatrix;
    qint64 current_imagekey;
    QPointF pixWorldConvert;

    //! keep track of selected regions
    QList<QRectF> selectedRegions_;

    //! view buffer variables
    quint32 buffer_pixel_size;
    quint8 * vbuffer[2];
    quint32 cur_vbuffer;
    quint32 cur_vbuffer_mod; 
    quint8 * view_buffer;

    const quint32 redPos;
    const quint32 greenPos;
    const quint32 bluePos;
    const quint32 alphaPos;
};

#endif 


