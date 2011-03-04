/**CFile***********************************************************************

  FileName    [fsmMgr.c]

  PackageName [fsm]

  Synopsis    [Utility functions to create, free and duplicate a FSM]

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

  Synopsis     [Initializes the FSM structure.]

  Description  [Notice: Direct access to fileds is used, because macros
    check for NULL fields to discover memory leaks.]

  SideEffects  [none]

******************************************************************************/

Fsm_Mgr_t *
Fsm_MgrInit (
  char *fsmName   /* Name of the FSM structure */
  )
{
  Fsm_Mgr_t *fsmMgr;

  /*------------------------- FSM Structure Creation -----------------------*/

  fsmMgr = Pdtutil_Alloc (Fsm_Mgr_t, 1);
  if (fsmMgr == NULL) {
    fprintf (stderr, "fsmUtil.c (Fsm_MgrInit) Error: Out of memory.\n");
    return (NULL);
  }

  /*------------------- Set the Name of the FSM Structure ------------------*/

  if (fsmName != NULL) {
     fsmMgr->fsmName = Pdtutil_StrDup (fsmName);
  } else {
     fsmMgr->fsmName = Pdtutil_StrDup ("fsm");
  }

  /*------------------------------- Set Fields -----------------------------*/

  fsmMgr->fileName = NULL;
  fsmMgr->dd = NULL;

  fsmMgr->size.i = (-1);
  fsmMgr->size.o = (-1);
  fsmMgr->size.l = (-1);
  fsmMgr->size.auxVar = (-1);

  fsmMgr->var.i = NULL;
  fsmMgr->var.o = NULL;
  fsmMgr->var.ps = NULL;
  fsmMgr->var.ns = NULL;
  fsmMgr->var.auxVar = NULL;

  fsmMgr->varnames = NULL;
  fsmMgr->varauxids = NULL;
  fsmMgr->invauxids = NULL;

  fsmMgr->index.i = NULL;
  fsmMgr->index.o = NULL;
  fsmMgr->index.ps = NULL;
  fsmMgr->index.ns = NULL;
  fsmMgr->index.auxVar = NULL;

  fsmMgr->name.i = NULL;
  fsmMgr->name.o = NULL;
  fsmMgr->name.ps = NULL;
  fsmMgr->name.ns = NULL;
  fsmMgr->name.auxVar = NULL;

  fsmMgr->ord.nodeid = NULL;

  Fsm_MgrSetBddFormat (fsmMgr, DDDMP_MODE_TEXT);

  fsmMgr->delta.name = NULL;
  fsmMgr->delta.bdd = NULL;
  fsmMgr->lambda.name = NULL;
  fsmMgr->lambda.bdd = NULL;
  fsmMgr->auxVar.name = NULL;
  fsmMgr->auxVar.bdd = NULL;

  fsmMgr->init.name = NULL;
  fsmMgr->init.string = NULL;
  fsmMgr->init.bdd = NULL;
  fsmMgr->ord.nodename = NULL;
  fsmMgr->ord.name = NULL;
  fsmMgr->tr.name = NULL;
  fsmMgr->tr.string = NULL;
  fsmMgr->tr.bdd = NULL;
  fsmMgr->from.name = NULL;
  fsmMgr->from.string = NULL;
  fsmMgr->from.bdd = NULL;
  fsmMgr->reached.name = NULL;
  fsmMgr->reached.string = NULL;
  fsmMgr->reached.bdd = NULL;

  /*
   *  Settings
   */

  fsmMgr->fsmTime = 0;
  Fsm_MgrSetVerbosity (fsmMgr, Pdtutil_VerbLevelNone_c);

  return (fsmMgr);
}

/**Function********************************************************************

  Synopsis     [Check the Congruence of a FSM structure.]

  Description  [Check the Congruence of a FSM structure.
    Return 1 if the FSM is a NULL pointer or it is not correct.
    Return 0 otherwise.]

  SideEffects  [none]

******************************************************************************/

int
Fsm_MgrCheck (
  Fsm_Mgr_t *fsmMgr   /* FSM structure */
  )
{
  int errorFlag;

  errorFlag = 0;

  if (fsmMgr==NULL) {
    errorFlag = 1;
  }

  if (fsmMgr->dd == NULL) {
    errorFlag = 1;
  }

  /*
  fsmMgr->size.i = (-1);
  fsmMgr->size.o = (-1);
  fsmMgr->size.l = (-1);
  fsmMgr->size.auxVar = (-1);

  fsmMgr->var.i = NULL;
  fsmMgr->var.o = NULL;
  fsmMgr->var.ps = NULL;
  fsmMgr->var.ns = NULL;
  fsmMgr->var.auxVar = NULL;

  fsmMgr->varnames = NULL;
  fsmMgr->varauxids = NULL;
  fsmMgr->invauxids = NULL;

  fsmMgr->index.i = NULL;
  fsmMgr->index.o = NULL;
  fsmMgr->index.ps = NULL;
  fsmMgr->index.ns = NULL;
  fsmMgr->index.auxVar = NULL;

  fsmMgr->name.i = NULL;
  fsmMgr->name.o = NULL;
  fsmMgr->name.ps = NULL;
  fsmMgr->name.ns = NULL;
  fsmMgr->name.auxVar = NULL;

  fsmMgr->ord.nodeid = NULL;

  Fsm_MgrSetBddFormat (fsmMgr, DDDMP_MODE_TEXT);

  fsmMgr->delta.name = NULL;
  fsmMgr->delta.bdd = NULL;
  fsmMgr->lambda.name = NULL;
  fsmMgr->lambda.bdd = NULL;
  fsmMgr->auxVar.name = NULL;
  fsmMgr->auxVar.bdd = NULL;

  fsmMgr->init.name = NULL;
  fsmMgr->init.string = NULL;
  fsmMgr->init.bdd = NULL;
  fsmMgr->ord.nodename = NULL;
  fsmMgr->ord.name = NULL;
  fsmMgr->tr.name = NULL;
  fsmMgr->tr.string = NULL;
  fsmMgr->tr.bdd = NULL;
  fsmMgr->from.name = NULL;
  fsmMgr->from.string = NULL;
  fsmMgr->from.bdd = NULL;
  fsmMgr->reached.name = NULL;
  fsmMgr->reached.string = NULL;
  fsmMgr->reached.bdd = NULL;
  */

  if (errorFlag == 1) {
    Pdtutil_Warning (1, "FSM Structure NOT Congruent.");
    return (1);
  } else {
    return (0);
  }
}

