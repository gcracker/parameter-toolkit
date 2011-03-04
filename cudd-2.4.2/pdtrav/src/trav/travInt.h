/**CHeaderFile*****************************************************************

  FileName    [travInt.h]

  PackageName [trav]

  Synopsis    [Definition of the struct of FSM and struct for options]

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

#ifndef _TRAVINT
#define _TRAVINT

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "dddmp.h"
#include "ddi.h"
#include "tr.h"
#include "part.h"
#include "trav.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************
 Synopsis    [Traversal Manager]
 Description [The Traversal manager structure]
******************************************************************************/

struct Trav_Mgr_s {
  char *travName;

  Ddi_Mgr_t         *ddiMgrTr;        /* TR manager */
  Ddi_Mgr_t         *ddiMgrR;         /* Reached states manager */
  Ddi_Mgr_t         *ddiMgrAux;       /* auxiliary manager */
  Tr_Tr_t           *tr;              /* transition relation */
  Ddi_Vararray_t    *i;               /* primary inputs */
  Ddi_Vararray_t    *ps;              /* present state variables */
  Ddi_Vararray_t    *ns;              /* next state variables */
  Ddi_Vararray_t    *aux;             /* auxiliary state variables */
  Ddi_Bdd_t          *from;            /* from set */
  Ddi_Bdd_t          *reached;         /* reached set */
  Ddi_Bdd_t          *assert;          /* assertion for reached */
  Ddi_Bddarray_t     *newi;            /* frontier sets */
  int               level;            /* traversal level */ 
  int               productPeak;      /* Product Peak value */
  int               assertFlag;       /* 1 if Assertion Failed */
  long travTime;                      /* Total Traversal Time */

  struct {
    Pdtutil_VerbLevel_e verbosity;
    int logPeriod;                  /* Period for verbosity activation */
    int savePeriod;                 /* Period for saving intermediate BDDs */
    char *savePeriodName;           /* Period Name (see savePeriod) */
    int backward;                   /* backward traversal flag */
    int keepNew;                    /* enable keeping frontier sets */
    int maxIter;                    /* max allowed traversal iterations */
    int mgrAuxFlag;                 /* Flag to indicate if to use or not
                                       the auxiliary manager during
                                       traversal */
    Trav_FromSelect_e fromSelect;   /* from selection */

    int enableDdR;     /* 1 if second dd manager for reached enabled */

   /* Parameter for Dynamic Profiling */
    int trProfileDynamicEnable,   /* 1 if Dunamic TR enabled */
        trProfileThreshold;       /* -1 if TR pruning disabled */
    Cuplus_PruneHeuristic_e trProfileMethod;

    /* ----- */
    int removeLLatches,  /* if 1 do lambda latches removal */
        partDisjTrThresh, /* 1= disjoint TR partitioning threshold */
        squaring,      /* 1 enable squaring */
        smoothVars,   /* select variables for smoothing within
                    img/closure */
        Iwls95Variant,  /* variant of IWLS95 image/traversal method */
        sorting;       /* method of sorting ( 0 = no sorting ) */

    int threshold;       /* threshold (size of BDD) for build clusters */

    /* weight for Weighted sorting method */
    double w1,
           w2,
           w3,
           w4;
  
  } settings;
};	

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void simulateSimple(Fsm_Mgr_t *fsmMgr, int iterNumberMax, int logPeriod, char *pattern, char *result);
EXTERN void simulateWave(Fsm_Mgr_t *fsmMgr, int iterNumberMax, int logPeriod, int simulationFlag, char *init, char *pattern, char *result);
EXTERN void simulateWithDac99(Fsm_Mgr_t *fsmMgr, int iterNumberMax, int deadEndMaxNumberOf, int logPeriod, int depthBreadth, int random, char *pattern, char *result);
EXTERN Ddi_Bdd_t * TravFromCompute(Ddi_Bdd_t *to, Ddi_Bdd_t *reached, int option);

/**AutomaticEnd***************************************************************/


#endif /* _TRAVINT */
