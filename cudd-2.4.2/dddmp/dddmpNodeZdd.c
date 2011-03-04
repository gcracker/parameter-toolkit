/**CFile**********************************************************************

   FileName     [dddmpNodeZdd.c]

   PackageName  [dddmp]

   Synopsis     [Functions to handle ZDD node infos and numbering]

   Description  [Functions to handle ZDD node infos and numbering.
   ]

   Author       [Gianpiero Cabodi and Stefano Quer]
   Author       [modified by Graham Price]
  

   Copyright    [
   Copyright (c) 2004 by Politecnico di Torino.
   All Rights Reserved. This software is for educational purposes only.
   Permission is given to academic institutions to use, copy, and modify
   this software and its documentation provided that this introductory
   message is not removed, that this software and its documentation is
   used for the institutions' internal research and educational purposes,
   and that no monies are exchanged. No guarantee is expressed or implied
   by the distribution of this code.
   Send bug-reports and/or questions to:
   {gianpiero.cabodi,stefano.quer}@polito.it.
   ]

******************************************************************************/

#include "dddmpInt.h"

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int NumberNodeRecurZdd(DdNode *f, int id);
static void RemoveFromUniqueRecurZdd(DdManager *ddMgr, DdNode *f);
static void RestoreInUniqueRecurZdd(DdManager *ddMgr, DdNode *f);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis    [Removes nodes from unique table and number them]

   Description [Node numbering is required to convert pointers to integers.
   Since nodes are removed from unique table, no new nodes should 
   be generated before re-inserting nodes in the unique table
   (DddmpUnnumberZddNodes ()).
   ]

   SideEffects [Nodes are temporarily removed from unique table]

   SeeAlso     [RemoveFromUniqueRecur(), NumberNodeRecur(), 
   DddmpUnnumberZddNodes ()]

******************************************************************************/

int
DddmpNumberZddNodes (
		     DdManager *ddMgr  /* IN: DD Manager */,
		     DdNode **f        /* IN: array of ZDDs */,
		     int n             /* IN: number of ZDD roots in the array of ZDDs */
		     )
{
  int id=0, i;

  for (i=0; i<n; i++) {
    RemoveFromUniqueRecurZdd (ddMgr, f[i]);
  }

  for (i=0; i<n; i++) {
    id = NumberNodeRecurZdd (f[i], id);
  }

  return (id);
}


/**Function********************************************************************

   Synopsis     [Restores nodes in unique table, loosing numbering]

   Description  [Node indexes are no more needed. Nodes are re-linked in the
   unique table.
   ]

   SideEffects  [None]

   SeeAlso      [DddmpNumberZddNode ()]

******************************************************************************/

void
DddmpUnnumberZddNodes(
		      DdManager *ddMgr  /* IN: DD Manager */,
		      DdNode **f        /* IN: array of ZDDs */,
		      int n             /* IN: number of ZDD roots in the array of ZDDs */
		      )
{
  int i;

  for (i=0; i<n; i++) {
    RestoreInUniqueRecurZdd (ddMgr, f[i]);
  }

  return;
}

/**Function********************************************************************

   Synopsis     [Write index to node]

   Description  [The index of the node is written in the "next" field of
   a DdNode struct. LSB is not used (set to 0). It is used as 
   "visited" flag in DD traversals.
   ]

   SideEffects  [None]

   SeeAlso      [DddmpReadNodeIndexZdd(), DddmpSetVisitedZdd (),
   DddmpVisitedZdd ()]

******************************************************************************/

void 
DddmpWriteNodeIndexZdd (
			DdNode *f   /* IN: ZDD node */,
			int id       /* IN: index to be written */
			)
{
  if (!Cudd_IsConstant (f)) {
    f->next = (struct DdNode *)((ptruint)((id)<<1));
  }

  return;
}

/**Function********************************************************************

   Synopsis     [Reads the index of a node]

   Description  [Reads the index of a node. LSB is skipped (used as visited
   flag).
   ]

   SideEffects  [None]

   SeeAlso      [DddmpWriteNodeIndexZdd (), DddmpSetVisitedZdd (),
   DddmpVisitedZdd ()]

******************************************************************************/

int
DddmpReadNodeIndexZdd (
		       DdNode *f    /* IN: ZDD node */
		       )
{
  if (!Cudd_IsConstant (f)) {
    return ((int)(((ptruint)(f->next))>>1));
  } else {
    return (1);
  }
}

/**Function********************************************************************

   Synopsis     [Returns true if node is visited]

   Description  [Returns true if node is visited]

   SideEffects  [None]

   SeeAlso      [DddmpSetVisitedZdd (), DddmpClearVisitedZdd ()]

******************************************************************************/

int
DddmpVisitedZdd (
		 DdNode *f    /* IN: ZDD node to be tested */
		 )
{
  return ((int)((ptruint)(f->next)) & (01));
}

/**Function********************************************************************

   Synopsis     [Marks a node as visited]
 
   Description  [Marks a node as visited]

   SideEffects  [None]

   SeeAlso      [DddmpVisitedZdd (), DddmpClearVisitedZdd ()]

******************************************************************************/

