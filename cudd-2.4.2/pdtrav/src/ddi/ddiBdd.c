/**CFile***********************************************************************

  FileName    [ddiBdd.c]

  PackageName [ddi]

  Synopsis    [Functions working on Boolean functions (Ddi_Bdd_t)]

  Description [Functions working on Boolean functions represented by the 
    Ddi_Bdd_t type. Type Ddi_Bdd_t is used for BDDs in monolitic, 
    partitioned (conjunctive/disjunctive) and metaBDD formats.
    Internally, functions are implemented using handles (wrappers) pointing
    to BDD roots. Externally, they are accessed only by pointers.
    Type Ddi_Bdd_t is cast to Ddi_Generic_t (generic DDI node) for internal
    operations.
    All the results obtained by operations are <B>implicitly</B> 
    allocated or referenced (CUDD nodes), so explicit freeing is required.<br>
    External procedures in this module include
    <ul> 
    <li> Basic Boolean operations: And, Or, ..., ITE
    <li> Specific BDD operators: Constrain, Restrict
    <li> Quantification operators: Exist, And-Exist (relational product)
    <li> Comparison operators: Equality, Inclusion, Tautology checks 
    <li> Manipulation of (disjunctively and/or conjunctively) 
         partitioned BDDs: create, add/remove partitions, 
    <li> translation from/to CUDD BDDs.
    <li> Dumping on file, based on the DDDMP format, distributed 
         with CUDD.
    </ul>
    ]

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


/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*
 *  Conversion, creation (make) and release (free) functions
 */

/**Function********************************************************************
  Synopsis    [Build a Ddi_Bdd_t from a given CUDD node.]
  Description [Build the Ddi_Bdd_t structure (by means of DdiGenericAlloc)
               from manager and CUDD node.
               The reference count of the node is increased.]
  SideEffects []
  SeeAlso     [DdiGenericAlloc Ddi_BddToCU]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakeFromCU (
  Ddi_Mgr_t *mgr,
  DdNode *bdd
)
{
  Ddi_Bdd_t *r;

  Pdtutil_Assert((mgr!=NULL)&&(bdd!=NULL),
    "NULL manager or BDD when generating DDI node");

  r = (Ddi_Bdd_t *)DdiGenericAlloc(Ddi_Bdd_c,mgr);
  r->common.code = Ddi_Bdd_Mono_c;
  r->data.bdd = bdd;
  Cudd_Ref(bdd);

  return (r);
}

/**Function********************************************************************
  Synopsis    [Convert a DDI function to the corresponding Cudd Node]
  Description [Convert a DDI function to the corresponding Cudd Node.
    This is done by reading the proper field (pointing to a cudd node) in the 
    DDI node. No ref is done on the returned node.]
  SideEffects []
  SeeAlso     [Ddi_BddMakeFromCU]
******************************************************************************/
Ddi_BddNode *
Ddi_BddToCU(
  Ddi_Bdd_t *f         
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  Pdtutil_Assert(f->common.code == Ddi_Bdd_Mono_c,
    "converting to CU a non monolitic BDD. Use proper MakeMono function!");
  return (f->data.bdd);
}

/**Function********************************************************************
  Synopsis    [Generate a literal from a variable]
  Description [Generate a literal from a variable.
    The literal can be either affirmed or complemented.
    ]
  SideEffects [none]
  SeeAlso     [Ddi_Bdd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakeLiteral (
  Ddi_Var_t *v,
  int polarity    /* non 0: affirmed (v), 0: complemented literal (!v) */
)
{
  DdiConsistencyCheck(v,Ddi_Var_c);
  if (polarity) {
    return (Ddi_BddMakeFromCU(Ddi_ReadMgr(v),Ddi_VarToCU(v)));
  }
  else {
    return (Ddi_BddMakeFromCU(Ddi_ReadMgr(v),Cudd_Not(Ddi_VarToCU(v))));
  }
}

/**Function********************************************************************
  Synopsis    [Generate a Ddi_Bdd_t constant node (BDD zero or one)]
  Description [Generate a Ddi_Bdd_t constant node (BDD zero or one).
    The proper constant node within the manager is duplicated.
  ]
  SideEffects []
  SeeAlso     [Ddi_BddToCU DdiBddMakeLiteral]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakeConst (
  Ddi_Mgr_t *mgr,
  int value    /* non 0: true (one), 0: false (zero) */
)
{
  if (value) {
    return (Ddi_BddDup(Ddi_MgrReadOne(mgr)));
  }
  else {
    return (Ddi_BddDup(Ddi_MgrReadZero(mgr)));
  }
}

