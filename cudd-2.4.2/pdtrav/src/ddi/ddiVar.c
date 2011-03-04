/**CFile***********************************************************************

  FileName    [ddiVar.c]

  PackageName [ddi]

  Synopsis    [Functions to manipulate BDD variables]

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


/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [Create a variable]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
DdiVarNewFromCU(
  Ddi_BddMgr   *ddm,
  DdNode      *varCU
)
{
  Ddi_Var_t *v;
  int id;

#if 0
  if (varCU->ref == 0)
#endif
    Cudd_Ref(varCU);

  id = Cudd_NodeReadIndex(varCU);
  v = (Ddi_Var_t *)DdiGenericAlloc(Ddi_Var_c,ddm);
  v->data.bdd = varCU;
  v->data.index = id;

  Ddi_VararrayWrite(ddm->variables,id,v);
  /* no variable group initially present */     
  Ddi_VarsetarrayWrite(ddm->varGroups,id,NULL);

  DdiMgrCheckVararraySize(ddm);

}

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************
  Synopsis    [Return the variable index (CUDD variable index)]
  SideEffects [none]
  SeeAlso     [Ddi_IthVar]
******************************************************************************/
int
Ddi_VarIndex (
  Ddi_Var_t *var 
)
{
  return (var->data.index);
}

/**Function********************************************************************
  Synopsis    [Return the variable of a given index]
  SideEffects [none]
  SeeAlso     [Ddi_VarIndex Ddi_VarAtLevel]
******************************************************************************/
Ddi_Var_t *
Ddi_IthVar (
  Ddi_BddMgr   *ddm,
  int index 
)
{
  DdNode *vCU;

  Pdtutil_Assert(index>=0,"negative variable index");
  if (index >= Ddi_ReadSize(ddm)) {
    vCU = Cudd_bddIthVar(ddm->mgrCU,index);
    DdiVarNewFromCU(ddm,vCU);
  }

  return (Ddi_VararrayRead(ddm->variables,index));
}

/**Function********************************************************************
  Synopsis    [Return variable at a given level in the order]
  SideEffects [none]
  SeeAlso     [Ddi_IthVar]
******************************************************************************/
Ddi_Var_t *
Ddi_VarAtLevel(
  Ddi_BddMgr   *ddm,
  int          lev
)
{
  return(Ddi_IthVar(ddm,Ddi_ReadInvPerm(ddm,lev)));
}

/**Function********************************************************************
  Synopsis    [Create a new variable before (in the variable order)
    the given variable.]
  SideEffects [none]
  SeeAlso     [Ddi_NewVarAtLevel]
******************************************************************************/
Ddi_Var_t *
Ddi_VarNewBeforeVar (
  Ddi_Var_t *var 
)
{
  Ddi_BddMgr *ddMgr;
  int level;

  ddMgr = var->common.mgr;
  level = Ddi_VarCurrPos (var);

  return (Ddi_VarNewAtLevel (ddMgr, level));
}

/**Function********************************************************************
  Synopsis    [Create a new variable after (in the variable order)
    the given variable.]
  SideEffects [none]
  SeeAlso     [Ddi_NewVarAtLevel]
******************************************************************************/
Ddi_Var_t *
Ddi_VarNewAfterVar (
  Ddi_Var_t *var 
)
{
  Ddi_BddMgr *ddMgr;
  int level;

  ddMgr = var->common.mgr;
  level = Ddi_VarCurrPos (var) + 1;

  return (Ddi_VarNewAtLevel (ddMgr, level));
}

/**Function********************************************************************
  Synopsis    [Create a new variable (generated within a CUDD manager)]
  SideEffects [none]
  SeeAlso     [Ddi_VarNewAtLevel Ddi_VarFromCU]
******************************************************************************/
Ddi_Var_t *
Ddi_VarNew(
  Ddi_BddMgr   *ddm
)
{
  DdNode *varCU;

  varCU = Cudd_bddNewVar(ddm->mgrCU);
  DdiVarNewFromCU(ddm,varCU);

  return(Ddi_IthVar(ddm,Cudd_NodeReadIndex(varCU)));
}


