/**CFile***********************************************************************

  FileName    [cmdMgr.c]

  PackageName [cmd]

  Synopsis    [Utility functions to create, free, duplicate, and access
    fieds of a CMD Manager.]

  Description [The CMD manager structure contains pointer to the main
    informations PdTRAV is dealing with.<br>
    It contains pointer to the other managers (fsm, tr, trav) and to a set
    of "temporary" user defined registers.<br>
    Managers can be singularly created and deleted.<br>
    Registers can be assign with a BDD or array of BDD values.
    Their name is user defined.
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

#include "cmdInt.h"

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
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Initializes the CMD structure.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

Cmd_Mgr_t *
Cmd_MgrInit (
  char *cmdName   /* Name of the CMD structure */
  )
{
  Cmd_Mgr_t *cmdMgr;
  int i;

  /*------------------------- CMD Structure Creation -----------------------*/

  cmdMgr = Pdtutil_Alloc (Cmd_Mgr_t, 1);
  if (cmdMgr == NULL) {
    fprintf (stderr, "cmdUtil.c (Cmd_MgrInit) Error: Out of memory.\n");
    return (NULL);
  }

  /*------------------- Set the Name of the CMD Structure ------------------*/

  if (cmdName != NULL) {
     Cmd_MgrSetName (cmdMgr, cmdName);
  } else {
     Cmd_MgrSetName (cmdMgr, "cmd");
  }

  /*------------------------------- Set Fields -----------------------------*/


  cmdMgr->dd = NULL;
  cmdMgr->fsmMgr = NULL;
  cmdMgr->trMgr = NULL;
  cmdMgr->travMgr = NULL;

  /*
  *  Register
  */

  for (i=0; i<REGISTER_NUMBER_OF; i++) {
    cmdMgr->reg[i].tag = NULL;
    cmdMgr->reg[i].bdd = NULL;
    cmdMgr->reg[i].bddArray = NULL;
  }
     
  /*
  *  Settings
  */

  Cmd_MgrSetVerbosity (cmdMgr, Pdtutil_VerbLevelUsrMed_c);
  cmdMgr->settings.helpName = Pdtutil_StrDup ("cmdHelp.txt");

  return (cmdMgr);
}

/**Function********************************************************************

  Synopsis    [Frees the CMD structure.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

void 
Cmd_MgrFree (
  Cmd_Mgr_t *cmdMgr   /* CMD Manager */
  )
{
  Pdtutil_Warning (1, "cmdUtil.c: Warning Cmd_MgrFree to be implemented.");

  return;
}

/**Function********************************************************************

  Synopsis    [Duplicates a CMD structure.]

  Description []

  SideEffects [None]

  SeeAlso     [Cmd_MgrInit ()]

******************************************************************************/

Cmd_Mgr_t *
Cmd_MgrDup (
  Cmd_Mgr_t *cmdMgr   /* CMD Manager */
)
{
  Cmd_Mgr_t *cmdMgrNew;

  Pdtutil_Warning (1, "Mgr_FsmDup till to test.");

  /*------------------------- CMD structure creation -----------------------*/

  cmdMgrNew = Cmd_MgrInit (NULL);

  /*------------------------------- Copy Fields ----------------------------*/

  return (cmdMgrNew);
}

/**Function********************************************************************

  Synopsis    [Reads the name of a CMD manager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

char *
Cmd_MgrReadName (
  Cmd_Mgr_t *cmdMgr
  )
{
  return (cmdMgr->cmdName);
}

/**Function********************************************************************

  Synopsis    [Read the "default" verbosity level of the CMD
    manager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

Pdtutil_VerbLevel_e
Cmd_MgrReadVerbosity (
  Cmd_Mgr_t *cmdMgr
  )          
{
  return (cmdMgr->settings.verbosity);
}

/**Function********************************************************************

  Synopsis    [Set the name of the CMD manager.]

  Description [It duplicates the string passed as a parameter
    and copy it in the CMD manager.]

  SideEffects []

  SeeAlso     []

******************************************************************************/

void
Cmd_MgrSetName (
  Cmd_Mgr_t *cmdMgr,
  char *cmdName
  )
{
  cmdMgr->cmdName = Pdtutil_StrDup (cmdName);

  return;
}

/**Function********************************************************************

  Synopsis    [Set the "default" verbosity value of the CMD manager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/

void 
Cmd_MgrSetVerbosity (
  Cmd_Mgr_t *cmdMgr, 
  Pdtutil_VerbLevel_e var
  )          
{
  cmdMgr->settings.verbosity = var;

  return;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
