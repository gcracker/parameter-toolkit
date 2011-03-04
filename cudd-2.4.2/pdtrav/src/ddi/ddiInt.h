/**CHeaderFile*****************************************************************

  FileName    [ddiInt.h]

  PackageName [ddi]

  Synopsis    [Decision diagram interface for the PdTrav package.]

  Description [External functions and data strucures of the DDI package]

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

#ifndef _DDIINT
#define _DDIINT

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "ddi.h"
#include "part.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define DDI_ARRAY_INIT_SIZE      4  

/* 
 *  activate garbage collection any specified number of free 
 *  This is only handle management, BDD nodes are dereferenced 
 *  any time a free is called.
 */
#define DDI_GARBAGE_THRESHOLD    100000  


/* 
 * max value in enum. Used for array size and iteration limit 
 */
#define DDI_NTYPE ((int)Ddi_Exprarray_c+1)

#define DDI_META_GROUP_SIZE_MIN_DEFAULT    10  

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct Ddi_Meta_t Ddi_Meta_t;
typedef struct Ddi_ArrayData_t Ddi_ArrayData_t;
typedef struct DdNode Ddi_BddNode_t;

/**Enum************************************************************************
  Synopsis    [DDI block types.]
  Description [DDI block types. Used for run time checks]
******************************************************************************/

typedef enum {
  Ddi_Bdd_c,
  Ddi_Var_c,
  Ddi_Varset_c,
  Ddi_Expr_c,
  Ddi_Bddarray_c,
  Ddi_Vararray_c,
  Ddi_Varsetarray_c,
  Ddi_Exprarray_c
}
Ddi_Type_e;

/**Enum************************************************************************
  Synopsis    [DDI block status.]
  Description [DDI block status.]
******************************************************************************/
typedef enum {
  Ddi_Free_c,
  Ddi_Unlocked_c,
  Ddi_Locked_c,
  Ddi_SchedFree_c,
  Ddi_Null_c
}
Ddi_Status_e;

/**Enum************************************************************************
  Synopsis    [Selector for accumulate/generate operation type.]
******************************************************************************/
typedef enum {
  Ddi_Accumulate_c,
  Ddi_Generate_c
} Ddi_OpType_e;

/**Enum************************************************************************
  Synopsis    [Selector for copy operation type.]
******************************************************************************/
typedef enum {
  Ddi_Dup_c,
  Ddi_Mov_c
} Ddi_CopyType_e;


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************
 Synopsis    [Dd Manager]
 Description [The Ddi Dd manager]
******************************************************************************/

struct Ddi_Mgr_t {
  char *name;                            /* DDI Structure Name */
  DdManager *mgrCU;                      /* CUDD manager */
  struct Ddi_Bdd_t *one;                  /* one constant */
  struct Ddi_Bdd_t *zero;                 /* zero constant */
  char **varnames;                       /* names of variables */
  int *varauxids;                        /* auxiliary variable ids */
  int vararraySize;                      /* size of varnames and varauxids */

  Ddi_Varsetarray_t *varGroups;          /* groups of variables */

  int currNodeId;                        /* integer unique id of DDI node */
  int tracedId;                          /* ids >= tracedId are logged 
                                            at creation */

  int freeNum;
  int lockedNum;
  int allocatedNum;
  int genericNum;
  int typeNum[DDI_NTYPE];
  int typeLockedNum[DDI_NTYPE];

  union Ddi_Generic_t *nodeList;
  struct Ddi_Vararray_t *variables;

  Cudd_ReorderingType autodynMethod; /* suspended autodyn method */
  int autodynSuspended;              /* flag active if dynord suspended */
  int autodynNestedSuspend;          /* counter of nested dynord suspend */
  int abortOnSift;                   /* enable abort op when sifting called */
  int abortedOp;                     /* flagging logging aborted operation */

  struct {
    int groupNum;
    Ddi_Varsetarray_t *groups; 
    int nvar;
    int *ord;
    int bddNum;
    int id;
  } meta;

  struct {
    Pdtutil_VerbLevel_e verbosity;
    struct {
      int groupSizeMin;
      Ddi_Meta_Method_e method;
    } meta;
    struct {
      int existClustThresh;
    } part;
    struct {
      int partOnSingleFile;
    } dump;
  } settings;

