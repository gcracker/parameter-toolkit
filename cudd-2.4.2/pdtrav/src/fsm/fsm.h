/**CHeaderFile*****************************************************************

  FileName    [fsm.h]

  PackageName [fsm]

  Synopsis    [External header file for package Fsm]

  Description [This package provides functions to read in and write out
    descriptions of FSMs in the PdTrav format.<br>
    Support to read a blif file are also given. <p>

    External procedures included in this module are:
    <ul>
    <li> Fsm_MgrLoad ()
    <li> Fsm_MgrStore ()
    <li> Fsm_MgrInit ()
    <li> Fsm_MgrQuit ()
    <li> Fsm_MgrDup ()
    </ul>
    <p>

    The FSM (name myFsm) structure follows the following schema:<p>
    .Fsm myFsm<br>
    </P>
    .Size<br>
      .i 4<br>
      .o 1<br>
      .l 3<br>
    .EndSize<br>
    </P>
    .Ord<br>
      .ordFile myFsmFSM.ord<br>
    .EndOrd<br>
    </P>
    .Name<br>
      .i G0 G1 G2 G3<br>
      .ps G5 G6 G7<br>
      .ns G5$NS G6$NS G7$NS<br>
    .EndName<br>
    </P>
    .Index<br>
      .i 0 1 2 3<br>
      .ps 4 5 6<br>
      .ns 7 8 9<br>
    .EndIndex<br>
    </P>
    .Delta<br>
      .bddFile myFsmdelta.bdd<br>
    .EndDelta<br>
    </P>
    .Lambda<br>
      .bddFile myFsmlambda.bdd<br>
    .EndLambda<br>
    </P>
    .InitState<br>
      .bddFile myFsms0.bdd<br>
    .EndInitState<br>
    </P>
    .Tr<br>
      .bddFile myFsmTR.bdd<br>
    .EndTr<br>
    </P>
    .Reached<br>
       .bddFile myFsmReached.bdd<br>
    .EndReached<br>
    </P>
    .EndFsm<p>

    The functions to read a blif file are partially taken, almost verbatim,
    from the nanotrav directory of the Cudd-2.3.0 package.<br>
    The original names have been modified changing the prefix in the
    following way:<p>
    Port_ ---> Fsm_Port<br>
    Port ---> FsmPort<br>
    port ---> fsmPort<br>
    PORT_ ---> FSM_<p>
    The functions directly called by nanotrav are:
    <pre>
    (name in nanotrav)     (name in the fsmPort package)
    </P>
    Bnet_ReadNetwork       Fsm_PortBnetReadNetwork
    Bnet_FreeNetwork       Fsm_PortBnetFreeNetwork
    Ntr_buildDDs           Fsm_PortNtrBuildDDs 
    Ntr_initState          Fsm_PortNtrInitState
    </pre>
    <p>
    fsmPortBnet.c contains the parts taken from bnet.c
    fsmPortNtr.c contains the parts taken from ntr.c.
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

#ifndef _FSM
#define _FSM

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "ddi.h"
#include "pdtutil.h"
#include "tr.h"
#include "part.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*
 *  FSM Manager Structure
 */

typedef struct Fsm_Mgr_s Fsm_Mgr_t;

/*
 *  Retime Structure
 */

struct Fsm_Retime_s {
  int retimeEqualFlag;
  int removeLatches;
  int *retimeArray;
  Ddi_Bddarray_t *dataArray;
  Ddi_Bddarray_t *enableArray;
  Ddi_Bddarray_t **retimeGraph;
  int *retimeVector;
  int *set;
  int *refEn;
  int *enCnt;
  };

typedef struct Fsm_Retime_s Fsm_Retime_t;

/*
 * Typedef for the FsmPort (nanotrav) interface
 */

typedef struct FsmPortBnetTabline_s FsmPortBnetTabline_t;
typedef struct FsmPortBnetNode_s FsmPortBnetNode_t;
typedef struct FsmPortBnetNetwork_s FsmPortBnetNetwork_t;
typedef struct FsmPortNtrOptions_s FsmPortNtrOptions_t;

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