/**Function********************************************************************

  Synopsis     [Frees unused FSM structure members ]

  Description  []

  SideEffects  [none]

  SeeAlso      [Ddi_MgrQuit, Tr_MgrQuit, Trav_MgrQuit]

******************************************************************************/

void 
Fsm_MgrQuit (
  Fsm_Mgr_t *fsmMgr   /* FSM Manager */
)
{
  /*------------------------ Deletes the FSM Manager -------------------*/

  /* Free fsmName */
  Pdtutil_Free (fsmMgr->fsmName);

  /* Free fileName */
  Pdtutil_Free (fsmMgr->fileName);

  /* var */
  Ddi_Free (fsmMgr->var.i);
  Ddi_Free (fsmMgr->var.ps);
  Ddi_Free (fsmMgr->var.ns);
  Ddi_Free (fsmMgr->var.auxVar);

  /* Free varnames */
  Pdtutil_StrArrayFree (Fsm_MgrReadVarnames (fsmMgr),
    Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr));
 
  /* Free varauxids */
  Pdtutil_Free (fsmMgr->varauxids);

  /* Free invauxids */
  Pdtutil_Free (fsmMgr->invauxids);

  /* Free index */
  Pdtutil_Free (fsmMgr->index.i);
  Pdtutil_Free (fsmMgr->index.o);
  Pdtutil_Free (fsmMgr->index.ps);
  Pdtutil_Free (fsmMgr->index.ns);
  Pdtutil_Free (fsmMgr->index.auxVar);

  /* Free ord */
  Pdtutil_Free (fsmMgr->ord.name);
 
  Pdtutil_StrArrayFree (fsmMgr->ord.nodename,
    Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr));
  Pdtutil_Free (fsmMgr->ord.nodename);

  Pdtutil_Free (fsmMgr->ord.nodeid);

  /* name */
  Pdtutil_Free (fsmMgr->name.i);
  Pdtutil_Free (fsmMgr->name.o);
  Pdtutil_Free (fsmMgr->name.ns);
  Pdtutil_Free (fsmMgr->name.ps);
  Pdtutil_Free (fsmMgr->name.auxVar);

  /* Delta */
  Ddi_Free (fsmMgr->delta.bdd);

  /* Lambda */
  Ddi_Free (fsmMgr->lambda.bdd);

  /* Auxiliary Variable  */
  Ddi_Free (fsmMgr->auxVar.bdd);
  
  /* init */
  Ddi_Free (fsmMgr->init.bdd);

  /* tr */
  Ddi_Free (fsmMgr->tr.bdd);

  /* from */
  Ddi_Free (fsmMgr->from.bdd);
 
  /* reached */
  Ddi_Free (fsmMgr->reached.bdd);
  
  /* structure */
  Pdtutil_Free (fsmMgr);

  return;
}

/**Function********************************************************************

  Synopsis    [Duplicates FSM structure.]

  Description [The new FSM is allocated and all the field are duplicated
    from the original FSM.]

  SideEffects [none]

  SeeAlso     [Fsm_MgrInit ()]

******************************************************************************/

