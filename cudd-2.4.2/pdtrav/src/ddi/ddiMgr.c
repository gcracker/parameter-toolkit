/**CFile***********************************************************************

  FileName    [ddiMgr.c]

  PackageName [ddi]

  Synopsis    [Functions to deal with DD Managers]

  Description []

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

#include "ddiInt.h"

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
  
  static char *typenames[] = 
  {
    "Bdd",
    "Var",
    "Varset",
    "Expr",
    "Bddarray",
    "Vararray",
    "Varsetarray",
    "Exprarray"
  };

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

/**Function********************************************************************
  Synopsis     [make checks and resizes arrays if required]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
DdiMgrCheckVararraySize(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  int i, sizeOld;
  int nvars = Ddi_ReadNumVars(dd);

  if (dd->vararraySize<nvars) {
    sizeOld = dd->vararraySize;
    if (sizeOld==0) {
      dd->vararraySize = nvars;
    }
    else while (dd->vararraySize<nvars) {
      dd->vararraySize *= 2;
    }
    dd->varnames = Pdtutil_Realloc(char *, dd->varnames, dd->vararraySize);
    dd->varauxids = Pdtutil_Realloc(int, dd->varauxids, dd->vararraySize);
    for (i = sizeOld; i<dd->vararraySize; i++) {
      dd->varnames[i] = NULL;
      dd->varauxids[i] = -1;
    }
  }

}


/**Function********************************************************************
  Synopsis     [Garbage collect freed nodes (handles) in manager node list]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
DdiMgrGarbageCollect (
  Ddi_Mgr_t *ddm
)
{
  Ddi_Generic_t *curr, *next, *last;

  Pdtutil_Assert(Ddi_MgrConsistencyCheck(ddm)!=0,
		 "Error found in manager check");

  if (ddm->nodeList == NULL)
    return;

  last = NULL;
  curr = ddm->nodeList;
  ddm->nodeList = NULL;

  /* 
   *  scan list: free nodes in free status, 
   *  rebuild ddm->nodeList keeping order
   */
  while (curr!=NULL) {
    next = curr->common.next;
    if (curr->common.status == Ddi_Free_c) {
      Pdtutil_Free(curr);
    }
    else {
      if (last == NULL) {
	ddm->nodeList = curr;
      }
      else {
	last->common.next = curr;
      }
      last = curr;
      curr->common.next = NULL;
    }
    curr = next;
  }

  ddm->allocatedNum = ddm->genericNum;
  ddm->freeNum = 0;

  return;
}

/**Function********************************************************************
  Synopsis     [Read the counter of internal references]
  Description  [Read the number of internally referenced DDI handles.
    Variable array and/or one/zero constants.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int 
DdiMgrReadIntRef(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  int intRef = 0;

  if (dd->varGroups != NULL) { 
     intRef++; /* variable group array */
  }
  if (dd->variables != NULL) { 
     intRef++; /* variable array */
  }
  if (dd->one != NULL) { 
     intRef++; /* one constant */
  }
  if (dd->zero != NULL) { 
     intRef++; /* zero constant */
  }
  return (intRef);
}



/**Function********************************************************************
  Synopsis    [Create a variable group]
  Description [Create a variable group. Method is one of MTR_FIXED, 
    MTR_DEFAULT]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
DdiMgrMakeVarGroup(
  Ddi_BddMgr   *dd,
  Ddi_Var_t   *v,
  int grpSize,
  int method
)
{
  int i, p0, j;
  Ddi_Varset_t *vs;

  vs = Ddi_VarsetVoid(dd);
  p0=Ddi_VarCurrPos(v);
  for (i=0;i<grpSize;i++) {
    j = Ddi_ReadInvPerm(dd,p0+i);
    Ddi_VarsetAddAcc(vs,Ddi_IthVar(dd,j));
  }
  for (i=0;i<grpSize;i++) {
    j = Ddi_ReadInvPerm(dd,p0+i);
    Ddi_VarsetarrayWrite(dd->varGroups,j,vs);
  }
  Ddi_Free(vs);

  Cudd_MakeTreeNode(dd->mgrCU,Ddi_VarIndex(v),grpSize,method);
}

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis     [Creates a DdManager.]

  SideEffects  [none]

  SeeAlso      [Ddi_MgrQuit]

******************************************************************************/

