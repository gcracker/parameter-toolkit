/**CFile***********************************************************************

  FileName    [fsmRetime.c]

  PackageName [fsm]

  Synopsis    [Functions to retiming a FSM for reachability analysis.]

  Description [External procedures included in this module:
    <ul>
    <li> Fsm_ComputeOptimalRetiming ()
    </ul>
    ]

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

#include "fsmInt.h"
#include "tr.h"
#include "part.h"

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
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis     []

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/

Fsm_Mgr_t *
Fsm_RetimeForRACompute (
  Fsm_Mgr_t *fsmMgr           /* FSM Manager */,
  Fsm_Retime_t *retimeStrPtr
  )
{
  Fsm_Mgr_t *fsmMgrRetimed;

  Fsm_EnableDependencyCompute (fsmMgr, retimeStrPtr);

  Fsm_CommonEnableCompute (fsmMgr, retimeStrPtr);

  Fsm_RetimeCompute (fsmMgr, retimeStrPtr);

  fsmMgrRetimed = Fsm_RetimeApply (fsmMgr, retimeStrPtr);

  return (fsmMgrRetimed);
}

/**Function********************************************************************

  Synopsis     []

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/

void
Fsm_EnableDependencyCompute (
  Fsm_Mgr_t *fsmMgr           /* FSM Manager */,
  Fsm_Retime_t *retimeStrPtr  /* Retime Structure */
  )
{
  Ddi_Bddarray_t *delta;
  Ddi_Vararray_t *piarray;
  Ddi_Vararray_t *sarray;
  Ddi_Vararray_t *yarray;
  Ddi_Var_t *s, *y;
  Ddi_Varset_t *supp;
  int i, ns;
  Ddi_Mgr_t *dd;
  Ddi_Bdd_t *d, *t, *en, *enbar, *tmp, *ten, *tenbar, *yeqs;

  delta = Fsm_MgrReadDeltaBDD (fsmMgr);
  piarray = Fsm_MgrReadVarI (fsmMgr);
  sarray = Fsm_MgrReadVarPS (fsmMgr);
  yarray = Fsm_MgrReadVarNS (fsmMgr);
  ns = Ddi_BddarrayNum (delta);

  dd = Ddi_ReadMgr (Ddi_BddarrayRead (delta, 0));

  retimeStrPtr->dataArray = Ddi_BddarrayAlloc(dd,ns);
  retimeStrPtr->enableArray = Ddi_BddarrayAlloc(dd,ns);

  for(i=0; i<ns; i++) {

    s = Ddi_VararrayRead(sarray,i);
    y = Ddi_VararrayRead(yarray,i);
    d = Ddi_BddarrayRead(delta,i);

#if 0
 fprintf (stdout, "VAR: %s ", Ddi_VarName(s));
#endif

    t = Ddi_BddXnor((tmp=Ddi_BddMakeLiteral(y,1)),d);
    Ddi_Free(tmp);
    supp = Ddi_VarsetVoid(dd);
    Ddi_VarsetAddAcc(supp,s);
    ten = Ddi_BddForall (t, supp);

#if 0
aux = Ddi_BddSupp(ten);
Ddi_VarsetPrint (aux, 1, NULL, stdout);
Ddi_BddPrintCubes(ten,aux,100,NULL,stdout);
#endif

    tenbar = Ddi_BddAnd(t,(tmp=Ddi_BddNot(ten)));
    Ddi_Free(tmp);

    yeqs = Ddi_BddMakeLiteral(y,1);
    yeqs = Ddi_BddEvalFree(Ddi_BddXnor((tmp=Ddi_BddMakeLiteral(s,1)),yeqs),yeqs);
    Ddi_Free(tmp);

#if 1
    if (Ddi_BddIncluded(tenbar,yeqs)) {
#else
    if (0) {
#endif
#if 0
      fprintf (stdout, "INCLUDED\n");
#endif
      Ddi_VarsetAddAcc(supp,y);
      enbar = Ddi_BddExist(tenbar,supp);
      en = Ddi_BddNot(enbar);
      Ddi_Free(enbar);
      d = Ddi_BddConstrain(ten,(tmp=Ddi_BddMakeLiteral(y,1)));
      Ddi_Free(tmp);
      Ddi_BddConstrainAcc(d,en);
    }
    else {
#if 0
      fprintf (stdout, "NOT INCLUDED\n");
      d = Ddi_BddAnd(tenbar,tmp=Ddi_BddNot(yeqs));
      aux = Ddi_BddSupp(d);
      Ddi_VarsetPrint (aux, 1, NULL,stdout);
      Ddi_BddPrintCubes(d,aux,100,NULL,stdout);
#endif

      en = Ddi_BddDup(Ddi_MgrReadOne(dd));
      d = Ddi_BddDup(d);
    }
    Ddi_Free(ten);
    Ddi_Free(tenbar);
    Ddi_Free(t);
    Ddi_Free(supp);

    Ddi_BddarrayWrite(retimeStrPtr->dataArray, i, d);
    Ddi_BddarrayWrite(retimeStrPtr->enableArray, i, en);

    Ddi_Free (d);
    Ddi_Free (en);

  }

  return;
}

/**Function********************************************************************

  Synopsis     []

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/

void
Fsm_CommonEnableCompute (
  Fsm_Mgr_t *fsmMgr           /* FSM Manager */,
  Fsm_Retime_t *retimeStrPtr  /* Retime Structure */
  )
{
  Ddi_Bddarray_t *delta;
  Ddi_Vararray_t *piarray;
  Ddi_Vararray_t *sarray;
  Ddi_Vararray_t *yarray;
  int i, j, ns;
  Ddi_Mgr_t *dd;

  delta = Fsm_MgrReadDeltaBDD (fsmMgr);
  piarray = Fsm_MgrReadVarI (fsmMgr);
  sarray = Fsm_MgrReadVarPS (fsmMgr);
  yarray = Fsm_MgrReadVarNS (fsmMgr);
  ns = Ddi_BddarrayNum (delta);

  dd = Ddi_ReadMgr (Ddi_BddarrayRead (delta, 0));

  retimeStrPtr->set = Pdtutil_Alloc (int, ns);
  retimeStrPtr->refEn = Pdtutil_Alloc (int, ns);
  retimeStrPtr->enCnt = Pdtutil_Alloc (int, ns);
  for (i=0; i<ns; i++) {
    retimeStrPtr->set[i] = 1;
    retimeStrPtr->refEn[i] = i;
    retimeStrPtr->enCnt[i] = 1;
  }

  for (i=0; i<ns; i++) {
    if (retimeStrPtr->set[i]) {
      for (j=i+1; j<ns; j++) {
        if ((retimeStrPtr->set[j])&&(retimeStrPtr->enCnt[j])&&
            (Ddi_BddEqual(Ddi_BddarrayRead (retimeStrPtr->enableArray, i),
            Ddi_BddarrayRead(retimeStrPtr->enableArray, j)))) {
          retimeStrPtr->enCnt[j] = 0;
          retimeStrPtr->enCnt[i]++;
          retimeStrPtr->refEn[j] = i;
        }
      }
    }
  }

  return;
}

