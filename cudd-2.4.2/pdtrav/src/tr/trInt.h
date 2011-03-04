/**CHeaderFile*****************************************************************

  FileName    [trInt.h]

  PackageName [Tr]

  Synopsis    [Internal header file]

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

#ifndef _TRINT
#define _TRINT

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "tr.h"
#include "pdtutil.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* error number */
#define TRSORT_OUT_OF_MEM -1
#define TRSORT_INVALID_SORTING_METHOD -2

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

 Synopsis    [Transition Relation Manager]

 Description [The Transition Relation manager structure]

******************************************************************************/

struct Tr_Mgr_s {
  char *trName;

  Ddi_Mgr_t      *DdiMgr;      /* DD manager */
  Tr_Tr_t        *tr;          /* transition relation function */
  Ddi_Vararray_t *i;           /* primary inputs */
  Ddi_Vararray_t *ps;          /* present state variables */
  Ddi_Vararray_t *ns;          /* next state variables */
  Ddi_Vararray_t *auxVars;     /* auxiliary variables */
  Ddi_Bddarray_t *auxFuns;     /* functions corresponding to auxiliary vars */
  Ddi_Bdd_t      *auxRel;      /* relation mapping auc vars and funs */

  Tr_Tr_t        *trList;

  long trTime;

  struct {
    Pdtutil_VerbLevel_e verbosity;
    struct {
      Tr_Sort_e method;
      double W[5];
    } sort;
    struct {
      int threshold;
      int smoothPi;
    } cluster;
    struct {
      int maxIter;
      int method;
    } closure;
    struct {
      Tr_ImgMethod_e method;      /* selects image method */
      int partThFrom;               /* partitioning threshold for from */
      int partThTr;                 /* partitioning threshold for tr */
      Part_Method_e partitionMethod;
      int smoothPi;
    } image;
  } settings;
};	


/**Struct*********************************************************************
 Synopsis    [Transition relation generalized pointer]
 Description [Transition relation generalized pointer]
******************************************************************************/
struct Tr_Tr_s {
  Tr_Mgr_t                *trMgr;
  Ddi_Expr_t              *ddiTr;
  struct Tr_Tr_s          *prev;
  struct Tr_Tr_s          *next;
};	

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Ddi_Bdd_t * TrBuildTransClosure(Tr_Mgr_t *trMgr, Ddi_Bdd_t *TR, Ddi_Vararray_t *s, Ddi_Vararray_t *y, Ddi_Vararray_t *z);
EXTERN Tr_Tr_t * TrAlloc(Tr_Mgr_t *trMgr);
EXTERN Tr_Tr_t * TrTrRelWrite(Tr_Tr_t *tr, Ddi_Bdd_t *bdd);

/**AutomaticEnd***************************************************************/


#endif /* _TRINT */



