/**CFile***********************************************************************

  FileName    [fsmLoad.c]

  PackageName [fsm]

  Synopsis    [Functions to read BDD based description of FSMs]

  Description [External procedures included in this module:
    <ul>
    <li> Fsm_MgrLoad () 
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

#include "fsmInt.h"

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

static int readFsm(FILE *fp, Fsm_Mgr_t *fsmMgr, char *fileOrdName, int bddFlag, Pdtutil_VariableOrderFormat_e ordFileFormat);
static void readFsmSize(FILE *fp, Fsm_Mgr_t * fsmMgr, char * word);
static void readFsmOrd(FILE *fp, Fsm_Mgr_t *fsmMgr, char *fileOrdName, char *word, Pdtutil_VariableOrderFormat_e ordFileFormat);
static void readFsmName(FILE *fp, Fsm_Mgr_t * fsmMgr, char * word);
static void readFsmIndex(FILE *fp, Fsm_Mgr_t * fsmMgr, char * word);
static void readFsmDelta(FILE * fp, Fsm_Mgr_t * fsmMgr, char * word, int bddFlag);
static void readFsmLambda(FILE * fp, Fsm_Mgr_t * fsmMgr, char * word, int bddFlag);
static void readFsmTransRel(FILE * fp, Fsm_Mgr_t * fsmMgr, char * word, int bddFlag);
static void readFsmFrom(FILE * fp, Fsm_Mgr_t * fsmMgr, char * word, int bddFlag);
static void readFsmReached(FILE * fp, Fsm_Mgr_t * fsmMgr, char * word, int bddFlag);
static void readFsmInitState(FILE * fp, Fsm_Mgr_t * fsmMgr, char * word, int bddFlag);
static void readOrdFile(FILE *fp, Fsm_Mgr_t *fsmMgr, Pdtutil_VariableOrderFormat_e ordFileFormat);
static void readBddFile(FILE * fp, Fsm_Mgr_t * fsmMgr, Ddi_Mgr_t * dd, char * word, char **pName, Ddi_Bdd_t **pBdd, Ddi_Bddarray_t **pBddarray, int bddFlag);
static int get_number(FILE *fp, char *word);
static char ** get_string_array(FILE *fp, char *word, int size);
static int * get_integer_array(FILE *fp, char *word, int size);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []
  
  Description        []
  
  SideEffects        []

  SeeAlso            [get_name_file,readFsm]

******************************************************************************/

