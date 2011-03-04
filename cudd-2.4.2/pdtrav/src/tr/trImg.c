/**CFile***********************************************************************

  FileName    [trImg.c]

  PackageName [tr]

  Synopsis    [Functions for image/preimage computations]

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

static Ddi_Bdd_t * TrImgConjPartTr(Tr_Mgr_t *trMgr, Ddi_Bdd_t *TR, Ddi_Bdd_t *from, Ddi_Vararray_t *psv, Ddi_Vararray_t *nsv, Ddi_Varset_t *smoothV);
static Ddi_Bdd_t * TrImgDisjPartTr(Tr_Mgr_t *TrMgr, Ddi_Bdd_t *TR, Ddi_Bdd_t *from, Ddi_Vararray_t *psv, Ddi_Vararray_t *nsv, Ddi_Varset_t *smoothV);
static Ddi_Bdd_t * TrImgDisjPartSet(Tr_Mgr_t *TrMgr, Ddi_Bdd_t *TR, Ddi_Bdd_t *part_from, Ddi_Vararray_t *psv, Ddi_Vararray_t *nsv, Ddi_Varset_t *smoothV);
static Ddi_Bdd_t * TrImgApproxConjPartTr(Tr_Mgr_t *TrMgr, Ddi_Bdd_t *TR, Ddi_Bdd_t *from, Ddi_Vararray_t *psv, Ddi_Vararray_t *nsv, Ddi_Varset_t *smoothV);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Compute image of a conjunctively partitioned transition
    relation.]

  Description []

  SideEffects [None]

  SeeAlso     [Part_BddarrayMultiwayAndExist]

******************************************************************************/