Ddi_Mgr_t *
Ddi_MgrInit (
  char *ddiName                /* Name of the DDI structure */,
  DdManager *CUMgr             /* Input CD manager. Created if NULL */,
  unsigned int nvar            /* Initial Number of Variables */,
  unsigned int numSlots        /* Initial Size of Unique Table */,
  unsigned int cacheSize       /* Initial Size of Computed Table (cache) */,
  unsigned long memorySizeMax  /* Max size of Memory */
  )
{
  Ddi_Mgr_t *ddiMgr;
  int i;

  if (memorySizeMax == 0) {
    memorySizeMax = getSoftDataLimit();
#if 0
    fprintf (stdout, "MemoryMax = %d\n", memorySizeMax);
#endif
  }

  /*------------------------- DDI Structure Creation -----------------------*/

  ddiMgr = Pdtutil_Alloc (Ddi_Mgr_t, 1);
  if (ddiMgr == NULL) {
    return (NULL);
  }

  /*------------------- Set the Name of the DDI Structure ------------------*/

  if (ddiName != NULL) {
     Ddi_SetDdiName (ddiMgr, ddiName);
  } else {
     Ddi_SetDdiName (ddiMgr, "ddm");
  }

  /*------------------------------- Set Fields -----------------------------*/

  if (CUMgr == NULL) {
    CUMgr = Cudd_Init (nvar, 0, numSlots, cacheSize, memorySizeMax);
    if (CUMgr == NULL) {
      Pdtutil_Free (ddiMgr);
      return (NULL);
    }
  }

  ddiMgr->mgrCU = CUMgr;
  ddiMgr->autodynMethod = (Cudd_ReorderingType) 0; 
  ddiMgr->autodynSuspended = 0; 
  ddiMgr->autodynNestedSuspend = 0; 
  ddiMgr->abortOnSift = 0; 
  ddiMgr->abortedOp = 0; 
  ddiMgr->currNodeId = 0;
  ddiMgr->tracedId = -1; /* trace disabled */
  ddiMgr->freeNum = 0;
  ddiMgr->lockedNum = 0;
  ddiMgr->allocatedNum = 0;
  ddiMgr->genericNum = 0;

  for (i=0; i<DDI_NTYPE; i++) {
    ddiMgr->typeNum[i] = 0;
    ddiMgr->typeLockedNum[i] = 0;
  }

  ddiMgr->nodeList = NULL;

  ddiMgr->one = Ddi_BddMakeFromCU(ddiMgr,Cudd_ReadOne(CUMgr));
  Ddi_Lock(ddiMgr->one);
  ddiMgr->zero = Ddi_BddMakeFromCU(ddiMgr,Cudd_Not(Cudd_ReadOne(CUMgr)));
  Ddi_Lock(ddiMgr->zero);

  ddiMgr->meta.groupNum = 0;
  ddiMgr->meta.nvar = 0;
  ddiMgr->meta.bddNum = 0;
  ddiMgr->meta.groups = NULL;
  ddiMgr->meta.ord = NULL;
  ddiMgr->meta.id = 0;

  /*---------------- Init variable names and auxids ------------------------*/

  ddiMgr->varnames = NULL;
  ddiMgr->varauxids = NULL;
  if (nvar > PDTUTIL_INITIAL_ARRAY_SIZE) {
    ddiMgr->vararraySize = nvar;
  } else {
    ddiMgr->vararraySize = PDTUTIL_INITIAL_ARRAY_SIZE;
  }
  ddiMgr->varnames = Pdtutil_Alloc(char *, ddiMgr->vararraySize);
  ddiMgr->varauxids = Pdtutil_Alloc(int, ddiMgr->vararraySize);

  for (i=0; i<ddiMgr->vararraySize; i++) {
    ddiMgr->varnames[i] = NULL;
    ddiMgr->varauxids[i] = (-1);
  }

  ddiMgr->variables = Ddi_VararrayAlloc(ddiMgr,nvar);
  Ddi_Lock(ddiMgr->variables);
  ddiMgr->varGroups = Ddi_VarsetarrayAlloc(ddiMgr,nvar);
  Ddi_Lock(ddiMgr->varGroups);
  for (i=0; i<nvar; i++) {
    DdiVarNewFromCU(ddiMgr,Cudd_bddIthVar(CUMgr,i));
  }

  ddiMgr->settings.verbosity = Pdtutil_VerbLevelUsrMax_c;

  ddiMgr->settings.part.existClustThresh = -1;

  ddiMgr->settings.meta.groupSizeMin = DDI_META_GROUP_SIZE_MIN_DEFAULT;
  ddiMgr->settings.meta.method = Ddi_Meta_Size;
  ddiMgr->settings.dump.partOnSingleFile = 1;

  ddiMgr->stats.peakProdLocal = 0;
  ddiMgr->stats.peakProdGlobal = 0;

  return (ddiMgr);
}

/**Function********************************************************************
  Synopsis     [make checks on DDI manager. Return 0 for failure]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int
Ddi_MgrConsistencyCheck (
  Ddi_Mgr_t *ddm
)
{
  int ret;
  int freeNum, totNum;
  Ddi_Generic_t *curr;

  DdiMgrCheckVararraySize(ddm);

  ret = 1;

  /* 
   *  scan node list counting nodes
   */
  totNum = freeNum = 0;
  curr = ddm->nodeList;
  while (curr!=NULL) {
    totNum++;
    if (curr->common.status == Ddi_Free_c) {
      freeNum++;
    }
    curr = curr->common.next;
  }

  if ((ddm->genericNum+ddm->freeNum)!=ddm->allocatedNum) {
    fprintf(stderr,
      "\nWrong number of free/allocated DDI nodes found in mgr check\n");
    fprintf(stderr,"allocated nodes #: %d\n", ddm->genericNum);
    fprintf(stderr,"free      nodes #: %d\n", ddm->freeNum);
    fprintf(stderr,"live      nodes #: %d\n", ddm->genericNum);
    ret = 0;
  }
  if (ddm->allocatedNum!=totNum) {
    fprintf(stderr,"\nWrong total number of DDI nodes found in mgr check\n");
    fprintf(stderr,"recorded nodes #: %d\n", ddm->allocatedNum);
    fprintf(stderr,"found    nodes #: %d\n", totNum);
    ret = 0;
  }
  if (ddm->freeNum!=freeNum) {
    fprintf(stderr,"\nWrong number of free DDI nodes found in mgr check\n");
    fprintf(stderr,"recorded nodes #: %d\n", ddm->freeNum);
    fprintf(stderr,"found    nodes #: %d\n", freeNum);
    ret = 0;
  }

  /* check CUDD manager */
  if (Cudd_DebugCheck (ddm->mgrCU)!=0) {
    fprintf(stderr,"\nCUDD manager check failed\n");
    ret = 0;
  };

#if 0
  if (Cudd_CheckKeys(ddm->mgrCU)!=0) {
    fprintf(stderr,"\nCUDD key check failed\n");
    ret = 0;
  };
#endif

  return(ret);
}

/**Function********************************************************************
  Synopsis     [Check number of externally referenced DDI handles]
  Description  [Check number of externally referenced DDI handles.
    This is the numer of generic nodes (allocated - freed), diminished by
    the number of locked nodes + 3 (2 constants + variable array). 
    Return 0 upon failure.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int
Ddi_MgrCheckExtRef (
  Ddi_Mgr_t *ddm,
  int n
)
{
  int ret=1;

  /* allocated DDI nodes include 2 constants and variable array */
  if (n!=Ddi_MgrReadExtRef(ddm)) {
    fprintf(stderr,"\nWrong number of external references in mgr check\n");
    fprintf(stderr,"Required: %d - found %d\n", n, Ddi_MgrReadExtRef(ddm));
    Ddi_MgrPrintAllocStats(ddm,stderr);
    /*Ddi_MgrPrintExtRef(ddm,0);*/
    ret = 0;
  }

  return(ret);
}


