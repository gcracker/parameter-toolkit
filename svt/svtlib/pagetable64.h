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
#ifndef _PAGETABLE64_H_
#define _PAGETABLE64_H_

#include <glib.h>

typedef struct _PageTable64 PageTable64;
typedef struct _PageTableEntry PageTableEntry;

/* Number of pointers that can be stored on a single node*/
#define PT64_INTERIOR_PAGE_BITS (12)
#define PT64_INTERIOR_PAGE_LEN (1 << PT64_INTERIOR_PAGE_BITS)
#define PT64_INTERIOR_PAGE_MASK (PT64_INTERIOR_PAGE_LEN - 1)

typedef struct _PageTable64InteriorNode PageTable64InteriorNode;
struct _PageTable64InteriorNode {
  void *nodes[PT64_INTERIOR_PAGE_LEN];
  /* (void * == MemInfo * if interior node of depth-1
     Otherwise, void * == PageTable64InteriorNode */

};

#define PT_HASHTABLE_BITS (10)
#define PT_HASHTABLE_SIZE (1 << (PT_HASHTABLE_BITS))
#define PT_HASHTABLE_MASK ((PT_HASHTABLE_SIZE)-1)

struct _PageTable64 {
  int depth;
  int num_entries;
  PageTableEntry *hasharray[PT_HASHTABLE_SIZE];
  PageTable64InteriorNode root;
};

/* ENTRY_LINE_BITS 1<<BITS entries per line */
#define ENTRY_LINE_BITS (16)
#define ENTRY_LINE_SIZE (((guint64)1) << ENTRY_LINE_BITS)
#define ENTRY_KEY_INDEX_MASK (ENTRY_LINE_SIZE-1)
#define ENTRY_KEY_LINE_MASK (~ENTRY_KEY_INDEX_MASK)

struct _PageTableEntry {
  guint64 key;
  void *data[ENTRY_LINE_SIZE];
};

PageTable64 *PageTable64_new();
PageTableEntry *PageTable64_lookup(PageTable64 *pt, guint64 pointer);
void PageTable64_add(PageTable64 *pt, guint64 pointer,
		     PageTableEntry *entry);
void PageTable64_destroy(PageTable64 *pt);

#endif /* _PAGETABLE64_H_ */
#ifdef __cplusplus
}
#endif