/**Function********************************************************************
  Synopsis    [Build a conjunctively partitioned BDD from a monolithic BDD]
  Description [Build a conjunctively partitioned BDD from a monolithic BDD]
  SideEffects []
  SeeAlso     [Ddi_BddMakePartDisjFromMono Ddi_BddMakePartConjFromArray
    Ddi_BddMakePartDisjFromArray]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakePartConjFromMono (
  Ddi_Bdd_t *mono
)
{
  Ddi_Bdd_t *r;
  Ddi_Mgr_t *mgr;
  
  mgr = Ddi_ReadMgr(mono);

  r = (Ddi_Bdd_t *)DdiGenericAlloc(Ddi_Bdd_c,mgr);
  r->common.code = Ddi_Bdd_Part_Conj_c;
  r->data.part = DdiArrayAlloc(1);
  DdiArrayWrite(r->data.part,0,(Ddi_Generic_t *)mono,Ddi_Dup_c);

  return(r);
}

/**Function********************************************************************
  Synopsis    [Build a conjunctively partitioned BDD with 0 partitions]
  Description [Build a conjunctively partitioned BDD with 0 partitions]
  SideEffects []
  SeeAlso     [Ddi_BddMakePartDisjVoid Ddi_BddMakePartConjFromMono 
    Ddi_BddMakePartConjFromArray]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakePartConjVoid (
  Ddi_Mgr_t *mgr
)
{
  Ddi_Bdd_t *r;
  
  r = (Ddi_Bdd_t *)DdiGenericAlloc(Ddi_Bdd_c,mgr);
  r->common.code = Ddi_Bdd_Part_Conj_c;
  r->data.part = DdiArrayAlloc(0);

  return(r);
}

/**Function********************************************************************
  Synopsis    [Build a disjunctively partitioned BDD with 0 partitions]
  Description [Build a disjunctively partitioned BDD with 0 partitions]
  SideEffects []
  SeeAlso     [Ddi_BddMakePartConjVoid Ddi_BddMakePartDisjFromMono 
    Ddi_BddMakePartDisjFromArray]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakePartDisjVoid (
  Ddi_Mgr_t *mgr
)
{
  Ddi_Bdd_t *r;
  
  r = (Ddi_Bdd_t *)DdiGenericAlloc(Ddi_Bdd_c,mgr);
  r->common.code = Ddi_Bdd_Part_Disj_c;
  r->data.part = DdiArrayAlloc(0);

  return(r);
}

/**Function********************************************************************
  Synopsis    [Build a disjunctively partitioned BDD from a monolithic BDD]
  Description [Build a disjunctively partitioned BDD from a monolithic BDD]
  SideEffects []
  SeeAlso     [Ddi_BddMakePartConjFromMono Ddi_BddMakePartConjFromArray
    Ddi_BddMakePartDisjFromArray]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakePartDisjFromMono (
  Ddi_Bdd_t *mono
)
{
  Ddi_Bdd_t *r;
  Ddi_Mgr_t *mgr;
  
  mgr = Ddi_ReadMgr(mono);
  
  r = (Ddi_Bdd_t *)DdiGenericAlloc(Ddi_Bdd_c,mgr);
  r->common.code = Ddi_Bdd_Part_Disj_c;
  r->data.part = DdiArrayAlloc(1);
  DdiArrayWrite(r->data.part,0,(Ddi_Generic_t *)mono,Ddi_Dup_c);

  return(r);
}

/**Function********************************************************************
  Synopsis    [Build a conjunctively partitioned BDD from array of partitions]
  Description [Build a conjunctively partitioned BDD from array of partitions]
  SideEffects []
  SeeAlso     [Ddi_BddMakePartConjFromMono Ddi_BddMakePartDisjFromMono 
    Ddi_BddMakePartDisjFromArray]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakePartConjFromArray (
  Ddi_Bddarray_t *array
)
{
  Ddi_Bdd_t *r;
  
  r = (Ddi_Bdd_t *)DdiGenericAlloc(Ddi_Bdd_c,Ddi_ReadMgr(array));
  r->common.code = Ddi_Bdd_Part_Conj_c;
  r->data.part = DdiArrayDup(array->array);

  return(r);
}

/**Function********************************************************************
  Synopsis    [Build a disjunctively partitioned BDD from array of BDDs]
  Description [Build a disjunctively partitioned BDD from array of BDDs]
  SideEffects []
  SeeAlso     [Ddi_BddMakePartConjFromMono Ddi_BddMakePartDisjFromMono 
    Ddi_BddMakePartConjFromArray]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakePartDisjFromArray (
  Ddi_Bddarray_t *array
)
{
  Ddi_Bdd_t *r;
  
  r = (Ddi_Bdd_t *)DdiGenericAlloc(Ddi_Bdd_c,Ddi_ReadMgr(array));
  r->common.code = Ddi_Bdd_Part_Disj_c;
  r->data.part = DdiArrayDup(array->array);

  return(r);
}

/**Function********************************************************************
  Synopsis    [Convert a BDD to conjunctively partitioned (if required). 
               Result accumulated]
  Description [Convert a BDD to conjunctively partitioned (if required). 
               Result accumulated]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSetPartConj (
  Ddi_Bdd_t *f      
)
{
  Ddi_Bdd_t *m;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Part_Conj_c:
    return (f);
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation not supported for meta BDD");
    break;
  case Ddi_Bdd_Mono_c:
    m = Ddi_BddMakePartConjFromMono(f);
    Cudd_RecursiveDeref (Ddi_MgrReadMgrCU(Ddi_ReadMgr(f)), f->data.bdd);
    f->common.code = Ddi_Bdd_Part_Conj_c;
    f->data.part = DdiArrayDup(m->data.part);
    Ddi_Free(m);
    break;
  case Ddi_Bdd_Part_Disj_c:
    m = Ddi_BddMakePartConjFromMono(f);
    DdiArrayFree (f->data.part);
    f->common.code = Ddi_Bdd_Part_Conj_c;
    f->data.part = DdiArrayDup(m->data.part);
    Ddi_Free(m);
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }
  return(f);
}

/**Function********************************************************************
  Synopsis    [Convert a BDD to disjunctively partitioned (if required). 
               Result accumulated]
  Description [Convert a BDD to disjunctively partitioned (if required). 
               Result accumulated]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSetPartDisj (
  Ddi_Bdd_t *f      
)
{
  Ddi_Bdd_t *m;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Part_Disj_c:
    return (f);
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation not supported for meta BDD");
    break;
  case Ddi_Bdd_Mono_c:
    m = Ddi_BddMakePartDisjFromMono(f);
    Cudd_RecursiveDeref (Ddi_MgrReadMgrCU(Ddi_ReadMgr(f)), f->data.bdd);
    f->common.code = Ddi_Bdd_Part_Disj_c;
    f->data.part = DdiArrayDup(m->data.part);
    Ddi_Free(m);
    break;
  case Ddi_Bdd_Part_Conj_c:
    m = Ddi_BddMakePartDisjFromMono(f);
    DdiArrayFree (f->data.part);
    f->common.code = Ddi_Bdd_Part_Disj_c;
    f->data.part = DdiArrayDup(m->data.part);
    Ddi_Free(m);
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }
  return(f);
}

/**Function********************************************************************
  Synopsis    [Generate a Ddi_Bdd_t relation from array of functions]
  Description [Generate a Ddi_Bdd_t relation from array of functions. 
    Relation is generated considering function variables domain, 
    and range variables as co-domain.
    I-th range variable corresponds to i-th function.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddRelMakeFromArray (
  Ddi_Bddarray_t *Fa        /* array of functions */,
  Ddi_Vararray_t *Va        /* array of range variables */
)
{
  Ddi_Mgr_t *ddm;
  Ddi_Bdd_t *rel, *r_i, *lit;
  int i, n;

  DdiConsistencyCheck(Fa,Ddi_Bddarray_c);
  DdiConsistencyCheck(Va,Ddi_Vararray_c);

  n = Ddi_BddarrayNum(Fa);
  ddm = Ddi_ReadMgr(Fa);
  Pdtutil_Assert (n == Ddi_VararrayNum(Va),
    "Number of range variables does not match number of functions.");
  Pdtutil_Assert (ddm == Ddi_ReadMgr(Va),
    "Different managers found while generating relation from functions");

  rel = Ddi_BddMakePartConjVoid(ddm);

  for (i=0; i<n; i++) {
    lit = Ddi_BddMakeLiteral(Ddi_VararrayRead(Va,i),1);
    r_i = Ddi_BddXnor(lit,Ddi_BddarrayRead(Fa,i));
    Ddi_Free (lit);
    Ddi_BddPartInsertLast(rel,r_i);
    Ddi_Free (r_i);
  }

  return (rel);
}

/**Function********************************************************************
  Synopsis     [Duplicate a Ddi_Bdd_t]
  Description  [Duplicate a Ddi_Bdd_t. All pointed objects are recursively 
    duplicated. In case of partitioned
    BDDs, array of partitions are duplicated. Cudd BDDs are referenced.]
  SideEffects  []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddDup (
  Ddi_Bdd_t *f    /* BDD to be duplicated */ 
)
{
  Ddi_Bdd_t *r;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  r = (Ddi_Bdd_t *)DdiGenericDup((Ddi_Generic_t *)f);

  return (r);
}

/**Function********************************************************************
  Synopsis     [Copy a Ddi_Bdd_t to a destination DDI manager]
  Description  [Copy a Ddi_Bdd_t to a destination DDI manager.
    Variable correspondence is established "by index", i.e. 
    variables with same index in different manager correspond. 
    Bdd is simply duplicated if destination manager is equal to the 
    source one.]
  SideEffects  []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCopy (
  Ddi_Mgr_t *ddm  /* destination manager */,
  Ddi_Bdd_t *old   /* BDD to be duplicated */ 
)
{
  Ddi_Bdd_t *newBdd;

  DdiConsistencyCheck(old,Ddi_Bdd_c);
  newBdd = (Ddi_Bdd_t *)DdiGenericCopy(ddm,(Ddi_Generic_t *)old,NULL,NULL);

  return (newBdd);
}

/**Function********************************************************************
  Synopsis     [Copy a Ddi_Bdd_t to a destination DDI manager]
  Description  [Copy a Ddi_Bdd_t to a destination DDI manager.
    Variable correspondence is established "by index", i.e. 
    variables with same index in different manager correspond. 
    Bdd is simply duplicated if destination manager is equal to the 
    source one.]
  SideEffects  []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCopyRemapVars (
  Ddi_Mgr_t *ddm            /* destination manager */,
  Ddi_Bdd_t *old            /* BDD to be duplicated */, 
  Ddi_Vararray_t *varsOld   /* old variable array */,
  Ddi_Vararray_t *varsNew   /* new variable array */
)
{
  Ddi_Bdd_t *newBdd;

  DdiConsistencyCheck(old,Ddi_Bdd_c);
  newBdd = (Ddi_Bdd_t *)DdiGenericCopy(ddm,(Ddi_Generic_t *)old,
    varsOld,varsNew);

  return (newBdd);
}

/**Function********************************************************************
  Synopsis     [Return BDD size (total amount of BDD nodes) of f]
  Description  [Return BDD size (total amount of BDD nodes) of f.
    In case of partitioned or meta BDDs the sharing size is returned
    (shared subgraphs are counted once).]
  SideEffects  []
******************************************************************************/
int
Ddi_BddSize(
  Ddi_Bdd_t  *f
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  return (DdiGenericBddSize((Ddi_Generic_t *)f));
}

/**Function********************************************************************
  Synopsis    [Return the top BDD variable of f]
  Description [Return the top BDD variable of f]
  SideEffects []
******************************************************************************/
Ddi_Var_t *
Ddi_BddTopVar (
  Ddi_Bdd_t *f
)
{
  Ddi_Mgr_t *ddm;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  ddm=Ddi_ReadMgr(f);
  return (Ddi_IthVar(ddm,Cudd_NodeReadIndex(Ddi_BddToCU(f))));
}

/**Function********************************************************************
  Synopsis     [Evaluate expression and free BDD node]
  Description  [Useful for accumulator like expressions (g=f(g,h)), i.e.
    computing a new value for a variable
    and the old value must be freed. Avoids using temporary
    variables. Since the f expression is evalued before
    passing actual parameters, freeing of g occurs as last
    operation. 
    <pre>
    E.g. 
          g=Ddi_BddEvalFree(Ddi_BddAnd(g,h),g).
    </pre>
    The "accumulator" style operations introduced from version 2.0 of
    pdtrav should stongly reduce the need for this technique. The above 
    example can now be written as:
    <pre>
          Ddi_BddAndAcc(g,h).
    </pre>
    ] 
  SideEffects  [none]
  SeeAlso      [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddEvalFree(
  Ddi_Bdd_t *f    /* expression */,
  Ddi_Bdd_t *g    /* BDD to be freed */
)
{
  Ddi_Free (g);
  return (f);
}