Fsm_Mgr_t *
Fsm_MgrDup (
  Fsm_Mgr_t *fsmMgr   /* FSM Manager */
)
{
  Fsm_Mgr_t *fsmMgrNew;

  /*------------------------- FSM structure creation -----------------------*/

  fsmMgrNew = Fsm_MgrInit (NULL);

  /*------------------------------- Copy Fields ----------------------------*/

  /*
   *  General Fields
   */

  Fsm_MgrSetDdManager (fsmMgrNew, Fsm_MgrReadDdManager (fsmMgr));
  
  if (Fsm_MgrReadFileName (fsmMgr) != NULL) {
    Fsm_MgrSetFileName (fsmMgrNew, Fsm_MgrReadFileName (fsmMgr));
  }

  Fsm_MgrSetBddFormat (fsmMgrNew, Fsm_MgrReadBddFormat (fsmMgr));

  /*
   *  Sizes
   */

  Fsm_MgrSetNI (fsmMgrNew, Fsm_MgrReadNI (fsmMgr));
  Fsm_MgrSetNO (fsmMgrNew, Fsm_MgrReadNO (fsmMgr));
  Fsm_MgrSetNL (fsmMgrNew, Fsm_MgrReadNL (fsmMgr));
  Fsm_MgrSetNAuxVar (fsmMgrNew, Fsm_MgrReadNAuxVar (fsmMgr));

  /*
   *  I, O, PS, NS, ... Names
   */

  if (Fsm_MgrReadNameI (fsmMgr) != NULL) {
    Fsm_MgrSetNameI (fsmMgrNew, Fsm_MgrReadNameI (fsmMgr));
  }
  if (Fsm_MgrReadNameO (fsmMgr) != NULL) {
    Fsm_MgrSetNameO (fsmMgrNew, Fsm_MgrReadNameO (fsmMgr));
  }
  if (Fsm_MgrReadNamePS (fsmMgr) != NULL) {
    Fsm_MgrSetNamePS (fsmMgrNew, Fsm_MgrReadNamePS (fsmMgr));
  }
  if (Fsm_MgrReadNameNS (fsmMgr) != NULL) {
    Fsm_MgrSetNameNS (fsmMgrNew, Fsm_MgrReadNameNS (fsmMgr));
  }
  if (Fsm_MgrReadNameNSTrueName (fsmMgr) != NULL) {
    Fsm_MgrSetNameNSTrueName (fsmMgrNew, Fsm_MgrReadNameNSTrueName (fsmMgr));
  }

  /*
   *  Indexes
   */

  if (Fsm_MgrReadIndexI (fsmMgr) != NULL) {
    Fsm_MgrSetIndexI (fsmMgrNew, Fsm_MgrReadIndexI (fsmMgr));
  }
  if (Fsm_MgrReadIndexO (fsmMgr) != NULL) {
    Fsm_MgrSetIndexO (fsmMgrNew, Fsm_MgrReadIndexO (fsmMgr));
  }
  if (Fsm_MgrReadIndexPS (fsmMgr) != NULL) {
    Fsm_MgrSetIndexPS (fsmMgrNew, Fsm_MgrReadIndexPS (fsmMgr));
  }
  if (Fsm_MgrReadIndexNS (fsmMgr) != NULL) {
    Fsm_MgrSetIndexNS (fsmMgrNew, Fsm_MgrReadIndexNS (fsmMgr));
  }

  /*
   *  Variables ...
   */

  if (Fsm_MgrReadVarI (fsmMgr) != NULL) {
    Fsm_MgrSetVarI (fsmMgrNew, Fsm_MgrReadVarI (fsmMgr));
  }
  if (Fsm_MgrReadVarPS (fsmMgr) != NULL) {
    Fsm_MgrSetVarPS (fsmMgrNew, Fsm_MgrReadVarPS (fsmMgr));
  }
  if (Fsm_MgrReadVarNS (fsmMgr) != NULL) {
    Fsm_MgrSetVarNS (fsmMgrNew, Fsm_MgrReadVarNS (fsmMgr));
  }
  if (Fsm_MgrReadVarAuxVar (fsmMgr) != NULL) {
    Fsm_MgrSetVarAuxVar (fsmMgrNew, Fsm_MgrReadVarAuxVar (fsmMgr));
  }
  if (Fsm_MgrReadVarO (fsmMgr) != NULL) {
    Fsm_MgrSetVarO (fsmMgrNew, Fsm_MgrReadVarO (fsmMgr));
  }

  if (Fsm_MgrReadVarnames (fsmMgr) != NULL) {
    Fsm_MgrSetVarnames (fsmMgrNew, Fsm_MgrReadVarnames (fsmMgr));
  }
  if (Fsm_MgrReadVarauxids (fsmMgr) != NULL) {
    Fsm_MgrSetVarauxids (fsmMgrNew, Fsm_MgrReadVarauxids (fsmMgr));
  }

  if (Fsm_MgrReadOrdNodeid (fsmMgr) != NULL) {
    Fsm_MgrSetOrdNodeid (fsmMgrNew, Fsm_MgrReadOrdNodeid (fsmMgr));
  }

  /*
   *  BDDs
   */
  
  if (Fsm_MgrReadInitBDD (fsmMgr) != NULL) {  
    Fsm_MgrSetInitBDD (fsmMgrNew, Fsm_MgrReadInitBDD (fsmMgr));
  }
  if (Fsm_MgrReadDeltaBDD (fsmMgr) != NULL) {
    Fsm_MgrSetDeltaBDD (fsmMgrNew, Fsm_MgrReadDeltaBDD (fsmMgr));
  }
  if (Fsm_MgrReadLambdaBDD (fsmMgr) != NULL) {
    Fsm_MgrSetLambdaBDD (fsmMgrNew, Fsm_MgrReadLambdaBDD (fsmMgr));
  }
  if (Fsm_MgrReadAuxVarBDD (fsmMgr) != NULL) {
    Fsm_MgrSetAuxVarBDD (fsmMgrNew, Fsm_MgrReadAuxVarBDD (fsmMgr));
  }
  if (Fsm_MgrReadTrBDD (fsmMgr) != NULL) {  
    Fsm_MgrSetTrBDD (fsmMgrNew, Fsm_MgrReadTrBDD (fsmMgr));
  }
  if (Fsm_MgrReadReachedBDD (fsmMgr) != NULL) {
    Fsm_MgrSetReachedBDD (fsmMgrNew, Fsm_MgrReadReachedBDD (fsmMgr));
  }

  /*
   *  BDD Names
   */

  if (Fsm_MgrReadDeltaName (fsmMgr) != NULL) {
    Fsm_MgrSetDeltaName (fsmMgrNew, Fsm_MgrReadDeltaName (fsmMgr));
  }
  if (Fsm_MgrReadLambdaName (fsmMgr) != NULL) {
    Fsm_MgrSetLambdaName (fsmMgrNew, Fsm_MgrReadLambdaName (fsmMgr));
  }
  if (Fsm_MgrReadAuxVarName (fsmMgr) != NULL) {
    Fsm_MgrSetAuxVarName (fsmMgrNew, Fsm_MgrReadAuxVarName (fsmMgr));
  }
  if (Fsm_MgrReadTrName (fsmMgr) != NULL) {
    Fsm_MgrSetTrName (fsmMgrNew, Fsm_MgrReadTrName (fsmMgr));
  }
  if (Fsm_MgrReadInitString (fsmMgr) != NULL) {
    Fsm_MgrSetInitString (fsmMgrNew, Fsm_MgrReadInitString (fsmMgr));
  }
  if (Fsm_MgrReadInitName (fsmMgr) != NULL) {
    Fsm_MgrSetInitName (fsmMgrNew, Fsm_MgrReadInitName (fsmMgr));
  }

  return (fsmMgrNew);
}

