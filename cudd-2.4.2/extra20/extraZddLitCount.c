/**CFile****************************************************************

  FileName    [extraZddLitCount.c]

  PackageName [extra]

  Synopsis    [Counting literals in the ZDD representing the cover.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddLitCount.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

typedef struct ni
{
    uint32_t level;         /* the level of this variable */
    uint64_t counter;       /* the number of combinations that have this variable */
    struct ni *next;   /* the pointer to the next entry in the list of vars */

} nodeinfo;

typedef struct nmb
{
  nodeinfo * headnode;   /* the pointer to the head entry in the list of vars */
  nodeinfo * tailnode;   /* the pointer to the head entry in the list of vars */
  nodeinfo * nextnode;   /* the pointer to the next entry in the list of vars */
  struct nmb * next; /* pointer to the next block of nodes */
} memblock;


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

// cache alignment
static size_t nodeAlignCount = 0;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
/* head pointer to the memory block list */
static memblock * s_pMemBlockHead;
/* current memory block */
static memblock * s_pMemBlockCurr;

/* pointer to memory allocated by the memory manager */
static nodeinfo *s_pMemory;
/* the number of links allocated */
static size_t s_nLinksAlloc;

/* the iterator */
static nodeinfo *s_pLinkedList;
static size_t s_nLinksReturned;

/* hash table for the nodeinfo structures */
static st_table *s_Table;

/* MG specific stuff */
static size_t mg_maxDepth = 0;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static nodeinfo* extraZddLitCount (DdManager * dd, DdNode * Set);
            
/* local memory manager */
static int       localMemManagerStart ();
static nodeinfo* localMemManagerLinkNext ();
static void      localMemManagerDissolve ();

