/**CFile***********************************************************************

  FileName    [testTrav.c]

  PackageName [test]

  Synopsis    [Functions to test the FSM package]

  Description [Functions to load a FSM from a blif file and to store it
    in the internal FSM format.
    This program is developed as a regression test, and as usage
    example for the FSM package.]

  SeeAlso   []  

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
  
  Revision  []

******************************************************************************/

#include "testInt.h"
#include "fsm.h"
#include "tr.h"
#include "trav.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define CIRCUIT_NAME "s27"
#define BLIF_NAME    "s27.blif"
#define FSM_NAME_A     "s27a.fsm"
#define FSM_NAME_B     "s27b.fsm"
#define ORD_NAME     "s27aFSM.ord"

#define MYTRAV_SIFT_THRESH 10000
#define MYTRAV_CLUST_THRESH 1000
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

/**Function*******************************************************************
  Synopsis    [Main test function for FSM]
  Description [Main test function for FSM. Calls specific FSM test functions.]
  SideEffects []
  SeeAlso     []
******************************************************************************/

int
TestTrav (
  )
{
  Ddi_Mgr_t *ddMgr;
  Fsm_Mgr_t *fsmMgr;
  Tr_Mgr_t *trMgr;
  Trav_Mgr_t *travMgr;
  int varNum, slotNum, cacheSize, memorySizeMax;
  int errorFlag;
  Ddi_Vararray_t *pi, *ps, *ns;
  Ddi_Vararray_t *psv, *nsv;
  Tr_Tr_t *tr;
  Ddi_Bdd_t *init;
  Ddi_Bddarray_t *delta;

  /*--------------------------- Init DD Manager ----------------------------*/

  varNum = 0;
  slotNum = DDI_UNIQUE_SLOTS;
  cacheSize = DDI_CACHE_SLOTS;
  memorySizeMax = 0;

  fprintf (stdout, "DDI Manager Init ... ");
  ddMgr = Ddi_MgrInit("DDI_test_manager", NULL, varNum, slotNum, cacheSize,
    memorySizeMax);

  Ddi_MgrAutodynEnable (ddMgr, Ddi_ReorderingMethodString2Enum("sift"));
  Ddi_MgrSetDynordThresh (ddMgr, MYTRAV_SIFT_THRESH);
  Ddi_MgrSetVerbosity (ddMgr, Pdtutil_VerbLevelAppMed_c);

  /*--------------------------- Init FSM Manager ---------------------------*/

  fsmMgr = Fsm_MgrInit ("fsm");

  /*---------------------- Read a FSM from BLIF File -----------------------*/

  fprintf (stdout, "FSM Loading ... from %s\n", BLIF_NAME); 

  errorFlag = Fsm_MgrLoadFromBlif (&fsmMgr, ddMgr, BLIF_NAME,
    NULL, 1, Pdtutil_VariableOrderDefault_c);

  Pdtutil_Assert (errorFlag==0, "FSM not loaded!");
  if (errorFlag == 0) {
    fprintf (stdout, "Done\n");
  }

  /*-------------------  Read Usefull Stuff From FSM Manager ---------------*/

  pi = Fsm_MgrReadVarI (fsmMgr);
  ps = Fsm_MgrReadVarPS (fsmMgr);
  ns = Fsm_MgrReadVarNS (fsmMgr);
  psv = Fsm_MgrReadVarPS (fsmMgr);
  nsv = Fsm_MgrReadVarNS (fsmMgr);
  delta = Fsm_MgrReadDeltaBDD (fsmMgr);
  init = Fsm_MgrReadInitBDD (fsmMgr);

  /*--------------------------- Generate TR manager ------------------------*/

  trMgr = Tr_MgrInit ("TR-manager", ddMgr);
  /* Iwls95 sorting method */
  Tr_MgrSetSortMethod (trMgr, Tr_SortWeight_c);
  /* enable smoothing PIs while clustering */
  Tr_MgrSetClustSmoothPi(trMgr,1);
  Tr_MgrSetClustThreshold (trMgr, MYTRAV_CLUST_THRESH);
  Tr_MgrSetVerbosity(trMgr,Pdtutil_VerbLevelAppMin_c);

  Tr_MgrSetI (trMgr, pi);
  Tr_MgrSetPS (trMgr, ps);
  Tr_MgrSetNS (trMgr, ns);

  tr = Tr_TrMakePartConjFromFuns (trMgr, delta, ns);

  Tr_TrSortIwls95 (tr); 
  Tr_TrSetClustered (tr);

  Ddi_MgrSetVerbosity (ddMgr, Pdtutil_VerbLevelAppMed_c);

  /*------------------------ Create Traversal Manager -----------------------*/

  travMgr = Trav_MgrInit (NULL, ddMgr);

  if (travMgr == NULL) {
    fprintf (stderr, "Error: Traversal Manager Init failed!\n");
    exit (1);
  } else {
    fprintf (stdout, "Done\n");
  }

  Trav_MgrSetI (travMgr, pi);
  Trav_MgrSetPS (travMgr, psv);
  Trav_MgrSetNS (travMgr, nsv);
  Trav_MgrSetTr (travMgr, tr);
  Trav_MgrSetReached (travMgr, init);
  Trav_MgrSetFrom (travMgr, init);
  Trav_MgrSetVerbosity (travMgr, Pdtutil_VerbLevelAppMed_c);

  /*--------------------------- Traverse the FSM ----------------------------*/

  Trav_Main (travMgr);

  /*-------------------------- Quit a TRAV Manager --------------------------*/

  Trav_MgrQuit (travMgr);

  /*-------------------------------- Quit tr ------------------------------*/

  Tr_TrFree(tr);

  /*-------------------------- Quit a TR Manager --------------------------*/

  Tr_MgrQuit(trMgr);

  /*-------------------------- Quit a FSM Manager --------------------------*/

  Fsm_MgrQuit(fsmMgr);

  if (Ddi_MgrCheckExtRef(ddMgr, 0)==0) {
    Ddi_MgrPrintExtRef(ddMgr,0);
  }

  /*--------------------------- Quit a DD Manager --------------------------*/

  Ddi_MgrQuit (ddMgr);

  return(1);
}

