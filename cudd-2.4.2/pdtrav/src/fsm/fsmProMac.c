/**CFile***********************************************************************

  FileName    [fsmProMac.c]

  PackageName [fsm]

  Synopsis    [Functions to build a Product Machine]

  Description [External procedures included in this module:
    <ul>
    <li> Fsm_MgrPMBuild () 
    </ul>
    ]

  SeeAlso     []  

  Author    [Gianpiero Cabodi and Stefano Quer]

  Copyright [Copyright (c) 2001 by Politecnico di Torino.
    All Rights Reserved. This software is for educational purposes only.
    Permission is given to academic institutions to use, copy, and modify
    this software and its documentation provided that this introductory
    message is not removed, that this software and its documentation is
    used for the institutions' internal research and educational purposes,
    and that no monies are exchanged. No guarantee is expressed or implied
    by the distribution of this code.
    Send bug-reports and/or questions to: {cabodi,quer}@polito.it.
    ]
  
  Revision    []

******************************************************************************/

#include "fsmInt.h"

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

  Synopsis           []
  
  Description        []
  
  SideEffects        []

  SeeAlso            [get_name_file,readFsm]

******************************************************************************/

int
Fsm_MgrPMBuild (
  Fsm_Mgr_t **fsmMgrPMP     /* FSM Product Machine Pointer */,
  Fsm_Mgr_t *fsmMgr1        /* First FSM */,
  Fsm_Mgr_t *fsmMgr2        /* Second FSM */
  )
{
  Fsm_Mgr_t *fsmMgrPM;
  Ddi_Mgr_t *ddMgr1, *ddMgr2;
  Ddi_Var_t *tmpVar1, *tmpVar2, *vOld, *vMap, *vNew;
  Ddi_Vararray_t *psv1, *psv2, *psv2New;
  Ddi_Bddarray_t *tmpBddArray1, *tmpBddArray2, *tmpBddArrayPM;
  Ddi_Bdd_t *bdd, *oldBDD, *newBDD;
  int i, j, flagError;
  int index, ni1, ni2, no1, no2, nl1, nl2;
  int *indexIFsm1, *indexIFsm2, *indexPSFsm1, *indexPSFsm2, *indexPSPM;
  int *indexMgr1, *indexMgr2;
  char **namePSPM, **nameOPM;
  char tmpArray[PDTUTIL_MAX_STR_LEN], *word;

  /*------------------------ Check For FSM Structure ------------------------*/

  fsmMgrPM = *fsmMgrPMP;
  if (fsmMgrPM == NULL) {
    Pdtutil_Warning (1, "Allocate a new FSM structure");
    fsmMgrPM = Fsm_MgrInit ("pm");
  }

  /* Debug ... Congruence Check */
  Fsm_MgrStore (fsmMgr1, "fsm1", NULL, 1, DDDMP_MODE_TEXT, 1);
  Fsm_MgrStore (fsmMgr2, "fsm2", NULL, 1, DDDMP_MODE_TEXT, 1); 

  /*
   *  Copy Fields From fsm1 and fsm2 into pm
   */

  ddMgr1 = Fsm_MgrReadDdManager (fsmMgr1);
  ddMgr2 = Fsm_MgrReadDdManager (fsmMgr2);

  ni1 = Fsm_MgrReadNI (fsmMgr1);
  no1 = Fsm_MgrReadNO (fsmMgr1);
  nl1 = Fsm_MgrReadNL (fsmMgr1);
  ni2 = Fsm_MgrReadNI (fsmMgr2);
  no2 = Fsm_MgrReadNO (fsmMgr2);
  nl2 = Fsm_MgrReadNL (fsmMgr2);

  psv1 = Fsm_MgrReadVarPS (fsmMgr1);
  psv2 = Fsm_MgrReadVarPS (fsmMgr2);

  /*
   *  A Few Checks
   */

  if (ni1!=ni2) {
    Pdtutil_Warning (1, "Number of Inputs do not coincide.");
    return (1);
  }

  /* For Now I FORCE PM ONLY for FSMs with Same Number of Latches */
  if (nl1!=nl2) {
    Pdtutil_Warning (1, "Number of Inputs do not coincide.");
    return (1);
  }

  if (no1!=no2) {
    Pdtutil_Warning (1, "Number of Outputs do not coincide.");
    return (1);
  }

  indexIFsm1 = Pdtutil_Alloc (int, ni1);
  indexIFsm2 = Pdtutil_Alloc (int, ni2);
  indexPSFsm1 = Pdtutil_Alloc (int, nl1);
  indexPSFsm2 = Pdtutil_Alloc (int, nl2);
  indexPSPM = Pdtutil_Alloc (int, (nl1+nl2));
  indexMgr1 = Pdtutil_Alloc (int, (ni1+nl1));
  indexMgr2 = Pdtutil_Alloc (int, (ni2+nl2));
  namePSPM = Pdtutil_Alloc (char *, (nl1+nl2));
  nameOPM = Pdtutil_Alloc (char *, no1);

  /*
   *  Remap Input Variables
   */

  if ((Fsm_MgrReadNameI (fsmMgr1))!=NULL &&
    (Fsm_MgrReadNameI (fsmMgr2))!=NULL) {
    for (i=0; i<ni1; i++) {
      flagError = 1;
      for (j=0; j<ni2; j++) {
        if (strcmp (fsmMgr1->name.i[i], fsmMgr2->name.i[j]) == 0) {
          indexIFsm1[i] = fsmMgr1->index.i[i];
          indexIFsm2[i] = fsmMgr2->index.i[j];
          flagError = 0;
          break;
        }
      }
      if (flagError == 1) {
        Pdtutil_Warning (1, "Input names do not coincide.");
      }
    }
  }

  /*
   *  Remap Present State Variables
   */

  psv2New = Ddi_VararrayAlloc(ddMgr2,0);
  for (i=0; i<Ddi_VararrayNum(psv1); i++) {
    vOld = Ddi_VararrayRead(psv1,i);
    vMap = Ddi_VarFromName(ddMgr2,Ddi_VarName(vOld));
    if (vMap != NULL) {
      vNew = Ddi_VarNewAfterVar(vMap);
    } else {
      vNew = Ddi_VarNew(ddMgr2);
      Pdtutil_Warning (1, "Present State Names do not coincide.");
    }
    Ddi_VararrayWrite(psv2New,i,vNew);

    indexPSPM[2*i] = Ddi_VarIndex (vOld);
    indexPSPM[2*i+1] = Ddi_VarIndex (vNew);

    sprintf (tmpArray, "fsm1-%s", Ddi_VarName (vOld));
    Ddi_VarDetachName (vOld);
    Ddi_VarAttachName (vOld, tmpArray);
    namePSPM[2*i] = Pdtutil_StrDup (tmpArray);

    sprintf (tmpArray, "fsm2-%s", Ddi_VarName (vOld));
    Ddi_VarAttachName (vNew, tmpArray);
    namePSPM[2*i+1] = Pdtutil_StrDup (tmpArray);
    /*Ddi_VarAttachAuxid (Ddi_IthVar (dd, i), varauxids[i]);*/
  }

  /* Check and Copy Output Names */
  if ((Fsm_MgrReadNameO (fsmMgr1))!=NULL &&
    (Fsm_MgrReadNameO (fsmMgr2))!=NULL ) {
    for (i=0; i<no1; i++) {
      flagError = 1;
      for (j=0; j<ni1; j++) {
        if (strcmp (fsmMgr1->name.o[i], fsmMgr2->name.o[j]) == 0) {
          sprintf (tmpArray, "%s-xor-%s", fsmMgr1->name.o[i],
            fsmMgr2->name.o[i]);
          nameOPM[i] = Pdtutil_StrDup (tmpArray);
          flagError = 0;
          break;
        }
      }
      if (flagError == 1) {
        Pdtutil_Warning (1, "Output names do not coincide.");
      }
    }
  }

  /*
   *  Start Setting Fileds
   */

  Fsm_MgrSetDdManager (fsmMgrPM, ddMgr2);

  fsmMgrPM->size.i = ni1;
  fsmMgrPM->size.o = no1;
  fsmMgrPM->size.l = nl1 + nl2;
  fsmMgrPM->size.auxVar = (-1);

  fsmMgrPM->name.i = Pdtutil_StrArrayDup (fsmMgr1->name.i, ni1);     
  fsmMgrPM->name.ps = Pdtutil_StrArrayDup (namePSPM, nl1+nl2);     
  fsmMgrPM->name.o = Pdtutil_StrArrayDup (nameOPM, no1); 

  Fsm_MgrSetIndexI (fsmMgrPM, indexIFsm1);
  Fsm_MgrSetIndexPS (fsmMgrPM, indexPSPM);

  /*
   *  Transfer BDDs
   */ 

  /* Debug Printing */
  /*
  fprintf (stdout, "Remapping INDEX from-> = ");
  Ddi_PrintVararray (psv1);
  fprintf (stdout, "Remapping INDEX ->to = ");
  Ddi_PrintVararray (psv2New);
  */

  /* Delta Functions */
  tmpBddArray1 = Fsm_MgrReadDeltaBDD (fsmMgr1);
  tmpBddArray2 = Fsm_MgrReadDeltaBDD (fsmMgr2);
  tmpBddArrayPM = Ddi_BddarrayAlloc(ddMgr1, 0);
  for (i=0; i<nl1; i++) {
    bdd = Ddi_BddarrayRead (tmpBddArray2, i);
    oldBDD = Ddi_BddarrayRead (tmpBddArray1, i);
    newBDD = Ddi_BddCopyRemapVars (ddMgr2, oldBDD, psv1, psv2New);
    Ddi_BddarrayWrite (tmpBddArrayPM, 2*i, bdd);
    Ddi_BddarrayWrite (tmpBddArrayPM, 2*i+1, newBDD);
    Ddi_Free (newBDD);
  }
  Fsm_MgrSetDeltaBDD (fsmMgrPM, tmpBddArrayPM);
  Ddi_Free (tmpBddArrayPM);

  /* Lambda Functions */
  tmpBddArray1 = Fsm_MgrReadLambdaBDD (fsmMgr1);
  tmpBddArray2 = Fsm_MgrReadLambdaBDD (fsmMgr2);
  tmpBddArrayPM = Ddi_BddarrayAlloc(ddMgr1, 0);
  for (i=0; i<no1; i++) {
    bdd = Ddi_BddarrayRead (tmpBddArray2, i);
    oldBDD = Ddi_BddarrayRead (tmpBddArray1, i);
    newBDD = Ddi_BddCopyRemapVars (ddMgr2, oldBDD, psv1, psv2New);
    Ddi_BddarrayWrite (tmpBddArrayPM, 2*i, bdd);
    Ddi_BddarrayWrite (tmpBddArrayPM, 2*i+1, newBDD);
    Ddi_Free (newBDD);
  }
  Fsm_MgrSetLambdaBDD (fsmMgrPM, tmpBddArrayPM);
  Ddi_Free (tmpBddArrayPM);

  /* Debug ... Congruence Check */
  Fsm_MgrStore (fsmMgrPM, "pm", NULL, 1, DDDMP_MODE_TEXT, 1);

  /*
   *  Free Memory
   */
  
  Pdtutil_Free (indexIFsm1);
  Pdtutil_Free (indexIFsm2);
  Pdtutil_Free (indexPSFsm1);
  Pdtutil_Free (indexPSFsm2);
  Pdtutil_Free (indexPSPM);
  Pdtutil_Free (indexMgr1);
  Pdtutil_Free (indexMgr2);
  Pdtutil_StrArrayFree(namePSPM, nl1);  
  Pdtutil_StrArrayFree(nameOPM, no1);

  /*
   *  return 0 = All OK
   */

  *fsmMgrPMP = fsmMgrPM;
  return (0);
}