/**Function********************************************************************

  Synopsis     []

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/

void
Fsm_RetimeCompute (
  Fsm_Mgr_t *fsmMgr           /* FSM Manager */,
  Fsm_Retime_t *retimeStrPtr  /* Retime Structure */
  )
{
  Ddi_Mgr_t *dd;
  Ddi_Bddarray_t *delta;
  Ddi_Vararray_t *piarray, *sarray, *yarray;
  Ddi_Var_t *s;
  Ddi_Varset_t *supp, *aux, *aux0, *latches, *inputs,
    *retimed, *notRetimed, *retimedInputs;
  Ddi_Varset_t **deltaSupp, **enableSupp, **retimeLatchSets, **auxSupp;
  Ddi_Bdd_t *en, *enk, *common;
  Ddi_Vararray_t *auxVararray;
  Ddi_Var_t *auxVar;
  Ddi_Bddarray_t *auxFunArray;
  Pdtutil_VerbLevel_e verbosity;
  int *retimeDeltas, *removableLatches, *appear, *disappear;
  int i, j, k, ns, max, imax, nret, nAuxVar, again;

  verbosity = Fsm_MgrReadVerbosity (fsmMgr);

  delta = Fsm_MgrReadDeltaBDD (fsmMgr);
  piarray = Fsm_MgrReadVarI (fsmMgr);
  sarray = Fsm_MgrReadVarPS (fsmMgr);
  yarray = Fsm_MgrReadVarNS (fsmMgr);
  ns = Ddi_BddarrayNum (delta);

  dd = Ddi_ReadMgr (Ddi_BddarrayRead (delta, 0));

  max = 0;
  imax = -1;

  removableLatches = Pdtutil_Alloc(int, ns);
  retimeDeltas = Pdtutil_Alloc(int, ns);
  retimeLatchSets = Pdtutil_Alloc(Ddi_Varset_t *, ns);
  inputs = Ddi_VarsetMakeFromArray (piarray);
  deltaSupp = Ddi_BddarraySuppArray (retimeStrPtr->dataArray);
  enableSupp = Ddi_BddarraySuppArray (retimeStrPtr->enableArray);

  nAuxVar = Fsm_MgrReadNAuxVar(fsmMgr);
  if (nAuxVar > 0) {
    auxFunArray = Fsm_MgrReadAuxVarBDD (fsmMgr);
    auxVararray = Fsm_MgrReadVarAuxVar (fsmMgr);
    auxSupp = Ddi_BddarraySuppArray (auxFunArray);

    do {
    again = 0;
    for (i=0; i<nAuxVar; i++) {
      auxVar = Ddi_VararrayRead(auxVararray,i);
      for (j=0; j<ns; j++) {
        Pdtutil_Assert ((!Ddi_VarInVarset(enableSupp[j],auxVar)),
          "NOT supported aux VAR in enable function");
        supp = deltaSupp[j];
        if (Ddi_VarInVarset(supp,auxVar)) {
          aux = Ddi_VarsetUnion(supp,auxSupp[i]);
          Ddi_Free(supp);
          deltaSupp[j] = Ddi_VarsetRemove(aux,auxVar);
          Ddi_Free(aux);
          again = 1;
	}
      }
    }
    } while (again);
  }

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    fprintf (stdout, "DATA and ENABLE supports\n");
    for (i=0; i<ns; i++) {
      s = Ddi_VararrayRead(sarray,i);
      fprintf (stdout, "S[%d]: %s\n", i, Ddi_VarName(s));
      fprintf (stdout, " D[%d]\n", i);
      Ddi_VarsetPrint (deltaSupp[i], 1, NULL,stdout);
      fprintf (stdout, " E[%d]\n", i);
      Ddi_VarsetPrint (enableSupp[i], 1, NULL, stdout);
      fprintf (stdout, "------------------------------\n");
    }
    fprintf (stdout, "DATA and ENABLE supports - END\n");
  }

  /* allocate data structures to generate retimed FSM */

  appear = Pdtutil_Alloc(int, ns);
  disappear = Pdtutil_Alloc(int, ns);
  for (i=0; i<ns; i++) {
    appear[i] = -1;
    disappear[i] = 0;
  }

  for (i=0; i<ns; i++) {
    retimeLatchSets[i] = NULL;
    if ((retimeStrPtr->set[i])&&(retimeStrPtr->refEn[i]==i)) {
      /* this is a set of latches with common enable */
      /* initialize set with reference latch */
      latches = Ddi_VarsetVoid(dd);
      s = Ddi_VararrayRead(sarray,i);
      Ddi_VarsetAddAcc(latches,s);

      if (retimeStrPtr->enCnt[i] > max) {
        max = retimeStrPtr->enCnt[i];
        imax = i;
      }
      en = Ddi_BddarrayRead (retimeStrPtr->enableArray, i);

      /* now add latches with common enable */

      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "\nref: %d (size: %d)\n", i, retimeStrPtr->enCnt[i]);
      }
      for (j=0; j<ns; j++) {
        if ((retimeStrPtr->set[j])&&(retimeStrPtr->refEn[j]==i)) {
          s = Ddi_VararrayRead(sarray,j);
          Ddi_VarsetAddAcc(latches,s);

          if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
            fprintf (stdout, "%s ", Ddi_VarName(s));
	  }
        }
      }
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "\n");
      }

      retimeLatchSets[i] = latches;

      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "latches: ");
        Ddi_VarsetPrint (latches, 1, NULL, stdout);
      }

      /* search candidates for retiming: deltas with support including
         PIs and FFs in set "latches". Other latches are allowed
         with enable excluded by enabling function of present set ("latches")
      */

      for (j=0; j<ns; j++) {
        supp = deltaSupp[j];
        retimeDeltas[j] = 0;
        aux0 = Ddi_VarsetDiff(supp,inputs);
        if (!Ddi_VarsetIsVoid(aux0)) {
          /* isolate FFs not in "latches" */
          aux = Ddi_VarsetDiff(aux0,latches);

          /* support of delta is in latches or inputs with the exception
             of latches disabled when "latches" enabled */

          if (Ddi_VarsetIsVoid(aux)) {
            /* support of delta is in latches or inputs */
            retimeDeltas[j] = 1;
          }
          else if (Ddi_VarsetNum(aux)<Ddi_VarsetNum(aux0)) {
            /* support of delta is in latches or inputs plus other latches */

            retimeDeltas[j] = 2;
            /* check enable of other latches: null intersection with present
               enable */
            while (!Ddi_VarsetIsVoid(aux)) {
              for (k=0;k<ns;k++) {
                s = Ddi_VararrayRead(sarray,k);
                if (Ddi_VarIndex(s) == Ddi_VarIndex(Ddi_VarsetTop(aux))) {
                  enk = Ddi_BddarrayRead(retimeStrPtr->enableArray, k);
                  common = Ddi_BddAnd(en,enk);
                  if (!Ddi_BddIsZero(common)) {
                    retimeDeltas[j] = 0;
                    Ddi_Free(common);
                  break;
                  }
                  else
                    Ddi_Free(common);
                }
              }
              Ddi_VarsetNextAcc(aux);
            }
          }
          Ddi_Free(aux);

          if (retimeDeltas[j] != 0) {
            if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
              s = Ddi_VararrayRead (sarray, j);
              fprintf (stdout, "DeltaSupp[%d](%s): ", j, Ddi_VarName (s));
              Ddi_VarsetPrint (supp, 1, NULL, stdout);
            }
          }
        }
        Ddi_Free(aux0);
      }

      notRetimed = Ddi_BddarraySupp(retimeStrPtr->enableArray);
      for (j=0; j<ns; j++) {
        if (retimeDeltas[j]==0) {
          
            Ddi_VarsetUnionAcc(notRetimed,deltaSupp[j]);
        }
      }

      Fsm_OptimalRetimingCompute (dd, verbosity, retimeStrPtr->retimeEqualFlag,
        retimeDeltas, deltaSupp, retimeStrPtr->enableArray, en, sarray, ns,
        latches, inputs, notRetimed);

      Ddi_Free(notRetimed);

      /* setup data for true retiming */

      retimed = Ddi_VarsetVoid(dd);
      /* notRetimed = Ddi_VarsetVoid(dd); */
      notRetimed = Ddi_BddarraySupp(retimeStrPtr->enableArray);
      
      for (j=0,nret=0; j<ns; j++) {
        if ((retimeDeltas[j])&&(appear[j]<0)) {
          
            Ddi_VarsetUnionAcc(retimed,deltaSupp[j]);
          nret++;
        }
        else {
          
            Ddi_VarsetUnionAcc(notRetimed,deltaSupp[j]);
        }
      }
      retimedInputs = Ddi_VarsetIntersect(retimed,inputs);
      
        Ddi_VarsetIntersectAcc(retimed,latches);

      if (!Ddi_VarsetIsVoid(retimed)) {

        aux = Ddi_VarsetIntersect(retimed,notRetimed);
        aux0 = Ddi_VarsetIntersect(retimedInputs,notRetimed);
        if (Ddi_VarsetIsVoid(aux)&&Ddi_VarsetIsVoid(aux0)) {
          fprintf (stdout, "Full Retiming Done: %d -> %d, gain = %d\n",
            Ddi_VarsetNum (retimed), nret, Ddi_VarsetNum (retimed)-nret);

          for (j=0; j<ns; j++) {
            s = Ddi_VararrayRead(sarray,j);
            if (Ddi_VarInVarset(retimed,s)) {
              disappear[j]=1;
            }
            if ((retimeDeltas[j])&&(appear[j]<0)) {
              appear[j]=i;
            }
          }

        }
        else {
          fprintf (stdout,
            "Partial Retiming Done: %d[shL: %d / shI: %d] -> %d, gain = %d\n",
            Ddi_VarsetNum(retimed), Ddi_VarsetNum(aux), Ddi_VarsetNum(aux0),
            nret, Ddi_VarsetNum (retimed)-nret);
        }
        Ddi_Free(aux);
        Ddi_Free(aux0);
      } else {
        fprintf (stdout, "ALL RETIMED LATCHES ARE SHARED.\n");
      }

      Ddi_Free(retimed);
      Ddi_Free(notRetimed);

    }

  }

  if (retimeStrPtr->removeLatches == 1) {
    Fsm_FindRemovableLatches (dd, removableLatches, delta, sarray, inputs, ns);
  }

  retimeStrPtr->retimeVector = Pdtutil_Alloc (int, ns);
  retimeStrPtr->retimeGraph = Pdtutil_Alloc (Ddi_Bddarray_t *, ns);

  for (j=0; j<ns; j++) {
    retimeStrPtr->retimeGraph[j] = Ddi_BddarrayAlloc (dd,0);
    if ((disappear[j]==0)&&
       ((retimeStrPtr->removeLatches==0)||(removableLatches[j]==0))) {
      /* keep current latch */
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "RA-INS[%d]\n", j);
      }
      en = Ddi_BddarrayRead (retimeStrPtr->enableArray, j);
      Ddi_BddarrayInsert (retimeStrPtr->retimeGraph[j], 0, en);
    }
    if (appear[j]>=0) {
      /* a new retimed latch appears */
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "RA-INS2[%d]\n", j);
      }
      Pdtutil_Assert((appear[j]>=0)&&(appear[j]<ns),"invalid retime ID");
      en = Ddi_BddarrayRead (retimeStrPtr->enableArray, appear[j]);
      Ddi_BddarrayInsert (retimeStrPtr->retimeGraph[j], 0, en);
      retimeStrPtr->retimeVector[j] = 1;
    }
    else {
      retimeStrPtr->retimeVector[j] = 0;
    }
  }

  return;
}


