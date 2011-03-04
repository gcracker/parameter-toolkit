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
adbdd_buffer * tuple_buffer_0;
adbdd_buffer * tuple_buffer_1;
guint64 bdd_buffer_mask = 0;

int oldDead = 0; //!! DEBUG


/*!
  Function: adamantbdd_layer_close - wraps up

  This function will close down the bdd layering
  system by adding any leftover material from the
  buffer into the BDD set.  This function also
  performs any needed cleanup operations.

  In this function each layer consists of a variable
  from each tuple member.

  Called with:DdManager * manager - the BDD manager
              DdNode * set - a ref to the BDD set  

 */
int adamantbdd_layer_close(DdManager * manager, DdNode ** set)
{
    //! make things a little easier
    //! and make a copy of the BDD
    //! set pointer
    DdNode * tempset = *set;
    int done = 0;  
    adbdd_buffer_member temp_member;

    //! convert the write buffer to a read buffer
    adamantbdd_buffer_write2read(tuple_buffer_0);
    
    //! initialize the buffer layer level
    bdd_buffer_level += incLayers;

    //! initilize the buffer layer mask
    //
    //! NOTE!! The bdd_buffer_mask is no
    //! longer a real mask
    bdd_buffer_mask = pow(2, bdd_buffer_level);

    //! now add in the remainder of the buffers
    while(bdd_buffer_level < 64)
        {
            
            while(adamantbdd_buffer_readbuffer(tuple_buffer_0, &temp_member))
                {

                    *set = adamantbdd_layer_findaddtuple(manager, *set,
                                                        temp_member.data[X_MEM],
                                                        temp_member.data[Y_MEM]);

/*                     //! test the tuple members and then perform */
/*                     //! a sort into either a buffer or a BDD */
/*                     adamantbdd_layer_testandsort(manager, set, */
/*                                                  temp_member.data[X_MEM], */
/*                                                  temp_member.data[Y_MEM]); */

                }
            
            //! increment the buffer layer level
            bdd_buffer_level += incLayers;

            //! update the buffer mask
            bdd_buffer_mask = pow(2, bdd_buffer_level);

            //! close and reopen tuple buffer 0 in write mode
            adamantbdd_buffer_read2write(tuple_buffer_0);

            //! close tuple buffer 1 and reopen in read mode
            adamantbdd_buffer_write2read(tuple_buffer_1);

            //! swap the tuple buffer pointers
            adbdd_buffer * temp_tuple_buffer = tuple_buffer_0;
            tuple_buffer_0 = tuple_buffer_1;
            tuple_buffer_1 = temp_tuple_buffer;            
        }
    
    //! close the tuple buffers
    adamantbdd_buffer_close(tuple_buffer_0);
    adamantbdd_buffer_close(tuple_buffer_1);

    return(0);
}


/*!
  Function: adamantbdd_layer_testandsort

  This function tests the size of the over tuple
  and then performs the appropriate layering

  Called with: 
              DdManager * manager - bdd manager
              DdNode ** set - the master set
              guint64 x - the first tuple member
              guint64 y - the second tuple member

  Returns:
          int - status

 */