void
DddmpSetVisitedZdd (
		    DdNode *f   /* IN: ZDD node to be marked (as visited) */
		    )
{

  f->next = (DdNode *)(ptruint)((int)((ptruint)(f->next))|01);

  return;
}

/**Function********************************************************************

   Synopsis     [Marks a node as not visited]

   Description  [Marks a node as not visited]

   SideEffects  [None]

   SeeAlso      [DddmpVisited (), DddmpSetVisited ()]

******************************************************************************/

void
DddmpClearVisitedZdd (
		      DdNode *f    /* IN: ZDD node to be marked (as not visited) */
		      )
{
  f->next = (DdNode *)(ptruint)((int)((ptruint)(f->next)) & (~01));

  return;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

   Synopsis     [Number nodes recursively in post-order]

   Description  [Number nodes recursively in post-order.
   The "visited" flag is used with inverse polarity, because all nodes
   were set "visited" when removing them from unique. 
   ]

   SideEffects  ["visited" flags are reset.]

   SeeAlso      []

******************************************************************************/

static int
NumberNodeRecurZdd (
		    DdNode *f  /*  IN: root of the ZDD to be numbered */,
		    int id     /* IN/OUT: index to be assigned to the node */
		    )
{
  if (!DddmpVisitedZdd (f)) {
    return (id);
  }

  if (!cuddIsConstant (f)) {
    id = NumberNodeRecurZdd (cuddT (f), id);
    id = NumberNodeRecurZdd (cuddE (f), id);
  }

  DddmpWriteNodeIndexZdd (f, ++id);
  DddmpClearVisitedZdd (f);

  return (id);
}

/**Function********************************************************************

   Synopsis    [Removes a node from unique table]

   Description [Removes a node from the unique table by locating the proper
   subtable and unlinking the node from it. It recurs on the 
   children of the node. Constants remain untouched.
   ]

   SideEffects [Nodes are left with the "visited" flag true.]

   SeeAlso     [RestoreInUniqueRecurZdd ()]

******************************************************************************/

static void
RemoveFromUniqueRecurZdd (
			  DdManager *ddMgr  /*  IN: DD Manager */,
			  DdNode *f         /*  IN: root of the ZDD to be extracted */
			  )
{
  DdNode *node, *last, *next;
  DdNode *sentinel = &(ddMgr->sentinel);
  DdNodePtr *nodelist;
  DdSubtable *subtable;
  int pos, level;

  if (DddmpVisitedZdd (f)) {
    return;
  }

  if (!cuddIsConstant (f)) {

    RemoveFromUniqueRecurZdd (ddMgr, cuddT (f));
    RemoveFromUniqueRecurZdd (ddMgr, cuddE (f));

    level = ddMgr->permZ[f->index];
    subtable = &(ddMgr->subtableZ[level]);

    nodelist = subtable->nodelist;

    pos = ddHash (cuddT (f), cuddE (f), subtable->shift);
    node = nodelist[pos];
    last = NULL;
    while (node != sentinel) {
      next = node->next;
      if (node == f) {
        if (last != NULL)  
  	  last->next = next;
        else 
          nodelist[pos] = next;
        break;
      } else {
        last = node;
        node = next;
      }
    }

    f->next = NULL;
  }

  DddmpSetVisitedZdd (f);

  return;
}

/**Function********************************************************************

   Synopsis     [Restores a node in unique table]

   Description  [Restores a node in unique table (recursively)]

   SideEffects  [Nodes are not restored in the same order as before removal]

   SeeAlso      [RemoveFromUnique()]

******************************************************************************/

static void
RestoreInUniqueRecurZdd (
			 DdManager *ddMgr /*  IN: DD Manager */,
			 DdNode *f        /*  IN: root of the ZDD to be restored */
			 )
{
  DdNodePtr *nodelist;
  DdSubtable *subtable;
  int pos, level;
#ifdef DDDMP_DEBUG
  DdNode *node;
  DdNode *sentinel = &(ddMgr->sentinel);
#endif

  if (!Cudd_IsComplement (f->next)) {
    return;
  }

  if (cuddIsConstant (f)) {
    /* StQ 11.02.2004:
       Bug fixed --> restore NULL within the next field */
    /*DddmpClearVisitedZdd (f);*/   
    f->next = NULL;

    return;
  }

  RestoreInUniqueRecurZdd (ddMgr, cuddT (f));
  RestoreInUniqueRecurZdd (ddMgr, cuddE (f));

  level = ddMgr->permZ[f->index];
  subtable = &(ddMgr->subtableZ[level]);

  nodelist = subtable->nodelist;

  pos = ddHash (cuddT (f), cuddE (f), subtable->shift);

#ifdef DDDMP_DEBUG
  /* verify uniqueness to avoid duplicate nodes in unique table */
  for (node=nodelist[pos]; node != sentinel; node=node->next)
    assert(node!=f);
#endif
  
  f->next = nodelist[pos];
  nodelist[pos] = f;
  return;
}


