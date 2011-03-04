/**CFile***********************************************************************

  FileName    [ddiGeneric.c]

  PackageName [ddi]

  Synopsis    [Functions working on generic DDI type Ddi_Generic_t]

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
  
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
  
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void GenericFreeIntern(Ddi_Generic_t *f);
static Ddi_Generic_t * GenericDupIntern(Ddi_Generic_t *r, Ddi_Generic_t *f);
static Ddi_Generic_t * ArraySupp(Ddi_ArrayData_t *array);
static void ArrayOpIterate(Ddi_OpCode_e opcode, Ddi_ArrayData_t *array, Ddi_Generic_t *g, Ddi_Generic_t *h);
static Ddi_ArrayData_t * GenBddRoots(Ddi_Generic_t *f);
static void GenBddRootsRecur(Ddi_Generic_t *f, Ddi_ArrayData_t *roots);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [Compute generic operation. Result generated]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Generic_t *
Ddi_GenericOp (
  Ddi_OpCode_e   opcode   /* operation code */,
  Ddi_Generic_t  *f       /* first operand */,
  Ddi_Generic_t  *g       /* first operand */,
  Ddi_Generic_t  *h       /* first operand */
)
{
  return(DdiGenericOp(opcode,Ddi_Generate_c,f,g,h));
}

/**Function********************************************************************
  Synopsis    [Compute generic operation. Result accumulated]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Generic_t *
Ddi_GenericOpAcc (
  Ddi_OpCode_e   opcode   /* operation code */,
  Ddi_Generic_t  *f       /* first operand */,
  Ddi_Generic_t  *g       /* first operand */,
  Ddi_Generic_t  *h       /* first operand */
)
{
  return(DdiGenericOp(opcode,Ddi_Accumulate_c,f,g,h));
}

/**Function********************************************************************
  Synopsis    [Generic dup]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Generic_t *
Ddi_GenericDup (
  Ddi_Generic_t  *f
)
{
  return(DdiGenericDup(f));
}

/**Function********************************************************************
  Synopsis    [Lock DDI node.]
  Description [Lock DDI node so that cannot be freed unless unlocked.
    Used as a protection mechanism for internal objects (array entries,
    partitions, ...]
  SideEffects []
  SeeAlso     [Ddi_GenericUnlock]
******************************************************************************/
void
Ddi_GenericLock (
  Ddi_Generic_t *f
) 
{
  if ((f!=NULL)/*&&(DdiType(f)!=Ddi_Var_c)*/) {
    if (f->common.status==Ddi_Locked_c) {
      /* 
       * only variables are may be locked several times as they are not
       * duplicated
       */
      Pdtutil_Assert(DdiType(f)==Ddi_Var_c,
        "Unlocked node expected");
    }
    else {
      f->common.mgr->lockedNum++;
      f->common.mgr->typeLockedNum[DdiType(f)]++;
      f->common.status = Ddi_Locked_c;
    }
  }
}

/**Function********************************************************************
  Synopsis    [Unlock DDI node.]
  Description [Unlock DDI node so that can be freed again.]
  SideEffects []
  SeeAlso     [Ddi_GenericLock]
******************************************************************************/
void
Ddi_GenericUnlock (
  Ddi_Generic_t *f
) 
{
  if ((f!=NULL)/*&&(DdiType(f)!=Ddi_Var_c)*/) {
    Pdtutil_Assert(f->common.status==Ddi_Locked_c,"Locked node expected");
    f->common.mgr->lockedNum--;
    f->common.mgr->typeLockedNum[DdiType(f)]--;
    f->common.status = Ddi_Unlocked_c;
  }
}

/**Function********************************************************************
  Synopsis     [Free the content of a generic DDI node]
  Description  [Free the content of a generic DDI node]
  SideEffects  []
  SeeAlso      []
******************************************************************************/

void
Ddi_GenericFree (
  Ddi_Generic_t *f    /* block to be freed */
)
{
  DdiGenericFree(f);
}

/**Function********************************************************************
  Synopsis     [Set name field of DDI node]
  Description  [Set name field of DDI node]
  SideEffects  []
  SeeAlso      []
******************************************************************************/
void
Ddi_GenericSetName (
  Ddi_Generic_t *f    /* block to be freed */,
  char *name
)
{
  if (f->common.name != NULL) {
    Pdtutil_Free(f->common.name);
  }
  f->common.name = Pdtutil_StrDup(name);
}


/*
 *  Read fields and components
 */

/**Function********************************************************************
  Synopsis    [called through Ddi_ReadCode.]
  SideEffects [Ddi_ReadCode]
******************************************************************************/
Ddi_Code_e
Ddi_GenericReadCode(
  Ddi_Generic_t *f
)
{
  Pdtutil_Assert (f!=NULL, "Accessing NULL DDI node");
  return (f->common.code);
}

/**Function********************************************************************
  Synopsis    [called through Ddi_ReadMgr.]
  SideEffects [Ddi_ReadMgr]
******************************************************************************/
Ddi_Mgr_t *
Ddi_GenericReadMgr (
  Ddi_Generic_t * f
)
{
  Pdtutil_Assert (f!=NULL, "Accessing NULL DDI node");
  return (f->common.mgr);
}