/**Function*******************************************************************

  Synopsis    []

  Description []

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

void
Fsm_OptimalRetimingCompute (
  Ddi_Mgr_t *dd,
  Pdtutil_VerbLevel_e verbosity, 
  int retimeEqual,
  int *retimeDeltas,
  Ddi_Varset_t **deltaSupp,
  Ddi_Bddarray_t *enableArray,
  Ddi_Bdd_t *en,
  Ddi_Vararray_t *sarray,
  int ns,
  Ddi_Varset_t *latches,
  Ddi_Varset_t *inputs,
  Ddi_Varset_t *notRetimed
  )
{
  int i, again, nret;
  int *retime0;
  Ddi_Var_t *s;
  Ddi_Varset_t *aux, *joinsupp=NULL, *common;

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    fprintf (stdout, "computing optimal retiming\n");
  }

  retime0 = Pdtutil_Alloc(int, ns);
  common = Ddi_VarsetUnion(inputs,latches);

  for (i=0; i<ns; i++) {
    retime0[i] = 0;
  }

  again = 1;
  nret = 0;
  while (again) {
    for (i=0,again=0; i<ns; i++) {
      if (retimeDeltas[i]) {
        if (joinsupp==NULL) {
          joinsupp = Ddi_VarsetIntersect(deltaSupp[i],common);
        }
        Pdtutil_Assert(deltaSupp[i]!=NULL,"NULL deltasupp");
        aux = Ddi_VarsetIntersect(deltaSupp[i],joinsupp);
        if (!Ddi_VarsetIsVoid(aux)) {
          /* a common support exists */
          retime0[i] = retimeDeltas[i];
          retimeDeltas[i] = 0;
          nret++;
          
            Ddi_VarsetUnionAcc(joinsupp,deltaSupp[i]);
          
            Ddi_VarsetIntersectAcc(joinsupp,common);
          again = 1;
        }
        Ddi_Free(aux);

      }
    }
  }

  if (nret) {
    /* progress: some retime attempted */

    /* recur */
    Fsm_OptimalRetimingCompute (dd, verbosity, retimeEqual, retimeDeltas, 
      deltaSupp, enableArray, en, sarray, ns, latches, inputs, notRetimed);

    /* check whether retiming local set decreases latch num */
    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "#latches %d - #deltas: %d\n",
        Ddi_VarsetNum(joinsupp), nret);
    }

    aux = Ddi_VarsetIntersect(joinsupp,notRetimed);
    if ((Ddi_VarsetIsVoid(aux))&&(
      ((retimeEqual == 2)) ||
      ((retimeEqual == 1) && (Ddi_VarsetNum (joinsupp) >= nret)) ||
      ((retimeEqual == 0) && (Ddi_VarsetNum (joinsupp) > nret))
      )) {

      /* more input latches than output ones */
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "\nRetiming group: ");
      }

      for (i=0; i<ns; i++) {
        switch (retime0[i]) {
        case 1:
          retimeDeltas[i] = 1;
          s = Ddi_VararrayRead(sarray,i);
          if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
            fprintf (stdout, "[%s] ", Ddi_VarName(s));
	  }
          break;
        case 2:
          retimeDeltas[i] = 1;
          s = Ddi_VararrayRead(sarray,i);
          if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
            printf ("{%s} ", Ddi_VarName(s));
          }
          break;
        }
      }
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        printf("\nRetimed latches:\n");
      }
      Ddi_VarsetPrint (joinsupp, 1, NULL, stdout);
    }
    else {
#if 0
      printf("\nNO Retiming for group: ");
      Ddi_VarsetPrint (latches, 1, NULL, stdout);
#endif
    }
    Ddi_Free(aux);
  }

  Pdtutil_Free(retime0);

  Ddi_Free(common);

  return;
}



