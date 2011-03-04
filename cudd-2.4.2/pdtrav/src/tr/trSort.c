/**CFile**********************************************************************

  FileName    [trSort.c]

  PackageName [Tr]

  Synopsis    [Sort a BDDs'array]

  Description []

  SeeAlso     []  

  Author      [Gianpiero Cabodi and Stefano Quer]

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

#include "trInt.h"

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct PartitionInfo_t {
  Ddi_Bdd_t *F;              /* BDD root of the function */
  Ddi_Varset_t *SmoothV;    /* Variables that will be smoothed in img/preimg */
  Ddi_Varset_t *RangeV;     /* Variables in the range of F */
  int id,                   /* function identifier in a struct array */
      size,                 /* BDD size of F */
      SmoothN,              /* number of smoothing variables */
      LocalSmoothN,         /* number of local smoothing variables */
      RangeN,               /* number of range variables */
      TopSmoothPos,         /* top smooth variable (pos. in the ordering) */
      BottomSmoothPos;      /* bottom smooth variable (pos. in the ordering) */
  } 
PartitionInfo_t;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* shell to qsort used for MaxMin and Size sorting */
#define PartitionInfoArraySort(PartInfoArray,n,CompareFunction) \
  qsort((void **)PartInfoArray,n,sizeof(PartitionInfo_t *),CompareFunction)

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int MaxMinVSupportCompare(const void *s1, const void *s2);
static int BddSizeCompare(const void *s1, const void *s2);
static int WeightedSort(Tr_Mgr_t *trMgr, Ddi_Mgr_t *dd, PartitionInfo_t **FSa, int nPart, Tr_Sort_e method);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of internal function                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external function                                           */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************

  Synopsis    [Sorting an array of BDDs]

  Description [Given an BDDs'array , the function sorts the BDDs in the
    array in order to find the best sort for compute esistential
    abstraction of BDDs product. The smoothing variable set should
    include the quantifying ones, too.
    It returns 1 if successfully sorted, 0 otherwise.]

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

