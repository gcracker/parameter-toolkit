/**CFile***********************************************************************

  FileName    [trMgr.c]

  PackageName [tr]

  Synopsis    [Functions to handle a TR manager]

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

static void SetSortWeights(Tr_Mgr_t *trMgr, char *weights);
static char * ShowSortWeights(Tr_Mgr_t *trMgr);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates a DdManager.]

  Description []

  SideEffects [none]

  SeeAlso     [Ddi_MgrQuit, Fsm_MgrQuit, Trav_MgrQuit]

******************************************************************************/

Tr_Mgr_t *
Tr_MgrInit (
  char *trName    /* Name of the FSM structure */,
  Ddi_Mgr_t *dd   /* Decision Diagram Manager */
)
{
  Tr_Mgr_t *trMgr;

  trMgr = Pdtutil_Alloc(Tr_Mgr_t, 1);
  if (trMgr == NULL) {
    Pdtutil_Warning (1, "Out of memory.");
    return (NULL);
  }

  /*------------------- Set the Name of the FSM Structure ------------------*/

  if (trName != NULL) {
     Tr_MgrSetTrName (trMgr, trName);
  } else {
     Tr_MgrSetTrName (trMgr, "tr");
  }

#if 0 
  /* not yet supported Ddi_VararrayReadtrMgr */
  Pdtutil_Assert(Ddi_BddReadtrMgr(tr)==Ddi_VararrayReadtrMgr(i),
    "Different DD Managers in use");
  Pdtutil_Assert(Ddi_BddReadtrMgr(tr)==Ddi_VararrayReadtrMgr(ns), 
    "Different DD Managers in use");
  Pdtutil_Assert(Ddi_BddReadtrMgr(tr)==Ddi_VararrayReadtrMgr(ps), 
    "Different DD Managers in use");
#endif

  trMgr->DdiMgr = dd;
  trMgr->tr = NULL;
  trMgr->i  = NULL;
  trMgr->ns = NULL;
  trMgr->ps = NULL;
  trMgr->auxVars = NULL;
  trMgr->auxFuns = NULL;
  trMgr->auxRel  = NULL;

  trMgr->trList  = NULL;

  trMgr->settings.verbosity = Pdtutil_VerbLevelNone_c;
  trMgr->settings.cluster.threshold = TR_IMAGE_CLUSTER_SIZE;
  trMgr->settings.cluster.smoothPi = 1;
  trMgr->settings.sort.method = Tr_SortNone_c;
  trMgr->settings.sort.W[0] = TRSORT_DEFAULT_W1;
  trMgr->settings.sort.W[1] = TRSORT_DEFAULT_W2;
  trMgr->settings.sort.W[2] = TRSORT_DEFAULT_W3;
  trMgr->settings.sort.W[3] = TRSORT_DEFAULT_W4;
  trMgr->settings.sort.W[4] = TRSORT_DEFAULT_W5;
  trMgr->settings.closure.maxIter = -1;
  trMgr->settings.closure.method = TR_SQUARING_FULL;
  trMgr->settings.image.method = Tr_ImgMethodIwls95_c;
  trMgr->settings.image.partThFrom = -1;
  trMgr->settings.image.partThTr = -1;
  trMgr->settings.image.partitionMethod = Part_MethodNone_c;
  trMgr->settings.image.smoothPi = 1;
  
  trMgr->trTime = 0;

  return (trMgr);
}

/**Function********************************************************************

  Synopsis     [Closes a Transition Relation Manager.]

  Description  [Closes a Transition Relation Manager freeing all the
    correlated fields.]

  SideEffects  [none]

  SeeAlso      [Ddi_BddiMgrInit]

******************************************************************************/

void
Tr_MgrQuit(
  Tr_Mgr_t *trMgr  /* Tr manager */
)
{
  Tr_TrFree (trMgr->tr);
  Ddi_Free (trMgr->i);
  Ddi_Free (trMgr->ps);
  Ddi_Free (trMgr->ns);
  Ddi_Free (trMgr->auxVars);
  Ddi_Free (trMgr->auxFuns);
  Ddi_Free (trMgr->auxRel);

  Pdtutil_Free (trMgr);
}

/**Function********************************************************************

  Synopsis    [Prints Statistics on a Transition Relation Manager.]

  Description [Prints Statistics on a Transition Relation Manager on
    standard output.]

  SideEffects []

  SeeAlso     []

******************************************************************************/

