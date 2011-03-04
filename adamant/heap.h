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

#ifndef _HEAP_H
#define _HEAP_H

/*
  Heap routines: a smallest-at-top heap.  You also supply a comparison
  function.  You allocate a heap using HeapAlloc, and HeapCmp is a pointer
  to a function to compare your entries.  HeapAlloc would like a
  reasonable guess as to the maximum size the heap will grow, but if you
  get it wrong, don't sweat it, these routines will use realloc(3) to
  increase the size if needed.  Entries are of type foint (union of int
  and void*), see misc.h for details.
*/

#include "misc.h"   /* for foint */

/* this is returned if any errors occur */
#define HEAPERROR ABSTRACT_ERROR

typedef struct _heaptype
{
  int HEAPSIZE;
  pCmpFcn HeapCmp;
  pFointFreeFcn FointFree;
  foint *heap;
} HEAP;


HEAP    *HeapAlloc(int maxNumber, pCmpFcn);
void    HeapReset(HEAP*);   /* make heap empty */
int     HeapSize(HEAP*);    /* how many things currently in heap? */
void    HeapFree(HEAP*);    /* free the entire heap */
foint   HeapPeek(HEAP*);    /* what's the top element? */
foint   HeapNext(HEAP*);    /* pop and return top element */
foint   HeapInsert(HEAP*, foint);
/*
** delete is slow O(n); if more than one match, only one is deleted, but
** no guarantees on which gets deleted.
*/
foint   HeapDelete(HEAP*, foint);
/*int     HeapSort(foint *list, pCmpFcn);*/
int     HeapSanityCheck(HEAP*);   /* sanity check, for debugging */
void    HeapPrint(HEAP*);   /* dump an entire heap.  You must define a HeapTypePrint function */
#if 0
void    HeapTypePrint(/*not-void*/);
#endif
extern void HeapTypePrint(foint);

#endif  /* _HEAP_H */