/**Function********************************************************************

  Synopsis    [Remove Auxiliary Variables from the FSM structure.]

  Description [The same FSM is returned in any case.]

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrAuxVarRemove (
  Fsm_Mgr_t *fsmMgr   /* FSM Manager */
  )
{
  Ddi_Mgr_t *ddMgr;
  Ddi_Vararray_t *auxVararray;
  Ddi_Varset_t *auxVarset;
  Ddi_Bddarray_t *auxFunArray, *deltaArray;
  Ddi_Bdd_t *delta, *deltaNew, *auxRelation, *tmpRelation;
  long startTime, currTime;
  int i, psNumber, auxVarNumber;

  auxVarNumber = Fsm_MgrReadNAuxVar (fsmMgr);
  fprintf (stdout, "Auxiliary Variable Number: %d\n", auxVarNumber);

  if (auxVarNumber != 0) {

    /*------------------------- Set What is Needed --------------------------*/

    startTime = util_cpu_time();
    ddMgr = Fsm_MgrReadDdManager (fsmMgr);
    psNumber =  Fsm_MgrReadNL (fsmMgr);
    deltaArray = Fsm_MgrReadDeltaBDD (fsmMgr);
    auxVararray = Fsm_MgrReadVarAuxVar (fsmMgr);
    auxFunArray = Fsm_MgrReadAuxVarBDD (fsmMgr);
    auxVarset = Ddi_VarsetMakeFromArray (auxVararray);

    if (auxVararray==NULL || auxFunArray==NULL) {
      Pdtutil_Warning (1, "NULL Arrays of Auxiliary Variables.");
      return;
    }

    /*------------- Compose Next State with Auxiliary Variables -------------*/

    auxRelation = Ddi_BddRelMakeFromArray(auxFunArray, auxVararray);

    for (i=0; i<psNumber; i++) {
      tmpRelation = Ddi_BddDup (auxRelation);
      delta = Ddi_BddarrayExtract (deltaArray, i);
      
      Ddi_BddPartInsert (tmpRelation, 0, delta);
      deltaNew = Part_BddMultiwayLinearAndExist(tmpRelation,auxVarset,-1);

      Ddi_Free (tmpRelation);
      Ddi_Free (delta);
      Ddi_BddarrayWrite (deltaArray, i, deltaNew);
    }

    /*------------------------------- Free ... ------------------------------*/

    Ddi_Free (auxRelation);
    Ddi_Free (auxFunArray);
    Ddi_Free (auxVararray);
    Ddi_Free (auxVarset);

    /*------------------------ Modify the FSM Manager -----------------------*/

    Fsm_MgrSetDeltaBDD (fsmMgr, deltaArray);
    Fsm_MgrSetNAuxVar (fsmMgr, 0);
    Fsm_MgrSetVarAuxVar (fsmMgr, NULL);
    Fsm_MgrSetAuxVarBDD (fsmMgr, NULL);

    /*---------------------------- Print-Out Time ---------------------------*/

    currTime = util_cpu_time();
    fsmMgr->fsmTime += currTime - startTime;
    fprintf (stdout, "Composition Time: %s\n",
      util_print_time (currTime - startTime));
  }

  return;
}



/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     [CmdMgrOperation, CmdRegOperation, Trav_MgrOperation,
    Tr_MgrOperation]

******************************************************************************/