  struct {
    int peakProdLocal;
    int peakProdGlobal;
  } stats;

};	

/**Struct*********************************************************************
 Synopsis  [Meta BDD description]
 Description [Meta BDD description]
******************************************************************************/

struct Ddi_Meta_t {
  Ddi_Bddarray_t *one, *zero, *dc;
  int metaId;
};


/**Struct*********************************************************************
 Synopsis  [DDI Arrays]
******************************************************************************/

struct Ddi_ArrayData_t {
  int num;              /* number of array elements */
  int nSize;            /* size of allocated data (in number of elements) */
  Ddi_Generic_t **data; /* ptr to the array data*/
};

/**Struct*********************************************************************
 Synopsis  [common fields]
 Description [fields present in all DDI node types]
******************************************************************************/

struct Ddi_Common_t {
  Ddi_Type_e              type;   /* type, for run time checks */
  Ddi_Code_e              code;   /* code, for selection within type */
  Ddi_Status_e            status; /* status */
  Ddi_Mgr_t               *mgr;   /* the manager */
  char                    *name;  /* optional name, useful for debug */
  union Ddi_Generic_t     *next;  /* pointer for linked lists */
  struct Ddi_Info_t       *info;  /* pointer to info block */
  union Ddi_Generic_t     *supp;  /* support */
  int                     nodeid; /* integer id: set for debug purposes */
};	

/**Struct*********************************************************************
 Synopsis  [Boolean function]
 Description [Handle to a Boolean function (a BDD or a partitioned BDD)]
******************************************************************************/

struct Ddi_Bdd_t {
  struct Ddi_Common_t     common;  /* common fields */
  union {
    Ddi_BddNode_t          *bdd;   /* ptr to the top node of the function */
    struct Ddi_ArrayData_t *part;  /* array of partitions */
    struct Ddi_Meta_t      *meta;  /* Meta BDD */
  } data;
};	

/**Struct*********************************************************************
 Synopsis  [Array of Boolean functions]
 Description [Handle to an array of Boolean functions]
******************************************************************************/

struct Ddi_Bddarray_t {
  struct Ddi_Common_t     common; /* common fields */
  struct Ddi_ArrayData_t  *array; /* array data */
};	

/**Struct*********************************************************************
 Synopsis  [Variable]
 Description [Handle to a variable]
******************************************************************************/

struct Ddi_Var_t {
  struct Ddi_Common_t     common; /* common fields */
  struct {
    Ddi_BddNode_t         *bdd;   /* ptr to the BDD node of the variable */
    int                   index;  /* variable index */
  } data;
};	

/**Struct*********************************************************************
 Synopsis  [Array of variables]
 Description [Handle to an array of variables]
******************************************************************************/

struct Ddi_Vararray_t {
  struct Ddi_Common_t     common; /* common fields */
  struct Ddi_ArrayData_t  *array; /* array data */
};	

/**Struct*********************************************************************
 Synopsis  [Variable set]
 Description [Handle to a set of variables (implemented as a Boolean function).
              Variable sets are repressnted as Boolean cubes. So this set
              completely replicates Ddi_dd_t. Internally, all operations are
              handled by casting Ddi_Varset_t* to Ddi_Bdd_t*
             ]
******************************************************************************/

struct Ddi_Varset_t {
  struct Ddi_Common_t     common; /* common fields */
  struct {
    Ddi_BddNode_t         *bdd;  
    Ddi_BddNode_t         *curr;  /* ptr to the current node within walk */
  } data;
};	

/**Struct*********************************************************************
 Synopsis  [Array of variable sets]
 Description [Handle to an array of set of variables]
******************************************************************************/

struct Ddi_Varsetarray_t {
  struct Ddi_Common_t     common; /* common fields */
  struct Ddi_ArrayData_t  *array; /* array data */
};

/**Struct*********************************************************************
 Synopsis  [Expression]
 Description [Handle to an expression (Logic, CTL, ...) represented as a tree]
******************************************************************************/

