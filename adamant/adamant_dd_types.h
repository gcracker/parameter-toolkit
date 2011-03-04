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

#ifndef __ADAMANT_DD_TYPES_H__
#define __ADAMANT_DD_TYPES_H__


G_BEGIN_DECLS

#include <pthread.h>
#include <bzlib.h>
#include <cudd.h>

//! this holds the number of BDD
//! tuple members
#define BDD_MEMBERS 2
#define ZDDNUM 128

typedef struct ddpt {
  guint64 X;
  guint64 Y;
} ddpoint;

typedef struct frg {
  ddpoint topLeft;
  ddpoint bottomRight;
  gint64 regionID;
  gint64 parallelCount;
} ddregion;

typedef struct adamantHotBuffer {
  guint64 din_;
  guint64 sin_;
} adamantHotBuffer;

typedef struct adamantHotManager {
  GHashTable * hot_code_hash_;
  BZFILE * hotbuffer_bz2_ptr_;
  FILE * hotbuffer_bz2_file_ptr_;
  gchar * hot_buffer_file_name;
  gint hot_buffer_fd;
  
  guint64 top_hot_value_;
  guint64 bottom_hot_value_;
  
  guint64 top_hot_sin_;
  guint64 bottom_hot_sin_;

} adamantHotManager;


//! the BDD simple member type
typedef struct adbdd_member{
    guint64 data[BDD_MEMBERS];
} adbdd_member;

//! the BDD buffer member type
typedef struct adbdd_buffer_member{
    struct adbdd_buffer_member * next;
    guint64 data[BDD_MEMBERS];
} adbdd_buffer_member;

//! the layer BDD buffer recursive type
typedef struct adbdd_layer_recur{
    DdNode * node;
    DdManager * manager;
    int num_complement;
    guint64 x;
    guint64 y;
} adbdd_layer_recur;

//! the BDD buffer type
typedef struct{
    char * type;
    DdManager * manager;
    DdNode * BDD;
    adbdd_buffer_member * top;
    guint64 bit;
    GString * buffer_file_name;
    BZFILE * buffer_bz2_ptr;
    FILE * buffer_bz2_file_ptr;
} adbdd_buffer;

struct t_thread_args {
    guint64 * x;
    guint64 * y;    
    DdNode * threadNode;
    DdManager * threadManager;
    pthread_mutex_t * threadMutex;
    pthread_mutex_t * holdMutex;
    int * stayalive;
};

typedef struct t_thread_args thread_args;


enum{
    X_MEM = 0,
    Y_MEM = 1
};

//! Not a type!  However, it is redefined
//! all over the place. So putting 
//! it here is cleaner
static guint64 mask(guint64 v) {
    return ((guint64)(1) << v);
}

G_END_DECLS

#endif

