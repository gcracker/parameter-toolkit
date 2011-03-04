/**CFile***********************************************************************

  FileName    [fsmPort.c]

  PackageName [fsm]

  Synopsis    [Functions to guarantee portability]

  Description [In this file are declarated function to deal with 
    BLIF format (as package Nanotrav in CUDD).
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
/* Constant declarations                                                    
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                    
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                        
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                    
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                       
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static FsmPortNtrOptions_t * mainInit();

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                         
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Reads description of a fsmMgr (BLIF format) from file]

  Description [The function gives a compatible DDI:
    <ul>
    <li>number of primary input
    <li>number of latches
    <li>array of primary input variables
    <li>array of present state variables
    <li>array of next state variables
    <li>array of next state functions
    <li>array of partitioned transition relation
    </ul>]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

int
Fsm_MgrLoadFromBlif (
  Fsm_Mgr_t **fsmMgrP                          /* FSM Pointer */,
  Ddi_Mgr_t *dd                                /* BDD manager */,
  char *fileFsmName                            /* FSM File Name */,
  char *fileOrdName                            /* ORD File Name */,
  int bddFlag                                  /* Not Used For Now */,
  Pdtutil_VariableOrderFormat_e ordFileFormat  /* Order File Format */
  )
{
  Fsm_Mgr_t *fsmMgr;
  FsmPortBnetNetwork_t *net;    /* network */
  FsmPortNtrOptions_t *option;  /* options */
  FILE *fp = NULL;
  Ddi_Bdd_t *temp;
  Ddi_Var_t *var;
  int i, flagFile, result;
  int xlevel;
  FsmPortBnetNode_t *node;
  char *name, *nameext;
  int index, ntotv, value;
  long fsmTime;

  fsmTime = util_cpu_time();

  /*-------------------------- Check For FSM File  --------------------------*/

  nameext = Pdtutil_FileName (fileFsmName, "", "blif", 0);
  fp = Pdtutil_FileOpen (fp, nameext, "r", &flagFile);
  if (fp==NULL) {
    Pdtutil_Warning (1, "NULL Finite State Machine Pointer.");
    return (1);
  }

  /*------------------------- FSM Manager Allocation ------------------------*/

  fsmMgr = *fsmMgrP;
  if (fsmMgr == NULL) {
    Pdtutil_Warning (1, "Allocate a new FSM structure");
    fsmMgr = Fsm_MgrInit (NULL);
  }

  if (fsmMgr->settings.verbosity > Pdtutil_VerbLevelDevMin_c) {
    fprintf (stdout, "(... loading %s ...)", fileFsmName) ;
    fflush (stdout);
  }

  /*------------------------------ Init Options -----------------------------*/

  option = mainInit();

  if (fileOrdName != NULL) {
    if (strcmp (fileOrdName, "file") == 0) {
       /* Enable Using Order From Blif File (default) */
      option->ordering = PI_PS_FROM_FILE;
    } else {
      if (strcmp (fileOrdName, "dfs") == 0) {
	 /* Enable Using Depth First Search */
        option->ordering = PI_PS_DFS;
      } else {
	 /* Enable Reading Order From File */
        option->ordering = PI_PS_GIVEN;
        option->orderPiPs = Pdtutil_StrDup (fileOrdName);
      }
    }
  }

  /*------------------ Load description of the FSM Manager ------------------*/

  net = Fsm_PortBnetReadNetwork (fp, option->verb);  
  Pdtutil_Warning (net==NULL, "Error : Description of fsmMgr not loaded.\n");
  if (net==NULL) {
    return (1);
  }

  Pdtutil_FileClose (fp, &flagFile);

  /*-------------- Build the BDDs for the Nodes of the Network --------------*/

  result = Fsm_PortNtrBuildDDs (net, dd, option,
    ordFileFormat);
  Pdtutil_Warning (result==0, "Impossible to build the BDDs for the network");
  if (result == 0) {
    return (1);
  }

#if 0
  /* since Fsm_PortNtrBuildDDs has operated on CUDD manager only, 
     DDI mgr update is required */
  Ddi_MgrUpdate(dd);
#endif

  Ddi_MgrConsistencyCheck(dd);
  i =  Ddi_CheckZeroRef (dd);
  if (i!=0) {
    fprintf(stderr, "%d non-zero DD reference counts after building DDs\n",i);
  }

  /*------- Translate CUDD format in our internal FSM Manager Format --------*/
  
  Fsm_MgrSetDdManager (fsmMgr, dd);

  /* Number of State Variable */
  Fsm_MgrSetNL (fsmMgr, net->nlatches);
  /* Number of Primary Input */
  Fsm_MgrSetNI (fsmMgr, net->npis);
  /* Number of Primary Outputs */
  Fsm_MgrSetNO (fsmMgr, net->npos);

  /* BDD of Initial & Reached State Set */
  if (Fsm_MgrReadNL(fsmMgr) > 0) {
    fsmMgr->init.bdd = Fsm_PortNtrInitState (dd, net, option);
    fsmMgr->reached.bdd = Ddi_BddDup (fsmMgr->init.bdd);
  } else {
    fsmMgr->init.bdd = NULL;
    fsmMgr->reached.bdd = NULL;
  }

  /*
   *  Init Temporary Set and Array
   */

  Ddi_MgrConsistencyCheck(dd);

  ntotv = Ddi_ReadSize(dd)+2*Fsm_MgrReadNL (fsmMgr);

  if (net->nlatches > 0) {
    fsmMgr->var.ps = Ddi_VararrayAlloc (dd,Fsm_MgrReadNL (fsmMgr));
    fsmMgr->var.ns = Ddi_VararrayAlloc (dd,Fsm_MgrReadNL (fsmMgr));
    fsmMgr->delta.bdd = Ddi_BddarrayAlloc (dd,Fsm_MgrReadNL (fsmMgr));
    fsmMgr->name.nsTrueName = Pdtutil_Alloc (char *, Fsm_MgrReadNL (fsmMgr));
  } else {
    fsmMgr->var.ps = NULL;
    fsmMgr->var.ns = NULL;
    fsmMgr->delta.bdd = NULL;
    fsmMgr->name.nsTrueName = NULL;
  }

  fsmMgr->lambda.bdd = Ddi_BddarrayAlloc (dd,Fsm_MgrReadNO (fsmMgr));
  fsmMgr->varnames = Pdtutil_Alloc (char *, ntotv); 
  fsmMgr->varauxids = Pdtutil_Alloc (int, ntotv);

  for (i=0; i<ntotv; i++) {
    fsmMgr->varnames[i] = NULL;
    fsmMgr->varauxids[i] = (-1);
  }

  Ddi_MgrConsistencyCheck(dd);

  /*------------------ Translate Array of Input Variables -------------------*/

  if (Fsm_MgrReadNI (fsmMgr) > 0) {
    fsmMgr->var.i = Ddi_VararrayAlloc (dd,Fsm_MgrReadNI (fsmMgr));
    for (i=0; i<net->npis; i++) {
      if (!st_lookup (net->hash, net->inputs[i], (char **)&node)) { 
        return (1);
      }
    
      var = node->var;
      Ddi_VararrayWrite (fsmMgr->var.i, i, var);
    
      index = Ddi_VarIndex(var);
      fsmMgr->varauxids[index] = i;
      name = net->inputs[i];
      fsmMgr->varnames[index] = Pdtutil_StrDup (name);
    }
  }
  else {
    fsmMgr->var.i = NULL;
  }

  Ddi_MgrConsistencyCheck(dd);
  /*-------- Translate Array of Present State Variables and Delta -----------*/

  fsmMgr->name.nsTrueName = Pdtutil_Alloc (char *, net->nlatches);

  for (i=0; i<net->nlatches; i++) {
    if (!st_lookup (net->hash, net->latches[i][1], (char **)&node)) {
      return (1);
    }

    var = node->var;
    Ddi_VararrayWrite (fsmMgr->var.ps, i, var);

    index = Ddi_VarIndex (var);
    fsmMgr->varauxids[index] = i+net->npis;
    fsmMgr->varnames[index] = Pdtutil_StrDup (net->latches[i][1]);

    fsmMgr->name.nsTrueName[i] = Pdtutil_StrDup (net->latches[i][0]);

    /* Create array of next state var */
    xlevel = Ddi_VarCurrPos (var)+ 1;
#if 1
    var = Ddi_VarNewAtLevel (dd, xlevel);
#else
    var = Ddi_VarNew (dd);
#endif
    Ddi_VararrayWrite (fsmMgr->var.ns, i, var);

    index = Ddi_VarIndex(var);
    fsmMgr->varauxids[index] = i+net->npis+net->nlatches;
    fsmMgr->varnames[index] = Pdtutil_Alloc (char,
      strlen(net->latches[i][1])+4);

    sprintf (fsmMgr->varnames[index], "%s$NS", net->latches[i][1]);

    /* Translate array of next state function (delta) */

    if (!st_lookup(net->hash, net->latches[i][0], (char **)&node)) {
      return (1);
    }

    Ddi_BddarrayWrite (fsmMgr->delta.bdd, i, node->dd);
  }

  Ddi_MgrConsistencyCheck(dd);
  /*---------------- Translate Array of Output Function ---------------------*/

  fsmMgr->name.o = Pdtutil_Alloc (char *, Fsm_MgrReadNO (fsmMgr));

  for (i=0;i<Fsm_MgrReadNO (fsmMgr);i++) {
    if (!st_lookup (net->hash, net->outputs[i], (char **)&node)) { 
      return (1);
    }
    
    Ddi_BddarrayWrite (fsmMgr->lambda.bdd, i, node->dd); 

    name = net->outputs[i];
    fsmMgr->name.o[i] = Pdtutil_StrDup (name);
  } 
   
   /* Dispose of network */
  for (node = net->nodes; node != NULL; node = node->next) {
    Ddi_Free(node->dd);
  }

  /*-------------------- Set Names of Variables -----------------------------*/

  if (Fsm_MgrReadVarnames (fsmMgr) != NULL) {
    fsmMgr->name.i = Pdtutil_Alloc (char *, Fsm_MgrReadNI (fsmMgr));
    fsmMgr->index.i  = Pdtutil_Alloc (int, Fsm_MgrReadNI (fsmMgr));

    for (i=0; i<Fsm_MgrReadNI (fsmMgr); i++) {
      value = Ddi_VarIndex (Ddi_VararrayRead
        (Fsm_MgrReadVarI (fsmMgr), i));
      fsmMgr->name.i[i] = Pdtutil_StrDup (fsmMgr->varnames[value]);
      fsmMgr->index.i[i] = fsmMgr->varauxids[value];
    }

    if (net->nlatches > 0) {
      fsmMgr->name.ps = Pdtutil_Alloc (char *, Fsm_MgrReadNL (fsmMgr));
      fsmMgr->index.ps = Pdtutil_Alloc (int, Fsm_MgrReadNL (fsmMgr));
      fsmMgr->name.ns = Pdtutil_Alloc (char *, Fsm_MgrReadNL (fsmMgr));
      fsmMgr->index.ns =  Pdtutil_Alloc (int, Fsm_MgrReadNL (fsmMgr));
    } else {
      fsmMgr->name.ps = NULL;
      fsmMgr->index.ps = NULL;
      fsmMgr->name.ns = NULL;
      fsmMgr->index.ns = NULL;
    }

    Fsm_PortBnetFreeNetwork (net);

    Ddi_MgrConsistencyCheck(dd);

    for (i=0; i<Fsm_MgrReadNL (fsmMgr); i++) {
      value = Ddi_VarIndex (Ddi_VararrayRead (
        Fsm_MgrReadVarPS (fsmMgr), i));
      fsmMgr->name.ps[i] = Pdtutil_StrDup (fsmMgr->varnames[value]);
      fsmMgr->index.ps[i] = fsmMgr->varauxids[value];

      value = Ddi_VarIndex (Ddi_VararrayRead (
        Fsm_MgrReadVarNS (fsmMgr), i));
      fsmMgr->name.ns[i] = Pdtutil_StrDup (fsmMgr->varnames[value]);
      fsmMgr->index.ns[i] = fsmMgr->varauxids[value];
    }

    for (i=0; i<Ddi_ReadNumVars(dd); i++) {
      Ddi_VarAttachName(Ddi_IthVar(dd,i),fsmMgr->varnames[i]);
      Ddi_VarAttachAuxid(Ddi_IthVar(dd,i),fsmMgr->varauxids[i]);
    }

  }

  /*---------------------- Safe Test and Return -----------------------------*/

  if (Fsm_MgrReadNL (fsmMgr) == 0) {
    Pdtutil_Warning (1, "Combinational Network.");
  }

  /*
   *  return 1 = Something went wrong
   */

  if (Fsm_MgrReadNI (fsmMgr) == 0|| Fsm_MgrReadNO (fsmMgr) == 0) {
    Fsm_MgrQuit (fsmMgr);
    return (1);  
  }

  /*
   *  return 0 = All OK
   */

  fsmTime = util_cpu_time() - fsmTime;
  fsmMgr->fsmTime += fsmTime;

  *fsmMgrP = fsmMgr;
  return (0);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Allocates the option structure for BLIF format and initializes
    it.]

  Description [This function was taken from main.c in <b>nanotrav</b> package]

  SideEffects [none]

  SeeAlso     [Fsm_MgrLoadFromBlif]