struct Ddi_Expr_t {
  struct Ddi_Common_t     common; /* common fields */
  union {
    Ddi_Generic_t          *bdd;  /* ptr to a decision diagram */
    struct Ddi_ArrayData_t *sub;  /* operand sub-expressions */
    char                   *string;  /* string identifier */
  } data;
  int                     opcode;
};

/**Struct*********************************************************************
 Synopsis  [Array of expressions]
 Description [Handle to an array of expressions (Logic, CTL, ...)]
******************************************************************************/

struct Ddi_Exprarray_t {
  struct Ddi_Common_t     common; /* common fields */
  struct Ddi_ArrayData_t  *array; /* array data */
};


/**Struct*********************************************************************
 Synopsis  [Generic DDI type]
 Description [Generic DDI type. Useful as general purpose block (e.g. as
              function parameter or array entry), mainly for 
              internal operations (bypassing compiler checks). 
              It is also used in block allocation, to guarantee equal size
              for all allocated DDI blocks]
******************************************************************************/

union Ddi_Generic_t {
  struct Ddi_Common_t      common;
  struct Ddi_Bdd_t         Bdd;
  struct Ddi_Var_t         Var;
  struct Ddi_Varset_t      Varset;
  struct Ddi_Expr_t        Expr;
  struct Ddi_Bddarray_t    Bddarray;
  struct Ddi_Vararray_t    Vararray;
  struct Ddi_Varsetarray_t Varsetarray;
  struct Ddi_Exprarray_t   Exprarray;
} ;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define DdiType(n) (((Ddi_Generic_t*)n)->common.type)

#define DdiConsistencyCheck(n,t) \
{\
  Pdtutil_Assert(n!=NULL,"check failed: NULL DDI node");\
  Pdtutil_Assert(DdiType(n)==t,"wrong node type");\
}
#define DdiConsistencyCheck2(n,t1,t2) \
{\
  Pdtutil_Assert(n!=NULL,"check failed: NULL DDI node");\
  Pdtutil_Assert((DdiType(n)==t1)||(DdiType(n)==t2),"wrong node type");\
}

#define DdiGenericOp1(op,acc,f) \
  DdiGenericOp(op,acc,(Ddi_Generic_t*)(f),NULL,NULL)
#define DdiGenericOp2(op,acc,f,g) \
  DdiGenericOp(op,acc,(Ddi_Generic_t*)(f),(Ddi_Generic_t*)(g),NULL)
#define DdiGenericOp3(op,acc,f,g,h) \
  DdiGenericOp(op,acc,(Ddi_Generic_t*)(f),(Ddi_Generic_t*)(g),\
  (Ddi_Generic_t*)(h))

/*
 *  Functions to be implemented
 */

#define DdiGenericPrint(f,fp) \
  fprintf(stderr,"DdiGenericPrint still to implement\n")
#define DdiGenericPrintStats(f,fp) \
  fprintf(stderr,"DdiGenericPrintStats still to implement\n")

#if 0
#define Part_BddMultiwayLinearAndExist(f,g,a,b,c) \
  (fprintf(stderr,"MultiwayAndExist still to implement\n"),NULL)

#define DdiArrayStore(f,g,a,b,c,x,y,z) \
  (fprintf(stderr,"DdiArrayStore still to implement\n"),0)
#endif


#define DdiMetaCopy(m,f) \
  (fprintf(stderr,"DdiMetaCopy still to implement\n"),NULL)
#define DdiMetaConstrain(f,g) \
  (fprintf(stderr,"DdiMetaConstrain still to implement\n"),NULL)
