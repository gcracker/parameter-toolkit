/**CHeaderFile*****************************************************************

  FileName    [cmd.h]

  PackageName [cmd]

  Synopsis    [Command user interface for the pdtrav package.]

  Description [The cmd package constitute the command interface to the PdTrav
    package.
    Each package (fsm, tr, trav, part, etc.) should be used separately and/or
    with a different command interface.
    cmd allows the user to issue commands to read source files, build BDDs,
    doing operation with them, or traverse the network.
    It basically deals with the following objects:
    <ul>
    <li>cmd manager: It keeps track of the different managers (see belove),
    user registers and command interface settings.
    <li>fsm manager: It keeps track of the FSM structure the PdTrav program
    is dealing with. See the FSM packege for further details.
    <li>tr manager: It contains information regardign the Transition Relation
    of the system. See the TR package for further details.
    <li>trav manager: It stores information regarding the reachability analysis
    process. See the TRAV package for further details.
    </ul>
    It finally contains a set of temporary registers. Each temporary register
    can be adressed by a "userd-defined" name, and can contains a BDD  pointer
    or a pointer to an array of BDDs. Operation on these registers are allowed.
    BDD transfer among registers and managers are also permitted.<br>
    Check on consistency and order of issued commands are also performed.<br>
    See file cmdHelp.txt for a guide on command.
    This file constitute the on-line and off-line command documentation of the
    CMD package. To have the documentation on-line a copy of it (or a link
    to it) has to be present in the working directory.
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

#ifndef _CMD
#define _CMD

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>

#ifdef HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif

#ifdef HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif

#include "pdtutil.h"
#include "ddi.h"
#include "fsm.h"
#include "tr.h"
#include "trav.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct CmdOptTable_s CmdOptTable_t;
typedef struct CmdParamTable_s CmdParamTable_t;
typedef struct CmdTable_s CmdTable_t;
typedef struct CmdOpt_s CmdOpt_t;
typedef struct CmdParam_s CmdParam_t; 

typedef struct Cmd_Mgr_s Cmd_Mgr_t;
typedef struct CmdSetting_s CmdSetting_t;
typedef struct CmdRegister_s CmdRegister_t;

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

EXTERN Cmd_Mgr_t * Cmd_MgrInit(char *cmdName);
EXTERN void Cmd_MgrFree(Cmd_Mgr_t *cmdMgr);
EXTERN Cmd_Mgr_t * Cmd_MgrDup(Cmd_Mgr_t *cmdMgr);
EXTERN char * Cmd_MgrReadName(Cmd_Mgr_t *cmdMgr);
EXTERN Pdtutil_VerbLevel_e Cmd_MgrReadVerbosity(Cmd_Mgr_t *cmdMgr);
EXTERN void Cmd_MgrSetName(Cmd_Mgr_t *cmdMgr, char *cmdName);
EXTERN void Cmd_MgrSetVerbosity(Cmd_Mgr_t *cmdMgr, Pdtutil_VerbLevel_e var);

/**AutomaticEnd***************************************************************/

#endif /* _CMD */
