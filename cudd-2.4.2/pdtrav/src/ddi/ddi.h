/**CHeaderFile*****************************************************************

  FileName    [ddi.h]

  PackageName [ddi]

  Synopsis    [Portability interface with the Decision Diagram package.]

  Description [The <b>DDI</b> package provides functions to manipulate the 
    following objects:
    <ul>
    <li>boolean functions (Ddi_Bdd_t)
    <li>arrays of boolean functions (Ddi_Bddarray_t)
    <li>variables (Ddi_Var_t)
    <li>arrays of variables (Ddi_Vararray_t)
    <li>sets of variables (Ddi_Varset_t).
    </ul>
    It can be viewed both as a portability interface, concentrating
    all dependancies from the BDD package, and as an upper level,
    manipulating the previously listed BDD based data types.
    A particular feature, distinguishing this interface from other ones,
    is the support for partitioned (conjunctive as well as disjunctive)
    functions.
    This implementation is written on top of the CUDD package.
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

#ifndef _DDI
#define _DDI

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "pdtutil.h"
#include "cuplus.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*
 *  Codes for generalized Boolean functions.
 */

#define DDI_MONO_BOOLF                    0
#define DDI_CPART_BOOLF                   1
#define DDI_DPART_BOOLF                   2

/*
 *  Default settings.
 */

#define DDI_NO_LIMIT                      ((1<<30)-2)
#define DDI_DFLT_ITE_ON                   TRUE
#define DDI_DFLT_ITE_RESIZE_AT            75
#define DDI_DFLT_ITE_MAX_SIZE             1000000
#define DDI_DFLT_ITE_CONST_ON             TRUE
#define DDI_DFLT_ITE_CONST_RESIZE_AT      75
#define DDI_DFLT_ITE_CONST_MAX_SIZE       1000000
#define DDI_DFLT_ADHOC_ON                 TRUE
#define DDI_DFLT_ADHOC_RESIZE_AT          0
#define DDI_DFLT_ADHOC_MAX_SIZE           10000000
#define DDI_DFLT_GARB_COLLECT_ON          TRUE
#define DDI_DFLT_DAEMON                   NIL(void)
#define DDI_DFLT_MEMORY_LIMIT             BDD_NO_LIMIT
#define DDI_DFLT_NODE_RATIO               2.0
#define DDI_DFLT_INIT_BLOCKS              10

#define DDI_UNIQUE_SLOTS CUDD_UNIQUE_SLOTS
#define DDI_CACHE_SLOTS CUDD_CACHE_SLOTS
#define DDI_MAX_CACHE_SIZE 0

/*
 *  Dynamic reordering.
 */

#define DDI_REORDER_SAME 0
#define DDI_REORDER_NONE 1
#define DDI_REORDER_RANDOM 2
#define DDI_REORDER_RANDOM_PIVOT 3
#define DDI_REORDER_SIFT 4
#define DDI_REORDER_SIFT_CONVERGE 5
#define DDI_REORDER_SYMM_SIFT 6
#define DDI_REORDER_SYMM_SIFT_CONV 7
#define DDI_REORDER_WINDOW2 8
#define DDI_REORDER_WINDOW3 9
#define DDI_REORDER_WINDOW4 10
#define DDI_REORDER_WINDOW2_CONV 11
#define DDI_REORDER_WINDOW3_CONV 12
#define DDI_REORDER_WINDOW4_CONV 13
#define DDI_REORDER_GROUP_SIFT 14
#define DDI_REORDER_GROUP_SIFT_CONV 15
#define DDI_REORDER_ANNEALING 16
#define DDI_REORDER_GENETIC 17
#define DDI_REORDER_LINEAR 18
#define DDI_REORDER_LINEAR_CONVERGE 19
#define DDI_REORDER_EXACT 20

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************
  Synopsis    [DDI block codes.]
  Description [DDI block coses. Used as subtypes for selections within objects
               of same type.]
******************************************************************************/

typedef enum {
  Ddi_Bdd_Mono_c,
  Ddi_Bdd_Part_Conj_c,
  Ddi_Bdd_Part_Disj_c,
  Ddi_Bdd_Meta_c,
  Ddi_Expr_Bool_c,
  Ddi_Expr_Ctl_c,
  Ddi_Expr_String_c,
  Ddi_Expr_Bdd_c,
  Ddi_Null_code_c
}
Ddi_Code_e;

/**Enum************************************************************************
  Synopsis    [Type for operation codes.]
  Description [Type for operation codes. Used for selection of DDI
               Boolean operation.]
******************************************************************************/
typedef enum {
  Ddi_BddNot_c,
  Ddi_BddSwapVars_c,
  Ddi_BddSubstVars_c,
  Ddi_BddCompose_c,
  Ddi_BddMakeMono_c,
  Ddi_BddAnd_c,
  Ddi_BddDiff_c,
  Ddi_BddNand_c,
  Ddi_BddOr_c,
  Ddi_BddNor_c,
  Ddi_BddXor_c,
  Ddi_BddXnor_c,
  Ddi_BddExist_c,
  Ddi_BddForall_c,
  Ddi_BddConstrain_c,
  Ddi_BddRestrict_c,
  Ddi_BddCproject_c,
  Ddi_BddAndExist_c,
  Ddi_BddIte_c,
  Ddi_BddSupp_c,
  Ddi_BddSuppAttach_c,
  Ddi_BddSuppDetach_c,
  Ddi_VarsetNext_c,
  Ddi_VarsetAdd_c,
  Ddi_VarsetRemove_c,
  Ddi_VarsetDiff_c,
  Ddi_VarsetUnion_c,
  Ddi_VarsetIntersect_c,
  Ddi_VarsetSwapVars_c,
  Ddi_VarsetSubstVars_c
} Ddi_OpCode_e;

/**Enum************************************************************************
  Synopsis    [Type for Dense Subsetting.]
  Description []
******************************************************************************/
typedef enum
  {
  Ddi_DenseNone_c,
  Ddi_DenseSubHeavyBranch_c,
  Ddi_DenseSupHeavyBranch_c,
  Ddi_DenseSubShortPath_c,
  Ddi_DenseSupShortPath,
  Ddi_DenseUnderApprox_c,
  Ddi_DenseOverApprox_c,
  Ddi_DenseRemapUnder_c,
  Ddi_DenseRemapOver_c,
  Ddi_DenseSubCompress_c,
  Ddi_DenseSupCompress_c
  }
Ddi_DenseMethod_e;

/**Enum************************************************************************
  Synopsis    [Selector for Meta BDD method.]
******************************************************************************/
typedef enum
{
  Ddi_Meta_Size,
  Ddi_Meta_EarlySched,
  Ddi_Meta_McM
}
Ddi_Meta_Method_e;

