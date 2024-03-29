/**CFile***********************************************************************

  FileName    [ddiVarsetarray.c]

  PackageName [ddi]

  Synopsis    [Functions to manage <em>arrays of Varsetss</em>]

  Description []

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
  Synopsis    [Allocate a new array of varsets]
  Description [Allocate a new array of varsets. The array slots are initialized 
    with NULL pointers, so further Write operations are required.]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayWrite]
******************************************************************************/
Ddi_Varsetarray_t *
Ddi_VarsetarrayAlloc (
  Ddi_Mgr_t *mgr     /* DDI manager */,
  int length  /* array length */
)
{
  Ddi_Varsetarray_t *array;

  array = (Ddi_Varsetarray_t *)DdiGenericAlloc(Ddi_Varsetarray_c,mgr);
  array->array = DdiArrayAlloc(length);

  return (array);
}


/**Function*******************************************************************
  Synopsis    [Return the number of entries in array]
  Description [Return the number of entries in array]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
int
Ddi_VarsetarrayNum(
  Ddi_Varsetarray_t *array
)
{
  DdiConsistencyCheck(array,Ddi_Varsetarray_c);
  return (DdiArrayNum(array->array));
}

/**Function*******************************************************************
  Synopsis    [Write varset in array at given position]
  Description [Write varset in array at given position. Previous non NULL entry
    is freed. The written varset is duplicated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayRead]
******************************************************************************/
void
Ddi_VarsetarrayWrite(
  Ddi_Varsetarray_t *array    /* array of varsets */,
  int pos                  /* position of new element */,
  Ddi_Varset_t *vs           /* varset to be written */ )
{
  DdiConsistencyCheck(array,Ddi_Varsetarray_c);
  DdiArrayWrite(array->array,pos,(Ddi_Generic_t *)vs,Ddi_Dup_c);
}


/**Function*******************************************************************
  Synopsis    [Insert varset in array at given position]
  Description [Insert varset in array at given position. 
    Previous non NULL entry is moved up. The written varset is duplicated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayRead]
******************************************************************************/
void
Ddi_VarsetarrayInsert(
  Ddi_Varsetarray_t *array    /* array of varsets */,
  int pos                  /* position of new element */,
  Ddi_Varset_t *vs           /* varset to be written */ )
{
  DdiConsistencyCheck(array,Ddi_Varsetarray_c);
  DdiArrayInsert(array->array,pos,(Ddi_Generic_t *)vs,Ddi_Dup_c);
}

/**Function*******************************************************************
  Synopsis    [Insert varset in array at last position]
  Description [Insert varset in array at last position. 
    The written varset is duplicated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayInsert]
******************************************************************************/
void
Ddi_VarsetarrayInsertLast(
  Ddi_Varsetarray_t *array    /* array of varsets */,
  Ddi_Varset_t *vs           /* varset to be written */ )
{
  Ddi_VarsetarrayInsert(array,Ddi_VarsetarrayNum(array),vs);
}

/**Function*******************************************************************
  Synopsis    [Read varset at i-th position in array]
  Description [Read varset at i-th position in array. As all read operations
    no data duplication is done, so the returned varset should be duplicated
    if further manipulations are required on it.]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayWrite]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetarrayRead (
  Ddi_Varsetarray_t *array    /* varset array */,
  int i                    /* position */ )
{
  DdiConsistencyCheck(array,Ddi_Varsetarray_c);
  return ((Ddi_Varset_t *)DdiArrayRead(array->array,i));
}


/**Function*******************************************************************
  Synopsis    [clear array at given position (BDD freed and replaced by NULL)]
  Description [clear array at given position (BDD freed and replaced by NULL)]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayRead]
******************************************************************************/
void
Ddi_VarsetarrayClear(
  Ddi_Varsetarray_t *array    /* array of varsets */,
  int pos                  /* position of element to be cleared */
)
{
  DdiConsistencyCheck(array,Ddi_Varsetarray_c);
  DdiArrayWrite(array->array,pos,NULL,Ddi_Mov_c);
}


/**Function*******************************************************************
  Synopsis    [Duplicate an array of Varsets]
  Description [Duplicate an array of Varsets]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayAlloc]
******************************************************************************/
Ddi_Varsetarray_t *
Ddi_VarsetarrayDup(
  Ddi_Varsetarray_t *old  /* array to be duplicated */ 
)
{
  Ddi_Varsetarray_t *newa;

  DdiConsistencyCheck(old,Ddi_Varsetarray_c);
  newa = (Ddi_Varsetarray_t *)DdiGenericDup((Ddi_Generic_t *)old);

  return (newa);
}


/**Function*******************************************************************
  Synopsis    [Copy an array of varsets to a destination manager]
  Description [Copy an array of varsets to a destination manager.
    Variable correspondence is established "by index", i.e. 
    variables with same index in different manager correspond]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetarrayDup]
******************************************************************************/
Ddi_Varsetarray_t *
Ddi_VarsetarrayCopy (
  Ddi_BddMgr *ddm          /* dd Manager */,
  Ddi_Varsetarray_t  *old    /* array of varsets */
)
{
  Ddi_Varsetarray_t *newa;

  DdiConsistencyCheck(old,Ddi_Varsetarray_c);
  newa = (Ddi_Varsetarray_t *)DdiGenericCopy(ddm,(Ddi_Generic_t *)old,
    NULL,NULL);

  return (newa);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