int
Fsm_MgrLoad (
  Fsm_Mgr_t **fsmMgrP     /* FSM Pointer */,
  Ddi_Mgr_t *dd           /* Main DD manager */,
  char *fileFsmName       /* Input file name */,
  char *fileOrdName       /* ORD File Name */,
  int bddFlag             /* 0=Do non load BDD (default), 1=Load BDD */,
  Pdtutil_VariableOrderFormat_e ordFileFormat
  )
{
  Fsm_Mgr_t *fsmMgr;
  FILE *fp = NULL;
  int i, flagFile;
  Ddi_Vararray_t *vars;
  Ddi_Var_t *v;
  int *auxids, flagError;
  char **names;
  char *nameext;
  long fsmTime;

  fsmTime = util_cpu_time();

  /*-------------------------- Check For FSM File  --------------------------*/

  nameext = Pdtutil_FileName (fileFsmName, "", "fsm", 0);
  fp = Pdtutil_FileOpen (fp, nameext, "r", &flagFile);
  if (fp==NULL) {
    Pdtutil_Warning (1, "NULL Finite State Machine Pointer.");
    return (1);
  }

  /*------------------------ Check For FSM Structure ------------------------*/

  fsmMgr = *fsmMgrP;
  if (fsmMgr == NULL) {
    Pdtutil_Warning (1, "Allocate a new FSM structure");
    fsmMgr = Fsm_MgrInit (NULL);
  }

  if (fsmMgr->settings.verbosity >= Pdtutil_VerbLevelDevMin_c) {
    fprintf (stdout, "(... loading %s ...)", fileFsmName) ;
    fflush (stdout);
  }

  Fsm_MgrSetDdManager (fsmMgr,  dd);

  flagError = readFsm (fp, fsmMgr, fileOrdName, bddFlag, ordFileFormat);

  /*
   *  return 1 = An Error Occurred
   */

  if (flagError == 1) {
    Fsm_MgrQuit (fsmMgr);
    return (1);
  }

  Pdtutil_FileClose (fp, &flagFile);
  
  auxids = Fsm_MgrReadIndexI(fsmMgr);
  names = Fsm_MgrReadNameI(fsmMgr);

  vars = Ddi_VararrayAlloc (dd, 0);
  if (Fsm_MgrReadNI (fsmMgr) > 0) {
    for (i=0; i<Fsm_MgrReadNI(fsmMgr); i++) {
      if (names != NULL)
        v = Ddi_VarFromName(dd,names[i]);
      else
        v = Ddi_VarFromAuxid(dd,auxids[i]);
      Ddi_VararrayWrite(vars,i,v);
    }
  }
  Fsm_MgrSetVarI (fsmMgr, vars);
  Ddi_Free (vars);

  if (Fsm_MgrReadNL (fsmMgr) > 0) {
    vars = Ddi_VararrayAlloc(dd,0);
    auxids = Fsm_MgrReadIndexPS(fsmMgr);
    names = Fsm_MgrReadNamePS(fsmMgr);
    for (i=0; i<Fsm_MgrReadNL(fsmMgr); i++) {
      if (names != NULL)
        v = Ddi_VarFromName(dd,names[i]);
      else
        v = Ddi_VarFromAuxid(dd,auxids[i]);
      Ddi_VararrayWrite(vars,i,v);
    }
    Fsm_MgrSetVarPS (fsmMgr, vars);
    Ddi_Free (vars);

    vars = Ddi_VararrayAlloc (dd,0);
    auxids = Fsm_MgrReadIndexNS (fsmMgr);
    names = Fsm_MgrReadNameNS (fsmMgr);
    for (i=0; i<Fsm_MgrReadNL (fsmMgr); i++) {
      if (names != NULL)
        v = Ddi_VarFromName (dd, names[i]);
      else
        v = Ddi_VarFromAuxid(dd, auxids[i]);
      Ddi_VararrayWrite (vars, i, v);
    }
    Fsm_MgrSetVarNS (fsmMgr, vars);
    Ddi_Free (vars);
  } else {
    Fsm_MgrSetVarPS (fsmMgr, NULL);
    Fsm_MgrSetVarNS (fsmMgr, NULL);
  }

  /*
   *  return 0 = All OK
   */

  fsmTime = util_cpu_time() - fsmTime;
  fsmMgr->fsmTime += fsmTime;

  *fsmMgrP = fsmMgr;
  return (0);
}

/**Function********************************************************************

  Synopsis           [Reads variable order from a <em>.ord</em> file]
  
  Description        []
  
  SideEffects        []

  SeeAlso            []

******************************************************************************/

void
Fsm_MgrReadOrdFile (
  FILE *fp, 
  Fsm_Mgr_t *fsmMgr,
  Pdtutil_VariableOrderFormat_e ordFileFormat
  )
{
  readOrdFile (fp, fsmMgr, ordFileFormat);

  return;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Reads current section of the <em>.fsm</em> file.]

  SideEffects []

  SeeAlso     [readFsmSize,readFsmName,readFsmIndex,
    readFsmDelta,readFsmLambda,readFsmTransRel,readFsmInitState]

******************************************************************************/