#define DdiMetaRestrict(f,g) \
  (fprintf(stderr,"DdiMetaRestrict still to implement\n"),NULL)


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Ddi_ArrayData_t * DdiArrayAlloc(int length);
EXTERN void DdiArrayFree(Ddi_ArrayData_t *array);
EXTERN DdNode ** DdiArrayToCU(Ddi_ArrayData_t *array);
EXTERN void DdiArrayWrite(Ddi_ArrayData_t *array, int i, Ddi_Generic_t *f, Ddi_CopyType_e copy);
EXTERN void DdiArrayInsert(Ddi_ArrayData_t *array, int i, Ddi_Generic_t *f, Ddi_CopyType_e copy);
EXTERN Ddi_Generic_t * DdiArrayRead(Ddi_ArrayData_t *array, int i);
EXTERN Ddi_Generic_t * DdiArrayExtract(Ddi_ArrayData_t *array, int i);
EXTERN int DdiArrayNum(Ddi_ArrayData_t *array);
EXTERN Ddi_ArrayData_t * DdiArrayDup(Ddi_ArrayData_t *old);
EXTERN Ddi_ArrayData_t * DdiArrayCopy(Ddi_Mgr_t *dd2, Ddi_ArrayData_t *old);
EXTERN void DdiArrayAppend(Ddi_ArrayData_t *array1, Ddi_ArrayData_t *array2);
EXTERN int DdiArrayStore(Ddi_ArrayData_t *array, char *ddname, char **vnames, char **rnames, int *vauxids, int mode, char *fname, FILE *fp);
EXTERN Ddi_Generic_t * DdiGenericAlloc(Ddi_Type_e type, Ddi_Mgr_t *mgr);
EXTERN void DdiTraceNodeAlloc(Ddi_Generic_t *r);
EXTERN void DdiGenericFree(Ddi_Generic_t *f);
EXTERN Ddi_Generic_t * DdiDeferredFree(Ddi_Generic_t *f);
EXTERN Ddi_Generic_t * DdiGenericDup(Ddi_Generic_t *f);
EXTERN Ddi_Generic_t * DdiGenericCopy(Ddi_Mgr_t *ddm, Ddi_Generic_t *f, Ddi_Vararray_t *varsOld, Ddi_Vararray_t *varsNew);
EXTERN void DdiGenericDataCopy(Ddi_Generic_t *d, Ddi_Generic_t *s);
EXTERN Ddi_Generic_t * DdiGenericOp(Ddi_OpCode_e opcode, Ddi_OpType_e optype, Ddi_Generic_t *f, Ddi_Generic_t *g, Ddi_Generic_t *h);
EXTERN int DdiGenericBddSize(Ddi_Generic_t *f);
EXTERN void DdiMetaFree(Ddi_Meta_t *m);
EXTERN Ddi_Meta_t * DdiMetaDup(Ddi_Meta_t *m);
EXTERN Ddi_Varset_t * DdiMetaSupp(Ddi_Meta_t *m);
EXTERN void DdiMetaDoCompl(Ddi_Meta_t *m);
EXTERN int DdiMetaIsConst(Ddi_Bdd_t *fMeta, int phase);
EXTERN Ddi_Bdd_t * DdiMetaAndExistAcc(Ddi_Bdd_t *fMeta, Ddi_Bdd_t *gBdd, Ddi_Varset_t *smooth);
EXTERN Ddi_Bdd_t * DdiMetaComposeAcc(Ddi_Bdd_t *fMeta, Ddi_Vararray_t *v, Ddi_Bddarray_t *g);
EXTERN Ddi_Bdd_t * DdiMetaSwapVarsAcc(Ddi_Bdd_t *fMeta, Ddi_Vararray_t *v1, Ddi_Vararray_t *v2);
EXTERN Ddi_Bdd_t * DdiMetaSubstVarsAcc(Ddi_Bdd_t *fMeta, Ddi_Vararray_t *v1, Ddi_Vararray_t *v2);
EXTERN Ddi_Bdd_t * DdiMetaAndAcc(Ddi_Bdd_t *fMeta, Ddi_Bdd_t *gMeta);
EXTERN Ddi_Bdd_t * DdiMetaOrAcc(Ddi_Bdd_t *fMeta, Ddi_Bdd_t *gMeta);
EXTERN void DdiMgrCheckVararraySize(Ddi_Mgr_t *dd);
EXTERN void DdiMgrGarbageCollect(Ddi_Mgr_t *ddm);
EXTERN int DdiMgrReadIntRef(Ddi_Mgr_t *dd);
EXTERN void DdiMgrMakeVarGroup(Ddi_BddMgr *dd, Ddi_Var_t *v, int grpSize, int method);
EXTERN void DdiVarNewFromCU(Ddi_BddMgr *ddm, DdNode *varCU);

/**AutomaticEnd***************************************************************/
#endif /* _DDIINT */
