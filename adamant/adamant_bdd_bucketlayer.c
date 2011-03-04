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

#include "adamant_bdd_layer.h"
#include "adamant_stats.h"
#include <math.h>

// external variables
extern AdamantConfig * global_config;
extern DdNode * bdd_buffer_bdd;
extern int bdd_buffer_level;

// external functions
extern int adamantbdd_buffer_init(adbdd_buffer * buffer, char * buffer_name, char * type);
extern void adamantbdd_buffer_close(adbdd_buffer * buffer);
extern void adamantbdd_buffer_addtuple(adbdd_buffer * buffer, guint64 x, guint64 y);
extern int adamantbdd_buffer_readbuffer(adbdd_buffer * buffer, 
                                        adbdd_buffer_member * member);
extern void adamantbdd_buffer_write2read(adbdd_buffer * buffer);
extern int adamantbdd_buffer_read2write(adbdd_buffer * buffer);
extern void adamantbdd_buffer_readreset(adbdd_buffer * buffer);
extern int adamantbdd_buffer_reset(adbdd_buffer * buffer);
extern DdNode *adamant_bdd_add_tuple(DdManager *manager, 
                                     DdNode * node, 
                                     guint64 x, guint64 y);

//! global variables


/*!
  Function: adamantbdd_bucketlayer_close - wraps up

  This function will close down the bdd layering
  system by adding any leftover material from the
  buffer into the BDD set.  This function also
  performs any needed cleanup operations.

  In this function each layer consists of a variable
  from each tuple member.

  Called with:DdManager * manager - the BDD manager
              DdNode * set - a ref to the BDD set  

 */
int adamantbdd_bucketlayer_close(DdManager * manager, DdNode ** set)
{
  



    return(0);
}


/*  
  This initializes a new bdd bucket layer system toolkit.
  
  Called with: void
  Returns: int - status
  Side effects: sets up many variables used by functions
                in this file.

 */
int adamantbdd_bucketlayer_init(void)
{
  
  
  


  return(return_value);
}



/*
  Function: adamantbdd_bucketlayer_addtuple

  This function adds a tuple to the bdd.

  Called with: DdManager * manager - BDD manager
               DdNode * set - the main BDD set
               guint64 x - x tuple member
               guint64 y - y tuple member

  Returns: int - status
  Side effects: modifies the current bdd

*/
DdNode * adamantbdd_bucketlayer_addtuple(DdManager * manager, DdNode * set, guint64 x, guint64 y)
{
    
    return(set);
}