typedef struct Ddi_Mgr_t Ddi_BddMgr;
typedef struct Ddi_Mgr_t Ddi_Mgr_t;

typedef struct DdNode Ddi_BddNode;
typedef struct DdNode Ddi_CuddNode;

typedef union  Ddi_Generic_t Ddi_Generic_t;

typedef struct Ddi_GenericArray_t Ddi_GenericArray_t;

typedef struct Ddi_Bdd_t Ddi_Bdd_t;
typedef struct Ddi_Var_t Ddi_Var_t;
typedef struct Ddi_Varset_t Ddi_Varset_t;
typedef struct Ddi_Expr_t Ddi_Expr_t;

typedef struct Ddi_Bddarray_t Ddi_Bddarray_t;
typedef struct Ddi_Vararray_t Ddi_Vararray_t;
typedef struct Ddi_Varsetarray_t Ddi_Varsetarray_t;
typedef struct Ddi_Exprarray_t Ddi_Exprarray_t;

typedef Ddi_Generic_t * (*NPFN)();

/* portability to 2.0 */
typedef struct Ddi_Bdd_t Ddi_Dd_t;
typedef struct Ddi_Bddarray_t Ddi_DdArray_t;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*
 *  Read fields and components
 */

/**Macro***********************************************************************
  Synopsis    [Read handle code.]
  Description [Read handle code. The code field of DDI nodes is a sort of 
    subtype. In case of BDDs, it selects among monolithic, partitioned and 
    meta formats. In other cases it is not used.
    Allowed return values are:
    <ul> 
    <li> Ddi_Bdd_Mono_c (monolithic BDD)
    <li> Ddi_Bdd_Part_Conj_c (disjunctively partitioned BDD)
    <li> Ddi_Bdd_Part_Disj_c (conjunctively partitioned BDD)
    <li> Ddi_Bdd_Meta_c (meta BDD)
    <li> Ddi_Expr_Op_c (operation node in expressions)
    <li> Ddi_Expr_String_c (string identifier terminal node of expressions)
    <li> Ddi_Expr_Bdd_c (BDD terminal of expressions)
    <li> Ddi_Null_code_c (no BDD pointed, possibly wrong DDI node)
    </ul>
  ]
  SideEffects []
******************************************************************************/
#define /* Ddi_Code_e */ Ddi_ReadCode(f /* any DDI node pointer */) \
  Ddi_GenericReadCode((Ddi_Generic_t *)(f))

/**Macro***********************************************************************
  Synopsis    [Read the DDI Manager field.]
  Description [Read the DDI Manager field. This is a pointer to the owner 
    DDI manager.]
  SideEffects []
******************************************************************************/
#define /* Ddi_Mgr_t * */ Ddi_ReadMgr(f /* any DDI node pointer */) \
  Ddi_GenericReadMgr((Ddi_Generic_t *)(f))

/**Macro***********************************************************************
  Synopsis    [Read the name field.]
  Description [Read the name field. Mainly thought for debug/report purposes.]
  SideEffects []
******************************************************************************/
#define /* char * */ Ddi_ReadName(f /* any DDI node pointer */) \
  Ddi_GenericReadName((Ddi_Generic_t *)(f))


/**Macro***********************************************************************
  Synopsis    [Compute Operation generating a BDD (Ddi_Bdd_t).]
  Description [Compute
    unary, binary or tenary operation returning a BDD. Result is generated.
    Requires an operation code and three operand parameters.
    It is also available in the ...Opi version, where i = 1, 2, or 3, is
    the number of operands.
    The same functions are also available in the accumulated version.
    The complete list of related functions follows:
    <ul>
    <li>   Ddi_BddOp(op,f,g,h)
    <li>   Ddi_BddOp1(op,f)    
    <li>   Ddi_BddOp2(op,f,g)  
    <li>   Ddi_BddOp3(op,f,g,h)
    <li>   Ddi_BddOpAcc(op,f,g,h)
    <li>   Ddi_BddOpAcc1(op,f)    
    <li>   Ddi_BddOpAcc2(op,f,g)  
    <li>   Ddi_BddOpAcc3(op,f,g,h)
    </ul>
    whereas the list of available operation codes is:
    <ul>
    <li>   Ddi_BddNot_c,
    <li>   Ddi_BddSwapVars_c,
    <li>   Ddi_BddMakeMono_c,
    <li>   Ddi_BddAnd_c,
    <li>   Ddi_BddDiff_c,
    <li>   Ddi_BddNand_c,
    <li>   Ddi_BddOr_c,
    <li>   Ddi_BddNor_c,
    <li>   Ddi_BddXor_c,
    <li>   Ddi_BddXnor_c,
    <li>   Ddi_BddExist_c,
    <li>   Ddi_BddForall_c,
    <li>   Ddi_BddConstrain_c,
    <li>   Ddi_BddRestrict_c,
    <li>   Ddi_BddCproject_c,
    <li>   Ddi_BddAndExist_c,
    <li>   Ddi_BddIte_c,
    <li>   Ddi_BddSupp_c,
    <li>   Ddi_BddSuppAttach_c,
    <li>   Ddi_BddSuppDetach_c,
    </ul>
    ]
  SideEffects []
  SeeAlso     [Ddi_BddarrayOp]
*****************************************************************************/

#define Ddi_BddOp(op,f,g,h) \
  ((Ddi_Bdd_t*)Ddi_GenericOp(op,\
  (Ddi_Generic_t*)(f),(Ddi_Generic_t*)(g),(Ddi_Generic_t*)(h)))
#define Ddi_BddOpAcc(op,f,g,h) \
  ((Ddi_Bdd_t*)Ddi_GenericOpAcc(op,\
  (Ddi_Generic_t*)(f),(Ddi_Generic_t*)(g),(Ddi_Generic_t*)(h)))

#define Ddi_BddOp1(op,f)             Ddi_BddOp(op,f,NULL,NULL)
#define Ddi_BddOp2(op,f,g)           Ddi_BddOp(op,f,g,NULL)
#define Ddi_BddOp3(op,f,g,h)         Ddi_BddOp(op,f,g,h)
#define Ddi_BddOpAcc1(op,f)          Ddi_BddOpAcc(op,f,NULL,NULL)
#define Ddi_BddOpAcc2(op,f,g)        Ddi_BddOpAcc(op,f,g,NULL)
#define Ddi_BddOpAcc3(op,f,g,h)      Ddi_BddOpAcc(op,f,g,h)

