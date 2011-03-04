/**CFile*****************************************************************

  FileName    [cmdMain.c]

  PackageName [cmd]

  Synopsis    [Command management for the pdtrav package.]

  Description [External procedures included in this module are:
    <ul>
    <li> main ()
    </ul>
    Static procedures included in this module are:
    <ul>
    <li>InterpreteCommands ()
    </ul>
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

#include <unistd.h>
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

static void InterpreteCommands(Cmd_Mgr_t *cmdMgr, FILE *fin, int nparams, char **params);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Main program for PdTRAV.]

  Description [Main program for the PdTRAV package.<br>
  It Performs initialization. Reads commands allowing all the operation for
  the PdTRAV package: reading network, creating BDDs, doing operations on BDDs,
  building the transition relation of the network, squaring it, traverse
  the network, etc.<br>
  It allows a strict control on the cudd-2.3.0 parameters (cache size, reordering
  threshold and algorithm, etc.
  ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int
main (
  int argc,
  char *argv[]
  )
{
  /*input stream : stdin or script */
  FILE* fin = stdin;
  Cmd_Mgr_t *cmdMgr;
  int i;
  char *line[OPT_NUM+PARAM_NUM];  /* list of arguments */ 
  char name[ROW_LEN_MAX];

  /*
   *  Print Out Command and Version and etc.
   */

  fprintf (stdout, "# Running Date: ");
  fflush (stdout); 
  system ("date");
  if (gethostname (name, ROW_LEN_MAX) == 0) {
    fprintf (stdout, "# HostName: %s\n", name);
  }
  fflush (stdout);
  fprintf (stdout, "# Command: ");
  for (i=0; i<argc; i++) {
    fprintf (stdout, "%s ", argv[i]);
  }
  fprintf (stdout, "\n");
  fprintf (stdout, "# PdTrav Version: %s\n", PDTRAV_VERSION);
  fprintf (stdout, "# DD Version: ");
  Ddi_PrintCuddVersion (stdout);

  /*
   *  Creates CMD Manager
   */

  fprintf (stdout, "Cmd Manager Init ... ");
  cmdMgr = Cmd_MgrInit ("cmd");
  if(cmdMgr == NULL) {
    fprintf (stderr,"Error : CMD Init failed!\n");
    return (1);
  } else {
    fprintf (stdout, "Done\n");
  }

  /*
   *  Read and execute commands from fin
   */

  line[0] = "@stdin";
  InterpreteCommands (cmdMgr, fin, 1, line);

  return (0);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Read and interpret commands from a file]

  Description [Reads commands from input file fin.
    Recurs through a call command, which allows command line parameters
    ($1, $2,...)
    ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
InterpreteCommands ( 
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  FILE *fin                   /* input stream */,
  int nparams                 /* number of command line parameters */,
  char **params               /* command line parameters */
  )
{
  int cmdNumber;
  CmdOpt_t cmdOpt[OPT_NUM];
  CmdParam_t cmdParam[PARAM_NUM];
  char *shellLine; /*shell command*/
  int i, n, param_ref, flagFile, narg; 
  char *line[OPT_NUM+PARAM_NUM];  /* list of arguments */ 
  FILE *script;

  if (cmdMgr->settings.verbosity >= Pdtutil_VerbLevelUsrMed_c) {
    fprintf (stdout, "Reading Commands File %s\n", &params[0][1]);
  }

  /*
   *  Main Loop
   */

  while  (1) {
    /*
     *  Read the Command; Return a Set of Strings 
     */

    n = CmdReadCommand (&fin, &narg, line, &shellLine);
    if (n==1) {
      /* Script File to Close and Return */
      flagFile = 0;
      Pdtutil_FileClose (fin, &flagFile);     

      if (cmdMgr->settings.verbosity >= Pdtutil_VerbLevelUsrMed_c) {
        fprintf (stdout, "Closing Commands File %s\n", &params[0][1]);
      }

      return;
    }

    /*
     *  Parameter Substitution: Process line looking for $x parameters
     */

    for (i=0; i<narg; i++) {
      if (line[i][0] == '$') {
        param_ref = atoi (&line[i][1]);
        if ( (param_ref<0) || (param_ref>=nparams) ) {
          fprintf (stderr, "Wrong parameter reference: %s\n", line[i]);
          continue;
        }
        Pdtutil_Free (line[i]);
        line[i] = Pdtutil_StrDup (params[param_ref]);
      }
    }    

    /*
     *  Print-out Command
     */

    if (cmdMgr->settings.verbosity >= Pdtutil_VerbLevelUsrMed_c) {
      fprintf (stdout, "%s:", params[0]);
      for (i=0; i<narg; i++) {
        fprintf (stdout, " %s", line[i]);
      }
      fprintf (stdout, "\n");
      fflush (stdout);
    }

    /*
     *  Check for comment: Continue
     */

    if (line[0][0]=='#') {
      continue;
    }

    /*
     *  Check for system command: Calls the shell
     */

    if (line[0][0]=='!') {
      system (shellLine);
      free (shellLine);
      continue;
    }

    /*
     *  If call recur
     */

    if (line[0][0]=='@') {
      char *fileName = &line[0][1];
      script = Pdtutil_FileOpen (NULL, fileName, "r", &flagFile);
      if (script == NULL ) {
        fprintf (stderr, "Opening %s failed!\n", fileName);
        continue;
      }

      InterpreteCommands (cmdMgr, script, narg, line);
      continue;
    }

    for (i=0; i<OPT_NUM; i++) {
      strcpy (cmdOpt[i].tag, "");
      strcpy (cmdOpt[i].value,"");
    }

    for (i=0; i<PARAM_NUM; i++) {
      strcpy (cmdParam[i].tag, "");
      strcpy (cmdParam[i].value, "");
    }

    CmdCheckCommand (narg, line, &cmdNumber, cmdOpt, cmdParam);
    CmdCommandExecute (cmdMgr, &fin, cmdNumber, cmdOpt, cmdParam);

    /* Housekeeping */
    for (i = 0; i < narg; i++) {
      Pdtutil_Free (line[i]);
    }
  }

  return;
}