static int
readFsm (
  FILE *fp              /* File */,
  Fsm_Mgr_t *fsmMgr     /* FSM Manager */,
  char *fileOrdName     /* File Name for the Order File */,
  int bddFlag,
  Pdtutil_VariableOrderFormat_e ordFileFormat
)
{
  char word[FSM_MAX_STR_LEN];
  Fsm_TokenType token;

  switch (get_token (fp, word)) {
    case KeyFSM_FSM :
      if (get_token (fp, word)==KeyFSM_STRING) {
        Fsm_MgrSetFileName (fsmMgr, word);
      }
      break;
    default :
      fprintf(stderr, "Error: Wrong keyword at begin of file\n");
      return (1);
  }

  while ((token=get_token(fp,word))!=KeyFSM_END) { 
    switch (token) {
      case KeyFSM_SIZE:
	readFsmSize (fp, fsmMgr, word);    
	break;
      case KeyFSM_ORD:
	readFsmOrd (fp, fsmMgr, fileOrdName, word, ordFileFormat);
	break;
      case KeyFSM_NAME:
	readFsmName (fp, fsmMgr, word);
	break;
      case KeyFSM_INDEX:
        readFsmIndex (fp, fsmMgr, word);
	break;
      case KeyFSM_DELTA:
        readFsmDelta (fp, fsmMgr, word, bddFlag);
	break;
      case KeyFSM_LAMBDA:
        readFsmLambda (fp, fsmMgr, word, bddFlag);
	break;
      case KeyFSM_INITSTATE:
        readFsmInitState (fp, fsmMgr, word, bddFlag);
	break;
      case KeyFSM_TRANS_REL:
        readFsmTransRel (fp, fsmMgr, word, bddFlag);
	break;
      case KeyFSM_FROM:
        readFsmFrom (fp, fsmMgr, word, bddFlag);
	break;
      case KeyFSM_REACHED:
        readFsmReached (fp, fsmMgr, word, bddFlag);
	break;
      default :
	fprintf (stderr, "Error : Wrong section in fsm file: %s\n", word);
	return (1);
      }
  }

  return (0);
}


/**Function********************************************************************

  Synopsis           [Reads <em>.size</em> section of the <em>.fsm</em> file.]

  Description        [Looks up ".size" section of the new ".fsm" file format 
    for input, output and latch number to be loaded in the FSM structure.]

  SideEffects        []

  SeeAlso            [number]

******************************************************************************/
static void 
readFsmSize (
FILE *fp                  /* file */,
Fsm_Mgr_t * fsmMgr           /* struttura FSM */,
char * word               /* parola corrente */
)
{
  Fsm_TokenType token;

  token = get_token (fp, word);

  while (token != KeyFSM_END) { 
    switch (token) {
      case KeyFSM_I :
        Fsm_MgrSetNI (fsmMgr,  get_number (fp, word)); 
        break;
      case KeyFSM_O :
        Fsm_MgrSetNO (fsmMgr,  get_number (fp, word)); 
        break;
      case KeyFSM_L :
        Fsm_MgrSetNL (fsmMgr,  get_number (fp, word)); 
        break;
      default :
        fprintf (stderr,
          "Error : Wrong keyword in .size segment -> %s\n", word);
    }

    /* Get Next Token */
    token = get_token (fp, word);
  }
}


/**Function********************************************************************

  Synopsis           [Reads <em>.ord</em> section of the <em>.fsm</em> file.]

  Description        []

  SideEffects        []

  SeeAlso            [get_string_array]

******************************************************************************/