/**Macro***********************************************************************
  Synopsis    [Compute Operation on all entries of an Array of BDDs.]
  Description [The function, for all elements of the array, computes
    unary, binary or tenary operation involving the <em>i-th</em> element 
    of the array. Result is generated.
    The function requires an operation code and three operand parameters.
    It is also available in the ...Op<i> version, where <i> = 1, 2, or 3, is
    the number of operands.
    All functions operate in accumulated mode, i.e. they modify the first 
    parameter instead of generating a new array.
    The complete list of related functions follows:
    </ul>
    <li>   Ddi_BddarrayOp(op,f,g,h)
    <li>   Ddi_BddarrayOp1(op,f)    
    <li>   Ddi_BddarrayOp2(op,f,g)  
    <li>   Ddi_BddarrayOp3(op,f,g,h)
    </ul>
    ]
  SideEffects []
  SeeAlso     [Ddi_BddOp]
*****************************************************************************/

#define Ddi_BddarrayOp(op,f,g,h) \
  ((Ddi_Bddarray_t*)Ddi_GenericOpAcc(op,\
  (Ddi_Generic_t*)(f),(Ddi_Generic_t*)(g),(Ddi_Generic_t*)(h)))

#define Ddi_BddarrayOp1(op,f)        Ddi_BddarrayOp(op,f,NULL,NULL)
#define Ddi_BddarrayOp2(op,f,g)      Ddi_BddarrayOp(op,f,g,NULL)
#define Ddi_BddarrayOp3(op,f,g,h)    Ddi_BddarrayOp(op,f,g,h)


/**Macro***********************************************************************
  Synopsis    [Free DDI node.]
  Description [Free DDI node (compatible with all DDI handles).]
  SideEffects []
******************************************************************************/
#define Ddi_Free(f/* any DDI node pointer */) \
  (((f)!=NULL)?(Ddi_GenericFree((Ddi_Generic_t *)(f)),(f)=NULL):NULL)

/**Macro***********************************************************************
  Synopsis    [Lock DDI node.]
  Description [Lock DDI node so that cannot be freed unless unlocked.
    Used as a protection mechanism for internal objects (array entries,
    partitions, ...]
  SideEffects [Ddi_Unlock]
******************************************************************************/
#define  Ddi_Lock(f/* any DDI node pointer */) \
  Ddi_GenericLock((Ddi_Generic_t *)(f))

/**Macro***********************************************************************
  Synopsis    [Unlock DDI node.]
  Description [Unlock DDI node so that can be freed again.]
  SideEffects [Ddi_Lock]
******************************************************************************/
#define  Ddi_Unlock(f/* any DDI node pointer */) \
  Ddi_GenericUnlock((Ddi_Generic_t *)(f));

/**Macro***********************************************************************
  Synopsis     [Set name field of DDI node]
  Description  [Set name field of DDI node]
  SideEffects []
******************************************************************************/
#define Ddi_SetName(f/* any DDI node pointer */, /* char * */ name) \
  Ddi_GenericSetName((Ddi_Generic_t *)(f),name)


/**Macro***********************************************************************
  Synopsis     [Read name field of DDI node]
  Description  [Read name field of DDI node]
  SideEffects []
******************************************************************************/
#define Ddi_ReadName(f/* any DDI node pointer */) \
  Ddi_GenericReadName((Ddi_Generic_t *)(f))



#define DDI_MAX(a,b)         ((a>b)? a:b)

/* hide dependancy on the CUDD package */

#define ddiP(F) cuddP((Ddi_ReadMgr(F))->mgrCU,Ddi_BddToCU(F))
#define Ddi_MgrAutodynDisable(dd) Cudd_AutodynDisable(Ddi_MgrReadMgrCU(dd))
#define Ddi_MgrAutodynEnable(dd,n) Cudd_AutodynEnable(Ddi_MgrReadMgrCU(dd),n)
#define Ddi_ReduceHeap(dd,m,n) Cudd_ReduceHeap(Ddi_MgrReadMgrCU(dd),m,n)
#define Ddi_MakeTreeNode(dd,i,j,mode) Cudd_MakeTreeNode(Ddi_MgrReadMgrCU(dd),i,j,mode)
#define Ddi_CheckZeroRef(dd) Cudd_CheckZeroRef(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadReorderingStatus(dd,pmethod) \
                       Cudd_ReorderingStatus(Ddi_MgrReadMgrCU(dd),\
                        ((Cudd_ReorderingType *)pmethod))
#define Ddi_ReadPerm(dd,i) Cudd_ReadPerm(Ddi_MgrReadMgrCU(dd),i)
#define Ddi_ReadInvPerm(dd,i) Cudd_ReadInvPerm(Ddi_MgrReadMgrCU(dd),i)
#define Ddi_ReadSize(dd) Cudd_ReadSize(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadNumVars(dd) (Cudd_ReadSize(Ddi_MgrReadMgrCU(dd)))
#define Ddi_ReadSlots(dd) Cudd_ReadSlots(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadKeys(dd) Cudd_ReadKeys(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadLiveNodes(dd) \
  (Cudd_ReadKeys(Ddi_MgrReadMgrCU(dd))-Cudd_ReadDead(Ddi_MgrReadMgrCU(dd)))
#define Ddi_ReadPeakNodeCount(dd) \
  (Cudd_ReadPeakNodeCount(Ddi_MgrReadMgrCU(dd)))
#define Ddi_ReadPeakLiveNodeCount(dd) \
  (Cudd_ReadPeakLiveNodeCount(Ddi_MgrReadMgrCU(dd)))
#define Ddi_ReadMinDead(dd) Cudd_ReadMinDead(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadMemoryInUse(dd) Cudd_ReadMemoryInUse(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadReorderings(dd) Cudd_ReadReorderings(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadReorderingTime(dd) Cudd_ReadReorderingTime(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadGarbageCollections(dd) Cudd_ReadGarbageCollections(Ddi_MgrReadMgrCU(dd))
#define Ddi_ReadGarbageCollectionTime(dd) Cudd_ReadGarbageCollectionTime(Ddi_MgrReadMgrCU(dd))

#define Ddi_BddEval(dd,f,inputs) Cudd_Eval(Ddi_MgrReadMgrCU(dd),Ddi_BddToCU(f),inputs)
#define Ddi_BddIsConstant(F) Cudd_IsConstant(Ddi_BddToCU(F))
#define Ddi_BddIsComplement(F) Cudd_IsComplement(Ddi_BddToCU(F))

#define Ddi_BddSetMaxCacheHard(dd,n) \
 Cudd_SetMaxCacheHard(Ddi_MgrReadMgrCU(dd),n)
#define Ddi_BddSetMaxMemory(dd,n) Cudd_SetMaxMemory(Ddi_MgrReadMgrCU(dd),n)
#define Ddi_BddSetNextReordering(dd,n) \
  Cudd_SetNextReordering(Ddi_MgrReadMgrCU(dd),n)

