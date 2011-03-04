/**CFile***********************************************************************

  FileName    [ddiBddarray.c]

  PackageName [ddi]

  Synopsis    [Functions to manage <em>arrays of BDDs</em>]

  Description [The basic manipulation allowed for BDD arrays are:<BR> 
    <OL>
    <LI>alloc, make, free arrays
    <LI>write, read, clear, insert, extract, remove array entries
    <LI>duplicate, copy, append
    <LI>store and load to/from file
    </OL>
    For each element of array can be apply any boolean operator
    as AND,OR,XOR,XNOR,Restrict and Constrain.
    ]

  SeeAlso     []  

  Author    [Gianpiero Cabodi and Stefano Quer]

  Copyright [  Copyright (c) 2001 by Politecnico di Torino.
    All Rights Reserved. This software is for educational purposes only.
    Permission is given to academic institutions to use, copy, and modify
    this software and its documentation provided that this introductory
    message is not removed, that this software and its documentation is
    used for the institutions' internal research and educational purposes,
    and that no monies are exchanged. No guarantee is expressed or implied
    by the distribution of this code.
    Send bug-reports and/or questions to: {cabodi,quer}@polito.it. ]
  
  Revision    []

******************************************************************************/

#include "ddiInt.h"

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


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of internal function                                           */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*
 *  Conversion, creation (make) and release (free) functions
 */

/**Function*******************************************************************
  Synopsis    [Allocate a new array of BDDs]
  Description [Allocate a new array of BDDs. The array slots are initialized 
    with NULL pointers, so further Write operations are required.]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayWrite]
******************************************************************************/
Ddi_Bddarray_t *
Ddi_BddarrayAlloc (
  Ddi_Mgr_t *mgr     /* DDI manager */,
  int length  /* array length */
)
{
  Ddi_Bddarray_t *array;

  array = (Ddi_Bddarray_t *)DdiGenericAlloc(Ddi_Bddarray_c,mgr);
  array->array = DdiArrayAlloc(length);

  return (array);
}

/**Function*******************************************************************
  Synopsis    [Generate a BDD array from CUDD BDDs]
  Description [Generate a BDD array from CUDD BDDs.
    The function allocates a Ddi_Bddarray_t structure, then write 
    monolithic components to proper array slots.]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayToCU]
*****************************************************************************/
Ddi_Bddarray_t *
Ddi_BddarrayMakeFromCU (
  Ddi_Mgr_t *mgr,
  DdNode **array,
  int n
)
{
  Ddi_Bddarray_t *data;
  Ddi_Bdd_t *tmp;
  int i;

  Pdtutil_Assert((mgr!=NULL)&&(array!=NULL),
    "NULL manager or BDD when generating DDI node");
  Pdtutil_Assert((n>0),
    "generating array of size 0");

  data = Ddi_BddarrayAlloc(mgr,n);
  for (i=0; i<n; i++) {
    tmp = Ddi_BddMakeFromCU(mgr,array[i]); 
    Ddi_BddarrayWrite(data,i,tmp);
    Ddi_Free(tmp); 
  }

  return(data);
}

/**Function*******************************************************************
  Synopsis    [Generate an array of pointers to CUDD nodes] 
  Description [Generate a dynamically allocated array of pointers to CUDD 
    BDDs, one for each entry in the DDI array. Array entries are required
    to be monolithic.
    The number of array entries is equal to Ddi_BddarrayNum(array), but the 
    array is overdimensioned (by one NULL slot) to make it NULL-terminated.
    The array of pointers is allocated (so explicit free is required), whereas
    the CUDD nodes are NOT referenced.]  
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayMakeFromCU]
*****************************************************************************/
DdNode **
Ddi_BddarrayToCU (
  Ddi_Bddarray_t *array 
)
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  return (DdiArrayToCU(array->array));
}

