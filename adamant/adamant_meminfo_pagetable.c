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

#include <glib.h>
#include <stdlib.h>
#include "adamant_meminfo_pagetable.h"

static inline int 
hash1(guint64 p)
{
  int pupper = p>>32;
  int plower = p;
  return (pupper ^ plower) & MIPT_HASHTABLE_MASK;
}

static inline int 
hash2(guint64 p)
{
  int pupper = p>>32;
  int plower = p;
  return (~(pupper ^ plower)) & MIPT_HASHTABLE_MASK;
}

MemInfoPageTableInteriorNode *
MemInfoPageTableInteriorNode_new() 
{
  MemInfoPageTableInteriorNode *inode;
  inode=g_new0(MemInfoPageTableInteriorNode,1);
  return inode;
}

void 
MemInfoPageTableInteriorNode_destroy(MemInfoPageTableInteriorNode *inode,
				     int curdepth,
				     int maxdepth) 
{
  int i;
  for(i=0;i<MIPT_INTERIOR_PAGE_LEN;i++) {
    if(inode->nodes[i]) {
      if(curdepth <  maxdepth) {
	MemInfoPageTableInteriorNode_destroy(inode->nodes[i],
					     curdepth+1,
					     maxdepth);
      } else {
	/* Free chunks here if in free-chunk-by-chunk mode */
      }
    }
  }
  // todo: we are using mem chunks now so cannot free normally
  g_free(inode);
}


MemInfoPageTable *
MemInfoPageTable_new() 
{
  MemInfoPageTable *pt;
  int intindexbits;

  pt=g_new0(MemInfoPageTable,1);
  intindexbits=sizeof(guint64)*8 - MIPT_LINE_BITS;
  if(intindexbits % MIPT_INTERIOR_PAGE_BITS) {
    g_print("** ERROR **: 64 - MIPT_LINE_BITS must be "
	   "divisible by MIPT_INTERIOR_PAGE_BITS\n");
    g_free(pt);
    exit(-1);
    return NULL;
  }
  pt->depth = intindexbits/MIPT_INTERIOR_PAGE_BITS;
  return pt;
}

void MemInfoPageTable_destroy(MemInfoPageTable *pt)
{
  int i;
  for(i=0;i<MIPT_INTERIOR_PAGE_LEN;i++) {
    if(pt->root.nodes[i]) {
      MemInfoPageTableInteriorNode_destroy(pt->root.nodes[i],1,pt->depth);
    }
  }
  g_free(pt);
}

MemInfoPageTableEntry *
MemInfoPageTable_lookup(MemInfoPageTable *pt, guint64 pointer)
{
  int i;
  int hashindex1, hashindex2;
  MemInfoPageTableEntry *ami;
  MemInfoPageTableInteriorNode *curnode;
  guint64 localpointer=pointer;

  localpointer >>= MIPT_LINE_BITS;
  hashindex1=hash1(localpointer);

  ami = pt->hasharray[hashindex1];
  if(ami) {
    if(ami->ea == pointer) {
      return ami;
    }
  } else {
    hashindex2=hash2(localpointer);
    ami = pt->hasharray[hashindex2];
    if(ami) {
      if(ami->ea == pointer) {
	return ami;
      }
    }
  }
  
  /* Slow lookup, we missed in the hash table */
  curnode = &pt->root;
  for(i=0;i<pt->depth;i++) {
    if(!curnode) {
      //      failcount++;
      return NULL;
    }
    curnode=(MemInfoPageTableInteriorNode *)(curnode->
      nodes[(localpointer >> ((pt->depth - i - 1) * MIPT_INTERIOR_PAGE_BITS)) 
	    & MIPT_INTERIOR_PAGE_MASK]);
  }
  ami=(MemInfoPageTableEntry *)curnode;

#if 0
  count++;
  succcount++;
  g_print("Succeeded = %g, failed = %g, hashhit = %g\n",
	 ((double)succcount)/count,
	 ((double)failcount)/count,
	 ((double)count - failcount - succcount)/count);
#endif

  // Add ami to the hash table evicting someone else,
  // at random for now, LRU later
  hashindex1=hash1(pointer >> MIPT_LINE_BITS);
  hashindex2=hash2(pointer >> MIPT_LINE_BITS);
  if(!pt->hasharray[hashindex1]) {
    pt->hasharray[hashindex1]=ami;
  } else if(!pt->hasharray[hashindex2]) {
    pt->hasharray[hashindex2]=ami;
  } else {
    if(g_random_boolean()) {
      pt->hasharray[hashindex1]=ami;
    } else {
      pt->hasharray[hashindex2]=ami;
    }
  }

  return ami;
}

void 
MemInfoPageTable_add(MemInfoPageTable *pt, guint64 pointer,
		     MemInfoPageTableEntry *entry)
{
  MemInfoPageTableInteriorNode *curnode;
  MemInfoPageTableInteriorNode *newcurnode;
  guint64 localpointer=pointer;
  int i;

  curnode = &pt->root;
  localpointer >>= MIPT_LINE_BITS;

  /* We're adding, probably because we missed before.  
     Just do a slow lookup */
  for(i=0;i<pt->depth-1;i++) {
    int index = 
      (localpointer >> ((pt->depth - i - 1) * MIPT_INTERIOR_PAGE_BITS)) & 
      MIPT_INTERIOR_PAGE_MASK;
    newcurnode=(MemInfoPageTableInteriorNode *)(curnode->nodes[index]);
    if(!newcurnode) {
      newcurnode = MemInfoPageTableInteriorNode_new();
      curnode->nodes[index] = newcurnode;
    }
    curnode=newcurnode;
  }
  pt->nummeminfos++;
  curnode->nodes[localpointer & MIPT_INTERIOR_PAGE_MASK] = entry;
}