/**Function********************************************************************
  Synopsis     [print ids of external refs]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrPrintExtRef (
  Ddi_Mgr_t *ddm,
  int minNodeId
)
{
  Ddi_Generic_t *curr;

  Pdtutil_Assert(Ddi_MgrConsistencyCheck(ddm)!=0,
		 "Error found in manager check");

  if (ddm->nodeList == NULL)
    return;

  printf ("\nExternally referenced DDI nodes:\n");
  for (curr=ddm->nodeList; curr!=NULL; curr=curr->common.next) {
    if (curr->common.status == Ddi_Unlocked_c) {
      if (curr->common.nodeid >= minNodeId) {
        printf ("<%s:%d> ", typenames[DdiType(curr)], 
          curr->common.nodeid); fflush(stdout);
      }
    }
  }
  printf("\n\n");

  return;
}


/**Function********************************************************************
  Synopsis     [update DDI manager after directly working on CUDD manager]
  Description  [update DDI manager after directly working on CUDD manager.
    New variables have possibly been created.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrUpdate(
  Ddi_Mgr_t *ddm
)
{
  int i;
  int nvars = Ddi_ReadNumVars(ddm);

  for (i=Ddi_VararrayNum(ddm->variables); i<nvars; i++) {
    DdiVarNewFromCU(ddm,Cudd_bddIthVar(ddm->mgrCU,i));
    /* no variable group initially present */
    Ddi_VarsetarrayWrite(ddm->varGroups,i,NULL);
  }

  DdiMgrCheckVararraySize(ddm);

}


/**Function********************************************************************
  Synopsis     [Close a DdManager.]
  SideEffects  [none]
  SeeAlso      [Ddi_MgrInit
******************************************************************************/

void
Ddi_MgrQuit(
  Ddi_Mgr_t *dd  /* dd manager */
)
{
  int i;

  Pdtutil_Free(dd->name);
  Ddi_Unlock(dd->one);
  Ddi_Free(dd->one);
  Ddi_Unlock(dd->zero);
  Ddi_Free(dd->zero);
  Pdtutil_StrArrayFree(dd->varnames,Ddi_ReadNumVars(dd));
  Pdtutil_Free(dd->varauxids);

  /* close meta handling */
  Ddi_MetaQuit(dd);

#if 0
  /* debugging print */
  printf ("Variable Groups:\n");
  for (i=0; i<Ddi_VarsetarrayNum(dd->varGroups); i++){
    if (Ddi_VarsetarrayRead(dd->varGroups,i)!=NULL) {
      Ddi_VarsetPrint (Ddi_VarsetarrayRead(dd->varGroups,i), 3, NULL, stdout);
    }
  }
#endif

  /* free variable groups */
  Ddi_Unlock(dd->varGroups);
  Ddi_Free(dd->varGroups);  

  /* free variables */
  /* free nodes: si possono includere le variabili */

  /* unlock variables so that they can be freed */
  for (i=0; i<Ddi_VararrayNum(dd->variables); i++) {
    Ddi_Unlock(Ddi_VararrayRead(dd->variables,i));
  }
  Ddi_Unlock(dd->variables);
  Ddi_Free(dd->variables);  

  DdiMgrGarbageCollect(dd);

  if (Ddi_MgrReadVerbosity(dd) >= Pdtutil_VerbLevelUsrMed_c) {
    fprintf (stdout, "Quitting DDI manager\n");
    if (Ddi_MgrCheckExtRef (dd,0)==0) {
      Ddi_MgrPrintExtRef(dd, 0);
    }
  }

  if (Ddi_MgrConsistencyCheck(dd)==0) {
    fprintf (stderr, "Error found in manager check before Quit");
  }

#if 0
  Cudd_Quit(dd->mgrCU);
#endif
  Pdtutil_Free(dd);
}

/**Function********************************************************************

  Synopsis     [Creates a copy of a DdManager.]
  SideEffects  [none]
  SeeAlso      [Ddi_MgrQuit]

******************************************************************************/

Ddi_Mgr_t *
Ddi_MgrDup (
  Ddi_Mgr_t *dd            /* source dd manager */
  )
{
  Ddi_Mgr_t *dd2;
  char **varnames;
  int *varauxids;
  unsigned int nvar         = Ddi_ReadNumVars(dd); 
  unsigned int numSlots     = DDI_UNIQUE_SLOTS /*Ddi_ReadSlots(dd)*/;
  unsigned int cacheSize    = Ddi_ReadCacheSlots(dd);
  unsigned int maxCacheSize = DDI_MAX_CACHE_SIZE;

  Cudd_ReorderingType dynordMethod;
  int autodyn = Ddi_ReadReorderingStatus (dd, &dynordMethod);  

  dd2 = Ddi_MgrInit (NULL, NULL, nvar, numSlots, cacheSize, maxCacheSize);
  if (dd2==NULL) {
    return (NULL);  
  }

  varnames = Ddi_MgrReadVarnames (dd);
  varauxids = Ddi_MgrReadVarauxids (dd);
  dd2->varnames = Pdtutil_StrArrayDup (varnames, nvar);
  dd2->varauxids = Pdtutil_IntArrayDup (varauxids, nvar);

  Ddi_MgrAlign (dd2, dd);

  if (autodyn) {
    Ddi_MgrAutodynEnable (dd2, dynordMethod);
    Ddi_MgrSetDynordThresh (dd2,Ddi_MgrReadDynordThresh(dd));
  }

  if (dd->autodynSuspended) {
    Ddi_MgrAutodynSuspend(dd2);
  }

  return (dd2);

}