int
Fsm_MgrOperation (
  Fsm_Mgr_t *fsmMgr               /* FSM Manager */,
  char *string                    /* String */,
  Pdtutil_MgrOp_t operationFlag   /* Operation Flag */,
  void **voidPointer              /* Generic Pointer */,
  Pdtutil_MgrRet_t *returnFlagP   /* Type of the Pointer Returned */
  )
{ 
  char *stringFirst, *stringSecond;
  Ddi_Mgr_t *defaultDdMgr, *tmpDdMgr;
  Ddi_Bdd_t *bdd;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (fsmMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return(1);
  }

  defaultDdMgr = Fsm_MgrReadDdManager (fsmMgr);

  /*----------------------- Take Main and Secondary Name --------------------*/
  
  Pdtutil_ReadSubstring (string, &stringFirst, &stringSecond);

  /*------------------------ Operation on the FSM Manager -------------------*/

  if (stringFirst == NULL) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "FSMManager Verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Fsm_MgrReadVerbosity (fsmMgr)),
          Fsm_MgrReadVerbosity (fsmMgr));
        return (0);
        break;
      case Pdtutil_MgrOpStats_c:
        Fsm_MgrPrintStats (fsmMgr);
        break;
      case Pdtutil_MgrOpMgrRead_c:
        *voidPointer = (void *) Fsm_MgrReadDdManager (fsmMgr);
        *returnFlagP = Pdtutil_MgrRetDdMgr_c;
        break;
      case Pdtutil_MgrOpMgrSet_c:
        tmpDdMgr = (Ddi_Mgr_t *) *voidPointer;
        Fsm_MgrSetDdManager (fsmMgr, tmpDdMgr);
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on FSM Manager");
	break;
    }

    return (0);
  }

  /*----------------------------- Package is DDI ----------------------------*/

  if (strcmp (stringFirst, "ddm")==0) {
    Ddi_MgrOperation (&fsmMgr->dd, stringSecond, operationFlag,
      voidPointer, returnFlagP);

    if (*returnFlagP == Pdtutil_MgrOpMgrDelete_c) {
      fsmMgr->dd = NULL;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*-------------------------------- Options -------------------------------*/

  if (strcmp(stringFirst, "verbosity")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
	Fsm_MgrSetVerbosity (fsmMgr,
	  Pdtutil_VerbosityString2Enum (*voidPointer));
	break;
      case Pdtutil_MgrOpOptionShow_c:
	fprintf (stdout, "Verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Fsm_MgrReadVerbosity (fsmMgr)),
          Fsm_MgrReadVerbosity (fsmMgr));
	break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on FSM Manager");
	break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*-------------------------- Transition Relation --------------------------*/

  if (strcmp(stringFirst, "tr")==0) {
    Ddi_BddOperation (defaultDdMgr, &(fsmMgr->tr.bdd),
      stringSecond, operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  

  if (strcmp(stringFirst, "reached")==0) {
    Ddi_BddOperation (defaultDdMgr, &(fsmMgr->reached.bdd),
      stringSecond, operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*---------------------------- Initial States -----------------------------*/

  if (strcmp(stringFirst, "init")==0) {
    Ddi_BddOperation (defaultDdMgr, &(fsmMgr->init.bdd),
      stringSecond, operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*------------------------------- From Set --------------------------------*/

  if (strcmp(stringFirst, "from")==0) {
    bdd = Fsm_MgrReadFromBDD (fsmMgr);
    Ddi_BddOperation (defaultDdMgr, &(fsmMgr->from.bdd),
      stringSecond, operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*---------------------------- Lambda Functions ---------------------------*/

  if (strcmp(stringFirst, "lambda")==0) {
    Ddi_BddarrayOperation (defaultDdMgr, &(fsmMgr->lambda.bdd),
      stringSecond, operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*---------------------------- Delta Functions ----------------------------*/

  if (strcmp (stringFirst, "delta")==0) {
    Ddi_BddarrayOperation (defaultDdMgr, &(fsmMgr->delta.bdd),
      stringSecond, operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*---------------------------- Delta Functions ----------------------------*/

  if (strcmp (stringFirst, "auxVar")==0) {
    Ddi_BddarrayOperation (defaultDdMgr, &(fsmMgr->auxVar.bdd),
      stringSecond, operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*------------------------------- No Match --------------------------------*/

  Pdtutil_Warning (1, "Operation on FSM manager not allowed");
  return (1);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char *
Fsm_MgrReadFileName (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->fileName);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char *
Fsm_MgrReadFsmName (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->fsmName);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Mgr_t * 
Fsm_MgrReadDdManager (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->dd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t *
Fsm_MgrReadReachedBDD (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->reached.bdd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int
Fsm_MgrReadBddFormat (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->bddFormat);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int 
Fsm_MgrReadNI (
  Fsm_Mgr_t *fsmMgr
  )                 
{
  return (fsmMgr->size.i);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int 
Fsm_MgrReadNO (
  Fsm_Mgr_t *fsmMgr
  )                 
{
  return (fsmMgr->size.o);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int 
Fsm_MgrReadNL (
  Fsm_Mgr_t *fsmMgr
  )                 
{
  return (fsmMgr->size.l);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int 
Fsm_MgrReadNAuxVar (
  Fsm_Mgr_t *fsmMgr
  )                 
{
  return (fsmMgr->size.auxVar);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadOrdFileName (
  Fsm_Mgr_t *fsmMgr
  )       
{
  return (fsmMgr->ord.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadOrdNodeName (
  Fsm_Mgr_t *fsmMgr
  )       
{
  return (fsmMgr->ord.nodename);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadOrdNodeid (
  Fsm_Mgr_t *fsmMgr
  )         
{
  return (fsmMgr->ord.nodeid);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadNameI (
  Fsm_Mgr_t *fsmMgr
  )             
{
  return (fsmMgr->name.i);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadNameO (
  Fsm_Mgr_t *fsmMgr
  )             
{
  return (fsmMgr->name.o);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadNamePS (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->name.ps);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadNameNS (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->name.ns);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadNameNSTrueName (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->name.nsTrueName);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadNameAuxVar (
  Fsm_Mgr_t *fsmMgr
  )             
{
  return (fsmMgr->name.auxVar);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadIndexI (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->index.i);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadIndexO (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->index.o);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadIndexNS (
  Fsm_Mgr_t *fsmMgr
  )           
{
  return (fsmMgr->index.ns);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadIndexPS (
  Fsm_Mgr_t *fsmMgr
  )           
{
  return (fsmMgr->index.ps);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadIndexAuxVar (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->index.auxVar);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t * 
Fsm_MgrReadVarI (
  Fsm_Mgr_t *fsmMgr
  )             
{
  return (fsmMgr->var.i);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t * 
Fsm_MgrReadVarO (
  Fsm_Mgr_t *fsmMgr
  )             
{
  return (fsmMgr->var.o);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t * 
Fsm_MgrReadVarPS (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->var.ps);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t * 
Fsm_MgrReadVarNS (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->var.ns);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t * 
Fsm_MgrReadVarAuxVar (
  Fsm_Mgr_t *fsmMgr
  )             
{
  return (fsmMgr->var.auxVar);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadDeltaName (
  Fsm_Mgr_t *fsmMgr
  )         
{
  return (fsmMgr->delta.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Bddarray_t * 
Fsm_MgrReadDeltaBDD (
  Fsm_Mgr_t *fsmMgr
  )          
{
  return (fsmMgr->delta.bdd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadLambdaName (
  Fsm_Mgr_t *fsmMgr
  )        
{
  return (fsmMgr->lambda.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Bddarray_t * 
Fsm_MgrReadLambdaBDD (
  Fsm_Mgr_t *fsmMgr
  )         
{
  return (fsmMgr->lambda.bdd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadAuxVarName (
  Fsm_Mgr_t *fsmMgr
  )         
{
  return (fsmMgr->auxVar.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Bddarray_t * 
Fsm_MgrReadAuxVarBDD (
  Fsm_Mgr_t *fsmMgr
  )          
{
  return (fsmMgr->auxVar.bdd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadTrName (
  Fsm_Mgr_t *fsmMgr
  )            
{
  return (fsmMgr->tr.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t * 
Fsm_MgrReadTrBDD (
  Fsm_Mgr_t *fsmMgr
  )             
{
  return (fsmMgr->tr.bdd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadInitName (
  Fsm_Mgr_t *fsmMgr
  )          
{
  return (fsmMgr->init.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadInitString (
  Fsm_Mgr_t *fsmMgr
  )        
{
  return (fsmMgr->init.string);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadTrString (
  Fsm_Mgr_t *fsmMgr
  )        
{
  return (fsmMgr->tr.string);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadReachedString (
  Fsm_Mgr_t *fsmMgr
  )        
{
  return (fsmMgr->reached.string);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char * 
Fsm_MgrReadFromString (
  Fsm_Mgr_t *fsmMgr
  )        
{
  return (fsmMgr->init.string);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t * 
Fsm_MgrReadInitBDD (
  Fsm_Mgr_t *fsmMgr
  )           
{
  return (fsmMgr->init.bdd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char *
Fsm_MgrReadFromName (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->from.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char *
Fsm_MgrReadReachedName (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->reached.name);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t *
Fsm_MgrReadFromBDD (
  Fsm_Mgr_t *fsmMgr
  )
{
  return (fsmMgr->from.bdd);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char ** 
Fsm_MgrReadVarnames (
  Fsm_Mgr_t *fsmMgr
  )          
{
  return (fsmMgr->varnames);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadVarauxids (
  Fsm_Mgr_t *fsmMgr
  )          
{
  return (fsmMgr->varauxids);
}

/**Function********************************************************************
  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

int * 
Fsm_MgrReadInvauxids (
  Fsm_Mgr_t *fsmMgr
  )          
{
  return (fsmMgr->invauxids);
}

/**Function********************************************************************
  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Pdtutil_VerbLevel_e
Fsm_MgrReadVerbosity (
  Fsm_Mgr_t *fsmMgr
  )          
{
  return (fsmMgr->settings.verbosity);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetFileName (
  Fsm_Mgr_t *fsmMgr, 
  char *fileName
  )
{
  if (fsmMgr->fileName != NULL) {
    Pdtutil_Free (fsmMgr->fileName);
  }

  if (fileName != NULL) {
    fsmMgr->fileName = Pdtutil_StrDup (fileName);
  } else {
    fsmMgr->fileName = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetFsmName (
  Fsm_Mgr_t *fsmMgr, 
  char *fsmName
  )
{
  if (fsmMgr->fsmName != NULL) {
    Pdtutil_Free (fsmMgr->fsmName);
  }

  if (fsmName != NULL) {
    fsmMgr->fsmName = Pdtutil_StrDup (fsmName);
  } else {
    fsmMgr->fsmName = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetDdManager (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Mgr_t *var
  )          
{
  fsmMgr->dd = var;

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void Fsm_MgrSetReachedBDD (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Bdd_t *var
  )
{
  if (var != NULL) {
    fsmMgr->reached.bdd = Ddi_BddDup (var);
  } else {
    fsmMgr->reached.bdd = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetBddFormat (
  Fsm_Mgr_t *fsmMgr,
  int var
  )
{
  fsmMgr->bddFormat = var;

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNI (
  Fsm_Mgr_t *fsmMgr, 
  int var
  )                 
{
  fsmMgr->size.i = var;

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNO (
  Fsm_Mgr_t *fsmMgr, 
  int var
  )                 
{
  fsmMgr->size.o = var;

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNL (
  Fsm_Mgr_t *fsmMgr, 
  int var
  )                 
{
  fsmMgr->size.l = var;

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNAuxVar (
  Fsm_Mgr_t *fsmMgr, 
  int var
  )                 
{
  fsmMgr->size.auxVar = var;

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetOrdFileName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )        
{
  if (fsmMgr->ord.name != NULL) {
    Pdtutil_Free (fsmMgr->ord.name);
  }

  if (var != NULL) { 
    fsmMgr->ord.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->ord.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetOrdNodeName (
  Fsm_Mgr_t *fsmMgr, 
  char **var
  )        
{
  int n;

  if (fsmMgr->ord.nodename != NULL) {
    Pdtutil_Free (fsmMgr->ord.nodename);
  }

  if (var != NULL) {
    n = Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr);
    fsmMgr->ord.nodename = Pdtutil_StrArrayDup (var, n);
  } else {
    fsmMgr->ord.nodename = NULL;
  }
 
  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetOrdNodeid (
  Fsm_Mgr_t *fsmMgr,
  int *var
  )
{
  int n;

  if (fsmMgr->ord.nodeid != NULL) {
    Pdtutil_Free (fsmMgr->ord.nodeid);
    exit (1);
  }

  if (var != NULL) {
    n = Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr);
    fsmMgr->ord.nodeid = Pdtutil_IntArrayDup (var, n);
  } else {
    fsmMgr->ord.nodeid = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNameI (
  Fsm_Mgr_t *fsmMgr, 
  char **var
  )              
{
  int ni;

  if (fsmMgr->name.i != NULL) {
    Pdtutil_Free (fsmMgr->name.i);
    exit (1);
  }
  
  if (var != NULL) {
    ni = Fsm_MgrReadNI (fsmMgr);
    fsmMgr->name.i = Pdtutil_StrArrayDup (var, ni);
  } else {
    fsmMgr->name.i = NULL;
  }
  
  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNameO (
  Fsm_Mgr_t *fsmMgr, 
  char **var
  )              
{
  int no;

  if (fsmMgr->name.o != NULL) {
    Pdtutil_Free (fsmMgr->name.o);
  }
  
  if(var != NULL) {
    no = Fsm_MgrReadNO(fsmMgr);
    fsmMgr->name.o = Pdtutil_StrArrayDup (var, no);
  } else {
    fsmMgr->name.o = NULL;
  }
 
  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNameNS (
  Fsm_Mgr_t *fsmMgr, 
  char **var
  )             
{
  int nl;

  if (fsmMgr->name.ns != NULL) {
    Pdtutil_Free (fsmMgr->name.ns);
  }

  if(var != NULL) {
    nl = Fsm_MgrReadNL(fsmMgr);
    fsmMgr->name.ns = Pdtutil_StrArrayDup (var, nl);
  } else {
    fsmMgr->name.ns = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNameNSTrueName (
  Fsm_Mgr_t *fsmMgr, 
  char **var
  )             
{
  int nl;

  if (fsmMgr->name.nsTrueName != NULL) {
    Pdtutil_Free (fsmMgr->name.nsTrueName);
  }

  if(var != NULL) {
    nl = Fsm_MgrReadNL(fsmMgr);
    fsmMgr->name.nsTrueName = Pdtutil_StrArrayDup (var, nl);
  } else {
    fsmMgr->name.nsTrueName = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNamePS (
  Fsm_Mgr_t *fsmMgr, 
  char **var
  )             
{
  int nl;

  if (fsmMgr->name.ps != NULL) {
    Pdtutil_Free (fsmMgr->name.ps);
  }

  if(var != NULL) {
    nl = Fsm_MgrReadNL(fsmMgr);
    fsmMgr->name.ps = Pdtutil_StrArrayDup (var, nl);
  } else {
    fsmMgr->name.ps = NULL;
  }

 
  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetNameAuxVar (
  Fsm_Mgr_t *fsmMgr, 
  char **var
  )              
{
  int auxVarN;
  
  if (fsmMgr->name.auxVar != NULL) {
    Pdtutil_Free (fsmMgr->name.auxVar);
  }

  if (var != NULL) {
    auxVarN = Fsm_MgrReadNAuxVar (fsmMgr);
    fsmMgr->name.auxVar = Pdtutil_StrArrayDup (var, auxVarN);
  } else {
    fsmMgr->name.auxVar = NULL;
  }
  
  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetIndexI (
  Fsm_Mgr_t *fsmMgr, 
  int *var
  )             
{
  int ni;
  
  if (fsmMgr->index.i != NULL) {
    Pdtutil_Free (fsmMgr->index.i);
  }

  if (var != NULL) {
    ni = Fsm_MgrReadNI (fsmMgr);
    fsmMgr->index.i = Pdtutil_IntArrayDup (var, ni);
  } else {
    fsmMgr->index.i = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetIndexO (
  Fsm_Mgr_t *fsmMgr, 
  int *var
  )             
{
  int no;
  
  if (fsmMgr->index.o != NULL) {
    Pdtutil_Free (fsmMgr->index.o);
  }

  if (var != NULL) {
    no = Fsm_MgrReadNO (fsmMgr);
    fsmMgr->index.o = Pdtutil_IntArrayDup (var, no);
  } else {
    fsmMgr->index.o = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetIndexNS (
  Fsm_Mgr_t *fsmMgr, 
  int *var
  )            
{
  int nl;
  
  if (fsmMgr->index.ns != NULL) {
    Pdtutil_Free (fsmMgr->index.ns);
  }

  if (var != NULL) {
    nl = Fsm_MgrReadNL (fsmMgr);
    fsmMgr->index.ns = Pdtutil_IntArrayDup (var, nl);
  } else {
    fsmMgr->index.ns = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetIndexPS (
  Fsm_Mgr_t *fsmMgr, 
  int *var
  )            
{
  int nl;
  
  if (fsmMgr->index.ps != NULL) {
    Pdtutil_Free (fsmMgr->index.ps);
  }

  if (var != NULL) {
    nl = Fsm_MgrReadNL (fsmMgr);
    fsmMgr->index.ps = Pdtutil_IntArrayDup (var, nl);
  } else {
    fsmMgr->index.ps = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetIndexAuxVar (
  Fsm_Mgr_t *fsmMgr, 
  int *var
  )             
{
  int auxVarN;
  
  if (fsmMgr->index.auxVar != NULL) {
    Pdtutil_Free (fsmMgr->index.auxVar);
  }

  if (var != NULL) {
    auxVarN = Fsm_MgrReadNAuxVar (fsmMgr);
    fsmMgr->index.auxVar = Pdtutil_IntArrayDup (var, auxVarN);
  } else {
    fsmMgr->index.auxVar = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetVarI (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Vararray_t *var
  )               
{
  if (fsmMgr->var.i != NULL) {
    Ddi_Free (fsmMgr->var.i);
  }

  if (var != NULL) {
    fsmMgr->var.i = Ddi_VararrayDup (var);
  } else {
    fsmMgr->var.i = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetVarO (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Vararray_t *var
  )               
{
  if (fsmMgr->var.o != NULL) {
    Ddi_Free (fsmMgr->var.o);
  }

  if (var != NULL) {
    fsmMgr->var.o = Ddi_VararrayDup (var);
  } else {
    fsmMgr->var.o = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetVarPS (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Vararray_t *var
  )              
{
  if (fsmMgr->var.ps != NULL) {
    Ddi_Free (fsmMgr->var.ps);
  }

  if (var != NULL) {
    fsmMgr->var.ps = Ddi_VararrayDup (var);
  } else {
    fsmMgr->var.ps = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetVarNS (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Vararray_t *var
  )              
{
  if (fsmMgr->var.ns != NULL) {
    Ddi_Free (fsmMgr->var.ns);
  }

  if (var != NULL) {
    fsmMgr->var.ns = Ddi_VararrayDup (var);
  } else {
    fsmMgr->var.ns = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetVarAuxVar (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Vararray_t *var
  )               
{
  if (fsmMgr->var.auxVar != NULL) {
    Ddi_Free (fsmMgr->var.auxVar);
  }

  if (var != NULL) {
    fsmMgr->var.auxVar = Ddi_VararrayDup (var);
  } else {
    fsmMgr->var.auxVar = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetDeltaName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )          
{
  if (fsmMgr->delta.name != NULL) {
    Pdtutil_Free (fsmMgr->delta.name);
  }

  if (var != NULL) {
    fsmMgr->delta.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->delta.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetDeltaBDD (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Bddarray_t *delta
  )           
{
  if (fsmMgr->delta.bdd != NULL) {
    Ddi_Free (fsmMgr->delta.bdd);
  }

  if (delta != NULL) {
    fsmMgr->delta.bdd = Ddi_BddarrayDup (delta);
  } else {
    fsmMgr->delta.bdd = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetLambdaName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )         
{
  if (fsmMgr->lambda.name != NULL) {
    Pdtutil_Free (fsmMgr->lambda.name);
  }

  if (var != NULL) {
    fsmMgr->lambda.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->lambda.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetLambdaBDD (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Bddarray_t *var
  )          
{
  if (fsmMgr->lambda.bdd != NULL) {
    Ddi_Free (fsmMgr->lambda.bdd);
  }

  if (var != NULL) {
    fsmMgr->lambda.bdd = Ddi_BddarrayDup (var);
  } else {
    fsmMgr->lambda.bdd = NULL;
  }

  return;
}


/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetAuxVarName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )          
{
  if (fsmMgr->auxVar.name != NULL) {
    Pdtutil_Free (fsmMgr->auxVar.name);
  }

  if (var != NULL) {
    fsmMgr->auxVar.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->auxVar.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetAuxVarBDD (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Bddarray_t *var
  )           
{
  if (fsmMgr->auxVar.bdd != NULL) {
    Ddi_Free (fsmMgr->auxVar.bdd);
  }

  if (var != NULL) {
    fsmMgr->auxVar.bdd = Ddi_BddarrayDup (var);
  } else {
    fsmMgr->auxVar.bdd = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetTrName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )             
{
  if (fsmMgr->tr.name != NULL) {
    Pdtutil_Free (fsmMgr->tr.name);
  }

  if (var != NULL) {
    fsmMgr->tr.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->tr.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetTrBDD (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Bdd_t *var
  )              
{
  if (fsmMgr->tr.bdd != NULL) {
    Ddi_Free (fsmMgr->tr.bdd);
  }

  if (var != NULL) {
    fsmMgr->tr.bdd = Ddi_BddDup (var);
  } else {
    fsmMgr->tr.bdd = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetInitName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )            
{
  if (fsmMgr->init.name != NULL) {
    Pdtutil_Free (fsmMgr->init.name);
  }

  if (var != NULL) {
    fsmMgr->init.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->init.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetInitString (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )         
{
  if (fsmMgr->init.string != NULL) {
    Pdtutil_Free (fsmMgr->init.string);
  }

  if (var != NULL) {
    fsmMgr->init.string = Pdtutil_StrDup (var);
  } else {
    fsmMgr->init.string = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetTrString (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )         
{
  if (fsmMgr->tr.string != NULL) {
    Pdtutil_Free (fsmMgr->tr.string);
  }

  if (var != NULL) {
    fsmMgr->tr.string = Pdtutil_StrDup (var);
  } else {
    fsmMgr->tr.string = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetFromString (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )         
{
  if (fsmMgr->from.string != NULL) {
    Pdtutil_Free (fsmMgr->from.string);
  }

  if (var != NULL) {
    fsmMgr->from.string = Pdtutil_StrDup (var);
  } else {
    fsmMgr->from.string = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetReachedString (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )         
{
  if (fsmMgr->reached.string != NULL) {
    Pdtutil_Free (fsmMgr->reached.string);
    exit (1);
  }

  if (var != NULL) {
    fsmMgr->reached.string = Pdtutil_StrDup (var);
  } else {
    fsmMgr->reached.string = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetInitBDD (
  Fsm_Mgr_t *fsmMgr, 
  Ddi_Bdd_t *var
  )            
{
  if (fsmMgr->init.bdd != NULL) {
    Ddi_Free (fsmMgr->init.bdd);
  }

  if (var != NULL) {
    fsmMgr->init.bdd = Ddi_BddDup (var);
  } else {
    fsmMgr->init.bdd = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetFromName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )             
{
  if (fsmMgr->from.name != NULL) {
    Pdtutil_Free (fsmMgr->from.name);
  }

  if (var != NULL) {
    fsmMgr->from.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->from.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetReachedName (
  Fsm_Mgr_t *fsmMgr, 
  char *var
  )             
{
  if (fsmMgr->reached.name != NULL) {
    Pdtutil_Free (fsmMgr->reached.name);
  }

  if (var != NULL) {
    fsmMgr->reached.name = Pdtutil_StrDup (var);
  } else {
    fsmMgr->reached.name = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetFromBDD (
  Fsm_Mgr_t *fsmMgr,
  Ddi_Bdd_t *var
  )
{
  if (fsmMgr->from.bdd != NULL) {
    Ddi_Free (fsmMgr->from.bdd);
  }

  if (var != NULL) {
    fsmMgr->from.bdd = Ddi_BddDup (var);
  } else {
    fsmMgr->from.bdd = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetVarnames (
  Fsm_Mgr_t *fsmMgr,
  char **var
  )
{
  int n;

  if (fsmMgr->varnames != NULL) {
    Pdtutil_Free (fsmMgr->varnames);
  }

  if (var != NULL) {
    n = Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr);
    fsmMgr->varnames = Pdtutil_StrArrayDup (var, n);
  } else {
    fsmMgr->varnames = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Fsm_MgrSetVarnamesOne (
  Fsm_Mgr_t *fsmMgr,
  char *var,
  int i
  )
{
  if (var != NULL) {
    fsmMgr->varnames[i] = Pdtutil_StrDup (var);
  } else {
    fsmMgr->varnames[i] = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetVarauxids (
  Fsm_Mgr_t *fsmMgr, 
  int *var
  )          
{
  int n;

  if (fsmMgr->varauxids != NULL) {
    Pdtutil_Free (fsmMgr->varauxids);
  }

  if (var != NULL) {
    n = Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr);
    fsmMgr->varauxids = Pdtutil_IntArrayDup (var, n);
  } else {
    fsmMgr->varauxids = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetInvauxids (
  Fsm_Mgr_t *fsmMgr, 
  int *var
  )          
{
  int n;

  if (fsmMgr->invauxids != NULL) {
    Pdtutil_Free (fsmMgr->invauxids);
}

  if (var != NULL) {
    n = Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr);
    fsmMgr->invauxids = Pdtutil_IntArrayDup (var, n);
  } else {
    fsmMgr->invauxids = NULL;
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Fsm_MgrSetVerbosity (
  Fsm_Mgr_t *fsmMgr, 
  Pdtutil_VerbLevel_e var
  )          
{
  fsmMgr->settings.verbosity = var;

  return;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