int adamantbdd_layer_testandsort(DdManager * manager, DdNode ** set, guint64 x, guint64 y)
{
    //! determine the BDD hash key of this tuple
    guint hash_key = (x < y) ? (guint)(ceil(log2(y))) : (guint)(ceil(log2(x)));
    
    //! get the BDD node for this tuple out of the hash
    DdNode * hash_bdd = (DdNode*)(g_hash_table_lookup(bdd_layer_hash, (gpointer)(&hash_key)));
    
    //! check for an empty return BDD
    if(hash_bdd == NULL)
        {
            hash_bdd = Cudd_ReadOne(manager);
        }                                                              

    //! check to see if a tuple member should be added */
    //! to the BDD at this time
    if (( x < bdd_buffer_mask ) && ( y < bdd_buffer_mask ))
        {
            //! add this tuple into the set BDD
            DdNode * tempNode = adamantbdd_layer_addtuple(manager, hash_bdd, x, y);            

            //! dereference the old set
            Cudd_IterDerefBdd(manager, hash_bdd);

            //! now replace the bdd for this hash entry
            hash_bdd = tempNode;

            //! add this tuple to the buffer for storage
            adamantbdd_buffer_addtuple(tuple_buffer_1, x, y);
        }
    else
        {
            //! add this tuple to the buffer for storage
            adamantbdd_buffer_addtuple(tuple_buffer_1, x, y);
        }

    //! insert the new BDD pointer into the hash
    g_hash_table_insert(bdd_layer_hash, (gpointer)(&hash_key), (gpointer)(hash_bdd));


  /*   //! check to see if a tuple member should be added */
/*     //! to the BDD at this time */
/*     if (( x < bdd_buffer_mask ) && ( y < bdd_buffer_mask )) */
/*         { */
/*             //! add this tuple into the set BDD */
/*             DdNode * tempNode = adamant_bdd_add_tuple(manager, *set, x, y); */
            
/*             //! dereference the old set */
/*             Cudd_IterDerefBdd(manager, *set); */

/*             //! now replace the set */
/*             *set = tempNode;                             */
/*         } */
/*     else */
/*         { */
/*             //! add this tuple to the buffer for storage */
/*             adamantbdd_buffer_addtuple(tuple_buffer_1, x, y); */
/*         } */
    
    return (0);
}


/*!
  Function: adamantbdd_layer_new - creates a new bdd layer tool kit
  
  This initializes a new bdd layer system toolkit.
  
  Called with: void
  Returns: int - status
  Side effects: sets up many variables used by functions
                in this file.

 */
int adamantbdd_layer_init(void){
    int return_value = 1;

    //! make sure the new buffer BDD is initialized
    if(tuple_buffer_0 == NULL)
        {
            //! make a new bdd buffer if needed
            if(tuple_buffer_0 == NULL)
                {
                    tuple_buffer_0 = g_new(adbdd_buffer, 1);
                }

            assert((adamantbdd_buffer_init(tuple_buffer_0, 
                                           "/tmp/test01.bz2", 
                                           "DINRDY")) == 0);
        }
    
    //! make sure the second BDD buffer is initialized
    if(tuple_buffer_1 == NULL)
        {
            //! make a new bdd buffer if needed
            if(tuple_buffer_1 == NULL)
                {
                    tuple_buffer_1 = g_new(adbdd_buffer, 1);
                }
            
            assert((adamantbdd_buffer_init(tuple_buffer_1, 
                                           "/tmp/test02.bz2", 
                                           "DINRDY")) == 0);
        }

    //! initialize the number of layers to add in
    //! during the first pass
    bdd_buffer_level = startLayers;

    //! initilize the buffer layer mask
    bdd_buffer_mask = pow(2, bdd_buffer_level);

    //    topNodes = g_new((DdNode*), topNodeNum_sizebuffer);
    //    newTopNodes = g_new((DdNode*), topNodeNum_sizebuffer);

    //! initialzie the bdd hash table
    bdd_layer_hash = g_hash_table_new(g_int_hash, 
                                      g_int_equal);
    
    return(return_value);
}



