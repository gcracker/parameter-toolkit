/**CHeaderFile*****************************************************************

  FileName    [trav.h]

  PackageName [trav]

  Synopsis    [Main module for a simple traversal of finite state machine]

  Description [This package contains the main function to exploring the state
    space of a FSM.<br>
    There are three methods for image computation:
    <ol>
    <li> <b>Monolithic:</b> This is the most naive approach possible.<br>
       This technique is based on building <em>monolithic transition 
       relation</em>. It is the conjunction of all latches transition
       relation. If we use <em>y</em> to denote the next state vector,
       <em>s</em> the present state vector, <em>x</em> the input 
       vector and <em>delta()</em> the next state function, we define the
       trasition relation of <em>i</em>-th latch to be the function 
       Ti (x,s,y) = yi <=> delta(i)(x,s).<br> Then, for a FSM of n latches,
       the monolhitic transition relation is:
       <p>
       T(x,s,y) = T1(x,s,y)*T2(x,s,y)* ... *Tn(x,s,y)
       <p>
       When the monolithic TR is built, the traversal algorithm is executed.
       This method is suitable for circuits with less than 20 latches

    <li> <b>IWLS95:</b> This technique is based on the early quantification
    heuristic.
       The initialization process consists of following steps:   
       <ul>
       <li> Create the clustered TR from transition relation by clustering 
            several function together. The size of clustering is controlled
            by a threshold value controlled by the user.
       <li> Order the clusters using the algorithm given in
            "Efficient BDD Algorithms for FSM Synthesis and
            Verification", by R. K. Ranjan et. al. in the proceedings of
            IWLS'95. 
       <li> For each cluster, quantify out the quantify variables which
            are local to that particular cluster.
       </ul>
    <li> <b>Iterative squaring:</b> This technique is based on building the
       <em>transitive closure (TC)</em> of a monolithic TR. Afterwards, TC
       replace TR in the traversal algorithm. 
    </ol>
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

#ifndef _TRAV
#define _TRAV

#include "pdtutil.h"
#include "ddi.h"
#include "part.h"
#include "fsm.h"

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/* smoothed variables */
#define TRAV_SMOOTH_SX     0
#define TRAV_SMOOTH_S      1

/* buffer size */
#define TRAV_FILENAME_BUFSIZE 80


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************

  Synopsis    [Type for From Selection]

  Description []

******************************************************************************/

typedef enum
  {
  Trav_FromSelectNew_c,
  Trav_FromSelectReached_c,
  Trav_FromSelectTo_c,
  Trav_FromSelectCofactor_c,
  Trav_FromSelectRestrict_c,
  Trav_FromSelectBest_c,
  Trav_FromSelectSame_c
  }
Trav_FromSelect_e;


/**Enum************************************************************************

  Synopsis    []

  Description []

******************************************************************************/

typedef struct Trav_Mgr_s Trav_Mgr_t;

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