******************************************************************************/

static FsmPortNtrOptions_t *
  mainInit (
  )
{
  FsmPortNtrOptions_t	*option;

  /* Initialize option structure. */
  option = ALLOC(FsmPortNtrOptions_t,1);

  option->initialTime    = util_cpu_time();
  option->verify         = FALSE;
  option->second         = FALSE;
  option->file1          = NULL;
  option->file2          = NULL;
  option->traverse       = FALSE;
  option->depend         = FALSE;
  option->image          = PORT_NTRIMAGE_MONO;
  option->imageClip      = 1.0;
  option->approx         = PORT_NTRUNDER_APPROX;
  option->threshold      = -1;
  option->from           = PORT_NTRFROM_NEW;
  option->groupnsps      = PORT_NTRGROUP_NONE;
  option->closure        = FALSE;
  option->closureClip    = 1.0;
  option->envelope       = FALSE;
  option->scc            = FALSE;
  option->maxflow        = FALSE;
  option->zddtest        = FALSE;
  option->sinkfile       = NULL;
  option->partition      = FALSE;
  option->char2vect      = FALSE;
  option->density        = FALSE;
  option->quality        = 1.0;
  option->decomp         = FALSE;
  option->cofest         = FALSE;
  option->clip           = -1.0;
  option->noBuild        = FALSE;
  option->stateOnly      = FALSE;
  option->node           = NULL;
  option->locGlob        = PORT_BNETGLOBAL_DD;
  option->progress       = FALSE;
  option->cacheSize      = DDI_CACHE_SLOTS;
  option->maxMemory      = 0; /* set automatically */
  option->slots          = CUDD_UNIQUE_SLOTS;

  /*
   * Create Variable Order while Building the BDDs
   */

  option->ordering       = PI_PS_FROM_FILE;
  option->orderPiPs      = NULL;
  option->reordering     = CUDD_REORDER_NONE;
  option->autoMethod     = CUDD_REORDER_SIFT;
  option->autoDyn        = 0;
  option->treefile       = NULL;
  option->firstReorder   = DD_FIRST_REORDER;
  option->countDead      = FALSE;
  option->maxGrowth      = 20;
  option->groupcheck     = CUDD_GROUP_CHECK7;
  option->arcviolation   = 10;
  option->symmviolation  = 10;
  option->recomb         = DD_DEFAULT_RECOMB;
  option->nodrop         = TRUE;
  option->signatures     = FALSE;
  option->verb           = 0;
  option->gaOnOff        = 0;
  option->populationSize = 0; /* use default */
  option->numberXovers   = 0; /* use default */
  option->bdddump        = FALSE;
  option->dumpFmt        = 0; /* dot */
  option->dumpfile       = NULL;
  option->store          = -1; /* do not store */
  option->storefile      = NULL;
  option->load           = FALSE;
  option->loadfile       = NULL;

  return(option);
}