int
Tr_MgrPrintStats (
  Tr_Mgr_t *trMgr  /* Tr manager */
  )
{ 
  fprintf (stdout, "Transition Relation Manager Name %s\n",
    trMgr->trName);
  fprintf (stdout, "Total CPU Time: %s\n",
    util_print_time (trMgr->trTime));
 
  return (1);
}

/**Function********************************************************************

  Synopsis    [Performs an Operation on a Transition Relation Manager.]

  Description [Performs an Operation on a Transition Relation Manager.
    The allowed operations are specified by the enumerated type
    Pdtutil_MgrOp_t. Returns the result of the operation, the enumerated
    type Pdtutil_MgrRet_t.]

  SideEffects []

  SeeAlso     [CmdMgrOperation, CmdRegOperation, Fsm_MgrOperation,
    Trav_MgrOperation]

******************************************************************************/

int
Tr_MgrOperation (
  Tr_Mgr_t *trMgr                 /* TR Manager */,
  char *string                    /* String */,
  Pdtutil_MgrOp_t operationFlag   /* Operation Flag */,
  void **voidPointer              /* Generic Pointer */,
  Pdtutil_MgrRet_t *returnFlagP   /* Type of the Pointer Returned */
  )
{
  char *stringFirst, *stringSecond;
  Ddi_Mgr_t *defaultDdMgr, *tmpDdMgr;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (trMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return (1);
  }

  defaultDdMgr = Tr_MgrReadDdiMgrDefault (trMgr);
  
  /*----------------------- Take Main and Secondary Name --------------------*/
 
  Pdtutil_ReadSubstring (string, &stringFirst, &stringSecond);

  /*----------------------- Operation on the FSM Manager --------------------*/

  if (stringFirst == NULL) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "TransitionRelationManager verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Tr_MgrReadVerbosity (trMgr)),
          Tr_MgrReadVerbosity (trMgr));
        fprintf (stdout,
          "TransitionRelationManager sortWeights %s\n",
          ShowSortWeights (trMgr)); 
        fprintf (stdout, "TransitionRelationManager clusterThreshold %d\n",
          Tr_MgrReadClustThreshold (trMgr));
        fprintf (stdout, "TransitionRelationManager imgMethod %s (%d)\n",
          Tr_ImgMethodEnum2String (Tr_MgrReadImgMethod (trMgr)),
          Tr_MgrReadImgMethod (trMgr));
        fprintf (stdout, "TransitionRelationManager partThFrom %d\n",
          Tr_MgrReadPartThFrom (trMgr));
        fprintf (stdout, "TransitionRelationManager partThTR %d\n",
          Tr_MgrReadPartThTr (trMgr));
        break;
      case Pdtutil_MgrOpStats_c:
        Tr_MgrPrintStats (trMgr);
        break;
      case Pdtutil_MgrOpMgrRead_c:
        *voidPointer = (void *) Tr_MgrReadDdiMgrDefault (trMgr);
        *returnFlagP= Pdtutil_MgrRetDdMgr_c;
        break;
      case Pdtutil_MgrOpMgrSet_c:
        tmpDdMgr = (Ddi_Mgr_t *) *voidPointer;
        Tr_MgrSetDdiMgrDefault (trMgr, tmpDdMgr);
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TR Manager");
        break;
    }

    return (0);
  }

  if (strcmp(stringFirst, "imgMethod")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Tr_MgrSetImgMethod (trMgr,
          Tr_ImgMethodString2Enum (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "ImgMethod %s (%d)\n",
          Tr_ImgMethodEnum2String (Tr_MgrReadImgMethod (trMgr)),
          Tr_MgrReadImgMethod (trMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TR Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "partThFrom")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Tr_MgrSetPartThFrom (trMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "PartThFrom %d\n", Tr_MgrReadPartThFrom (trMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TR Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "partThTr")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Tr_MgrSetPartThTr (trMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "PartThTR %d\n", Tr_MgrReadPartThTr (trMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TR Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }


  /*----------------------------- Package is DDI ----------------------------*/

  if (strcmp (stringFirst, "ddm")==0) {
    Ddi_MgrOperation (&(trMgr->DdiMgr), stringSecond, operationFlag,
      voidPointer, returnFlagP);

    if (*returnFlagP == Pdtutil_MgrOpMgrDelete_c) {
      trMgr->DdiMgr = NULL;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*--------------------------------- Options -------------------------------*/

  if (strcmp(stringFirst, "verbosity")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Tr_MgrSetVerbosity (trMgr,
          Pdtutil_VerbosityString2Enum (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "Verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Tr_MgrReadVerbosity (trMgr)),
          Tr_MgrReadVerbosity (trMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TR Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "sortWeights")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
	SetSortWeights (trMgr, (char *) *voidPointer); 
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "SortWeights %s\n", ShowSortWeights (trMgr)); 
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TR Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  if (strcmp(stringFirst, "clustTh")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Tr_MgrSetClustThreshold (trMgr, atoi (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "ClusterThreshold %d\n",
          Tr_MgrReadClustThreshold (trMgr));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TR Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*-------------------------- Transition Relation --------------------------*/

  if (strcmp(stringFirst, "tr")==0) {
    Pdtutil_Warning (1, "Bdd operation not allowed on tr");
#if 0
    Ddi_BddOperation (defaultDdMgr, &(Ddi_ExprToBdd(trMgr->tr->ddiTr)), 
      stringSecond, operationFlag, voidPointer, returnFlagP);
#endif
    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*------------------------------- No Match --------------------------------*/

  Pdtutil_Warning (1, "Operation on TR manager not allowed");
  return (1);
}

/**Function********************************************************************

  Synopsis    [Set default DDi Mgr on the Transition Relation Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetDdiMgrDefault (
  Tr_Mgr_t *trMgr      /* trersal manager */,
  Ddi_Mgr_t *mgr         /* dd Manager */
)
{
  trMgr->DdiMgr = mgr;
}


/**Function*******************************************************************

  Synopsis    [Set sort settings in the Transition Relation Manager.]

  Description [Set sort settings in the Transition Relation Manager
    reading them from a string.]

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

static void
SetSortWeights(
  Tr_Mgr_t *trMgr            /* Traversal Manager */,
  char *weights              /* Weights */
  )
{
  float w[5];
  int i, n;

  n = sscanf (weights, "%f/%f/%f/%f/%f", &w[0],&w[1],&w[2],&w[3],&w[4]);
  
  if (n < 5) {
    Pdtutil_Warning (1, "Error reading weights; Format: w1/w2/w3/w4/w5.");
  }
  else {
    for (i=0; i<5; i++) {
      Tr_MgrSetSortW (trMgr, i+1, (double)w[i]);
    }
  }
}
  
/**Function*******************************************************************

  Synopsis    [Show sort settings.]

  Description [Show sort settings reading them from the Transition
    Relation manager. Return a string containing them.]

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

static char *
ShowSortWeights(
  Tr_Mgr_t *trMgr            /* Traversal Manager */
  )
{
  static char buf[200];
  double w[5];
  int i;

  for (i=0; i<5; i++) {
    w[i] = Tr_MgrReadSortW(trMgr,i+1);
  }
  sprintf (buf, "%g/%g/%g/%g/%g", w[0],w[1],w[2],w[3],w[4]);

  return (buf);
}

/**Function********************************************************************

  Synopsis     [Read default DDi Mgr]

  Description  []

  SideEffects  [none]

  SeeAlso      []

******************************************************************************/

Ddi_Mgr_t *
Tr_MgrReadDdiMgrDefault (
  Tr_Mgr_t *trMgr      /* trersal manager */
)
{
  return (trMgr->DdiMgr);
}

/**Function********************************************************************

  Synopsis     [Read PI array]

  Description  []

  SideEffects  [none]

  SeeAlso      []

******************************************************************************/

Ddi_Vararray_t *
Tr_MgrReadI(
  Tr_Mgr_t *trMgr            /* tr manager */
)
{
  return (trMgr->i);
}

/**Function********************************************************************

  Synopsis    [Set the PI array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetI(
  Tr_Mgr_t *trMgr       /* Transition Relation Manager */,
  Ddi_Vararray_t *i     /* Array of variables */
)
{
  Ddi_Free(trMgr->i);
  trMgr->i = Ddi_VararrayCopy(trMgr->DdiMgr,i);
}

/**Function********************************************************************

  Synopsis    [Read PS array in the Transition Relation Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t *
Tr_MgrReadPS(
  Tr_Mgr_t *trMgr            /* tr manager */
)
{
  return (trMgr->ps);
}

/**Function********************************************************************

  Synopsis    [Set the PS array]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetPS(
  Tr_Mgr_t *trMgr        /* Transition Relation Manager */,
  Ddi_Vararray_t *ps     /* Array of Variables */
)
{
  Ddi_Free(trMgr->ps);
  trMgr->ps = Ddi_VararrayCopy(trMgr->DdiMgr,ps);
}

/**Function********************************************************************

  Synopsis    [Read NS array in the Transition Relation Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Ddi_Vararray_t *
Tr_MgrReadNS(
  Tr_Mgr_t *trMgr    /* tr manager */
)
{
  return (trMgr->ns);
}

/**Function********************************************************************

  Synopsis    [Set the NS array in the Transition Relation Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetNS(
  Tr_Mgr_t *trMgr      /* Transition Relation Manager */,
  Ddi_Vararray_t *ns   /* Array of variables */
)
{
  Ddi_Free(trMgr->ns);
  trMgr->ns = Ddi_VararrayCopy(trMgr->DdiMgr,ns);
}

/**Function********************************************************************

  Synopsis    [Set the auxVars array in the Transition Relation Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetAuxVars(
  Tr_Mgr_t *trMgr      /* Transition Relation Manager */,
  Ddi_Vararray_t *auxVars   /* Array of variables */
)
{
  Ddi_Free(trMgr->auxVars);
  trMgr->auxVars = Ddi_VararrayCopy(trMgr->DdiMgr,auxVars);
}

/**Function********************************************************************

  Synopsis    [Set the auxVars array in the Transition Relation Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetAuxFuns(
  Tr_Mgr_t *trMgr      /* Transition Relation Manager */,
  Ddi_Bddarray_t *auxFuns   /* Array of functions */
)
{
  Ddi_Free(trMgr->auxFuns);
  trMgr->auxFuns = Ddi_BddarrayCopy(trMgr->DdiMgr,auxFuns);
}

/**Function********************************************************************

  Synopsis    [Read TR in the Transition Relation Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Tr_Tr_t *
Tr_MgrReadTr(
  Tr_Mgr_t *trMgr   /* Transition Relation Manager */
)
{
  return (trMgr->tr);
}

