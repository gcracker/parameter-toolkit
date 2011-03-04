/**CFile***********************************************************************

  FileName    [ddiVarset.c]

  PackageName [ddi] 

  Synopsis    [Functions to manage <em>sets of variables</em>]

  Description [Sets of variables (var-set) are implemented as cubes.<BR>
               The basic manipulation allowed on a var-set are:<BR> 
               <OL>
               <LI>Create an empty var-set (<B>New</B>) 
	             and destroy one (<B>Free</B>)
               <LI><B>Add</B> a new variable or <B>remove</B> one
               <LI><B>Dup</B>licate a var-set
               <LI><B>Union</B>, <B>intersect</B> and <B>difference</B>
                   of two var-sets
               <LI>Other specific operations
               </OL>]

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
/* Structure declaration                                                     */ 
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declaration                                                          */ 
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declaration                                                      */ 
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declaration                                                         */ 
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
/* Definition of exported function                                           */ 
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [Return an empty var-set]
  Description [The varset is generated, so it must eventually be freed]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetVoid (
  Ddi_BddMgr *ddm
)
{
  DdNode *oneCU;

  oneCU = Ddi_BddToCU(Ddi_MgrReadOne(ddm));
  return (Ddi_VarsetMakeFromCU(ddm,oneCU));
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if var-set is empty]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/

int
Ddi_VarsetIsVoid (
  Ddi_Varset_t *varset 
)
{
  return (Cudd_IsConstant(Ddi_VarsetToCU(varset)));
}

/**Function********************************************************************
  Synopsis    [Remove top variable from var-set. Result generated]
  SideEffects [none]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetNext (
  Ddi_Varset_t *vs
)
{
  return ((Ddi_Varset_t *)DdiGenericOp1(Ddi_VarsetNext_c,
    Ddi_Generate_c,vs));
}

/**Function********************************************************************
  Synopsis    [Remove top variable from var-set. Result accumulated]
  SideEffects [none]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetNextAcc (
  Ddi_Varset_t *vs
)
{
  return ((Ddi_Varset_t *)DdiGenericOp1(Ddi_VarsetNext_c,
    Ddi_Accumulate_c,vs));
}

/**Function********************************************************************
  Synopsis    [Add variable to var-set. Result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetAddAcc Ddi_VarsetRemove Ddi_VarsetRemoveAcc]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetAdd (
  Ddi_Varset_t *vs     /* var-set */,
  Ddi_Var_t *v         /* the new variable added to var-set */
)
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetAdd_c,
    Ddi_Generate_c,vs,v));
}

/**Function********************************************************************
  Synopsis    [Add variable to var-set. Result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetAdd]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetAddAcc (
  Ddi_Varset_t *vs   /* var-set */,
  Ddi_Var_t *v       /* the new variable added to var-set */
  )
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetAdd_c,
    Ddi_Accumulate_c,vs,v));
}

/**Function********************************************************************
  Synopsis    [Remove variable from var-set. Result is generated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetAdd Ddi_VarsetRemoveAcc]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetRemove (
  Ddi_Varset_t *vs,
  Ddi_Var_t *v         
)
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetRemove_c,
    Ddi_Generate_c,vs,v));
}

/**Function********************************************************************
  Synopsis    [Remove variable from var-set. Result is accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetAdd Ddi_VarsetRemove]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetRemoveAcc (
  Ddi_Varset_t *vs,
  Ddi_Var_t *v         
)
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetRemove_c,
    Ddi_Accumulate_c,vs,v));
}

/**Function********************************************************************
  Synopsis    [Return union of two var-sets. Result generated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetUnionAcc,Ddi_VarsetIntersect,Ddi_VarsetDiff]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetUnion (
  Ddi_Varset_t *v1    /* first var-set */ ,
  Ddi_Varset_t *v2    /* second var-set */
  )
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetUnion_c,
    Ddi_Generate_c,v1,v2));
}

/**Function********************************************************************
  Synopsis    [Return union of two var-sets. Result accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetUnion]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetUnionAcc (
  Ddi_Varset_t *v1    /* first var-set */ ,
  Ddi_Varset_t *v2    /* second var-set */
  )
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetUnion_c,
    Ddi_Accumulate_c,v1,v2));
}

/**Function********************************************************************
  Synopsis    [Return intersection of two var-sets. Result generated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetUnion,Ddi_VarsetIntersectAcc,Ddi_VarsetDiff]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetIntersect (
  Ddi_Varset_t *v1    /* first var-set */ ,
  Ddi_Varset_t *v2    /* second var-set */
  )
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetIntersect_c,
    Ddi_Generate_c,v1,v2));
}