/*
 *  relational operations
 */

/**Function********************************************************************
  Synopsis    [Return true (non 0) if the two DDs are equal (f==g).]
  Description [Return true (non 0) if the two DDs are equal (f==g).
    This test is presently limited to monolithic BDDs.]
  SideEffects []
******************************************************************************/
int
Ddi_BddEqual (
  Ddi_Bdd_t *f  /* first dd */,
  Ddi_Bdd_t *g  /* second dd */)
{
  return (Ddi_BddToCU(f)==Ddi_BddToCU(g));
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if f is the zero constant.]
  Description [Return true (non 0) if f is the zero constant.
    This test is presently limited to monolithic BDDs.]
  SideEffects [none]
  SeeAlso     [Ddi_BddIsOne]
******************************************************************************/
int
Ddi_BddIsZero (
  Ddi_Bdd_t *f
)
{
  if (Ddi_BddIsMono(f)) {
    return (Ddi_BddEqual (f, Ddi_MgrReadZero(Ddi_ReadMgr(f))));
  }
  else if (Ddi_BddIsMeta(f)) {
    return (DdiMetaIsConst(f,0));
  }
  /* pessimistic! Don't consider so far partitioned BDDs */
  else return (0);
  return Ddi_BddEqual(f,Ddi_MgrReadZero(Ddi_ReadMgr(f)));
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if f is the one constant.]
  Description [Return true (non 0) if f is the one constant.
    This test is presently limited to monolithic BDDs.]
  SideEffects [none]
  SeeAlso     [Ddi_BddIsZero]
******************************************************************************/
int
Ddi_BddIsOne (
  Ddi_Bdd_t *f
)
{
  if (Ddi_BddIsMono(f)) {
    return (Ddi_BddEqual (f, Ddi_MgrReadOne(Ddi_ReadMgr(f))));
  }
  else if (Ddi_BddIsMeta(f)) {
    return (DdiMetaIsConst(f,1));
  }
  /* pessimistic! Don't consider so far partitioned BDDs */
  else return (0);
}

/**Function********************************************************************
  Synopsis     [Check for inclusion (f in g). Return non 0 if true]
  Description  [Check for inclusion (f in g). Return non 0 if true.
    This test requires the second operand (g) to be monolithic, whereas
    monolithic and disjunctively partitioned forms are allowed for first
    operand (f).]
  SideEffects  []
******************************************************************************/
double
Ddi_CountMinterm (
  Ddi_Bdd_t *f,
  int nvar
)
{
  if (f->common.code == Ddi_Bdd_Meta_c) {
    return (-1.0);
  }
  else {
    return (
      Cudd_CountMinterm(Ddi_MgrReadMgrCU(Ddi_ReadMgr(f)),Ddi_BddToCU(f),nvar)
    );
  }
}

/**Function********************************************************************
  Synopsis     [Check for inclusion (f in g). Return non 0 if true]
  Description  [Check for inclusion (f in g). Return non 0 if true.
    This test requires the second operand (g) to be monolithic, whereas
    monolithic and disjunctively partitioned forms are allowed for first
    operand (f).]
  SideEffects  []
******************************************************************************/
int
Ddi_BddIncluded (
  Ddi_Bdd_t *f,
  Ddi_Bdd_t *g
)
{
  Ddi_Bdd_t *p, *gg, *ff;
  int r=0;
  Ddi_Mgr_t *ddm;
 
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  DdiConsistencyCheck(g,Ddi_Bdd_c);

  if (f->common.code == Ddi_Bdd_Meta_c) {
    return (0);
  }
  if (g->common.code == Ddi_Bdd_Meta_c) {
    return (0);
  }

  ddm = Ddi_ReadMgr(f);

  Pdtutil_Warning(g->common.code!=Ddi_Bdd_Mono_c,
    "inclusion test requires monolitic 2-nd operand. MakeMono forced");
  gg = Ddi_BddMakeMono(g);

  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
    r = (Cudd_bddIteConstant(Ddi_MgrReadMgrCU(ddm),
      Ddi_BddToCU(f),Ddi_BddToCU(gg),Ddi_BddToCU(Ddi_MgrReadOne(ddm)))
       == Ddi_BddToCU(Ddi_MgrReadOne(ddm)));
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Meta_c:
    Pdtutil_Warning(1,
    "conj/meta 1-st operand not allowed by inclusion test. MakeMono forced");
    ff = Ddi_BddMakeMono(f);
    r = Ddi_BddIncluded(ff,gg);
    Ddi_Free(ff);
    break;
  case Ddi_Bdd_Part_Disj_c:
    ff = Ddi_BddDup(f);
    p = Ddi_BddPartExtract(ff,0);
    r = Ddi_BddIncluded(p,gg) && Ddi_BddIncluded(ff,gg);
    Ddi_Free(ff);
    Ddi_Free(p);
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }

  Ddi_Free(gg);

  return (r);
}

/*
 *  Booleand operations & co.
 */

