/**CFile***********************************************************************

  FileName    [ddiArray.c]

  PackageName [ddi]

  Synopsis    [Internal Functions for <em>array manipulations</em>]

  Description [Arrays of ddi objects (Boolean functions, variables, ...)
    are implemented using the generic ddi
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

static void ArrayResize(Ddi_ArrayData_t *array, int nSizeNew);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************
  Synopsis    [Allocate a new array]
  Description [Array allocation and initialization.]
  SideEffects [none]
  SeeAlso     [Ddi_Free]
******************************************************************************/

Ddi_ArrayData_t *
DdiArrayAlloc (
  int length        /* array length */ 
)
{
  Ddi_ArrayData_t *array;
  int i;

  array = Pdtutil_Alloc(Ddi_ArrayData_t,1);

  array->num = length;
  array->nSize = DDI_MAX(length, DDI_ARRAY_INIT_SIZE);
  array->data = Pdtutil_Alloc(Ddi_Generic_t *, array->nSize);

  for (i=0;i<array->nSize;i++)
    array->data[i] = NULL;

  return array;
}


/**Function*******************************************************************
  Synopsis    [Frees a DDI array]
  Description [Frees a DDI array. Array entries are recursively freed.]
  SideEffects [none]
  SeeAlso     [DdiArrayAlloc]
******************************************************************************/
void
DdiArrayFree (
  Ddi_ArrayData_t *array   /* array to be freed */ 
)
{
  int i;

  if (array==NULL)
    return;

  for (i=0; i<array->num; i++) {
    if (array->data[i]!=NULL) {
      if (DdiType(array->data[i])!=Ddi_Var_c) {
        Ddi_Unlock(array->data[i]);
        DdiGenericFree(array->data[i]);
      }
      else if (array->data[i]->common.status==Ddi_Unlocked_c) {
        DdiGenericFree(array->data[i]);
      }
    }
  }

  Pdtutil_Free(array->data);
  Pdtutil_Free(array);
}


/**Function*******************************************************************
  Synopsis    [Generate an array of pointers to CUDD nodes] 
  Description [Generate a dynamically allocated array of pointers to CUDD 
    BDDs, one for each entry in the DDI array. Array entries are required
    to be monolithic.
    The number of array entries is equal to DdiArrayNum(array), but the 
    array is overdimensioned (by one NULL slot) to make it NULL-terminated.
    The array of pointers is allocated (so explicit free is required), whereas
    the CUDD nodes are NOT referenced.] 
  SideEffects [none]
  SeeAlso     [Ddi_BddarrayMakeFromCU]
*****************************************************************************/
DdNode **
DdiArrayToCU (
  Ddi_ArrayData_t *array 
)
{
  DdNode **vett;
  Ddi_Generic_t *f;
  int i,n;

  n = DdiArrayNum(array);
  vett = Pdtutil_Alloc(DdNode *,n+1);

  for(i=0;i<n;i++) {
    f = DdiArrayRead(array,i);
    DdiConsistencyCheck(f,Ddi_Bdd_c);
    vett[i] = Ddi_BddToCU((Ddi_Bdd_t*)f);
  }
  vett[n] = NULL;

  return (vett);
}

/**Function*******************************************************************
  Synopsis    [Write in array at i-th position]
  Description [Write <em>f</em> in <em>array</em> at <em>i</em>-th
               position. The inserted item is duplicated.
               No shift is done. The previous content of array
               is freed (if not NULL) and overwritten.]
  SideEffects [none]
  SeeAlso     [DdiArrayInsert]
******************************************************************************/