/**Function********************************************************************
  Synopsis    [Return intersection of two var-sets. Result accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetUnion]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetIntersectAcc (
  Ddi_Varset_t *v1    /* first var-set */ ,
  Ddi_Varset_t *v2    /* second var-set */
  )
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetIntersect_c,
    Ddi_Accumulate_c,v1,v2));
}

/**Function********************************************************************
  Synopsis    [Return difference of two var-sets. Result generated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetUnionAcc,Ddi_VarsetIntersect,Ddi_VarsetDiff]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetDiff (
  Ddi_Varset_t *v1    /* first var-set */ ,
  Ddi_Varset_t *v2    /* second var-set */
  )
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetDiff_c,
    Ddi_Generate_c,v1,v2));
}

/**Function********************************************************************
  Synopsis    [Return difference of two var-sets. Result accumulated]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetUnion]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetDiffAcc (
  Ddi_Varset_t *v1    /* first var-set */ ,
  Ddi_Varset_t *v2    /* second var-set */
  )
{
  return((Ddi_Varset_t *)DdiGenericOp2(Ddi_VarsetDiff_c,
    Ddi_Accumulate_c,v1,v2));
}

/**Function********************************************************************
  Synopsis    [Duplicate a var-set]
  SideEffects [none]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetDup (  
  Ddi_Varset_t *src   /* var-set to be copied */
)
{
  return ((Ddi_Varset_t *)DdiGenericDup((Ddi_Generic_t *)src));
}

/**Function********************************************************************
  Synopsis    [Copy a var-set to another manager]
  SideEffects [none]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetCopy (  
  Ddi_BddMgr *dd2 /* destination manager */,
  Ddi_Varset_t *src   /* var-set to be copied */ 
)
{
  return ((Ddi_Varset_t *)DdiGenericCopy(dd2,(Ddi_Generic_t *)src,NULL,NULL));
}

/**Function********************************************************************
  Synopsis    [Evaluates an expression and frees first argument]
  Description []
  SideEffects [none]
  SeeAlso     []
******************************************************************************/

Ddi_Varset_t *
Ddi_VarsetEvalFree(
  Ddi_Varset_t *f  /* expression */,
  Ddi_Varset_t *g  /* varset to be freed */
  )
{
 Ddi_Free(g);
 return f;
}


/**Function********************************************************************
  Synopsis    [Return the number of variables in varset]
  SideEffects [none]
******************************************************************************/
int
Ddi_VarsetNum (
  Ddi_Varset_t *vars  /* varset */
)
{
  return (Cudd_DagSize(Ddi_VarsetToCU(vars))-1);
}

/**Function********************************************************************
  Synopsis    [Print the varset variables sorted by id]
  Description [If numberPerRow is less or equal to 0 all the names are
    printed-out on a single row.
    ]
  SideEffects [none]
******************************************************************************/
void
Ddi_VarsetPrint (
  Ddi_Varset_t *vars  /* Varset to Print */,
  int numberPerRow    /* Number of Names Printed on a Single Row */,
  char *filename      /* File Name */,
  FILE *fp            /* output file pointer */
)
{
  Ddi_Varset_t *src;
  Ddi_Var_t *var;
  char *name;
  int i, id, flagReturn = 0, flagFile, numberPrinted;
  char *buf;
  Ddi_Mgr_t *dd = Ddi_ReadMgr(vars);

  DdiConsistencyCheck(vars,Ddi_Varset_c);

  fp = Pdtutil_FileOpen (fp, filename, "w", &flagFile);
  buf = Pdtutil_Alloc (char, (Ddi_ReadNumVars (dd) + 1));
  for (i=0; i<Ddi_ReadNumVars(dd); i++) {
    buf[i]='.';
  }
  buf[i]='\0';

  src = Ddi_VarsetDup (vars);
  while ( !Ddi_VarsetIsVoid (src) ) {
    var = Ddi_VarsetTop (src);
    id = Ddi_VarIndex(var);
    buf[id] = '*';
    Ddi_VarsetNextAcc(src); 
  }
  Ddi_Free(src);

  for (numberPrinted=0, id=0; id<Ddi_ReadNumVars (dd); id++) {
    if (buf[id]=='*') {
      if ((name = Ddi_VarName (Ddi_IthVar(dd, id))) != NULL) {
        fprintf (fp, "%s", name);
      } else { 
        fprintf (fp, "<ID:%d>", id);
      }
      numberPrinted++;
      if ( (numberPerRow>0) && ((numberPrinted%numberPerRow)==0) ) {
        fprintf (fp, "\n");
        flagReturn = 1;
      } else {
        fprintf (fp, " ");
        flagReturn = 0;
      }
    }
  }

  /* Last Return to Print-out? */
  if (flagReturn == 0) {
    fprintf (fp, "\n");
  }

  Pdtutil_Free (buf);
  Pdtutil_FileClose (fp, &flagFile);

  return;
}

