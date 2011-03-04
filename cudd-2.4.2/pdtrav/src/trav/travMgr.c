/**CFile***********************************************************************

  FileName    [travMgr.c]

  PackageName [trav]

  Synopsis    [Functions to handle a Trav manager]

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

#include "travInt.h"

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

  Synopsis    [Create a Manager.]

  Description []

  SideEffects [none]

  SeeAlso     [Ddi_MgrQuit, Fsm_MgrQuit, Tr_MgrQuit]

******************************************************************************/

Trav_Mgr_t *
Trav_MgrInit (
  char *travName   /* Name of the Trav structure */,
  Ddi_Mgr_t *dd    /* ddi manager */
)
{
  Trav_Mgr_t *travMgr;

  /*------------------------- Allocation -----------------------------*/

  travMgr = Pdtutil_Alloc (Trav_Mgr_t, 1);
  if (travMgr == NULL) {
    fprintf (stderr, "travMgr.c Error: Out of memory.\n");
    return (NULL);
  }

  /*------------------- Set the Name of the Structure ------------------*/

  if (travName == NULL) {
    travName = "trav";
  }

  travMgr->travName = Pdtutil_StrDup(travName);

  /*------------------------ Initializations -------------------------*/

  travMgr->ddiMgrTr = dd;
  travMgr->ddiMgrR = dd;
  travMgr->ddiMgrAux = NULL;
  travMgr->tr = NULL;
  travMgr->i  = NULL;
  travMgr->ns = NULL;
  travMgr->ps = NULL;
  travMgr->aux = NULL;
  travMgr->from = NULL;
  travMgr->reached = NULL;
  travMgr->assert = NULL;
  travMgr->newi = NULL;
  travMgr->level = 1;
  travMgr->productPeak = 0;
  travMgr->travTime = 0.0;
  travMgr->assertFlag = 0;

  travMgr->settings.verbosity = Pdtutil_VerbLevelUsrMax_c;
  travMgr->settings.logPeriod = 1;
  travMgr->settings.savePeriod = -1;
  travMgr->settings.savePeriodName = NULL;
  travMgr->settings.maxIter = -1;
  travMgr->settings.backward = 0;
  travMgr->settings.keepNew = 0;
  travMgr->settings.fromSelect = Trav_FromSelectCofactor_c;
  travMgr->settings.mgrAuxFlag = 0;

  /*------------------------ Profile Options -------------------------*/

  travMgr->settings.trProfileDynamicEnable = 0;
  travMgr->settings.trProfileThreshold = -1;
  travMgr->settings.trProfileMethod = Cuplus_None;

  /*----------------- Old Options Inizializations --------------------*/

  travMgr->settings.squaring = 0;
  travMgr->settings.enableDdR = 0;
  travMgr->settings.removeLLatches = 0;
  travMgr->settings.smoothVars = TRAV_SMOOTH_SX;
  travMgr->settings.partDisjTrThresh = -1;
  travMgr->settings.sorting = 0;
  travMgr->settings.w1 = TRSORT_DEFAULT_W1;
  travMgr->settings.w2 = TRSORT_DEFAULT_W2;
  travMgr->settings.w3 = TRSORT_DEFAULT_W3;
  travMgr->settings.w4 = TRSORT_DEFAULT_W4;
  travMgr->settings.threshold = TR_IMAGE_CLUSTER_SIZE;

  return (travMgr);
}

/**Function********************************************************************

  Synopsis    [Closes a Manager.]

  Description []

  SideEffects [none]

  SeeAlso     [Ddi_BddiMgrInit]

******************************************************************************/

void
Trav_MgrQuit(
  Trav_Mgr_t *travMgr     /* Traversal manager */
)
{
  Pdtutil_Free(travMgr->travName);
  
  Ddi_Free (travMgr->i);
  Ddi_Free (travMgr->ps);
  Ddi_Free (travMgr->ns);

  Tr_TrFree (travMgr->tr);
  Ddi_Free (travMgr->from);
  Ddi_Free (travMgr->reached);

  Ddi_Free (travMgr->assert);

  Ddi_Free (travMgr->newi);

  Pdtutil_Free (travMgr->settings.savePeriodName);

  Pdtutil_Free (travMgr);

  return;
}