/**Function********************************************************************
  Synopsis    [Returns a new variable at a given level in the order]
  SideEffects [none]
  SeeAlso     [Ddi_VarNew]
******************************************************************************/
Ddi_Var_t *
Ddi_VarNewAtLevel(
  Ddi_BddMgr   *ddm,
  int          lev
)
{
  DdNode *varCU;

  varCU = Cudd_bddNewVarAtLevel(ddm->mgrCU,lev);
  DdiVarNewFromCU(ddm,varCU);

  return(Ddi_IthVar(ddm,Cudd_NodeReadIndex(varCU)));
}


/**Function********************************************************************
  Synopsis    [Return current position of var in variable order]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
int
Ddi_VarCurrPos (
  Ddi_Var_t *var
)
{
  return Ddi_ReadPerm(var->common.mgr,Ddi_VarIndex(var));
}

/**Function********************************************************************
  Synopsis    [Return the name of a variable]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
char *
Ddi_VarName (
  Ddi_Var_t *var 
)
{
  return (var->common.name);
}

/**Function********************************************************************
  Synopsis    [Attach a given name to the variable]
  SideEffects [none]
  SeeAlso     [Ddi_VarDetachName ]
******************************************************************************/
void
Ddi_VarAttachName (
  Ddi_Var_t *var,
  char *name
)
{
  int index;
  char **varnames;
  Ddi_Mgr_t *ddm;

  ddm = var->common.mgr;
  var->common.name = Pdtutil_StrDup(name);

  DdiMgrCheckVararraySize(ddm);

  varnames=Ddi_MgrReadVarnames(ddm);
  Pdtutil_Assert(varnames!=NULL, "null varnames array in dd manager");

  index = Ddi_VarIndex(var);
  Pdtutil_Assert(varnames[index]==NULL,"setting name to a named variable");
  varnames[index] = Pdtutil_StrDup(name);
}

/**Function********************************************************************
  Synopsis    [Clear the name of a variable]
  SideEffects [none]
  SeeAlso     [Ddi_VarAttachName]
******************************************************************************/
void
Ddi_VarDetachName (
  Ddi_Var_t *var
)
{
  int index;
  char **varnames;
  Ddi_Mgr_t *ddm;

  ddm = var->common.mgr;
  Pdtutil_Free (var->common.name);
  var->common.name = NULL;

  varnames=Ddi_MgrReadVarnames(ddm);
  Pdtutil_Assert(varnames!=NULL, "null varnames array in dd manager");
  index = Ddi_VarIndex(var);
  Pdtutil_Free (varnames[index]);

  varnames[index] = NULL;

  return;
}

/**Function********************************************************************
  Synopsis    [Return the variable auxid (-1 if auxids not defined)]
  SideEffects [none]
  SeeAlso     [Ddi_VarName]
******************************************************************************/
int
Ddi_VarAuxid (
  Ddi_Var_t *var
)
{
  Ddi_BddMgr   *ddm;
  int index, *varauxids;

  ddm = var->common.mgr;
  index = Ddi_VarIndex(var);

  varauxids=Ddi_MgrReadVarauxids(ddm);
  Pdtutil_Assert(varauxids!=NULL, "null varauxids array in dd manager");

  return (varauxids[index]);
}

/**Function********************************************************************
  Synopsis    [Set the variable auxid of a variable]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
void
Ddi_VarAttachAuxid (
  Ddi_Var_t *var,
  int auxid
)
{
  Ddi_BddMgr   *ddm;
  int index, *varauxids;

  ddm = var->common.mgr;
  index = Ddi_VarIndex(var);

  varauxids=Ddi_MgrReadVarauxids(ddm);
  Pdtutil_Assert(varauxids!=NULL, "null varauxids array in dd manager");

  Pdtutil_Assert(varauxids[index]==-1,"setting auxid to a variable twice");
  varauxids[index] = auxid;
}

/**Function********************************************************************
  Synopsis    [Search a variable given the name]
  Description [Still a linear search !] 
  SideEffects [none]
  SeeAlso     [Ddi_VarFromAuxid]
******************************************************************************/
Ddi_Var_t *
Ddi_VarFromName (
  Ddi_BddMgr *ddm,
  char *name
)
{
  int i;
  char **varnames=Ddi_MgrReadVarnames(ddm);
  Pdtutil_Assert(varnames!=NULL, "null varnames array in dd manager");

  for (i=0;i<Ddi_ReadSize(ddm);i++) {
    if ((varnames[i]!=NULL) && (strcmp(varnames[i],name)==0)) {
      return (Ddi_IthVar(ddm,i));
    }
  }

  return (NULL);
}

