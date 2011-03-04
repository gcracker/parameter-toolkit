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

/*
  The BDD sort routine should perform a specific sort
  on the input data.
 */

#include "adamant_bdd_sort.h"
#include "adamant_dd_types.h"
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
  Function: adamantbdd_sort_close - wraps up

  This function will close down the bdd layering
  system by adding any leftover material from the
  buffer into the BDD set.  This function also
  performs any needed cleanup operations.

  In this function each layer consists of a variable
  from each tuple member.

  Called with:DdManager * manager - the BDD manager
              DdNode * set - a ref to the BDD set  

 */
int adamantbdd_sort_close(DdManager * manager, DdNode ** set)
{
  



    return(0);
}


/*!
  Function: adamantbdd_hash_close - wraps up

  This function will close down the hash by iterating
  over the hash, closing bdd buffers, and killing
  the hash.

  Called with: gpointer * key - the hash member key
               gpointer * value - the hash member value
               gpointer * user_data - the hash member user data

 */
void adamantbdd_hash_close(gpointer key, gpointer value, gpointer user_data)
{
  
  //! get the buffer pointer
  adbdd_buffer * buffer = (adbdd_buffer *)(value);
}


/*  
  This initializes a new bdd bucket layer system toolkit.
  
  Called with: void
  Returns: int - status
  Side effects: sets up many variables used by functions
                in this file.

 */
int adamantbdd_sort_init(void)
{
  int return_value = 0;

  g_weightgradient = 8;

  //! initialize the buffer hash
  //! this associates a Hamming distance
  //! to a buffer.
  g_bufferHash = g_hash_table_new(g_int_hash, g_int_equal);  

  return(return_value);
}



/*
  Function: adamantbdd_sort_addtuple

  This function adds a tuple to the bdd.

  Called with: DdManager * manager - BDD manager
               DdNode * set - the main BDD set
               guint64 x - x tuple member
               guint64 y - y tuple member

  Returns: int - status
  Side effects: modifies the current bdd

*/
int adamantbdd_sort_addtuple(guint64 x, guint64 y)
{
  adbdd_member a, b;
  GString * filename = NULL;
  GString * tempdir = g_string_new( "/tmp");

  // put the comparison tuple into the bdd holder
  a.data[0] = 0;
  a.data[1] = 0;

  // put the given tuple into a temporary bdd holder 
  b.data[0] = x;
  b.data[1] = y;

  // get the weighted Hamming distance 
  guint t_hamming_dist = adamantbdd_sort_hammingdist(a, b);

  //! Look for the buffer holding other tuples of this
  //! distance.  If it does not exist, make a new one.
  adbdd_buffer * buffer = g_hash_table_lookup(g_bufferHash, &t_hamming_dist);

  if (buffer == NULL)
    {
      //! set up the temp directory
      g_string_printf(filename, "%s%d", tempdir->str, t_hamming_dist);
      
      //! get memory for the buffer structure
      buffer = malloc(sizeof(adbdd_buffer));

      //! initialize the bdd buffer
      adamantbdd_buffer_init(buffer, filename->str, "unknown");
    }

  //! we should now have a functional bdd buffer
  //! so add the tuple to the buffer
  //! we will need to clean up the buffer and close it down
  //! later.
  adamantbdd_buffer_addtuple( buffer, x, y);

  return(0);
}


/*
  Function: adamantbdd_sort_hammingdist

  This funtion returns the Hamming distance
  of two BDD structs.  This function must
  be aware of all the members of the BDD struct
  in order to produce the compare.  This function
  should also stay very fast and lean.

  Currently this uses a linear weight gradient.

  Called with: adbdd_member * bdds
               int region

  Returns: String - status
  Side effects: modifies the current bdd


 */
int adamantbdd_sort_hammingdist(adbdd_member a, adbdd_member b)
{
  int total_dist = 0;
  int n_members = 2;
  guint64 memberdiff[n_members];
  guint adj_weightspan = (guint)(64 / g_weightgradient);
  guint64 weightmask = (mask(adj_weightspan) - 1);

  //! take the XOR of each struct member
  memberdiff[0] = (a.data[0] ^ b.data[0]);
  memberdiff[1] = (a.data[1] ^ b.data[1]);

  int i = 0;
  
  for (i = 0; i < n_members; i++)
    {

      int count = 0;

      //! right now this uses the slowest
      //! populatation count method.  Using the 
      //! Kernighan & Ritchie method would be faster
      //! but it is not easy to incorporate weights
      //! into that algorithm
      while(memberdiff[i] != 0)
	{
	  	  
	  //! give the distance a weight and add it to the total distance
	  //! for this population count
	  total_dist += (memberdiff[i] & 1)?(int)(floor((64 - count) / (adj_weightspan))):0; 

	  //! shift bits one place right
	  memberdiff[i] >>= 1;
	  
	  //! increment the current bit position (zero based)
	  count++;
	} 
    }

  return(total_dist);
}