/**Function********************************************************************
  Synopsis     [Reorders all DDs in a manager.]
  Description  [Reorders all DDs in a manager according to the input
    order. The input specification may be partial, i.e.
    it may include only a subset of variables.
    ]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/

void
Ddi_MgrShuffle (
  Ddi_Mgr_t *dd      /* dd Manager to be Alligned */,
  int *sortedIds     /* Array of sorted ids */,
  int nids           /* Number of ids */
)
{
  int *newperm, *ids;
  int nv, i, j, result;

  nv = Ddi_ReadNumVars(dd);

  Pdtutil_Assert (nids<=nv, "Input array for MgrShuffle is too large.");

  newperm = Pdtutil_Alloc(int,nv); 
  ids = Pdtutil_Alloc(int,nv); 

  for(i=0; i<nv; i++) {
    newperm[i] = Ddi_ReadInvPerm(dd,i);
  }

  for(i=0; i<nv; i++) {
    ids[i] = -1;
  }

  for(i=0; i<nids; i++) {
    Pdtutil_Assert(sortedIds[i]<nv,"imput id out of range in MgrShuffle"); 
    ids[sortedIds[i]] = 1;
  }

  for(i=0,j=0; i<nv; i++) {
    /* if variable is in sortedIds shuffle (pick up from sortedIds) */
    if (ids[newperm[i]] > 0) {
      newperm[i] = sortedIds[j++];
    }
  }

  Pdtutil_Assert (j==nids, "Not all input ids used for MgrShuffle.");

  if (Ddi_MgrReadVerbosity (dd) >= Pdtutil_VerbLevelDevMin_c) {
    fprintf(stdout,"Partial Sorted ids: ");
    for(i=0; i<nids; i++) {
      fprintf(stdout,"%d ", sortedIds[i]);
    }
    fprintf(stdout,"\n");
    fprintf(stdout,"Full Sorted ids: ");
    for(i=0; i<nv; i++) {
      fprintf(stdout,"%d ", newperm[i]);
    }
    fprintf(stdout,"\n");
  }

  result = Cudd_ShuffleHeap (Ddi_MgrReadMgrCU (dd), newperm);

  Pdtutil_Warning (result==0, "ERROR during ShuffleHeap.");

  Pdtutil_Free (newperm);
  Pdtutil_Free (ids);

  return;
}


/**Function********************************************************************
  Synopsis     [Aligns the order of two managers.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/

void
Ddi_MgrAlign (
  Ddi_Mgr_t *dd            /* dd manager to be aligned */,
  Ddi_Mgr_t *ddRef         /* reference dd manager */
)
{
  int *newperm;
  int nv, nvRef, i, j, idRef, result;

  nv = Ddi_ReadNumVars(dd);
  nvRef = Ddi_ReadNumVars(ddRef);

  newperm = Pdtutil_Alloc(int,nv); 

  for(i=0,j=0; i<nvRef; i++) {
    idRef = Ddi_ReadInvPerm(ddRef,i);
    if (idRef < nv) { 
      /* variable has a corresponding one in dd */
      newperm[j++] = idRef;
    }
  }

  if (Ddi_MgrReadVerbosity (dd) >= Pdtutil_VerbLevelDevMin_c) {
    fprintf(stdout,"Align ids: ");
    for(i=0; i<nv; i++) {
      fprintf(stdout,"%d ", newperm[i]);
    }
    fprintf(stdout,"\n");
  }

  Pdtutil_Assert(j==nv,"missing variables when aligning managers");

  result = Cudd_ShuffleHeap(Ddi_MgrReadMgrCU(dd),newperm);

  if (!result) {
    fprintf(stderr,"ERROR during ShuffleHeap\n");
  }

  Pdtutil_Free(newperm);

  return;
}


/**Function********************************************************************
  Synopsis    [Create groups of 2 variables ]
  Description [Create groups of 2 variables: variables of corresponding 
               indexes in vfix and vmov are coupled. If vmov[i] and vfix[i]
               are not adjacent in the ordering, vmov[i] is moved to the
               position below vfix[i].
              ]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/

void
Ddi_MgrCreateGroups2(
  Ddi_Mgr_t *dd              /* manager */,
  Ddi_Vararray_t *vfix       /* first array */,
  Ddi_Vararray_t *vmov       /* first array */
)
{
  int i, j, fixpos, movpos, nvars, nvarstot;
  int *tmpIds, *sortedIds, *grpIds;
  Ddi_Var_t *fv, *mv;
  int do_shuffle = 0;

  Pdtutil_Assert(Ddi_VararrayNum(vfix)==Ddi_VararrayNum(vmov),
    "Variable arrays with different size in grouping");

  nvars = Ddi_VararrayNum(vfix);
  nvarstot = Ddi_ReadNumVars(dd);

  /* moving vmov */

  tmpIds = Pdtutil_Alloc(int,nvarstot); 
  grpIds = Pdtutil_Alloc(int,nvarstot); 
  sortedIds = Pdtutil_Alloc(int,nvarstot); 

  /* creating interleaved ordering for group variables */

  /* initialize tmpIds with present ordering */
  for (i=0,j=0; i<nvarstot; i++) {
    tmpIds[i] = Ddi_ReadInvPerm(dd,i); 
    grpIds[i] = -1;
  }

  for (i=0,j=0; i<nvars; i++) {
    /* obtain variable positions and ids */
    fv = Ddi_VararrayRead(vfix,i);
    mv = Ddi_VararrayRead(vmov,i);
    fixpos = Ddi_VarCurrPos(fv);
    movpos = Ddi_VarCurrPos(mv);

    if (abs(fixpos-movpos)!=1) /* check enabling shuffle */
      do_shuffle = 1;

    /* clear tmpIds, leaving variables outside groups */
    Pdtutil_Assert(tmpIds[fixpos]==Ddi_VarIndex(fv),
      "id mismatch creating groups");
    Pdtutil_Assert(tmpIds[movpos]==Ddi_VarIndex(mv),
       "id mismatch creating groups");
    tmpIds[fixpos] = -1;
    tmpIds[movpos] = -1;

    /* collect fix variable array ids */
    grpIds[fixpos] = i;
  }

  if (do_shuffle) {
    /*
     * now group variables may be retrieved from grpIds, non group 
     * variables from tmpIds. Do merging
     */
    for (i=0,j=0; i<nvarstot; i++) {
      if (tmpIds[i] >= 0) {
        /* non group variable: keep as it is in new ordering */
        Pdtutil_Assert(grpIds[i]==-1,
          "a variable appears to be both group and no group");
        sortedIds[j++] = tmpIds[i];
      }
      else if (grpIds[i]>=0) { /* skip old positions of vmov variables */ 
        /* group variable: put a group (2 vars) in final ordering */
        Pdtutil_Assert(grpIds[i]<nvars,
          "invalid array index while grouping variables");
        fv = Ddi_VararrayRead(vfix,grpIds[i]);
        mv = Ddi_VararrayRead(vmov,grpIds[i]);
        fixpos = Ddi_VarCurrPos(fv);
        movpos = Ddi_VarCurrPos(mv);
	/* keep relative ordering within group */
        if (fixpos<movpos) {
          sortedIds[j++] = Ddi_VarIndex(fv);
          sortedIds[j++] = Ddi_VarIndex(mv);
        }
        else {
          sortedIds[j++] = Ddi_VarIndex(mv);
          sortedIds[j++] = Ddi_VarIndex(fv);
        }
      }
    } /* end for */

    Pdtutil_Assert(j==nvarstot,"sorted id array not completed while grouping");

    /* do variable shuffle */
    Ddi_MgrShuffle (dd, sortedIds, nvarstot);

  }

  for (i=0,j=0; i<nvars; i++) {
    /* obtain variable positions and ids */
    fv = Ddi_VararrayRead(vfix,i);
    mv = Ddi_VararrayRead(vmov,i);
    fixpos = Ddi_VarCurrPos(fv);
    movpos = Ddi_VarCurrPos(mv);

    Pdtutil_Assert(abs(fixpos-movpos)==1,"grouping non adjacent variables");
    if (fixpos<movpos) {
      DdiMgrMakeVarGroup(dd,fv,2,MTR_DEFAULT);
    } else {
      DdiMgrMakeVarGroup(dd,mv,2,MTR_DEFAULT);
    }
  }

  return;
}