/* old pdtrav supported function */
#define Ddi_CuddNodeIsVisited(fcu) \
  Cudd_IsComplement(Cudd_Regular(fcu)->next)
#define Ddi_CuddNodeSetVisited(fcu) \
{\
  Cudd_Regular(fcu)->next = Cudd_Complement(Cudd_Regular(fcu)->next);\
}
#define Ddi_CuddNodeClearVisited(fcu) \
{\
  if (Ddi_CuddNodeIsVisited(fcu)) {\
    Cudd_Regular(fcu)->next = Cudd_Regular(Cudd_Regular(fcu)->next);\
  }\
}

#define Ddi_CuddNodeElse(fcu) \
  (Cudd_IsComplement(fcu) ? Cudd_Not(Cudd_E(fcu)) : Cudd_E(fcu))

#define Ddi_CuddNodeThen(fcu) \
  (Cudd_IsComplement(fcu) ? Cudd_Not(Cudd_T(fcu)) : Cudd_T(fcu))
 
#define Ddi_CuddNodeIsConstant(F) Cudd_IsConstant(F)
#define Ddi_CuddNodeReadIndex(F) Cudd_NodeReadIndex(F)


#define Ddi_BddPrintDbg(msg,f,fp) \
{\
  Ddi_Varset_t *supp;\
  supp = Ddi_BddSupp(f);\
  fprintf(fp,"%s",msg);\
  Ddi_VarsetPrint(supp,100,NULL,fp);\
  Ddi_BddPrintCubes(f,NULL,100,0,NULL,fp);\
  Ddi_Free(supp);\
}

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Ddi_Bdd_t * Ddi_BddMakeFromCU(Ddi_Mgr_t *mgr, DdNode *bdd);
EXTERN Ddi_BddNode * Ddi_BddToCU(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddMakeLiteral(Ddi_Var_t *v, int polarity);
EXTERN Ddi_Bdd_t * Ddi_BddMakeConst(Ddi_Mgr_t *mgr, int value);
EXTERN Ddi_Bdd_t * Ddi_BddMakePartConjFromMono(Ddi_Bdd_t *mono);
EXTERN Ddi_Bdd_t * Ddi_BddMakePartConjVoid(Ddi_Mgr_t *mgr);
EXTERN Ddi_Bdd_t * Ddi_BddMakePartDisjVoid(Ddi_Mgr_t *mgr);
EXTERN Ddi_Bdd_t * Ddi_BddMakePartDisjFromMono(Ddi_Bdd_t *mono);
EXTERN Ddi_Bdd_t * Ddi_BddMakePartConjFromArray(Ddi_Bddarray_t *array);
EXTERN Ddi_Bdd_t * Ddi_BddMakePartDisjFromArray(Ddi_Bddarray_t *array);
EXTERN Ddi_Bdd_t * Ddi_BddSetPartConj(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddSetPartDisj(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddRelMakeFromArray(Ddi_Bddarray_t *Fa, Ddi_Vararray_t *Va);
EXTERN Ddi_Bdd_t * Ddi_BddDup(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddCopy(Ddi_Mgr_t *ddm, Ddi_Bdd_t *old);
EXTERN Ddi_Bdd_t * Ddi_BddCopyRemapVars(Ddi_Mgr_t *ddm, Ddi_Bdd_t *old, Ddi_Vararray_t *varsOld, Ddi_Vararray_t *varsNew);
EXTERN int Ddi_BddSize(Ddi_Bdd_t *f);
EXTERN Ddi_Var_t * Ddi_BddTopVar(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddEvalFree(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN int Ddi_BddEqual(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN int Ddi_BddIsZero(Ddi_Bdd_t *f);
EXTERN int Ddi_BddIsOne(Ddi_Bdd_t *f);
EXTERN double Ddi_CountMinterm(Ddi_Bdd_t *f, int nvar);
EXTERN int Ddi_BddIncluded(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddNot(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddNotAcc(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddAnd(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddAndAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddDiff(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddDiffAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddNand(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddNandAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddOr(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddOrAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddNor(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddNorAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddXor(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddXorAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddXnor(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddXnorAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddIte(Ddi_Bdd_t *f, Ddi_Bdd_t *g, Ddi_Bdd_t *h);
EXTERN Ddi_Bdd_t * Ddi_BddIteAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g, Ddi_Bdd_t *h);
EXTERN Ddi_Bdd_t * Ddi_BddExist(Ddi_Bdd_t *f, Ddi_Varset_t *vars);
EXTERN Ddi_Bdd_t * Ddi_BddExistAcc(Ddi_Bdd_t *f, Ddi_Varset_t *vars);
EXTERN Ddi_Bdd_t * Ddi_BddForall(Ddi_Bdd_t *f, Ddi_Varset_t *vars);
EXTERN Ddi_Bdd_t * Ddi_BddForallAcc(Ddi_Bdd_t *f, Ddi_Varset_t *vars);
EXTERN Ddi_Bdd_t * Ddi_BddAndExist(Ddi_Bdd_t *f, Ddi_Bdd_t *g, Ddi_Varset_t *vars);
EXTERN Ddi_Bdd_t * Ddi_BddAndExistAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g, Ddi_Varset_t *vars);
EXTERN Ddi_Bdd_t * Ddi_BddCofactor(Ddi_Bdd_t *f, Ddi_Var_t *v, int phase);
EXTERN Ddi_Bdd_t * Ddi_BddCofactorAcc(Ddi_Bdd_t *f, Ddi_Var_t *v, int phase);
EXTERN Ddi_Bdd_t * Ddi_BddConstrain(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddConstrainAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddRestrict(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddRestrictAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddCofexist(Ddi_Bdd_t *f, Ddi_Bdd_t *g, Ddi_Varset_t *smooth);
EXTERN Ddi_Bdd_t * Ddi_BddCproject(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddCprojectAcc(Ddi_Bdd_t *f, Ddi_Bdd_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddSwapVars(Ddi_Bdd_t *f, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN Ddi_Bdd_t * Ddi_BddSwapVarsAcc(Ddi_Bdd_t *f, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN Ddi_Bdd_t * Ddi_BddSubstVars(Ddi_Bdd_t *f, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN Ddi_Bdd_t * Ddi_BddSubstVarsAcc(Ddi_Bdd_t *f, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN Ddi_Bdd_t * Ddi_BddCompose(Ddi_Bdd_t *f, Ddi_Vararray_t *x, Ddi_Bddarray_t *g);
EXTERN Ddi_Bdd_t * Ddi_BddComposeAcc(Ddi_Bdd_t *f, Ddi_Vararray_t *x, Ddi_Bddarray_t *g);
EXTERN int Ddi_BddIsCube(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddPickOneCube(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddPickOneCubeAcc(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddPickOneMinterm(Ddi_Bdd_t *f, Ddi_Varset_t *vars);
EXTERN Ddi_Bdd_t * Ddi_BddPickOneMintermAcc(Ddi_Bdd_t *f, Ddi_Varset_t *vars);
EXTERN Ddi_Varset_t * Ddi_BddSupp(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddSuppAttach(Ddi_Bdd_t *f);
EXTERN Ddi_Varset_t * Ddi_BddSuppRead(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddSuppDetach(Ddi_Bdd_t *f);
EXTERN int Ddi_BddIsMono(Ddi_Bdd_t *f);
EXTERN int Ddi_BddIsMeta(Ddi_Bdd_t *f);
EXTERN int Ddi_BddIsPartConj(Ddi_Bdd_t *f);
EXTERN int Ddi_BddIsPartDisj(Ddi_Bdd_t *f);
EXTERN int Ddi_BddPartNum(Ddi_Bdd_t * f);
EXTERN Ddi_Bdd_t * Ddi_BddPartRead(Ddi_Bdd_t * f, int i);
EXTERN Ddi_Bdd_t * Ddi_BddPartWrite(Ddi_Bdd_t *f, int i, Ddi_Bdd_t *newp);
EXTERN Ddi_Bdd_t * Ddi_BddPartExtract(Ddi_Bdd_t * f, int i);
EXTERN Ddi_Bdd_t * Ddi_BddPartInsert(Ddi_Bdd_t *f, int i, Ddi_Bdd_t *newp);
EXTERN Ddi_Bdd_t * Ddi_BddPartInsertLast(Ddi_Bdd_t *f, Ddi_Bdd_t *newp);
EXTERN Ddi_Bdd_t * Ddi_BddMakeMono(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddSetMono(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddMakeClustered(Ddi_Bdd_t *f, int threshold);
EXTERN Ddi_Bdd_t * Ddi_BddSetClustered(Ddi_Bdd_t *f, int threshold);
EXTERN void Ddi_BddPrint(Ddi_Bdd_t *f, FILE *fp);
EXTERN void Ddi_BddPrintStats(Ddi_Bdd_t *f, FILE *fp);
EXTERN int Ddi_BddPrintCubeToString(Ddi_Bdd_t *f, Ddi_Varset_t *vars, char *string);
EXTERN int Ddi_BddPrintCubes(Ddi_Bdd_t *f, Ddi_Varset_t *vars, int cubeNumberMax, int formatPla, char *filename, FILE *fp);
EXTERN int Ddi_BddStore(Ddi_Bdd_t *f, char *ddname, char mode, char *filename, FILE *fp);
EXTERN Ddi_Bdd_t * Ddi_BddLoad(Ddi_BddMgr *dd, int varmatchmode, char mode, char *filename, FILE *fp);
EXTERN Ddi_Bdd_t * Ddi_BddDenseSet(Ddi_DenseMethod_e method, Ddi_Bdd_t *f, int threshold, int safe, int quality, double hardlimit);
EXTERN Ddi_Bddarray_t * Ddi_BddarrayAlloc(Ddi_Mgr_t *mgr, int length);
EXTERN Ddi_Bddarray_t * Ddi_BddarrayMakeFromCU(Ddi_Mgr_t *mgr, DdNode **array, int n);
EXTERN DdNode ** Ddi_BddarrayToCU(Ddi_Bddarray_t *array);
EXTERN Ddi_Bddarray_t * Ddi_BddarrayMakeFromBddPart(Ddi_Bdd_t *part);
EXTERN int Ddi_BddarrayNum(Ddi_Bddarray_t *array);
EXTERN void Ddi_BddarrayWrite(Ddi_Bddarray_t *array, int pos, Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddarrayRead(Ddi_Bddarray_t *array, int i);
EXTERN void Ddi_BddarrayClear(Ddi_Bddarray_t *array, int pos);
EXTERN void Ddi_BddarrayInsert(Ddi_Bddarray_t *array, int pos, Ddi_Bdd_t *f);
EXTERN void Ddi_BddarrayInsertLast(Ddi_Bddarray_t *array, Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddarrayExtract(Ddi_Bddarray_t *array, int i);
EXTERN void Ddi_BddarrayRemove(Ddi_Bddarray_t *array, int pos);
EXTERN Ddi_Bddarray_t * Ddi_BddarrayDup(Ddi_Bddarray_t *old);
EXTERN Ddi_Bddarray_t * Ddi_BddarrayCopy(Ddi_BddMgr *ddm, Ddi_Bddarray_t *old);
EXTERN void Ddi_BddarrayAppend(Ddi_Bddarray_t *array1, Ddi_Bddarray_t *array2);
EXTERN int Ddi_BddarraySize(Ddi_Bddarray_t *array);
EXTERN int Ddi_BddarrayStore(Ddi_Bddarray_t *array, char *ddname, char **vnames, char **rnames, int *vauxids, int mode, char *fname, FILE *fp);
EXTERN Ddi_Bddarray_t * Ddi_BddarrayLoad(Ddi_BddMgr *dd, char **vnames, int *vauxids, int mode, char *file, FILE *fp);
EXTERN Ddi_Varset_t * Ddi_BddarraySupp(Ddi_Bddarray_t *array);
EXTERN Ddi_Varset_t ** Ddi_BddarraySuppArray(Ddi_Bddarray_t *fArray);
EXTERN Ddi_Expr_t * Ddi_ExprMakeFromBdd(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_ExprToBdd(Ddi_Expr_t *e);
EXTERN Ddi_Expr_t * Ddi_ExprMakeFromString(Ddi_Mgr_t *mgr, char *s);
EXTERN char * Ddi_ExprToString(Ddi_Expr_t *e);
EXTERN Ddi_Expr_t * Ddi_ExprCtlMake(Ddi_Mgr_t *mgr, int opcode, Ddi_Expr_t *op1, Ddi_Expr_t *op2, Ddi_Expr_t *op3);
EXTERN Ddi_Expr_t * Ddi_ExprBoolMake(Ddi_Mgr_t *mgr, Ddi_Expr_t *op1, Ddi_Expr_t *op2);
EXTERN Ddi_Expr_t * Ddi_ExprWriteSub(Ddi_Expr_t *e, int pos, Ddi_Expr_t *op);
EXTERN int Ddi_ExprSubNum(Ddi_Expr_t *e);
EXTERN Ddi_Expr_t * Ddi_ExprReadSub(Ddi_Expr_t *e, int i);
EXTERN int Ddi_ExprReadOpcode(Ddi_Expr_t *e);
EXTERN int Ddi_ExprIsTerminal(Ddi_Expr_t *e);
EXTERN Ddi_Expr_t * Ddi_ExprLoad(Ddi_BddMgr *dd, char *filename, FILE *fp);
EXTERN Ddi_Expr_t * Ddi_ExprDup(Ddi_Expr_t *f);
EXTERN void Ddi_ExprPrint(Ddi_Expr_t *f, FILE *fp);
EXTERN Ddi_Generic_t * Ddi_GenericOp(Ddi_OpCode_e opcode, Ddi_Generic_t *f, Ddi_Generic_t *g, Ddi_Generic_t *h);
EXTERN Ddi_Generic_t * Ddi_GenericOpAcc(Ddi_OpCode_e opcode, Ddi_Generic_t *f, Ddi_Generic_t *g, Ddi_Generic_t *h);
EXTERN Ddi_Generic_t * Ddi_GenericDup(Ddi_Generic_t *f);
EXTERN void Ddi_GenericLock(Ddi_Generic_t *f);
EXTERN void Ddi_GenericUnlock(Ddi_Generic_t *f);
EXTERN void Ddi_GenericFree(Ddi_Generic_t *f);
EXTERN void Ddi_GenericSetName(Ddi_Generic_t *f, char *name);
EXTERN Ddi_Code_e Ddi_GenericReadCode(Ddi_Generic_t *f);
EXTERN Ddi_Mgr_t * Ddi_GenericReadMgr(Ddi_Generic_t * f);
EXTERN char * Ddi_GenericReadName(Ddi_Generic_t * f);
EXTERN int Ddi_MetaActive(Ddi_Mgr_t *ddm);
EXTERN void Ddi_MetaInit(Ddi_Mgr_t *ddm, Ddi_Meta_Method_e method, Ddi_Bdd_t *ref, Ddi_Varset_t *firstGroup, int sizeMin);
EXTERN void Ddi_MetaQuit(Ddi_Mgr_t *ddm);
EXTERN Ddi_Bdd_t * Ddi_BddMakeMeta(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddSetMeta(Ddi_Bdd_t *f);
EXTERN Ddi_Bddarray_t * Ddi_BddarrayMakeMeta(Ddi_Bddarray_t *f);
EXTERN Ddi_Bddarray_t * Ddi_BddArraySetMeta(Ddi_Bddarray_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddMakeFromMeta(Ddi_Bdd_t *f);
EXTERN Ddi_Bdd_t * Ddi_BddFromMeta(Ddi_Bdd_t *f);
EXTERN Ddi_Mgr_t * Ddi_MgrInit(char *ddiName, DdManager *CUMgr, unsigned int nvar, unsigned int numSlots, unsigned int cacheSize, unsigned long memorySizeMax);
EXTERN int Ddi_MgrConsistencyCheck(Ddi_Mgr_t *ddm);
EXTERN int Ddi_MgrCheckExtRef(Ddi_Mgr_t *ddm, int n);
EXTERN void Ddi_MgrPrintExtRef(Ddi_Mgr_t *ddm, int minNodeId);
EXTERN void Ddi_MgrUpdate(Ddi_Mgr_t *ddm);
EXTERN void Ddi_MgrQuit(Ddi_Mgr_t *dd);
EXTERN Ddi_Mgr_t * Ddi_MgrDup(Ddi_Mgr_t *dd);
EXTERN void Ddi_MgrShuffle(Ddi_Mgr_t *dd, int *sortedIds, int nids);
EXTERN void Ddi_MgrAlign(Ddi_Mgr_t *dd, Ddi_Mgr_t *ddRef);
EXTERN void Ddi_MgrCreateGroups2(Ddi_Mgr_t *dd, Ddi_Vararray_t *vfix, Ddi_Vararray_t *vmov);
EXTERN int Ddi_MgrOrdWrite(Ddi_Mgr_t *dd, char *filename, FILE *fp, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN int Ddi_MgrReadOrdNamesAuxids(Ddi_Mgr_t *dd, char *filename, FILE *fp, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN void Ddi_MgrAutodynSuspend(Ddi_Mgr_t *dd);
EXTERN void Ddi_MgrAutodynResume(Ddi_Mgr_t *dd);
EXTERN void Ddi_MgrAbortOnSiftEnable(Ddi_Mgr_t *dd);
EXTERN void Ddi_MgrAbortOnSiftDisable(Ddi_Mgr_t *dd);
EXTERN int Ddi_MgrOperation(Ddi_Mgr_t **ddMgrP, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlagP);
EXTERN void Ddi_MgrPrintStats(Ddi_Mgr_t *dd);
EXTERN DdManager * Ddi_MgrReadMgrCU(Ddi_Mgr_t *dd);
EXTERN Ddi_Bdd_t * Ddi_MgrReadOne(Ddi_Mgr_t *dd);
EXTERN Ddi_Bdd_t * Ddi_MgrReadZero(Ddi_Mgr_t *dd);
EXTERN int Ddi_MgrReadCurrNodeId(Ddi_Mgr_t *dd);
EXTERN void Ddi_MgrSetTracedId(Ddi_Mgr_t *dd, int id);
EXTERN void Ddi_MgrSetExistClustThresh(Ddi_Mgr_t *dd, int th);
EXTERN int Ddi_MgrReadExistClustThresh(Ddi_Mgr_t *dd);
EXTERN char** Ddi_MgrReadVarnames(Ddi_Mgr_t *dd);
EXTERN int * Ddi_MgrReadVarauxids(Ddi_Mgr_t *dd);
EXTERN int Ddi_MgrReadExtRef(Ddi_Mgr_t *dd);
EXTERN int Ddi_MgrReadExtBddRef(Ddi_Mgr_t *dd);
EXTERN int Ddi_MgrReadExtBddarrayRef(Ddi_Mgr_t *dd);
EXTERN int Ddi_MgrReadExtVarsetRef(Ddi_Mgr_t *dd);
EXTERN unsigned int Ddi_MgrReadDynordThresh(Ddi_Mgr_t *dd);
EXTERN char * Ddi_ReadDdiName(Ddi_Mgr_t *dd);
EXTERN void Ddi_SetDdiName(Ddi_Mgr_t *dd, char *ddiName);
EXTERN Pdtutil_VerbLevel_e Ddi_MgrReadVerbosity(Ddi_Mgr_t *ddiMgr);
EXTERN void Ddi_MgrSetVerbosity(Ddi_Mgr_t *ddiMgr, Pdtutil_VerbLevel_e verbosity);
EXTERN int Ddi_MgrReadPeakProdLocal(Ddi_Mgr_t *ddiMgr);
EXTERN int Ddi_MgrReadPeakProdGlobal(Ddi_Mgr_t *ddiMgr);
EXTERN void Ddi_MgrPeakProdLocalReset(Ddi_Mgr_t *ddiMgr);
EXTERN void Ddi_MgrPeakProdUpdate(Ddi_Mgr_t *ddiMgr, int size);
EXTERN void Ddi_MgrSetMgrCU(Ddi_Mgr_t *dd, DdManager *m);
EXTERN void Ddi_MgrSetOne(Ddi_Mgr_t *dd, Ddi_Bdd_t *one);
EXTERN void Ddi_MgrSetZero(Ddi_Mgr_t *dd, Ddi_Bdd_t *zero);
EXTERN void Ddi_MgrSetVarnames(Ddi_Mgr_t *dd, char **vn);
EXTERN void Ddi_MgrSetVarauxids(Ddi_Mgr_t *dd, int *va);
EXTERN void Ddi_MgrSetDynordThresh(Ddi_Mgr_t *dd, unsigned int th);
EXTERN unsigned int Ddi_ReadCacheSlots(Ddi_Mgr_t *dd);
EXTERN double Ddi_ReadCacheLookUps(Ddi_Mgr_t *dd);
EXTERN double Ddi_ReadCacheHits(Ddi_Mgr_t *dd);
EXTERN unsigned int Ddi_ReadMinHit(Ddi_Mgr_t *dd);
EXTERN unsigned int Ddi_ReadMaxCacheHard(Ddi_Mgr_t *dd);
EXTERN unsigned int Ddi_ReadMaxCache(Ddi_Mgr_t* dd);
EXTERN void Ddi_MgrPrintAllocStats(Ddi_Mgr_t *ddm, FILE *fp);
EXTERN int Ddi_BddOperation(Ddi_Mgr_t *defaultDdMgr, Ddi_Bdd_t **bddP, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlag);
EXTERN int Ddi_BddarrayOperation(Ddi_Mgr_t *defaultDdMgr, Ddi_Bddarray_t **bddArrayP, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlag);
EXTERN Ddi_Bdd_t * Ddi_ReadCube(Ddi_Mgr_t *dd, FILE *fp, int idOrName);
EXTERN Ddi_Varset_t * Ddi_ReadVarset(Ddi_Mgr_t *dd, FILE *fp, int idOrName);
EXTERN Ddi_Bdd_t * Ddi_VarSubst(Ddi_Bdd_t *f, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN void Ddi_PrintVararray(Ddi_Vararray_t *array);
EXTERN void Ddi_PrintDdArray(Ddi_Bddarray_t *array);
EXTERN Cuplus_PruneHeuristic_e Ddi_ProfileHeuristicString2Enum(char *string);
EXTERN char * Ddi_ProfileHeuristicEnum2String(Cuplus_PruneHeuristic_e enumType);
EXTERN Cudd_ReorderingType Ddi_ReorderingMethodString2Enum(char *string);
EXTERN char * Ddi_ReorderingMethodEnum2String(Cudd_ReorderingType enumType);
EXTERN Ddi_DenseMethod_e Ddi_DenseMethodString2Enum(char *string);
EXTERN char * Ddi_DenseMethodEnum2String(Ddi_DenseMethod_e enumType);
EXTERN void Ddi_PrintCuddVersion(FILE *fp);
EXTERN int Ddi_BddPrintSupportAndCubes(Ddi_Bdd_t *f, int numberPerRow, int cubeNumberMax, int formatPla, char *filename, FILE *fp);
EXTERN int Ddi_BddarrayPrintSupportAndCubes(Ddi_Bddarray_t *fArray, int numberPerRow, int cubeNumberMax, int formatPla, int reverse, char *filename, FILE *fp);
EXTERN int Ddi_VarIndex(Ddi_Var_t *var);
EXTERN Ddi_Var_t * Ddi_IthVar(Ddi_BddMgr *ddm, int index);
EXTERN Ddi_Var_t * Ddi_VarAtLevel(Ddi_BddMgr *ddm, int lev);
EXTERN Ddi_Var_t * Ddi_VarNewBeforeVar(Ddi_Var_t *var);
EXTERN Ddi_Var_t * Ddi_VarNewAfterVar(Ddi_Var_t *var);
EXTERN Ddi_Var_t * Ddi_VarNew(Ddi_BddMgr *ddm);
EXTERN Ddi_Var_t * Ddi_VarNewAtLevel(Ddi_BddMgr *ddm, int lev);
EXTERN int Ddi_VarCurrPos(Ddi_Var_t *var);
EXTERN char * Ddi_VarName(Ddi_Var_t *var);
EXTERN void Ddi_VarAttachName(Ddi_Var_t *var, char *name);
EXTERN void Ddi_VarDetachName(Ddi_Var_t *var);
EXTERN int Ddi_VarAuxid(Ddi_Var_t *var);
EXTERN void Ddi_VarAttachAuxid(Ddi_Var_t *var, int auxid);
EXTERN Ddi_Var_t * Ddi_VarFromName(Ddi_BddMgr *ddm, char *name);
EXTERN Ddi_Var_t * Ddi_VarFromAuxid(Ddi_BddMgr *ddm, int auxid);
EXTERN DdNode * Ddi_VarToCU(Ddi_Var_t * v);
EXTERN Ddi_Var_t * Ddi_VarFromCU(Ddi_BddMgr *ddm, DdNode * v);
EXTERN Ddi_Var_t * Ddi_VarCopy(Ddi_BddMgr *dd2, Ddi_Var_t *v);
EXTERN void Ddi_VarMakeGroup(Ddi_BddMgr *dd, Ddi_Var_t *v, int grpSize);
EXTERN void Ddi_VarMakeGroupFixed(Ddi_BddMgr *dd, Ddi_Var_t *v, int grpSize);
EXTERN Ddi_Varset_t * Ddi_VarReadGroup(Ddi_Var_t *v);
EXTERN int Ddi_VarIsGrouped(Ddi_Var_t *v);
EXTERN Ddi_Vararray_t * Ddi_VararrayMakeFromCU(Ddi_Mgr_t *mgr, DdNode **array, int n);
EXTERN DdNode ** Ddi_VararrayToCU(Ddi_Vararray_t *array);
EXTERN Ddi_Vararray_t * Ddi_VararrayMakeFromInt(Ddi_Mgr_t *mgr, int *array, int n);
EXTERN int * Ddi_VararrayToInt(Ddi_Vararray_t *array);
EXTERN Ddi_Vararray_t * Ddi_VararrayAlloc(Ddi_Mgr_t *mgr, int size);
EXTERN int Ddi_VararrayNum(Ddi_Vararray_t *array);
EXTERN void Ddi_VararrayWrite(Ddi_Vararray_t *array, int pos, Ddi_Var_t *var);
EXTERN Ddi_Var_t * Ddi_VararrayRead(Ddi_Vararray_t *array, int i);
EXTERN void Ddi_VararrayClear(Ddi_Vararray_t *array, int pos);
EXTERN void Ddi_VararrayInsert(Ddi_Vararray_t *array, int pos, Ddi_Var_t *v);
EXTERN void Ddi_VararrayInsertLast(Ddi_Vararray_t *array, Ddi_Var_t *v);
EXTERN Ddi_Var_t * Ddi_VararrayExtract(Ddi_Vararray_t *array, int i);
EXTERN void Ddi_VararrayRemove(Ddi_Vararray_t *array, int pos);
EXTERN Ddi_Vararray_t * Ddi_VararrayDup(Ddi_Vararray_t *old);
EXTERN Ddi_Vararray_t * Ddi_VararrayCopy(Ddi_BddMgr *ddm, Ddi_Vararray_t *old);
EXTERN void Ddi_VararrayAppend(Ddi_Vararray_t *array1, Ddi_Vararray_t *array2);
EXTERN Ddi_Varset_t * Ddi_VarsetVoid(Ddi_BddMgr *ddm);
EXTERN int Ddi_VarsetIsVoid(Ddi_Varset_t *varset);
EXTERN Ddi_Varset_t * Ddi_VarsetNext(Ddi_Varset_t *vs);
EXTERN Ddi_Varset_t * Ddi_VarsetNextAcc(Ddi_Varset_t *vs);
EXTERN Ddi_Varset_t * Ddi_VarsetAdd(Ddi_Varset_t *vs, Ddi_Var_t *v);
EXTERN Ddi_Varset_t * Ddi_VarsetAddAcc(Ddi_Varset_t *vs, Ddi_Var_t *v);
EXTERN Ddi_Varset_t * Ddi_VarsetRemove(Ddi_Varset_t *vs, Ddi_Var_t *v);
EXTERN Ddi_Varset_t * Ddi_VarsetRemoveAcc(Ddi_Varset_t *vs, Ddi_Var_t *v);
EXTERN Ddi_Varset_t * Ddi_VarsetUnion(Ddi_Varset_t *v1, Ddi_Varset_t *v2);
EXTERN Ddi_Varset_t * Ddi_VarsetUnionAcc(Ddi_Varset_t *v1, Ddi_Varset_t *v2);
EXTERN Ddi_Varset_t * Ddi_VarsetIntersect(Ddi_Varset_t *v1, Ddi_Varset_t *v2);
EXTERN Ddi_Varset_t * Ddi_VarsetIntersectAcc(Ddi_Varset_t *v1, Ddi_Varset_t *v2);
EXTERN Ddi_Varset_t * Ddi_VarsetDiff(Ddi_Varset_t *v1, Ddi_Varset_t *v2);
EXTERN Ddi_Varset_t * Ddi_VarsetDiffAcc(Ddi_Varset_t *v1, Ddi_Varset_t *v2);
EXTERN Ddi_Varset_t * Ddi_VarsetDup(Ddi_Varset_t *src);
EXTERN Ddi_Varset_t * Ddi_VarsetCopy(Ddi_BddMgr *dd2, Ddi_Varset_t *src);
EXTERN Ddi_Varset_t * Ddi_VarsetEvalFree(Ddi_Varset_t *f, Ddi_Varset_t *g);
EXTERN int Ddi_VarsetNum(Ddi_Varset_t *vars);
EXTERN void Ddi_VarsetPrint(Ddi_Varset_t *vars, int numberPerRow, char *filename, FILE *fp);
EXTERN int Ddi_VarInVarset(Ddi_Varset_t *varset, Ddi_Var_t *var);
EXTERN int Ddi_VarsetEqual(Ddi_Varset_t *varset1, Ddi_Varset_t *varset2);
EXTERN Ddi_Var_t * Ddi_VarsetTop(Ddi_Varset_t *varset);
EXTERN Ddi_Var_t * Ddi_VarsetBottom(Ddi_Varset_t *varset);
EXTERN Ddi_BddNode * Ddi_VarsetToCU(Ddi_Varset_t *vs);
EXTERN Ddi_Varset_t * Ddi_VarsetMakeFromCU(Ddi_Mgr_t *mgr, DdNode *bdd);
EXTERN Ddi_Varset_t * Ddi_VarsetMakeFromVar(Ddi_Var_t *v);
EXTERN Ddi_Varset_t * Ddi_VarsetMakeFromArray(Ddi_Vararray_t *va);
EXTERN Ddi_Varset_t * Ddi_VarsetSwapVars(Ddi_Varset_t *vs, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN Ddi_Varset_t * Ddi_VarsetSwapVarsAcc(Ddi_Varset_t *vs, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN Ddi_Varset_t * Ddi_VarsetSubstVars(Ddi_Varset_t *vs, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN Ddi_Varset_t * Ddi_VarsetSubstVarsAcc(Ddi_Varset_t *vs, Ddi_Vararray_t *x, Ddi_Vararray_t *y);
EXTERN void Ddi_VarsetWalkStart(Ddi_Varset_t *vs);
EXTERN void Ddi_VarsetWalkStep(Ddi_Varset_t *vs);
EXTERN int Ddi_VarsetWalkEnd(Ddi_Varset_t *vs);
EXTERN Ddi_Var_t * Ddi_VarsetWalkCurr(Ddi_Varset_t *vs);
EXTERN Ddi_Varsetarray_t * Ddi_VarsetarrayAlloc(Ddi_Mgr_t *mgr, int length);
EXTERN int Ddi_VarsetarrayNum(Ddi_Varsetarray_t *array);
EXTERN void Ddi_VarsetarrayWrite(Ddi_Varsetarray_t *array, int pos, Ddi_Varset_t *vs);
EXTERN void Ddi_VarsetarrayInsert(Ddi_Varsetarray_t *array, int pos, Ddi_Varset_t *vs);
EXTERN void Ddi_VarsetarrayInsertLast(Ddi_Varsetarray_t *array, Ddi_Varset_t *vs);
EXTERN Ddi_Varset_t * Ddi_VarsetarrayRead(Ddi_Varsetarray_t *array, int i);
EXTERN void Ddi_VarsetarrayClear(Ddi_Varsetarray_t *array, int pos);
EXTERN Ddi_Varsetarray_t * Ddi_VarsetarrayDup(Ddi_Varsetarray_t *old);
EXTERN Ddi_Varsetarray_t * Ddi_VarsetarrayCopy(Ddi_BddMgr *ddm, Ddi_Varsetarray_t *old);

/**AutomaticEnd***************************************************************/

#endif /* _DDI */
