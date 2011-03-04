/**CFile***********************************************************************

  FileName    [ddiVararray.c]

  PackageName [ddi]

  Synopsis    [Functions to manage <em>arrays of variables</em>]

  Description [Array of variables is implemented as an array of BDD nodes
               (called "projection funtions" in CUDD package).<BR> 
               The basic manipulation allowed for variable arrays are:<BR> 
               <OL>
               <LI><B>Alloc</B> and <B>free</B> an array
               <LI><B>Insert</B> and <B>fetch</B> a variable
               <LI><B>Duplicate</B> an array
               <LI><B>Join</B> and <B>append</B> a new array
               <LI>Convert into array of integer
               <LI>Convert into a set of variables
               </OL>]

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
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************
  Synopsis    [Generate a variable array from CU vars (BDD nodes)]
  Description [No variable dup is done (as all DDI operations working with
               variables)]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayToCU]
*****************************************************************************/
Ddi_Vararray_t *
Ddi_VararrayMakeFromCU (
  Ddi_Mgr_t *mgr,
  DdNode **array,
  int n
)
{
  Ddi_Vararray_t *data;
  int i;

  data = Ddi_VararrayAlloc(mgr,n);
  for (i=0; i<n; i++)
    Ddi_VararrayWrite(data,i,Ddi_VarFromCU(mgr,array[i])); 

  return data;
}

/**Function*******************************************************************
  Synopsis    [Generate an array of pointers to CUDD variables] 
  Description [Generate a dynamic allocated array of pointers to CUDD BDDs 
              representing variables in input array.]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayMakeFromCU]
*****************************************************************************/
DdNode **
Ddi_VararrayToCU (
  Ddi_Vararray_t *array )
{
  DdNode **vett;
  int i;

  if (array==NULL)
    return NULL;

  vett = Pdtutil_Alloc(DdNode *,Ddi_VararrayNum(array));

  for(i=0;i<Ddi_VararrayNum(array);i++) {
    vett[i] = Ddi_VarToCU(Ddi_VararrayRead(array,i));
  } 

  return vett;
}

/**Function*******************************************************************
  Synopsis    [Generate a variable array from array of integer indexes]
  Description [Integer indexes are used as CUDD indexes.
               No variable dup is done (as all DDI operations working with
               variables)]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayToCU]
*****************************************************************************/
Ddi_Vararray_t *
Ddi_VararrayMakeFromInt (
  Ddi_Mgr_t *mgr,
  int *array,
  int n
)
{
  Ddi_Vararray_t *data;
  int i;

  data = Ddi_VararrayAlloc(mgr,n);
  for (i=0; i<n; i++)
    Ddi_VararrayWrite(data,i,Ddi_IthVar(mgr,array[i])); 

  return data;
}

/**Function*******************************************************************
  Synopsis    [Generate an array of integer variable indexes] 
  Description [Generate a dynamically allocated array of integer variable 
    indexes. Integer indexes are taken from CUDD indexes.]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayToCU]
*****************************************************************************/
int *                    /* array of integer */
Ddi_VararrayToInt (
  Ddi_Vararray_t *array  /* array of variables */ )
{
  int i;
  int *vett;

  vett = Pdtutil_Alloc(int,Ddi_VararrayNum(array));

  for (i=0; i<Ddi_VararrayNum(array); i++) {
    vett[i] = Ddi_VarIndex(Ddi_VararrayRead(array,i));
  }

 return vett;
}

/**Function*******************************************************************
  Synopsis    [Allocate a new array of variables of given length]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Vararray_t *
Ddi_VararrayAlloc (
  Ddi_Mgr_t *mgr     /* DDI manager */,
  int size           /* array length */
)
{
  Ddi_Vararray_t *array;

  array = (Ddi_Vararray_t *)DdiGenericAlloc(Ddi_Vararray_c,mgr);
  array->array = DdiArrayAlloc(size);

  return (array);
}

/**Function*******************************************************************
  Synopsis    [Return the number of variables (entries) in array]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
int
Ddi_VararrayNum(
  Ddi_Vararray_t *array
)
{
  DdiConsistencyCheck(array,Ddi_Vararray_c);
  return (DdiArrayNum(array->array));
}


/**Function*******************************************************************
  Synopsis    [Write a variable in array at given position]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayRead]
******************************************************************************/
void
Ddi_VararrayWrite(
  Ddi_Vararray_t *array    /* array of variables */,
  int pos                  /* position of new element */,
  Ddi_Var_t *var           /* variable to be inserted */ )
{
  DdiConsistencyCheck(array,Ddi_Vararray_c);
  DdiArrayWrite(array->array,pos,(Ddi_Generic_t *)var,Ddi_Mov_c);
}

/**Function*******************************************************************
  Synopsis    [Return the variable at i-th position in array]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayInsert]
******************************************************************************/
Ddi_Var_t *
Ddi_VararrayRead (
  Ddi_Vararray_t *array    /* variable array */,
  int i                    /* position */ )
{
  DdiConsistencyCheck(array,Ddi_Vararray_c);
  return ((Ddi_Var_t *)DdiArrayRead(array->array,i));
}