/**Function*******************************************************************
  Synopsis    [Generate a BDD array from partitions of partitioned BDD]
  Description [Generate a BDD array from partitions of partitioned BDD]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayToCU]
*****************************************************************************/
Ddi_Bddarray_t *
Ddi_BddarrayMakeFromBddPart (
  Ddi_Bdd_t *part
)
{
  Ddi_Mgr_t *mgr;
  Ddi_Bddarray_t *array;
  int i, n;

  DdiConsistencyCheck(part,Ddi_Bdd_c);
  Pdtutil_Assert(
    (Ddi_ReadCode(part) == Ddi_Bdd_Part_Conj_c) ||
    (Ddi_ReadCode(part) == Ddi_Bdd_Part_Disj_c),
    "Partitioned BDD required to generate array of partitions!");

  mgr = Ddi_ReadMgr(part);
  n = Ddi_BddPartNum(part);
  array = Ddi_BddarrayAlloc(mgr,n);

  for (i=0; i<n; i++) {
    Ddi_BddarrayWrite(array,i,Ddi_BddPartRead(part,i)); 
  }

  return(array);
}

/**Function*******************************************************************
  Synopsis    [Return the number of BDDs (entries) in array]
  Description [Return the number of BDDs (entries) in array]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
int
Ddi_BddarrayNum(
  Ddi_Bddarray_t *array
)
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  return (DdiArrayNum(array->array));
}

/**Function*******************************************************************
  Synopsis    [Write a BDD in array at given position]
  Description [Write a BDD in array at given position. Previous non NULL entry
    is freed. The written BDD (f) is duplicated]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayRead]
******************************************************************************/
void
Ddi_BddarrayWrite(
  Ddi_Bddarray_t *array    /* array of BDDs */,
  int pos                  /* position of new element */,
  Ddi_Bdd_t *f           /* BDD to be written */ )
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  DdiArrayWrite(array->array,pos,(Ddi_Generic_t *)f,Ddi_Dup_c);
}

/**Function*******************************************************************
  Synopsis    [Read the BDD at i-th position in array]
  Description [Read the BDD at i-th position in array. As all read operations
    no data duplication is done, so the returned BDD should be duplicated
    if further manipulations are required on it.]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayWrite]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddarrayRead (
  Ddi_Bddarray_t *array    /* BDD array */,
  int i                    /* position */ )
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  return ((Ddi_Bdd_t *)DdiArrayRead(array->array,i));
}


/**Function*******************************************************************
  Synopsis    [clear array at given position (BDD freed and replaced by NULL)]
  Description [clear array at given position (BDD freed and replaced by NULL)]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayRead]
******************************************************************************/
void
Ddi_BddarrayClear(
  Ddi_Bddarray_t *array    /* array of BDDs */,
  int pos                  /* position of element to be cleared */
)
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  DdiArrayWrite(array->array,pos,NULL,Ddi_Mov_c);
}

/**Function*******************************************************************
  Synopsis    [Insert a BDD in array at given position]
  Description [Insert a BDD in array at given position. 
    Following entries are shifted down.
    The written BDD (f) is duplicated]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayExtract]
******************************************************************************/
void
Ddi_BddarrayInsert(
  Ddi_Bddarray_t *array    /* array of BDDs */,
  int pos                  /* position of new element */,
  Ddi_Bdd_t *f           /* BDD to be written */ )
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  DdiArrayInsert(array->array,pos,(Ddi_Generic_t *)f,Ddi_Dup_c);
}

/**Function*******************************************************************
  Synopsis    [Insert a BDD in array at last (new) position]
  Description [Insert a BDD in array at last (new) position] 
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayExtract]
******************************************************************************/
void
Ddi_BddarrayInsertLast(
  Ddi_Bddarray_t *array    /* array of BDDs */,
  Ddi_Bdd_t *f           /* BDD to be written */ )
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  Ddi_BddarrayInsert(array,Ddi_BddarrayNum(array),f);
}