EXTERN int Fsm_BlifStore(Fsm_Mgr_t *fsmMgr, int reduceDelta, char *fileName, FILE *fp);
EXTERN int Fsm_MgrLoad(Fsm_Mgr_t **fsmMgrP, Ddi_Mgr_t *dd, char *fileFsmName, char *fileOrdName, int bddFlag, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN void Fsm_MgrReadOrdFile(FILE *fp, Fsm_Mgr_t *fsmMgr, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN Fsm_Mgr_t * Fsm_MgrInit(char *fsmName);
EXTERN int Fsm_MgrCheck(Fsm_Mgr_t *fsmMgr);
EXTERN void Fsm_MgrQuit(Fsm_Mgr_t *fsmMgr);
EXTERN Fsm_Mgr_t * Fsm_MgrDup(Fsm_Mgr_t *fsmMgr);
EXTERN void Fsm_MgrAuxVarRemove(Fsm_Mgr_t *fsmMgr);
EXTERN int Fsm_MgrOperation(Fsm_Mgr_t *fsmMgr, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlagP);
EXTERN char * Fsm_MgrReadFileName(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadFsmName(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Mgr_t * Fsm_MgrReadDdManager(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Bdd_t * Fsm_MgrReadReachedBDD(Fsm_Mgr_t *fsmMgr);
EXTERN int Fsm_MgrReadBddFormat(Fsm_Mgr_t *fsmMgr);
EXTERN int Fsm_MgrReadNI(Fsm_Mgr_t *fsmMgr);
EXTERN int Fsm_MgrReadNO(Fsm_Mgr_t *fsmMgr);
EXTERN int Fsm_MgrReadNL(Fsm_Mgr_t *fsmMgr);
EXTERN int Fsm_MgrReadNAuxVar(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadOrdFileName(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadOrdNodeName(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadOrdNodeid(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadNameI(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadNameO(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadNamePS(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadNameNS(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadNameNSTrueName(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadNameAuxVar(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadIndexI(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadIndexO(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadIndexNS(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadIndexPS(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadIndexAuxVar(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Vararray_t * Fsm_MgrReadVarI(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Vararray_t * Fsm_MgrReadVarO(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Vararray_t * Fsm_MgrReadVarPS(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Vararray_t * Fsm_MgrReadVarNS(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Vararray_t * Fsm_MgrReadVarAuxVar(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadDeltaName(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Bddarray_t * Fsm_MgrReadDeltaBDD(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadLambdaName(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Bddarray_t * Fsm_MgrReadLambdaBDD(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadAuxVarName(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Bddarray_t * Fsm_MgrReadAuxVarBDD(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadTrName(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Bdd_t * Fsm_MgrReadTrBDD(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadInitName(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadInitString(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadTrString(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadReachedString(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadFromString(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Bdd_t * Fsm_MgrReadInitBDD(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadFromName(Fsm_Mgr_t *fsmMgr);
EXTERN char * Fsm_MgrReadReachedName(Fsm_Mgr_t *fsmMgr);
EXTERN Ddi_Bdd_t * Fsm_MgrReadFromBDD(Fsm_Mgr_t *fsmMgr);
EXTERN char ** Fsm_MgrReadVarnames(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadVarauxids(Fsm_Mgr_t *fsmMgr);
EXTERN int * Fsm_MgrReadInvauxids(Fsm_Mgr_t *fsmMgr);
EXTERN Pdtutil_VerbLevel_e Fsm_MgrReadVerbosity(Fsm_Mgr_t *fsmMgr);
EXTERN void Fsm_MgrSetFileName(Fsm_Mgr_t *fsmMgr, char *fileName);
EXTERN void Fsm_MgrSetFsmName(Fsm_Mgr_t *fsmMgr, char *fsmName);
EXTERN void Fsm_MgrSetDdManager(Fsm_Mgr_t *fsmMgr, Ddi_Mgr_t *var);
EXTERN void Fsm_MgrSetReachedBDD(Fsm_Mgr_t *fsmMgr, Ddi_Bdd_t *var);
EXTERN void Fsm_MgrSetBddFormat(Fsm_Mgr_t *fsmMgr, int var);
EXTERN void Fsm_MgrSetNI(Fsm_Mgr_t *fsmMgr, int var);
EXTERN void Fsm_MgrSetNO(Fsm_Mgr_t *fsmMgr, int var);
EXTERN void Fsm_MgrSetNL(Fsm_Mgr_t *fsmMgr, int var);
EXTERN void Fsm_MgrSetNAuxVar(Fsm_Mgr_t *fsmMgr, int var);
EXTERN void Fsm_MgrSetOrdFileName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetOrdNodeName(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetOrdNodeid(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetNameI(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetNameO(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetNameNS(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetNameNSTrueName(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetNamePS(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetNameAuxVar(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetIndexI(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetIndexO(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetIndexNS(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetIndexPS(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetIndexAuxVar(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetVarI(Fsm_Mgr_t *fsmMgr, Ddi_Vararray_t *var);
EXTERN void Fsm_MgrSetVarO(Fsm_Mgr_t *fsmMgr, Ddi_Vararray_t *var);
EXTERN void Fsm_MgrSetVarPS(Fsm_Mgr_t *fsmMgr, Ddi_Vararray_t *var);
EXTERN void Fsm_MgrSetVarNS(Fsm_Mgr_t *fsmMgr, Ddi_Vararray_t *var);
EXTERN void Fsm_MgrSetVarAuxVar(Fsm_Mgr_t *fsmMgr, Ddi_Vararray_t *var);
EXTERN void Fsm_MgrSetDeltaName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetDeltaBDD(Fsm_Mgr_t *fsmMgr, Ddi_Bddarray_t *delta);
EXTERN void Fsm_MgrSetLambdaName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetLambdaBDD(Fsm_Mgr_t *fsmMgr, Ddi_Bddarray_t *var);
EXTERN void Fsm_MgrSetAuxVarName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetAuxVarBDD(Fsm_Mgr_t *fsmMgr, Ddi_Bddarray_t *var);
EXTERN void Fsm_MgrSetTrName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetTrBDD(Fsm_Mgr_t *fsmMgr, Ddi_Bdd_t *var);
EXTERN void Fsm_MgrSetInitName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetInitString(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetTrString(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetFromString(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetReachedString(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetInitBDD(Fsm_Mgr_t *fsmMgr, Ddi_Bdd_t *var);
EXTERN void Fsm_MgrSetFromName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetReachedName(Fsm_Mgr_t *fsmMgr, char *var);
EXTERN void Fsm_MgrSetFromBDD(Fsm_Mgr_t *fsmMgr, Ddi_Bdd_t *var);
EXTERN void Fsm_MgrSetVarnames(Fsm_Mgr_t *fsmMgr, char **var);
EXTERN void Fsm_MgrSetVarnamesOne(Fsm_Mgr_t *fsmMgr, char *var, int i);
EXTERN void Fsm_MgrSetVarauxids(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetInvauxids(Fsm_Mgr_t *fsmMgr, int *var);
EXTERN void Fsm_MgrSetVerbosity(Fsm_Mgr_t *fsmMgr, Pdtutil_VerbLevel_e var);
EXTERN int Fsm_MgrLoadFromBlif(Fsm_Mgr_t **fsmMgrP, Ddi_Mgr_t *dd, char *fileFsmName, char *fileOrdName, int bddFlag, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN FsmPortBnetNetwork_t * Fsm_PortBnetReadNetwork(FILE * fp, int pr);
EXTERN int Fsm_PortBnetBuildNodeBDD(Ddi_Mgr_t *dd, FsmPortBnetNode_t * nd, st_table * hash, int params, int nodrop);
EXTERN int Fsm_PortBnetDfsVariableOrder(Ddi_Mgr_t * dd, FsmPortBnetNetwork_t * net);
EXTERN int Fsm_PortBnetReadOrder(Ddi_Mgr_t *dd, char *ordFile, FsmPortBnetNetwork_t *net, int locGlob, int nodrop, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN void Fsm_PortBnetFreeNetwork(FsmPortBnetNetwork_t * net);
EXTERN int Fsm_PortNtrBuildDDs(FsmPortBnetNetwork_t *net, Ddi_Mgr_t *dd, FsmPortNtrOptions_t *option, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN Ddi_Bdd_t * Fsm_PortNtrInitState(Ddi_Mgr_t * dd, FsmPortBnetNetwork_t * net, FsmPortNtrOptions_t * option);
EXTERN int Fsm_MgrPMBuild(Fsm_Mgr_t **fsmMgrPMP, Fsm_Mgr_t *fsmMgr1, Fsm_Mgr_t *fsmMgr2);
EXTERN Fsm_Mgr_t * Fsm_RetimeForRACompute(Fsm_Mgr_t *fsmMgr, Fsm_Retime_t *retimeStrPtr);
EXTERN void Fsm_EnableDependencyCompute(Fsm_Mgr_t *fsmMgr, Fsm_Retime_t *retimeStrPtr);
EXTERN void Fsm_CommonEnableCompute(Fsm_Mgr_t *fsmMgr, Fsm_Retime_t *retimeStrPtr);
EXTERN void Fsm_RetimeCompute(Fsm_Mgr_t *fsmMgr, Fsm_Retime_t *retimeStrPtr);
EXTERN void Fsm_OptimalRetimingCompute(Ddi_Mgr_t *dd, Pdtutil_VerbLevel_e verbosity, int retimeEqual, int *retimeDeltas, Ddi_Varset_t **deltaSupp, Ddi_Bddarray_t *enableArray, Ddi_Bdd_t *en, Ddi_Vararray_t *sarray, int ns, Ddi_Varset_t *latches, Ddi_Varset_t *inputs, Ddi_Varset_t *notRetimed);
EXTERN void Fsm_FindRemovableLatches(Ddi_Mgr_t *dd, int *removableLatches, Ddi_Bddarray_t *delta, Ddi_Vararray_t *sarray, Ddi_Varset_t *inputs, int ns);
EXTERN Fsm_Mgr_t * Fsm_RetimeApply(Fsm_Mgr_t *fsmMgr, Fsm_Retime_t *retimeStrPtr);
EXTERN int Fsm_MgrStore(Fsm_Mgr_t *fsmMgr, char *fileName, FILE *fp, int bddFlag, int bddFormat, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN int Fsm_BddFormatString2Int(char *string);
EXTERN char * Fsm_BddFormatInt2String(int intValue);
EXTERN void Fsm_MgrPrintStats(Fsm_Mgr_t *fsmMgr);
EXTERN void Fsm_MgrPrintPrm(Fsm_Mgr_t *fsmMgr);

/**AutomaticEnd***************************************************************/

#endif /* _FSM */
