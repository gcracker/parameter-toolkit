/**CFile***********************************************************************

  FileName    [fsmStore.c]

  PackageName [fsm]

  Synopsis    [Functions to write description of FSMs]

  Description [External procedures included in this module:
    <ul>
    <li> Fsm_MgrStore ()
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

  Synopsis     [Stores (on file o stdout) of FSM structure.]

  Description  []

  SideEffects  []

  SeeAlso      [Ddi_BddarrayStore,Ddi_BddarrayRead,Ddi_BddarrayNum]

******************************************************************************/

int
Fsm_MgrStore (
  Fsm_Mgr_t *fsmMgr                    /* FSM Structure */,
  char *fileName                       /* Output File Name */,
  FILE *fp                             /* File Pointer */,
  int bddFlag                          /* Flag to Store or Not BDD on Files */,
  int bddFormat                        /* 0 = default, 1 = text, 2 = binary */,
  Pdtutil_VariableOrderFormat_e ordFileFormat
  )
{
  int i, flagFile, v;
  char *word, *fsmName, *tmpName;
  Ddi_Mgr_t *dd;
  long fsmTime;

  fsmTime = util_cpu_time();

  /*------------------------- Check For Correctness -------------------------*/

  if (Fsm_MgrCheck (fsmMgr) == 1) {
    return (1);
  }
 
  /*---------------------- Create Name and Open File ------------------------*/
  
  fsmName = Pdtutil_FileName (fileName, "", "fsm", 0);
  fp = Pdtutil_FileOpen (fp, fsmName, "w", &flagFile);

  /*--------------------------- Take dd Manager -----------------------------*/
  
  dd = Fsm_MgrReadDdManager (fsmMgr);

  /*---------------------------- Set BDD Format -----------------------------*/
  
  if (bddFormat==DDDMP_MODE_DEFAULT) { 
    bddFormat = Fsm_MgrReadBddFormat (fsmMgr);
  }

  /*----------------------------- Store Header ------------------------------*/
  
  if (Fsm_MgrReadFileName (fsmMgr) != NULL) {
    fprintf (fp, ".%s %s\n", FsmToken2String (KeyFSM_FSM),
      Fsm_MgrReadFileName (fsmMgr));
  } else {
    if (Fsm_MgrReadFsmName (fsmMgr) != NULL) {
      fprintf (fp, ".%s %s\n", FsmToken2String (KeyFSM_FSM),
	       Fsm_MgrReadFsmName (fsmMgr));
    } else {
      fprintf (fp, ".%s %s\n", FsmToken2String (KeyFSM_FSM), "fsm");
    }
  }
       
  fprintf (fp, "\n");
  fflush (fp);
  
  /*------------------------------ Store Size -------------------------------*/
  
  fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_SIZE));
  fprintf (fp, "   .%s %d\n", FsmToken2String (KeyFSM_I),
    Fsm_MgrReadNI (fsmMgr));
  fprintf (fp, "   .%s %d\n", FsmToken2String (KeyFSM_O),
    Fsm_MgrReadNO (fsmMgr));
  fprintf (fp, "   .%s %d\n", FsmToken2String (KeyFSM_L),
    Fsm_MgrReadNL (fsmMgr));
  fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
    FsmToken2String (KeyFSM_SIZE));
  fprintf (fp, "\n");
  fflush (fp);

  /*------------------------------Store Order ------------------------------*/

  fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_ORD));
  fflush (fp);
  fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_ORD));
  tmpName = Pdtutil_FileName (fileName, "FSM", "ord", 1);
  fprintf (fp, "%s\n", tmpName);

  /* Force Order with Auxiliary Index and Comments */
  Ddi_MgrOrdWrite (dd, tmpName, NULL, ordFileFormat);

  fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
    FsmToken2String (KeyFSM_ORD));
  fprintf (fp, "\n");
  fflush (fp);
  
  /*----------------------------- Store Name -------------------------------*/
  
  fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_NAME));
  
  /*
   *  .i
   */

  if ((Fsm_MgrReadNameI (fsmMgr))!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_I));
    for (i=0; i<Fsm_MgrReadNI (fsmMgr); i++) {
      word = Fsm_MgrReadNameI (fsmMgr)[i];
      fprintf (fp, "%s ", word);
    }
    fprintf (fp, "\n");
  }
  
  /*
   *  .o
   */

  if ((Fsm_MgrReadNameO (fsmMgr))!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_O));
    for (i=0; i<Fsm_MgrReadNO (fsmMgr); i++) {
      word = Fsm_MgrReadNameO (fsmMgr)[i];
      fprintf (fp, "%s ", word);
    }
    fprintf (fp, "\n");
  }
  
  /*
   *  .ps
   */

  if (Fsm_MgrReadNL (fsmMgr) > 0 && Fsm_MgrReadNamePS (fsmMgr)!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_PS));
    for (i=0; i<Fsm_MgrReadNL (fsmMgr); i++) {
      word = Fsm_MgrReadNamePS (fsmMgr)[i];
      fprintf (fp, "%s ", word);
    }
    fprintf (fp, "\n");
  }
  
  /*
   *  .ns
   */

  if (Fsm_MgrReadNL (fsmMgr) > 0 && Fsm_MgrReadNameNS (fsmMgr)!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_NS));
    for (i=0; i<Fsm_MgrReadNL (fsmMgr); i++) {
      word = Fsm_MgrReadNameNS (fsmMgr)[i];
      fprintf (fp, "%s ", word);
    }
    fprintf (fp, "\n");
  }
  
  fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
    FsmToken2String (KeyFSM_NAME));
  fprintf (fp, "\n");
  fflush (fp);
  
  /*------------------------------ Store Index ------------------------------*/
  
  fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_INDEX));
  
  /*
   *  .i
   */

  if ((Fsm_MgrReadIndexI (fsmMgr))!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_I));
    for (i=0; i<Fsm_MgrReadNI (fsmMgr); i++) {
      v = Fsm_MgrReadIndexI (fsmMgr)[i];
      fprintf (fp, "%d ", v);
    }
    fprintf (fp, "\n");
  }
  
  /*
   *  .o
   */

  if ((Fsm_MgrReadIndexO (fsmMgr))!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_O));
    for (i=0; i<Fsm_MgrReadNO (fsmMgr); i++) {
      v = Fsm_MgrReadIndexO (fsmMgr)[i];
      fprintf (fp, "%d ", v);
    }
    fprintf (fp, "\n");
  }
  
  /*
   *  .ps
   */

  if (Fsm_MgrReadNL (fsmMgr) > 0 && Fsm_MgrReadIndexPS (fsmMgr)!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_PS));
    for (i=0; i<Fsm_MgrReadNL (fsmMgr); i++) {
      v = Fsm_MgrReadIndexPS (fsmMgr)[i];
      fprintf (fp, "%d ", v);
    }
    fprintf (fp, "\n");
  }
  
  /*
   *  .ns
   */

  if (Fsm_MgrReadNL (fsmMgr) > 0 && Fsm_MgrReadIndexNS (fsmMgr)!=NULL) {
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_NS));
    for (i=0; i<Fsm_MgrReadNL (fsmMgr); i++) {
      v = Fsm_MgrReadIndexNS (fsmMgr)[i];
      fprintf (fp, "%d ", v);
    }
    fprintf (fp, "\n");
  }
  fflush (fp);
  
  fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
    FsmToken2String (KeyFSM_INDEX));
  fprintf (fp, "\n");
  fflush (fp);

  /*---------------------------- Store Delta --------------------------------*/

  /*
   *  If (BDDs on File YES and BDDs EXISTS) ... go on 
   */

  if (Fsm_MgrReadNL (fsmMgr) > 0 &&
      bddFlag != 0 && Fsm_MgrReadDeltaBDD (fsmMgr) != NULL) {
    fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_DELTA));
  
    /* BDD on a Separate File */
    if (bddFlag == 1) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_BDD));
      tmpName = Pdtutil_FileName (fileName, "delta", "bdd", 1);
      fprintf (fp, "%s\n", tmpName);
  
      Ddi_BddarrayStore (Fsm_MgrReadDeltaBDD (fsmMgr),
        tmpName, Fsm_MgrReadVarnames (fsmMgr), NULL,
        Fsm_MgrReadVarauxids (fsmMgr), bddFormat, tmpName, NULL);

      Fsm_MgrSetDeltaName (fsmMgr, tmpName);
    }
  
    /* BDD on the Same File */
    if (bddFlag == 2) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_TEXT)); 

      Ddi_BddarrayStore (Fsm_MgrReadDeltaBDD (fsmMgr),
        NULL,Fsm_MgrReadVarnames (fsmMgr), NULL, Fsm_MgrReadVarauxids (fsmMgr),
        bddFormat, NULL, fp);

      fprintf (fp, "   .%s%s\n", FsmToken2String (KeyFSM_END),
        FsmToken2String (KeyFSM_FILE_TEXT));

      Fsm_MgrSetDeltaName (fsmMgr, fsmName);
    }
  
    fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
      FsmToken2String (KeyFSM_DELTA));

    fprintf (fp, "\n");
    fflush (fp);
  }
  
  /*--------------------------- Store Lambda --------------------------------*/

  /*
   *  If (BDDs on File YES and BDDs EXIST) ... go on 
   */

  if (bddFlag != 0 && Fsm_MgrReadLambdaBDD (fsmMgr) != NULL) {
    fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_LAMBDA));
  
    /* BDD on a Separate File */
    if (bddFlag == 1) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_BDD));
      tmpName = Pdtutil_FileName (fileName, "lambda", "bdd", 1);
      fprintf (fp, "%s\n", tmpName);
  
      Ddi_BddarrayStore (Fsm_MgrReadLambdaBDD (fsmMgr), tmpName,
        Fsm_MgrReadVarnames (fsmMgr), NULL, Fsm_MgrReadVarauxids (fsmMgr),
        bddFormat, tmpName,NULL);

      Fsm_MgrSetLambdaName (fsmMgr, tmpName);
    }
  
    /* BDD on the Same File */
    if (bddFlag == 2) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_TEXT)); 

      Ddi_BddarrayStore (Fsm_MgrReadLambdaBDD (fsmMgr),
        NULL,Fsm_MgrReadVarnames (fsmMgr), NULL, Fsm_MgrReadVarauxids (fsmMgr),
        bddFormat, NULL, fp);

      fprintf (fp, "   .%s%s\n", FsmToken2String (KeyFSM_END),
       FsmToken2String (KeyFSM_FILE_TEXT));

      Fsm_MgrSetLambdaName (fsmMgr, fsmName);
    }

    fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
      FsmToken2String (KeyFSM_LAMBDA));

    fprintf (fp, "\n");
    fflush (fp);
  }
  
  /*-------------------------- Store InitState ------------------------------*/
  
  /*
   *  If there is the string ... store it
   */

  if (Fsm_MgrReadInitString (fsmMgr) != NULL ) {
    fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_INITSTATE));
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_STRING));
    fprintf (fp, "%s\n", Fsm_MgrReadInitString (fsmMgr));
    fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
      FsmToken2String (KeyFSM_INITSTATE));
    fprintf (fp, "\n");
    fflush (fp);
  }
  
  else

  /*
   *  If (BDDs on File YES and BDDs EXIST) ... go on 
   */

  if (bddFlag != 0 && Fsm_MgrReadInitBDD (fsmMgr) != NULL) {
    fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_INITSTATE));
    fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_BDD));

    /* BDD on a Separate File */
    if (bddFlag == 1) {
      tmpName = Pdtutil_FileName (fileName, "s0", "bdd", 1);
      fprintf (fp, "%s\n", tmpName);
  
      Ddi_BddStore (Fsm_MgrReadInitBDD (fsmMgr), NULL,
        bddFormat, tmpName, NULL);

      Fsm_MgrSetInitName (fsmMgr, tmpName);
    }
  
    /* BDD on the Same File */
    if (bddFlag == 2) {
      fprintf (stderr, "fsmLoad.c (fsmLoad): Not Yet Implemented!\n");
      Fsm_MgrSetInitName (fsmMgr, fsmName);
    }

    fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
      FsmToken2String (KeyFSM_INITSTATE));
    fprintf (fp, "\n");
    fflush (fp);
  }
  
  /*------------------------ Store Transition Relation ----------------------*/

  /*
   *  If (BDD on File YES and BDD EXISTS) ... go on 
   */

  if (bddFlag != 0 && Fsm_MgrReadTrBDD (fsmMgr) != NULL) {
    fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_TRANS_REL));
  
    /* BDD on a Separate File */
    if (bddFlag == 1) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_BDD));
      tmpName = Pdtutil_FileName (fileName, "TR", "bdd", 1);
      fprintf (fp, "%s\n", tmpName);
  
      Ddi_BddStore (Fsm_MgrReadTrBDD (fsmMgr), tmpName, bddFormat,
        tmpName, NULL);

      Fsm_MgrSetTrName (fsmMgr, tmpName);
    }
  
    /* BDD on the Same File */
    if (bddFlag == 2) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_TEXT));

      Ddi_BddStore (Fsm_MgrReadTrBDD (fsmMgr),NULL, bddFormat, NULL, fp);

      fprintf (fp, "   .%s%s\n", FsmToken2String (KeyFSM_END),
        FsmToken2String (KeyFSM_FILE_TEXT));

      Fsm_MgrSetTrName (fsmMgr, fsmName);
      }
  
    fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
      FsmToken2String (KeyFSM_TRANS_REL));

    fprintf (fp, "\n");
    fflush (fp);
  }
  
  /*------------------------------ Store From Set ---------------------------*/

  /*
   *  If (BDD on File YES and BDD EXISTS) ... go on 
   */

  if (bddFlag != 0 && Fsm_MgrReadFromBDD (fsmMgr) != NULL) {
    fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_FROM));
  
    /* BDD on a Separate File */
    if (bddFlag == 1) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_BDD));
      tmpName = Pdtutil_FileName (fileName, "From", "bdd", 1);
      fprintf (fp, "%s\n", tmpName);
  
      Ddi_BddStore (Fsm_MgrReadFromBDD (fsmMgr), tmpName, bddFormat,
        tmpName, NULL);

      Fsm_MgrSetFromName (fsmMgr, tmpName);
    }
  
    /* BDD on the Same File */
    if (bddFlag == 2) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_TEXT));

      Ddi_BddStore (Fsm_MgrReadFromBDD (fsmMgr), NULL,
        bddFormat, NULL, fp);

      fprintf (fp, "   .%s%s\n", FsmToken2String (KeyFSM_END),
        FsmToken2String (KeyFSM_FILE_TEXT));

      Fsm_MgrSetFromName (fsmMgr, fsmName);
    }

    fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
      FsmToken2String (KeyFSM_FROM));

    fprintf (fp, "\n");
    fflush (fp);
  }
  
  /*--------------------------- Store Reached Set ---------------------------*/

  /*
   *  If (BDD on File YES and BDD EXISTS) ... go on 
   */

  if (bddFlag != 0 && Fsm_MgrReadReachedBDD (fsmMgr) != NULL) {
    fprintf (fp, ".%s\n", FsmToken2String (KeyFSM_REACHED));
  
    /* BDD on a Separate File */
    if (bddFlag == 1) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_BDD));
      tmpName = Pdtutil_FileName (fileName, "Reached", "bdd", 1);
      fprintf (fp, "%s\n", tmpName);
  
      Ddi_BddStore (Fsm_MgrReadReachedBDD (fsmMgr), tmpName,
        bddFormat, tmpName, NULL);

      Fsm_MgrSetReachedName (fsmMgr, tmpName);
    }
  
    /* BDD on the Same File */
    if (bddFlag == 2) {
      fprintf (fp, "   .%s ", FsmToken2String (KeyFSM_FILE_TEXT));

      Ddi_BddStore (Fsm_MgrReadReachedBDD (fsmMgr), NULL, bddFormat, NULL, fp);

      fprintf (fp, "   .%s%s\n", FsmToken2String (KeyFSM_END),
        FsmToken2String (KeyFSM_FILE_TEXT));

      Fsm_MgrSetReachedName (fsmMgr, fsmName);
      }
  
    fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
      FsmToken2String (KeyFSM_REACHED));

    fprintf (fp, "\n");
    fflush (fp);
  }
  
  /*----------------------------- Close FSM ---------------------------------*/

  fprintf (fp, ".%s%s\n", FsmToken2String (KeyFSM_END),
    FsmToken2String (KeyFSM_FSM));
  fflush (fp);
  Pdtutil_FileClose (fp, &flagFile);
  
  /*
   *  return 0 = All OK
   */

  fsmTime = util_cpu_time() - fsmTime;
  fsmMgr->fsmTime += fsmTime;

  return (0);
}