/**Function********************************************************************

  Synopsis    [Set the Transition Relation in the Transition Relation
    Manager.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetTr(
  Tr_Mgr_t *trMgr    /* Transition Relation Manager */,
  Tr_Tr_t *tr       /* Transition Relation */
)
{
  Tr_TrFree(trMgr->tr);
  trMgr->tr = Tr_TrDup(tr);
}


/**Function********************************************************************

  Synopsis    [Read verbosity]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Pdtutil_VerbLevel_e
Tr_MgrReadVerbosity(
  Tr_Mgr_t *trMgr            /* Transition Relation Manager */
)
{
  return (trMgr->settings.verbosity);
}

/**Function********************************************************************

  Synopsis    [Set the period for verbosity enabling.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetVerbosity(
  Tr_Mgr_t *trMgr                 /* Transition Relation Manager */,
  Pdtutil_VerbLevel_e verbosity   /* Verbosity */
)
{
  trMgr->settings.verbosity = verbosity;
}

/**Function********************************************************************

  Synopsis    [Read sort method]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Tr_Sort_e
Tr_MgrReadSortMethod(
  Tr_Mgr_t *trMgr     /* Transition Relation Manager */
)
{
  return (trMgr->settings.sort.method);
}

/**Function********************************************************************

  Synopsis    [Set the sort Method]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetSortMethod(
  Tr_Mgr_t *trMgr            /* Transition Relation Manager */,
  Tr_Sort_e sortMethod       /* Method */
)
{
  trMgr->settings.sort.method = sortMethod;
}