/* local memory manager modified my Manish V and Graham P*/
static int       mg_localMemManagerStart ();
static nodeinfo* mg_localMemManagerLinkNext ();
static void      mg_localMemManagerDissolve ();
static nodeinfo * mg_localMemManagerAddLinks();
static int mg_MaxNodeDepth(DdManager *manager, DdNode * node);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function*************************************************************

  Synopsis    [Counts the number of literals in one combination.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_zddLitCountComb( DdManager * dd, DdNode * zComb )
{
    int Counter;
    if ( zComb == z0 )
        return 0;
    Counter = 0;
    for ( ; zComb != z1; zComb = cuddT(zComb) )
        Counter++;
    return Counter;
}

/**Function********************************************************************

  Synopsis    [Computes how many times variables occur in combinations of the ZDD.]

  Description [Returns values in the array of integers with as many cells 
               as there are ZDD variables in the manager. The i-th cell 
               of the array contains the number of times the i-th ZDD 
               variable occurs in combinations represented by the ZDD Set.
               It is the user's responsibility to delocate the array 
               using function free().
	       Modified by Manish and Graham]

  SideEffects []

  SeeAlso     [Cudd_zddSubSet, Cudd_zddSupSet, Cudd_zddNotSupSet]

******************************************************************************/
int *
mg_Extra_zddLitCount(
		     DdManager * dd,
		     DdNode * Set,
		     const int maxDepth)
{
  size_t tempDepth = mg_maxDepth;
  mg_maxDepth = maxDepth;
  int * returnArray = Extra_zddLitCount(dd, Set);
  mg_maxDepth = tempDepth;

  return(returnArray);
}


/**Function********************************************************************

  Synopsis    [Computes how many times variables occur in combinations of the ZDD.]

  Description [Returns values in the array of integers with as many cells 
               as there are ZDD variables in the manager. The i-th cell 
               of the array contains the number of times the i-th ZDD 
               variable occurs in combinations represented by the ZDD Set.
               It is the user's responsibility to delocate the array 
               using function free().]

  SideEffects []

  SeeAlso     [Cudd_zddSubSet, Cudd_zddSupSet, Cudd_zddNotSupSet]

******************************************************************************/
int *
Extra_zddLitCount(
  DdManager * dd,
  DdNode * Set)
{
    nodeinfo *NodeInfo = NULL;
    int* Result = NULL;

    /* start the local memory manager (and prepare the interator) */
    int MemAlloc = mg_localMemManagerStart(dd,Set);
    if ( MemAlloc == 0 ) 
        goto failure;

    /* start the hash table for nodeinfo structures */
    s_Table = NULL;
    s_Table = st_init_table(st_ptrcmp,st_ptrhash);
    if ( s_Table == NULL ) 
        goto failure;

    /* call the function recursively */
    NodeInfo = extraZddLitCount(dd, Set);
    if ( NodeInfo == NULL ) 
        goto failure;

    /* debugging */
//  printf( "\nExtra_zddLitCount(): memory internally allocated %dK\n", MemAlloc );
//  printf( "Extra_zddLitCount(): the number of entries allocated %d\n", s_nLinksAlloc );
//  printf( "Extra_zddLitCount(): the number of entries used %d\n", s_nLinksReturned );
//  printf( "\nExtra_zddLitCount(): the number of paths is %d\n", NodeInfo->counter );

    /* allocate memory for the return result */
    Result = (int*) malloc( dd->sizeZ * sizeof( int ) );
    if ( Result == NULL )
        goto failure;
    memset( Result, 0, dd->sizeZ * sizeof( int ) );

    /* skip the path counting nodeinfo */
    NodeInfo = NodeInfo->next;
    /* copy information into this array */
    while ( NodeInfo )
    {
        /* write the number of occurences into the array */
        Result[ dd->invpermZ[NodeInfo->level] ] = NodeInfo->counter;
        /* take the next nodeinfo */
        NodeInfo = NodeInfo->next;
    }

    /* delocate hash table and memory for node info structures */
    mg_localMemManagerDissolve();
    st_free_table( s_Table );
    return Result;

failure:
    dd->errorCode = CUDD_MEMORY_OUT;
    if ( MemAlloc ) mg_localMemManagerDissolve();
    if ( s_Table ) st_free_table( s_Table );
    return NULL;

} /* end of Extra_zddLitCount */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddSupSet.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
nodeinfo * 
extraZddLitCount( 
    DdManager *dd, 
    DdNode *Set)
{   
    nodeinfo *pNode = NULL;

    /* terminal cases */
    if ( Set == dd->zero || Set == dd->one )
        return NULL;

    /* chech whether this nodeinfo entry is in the hash table */
    if ( st_lookup( s_Table, (char*)Set, (char**)&pNode ) )
        return pNode;
    
    if(1 == mg_MaxNodeDepth(dd, Set))
      {
	return NULL;
      }
    
    /* this nodeinfo entry does not exist - build it */

    int nPathsE, nPathsT;
    /* to get the number of paths in the branch, 
     * we can look up the counter of the top most entry */
    nodeinfo *pNodeE, *pNodeT, **pp;

    /* solve subproblems */
    pNodeE = extraZddLitCount( dd, cuddE( Set ) );
    
    /* terminal node */
    if ( pNodeE == NULL )
      {
	nPathsE = (int)( cuddE( Set ) == dd->one ) 
	  + mg_MaxNodeDepth(dd, cuddT( Set ));
      }
    else
      {
	assert( pNodeE->level == -1 );
	nPathsE = pNodeE->counter;
	pNodeE = pNodeE->next;
      }

    pNodeT = extraZddLitCount( dd, cuddT( Set ) );

    /* terminal node */
    if ( pNodeT == NULL )
      {
	nPathsT = (int)( cuddT( Set ) == dd->one )
	  + mg_MaxNodeDepth(dd, cuddT( Set ));
      }
    else
      {
	assert( pNodeT->level == -1 );
	nPathsT = pNodeT->counter;
	pNodeT = pNodeT->next;
      }

    /* add the path count nodeinfo structure to the list */
    pNode = mg_localMemManagerLinkNext();
    pNode->level = -1;
    pNode->counter = nPathsE + nPathsT;

    /* add the current level nodeinfo structure to the list */
    pNode->next = mg_localMemManagerLinkNext();
    pNode->next->level = dd->permZ[ Set->index ];
    pNode->next->counter = nPathsT;

    /* set the pointer to the tail of the list */
    pp = &(pNode->next->next);

    /* merge two nodeinfo lists */
    while ( pNodeE && pNodeT )
      {
	(*pp) = mg_localMemManagerLinkNext();
	if ( pNodeE->level == pNodeT->level )
	  {
	    (*pp)->level = pNodeE->level;
	    (*pp)->counter = pNodeE->counter + pNodeT->counter;
	    pNodeE = pNodeE->next;
	    pNodeT = pNodeT->next;
	  }
	else if ( pNodeE->level < pNodeT->level )
	  {
	    (*pp)->level = pNodeE->level;
	    (*pp)->counter = pNodeE->counter;
	    pNodeE = pNodeE->next;
	  }
	else /* if ( pNodeE->level > pNodeT->level ) */
	  {
	    (*pp)->level = pNodeT->level;
	    (*pp)->counter = pNodeT->counter;
	    pNodeT = pNodeT->next;
	  }
	pp = &((*pp)->next);
      }

    if ( pNodeE || pNodeT )
      {
	if ( pNodeE )
	  (*pp) = pNodeE;
	else
	  (*pp) = pNodeT;
	/* no need to copy the node list */
      }
    else
      (*pp) = NULL;

    if ( st_add_direct( s_Table, (char*)Set, (char*)pNode ) == ST_OUT_OF_MEM )
      return NULL;

    return pNode;
    

} /* end of extraZddLitCount */


/**Function********************************************************************

  Synopsis [Allocates memory used locally for node info structures.]

  Description [Returns the number of Kbytes allocated; 0 in case of failure.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int localMemManagerStart( 
  DdManager * dd,
  DdNode * Set)
{
    size_t MemSize;
    nodeinfo* pTemp;
    size_t i;

    /* estimate roughly the number of needed nodeinfo structures */
    /* count the number of nodes and multiply it by the number of variables */
    /* this is a very crude estimation - may want to improve in the future */
    s_nLinksAlloc = (size_t)(Cudd_DagSize( Set )) * (size_t)(Cudd_SupportSize( dd, Set ));
    MemSize = s_nLinksAlloc * sizeof( nodeinfo );

    /* allocate memory for these structures as one large chunk */
    s_pMemory = (nodeinfo*) malloc( MemSize );
    if ( s_pMemory == NULL )
        return 0;
    memset( s_pMemory, 0, MemSize );

    /* connect all links into a linked list */
    pTemp = s_pMemory;
    for ( i = 0; i < s_nLinksAlloc-1; i++ )
    {
        pTemp->next = pTemp + 1;
        pTemp = pTemp->next;
    }

    /* prepare the iterator */
    s_pLinkedList = s_pMemory;
    s_nLinksReturned = 0;

    return MemSize/1024 + (MemSize%1024 > 0);
}