/**Function********************************************************************
  Synopsis    [Boolean NOT. New result is generated]
  Description[Boolean NOT. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddNot ( 
  Ddi_Bdd_t *f
)
{
  return (Ddi_BddOp1(Ddi_BddNot_c,f));
}

/**Function********************************************************************
  Synopsis    [Boolean NOT. Result is accumulated]
  Description[Boolean NOT. Result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddNotAcc ( 
  Ddi_Bdd_t *f
)
{
  return (Ddi_BddOpAcc1(Ddi_BddNot_c,f));
}

/**Function********************************************************************
  Synopsis    [Boolean AND. New result is generated]
  Description [Compute f & g. A new result is generated and returned. 
               Input parameters are NOT changed]
  SideEffects [none]
  SeeAlso     [Ddi_BddNot Ddi_BddOr ]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddAnd (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddAnd_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean AND. Result is accumulated]
  Description [Compute f & g. Previous content of f is freed and new result
               is copyed to f. Since f points to a handle, it can be passed
               by value: the handle is kept when freeing old content.
               Accumulate type operations are useful to avoid temporary
               variables and explicit free of old data.
               The pointer to f (old handle) is returned so that the function
               may be used as operand for other functions.]
  SideEffects [none]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddAndAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddAnd_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean difference (f & !g). New result is generated]
  Description[Boolean difference (f & !g). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddDiff (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddDiff_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean difference (f & !g). Result is accumulated]
  Description[Boolean difference (f & !g). Result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddDiffAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddDiff_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean NAND (!(f&g)). New result is generated]
  Description[Boolean NAND (!(f&g)). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddNand (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddNand_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean NAND (!(f&g)). New result is accumulated]
  Description[Boolean NAND (!(f&g)). New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddNandAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddNand_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean OR (f|g). New result is generated]
  Description[Boolean OR (f|g). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddOr (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddOr_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean OR (f|g). New result is accumulated]
  Description[Boolean OR (f|g). New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddOrAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddOr_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean NOR (!(f|g)). New result is generated]
  Description[Boolean NOR (!(f|g)). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddNor (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddNor_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean NOR (!(f|g)). New result is accumulated]
  Description[Boolean NOR (!(f|g)). New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddNorAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddNor_c,f,g));
}


/**Function********************************************************************
  Synopsis    [Boolean XOR (f^g). New result is generated]
  Description[Boolean XOR (f^g). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddXor (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddXor_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean XOR (f^g). New result is accumulated]
  Description[Boolean XOR (f^g). New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddXorAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddXor_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean XNOR (!(f^g)). New result is generated]
  Description[Boolean XNOR (!(f^g)). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddXnor (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddXnor_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Boolean XNOR (!(f^g)). New result is accumulated]
  Description[Boolean XNOR (!(f^g)). New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddXnorAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddXnor_c,f,g));
}

/**Function********************************************************************
  Synopsis    [If-Then-Else (ITE(f,g,h)). New result is generated]
  Description[If-Then-Else (ITE(f,g,h)). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddIte (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g,
  Ddi_Bdd_t  *h
)
{
  return (Ddi_BddOp3(Ddi_BddIte_c,f,g,h));
}

/**Function********************************************************************
  Synopsis    [If-Then-Else (ITE(f,g,h)). New result is accumulated]
  Description[If-Then-Else (ITE(f,g,h)). New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddIteAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g,
  Ddi_Bdd_t  *h
)
{
  return (Ddi_BddOpAcc3(Ddi_BddIte_c,f,g,h));
}

/**Function********************************************************************
  Synopsis    [Existential abstraction. New result is generated]
  Description[Existential abstraction. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddExist (
  Ddi_Bdd_t  *f,
  Ddi_Varset_t *vars
)
{
  return (Ddi_BddOp2(Ddi_BddExist_c,f,vars));
}

/**Function********************************************************************
  Synopsis    [Existential abstraction. New result is accumulated]
  Description[Existential abstraction. New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddExistAcc (
  Ddi_Bdd_t  *f,
  Ddi_Varset_t *vars
)
{
  return (Ddi_BddOpAcc2(Ddi_BddExist_c,f,vars));
}

/**Function********************************************************************
  Synopsis    [Universal abstraction. New result is generated]
  Description[Universal abstraction. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddForall (
  Ddi_Bdd_t  *f,
  Ddi_Varset_t *vars
)
{
  return (Ddi_BddOp2(Ddi_BddForall_c,f,vars));
}

/**Function********************************************************************
  Synopsis    [Universal abstraction. New result is accumulated]
  Description[Universal abstraction. New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddForallAcc (
  Ddi_Bdd_t  *f,
  Ddi_Varset_t *vars
)
{
  return (Ddi_BddOpAcc2(Ddi_BddForall_c,f,vars));
}

/**Function********************************************************************
  Synopsis    [Relational product (Exist(f&g,vars)). New result is generated]
  Description[Relational product (Exist(f&g,vars)). New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddAndExist (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g,
  Ddi_Varset_t *vars
)
{
  Ddi_Bdd_t *r;

  if (Ddi_BddIsPartConj(g)) {
    r = Ddi_BddOp3(Ddi_BddAndExist_c,g,f,vars);
  }
  else {
    r = Ddi_BddOp3(Ddi_BddAndExist_c,f,g,vars);
  }

  return(r);
}

/**Function********************************************************************
  Synopsis    [Relational product (Exist(f&g,vars)). New result is accumulated]
  Description[Relational product (Exist(f&g,vars)). New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddAndExistAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g,
  Ddi_Varset_t *vars
)
{
  return (Ddi_BddOpAcc3(Ddi_BddAndExist_c,
    f,g,vars));
}

/**Function********************************************************************
  Synopsis    [Cofactor with variable. New result is generated]
  Description [Cofactor with variable. New result is generated]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCofactor (
  Ddi_Bdd_t  *f,
  Ddi_Var_t  *v,
  int phase
)
{
  Ddi_Bdd_t *r, *lit;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  DdiConsistencyCheck(v,Ddi_Var_c);

  lit = Ddi_BddMakeLiteral(v,phase);
  r = Ddi_BddConstrain(f,lit);
  Ddi_Free(lit);

  return (r);
}

/**Function********************************************************************
  Synopsis    [Cofactor with variable. New result is accumulated]
  Description [Cofactor with variable. New result is accumulated]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCofactorAcc (
  Ddi_Bdd_t  *f,
  Ddi_Var_t  *v,
  int phase
)
{
  Ddi_Bdd_t *lit;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  DdiConsistencyCheck(v,Ddi_Var_c);

  lit = Ddi_BddMakeLiteral(v,phase);
  Ddi_BddConstrainAcc(f,lit);
  Ddi_Free(lit);

  return (f);
}


/**Function********************************************************************
  Synopsis    [Constrain cofactor. New result is generated]
  Description[Constrain cofactor. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddConstrain (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  Ddi_Bdd_t *r;
  Ddi_Varset_t *sf, *sg, *common;

  if (Ddi_BddIsOne(g)||Ddi_BddIsConstant(f)) {
    return(Ddi_BddDup(f));
  }

  Ddi_MgrAutodynSuspend(Ddi_ReadMgr(f));
  
  sf = Ddi_BddSupp(f);
  sg = Ddi_BddSupp(g);
  common = Ddi_VarsetIntersect(sf,sg);

  Ddi_MgrAutodynResume(Ddi_ReadMgr(f));

  if (Ddi_VarsetIsVoid(common)) {
    r = Ddi_BddDup(f);
  }
  else {
    r = Ddi_BddOp2(Ddi_BddConstrain_c,f,g);
  }

  Ddi_Free(sf);
  Ddi_Free(sg);
  Ddi_Free(common);
  return (r);
}

/**Function********************************************************************
  Synopsis    [Constrain cofactor. New result is accumulated]
  Description[Constrain cofactor. New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddConstrainAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  if (Ddi_BddIsOne(g)||Ddi_BddIsConstant(f)) {
    return(f);
  }
  return (Ddi_BddOpAcc2(Ddi_BddConstrain_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Restrict cofactor. New result is generated]
  Description[Restrict cofactor. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddRestrict (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddRestrict_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Restrict cofactor. New result is accumulated]
  Description[Restrict cofactor. New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddRestrictAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddRestrict_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Cofexist cofactor. New result is generated]
  Description[Constrain cofactor. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCofexist (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g,
  Ddi_Varset_t *smooth
)
{
  Ddi_Bdd_t *r, *gMask;
  DdNode *cuMask;
  Ddi_Mgr_t *ddm = Ddi_ReadMgr(f);

  Pdtutil_Assert(Ddi_BddIsMono(g),"cofexist cofactoring term must be mono");
  
  Ddi_MgrAutodynSuspend(ddm);
  Ddi_MgrReadMgrCU(ddm)->reordered = 0;
  
  cuMask = Cuplus_CofexistMask(Ddi_MgrReadMgrCU(ddm),
    Ddi_BddToCU(g),Ddi_VarsetToCU(smooth));
  gMask = Ddi_BddMakeFromCU (ddm, cuMask);
  
  r = Ddi_BddConstrain(f,g);
#if 0
  Ddi_BddSetPartConj(r);
  Ddi_BddPartInsert(r,0,gMask);
#else
  Ddi_BddAndAcc(r,gMask);
#endif

  Ddi_Free(gMask);
  
  Ddi_MgrAutodynResume(ddm);

  return (r);
}

/**Function********************************************************************
  Synopsis    [Compatible projector. New result is generated]
  Description[Compatible projector. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCproject (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOp2(Ddi_BddCproject_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Compatible projector. New result is accumulated]
  Description[Compatible projector. New result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCprojectAcc (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  return (Ddi_BddOpAcc2(Ddi_BddCproject_c,f,g));
}

/**Function********************************************************************
  Synopsis    [Swap x and y variables in f. New result is generated]
  Description[Swap x and y variables in f. New result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSwapVars (
  Ddi_Bdd_t  *f,
  Ddi_Vararray_t *x  /* 1-st array of variables */,
  Ddi_Vararray_t *y  /* 2-nd array of variables */
)
{
  return (Ddi_BddOp3(Ddi_BddSwapVars_c,f,x,y));
}

/**Function********************************************************************
  Synopsis    [Swap x and y variables in f. Result is accumulated]
  Description[Swap x and y variables in f. Result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSwapVarsAcc (
  Ddi_Bdd_t  *f,
  Ddi_Vararray_t *x  /* 1-st array of variables */,
  Ddi_Vararray_t *y  /* 2-nd array of variables */
)
{
  return (Ddi_BddOpAcc3(Ddi_BddSwapVars_c,f,x,y));
}

/**Function********************************************************************
  Synopsis    [Variable substitution x <- y in f. New result is generated]
  Description [Variable substitution x <- y in f. New result is generated.  
               Variable correspondence is established by position in x, y.
               Substitution is done by compose. This differs from variable 
               swapping since some y vars may be present in x as well as in 
               the support of f.]
  SideEffects [none]
  SeeAlso     [Ddi_BddSwapVars Ddi_BddSubstVarsAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSubstVars (
  Ddi_Bdd_t  *f,
  Ddi_Vararray_t *x  /* 1-st array of variables */,
  Ddi_Vararray_t *y  /* 2-nd array of variables */
)
{
  return (Ddi_BddOp3(Ddi_BddSubstVars_c,f,x,y));
}

/**Function********************************************************************
  Synopsis    [Variable substitution x <- y in f. New result is accumulated]
  Description [Variable substitution x <- y in f. New result is accumulated.  
               Variable correspondence is established by position in x, y.
               Substitution is done by compose. This differs from variable 
               swapping since some y vars may be present in x as well as in 
               the support of f.]
  SideEffects [none]
  SeeAlso     [Ddi_BddSwapVars Ddi_BddSubstVars]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSubstVarsAcc (
  Ddi_Bdd_t  *f,
  Ddi_Vararray_t *x  /* 1-st array of variables */,
  Ddi_Vararray_t *y  /* 2-nd array of variables */
)
{
  return (Ddi_BddOpAcc3(Ddi_BddSubstVars_c,f,x,y));
}