/**Function*******************************************************************

  Synopsis    []

  Description []

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

void
Fsm_FindRemovableLatches (
  Ddi_Mgr_t *dd,
  int *removableLatches,
  Ddi_Bddarray_t *delta,
  Ddi_Vararray_t *sarray,
  Ddi_Varset_t *inputs,
  int ns
  )
{
  Ddi_Var_t *s;
  Ddi_Varset_t *supp, *supp2, *aux, *aux2;
  int j, k, again, remove, rinp, self, inp;
  Ddi_Varset_t **deltaSupp;

  remove = 0;
  rinp = 0;
  inp = 0;
  self = 0;

  for (j=0; j<ns; j++) {
    removableLatches[j] = 0;
  }

#if 1
  deltaSupp = Ddi_BddarraySuppArray (delta);

  for (j=0; j<ns; j++) {

    s = Ddi_VararrayRead(sarray,j);
    supp = deltaSupp[j];

    aux = Ddi_VarsetDiff(supp,inputs);
    if (Ddi_VarsetIsVoid(aux)) {
      inp++;
      removableLatches[j] = 1;            
    }

    Ddi_Free(aux);

  }

  do {

    again = 0;
    for (j=0; j<ns; j++) {

      if (removableLatches[j] == 1) {
   
        supp = deltaSupp[j];

        for (k=0; ((k<ns)&&(removableLatches[j])); k++) {
          if (removableLatches[k] == 0) {
            supp2 = deltaSupp[k];
            aux2 = Ddi_VarsetIntersect(supp2,supp);
            if (!Ddi_VarsetIsVoid(aux2)) {
              removableLatches[j] = 0;            
              again = 1;
	    }
            Ddi_Free(aux2);
	  }
        }

      }
    }

  } while (again);


  for (j=0; j<ns; j++) {
    if (removableLatches[j] == 1) {
      rinp++;
    }
  }

#endif

#if 0
  Ddi_Bdd_t *d, *s0, *s1, *eq, *totDep, *dep, *cof0, *cof1;

    else {

      s0 = Ddi_BddMakeLiteral(s,0);
      s1 = Ddi_BddMakeLiteral(s,1);
      eq = Ddi_BddXnor(s1,d);

      totDep = Ddi_MgrReadZero(dd);

      for (k=0; k<ns; k++) {
        if (k!=j) {
          d = Ddi_BddarrayRead(delta,k);
          cof0 = Ddi_BddConstrain(d,s0);
          cof1 = Ddi_BddConstrain(d,s1);
          dep = Ddi_BddXor(cof0,cof1);
          Ddi_Free(cof0);
          Ddi_Free(cof1);
          Ddi_BddOrAcc(totDep,dep);
          Ddi_Free(dep);
        }
      }

      Ddi_Free(s0);
      Ddi_Free(s1);

      if (Ddi_BddIncluded(totDep,eq)) {
        remove++;
        removableLatches[j] = 1;
      }

      Ddi_Free(eq);
      Ddi_Free(totDep);

    }


    if ((removableLatches[j]==1)&&(Ddi_VarInVarset(supp,s))) {
      self++;
    }



  }
#endif

  fprintf (stdout, "REMOVABLE LATCHES: (%d+%d/%d)(self: %d)/%d\n", 
    remove, rinp, inp, self, ns);

  return;
}

/**Function********************************************************************

  Synopsis     [Given a Retiming Array Apply it to the FSM Manager.
    Return a new FSM Manager.]

  Description  [There should be a 1 to 1 correspondence between the
    retiming array (retimeStrPtr->retimeArray) and the delta array.]

  SideEffects  []

  SeeAlso      []

******************************************************************************/