/**Function********************************************************************

  Synopsis    [Stores the variable ordering]

  Description [This function stores the variable ordering of a dd manager.
    Variable names and aux ids are used.
    ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int
Ddi_MgrOrdWrite (
  Ddi_Mgr_t *dd                                 /* Decision Diagram Manager */,
  char *filename                                /* File Name */,
  FILE *fp                                      /* Pointer to Store File */,
  Pdtutil_VariableOrderFormat_e ordFileFormat   /* File Format */
  )
{
  char **vnames = Ddi_MgrReadVarnames (dd);
  int *vauxids = Ddi_MgrReadVarauxids (dd);
  int *iperm, nvars, i;

  nvars = Ddi_ReadNumVars (dd);
  iperm = Pdtutil_Alloc (int, nvars); 
  for (i=0; i<nvars; i++) {
    iperm[i] = Ddi_ReadInvPerm (dd, i);
  }

  Pdtutil_OrdWrite (vnames, vauxids, iperm, nvars, filename, fp,
    ordFileFormat);

  Pdtutil_Free(iperm);

  return (0);
}

/**Function********************************************************************

  Synopsis    [Reads the variable ordering]

  Description [This function reads the variable ordering of a dd manager.
    Existing variables with names in the ordering are shuffled
    to match the ordering.
    ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int
Ddi_MgrReadOrdNamesAuxids (
  Ddi_Mgr_t *dd                                /* Manager */,
  char *filename                               /* File Name */,
  FILE *fp                                     /* Pointer to the Store File */,
  Pdtutil_VariableOrderFormat_e ordFileFormat  /* File Format */
  )
{
  int nvars, i, id, *sortedIds;
  Ddi_Var_t *var;
  char **vnames;
  int *vauxids;

  nvars = Pdtutil_OrdRead (&vnames, &vauxids, filename, fp, ordFileFormat);

  if (nvars <= 0) {
    return (nvars);
  }

  sortedIds = Pdtutil_Alloc (int, nvars); 
  Pdtutil_Assert(vnames!=NULL,"not yet supported read ord without varnames");
  if (vnames != NULL) {
    for (i=0; i<nvars; i++) {
      Pdtutil_Assert(vnames[i] != NULL, "NULL varname in OrdRead");
      id = Ddi_VarIndex(Ddi_VarFromName(dd,vnames[i]));
      if (id < 0) { /* create a new var */
        var = Ddi_VarNew(dd);
        Ddi_VarAttachName(var,vnames[i]);
        if (vauxids != NULL)
          Ddi_VarAttachAuxid(var,vauxids[i]);
        id = Ddi_VarIndex(var);
      }
      sortedIds[i] = id;
    }
  }

  Ddi_MgrShuffle (dd, sortedIds, nvars);

  Pdtutil_Free(sortedIds);
  
  return nvars;
}


/**Function********************************************************************
  Synopsis     [Suspend autodyn if active.]
  SideEffects  [none]
  SeeAlso      [Ddi_MgrInit]
******************************************************************************/
void
Ddi_MgrAutodynSuspend(
  Ddi_Mgr_t *dd  /* dd manager */
)
{
  Cudd_ReorderingType dynordMethod;
  int autodyn = Ddi_ReadReorderingStatus (dd, &dynordMethod);  

  /* increment suspend counter so that nested suspend are supported */
  dd->autodynNestedSuspend++;
 
  /* actually suspend only if enabled */
  if (autodyn) {
    Ddi_MgrAutodynDisable (dd);
    dd->autodynMethod = dynordMethod; /* suspended autodyn method */
    dd->autodynSuspended = 1;
  }

}


