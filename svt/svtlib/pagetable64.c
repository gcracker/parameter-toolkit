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

#ifdef __cplusplus
extern "C" {
#endif
#include <svt.h>
#include <glib.h>
#include <stdlib.h>

PageTable64 *PageTable64_new() 
{
  PageTable64 *pt;
  int intindexbits;

  pt=g_new0(PageTable64,1);
  intindexbits=sizeof(guint64)*8 - ENTRY_LINE_BITS;
  if(intindexbits % PT64_INTERIOR_PAGE_BITS) {
    printf("** ERROR **: 64 - ENTRY_LINE_BITS must be "
	   "divisible by PT64_INTERIOR_PAGE_BITS\n");
    g_free(pt);
    exit(-1);
    return NULL;
  }
  pt->depth = intindexbits/PT64_INTERIOR_PAGE_BITS;
  //printf("PT depth = %d\n",pt->depth);
  return pt;
}

PageTable64InteriorNode *PageTable64InteriorNode_new() 
{
  PageTable64InteriorNode *inode;

  inode=g_new0(PageTable64InteriorNode,1);
  return inode;
}

static inline int hash1(guint64 p)
{
  int pupper = p>>32;
  int plower = p;
  return (pupper ^ plower) & PT_HASHTABLE_MASK;
}

static inline int hash2(guint64 p)
{
  int pupper = p>>32;
  int plower = p;
  return (~(pupper ^ plower)) & PT_HASHTABLE_MASK;
}

PageTableEntry *PageTable64_lookup(PageTable64 *pt, guint64 pointer)
{
  PageTable64InteriorNode *curnode;
  int i;
  int hashindex1, hashindex2;
  PageTableEntry *ami;
  guint64 localpointer=pointer;

  //  count++;
  localpointer >>= ENTRY_LINE_BITS;

  hashindex1=hash1(localpointer);

  ami = pt->hasharray[hashindex1];
  if(ami) {
    if(ami->key == pointer) {
      return ami;
    }
  } else {
    hashindex2=hash2(localpointer);
    ami = pt->hasharray[hashindex2];
    if(ami) {
      if(ami->key == pointer) {
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
    curnode=(PageTable64InteriorNode *)(curnode->
      nodes[(localpointer >> ((pt->depth - i - 1) * PT64_INTERIOR_PAGE_BITS)) 
	    & PT64_INTERIOR_PAGE_MASK]);
  }

  ami=(PageTableEntry *)curnode;

#if 0
  succcount++;
  printf("Succeeded = %g, failed = %g, hashhit = %g\n",
	 ((double)succcount)/count,
	 ((double)failcount)/count,
	 ((double)count - failcount - succcount)/count);
#endif

  /* Add ami to the hash table evicting someone else, 
     at random for now, LRU later */

  hashindex1=hash1(pointer >> ENTRY_LINE_BITS);
  hashindex2=hash2(pointer >> ENTRY_LINE_BITS);
  if(!pt->hasharray[hashindex1]) {
    pt->hasharray[hashindex1]=ami;
  } else if(!pt->hasharray[hashindex2]) {
    pt->hasharray[hashindex2]=ami;
  } else {
    if(random() & 0x1) {
      pt->hasharray[hashindex1]=ami;
    } else {
      pt->hasharray[hashindex2]=ami;
    }
  }

  return ami;
}

void PageTable64_add(PageTable64 *pt, guint64 pointer,
		     PageTableEntry *entry)
{
  PageTable64InteriorNode *curnode;
  PageTable64InteriorNode *newcurnode;
  guint64 localpointer=pointer;
  int i;

  curnode = &pt->root;
  localpointer >>= ENTRY_LINE_BITS;

  /* We're adding, probably because we missed before.  
     Just do a slow lookup */
  for(i=0;i<pt->depth-1;i++) {
    int index = 
      (localpointer >> ((pt->depth - i - 1) * PT64_INTERIOR_PAGE_BITS)) & 
      PT64_INTERIOR_PAGE_MASK;
    newcurnode=(PageTable64InteriorNode *)(curnode->nodes[index]);
    if(!newcurnode) {
      newcurnode = PageTable64InteriorNode_new();
      curnode->nodes[index] = newcurnode;
    }
    curnode=newcurnode;
  }
  pt->num_entries++;
  curnode->nodes[localpointer & PT64_INTERIOR_PAGE_MASK] = entry;
}

void PageTable64InteriorNode_destroy(PageTable64InteriorNode *inode,
				     int curdepth,
				     int maxdepth) 
{
  int i;
  
  for(i=0;i<PT64_INTERIOR_PAGE_LEN;i++) {
    if(inode->nodes[i]) {
      if(curdepth <  maxdepth) {
	PageTable64InteriorNode_destroy(inode->nodes[i],
					curdepth+1,
					maxdepth);
      } else {
	/* Free chunks here if in free-chunk-by-chunk mode */
      }
    }
  }
  g_free(inode);
}

void PageTable64_destroy(PageTable64 *pt)
{
  int i;
  for(i=0;i<PT64_INTERIOR_PAGE_LEN;i++) {
    if(pt->root.nodes[i]) {
      PageTable64InteriorNode_destroy(pt->root.nodes[i],1,pt->depth);
    }
  }
  g_free(pt);
}
#ifdef __cplusplus
}
#endif

