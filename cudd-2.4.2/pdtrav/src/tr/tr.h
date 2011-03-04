/**CHeaderFile*****************************************************************

  FileName    [tr.h]

  PackageName [Tr]

  Synopsis    [Funtions to manipolate Transition Relation]

  Description [The <b>Tr</b> (<em>Transition Relation</em>) package provides
    a set of functions to manipolate the Transition Relation.<br>
    The transition relation can be built using the monolithic form or
    clustering heuristics.<br>
    Squaring it, transitive closure, is also possible.<br>   
    External procedures included in this package:
    <ul>
    <li>Tr_BddarrayAndExist() 
    <li>Tr_CreateTrv() 
    <li>Tr_BddarraySort() 
    </ul>
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

#ifndef _TR
#define _TR

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include "pdtutil.h"
#include "ddi.h"
#include "part.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* default value for the four weight in the WEIGHTED-SORT method */
#define TRSORT_DEFAULT_W1 6.0
#define TRSORT_DEFAULT_W2 1.0
#define TRSORT_DEFAULT_W3 1.0
#define TRSORT_DEFAULT_W4 2.0
#define TRSORT_DEFAULT_W5 0.0

/* default value for threshold (number of BDD nodes) for clustering */
#define TR_IMAGE_CLUSTER_SIZE 1000

/* squaring methods */
#define TR_SQUARING_FULL 0
#define TR_SQUARING_POWER 1

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct Tr_Mgr_s Tr_Mgr_t;
typedef struct Tr_Tr_s Tr_Tr_t;

/**Enum************************************************************************

  Synopsis    [Type for Transition Relation Sorting Method.]

  Description []

******************************************************************************/

typedef enum
  {
  Tr_SortNone_c,
  Tr_SortDefault_c,
  Tr_SortMinMax_c,
  Tr_SortSize_c,
  Tr_SortWeight_c,
  Tr_SortWeightDac99_c
  }
Tr_Sort_e;


/**Enum************************************************************************

  Synopsis    [Type for Image Selection]

  Description []

******************************************************************************/

typedef enum
  {
   /* image method of symbolic traversal */
   Tr_ImgMethodMonolithic_c,
   Tr_ImgMethodIwls95_c,
   Tr_ImgMethodApprox_c,
   Tr_ImgMethodCofactor_c
  }