/**Function********************************************************************

  Synopsis           [Print Statistics on the Traversal Manager.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/

int
Trav_MgrPrintStats (
  Trav_Mgr_t *travMgr     /* Traversal manager */
  )
{
  fprintf (stdout, "Traversal Manager Name %s\n",
    travMgr->travName);
  fprintf (stdout, "Total CPU Time: %s\n",
    util_print_time (travMgr->travTime));
 
  return (1);
}


/**Function********************************************************************

  Synopsis    [Perform an Operation on the Traversal Manager.]

  Description []

  SideEffects []

  SeeAlso     [CmdMgrOperation, CmdRegOperation, Fsm_MgrOperation,
    Tr_MgrOperation]

******************************************************************************/

int
Trav_MgrOperation (
  Trav_Mgr_t *travMgr              /* Traversal Manager */,
  char *string                     /* String */,
  Pdtutil_MgrOp_t operationFlag    /* Operation Flag */,
  void **voidPointer               /* Generic Pointer */,
  Pdtutil_MgrRet_t *returnFlagP    /* Type of the Pointer Returned */
  )
{
  char *stringFirst, *stringSecond;
  Ddi_Mgr_t *defaultDdMgr, *tmpDdMgr;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (travMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return(1);
  }

  defaultDdMgr = Trav_MgrReadDdiMgrDefault (travMgr);

  /*----------------------- Take Main and Secondary Name --------------------*/

  Pdtutil_ReadSubstring (string, &stringFirst, &stringSecond);

  /*----------------------- Operation on the TRAV Manager -------------------*/

  if (stringFirst == NULL) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "TraversalManager verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Trav_MgrReadVerbosity (travMgr)),
          Trav_MgrReadVerbosity (travMgr));
        fprintf (stdout, "TraversalManager logPeriod %d\n",
          Trav_MgrReadLogPeriod (travMgr));
        fprintf (stdout, "TraversalManager savePeriod %d\n",
          Trav_MgrReadSavePeriod (travMgr));
        fprintf (stdout, "TraversalManager savePeriodName %s\n",
          Trav_MgrReadSavePeriodName (travMgr));
        fprintf (stdout, "TraversalManager backward %d\n",
          Trav_MgrReadBackward (travMgr));
        fprintf (stdout, "TraversalManager keepNew %d\n",
          Trav_MgrReadKeepNew (travMgr));
        fprintf (stdout, "TraversalManager maxIter %d\n",
          Trav_MgrReadMaxIter (travMgr));
        fprintf (stdout, "TraversalManager fromSelect %s (%d)\n",
          Trav_FromSelectEnum2String (Trav_MgrReadFromSelect (travMgr)),
          Trav_MgrReadFromSelect (travMgr));
        break;
      case Pdtutil_MgrOpStats_c:
        Trav_MgrPrintStats(travMgr);
        break;
      case Pdtutil_MgrOpMgrRead_c:
        *voidPointer = (void *) Trav_MgrReadDdiMgrDefault (travMgr);
        *returnFlagP = Pdtutil_MgrRetDdMgr_c;
        break;
      case Pdtutil_MgrOpMgrSet_c:
        tmpDdMgr = (Ddi_Mgr_t *) voidPointer;
        Trav_MgrSetDdiMgrDefault (travMgr, tmpDdMgr);
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }
    return (0);
  }

  /*----------------------------- Package is DDI ----------------------------*/

  if ( strcmp (stringFirst, "ddmr")==0 ) {
    Ddi_MgrOperation (&(travMgr->ddiMgrR), stringSecond, operationFlag,
      voidPointer, returnFlagP);

    if (*returnFlagP == Pdtutil_MgrOpMgrDelete_c) {
      travMgr->ddiMgrR = NULL;
    }
 
    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if ( strcmp (stringFirst, "ddmtr")==0 ) {
    Ddi_MgrOperation (&(travMgr->ddiMgrTr), stringSecond, operationFlag,
      voidPointer, returnFlagP);

    if (*returnFlagP == Pdtutil_MgrOpMgrDelete_c) {
      travMgr->ddiMgrTr = NULL;
    }
 
    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*-------------------------------- Options --------------------------------*/

  if (strcmp(stringFirst, "verbosity")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetVerbosity (travMgr,
          Pdtutil_VerbosityString2Enum (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "Verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Trav_MgrReadVerbosity (travMgr)),
          Trav_MgrReadVerbosity (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "currLevel")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        travMgr->level = atoi (*voidPointer);
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "currLevel %d\n", travMgr->level);
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "logPeriod")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetLogPeriod (travMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "LogPeriod %d\n", Trav_MgrReadLogPeriod (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "savePeriod")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetSavePeriod (travMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "SavePeriod %d\n", Trav_MgrReadSavePeriod (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "savePeriodName")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetSavePeriodName (travMgr, (char *) *voidPointer);
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "SavePeriod %s\n",
          Trav_MgrReadSavePeriodName (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "maxIter")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetMaxIter (travMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "MaxIter %d\n", Trav_MgrReadMaxIter (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "backward")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetBackward (travMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "Backward %d\n", Trav_MgrReadBackward (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "keepNew")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetKeepNew (travMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "KeepNew %d\n", Trav_MgrReadKeepNew (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "fromSelect")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetFromSelect (travMgr,
          Trav_FromSelectString2Enum (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "FromSelect %s (%d)\n",
          Trav_FromSelectEnum2String (Trav_MgrReadFromSelect (travMgr)),
          Trav_MgrReadFromSelect (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "trProfileDynamicEnable")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetTrProfileDynamicEnable (travMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, " %d\n",
          Trav_MgrReadTrProfileDynamicEnable (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "trProfileThreshold")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetTrProfileThreshold (travMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, " %d\n", Trav_MgrReadTrProfileThreshold (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "trProfileMethod")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Trav_MgrSetTrProfileMethod (travMgr,
          Ddi_ProfileHeuristicString2Enum (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, " %s (%d)\n",
          Ddi_ProfileHeuristicEnum2String (Trav_MgrReadTrProfileMethod
            (travMgr)),
          Trav_MgrReadTrProfileMethod (travMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*------------------------------ Reached Set ------------------------------*/

  if (strcmp(stringFirst, "reached")==0) {
    Ddi_BddOperation (defaultDdMgr, &(travMgr->reached), stringSecond,
      operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*-------------------------------- From Set -------------------------------*/

  if (strcmp(stringFirst, "from")==0) {
    Ddi_BddOperation (defaultDdMgr, &(travMgr->from), stringSecond,
      operationFlag, voidPointer,
      returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*------------------------------- Assertion -------------------------------*/

  if (strcmp (stringFirst, "assert") ==0 ) {
    Ddi_BddOperation (defaultDdMgr, &(travMgr->assert), stringSecond,
      operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*------------------------------- Frontier sets ---------------------------*/

  if (strcmp(stringFirst, "newi")==0) {
    Ddi_BddarrayOperation (defaultDdMgr, &(travMgr->newi), stringSecond,
      operationFlag, voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*------------------------------- No Match --------------------------------*/

  Pdtutil_Warning (1, "Operation on TRAV manager not allowed");
  return (1);
}


/**Function********************************************************************

  Synopsis    [Read transition relation]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Tr_Tr_t *
Trav_MgrReadTr(
  Trav_Mgr_t *travMgr            /* traversal manager */
  )
{
  return (travMgr->tr);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetTr (
  Trav_Mgr_t *travMgr       /* Traversal manager */,
  Tr_Tr_t   *tr            /* transition relation */
)
{
  if (travMgr->tr != NULL) {
    Tr_TrFree (travMgr->tr);
  }
  travMgr->tr = Tr_TrDup(tr);

  return;
}

/**Function********************************************************************

  Synopsis    [Read Number of Level]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadLevel (
  Trav_Mgr_t *travMgr            /* traversal manager */
  )
{
  return (travMgr->level);
}

/**Function********************************************************************

  Synopsis    [Read Product Peak Value]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadProductPeak (
  Trav_Mgr_t *travMgr    /* Traversal manager */
  )
{
  return (travMgr->productPeak);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetProductPeak (
  Trav_Mgr_t *travMgr       /* Traversal Manager */,
  int productPeak           /* Product Peak  */
)
{
  travMgr->productPeak = productPeak;

  return;
}

/**Function********************************************************************

  Synopsis    [Read PI array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t *
Trav_MgrReadI(
  Trav_Mgr_t *travMgr            /* tr manager */
)
{
  return (travMgr->i);
}

/**Function********************************************************************

  Synopsis    [Set the PI array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetI(
  Trav_Mgr_t *travMgr      /* Traversal manager */,
  Ddi_Vararray_t *i        /* Array of variables */
)
{
  travMgr->i = Ddi_VararrayCopy(travMgr->ddiMgrTr,i);
}

/**Function********************************************************************

  Synopsis    [Read PS array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t *
Trav_MgrReadPS(
  Trav_Mgr_t *travMgr    /* Traversal Manager */
)
{
  return (travMgr->ps);
}

/**Function********************************************************************

  Synopsis    [Set the PS array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetPS(
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  Ddi_Vararray_t *ps       /* Array of Variables */
)
{
  travMgr->ps = Ddi_VararrayCopy(travMgr->ddiMgrTr,ps);
}

/**Function********************************************************************

  Synopsis    [Read AUX array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t *
Trav_MgrReadAux (
  Trav_Mgr_t *travMgr    /* Traversal Manager */
  )
{
  return (travMgr->aux);
}

/**Function********************************************************************

  Synopsis    [Set the aux array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetAux(
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  Ddi_Vararray_t *aux      /* Array of Variables */
  )
{
  travMgr->aux = Ddi_VararrayCopy(travMgr->ddiMgrTr,aux);

  return;
}

/**Function********************************************************************

  Synopsis    [Set the flag to indicate that during traversal there is an
    additional auziliary manager]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetMgrAuxFlag (
  Trav_Mgr_t *travMgr            /* traversal manager */,
  int flag
)
{
  travMgr->settings.mgrAuxFlag = flag;

  return;
}

/**Function********************************************************************

  Synopsis    [Read NS array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t *
Trav_MgrReadNS(
  Trav_Mgr_t *travMgr            /* tr manager */
)
{
  return (travMgr->ns);
}

/**Function********************************************************************

  Synopsis    [Set the NS array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetNS(
  Trav_Mgr_t *travMgr     /* Traversal manager */,
  Ddi_Vararray_t *ns      /* Array of variables */
)
{
  travMgr->ns = Ddi_VararrayCopy(travMgr->ddiMgrTr,ns);
}

/**Function********************************************************************

  Synopsis    [Read reached]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t *
Trav_MgrReadReached(
  Trav_Mgr_t *travMgr   /* Traversal manager */
)
{
  return (travMgr->reached);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetReached (
  Trav_Mgr_t *travMgr       /* Traversal manager */,
  Ddi_Bdd_t *reached         /* Reached set */
)
{
  if (travMgr->reached != NULL)
    Ddi_Free(travMgr->reached);
  travMgr->reached = Ddi_BddCopy(travMgr->ddiMgrR,reached);

  return;
}

/**Function********************************************************************

  Synopsis    [Read Assert.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t *
Trav_MgrReadAssert (
  Trav_Mgr_t *travMgr    /* Traversal manager */
  )
{
  return (travMgr->assert);
}

/**Function********************************************************************

  Synopsis    [Set Assert.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetAssert (
  Trav_Mgr_t *travMgr       /* Traversal manager */,
  Ddi_Bdd_t *assert          /* Assertion */
  )
{
  if (travMgr->assert != NULL)
    Ddi_Free(travMgr->assert);
  travMgr->assert = Ddi_BddCopy(travMgr->ddiMgrR, assert);

  return;
}

/**Function********************************************************************

  Synopsis    [Read Assert Flag.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadAssertFlag (
  Trav_Mgr_t *travMgr    /* Traversal manager */
  )
{
  return (travMgr->assertFlag);
}

/**Function********************************************************************

  Synopsis    [Set Assert Flag.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetAssertFlag (
  Trav_Mgr_t *travMgr       /* Traversal manager */,
  int assertFlag            /* Assertion */
  )
{
  travMgr->assertFlag = assertFlag;

  return;
}

/**Function********************************************************************

  Synopsis    [Read newi]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Bddarray_t *
Trav_MgrReadNewi(
  Trav_Mgr_t *travMgr    /* Traversal manager */
)
{
  return (travMgr->newi);
}

/**Function********************************************************************

  Synopsis    [Set Newi]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetNewi (
  Trav_Mgr_t *travMgr       /* Traversal manager */,
  Ddi_Bddarray_t *newi          /* Frontier sets */
)
{
  if (travMgr->newi != NULL)
    Ddi_Free(travMgr->newi);
  travMgr->newi = Ddi_BddarrayCopy(travMgr->ddiMgrR,newi);

  return;
}

/**Function********************************************************************

  Synopsis    [Read from]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t *
Trav_MgrReadFrom(
  Trav_Mgr_t *travMgr   /* Traversal manager */
)
{
  return (travMgr->from);
}

/**Function********************************************************************

  Synopsis    [Read]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetFrom (
  Trav_Mgr_t *travMgr       /* Traversal manager */,
  Ddi_Bdd_t *from            /* From set */
)
{
  if (travMgr->from != NULL) {
    Ddi_Free(travMgr->from);
  }
  travMgr->from = Ddi_BddCopy(travMgr->ddiMgrR,from);

  return;
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

char *
Trav_MgrReadName (
  Trav_Mgr_t *travMgr       /* Traversal manager */
)
{
  return (travMgr->travName);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetName (
  Trav_Mgr_t *travMgr       /* Traversal manager */,
  char *travName
)
{
  travMgr->travName = Pdtutil_StrDup (travName);

  return;
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadSmoothVar(
  Trav_Mgr_t *travMgr       /* Traversal manager */
)
{
  return (travMgr->settings.smoothVars);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

double
Trav_MgrReadW1 (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.w1);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

double
Trav_MgrReadW2 (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.w2);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

double
Trav_MgrReadW3 (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.w3);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

double
Trav_MgrReadW4 (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.w4);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadEnableDdR(
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.enableDdR);
}


/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadTrProfileDynamicEnable (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.trProfileDynamicEnable);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetTrProfileDynamicEnable (
  Trav_Mgr_t *travMgr       /* Traversal Manager */,
  int enable
  )
{
  travMgr->settings.trProfileDynamicEnable = enable;

  return;
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadTrProfileThreshold (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.trProfileThreshold);
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetTrProfileThreshold (
  Trav_Mgr_t *travMgr       /* Traversal Manager */,
  int threshold
)
{
  travMgr->settings.trProfileThreshold = threshold;

  return;
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Cuplus_PruneHeuristic_e
Trav_MgrReadTrProfileMethod (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.trProfileMethod);
}

/**Function********************************************************************

  Synopsis    [Set]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetTrProfileMethod (
  Trav_Mgr_t *travMgr       /* Traversal Manager */,
  Cuplus_PruneHeuristic_e method
)
{
  travMgr->settings.trProfileMethod = method;

  return;
}

/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadThreshold (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.threshold);
}


/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadSquaring (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.squaring);
}


/**Function********************************************************************

  Synopsis    [Read verbosity]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Pdtutil_VerbLevel_e
Trav_MgrReadVerbosity(
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.verbosity);
}

/**Function********************************************************************

  Synopsis    [Read verbosity]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetSorting (
  Trav_Mgr_t *travMgr       /* Traversal Manager */,
  int sorting
)
{
  travMgr->settings.sorting = sorting;

  return;
}


/**Function********************************************************************

  Synopsis    [Read verbosity]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadSorting (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.sorting);
}

/**Function********************************************************************

  Synopsis    [Read the backward traversal flag]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadBackward(
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.backward);
}

/**Function********************************************************************

  Synopsis    [Read the keepNew  flag]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadKeepNew(
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.keepNew);
}



/**Function********************************************************************

  Synopsis    [Read the maximum number of traversal iterations]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadMaxIter(
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.maxIter);
}


/**Function********************************************************************

  Synopsis    [Read the period for verbosity enabling]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadLogPeriod(
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.logPeriod);
}



/**Function********************************************************************

  Synopsis    [Read the period for save BDDs enabling]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Trav_MgrReadSavePeriod (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.savePeriod);
}



/**Function********************************************************************

  Synopsis    [Read the period for save BDDs enabling]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

char *
Trav_MgrReadSavePeriodName (
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.savePeriodName);
}



/**Function********************************************************************

  Synopsis    [Read from selection]

  SideEffects [none]

  Description []

  SeeAlso     []

******************************************************************************/

Trav_FromSelect_e
Trav_MgrReadFromSelect(
  Trav_Mgr_t *travMgr       /* Traversal Manager */
)
{
  return (travMgr->settings.fromSelect);
}


/**Function********************************************************************

  Synopsis    [Set DDi Mgr TR]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetDdiMgrTr (
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  Ddi_Mgr_t *mgrTr         /* dd Manager */
)
{
  travMgr->ddiMgrTr = mgrTr;

  return;
}

/**Function********************************************************************

  Synopsis    [Set default DDi Mgr]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetDdiMgrDefault (
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  Ddi_Mgr_t *mgrTr         /* dd Manager */
)
{
  travMgr->ddiMgrTr = mgrTr;

  return;
}

/**Function********************************************************************

  Synopsis    [Set DDi Mgr R]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetDdiMgrR (
  Trav_Mgr_t *travMgr    /* Traversal Manager */,
  Ddi_Mgr_t *mgrR        /* Decision Diagram Manager */
)
{
  travMgr->ddiMgrR = mgrR;

  return;
}

/**Function********************************************************************

  Synopsis    [Read default DDi Mgr]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Mgr_t *
Trav_MgrReadDdiMgrDefault (
  Trav_Mgr_t *travMgr      /* Traversal Manager */
)
{
  return (travMgr->ddiMgrTr);
}