void
DdiArrayWrite (
  Ddi_ArrayData_t *array        /* destination array */,
  int i                         /* position of new element */,
  Ddi_Generic_t *f              /* generic object to be written */,
  Ddi_CopyType_e copy           /* dup/mov selector */
)
{
  Pdtutil_Assert(array!=NULL,"accessing a NULL Array");
  Pdtutil_Assert(i>=0,"Array access out of bounds");

  if (i >= array->nSize) {
    ArrayResize(array, i+1);
  }
  if (i >= array->num) {
    array->num = i+1;
  }

  if ((array->data[i]!=NULL)&&(DdiType(array->data[i])!=Ddi_Var_c)) {
    Ddi_Unlock(array->data[i]);
    DdiGenericFree(array->data[i]);
  }
  switch (copy) {
  case Ddi_Dup_c:
    array->data[i] = DdiGenericDup(f);
    break;
  case Ddi_Mov_c:
    array->data[i] = f;
    break;
  }
  Ddi_Lock(array->data[i]);
}

/**Function*******************************************************************
  Synopsis    [Insert in array at i-th position]
  Description [Insert <em>f</em> in <em>array</em> at <em>i</em>-th
               position. The inserted item is duplicated and the 
               <em>array</em> slots from <em>i</em> to the end are
               shifted to make room for the new element.]
  SideEffects [none]
  SeeAlso     [DdiArrayWrite]
******************************************************************************/

void
DdiArrayInsert(
  Ddi_ArrayData_t *array        /* destination array */,
  int i                         /* position of new element */,
  Ddi_Generic_t *f              /* generic object to be inserted */,
  Ddi_CopyType_e copy           /* dup/mov selector */
)
{
  int j;

  Pdtutil_Assert(array!=NULL,"accessing a NULL Array");
  Pdtutil_Assert(i>=0,"Array access out of bounds");

  /* insert after last element */
  if (i >= array->num) {
    DdiArrayWrite(array,i,f,copy);
    return;
  }
  /* no room for new element. Resize */
  if (array->num==array->nSize) {
    ArrayResize(array, array->num+1);
  }

  array->num++;

  /* shift elements after i-th position */
  for (j=array->num-1;j>=i+1;j--)
    array->data[j]=array->data[j-1];

  switch (copy) {
  case Ddi_Dup_c:
    array->data[i] = DdiGenericDup(f);
    break;
  case Ddi_Mov_c:
    array->data[i] = f;
    break;
  }
  Ddi_Lock(array->data[i]);
}

/**Function*******************************************************************
  Synopsis    [Reads i-th element array]
  Description [Return the pointer to the <em>i-th</em>
               element array. The object is NOT duplicated (this should 
               be done externally if required).]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Generic_t *
DdiArrayRead (
  Ddi_ArrayData_t *array    /* BDDs' array */,
  int i                        /* index */ )
{
  Pdtutil_Assert(array!=NULL,"accessing a NULL Array");
  Pdtutil_Assert(((i>=0)&&(i<array->num)),"Array access out of bounds");
  return (array->data[i]);
}

/**Function*******************************************************************
  Synopsis    [Extracts i-th element array]
  Description [Return the pointer to the <em>i-th</em>
               element array. The object is removed from array and slots
               after i-th position shifted up.] 
  SideEffects [none]
  SeeAlso     []
******************************************************************************/

Ddi_Generic_t *
DdiArrayExtract (
  Ddi_ArrayData_t *array    /* array */,
  int i                   /* index */ )
{
  Ddi_Generic_t *f;
  int j;

  Pdtutil_Assert(array!=NULL,"accessing a NULL Array");
  Pdtutil_Assert(((i>=0)&&(i<array->num)),"Array access out of bounds");

  f = array->data[i];
  if (DdiType(f)!=Ddi_Var_c) {
    Ddi_Unlock(f);
  }
  array->data[i]=NULL;

  for (j=i; j<array->num-1;j++) {
    array->data[j]=array->data[j+1];
  }
  array->data[array->num-1]=NULL;
  array->num--;

  return(f);
}

/**Function*******************************************************************

  Synopsis    [Returns the number of elements of the array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
DdiArrayNum (
  Ddi_ArrayData_t *array
)
{
  Pdtutil_Assert(array!=NULL,"accessing a NULL Array");
  return (array->num);
}


/**Function*******************************************************************

  Synopsis    [Duplicate an array.]

  Description [Duplicates an array. Duplication is recursively applied 
               to array entries. 
               The unused room of the first array is not copied.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_ArrayData_t *
DdiArrayDup(
  Ddi_ArrayData_t *old  /* array to be duplicated */ )
{
  Ddi_ArrayData_t *newa;
  int i;

  Pdtutil_Assert(old!=NULL,"accessing a NULL Array");

  newa = DdiArrayAlloc (DdiArrayNum(old));

  for (i=0; i<DdiArrayNum(newa); i++) {
    newa->data[i] = DdiGenericDup(old->data[i]);
    Ddi_Lock(newa->data[i]);
  }
  return newa;
}