/*
  Function: adamantbdd_layer_addtuple

  This function adds a tuple to the bdd.

  Called with: DdManager * manager - BDD manager
               DdNode * set - the main BDD set
               guint64 x - x tuple member
               guint64 y - y tuple member

  Returns: int - status
  Side effects: modifies the current bdd

*/
DdNode * adamantbdd_layer_addtuple(DdManager * manager, DdNode * set, guint64 x, guint64 y)
{
    guint64 bitset;
    int v;
    int i, j;
	DdNode * tmp, * retnode, * itenode, * logicZero;

    //! create a BDD that contains the
    //! tuple up to and including
    //! the current buffer level
    //! 
    //! garbage collection should be minimal
    //! because the BDD should already exist
    //! in the CUDD unique table for the
    //! tuple up to the current buffer level
	
	logicZero = Cudd_ReadLogicZero(manager);
	Cudd_Ref(logicZero);

    /* make a new bdd for this tuple */
	itenode = Cudd_ReadOne(manager);
	Cudd_Ref(itenode);

    // This code only works for a total of 128 BDD vars, beware
    assert(Cudd_ReadSize(manager) <= 128);

    //! make sure the current buffer level
    //! is within the size of the BDD
    if(bdd_buffer_level < (sizeof(guint64) * 8 * 2))
        {
            /* iterate through the 64 bit values for both X and Y */
            for (i = 0; i <= bdd_buffer_level; i++) 
                {
                    guint64 bitset;
                    int v;
            
                    //! grab the BDD variable index at the i permutation
                    v = Cudd_ReadInvPerm(manager,127-i);

                    tmp = itenode;	  
            
                    //! if the bit is under 64 bits, it is part of the x variable
                    //! if the bit location is < 127 but > 63, is is part
                    //! of the y variable
                    //! grab the state of the bit from that tuple member
                    bitset = (v < 64)?(x & mask((guint64)(v))):(y & mask((guint64)(v-64)));

                    if(bitset) 
                        {
                            itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager,v), tmp,
                                                  logicZero);
                            Cudd_Ref(itenode);
                        } 
                    else 
                        {
                            itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager, v),
                                                  logicZero, tmp);
                            Cudd_Ref(itenode);
                        }
	  
                    //! DEBUG
                    /*                 if(cuddTimesInDeathRow(manager, itenode) > 1) */
                    /*                     { */
                    /*                         printf("DR - x:%d,y:%d,i%d,DR:%d\n", x, y, i, (cuddTimesInDeathRow(manager, itenode))); */
                    /*                     } */
                
                    Cudd_DelayedDerefBdd(manager, tmp);
                    //                Cudd_IterDerefBdd(manager, tmp);
                }
        
            //! make sure we have a valid BDD
            assert(itenode != NULL);     

            //! DEBUG
            if( manager->dead != oldDead )
                {
                    oldDead = manager->dead;                        
                }                   

            //! add in the new variable
            tmp = Cudd_bddOr(manager, itenode, set);
        
            //! increment the ref count of the new buffer node
            Cudd_Ref(tmp);

            //! decrement the reference count of set
            Cudd_IterDerefBdd(manager, set);
            //            Cudd_DelayedDerefBdd(manager, set);
        
            //! decrement the reference count of the temp bdd variable
            //            Cudd_IterDerefBdd(manager, itenode);
            Cudd_DelayedDerefBdd(manager, itenode);
        
            //! DEBUG
            // g_printf("set ref:%d, itenode ref:%d, tmp ref:%d\n", set->ref, itenode->ref, tmp->ref);

            //! reset the bdd buffer node
            set = tmp;

            //! decrement the reference count of logicZero
            //            Cudd_IterDerefBdd(manager, logicZero);
            Cudd_DelayedDerefBdd(manager, logicZero);
        }
    
    return(set);
}


/*
  Function: adamantbdd_layer_findaddtuple

  This function finds the tuple in the BDD node
  then adds the new layers into the BDD from the 
  tuple set.

  Called with: DdManager * manager - BDD manager
               DdNode * set - the main BDD set
               guint64 x - x tuple member
               guint64 y - y tuple member

  Returns: int - status
  Side effects: modifies the current bdd and 
                adds to the bdd buffer

*/
DdNode * adamantbdd_layer_findaddtuple(DdManager * manager, DdNode * node, 
                                       guint64 x, guint64 y)
{    
    adbdd_layer_recur search_member;
    DdNode * return_node = NULL;

    //! make sure the buffer level is ok
    assert(bdd_buffer_level <= 64);

/*     //! get the buffer level for  */
/*     DdNode * proj_func_v1 = Cudd_bddIthVar(manager,  */
/*                                            Cudd_ReadInvPerm(manager, bdd_buffer_level)); */

/*     DdNode * proj_func_v2 = bdd_buffer_level + 64; */

/*     //! get the projection function for the variable */
/*     node_b_level = Cudd_bddIthVar(manager,  */
/*                                   Cudd_ReadInvPerm(manager, (level - 1))); */

    //! initialize the recursive search structure
    search_member.manager = manager;
    search_member.node = node;
    search_member.num_complement = 0;
    search_member.x = x;
    search_member.y = y;

    //! find the node that contains the existing 
    //! layers of this tuple
    return_node = adamantbdd_layer_findtuple(&search_member);

    //! TODO
    //! now add the new layers on top of the current node
    guint64 bitset = 0;
    int v = 0;
    DdNode * itenode = NULL;

	DdNode * logicZero = Cudd_ReadLogicZero(manager);
	Cudd_Ref(logicZero);

    //! grab the BDD variable index at the i permutation
    v = Cudd_ReadInvPerm(manager, 127-bdd_buffer_level);
            
    //! if the bit is under 64 bits, it is part of the x variable
    //! if the bit location is < 127 but > 63, is is part
    //! of the y variable
    //! grab the state of the bit from that tuple member
    bitset = (v < 64)?(x & mask((guint64)(v))):(y & mask((guint64)(v-64)));

    if(bitset) 
        {
            itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager,v), return_node,
                                  logicZero);
            Cudd_Ref(itenode);
        } 
    else 
        {
            itenode = Cudd_bddIte(manager, Cudd_bddIthVar(manager, v),
                                  logicZero, return_node);
            Cudd_Ref(itenode);
        }

    Cudd_DelayedDerefBdd(manager, return_node);

    //! reset the return node
    return_node = itenode;
 
    return(return_node);
}
    

