/**CHeaderFile*****************************************************************

  FileName    [cmdInt.h]

  PackageName [cmd]

  Synopsis    [Internal header file for package cmd]

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

#ifndef _CMDINT
#define _CMDINT

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "cmd.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define ROW_LEN_MAX 120
#define STR_LEN 50
#define OPT_NUM 15
#define PARAM_NUM 3
#define OPT_OPT 0
#define OPT_MAN 1

#define OPT_SENTINEL {"", OPT_OPT, "", "", "", "", ""}
#define PARAM_SENTINEL {"", OPT_OPT, "", ""} 

#define CMD_OPT_NULL {"", ""}
#define CMD_PARAM_NULL {"", ""}

/* Number of (BDD or Array of BDD) Registers Available for the User */
#define REGISTER_NUMBER_OF 10

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/

/*
 *  Option for a command as a parameter
 *  tag e value
 */

struct CmdOpt_s {
  char tag[STR_LEN];
  char value[STR_LEN];
  };

/*
 *  Parameter for a command as a parameter
 *  tag e value
 */

struct CmdParam_s {
  char tag[STR_LEN];
  char value[STR_LEN];
  };

/*
 *  Options for a command as a table
 */

struct CmdOptTable_s {
  char tag[STR_LEN];
  int optOptMan;
  char nameShort[STR_LEN];
  char name[STR_LEN];
  char type[STR_LEN];
  char valueDefault[STR_LEN];
  char value[STR_LEN];
  };

/*
 *  Parameters for a command as a table
 */

struct CmdParamTable_s {
  char tag[STR_LEN];
  int paramOptMan;
  char type[STR_LEN];
  char value[STR_LEN];
  };

/*
 *  Global table for a command
 */

struct CmdTable_s {
  char name[STR_LEN];

  struct CmdOptTable_s opt[OPT_NUM];
  struct CmdParamTable_s param[PARAM_NUM];

  void (*command)(
    FILE **fin, 
    Cmd_Mgr_t *cmdMgr,
    CmdOpt_t cmdOpt[],
    CmdParam_t cmdParam[]
    );
  };

/*
 *  Command Registers
 */

struct CmdRegister_s {
  char *tag;                    /* Name of the Register */
  Ddi_Bdd_t *bdd;                /* BDD operand */
  Ddi_Bddarray_t *bddArray;      /* Array of BDD operand */
  };

/*
 *  Command Manager
 */

struct Cmd_Mgr_s {
  char *cmdName;
  Ddi_Mgr_t *dd;                               /* DDI Manager */
  Fsm_Mgr_t *fsmMgr;                           /* FSM Manager */
  Tr_Mgr_t *trMgr;                             /* TR Manager */
  Trav_Mgr_t *travMgr;                         /* TRAV Manager */
  CmdRegister_t reg[REGISTER_NUMBER_OF];       /* BDD Registers */

  struct {
    Pdtutil_VerbLevel_e verbosity;
    int ddNumberOf;
    char *helpName;
  } settings;

  };

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
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int CmdReadCommand(FILE **fin, int *narg, char **line, char **shellLine);
EXTERN int main(int argc, char *argv[]);
EXTERN void CmdCheckCommand(int narg, char *line[], int *cmdNumber, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
EXTERN void CmdCommandExecute(Cmd_Mgr_t *cmdMgr, FILE **fin, int cmdNumber, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);

/**AutomaticEnd***************************************************************/

#endif /* _CMDINT */