/**Function********************************************************************
  Synopsis    [Function composition x <- g in f. New result is generated]
  Description [Function composition x <- g in f. New result is generated.  
               Vector composition algorithm is used.]
  SideEffects [none]
  SeeAlso     [Ddi_BddSwapVars Ddi_BddSubstVars]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddCompose (
  Ddi_Bdd_t  *f,
  Ddi_Vararray_t *x  /* array of variables */,
  Ddi_Bddarray_t *g  /* array of functions */
)
{
  return (Ddi_BddOp3(Ddi_BddCompose_c,f,x,g));
}

/**Function********************************************************************
  Synopsis    [Function composition x <- g in f. New result is accumulated]
  Description [Function composition x <- g in f. New result is accumulated.  
               Vector composition algorithm is used.]
  SideEffects [none]
  SeeAlso     [Ddi_BddSwapVars Ddi_BddSubstVars]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddComposeAcc (
  Ddi_Bdd_t  *f,
  Ddi_Vararray_t *x  /* array of variables */,
  Ddi_Bddarray_t *g  /* array of functions */
)
{
  return (Ddi_BddOpAcc3(Ddi_BddCompose_c,f,x,g));
}


/**Function********************************************************************
  Synopsis    [Return true (non 0) if f is a cube.]
  Description [Return true (non 0) if f is a cube. Monolithic BDD required.]
  SideEffects [none]
  SeeAlso     [Ddi_BddIsOne]
******************************************************************************/
int
Ddi_BddIsCube (
  Ddi_Bdd_t *f
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  if (!Ddi_BddIsMono(f)) {
    return(0);
  }
  return(cuddCheckCube(Ddi_MgrReadMgrCU(Ddi_ReadMgr(f)),Ddi_BddToCU(f)));
}

/**Function********************************************************************
  Synopsis    [Pick one random on-set cube. Result is generated]
  Description[Pick one random on-set cube. Result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPickOneCube (
  Ddi_Bdd_t  *f
)
{
  Ddi_Bdd_t *cubeDd, *literal=NULL;
  int i;
  char *cube;
  Ddi_Mgr_t *ddm;
  int nMgrVars;
  DdNode *f_cu;
  DdManager *mgrCU;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  ddm = Ddi_ReadMgr(f);
  mgrCU = Ddi_MgrReadMgrCU(ddm);
  nMgrVars = Ddi_ReadNumVars(ddm);

  if (Ddi_BddIsZero(f))
    return (Ddi_BddDup(f));

  Pdtutil_Assert(f->common.code == Ddi_Bdd_Mono_c,
    "Monolitic DD required for Cube selection. Use proper MakeMono function!");

  cube = Pdtutil_Alloc(char, nMgrVars);
  f_cu = Ddi_BddToCU(f);

  Pdtutil_Assert(Cudd_bddPickOneCube(mgrCU,f_cu,cube)!=0,
		 "Error returned by Cudd_bddPickOneCube");

  /* Build result BDD. */
  cubeDd = Ddi_BddDup(Ddi_MgrReadOne(ddm));

  for (i=0; i<nMgrVars; i++) {
    switch (cube[i]) {
      case 0:
        literal = Ddi_BddMakeLiteral(Ddi_IthVar(ddm,i),0);
        break;
      case 1:
        literal = Ddi_BddMakeLiteral(Ddi_IthVar(ddm,i),1);
        break;
      case 2:
        literal = Ddi_BddDup(Ddi_MgrReadOne(ddm));
        break;
      default:
        Pdtutil_Warning (1,"Wrong code found in cube string");
        literal = Ddi_BddDup(Ddi_MgrReadOne(ddm));
        break;
    }
    cubeDd = Ddi_BddEvalFree(Ddi_BddAnd(cubeDd,literal),cubeDd);
    Ddi_Free(literal);
  }
  return (cubeDd);
}

/**Function********************************************************************
  Synopsis    [Pick one random on-set cube. Result is accumulated]
  Description[Pick one random on-set cube. Result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAndAcc]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPickOneCubeAcc (
  Ddi_Bdd_t  *f
)
{
  Ddi_Bdd_t *tmp;

  tmp = Ddi_BddPickOneCube(f);
  DdiGenericDataCopy((Ddi_Generic_t *)f,(Ddi_Generic_t *)tmp);
  Ddi_Free(tmp);
  return (f);
}

/**Function********************************************************************
  Synopsis    [Pick one random on-set minterm. Result is generated]
  Description[Pick one random on-set minterm. Result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPickOneMinterm (
  Ddi_Bdd_t  *f,
  Ddi_Varset_t *vars   /* set of variables defining the minterm space */
)
{
  Ddi_Bdd_t *mintermDd;
  int i, nMgrVars;
  Ddi_Mgr_t *ddm;
  DdNode **vars_cu;
  DdNode *f_cu, *minterm_cu, *scan_cu;
  DdManager *mgrCU;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  ddm = Ddi_ReadMgr(f);
  mgrCU = Ddi_MgrReadMgrCU(ddm);
  nMgrVars = Ddi_ReadNumVars(ddm);

  if (Ddi_BddIsZero(f))
    return (Ddi_BddDup(f));

  Pdtutil_Assert(f->common.code == Ddi_Bdd_Mono_c,
    "Monolitic DD required for Cube selection. Use proper MakeMono function!");

  f_cu = Ddi_BddToCU(f);
  vars_cu = Pdtutil_Alloc(DdNode *, Ddi_VarsetNum(vars));

  scan_cu = Ddi_VarsetToCU(vars);
  for (i=0;i<Ddi_VarsetNum(vars);i++) {
    vars_cu[i] = Cudd_bddIthVar(mgrCU,Cudd_NodeReadIndex(scan_cu));
    scan_cu = Cudd_T(scan_cu);
  }

  minterm_cu = Cudd_bddPickOneMinterm(mgrCU,f_cu,vars_cu,Ddi_VarsetNum(vars));
  free(vars_cu);
  Pdtutil_Assert(minterm_cu!=NULL,"Error returned by Cudd_bddPickOneMinterm");
  mintermDd = Ddi_BddMakeFromCU (ddm, minterm_cu);

  return (mintermDd);
}

/**Function********************************************************************
  Synopsis    [Pick one random on-set minterm. Result is accumulated]
  Description[Pick one random on-set minterm. Result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_BddAnd]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPickOneMintermAcc (
  Ddi_Bdd_t  *f,
  Ddi_Varset_t *vars   /* set of variables defining the minterm space */
)
{
  Ddi_Bdd_t *tmp;

  tmp = Ddi_BddPickOneMinterm(f,vars);
  DdiGenericDataCopy((Ddi_Generic_t *)f,(Ddi_Generic_t *)tmp);
  Ddi_Free(tmp);
  return (f);
}

/**Function********************************************************************
  Synopsis    [Support of f. New result is generated]
  Description [The support of f is the set of variables f depends on.
               This function has no "accumulated" version, but a related
               function (Ddi_BddSuppAttach) which attaches the support to
               a function, so that no BDD traversal is done in further
               calls of Ddi_BddSupp.]
  SideEffects [none]
  SeeAlso     [Ddi_BddSuppAttach]
******************************************************************************/
Ddi_Varset_t *
Ddi_BddSupp ( 
  Ddi_Bdd_t *f
)
{
  return ((Ddi_Varset_t *)DdiGenericOp1(Ddi_BddSupp_c,Ddi_Generate_c,f));
}

/**Function********************************************************************
  Synopsis    [Attach support of f to f. Return pointer to f]
  Description [The support of f is the set of variables f depends on.
               This function generates the support of f 
               and hooks it to proper field of f, so that no BDD traversal 
               is done in further calls of Ddi_BddSupp.]
  SideEffects [support is attached to f]
  SeeAlso     [Ddi_BddSupp Ddi_BddSuppDetach]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSuppAttach ( 
  Ddi_Bdd_t *f
)
{
  return ((Ddi_Bdd_t*)DdiGenericOp1(Ddi_BddSuppAttach_c,Ddi_Accumulate_c,f));
}

/**Function********************************************************************
  Synopsis    [Read the support attached to a Bdd.]
  Description [Read the support attached to a Bdd. The support is not 
    duplicated, as would Ddi_BddSupp with attached support. Return NULL if
    support is not attached.]
  SideEffects [none]
  SeeAlso     [Ddi_BddSupp Ddi_BddSuppAttach]
******************************************************************************/
Ddi_Varset_t *
Ddi_BddSuppRead(
  Ddi_Bdd_t *f 
)
{
  return ((Ddi_Varset_t*)(f->common.supp));
}
/**Function********************************************************************
  Synopsis    [Detach (and free) support attached to f. Return pointer to f]
  Description[Detach (and free) support attached to f. Return pointer to f]
  SideEffects [none]
  SeeAlso     [Ddi_BddSupp Ddi_BddSuppAttach]
******************************************************************************/