/**Function*******************************************************************
  Synopsis    [clear array at given position (variable is replaced by NULL)]
  Description [clear array at given position (variable is replaced by NULL)]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayRead]
******************************************************************************/
void
Ddi_VararrayClear(
  Ddi_Vararray_t *array    /* array of variables */,
  int pos                  /* position of element to be cleared */
)
{
  DdiConsistencyCheck(array,Ddi_Vararray_c);
  DdiArrayWrite(array->array,pos,NULL,Ddi_Mov_c);
}

/**Function*******************************************************************
  Synopsis    [Insert a variable in array at given position]
  Description [Insert a variable in array at given position. 
    Following entries are shifted down.]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayExtract]
******************************************************************************/
void
Ddi_VararrayInsert(
  Ddi_Vararray_t *array    /* array of variables */,
  int pos                  /* position of new element */,
  Ddi_Var_t *v             /* variable to be written */ )
{
  DdiConsistencyCheck(array,Ddi_Vararray_c);
  DdiArrayInsert(array->array,pos,(Ddi_Generic_t *)v,Ddi_Dup_c);
}

/**Function*******************************************************************
  Synopsis    [Insert a variable in array at last (new) position]
  Description [Insert a variable in array at last (new) position] 
  SideEffects [none]
  SeeAlso     [Ddi_VararrayExtract]
******************************************************************************/
void
Ddi_VararrayInsertLast(
  Ddi_Vararray_t *array    /* array of variables */,
  Ddi_Var_t *v             /* variable to be written */ )
{
  DdiConsistencyCheck(array,Ddi_Vararray_c);
  Ddi_VararrayInsert(array,Ddi_VararrayNum(array),v);
}

/**Function*******************************************************************
  Synopsis    [Extract the variable at i-th position in array]
  Description [Extract the variable at i-th position in array. 
    The extracted variable is removed from the array and the following entries
    are shifted up.]
  SideEffects []
  SeeAlso     [Ddi_VararrayInsert]
******************************************************************************/
Ddi_Var_t *
Ddi_VararrayExtract (
  Ddi_Vararray_t *array    /* variable array */,
  int i                    /* position */ )
{
  DdiConsistencyCheck(array,Ddi_Vararray_c);
  return ((Ddi_Var_t *)DdiArrayExtract(array->array,i));
}


/**Function*******************************************************************
  Synopsis    [Remove array entry at given position]
  Description [Remove array entry at given position.
    This operation is equivalent to extract (but return type is void).]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayExtract Ddi_VararrayClear]
******************************************************************************/
void
Ddi_VararrayRemove(
  Ddi_Vararray_t *array    /* array of variables */,
  int pos                  /* position of element to be cleared */
)
{
  Ddi_Var_t *tmp;

  DdiConsistencyCheck(array,Ddi_Vararray_c);
  tmp = Ddi_VararrayExtract(array,pos);
}


/**Function*******************************************************************
  Synopsis    [Duplicate an array of variables]
  Description [Only the "array" part is duplicated. Variables are never 
               duplicated nor freed, except when closing the owner manager]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayAlloc]
******************************************************************************/
Ddi_Vararray_t *
Ddi_VararrayDup(
  Ddi_Vararray_t *old  /* array to be duplicated */ 
)
{
  Ddi_Vararray_t *newa;

  DdiConsistencyCheck(old,Ddi_Vararray_c);
  newa = (Ddi_Vararray_t *)DdiGenericDup((Ddi_Generic_t *)old);

  return (newa);
}

/**Function*******************************************************************
  Synopsis    [Copy an array of variables to a destination maneger]
  Description [Variable correspondence is established "by index", i.e. 
               variables with same index in different manager correspond]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayDup]
******************************************************************************/
Ddi_Vararray_t *
Ddi_VararrayCopy (
  Ddi_BddMgr *ddm          /* dd Manager */,
  Ddi_Vararray_t  *old    /* array of variables */
)
{
  Ddi_Vararray_t *newa;

  DdiConsistencyCheck(old,Ddi_Vararray_c);
  newa = (Ddi_Vararray_t *)DdiGenericCopy(ddm,(Ddi_Generic_t *)old,NULL,NULL);

  return (newa);
}

/**Function*******************************************************************
  Synopsis    [Append the elements of array2 at the end of array1]
  SideEffects [none]
  SeeAlso     [Ddi_VararrayJoin]
******************************************************************************/
void
Ddi_VararrayAppend (
  Ddi_Vararray_t *array1   /* first array */,
  Ddi_Vararray_t *array2   /* array to be appended */
)
{
  DdiConsistencyCheck(array1,Ddi_Vararray_c);
  DdiConsistencyCheck(array2,Ddi_Vararray_c);
  DdiArrayAppend(array1->array,array2->array);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                          */
/*---------------------------------------------------------------------------*/