/**Function********************************************************************
 Synopsis    [Return true (non 0) if var is in varset]
 SideEffects [none]
******************************************************************************/
int
Ddi_VarInVarset (
  Ddi_Varset_t *varset /* var-set */,
  Ddi_Var_t    *var    /* variable */ )
{
  Ddi_Varset_t *tmp;
  int var_in_set;

  tmp = Ddi_VarsetAdd(varset,var);
  var_in_set = Ddi_VarsetEqual(varset,tmp);
  Ddi_Free(tmp);

  return(var_in_set);
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if the two var-sets are equal]
  SideEffects [none]
******************************************************************************/
int
Ddi_VarsetEqual (
  Ddi_Varset_t *varset1  /* first var-set */,
  Ddi_Varset_t *varset2  /* second var-set */)
{
  return (Ddi_VarsetToCU(varset1)==Ddi_VarsetToCU(varset2));
}

/**Function********************************************************************
  Synopsis    [Return the top variable (in the ordering) in varset]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetBottom]
******************************************************************************/

Ddi_Var_t *
Ddi_VarsetTop (
  Ddi_Varset_t *varset   /* var-set */
)
{
  Ddi_Mgr_t *ddm;

  DdiConsistencyCheck(varset,Ddi_Varset_c);
  if (Ddi_VarsetIsVoid (varset))
    return (NULL);
  ddm=Ddi_ReadMgr(varset);
  return (Ddi_IthVar(ddm,Cudd_NodeReadIndex(Ddi_VarsetToCU(varset))));
}

/**Function********************************************************************
  Synopsis    [Return the bottom variable in the ordering]
  SideEffects [none]
  SeeAlso     [Ddi_VarsetTop]
******************************************************************************/
Ddi_Var_t *
Ddi_VarsetBottom (
  Ddi_Varset_t *varset   /* var-set */
)
{
  DdNode *prev, *next;
  Ddi_Mgr_t *ddm;

  DdiConsistencyCheck(varset,Ddi_Varset_c);
  ddm=Ddi_ReadMgr(varset);
  if (Ddi_VarsetIsVoid (varset))
    return (NULL);

  prev = Ddi_VarsetToCU(varset);
  next = Cudd_T(prev);  
  while ( !Cudd_IsConstant(next) ) {
    prev = next;
    next = Cudd_T(prev);  
  }

  return (Ddi_IthVar(ddm,Cudd_NodeReadIndex(prev)));
}

/**Function********************************************************************
  Synopsis    [Convert a varset to a Cudd cube]
  SideEffects [Convert a varset to a Cudd cube. 
    The returned node is NOT referenced]
  SeeAlso     [Ddi_VarsetMakeFromCU]
******************************************************************************/
Ddi_BddNode *
Ddi_VarsetToCU(
  Ddi_Varset_t *vs
)
{
  DdiConsistencyCheck(vs,Ddi_Varset_c);
  return (vs->data.bdd);
}

/**Function********************************************************************
  Synopsis    [Build a Ddi_Varset_t from a given CUDD cube.]
  Description [Builds the Ddi_Varset_t structure from manager and node.
               The reference count of the node is increased.]
  SideEffects []
  SeeAlso     [DdiGenericAlloc]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetMakeFromCU (
  Ddi_Mgr_t *mgr,
  DdNode *bdd
)
{
  Ddi_Varset_t *r;

  Pdtutil_Assert(cuddCheckCube(mgr->mgrCU,bdd),
    "CUDD variable set must be a cube");

  r = (Ddi_Varset_t *)DdiGenericAlloc(Ddi_Varset_c,mgr);
  r->common.code = Ddi_Bdd_Mono_c;
  r->data.bdd = bdd;
  Cudd_Ref(bdd);

  return (r);
}

/**Function********************************************************************
  Synopsis    [Build a Ddi_Varset_t from a given variable.]
  Description [Builds a Ddi_Varset_t structure from variable.]
  SideEffects []
  SeeAlso     [Ddi_VarsetMakeFromCU Ddi_VarsetMakeFromArray]
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetMakeFromVar (
  Ddi_Var_t *v
)
{
  Ddi_Varset_t *r;
  Ddi_Mgr_t *dd = Ddi_ReadMgr(v);

  r = Ddi_VarsetVoid(dd);
  Ddi_VarsetAddAcc(r,v);

  return (r);
}

/**Function********************************************************************
  Synopsis    [Build a Ddi_Varset_t from a given variable array.]
  Description [Build a Ddi_Varset_t from a given variable array.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetMakeFromArray (
  Ddi_Vararray_t *va
)
{
  Ddi_Mgr_t *mgr;
  Ddi_Varset_t *r;
  int i;
 
  DdiConsistencyCheck(va,Ddi_Vararray_c);

  mgr = Ddi_ReadMgr(va);
  r = Ddi_VarsetVoid(mgr);
  for (i=0; i<Ddi_VararrayNum(va); i++) {
    Ddi_VarsetAddAcc(r,Ddi_VararrayRead(va,i));
  }
  return (r);
}

/**Function********************************************************************
  Synopsis    [Swap two sets of variables in varset. Result generated]
  Description [Return a new varset with the variables swapped 
               (x replaces y and viceversa).]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetSwapVars(
  Ddi_Varset_t   *vs    /* input varset */,
  Ddi_Vararray_t *x    /* 1-st array of variables */,
  Ddi_Vararray_t *y    /* 2-nd array of variables */
  )
{
  return ((Ddi_Varset_t *)DdiGenericOp3(Ddi_VarsetSwapVars_c,
    Ddi_Generate_c,vs,x,y));
}