/**Function********************************************************************

  Synopsis [Returns the next node info structure.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static nodeinfo* localMemManagerLinkNext( void )
{
    assert( s_nLinksReturned < s_nLinksAlloc );

    s_nLinksReturned++;
    return s_pLinkedList++;
}


/**Function********************************************************************

  Synopsis [Releases the allocated memory.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void localMemManagerDissolve( void )
{
    free( s_pMemory );
}

/**Function********************************************************************

  Synopsis [Allocates memory used locally for node info structures.]

  Description [Returns the number of Kbytes allocated; 0 in case of failure.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int mg_localMemManagerStart( 
  DdManager * dd,
  DdNode * Set)
{
  size_t MemSize;
  
  nodeAlignCount = (size_t) floor ((double) (getpagesize()) /
				   ((double) (sizeof( nodeinfo ))));
  
  s_pMemory = mg_localMemManagerLinkNext( );
  if ( s_pMemory == NULL )
    return 0;
  
  s_pMemBlockHead = s_pMemBlockCurr;
  
  MemSize = (s_nLinksAlloc * sizeof( nodeinfo )) + sizeof(memblock);

  /* prepare the iterator */
  s_pLinkedList = s_pMemory;
  s_nLinksReturned = 0;

  return MemSize/1024 + (MemSize%1024 > 0);
}