/**Function********************************************************************
  Synopsis    [Search a variable given the auxid]
  Description [Still a linear search !] 
  SideEffects [none]
  SeeAlso     [Ddi_VArFromName]
******************************************************************************/
Ddi_Var_t *
Ddi_VarFromAuxid (
  Ddi_BddMgr   *ddm,
  int auxid
)
{
  int i;
  int *varauxids=Ddi_MgrReadVarauxids(ddm);
  Pdtutil_Assert(varauxids!=NULL, "null varauxids array in dd manager");

  for (i=0;i<Ddi_ReadSize(ddm);i++) {
    if (varauxids[i]==auxid)
      return (Ddi_IthVar(ddm,i));
  }
  return (NULL);
}

/**Function********************************************************************
  Synopsis    [Return the CUDD bdd node of a variable]
  Description [Return the CUDD bdd node of a variable]
  SideEffects [none]
******************************************************************************/
DdNode *
Ddi_VarToCU(
  Ddi_Var_t * v
)
{
  return (v->data.bdd);
}

/**Function********************************************************************
  Synopsis    [Convert a CUDD variable to a DDI variable]
  Description [Convert a CUDD variable to a DDI variable]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Var_t *
Ddi_VarFromCU (
  Ddi_BddMgr   *ddm,
  DdNode *  v
)
{
  return(Ddi_IthVar(ddm,Cudd_NodeReadIndex(v)));
}

/**Function********************************************************************
  Synopsis     [Copy a variable to a destination dd manager]
  Description  [Find the variable corresponding to v in the destination
               manager. Variable correspondence is for now limited to
               index matching.] 
  SideEffects  [none]
  SeeAlso      []
******************************************************************************/
Ddi_Var_t *
Ddi_VarCopy(
  Ddi_BddMgr *dd2 /* destination manager */,
  Ddi_Var_t *v    /* variable to be copied */ 
)
{
  Ddi_Var_t *v2;
  int id;

  id = Ddi_VarIndex(v);
  v2 = Ddi_IthVar(dd2,id);

  return (v2);
}

/**Function********************************************************************
  Synopsis    [Create a variable group]
  Description [A group of variables is created for group sifting. 
               The group starts at v and contains grpSize variables
               (following v in the ordering. Sifting is allowed within the
               group]
  SideEffects [none]
  SeeAlso     [Ddi_VarMakeGroupFixed]
******************************************************************************/
void
Ddi_VarMakeGroup(
  Ddi_BddMgr   *dd,
  Ddi_Var_t   *v,
  int grpSize
)
{
  DdiMgrMakeVarGroup(dd,v,grpSize,MTR_DEFAULT);
}

/**Function********************************************************************
  Synopsis    [Create a variable group with fixed inner order]
  Description [Same as Ddi_VarMakeGroup but no dynamic reordering allowed
               within group]
  SideEffects [none]
  SeeAlso     [Ddi_VarMakeGroupFixed]
******************************************************************************/
void
Ddi_VarMakeGroupFixed(
  Ddi_BddMgr   *dd,
  Ddi_Var_t   *v,
  int grpSize
)
{
  DdiMgrMakeVarGroup(dd,v,grpSize,MTR_FIXED);
}

/**Function********************************************************************
  Synopsis    [Return variable group including v. NULL if v is not in a group]
  Description [Return variable group including v. NULL if v is not in a group]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
Ddi_VarReadGroup(
  Ddi_Var_t   *v
)
{
  int i;
  Ddi_Mgr_t *ddm;

  ddm = Ddi_ReadMgr(v);
  i = Ddi_VarIndex(v);

  return(Ddi_VarsetarrayRead(ddm->varGroups,i));
}

/**Function********************************************************************
  Synopsis    [Return true (non 0) if variable is in variable group]
  Description [Return true (non 0) if variable is in variable group]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
int
Ddi_VarIsGrouped(
  Ddi_Var_t   *v
)
{
  return(Ddi_VarReadGroup(v)!=NULL);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/