/**Function********************************************************************
  Synopsis     [Resume autodyn if suspended.]
  SideEffects  [none]
  SeeAlso      [Ddi_MgrInit]
******************************************************************************/
void
Ddi_MgrAutodynResume(
  Ddi_Mgr_t *dd  /* dd manager */
  )
{
  Pdtutil_Assert(dd->autodynNestedSuspend>0, 
    "Unbalanced dynord suspend/resume");
  if (--dd->autodynNestedSuspend == 0) {
     /* enable only if all nested suspend closed */
    if (dd->autodynSuspended) {
      Ddi_MgrAutodynEnable (dd, dd->autodynMethod);
    }
  }
  return;
}


/**Function********************************************************************
  Synopsis     [Enable Abort on sift.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrAbortOnSiftEnable(
  Ddi_Mgr_t *dd  /* dd manager */
)
{
  Pdtutil_Assert(dd->abortOnSift==0,"Abort on sift already enabled");
  dd->abortOnSift=1;
  dd->abortedOp=0;
  Cuplus_bddOpEnableAbortOnSift(dd->mgrCU);
}

/**Function********************************************************************
  Synopsis     [Disable Abort on sift.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrAbortOnSiftDisable(
  Ddi_Mgr_t *dd  /* dd manager */
)
{
  Pdtutil_Assert(dd->abortOnSift==1,"Abort on sift already disabled");
  dd->abortOnSift=0;
  dd->abortedOp=0;
  Cuplus_bddOpDisableAbortOnSift(dd->mgrCU);
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        [To be congruent operationFlag should be a Pdtutil_MgrOp_t
   type, and returnFlag of Pdtutil_MgrRet_t type.]

  SeeAlso            []

******************************************************************************/

int
Ddi_MgrOperation (
  Ddi_Mgr_t **ddMgrP                 /* DD Manager Pointer */,
  char *string                       /* String */,
  Pdtutil_MgrOp_t operationFlag      /* Operation Flag */,
  void **voidPointer                 /* Generic Pointer */,
  Pdtutil_MgrRet_t *returnFlagP      /* Type of the Pointer Returned */
  )
{
  char *stringFirst, *stringSecond;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (*ddMgrP == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return (1);
  }

  /*--------------------------- Print All Options ---------------------------*/

  /* Nothing for Now */

  /*----------------------- Take Main and Secondary Name --------------------*/

  Pdtutil_ReadSubstring (string, &stringFirst, &stringSecond);
 
  /*----------------------- Operation on the DD Manager ---------------------*/

  if (stringFirst == NULL) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "DecisionDiagramManager Verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Ddi_MgrReadVerbosity (*ddMgrP)),
          Ddi_MgrReadVerbosity (*ddMgrP));
        break;
      case Pdtutil_MgrOpStats_c:
        Ddi_MgrPrintStats (*ddMgrP);
        break;
      case Pdtutil_MgrOpMgrRead_c:
        *voidPointer = (void *) *ddMgrP;
        *returnFlagP = Pdtutil_MgrRetDdMgr_c;
        break;
      case Pdtutil_MgrOpMgrSet_c:
        *ddMgrP = (Ddi_Mgr_t *) *voidPointer;
        break;
      case Pdtutil_MgrOpMgrDelete_c:
        Ddi_MgrQuit (*ddMgrP);
        *returnFlagP = Pdtutil_MgrOpMgrDelete_c;
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on DDI Manager");
        break;
     }

    return (0);
  }

  /*-------------------------------- Options --------------------------------*/

  if (strcmp(stringFirst, "verbosity")==0) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Ddi_MgrSetVerbosity (*ddMgrP,
          Pdtutil_VerbosityString2Enum (*voidPointer));
        break;
      case Pdtutil_MgrOpOptionShow_c:
        fprintf (stdout, "Verbosity %s (%d)\n",
          Pdtutil_VerbosityEnum2String (Ddi_MgrReadVerbosity (*ddMgrP)),
          Ddi_MgrReadVerbosity (*ddMgrP));
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on TRAV Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*------------------------------- No Match --------------------------------*/

  Pdtutil_Warning (1, "Operation on DDI manager not allowed");
  return (1);
}

/**Function********************************************************************
  Synopsis     [Prints on standard outputs statistics on a DD manager]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/

void
Ddi_MgrPrintStats (
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  Pdtutil_ChrPrint (stdout, '*', 50);
  fprintf (stdout, "\nDecision Diagram Statistics Summary:\n");

  Cudd_PrintInfo (dd->mgrCU, stdout);

  Pdtutil_ChrPrint (stdout, '*', 50);
  fprintf (stdout, "\n");

  return;
}


/**Function********************************************************************
  Synopsis     [Reads the Cudd Manager]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
DdManager *
Ddi_MgrReadMgrCU(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return (dd->mgrCU);
}

/**Function********************************************************************
  Synopsis     [Reads one constant]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
Ddi_Bdd_t *
Ddi_MgrReadOne(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return dd->one;
}

/**Function********************************************************************
  Synopsis     [Reads zero constant]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
Ddi_Bdd_t *
Ddi_MgrReadZero(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return dd->zero;
}

/**Function********************************************************************
  Synopsis     [Read current node id field]
  Description  [Read current node id field. DDI nodes are identified by this
                id, which is incremented at any new node creation.]
  SideEffects  [none]
  SeeAlso      [Ddi_MgrSetTracedId]
******************************************************************************/
int
Ddi_MgrReadCurrNodeId(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return(dd->currNodeId);
}

/**Function********************************************************************
  Synopsis     [Set traced node id field]
  Description  [Set traced node id field. Creation of node >= id are logged
                using DdiTraceNodeAlloc.
                This is expecially useful for debugging BDD leaks and memory
                bugs. To watch generation of node with given ID, put a 
                breakpoint on DdiTraceNodeAlloc after setting manager 
                tracedId to ID (Ddi_MgrSetTracedId(dd,ID)).]
  SideEffects  [none]
  SeeAlso      [Ddi_MgrReadCurrNodeId]
******************************************************************************/
void
Ddi_MgrSetTracedId(
  Ddi_Mgr_t *dd            /* source dd manager */,
  int id
)
{
  dd->tracedId = id;
}