Ddi_Bdd_t *
Ddi_BddSuppDetach(
  Ddi_Bdd_t *f 
)
{
  return ((Ddi_Bdd_t*)DdiGenericOp1(Ddi_BddSuppDetach_c,Ddi_Accumulate_c,f));
}

/*
 *  Partition management
 */

/**Function********************************************************************
  Synopsis    [Return true (non 0) if f is a monolithic BDD.]
  Description [Return true (non 0) if f is a monolithic BDD.]
  SideEffects [none]
  SeeAlso     [Ddi_BddIsPartConj Ddi_BddIsPartDisj]
******************************************************************************/
int
Ddi_BddIsMono (
  Ddi_Bdd_t *f
)
{
  return (f->common.code == Ddi_Bdd_Mono_c);
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if f is a meta BDD.]
  Description [Return true (non 0) if f is a meta BDD.]
  SideEffects [none]
  SeeAlso     [Ddi_BddIsPartConj Ddi_BddIsPartDisj]
******************************************************************************/
int
Ddi_BddIsMeta (
  Ddi_Bdd_t *f
)
{
  return (f->common.code == Ddi_Bdd_Meta_c);
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if f is a conjunctively partitioned BDD.]
  Description [Return true (non 0) if f is a conjunctively partitioned BDD.]
  SideEffects [none]
  SeeAlso     [Ddi_BddIsMono Ddi_BddIsPartDisj]
******************************************************************************/
int
Ddi_BddIsPartConj (
  Ddi_Bdd_t *f
)
{
  return (f->common.code == Ddi_Bdd_Part_Conj_c);
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if f is a disjunctively partitioned BDD.]
  Description [Return true (non 0) if f is a disjunctively partitioned BDD.]
  SideEffects [none]
  SeeAlso     [Ddi_BddIsMono Ddi_BddIsPartConj]
******************************************************************************/
int
Ddi_BddIsPartDisj (
  Ddi_Bdd_t *f
)
{
  return (f->common.code == Ddi_Bdd_Part_Disj_c);
}


/**Function********************************************************************
  Synopsis    [Read the number of partitions (conj/disj).]
  Description [Read the number of partitions (conj/disj).
    In case of monolithic BDD, 1 is returned, in case of partitioned
    BDDs, the number of partitions.]
  SideEffects []
******************************************************************************/
int
Ddi_BddPartNum (
  Ddi_Bdd_t * f
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation requires partitioned BDD");
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    return (DdiArrayNum(f->data.part));
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }
  return (-1);
}

/**Function********************************************************************
  Synopsis    [Read the i-th partition (conj/disj) of f.]
  Description [Read the i-th partition (conj/disj) of f.]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPartRead(
  Ddi_Bdd_t * f,
  int i
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  Pdtutil_Assert((i>=0)&&(i<Ddi_BddPartNum(f)),
    "partition index out of bounds");
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation requires partitioned BDD");
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    return ((Ddi_Bdd_t *)DdiArrayRead(f->data.part,i));
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }
  return(NULL);
}

/**Function********************************************************************
  Synopsis    [Write i-th partition. Result accumulated]
  Description [Write new partition at position i. Result accumulated.
    Same as insert if position is PartNum+1. Otherwise i-th partition is
    freed and overwritten, so Write is acrually a partition "replace" or
    "rewrite" operation.]
  SideEffects []
  SeeAlso     [Ddi_BddPartInsert]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPartWrite (
  Ddi_Bdd_t *f        /* partitioned BDD */,
  int i               /* position of new partition */,
  Ddi_Bdd_t *newp     /* new partition */
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation requires partitioned BDD");
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    DdiArrayWrite(f->data.part,i,(Ddi_Generic_t *)newp,Ddi_Dup_c);
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }
  return(f);
}

/**Function********************************************************************
  Synopsis    [Return the i-th partition (conj/disj), and remove it from f.]
  Description [Return the i-th partition (conj/disj), and remove it from f.]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPartExtract(
  Ddi_Bdd_t * f,
  int i
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  Pdtutil_Assert((i>=0)&&(i<Ddi_BddPartNum(f)),
    "partition index out of bounds");
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"partition extract requires partitioned BDD");
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    return ((Ddi_Bdd_t *)DdiArrayExtract(f->data.part,i));
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }
  return(NULL);
}

/**Function********************************************************************
  Synopsis    [Add i-th partition. Result accumulated]
  Description [Add new partition at position i. Result accumulated.
    Higher partitions are shifted.]
  SideEffects []
  SeeAlso     [Ddi_BddPartWrite]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPartInsert (
  Ddi_Bdd_t *f        /* partitioned BDD */,
  int i               /* position of new partition */,
  Ddi_Bdd_t *newp     /* new partition */
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation requires partitioned BDD");
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    DdiArrayInsert(f->data.part,i,(Ddi_Generic_t *)newp,Ddi_Dup_c);
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }
  return(f);
}

/**Function********************************************************************
  Synopsis    [Add last partition. Result accumulated]
  Description [Add new partition at last position. Result accumulated]
  SideEffects []
  SeeAlso     [Ddi_BddPartInsert]
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddPartInsertLast (
  Ddi_Bdd_t *f        /* partitioned BDD */,
  Ddi_Bdd_t *newp     /* new partition */
)
{
  return(Ddi_BddPartInsert(f,Ddi_BddPartNum(f),newp));
}

/**Function********************************************************************
  Synopsis    [Create a monolithic BDD from a partitioned one]
  Description [Create a monolithic BDD from a partitioned one]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakeMono (
  Ddi_Bdd_t *f        /* input function */
)
{
  Ddi_Bdd_t *r=NULL, *p;
  int i;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
  case Ddi_Bdd_Meta_c:
    return (Ddi_BddDup(f));
    break;
  case Ddi_Bdd_Part_Conj_c:
    r = Ddi_BddMakeMono(Ddi_BddPartRead(f,0));
    for (i=1;i<Ddi_BddPartNum(f);i++) {
       p = Ddi_BddMakeMono(Ddi_BddPartRead(f,i));
       Ddi_BddAndAcc(r,p);
       Ddi_Free(p);
    }
    break;
  case Ddi_Bdd_Part_Disj_c:
    r = Ddi_BddMakeMono(Ddi_BddPartRead(f,0));
    for (i=1;i<Ddi_BddPartNum(f);i++) {
       p = Ddi_BddMakeMono(Ddi_BddPartRead(f,i));
       Ddi_BddOrAcc(r,p);
       Ddi_Free(p);
    }
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }

  return(r);
}

/**Function********************************************************************
  Synopsis    [Convert a BDD to monolitic (if required). Result accumulated]
  Description [Convert a BDD to monolitic (if required). Result accumulated]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSetMono (
  Ddi_Bdd_t *f        /* input function */
)
{
  Ddi_Bdd_t *m;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
    return (f);
  case Ddi_Bdd_Meta_c:
    Ddi_BddFromMeta(f);
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    m = Ddi_BddMakeMono(f);
    DdiArrayFree(f->data.part);
    f->common.code = m->common.code;
    switch (f->common.code) {
      case Ddi_Bdd_Mono_c:
        Cudd_Ref(m->data.bdd);
        f->data.bdd = m->data.bdd;
      break;
      case Ddi_Bdd_Meta_c:
        f->data.meta = DdiMetaDup(m->data.meta);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
    }
    Ddi_Free(m);
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }

  return(f);
}

/**Function********************************************************************
  Synopsis    [Create a clustered BDD from a partitioned one]
  Description [Create a clustered BDD from a partitioned one. 
    Conjunctions/disjunctions are executed up to the size threshold
    (sizes greater than threshold are aborted).]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakeClustered (
  Ddi_Bdd_t *f        /* input function */,
  int threshold       /* size threshold */
)
{
  Ddi_Bdd_t *r=NULL;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
  case Ddi_Bdd_Meta_c:
    return (Ddi_BddDup(f));
    break;
  case Ddi_Bdd_Part_Conj_c:
    {
      Ddi_Varset_t *voidVs = Ddi_VarsetVoid(Ddi_ReadMgr(f));
      r = Part_BddMultiwayLinearAndExist(f,voidVs,threshold);
      Ddi_Free(voidVs);
    }
    break;
  case Ddi_Bdd_Part_Disj_c:
    {
      Ddi_Bdd_t *aux = Ddi_BddNot(f);
      r = Ddi_BddNotAcc(Ddi_BddMakeClustered(aux,threshold));
      Ddi_Free(aux);
    }
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }

  return(r);
}

