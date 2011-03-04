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

#ifndef __ADAMANT_BDD_LAYER_H__
#define __ADAMANT_BDD_LAYER_H__

/*
 * Header dependences
 */
#include <glib.h>
#include <tmt.h>
#include <svt.h>
#include <stdlib.h>
#include <cudd.h>
#include "adamant_dd_types.h"

G_BEGIN_DECLS

//! this holds the number of BDD
//! tuple members
#define BDD_MEMBERS 2

//! ** Global variables **
DdNode ** topNodes = NULL;
DdNode ** newTopNodes = NULL;
guint64 topNodeNum = 10;
guint64 topNodeNum_sizebuffer = 10;
GHashTable * bdd_layer_hash;

//! variables that deal with the number
//! of layers to add in per pass
guint32 startLayers = 1;
guint32 incLayers = 1;


//! ** Global Functions **
DdNode * adamantbdd_layer_addtuple(DdManager * manager, DdNode * set, guint64 x, guint64 y);
DdNode * adamantbdd_layer_addtuple_wb(DdManager * manager, DdNode * set, guint64 x, guint64 y);
int adamantbdd_layer_init(void);
int adamantbdd_layer_testandsort(DdManager * manager, DdNode ** set, guint64 x, guint64 y);
DdNode * adamantbdd_layer_findtuple(adbdd_layer_recur * member);
DdNode * adamantbdd_layer_findaddtuple(DdManager * manager, DdNode * node, guint64 x, guint64 y);



G_END_DECLS

#endif