/*
  Function: adamantbdd_findtuple

  This function finds the tuple in the BDD and
  returns the node that occurs right before
  the bdd layer

  Called with: guint64 x - x tuple member
               guint64 y - y tuple member

  Returns: int - status
  Side effects: none

*/
DdNode * adamantbdd_layer_findtuple(adbdd_layer_recur * member)
{
    DdNode * return_node = member->node;

	//! check to see if this node is complemented
	if((Cudd_IsComplement(member->node) == 1))
        {
            member->num_complement++;
        } 

	//! check to see if this node is a constant
	if(Cudd_IsConstant(Cudd_Regular(member->node)))
        {
            return(return_node);            
        }
	
    //! find out what level I am at in the BDD
    int v = Cudd_NodeReadIndex(member->node);

    //! determine the permuation at which this variable occurs
    int level = Cudd_ReadPerm(member->manager, v);

    //! test tuple membership
    if((v < 64) && (level != bdd_buffer_level))
        {

            //! find the next direction that we want
            //! to take
            guint64 bitset = (member->x & mask((guint64)(v)));

            if(bitset) 
                {
                    member->node =  Cudd_T(Cudd_Regular(member->node));
                } 
            else 
                {
                    member->node =  Cudd_E(Cudd_Regular(member->node));
                }

            //! recurse
            return_node = adamantbdd_layer_findtuple(member);

        }

    else if (level != (bdd_buffer_level+64))
        {
            //! find the next direction that we want
            //! to take
            guint64 bitset = (member->y & mask((guint64)(v-64)));

            if(bitset) 
                {
                    member->node =  Cudd_T(Cudd_Regular(member->node));
                } 
            else 
                {
                    member->node =  Cudd_E(Cudd_Regular(member->node));
                }

            //! recurse
            return_node = adamantbdd_layer_findtuple(member);
        }

    return(return_node);
}


#if 0

/*!
 * This function will extract all of the DINs for a given
 * Ready time using a recursive procedure
 */
