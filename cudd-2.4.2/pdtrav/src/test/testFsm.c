/**CFile***********************************************************************

  FileName    [testFsm.c]

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

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define SEQ_NAME "s27"
#define COM_NAME "c17"

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

int TestFsmInt (char *circuitName);

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
TestFsm (
  )
{
  /*-------------------- Check on a Sequential Circuit ---------------------*/

  fprintf (stdout,
    "***************** FSM TEST on SEQUENTIAL CIRCUIT ****************\n");
  TestFsmInt (SEQ_NAME);

  /*------------------- Check on a Combinational Circuit --------------------*/

  fprintf (stdout,
    "***************** FSM TEST on COMBINATIONAL CIRCUIT ************\n");
  TestFsmInt (COM_NAME);

  return(1);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************
  Synopsis    [Main test function for FSM]
  Description [Main test function for FSM. Calls specific FSM test functions.]
  SideEffects []
  SeeAlso     []
******************************************************************************/

int
TestFsmInt (
  char *circuitName
  )
{
  Ddi_Mgr_t *ddMgr;
  Fsm_Mgr_t *fsmMgr;
  int varNum, slotNum, cacheSize, memorySizeMax;
  int errorFlag;
  char tmpName[PDTUTIL_MAX_STR_LEN], tmpName2[PDTUTIL_MAX_STR_LEN];

  varNum = 0;
  slotNum = DDI_UNIQUE_SLOTS;
  cacheSize = DDI_CACHE_SLOTS;
  memorySizeMax = 0;

  /*--------------------------- Init DD Manager ----------------------------*/

  fprintf (stdout, "DDI Manager Init ... ");
  fflush (stdout);

  ddMgr = Ddi_MgrInit ("DDI_test_manager", NULL, varNum, slotNum, cacheSize,
    memorySizeMax);

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*--------------------------- Init FSM Manager ---------------------------*/

  fprintf (stdout, "FSM Manager Init ... ");
  fflush (stdout);

  fsmMgr = Fsm_MgrInit ("fsm");

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*---------------------- Read a FSM from BLIF File -----------------------*/

  sprintf (tmpName, "%s.blif", circuitName);
  fprintf (stdout, "FSM Loading ... from %s ... ", tmpName); 
  fflush (stdout);

  errorFlag = Fsm_MgrLoadFromBlif (&fsmMgr, ddMgr, tmpName,
    NULL, 1, Pdtutil_VariableOrderDefault_c);

  /*
  one = Ddi_MgrReadOne (ddMgr); 
  Fsm_MgrSetReachedBDD (fsmMgr, one); 
  */

  Pdtutil_Warning (errorFlag, "FSM not loaded!");
  if (errorFlag == 0) {
    fprintf (stdout, "Done\n");
  }

  /*----------------------------- Store a FSM ------------------------------*/

  sprintf (tmpName, "%sa.fsm", circuitName);
  fprintf (stdout, "FSM Storing ... to %s ... ", tmpName);
  fflush (stdout);

  /* Ascii=((int) 'A'), Binary=((int)'B'), Default=((int)'D') */
  Fsm_MgrStore (fsmMgr, tmpName, NULL, 1, ((int) 'A'),
    Pdtutil_VariableOrderIndex_c);
  /*Pdtutil_VariableOrderDefault_c);*/

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*-------------------------- Quit a FSM Manager --------------------------*/

  fprintf (stdout, "FSM Manager Quit ... ");
  fflush (stdout);

  Fsm_MgrQuit (fsmMgr);

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*--------------------------- Quit a DD Manager --------------------------*/

  fprintf (stdout, "DDI Manager Quit ... ");
  fflush (stdout);

  Ddi_MgrQuit (ddMgr);

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*--------------------------- Init DD Manager ----------------------------*/

  fprintf (stdout, "DDI Manager Init ... ");
  fflush (stdout);

  ddMgr = Ddi_MgrInit ("DDI_test_manager", NULL, varNum, slotNum, cacheSize,
    memorySizeMax);

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*--------------------------- Init FSM Manager ---------------------------*/

  fprintf (stdout, "FSM Manager Init ... ");
  fflush (stdout);

  fsmMgr = Fsm_MgrInit ("fsm");

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*---------------------- Read a FSM from FSM File ------------------------*/

  sprintf (tmpName, "%sa.fsm", circuitName);
  sprintf (tmpName2, "%saFSM.ord", circuitName);
  fprintf (stdout, "FSM Loading ... from %s ", tmpName); 
  fflush (stdout);

  errorFlag = Fsm_MgrLoad (&fsmMgr, ddMgr, tmpName, tmpName2,
    /* 0=Do non load BDD (default), 1=Load BDD */       
    0, Pdtutil_VariableOrderIndex_c);
  /*Pdtutil_VariableOrderDefault_c);*/

  Pdtutil_Warning (errorFlag, "FSM not loaded!");
  if (errorFlag == 0) {
    fprintf (stdout, "Done\n");
  }
  fflush (stdout);

  /*----------------------------- Store a FSM ------------------------------*/

  sprintf (tmpName, "%sb.fsm", circuitName);
  fprintf (stdout, "FSM Storing ... to %s ", tmpName);
  fflush (stdout);

  Fsm_MgrStore (fsmMgr, tmpName, NULL, 1, ((int) 'A'),
    Pdtutil_VariableOrderDefault_c);

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*-------------------------- Quit a FSM Manager --------------------------*/

  fprintf (stdout, "FSM Manager Quit ... ");
  fflush (stdout);

  Fsm_MgrQuit (fsmMgr);

  fprintf (stdout, "Done\n");
  fflush (stdout);

  /*--------------------------- Quit a DD Manager --------------------------*/

  fprintf (stdout, "DDI Manager Quit ... ");
  fflush (stdout);

  Ddi_MgrQuit (ddMgr);

  fprintf (stdout, "Done\n");
  fflush (stdout);

  return(1);
}