/**Function********************************************************************
  Synopsis    [Create a clustered BDD from a partitioned one]
  Description [Create a clustered BDD from a partitioned one. 
    Conjunctions/disjunctions are executed up to the size threshold
    (sizes greater than threshold are aborted).]
  SideEffects []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSetClustered (
  Ddi_Bdd_t *f        /* input function */,
  int threshold       /* size threshold */
)
{
  Ddi_Bdd_t *m;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
    return (f);
  case Ddi_Bdd_Meta_c:
    Ddi_BddFromMeta(f);
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    m = Ddi_BddMakeClustered(f,threshold);
    DdiArrayFree(f->data.part);
    f->data.part = DdiArrayDup(m->data.part);
    Ddi_Free(m);
    break;
  default:
    Pdtutil_Assert(0,"Wrong DDI node code");
    break;
  }

  return(f);
}

/**Function********************************************************************
  Synopsis     [Prints a BDD]
  Description  [] 
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_BddPrint (
  Ddi_Bdd_t  *f,
  FILE *fp
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  DdiGenericPrint((Ddi_Generic_t *)f,fp);
}

/**Function********************************************************************
  Synopsis     [Prints Statistics of a BDD]
  Description  [] 
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
void
Ddi_BddPrintStats (
  Ddi_Bdd_t  *f,
  FILE *fp
)
{
  DdiConsistencyCheck(f,Ddi_Bdd_c);
  DdiGenericPrintStats((Ddi_Generic_t *)f,fp);
}

/**Function********************************************************************
  Synopsis    [Output a cube to string. Return true if succesful.]
  Description [Output a cube to string. Return true if succesful.
    The set of variables to be considered
    is given as input (if NULL, true support is used). 
    Variables are sorted by absolute index (which is constant across sifting),
    NOT by variable ordering.
    The procedure allows omitting variables in the true support, which
    are existentially quantified out by cube.
    ]
  SideEffects [None]
  SeeAlso     []
******************************************************************************/
int
Ddi_BddPrintCubeToString (
  Ddi_Bdd_t *f         /* BDD */,
  Ddi_Varset_t *vars  /* Variables */,
  char *string        /* output string */
)
{
  int i, j=0;
  Ddi_BddMgr *dd;
  int nMgrVars;
  DdNode *f_cu;
  DdManager *mgrCU;

  if (!Ddi_BddIsCube(f)) {
    return(0);
  }

  if (vars == NULL) {
    vars = Ddi_BddSupp(f);
  } else {
    vars = Ddi_VarsetDup(vars);
  }

  dd = Ddi_ReadMgr(f);
  nMgrVars = Ddi_ReadNumVars(dd);
  mgrCU = Ddi_MgrReadMgrCU(dd);
    
  /*
   *  Here we work in CUDD !
   */
  
  f_cu = Ddi_BddToCU(f);
  {
    int *cube;
    CUDD_VALUE_TYPE value;
    DdGen *gen;
  
    Cudd_ForeachCube(mgrCU,f_cu,gen,cube,value) {
      for (i=0,j=0; i<mgrCU->size; i++) {
        /* skip variables not in varset */
	if (!Ddi_VarInVarset(vars,Ddi_IthVar(dd,i))) {
          continue;
        }
        switch (cube[i]) {
    	case 0:
          string[j++]='0';
          break;
        case 1:
          string[j++]='1';
          break;
        case 2:
          string[j++]='-';
          break;
        default:
          string[j++]='?';
          break;
        }
      }
    }
    string[j++]='\0';
  }
  
  /*
   *  End working in CUDD !
   */

  Ddi_Free(vars);

  return (1);
}

/**Function********************************************************************
  Synopsis    [Outputs the cubes of a BDD on file]
  Description [This function outputs the cubes of a BDD on file.
    Only monolithic BDDs are supported. The set of variables to be considered
    is given as input (if NULL the true support is used), to allow generating
    don't cares, data are sorted by absolute index, NOT by variable ordering.
    The procedure allows omitting variables in the true support, which
    are existentially quantified out before generating cubes, to avoid 
    repetitions.
    A limit on the number of cubes generated can be specified.
    Use a negative value for no bound.
    ]
  SideEffects [None]
  SeeAlso     []
******************************************************************************/
int
Ddi_BddPrintCubes (
  Ddi_Bdd_t *f         /* BDD */,
  Ddi_Varset_t *vars  /* Variables */,
  int cubeNumberMax   /* Maximum number of cubes printed */,
  int formatPla       /* Prints a 1 at the end of the cube (PLA format) */,
  char *filename      /* File Name */,
  FILE *fp            /* Pointer to the store file */
  )
{
  int i, flagFile, ncubes, id, ret = 1;
  Ddi_Varset_t *aux;
  char *printvars;
  Ddi_BddMgr *dd;
  int nMgrVars;
  DdNode *f_cu;
  DdManager *mgrCU;

  DdiConsistencyCheck(f,Ddi_Bdd_c);

  if (vars == NULL) {
    vars = Ddi_BddSupp(f);
  } else {
    vars = Ddi_VarsetDup(vars);
  }
  fp = Pdtutil_FileOpen (fp, filename, "w", &flagFile);

  switch (f->common.code) {
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation not supported for meta BDD");
    break;
  case Ddi_Bdd_Mono_c:

    dd = Ddi_ReadMgr(f);
    nMgrVars = Ddi_ReadNumVars(dd);
    mgrCU = Ddi_MgrReadMgrCU(dd);
    
    printvars = Pdtutil_Alloc(char, nMgrVars);
    
    aux = Ddi_VarsetDiffAcc(Ddi_BddSupp(f),vars);
    f = Ddi_BddExist(f,aux);
    Ddi_Free(aux);
    
    for (i=0; i<nMgrVars; i++)
      printvars[i] = 0;
    
    while ( !Ddi_VarsetIsVoid (vars) ) {
      id = Ddi_VarIndex(Ddi_VarsetTop(vars));
      printvars[id] = 1;
      Ddi_VarsetNextAcc(vars); 
    }
    
    /*
     *  Here we work in CUDD !
     */
    
    f_cu = Ddi_BddToCU(f);
    {
      int *cube;
      CUDD_VALUE_TYPE value;
      DdGen *gen;
    
      ncubes = 0;
      Cudd_ForeachCube(mgrCU,f_cu,gen,cube,value) {
        if ((cubeNumberMax>0) && (++ncubes>cubeNumberMax)) {
          break; 
        }
        for (i=0; i<mgrCU->size; i++) {
          /* skip variables not to be printed */
          if (printvars[i]==0) {
            continue;
          }
    
    	switch (cube[i]) {
    	  case 0:
              fprintf (fp, "0");
              break;
            case 1:
              fprintf (fp, "1");
              break;
            case 2:
              fprintf (fp, "-");
              break;
    	  default:
              fprintf (fp, "?");
              break;
          }
        }
        if (formatPla == 1) {
          fprintf (fp, " 1");
        }
        fprintf (fp,"\n");
      }
    }
    
    /*
     *  End working in CUDD !
     */
    
    Pdtutil_Free (printvars);
    Ddi_Free (f);
      
    break;
  case Ddi_Bdd_Part_Conj_c:
  case Ddi_Bdd_Part_Disj_c:
    fprintf(fp,"Partitioned BDD\n");
    for (i=0; i<Ddi_BddPartNum(f); i++) {
      fprintf(fp,"Partition %d\n", i);
      Ddi_BddPrintCubes (Ddi_BddPartRead(f,i),
        vars,cubeNumberMax,formatPla,NULL,fp);
    }
    break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
  }

  Ddi_Free(vars);

  Pdtutil_FileClose (fp, &flagFile);

  return (ret);
}

/**Function********************************************************************
  Synopsis    [Stores BDD on file]
  Description [This function stores only a BDD (not a BDD array).
    The BDD is stored in the DDDMP format. The parameter "mode" 
    can be DDDMP_MODE_TEXT, DDDMP_MODE_BINARY or
    DDDMP_MODE_DEFAULT.<br>
    The function returns 1 if succefully stored, 0 otherwise.
    ]
  SideEffects [None]
  SeeAlso     [Ddi_BddLoad,Dddmp_cuddBddStore]
******************************************************************************/

