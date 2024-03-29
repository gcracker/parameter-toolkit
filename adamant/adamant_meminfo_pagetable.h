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

#ifndef __ADAMANT_MEMINFO_PAGETABLE_H__
#define __ADAMANT_MEMINFO_PAGETABLE_H__

#include <glib.h>
#include <svt.h>

/* MIPT_LINE_BITS 1<<BITS entries per line */
#define MIPT_LINE_BITS (16)
#define MIPT_LINE_SIZE (((guint64)1) << MIPT_LINE_BITS)
#define MIPT_INDEX_MASK (MIPT_LINE_SIZE-1)
#define MIPT_LINE_MASK (~MIPT_INDEX_MASK)

typedef struct _MemInfoPageTableEntry MemInfoPageTableEntry;
struct _MemInfoPageTableEntry {
  guint64 ea;
  SchedInfo data[MIPT_LINE_SIZE];
};

/* Number of pointers that can be stored on a single node*/
#define MIPT_INTERIOR_PAGE_BITS (12)
#define MIPT_INTERIOR_PAGE_LEN (1 << MIPT_INTERIOR_PAGE_BITS)
#define MIPT_INTERIOR_PAGE_MASK (MIPT_INTERIOR_PAGE_LEN - 1)

typedef struct _MemInfoPageTableInteriorNode MemInfoPageTableInteriorNode;
struct _MemInfoPageTableInteriorNode {
  void * nodes[MIPT_INTERIOR_PAGE_LEN];
  /* (void * == MemInfoPageTableEntry * if interior node of depth-1
     Otherwise, void * == MemInfoPageTableInteriorNode */
};

#define MIPT_HASHTABLE_BITS (10)
#define MIPT_HASHTABLE_SIZE (1 << (MIPT_HASHTABLE_BITS))
#define MIPT_HASHTABLE_MASK ((MIPT_HASHTABLE_SIZE)-1)

typedef struct _MemInfoPageTable MemInfoPageTable;
struct _MemInfoPageTable {
  int depth;
  int nummeminfos;
  MemInfoPageTableEntry *hasharray[MIPT_HASHTABLE_SIZE];
  MemInfoPageTableInteriorNode root;
};

MemInfoPageTable *
MemInfoPageTable_new();

MemInfoPageTableEntry *
MemInfoPageTable_lookup(MemInfoPageTable * pt, guint64 pointer);

void 
MemInfoPageTable_add(MemInfoPageTable * pt, guint64 pointer,
		     MemInfoPageTableEntry * entry);

void 
MemInfoPageTable_destroy(MemInfoPageTable * pt);

#endif /* __ADAMANT_MEMINFO_PAGETABLE_H__ */