/**Function********************************************************************

  Synopsis    [Read DDi Mgr TR]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Mgr_t *
Trav_MgrReadDdiMgrTr (
  Trav_Mgr_t *travMgr      /* Traversal Manager */
)
{
  return (travMgr->ddiMgrTr);
}

/**Function********************************************************************

  Synopsis    [Read DDi Mgr R]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Mgr_t *
Trav_MgrReadDdiMgrR (
  Trav_Mgr_t *travMgr      /* Traversal Manager */
)
{
  return (travMgr->ddiMgrR);
}

/**Function********************************************************************

  Synopsis    [Set verbosity]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetVerbosity(
  Trav_Mgr_t *travMgr             /* Traversal Manager */,
  Pdtutil_VerbLevel_e verbosity   /* Verbosity Level */
)
{
  travMgr->settings.verbosity = verbosity;

  return;
}

/**Function********************************************************************

  Synopsis    [Set the backward traversal flag]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetBackward(
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  int backward             /* Max iterations */
)
{
  travMgr->settings.backward = backward;
}

/**Function********************************************************************

  Synopsis    [Set the keepNew flag]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetKeepNew(
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  int keepNew              /* Max iterations */
)
{
  travMgr->settings.keepNew = keepNew;
}


/**Function********************************************************************

  Synopsis    [Set the maximum number of traversal iterations]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetMaxIter(
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  int maxIter              /* Max iterations */
)
{
  travMgr->settings.maxIter = maxIter;
}


