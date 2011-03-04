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

// HQDeleter.cpp - implementation of the HOOPS/Qt class HQDeleter 
// More about this class 
  
  
// HOOPS/Qt includes 
#include "HQDeleter.h" 

// HOOPS/MVO includes 
#include "HTools.h" 
//Added by qt3to4:
#include <QEvent>
#include <QHash>
  

HQDeleter::HQDeleter( QObject * parent, const char * name ) 
 : QObject( parent ) 
{ 
	
	objects = new QHash<int, QObject*>;
} 

HQDeleter::~HQDeleter() 
{ 
	processDeletes();
	delete objects; 
} 
  

void HQDeleter::processDeletes() 
{ 
	
	QHash<int, QObject*>::iterator i = objects->begin();
	while( objects->erase(i) != objects->end() )
	{ 
		++i;
	} 
} 

void HQDeleter::detectOtherDelete() 
{ 
	objects->remove( objects->key(sender()) ); 
} 
  

bool HQDeleter::event( QEvent * e )
{
#if (QT_VERSION >= 200)
	if ( e && e->type() == QEvent::User ) {
#else
	if ( e && e->type() == 12345 ) {
#endif
		processDeletes();
		return true;
	}else{
		return false;
	}
}
  
void HQDeleter::deleteLater( QObject * o )
{
	if ( objects->count() == 0 ) {
#if (QT_VERSION >= 200)
		QEvent * e = new QEvent( QEvent::User );
#else
		QEvent * e = new QEvent( 12345 );
#endif

		QApplication::postEvent( this, e );
	}


	if ( !objects->contains( objects->key(o) ) ) {
		objects->insert( objects->key(o), (QObject*) o);
		connect( o, SIGNAL(destroyed()), this, SLOT(detectOtherDelete()) );
	}
	
}

 