/**Function********************************************************************
  Synopsis    [Swap two sets of variables in varset. Result accumulated]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetSwapVarsAcc (
  Ddi_Varset_t   *vs    /* input varset */,
  Ddi_Vararray_t *x    /* 1-st array of variables */,
  Ddi_Vararray_t *y    /* 2-nd array of variables */
  )
{
  return ((Ddi_Varset_t *)DdiGenericOp3(Ddi_VarsetSwapVars_c,
    Ddi_Accumulate_c,vs,x,y));
}

/**Function********************************************************************
  Synopsis    [Variable substitution in varset. Result generated]
  Description [Return a new varset with variable substitution x <- y.]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetSubstVars(
  Ddi_Varset_t   *vs    /* input varset */,
  Ddi_Vararray_t *x    /* 1-st array of variables */,
  Ddi_Vararray_t *y    /* 2-nd array of variables */
  )
{
  return ((Ddi_Varset_t *)DdiGenericOp3(Ddi_VarsetSubstVars_c,
    Ddi_Generate_c,vs,x,y));
}

/**Function********************************************************************
  Synopsis    [Variable substitution in varset. Result accumulated]
  Description [Return a new varset with variable substitution x <- y.]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
Ddi_VarsetSubstVarsAcc (
  Ddi_Varset_t   *vs    /* input varset */,
  Ddi_Vararray_t *x    /* 1-st array of variables */,
  Ddi_Vararray_t *y    /* 2-nd array of variables */
  )
{
  return ((Ddi_Varset_t *)DdiGenericOp3(Ddi_VarsetSubstVars_c,
    Ddi_Accumulate_c,vs,x,y));
}

/*
 * Varset walk operations
 */

/**Function********************************************************************
  Synopsis    [Start Varset walk process]
  Description [Start Varset walk process. Set curr pointer to top BDD node]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
Ddi_VarsetWalkStart (
  Ddi_Varset_t *vs 
)
{
  DdiConsistencyCheck(vs,Ddi_Varset_c);
  vs->data.curr = vs->data.bdd;
}

/**Function********************************************************************
  Synopsis    [Varset walk process step]
  Description [Varset walk process step. Set curr pointer to next BDD node]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
Ddi_VarsetWalkStep (
  Ddi_Varset_t *vs 
)
{
  DdiConsistencyCheck(vs,Ddi_Varset_c);
  Pdtutil_Assert(vs->data.curr!=NULL,
    "NULL varset walk pointer - missing WalkStart or extra WalkStep");

  if (Cudd_IsConstant(vs->data.curr)) {
    vs->data.curr = NULL;
  }
  else {
    vs->data.curr = Cudd_T(vs->data.curr);
  }
}

/**Function********************************************************************
  Synopsis    [test for varset walk end]
  Description [test for varset walk end. Return trur (non 0) if end reached]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
int
Ddi_VarsetWalkEnd (
  Ddi_Varset_t *vs 
)
{
  DdiConsistencyCheck(vs,Ddi_Varset_c);
  Pdtutil_Assert(vs->data.curr!=NULL,
    "NULL varset walk pointer - missing WalkStart or extra WalkStep");

  return(Cudd_IsConstant(vs->data.curr));
}


/**Function********************************************************************
  Synopsis    [Return curr var in varset walk]
  Description [Return curr var in varset walk]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Var_t *
Ddi_VarsetWalkCurr (
  Ddi_Varset_t *vs 
)
{
  Ddi_Mgr_t *ddm;

  DdiConsistencyCheck(vs,Ddi_Varset_c);
  Pdtutil_Assert(vs->data.curr!=NULL,
    "NULL varset walk pointer - missing WalkStart or extra WalkStep");

  ddm = Ddi_ReadMgr(vs);
  if (Cudd_IsConstant(vs->data.curr)) {
    return (NULL);
  }
  else {
    return(Ddi_IthVar(ddm,Cudd_NodeReadIndex(vs->data.curr)));
  }
}