static void 
readFsmOrd (
  FILE *fp                 /* File Pointer */,
  Fsm_Mgr_t *fsmMgr        /* FSM Manager */,
  char *fileOrdName,       /* File Name for the Order File */
  char *word               /* Current Word */,
  Pdtutil_VariableOrderFormat_e ordFileFormat
  )
{           
  FILE *filep = NULL;
  char *fileName;
  int i, flagFile;
  char **varnames;
  int *varauxids;
  Fsm_TokenType token;
  int nvars;
  Ddi_Mgr_t * dd=Fsm_MgrReadDdManager(fsmMgr);

  token = get_token (fp, word);

  while (token != KeyFSM_END) { 
    switch (token) {
      case KeyFSM_FILE_ORD:  
    	get_token (fp, word);
        if (fileOrdName == NULL) {
          fileName = Pdtutil_StrDup (word);
        } else {
          /* OverWrite Internal (of FSM structure) File Name */
          fileName = Pdtutil_StrDup (fileOrdName);
        }
        Fsm_MgrSetOrdFileName (fsmMgr, fileName);
        filep = Pdtutil_FileOpen (filep, fileName, "r", &flagFile) ;
        Pdtutil_Warning (filep==NULL, "Cannot open FSM file");
        readOrdFile (filep, fsmMgr, ordFileFormat);
        Pdtutil_FileClose (filep, &flagFile);
        break;
      default:
        fprintf (stderr,
          "Error : Wrong keyword (%s) .ord expected\n",word);
        break;
    }
    token = get_token (fp, word);
  }

  nvars = Fsm_MgrReadNI (fsmMgr) + 2 * Fsm_MgrReadNL (fsmMgr);
  varnames = Fsm_MgrReadVarnames (fsmMgr);
  varauxids = Fsm_MgrReadVarauxids (fsmMgr);
  if (varauxids == NULL) {
    fprintf (stdout, "not yet supported order without IDs\n");
    exit(1);
  }

  for (i=0; i<nvars; i++) {
    Ddi_VarAttachName (Ddi_IthVar (dd, i), varnames[i]);
    Ddi_VarAttachAuxid (Ddi_IthVar (dd, i), varauxids[i]);
  }

  return;
}


/**Function********************************************************************

  Synopsis           [Reads <em>.name</em> section of the <em>.fsm</em> file.]

  Description        [Reads ".name" section of the new ".fsm" file format
    and then loads the FSM structure by means of "get_string_array" 
    with input (".i"), output (".o"), latch input (".ps") and latch
    output (".ns") names in the circuit.]

  SideEffects        []

  SeeAlso            [get_string_array]

******************************************************************************/