/**Function********************************************************************

  Synopsis    [Read cluster threshold]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Tr_MgrReadClustThreshold(
  Tr_Mgr_t *trMgr      /* Transition Relation Manager */
)
{
  return (trMgr->settings.cluster.threshold);
}

/**Function********************************************************************
  Synopsis    [Set the cluster threshold]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
Tr_MgrSetClustThreshold(
  Tr_Mgr_t *trMgr          /* Transition Relation Manager */,
  int ClustThreshold       /* Threshold */
)
{
  trMgr->settings.cluster.threshold = ClustThreshold;
}

/**Function********************************************************************
  Synopsis    [Set the cluster smoothPi flag]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
Tr_MgrSetClustSmoothPi(
  Tr_Mgr_t *trMgr          /* Transition Relation Manager */,
  int val
)
{
  trMgr->settings.cluster.smoothPi = val;
}

/**Function********************************************************************
  Synopsis    [Set the image smoothPi flag]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
Tr_MgrSetImgSmoothPi(
  Tr_Mgr_t *trMgr          /* Transition Relation Manager */,
  int val
)
{
  trMgr->settings.image.smoothPi = val;
}

/**Function********************************************************************

  Synopsis    [Read sort weight]

  SideEffects [none]

  Description []

  SeeAlso     []

******************************************************************************/