/**Function********************************************************************
  Synopsis    [called through Ddi_ReadName.]
  SideEffects [Ddi_ReadName]
******************************************************************************/
char *
Ddi_GenericReadName (
  Ddi_Generic_t * f
)
{
  Pdtutil_Assert (f!=NULL, "Accessing NULL DDI node");
  return (f->common.name);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************
  Synopsis    [Allocate and initialize a new DDI block]
  Description [Allocation and initialization of generic DDI block.
               owner DDI manager always required.] 
  SideEffects [none]
  SeeAlso     [DdiGenericNew DdiGenericFree]
******************************************************************************/

Ddi_Generic_t *
DdiGenericAlloc (
  Ddi_Type_e type           /* type of allocated object */,
  Ddi_Mgr_t *mgr            /* DDI manager */
)
{
  Ddi_Generic_t *r;

  Pdtutil_Assert (mgr!=NULL, "non NULL DDI manager required for node alloc");

  /*
   *  fields common to all node types
   */

  r = Pdtutil_Alloc(Ddi_Generic_t,1);

  r->common.type = type;
  r->common.code = Ddi_Null_code_c;
  r->common.status = Ddi_Unlocked_c;
  r->common.name = NULL;
  r->common.mgr = mgr;
  r->common.info = NULL;
  r->common.supp = NULL;
  r->common.nodeid = mgr->currNodeId++;

  r->common.next = mgr->nodeList;
  mgr->allocatedNum++;
  mgr->genericNum++;
  mgr->typeNum[type]++;
  mgr->nodeList = (Ddi_Generic_t *) r;

  switch (type) {

  case Ddi_Bdd_c:
    r->Bdd.data.bdd = NULL;
    break;
  case Ddi_Var_c:
    r->Var.data.bdd = NULL;
    r->Var.data.index = -1;
    break;
  case Ddi_Varset_c:
    r->Varset.data.bdd = NULL;
    r->Varset.data.curr = NULL;
    break;
  case Ddi_Expr_c:
    r->Expr.data.bdd = NULL;
    r->Expr.opcode = 0;
    break;
  case Ddi_Bddarray_c:
    r->Bddarray.array = NULL;
    break;
  case Ddi_Vararray_c:
    r->Vararray.array = NULL;
    break;
  case Ddi_Varsetarray_c:
    r->Varsetarray.array = NULL;
    break;
  case Ddi_Exprarray_c:
    r->Exprarray.array = NULL;
    break;
  default:
    Pdtutil_Assert (0, "Wrong DDI node type");
  }

  if ((mgr->tracedId>=0)&&(r->common.nodeid >= mgr->tracedId)) {
    DdiTraceNodeAlloc(r);
  }


  return(r);

}

/**Function*******************************************************************
  Synopsis    [Trace allocation of DDI node]
  Description [Trace allocation of DDI node. Put a debugger breakpoint on this
               routine to watch allocation of a node, given its ID. The id is
               retrieved from a previous run (e.g. logged as non freed DDI 
               node in MgrCheck by Ddi_MgrPrintExtRef.]
  SideEffects [none]
  SeeAlso     [Ddi_MgrSetTracedId Ddi_MgrPrintExtRef]
******************************************************************************/

void
DdiTraceNodeAlloc (
  Ddi_Generic_t *r
)
{
  fprintf (stdout, "DDI node with ID=%d CREATED NOW\n", r->common.nodeid);
}

/**Function********************************************************************

  Synopsis     [Free the content of a generic DDI node]
  Description  [Free the content of a generic DDI node and activates DDI
    manager garbage collection (of DDI handles) if required. ]
  Description  [Frees a generic block.] 
  SideEffects  [none]
  SeeAlso      []

******************************************************************************/

void
DdiGenericFree (
  Ddi_Generic_t *f    /* block to be freed */
)
{
  Ddi_Mgr_t *ddm;  /* dd manager */

  if (f == NULL) {
     /* this may happen with NULL entries in freed arrays */ 
    return;
  }

  ddm = f->common.mgr;    

  GenericFreeIntern(f);

  if (ddm->freeNum > DDI_GARBAGE_THRESHOLD) {
    DdiMgrGarbageCollect(ddm);
  }

}

/**Function********************************************************************

  Synopsis     [Schedule free of a generic block]

  Description  [Schedule free a generic block.] 

  SideEffects  [none]

  SeeAlso      []

******************************************************************************/

Ddi_Generic_t *
DdiDeferredFree (
  Ddi_Generic_t *f    /* block to be freed */
)
{

  Pdtutil_Assert (f!=NULL, "deferred free of NULL DDI node");
  f->common.status = Ddi_SchedFree_c;
  return(f);
}

/**Function********************************************************************
  Synopsis     [Duplicate a DDI node]
  Description  [] 
  SideEffects  []
  SeeAlso      []
******************************************************************************/
Ddi_Generic_t *
DdiGenericDup (
  Ddi_Generic_t *f    /* BDD to be duplicated */ 
  )
{
  Ddi_Generic_t *r;
  Ddi_Mgr_t *ddm; 

  if (f == NULL) {
    return (NULL);
  }

  if (f->common.type == Ddi_Var_c) {
    return (f);
  }

  ddm = f->common.mgr;
  r = DdiGenericAlloc (f->common.type, ddm);

  GenericDupIntern(r,f);

  return(r);
}

/**Function********************************************************************
  Synopsis     [Copy a DDI node to a destination manager]
  Description  [Copy a DDI node to a destination manager]
  SideEffects  []
  SeeAlso      []
******************************************************************************/
Ddi_Generic_t *
DdiGenericCopy (
  Ddi_Mgr_t *ddm           /* destination manager */,
  Ddi_Generic_t *f         /* BDD to be duplicated */,
  Ddi_Vararray_t *varsOld  /* old variable array */,
  Ddi_Vararray_t *varsNew  /* new variable array */
)
{
  Ddi_Generic_t *r;
  Ddi_Mgr_t *ddm0; 

  if (f == NULL) {
    return (NULL);
  }

  ddm0 = f->common.mgr;

  if (varsOld != NULL) {
    Ddi_Vararray_t *varsOld2;
    int nv,i;
    Ddi_Var_t *vOld;

    Pdtutil_Assert(varsNew != NULL,"Missing variable array for var remap");
    nv = Ddi_VararrayNum(varsOld);
    Pdtutil_Assert(nv == Ddi_VararrayNum(varsNew),
      "Var array sizes do not match");

    varsOld2 = Ddi_VararrayAlloc(ddm,nv);
    for (i=0; i<nv; i++) {
      vOld = Ddi_VararrayRead(varsOld,i);
      Ddi_VararrayWrite(varsOld2,i,Ddi_IthVar(ddm,Ddi_VarIndex(vOld)));
    }
    r = DdiGenericCopy(ddm,f,NULL,NULL);
    Ddi_BddSubstVarsAcc ((Ddi_Bdd_t *)r,varsOld2,varsNew);
    return(r);
  }

  /* Same manager: call dup */
  if (ddm == ddm0) {
    return (DdiGenericDup(f));
  }

  r = DdiGenericAlloc (f->common.type, ddm);
  r->common.type = f->common.type;
  r->common.code = f->common.code; 
  r->common.status = Ddi_Unlocked_c;
  r->common.name = Pdtutil_StrDup(f->common.name); 
  r->common.supp = NULL;

  switch (f->common.type) {

  case Ddi_Bdd_c:
    switch (f->common.code) {
    case Ddi_Bdd_Mono_c:
      r->Bdd.data.bdd = Cudd_bddTransfer(ddm0->mgrCU,ddm->mgrCU,
        f->Bdd.data.bdd);
      Cudd_Ref(r->Bdd.data.bdd);
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
        r->Bdd.data.part = DdiArrayCopy(ddm,f->Bdd.data.part);
      break;
    case Ddi_Bdd_Meta_c:
        r->Bdd.data.meta = DdiMetaCopy(ddm,f->Bdd.data.meta);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;
  case Ddi_Var_c:
    DdiGenericFree(r->common.supp);
    r->Var.data.bdd = NULL;
    DdiGenericFree(r);
    r = (Ddi_Generic_t *)Ddi_IthVar(ddm,Ddi_VarIndex((Ddi_Var_t *)f));
    break;
  case Ddi_Varset_c:
    r->Varset.data.bdd = 
      Cudd_bddTransfer(ddm0->mgrCU,ddm->mgrCU,f->Varset.data.bdd);
    Cudd_Ref(r->Varset.data.bdd);
    break;
  case Ddi_Expr_c:
    switch (f->common.code) {
    case Ddi_Expr_Bdd_c:
      r->Expr.data.bdd = DdiGenericCopy(ddm,f->Expr.data.bdd,NULL,NULL);
      break;
    case Ddi_Expr_String_c:
      r->Expr.data.string = Pdtutil_StrDup(f->Expr.data.string);
      break;
    case Ddi_Expr_Bool_c:
    case Ddi_Expr_Ctl_c:
      r->Expr.data.sub = DdiArrayCopy(ddm,f->Expr.data.sub);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;
  case Ddi_Bddarray_c:
  case Ddi_Vararray_c:
  case Ddi_Varsetarray_c:
  case Ddi_Exprarray_c:
    r->Bddarray.array = DdiArrayCopy(ddm,f->Bddarray.array);
    break;
  default:
    Pdtutil_Assert (0, "Wrong DDI node type");
  }

  return(r);
}


/**Function********************************************************************
  Synopsis     [Copy the content of a DDI node to another one]
  Description  [Copy the content of a DDI node to another one.
    Freing is done before copy. Data are duplicated]
  SideEffects  []
  SeeAlso      []
******************************************************************************/

void
DdiGenericDataCopy (
  Ddi_Generic_t *d    /* destination */,
  Ddi_Generic_t *s    /* source */
)
{
  Ddi_Mgr_t *ddm;  /* dd manager */
  int locked=0;

  ddm = s->common.mgr;    
  
  if (d->common.status==Ddi_Locked_c) {
    locked = 1;
    Ddi_Unlock(d);
  }

  GenericFreeIntern(d);

  /* resume block */
  ddm->genericNum++;
  ddm->freeNum--;
  d->common.status = Ddi_Unlocked_c;

  GenericDupIntern(d,s);


  if (locked) {
    Ddi_Lock(d);
  }

}

/**Function********************************************************************
  Synopsis    [Compute operation]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Generic_t *
DdiGenericOp (
  Ddi_OpCode_e   opcode   /* operation code */,
  Ddi_OpType_e   optype   /* operation type (accumulate/generate) */,
  Ddi_Generic_t  *f       /* first operand */,
  Ddi_Generic_t  *g       /* first operand */,
  Ddi_Generic_t  *h       /* first operand */
)
{
  int i;
  DdNode *rCU = NULL;                 /* CUDD result */
  Ddi_Generic_t *r=NULL;              /* result */
  Ddi_Generic_t *tmp;
  Ddi_BddMgr *ddm;                      /* dd manager */
  DdManager *mgrCU;                    /* CUDD manager */
  int attachSupport = 0;

  Pdtutil_Assert (f!=NULL, "NULL first operand of DDI operation");

  ddm = f->common.mgr;
  mgrCU = Ddi_MgrReadMgrCU(ddm);

  switch (optype) {
  case Ddi_Accumulate_c:
    r = f;
    if (r->common.supp != NULL) {
      /* support must be recomputed after operation */
      attachSupport = 1;
    }
    if ((g!=NULL)&&(g->common.code==Ddi_Bdd_Meta_c)
      &&(r->common.code==Ddi_Bdd_Mono_c)) {
      tmp = DdiGenericOp(opcode,Ddi_Generate_c,g,r,h);
      DdiGenericDataCopy(r,tmp);
      DdiGenericFree(tmp);
      return(r);
    }
    break;
  case Ddi_Generate_c:
    /* swap f and g if g is meta */
    if ((g!=NULL)&&(g->common.code==Ddi_Bdd_Meta_c)
	&&(f->common.code==Ddi_Bdd_Mono_c)) {
      tmp = g; g = f; f = tmp; 
    }
    r = DdiGenericDup(f);
    break;
  default:
    Pdtutil_Assert (0, 
      "Wrong DDI operation type (accumulate/generate accepted)");
  }

  /*
   * DdArray operations are iterated on all array items
   */
  if ((DdiType(f)==Ddi_Bddarray_c) &&
      (opcode != Ddi_BddSupp_c)) {
    ArrayOpIterate(opcode,r->Bddarray.array,g,h);
    return(r);
  }

  switch (opcode) {

  /*
   *  Boolean operators
   */

  case Ddi_BddNot_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      /* no ref/deref required */
      r->Bdd.data.bdd = Cudd_Not(r->Bdd.data.bdd);
      break;
    case Ddi_Bdd_Part_Conj_c:
      r->common.code = Ddi_Bdd_Part_Disj_c;
      ArrayOpIterate(opcode,r->Bdd.data.part,NULL,NULL);
      break;
    case Ddi_Bdd_Part_Disj_c:
      r->common.code = Ddi_Bdd_Part_Conj_c;
      ArrayOpIterate(opcode,r->Bdd.data.part,NULL,NULL);
      break;
    case Ddi_Bdd_Meta_c:
      DdiMetaDoCompl(r->Bdd.data.meta);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }

    break;

  case Ddi_BddAnd_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Bdd_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      rCU = Cudd_bddAnd(mgrCU,r->Bdd.data.bdd,g->Bdd.data.bdd);
      if (rCU != NULL) {
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      else {
        Pdtutil_Assert(ddm->abortOnSift,
          "NULL result only allowed with abortOnSift set");
        ddm->abortedOp = 1;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
      /* AND is applied to first partition */ 
      (void *)DdiGenericOp(opcode,Ddi_Accumulate_c,
        DdiArrayRead(r->Bdd.data.part,0),g,NULL);
      /* 
       * partitions sharing support vars with g are moved at first positions 
       */
      if (0)
      {
        Ddi_ArrayData_t *a = r->Bdd.data.part;
        Ddi_Generic_t *p;
        Ddi_Varset_t *suppg, *suppp, *common;
        suppg = Ddi_BddSupp((Ddi_Bdd_t *)g);
        for (i=DdiArrayNum(a)-1;i>=0;i--) {
          suppp = Ddi_BddSupp((Ddi_Bdd_t *)DdiArrayRead(a,i));
          common = Ddi_VarsetIntersect(suppg,suppp);
          if (!Ddi_VarsetIsVoid(common)) {
            p = DdiArrayExtract(a,i);
            DdiArrayInsert(a,0,p,Ddi_Mov_c);
	  }
          Ddi_Free(suppp);
          Ddi_Free(common);
	}
        Ddi_Free(suppg);
      }
      break;
    case Ddi_Bdd_Part_Disj_c:
      /* AND is distributed over disjunction */ 
      ArrayOpIterate(opcode,r->Bdd.data.part,g,NULL);
      break;
    case Ddi_Bdd_Meta_c:
      DdiMetaAndAcc((Ddi_Bdd_t *)r,(Ddi_Bdd_t *)g);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  case Ddi_BddDiff_c:
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,g,NULL,NULL);
    (void *)DdiGenericOp(Ddi_BddAnd_c,Ddi_Accumulate_c,r,g,NULL);
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,g,NULL,NULL);
    break;

  case Ddi_BddOr_c:
    /* bring to And operator using DeMorgan transformation */
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,r,NULL,NULL);
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,g,NULL,NULL);

    (void *)DdiGenericOp(Ddi_BddAnd_c,Ddi_Accumulate_c,r,g,NULL);

    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,r,NULL,NULL);
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,g,NULL,NULL);
    break;

  case Ddi_BddXor_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Bdd_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      rCU = Cudd_bddXor(mgrCU,r->Bdd.data.bdd,g->Bdd.data.bdd);
      Cudd_Ref(rCU);
      Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
      r->Bdd.data.bdd = rCU;
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
    case Ddi_Bdd_Meta_c:
      /* XOR not allowed with partitioned and meta BDDs */
      Pdtutil_Warning (1,
        "XOR not supported with partitioned/meta BDDs\nTransformed to mono"); 
     (void *)DdiGenericOp(Ddi_BddMakeMono_c,Ddi_Accumulate_c,r,NULL,NULL);
     (void *)DdiGenericOp(opcode,Ddi_Accumulate_c,r,g,NULL);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  case Ddi_BddNand_c:
    (void *)DdiGenericOp(Ddi_BddAnd_c,Ddi_Accumulate_c,r,g,NULL);
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,r,NULL,NULL);
    break;

  case Ddi_BddNor_c:
    (void *)DdiGenericOp(Ddi_BddOr_c,Ddi_Accumulate_c,r,g,NULL);
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,r,NULL,NULL);
    break;

  case Ddi_BddXnor_c:
    (void *)DdiGenericOp(Ddi_BddXor_c,Ddi_Accumulate_c,r,g,NULL);
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,r,NULL,NULL);
    break;

  case Ddi_BddIte_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Bdd_c);
    DdiConsistencyCheck(h,Ddi_Bdd_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      rCU = Cudd_bddIte(mgrCU,r->Bdd.data.bdd,g->Bdd.data.bdd,h->Bdd.data.bdd);
      if (rCU != NULL) {
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      else {
        Pdtutil_Assert(ddm->abortOnSift,
          "NULL result only allowed with abortOnSift set");
        ddm->abortedOp = 1;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
    case Ddi_Bdd_Meta_c:
      /* ITE not allowed with partitioned and meta BDDs */
      Pdtutil_Warning (1,
        "ITE not supported with partitioned/meta BDDs\nTransformed to mono");
      (void *)DdiGenericOp(Ddi_BddMakeMono_c,Ddi_Accumulate_c,r,NULL,NULL);
      (void *)DdiGenericOp(opcode,Ddi_Accumulate_c,r,g,h);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  /*
   *  Generalized cofactors
   */

  case Ddi_BddConstrain_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Bdd_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      rCU = Cudd_bddConstrain(mgrCU,r->Bdd.data.bdd,g->Bdd.data.bdd);
      if (rCU != NULL) {
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      else {
        Pdtutil_Assert(ddm->abortOnSift,
          "NULL result only allowed with abortOnSift set");
        ddm->abortedOp = 1;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      ArrayOpIterate(opcode,r->Bdd.data.part,g,NULL);
      break;
    case Ddi_Bdd_Meta_c:
      r->Bdd.data.meta = DdiMetaConstrain(r->Bdd.data.meta,g->Bdd.data.meta);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  case Ddi_BddRestrict_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Bdd_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      rCU = Cudd_bddRestrict(mgrCU,r->Bdd.data.bdd,g->Bdd.data.bdd);
      Cudd_Ref(rCU);
      Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
      r->Bdd.data.bdd = rCU;
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      ArrayOpIterate(opcode,r->Bdd.data.part,g,NULL);
      break;
    case Ddi_Bdd_Meta_c:
      r->Bdd.data.meta = DdiMetaRestrict(r->Bdd.data.meta,g->Bdd.data.meta);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  /*
   *  Quantifiers
   */

  case Ddi_BddForall_c:
    /* bring to And operator using DeMorgan transformation */
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,r,NULL,NULL);
    (void *)DdiGenericOp(Ddi_BddExist_c,Ddi_Accumulate_c,r,g,NULL);
    (void *)DdiGenericOp(Ddi_BddNot_c,Ddi_Accumulate_c,r,NULL,NULL);
    break;

  case Ddi_BddExist_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Varset_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      rCU = Cudd_bddExistAbstract(mgrCU,r->Bdd.data.bdd,g->Varset.data.bdd);
      if (rCU != NULL) {
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      else {
        Pdtutil_Assert(ddm->abortOnSift,
          "NULL result only allowed with abortOnSift set");
        ddm->abortedOp = 1;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
      tmp = (Ddi_Generic_t *)
        Part_BddMultiwayLinearAndExist((Ddi_Bdd_t*)r,(Ddi_Varset_t*)g,
          ddm->settings.part.existClustThresh);
      /* result data is copied to the *r node */
      DdiGenericDataCopy(r,tmp);
      DdiGenericFree(tmp);
      if (Ddi_BddPartNum((Ddi_Bdd_t *)r)==1) {
        Ddi_BddSetMono((Ddi_Bdd_t *)r);
      }
      break;
    case Ddi_Bdd_Part_Disj_c:
      /* EXIST is distributed over disjunction */ 
      ArrayOpIterate(opcode,r->Bdd.data.part,g,NULL);
      break;
    case Ddi_Bdd_Meta_c:
      DdiMetaAndExistAcc((Ddi_Bdd_t *)r,NULL,(Ddi_Varset_t *)g);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  case Ddi_BddAndExist_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Bdd_c);
    DdiConsistencyCheck(h,Ddi_Varset_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c: 
      rCU = Cudd_bddAndAbstract(mgrCU,
        r->Bdd.data.bdd,g->Bdd.data.bdd,h->Varset.data.bdd);
      if (rCU != NULL) {
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      else {
        Pdtutil_Assert(ddm->abortOnSift,
          "NULL result only allowed with abortOnSift set");
        ddm->abortedOp = 1;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
      DdiArrayInsert(r->Bdd.data.part,0,g,Ddi_Dup_c);
      (void *)DdiGenericOp(Ddi_BddExist_c,Ddi_Accumulate_c,r,h,NULL);
      break;
    case Ddi_Bdd_Part_Disj_c:
      /* AND-EXIST is distributed over disjunction */ 
      ArrayOpIterate(opcode,r->Bdd.data.part,g,h);
      break;
    case Ddi_Bdd_Meta_c:
      DdiMetaAndExistAcc((Ddi_Bdd_t *)r,(Ddi_Bdd_t *)g,(Ddi_Varset_t *)h);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  /*
   *  Compatible projector
   */

  case Ddi_BddCproject_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Bdd_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      rCU = Cudd_CProjection(mgrCU,r->Bdd.data.bdd,g->Bdd.data.bdd);
      Cudd_Ref(rCU);
      Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
      r->Bdd.data.bdd = rCU;
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
    case Ddi_Bdd_Meta_c:
      /* CPROJECT not allowed with partitioned and meta BDDs */
      Pdtutil_Warning (1,
        "CPROJECT not supported with part/meta BDDs\nTransformed to mono");
      (void *)DdiGenericOp(Ddi_BddMakeMono_c,Ddi_Accumulate_c,r,NULL,NULL);
      (void *)DdiGenericOp(opcode,Ddi_Accumulate_c,r,g,h);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  /*
   *  Composition and variable substitution
   */

  case Ddi_BddCompose_c:
    DdiConsistencyCheck(r,Ddi_Bdd_c);
    DdiConsistencyCheck(g,Ddi_Vararray_c);
    DdiConsistencyCheck(h,Ddi_Bddarray_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      {
        DdNode **Fv;
        int n, nMgrVars, j, id;

        nMgrVars = Ddi_ReadNumVars(ddm);
        /* the length of the two arrays must be the same */
        n = DdiArrayNum(g->Vararray.array);
        Pdtutil_Assert(n==DdiArrayNum(h->Bddarray.array),
          "different length of var/func arrays in compose");
        Fv = Pdtutil_Alloc(DdNode *,nMgrVars);
        for (j=0;j<nMgrVars;j++) {
           Fv[j] = Ddi_VarToCU(Ddi_IthVar(ddm,j));
	}
        for (j=0;j<n;j++) {
           id = Ddi_VarIndex(Ddi_VararrayRead((Ddi_Vararray_t *)g,j));
           Fv[id] = Ddi_BddToCU(Ddi_BddarrayRead((Ddi_Bddarray_t *)h,j));
	}
        rCU = Cudd_bddVectorCompose(mgrCU,r->Bdd.data.bdd,Fv);
        Pdtutil_Free(Fv);
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      ArrayOpIterate(opcode,r->Bdd.data.part,g,h);
      break;
    case Ddi_Bdd_Meta_c:
      DdiMetaComposeAcc((Ddi_Bdd_t *)r,
        (Ddi_Vararray_t*)g,(Ddi_Bddarray_t*)h);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;


  case Ddi_BddSwapVars_c:
  case Ddi_VarsetSwapVars_c:
    DdiConsistencyCheck2(r,Ddi_Bdd_c,Ddi_Varset_c);
    DdiConsistencyCheck(g,Ddi_Vararray_c);
    DdiConsistencyCheck(h,Ddi_Vararray_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      {
        DdNode **Xv,**Yv;
        int n;

        /* the length of the two arrays of variables must be the same */
        n = DdiArrayNum(g->Vararray.array);
        Pdtutil_Assert(n==DdiArrayNum(h->Vararray.array),
          "different length of var arrays in var swap");
        Xv = Ddi_VararrayToCU((Ddi_Vararray_t *)g);
        Yv = Ddi_VararrayToCU((Ddi_Vararray_t *)h);
        rCU = Cudd_bddSwapVariables(mgrCU,r->Bdd.data.bdd,Xv,Yv,n);
        Pdtutil_Free(Xv);
        Pdtutil_Free(Yv);
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      ArrayOpIterate(opcode,r->Bdd.data.part,g,h);
      break;
    case Ddi_Bdd_Meta_c:
      DdiMetaSwapVarsAcc((Ddi_Bdd_t *)r,
        (Ddi_Vararray_t*)g,(Ddi_Vararray_t*)h);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;


  case Ddi_BddSubstVars_c:
  case Ddi_VarsetSubstVars_c:
    DdiConsistencyCheck2(r,Ddi_Bdd_c,Ddi_Varset_c);
    DdiConsistencyCheck(g,Ddi_Vararray_c);
    DdiConsistencyCheck(h,Ddi_Vararray_c);
    switch (r->common.code) {
    case Ddi_Bdd_Mono_c:
      {
        DdNode **Fv;
        int n, nMgrVars, j, id;

        nMgrVars = Ddi_ReadNumVars(ddm);
        /* the length of the two arrays must be the same */
        n = DdiArrayNum(g->Vararray.array);
        Pdtutil_Assert(n==DdiArrayNum(h->Vararray.array),
          "different length of var arrays in var subst");
        Fv = Pdtutil_Alloc(DdNode *,nMgrVars);
        for (j=0;j<nMgrVars;j++) {
           Fv[j] = Ddi_VarToCU(Ddi_IthVar(ddm,j));
	}
        for (j=0;j<n;j++) {
           id = Ddi_VarIndex(Ddi_VararrayRead((Ddi_Vararray_t *)g,j));
           Fv[id] = Ddi_VarToCU(Ddi_VararrayRead((Ddi_Vararray_t *)h,j));
	}
        rCU = Cudd_bddVectorCompose(mgrCU,r->Bdd.data.bdd,Fv);
        Pdtutil_Free(Fv);
        Cudd_Ref(rCU);
        Cudd_RecursiveDeref (mgrCU, r->Bdd.data.bdd);
        r->Bdd.data.bdd = rCU;
      }
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      ArrayOpIterate(opcode,r->Bdd.data.part,g,h);
      break;
    case Ddi_Bdd_Meta_c:
      DdiMetaSubstVarsAcc((Ddi_Bdd_t *)r,
        (Ddi_Vararray_t*)g,(Ddi_Vararray_t*)h);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    break;

  /*
   *  Support handling
   */

  case Ddi_BddSupp_c:
    Pdtutil_Assert(optype==Ddi_Generate_c,
      "support computation must be a generate function");
    DdiGenericFree(r);
    if (f->common.supp!=NULL) {
      return(DdiGenericDup(f->common.supp));
    }
    switch (f->common.type) {
    case Ddi_Bdd_c: 
      switch (f->common.code) {
      case Ddi_Bdd_Mono_c:
        r = (Ddi_Generic_t *) Ddi_VarsetMakeFromCU(ddm,
          Cudd_Support(mgrCU,f->Bdd.data.bdd));    
        break;
      case Ddi_Bdd_Part_Conj_c:
      case Ddi_Bdd_Part_Disj_c:
        if (DdiArrayNum(f->Bdd.data.part)==0) {
          r = (Ddi_Generic_t *)Ddi_VarsetVoid(ddm);
        }
        else { 
          r = ArraySupp(f->Bdd.data.part);
        }
        break;
      case Ddi_Bdd_Meta_c:
        r = (Ddi_Generic_t *) DdiMetaSupp(f->Bdd.data.meta);
        break;
      default:
        Pdtutil_Assert(0,"Wrong DDI node code");
      break;
      }
      break;
    case Ddi_Bddarray_c: 
      r = ArraySupp(f->Bddarray.array);
      break;
    case Ddi_Var_c: 
      r = (Ddi_Generic_t *)Ddi_VarsetMakeFromCU(ddm,f->Var.data.bdd);    
      break;
    case Ddi_Vararray_c: 
      r = ArraySupp(f->Vararray.array);
      break;
    case Ddi_Varset_c: 
      r = (Ddi_Generic_t *) Ddi_VarsetMakeFromCU(ddm,
        Cudd_Support(mgrCU,f->Bdd.data.bdd));    
      break;
    case Ddi_Varsetarray_c: 
      r = ArraySupp(f->Varsetarray.array);
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;

  case Ddi_BddSuppAttach_c:
    Pdtutil_Assert(optype==Ddi_Accumulate_c,
      "support computation must be an accumulate function");
    /* support computed after all operations */
    attachSupport = 1;
    break;
  case Ddi_BddSuppDetach_c:
    Pdtutil_Assert(optype==Ddi_Accumulate_c,
      "support detach must be an accumulate function");
    Ddi_Unlock(r->common.supp);
    DdiGenericFree(r->common.supp);
    r->common.supp = NULL;
    break;

  /*
   *  Variable set operations
   */

  case Ddi_VarsetNext_c:
    DdiConsistencyCheck(r,Ddi_Varset_c);
    rCU = Cudd_T(r->Varset.data.bdd);
    Cudd_Ref(rCU);
    Cudd_RecursiveDeref (mgrCU, r->Varset.data.bdd);
    r->Varset.data.bdd = rCU;
    break;
  case Ddi_VarsetUnion_c:
  case Ddi_VarsetAdd_c:
    DdiConsistencyCheck(r,Ddi_Varset_c);
    DdiConsistencyCheck2(g,Ddi_Var_c,Ddi_Varset_c);
    if (g->common.type==Ddi_Var_c) {
      rCU = Cudd_bddAnd(mgrCU,r->Varset.data.bdd,g->Var.data.bdd);
    }
    else {
      rCU = Cudd_bddAnd(mgrCU,r->Varset.data.bdd,g->Varset.data.bdd);
    }
    Cudd_Ref(rCU);
    Cudd_RecursiveDeref (mgrCU, r->Varset.data.bdd);
    r->Varset.data.bdd = rCU;
    break;
  case Ddi_VarsetIntersect_c:
    DdiConsistencyCheck(r,Ddi_Varset_c);
    DdiConsistencyCheck2(g,Ddi_Var_c,Ddi_Varset_c);
    if (g->common.type==Ddi_Var_c) {
      rCU = Cudd_bddLiteralSetIntersection(mgrCU,
        r->Varset.data.bdd,g->Var.data.bdd);
    }
    else {
      rCU = Cudd_bddLiteralSetIntersection(mgrCU,
        r->Varset.data.bdd,g->Varset.data.bdd);
    }
    Cudd_Ref(rCU);
    Cudd_RecursiveDeref (mgrCU, r->Varset.data.bdd);
    r->Varset.data.bdd = rCU;
    break;
  case Ddi_VarsetDiff_c:
  case Ddi_VarsetRemove_c:
    DdiConsistencyCheck(r,Ddi_Varset_c);
    DdiConsistencyCheck2(g,Ddi_Var_c,Ddi_Varset_c);
    if (g->common.type==Ddi_Var_c) {
      rCU = Cudd_bddExistAbstract(mgrCU,r->Varset.data.bdd,g->Var.data.bdd);
    }
    else {
      rCU = Cudd_bddExistAbstract(mgrCU,r->Varset.data.bdd,g->Varset.data.bdd);
    }
    Cudd_Ref(rCU);
    Cudd_RecursiveDeref (mgrCU, r->Varset.data.bdd);
    r->Varset.data.bdd = rCU;
    break;

  default:
    Pdtutil_Assert(0,"Unknown operation code in generic op");
    break;
  }

  if (attachSupport) {
    if (r->common.supp != NULL) {
      Ddi_Unlock(r->common.supp);
      DdiGenericFree(r->common.supp);
      r->common.supp = NULL;
    }
    r->common.supp = DdiGenericOp(Ddi_BddSupp_c,Ddi_Generate_c,r,NULL,NULL);
    Ddi_Lock(r->common.supp);
  }


  return (r);
}

/**Function********************************************************************
  Synopsis     [Compute BDD size]
  Description  [Compute BDD size. Sharing size is used, so shared nodes are 
    counted only once.] 
  SideEffects  []
  SeeAlso      []
******************************************************************************/
int
DdiGenericBddSize (
  Ddi_Generic_t *f    /* BDD to be duplicated */ 
  )
{
  Ddi_ArrayData_t *roots;
  DdNode **data;
  int size;

  /*
   *  generate an array of monolithic BDDs from CUDD BDD roots
   */
  roots = GenBddRoots(f);

  /*
   *  convert to CUDD array, call CUDD
   */
  data = DdiArrayToCU (roots);
  size = Cudd_SharingSize (data, DdiArrayNum (roots));

  Pdtutil_Free (data);
  DdiArrayFree (roots);

  return(size);
}

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis     [Frees a generic block]
  Description  [Frees a generic block. Internal procedure] 
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
static void
GenericFreeIntern (
  Ddi_Generic_t *f    /* block to be freed */
)
{
  Ddi_Mgr_t *ddm;  /* dd manager */

  if (f == NULL) {
     /* this may happen with NULL entries in freed arrays */ 
    return;
  }

  ddm = f->common.mgr;    

  Pdtutil_Assert (ddm!=NULL, "DDI with NULL DDI manager");
  Pdtutil_Assert (f->common.status==Ddi_Unlocked_c, 
    "Wrong status for freed node. Must be unlocked");

  Ddi_Unlock(f->common.supp);
  GenericFreeIntern(f->common.supp);

  switch (f->common.type) {

  case Ddi_Bdd_c:
    switch (f->common.code) {
    case Ddi_Bdd_Mono_c:
      Cudd_RecursiveDeref (ddm->mgrCU, f->Bdd.data.bdd);
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
        DdiArrayFree(f->Bdd.data.part);
      break;
    case Ddi_Bdd_Meta_c:
        DdiMetaFree(f->Bdd.data.meta);
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;
  case Ddi_Var_c:
    if (f->Var.data.bdd != NULL) {
      Cudd_RecursiveDeref (ddm->mgrCU, f->Var.data.bdd);
    }
    break;
  case Ddi_Varset_c:
    Pdtutil_Assert (f->common.code==Ddi_Bdd_Mono_c, 
      "Variable set must be a monolitic BDD");
    Cudd_RecursiveDeref (ddm->mgrCU, f->Varset.data.bdd);
    break;
  case Ddi_Expr_c:
    switch (f->common.code) {
    case Ddi_Expr_Bdd_c:
      Ddi_Unlock(f->Expr.data.bdd);
      GenericFreeIntern(f->Expr.data.bdd);
      break;
    case Ddi_Expr_String_c:
      Pdtutil_Free(f->Expr.data.string);
      break;
    case Ddi_Expr_Bool_c:
    case Ddi_Expr_Ctl_c:
      DdiArrayFree(f->Expr.data.sub);
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;
  case Ddi_Bddarray_c:
  case Ddi_Vararray_c:
  case Ddi_Varsetarray_c:
  case Ddi_Exprarray_c:
    DdiArrayFree(f->Bddarray.array);
    break;
  default:
    Pdtutil_Assert (0, "Wrong DDI node type");
  }

  ddm->genericNum--;
  ddm->freeNum++;
  ddm->typeNum[f->common.type]--;
  f->common.status = Ddi_Free_c;

}

/**Function********************************************************************
  Synopsis     [Duplicate a DDI node]
  Description  [] 
  SideEffects  []
  SeeAlso      []
******************************************************************************/
static Ddi_Generic_t *
GenericDupIntern (
  Ddi_Generic_t *r    /* destination */,
  Ddi_Generic_t *f    /* source */ 
  )
{
  Ddi_Mgr_t *ddm; 

  ddm = f->common.mgr;

  r->common.type = f->common.type;
  r->common.code = f->common.code; 
  r->common.status = Ddi_Unlocked_c;
  r->common.name = Pdtutil_StrDup(f->common.name); 
  r->common.supp = NULL;
  Ddi_Lock(r->common.supp);

  switch (f->common.type) {

  case Ddi_Bdd_c:
    switch (f->common.code) {
    case Ddi_Bdd_Mono_c:
      r->Bdd.data.bdd = f->Bdd.data.bdd;
      Cudd_Ref(r->Bdd.data.bdd);
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      r->Bdd.data.part = DdiArrayDup(f->Bdd.data.part);
      break;
    case Ddi_Bdd_Meta_c:
      r->Bdd.data.meta = DdiMetaDup(f->Bdd.data.meta);
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;
  case Ddi_Var_c:
    Pdtutil_Assert (0, "Variables cannot be duplicated");
    break;
  case Ddi_Varset_c:
    Pdtutil_Assert (f->common.code==Ddi_Bdd_Mono_c, 
      "Variable set must be a monolitic BDD");
    r->Varset.data.bdd = f->Varset.data.bdd;
    Cudd_Ref(r->Varset.data.bdd);
    break;
  case Ddi_Expr_c:
    switch (f->common.code) {
    case Ddi_Expr_Bdd_c:
      r->Expr.data.bdd = DdiGenericDup(f->Expr.data.bdd);
      Ddi_Lock(r->Expr.data.bdd);
      break;
    case Ddi_Expr_String_c:
      r->Expr.data.string = Pdtutil_StrDup(f->Expr.data.string);
      break;
    case Ddi_Expr_Bool_c:
    case Ddi_Expr_Ctl_c:
      r->Expr.data.sub = DdiArrayDup(f->Expr.data.sub);
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;
  case Ddi_Bddarray_c:
  case Ddi_Vararray_c:
  case Ddi_Varsetarray_c:
  case Ddi_Exprarray_c:
    r->Bddarray.array = DdiArrayDup(f->Bddarray.array);
    break;
  default:
    Pdtutil_Assert (0, "Wrong DDI node type");
  }

  return(r);
}


/**Function*******************************************************************
  Synopsis    [Iterate operation on array entries (accumulate mode used)]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
static Ddi_Generic_t *
ArraySupp (
  Ddi_ArrayData_t *array
)
{
  Ddi_Generic_t *supp, *totsupp, *f;
  int i;

  f = DdiArrayRead(array,0);
  totsupp = DdiGenericOp1(Ddi_BddSupp_c,Ddi_Generate_c,f);

  for (i=1;i<DdiArrayNum(array);i++) {
    f = DdiArrayRead(array,i);
    supp = DdiGenericOp1(Ddi_BddSupp_c,Ddi_Generate_c,f);
    (void *)DdiGenericOp2(Ddi_VarsetUnion_c,Ddi_Accumulate_c,totsupp,supp);
    DdiGenericFree(supp);
  }

  return (totsupp);
}

/**Function*******************************************************************
  Synopsis    [Iterate operation on array entries (accumulate mode used)]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
static void
ArrayOpIterate (
  Ddi_OpCode_e   opcode   /* operation code */,
  Ddi_ArrayData_t *array   /* array */,
  Ddi_Generic_t  *g       /* first operand */,
  Ddi_Generic_t  *h       /* first operand */
)
{
  int i;
  for (i=0;i<DdiArrayNum(array);i++) {
    (void *)DdiGenericOp(opcode,Ddi_Accumulate_c,DdiArrayRead(array,i),g,h);
  }
}


/**Function********************************************************************
  Synopsis     [Generate root pointers of leave BDDs for partitioned DDs]
  Description  [Recursively visits a partitioned Dd structure and builds
                the array of leaves bdds. The array is used by functions
                counting BDD nodes, computing support, printing/storing]
  SideEffects  [none]
  SeeAlso      [GenPartRootsRecur]
******************************************************************************/
static Ddi_ArrayData_t *
GenBddRoots(
  Ddi_Generic_t  *f
)
{
  Ddi_ArrayData_t *roots;

  roots = DdiArrayAlloc(0);
  GenBddRootsRecur(f, roots);

  return (roots);
}
 

/**Function********************************************************************
  Synopsis     [Recursive step of root pointers generation]
  SideEffects  [none]
  SeeAlso      [GenBddRoots]
******************************************************************************/
static void
GenBddRootsRecur(
  Ddi_Generic_t  *f,
  Ddi_ArrayData_t *roots 
)
{
  int i;

  if (f==NULL)
    return;

  switch (f->common.type) {

  case Ddi_Bdd_c:
    switch (f->common.code) {
    case Ddi_Bdd_Mono_c:
      DdiArrayWrite(roots,DdiArrayNum(roots),f,Ddi_Dup_c);
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      for (i=0;i<DdiArrayNum(f->Bdd.data.part);i++) {
        GenBddRootsRecur(DdiArrayRead(f->Bdd.data.part,i),roots);
      }
      break;
    case Ddi_Bdd_Meta_c:
      DdiArrayAppend(roots,f->Bdd.data.meta->one->array);
      DdiArrayAppend(roots,f->Bdd.data.meta->zero->array);
      DdiArrayAppend(roots,f->Bdd.data.meta->dc->array);
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;
  case Ddi_Var_c:
  case Ddi_Varset_c:
    DdiArrayWrite(roots,DdiArrayNum(roots),f,Ddi_Dup_c);
    break;
  case Ddi_Expr_c:
    switch (f->common.code) {
    case Ddi_Expr_Bdd_c:
      DdiArrayWrite(roots,DdiArrayNum(roots),f,Ddi_Dup_c);
      break;
    case Ddi_Expr_String_c:
      break;
    case Ddi_Expr_Bool_c:
    case Ddi_Expr_Ctl_c:
      for (i=0;i<DdiArrayNum(f->Expr.data.sub);i++) {
	GenBddRootsRecur(DdiArrayRead(f->Expr.data.sub,i),roots);
      }
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;
  case Ddi_Bddarray_c:
  case Ddi_Vararray_c:
  case Ddi_Varsetarray_c:
  case Ddi_Exprarray_c:
    for (i=0;i<DdiArrayNum(f->Bddarray.array);i++) {
      GenBddRootsRecur(DdiArrayRead(f->Bddarray.array,i),roots);
    }
    break;
  default:
    Pdtutil_Assert (0, "Wrong DDI node type");

  }

}