/**Function*******************************************************************

  Synopsis    [Copy a BDD array to a destination manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_ArrayData_t *
DdiArrayCopy(
  Ddi_Mgr_t     *dd2 /* destination manager */,
  Ddi_ArrayData_t *old  /* array to be duplicated */ )
{
  Ddi_ArrayData_t *newa;
  int i;

  Pdtutil_Assert(old!=NULL,"accessing a NULL Array");
  Pdtutil_Assert(dd2!=NULL,"NULL ddi manager in array copy");

  newa = DdiArrayAlloc (DdiArrayNum(old));

  for (i=0; i<DdiArrayNum(newa); i++) {
    newa->data[i] = DdiGenericCopy(dd2,old->data[i],NULL,NULL);
    Ddi_Lock(newa->data[i]);
  }
  return newa;
}



/**Function*******************************************************************

  Synopsis    [Append array2 at the end of array1]
  Description [Writes the elements of array2 after the last
               element of array1.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
DdiArrayAppend (
  Ddi_ArrayData_t *array1   /* first array */,
  Ddi_ArrayData_t *array2   /* array to be appended */ 
)
{
  int i;

  Pdtutil_Assert(array1!=NULL,"accessing a NULL Array");
  Pdtutil_Assert(array2!=NULL,"accessing a NULL Array");

  for (i=0; i<DdiArrayNum(array2); i++)
    DdiArrayInsert(array1,DdiArrayNum(array1),array2->data[i],Ddi_Dup_c);
}

#if 0
/*@Function*******************************************************************
  Synopsis    [Sort elements of array]
  Description [Sort elements of array]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
DdiArraySort (
  Ddi_ArrayData_t *array    /* array */,
  DDI_CMPFUN       cmpFun
)
{
@@@
}
#endif

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
DdiArrayStore (
  Ddi_ArrayData_t *array    /* array to be stored */,
  char *ddname            /* dd name (or NULL) */,
  char **vnames           /* array of variable names (or NULL) */,
  char **rnames           /* array of root names (or NULL) */,
  int *vauxids             /* array of aux var ids (or NULL) */,
  int mode                /* storing mode selector */,
  char *fname             /* file name */,
  FILE *fp                /* pointer to the store file */ )
{
  DdNode **f;   /* array of BDD roots to be stored */
  int i;
  Ddi_Mgr_t *dd;

  Pdtutil_Assert(DdiArrayNum(array)>=1, "No partitions");
  dd = Ddi_ReadMgr(DdiArrayRead(array,0));

  f = DdiArrayToCU(array);
  if (f==NULL) {
   return (0);
  }

  i = Dddmp_cuddBddArrayStore (dd->mgrCU,ddname,DdiArrayNum(array),
       f,rnames,vnames,vauxids,mode,DDDMP_VARIDS,fname,fp);

  Pdtutil_Free(f);   

  return (i);
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************
  Synopsis    [Resize a DDI array] 
  Description [Resize a DDI array using proper realloc function.]
  SideEffects [none]
  SeeAlso     []
*****************************************************************************/
static void
ArrayResize (
  Ddi_ArrayData_t  *array      /* array to be resized */,
  int            nSizeNew    /* new size of array */ )
{
  int nSizeOld;
  int i;

  nSizeOld = array->nSize;
  array->nSize = DDI_MAX(array->nSize * 2, nSizeNew);
  array->data = Pdtutil_Realloc(Ddi_Generic_t *, array->data, array->nSize);
  
  for (i=nSizeOld; i<array->nSize ; i++) {
     array->data[i] = NULL;
  }

}