double
Tr_MgrReadSortW(
  Tr_Mgr_t *trMgr            /* Transition Relation Manager */,
  int       i
)
{
  Pdtutil_Assert(((i<=5)&&(i>0)),"Weight index out of bounds: 1..5");
  return (trMgr->settings.sort.W[i-1]);
}

/**Function********************************************************************

  Synopsis    [Set the sort weight]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetSortW(
  Tr_Mgr_t *trMgr    /* Transition Relation Manager */,
  int i,
  double SortW       /* Weight */
)
{
  Pdtutil_Assert(((i<=5)&&(i>0)),"Weight index out of bounds: 1..5");
  trMgr->settings.sort.W[i-1] = SortW;
}


/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetTrName (
  Tr_Mgr_t *trMgr     /* Transition Relation Manager */,
  char *trName        /* Traversal Manager Name */
)
{
  trMgr->trName = Pdtutil_StrDup (trName);
}

/**Function********************************************************************

  Synopsis    [Read verbosity]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

char *
Tr_MgrReadTrName (
  Tr_Mgr_t *trMgr    /* Transition Relation Manager */
)
{
  return (trMgr->trName);
}

/**Function********************************************************************

  Synopsis    [Read the maximum number of closure iterations.]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Tr_MgrReadMaxIter(
  Tr_Mgr_t *trMgr    /* Transition Relation Manager */
)
{
  return (trMgr->settings.closure.maxIter);
}


/**Function********************************************************************

  Synopsis    [Set the maximum number of closure iterations]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetMaxIter(
  Tr_Mgr_t *trMgr    /* Transition Relation Manager */,
  int maxIter        /* max iterations */
)
{
  trMgr->settings.closure.maxIter = maxIter;
}


/**Function********************************************************************

  Synopsis    [Read the squaring method]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Tr_MgrReadSquaringMethod(
  Tr_Mgr_t *trMgr    /* Transition Relation Manager */
)
{
  return (trMgr->settings.closure.method);
}


/**Function********************************************************************

  Synopsis    [Set the squaring method]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetSquaringMethod(
  Tr_Mgr_t *trMgr    /* Transition Relation Manager */,
  int method         /* Method */
)
{
  trMgr->settings.closure.method = method;
}


/**Function********************************************************************

  Synopsis    [Read the image method selection]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Tr_ImgMethod_e
Tr_MgrReadImgMethod(
  Tr_Mgr_t *trMgr       /* Tr Manager */
)
{
  return (trMgr->settings.image.method);
}


/**Function********************************************************************

  Synopsis    [Set the image method selection]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetImgMethod(
  Tr_Mgr_t *trMgr           /* Tr Manager */,
  Tr_ImgMethod_e imgMethod    /* Image Method */
)
{
  trMgr->settings.image.method = imgMethod;
  return;
}


/**Function********************************************************************

  Synopsis    [Read partitioning threshold for from]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Tr_MgrReadPartThFrom(
  Tr_Mgr_t *trMgr       /* Tr Manager */
)
{
  return (trMgr->settings.image.partThFrom);
}


/**Function********************************************************************

  Synopsis    [Read partitioning threshold for TR]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

int
Tr_MgrReadPartThTr(
  Tr_Mgr_t *trMgr         /* Tr Manager */
)
{
  return (trMgr->settings.image.partThTr);
}

/**Function********************************************************************

  Synopsis    [Set partitioning threshold for from]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetPartThFrom(
  Tr_Mgr_t *trMgr       /* Tr Manager */,
  int       threshold       /* Threshold value (-1 for no threshold) */
)
{
  trMgr->settings.image.partThFrom = threshold;
  return;
}

/**Function********************************************************************

  Synopsis    [Set partitioning threshold for TR]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetPartThTr(
  Tr_Mgr_t *trMgr     /* Tr Manager */,
  int threshold           /* Threshold value (-1 for no threshold) */
)
{
  trMgr->settings.image.partThTr = threshold;
  return;
}



/**Function********************************************************************

  Synopsis    [Read ]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

Part_Method_e
Tr_MgrReadPartitionMethod (
  Tr_Mgr_t *trMgr       /* Tr Manager */
)
{
  return (trMgr->settings.image.partitionMethod);
}

/**Function********************************************************************

  Synopsis    [Set]

  Description []

  SideEffects [none]

  SeeAlso     []

******************************************************************************/

void
Tr_MgrSetPartitionMethod (
  Tr_Mgr_t *trMgr                  /* Tr Manager */,
  Part_Method_e partitionMethod    /* Partition Method */
)
{
  trMgr->settings.image.partitionMethod = partitionMethod;
}