/**Function********************************************************************

  Synopsis    [Set the period for verbosity enabling]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetLogPeriod(
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  int logPeriod            /* Period */
)
{
  travMgr->settings.logPeriod = logPeriod;
}


/**Function********************************************************************

  Synopsis    [Set the period for verbosity enabling]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetSavePeriod(
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  int savePeriod           /* Period */
)
{
  travMgr->settings.savePeriod = savePeriod;

  return;
}



/**Function********************************************************************

  Synopsis    [Set the period for verbosity enabling]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetSavePeriodName (
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  char *savePeriodName     /* Period File Name*/
)
{
  travMgr->settings.savePeriodName = Pdtutil_StrDup (savePeriodName);

  return;
}


/**Function********************************************************************

  Synopsis    [Set the from selection]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Trav_MgrSetFromSelect(
  Trav_Mgr_t *travMgr             /* Traversal Manager */,
  Trav_FromSelect_e fromSelect    /* Selection */
)
{
  if (fromSelect != Trav_FromSelectSame_c) {
    travMgr->settings.fromSelect = fromSelect;
  }

  return;
}

/**Function********************************************************************

  Synopsis    [Read the flag to indicate that during traversal there is an
    additional auziliary manager]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int 
Trav_MgrReadMgrAuxFlag (
  Trav_Mgr_t *travMgr      /* Traversal Manager */
  )
{
  return (travMgr->settings.mgrAuxFlag);
}