Tr_ImgMethod_e;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Tr_RemoveLambdaLatches(Tr_Tr_t *tr);
EXTERN Tr_Tr_t * Tr_TransClosure(Tr_Tr_t *tr);
EXTERN Ddi_Bdd_t * Tr_Img(Tr_Tr_t *TR, Ddi_Bdd_t *from);
EXTERN Tr_Mgr_t * Tr_MgrInit(char *trName, Ddi_Mgr_t *dd);
EXTERN void Tr_MgrQuit(Tr_Mgr_t *trMgr);
EXTERN int Tr_MgrPrintStats(Tr_Mgr_t *trMgr);
EXTERN int Tr_MgrOperation(Tr_Mgr_t *trMgr, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlagP);
EXTERN void Tr_MgrSetDdiMgrDefault(Tr_Mgr_t *trMgr, Ddi_Mgr_t *mgr);
EXTERN Ddi_Mgr_t * Tr_MgrReadDdiMgrDefault(Tr_Mgr_t *trMgr);
EXTERN Ddi_Vararray_t * Tr_MgrReadI(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetI(Tr_Mgr_t *trMgr, Ddi_Vararray_t *i);
EXTERN Ddi_Vararray_t * Tr_MgrReadPS(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetPS(Tr_Mgr_t *trMgr, Ddi_Vararray_t *ps);
EXTERN Ddi_Vararray_t * Tr_MgrReadNS(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetNS(Tr_Mgr_t *trMgr, Ddi_Vararray_t *ns);
EXTERN void Tr_MgrSetAuxVars(Tr_Mgr_t *trMgr, Ddi_Vararray_t *auxVars);
EXTERN void Tr_MgrSetAuxFuns(Tr_Mgr_t *trMgr, Ddi_Bddarray_t *auxFuns);
EXTERN Tr_Tr_t * Tr_MgrReadTr(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetTr(Tr_Mgr_t *trMgr, Tr_Tr_t *tr);
EXTERN Pdtutil_VerbLevel_e Tr_MgrReadVerbosity(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetVerbosity(Tr_Mgr_t *trMgr, Pdtutil_VerbLevel_e verbosity);
EXTERN Tr_Sort_e Tr_MgrReadSortMethod(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetSortMethod(Tr_Mgr_t *trMgr, Tr_Sort_e sortMethod);
EXTERN int Tr_MgrReadClustThreshold(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetClustThreshold(Tr_Mgr_t *trMgr, int ClustThreshold);
EXTERN void Tr_MgrSetClustSmoothPi(Tr_Mgr_t *trMgr, int val);
EXTERN void Tr_MgrSetImgSmoothPi(Tr_Mgr_t *trMgr, int val);
EXTERN double Tr_MgrReadSortW(Tr_Mgr_t *trMgr, int i);
EXTERN void Tr_MgrSetSortW(Tr_Mgr_t *trMgr, int i, double SortW);
EXTERN void Tr_MgrSetTrName(Tr_Mgr_t *trMgr, char *trName);
EXTERN char * Tr_MgrReadTrName(Tr_Mgr_t *trMgr);
EXTERN int Tr_MgrReadMaxIter(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetMaxIter(Tr_Mgr_t *trMgr, int maxIter);
EXTERN int Tr_MgrReadSquaringMethod(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetSquaringMethod(Tr_Mgr_t *trMgr, int method);
EXTERN Tr_ImgMethod_e Tr_MgrReadImgMethod(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetImgMethod(Tr_Mgr_t *trMgr, Tr_ImgMethod_e imgMethod);
EXTERN int Tr_MgrReadPartThFrom(Tr_Mgr_t *trMgr);
EXTERN int Tr_MgrReadPartThTr(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetPartThFrom(Tr_Mgr_t *trMgr, int threshold);
EXTERN void Tr_MgrSetPartThTr(Tr_Mgr_t *trMgr, int threshold);
EXTERN Part_Method_e Tr_MgrReadPartitionMethod(Tr_Mgr_t *trMgr);
EXTERN void Tr_MgrSetPartitionMethod(Tr_Mgr_t *trMgr, Part_Method_e partitionMethod);
EXTERN int Tr_TrSortIwls95(Tr_Tr_t *tr);
EXTERN Tr_Tr_t * Tr_TrMakeFromRel(Tr_Mgr_t *trMgr, Ddi_Bdd_t *bdd);
EXTERN Tr_Tr_t * Tr_TrMakeFromExpr(Tr_Mgr_t *trMgr, Ddi_Expr_t *expr);
EXTERN Tr_Tr_t * Tr_TrMakePartConjFromFuns(Tr_Mgr_t *trMgr, Ddi_Bddarray_t *Fa, Ddi_Vararray_t *Va);
EXTERN Tr_Tr_t * Tr_TrSetMono(Tr_Tr_t *tr);
EXTERN Tr_Tr_t * Tr_TrSetClustered(Tr_Tr_t *tr);
EXTERN Tr_Tr_t * Tr_TrDup(Tr_Tr_t *old);
EXTERN Tr_Mgr_t * Tr_TrMgr(Tr_Tr_t *tr);
EXTERN Ddi_Bdd_t * Tr_TrBdd(Tr_Tr_t *tr);
EXTERN void Tr_TrFree(Tr_Tr_t *tr);
EXTERN Tr_Tr_t * Tr_TrReverse(Tr_Tr_t *old);
EXTERN Tr_Tr_t * Tr_TrReverseAcc(Tr_Tr_t *tr);
EXTERN Tr_Sort_e Tr_TrSortString2Enum(char *string);
EXTERN char * Tr_TrSortEnum2String(Tr_Sort_e enumType);
EXTERN Tr_ImgMethod_e Tr_ImgMethodString2Enum(char *string);
EXTERN char * Tr_ImgMethodEnum2String(Tr_ImgMethod_e enumType);

/**AutomaticEnd***************************************************************/


#endif /* _TR */