Ddi_Bdd_t *
Tr_Img (
  Tr_Tr_t *TR               /* Partitioned TR */,
  Ddi_Bdd_t *from             /* Input constrain */
)
{
  Tr_Mgr_t  *trMgr=Tr_TrMgr(TR);
  Ddi_Vararray_t *psv;
  Ddi_Vararray_t *nsv;
  Ddi_Varset_t *smoothV, *pis;
  Ddi_Bdd_t *to=NULL, *trBdd, *myTr, *myFrom;
  Ddi_Mgr_t *ddTR = Ddi_ReadMgr(TR);     /* TR DD manager */
  Ddi_Mgr_t *ddR = Ddi_ReadMgr(from);  /* Reached DD manager */
  int verbosity = Tr_MgrReadVerbosity (trMgr);
  int setMonoTo = 0;
  int extRef, nodeId;

  trMgr = TR->trMgr;

  extRef = Ddi_MgrReadExtRef(ddR);
  nodeId = Ddi_MgrReadCurrNodeId(ddR);

  psv = trMgr->ps;
  nsv = trMgr->ns;

  smoothV = Ddi_VarsetMakeFromArray(trMgr->ps);
  if ((trMgr->settings.image.smoothPi)&&(trMgr->i != NULL)) {
    pis = Ddi_VarsetMakeFromArray(trMgr->i);
    Ddi_VarsetUnionAcc(smoothV,pis);
    Ddi_Free(pis);
  }

  trBdd = Tr_TrBdd(TR);

  if (trMgr->settings.image.partThTr>=0) {
    myTr = Part_PartitionDisjSet (trBdd, NULL,
      trMgr->settings.image.partitionMethod, 
      trMgr->settings.image.partThTr, verbosity);
    setMonoTo = 1;
  }
  else {
    myTr = Ddi_BddDup(trBdd);
  }
  if (trMgr->settings.image.partThFrom>=0) {
    myFrom = Part_PartitionDisjSet (from, NULL,
      trMgr->settings.image.partitionMethod, 
      trMgr->settings.image.partThFrom, verbosity);
    setMonoTo = 1;
  }
  else {
    myFrom = Ddi_BddDup(from);
  }

  if (Ddi_BddIsPartDisj(myTr)) {
    to = TrImgDisjPartTr(trMgr, myTr, myFrom, psv, nsv, smoothV);
    setMonoTo = 1;
  }
  else if (Ddi_BddIsPartDisj(myFrom)) {
    to = TrImgDisjPartSet(trMgr, myTr, myFrom, psv, nsv, smoothV);
  }
  else {
    Pdtutil_Assert (Ddi_BddIsPartConj(myTr)||Ddi_BddIsMono(myTr),
      "Unexpected TR decomposition in image computation");

    switch (trMgr->settings.image.method) {
      case Tr_ImgMethodCofactor_c:
        Ddi_MgrAutodynSuspend(ddTR);
        Ddi_BddConstrainAcc(myTr,myFrom);
        Ddi_MgrAutodynResume(ddTR);
        Ddi_Free(myFrom);
        myFrom = Ddi_BddMakeConst(ddR,1);
      case Tr_ImgMethodMonolithic_c:
      case Tr_ImgMethodIwls95_c:
        to = TrImgConjPartTr(trMgr,myTr,myFrom,psv,nsv,smoothV);
        break;
      case Tr_ImgMethodApprox_c:
        to = TrImgApproxConjPartTr(trMgr,myTr,myFrom,psv,nsv,smoothV);
        break;
    }
  }

  Ddi_Free(myFrom);
  Ddi_Free(myTr);
  Ddi_Free(smoothV);

  if (Ddi_MgrCheckExtRef(ddR,extRef+1)==0) {
    Ddi_MgrPrintExtRef(ddR,nodeId);
  }


  if (setMonoTo)
    Ddi_BddSetMono(to);

  return(to);

}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Internal image computation function.]

  Description [Internal image computation function.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static Ddi_Bdd_t *
TrImgConjPartTr (
  Tr_Mgr_t *trMgr            /* Tr manager */,
  Ddi_Bdd_t *TR              /* Partitioned TR */,
  Ddi_Bdd_t *from            /* Input constrain */,
  Ddi_Vararray_t *psv        /* Array of present state variables */,
  Ddi_Vararray_t *nsv        /* Array of next state variables */,
  Ddi_Varset_t *smoothV      /* Variables to be abstracted */
  )
{
  Ddi_Bdd_t *to;
  Ddi_Mgr_t *ddTR = Ddi_ReadMgr(TR);     /* TR DD manager */
  Ddi_Mgr_t *ddR = Ddi_ReadMgr(from);  /* Reached DD manager */

  if (Ddi_BddIsPartDisj(TR)) {
    to = TrImgDisjPartTr(trMgr, TR, from, psv, nsv, smoothV);
  }
  else if (Ddi_BddIsPartDisj(from)) {
    to = TrImgDisjPartSet(trMgr, TR, from, psv, nsv, smoothV);
  }

  Pdtutil_Assert (Ddi_BddIsPartConj(TR)||Ddi_BddIsMono(TR),
    "Unexpected TR decomposition in image computation");

  from = Ddi_BddCopy (ddTR, from);
  
  to = Ddi_BddAndExist(TR,from,smoothV);

  Ddi_Free (from);

  /* Swap present state variables (PS) next state variables (NS) */
  Ddi_BddSwapVarsAcc(to,nsv,psv);

  /* transfer to reached manager */
  to = Ddi_BddEvalFree(Ddi_BddCopy(ddR,to),to);

  return (to);
}


/**Function********************************************************************

  Synopsis    [Compute image of a disjunctively partitioned transition
    relation]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static Ddi_Bdd_t *
TrImgDisjPartTr (
  Tr_Mgr_t *TrMgr           /* Tr manager */,
  Ddi_Bdd_t *TR              /* Partitioned TR */,
  Ddi_Bdd_t *from            /* Input state set */,
  Ddi_Vararray_t *psv       /* Array of present state variables */,
  Ddi_Vararray_t *nsv       /* Array of next state variables */,
  Ddi_Varset_t *smoothV     /* Variables to be abstracted */
)
{
  Ddi_Bdd_t *to, *to_i, *TR_i;
  int i;
  Ddi_Mgr_t *ddR = Ddi_ReadMgr(from);             /* dd manager */
  int verbosity = Tr_MgrReadVerbosity(TrMgr);
  
  if (Ddi_BddIsPartConj(TR))
    return TrImgConjPartTr (TrMgr,TR,from,psv,nsv,smoothV);

  to = Ddi_BddMakePartDisjVoid(ddR);

  if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
    fprintf (stdout, "DIsj TR IMG (%d partitions)\n",
       Ddi_BddPartNum(TR));
    fflush (stdout);
  }
    
  for (i=0;i<Ddi_BddPartNum(TR);i++) {
    TR_i = Ddi_BddPartRead(TR,i);
    if (verbosity > Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "Part[%d]\n", i);fflush(stdout);
    }
    to_i = TrImgConjPartTr (TrMgr,TR_i,from,psv,nsv,smoothV);
    if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
      fprintf (stdout, "\n|To[%d]|=%d\n", i, Ddi_BddSize(to_i));
      fflush (stdout);
    }
    Ddi_BddPartInsertLast(to,to_i);
    Ddi_Free (to_i);
  }

  return (to);
}