guint64 range_recurs_builder(recurseFuncStruct recurStruct)
{
	guint64 return_num = 0;	//! return number of collected numbers
	DdNode * nextNode;
    int level = 0, varIndex = 0;
    guint64 saved_collected_num = recurStruct.collectedNumber;
    guint64 saved_set_num = recurStruct.setNumber;
    collectFuncStruct t_col_struct;


    //! this should have a value
    assert(variableAssign != NULL);
    
	//! check to see if this node is complemented
	if((Cudd_IsComplement(recurStruct.myNode) == 1))
        {
#if 0
            g_print("%"G_GUINT64_FORMAT": is COMPLEMENTED\n", recurStruct.myNode);
            
#endif
            recurStruct.numComplement++;	
        }    
	
	//! check to see if this node is a constant
	if(Cudd_IsConstant(Cudd_Regular(recurStruct.myNode)))
        {
            //! determine if this pointer value is not complemented an odd
            //! number of times, and the constant value is One
            if(((recurStruct.numComplement & 0x1) != 0x1))
                {   
      
                    //! check for "don't care" values
                    if(((recurStruct.collectedNumber == recurStruct.collectedNumberNot)
                        &&(recurStruct.setNumber == recurStruct.setNumberNot)))
                        {
                            //! increment the count of collected numbers
                            return_num++;
                            
                            //! call the function that will handle the collected
                            //! number
                            t_col_struct.manager = recurStruct.manager;
                            t_col_struct.newNum = recurStruct.collectedNumber;
                            t_col_struct.presetNum = recurStruct.setNumber;
                            (*this.*recurStruct.ptr2bddcollect)(&t_col_struct);
                        }
                    else
                        {                    
                              
                            //! expand any "Don't Care" values which show up 
                            //! as skipped levels in the BDD traversal.
                            DCExpand_recur(recurStruct, ((sizeof(guint64)*8)-1), 
                                           recurStruct.setNumber, recurStruct.collectedNumber);
                           
                        }
                }	        
        }
    else
        {		
            
            //! Determine the variable that this node represents
            varIndex = Cudd_NodeReadIndex(recurStruct.myNode);
            
            //! Determine the permuation at which this variable occurs
            level = Cudd_ReadPerm(recurStruct.manager, varIndex);
            
            assert(varIndex != 65535);

        
            //!***********************************
            //! ** first handle the "Then" node **
            //!***********************************
            
            nextNode = Cudd_T(Cudd_Regular(recurStruct.myNode));	// grab the then node
            if(NodeCheckRange(1, &recurStruct) == 1)//! check to see if we are in bounds
                {
                    
                    DdNode * save_node = recurStruct.myNode;
                    recurStruct.myNode = nextNode;

                    // g_print("Diving down THEN branch:%"G_GUINT64_FORMAT"\n", myNode);
                            
                            
                    //! if we have a true value, OR in a 1 into the correct spot in the out_num
                    if(variableAssign[level] == DIN)
                        {
                            recurStruct.collectedNumber = 
                                (recurStruct.collectedNumber | mask((guint64)(varIndex - collection_bottom)));
                        }
                    if(variableAssign[level] == RDY)
                        {
                            recurStruct.setNumber = 
                                (recurStruct.setNumber | mask((guint64)(varIndex - rdybottom)));
                        }
  
                    
                    //! Here is the recursive call
                    return_num += range_recurs_builder(recurStruct);		
                            
                    //! reset the local copy struct members
                    recurStruct.collectedNumber = saved_collected_num;
                    recurStruct.setNumber = saved_set_num;
                    recurStruct.myNode = save_node;      
                 }            

             
            //!********************************
            //! ** now handle the else case **
            //!********************************            
 
            nextNode = Cudd_E(Cudd_Regular(recurStruct.myNode));
            if(NodeCheckRange(0, &recurStruct) == 1)// ! check bounds
                {
#if 0
                    // TEST
                    if(debug_pred || true)
                        {
                            g_print("Doing ELSE for:%"G_GUINT64_FORMAT", level:%d\n", recurStruct.myNode, level);			
                        }
#endif
                    
                    DdNode * save_node = recurStruct.myNode;
                    recurStruct.myNode = nextNode;

                    if(variableAssign[level] == DIN)
                        {
                            //! if we have a true value, AND in a 0 into the correct spot in the out_num
                            recurStruct.collectedNumberNot = 
                                (recurStruct.collectedNumberNot & ~mask((guint64)(varIndex - collection_bottom)));
                        }                            
                    if(variableAssign[level] == RDY)
                        {
                            //! if we have a true value, AND in a 0 into the correct spot in the out_num
                            recurStruct.setNumberNot = 
                                (recurStruct.setNumberNot & ~mask((guint64)(varIndex - rdybottom)));
                        }
                            
                    //			g_print("Diving down ELSE branch:%"G_GUINT64_FORMAT"\n", myNode);
                            
                    //! Here is the else recursive call
                    return_num += range_recurs_builder(recurStruct);

                    recurStruct.myNode = save_node;//!restore the current node 
                }
        }
             
	
	return (return_num);
}

#endif

