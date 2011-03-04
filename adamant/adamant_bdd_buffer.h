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

#ifndef __ADAMANT_BDD_BUFFER_H__
#define __ADAMANT_BDD_BUFFER_H__

/*
 * Header dependences
 */
#include <glib.h>
#include <tmt.h>
#include <svt.h>
#include <stdlib.h>
#include <cudd.h>
#include <bzlib.h>
#include <zlib.h>
#include "adamant_dd_types.h"

G_BEGIN_DECLS

//! ** Global variables **
    
//! these variables hold onto the BDD that is 
//! used to keep track the tuple members that have
//! already been added to the main BDD
int bdd_buffer_level = 0;
DdNode * bdd_buffer_bdd = NULL;

//! ** Global Functions **
int adamantbdd_buffer_readbuffer(adbdd_buffer * buffer, adbdd_buffer_member * member);
void adamantbdd_buffer_write2read(adbdd_buffer * buffer);
int adamantbdd_buffer_read2write(adbdd_buffer * buffer);
int adamantbdd_buffer_init(adbdd_buffer * buffer, char * buffer_name, char * type);
void adamantbdd_buffer_close(adbdd_buffer * buffer);
void adamantbdd_buffer_addtuple(adbdd_buffer * buffer, guint64 x, guint64 y);
void adamantbdd_buffer_readreset(adbdd_buffer * buffer);
//void adamantbdd_buffer_member_close (adbdd_buffer_member * member);
//adbdd_buffer_member * adamantbdd_new_buffer_member(DdManager * manager);

G_END_DECLS

#endif