int
Ddi_BddStore (
  Ddi_Bdd_t  *f       /* BDD */,
  char *ddname       /* dd name (or NULL) */,
  char mode          /* storing mode */,
  char *filename     /* file name */,
  FILE *fp           /* pointer to the store file */ )
{
  int flagFile, ret, i;
  Ddi_BddMgr *dd;
  char **vnames;
  int *vauxids;


  DdiConsistencyCheck(f,Ddi_Bdd_c);

  dd = Ddi_ReadMgr(f);    /* dd manager */
  vnames = Ddi_MgrReadVarnames(dd);
  vauxids = Ddi_MgrReadVarauxids(dd);

  fp = Pdtutil_FileOpen (fp, filename, "w", &flagFile);
  if (fp == NULL) {
     return(0);
  }

  switch (f->common.code) {
  case Ddi_Bdd_Mono_c:
    fprintf(fp,"# MONO\n");
    ret = Dddmp_cuddBddStore(dd->mgrCU,ddname,Ddi_BddToCU(f),
      vnames,vauxids,mode,DDDMP_VARNAMES,filename,fp);
    break;
  case Ddi_Bdd_Meta_c:
    Pdtutil_Assert(0,"operation not supported for meta BDD");
    break;
  case Ddi_Bdd_Part_Conj_c:
    fprintf(fp,"# CPART\n");
    ret = DdiArrayStore (f->data.part,ddname,vnames,
      NULL,vauxids,mode,filename,fp);
    break;
  case Ddi_Bdd_Part_Disj_c:
    if (dd->settings.dump.partOnSingleFile) {
      fprintf(fp,"# DPART\n");
      ret = DdiArrayStore (f->data.part,ddname,vnames,
        NULL,vauxids,mode,filename,fp);
    }
    else {
      char name[PDTUTIL_MAX_STR_LEN];
      fprintf(fp,"# FILEDPART\n");
      for (i=0;i<Ddi_BddPartNum(f);i++) {
        fprintf(fp,"%s-p%d\n",filename,i);
        Ddi_BddStore(Ddi_BddPartRead(f,i),NULL,mode,name,NULL);
      }
    }
    break;
  default:
    Pdtutil_Assert(0,"illegal bdd node code");
  }

  Pdtutil_FileClose (fp, &flagFile);

  return(1);
}

/**Function********************************************************************
  Synopsis    [Loads BDD from file]
  Description [This function loads only a BDD. If the file contain a BDDs'
    array, then will be load only the first BDD.<br>
    The BDD on file must be in the DDDMP format. The parameter 
    "mode" can be DDDMP_MODE_TEXT, DDDMP_MODE_BINARY or
    DDDMP_MODE_DEFAULT.<br>
    The function returns the pointer of BDD root if succefully
    loaded, NULL otherwise.]
  SideEffects [None]
  SeeAlso     [Ddi_BddStore,Dddmp_cuddBddLoad]
******************************************************************************/

Ddi_Bdd_t *
Ddi_BddLoad (
  Ddi_BddMgr *dd      /* dd manager */,
  int varmatchmode   /* variable matching mode */,
  char mode          /* loading mode */,
  char *filename     /* file name */,  
  FILE *fp           /* file pointer */
  )             
{
  Ddi_Bdd_t *f, *p;
  int flagFile;
  char code[200], auxname[200];
  Ddi_Bddarray_t *array;    /* descriptor of array */
  char **vnames = Ddi_MgrReadVarnames(dd);
  int *vauxids = Ddi_MgrReadVarauxids(dd);
  int firstC;

  f = NULL;

  fp = Pdtutil_FileOpen (fp, filename, "r", &flagFile);
  if (fp == NULL) {
    return (0);
  }

  firstC = fgetc(fp);
  ungetc (firstC, fp);
  if (((char)firstC) != '#') {
    /*
     *  No format comment: default is mono (1 component) 
     *  or conjunctive part (>1 components)
     */
    array = Ddi_BddarrayLoad(dd,
             vnames,vauxids,mode,filename,fp);
    if (Ddi_BddarrayNum(array)==1) {
      f = Ddi_BddDup(Ddi_BddarrayRead(array,0));
      Ddi_Free(array);
    }
    else {
      f = Ddi_BddMakePartConjFromArray(array);
      Ddi_Free(array);
    }  
  }
  else {
    if (fgets(code,199,fp)==NULL)
      return NULL;
    if ((strncmp(code,"# MONO",6)==0)
      || (strncmp(code,"MONO",4)==0)) {
      /*
       * monolithic (1 component) 
       */
      f = Ddi_BddMakeFromCU(dd,
        Dddmp_cuddBddLoad(dd->mgrCU,varmatchmode,vnames,vauxids,
        NULL,mode,filename,fp));
    }
    if ((strncmp(code,"# CPART",7)==0)
       || (strncmp(code,"CPART",5)==0)) {
      /*
       * conjunctive part 
       */
      array = Ddi_BddarrayLoad(dd,
        vnames,vauxids,mode,filename,fp);
      f = Ddi_BddMakePartConjFromArray(array);
      Ddi_Free(array);
    }
    if ((strncmp(code,"# DPART",7)==0)
      || (strncmp(code,"DPART",5)==0)) {
      /*
       * disjunctive part 
       */
      array = Ddi_BddarrayLoad(dd,
               vnames,vauxids,mode,filename,fp);
      f = Ddi_BddMakePartDisjFromArray(array);
      Ddi_Free(array);
    }
    if ((strncmp(code,"# FILEDPART",11)==0)
      || (strncmp(code,"FILEDPART",9)==0)) {
      /*
       * disjunctive part - partitions on separate files
       */
      f = NULL;
      while (fscanf(fp,"%s",auxname)!=EOF) {
        p = Ddi_BddLoad(dd,varmatchmode,mode,auxname,NULL);
        if (f==NULL) {
          f = Ddi_BddMakePartDisjFromMono(p);
	}
        else {
          Ddi_BddPartInsertLast(f,p);
        }
        Ddi_Free(p);
      }
    }
    if ((strncmp(code,"# EXPR",6)==0)
      || (strncmp(code,"EXPR",4)==0)) {
      /*
       * expression 
       */
      f = Ddi_ReadCube(dd,fp,0);
    }
  }

  Pdtutil_FileClose (fp, &flagFile);

  return (f);
}


/**Function********************************************************************
  Synopsis    [Compute the Dense Super or Subset of a Boolean functions]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddDenseSet (
  Ddi_DenseMethod_e method      /* Operation Code */,
  Ddi_Bdd_t *f                   /* Operand */,
  int threshold,
  int safe,
  int quality,
  double hardlimit
 )
{
  int suppSize;
  DdNode *sourceCudd, *resultCudd;
  Ddi_Bdd_t *result;
  Ddi_BddMgr *dd = Ddi_ReadMgr(f);
  DdManager *cuddMgr = Ddi_MgrReadMgrCU (dd);
  Ddi_Varset_t *supp;

  supp = Ddi_BddSupp (f);
  suppSize = Ddi_VarsetNum(supp);
  sourceCudd = Ddi_BddToCU(f);

  switch (method) {
    case Ddi_DenseNone_c:
      resultCudd = sourceCudd;
      break;
    case Ddi_DenseSubHeavyBranch_c:
      resultCudd = Cudd_SubsetHeavyBranch (cuddMgr, sourceCudd, suppSize,
        threshold);
      break;
    case Ddi_DenseSupHeavyBranch_c:
      resultCudd = Cudd_SupersetHeavyBranch (cuddMgr, sourceCudd, suppSize,
        threshold);
      break;
    case Ddi_DenseSubShortPath_c:
      resultCudd = Cudd_SubsetShortPaths (cuddMgr, sourceCudd, suppSize,
        threshold, hardlimit);
      break;
    case Ddi_DenseSupShortPath:
      resultCudd = Cudd_SupersetShortPaths (cuddMgr, sourceCudd, suppSize,
        threshold, hardlimit);
      break;
    case Ddi_DenseUnderApprox_c:
      resultCudd = Cudd_UnderApprox (cuddMgr, sourceCudd, suppSize, threshold,
        safe, quality);
      break;
    case Ddi_DenseOverApprox_c:
      resultCudd = Cudd_OverApprox (cuddMgr, sourceCudd, suppSize, threshold,
        safe, quality);
      break;
    case Ddi_DenseRemapUnder_c:
      resultCudd = Cudd_RemapUnderApprox (cuddMgr, sourceCudd, suppSize,
        threshold, quality);
      break;
    case Ddi_DenseRemapOver_c:
      resultCudd = Cudd_RemapOverApprox (cuddMgr, sourceCudd, suppSize,
        threshold, quality);
      break;
    case Ddi_DenseSubCompress_c:
      resultCudd = Cudd_SubsetCompress (cuddMgr, sourceCudd, suppSize,
        threshold);
      break;
    case Ddi_DenseSupCompress_c:
      resultCudd = Cudd_SupersetCompress (cuddMgr, sourceCudd, suppSize,
        threshold);
      break;
    default:
      resultCudd = sourceCudd;
      break;
  }

  result = Ddi_BddMakeFromCU (dd, resultCudd);

  Ddi_Free (supp);
  return (result);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/