static void 
readFsmName (
FILE *fp                  /* file */,
Fsm_Mgr_t * fsmMgr           /* struttura FSM */,
char * word               /* parola corrente */
)
{
  Fsm_TokenType token;

  while ((token=get_token (fp, word))!=KeyFSM_END) { 
    switch (token) {
    case KeyFSM_I :
      Fsm_MgrSetNameI (fsmMgr, get_string_array(fp, word,
        Fsm_MgrReadNI(fsmMgr)));
      break;
    case KeyFSM_O :
      Fsm_MgrSetNameO (fsmMgr, get_string_array(fp, word,
        Fsm_MgrReadNO(fsmMgr)));
      break;
    case KeyFSM_PS :
      Fsm_MgrSetNamePS (fsmMgr, get_string_array(fp, word,
        Fsm_MgrReadNL(fsmMgr)));
      break;
    case KeyFSM_NS :
      Fsm_MgrSetNameNS (fsmMgr, get_string_array(fp, word,
        Fsm_MgrReadNL(fsmMgr)));
      break;
    default :
      fprintf (stderr ,"Error : %s option in .name Not recognised\n", word);
      exit (1);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Reads <em>.index</em> section of the <em>.fsm</em> file.]

  Description        [Reads ".name" section of the new ".fsm" file format
    and then loads the FSM structure by means of "get_integer_array" 
    with input (".i"), output (".o"), latch input (".ps") and latch output
    (".ns") indexes in the circuit.]

  SideEffects        []

  SeeAlso            [get_integer_array]

******************************************************************************/

static void
readFsmIndex (
  FILE *fp            /* File */,
  Fsm_Mgr_t * fsmMgr  /* FSM Strutture */,
  char * word         /* Current Word */
  )
{
  Fsm_TokenType token;

  while ((token=get_token (fp, word))!=KeyFSM_END) { 
    switch (token) {
    case KeyFSM_I :
      Fsm_MgrSetIndexI (fsmMgr, get_integer_array(fp, word,
        Fsm_MgrReadNI(fsmMgr)));
      break;
    case KeyFSM_O :
      Fsm_MgrSetIndexO (fsmMgr, get_integer_array(fp, word,
        Fsm_MgrReadNO(fsmMgr)));
      break;
    case KeyFSM_PS :
      Fsm_MgrSetIndexPS (fsmMgr, get_integer_array(fp, word,
        Fsm_MgrReadNL(fsmMgr)));
      break;
    case KeyFSM_NS :
      Fsm_MgrSetIndexNS (fsmMgr, get_integer_array(fp, word,
        Fsm_MgrReadNL(fsmMgr)));
      break;
    default :
      fprintf (stderr, "Error : %s option in .index Not recognised\n",
	      word);
      exit (1);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Reads <em>.delta</em> section of the <em>.fsm</em> file.]

  Description        [Reads ".delta" section of the new ".fsm" file format
    and then loads the FSM structure with future state function BDDs.
    In order to do that, uses "Ddi_BddarrayLoad" procedure, 
    preceded by "Pdtutil_FileOpen" and followed by "Pdtutil_FileClose" in case
    BDD is specified with a fileName.]

  SideEffects        []

  SeeAlso            [Poli_bddRead_new]

******************************************************************************/

static void 
readFsmDelta (
  FILE * fp                 /* File */, 
  Fsm_Mgr_t * fsmMgr        /* FSM Manager */,
  char * word               /* Current Word */,
  int bddFlag
  )
{
  char *name;               /* delta file name */
  Ddi_Bddarray_t *delta;     /* array of bdds */

  readBddFile (fp, fsmMgr, Fsm_MgrReadDdManager (fsmMgr), word,
    &name, NULL, &delta, bddFlag);

  Fsm_MgrSetDeltaName (fsmMgr,  name);
  if (bddFlag == 1) {
    Fsm_MgrSetDeltaBDD (fsmMgr, delta);
    Ddi_Free (delta);
  } else {
    Fsm_MgrSetDeltaBDD (fsmMgr, NULL);
  }
}

/**Function********************************************************************

  Synopsis           [Reads <em>.lambda</em> section of the
    <em>.fsm</em> file.
    ]

  Description        [Reads ".lambda" section of the new ".fsm" file format 
    and then loads the FSM structure with output function BDD. 
    In order to do that, it uses "Ddi_BddarrayLoad" procedure, preceded by
    "Pdtutil_FileOpen" and followed by "Pdtutil_FileClose" in case
    BDD is specified with a fileName.
    ]

  SideEffects        []

  SeeAlso            [Poli_bddRead_new]

******************************************************************************/

static void 
readFsmLambda (
  FILE * fp                /* file */, 
  Fsm_Mgr_t * fsmMgr       /* struttura FSM */,
  char * word              /* parola corrente */,
  int bddFlag
)
{
  char *name;              /* lambda file name */
  Ddi_Bddarray_t *lambda;   /* array of bdds */

  readBddFile (fp, fsmMgr, Fsm_MgrReadDdManager(fsmMgr), word,
    &name, NULL, &lambda, bddFlag);

  Fsm_MgrSetLambdaName (fsmMgr, name);

  if (bddFlag == 1) {
    Fsm_MgrSetLambdaBDD (fsmMgr, lambda);
    Ddi_Free (lambda);
  } else {
    Fsm_MgrSetLambdaBDD (fsmMgr, NULL);
  }
}


/**Function********************************************************************

  Synopsis           [Reads <em>.tr</em> section of the <em>.fsm</em> file.]

  Description        [Reads ".tr" section of the new ".fsm" file format 
    and then loads the FSM structure with transition relation BDD. 
    In order to do that, it uses "Ddi_BddarrayLoad" procedure, preceded by
    "Pdtutil_FileOpen" and followed by "Pdtutil_FileClose" in case
    BDD is specified with a fileName.
    ]

  SideEffects        []

  SeeAlso            [Poli_bddRead_new]

******************************************************************************/

static void 
readFsmTransRel (
  FILE * fp                 /* file */, 
  Fsm_Mgr_t * fsmMgr        /* struttura FSM */,
  char * word               /* parola corrente */,
  int bddFlag
)
{
  char *name;               /* tr file name */
  Ddi_Bdd_t *tr;             /* bdd */

  readBddFile (fp, fsmMgr, Fsm_MgrReadDdManager(fsmMgr), word, &name,
    &tr, NULL, bddFlag);

  Fsm_MgrSetTrName (fsmMgr, name);
  if (bddFlag == 1) {
    Fsm_MgrSetTrBDD (fsmMgr, tr);
    Ddi_Free (tr);
  } else {
    Fsm_MgrSetTrBDD (fsmMgr, NULL);
  }
}

/**Function********************************************************************

  Synopsis           [Reads <em>.tr</em> section of the <em>.fsm</em> file.]

  Description        [Reads ".from" section.]

  SideEffects        []

  SeeAlso            [Poli_bddRead_new]

******************************************************************************/

static void 
readFsmFrom (
  FILE * fp                 /* file */, 
  Fsm_Mgr_t * fsmMgr        /* struttura FSM */,
  char * word               /* parola corrente */,
  int bddFlag
)
{
  char *name;               /* tr file name */
  Ddi_Bdd_t *from;           /* bdd */

  readBddFile (fp, fsmMgr, Fsm_MgrReadDdManager (fsmMgr), word, &name,
    &from, NULL, bddFlag);

  Fsm_MgrSetFromName (fsmMgr,  name);
  if (bddFlag == 1) {
    Fsm_MgrSetFromBDD (fsmMgr, from);
    Ddi_Free (from);
  } else {
    Fsm_MgrSetFromBDD (fsmMgr, NULL);
  }

  return;
}

/**Function********************************************************************

  Synopsis           [Reads <em>.tr</em> section of the <em>.fsm</em> file.]

  Description        [Reads ".reached" section.]

  SideEffects        []

  SeeAlso            [Poli_bddRead_new]

******************************************************************************/

static void 
readFsmReached (
  FILE * fp                 /* file */, 
  Fsm_Mgr_t * fsmMgr        /* struttura FSM */,
  char * word               /* parola corrente */,
  int bddFlag
)
{
  char *name;             /* tr file name */
  Ddi_Bdd_t *reached;      /* bdd */

  readBddFile (fp, fsmMgr, Fsm_MgrReadDdManager(fsmMgr), word, &name,
    &reached, NULL, bddFlag);

  Fsm_MgrSetReachedName (fsmMgr, name);
  if (bddFlag == 1) {
    Fsm_MgrSetReachedBDD (fsmMgr, reached);
    Ddi_Free (reached);
  } else {
    Fsm_MgrSetReachedBDD (fsmMgr, NULL);
  }
 
  return;
}


/**Function********************************************************************

  Synopsis     [Reads <em>.init</em> section of the <em>.fsm</em> file.]

  Description  [Reads ".init" section of the new ".fsm" file format 
    and then loads the FSM structure with initial state BDD. 
    In order to do that, it uses "Ddi_BddarrayLoad" procedure, preceded by
    "Pdtutil_FileOpen" and followed by "Pdtutil_FileClose" in case
    BDD is specified with a fileName.
    ]

  SideEffects []

  SeeAlso     [Poli_bddRead_new,readInitFile]

******************************************************************************/

static void 
readFsmInitState (
  FILE * fp                 /* file */, 
  Fsm_Mgr_t * fsmMgr        /* struttura FSM */,
  char * word               /* parola corrente */,
  int bddFlag
  )
{
  char *name;               /* s0 file name */
  Ddi_Bdd_t *s0;             /* bdd */

  readBddFile (fp, fsmMgr, Fsm_MgrReadDdManager(fsmMgr), word, &name,
    &s0, NULL, bddFlag);

  Fsm_MgrSetInitName (fsmMgr, name);
  if (bddFlag == 1) {
    Fsm_MgrSetInitBDD (fsmMgr, s0);
    Ddi_Free (s0);
  } else {
    Fsm_MgrSetInitBDD (fsmMgr, NULL);
  }

  return;
}


/**Function********************************************************************

  Synopsis           [Reads the order in the <em>.ord</em> file.]

  Description        []

  SideEffects        []

******************************************************************************/

static void 
readOrdFile (
  FILE *fp                  /* File Pointer */,
  Fsm_Mgr_t *fsmMgr         /* FSM Manager */,
  Pdtutil_VariableOrderFormat_e ordFileFormat
  )
{
  char **varnames;
  int *varauxids;
  int nvars;

  nvars = Pdtutil_OrdRead (&varnames, &varauxids, NULL, fp, ordFileFormat);

  if (nvars<0) {
    Pdtutil_Warning (1, "Error reading variable ordering.");
    return;
  }

  if (nvars!= (Fsm_MgrReadNI (fsmMgr)+2*Fsm_MgrReadNL (fsmMgr))) {
    Pdtutil_Warning (1, "Wrong number of variables in ord file.");
    return;
  }

  Fsm_MgrSetVarnames (fsmMgr, varnames);
  Fsm_MgrSetVarauxids (fsmMgr, varauxids);

  return;
}

/**Function********************************************************************

  Synopsis           [read a bdd file]

  Description        [read a BDD dump file. Returns an array of BDD roots]

  SideEffects        []

  SeeAlso            []

******************************************************************************/

static void 
readBddFile (
  FILE * fp                 /* file */, 
  Fsm_Mgr_t * fsmMgr        /* struttura FSM */,
  Ddi_Mgr_t * dd            /* DD manager */,
  char * word               /* parola corrente */,
  char **pName              /* return file name by reference */,
  Ddi_Bdd_t **pBdd           /* return bdd by reference */,
  Ddi_Bddarray_t **pBddarray /* return bdd array by reference */,
  int bddFlag               /* Load - NotLoad BDD from files */
)
{
  Fsm_TokenType token;
  FILE *bddfp;

  *pName = NULL;
  bddfp = fp;

  token = get_token (fp, word);
  while (token != KeyFSM_END) {
    switch (token) {
      case KeyFSM_FILE_BDD:
        get_token (fp, word);
        *pName = Pdtutil_StrDup (word);
        bddfp = NULL;        
      case KeyFSM_FILE_TEXT:
        if (bddFlag == 1) {
          if (pBdd != NULL) { 
            *pBdd = Ddi_BddLoad(dd,DDDMP_VAR_MATCHNAMES,
              DDDMP_MODE_DEFAULT,*pName,bddfp);  
          } else {
            *pBddarray = Ddi_BddarrayLoad(dd,Fsm_MgrReadVarnames(fsmMgr),
              Fsm_MgrReadVarauxids(fsmMgr),DDDMP_MODE_DEFAULT,*pName,bddfp);
          }
        }
        break;
      default :
        fprintf (stderr,
          "Error: %s option Not recognised!\n", word);
        exit (1);
        break;
      }

  token = get_token (fp, word);
  }

  return;
}

#if 0
static void 
readInitFile (
FILE *fp                  /* file */,
Fsm_Mgr_t * fsmMgr           /* struttura FSM */,
char * word               /* parola corrente */
)
{
  Ddi_Bdd_t *one;
  Ddi_Mgr_t *dd = Fsm_MgrReadDdManager (fsmMgr);

  one = Ddi_MgrReadOne(dd);

  fprintf (stdout, "cube loading to be rewritten\n");

  METTERE A POSTO

  string[0]=0;

  while (get_token (fp, word)!=BUF_END) {
    strcat (string, word); 
  }
  /* printf("Init_state:%s\n",string); */

  Fsm_MgrReadInitString(fsmMgr) = Pdtutil_Alloc(char,strlen(string)+1);
  strcpy (Fsm_MgrReadInitString(fsmMgr), &string[0]); /* puo'essere necessario */
  S0 = one;                          /* saltare il primo spazio di string */
  for (i=Fsm_MgrReadNL(fsmMgr)-1; i>=0; i--) {
    V = Ddi_VararrayRead(Fsm_MgrReadVarPS(fsmMgr), i);
    if (Fsm_MgrReadInitString(fsmMgr)[i] == '0') {
      F = Ddi_BddMakeLiteral(V,0);
    } else {
      if (Fsm_MgrReadInitString(fsmMgr)[i] == '1') {
  	  F = Ddi_BddMakeLiteral(V,1);
      } else {
	if (Fsm_MgrReadInitString(fsmMgr)[i] != '-') {
	  fprintf (stdout, "Not Valid init_state.\n");
	  exit (1);
	}
      }
    }
    Ddi_BddAndAcc(F,S0);
  }

  Fsm_MgrSetInitBDD (fsmMgr, S0);
}
#endif

/**Function********************************************************************

  Synopsis           [Read a String from the File.]

  Description        [Read a String from the File. It returns the
    string and a code that indicates the type of the string.
    ]

  SideEffects        []

  SeeAlso            [get_line]

******************************************************************************/

Fsm_TokenType 
get_token (
  FILE *fp,
  char *word      /* Current Word */
)
{
  Fsm_TokenType val;

  fscanf (fp, "%s", word);

  val = FsmString2Token (word);
  return (val);
}


/**Function********************************************************************

  Synopsis           [Returns the current word as integer.]

  SideEffects        []

******************************************************************************/

static int 
get_number (
  FILE *fp,
  char *word
  )
{
  int num;

  if (get_token (fp, word)!=KeyFSM_STRING) { 
    fprintf (stderr, "Error: String expected -> %s\n", word);
  }   
  if (sscanf (word, "%d", &num) < 1) {
    fprintf (stderr, "Error : %s string must be a number -> \n", word);
  }
  return num ;
}


/**Function********************************************************************

  Synopsis           [Inse le parole in cima all'array "datum_str".]

  SideEffects        []

  SeeAlso            [array_insert]

******************************************************************************/

static char ** 
get_string_array (
  FILE *fp,
  char *word,
  int size      /* number of expected items */
)
{
  char *str;
  int i;
  char **array;
  
  array=Pdtutil_Alloc(char *,size);
  if (array==NULL) {
    fprintf (stderr, "Error : Out of memory allocating string array\n");
    return(NULL);
  }
  for (i=0; i<size;i++) {
    if (get_token(fp,word) != KeyFSM_STRING) {
      fprintf (stderr, "Error : String expected for array\n");
    }
    str=Pdtutil_StrDup(word);
    array[i]=str;
  }

  return array;
}


/**Function********************************************************************

  Synopsis           [Inserisce i numeri in cima all'array "datum_nmb".]

  Description        [Trasforma le parole estratte da "get_token" in interi
  per mezzo della procedura "number". Tramite la procedura "Ddi_BddarrayWrite" 
  inserisce i numeri in cima all'array "datum_nmb".]

  SideEffects        []

  SeeAlso            [number,Ddi_BddarrayWrite]

******************************************************************************/

static int * 
get_integer_array (
FILE *fp,
char *word,
int size      /* number of expected items */
)
{
  int i;
  int *array;
  
  array = Pdtutil_Alloc (int, size);
  if (array==NULL) {
    fprintf (stderr, "Error: Out of memory allocating integer array\n");
    return(NULL);
  }

  for (i=0; i<size;i++) {
    array[i] = get_number (fp,word);
  }

  return (array);
}