/**Function********************************************************************

  Synopsis    [Compute image of a disjunctively partitioned from set.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static Ddi_Bdd_t *
TrImgDisjPartSet (
  Tr_Mgr_t *TrMgr          /* Tr manager */,
  Ddi_Bdd_t *TR             /* Partitioned TR */,
  Ddi_Bdd_t *part_from      /* Input state set */,
  Ddi_Vararray_t *psv      /* Array of present state variables */,
  Ddi_Vararray_t *nsv      /* Array of next state variables */,
  Ddi_Varset_t *smoothV    /* Variables to be abstracted */
)
{
  Ddi_Bdd_t *to, *to_i, *from_i;
  int i;
  Ddi_Mgr_t *ddR = Ddi_ReadMgr(part_from);             /* dd manager */
  int verbosity = Tr_MgrReadVerbosity(TrMgr);
  
  Pdtutil_Assert (Ddi_BddIsPartDisj(part_from),
    "The from set of a partitioned image is not partitioned");

  to = Ddi_BddMakePartDisjVoid(ddR);

  if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
    fprintf (stdout, "DIsj FROM (%d partitions)\n",
       Ddi_BddPartNum (part_from));
  }
    
  for (i=0; i<Ddi_BddPartNum (part_from); i++) {
    if (verbosity > Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "Part[%d]\n", i);fflush(stdout);
    }
    from_i = Ddi_BddPartRead (part_from, i);
    to_i = TrImgConjPartTr (TrMgr,TR,from_i,psv,nsv,smoothV);
    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "\n|To[%d]|=%d\n", i, Ddi_BddSize(to_i));
      fflush (stdout);
    }
    Ddi_BddPartInsertLast(to,to_i);
    Ddi_Free(to_i);
  }

  return (to);
}

/**Function********************************************************************

  Synopsis    [Compute approx image of a conjunctively partitioned transition 
    relation.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static Ddi_Bdd_t *
TrImgApproxConjPartTr (
  Tr_Mgr_t *TrMgr          /* Tr manager */,
  Ddi_Bdd_t *TR            /* Partitioned TR */,
  Ddi_Bdd_t *from          /* Input state set */,
  Ddi_Vararray_t *psv      /* Array of present state variables */,
  Ddi_Vararray_t *nsv      /* Array of next state variables */,
  Ddi_Varset_t *smoothV    /* Variables to be abstracted */
)
{
  Ddi_Bdd_t *to, *to_i, *TR_i;
  int i;
  Ddi_Mgr_t *ddR = Ddi_ReadMgr(from);             /* dd manager */
  int verbosity = Tr_MgrReadVerbosity (TrMgr);
  
  if (!Ddi_BddIsPartConj(TR)) {
    Pdtutil_Warning (1,
      "No conjunctive TR for approx img - exact image done.");
    return (TrImgConjPartTr (TrMgr,TR,from,psv,nsv,smoothV));
  }

  to = Ddi_BddMakePartConjVoid(ddR);

  for (i=0; i<Ddi_BddPartNum(TR); i++) {
    TR_i = Ddi_BddPartRead (TR, i);
    to_i = TrImgConjPartTr (TrMgr,TR_i,from,psv,nsv,smoothV);
    if (verbosity > Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "[%d]", Ddi_BddSize (to_i));
      fflush(stdout);
    }
    Ddi_BddPartInsertLast(to,to_i);
    Ddi_Free (to_i);
  }

  Ddi_BddSetMono(to);

  return (to);
}










