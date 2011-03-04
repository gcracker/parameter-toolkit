/**CFile***********************************************************************

  FileName    [trTr.c]

  PackageName [tr]

  Synopsis    [Functions to handle a TR object]

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
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

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

/**Function********************************************************************
  Synopsis    [Create a TR from relation.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrMakeFromRel (
  Tr_Mgr_t *trMgr,
  Ddi_Bdd_t *bdd
)
{
  Tr_Tr_t *tr;

  tr = TrAlloc(trMgr);
  tr->ddiTr = Ddi_ExprMakeFromBdd(bdd);
  Ddi_Lock(tr->ddiTr);

  return (tr);
}

/**Function********************************************************************
  Synopsis    [Create a TR from expression.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrMakeFromExpr (
  Tr_Mgr_t *trMgr,
  Ddi_Expr_t *expr
)
{
  Tr_Tr_t *tr;

  tr = TrAlloc(trMgr);
  tr->ddiTr = Ddi_ExprDup(expr);
  Ddi_Lock(tr->ddiTr);

  return (tr);
}

/**Function********************************************************************
  Synopsis    [Create a conjunctively partitioned TR from array of functions.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrMakePartConjFromFuns (
  Tr_Mgr_t *trMgr,
  Ddi_Bddarray_t *Fa,
  Ddi_Vararray_t *Va
)
{
  Tr_Tr_t *tr;
  Ddi_Bdd_t *rel;

  rel = Ddi_BddRelMakeFromArray(Fa,Va);
  tr = Tr_TrMakeFromRel(trMgr,rel);
  Ddi_Free(rel);

  return (tr);
}

/**Function********************************************************************
  Synopsis    [Reduce tr from partitioned/clustered to monolithic.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrSetMono (
  Tr_Tr_t *tr
)
{
  Ddi_Varset_t *qvars;

  if (tr->trMgr->settings.cluster.smoothPi) {
    qvars = Ddi_VarsetMakeFromArray(tr->trMgr->i);
    Ddi_BddExistAcc(Tr_TrBdd(tr),qvars);
    Ddi_Free(qvars);
  }
  else {
    Ddi_BddSetMono(Tr_TrBdd(tr));
  }
  return (tr);
}

/**Function********************************************************************
  Synopsis    [Transform tr to clustered form.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrSetClustered (
  Tr_Tr_t *tr
)
{
  Tr_Mgr_t *trMgr;
  int threshold;
  Ddi_Varset_t  *qvars;     /* Set of quantifying variables */
  Ddi_Bdd_t *rel;
  long trTime;

  trMgr = tr->trMgr;
  threshold = trMgr->settings.cluster.threshold;

  trTime = util_cpu_time ();

  /* no clustering done if threshold <= 0 */
  if (threshold > 0) {

    if (trMgr->settings.cluster.smoothPi) {
      qvars = Ddi_VarsetMakeFromArray(trMgr->i);
    }
    else {
      qvars = Ddi_VarsetVoid(trMgr->DdiMgr);
    }
    if (Tr_MgrReadVerbosity (trMgr) >= Pdtutil_VerbLevelAppMin_c) {
      fprintf(stdout, "TR Clustering\n");
      fprintf(stdout, "|q|: %d\n", Ddi_VarsetNum (qvars));
      fprintf(stdout, "thr: %d\n", threshold);
    }

    rel = Part_BddMultiwayLinearAndExist(Tr_TrBdd(tr),
      qvars,threshold);

    TrTrRelWrite(tr,rel);
    Ddi_Free(rel);
    Ddi_Free(qvars);
  }

  trTime = util_cpu_time() - trTime;
  trMgr->trTime += trTime;

  return (tr);
}


/**Function********************************************************************
  Synopsis    [Duplicate a TR]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrDup (
  Tr_Tr_t *old
)
{
  Tr_Tr_t *newt;

  newt = TrAlloc(old->trMgr);
  newt->ddiTr = Ddi_ExprDup(old->ddiTr);
  Ddi_Lock(newt->ddiTr);

  return (newt);
}

/**Function********************************************************************
  Synopsis    [Return tr Manager]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Mgr_t *
Tr_TrMgr (
  Tr_Tr_t *tr
)
{
  return (tr->trMgr);
}

/**Function********************************************************************
  Synopsis    [Return Bdd relation field]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Tr_TrBdd (
  Tr_Tr_t *tr
)
{
  return (Ddi_ExprToBdd(tr->ddiTr));
}

/**Function********************************************************************
  Synopsis    [Release a TR.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
Tr_TrFree (
  Tr_Tr_t *tr
)
{
  
  if (tr == NULL) {
    return;
  }
  Ddi_Unlock(tr->ddiTr);
  Ddi_Free(tr->ddiTr);
  if (tr->prev != NULL) {
    tr->prev->next = tr->next;
  }
  else {
    tr->trMgr->trList = tr->next;
  }
  if (tr->next != NULL) {
    tr->next->prev = tr->prev;
  }
  Pdtutil_Free(tr);
}


/**Function********************************************************************
  Synopsis    [Reverse a TR by swapping present/next state variables]
  Description [Reverse a TR by swapping present/next state variables]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrReverse (
  Tr_Tr_t *old
)
{
  Tr_Tr_t *newt;

  newt = Tr_TrDup(old);
  Ddi_BddSwapVarsAcc(Tr_TrBdd(newt),newt->trMgr->ps,newt->trMgr->ns);

  return (newt);
}

/**Function********************************************************************
  Synopsis    [Reverse a TR by swapping present/next state variables]
  Description [Reverse a TR by swapping present/next state variables]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
Tr_TrReverseAcc (
  Tr_Tr_t *tr
)
{
  Ddi_BddSwapVarsAcc(Tr_TrBdd(tr),tr->trMgr->ps,tr->trMgr->ns);

  return (tr);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [insert a TR in TR manager list.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/

Tr_Tr_t *
TrAlloc (
  Tr_Mgr_t *trMgr
)
{
  Tr_Tr_t  *tr;

  tr = Pdtutil_Alloc(Tr_Tr_t, 1);
  tr->trMgr = trMgr;
  tr->ddiTr = NULL;
  tr->prev = NULL;
  tr->next = trMgr->trList;
  if (trMgr->trList != NULL) {
    trMgr->trList->prev = tr;
  }
  trMgr->trList = tr;

  return (tr);
}

/**Function********************************************************************
  Synopsis    [Write (replace) Bdd relation.]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Tr_Tr_t *
TrTrRelWrite (
  Tr_Tr_t *tr,
  Ddi_Bdd_t *bdd
)
{
  Ddi_Unlock(tr->ddiTr);
  Ddi_Free(tr->ddiTr);
  tr->ddiTr = Ddi_ExprMakeFromBdd(bdd);
  Ddi_Lock(tr->ddiTr);

  return (tr);
}