EXTERN void Trav_Main(Trav_Mgr_t *travMgr);
EXTERN Trav_Mgr_t * Trav_MgrInit(char *travName, Ddi_Mgr_t *dd);
EXTERN void Trav_MgrQuit(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrPrintStats(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrOperation(Trav_Mgr_t *travMgr, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlagP);
EXTERN Tr_Tr_t * Trav_MgrReadTr(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetTr(Trav_Mgr_t *travMgr, Tr_Tr_t *tr);
EXTERN int Trav_MgrReadLevel(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadProductPeak(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetProductPeak(Trav_Mgr_t *travMgr, int productPeak);
EXTERN Ddi_Vararray_t * Trav_MgrReadI(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetI(Trav_Mgr_t *travMgr, Ddi_Vararray_t *i);
EXTERN Ddi_Vararray_t * Trav_MgrReadPS(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetPS(Trav_Mgr_t *travMgr, Ddi_Vararray_t *ps);
EXTERN Ddi_Vararray_t * Trav_MgrReadAux(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetAux(Trav_Mgr_t *travMgr, Ddi_Vararray_t *aux);
EXTERN void Trav_MgrSetMgrAuxFlag(Trav_Mgr_t *travMgr, int flag);
EXTERN Ddi_Vararray_t * Trav_MgrReadNS(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetNS(Trav_Mgr_t *travMgr, Ddi_Vararray_t *ns);
EXTERN Ddi_Bdd_t * Trav_MgrReadReached(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetReached(Trav_Mgr_t *travMgr, Ddi_Bdd_t *reached);
EXTERN Ddi_Bdd_t * Trav_MgrReadAssert(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetAssert(Trav_Mgr_t *travMgr, Ddi_Bdd_t *assert);
EXTERN int Trav_MgrReadAssertFlag(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetAssertFlag(Trav_Mgr_t *travMgr, int assertFlag);
EXTERN Ddi_Bddarray_t * Trav_MgrReadNewi(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetNewi(Trav_Mgr_t *travMgr, Ddi_Bddarray_t *newi);
EXTERN Ddi_Bdd_t * Trav_MgrReadFrom(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetFrom(Trav_Mgr_t *travMgr, Ddi_Bdd_t *from);
EXTERN char * Trav_MgrReadName(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetName(Trav_Mgr_t *travMgr, char *travName);
EXTERN int Trav_MgrReadSmoothVar(Trav_Mgr_t *travMgr);
EXTERN double Trav_MgrReadW1(Trav_Mgr_t *travMgr);
EXTERN double Trav_MgrReadW2(Trav_Mgr_t *travMgr);
EXTERN double Trav_MgrReadW3(Trav_Mgr_t *travMgr);
EXTERN double Trav_MgrReadW4(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadEnableDdR(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadTrProfileDynamicEnable(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetTrProfileDynamicEnable(Trav_Mgr_t *travMgr, int enable);
EXTERN int Trav_MgrReadTrProfileThreshold(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetTrProfileThreshold(Trav_Mgr_t *travMgr, int threshold);
EXTERN Cuplus_PruneHeuristic_e Trav_MgrReadTrProfileMethod(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetTrProfileMethod(Trav_Mgr_t *travMgr, Cuplus_PruneHeuristic_e method);
EXTERN int Trav_MgrReadThreshold(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadSquaring(Trav_Mgr_t *travMgr);
EXTERN Pdtutil_VerbLevel_e Trav_MgrReadVerbosity(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetSorting(Trav_Mgr_t *travMgr, int sorting);
EXTERN int Trav_MgrReadSorting(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadBackward(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadKeepNew(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadMaxIter(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadLogPeriod(Trav_Mgr_t *travMgr);
EXTERN int Trav_MgrReadSavePeriod(Trav_Mgr_t *travMgr);
EXTERN char * Trav_MgrReadSavePeriodName(Trav_Mgr_t *travMgr);
EXTERN Trav_FromSelect_e Trav_MgrReadFromSelect(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetDdiMgrTr(Trav_Mgr_t *travMgr, Ddi_Mgr_t *mgrTr);
EXTERN void Trav_MgrSetDdiMgrDefault(Trav_Mgr_t *travMgr, Ddi_Mgr_t *mgrTr);
EXTERN void Trav_MgrSetDdiMgrR(Trav_Mgr_t *travMgr, Ddi_Mgr_t *mgrR);
EXTERN Ddi_Mgr_t * Trav_MgrReadDdiMgrDefault(Trav_Mgr_t *travMgr);
EXTERN Ddi_Mgr_t * Trav_MgrReadDdiMgrTr(Trav_Mgr_t *travMgr);
EXTERN Ddi_Mgr_t * Trav_MgrReadDdiMgrR(Trav_Mgr_t *travMgr);
EXTERN void Trav_MgrSetVerbosity(Trav_Mgr_t *travMgr, Pdtutil_VerbLevel_e verbosity);
EXTERN void Trav_MgrSetBackward(Trav_Mgr_t *travMgr, int backward);
EXTERN void Trav_MgrSetKeepNew(Trav_Mgr_t *travMgr, int keepNew);
EXTERN void Trav_MgrSetMaxIter(Trav_Mgr_t *travMgr, int maxIter);
EXTERN void Trav_MgrSetLogPeriod(Trav_Mgr_t *travMgr, int logPeriod);
EXTERN void Trav_MgrSetSavePeriod(Trav_Mgr_t *travMgr, int savePeriod);
EXTERN void Trav_MgrSetSavePeriodName(Trav_Mgr_t *travMgr, char *savePeriodName);
EXTERN void Trav_MgrSetFromSelect(Trav_Mgr_t *travMgr, Trav_FromSelect_e fromSelect);
EXTERN int Trav_MgrReadMgrAuxFlag(Trav_Mgr_t *travMgr);
EXTERN void Trav_SimulateMain(Fsm_Mgr_t *fsmMgr, int iterNumberMax, int deadEndNumberOf, int logPeriod, int simulationFlag, int depthBreadth, int random, char *init, char *pattern, char *result);
EXTERN Ddi_Bdd_t * Trav_Traversal(Trav_Mgr_t *travMgr);
EXTERN Ddi_Bddarray_t * Trav_MismatchPat(Trav_Mgr_t *travMgr, Tr_Tr_t *TR, Ddi_Bdd_t *firstC, Ddi_Bdd_t *lastC, Ddi_Bdd_t **startp, Ddi_Bdd_t **endp, Ddi_Bddarray_t *newi, Ddi_Vararray_t *psv, Ddi_Vararray_t *nsv, Ddi_Varset_t *pivars);
EXTERN Ddi_Bddarray_t * Trav_UnivAlignPat(Trav_Mgr_t *travMgr, Tr_Tr_t *TR, Ddi_Bdd_t *goal, Ddi_Bdd_t **endp, Ddi_Bddarray_t *rings, Ddi_Vararray_t *psv, Ddi_Vararray_t *nsv, Ddi_Varset_t *pivars, int maxDepth);
EXTERN Trav_FromSelect_e Trav_FromSelectString2Enum(char *string);
EXTERN char * Trav_FromSelectEnum2String(Trav_FromSelect_e enumType);
EXTERN void Trav_TrPartition(Trav_Mgr_t *travMgr, char *varname, int toggle);

/**AutomaticEnd***************************************************************/


#endif /* _TRAV */