/**Function********************************************************************
  Synopsis     [Set part clustering threshold]
  Description  [Set part clustering threshold. This is the threshold used for
                quantification operators on partitioned BDDs.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrSetExistClustThresh(
  Ddi_Mgr_t *dd            /* source dd manager */,
  int th
)
{
  dd->settings.part.existClustThresh = th;
}

/**Function********************************************************************
  Synopsis     [Read part clustering threshold]
  Description  [Read part clustering threshold. This is the threshold used for
                quantification operators on partitioned BDDs.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int
Ddi_MgrReadExistClustThresh(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return(dd->settings.part.existClustThresh);
}

/**Function********************************************************************
  Synopsis     [Reads the variable names]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
char**
Ddi_MgrReadVarnames(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  int i;
  int nvars = Ddi_ReadNumVars(dd);

  if (dd->vararraySize<nvars) {
    dd->vararraySize *= 2;
    dd->varnames = Pdtutil_Realloc(char *, dd->varnames, dd->vararraySize);
    dd->varauxids = Pdtutil_Realloc(int, dd->varauxids, dd->vararraySize);
    for (i = dd->vararraySize/2; i<dd->vararraySize; i++) {
      dd->varnames[i] = NULL;
      dd->varauxids[i] = -1;
    }
  }

  return dd->varnames;
}

/**Function********************************************************************
  Synopsis     [Reads the variable auxiliary ids]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int *
Ddi_MgrReadVarauxids(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  int i;
  int nvars = Ddi_ReadNumVars(dd);

  if (dd->vararraySize<nvars) {
    dd->vararraySize *= 2;
    dd->varnames = Pdtutil_Realloc(char *, dd->varnames, dd->vararraySize);
    dd->varauxids = Pdtutil_Realloc(int, dd->varauxids, dd->vararraySize);
    for (i = dd->vararraySize/2; i<dd->vararraySize; i++) {
      dd->varnames[i] = NULL;
      dd->varauxids[i] = -1;
    }
  }

  return dd->varauxids;
}

/**Function********************************************************************
  Synopsis     [Read the counter of external references]
  Description  [Read the number of externally referenced DDI handles.
    This is the numer of generic nodes (allocated - freed), diminished by
    the number of locked nodes + 3 (2 constants + variable array).]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int 
Ddi_MgrReadExtRef(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return (dd->genericNum-dd->lockedNum);
}

/**Function********************************************************************
  Synopsis     [Read the counter of external references to BDDs]
  Description  [Read the number of externally referenced BDD handles.
    This is the number of allocated - freed, diminished by
    the number of locked nodes.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int 
Ddi_MgrReadExtBddRef(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return (dd->typeNum[Ddi_Bdd_c]-dd->typeLockedNum[Ddi_Bdd_c]);
}

/**Function********************************************************************
  Synopsis     [Read the counter of external references to BDD arrays]
  Description  [Read the number of externally referenced BDD array handles.
    This is the number of allocated - freed, diminished by
    the number of locked nodes.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int 
Ddi_MgrReadExtBddarrayRef(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return (dd->typeNum[Ddi_Bddarray_c]-dd->typeLockedNum[Ddi_Bddarray_c]);
}

/**Function********************************************************************
  Synopsis     [Read the counter of external references to varsets]
  Description  [Read the number of externally referenced varset handles.
    This is the number of allocated - freed, diminished by
    the number of locked nodes.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int 
Ddi_MgrReadExtVarsetRef(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return (dd->typeNum[Ddi_Varset_c]-dd->typeLockedNum[Ddi_Varset_c]);
}


/**Function********************************************************************
  Synopsis     [Returns the threshold for the next dynamic reordering.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
unsigned int 
Ddi_MgrReadDynordThresh(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return Cudd_ReadNextReordering(Ddi_MgrReadMgrCU(dd));
}

/**Function********************************************************************
  Synopsis     [Returns the threshold for the next dynamic reordering.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/

char *
Ddi_ReadDdiName (
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return (dd->name);
}


/**Function********************************************************************
  Synopsis     [Returns the threshold for the next dynamic reordering.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/

void
Ddi_SetDdiName (
  Ddi_Mgr_t *dd            /* source dd manager */,
  char *ddiName
)
{
  dd->name = Pdtutil_StrDup (ddiName);
}


/**Function********************************************************************
  Synopsis     [Read verbosity]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/

Pdtutil_VerbLevel_e
Ddi_MgrReadVerbosity(
  Ddi_Mgr_t *ddiMgr         /* Decision Diagram Interface Manager */
)
{
  return (ddiMgr->settings.verbosity);
}

/**Function********************************************************************
  Synopsis     [Set verbosity]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/

void
Ddi_MgrSetVerbosity(
  Ddi_Mgr_t *ddiMgr              /* Decision Diagram Interface Manager */,
  Pdtutil_VerbLevel_e verbosity   /* Verbosity Level */
)
{
  ddiMgr->settings.verbosity = verbosity;

  return;
}

/**Function********************************************************************
  Synopsis     [Read peak product local]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int
Ddi_MgrReadPeakProdLocal(
  Ddi_Mgr_t *ddiMgr         /* Decision Diagram Interface Manager */
)
{
  return (ddiMgr->stats.peakProdLocal);
}

/**Function********************************************************************
  Synopsis     [Read peak product global]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
int
Ddi_MgrReadPeakProdGlobal(
  Ddi_Mgr_t *ddiMgr         /* Decision Diagram Interface Manager */
)
{
  return (ddiMgr->stats.peakProdGlobal);
}

/**Function********************************************************************
  Synopsis     [Reset peak product local]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrPeakProdLocalReset(
  Ddi_Mgr_t *ddiMgr              /* Decision Diagram Interface Manager */
)
{
  ddiMgr->stats.peakProdLocal = 0;
}