int
Tr_TrSortIwls95 (
  Tr_Tr_t      *tr
)
{
  PartitionInfo_t **FSa;    /* pointer to a vector of struct pointers */
  PartitionInfo_t *FS;      /* struct pointer */
  Ddi_Varset_t *supp;       /* function support */
  int n,i;
  int retval=1;             /* default is correct return */
  Ddi_Varset_t *pivars;     /* Set of smoothing variables */
  Ddi_Varset_t *svars;      /* Set of smoothing variables */
  Ddi_Varset_t *rvars;      /* Set of range variables */
  Ddi_Bdd_t     *F;         /* Partitioned TR rel */
  Tr_Mgr_t     *trMgr;
  Tr_Sort_e method;
  Ddi_Mgr_t *dd;            /* dd manager */

  trMgr=tr->trMgr;
  method = Tr_MgrReadSortMethod(trMgr);
  F = Tr_TrBdd(tr);
  dd = Ddi_ReadMgr(F);

  if (method==Tr_SortNone_c) {
    return (1);
  }

  svars = Ddi_VarsetMakeFromArray(trMgr->ps);
  pivars = Ddi_VarsetMakeFromArray(trMgr->i);
  Ddi_VarsetUnionAcc(svars,pivars);  
  Ddi_Free(pivars);
  rvars = Ddi_VarsetMakeFromArray(trMgr->ns);

  /* disable dinamic ordering if enabled */
  Ddi_MgrAutodynSuspend(dd);

  n = Ddi_BddPartNum(F);
 
  FSa = Pdtutil_Alloc(PartitionInfo_t *,n);
  if (FSa==NULL) {
    fprintf(stderr, "Error : Out of memory\n");
    return 0;
  }
  /* init with NULL pointers for safe free in case of failure */
  for (i=0;i<n;i++) {
    FSa[i] = NULL;
  }

  /*
   *  Load array of partition infos 
   */

  for (i=0;i<n;i++) {
    FS = Pdtutil_Alloc (PartitionInfo_t, 1);
    if (FS==NULL) {
      fprintf(stderr, "Error : Out of memory\n");
      retval = 0;
      goto end_sort;
    }
   
    FS->F = Ddi_BddDup(Ddi_BddPartRead(F,i));       /* F function */
    FS->id = i;                           /* id = original array index */ 
    FS->size = Ddi_BddSize(FS->F);         /* BDD size of F */

    supp = Ddi_BddSupp(FS->F);          /* support of F */
    FS->SmoothV = Ddi_VarsetIntersect(supp,svars);
    FS->RangeV = Ddi_VarsetIntersect(supp,rvars);
    Ddi_Free(supp);

    FS->SmoothN = Ddi_VarsetNum(FS->SmoothV);
    FS->RangeN = Ddi_VarsetNum(FS->RangeV);
    if (!Ddi_VarsetIsVoid(FS->SmoothV)) {
      FS->TopSmoothPos = Ddi_VarCurrPos(Ddi_VarsetTop(FS->SmoothV));
      FS->BottomSmoothPos = Ddi_VarCurrPos(Ddi_VarsetBottom(FS->SmoothV));
    } else {
      FS->TopSmoothPos = -1;
      FS->BottomSmoothPos = -1;
    }


    FSa[i] = FS;

  } /* end lood info array */
 
  /*
   * Select sorting method
   */ 

  switch (method) {

    case Tr_SortNone_c:
    case Tr_SortDefault_c:
      /* default is no sorting */
      break;

    case Tr_SortMinMax_c:
      PartitionInfoArraySort(FSa,n,MaxMinVSupportCompare);
      break;

    case Tr_SortSize_c:
      PartitionInfoArraySort(FSa,n,BddSizeCompare);
      break;

    case Tr_SortWeight_c:
    case Tr_SortWeightDac99_c:
      if (WeightedSort (trMgr, dd, FSa, n, method) > 0) {
        break; 
      } else {
        retval = 0;
        goto end_sort;
      }
      
    default:
      fprintf(stderr, "Error : Invalid TR sorting method selected\n");
      retval = 0;
      goto end_sort;
  }
 
  for (i=0;i<n;i++) {
    FS = FSa[i];
    Ddi_BddPartWrite(F,i,FS->F);
    Ddi_Free(FS->F);  
  }

  if (Tr_MgrReadVerbosity(trMgr) >= Pdtutil_VerbLevelUsrMin_c) {
    fprintf(stdout, "Sorted partitions [\n");
    for (i=0;i<n;i++) {
      fprintf(stdout, "%d ",FSa[i]->id);
    }
    fprintf(stdout, "]\n");
  } 

  end_sort:

  Ddi_MgrAutodynResume(dd);

  for (i=0;i<n;i++) {
    Ddi_Free(FSa[i]->SmoothV);
    Ddi_Free(FSa[i]->RangeV);
    Pdtutil_Free(FSa[i]);
  }
  Pdtutil_Free(FSa);

  Ddi_Free(svars);
  Ddi_Free(rvars);

  return (retval);
}


/*---------------------------------------------------------------------------*/
/* Definition of static function                                             */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************

  Synopsis    [Compares BDDs for MAX-MIN-V-SORTING method.]

  Description []

  SideEffects [none]

  SeeAlso     [Tr_BddarraySort()]

*****************************************************************************/

static int
MaxMinVSupportCompare(
  const void *s1,
  const void *s2
)
{
  int a,b,r;
  PartitionInfo_t *pFS1 = *((PartitionInfo_t **)s1);
  PartitionInfo_t *pFS2 = *((PartitionInfo_t **)s2);

  r = 0;

  if ((pFS1->SmoothN == 0) || (pFS2->SmoothN == 0)) {
    a = pFS1->SmoothN;
    b = pFS2->SmoothN;
  }
  else if (pFS1->BottomSmoothPos != pFS2->BottomSmoothPos) {
    a = pFS1->BottomSmoothPos;
    b = pFS2->BottomSmoothPos;
  }
  else if (pFS1->TopSmoothPos != pFS2->TopSmoothPos) {
    b = pFS1->TopSmoothPos;
    a = pFS2->TopSmoothPos;
  } else {
    a = pFS1->SmoothN;
    b = pFS2->SmoothN;
  }

  if (a > b)
    r = 1;
  else if (a == b)
    r = 0;
  else if (a < b)
    r = -1;

  return (r);
}

  
/**Function*******************************************************************

  Synopsis    [Compares BDDs for SIZE-SORTING method]

  Description [The function compares two BDDs and return:<BR>
    1, if size of 1st BDD is greater then 2nd<BR>
    -1, if size of 1st BDD is smaller then 2nd<BR>
    0, if both 1st and 2nd BDD have the same size.]

  SideEffects [none]

  SeeAlso     [Tr_BddarraySort()]

*****************************************************************************/