/**Function********************************************************************

  Synopsis [Returns the next node info structure.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static nodeinfo * mg_localMemManagerLinkNext( void )
{  
  if ((NULL == s_pMemBlockCurr) ||
      (NULL == s_pMemBlockCurr->nextnode) ||
      (NULL == s_pMemBlockCurr->nextnode->next) ||
      (NULL == s_pMemBlockCurr->nextnode->next->next) )
    {
      if (NULL == mg_localMemManagerAddLinks())
	{
	  return (NULL);
	}
    }

  s_pLinkedList = s_pMemBlockCurr->nextnode;
  
  if (s_pMemBlockCurr->tailnode == s_pMemBlockCurr->nextnode)
    {
      s_pMemBlockCurr = s_pMemBlockCurr->next;
    }
  else
    {      
      s_pMemBlockCurr->nextnode = s_pMemBlockCurr->nextnode->next;
    }
  
  s_nLinksReturned++;
  return (s_pLinkedList);
}

static nodeinfo * mg_localMemManagerAddLinks( void )
{
  memblock * newMemBlock = (memblock *) malloc(sizeof(memblock));
  nodeinfo * newNodes  = (nodeinfo *) malloc (nodeAlignCount *
					      sizeof( nodeinfo ));
  if((NULL == newMemBlock) || (NULL == newNodes))
    {
      return (NULL);
    }
  memset(newMemBlock, 0, sizeof(memblock));
  memset(newNodes, 0, nodeAlignCount *
	 sizeof( nodeinfo ) );
  
  /* connect all links into a linked list */
  nodeinfo * pTemp = newNodes;
  int i = 0;
  for ( i = 0; i < nodeAlignCount - 1; ++i )
    {
	  pTemp->next = pTemp + 1;
	  pTemp = pTemp->next;
    }
  pTemp->next = NULL;
  
  newMemBlock->headnode = newNodes;
  newMemBlock->tailnode = pTemp;     
  newMemBlock->nextnode = newNodes; 
  newMemBlock->next = NULL;     
  
  // connect to the memory block linked list
  if(NULL != s_pMemBlockCurr)
    {
      memblock * tempBlock = s_pMemBlockCurr;
      while(NULL != tempBlock->next)
	{
	  tempBlock = tempBlock->next;
	}
      
      tempBlock->tailnode->next = newMemBlock->headnode;
      tempBlock->next = newMemBlock;
    }
  else
    {
      s_pMemBlockCurr = newMemBlock;
    }
  
  s_nLinksAlloc += nodeAlignCount;
  
  return(newNodes);
}


/**Function********************************************************************

  Synopsis [Releases the allocated memory.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void mg_localMemManagerDissolve( void )
{
  memblock * tempHead = s_pMemBlockHead;

  while(NULL != tempHead)
    {
      free(s_pMemBlockHead->headnode);
      tempHead = s_pMemBlockHead->next;
      free(s_pMemBlockHead);
      s_pMemBlockHead = tempHead;
    }
}


static int mg_MaxNodeDepth(DdManager *manager, DdNode * node)
{
  int returnValue = 0;
  int varIndex = 0;

  // Check for the constant
  if(node == manager->one)
    {
      return 0;      
    }

  //! Determine the variable that this node represents
  varIndex = node->index;
  
  //! Normalize this variable for a 64 bit checks
  if(varIndex > 63)
    {
      varIndex = varIndex - 64;
    }
  
  if((mg_maxDepth != 0) && (varIndex < mg_maxDepth))
    {
      returnValue = 1;
    }
  else
    {
      returnValue = 0;
    }

  return(returnValue);
}