/**Function*******************************************************************
  Synopsis    [Extract the BDD at i-th position in array]
  Description [Extract the BDD at i-th position in array. 
    The extracted BDD is removed from the array and the following entries
    are shifted up.]
  SideEffects []
  SeeAlso     [Ddi_BddarrayInsert]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddarrayExtract (
  Ddi_Bddarray_t *array    /* BDD array */,
  int i                    /* position */ )
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  return ((Ddi_Bdd_t *)DdiArrayExtract(array->array,i));
}


/**Function*******************************************************************
  Synopsis    [Remove array entry at given position]
  Description [Remove array entry at given position.
    This operation is equivalent to extract + free of extracted BDD.]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayExtract Ddi_BddarrayClear]
******************************************************************************/
void
Ddi_BddarrayRemove(
  Ddi_Bddarray_t *array    /* array of BDDs */,
  int pos                  /* position of element to be cleared */
)
{
  Ddi_Bdd_t *tmp;

  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  tmp = Ddi_BddarrayExtract(array,pos);
  Ddi_Free(tmp);
}

/**Function*******************************************************************
  Synopsis    [Duplicate an array of BDDs]
  Description [Duplicate an array of BDDs]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayAlloc]
******************************************************************************/
Ddi_Bddarray_t *
Ddi_BddarrayDup(
  Ddi_Bddarray_t *old  /* array to be duplicated */ 
)
{
  Ddi_Bddarray_t *newa;

  DdiConsistencyCheck(old,Ddi_Bddarray_c);
  newa = (Ddi_Bddarray_t *)DdiGenericDup((Ddi_Generic_t *)old);

  return (newa);
}


/**Function*******************************************************************
  Synopsis    [Copy an array of BDDs to a destination manager]
  Description [Copy an array of BDDs to a destination manager.
    Variable correspondence is established "by index", i.e. 
    variables with same index in different manager correspond]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayDup]
******************************************************************************/
Ddi_Bddarray_t *
Ddi_BddarrayCopy (
  Ddi_BddMgr *ddm          /* dd Manager */,
  Ddi_Bddarray_t  *old    /* array of BDDs */
)
{
  Ddi_Bddarray_t *newa;

  DdiConsistencyCheck(old,Ddi_Bddarray_c);
  newa = (Ddi_Bddarray_t *)DdiGenericCopy(ddm,(Ddi_Generic_t *)old,NULL,NULL);

  return (newa);
}

/**Function*******************************************************************
  Synopsis    [Append the elements of array2 at the end of array1]
  Description [Append the elements of array2 at the end of array1. As all
    array write/insert operations, new entries are duplicated.]
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayWrite Ddi_BddarrayInsert]
******************************************************************************/
void
Ddi_BddarrayAppend (
  Ddi_Bddarray_t *array1   /* first array */,
  Ddi_Bddarray_t *array2   /* array to be appended */
)
{
  DdiConsistencyCheck(array1,Ddi_Bddarray_c);
  DdiConsistencyCheck(array2,Ddi_Bddarray_c);
  DdiArrayAppend(array1->array,array2->array);
}


/**Function*******************************************************************
  Synopsis    [Return the number of BDD nodes in a BDD array]
  Description [Count the  numbers of BDD nodes in a BDD array. Shared
    nodes are counted once.]
  SideEffects [none]
  SeeAlso     [Ddi_BddSize]
*****************************************************************************/
int
Ddi_BddarraySize (
  Ddi_Bddarray_t *array
)
{
  DdiConsistencyCheck(array,Ddi_Bddarray_c);
  return (DdiGenericBddSize((Ddi_Generic_t *)array));
}