static int
BddSizeCompare(
  const void *s1,
  const void *s2
)
{
  int a,b,r;
  PartitionInfo_t *pFS1 = *((PartitionInfo_t **)s1);
  PartitionInfo_t *pFS2 = *((PartitionInfo_t **)s2);

  r = 0;
  a = pFS1->size;
  b = pFS2->size;

  if (a > b)
    r = 1;
  else if (a == b)
    r = 0;
  else if (a < b)
    r = -1;

  return (r);
}
 

/**Function*******************************************************************

  Synopsis    [Compares BDDs for WEIGTHED-SORT method]

  Description [The method of sorting is the heuristic method of Ranjan]

  SideEffects [none]

  SeeAlso     [Tr_BddarraySort()]

*****************************************************************************/ 

static int
WeightedSort (
  Tr_Mgr_t *trMgr             /* Tr manager */,
  Ddi_Mgr_t *dd               /* dd manager */,
  PartitionInfo_t **FSa       /* array of Partition Infos */,
  int nPart                   /* number of partitions */,
  Tr_Sort_e method            /* sorting method */
)
{
  Ddi_Varset_t *supp;
  PartitionInfo_t *FS, *FS1;
  int i,j,k,nvars,pos;
  int *var_cnt;     /* variable occourrences */
  int maxLocalSmoothN, maxSmoothN, totSmoothN, 
      maxBottomSmoothPos, maxRangeN;
  double benefit,  /* cost function */
         best;      /* the best cost function */
  /* Read the weights for WEIGHTED-SORT method */
  double w1 = Tr_MgrReadSortW (trMgr, 1);
  double w2 = Tr_MgrReadSortW (trMgr, 2);
  double w3 = Tr_MgrReadSortW (trMgr, 3);
  double w4 = Tr_MgrReadSortW (trMgr, 4);
  double w5 = Tr_MgrReadSortW (trMgr, 5);
 
  nvars = Ddi_ReadSize(dd);

  var_cnt = Pdtutil_Alloc(int,nvars);
  if (var_cnt == NULL) {
    fprintf(stderr, "Error : Out of memory\n");
    return 0;
  }

  for (i=0; i<nvars; i++)
    var_cnt[i] = 0;

  maxLocalSmoothN = maxSmoothN = maxBottomSmoothPos = maxRangeN = 0;
  totSmoothN = 0;

  for (i=0; i<nPart; i++) {
    FS = FSa[i]; 
    if (FS->SmoothN > maxSmoothN)
      maxSmoothN = FS->SmoothN;    
    if (FS->BottomSmoothPos > maxBottomSmoothPos)
      maxBottomSmoothPos = FS->BottomSmoothPos;    
    if (FS->RangeN > maxRangeN)
      maxRangeN = FS->RangeN;    

    supp = Ddi_VarsetDup(FS->SmoothV);
    /* upgrade var_cnt with variable occourrences in supp */
    while (!Ddi_VarsetIsVoid(supp)) {
      pos = Ddi_VarCurrPos(Ddi_VarsetTop(supp));
      if (var_cnt[pos]==0)
        totSmoothN++;
      var_cnt[pos]++;

      if (method==Tr_SortWeight_c) {
Ddi_VarsetNextAcc(supp);
      } else {
        supp = Ddi_VarsetNext(supp);
      }

    }
    Ddi_Free(supp);
  }

  /*
   * Do sorting 
   */
  for (i=0; i<nPart; i++) {
    /* dynamically upgrade maxBottomSmoothPos with remaining partitions */
    while (var_cnt[maxBottomSmoothPos] == 0)   
      maxBottomSmoothPos--;

    maxLocalSmoothN = 0;
    for (j=i; j<nPart; j++) {
      FS = FSa[j];

      FS->LocalSmoothN = 0;

      supp = Ddi_VarsetDup(FS->SmoothV);
      /* upgrade var_cnt with variable occourrences in supp */
      while (!Ddi_VarsetIsVoid(supp)) {
        pos = Ddi_VarCurrPos(Ddi_VarsetTop(supp));
        if (var_cnt[pos] == 1)  /* local variable */
          FS->LocalSmoothN++;

        if (method==Tr_SortWeight_c) {
Ddi_VarsetNextAcc(supp);
        } else {
          supp = Ddi_VarsetNext(supp);
        }

      }
      Ddi_Free(supp);

      if (FS->LocalSmoothN > maxLocalSmoothN) 
        maxLocalSmoothN = FS->LocalSmoothN;
    }

    best = 0.0;  
    for (k=j=i; j<nPart; j++) {
      FS = FSa[j];


      if (Tr_MgrReadVerbosity(trMgr) >= Pdtutil_VerbLevelAppMed_c) {
        fprintf(stdout, "\nsorting benefit \n");
        fprintf(stdout, "Sort p[%d]: %f * %d / %d\n", j, 
          w1, FSa[j]->LocalSmoothN, maxLocalSmoothN);
        fprintf(stdout, "Sort p[%d]: %f * %d / %d\n", j, 
          w2, FSa[j]->SmoothN, maxSmoothN);
        fprintf(stdout, "Sort p[%d]: %f * %d / %d\n", j, 
          w3, FSa[j]->BottomSmoothPos, maxBottomSmoothPos);
        fprintf(stdout, "Sort p[%d]: %f * %d / %d\n", j, 
          -w4, FSa[j]->RangeN, maxRangeN);
        fprintf(stdout, "Sort p[%d]: %f * %d / %d\n", j, 
          w5, FSa[j]->SmoothN, totSmoothN);
      }

      benefit = 0,0;

      if (maxLocalSmoothN > 0)
        benefit += (w1 * FSa[j]->LocalSmoothN)/maxLocalSmoothN;

      if (maxSmoothN > 0) {
        benefit += (w2 * FSa[j]->SmoothN)/maxSmoothN;
      }

      if (maxBottomSmoothPos > 0) {
        benefit += (w3 * FSa[j]->BottomSmoothPos)/maxBottomSmoothPos;
      }

      if (maxRangeN > 0) {
        benefit -= (w4 * FSa[j]->RangeN)/maxRangeN;
      }

      if (totSmoothN > 0) {
        benefit += (w5 * FSa[j]->SmoothN)/totSmoothN;
      }

      if (Tr_MgrReadVerbosity (trMgr) >= Pdtutil_VerbLevelAppMed_c) {
        fprintf(stdout, "\nsorting benefit \n");
        fprintf(stdout, "Sort p[%d]: Benefit: %f\n\n", j, benefit);
      }

      if (benefit > best) {
        best = benefit;
        k = j;
      }
    }  /* end-for (j) */

    /* swap k (function with best benefit) and i */

    FS1 = FSa[i]; 
    FS = FSa[k]; 
    FSa[i]=FS;
    FSa[k]=FS1;
  
    /* upgrade counters , x and z */

    supp = Ddi_VarsetDup(FS->SmoothV);
    /* upgrade var_cnt with variable occourrences in supp */
    while (!Ddi_VarsetIsVoid(supp)) {
      pos = Ddi_VarCurrPos(Ddi_VarsetTop(supp));
      var_cnt[pos]--;
      Pdtutil_Assert(var_cnt[pos]>=0,"var count is < 0");

      if (method==Tr_SortWeight_c) {
Ddi_VarsetNextAcc(supp);
      } else {
        supp = Ddi_VarsetNext(supp);
      }

    }
    Ddi_Free(supp);

  }  /* end-for (i) */

  free (var_cnt);
  return (1);
}
