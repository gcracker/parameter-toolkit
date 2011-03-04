/**CFile***********************************************************************

  FileName    [cmdTable.c]

  PackageName [cmd]

  Synopsis    [Command management for the pdtrav package.]

  Description [In the PdTRAV-cmd package commands are defined in a
    command table Cmd_Table_s. This table is initialized in this
    files allowing compiler checks. For each command it contains:
    <ul>
    <li> the name of the command
    <li> the set of options available for that command
    <li> the set of parameters necessary to issue that command
    <li> the function called to execute the command.
    <\ul>
    For each command a search in the table is performed, and the command
    is issued calling the related function. This function check for
    optional and mandatory options and parameters and execute the
    command.<br>
    Check on consistency and order of issued commands are also performed.<br>
    See file cmdHelp.txt for a guide on command.
    This file constitute the on-line and off-line command documentation of the
    CMD package. To have the documentation on-line a copy of it (or a link
    to it) has to be present in the working directory.
    ]

  SeeAlso     []

  Author      [Gianpiero Cabodi and Stefano Quer]

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

/* Number of Command per Row in the help command print out */
#define COMMAND_PER_ROW 3

/* Option per Row */
#define OPTION_PER_ROW 3

/* Number of Line per Screen (before asking for return) */
#define LINE_PER_SCREEN 10

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*
 *  Notice:
 *  The following Automatic Prototypes Section usually appears after the
 *  'Variables Declarations' Section. This because we need to have all
 *  the prototypes defined before the definition of the structure (that
 *  contains pointers to the local functions.
 *  If problems are encountered during the prototypes extraction, local
 *  functions can be transformed into internal ones, putting the prototypes
 *  into the cmdInt.h file instead.
 */

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void CmdBlifWrite(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdQuit(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmInit(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmShare(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmDup(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmAlign(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmSet(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmDynOrd(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmGroup(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDdmDelete(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdDevel(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddRead(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddWrite(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddWriteCubes(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddGenLiteral(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddNot(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddAnd(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddOr(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddCof(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddRes(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddExist(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddForall(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddDelete(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddDenseSet(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdSetOption(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdShowOption(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdHelp(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdFsmRetime(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdFsmAuxVarRemove(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdFsmRead(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdFsmPmBuild(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdFsmWrite(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdFsmDelete(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTrInit(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTrDelete(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTravInit(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTravDelete(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTravProfile(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTravTraverse(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTravSimulate(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTravMismatch(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdOrdRead(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdOrdWrite(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTrClusterExtract(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdLambdaLatchRemove(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTrCluster(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTrPartition(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdTrClosure(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdStatsPrint(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddPartMake(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddPartAdd(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddPartRead(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddPartDelete(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static void CmdBddCopy(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);
static int LocateOption(CmdOpt_t *cmdOpt, char *tag);
static int LocateParam(CmdParam_t *cmdParam, char *tag);
static int CmdMgrOperation(Cmd_Mgr_t *cmdMgr, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlagP);
static int CmdRegOperation(Cmd_Mgr_t *cmdMgr, char *string, Pdtutil_MgrOp_t operationFlag, void **voidPointer, Pdtutil_MgrRet_t *returnFlagP);
static int RegFullFind(Cmd_Mgr_t *cmdMgr, char *string);
static int RegEmptyFind(Cmd_Mgr_t *cmdMgr);
static int CommandCompare(const void *s1, const void *s2);
static void Pdtutil_MgrOpenScript(FILE **fin, Cmd_Mgr_t *cmdMgr, CmdOpt_t cmdOpt[], CmdParam_t cmdParam[]);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************
  Synopsis    [Generic Table for Command]

  Description [This table contains all the command available with the PdTRAV
    package. For each command options and parameters are reported.
    Each options and each parameters can be mandatory or optional.
    They specifies tag (name) of the option/parameter, how to specify it
    (using -<one letter> or --<one string>), initial and default values.
    Finally it specifies the function called by the command.<br>
    OPT_SENTINEL and PARAM_SENTINEL are sentinel flags to delimit the
    size of the structure (whose size is defned during initialization). 
    ]
******************************************************************************/

struct CmdTable_s cmdTable[] =
  {
    /*--------------- Command ---------------*/
    {
      /* Dummy Shell Command: inserted only to provide help */	
      "!",
      /* Option */
      {    
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"shell_command", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      NULL
    },


  /*-------------------------- DD Manager Commands --------------------------*/

  /*----------------------------- BDDs Commands -----------------------------*/
    /*--------------- Command ---------------*/
    {
      "ddm_init",
      /* Option */
      {    
        {"varNum", OPT_OPT, "-v", "--varNum", "int", "0", "0"},
        {"slotNum", OPT_OPT, "-s", "--slotNum", "int", "-1", "-1"},
        {"cacheSize", OPT_OPT, "-c", "--cacheSize", "int", "-1", "-1"},
        {"memorySizeMax", OPT_OPT, "-m", "--memorySizeMax", "int", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmInit
    },

    /*--------------- Command ---------------*/
    /* Copy (the pointer) of a DD Manager from one field to another */	
    {
      "ddm_share",
      /* Option */
      {    
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmShare
    },

    /*--------------- Command ---------------*/
    /* Duplicate a DD Manager from one field to another */	
    {
      "ddm_dup",
      /* Option */
      {    
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmDup
    },

    /*--------------- Command ---------------*/
    {
      /* Align a DD Manager to another one */	
      "ddm_align",
      /* Option */
      {    
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmAlign
    },

    /*--------------- Command ---------------*/
    {
      /* Control Dynamic Reordering on a DD Manager */	
      "ddm_set",
      /* Option */
      {    
        {"cacheSize", OPT_OPT, "-c", "--cacheSize", "int", "-1", "-1"},
        {"memoryMax", OPT_OPT, "-x", "--memoryMax", "int", "-1", "-1"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmSet
    },

 {
      /* Control Dynamic Reordering on a DD Manager */	
      "ddm_dynord",
      /* Option */
      {    
        {"method", OPT_OPT, "-m", "--method", "string", "same", "same"},
        /* enable and disable may be incongruent */
        {"enable", OPT_OPT, "-e", "--enable", "bool", "0", "0"},
        {"disable", OPT_OPT, "-d", "--disable", "bool", "0", "0"},
        {"force", OPT_OPT, "-f", "--force", "bool", "0", "0"},
        {"threshold", OPT_OPT, "-t", "--threshold", "int", "10000", "10000"},
        {"nextThreshold", OPT_OPT, "-n", "--nextThreshold", "int", "-1", "-1"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmDynOrd
    },

    /*--------------- Command ---------------*/
    {
      /* Fix Group Information on a DD Manager */	
      "ddm_group",
      /* Option */
      {    
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmGroup
    },

    /*--------------- Command ---------------*/
    {
      /* Deletes and frees a DD Manager structure*/	
      "ddm_delete",
      /* Option */
      {    
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"manager", OPT_MAN, "string", "ddm"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDdmDelete
    },


    /*--------------- Command ---------------*/
    {
      /* Read BDD from a file */	
      "bdd_read",
      /* Option */
      {
        {"matchmode", OPT_OPT, "-mm", "--matchmode", "string", "names",
          "names"},
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        {"loadmode", OPT_OPT, "-lm", "--loadmode", "string", "auto", "auto"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddRead,
    },

    /*--------------- Command ---------------*/
    {
      /* Write BDD to a file */	
      "bdd_write",
      /* Option */
      {
        /* -f or --format = default (0), text (1) or binary (2) use that
           format for BDDs on Files */
        {"format", OPT_OPT, "-f", "--format", "string", "default", "default"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddWrite
    },

    /*--------------- Command ---------------*/
    {
      /* Write cubes of a BDD to a file */	
      "bdd_write_cubes",
      /* Option */
      {
        /* max number of cubes generated */
        {"maxcubes", OPT_OPT, "-m", "--maxcubes", "int", "100", "100"},
        {"reverse", OPT_OPT, "-r", "--reverse", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddWriteCubes
    },

    /*--------------- Command ---------------*/
    {
      "bdd_not",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddNot
    },

    /*--------------- Command ---------------*/
    {
      "bdd_gen_literal",
      /* Option */
      {
        {"var", OPT_OPT, "-v", "--var", "string", "NULL", "NULL"},
        {"not", OPT_OPT, "-n", "--not", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddGenLiteral
    },

    /*--------------- Command ---------------*/
    {
      "bdd_and",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddAnd
    },

    /*--------------- Command ---------------*/
    {
      "bdd_or",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },    
      /* Routine */
      CmdBddOr
    },

    /*--------------- Command ---------------*/
    {
      "bdd_cof",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },    
      /* Routine */
      CmdBddCof
    },

    /*--------------- Command ---------------*/
    {
      "bdd_res",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },    
      /* Routine */
      CmdBddRes
    },

    /*--------------- Command ---------------*/
    {
      "bdd_exist",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"varset", OPT_MAN, "string", "NULL"},
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddExist
    },

    /*--------------- Command ---------------*/
    {
      "bdd_forall",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"varset", OPT_MAN, "string", "NULL"},
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddForall
    },

    /*--------------- Command ---------------*/
    {
      /* Delete a BDD from the main memory */
      "bdd_delete",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddDelete
    },

    /*--------------- Command ---------------*/
    {
      /* Move a BDD from memory to memory */
      "bdd_copy",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", /* bdd */ OPT_MAN, "string", "NULL"},
        {"source", /* bdd */ OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddCopy
    },

    /*--------------- Command ---------------*/
    {
      /* Take the dense Sub or Super Settings of a BDD */
      "bdd_denseset",
      /* Option */
      {
        {"array", OPT_OPT, "-a", "--array", "bool", "0", "0"},
        {"method", OPT_OPT, "-m", "--method", "string", "none", "none"},
        {"threshold", OPT_OPT, "-t", "--threshold", "int", "0", "0"},
        {"safe", OPT_OPT, "-s", "--safe", "int", "1", "1"},
        {"quality", OPT_OPT, "-q", "--quality", "double", "1.0", "1.0"},
        {"hardlimit", OPT_OPT, "-h", "--hardlimit", "int", "1", "1"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddDenseSet
    },

    /*--------------- Command ---------------*/
    {
      "blif_write",
      /* Option */
      {
        {"reduceDelta", OPT_OPT, "-r", "--reduceDelta", "int", "1", "1"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"filename", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBlifWrite
    },

    /*--------------- Command ---------------*/
    {
      /* Partition a BDD in a monolithic or disjunctive form */
      "part_make",
      /* Option */
      {
        {"verbosity", OPT_OPT, "-v", "--verbosity", "string", "none", "none"},
        {"method", OPT_OPT, "-m", "--method", "string", "none", "none"},
        {"threshold", OPT_OPT, "-t", "--threshold", "int", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", /* bdd */ OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddPartMake
    },

    /*--------------- Command ---------------*/
    {
      /* Add a Partition to a BDD */
      "part_add",
      /* Option */
      {
        {"index", OPT_OPT, "-i", "--index", "int", "0", "0"},
        {"conjunction", OPT_OPT, "-c", "--conjunction", "bool", "1", "1"},
        {"disjunction", OPT_OPT, "-d", "--disjunction", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", /* bdd */ OPT_MAN, "string", "NULL"},
        {"source", /* bdd */ OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddPartAdd
    },

    /*--------------- Command ---------------*/
    {
      "part_read",
      /* Option */
       {
        {"index", OPT_OPT, "-i", "--index", "int", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", /* bdd */ OPT_MAN, "string", "NULL"},
        {"source", /* bdd */ OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddPartRead
    },

    /*--------------- Command ---------------*/
    {
      "part_delete",
      /* Option */
      {
        {"index", OPT_OPT, "-i", "--index", "int", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"bdd", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdBddPartDelete
    },

  /*---------------------------- FSMs Commands ------------------------------*/
  /*
  *  Here it is probably necessary to have a greater degree of freedom
  *  (e.g, save only FSM with a new BDD ordering but no BDD vs if I do not
  * save BDDs why do I have to put the names?)
  */

    /*--------------- Command ---------------*/
    {
      "fsm_auxvarrm",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdFsmAuxVarRemove
    },

    /*--------------- Command ---------------*/
    {
      "fsm_delete",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdFsmDelete
    },

    /*--------------- Command ---------------*/
    {
      "fsm_read",
      /* Option */
      {
        {"blif", OPT_OPT, "-l", "--blif", "bool", "0", "0"},
        {"bdd", OPT_OPT, "-b", "--bdd", "bool", "1", "1"},
        {"nogroup", OPT_OPT, "-n", "--nogroup", "bool", "0", "0"},
        {"ord", OPT_OPT, "-o", "--ord", "string", "", ""},
        {"ordFormat", OPT_OPT, "-f", "--ordFormat", "string", "default",
         "default"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"filename", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdFsmRead
    },

    /*--------------- Command ---------------*/
    {
      "fsm_retime",
      /* Option */
      {
        {"type", OPT_OPT, "-t", "--type", "string", "pdtrav", "pdtrav"},
        {"retimeEqual", OPT_OPT, "-e", "--retimeEqual", "int", "1", "1"},
        {"removeLatches", OPT_OPT, "-e", "--removeLatches", "int", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdFsmRetime
    },

    /*--------------- Command ---------------*/
    {
      "fsm_pmbuild",
      /* Option */
      {
        {"blif", OPT_OPT, "-f", "--blif", "bool", "0", "0"},
        {"bdd", OPT_OPT, "-b", "--bdd", "bool", "0", "0"},
        {"ord", OPT_OPT, "-o", "--ord", "string", "", ""},
        {"ordFormat", OPT_OPT, "-f", "--ordFormat", "string", "default",
         "default"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"filename", OPT_OPT, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdFsmPmBuild
    },

    /*--------------- Command ---------------*/
    {
      "fsm_write",
      /* Option */
      {
        /* -b or --bdd Stores BDDs on Files */
        {"bdd", OPT_OPT, "-b", "--bdd", "bool", "0", "0"},
        /* -f or --format = default (0), text (1) or binary (2) use that format
           for BDDs on Files */
        {"bddFormat", OPT_OPT, "-d", "--bddFormat", "string",
         "default", "default"},
        {"ordFormat", OPT_OPT, "-f", "--ordFormat", "string", "default",
         "default"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"filename", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdFsmWrite
    },

  /*---------------------------- ORDs Commands ------------------------------*/
  /*
   *  To extend to tr_ ... fsm_... trav_... read and write ord
   */

    /*--------------- Command ---------------*/
    {
      "ord_read",
      /* Option */
      {  
        {"ordFormat", OPT_OPT, "-f", "--ordFormat", "string", "default",
         "default"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", /* bdd */ OPT_MAN, "string", "NULL"},
        {"source", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdOrdRead
    },

    /*--------------- Command ---------------*/
    {
      "ord_write",
      /* Option */
      {     
        {"ordFormat", OPT_OPT, "-f", "--ordFormat", "string", "default",
         "default"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        {"source", /* bdd */ OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdOrdWrite
    },

  /*--------------------- Transition Relation Commands ----------------------*/

    /*--------------- Command ---------------*/
    {
      /* Mandatory command: it copies set of variables */
      "tr_init",
      /* Option */
      {
        /* build = start from delta and re-build otherwise
           copy from FSM structure */	
        {"build", OPT_OPT, "-b", "--build", "bool", "0", "0"},
        /* file = read a BDD file and copy variable from FSM structure */
        {"file", OPT_OPT, "-f", "--file", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routie */
      CmdTrInit 
   },

    /*--------------- Command ---------------*/
    {
      "tr_cluster_extract",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"index", OPT_MAN, "int", "0"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTrClusterExtract
    },

    /*--------------- Command ---------------*/
    {
      "remove_lambda_latches",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdLambdaLatchRemove
    },


    /*--------------- Command ---------------*/
    {
      "tr_cluster",
      /* Option */
      {
        {"keepPi", OPT_OPT, "-k", "--keepPi", "bool", "0", "0"},
        {"mono", OPT_OPT, "-m", "--mono", "bool", "0", "0"},
        /* 0, 1, 2, 3 */
        {"sort", OPT_OPT, "-s", "--sort", "string", "default", "default"},
        {"threshold", OPT_OPT, "-t", "--threshold", "int", "-1", "-1"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTrCluster
    },

    /*--------------- Command ---------------*/
    {
      /* Call partitioning (part) on TR */
      "tr_partition",
      /* Option */
      {
        {"var", OPT_OPT, "-v", "--var", "string", "NULL", "NULL"},
        {"threshold", OPT_OPT, "-t", "--threshold", "int", "-1", "-1"},
        {"forcetoggle", OPT_OPT, "-f", "--forcetoggle", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTrPartition
    },

    /*--------------- Command ---------------*/
    {
      "tr_closure",
      /* Option */
      {
        {"linear", OPT_OPT, "-l", "--linear", "bool", "0", "0"},
        /* Number of Iterations of Closure */
        {"iter", OPT_OPT, "-i", "--iter", "bool", "-1", "-1"},
        {"squaring", OPT_OPT, "-s", "--squaring", "bool", "1", "1"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTrClosure
    },

    /*--------------- Command ---------------*/
    {
      "tr_delete",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTrDelete
    },


  /*-------------------------- Traversal Commands ---------------------------*/

    /*--------------- Command ---------------*/
    {
      /* Mandatory command: it copies set of variables */
      "trav_init",
      /* Option */
      {
        /* Read BDD from files otherwise from FSM structure */
        {"tr", OPT_OPT, "-t", "--tr", "string", "NULL", "NULL"},
        {"reached", OPT_OPT, "-r", "--reached", "string", "NULL", "NULL"},
        {"from", OPT_OPT, "-f", "--from", "string", "NULL", "NULL"},
        {"backward", OPT_OPT, "-b", "--backward", "bool", "0", "0"},
        {"keepNew", OPT_OPT, "-b", "--keepNew", "bool", "0", "0"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTravInit
    },

    /*--------------- Command ---------------*/
    {
      /* Mandatory command: it copies set of variables */
      "trav_delete",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTravDelete
    },

    /*--------------- Command ---------------*/
    {
      "traverse",
      /* Option */
      {
        {"auxMgr", OPT_OPT, "-a", "--auxMgr", "bool", "0", "0"},
        {"depth", OPT_OPT, "-d", "--depth", "int", "-1", "-1"},
        {"fromSelect", OPT_OPT, "-f", "--fromSelect", "string", "same",
         "same"},
        {"logPeriod", OPT_OPT, "-l", "--logPeriod", "int", "-1", "-1"},
        {"partMethod", OPT_OPT, "-m", "--partMethod", "string", "none",
         "none"},
        {"partThreshold", OPT_OPT, "-t", "--partThreshold", "int", "-1", "-1"},
        {"verbosity", OPT_OPT, "-v", "--verbosity", "string", "appMax",
         "appMax"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTravTraverse
    },

    /*--------------- Command ---------------*/
    {
      "simulate",
      /* Option */
      {
        {"depth", OPT_OPT, "-d", "--depth", "int", "-1", "-1"},
        {"logPeriod", OPT_OPT, "-l", "--logPeriod", "int", "-1", "-1"},
        {"deadEnd", OPT_OPT, "-e", "--deadEnd", "int", "-1", "-1"},
        {"verbosity", OPT_OPT, "-v", "--verbosity", "string", "appMax",
         "appMax"},
        {"simFlag", OPT_OPT, "-s", "--simFlag", "int", "0", "0"},
        {"depthBreadth", OPT_OPT, "-b", "--depthBreadth", "int", "0", "0"},
        {"random", OPT_OPT, "-r", "--random", "int", "1", "1"},
        {"init", OPT_OPT, "-i", "--init", "string", "NULL", "NULL"},
        {"pattern", OPT_OPT, "-p", "--pattern", "string", "random", "random"},
        {"result", OPT_OPT, "-o", "--result", "string", "stdout", "stdout"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTravSimulate
    },

    /*--------------- Command ---------------*/
    {
      /* generate a counterexample sequence */
      "trav_mismatch_pat",
      /* Option */
      {
        {"maxDepth", OPT_OPT, "-d", "--maxDepth", "int", "-1", "-1"},
        {"method", OPT_OPT, "-m", "--method", "string", "none",
         "none"},
        {"initialConstr", OPT_OPT, "-i", "--initialConstr", "string", "none",
         "none"},
        {"finalConstr", OPT_OPT, "-f", "--finalConstr", "string", "none",
         "none"},
        {"startPoint", OPT_OPT, "-s", "--startPoint", "string", "none",
         "none"},
        {"endPoint", OPT_OPT, "-e", "--endPoint", "string", "none",
         "none"},
        {"verbosity", OPT_OPT, "-v", "--verbosity", "string", "appMax",
         "appMax"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"destination", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTravMismatch
    },

    /*--------------- Command ---------------*/
    {
      /* Manage TR profiles */
      "profile",
      /* Option */
      {
        {"dac99", OPT_OPT, "-d", "--dac99", "bool", "0", "0"},
        {"heuristic", OPT_OPT, "-h", "--heuristic", "int", "1", "1"},
        {"threshold", OPT_OPT, "-t", "--threshold", "int", "100", "100"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"action", OPT_MAN, "string", "create"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdTravProfile
    },

  /*----------------------------- Miscellaneous -----------------------------*/

   /*--------------- Command ---------------*/
    {
      "help",
      /* Option */
      {
        {"verbosity", OPT_OPT, "-v", "--verbosity", "string", "usrMax",
         "usrMax"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"command", OPT_OPT, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdHelp
    },

    /*--------------- Command ---------------*/
    {
      "source",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"scriptfile", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      Pdtutil_MgrOpenScript
      },

    /*--------------- Command ---------------*/
    {
      "set",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"cmdParam", OPT_MAN, "string", "NULL"},
        {"newValue", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdSetOption
    },

    /*--------------- Command ---------------*/
    {
      "show",
      /* Option */
      {
      /*
        {"column", OPT_OPT, "-c", "--column", "int", "1", "1"},
        {"separator", OPT_OPT, "-s", "--separator", "int", "3", "3"},
      */
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"cmdParam", OPT_OPT, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdShowOption
    },

    /*--------------- Command ---------------*/
    {
      /* statistics of fsm, trav, ..., fsm.tr, ... */
      "stats",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"topic", OPT_MAN, "string", "NULL"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdStatsPrint
    },

    /*--------------- Command ---------------*/
    {
      "devel",
      /* Option */
      {
        {"method", OPT_OPT, "-m", "--method", "string", "size", "size"},
        OPT_SENTINEL
      },
      /* Parameter */
      {
        {"size", OPT_MAN, "int", "0"},
        PARAM_SENTINEL
      },
      /* Routine */
      CmdDevel
    },

    /*--------------- Command ---------------*/
    {
      "quit",
      /* Option */
      {
        OPT_SENTINEL
      },
      /* Parameter */
      {
        PARAM_SENTINEL
      },
      /* Routine */
      CmdQuit
    }

  /*-------------------------- End Command Table ----------------------------*/
  };

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* shell to qsort used for cmd table sorting */
#define CmdTableSort(CmdTable,n,CompareFunction) \
  qsort((void **)CmdTable,n,sizeof(struct CmdTable_s),CompareFunction)

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Recognize a command.]

  Description [Check the correctness of a command, reading options and
    parameters.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

void
CmdCheckCommand (
  int narg                     /* number of arguments */,
  char *line[]                 /* list of arguments */,
  int *cmdNumber               /* command CmdTable index */,
  CmdOpt_t cmdOpt[]            /* command options */,
  CmdParam_t cmdParam[]        /* command parameters */
)
{
  int i, j, k, optNumber, parNumber;
  int optNameShort; /*signals option name format*/
  int valueFollows; /*if 1, there are no blanks between value and option name*/
  int optNameLen;   /*option name length*/
  char *input, *prompt;

  input = Pdtutil_Alloc (char, STR_LEN);
  prompt = Pdtutil_Alloc (char, STR_LEN);

  /*---------------------------- Check the Command --------------------------*/

  *cmdNumber = (-1);
  for (i=0; i<(sizeof(cmdTable)/sizeof(CmdTable_t)); i++) {
    if (strcmp (line[0], cmdTable[i].name) == 0) {
      *cmdNumber = i;
      break;
    }
  }

  if (*cmdNumber < 0) {
     Pdtutil_Warning (1, "Command not found.");
    /* check option is useless */
    return;
  } else {
    /*strcpy (cmdName, cmdTable[*cmdNumber].name);*/
  }

  /*---------------------------- Check Options ------------------------------*/

  for (i=1, optNumber = 0; i<narg && line[i][0]=='-'; i++) {

      for (j=0; (strcmp (cmdTable[*cmdNumber].opt[j].tag, "")!=0); j++) {
	if ((optNameShort =
            (strcmp (line[i], cmdTable[*cmdNumber].opt[j].nameShort)==0))  
          || 
          (strcmp (line[i], cmdTable[*cmdNumber].opt[j].name) == 0) ) {
     
          /* Copy options to option as a parameter */
	  strcpy (cmdOpt[optNumber].tag, cmdTable[*cmdNumber].opt[j].tag);

          /* Copy value, except for boolean option */
          if( strcmp(cmdTable[*cmdNumber].opt[j].type, "bool") != 0) {
            if(optNameShort) {
	      valueFollows = strcmp(line[i],
                cmdTable[*cmdNumber].opt[j].nameShort);
	      optNameLen = strlen(cmdTable[*cmdNumber].opt[j].nameShort);
	    } else {
	      valueFollows = strcmp(line[i], cmdTable[*cmdNumber].opt[j].name);
              optNameLen = strlen(cmdTable[*cmdNumber].opt[j].name);
            }
	    if(valueFollows != 0) { 
              strcpy (cmdOpt[optNumber].value, &line[i][optNameLen]);
            } else {
              i++;
              strcpy (cmdOpt[optNumber].value, line[i]);
            }
          } else {
            /* the presence of the boolean option itself means 1 */ 
            strcpy (cmdOpt[optNumber].value, "1");
          }
          
          optNumber++;
          break;
        }
      }
     
      if (strcmp (cmdTable[*cmdNumber].opt[j].tag, "")==0) {
         Pdtutil_Warning (1, "Option not found.");
      }
  }

  
  /*------------------------ Check Parameters -------------------------*/

  /*inserts the parameters in the same order they are given*/
  for (parNumber = 0; 
    i<narg && (strcmp(cmdTable[*cmdNumber].param[parNumber].tag, "") != 0);
    i++, parNumber++) {
    strcpy (cmdParam[parNumber].tag,
      cmdTable[*cmdNumber].param[parNumber].tag);
    strcpy (cmdParam[parNumber].value, line[i]);
  }

  /*-------------------------- Check Mandatory Parameters -------------------*/

  if(strcmp(cmdTable[*cmdNumber].param[parNumber].tag, "") != 0) {
    /*
     * Other parameters could be recognized, but are missing in the
     * command line
     */

    /* Checks if any are compulsory */
    for (j=parNumber; strcmp(cmdTable[*cmdNumber].param[j].tag, "") != 0;
      j++) {
      if(cmdTable[*cmdNumber].param[j].paramOptMan == OPT_MAN) {
       	strcpy(cmdParam[parNumber].tag, cmdTable[*cmdNumber].param[j].tag);
        do {
          sprintf (prompt, "%c%s: ",
            toupper(cmdTable[*cmdNumber].param[j].tag[0]),
            &cmdTable[*cmdNumber].param[j].tag[1]);
#if HAVE_LIBREADLINE
          input = readline (prompt);
          add_history (input);
#else
          fprintf (stdout, "\n%s: ", prompt);
	  fgets (input, STR_LEN, stdin);
#endif

	} while(sscanf(input, "%s", cmdParam[parNumber].value) != 1);
        fputc('\n', stdout);
        parNumber++;
      }  
    }
  } else if(i<narg) {
    /*
     *  If all possible parameters have been recognized,
     *  those remaining  in the command line are ignored
     */
    for(; i<narg; i++) {
      fprintf (stdout,"Parameter %s not legal\n",line[i]);
    }
  } 

  /*sentinel*/
  if(parNumber<PARAM_NUM) {
    strcpy(cmdParam[parNumber].tag, "");
  }

  /*---------------------- Check Mandatory Options & Set Defaults -----------*/

  for (i=0, k=optNumber; (strcmp (cmdTable[*cmdNumber].opt[i].tag, "")!=0);
    i++) {
    /*check if option has already been specified*/
    for(j=0; j<k; j++) {
      if(strcmp(cmdOpt[j].tag,cmdTable[*cmdNumber].opt[i].tag) == 0) {
	break;
      }
    }
    /* if not, set it */
    if(j >= k) {
      strcpy(cmdOpt[optNumber].tag, cmdTable[*cmdNumber].opt[i].tag);
      /* if compulsory, prompt the user for a value */
      if(cmdTable[*cmdNumber].opt[i].optOptMan == OPT_MAN) {
        do {
          fputc ('\n', stdout);
          fputc (toupper(cmdTable[*cmdNumber].opt[i].tag[0]), stdout);
	  fprintf (stdout, "%s : ",&cmdTable[*cmdNumber].opt[i].tag[1]);
	  fgets (input, STR_LEN, stdin);
        } while (sscanf (input, "%s", cmdOpt[optNumber].value) != 1);
	fputc ('\n', stdout);
      } else {
        /* if optional, use the default */
        strcpy (cmdOpt[optNumber].value,
          cmdTable[*cmdNumber].opt[i].valueDefault);          
      }
      optNumber++;
    }
  }

  /*sentinel*/
  if(optNumber<OPT_NUM) {
    strcpy(cmdOpt[optNumber].tag, "");
  }
	
  return;
}

/**Function********************************************************************

  Synopsis    [Execute a Command.]

  Description [Easy work given the command table: For each command the
    executable function is associated and directly called.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

void
CmdCommandExecute (
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  FILE **fin                  /* input stream */,
  int cmdNumber               /* command CmdTable index */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
)
{
  /* Given the Command It Calls the Function */
  if (cmdNumber >= 0) {
    (*cmdTable[cmdNumber].command) (fin, cmdMgr, cmdOpt, cmdParam);
  }

  return;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Stores a BLIF file starting from the FSM description.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBlifWrite (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  char *fileName;
  int optPos, paramPos, reduceDelta;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->fsmMgr==NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return;
  }

  /*--------------------- Read Options and Parameters -----------------------*/

  optPos = LocateOption (cmdOpt, "reduceDelta");
  reduceDelta = atoi (cmdOpt[optPos].value);

  paramPos = LocateParam (cmdParam, "filename");
  if (paramPos < 0) {
    fileName = NULL;
  } else {
    fileName =  Pdtutil_StrDup (cmdParam[paramPos].value);
  };

  /*----------------------------- Store FSM ---------------------------------*/

  fprintf (stdout, "Writing Blif File ... "); 

  Fsm_BlifStore (cmdMgr->fsmMgr, reduceDelta, fileName, NULL);

  fprintf (stdout, "Done\n"); 

  return;
}

/**Function********************************************************************

  Synopsis    [Quits the program]

  Description [Free the different managers, registers, etc. pointed by the
    CMD manager and free the CMD manager itsef. Quit the PdTRAV program.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdQuit ( 
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int i;

  /*
   *  Frees managers
   */

  if(cmdMgr->fsmMgr != NULL) {
    Fsm_MgrQuit (cmdMgr->fsmMgr);
  }
  if(cmdMgr->travMgr != NULL) {
    Trav_MgrQuit (cmdMgr->travMgr);
  }
  if(cmdMgr->trMgr != NULL) {
    Tr_MgrQuit (cmdMgr->trMgr);
  }

  /*
   *  Frees Registers
   */

  for (i=0; i<REGISTER_NUMBER_OF; i++) {
    if (cmdMgr->reg[i].tag != NULL) {
      Pdtutil_Free (cmdMgr->reg[i].tag);
    }
    if (cmdMgr->reg[i].bdd != NULL) {
      Ddi_Free (cmdMgr->reg[i].bdd);
    }
    if (cmdMgr->reg[i].bddArray != NULL) {
      Ddi_Free (cmdMgr->reg[i].bddArray);
    }
  }

  /*
   *  Frees DD Manager
   */

  if(cmdMgr->dd != NULL) {
    Ddi_MgrQuit (cmdMgr->dd);
  }

  /*
   *  Quit
   */

  fprintf (stdout, "PdTrav end!\n");

  exit (1);
}

/**Function********************************************************************

  Synopsis    [Init a Decision Manager.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmInit (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int optPos;
  unsigned int varNum, slotNum, cacheSize;
  unsigned long memorySizeMax;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->dd != NULL) {
    Pdtutil_Warning (1, "Decision Diagram Manager already Present.");
    return;
  }

  /*------------------------------ Read Options -----------------------------*/

  optPos = LocateOption (cmdOpt, "varNum");
  varNum = (unsigned int) atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "slotNum");
  slotNum = (unsigned int) atoi (cmdOpt[optPos].value);
  if (slotNum == (-1)) {
    slotNum = DDI_UNIQUE_SLOTS;
  }

  optPos = LocateOption (cmdOpt, "cacheSize");
  cacheSize = (unsigned int) atoi (cmdOpt[optPos].value);
  if (cacheSize == (-1)) {
    cacheSize = DDI_CACHE_SLOTS;
  }

  optPos = LocateOption (cmdOpt, "memorySizeMax");
  memorySizeMax = (unsigned long) atoi (cmdOpt[optPos].value);

  /*------------------------- Creates DD Manager ----------------------------*/

  fprintf (stdout, "Dd Manager Init ... ");
  cmdMgr->dd = Ddi_MgrInit ("ddm", NULL, varNum, slotNum, cacheSize,
    memorySizeMax);
  if(cmdMgr->dd == NULL) {
    Pdtutil_Warning (1, "DD Manager Init failed.");
    exit (1);
  } else {
    fprintf (stdout, "Done\n");
  }     

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description [Not yet supported.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmShare (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Pdtutil_Warning (1, "Operation NOT YET supported.");

  return;
}

/**Function********************************************************************

  Synopsis    [Duplicate a Decision Diagram manager.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmDup (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Ddi_Mgr_t *dd, *dd2; 
  int optPos;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  optPos = LocateParam (cmdParam, "source");
  Pdtutil_Assert(optPos>=0,"no source for dd dup command");

  CmdMgrOperation (cmdMgr, cmdParam[optPos].value, Pdtutil_MgrOpMgrRead_c, 
    &voidPointer,&returnFlag);

  if (returnFlag != Pdtutil_MgrRetDdMgr_c) {
    Pdtutil_Warning (1, "Error looking for ddi manager.");
    return;
  }

  dd = (Ddi_Mgr_t *) voidPointer;

  dd2 = Ddi_MgrDup(dd);

  optPos = LocateParam (cmdParam, "destination");
  Pdtutil_Assert(optPos>=0,"no destination for dd dup command");

  voidPointer = (void *) dd2;
  CmdMgrOperation (cmdMgr, cmdParam[optPos].value, Pdtutil_MgrOpMgrSet_c, 
    &voidPointer,&returnFlag);

  return;
}

/**Function********************************************************************

  Synopsis    [Align two Decision Diagram managers.]

  Description [Not yet supported.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmAlign (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Pdtutil_Warning (1, "Operation NOT YET supported.");

  return;
}

/**Function********************************************************************

  Synopsis    [Set option in a Decision Diagram Manager.]

  Description [It allow a strict control over the cudd-2.3.0 internal
    parameters setting cache size, maximum memory, etc..]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmSet (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Ddi_Mgr_t *dd; 
  int optPos, optParam;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;
  int cacheSize, memoryMax;

  /*----------------------------- Read Parameter ----------------------------*/

  optParam = LocateParam (cmdParam, "destination");
  Pdtutil_Assert (optParam>=0, "No destination for dynord command.");

  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, Pdtutil_MgrOpMgrRead_c, 
    &voidPointer, &returnFlag);

  if (returnFlag != Pdtutil_MgrRetDdMgr_c) {
    Pdtutil_Assert (1, "Error looking for ddi manager.");
    return;
  }
  dd = (Ddi_Mgr_t *) voidPointer;

  /*------------------------------ Read Options -----------------------------*/

  optPos = LocateOption (cmdOpt, "memoryMax");
  memoryMax = atoi (cmdOpt[optPos].value);
  if (memoryMax>0) {
    Ddi_BddSetMaxMemory (dd, (long) memoryMax);
  }

  optPos = LocateOption (cmdOpt, "cacheSize");
  cacheSize = atoi (cmdOpt[optPos].value);
  if (cacheSize>0) {
    Ddi_BddSetMaxCacheHard (dd, ((unsigned int) cacheSize));
  }
 
  return;
}



/**Function********************************************************************

  Synopsis    [Set Decision Diagram manager parameter as far as dynamic
    variable reordering is concern.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmDynOrd (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Ddi_Mgr_t *dd; 
  int optPos, optParam;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;
  int dynordThreshold, nextThreshold;
  Cudd_ReorderingType dynordMethod;

  /*----------------------------- Read Parameter ----------------------------*/

  optParam = LocateParam (cmdParam, "destination");
  Pdtutil_Assert (optParam>=0, "No destination for dynord command.");

  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, Pdtutil_MgrOpMgrRead_c, 
    &voidPointer, &returnFlag);

  if (returnFlag != Pdtutil_MgrRetDdMgr_c) {
    Pdtutil_Assert (1, "Error looking for ddi manager.");
    return;
  }
  dd = (Ddi_Mgr_t *) voidPointer;

  /*------------------------------ Read Options -----------------------------*/

  optPos = LocateOption (cmdOpt, "nextThreshold");
  nextThreshold = atoi (cmdOpt[optPos].value);
  if (nextThreshold>0) {
    Ddi_BddSetNextReordering (dd, (unsigned int) nextThreshold);
  }

  optPos = LocateOption (cmdOpt, "method");
  dynordMethod = Ddi_ReorderingMethodString2Enum
    (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "threshold");
  dynordThreshold = atoi(cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "enable");
  if (atoi(cmdOpt[optPos].value)!=0) {
    Ddi_MgrAutodynEnable (dd, dynordMethod);
    Ddi_MgrSetDynordThresh (dd, dynordThreshold);
  }

  optPos = LocateOption (cmdOpt, "disable");
  if (atoi(cmdOpt[optPos].value)!=0) {
    Ddi_MgrAutodynDisable (dd);
  }

  optPos = LocateOption (cmdOpt, "force");
  if (atoi(cmdOpt[optPos].value)!=0) {
    Ddi_ReduceHeap (dd, dynordMethod, 0/*threshold for no reordering*/);
  }

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmGroup (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Pdtutil_Warning (1, "Operation NOT YET supported.");

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDdmDelete (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int optParam;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------------- Read Parameter ----------------------------*/

  optParam = LocateParam (cmdParam, "manager");

  /*---------------------------- Delete DDI Manager -------------------------*/

  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, Pdtutil_MgrOpMgrDelete_c, 
    &voidPointer, &returnFlag);
  
  return;
}

/**Function********************************************************************

  Synopsis    [Test wrapper]

  Description [NULL function to develop new command.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdDevel (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int i, sizeMin;
  Ddi_Bdd_t *mt, *tr;
  Ddi_Bddarray_t *d, *md;
  Ddi_Meta_Method_e method;
  Ddi_Varset_t *ns;

  i = LocateOption (cmdOpt, "method");
  if (strcmp(cmdOpt[i].value,"size")==0) {
    method = Ddi_Meta_Size;
  }
  else if (strcmp(cmdOpt[i].value,"sched")==0) {
    method = Ddi_Meta_EarlySched;
  }
  else {
    printf("Unknown method: %s\n", cmdOpt[i].value);
    return;
  }

  i = LocateParam (cmdParam, "size");
  sizeMin = atoi(cmdParam[i].value);

  tr = Tr_TrBdd(Tr_MgrReadTr(cmdMgr->trMgr));
  d = Fsm_MgrReadDeltaBDD(cmdMgr->fsmMgr);

  ns = Ddi_VarsetMakeFromArray(Tr_MgrReadPS(cmdMgr->trMgr));
  Ddi_MetaInit(cmdMgr->dd,method,tr,NULL,sizeMin);
  Ddi_Free(ns);
#if 0

  mt = Ddi_BddMakeMeta(tr);
  md = Ddi_BddarrayMakeMeta(d);

  printf("|TR(Bdd)|  = %d\n", Ddi_BddSize(tr));
  printf("|TR(Meta)| = %d\n", Ddi_BddSize(mt));
  printf("|delta(Bdd)|  = %d\n", Ddi_BddarraySize(d));
  printf("|delta(Meta)| = %d\n", Ddi_BddarraySize(md));

  Ddi_BddFromMeta(mt);
  Pdtutil_Assert(Ddi_BddEqual(Ddi_BddSetMono(mt),Ddi_BddSetMono(tr)),
		 "Reconverted Meta-Bdd do not match with original BDD");

  Ddi_Free(mt);
  Ddi_Free(md);

  Ddi_MetaQuit(cmdMgr->dd);
#endif
  return;
}

/**Function********************************************************************

  Synopsis    [Loads a bdd stored in a file]

  Description []

  SideEffects [None]

  SeeAlso     [CmdBddWrite]

******************************************************************************/

static void
CmdBddRead(
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  char *fileName;
  int varMatchMode;   /* variable matching mode */
  char mode;          /* loading mode */
  int i, optPos, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------------- Get Parameters ----------------------------*/

  optPos = LocateParam (cmdParam, "source");
  fileName =  Pdtutil_StrDup (cmdParam[optPos].value);

  /*------------------------- Get and Check Options -------------------------*/

  /* Check What To Read (BDD or Array of BDDs) */
  i = LocateParam (cmdParam, "array");
  arrayFlag = atoi (cmdOpt[i].value);

  /* Retrieve variable matching mode */
  i = LocateOption (cmdOpt, "matchmode");
  if(strcmp(cmdOpt[i].value, "names")==0) {
    varMatchMode = DDDMP_VAR_MATCHNAMES;
  } else if(strcmp(cmdOpt[i].value, "ids")==0) {
    varMatchMode = DDDMP_VAR_MATCHIDS;
  } else {
    Pdtutil_Warning (1,
      "Variable matching mode Not recognized: names/ids required.");
    return;
  }

  /* Retrieve loading mode */
  i = LocateOption (cmdOpt, "loadmode");
  if(strcmp(cmdOpt[i].value, "auto")==0) {
    mode = DDDMP_MODE_DEFAULT;
  } else
  if(strcmp(cmdOpt[i].value, "text")==0) {
    mode = DDDMP_MODE_TEXT;
  } else
  if(strcmp(cmdOpt[i].value, "binary")==0) {
    mode = DDDMP_MODE_BINARY;
  } else {
    Pdtutil_Warning (1, "Loading mode Not recognized.");
    return;
  }

  /*---------------------- Get Source BDD from File -------------------------*/

  fprintf (stdout, "Loading BDD...\n");

  if (arrayFlag == 0) {
    voidPointer = (Ddi_Bdd_t *)
      Ddi_BddLoad (cmdMgr->dd, varMatchMode, mode, fileName, NULL);
  } else {
    voidPointer = (Ddi_Bddarray_t *)
      Ddi_BddarrayLoad (cmdMgr->dd, Fsm_MgrReadVarnames(cmdMgr->fsmMgr),
        Fsm_MgrReadVarauxids(cmdMgr->fsmMgr), mode, fileName, NULL);
  }

  if ( voidPointer==NULL ) {
    Pdtutil_Warning (1, "Cannot load BDD");
    return;
  } 

  /*----------------------------- Set Destination ---------------------------*/

  i = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddSet_c,
    &voidPointer, &returnFlag);
  Ddi_Free (voidPointer);

  return;
}

/**Function********************************************************************

  Synopsis    [Stores a bdd]

  Description []

  SideEffects [None]

  SeeAlso     [CmdBddRead]

******************************************************************************/

static void
CmdBddWrite( 
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int i, flag, optPos, bddFileFormat;
  char *fileName;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------- Read Options and Parameters ---------------------*/

  optPos = LocateOption (cmdOpt, "format");
  bddFileFormat = Fsm_BddFormatString2Int (cmdOpt[optPos].value);

  i = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpRead_c,
    &voidPointer, &returnFlag);

  if (returnFlag==Pdtutil_MgrRetNone_c) {
    Pdtutil_Warning (1, "BDD is empty.");
    return;
  }

  /*----------------------- Store Source BDD to File ------------------------*/

  i = LocateParam (cmdParam, "destination");
  fileName = cmdParam[i].value;
  if (returnFlag==Pdtutil_MgrRetBdd_c) {
    flag = Ddi_BddStore ((Ddi_Bdd_t *) voidPointer, NULL,
      bddFileFormat, fileName, NULL);
  } else {
    flag =  Ddi_BddarrayStore ((Ddi_Bddarray_t *) voidPointer,
      NULL, Fsm_MgrReadVarnames (cmdMgr->fsmMgr), NULL,
      Fsm_MgrReadVarauxids (cmdMgr->fsmMgr), bddFileFormat,
      fileName, NULL);
  }

  if (flag == 0) {
    Pdtutil_Warning (1, "Failed.");
  } 

  return;
}

/**Function********************************************************************

  Synopsis    [Outputs the cubes of a bdd]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddWriteCubes ( 
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int flag, flagFile, optPos, maxcubes, reverse;
  char *fileName;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;
  Ddi_Bdd_t *f;
  Ddi_Bddarray_t *farray;
  FILE *fp = NULL;

  /*----------------------- Read Options and Parameters ---------------------*/

  optPos = LocateOption (cmdOpt, "reverse");
  reverse = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "maxcubes");
  maxcubes = atoi (cmdOpt[optPos].value);

  optPos = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[optPos].value, Pdtutil_MgrOpRead_c,
    &voidPointer, &returnFlag);

  if (returnFlag==Pdtutil_MgrRetNone_c) {
    Pdtutil_Warning (1, "BDD is empty.");
    return;
  }
  if (voidPointer==NULL) {
    Pdtutil_Warning (1, "BDD is NULL.");
    return;
  }

  /*----------------------- Store Source BDD to File ------------------------*/

  optPos = LocateParam (cmdParam, "destination");
  fileName = cmdParam[optPos].value;

  fp = Pdtutil_FileOpen (fp, fileName, "w", &flagFile);
  if (fp == NULL) {
    Pdtutil_Warning (1, "Error opening output file.");
    return;
  }

  /* OK in case print is not called */
  flag = 1;

  if (returnFlag==Pdtutil_MgrRetBdd_c) {
    f = (Ddi_Bdd_t *) voidPointer;
    flag = Ddi_BddPrintSupportAndCubes (f, 1, maxcubes, 0, NULL, fp);
  } else {
    farray = (Ddi_Bddarray_t *) voidPointer;
    flag = Ddi_BddarrayPrintSupportAndCubes (
      farray, 1, maxcubes, 0, reverse, NULL, fp);
  }

  Pdtutil_FileClose (fp, &flagFile);

  if (flag == 0) {
    Pdtutil_Warning (1, "Store cube Failed.");
  } 

  return;
}

/**Function********************************************************************

  Synopsis    [Generates a literal BDD]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddGenLiteral( 
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{ 
  int optPos, optParam, notFlag, varid;
  Ddi_Bdd_t *dest;
  void *voidPointer;
  Pdtutil_MgrOp_t opSet;
  Pdtutil_MgrRet_t returnFlag;
  char *varname;
  Ddi_Var_t *v=NULL;

  /*------------------- Retrieve Source and Destination ---------------------*/

  optPos = LocateOption (cmdOpt, "var");
  varname = cmdOpt[optPos].value;
  if (strcmp(varname,"NULL")!=0) {
    v = Ddi_VarFromName(cmdMgr->dd,varname);
  }

  if (v==NULL) {
    fprintf (stderr, "Error: variable %s not found\n", varname);
    return;
  }

  optPos = LocateOption (cmdOpt, "not");
  notFlag = atoi (cmdOpt[optPos].value);

  dest = Ddi_BddMakeLiteral(v,!notFlag);

  /*---------------------------- Set Destination ---------------------------*/

  opSet = Pdtutil_MgrOpBddSet_c;

  voidPointer = (void *) dest;
  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);
  /*
  Ddi_Free (dest);
  */

  return;
}

/**Function********************************************************************

  Synopsis    [Complements the default BDD]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddNot( 
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{ 
  int optPos, optParam, arrayFlag;
  Ddi_Bdd_t *dest, *tmp;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;

  /*------------------- Retrieve Source and Destination ---------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;
    
    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);

  if (returnFlag==Pdtutil_MgrRetNone_c) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  }

  /*------------------------ Do the Operation -------------------------------*/

  dest = (Ddi_Bdd_t *) voidPointer;
  Ddi_Free (dest);

  tmp = Ddi_BddNot (dest);

  /*---------------------------- Set Destination ---------------------------*/

  voidPointer = (void *) tmp;
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);

  Ddi_Free (tmp);
  return;
}

/**Function********************************************************************

  Synopsis    [And]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddAnd(
  FILE **fin                  /* Input Stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse Manager */,
  CmdOpt_t cmdOpt[]           /* Command Options */,
  CmdParam_t cmdParam[]       /* Command Parameters */
  )
{ 
  Ddi_Bdd_t *dest; /* destination operand*/
  Ddi_Bdd_t *src;   /* source operand*/
  Ddi_Bdd_t *tmp;
  int optPos, optParam, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;
  /*------------------------------ Retrieve Source --------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  optParam = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  src = (Ddi_Bdd_t *) voidPointer;

  if (src == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  } 

  /*-------------------------- Retrieve Destination -------------------------*/

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  dest = (Ddi_Bdd_t *) voidPointer;

  if (dest == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  }

  /*------------------ Do the Operation and Set Destination -----------------*/

  tmp = Ddi_BddAnd (dest, src);
  Ddi_Free (dest);

  voidPointer = (void *) tmp;

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);

  Ddi_Free (tmp);
  return;
}

/**Function********************************************************************

  Synopsis    [Or]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddOr(
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{ 
  Ddi_Bdd_t *dest;
  Ddi_Bdd_t *src;
  Ddi_Bdd_t *tmp;
  int optPos, optParam, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;

  /*------------------------------ Retrieve Source --------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  optParam = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  src = (Ddi_Bdd_t *) voidPointer;

  if (src == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  } 

  /*-------------------------- Retrieve Destination -------------------------*/

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  dest = (Ddi_Bdd_t *) voidPointer;

  if (dest == NULL) {
    fprintf (stderr, "Error : %s is empty!\n", cmdParam[optParam].value);
    return;
  }

  /*----------------- Do the Operation and Set Destination ------------------*/

  tmp = Ddi_BddOr(dest, src);
  Ddi_Free (dest);

  voidPointer = (void *) tmp;
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);
                
  Ddi_Free(tmp);
  return;
}

/**Function********************************************************************

  Synopsis    [Cofactor]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddCof (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{ 
  Ddi_Bdd_t *dest;
  Ddi_Bdd_t *src;
  Ddi_Bdd_t *tmp;
  int optPos,optParam, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;

  /*------------------------------ Retrieve Source --------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  optParam = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  src = (Ddi_Bdd_t *) voidPointer;

  if (src == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  } 

  /*-------------------------- Retrieve Destination -------------------------*/

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  dest = (Ddi_Bdd_t *) voidPointer;

  if (dest == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  }

  /*----------------- Do the Operation and Set Destination ------------------*/

  tmp = Ddi_BddConstrain (dest, src);
  Ddi_Free (dest);

  voidPointer = (void *) tmp;
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);
                
  Ddi_Free(tmp);
  return;
}

/**Function********************************************************************

  Synopsis    [Or]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddRes (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{ 
  Ddi_Bdd_t *dest;
  Ddi_Bdd_t *src;
  Ddi_Bdd_t *tmp;
  int optPos, optParam, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;

  /*------------------------------ Retrieve Source --------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  optParam = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  src = (Ddi_Bdd_t *) voidPointer;

  if (src == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  } 

  /*-------------------------- Retrieve Destination -------------------------*/

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  dest = (Ddi_Bdd_t *) voidPointer;

  if (dest == NULL) {
    fprintf (stderr, "Error : %s is empty!\n", cmdParam[optParam].value);
    return;
  }

  /*----------------- Do the Operation and Set Destination ------------------*/

  tmp = Ddi_BddRestrict (dest, src);
  Ddi_Free (dest);

  voidPointer = (void *) tmp;
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);
                
  Ddi_Free (tmp);
  return;
}


/**Function********************************************************************

  Synopsis    [Existential quantification]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddExist(
  FILE **fin                  /* Input Stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse Manager */,
  CmdOpt_t cmdOpt[]           /* Command Options */,
  CmdParam_t cmdParam[]       /* Command Parameters */
  )
{ 
  Ddi_Bdd_t *dest; /* destination operand*/
  Ddi_Varset_t *vars;   /* source operand*/
  Ddi_Bdd_t *tmp;
  int optPos, optParam, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;

  /*-------------------------- Retrieve Destination -------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  dest = (Ddi_Bdd_t *) voidPointer;

  if (dest == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  }

  /*------------------------------ Retrieve Varset --------------------------*/

  optParam = LocateParam (cmdParam, "varset");
  if ((strcmp(cmdParam[optParam].value,"X")==0) ||
      (strcmp(cmdParam[optParam].value,"x")==0)) {
    vars = Ddi_VarsetMakeFromArray(Fsm_MgrReadVarI(cmdMgr->fsmMgr));
  } else {
    Pdtutil_Warning (1, "This variable set is NOT supported.");
    return;
  }

  /*------------------ Do the Operation and Set Destination -----------------*/

  tmp = Ddi_BddExist(dest,vars);
  Ddi_Free (dest);

  voidPointer = (void *) tmp;
  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);

  Ddi_Free (tmp);
  Ddi_Free(vars);
        
  return;
}


/**Function********************************************************************

  Synopsis    [Universal quantification]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddForall(
  FILE **fin                  /* Input Stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse Manager */,
  CmdOpt_t cmdOpt[]           /* Command Options */,
  CmdParam_t cmdParam[]       /* Command Parameters */
  )
{ 
  Ddi_Bdd_t *dest; /* destination operand*/
  Ddi_Varset_t *vars;   /* source operand*/
  Ddi_Bdd_t *tmp;
  int optPos, optParam, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;

  /*-------------------------- Retrieve Destination -------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }


  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);
  dest = (Ddi_Bdd_t *) voidPointer;

  if (dest == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[optParam].value);
    return;
  }

  /*------------------------------ Retrieve Varset --------------------------*/

  optParam = LocateParam (cmdParam, "varset");
  if ((strcmp(cmdParam[optParam].value,"X")==0) ||
      (strcmp(cmdParam[optParam].value,"x")==0)) {
    vars = Ddi_VarsetMakeFromArray(Fsm_MgrReadVarI(cmdMgr->fsmMgr));
  }
  else {
    Pdtutil_Warning (1, "This variable set is NOT supported.");
    return;
  }

  /*------------------ Do the Operation and Set Destination -----------------*/

  tmp = Ddi_BddNot(dest);
  Ddi_BddExistAcc(tmp,vars);
  Ddi_BddNotAcc(tmp);
  voidPointer = (void *) tmp;
  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);

  Ddi_Free (tmp);
  Ddi_Free(vars);
        
  return;
}

/**Function********************************************************************

  Synopsis    [Deletes Bdd]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddDelete(
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* command manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int optPos, optParam, arrayFlag;
  Pdtutil_MgrOp_t opDel;
  Pdtutil_MgrRet_t returnFlag;

  /*-------------------------- Retrieve Destination -------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opDel = Pdtutil_MgrOpBddDelete_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opDel = Pdtutil_MgrOpArrayDelete_c;
  }

  optParam = LocateParam (cmdParam, "destination");

  /*------------------------- Do the Operation  -----------------------------*/

  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opDel, NULL,
    &returnFlag);

  return;
}

/**Function********************************************************************

  Synopsis    [Take a dense subset of a BDD]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddDenseSet (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* command manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int paramPos, optPos, arrayFlag;
  int threshold, safe, quality;
  double hardlimit;
  void *voidPointer;
  Ddi_Bdd_t *dest, *tmp;
  Pdtutil_MgrRet_t returnFlag;
  Ddi_DenseMethod_e method;
  Pdtutil_MgrOp_t opRead, opSet;

  /*---------------------------- Retrieve Options ---------------------------*/

  optPos = LocateOption (cmdOpt, "method");
  method = Ddi_DenseMethodString2Enum (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "threshold");
  threshold = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "safe");
  safe = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "quality");
  quality = (double) atof (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "hardlimit");
  hardlimit = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;
    
    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  /*-------------------------- Retrieve Destination -------------------------*/

  paramPos = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[paramPos].value, opRead,
    &voidPointer, &returnFlag);

  if (returnFlag != Pdtutil_MgrRetBdd_c) {
    fprintf (stderr, "Error: %s is of wrong type!\n", cmdParam[1].value);
    return;
  }

  dest = (Ddi_Bdd_t *) voidPointer;

  /*--------------------------- Execute Command -----------------------------*/

  tmp = Ddi_BddDenseSet (method, dest, threshold, safe, quality, hardlimit);
  Ddi_Free (dest);
/*@@@ next line updated */
  Ddi_BddPrintStats (tmp,stdout);
  CmdMgrOperation (cmdMgr, cmdParam[paramPos].value, opSet,
    (void *) &tmp, &returnFlag);

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdSetOption (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int paramPos;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------------- Set Destination ---------------------------*/

  paramPos = LocateParam (cmdParam, "newValue");
  voidPointer = (void *) cmdParam[paramPos].value;
  paramPos = LocateParam (cmdParam, "cmdParam");
  CmdMgrOperation (cmdMgr, cmdParam[paramPos].value, Pdtutil_MgrOpOptionSet_c,
    &voidPointer, &returnFlag);

  return;
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdShowOption (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int paramPos;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------------- Set Destination ---------------------------*/

  voidPointer = (void *) NULL;
  paramPos = LocateParam (cmdParam, "cmdParam");
  if (paramPos<0) {
    CmdMgrOperation (cmdMgr, NULL, Pdtutil_MgrOpOptionShow_c,
      &voidPointer, &returnFlag);
  } else {
    CmdMgrOperation (cmdMgr, cmdParam[paramPos].value,
      Pdtutil_MgrOpOptionShow_c, &voidPointer, &returnFlag);
  }
  return;
}

/**Function********************************************************************

  Synopsis    [On-line help: If there is no parameter, list all commands,
    otherwise provides the usage for the command passed as parameter.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdHelp (
  FILE **fin                /* Input Stream */,
  Cmd_Mgr_t *cmdMgr         /* Traverse Manager */,
  CmdOpt_t cmdOpt[]         /* Command Options */,
  CmdParam_t cmdParam[]     /* Command Parameters */
  )
{
  FILE *help;
  char *lineStdin, *lineFile, string[STR_LEN], *command;
  int i, j, flagFile, numberOfRow, cmdNumber, cmdTableSize, optPos,
      optionFound, parameterFound;
  Pdtutil_VerbLevel_e verbosity;

  /*-------------------------- List All Commands ----------------------------*/

  if(strcmp(cmdParam[0].tag, "") == 0) {

    cmdTableSize = sizeof (cmdTable)/sizeof (CmdTable_t);
    CmdTableSort (cmdTable, cmdTableSize, CommandCompare);

    for (i=0; i<(cmdTableSize); i++) {
      fprintf (stdout, "%-25s ", cmdTable[i].name);
      if (((i+1)%COMMAND_PER_ROW)==0) {
        fprintf (stdout, "\n");
      }
    }

    fprintf (stdout, "\n");
    return;
  }

  /*-------------- List One Commands with Options and Parameters -----------*/

  Pdtutil_ChrPrint (stdout, ' ', 2);
  Pdtutil_ChrPrint (stdout, '-', 76);
  fprintf (stdout, "\n");

  command = cmdParam[0].value;

  cmdNumber = (-1);
  for (i=0; i<(sizeof(cmdTable)/sizeof(CmdTable_t)); i++) {
    if(strcmp(command, cmdTable[i].name) == 0) {
      cmdNumber = i;
      break;
    }
  }

  if (cmdNumber == (-1)) {
    Pdtutil_Warning (1, "Command not found");
    return;
  } else { 
    Pdtutil_ChrPrint (stdout, ' ', 2);
    fprintf (stdout, "%s ", cmdTable[cmdNumber].name);
  }

  /*
   *  Option List
   */

  for (j = 0; strcmp (cmdTable[cmdNumber].opt[j].tag, "") !=0 ; j++) {
    if(cmdTable[cmdNumber].opt[j].optOptMan == OPT_OPT) {
      fprintf (stdout, "[%s|%s] ", cmdTable[cmdNumber].opt[j].nameShort,
        cmdTable[cmdNumber].opt[j].name);
    } else {
      fprintf (stdout, "%s|%s  ", cmdTable[cmdNumber].opt[j].nameShort,
        cmdTable[cmdNumber].opt[j].name);
    }

    if (((j+1)%OPTION_PER_ROW)==0) {
      fprintf (stdout, "\n");    
      Pdtutil_ChrPrint (stdout, ' ', (3 + strlen (cmdTable[cmdNumber].name)));
    }
  }

  /* 
   *  Parameter list
   */

  for (j=0; (strcmp (cmdTable[cmdNumber].param[j].tag, "")!=0); j++) {
    if(cmdTable[cmdNumber].param[j].paramOptMan == OPT_OPT) {
      fprintf (stdout, "[%s] ", cmdTable[cmdNumber].param[j].tag);
    } else {
      fprintf (stdout, "%s  ", cmdTable[cmdNumber].param[j].tag);
    }
  }
  fprintf (stdout, "\n");

  /*-------- If a high level of verbosity is set, give more details ---------*/

  optPos = LocateOption (cmdOpt, "verbosity");
  verbosity = Pdtutil_VerbosityString2Enum (cmdOpt[optPos].value);
  
  /*
   *  It is more useful to give details ALWAYS (so it is the actual default
   *  setting)
   */

  if (verbosity < Pdtutil_VerbLevelUsrMax_c) {
    return;
  }

  /* Open Help File */
  help = Pdtutil_FileOpen (NULL, cmdMgr->settings.helpName, "r", &flagFile);
  if (help == NULL) {
    Pdtutil_ChrPrint (stdout, ' ', 2);
    Pdtutil_Warning (1,
      "Help file missing: Copy-Link the cmdHelp.txt file (cmd directory).");
    Pdtutil_ChrPrint (stdout, ' ', 2);
    Pdtutil_ChrPrint (stdout, '-', 76);
    fprintf (stdout, "\n");

    return;
  }
    
  /*
   * Look for the  command
   */

  lineStdin = Pdtutil_Alloc (char, (ROW_LEN_MAX));
  lineFile = Pdtutil_Alloc (char, (ROW_LEN_MAX));

  do {
    fgets(lineFile, ROW_LEN_MAX, help);

    if (strstr(lineFile, ".c") != NULL) {
      sscanf (lineFile, "%*s%s", string);
      if (strcmp (string, cmdTable[cmdNumber].name) == 0) {
        break;
      }
    }
  } while(feof(help) == 0);

  /*
   * If no command has been found print warning
   */

  if (feof (help)) {
    Pdtutil_ChrPrint (stdout, ' ', 2);
    fprintf (stdout, "Undocumented command!\n");

    free (lineFile);
    free (lineStdin);
    Pdtutil_FileClose (help, &flagFile);

    Pdtutil_ChrPrint (stdout, ' ', 2);
    Pdtutil_ChrPrint (stdout, '-', 76);
    fprintf (stdout, "\n");

    return;
  }

  /*
   * Command found
   */

  numberOfRow = 0;
  optionFound = 0;
  parameterFound = 0;

  fprintf (stdout, "\n");
  numberOfRow++;
  while (fgets (lineFile, ROW_LEN_MAX, help) != NULL) {

    if ((strncmp (lineFile, ".c", 2)) == 0) {
      break;
    }
    
    if ((strncmp (lineFile, ".o", 2)) == 0) {
      if (optionFound==0) {
        fprintf (stdout, "\n");
        Pdtutil_ChrPrint (stdout, ' ', 2);
        fprintf (stdout, "Command Options\n");
        optionFound = 1;
      }

      Pdtutil_ChrPrint (stdout, ' ', 4);
      fprintf (stdout, "%s", lineFile+3);

      sscanf (lineFile+2, "%s", string);
      for (j = 0; strcmp (cmdTable[cmdNumber].opt[j].tag, "") !=0 ; j++) {
        if (strcmp(cmdTable[cmdNumber].opt[j].tag, string) == 0) {
          Pdtutil_ChrPrint (stdout, ' ', 8);
          fprintf (stdout, "Default: %s\n",
            cmdTable[cmdNumber].opt[j].valueDefault);
        }
      }
    }

    if ((strncmp (lineFile, ".p", 2)) == 0) {
      if (parameterFound==0) {
        fprintf (stdout, "\n");
        Pdtutil_ChrPrint (stdout, ' ', 2);
        fprintf (stdout, "Command Parameters\n");
        parameterFound = 0;
      }

      Pdtutil_ChrPrint (stdout, ' ', 4);
      fprintf (stdout, "%s", lineFile+3);
    }

    if (lineFile[0] != '.') {
      if (optionFound==0 && parameterFound==0) {
        Pdtutil_ChrPrint (stdout, ' ', 2);
      } else {
        Pdtutil_ChrPrint (stdout, ' ', 8);
      }
      for (i=0; i< strlen (lineFile); i++) {
        if (lineFile[i] != ' ') {
          break;
        }
      }
      fprintf (stdout, "%s", &lineFile[i]);
    }
     
    numberOfRow++;

    if ((numberOfRow%LINE_PER_SCREEN)==0) {
      fprintf (stdout, "<press return>");
      fgets (lineStdin, ROW_LEN_MAX, stdin);
    }
  }

  free (lineFile);
  free (lineStdin);
  Pdtutil_FileClose (help, &flagFile);

  Pdtutil_ChrPrint (stdout, ' ', 2);
  Pdtutil_ChrPrint (stdout, '-', 76);
  fprintf (stdout, "\n");

  return;
}


/**Function********************************************************************

  Synopsis    [Retime FSMs for Verification Purpose.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdFsmRetime (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int optPos;
  Fsm_Mgr_t *fsmMgrRetimed;
  Fsm_Retime_t retimeStr;

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "retimeEqual");
  retimeStr.retimeEqualFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "removeLatches");
  retimeStr.removeLatches = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "type");

  if (strcmp (cmdOpt[optPos].value, "pdtrav") == 0) {
    fsmMgrRetimed = Fsm_RetimeForRACompute (cmdMgr->fsmMgr, &retimeStr);
  } else {
    fsmMgrRetimed = NULL;
    /* Retime with sis ... to be done */
  }

  Fsm_MgrQuit (cmdMgr->fsmMgr);
  cmdMgr->fsmMgr = fsmMgrRetimed;

  return;
}

/**Function********************************************************************

  Synopsis    [Remove Auxiliary Variable From FSM Structure]

  Description [Returns the old FSM if Auxiliary Variables are not present.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdFsmAuxVarRemove (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  /*---------------------- Read Options and Parameters ----------------------*/

  /*-------------------------- Delete FSM Manager ---------------------------*/

  Fsm_MgrAuxVarRemove (cmdMgr->fsmMgr);

  return;
}

/**Function********************************************************************

  Synopsis    [loads a BDD based FSM description]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdFsmRead (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Fsm_Mgr_t *fsmMgr;
  char *fileFsmName, *ordFileName;
  int blifFlag, bddFlag, nogroupFlag, errorFlag, optPos, paramPos;
  Pdtutil_VariableOrderFormat_e ordFileFormat;

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "ordFormat");
  ordFileFormat = Pdtutil_VariableOrderFormatString2Enum
    (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "blif");
  blifFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "bdd");
  bddFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "nogroup");
  nogroupFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "ord");
  if (strcmp (cmdOpt[optPos].value, "") == 0) {
    ordFileName = NULL;
  } else {
    ordFileName = Pdtutil_StrDup (cmdOpt[optPos].value);
  }

  paramPos = LocateParam (cmdParam, "filename");
  if (paramPos < 0) {
    fileFsmName = NULL;
  } else {
    fileFsmName =  Pdtutil_StrDup (cmdParam[paramPos].value);
  };

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->dd == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence (use ddm_init first).");
    return;
  }

  if (cmdMgr->fsmMgr != NULL) {
    Pdtutil_Warning (1, "FSM Structure Already Stored.");
    return;
  }

  /*-------------------------- Create FSM Manager ---------------------------*/

  fprintf (stdout, "FSM Manager Init ... ");
  fsmMgr = Fsm_MgrInit ("fsm");
  if (fsmMgr == NULL) {
    fprintf (stderr,"Error: FSM Init failed!\n");
    exit (1);
  } else {
    fprintf (stdout, "Done\n");
  }     

  /*------------------------------- Load FSM --------------------------------*/

  fprintf (stdout, "FSM Loading ...\n"); 

  if (blifFlag == 0) {
    errorFlag = Fsm_MgrLoad (&fsmMgr, cmdMgr->dd, fileFsmName, ordFileName,
      bddFlag, ordFileFormat);
  } else {
    errorFlag = Fsm_MgrLoadFromBlif (&fsmMgr, cmdMgr->dd, fileFsmName,
      ordFileName, bddFlag, ordFileFormat);
  }

  Pdtutil_Warning (errorFlag, "FSM not loaded!");
  if (errorFlag == 0) {
    fprintf (stdout, "Done\n");

    cmdMgr->fsmMgr = fsmMgr;

    if (Ddi_MgrReadVarauxids (cmdMgr->dd) == NULL) {
      Ddi_MgrSetVarauxids (cmdMgr->dd, Fsm_MgrReadVarauxids (cmdMgr->fsmMgr));
    }

    if (Ddi_MgrReadVarnames (cmdMgr->dd) == NULL) {
      Ddi_MgrSetVarnames (cmdMgr->dd, Fsm_MgrReadVarnames (cmdMgr->fsmMgr));
    }

    if (nogroupFlag == 0) {
      Ddi_MgrCreateGroups2 (Fsm_MgrReadDdManager (cmdMgr->fsmMgr),
        Fsm_MgrReadVarPS (cmdMgr->fsmMgr), Fsm_MgrReadVarNS (cmdMgr->fsmMgr));
    }
  }

  return;
}


/**Function********************************************************************

  Synopsis    [loads a BDD based FSM description]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdFsmPmBuild (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Ddi_Mgr_t *ddTmp;
  char *fileFsmName, *ordFileName;
  int blifFlag, bddFlag, errorFlag, optPos, paramPos;
  Fsm_Mgr_t *fsmTmp, *fsmPM;
  Pdtutil_VariableOrderFormat_e ordFileFormat;

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "ordFormat");
  ordFileFormat = Pdtutil_VariableOrderFormatString2Enum
    (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "blif");
  blifFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "bdd");
  bddFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "ord");
  if (strcmp (cmdOpt[optPos].value, "") == 0) {
    ordFileName = NULL;
  } else {
    ordFileName =  Pdtutil_StrDup (cmdOpt[optPos].value);
  }

  paramPos = LocateParam (cmdParam, "filename");
  if (paramPos < 0) {
    fileFsmName = NULL;
  } else {
    fileFsmName =  Pdtutil_StrDup (cmdParam[paramPos].value);
  }

  /*---------------------- Create Temporary DD Manager ----------------------*/

  ddTmp = Ddi_MgrInit ("ddm", NULL, 0, DDI_UNIQUE_SLOTS, DDI_CACHE_SLOTS, 0);
  if (ddTmp == NULL) {
    fprintf (stderr, "Error: DD Manager Init failed!\n");
    return;
  }

  /*------------------------- Create Temporary FSM --------------------------*/

  fsmTmp = Fsm_MgrInit ("fsm");
  fsmPM = Fsm_MgrInit ("pm");
  if (fsmTmp==NULL || fsmPM==NULL) {
    fprintf (stderr, "Error: FSM Init failed!\n");
    return;
  }

  /*------------------------------- Load FSM --------------------------------*/

  fprintf (stdout, "FSM Loading ...\n"); 

  if (blifFlag == 0) {
    errorFlag = Fsm_MgrLoad (&fsmTmp, ddTmp, fileFsmName, ordFileName,
      bddFlag, ordFileFormat);
  } else {
    errorFlag = Fsm_MgrLoadFromBlif (&fsmTmp, ddTmp, fileFsmName,
      ordFileName, bddFlag, ordFileFormat);
  }

  Pdtutil_Warning (errorFlag, "FSM not loaded!");
  if (errorFlag == 0) {
    fprintf (stdout, "Done\n");

    if (Ddi_MgrReadVarauxids (cmdMgr->dd) == NULL) {
      Ddi_MgrSetVarauxids (cmdMgr->dd, Fsm_MgrReadVarauxids (cmdMgr->fsmMgr));
    }
    if (Ddi_MgrReadVarnames (cmdMgr->dd) == NULL) {
      Ddi_MgrSetVarnames (cmdMgr->dd, Fsm_MgrReadVarnames (cmdMgr->fsmMgr));
    }
 
    Ddi_MgrCreateGroups2 (Fsm_MgrReadDdManager (cmdMgr->fsmMgr),
      Fsm_MgrReadVarPS (cmdMgr->fsmMgr), Fsm_MgrReadVarNS (cmdMgr->fsmMgr));
  }

  /*------------------------------- Build PM --------------------------------*/

  errorFlag = Fsm_MgrPMBuild (&fsmPM, cmdMgr->fsmMgr, fsmTmp);

  Pdtutil_Warning (errorFlag, "PM not created!");
  if (errorFlag == 0) {
    fprintf (stdout, "Done\n");
  }

  return;
}


/**Function********************************************************************

  Synopsis    [Stores a BDD based FSM description file.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdFsmWrite (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  char *fileName;
  int optPos, paramPos, bddFlag, bddFileFormat;
  Pdtutil_VariableOrderFormat_e ordFileFormat;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->fsmMgr==NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return;
  }

  /*--------------------- Read Options and Parameters -----------------------*/

  optPos = LocateOption (cmdOpt, "ordFormat");
  ordFileFormat = Pdtutil_VariableOrderFormatString2Enum
    (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "bdd");
  bddFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "bddFormat");
  bddFileFormat = Fsm_BddFormatString2Int (cmdOpt[optPos].value);

  paramPos = LocateParam (cmdParam, "filename");
  if (paramPos < 0) {
    fileName = NULL;
  } else {
    fileName =  Pdtutil_StrDup (cmdParam[paramPos].value);
  };

  /*----------------------------- Store FSM ---------------------------------*/

  fprintf (stdout, "Writing ... "); 

  Fsm_MgrStore (cmdMgr->fsmMgr, fileName, NULL, bddFlag, bddFileFormat,
    ordFileFormat);

  fprintf (stdout, "Done\n"); 

  return;
}

/**Function********************************************************************

  Synopsis    [Deletes a BDD based FSM description]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdFsmDelete (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->fsmMgr == NULL) {
    Pdtutil_Warning (1, "No FSM Structure loaded.");
    return;
  }

  /*-------------------------- Delete FSM Manager ---------------------------*/

  fprintf (stdout, "Deleting FSM Manager ... ");

  Fsm_MgrQuit (cmdMgr->fsmMgr);
  cmdMgr->fsmMgr = NULL;

  fprintf (stdout, "Done\n");
  fprintf (stdout,
    "(Before Reading-In Another FSM, Delete the Decision Manager)\n");
  
  return;
}

/**Function********************************************************************

  Synopsis    [Build a Transition Relation]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTrInit (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Fsm_Mgr_t *fsmMgr; 
  Tr_Mgr_t *trMgr; 
  Tr_Tr_t *tr;
  Ddi_Bdd_t *trBdd, *fsmTr;
  Ddi_Vararray_t *iVararray;
  int optPos, build, file;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->fsmMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence: Create FSM Manager first.");
    return;
  }

  if (cmdMgr->trMgr != NULL) {
    Pdtutil_Warning (1,
      "Command Out of Sequence: TR Manager already present.");
    return;
  }

  /*-------------------- Read Options and Parameters ------------------------*/

  optPos = LocateOption (cmdOpt, "file");
  file = atoi (cmdOpt[optPos].value);
  if (file == 1) {
    Pdtutil_Warning (1, "Option NOT YET supported.");
    return;
  }

  optPos = LocateOption (cmdOpt, "build");
  build = atoi (cmdOpt[optPos].value);

  fsmMgr = cmdMgr->fsmMgr;
  fsmTr = Fsm_MgrReadTrBDD (fsmMgr);
  if ((!build)&&(fsmTr == NULL)) {
    fprintf (stdout, "FSM Transition Relation is NULL.\n");
    fprintf (stdout, "Try rebuild from delta's (build option).\n");
    return;
  }

  /*------------------- Create Transition Relation Manager ------------------*/

  fprintf (stdout, "Transition Relation Manager Init ... ");
  cmdMgr->trMgr = Tr_MgrInit (NULL, cmdMgr->dd);
  if(cmdMgr->trMgr == NULL) {
    fprintf (stderr,
      "Error: Transition Relation Manager Init failed!\n");
    exit (1);
  } else {
    fprintf (stdout, "Done\n");
  }

  /*-------------------------- Assign Tr Manager fields  --------------------*/

  iVararray = Fsm_MgrReadVarI (fsmMgr);
  if (Fsm_MgrReadVarAuxVar (fsmMgr) != NULL) {
    Ddi_VararrayAppend (iVararray, Fsm_MgrReadVarAuxVar (fsmMgr));
  }

  Tr_MgrSetI (cmdMgr->trMgr, iVararray);
  Tr_MgrSetPS (cmdMgr->trMgr, Fsm_MgrReadVarPS (fsmMgr));
  Tr_MgrSetNS (cmdMgr->trMgr, Fsm_MgrReadVarNS (fsmMgr));

  trMgr = cmdMgr->trMgr;

  if (build) {
    tr = Tr_TrMakePartConjFromFuns (trMgr,Fsm_MgrReadDeltaBDD (fsmMgr),
      Fsm_MgrReadVarNS (fsmMgr));
#if 0
    , Fsm_MgrReadAuxVarBDD (fsmMgr),
      Fsm_MgrReadVarAuxVar (fsmMgr));
#endif
  } else {
    tr = Tr_TrMakeFromRel(trMgr,fsmTr);
  }

  Tr_MgrSetTr (cmdMgr->trMgr, tr);

  Tr_TrFree (tr);

  return;
}

/**Function********************************************************************

  Synopsis    [Deletes a Transition Relation Manager]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTrDelete (
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->trMgr == NULL) {
    Pdtutil_Warning (1, "No Transition relation Manager to delete.");
    return;
  }

  /*------------------- Delete Transition Relation Manager ------------------*/

  fprintf (stdout, "Deleting Transition Relation Manager... ");

  Tr_MgrQuit (cmdMgr->trMgr);
  cmdMgr->trMgr = NULL;

  fprintf (stdout, "Done\n");
  
  return;
}

/**Function********************************************************************

  Synopsis    [Init Traversal Manager.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTravInit (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Fsm_Mgr_t *fsmMgr; 
  Tr_Mgr_t *trMgr; 
  Trav_Mgr_t *travMgr; 
  Tr_Tr_t *tr=NULL;
  Ddi_Bdd_t *trBdd, *reached, *from;
  Ddi_Vararray_t *pi, *psv, *nsv;
  int optPos;
  int backward = 0;
  int keepNew = 0;
  char *reached_file, *from_file, *tr_file;
  Ddi_Mgr_t *ddTr, *ddR;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->trMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence: Create TR Manager first.");
    return;
  }

  if (cmdMgr->travMgr != NULL) {
    Pdtutil_Warning (1,
      "Command Out of Sequence: TRAV Manager already present.");
    return;
  }

  /*------------------------ Create Traversal Manager -----------------------*/

  fprintf (stdout, "Traversal Manager Init ... ");
  cmdMgr->travMgr = Trav_MgrInit (NULL, cmdMgr->dd);
  if (cmdMgr->travMgr == NULL) {
    fprintf (stderr, "Error: Traversal Manager Init failed!\n");
    exit (1);
  } else {
    fprintf (stdout, "Done\n");
  }

  /*-------------------- Read Options and Parameters ------------------------*/

  optPos = LocateOption (cmdOpt, "reached");
  reached_file = cmdOpt[optPos].value;
  optPos = LocateOption (cmdOpt, "from");
  from_file = cmdOpt[optPos].value;
  optPos = LocateOption (cmdOpt, "tr");
  tr_file = cmdOpt[optPos].value;
  optPos = LocateOption (cmdOpt, "backward");
  backward = atoi(cmdOpt[optPos].value);
  optPos = LocateOption (cmdOpt, "keepNew");
  keepNew = atoi(cmdOpt[optPos].value);

  fsmMgr = cmdMgr->fsmMgr;
  trMgr = cmdMgr->trMgr;
  travMgr = cmdMgr->travMgr;

  ddR = Trav_MgrReadDdiMgrR (travMgr);
  ddTr = Trav_MgrReadDdiMgrTr (travMgr);

  if (Tr_MgrReadTr (trMgr) != NULL) {
    pi = Tr_MgrReadI (trMgr);
    psv = Tr_MgrReadPS (trMgr);
    nsv = Tr_MgrReadNS (trMgr);
    tr = Tr_TrDup(Tr_MgrReadTr(trMgr));
  } else {
    if (Fsm_MgrReadTrBDD (fsmMgr) != NULL) {
      pi = Fsm_MgrReadVarI (fsmMgr);
      psv = Fsm_MgrReadVarPS (fsmMgr);
      nsv = Fsm_MgrReadVarNS (fsmMgr);
      trBdd = Fsm_MgrReadTrBDD (fsmMgr);
      tr = Tr_TrMakeFromRel (trMgr,trBdd);
    } else {
      Pdtutil_Warning (1, "No FSM loaded.");
      return; 
    }
  }

  if (strcmp (from_file, "NULL") != 0) {
    from = Ddi_BddLoad (Trav_MgrReadDdiMgrR (travMgr), DDDMP_VAR_MATCHNAMES, 
      DDDMP_MODE_DEFAULT, from_file, NULL);
  } else {
    from = Ddi_BddDup (Fsm_MgrReadInitBDD (fsmMgr));
  }
  
  if (strcmp(reached_file,"NULL")!=0) {
    reached = Ddi_BddLoad (Trav_MgrReadDdiMgrR (travMgr), DDDMP_VAR_MATCHNAMES, 
      DDDMP_MODE_DEFAULT, reached_file, NULL);
  } else {
    reached = Ddi_BddDup (from);
  }
  
  if (strcmp(tr_file,"NULL")!=0) {
    Tr_TrFree(tr);
    trBdd = Ddi_BddLoad (Trav_MgrReadDdiMgrTr (travMgr), DDDMP_VAR_MATCHNAMES, 
      DDDMP_MODE_DEFAULT, tr_file, NULL);
    tr = Tr_TrMakeFromRel (trMgr,trBdd);
    Ddi_Free(trBdd);
  }
  
  if (backward && !Trav_MgrReadBackward(travMgr)) {
    /* backward traversal: swap present/next state variables */
    Trav_MgrSetBackward(travMgr,1);
    Tr_TrReverseAcc (tr);
  }

  if (keepNew) {
    Trav_MgrSetKeepNew(travMgr,1);
  }

  Trav_MgrSetI (travMgr, pi);
  Trav_MgrSetPS (travMgr, psv);
  Trav_MgrSetNS (travMgr, nsv);
  Trav_MgrSetTr (travMgr, tr);
  Trav_MgrSetReached (travMgr, reached);
  Trav_MgrSetFrom (travMgr, from);

  Tr_TrFree (tr);
  Ddi_Free (reached);
  Ddi_Free (from);

  return;
}

/**Function********************************************************************

  Synopsis    [ Delete a Traversal Manager]

  Description []

  SideEffects [None]

  SeeAlso     [CmdTravInit]

******************************************************************************/

static void
CmdTravDelete (
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->travMgr == NULL) {
    Pdtutil_Warning (1, "No Transition Relation Manager to delete.");
    return;
  }

  /*------------------- Delete Traversal Manager ----------------------------*/

  fprintf (stdout, "Deleting Traversal Manager... ");

  Trav_MgrQuit (cmdMgr->travMgr);
  cmdMgr->travMgr = NULL;

  fprintf (stdout, "Done\n");
  
  return;
}

/**Function********************************************************************

  Synopsis    [Manage TR profiles]

  Description [It allows three basic action:
    1) profile creation
    2) profile printing statistic
    3) pruning TR based on the profile collected.
    ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTravProfile (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int optPos, threshold;
  Ddi_Bdd_t *Tr;
  Cuplus_PruneHeuristic_e heuristic;

  Pdtutil_Warning (1, "Profiles non handled in pdtrav-2.0");

#if 0
  optPos = LocateParam (cmdParam, "action");
  Pdtutil_Assert (optPos>=0, "No action for profile command.");

  Tr = Trav_MgrReadTr (cmdMgr->travMgr);

  /*
   *  Profile Create
   */

  if (strcmp(cmdParam[optPos].value, "create")==0) {
    optPos = LocateOption (cmdOpt, "dac99");
    if (atoi(cmdOpt[optPos].value)!=0) {
      Ddi_ProfileInfoInit (Tr, NULL, (unsigned char) 1);
    } else {
      Ddi_ProfileInfoInit (Tr, NULL, (unsigned char) 0);
    }

    return;
  }

  /*
   *  Profile Print
   */

  if (strcmp(cmdParam[optPos].value, "print")==0) {
    Ddi_ProfileInfoPrint (Tr);

    return;
  }

  /*
   *  Profile Prune
   */

  if (strcmp(cmdParam[optPos].value, "prune")==0) {
    optPos = LocateOption (cmdOpt, "heuristic");
    Pdtutil_Assert (optPos>=0, "Missing prune heuristic.");
    heuristic = Ddi_ProfileHeuristicString2Enum (cmdOpt[optPos].value);
    optPos = LocateOption (cmdOpt, "threshold");
    Pdtutil_Assert (optPos>=0, "Missing prune threshold.");
    threshold = atoi (cmdOpt[optPos].value);

    Tr = Ddi_PruneProfiled (Tr, heuristic, threshold);
    Trav_MgrSetTr (cmdMgr->travMgr, Tr);

    return;
  }

  /*
   *  Profile Wrong Option
   */

  Pdtutil_Warning (1, "Unknown action (create/print/prune allowed).");

#endif
  return;
}

/**Function********************************************************************

  Synopsis    [Main traversal command]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTravTraverse (
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  int optPos;
  int logPeriod, depth, partThreshold, auxMgr;
  int logPeriodSave = (-1), depthSave = (-1),
      partThresholdSave = (-1), auxMgrSave = 0;
  int logPeriodFlag, depthFlag, partThresholdFlag, auxMgrFlag;
  int partMethodFlag, fromSelectFlag, verbosityFlag;
  Part_Method_e partMethod,
    partMethodSave = Part_MethodNone_c;
  Trav_FromSelect_e fromSelect,
    fromSelectSave = Trav_FromSelectSame_c;
  Pdtutil_VerbLevel_e verbosity,
    verbositySave = Pdtutil_VerbLevelAppMax_c;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->travMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return;
  }

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "auxMgr");
  auxMgr = atoi (cmdOpt[optPos].value);
  if (auxMgr == 0) {
    auxMgrFlag = 0;
  } else {
    auxMgrFlag = 1;
    auxMgrSave = Trav_MgrReadMgrAuxFlag (cmdMgr->travMgr);
    Trav_MgrSetMgrAuxFlag (cmdMgr->travMgr, auxMgr);
  }

  optPos = LocateOption (cmdOpt, "depth");
  depth = atoi (cmdOpt[optPos].value);
  if (depth == (-1)) {
    depthFlag = 0;
  } else {
    depthFlag = 1;
    depthSave = Trav_MgrReadMaxIter (cmdMgr->travMgr);
    Trav_MgrSetMaxIter (cmdMgr->travMgr, depth);
  }

  optPos = LocateOption (cmdOpt, "fromSelect");
  fromSelect = Trav_FromSelectString2Enum (cmdOpt[optPos].value);
  if (fromSelect == Trav_FromSelectSame_c) {
    fromSelectFlag = 0;
  } else {
    fromSelectFlag = 1;
    fromSelectSave = Trav_MgrReadFromSelect (cmdMgr->travMgr);
    Trav_MgrSetFromSelect (cmdMgr->travMgr, fromSelect);
  }

  optPos = LocateOption (cmdOpt, "logPeriod");
  logPeriod = atoi (cmdOpt[optPos].value);
  if (logPeriod == (-1)) {
    logPeriodFlag = 0;
  } else {
    logPeriodFlag = 1;
    logPeriodSave = Trav_MgrReadLogPeriod (cmdMgr->travMgr);
    Trav_MgrSetLogPeriod (cmdMgr->travMgr, logPeriod);
  }

  optPos = LocateOption (cmdOpt, "partMethod");
  partMethod = Part_MethodString2Enum (cmdOpt[optPos].value);
  if (partMethod == Part_MethodNone_c) {
    partMethodFlag = 0;
  } else {
    partMethodFlag = 1;
    partMethodSave = Tr_MgrReadPartitionMethod (cmdMgr->trMgr);
    Tr_MgrSetPartitionMethod (cmdMgr->trMgr, partMethod);
  }

  optPos = LocateOption (cmdOpt, "partThreshold");
  partThreshold = atoi (cmdOpt[optPos].value);
  if (partThreshold == (-1)) {
    partThresholdFlag = 0;
  } else {
    partThresholdFlag = 1;
    partThresholdSave = Tr_MgrReadPartThFrom (cmdMgr->trMgr);
    Tr_MgrSetPartThFrom (cmdMgr->trMgr, partThreshold);
  }

  optPos = LocateOption (cmdOpt, "verbosity");
  verbosity = Pdtutil_VerbosityString2Enum (cmdOpt[optPos].value);
  if (verbosity == Pdtutil_VerbLevelAppMax_c) {
    verbosityFlag = 0;
  } else {
    verbosityFlag = 1;
    verbositySave = Trav_MgrReadVerbosity (cmdMgr->travMgr);
    Trav_MgrSetVerbosity (cmdMgr->travMgr, verbosity);
  }

  /*------------------------------- Traverse --------------------------------*/

  Trav_Main (cmdMgr->travMgr);
 
  /*-------------------- Put Back Old Default Options ----------------------*/

  if (auxMgrFlag == 1) {
    Trav_MgrSetMgrAuxFlag (cmdMgr->travMgr, auxMgrSave);
  }
  if (depthFlag == 1) {
    Trav_MgrSetMaxIter (cmdMgr->travMgr, depthSave);
  }
  if (fromSelectFlag == 1) {
    Trav_MgrSetFromSelect (cmdMgr->travMgr, fromSelectSave);
  }
  if (logPeriodFlag == 1) {
    Trav_MgrSetLogPeriod (cmdMgr->travMgr, logPeriodSave);
  }
  if (partMethodFlag == 1) {
    Tr_MgrSetPartitionMethod (cmdMgr->trMgr, partMethodSave);
  }
  if (partThresholdFlag == 1) {
    Tr_MgrSetPartThFrom (cmdMgr->trMgr, partThresholdSave);
  }
  if (verbosityFlag == 1) {
    Trav_MgrSetVerbosity (cmdMgr->travMgr, verbositySave);
  }

  return;
}

/**Function********************************************************************

  Synopsis    [Main Simulation Command]

  Description [Following CmdTravTraverse call the symbolic simulation
    instead]

  SideEffects [None]

  SeeAlso     [CmdTravTraverse]

******************************************************************************/

static void
CmdTravSimulate (
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  int optPos, logPeriod, deadEndNumberOf, depth, simFlag,
    depthBreadth, random;
  char *pattern, *init, *result;

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "depth");
  depth = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "logPeriod");
  logPeriod = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "deadEnd");
  deadEndNumberOf = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "simFlag");
  simFlag = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "depthBreadth");
  depthBreadth = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "random");
  random = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "pattern");
  pattern = Pdtutil_Alloc (char, strlen (cmdOpt[optPos].value) + 1);
  strcpy (pattern, cmdOpt[optPos].value);
  
  optPos = LocateOption (cmdOpt, "init");
  if (strcmp(cmdOpt[optPos].value, "NULL")==0) {
    init = NULL;
  } else {
    init = Pdtutil_Alloc (char, strlen (cmdOpt[optPos].value) + 1);
    strcpy (init, cmdOpt[optPos].value);
  }

  optPos = LocateOption (cmdOpt, "result");
  result = Pdtutil_Alloc (char, strlen (cmdOpt[optPos].value) + 1);
  strcpy (result, cmdOpt[optPos].value);

  /*------------------------------- Traverse --------------------------------*/

  Trav_SimulateMain (cmdMgr->fsmMgr, depth, deadEndNumberOf, logPeriod,
    simFlag, depthBreadth, random, init, pattern, result);

  /*-------------------- Put Back Old Default Options ----------------------*/

  return;
}

/**Function********************************************************************

  Synopsis    [Generate mismatch sequence]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTravMismatch (
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Traverse manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  int optPos, maxDepth;
  int verbosityFlag;
  Pdtutil_VerbLevel_e verbosity,
    verbositySave = Pdtutil_VerbLevelAppMax_c;
  Trav_Mgr_t *travMgr = cmdMgr->travMgr; 
  Ddi_Bddarray_t *pattern;
  Ddi_Bdd_t *start, **startPtr, *end, **pend, *initialC, *finalC;
  Tr_Tr_t *TrReverse;
  Ddi_Vararray_t *pi, *psv, *nsv;
  Ddi_Varset_t *inputVars;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->travMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return;
  }

  if (Trav_MgrReadNewi(travMgr) == NULL) {
    Pdtutil_Warning (1, "no frontier set found");
    return;
  }

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "maxDepth");
  maxDepth = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "initialConstr");
  if (strcmp(cmdOpt[optPos].value,"none")!=0) { 
    CmdMgrOperation (cmdMgr, cmdOpt[optPos].value, Pdtutil_MgrOpRead_c,
      &voidPointer, &returnFlag);

    if (returnFlag==Pdtutil_MgrRetNone_c) {
      Pdtutil_Warning (1, "goal BDD is empty.");
      return;
    }
    initialC = (Ddi_Bdd_t *) voidPointer;
  } else {
    initialC = NULL;
  }

  optPos = LocateOption (cmdOpt, "finalConstr");
  if (strcmp(cmdOpt[optPos].value,"none")!=0) { 
    CmdMgrOperation (cmdMgr, cmdOpt[optPos].value, Pdtutil_MgrOpRead_c,
      &voidPointer, &returnFlag);

    if (returnFlag==Pdtutil_MgrRetNone_c) {
      Pdtutil_Warning (1, "goal BDD is empty.");
      return;
    }
    finalC = (Ddi_Bdd_t *) voidPointer;
  } else {
    finalC = NULL;
  }

  optPos = LocateOption (cmdOpt, "startPoint");
  if (strcmp(cmdOpt[optPos].value,"none")!=0) { 
    startPtr = &start;
  } else {
    startPtr = NULL;
  }

  optPos = LocateOption (cmdOpt, "endPoint");
  if (strcmp(cmdOpt[optPos].value,"none")!=0) { 
    pend = &end;
  } else {
    pend = NULL;
  }

  optPos = LocateOption (cmdOpt, "verbosity");
  verbosity = Pdtutil_VerbosityString2Enum (cmdOpt[optPos].value);
  if (verbosity == Pdtutil_VerbLevelAppMax_c) {
    verbosityFlag = 0;
  } else {
    verbosityFlag = 1;
    verbositySave = Trav_MgrReadVerbosity (cmdMgr->travMgr);
    Trav_MgrSetVerbosity (cmdMgr->travMgr, verbosity);
  }

  /*----------------------- Call proper routine -----------------------------*/

  psv = Trav_MgrReadPS (travMgr);
  nsv = Trav_MgrReadNS (travMgr);
  pi = Trav_MgrReadI (travMgr);

  /*
   *  Here the TrReverse is computed to respect to the one in
   *  travMgr (that can already be a TrReverse if backward analysis
   *  has been performed; the idea is to obvioulsy use for the
   *  mismatch analysis an opposite direction to the one used for
   *  traversal
   */

  TrReverse = Tr_TrReverse(Trav_MgrReadTr(travMgr));

/*@@@ next line updated */
  inputVars = Ddi_VarsetMakeFromArray(pi);

  optPos = LocateOption (cmdOpt, "method");
  if (strcmp(cmdOpt[optPos].value,"forall")==0) { 
    pattern = Trav_UnivAlignPat (travMgr,TrReverse,
      finalC, startPtr,Trav_MgrReadNewi(travMgr),psv,nsv,inputVars,maxDepth);
    if (startPtr!=NULL) {
      *startPtr = Ddi_BddDup(Ddi_MgrReadOne(Trav_MgrReadDdiMgrR(travMgr)));
    }
  }
  else { /* find one counterexample */
    pattern = Trav_MismatchPat (travMgr,TrReverse,
      initialC,finalC,startPtr,pend,Trav_MgrReadNewi(travMgr),psv,nsv,inputVars);
  }

  Tr_TrFree(TrReverse);
  Ddi_Free(inputVars);

  if (pattern == NULL) {
/*@@@ next line updated */
/*@@@ Manual changes (DDI mgr) required for ArrayAlloc */
    pattern = Ddi_BddarrayAlloc(Trav_MgrReadDdiMgrR(travMgr),0);
  }
  voidPointer = (void *) pattern;

  optPos = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optPos].value, Pdtutil_MgrOpArraySet_c,
    &voidPointer, &returnFlag);

  Ddi_Free(pattern);
 
  optPos = LocateOption (cmdOpt, "startPoint");
  if (strcmp(cmdOpt[optPos].value,"none")!=0) { 
    Pdtutil_Assert(startPtr!=NULL,"NULL pointer to start set for mismatch\n");
    Pdtutil_Assert(start!=NULL,"NULL start set for mismatch\n");

    voidPointer = (void *) start;

    CmdMgrOperation (cmdMgr, cmdOpt[optPos].value, Pdtutil_MgrOpBddSet_c,
      &voidPointer, &returnFlag);

    Ddi_Free(start);
  }

  optPos = LocateOption (cmdOpt, "endPoint");
  if (strcmp(cmdOpt[optPos].value,"none")!=0) { 
    Pdtutil_Assert(pend!=NULL,"NULL pointer to end set for mismatch\n");
    Pdtutil_Assert(end!=NULL,"NULL end set for mismatch\n");

    voidPointer = (void *) end;

    CmdMgrOperation (cmdMgr, cmdOpt[optPos].value, Pdtutil_MgrOpBddSet_c,
      &voidPointer, &returnFlag);

    Ddi_Free(end);
  }

  return;
}

/**Function********************************************************************

  Synopsis    [Reads variable order]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdOrdRead (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr    /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  char *fileName;
  Ddi_Mgr_t *dd; 
  int optPos, paramPos;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;
  Pdtutil_VariableOrderFormat_e ordFileFormat;

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "ordFormat");
  ordFileFormat = Pdtutil_VariableOrderFormatString2Enum
    (cmdOpt[optPos].value);

  paramPos = LocateParam (cmdParam, "destination");
  Pdtutil_Assert (paramPos>=0, "No destination for read ord");

  CmdMgrOperation (cmdMgr, cmdParam[paramPos].value, Pdtutil_MgrOpMgrRead_c, 
    &voidPointer,&returnFlag);

  if (returnFlag != Pdtutil_MgrRetDdMgr_c) {
    Pdtutil_Assert (1, "Error looking for ddi manager.");
    return;
  }
  dd = (Ddi_Mgr_t *) voidPointer;

  paramPos = LocateParam (cmdParam, "source");
  Pdtutil_Assert (paramPos>=0, "no file for read ord");

  fileName = Pdtutil_StrDup (cmdParam[paramPos].value);

  Ddi_MgrReadOrdNamesAuxids (dd, fileName, NULL, ordFileFormat);

  Pdtutil_Free (fileName);

  return;
}

/**Function********************************************************************

  Synopsis    [Writes variable order]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdOrdWrite (
  FILE **fin               /* input stream */,
  Cmd_Mgr_t *cmdMgr        /* traverse manager */,
  CmdOpt_t cmdOpt[]        /* command options */,
  CmdParam_t cmdParam[]    /* command parameters */
  )
{
  char *fileName;
  Ddi_Mgr_t *dd; 
  int paramPos, optPos;
  void *voidPointer;
  Pdtutil_VariableOrderFormat_e ordFileFormat;
  Pdtutil_MgrRet_t returnFlag;

  /*---------------------- Read Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "ordFormat");
  ordFileFormat = Pdtutil_VariableOrderFormatString2Enum
    (cmdOpt[optPos].value);

  paramPos = LocateParam (cmdParam, "source");
  Pdtutil_Assert (paramPos>=0, "No source for write ord.");

  CmdMgrOperation (cmdMgr, cmdParam[paramPos].value, Pdtutil_MgrOpMgrRead_c, 
    &voidPointer,&returnFlag);

  if (returnFlag != Pdtutil_MgrRetDdMgr_c) {
    Pdtutil_Assert (1, "Error looking for dd manager.");
    return;
  }
  dd = (Ddi_Mgr_t *) voidPointer;

  paramPos = LocateParam (cmdParam, "destination");
  Pdtutil_Assert (paramPos>=0, "No file for read ord");

  fileName = Pdtutil_StrDup (cmdParam[paramPos].value);

  Ddi_MgrOrdWrite (dd, fileName, NULL, ordFileFormat);

  Pdtutil_Free (fileName);

  return;
}

/**Function********************************************************************

  Synopsis    [Extracts a Transition Relation Cluster]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTrClusterExtract(
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr    /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int paramPos, index;
  Ddi_Bdd_t *tmp;

  /*-------------------- Read Options and Parameters ------------------------*/

  paramPos = LocateParam (cmdParam, "index");
  index = atoi(cmdParam[paramPos].value);

  /*------------------ Extract/Insert TR clusters --------------------------*/
  
  tmp = Ddi_BddPartExtract(Fsm_MgrReadTrBDD(cmdMgr->fsmMgr),index);
  Ddi_Free(tmp);
  
  Pdtutil_Warning (1, "Cluster Insertion NOT YET supported.");

  return;
}

/**Function********************************************************************

  Synopsis    [Removes Lambda Latches]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
CmdLambdaLatchRemove (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr    /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  if(cmdMgr->trMgr == NULL) {
    fprintf(stderr, "Command aborted!\nNo TR manager.\nPlease build one.\n");
    return;
  }

  Tr_RemoveLambdaLatches(Tr_MgrReadTr(cmdMgr->trMgr));

  return;
}

/**Function********************************************************************

  Synopsis    [Builds a clustered Transition Relation]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTrCluster (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  Tr_Tr_t *tr;
  int optPos, threshold, saveThreshold, mono, keepPi;
  Tr_Sort_e sortMethod;

  /*------------ Check for the Correctness of the Command Sequence ----------*/

  if (cmdMgr->trMgr == NULL) {
    Pdtutil_Warning (1, "Command Out of Sequence.");
    return;
  }

  /*-------------------- Get Options and Parameters --------------------*/


  optPos = LocateOption (cmdOpt, "keepPi");
  keepPi = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "mono");
  mono = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "sort");
  sortMethod = Tr_TrSortString2Enum (cmdOpt[optPos].value);
  Tr_MgrSetSortMethod (cmdMgr->trMgr, sortMethod);

  optPos = LocateOption (cmdOpt, "threshold");
  threshold = atoi (cmdOpt[optPos].value);
  saveThreshold = Tr_MgrReadClustThreshold (cmdMgr->trMgr);
  if (threshold != (-1)) {
    Tr_MgrSetClustThreshold (cmdMgr->trMgr,threshold);
  }
  
  /*--------------- Build Clustered Transition Relation ----------------*/

  tr = Tr_MgrReadTr (cmdMgr->trMgr);
  if (tr == NULL) {
    Pdtutil_Warning (1, "Clustering a NULL TR.");
    return;
  }

  Tr_TrSortIwls95(tr);

  Tr_MgrSetClustSmoothPi(cmdMgr->trMgr,!keepPi);
  Tr_TrSetClustered(tr);
  if (mono) {
    Tr_TrSetMono(tr);
  } 

  Tr_MgrSetClustThreshold (cmdMgr->trMgr,saveThreshold);

  return;
}

/**Function********************************************************************

  Synopsis    [Partitions the Transition Relation]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTrPartition(
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int optPos, threshold, toggle=0;
  char *varname;

  /*--------------- Build Disjoint Transition Relation ----------------*/
  optPos = LocateOption (cmdOpt, "forcetoggle");
  toggle = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "threshold");
  threshold = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "var");
  varname = cmdOpt[optPos].value;
  if (strcmp(varname,"NULL")==0) {
    if (Tr_MgrReadPartThTr (cmdMgr->trMgr) >=0 ) {
      Tr_Tr_t *newTr;
      Ddi_Bdd_t *newTrBdd;

      newTrBdd = Part_PartitionDisjSet (
            Tr_TrBdd(Trav_MgrReadTr (cmdMgr->travMgr)),
            NULL,
            Tr_MgrReadPartitionMethod (cmdMgr->trMgr),
            Tr_MgrReadPartThTr (cmdMgr->trMgr),
            Tr_MgrReadVerbosity (cmdMgr->trMgr));
      newTr = Tr_TrMakeFromRel(cmdMgr->trMgr,newTrBdd);
      Trav_MgrSetTr (cmdMgr->travMgr,newTr);
      Ddi_Free(newTrBdd);
      Tr_TrFree(newTr);
    }
  } else {
    Trav_TrPartition(cmdMgr->travMgr,varname,toggle);
  }

  return;
}


/**Function********************************************************************

  Synopsis    [Computes the transitive closure]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdTrClosure(
  FILE **fin              /* Input stream */,
  Cmd_Mgr_t *cmdMgr       /* Command  manager */,
  CmdOpt_t cmdOpt[]       /* Command options */,
  CmdParam_t cmdParam[]   /* Command parameters */
  )
{
  Tr_Tr_t *tr;
  int optPos, maxiter, squaring;

  /*-------------------- Get Options and Parameters --------------------*/

  optPos = LocateOption (cmdOpt, "linear");
  maxiter = atoi (cmdOpt[optPos].value);
  optPos = LocateOption (cmdOpt, "linear");
  maxiter = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "squaring");
  squaring = atoi (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "iter");
  maxiter = atoi (cmdOpt[optPos].value);

  Tr_MgrSetMaxIter (cmdMgr->trMgr, maxiter);

  /*--------------- Build Closure of a Transition Relation ----------------*/

  tr = Tr_MgrReadTr(cmdMgr->trMgr);
  if (tr == NULL) {
    Pdtutil_Warning (1, "Closing a NULL TR");
    return;
  }

  if (squaring) {
    Tr_TransClosure(tr);
  } else {
    Pdtutil_Warning (1, "Linear closure NOT YET supported.");
  }

  return;
}


/**Function********************************************************************

  Synopsis    [Prints statistics]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdStatsPrint(
  FILE **fin                  /* Input Stream */,
  Cmd_Mgr_t *cmdMgr           /* Command Manager */,
  CmdOpt_t cmdOpt[]           /* Command Options */,
  CmdParam_t cmdParam[]       /* Command Parameters */
  )
{
  int i;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*--------------------------- Retrieve Destination ------------------------*/

  i = LocateParam (cmdParam, "topic");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpStats_c,
    &voidPointer, &returnFlag);

  return;
}


/**Function********************************************************************

  Synopsis    [Make partitions starting from a Bdd]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddPartMake (
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Command manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  Ddi_Bdd_t *f, *dest;
  Part_Method_e partitionMethod;
  int i, optPos, threshold;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;
  Pdtutil_VerbLevel_e verbosity;

  /*----------------------- Get Options and Parameters ----------------------*/

  optPos = LocateOption (cmdOpt, "verbosity");
  verbosity = Pdtutil_VerbosityString2Enum (cmdOpt[optPos].value);
  /* Get verbosity from cmdMgr */
  if (verbosity==Pdtutil_VerbLevelNone_c) {
    verbosity = Cmd_MgrReadVerbosity (cmdMgr);
  }

  optPos = LocateOption (cmdOpt, "method");
  partitionMethod = Part_MethodString2Enum (cmdOpt[optPos].value);

  optPos = LocateOption (cmdOpt, "threshold");
  threshold = atoi (cmdOpt[optPos].value);

  /*--------------------- Set the BDD to partition ---------------------*/

  i = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddRead_c,
    &voidPointer, &returnFlag);
  f = (Ddi_Bdd_t *) voidPointer;

  /*----------------------- Execute the Command ------------------------*/

  dest = Part_PartitionSetInterface (f, NULL, partitionMethod, threshold,
    verbosity); 
 
  /*----------------------- Set the resulting BDD ----------------------*/

  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddSet_c,
    (void *) &dest, &returnFlag);

  return;
}

/**Function********************************************************************

  Synopsis    [Adds a partition to a Bdd]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddPartAdd(
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Command manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  Ddi_Bdd_t *src, *dest;
  int index, code, i;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  Pdtutil_Warning (1, "Command disabled in pdtrav-2.0.");
#if 0

  /*------------------------------ Retrieve Source --------------------------*/

  i = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddRead_c,
    &voidPointer, &returnFlag);
  src = (Ddi_Bdd_t *) voidPointer;

  if (src == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[i].value);
    return;
  } 

  /*--------------------------- Retrieve Destination ------------------------*/

  i = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddRead_c,
    &voidPointer, &returnFlag);

  dest = (Ddi_Bdd_t *) voidPointer;
  if (dest == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[i].value);
    return;
  }

  /*------------------------------  Retrieve Options ------------------------*/

  /* Retrieves index*/
  i = LocateOption (cmdOpt, "index");
  index = atoi (cmdOpt[i].value);

  /* Retrieves Conj/Disj flag */
  /* checks only if conj flag is on or off*/
  i = LocateOption (cmdOpt, "conjunction");

  if( atoi (cmdOpt[i].value)) {
    code = DDI_CPART_BOOLF;
  } else {
    code = DDI_DPART_BOOLF;
  }
  
  /*----------------------- Execute the Command ------------------------*/

  if(dest == NULL)
    dest = Ddi_BddMakeFromCU (cmdMgr->dd, NULL);

  if (code == DDI_CPART_BOOLF) {
/*@@@ next line updated */
/*@@@ Manual changes required by partitioned operations*/
    dest =Ddi_BddAddIthCpart (dest, index, src);
  } else {
/*@@@ next line updated */
/*@@@ Manual changes required by partitioned operations*/
    dest = Ddi_BddAddIthDpart (dest, index, src); 
  }

  /*----------------------- Set the resulting BDD ----------------------*/

  i = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddSet_c,
    (void *) &dest, &returnFlag);
#endif
  return;
}

/**Function********************************************************************

  Synopsis    [Reads a partition]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddPartRead(
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Command manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  Ddi_Bdd_t *src, *dest, *tmp;
  int index, i;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------------- Retrieve Operand --------------------------*/

  /* Retrieve Source */
  i = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddRead_c,
    &voidPointer, &returnFlag);
  src = (Ddi_Bdd_t *) voidPointer;

  if (src == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[1].value);
    return;
  } 

  /* Retrieve Destination */
  i = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddRead_c,
    &voidPointer, &returnFlag);
  dest = (Ddi_Bdd_t *) voidPointer;

  /* Retrieve Index (default is 0) */
  i = LocateOption (cmdOpt, "index");
  index = atoi(cmdOpt[i].value);
 
  /*---------------------------- Do the Operation ---------------------------*/

  /* Get the Partition from Source */
  tmp = Ddi_BddPartRead (src, index);

  /* Set Destination (and free old one) */
  if (tmp != NULL) {
    Ddi_Free (dest);
    dest = tmp;    
  }

  /*---------------------------- Set Destination ----------------------------*/

  i = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddRead_c,
    &voidPointer, &returnFlag);

  return;
}

/**Function********************************************************************

  Synopsis    [Deletes a partition]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddPartDelete(
  FILE **fin                  /* Input stream */,
  Cmd_Mgr_t *cmdMgr           /* Command manager */,
  CmdOpt_t cmdOpt[]           /* Command options */,
  CmdParam_t cmdParam[]       /* Command parameters */
  )
{
  Ddi_Bdd_t *op, *tmp;
  int i, index;
  void *voidPointer;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------------- Retrieve Operand --------------------------*/

  i = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[i].value, Pdtutil_MgrOpBddRead_c,
    &voidPointer, &returnFlag); 

  op = (Ddi_Bdd_t *) voidPointer;
  if (op == NULL) {
    fprintf (stderr, "Error: %s is empty!\n", cmdParam[i].value);
    return;
  } 

  /* Retrieve Index  (default is 0) */
  i = LocateParam (cmdParam, "index");
  index = atoi (cmdOpt[i].value);
 
  /*----------------------------- Do the Operation --------------------------*/

  if (!Ddi_BddIsMono(op)) {
    tmp=Ddi_BddPartExtract(op, index);
    Ddi_Free(tmp);
  } else {
    Pdtutil_Warning (1, "Parameter is a monolithic BDD.");
  }

  return;
}

/**Function********************************************************************

  Synopsis    [Copies a Bdd]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
CmdBddCopy (
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* command manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  int optPos, optParam, arrayFlag;
  void *voidPointer;
  Pdtutil_MgrOp_t opRead, opSet;
  Pdtutil_MgrRet_t returnFlag;

  /*----------------------------- Retrieve Source ---------------------------*/

  optPos = LocateOption (cmdOpt, "array");
  arrayFlag = atoi (cmdOpt[optPos].value);
  if (arrayFlag == 0) {
    opRead = Pdtutil_MgrOpBddRead_c;
    opSet = Pdtutil_MgrOpBddSet_c;
  } else {
    Pdtutil_Warning (1, "Operation on Array NOT supported.");
    return;

    opRead = Pdtutil_MgrOpArrayRead_c;
    opSet = Pdtutil_MgrOpArraySet_c;
  }

  optParam = LocateParam (cmdParam, "source");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opRead, &voidPointer,
    &returnFlag);

  if (returnFlag==Pdtutil_MgrRetNone_c) {
    Pdtutil_Warning (1, "The operand is empty.");
    return;
  } 

  /*----------------------------- Set Destination ---------------------------*/

  optParam = LocateParam (cmdParam, "destination");
  CmdMgrOperation (cmdMgr, cmdParam[optParam].value, opSet, &voidPointer,
    &returnFlag);

  return;
}
  
/**Function********************************************************************

  Synopsis    [searches an option in the array of options]

  Description [Returns the index of an option in the array of options, given
    the tag associated to it. The function returns -1 for a missing
    tag.]

  SideEffects []

  SeeAlso     []

******************************************************************************/

static int
LocateOption(
  CmdOpt_t *cmdOpt     /* Command Options */,
  char *tag            /* Searched Tag */
  )
{
  int i;
  
  for (i=0; (strlen(cmdOpt[i].tag)>0); i++) {
    if (strcmp(cmdOpt[i].tag,tag)==0)
      return (i);
  }
  
  return (-1);
}
  
/**Function********************************************************************

  Synopsis    [Searches a parameter in the array of parameters]

  Description [Returns the index of an option in the array of parameterss,
    given the tag associated to it. The function returns -1 for a missing
    tag.]

  SideEffects []

  SeeAlso     []

******************************************************************************/

static int
LocateParam(
  CmdParam_t *cmdParam       /* command parameterss */,
  char *tag                  /* searched tag */
)
{
  int i;
  
  for (i=0; (strlen(cmdParam[i].tag)>0); i++) {
    if (strcmp(cmdParam[i].tag,tag)==0)
      return (i);
  }
  
  return (-1);
}
  
/**Function********************************************************************

  Synopsis    [Read, Set or Print Statistics for "something"]

  Description []

  SideEffects []

  SeeAlso     [CmdRegOperation, Fsm_MgrOperation, Trav_MgrOperation,
    Tr_MgrOperation]

******************************************************************************/

static int
CmdMgrOperation (
  Cmd_Mgr_t *cmdMgr,
  char *string                    /* String to parse */,
  Pdtutil_MgrOp_t operationFlag   /* Operation Flag */,
  void **voidPointer              /* Generic Pointer */,
  Pdtutil_MgrRet_t *returnFlagP   /* Type of the Pointer Returned */
  )
{
  char *stringFirst, *stringSecond;

  /*-------------------- Set Return Type as None (Default) ------------------*/

  /* In the sequel I change it only IFF necessary */
  *returnFlagP = Pdtutil_MgrRetNone_c;

  /*----------------------- Take Main and Secondary Name --------------------*/

  Pdtutil_ReadSubstring (string, &stringFirst, &stringSecond);

  /*
   *  The Function should be a Sequence of if ... `then` ... else
   *  but it is implemented as a sequence of if for
   *  simplicity
   */

  /*--------------------------- Print All Options ---------------------------*/

  if (stringFirst == NULL) {
    switch (operationFlag) {
      case Pdtutil_MgrOpOptionShow_c:
        Ddi_MgrOperation (&(cmdMgr->dd), stringSecond, operationFlag,
          voidPointer, returnFlagP);
        Fsm_MgrOperation (cmdMgr->fsmMgr, stringSecond, operationFlag,
          voidPointer, returnFlagP);
        Tr_MgrOperation (cmdMgr->trMgr, stringSecond, operationFlag,
          voidPointer, returnFlagP);
        Trav_MgrOperation (cmdMgr->travMgr, stringSecond, operationFlag,
          voidPointer, returnFlagP);
        CmdRegOperation (cmdMgr, string, operationFlag, voidPointer,
          returnFlagP);
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on CMD Manager");
        break;
    }
    return (0);
  }

  /*----------------------- Package is the CMD Manager ----------------------*/
  
  if ( strcmp (stringFirst, "helpName")==0 ) {
   switch (operationFlag) {
      case Pdtutil_MgrOpOptionSet_c:
        Pdtutil_Free (cmdMgr->settings.helpName);
        cmdMgr->settings.helpName = Pdtutil_StrDup (*voidPointer);
        break;
      default:
        Pdtutil_Warning (1, "Operation Non Allowed on CMD Manager");
        break;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*------------------------ Package is the DD Manager ----------------------*/
  
  if ( strcmp (stringFirst, "ddm")==0 ) {
    Ddi_MgrOperation (&(cmdMgr->dd), stringSecond, operationFlag,
      voidPointer, returnFlagP);
    if (*returnFlagP == Pdtutil_MgrOpMgrDelete_c) {
      cmdMgr->dd = NULL;
    }

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*----------------------------- Package is FSM ----------------------------*/
  
  if ( strcmp (stringFirst, "fsm")==0 ) {
    Fsm_MgrOperation (cmdMgr->fsmMgr, stringSecond, operationFlag,
      voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }

  /*--------------------------- Package is TR -------------------------------*/
  
  if ( strcmp (stringFirst, "tr")==0 ) {
    Tr_MgrOperation (cmdMgr->trMgr, stringSecond, operationFlag,
      voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*--------------------------- Package is TRAV -----------------------------*/
  
  if ( strcmp (stringFirst, "trav")==0 ) {
    Trav_MgrOperation (cmdMgr->travMgr, stringSecond, operationFlag,
      voidPointer, returnFlagP);

    Pdtutil_Free (stringFirst);
    Pdtutil_Free (stringSecond);
    return (0);
  }
  
  /*-------------------------- Source Is a Register -------------------------*/

  CmdRegOperation (cmdMgr, string, operationFlag, voidPointer,
    returnFlagP);
  Pdtutil_Free (stringFirst);
  Pdtutil_Free (stringSecond);
  return (0);
}

/**Function********************************************************************

  Synopsis    [Read, Set or Print Statistics for "something"]

  Description []

  SideEffects []

  SeeAlso     [CmdRegOperation, Fsm_MgrOperation, Trav_MgrOperation,
    Tr_MgrOperation]

******************************************************************************/

static int
CmdRegOperation (
  Cmd_Mgr_t *cmdMgr,
  char *string                    /* String to parse */,
  Pdtutil_MgrOp_t operationFlag   /* Operation Flag */,
  void **voidPointer              /* Generic Pointer */,
  Pdtutil_MgrRet_t *returnFlagP   /* Type of the Pointer Returned */
  )
{
  int i, position;

  /* 
   *  Do the Operation
   */

  switch (operationFlag) {
    case Pdtutil_MgrOpOptionShow_c:
      position = RegFullFind (cmdMgr, string);
      if (position!=(-1)) {
        /* Show One */
        fprintf (stdout, "RegisterName %s\n", cmdMgr->reg[position].tag);
      } else {
        /* Show All */
        for (i=0; i<REGISTER_NUMBER_OF; i++) {
          if (cmdMgr->reg[i].tag != NULL) {
            fprintf (stdout, "RegisterName %s\n",
              cmdMgr->reg[i].tag);
          }     
        }
      }
      break;

    case Pdtutil_MgrOpDelete_c:
      position = RegFullFind (cmdMgr, string);
      if (position!=(-1)) {
        Pdtutil_Free (cmdMgr->reg[position].tag);
        cmdMgr->reg[position].bdd = NULL;
        cmdMgr->reg[position].bddArray = NULL;
      }
      break;

    case Pdtutil_MgrOpBddDelete_c:
    case Pdtutil_MgrOpArrayDelete_c:
      position = RegFullFind (cmdMgr, string);
      if (position!=(-1)) {
        Pdtutil_Free (cmdMgr->reg[position].tag);
        cmdMgr->reg[position].bdd = NULL;
        cmdMgr->reg[position].bddArray = NULL;
      }
      break;

    case Pdtutil_MgrOpRead_c:
    case Pdtutil_MgrOpBddRead_c:
    case Pdtutil_MgrOpArrayRead_c:
      position = RegFullFind (cmdMgr, string);
      if (position != (-1)) {
        if (cmdMgr->reg[position].bdd != NULL) {
          *voidPointer = (void *) cmdMgr->reg[position].bdd;
          *returnFlagP = Pdtutil_MgrRetBdd_c;
          break;
        }
        if (cmdMgr->reg[position].bddArray != NULL) {
          *voidPointer = (void *) cmdMgr->reg[position].bddArray;
          *returnFlagP = Pdtutil_MgrRetBddarray_c;
          break;
        }
      }
      *voidPointer = NULL;
      *returnFlagP = Pdtutil_MgrRetNone_c;
      break;

    case Pdtutil_MgrOpBddSet_c:
      position = RegFullFind (cmdMgr, string);
      if (position == (-1)) {
        position = RegEmptyFind (cmdMgr);
      }
      if (position != (-1)) {
        cmdMgr->reg[position].tag = Pdtutil_StrDup (string);
        cmdMgr->reg[position].bdd = Ddi_BddDup((Ddi_Bdd_t *) *voidPointer);
      } else {
        Pdtutil_Warning (1, "No More Free Register.");
      }
      break;

    case Pdtutil_MgrOpArraySet_c:
      position = RegFullFind (cmdMgr, string);
      if (position == (-1)) {
        position = RegEmptyFind (cmdMgr);
      }
      if (position != (-1)) {
        cmdMgr->reg[position].tag = Pdtutil_StrDup (string);
        cmdMgr->reg[position].bddArray = 
          Ddi_BddarrayDup((Ddi_Bddarray_t *) *voidPointer);
      } else {
        Pdtutil_Warning (1, "No More Free Register.");
      }
      break;

    case Pdtutil_MgrOpStats_c:
      position = RegFullFind (cmdMgr, string);
      if (position != (-1)) {
        if (cmdMgr->reg[position].bdd!=NULL) {
          *voidPointer = (void *) cmdMgr->reg[position].bdd;
/*@@@ next line updated */
          Ddi_BddPrintStats ((Ddi_Bdd_t *) *voidPointer,stdout);
        } else {
          *voidPointer = (void *) cmdMgr->reg[position].bddArray;
          fprintf (stdout, "Size: %d\n",
            Ddi_BddarrayNum (cmdMgr->reg[position].bddArray));
        }
      }
      break;

    default:
      Pdtutil_Warning (1, "Operation Non Allowed on the Manager");
      break;
  }

  return (0);
}



/**Function*******************************************************************

  Synopsis    [Find a Full Register]

  Description [It returns the index of the register found or
    -1 if there is no match]

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

static int
RegFullFind (
  Cmd_Mgr_t *cmdMgr,
  char *string          /* String to parse */
  )
{
  int i;

  if (string==NULL) {
    return (-1);
  }

  for (i=0; i<REGISTER_NUMBER_OF; i++) {
    if (cmdMgr->reg[i].tag!=NULL && strcmp (cmdMgr->reg[i].tag, string) == 0) {
      return (i);
    }
  }

  return (-1);
} 



/**Function*******************************************************************

  Synopsis    [Find an Empty Register]

  Description [It returns the index of the register found or
    -1 if there is no match]

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

static int
RegEmptyFind (
  Cmd_Mgr_t *cmdMgr
  )
{
  int i;

  for (i=0; i<REGISTER_NUMBER_OF; i++) {
    if (cmdMgr->reg[i].tag == NULL) {
      return (i);
    }
  } 

  return (-1);
}


 
/**Function*******************************************************************

  Synopsis    [Compares two command definition structs]

  Description [The function compares two command definitions. 
    Returns string comparison between command names.
    ]

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/

static int
CommandCompare(
  const void *s1,
  const void *s2
  )
{
  struct CmdTable_s *c1 = ((struct CmdTable_s *)s1);
  struct CmdTable_s *c2 = ((struct CmdTable_s *)s2);

  return (strcmp(c1->name,c2->name));
}

/**Function********************************************************************

  Synopsis    [Loads a script]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void
Pdtutil_MgrOpenScript(
  FILE **fin                  /* input stream */,
  Cmd_Mgr_t *cmdMgr           /* traverse manager */,
  CmdOpt_t cmdOpt[]           /* command options */,
  CmdParam_t cmdParam[]       /* command parameters */
  )
{
  FILE *script;
  int flagFile;
  char *fileName = cmdParam[0].value;

  script = Pdtutil_FileOpen (NULL, fileName, "r", &flagFile);

  if (script == NULL ) {
    fprintf (stdout, "Opening %s failed!\n", fileName);
  } else {
    *fin = script;
  }

  return;
}