Fsm_Mgr_t *
Fsm_RetimeApply (
  Fsm_Mgr_t *fsmMgr           /* FSM Manager */,
  Fsm_Retime_t *retimeStrPtr  /* Retime Structure */
  )
{
  Ddi_Mgr_t *ddMgr;
  Ddi_Bddarray_t *deltaOldArray, *deltaArray, *dataArray, *enableArray,
    *auxFunArray;
  Ddi_Bddarray_t **retimeGraph, *retimeArray, *recodeDelta;
  Ddi_Vararray_t *psOldArray, *nsOldArray, *psArray, *nsArray,
    *auxVararray;
  Ddi_Var_t *auxVar, *psVar, *nsVar;
  Ddi_Bdd_t *one, *delta, *data, *enable, *lit = NULL, *s0, *recodeTR;
  Fsm_Mgr_t *fsmMgrRetimed;
  Ddi_Varset_t *quantify, *pis;
  int i, j, level, level1,
    psOldNumber, psNumber, nFF, auxVarNumber;
  char varname[FSM_MAX_STR_LEN];

  Ddi_Mgr_t *dd = Fsm_MgrReadDdManager(fsmMgr);

  /*----------------------- Take Fields From FSM Manager --------------------*/

  ddMgr = Fsm_MgrReadDdManager (fsmMgr);

  one = Ddi_MgrReadOne (ddMgr);

  psOldArray = Fsm_MgrReadVarPS (fsmMgr);
  nsOldArray = Fsm_MgrReadVarNS (fsmMgr);
  psOldNumber = Fsm_MgrReadNL (fsmMgr);
  deltaOldArray = Fsm_MgrReadDeltaBDD (fsmMgr);

  /*---------------------- Make a Copy of the FSM Manager -------------------*/

  fsmMgrRetimed = Fsm_MgrDup (fsmMgr);

  /*----------- PATCH:  Set Fields I Want into Retime Structure -------------*/

#if 0
  retimeStrPtr->dataArray =  Ddi_BddarrayAlloc (psOldNumber);
  retimeStrPtr->enableArray = Ddi_BddarrayAlloc (psOldNumber);
  retimeStrPtr->retimeGraph = Pdtutil_Alloc (Ddi_Bddarray_t *, psOldNumber);

  for (i=0; i<psOldNumber; i++) {
    delta = Ddi_BddarrayRead (deltaOldArray, i);
    Ddi_BddarrayWrite (retimeStrPtr->dataArray, i, delta);
    Ddi_BddarrayWrite (retimeStrPtr->enableArray, i, one);
    retimeStrPtr->retimeGraph[i] = Ddi_BddarrayAlloc (0);
    Ddi_BddarrayWrite (retimeStrPtr->retimeGraph[i], 0, one);
  }
#endif

  /*-------------------------- Take Retiming Options ------------------------*/

  retimeGraph = retimeStrPtr->retimeGraph;
  dataArray = retimeStrPtr->dataArray;
  enableArray = retimeStrPtr->enableArray;

  /*---------------------------- Apply Retiming -----------------------------*/

  /* Auxiliary Structures */
  auxVarNumber = Fsm_MgrReadNAuxVar (fsmMgr);
  if (auxVarNumber == 0) {
    auxFunArray = Ddi_BddarrayAlloc (dd,0);
    auxVararray = Ddi_VararrayAlloc (dd,0);
  }
  else {
    auxFunArray = Ddi_BddarrayDup(Fsm_MgrReadAuxVarBDD (fsmMgr));
    auxVararray = Ddi_VararrayDup(Fsm_MgrReadVarAuxVar (fsmMgr));
  }

  /* Present State and Next State Variables, Next State Functions */
  psNumber = 0;
  deltaArray = Ddi_BddarrayAlloc (dd,0);
  psArray = Ddi_VararrayAlloc (dd,0);
  nsArray = Ddi_VararrayAlloc (dd,0);

  recodeDelta = Ddi_BddarrayAlloc (dd,0);

  /*
   *  Main Cycle
   */

  for (i=0; i<psOldNumber; i++) {
    retimeArray = retimeGraph[i];

    nFF = Ddi_BddarrayNum (retimeArray);

    /*
     *  Empty Retime Array: Create Auxiliary Variable
     */

    if (retimeArray == NULL || nFF == 0) {
      data = Ddi_BddarrayRead (dataArray, i);
      auxVar = Ddi_VararrayRead (psOldArray, i);

      Ddi_VararrayWrite (auxVararray, auxVarNumber, auxVar);
      Ddi_BddarrayInsertLast (auxFunArray, data);

      auxVarNumber++;

      continue;
    }

    /*
     *  Create Delta Functions with psArray and nsArray
     */

    for (j=0; j<nFF; j++) {
      enable = Ddi_BddarrayRead (retimeArray, j);

      /* First Element: Use Usual Data */
      if (j == 0) {
        data = Ddi_BddarrayRead (dataArray, i);
      } else {
        data = Ddi_BddDup (lit);
      }

      /* Last Element: Use Usual Present State Variable */
      if (j == (nFF-1)) {
        psVar = Ddi_VararrayRead (psOldArray, i);
        nsVar = Ddi_VararrayRead (nsOldArray, i);
      } else {
        psVar = Ddi_VararrayRead (psOldArray, i);
        nsVar = Ddi_VararrayRead (nsOldArray, i);

        sprintf (varname, "PdtAuxVar_%s", Ddi_VarName(psVar));

        level = Ddi_VarCurrPos(psVar);
        level1 = Ddi_VarCurrPos(nsVar);
        if (level1 > level) {
          level = level1;
        }

        nsVar = Ddi_VarNewAtLevel (ddMgr, level+1);
        psVar = Ddi_VarNewAtLevel (ddMgr, level+1);

        Ddi_VarAttachName(psVar,varname);
        strcat(varname,"$NS");
        Ddi_VarAttachName(nsVar,varname);

      }

      lit = Ddi_BddMakeLiteral(psVar,1);
      delta = Ddi_BddIte (enable, data, lit);

      Ddi_VararrayWrite (psArray, psNumber, psVar);
      Ddi_VararrayWrite (nsArray, psNumber, nsVar);
      Ddi_BddarrayInsertLast (deltaArray, delta);

      if ((j==0)&&(retimeStrPtr->retimeVector[i]==1)) {
        Ddi_BddarrayInsertLast (recodeDelta, data);
      } else {
        Pdtutil_Assert(j==nFF-1,"wrong data for initial state");
        Ddi_BddarrayInsertLast (recodeDelta, lit);
      }

      psNumber++;
    }
  }

  /*--------------------- Compute new init state set  ----------------------*/

  recodeTR = Ddi_BddRelMakeFromArray(recodeDelta, nsArray);

  quantify = Ddi_VarsetMakeFromArray (Fsm_MgrReadVarPS (fsmMgrRetimed));
  pis = Ddi_VarsetMakeFromArray (Fsm_MgrReadVarI (fsmMgrRetimed));
  Ddi_VarsetUnionAcc (pis, quantify);
  Ddi_Free(pis);

  Ddi_BddPartInsert(recodeTR,0, Fsm_MgrReadInitBDD (fsmMgrRetimed));
  s0 = Part_BddMultiwayLinearAndExist (recodeTR,quantify,-1);
  Ddi_BddSwapVarsAcc(s0,psArray,nsArray);

  Ddi_Free(quantify);

  /*----------------------- Free Old Functions, ... ------------------------*/

  Ddi_Free (fsmMgrRetimed->var.i);
  Ddi_Free (fsmMgrRetimed->var.ps);
  Ddi_Free (fsmMgrRetimed->var.ns);
  Ddi_Free (fsmMgrRetimed->delta.bdd);
  Ddi_Free (fsmMgrRetimed->init.bdd);

  /*--------- Modify the FSM Manager Following Retiming Results  ------------*/

  Fsm_MgrSetNL (fsmMgrRetimed, psNumber);
  Fsm_MgrSetVarPS (fsmMgrRetimed, psArray);
  Fsm_MgrSetVarNS (fsmMgrRetimed, nsArray);
  Fsm_MgrSetDeltaBDD (fsmMgrRetimed, deltaArray);
  Fsm_MgrSetNAuxVar (fsmMgrRetimed, auxVarNumber);
  Fsm_MgrSetVarAuxVar (fsmMgrRetimed, auxVararray);
  Fsm_MgrSetAuxVarBDD (fsmMgrRetimed, auxFunArray);

#if 0
  s0 = Ddi_MgrReadOne (ddMgr);
  for (i=0; i<psNumber; i++) {
    var = Ddi_VararrayRead (psArray, i);
    lit = Ddi_BddMakeLiteral(var,1);
    Ddi_BddAndAcc (s0, lit);
  }
#endif

  Fsm_MgrSetInitBDD (fsmMgrRetimed, s0);


#if 0
  Problema:
  ora Ddi_MgrOrdWrite cerca di scrivere piu` variabili di quante ne
  ha la FSM in quanto alcune variabili possono essere create durante
  il retiming
  Fsm_MgrStore (fsmMgr, "fsmOld", NULL, 1, DDDMP_MODE_TEXT, 1);
  Fsm_MgrStore (fsmMgrRetimed, "fsmNew", NULL, 1, DDDMP_MODE_TEXT, 1);
#endif

  /*------------------------- Return New FSM Manager ------------------------*/

  /*
   *  Alla fine di questo ambaradan ho una FSM con auxVar e auxFun ...
   *  devo tenere in conto di questo in tr_init o simili ...
   */

  return (fsmMgrRetimed);
}