/**Function*******************************************************************

  Synopsis    [Writes array of BDDs in a dump file]

  Description [This function stores a BDD array using the
               DDDMP format. The parameter "mode" 
               can be DDDMP_MODE_TEXT, DDDMP_MODE_COMPRESSED or
               DDDMP_MODE_AUTOMATIC.<br>
               The function returns 1 if succefully stored, 0 otherwise.]

  SideEffects [none]

  SeeAlso     [Dddmp_cuddBddStore Ddi_BddStore]

*****************************************************************************/
int
Ddi_BddarrayStore (
  Ddi_Bddarray_t *array    /* array to be stored */,
  char *ddname            /* dd name (or NULL) */,
  char **vnames           /* array of variable names (or NULL) */,
  char **rnames           /* array of root names (or NULL) */,
  int *vauxids             /* array of aux var ids (or NULL) */,
  int mode                /* storing mode selector */,
  char *fname             /* file name */,
  FILE *fp                /* pointer to the store file */ )
{
  return (DdiArrayStore (array->array,ddname,vnames,
    rnames,vauxids,mode,fname,fp));
}


/**Function*******************************************************************

  Synopsis    [Reads array of BDDs from a dump file]

  Description [This function loads a BDDs'array.<br>
               The BDD on file must be in the DDDMP format. The parameter 
               "mode" can be DDDMP_MODE_TEXT, DDDMP_MODE_COMPRESSED or
               DDDMP_MODE_AUTOMATIC.<br>
               The function returns the pointer of array if succefully
               loaded, NULL otherwise.]

  SideEffects [none]

  SeeAlso     [Dddmp_cuddBddLoad,Ddi_BddLoad]

*****************************************************************************/

Ddi_Bddarray_t *
Ddi_BddarrayLoad (
  Ddi_BddMgr *dd           /* dd manager */,
  char **vnames           /* variable names */,
  int *vauxids            /* variable auxids */,
  int mode                /* storing mode selector */,
  char *file              /* name file */,
  FILE *fp                /* file pointer */ )             
{
  DdNode   **roots;        /* array of BDD roots to be loaded */
  Ddi_Bddarray_t *array;    /* descriptor of array */
  int nroots,              /* number of BDD roots */
      i;
 
  nroots = Dddmp_cuddBddArrayLoad(dd->mgrCU,DDDMP_ROOT_MATCHLIST,NULL,
             DDDMP_VAR_MATCHNAMES,vnames,vauxids,NULL,
             mode,file,fp,&roots);

  if (nroots<=0)
    return (NULL);

  array = Ddi_BddarrayMakeFromCU(dd,roots,nroots);

  for (i=0;i<nroots;i++)
    Cudd_RecursiveDeref(dd->mgrCU,roots[i]);

  Pdtutil_Free(roots);   

  return (array); 
}


/**Function*******************************************************************
  Synopsis    [Return the support of a BDD array]
  Description [Returns a var-set representing the global support of the array]
  SideEffects []
  SeeAlso     []
*****************************************************************************/
Ddi_Varset_t *
Ddi_BddarraySupp (
  Ddi_Bddarray_t *array   /* BDDs'array */ )
{
  return((Ddi_Varset_t *)DdiGenericOp1(Ddi_BddSupp_c,Ddi_Generate_c,array));
} 



/**Function*******************************************************************
  Synopsis    [OLD(pdt-1). Return a vector of supports of BDD array elements]
  Description [OLD(pdt-1). Return a vector of supports of BDD array elements.
    Should be replaced by Varsetarray usage]
  SideEffects [none]
  SeeAlso     []
*****************************************************************************/

Ddi_Varset_t **
Ddi_BddarraySuppArray (
  Ddi_Bddarray_t *fArray   /* array of function */ )
{
  Ddi_Varset_t **fArraySupp;  /* array of function supports */
  Ddi_Bdd_t *F;              /* function */
  Ddi_Varset_t *S;        /* support of function */
  int i,n;                    

  n = Ddi_BddarrayNum(fArray);    /* length of array */

  fArraySupp = Pdtutil_Alloc(Ddi_Varset_t *,n);

  for (i=0;i<n;i++) {
    F = Ddi_BddarrayRead(fArray,i);
    S = Ddi_BddSupp(F);
    fArraySupp[i]=S;
  }

  return (fArraySupp); 
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