/**Function********************************************************************
  Synopsis     [Update peak product stats]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrPeakProdUpdate(
  Ddi_Mgr_t *ddiMgr              /* Decision Diagram Interface Manager */,
  int size
)
{
   if (size > ddiMgr->stats.peakProdLocal) {
     ddiMgr->stats.peakProdLocal = size;
     if (size > ddiMgr->stats.peakProdGlobal) {
       ddiMgr->stats.peakProdGlobal = size;
     }
   }
}

/**Function********************************************************************
  Synopsis     [Sets the CUDD manager]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrSetMgrCU(
  Ddi_Mgr_t *dd            /* source dd manager */,
  DdManager *m             /* CUDD manager */
)
{
  dd->mgrCU=m;
  return;
}

/**Function********************************************************************
  Synopsis     [Sets the one constant]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrSetOne(
  Ddi_Mgr_t *dd            /* source dd manager */,
  Ddi_Bdd_t   *one          /* one constant */
)
{
  dd->one = one;
  return;
}

/**Function********************************************************************
  Synopsis     [Sets the zero constant]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrSetZero(
  Ddi_Mgr_t *dd            /* source dd manager */,
  Ddi_Bdd_t   *zero          /* zero constant */
)
{
  dd->zero = zero;
  return;
}

/**Function********************************************************************
  Synopsis     [Sets the names of variables]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrSetVarnames(
  Ddi_Mgr_t *dd            /* source dd manager */,
  char      **vn           /* names of variables */
)
{

printf("set varnames: funzione da eliminare!\n");
  dd->varnames=vn;
  return;
}

/**Function********************************************************************
  Synopsis     [Sets the auxiliary variable ids]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrSetVarauxids(
  Ddi_Mgr_t *dd            /* source dd manager */,
  int       *va            /* auxiliary variable ids */
)
{
  dd->varauxids=va;
  return;
}

/**Function********************************************************************
  Synopsis     [Returns the threshold for the next dynamic reordering.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrSetDynordThresh(
  Ddi_Mgr_t *dd            /* source dd manager */,
  unsigned int th           /* threshold */
)
{
  Cudd_SetNextReordering(Ddi_MgrReadMgrCU(dd), th);
  return;
}

/*
*********************** Computed Table (Cache) ********************************
*/

/**Function********************************************************************
  Synopsis     [Reads the number of slots in the cache.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
unsigned int
Ddi_ReadCacheSlots(
  Ddi_Mgr_t *dd            /* source dd manager */
)
{
  return Cudd_ReadCacheSlots(Ddi_MgrReadMgrCU(dd));
}

/**Function********************************************************************
  Synopsis     [Returns the number of cache look-ups.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
double
Ddi_ReadCacheLookUps(
  Ddi_Mgr_t *dd            /* dd manager */
)
{
  /* §DM : questo campo non esiste più, ma viene calcolato come
  * somma di altri campi 
  */
  return Cudd_ReadCacheLookUps(Ddi_MgrReadMgrCU(dd));
}

/**Function********************************************************************
  Synopsis     [Returns the number of cache hits.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
double
Ddi_ReadCacheHits(
  Ddi_Mgr_t *dd            /* dd manager */
)
{
  /* §DM : fornisce cacheHits+totCacheHits anziché
  * solo cacheHits previsto dalla define
  */
  return Cudd_ReadCacheHits(Ddi_MgrReadMgrCU(dd));
}

/**Function********************************************************************
  Synopsis     [Reads the hit ratio that causes resizing of the computed
  table.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
unsigned int
Ddi_ReadMinHit(
  Ddi_Mgr_t *dd            /* dd manager */
)
{
  /* §DM : fornisce un'espressione razionale di minHit anziché
  * solo minHit previsto dalla define
  */
  return Cudd_ReadMinHit(Ddi_MgrReadMgrCU(dd));
}

/**Function********************************************************************
  Synopsis     [Reads the maxCacheHard parameter of the manager.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
unsigned int
Ddi_ReadMaxCacheHard(
  Ddi_Mgr_t *dd            /* dd manager */
)
{
  return Cudd_ReadMaxCacheHard(Ddi_MgrReadMgrCU(dd));
}

/**Function********************************************************************
  Synopsis     [Returns the soft limit for the cache size.]
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
unsigned int
Ddi_ReadMaxCache(
  Ddi_Mgr_t* dd            /* dd manager */
)
{
  /* §DM : questo campo non esiste più, ma viene calcolato come
  * somma di altri campi 
  */
  return Cudd_ReadMaxCache( Ddi_MgrReadMgrCU( dd ) );
}



/**Function********************************************************************
  Synopsis     [Print DDI manager allocation statistics]
  Description  [Print DDI manager allocation statistics]
  SideEffects  []
  SeeAlso      []
******************************************************************************/
void
Ddi_MgrPrintAllocStats (
  Ddi_Mgr_t *ddm,
  FILE *fp
)
{
  int i;

  fprintf(fp,"\nDDI Manager alloc statistics (generic)\n");
  fprintf(fp,"allocated nodes       #: %d\n", ddm->allocatedNum);
  fprintf(fp,"free nodes            #: %d\n", ddm->freeNum);
  fprintf(fp,"gen. nodes (all-free) #: %d\n", ddm->genericNum);
  if (ddm->variables != NULL) {
    fprintf(fp,"variables             #: %d\n", 
      Ddi_VararrayNum(ddm->variables));
  }
  fprintf(fp,"const+vararray/groups #: %d\n", DdiMgrReadIntRef(ddm));
  fprintf(fp,"locked                #: %d\n", ddm->lockedNum);
  fprintf(fp,"non locked            #: %d\n", Ddi_MgrReadExtRef(ddm));
  fprintf(fp,"\nDDI Manager alloc statistics (by type)\n");
  for (i=0; i<DDI_NTYPE; i++) {
    fprintf(fp,"%-12s: %3d(ext) = %3d(tot) - %3d(locked)\n", 
      typenames[i],
      ddm->typeNum[i]-ddm->typeLockedNum[i],
      ddm->typeNum[i],ddm->typeLockedNum[i]
    );
  }

}






