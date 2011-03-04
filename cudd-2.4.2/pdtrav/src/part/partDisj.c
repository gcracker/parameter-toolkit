/**CFile***********************************************************************

  FileName    [partDisj.c]

  PackageName [part]

  Synopsis    [Functions for Disjunctive Partitioning.]

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
                                         
#include "partInt.h"                                          

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define COST_LARGE 1000000.0
#define DISPLAY_WEIGHTS 0

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

static void partPartitionDisjSet(Ddi_Bdd_t *f, Ddi_Bdd_t *c, Ddi_Varset_t *careVars, Part_Method_e partitionMethod, int threshold, Ddi_Bddarray_t *ddVet, Pdtutil_VerbLevel_e verbosity);
static void exactCofactor(Ddi_Bdd_t *f, Ddi_Var_t *topVar, int *thenSize, int *elseSize);
static void estimateCofactor(Ddi_Bdd_t *f, Ddi_Var_t *topVar, int *thenSize, int *elseSize);
static Ddi_Var_t * varSelectionFast(Ddi_Bdd_t *f, Ddi_Varset_t *careVars);
static void estimateCofactorComplex(Ddi_Bdd_t *f, Ddi_Var_t *topVar, int *thenSize, int *elseSize);
static void estimateCofactorFreeOrder(Ddi_Bdd_t *f, Ddi_Var_t *topVar, int *thenSize, int *elseSize, int *thenArray, int *elseArray);
static void comparisonRoutine(Ddi_Bdd_t *f, Ddi_Var_t *topVar, int *thenSize, int *elseSize);
static float estimateCost(int size, int thenSize, int elseSize, int varPos, int suppSize);
static float estimateCostFreeOrder(int size, int thenSize, int elseSize, int *thenArray, int *elseArray, int varPos, int suppSize);
static Ddi_Var_t * varSelection(Ddi_Bdd_t *f, Ddi_Varset_t *careVars, Part_Method_e partitionMethod);
static Ddi_Var_t * varSelectionManual(Ddi_Bdd_t *f);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Interface Routine to Partition a state set]

  Description [It calls Part_PartitionDisjSet (to partition a BDD
    using "internal" routines) or Part_PartitionSetCudd (to partition a
    BDD using the CUDD routines.]

  SideEffects [none]

  SeeAlso     [Trav_Traversal, partPartitionDisjSet]

******************************************************************************/

Ddi_Bdd_t *
Part_PartitionSetInterface (
  Ddi_Bdd_t *f                      /* Input BDD */,
  Ddi_Varset_t *careVars           /* Set of Vars */,
  Part_Method_e partitionMethod    /* Partition Method */,
  int threshold                    /* Threshold Size */,
  Pdtutil_VerbLevel_e verbosity    /* Verbosity Level */
  )
{
  if (partitionMethod<=Part_MethodAppCon_c) {

    /*
     *  Call "internal" method
     */
	            
    return (Part_PartitionDisjSet (f, NULL, partitionMethod, threshold,
      verbosity));
  } else {

    /*
     *  Call Cudd method
     */

    return (Part_PartitionSetCudd (f, partitionMethod, threshold,
      verbosity));
  }
}

/**Function********************************************************************

  Synopsis           [Partition a state set]

  Description        []

  SideEffects        [none]

  SeeAlso            [Trav_Traversal, partPartitionDisjSet]

******************************************************************************/

Ddi_Bdd_t *
Part_PartitionDisjSet (
  Ddi_Bdd_t *f                      /* Input BDD */,
  Ddi_Varset_t *careVars           /* Set of Vars */,
  Part_Method_e partitionMethod    /* Partition Method */,
  int threshold                    /* Threshold Size */,
  Pdtutil_VerbLevel_e verbosity    /* Verbosity Level */
  )
{
  int i, thenSize, elseSize, bddSize=0;
  long startTime=0, currTime;
  Ddi_Bddarray_t *ddVet;
  Ddi_Bdd_t *fdup, *f0, *f1;
  Ddi_Bdd_t *result, *dpart;
  Ddi_Mgr_t *dd = Ddi_ReadMgr(f);
  Ddi_Bdd_t *one = Ddi_BddDup(Ddi_MgrReadOne(dd));

  ddVet = Ddi_BddarrayAlloc (dd,0);
  
  /* f is freed by partPartitionDisjSet */ 
  fdup = Ddi_BddDup(f);
  
  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    startTime = util_cpu_time();
    bddSize = Ddi_BddSize (fdup);
  }

  /*----------------------------- Partitioning ------------------------------*/

  switch (Ddi_ReadCode(fdup)) {
    case Ddi_Bdd_Mono_c:
      partPartitionDisjSet (fdup, one, careVars, partitionMethod,
        threshold, ddVet, verbosity);
      break;
    case Ddi_Bdd_Part_Conj_c:
      Pdtutil_Warning (Ddi_BddPartNum(f)>1, "One partition expected");
      partPartitionDisjSet (Ddi_BddPartRead (fdup,0), one, careVars,
        partitionMethod, threshold, ddVet, verbosity);
      break;
    default:
      Pdtutil_Assert(0,"Wrong DDI node code");
      break;
  }
  
  /*--------------------- From BDD-Array to BDD-Disjunctive -----------------*/

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    fprintf (stdout, "PARTITIONING (%d) ==> [", bddSize);
  }

  result = Ddi_BddMakePartDisjVoid(dd);

  for (i=0; i< Ddi_BddarrayNum (ddVet); i+=2) {
    f0 = Ddi_BddarrayRead (ddVet, i);
    f1 = Ddi_BddarrayRead (ddVet, i+1);
    thenSize = Ddi_BddSize (f0);
    elseSize = Ddi_BddSize (f1);

    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "%d+", thenSize+elseSize-1);
      fflush(stdout);
    }

    /* Generate constrained disjoined partition */
    dpart = Ddi_BddAnd (f0, f1);

    /* Add partition to disjoined result */
    Ddi_BddPartInsertLast(result,dpart);
    Ddi_Free (dpart);
  }

  /*------------------------------ Final Statistics -------------------------*/

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    bddSize = Ddi_BddarrayNum (ddVet);
    currTime = util_cpu_time();

    fprintf (stdout, "=%d]", bddSize);
    fprintf (stdout, "(time:%s)\n", util_print_time (currTime - startTime));
    fflush(stdout);
  }

  Ddi_Free (ddVet);
  
  return (result);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Splitting a set]

  Description []

  SideEffects [None]

  SeeAlso     [Part_PartitionSet, varSelectionManual, varSelection,
    varSelectionFreeOrder]

******************************************************************************/

static void 
partPartitionDisjSet(
  Ddi_Bdd_t *f                      /* Input BDD */,
  Ddi_Bdd_t *c                      /* Constraining Cube */,
  Ddi_Varset_t *careVars           /* Set of Vars */,
  Part_Method_e partitionMethod    /* Partition Method */,
  int threshold                    /* Threshold Size */,
  Ddi_Bddarray_t *ddVet             /* Partition Array */,
  Pdtutil_VerbLevel_e verbosity    /* Verbosity Level */
  )
{
  int bddSize, thenSize, elseSize;
  long startTime, currTime;
  Ddi_Bdd_t *f0, *f1, *lit;
  Ddi_Var_t *var;
  Ddi_Bdd_t *c0, *c1;
  char *varName;

  startTime = util_cpu_time ();
  
  bddSize = Ddi_BddSize (f);
  
  /*--------------------- Check NULL method and BDD size --------------------*/

  if (partitionMethod==Part_MethodNone_c || bddSize<threshold) {
    Ddi_BddarrayWrite (ddVet, Ddi_BddarrayNum(ddVet), c);
    Ddi_BddarrayWrite (ddVet, Ddi_BddarrayNum(ddVet), f);
    Ddi_Free (c);
    Ddi_Free (f);

    return;
  }

  /*----------------------- Select the Splitting Variable -------------------*/

  switch (partitionMethod) {
    case Part_MethodManual_c:
      var = varSelectionManual (f);
      break;
    case Part_MethodEstimateFast_c:
      var = varSelectionFast (f, careVars);
      break;
    case Part_MethodCofactor_c:
    case Part_MethodEstimate_c:
    case Part_MethodEstimateComplex_c:
    case Part_MethodEstimateFreeOrder_c:
    case Part_MethodComparison_c:
      var = varSelection (f, careVars, partitionMethod);
      break;
    default:
      fprintf (stderr, "Error: Decomposition choice.\n");
      return;
      break;
  }
  
  /*------------------------------- No Split to DO --------------------------*/

  if (var == NULL) {
    Ddi_BddarrayWrite (ddVet, Ddi_BddarrayNum (ddVet), c);
    Ddi_BddarrayWrite (ddVet, Ddi_BddarrayNum (ddVet), f);

    return;
  }

  /*-------------------------------- Splitting ------------------------------*/

  lit = Ddi_BddMakeLiteral(var,0);
  f0 = Ddi_BddConstrain (f, lit); 
  c0 = Ddi_BddAnd (c, lit);
  Ddi_Free (lit);
  lit = Ddi_BddMakeLiteral(var,1);
  f1 = Ddi_BddConstrain (f, lit); 
  c1 = Ddi_BddAnd (c, lit); 
  Ddi_Free (lit);
  Ddi_Free (c);
  Ddi_Free (f);
  thenSize = Ddi_BddSize (f0);
  elseSize = Ddi_BddSize (f1);
  
  currTime = util_cpu_time ();

  if (verbosity >= Pdtutil_VerbLevelNone_c) {
    varName = Ddi_VarName (var);
    if (varName == NULL) {
      varName = Pdtutil_Alloc (char, 4);
      strcpy (varName, "???");
    }

    fprintf (stdout,
      "[%d--->%d,%d(id:%d/name:%s)(estimateCost:%.3f)(time:%s)]\n", 
      bddSize, thenSize, elseSize, Ddi_VarIndex(var), varName,
      estimateCost (bddSize, thenSize, elseSize, 0, 0),
      util_print_time(currTime - startTime));
  }

  /*---------------------------------- Recur --------------------------------*/

  partPartitionDisjSet (f0, c0, careVars, partitionMethod, threshold, ddVet,
    verbosity);
  partPartitionDisjSet (f1, c1, careVars, partitionMethod, threshold, ddVet,
    verbosity);
  
  return;
}


/**Function********************************************************************

  Synopsis           [Counts the number of nodes of the cofactors]

  Description        []

  SideEffects        [None]

  SeeAlso            [varSelection]

******************************************************************************/

static void
exactCofactor (
  Ddi_Bdd_t *f         /* Input BDD */,
  Ddi_Var_t *topVar   /* Top variable */,
  int *thenSize       /* Number nodes cofactors */,
  int *elseSize       /* Number nodes cofactors */
  )
{
  Ddi_Bdd_t *f0, *f1, *lit;

  f0 = Ddi_BddConstrain (f, (lit=Ddi_BddMakeLiteral(topVar,0)));
  Ddi_Free (lit);
  f1 = Ddi_BddConstrain (f, (lit=Ddi_BddMakeLiteral(topVar,1))); 
  Ddi_Free (lit);
  
  *thenSize = Ddi_BddSize (f0);
  *elseSize = Ddi_BddSize (f1);
  
  Ddi_Free (f0);
  Ddi_Free (f1);

  return;
}


/**Function********************************************************************

  Synopsis    [Calls function for to count the number of nodes of the
    cofactors: ICCAD'96 original solution]

  Description []

  SideEffects [none]

  SeeAlso     [varSelection, Part_EstimateCofactor]

******************************************************************************/

static void
estimateCofactor (
  Ddi_Bdd_t *f                    /* input BDD */,
  Ddi_Var_t *topVar           /* top variable */,
  int *thenSize                        /* number nodes cofactors */,
  int *elseSize                        /* number nodes cofactors */
  )
{
  Ddi_Bdd_t *lit;

  *thenSize = Part_EstimateCofactor (f, (lit=Ddi_BddMakeLiteral(topVar,0)));
  Ddi_Free (lit);
  *elseSize = Part_EstimateCofactor (f, (lit=Ddi_BddMakeLiteral(topVar,1))); 
  Ddi_Free (lit);
  
  *thenSize = *thenSize + 1;
  *elseSize = *elseSize + 1;
  
  return;
}


/**Function********************************************************************

  Synopsis    [Call function for to count the number of nodes of the
    cofactors: TCAD'99 fast solution]

  Description []

  SideEffects [none]

  SeeAlso     [selectBestVar, stqCntNodesVar]

******************************************************************************/

static Ddi_Var_t *
varSelectionFast (
  Ddi_Bdd_t *f                  /* Input BDD */,
  Ddi_Varset_t *careVars       /* Set of Variables */
  )
{
  Ddi_Mgr_t *dd;
  Ddi_Varset_t *supp, *allowedVars;
  Ddi_Var_t *var, *saveVar;
  int i, minThen, minElse;
  int *totalArray, *thenThenArray, *elseThenArray, *thenElseArray,
    *elseElseArray, *upArray, *downArray, *thenArray, *elseArray;
  int suppSize, totalSize, bddSize;
  float costRef, costNew;

  /*-------------------------- Collect Statistics ---------------------------*/

  dd = Ddi_ReadMgr (f);
  supp = Ddi_BddSupp (f);
#if 0
  suppSize = Ddi_BddSize (supp) - 1;
#else
  suppSize = Ddi_ReadNumVars(dd);
#endif
  bddSize = Ddi_BddSize (f);

  /*------------------------ Take Care-Variable set -------------------------*/

  if (careVars != NULL) {
    allowedVars = Ddi_VarsetIntersect (supp, careVars);  
  } else {
    allowedVars = Ddi_VarsetDup (supp);
  }

  /*---------------- Initialize Array to get Counting Results ---------------*/

  totalArray  = Pdtutil_Alloc (int, suppSize);
  thenThenArray = Pdtutil_Alloc (int, suppSize);
  elseThenArray = Pdtutil_Alloc (int, suppSize);
  thenElseArray = Pdtutil_Alloc (int, suppSize);
  elseElseArray = Pdtutil_Alloc (int, suppSize);
  upArray = Pdtutil_Alloc (int, suppSize);
  downArray = Pdtutil_Alloc (int, suppSize);
  thenArray = Pdtutil_Alloc (int, suppSize);
  elseArray = Pdtutil_Alloc (int, suppSize);

  /*------------ Estimate Cofactor with Fast Euristic: Phase One  -----------*/

  for (i=0; i<suppSize; i++) {
    totalArray[i] = thenThenArray[i] = thenElseArray[i] = 0;
  }

  totalSize = Part_EstimateCofactorFast (f, 1, supp, suppSize,
    totalArray, thenThenArray, thenElseArray);

  /*------------ Estimate Cofactor with Fast Euristic: Phase Two ------------*/

  for (i=0; i<suppSize; i++) {
    totalArray[i] = elseThenArray[i] = elseElseArray[i] = 0;
  }

  totalSize = Part_EstimateCofactorFast (f, 0, supp, suppSize,
    totalArray, elseThenArray, elseElseArray);

  /*---------------------- Compute upArray and downArray --------------------*/

  upArray[0] = 0;
  for (i=1; i<suppSize; i++) {
    upArray[i] = upArray[i-1] + totalArray[i-1];
  }
  downArray[suppSize-1] = 0;
  for (i=suppSize-2; i>=0; i--) {
    downArray[i] = downArray[i+1] + totalArray[i+1];
  }

  /*--------------------- Compute thenArray and elseArray -------------------*/

  for (i=1; i<suppSize; i++) {
    if (thenThenArray[i] < elseThenArray[i]) {
      minThen = thenThenArray[i];
    } else {
      minThen = elseThenArray[i];
    }
    if (thenElseArray[i] < elseElseArray[i]) {
      minElse = thenElseArray[i];
    } else {
      minElse = elseElseArray[i];
    }

    thenArray[i] = downArray[i] + upArray[i] - minElse;
    elseArray[i] = downArray[i] + upArray[i] - minThen;

    /*
    *  Correction Factors Due to Sharing
    */ 

    elseArray[i] += ((int) (
      (((float) totalArray[i])/((float) totalSize)) *
      (((float) downArray[i])+((float) upArray[i]))
                        ));
    thenArray[i] += ((int) (
      (((float) totalArray[i])/((float) totalSize)) *
      (((float) downArray[i])+((float) upArray[i]))
                        ));
  }

  /*----------------------------- Evaluate Costs ----------------------------*/

  costRef = COST_LARGE;
  saveVar = NULL;

  var = Ddi_VarsetTop (supp);
  for (i=0; i<suppSize; i++) {
    if (totalArray[i] == 0) { 
      continue;
    }
    costNew = estimateCost (bddSize, thenArray[i], elseArray[i], i, suppSize);

#if 0
    tmp = Ddi_VarsetNext (supp);
    Ddi_Free (supp);
    supp = tmp;
    var = Ddi_VarsetTop (supp);
#else
    var = Ddi_IthVar (dd, Ddi_ReadInvPerm (dd, i));
#endif

#if 0
    {
    Ddi_Bdd_t *lit;

    fprintf (stdout, "id=%3d ", Ddi_VarIndex(var));

    fprintf (stdout, "CofSx=%d CofDx=%d ",
      Ddi_BddSize (Ddi_BddConstrain (f, (lit=Ddi_BddMakeLiteral(var,1)))),
      Ddi_BddSize (Ddi_BddConstrain (f, (lit=Ddi_BddMakeLiteral(var,0))))); 
    
    fprintf (stdout,
      "(tot,tT,eT,tE,eE)(%5d,%5d,%5d,%5d,%5d)\n",
      totalArray[i], thenThenArray[i], elseThenArray[i], thenElseArray[i],
      elseElseArray[i]);
 
    fprintf (stdout,
      "[up,down,t,e,cost][%5d,%5d,%5d,%5d,%5.3f]\n",
      upArray[i], downArray[i], thenArray[i], elseArray[i], costNew);
    }
#endif

    if (costNew < costRef) {
      costRef = costNew;
      saveVar = var;
    }
  }

  return (saveVar);
}

/**Function********************************************************************

  Synopsis    [Calls function for to count the number of nodes of the 
    cofactors: Fabio Somenzi Private Communication Solution]

  Description []

  SideEffects [none]

  SeeAlso     [varSelection, Part_EstimateCofactor]

******************************************************************************/

static void
estimateCofactorComplex (
  Ddi_Bdd_t *f         /* input BDD */,
  Ddi_Var_t *topVar   /* top variable */,
  int *thenSize       /* number nodes cofactors */,
  int *elseSize       /* number nodes cofactors */
  )
{
  Ddi_Bdd_t *lit;

  *thenSize = Part_EstimateCofactorComplex (f,
    (lit=Ddi_BddMakeLiteral(topVar,0)));
  Ddi_Free (lit);
  *elseSize = Part_EstimateCofactorComplex (f,
    (lit=Ddi_BddMakeLiteral(topVar,1))); 
  Ddi_Free (lit);
  
  *thenSize = *thenSize + 1;
  *elseSize = *elseSize + 1;
  
  return;
}


/**Function********************************************************************

  Synopsis    [Calls function for to count the number of nodes of the
    cofactors: Free BDD Solution]

  Description []

  SideEffects [none]

  SeeAlso     [varSelection, Part_EstimateCofactor]

******************************************************************************/

static void
estimateCofactorFreeOrder (
  Ddi_Bdd_t *f            /* input BDD */,
  Ddi_Var_t *topVar      /* top variable */,
  int *thenSize          /* number nodes cofactors */,
  int *elseSize          /* number nodes cofactors */,
  int *thenArray         /* thenArray */,
  int *elseArray         /* elseArray */
  )
{
  Ddi_Mgr_t *dd;
  Ddi_Bdd_t *lit;
  Ddi_Varset_t *supp;
  int suppSize, bddSize;

  /*--------------------------- Collect Statistics --------------------------*/

  dd = Ddi_ReadMgr (f);
  supp = Ddi_BddSupp (f);
  suppSize = Ddi_VarsetNum(supp);
  bddSize = Ddi_BddSize (f);

  *thenSize = Part_EstimateCofactorFreeOrder (f,
    (lit=Ddi_BddMakeLiteral(topVar,0)), supp, thenArray);
  Ddi_Free (lit);
  *elseSize = Part_EstimateCofactorFreeOrder (f,
    (lit=Ddi_BddMakeLiteral(topVar,1)), supp, elseArray); 
  Ddi_Free (lit);
  
  *thenSize = *thenSize + 1;
  *elseSize = *elseSize + 1;
  
  return;
}

/**Function********************************************************************

  Synopsis    [Comparison between cofactor, stq, fs solution]

  Description []

  SideEffects [none]

  SeeAlso     [varSelection, exactCofactor, estimateCofactor,
    estimateCofactor]

******************************************************************************/

static void
comparisonRoutine (
  Ddi_Bdd_t *f        /* input BDD */,
  Ddi_Var_t *topVar  /* top variable */,
  int *thenSize      /* number nodes cofactors */,
  int *elseSize      /* number nodes cofactors */
  )
{
  static int n_call = 0;
  static float err_stq_tot_0 = 0.0, err_stq_tot_1 = 0.0,
    err_fs_tot_0 = 0.0, err_fs_tot_1 = 0.0,
    time_c = 0.0, time_stq = 0.0, time_fs = 0.0;
  static float err_stq_0, err_stq_1, err_fs_0, err_fs_1;
  float ft;
  long startTime, currTime;
  int n_c_0, n_c_1, n_stq_0, n_stq_1, n_fs_0, n_fs_1;
  char st[100];
  
  if (n_call == 0) {
    fprintf (stdout, "t = time[sec], Sx & Dx = Node Number, [e] = error\n");
    fprintf (stdout,
	    "(Var)(t:Cof/StQ/FS)(Sx:Cof/StQ[e]/FS[e])(Dx:Cof/StQ[e]/FS[e])\n");
  }
  n_call++;
  
  fprintf (stdout, "(%d)(", Ddi_VarIndex(topVar));
  
  startTime = util_cpu_time();
  exactCofactor (f, topVar, &n_c_0, &n_c_1);
  currTime = util_cpu_time();
  strcpy (st, util_print_time(currTime - startTime));
  sscanf (st, "%f%*s", &ft);
  fprintf (stdout, "%.2f/", ft);
  time_c += ft;
  
  startTime = util_cpu_time();
  estimateCofactor (f, topVar, &n_stq_0, &n_stq_1);
  currTime = util_cpu_time();
  strcpy (st, util_print_time(currTime - startTime));
  sscanf (st, "%f%*s", &ft);
  fprintf (stdout, "%.2f/", ft);
  time_stq += ft;
  
  err_stq_0 = 100.0*((float)(n_stq_0-n_c_0))/((float)n_stq_0);
  err_stq_1 = 100.0*((float)(n_stq_1-n_c_1))/((float)n_stq_1);
  err_stq_tot_0 += fabs (err_stq_0);
  err_stq_tot_1 += fabs (err_stq_1);
  
  startTime = util_cpu_time();
  estimateCofactor (f, topVar, &n_fs_0, &n_fs_1);
  currTime = util_cpu_time();
  strcpy (st, util_print_time(currTime - startTime));
  sscanf (st, "%f%*s", &ft);
  fprintf (stdout, "%.2f)\n", ft);
  time_fs += ft;
  
  err_fs_0 = 100.0*((float)(n_fs_0-n_c_0))/((float)n_fs_0);
  err_fs_1 = 100.0*((float)(n_fs_1-n_c_1))/((float)n_fs_1);
  err_fs_tot_0 += fabs (err_fs_0);
  err_fs_tot_1 += fabs (err_fs_1);
  
  fprintf (stdout, "  (%d/%d[%.3f]/%d[%.3f])(%d/%d[%.3f]/%d[%.3f])\n",
	   n_c_0, n_stq_0, err_stq_0, n_fs_0, err_fs_0,
	   n_c_1, n_stq_1, err_stq_1, n_fs_1, err_fs_1);
  fprintf (stdout,
	   "   Errori medi: [stq (sx %.3f/dx %.3f)] [fs (sx %.3f/dx %.3f)]\n",
	   err_stq_tot_0/((float) n_call), 
	   err_stq_tot_1/((float) n_call), 
	   err_fs_tot_0/((float) n_call), 
	   err_fs_tot_1/((float) n_call) );
  fprintf (stdout,
	   "   Total Time: [c %.3f] [stq %.3f] [fs %.3f]\n",
	   time_c, time_stq, time_fs);
  fflush(stdout);
  
  *thenSize = n_c_0;
  *elseSize = n_c_1;
  
  return;
}


/**Function********************************************************************

  Synopsis    [Evaluating estimateCost of splitting choice]

  Description []

  SideEffects [none]

  SeeAlso     [partPartitionDisjSet]

******************************************************************************/

static float 
estimateCost (
  int size           /* Total Size */,
  int thenSize       /* Left Child Size */,
  int elseSize       /* Right Child Size */,
  int varPos         /* Variable Position */,
  int suppSize       /* Variable Number */
  )
{
  float w1 = 1.0;    /*w1 = 1.2*/
  float w2 = 0.6;    /*w2 = 0.4*/
  float w3 = 0.2;
  float thenSizeFloat, elseSizeFloat, sizeFloat, overall, unbalance,
    position, costNew;

  thenSizeFloat = (float) thenSize;
  elseSizeFloat = (float) elseSize;
  sizeFloat = (float) size;

  overall = ((fabs) (thenSizeFloat + elseSizeFloat /*-sizeFloat*/)) / sizeFloat;

  /*
  unbalance = ((fabs) (2*(thenSizeFloat-elseSizeFloat))) /
    (thenSizeFloat+elseSizeFloat);
  */
  unbalance = ((fabs) (thenSizeFloat-elseSizeFloat)) / sizeFloat;

  /*
    ratio = thenSize/elseSize; 
    ratio += (thenSize>=size) ? 10.0 : fsize / (fsize - thenSizeFloat);
    if (ratio > 12) {
    unbalance = ratio;
    }
  */

  /* n=0 NO correction (e.g., cofactor) */
  if (suppSize!=0) {
    position = ((float) ( /*suppSize - */ varPos)) / ((float) suppSize);
  } else {
    position = 0.0;
  }

  costNew = (w1 * overall) + (w2 * unbalance) + (w3 * position);
  
#if DISPLAY_WEIGHTS
  fprintf (stdout, "#%.3f#", costNew);
  fprintf (stdout, "({%d}%d,%d<%5.2f,%5.2f%5.2f>->%5.2f) ", i,thenSize,elseSize,
    (w1*(thenSize+elseSize-size))/size,(w2*unbalance),w3*((n-i)/n),costNew);
  fflush(stdout);        
#endif
  
  return (costNew);
}


/**Function********************************************************************

  Synopsis    [Select the best splitting variable]

  Description []

  SideEffects [done]

  SeeAlso     [partitionSetInt, cofactorBased, fsBased, stqBased
    comparisonRoutine]

******************************************************************************/

static float
estimateCostFreeOrder (
  int size         /* total size */,
  int thenSize     /* left child size */,
  int elseSize     /* right child size */,
  int *thenArray   /* left statistics */,
  int *elseArray   /* right statistics */,
  int varPos       /* Variable Position */,
  int suppSize     /* Number of Variables */
  )
{
  int i;
  float thenSizeFloat, elseSizeFloat, fs2, sizeFloat, costNew;

  /*------------------- To Balance Unbalance add cost/4 ---------------------*/

  costNew = estimateCost (size, thenSize, elseSize, varPos, suppSize) / 4;

  sizeFloat = (float) size;
  thenSizeFloat = (float) thenSize;
  elseSizeFloat = (float) elseSize;
  fs2 = 0.0;

  if ( ((fabs) (thenSizeFloat-elseSizeFloat)) >
    ((thenSizeFloat+elseSizeFloat)/2) ) {
    return (COST_LARGE);
    }
    
  for (i=0; i<suppSize; i++)
    fs2 = fs2 + (fabs (
                ((float) (thenArray[i])) / thenSizeFloat -
                ((float) (elseArray[i])) / elseSizeFloat
                ));
  /*
    fs2 = fs2 + (fabs (
                ((float) (thenArray[i])) -
                ((float) (elseArray[i]))
                ));
  */

  costNew = costNew + 1.0 - fs2;

  return (costNew);
}



/**Function********************************************************************

  Synopsis    [Selects the best splitting variable]

  Description []

  SideEffects [done]

  SeeAlso     [partPartitionDisjSet, exactCofactor, estimateCofactor,
    estimateCofactor, comparisonRoutine]

******************************************************************************/

static Ddi_Var_t *
varSelection (
  Ddi_Bdd_t *f                     /* Input BDD */,
  Ddi_Varset_t *careVars          /* Set of Variables */,
  Part_Method_e partitionMethod   /* Partition Method */
  )
{
  int i, j, bddSize, thenSize, elseSize, fail, suppSize;
  int *thenArray = NULL, *elseArray = NULL;
  float costRef, costNew;
  Ddi_Varset_t *supp, *allowedVars;
  Ddi_Var_t *topVar, *savev, *auxv;
  Ddi_Mgr_t *dd;
  
  /*-------------------------- Collect Statistics ---------------------------*/

  dd = Ddi_ReadMgr(f);

  bddSize = Ddi_BddSize (f);
  costRef = COST_LARGE;
  
  supp = Ddi_BddSupp (f);
  suppSize = Ddi_VarsetNum(supp);
  
  /*------------------------ Take Care-Variable set -------------------------*/

  if (careVars != NULL) {
    allowedVars = Ddi_VarsetIntersect (supp, careVars);  
  } else {
    allowedVars = Ddi_VarsetDup (supp);
  }

  /*----------------- Create Array For The FreeOrder Method -----------------*/

  if (partitionMethod==Part_MethodEstimateFreeOrder_c) {
    thenArray = Pdtutil_Alloc (int, suppSize);
    elseArray = Pdtutil_Alloc (int, suppSize);
  }

  /*--------------------------- Select the Variable -------------------------*/

  savev = NULL;
  i=0;
  fail = 0;
  
  while (!Ddi_VarsetIsVoid (allowedVars)) {

    Pdtutil_Assert (!Ddi_VarsetIsVoid (supp), "Variable set empty");

    topVar = Ddi_VarsetTop(supp);
    i++;
    Ddi_VarsetNextAcc (supp); 
    auxv = Ddi_VarsetTop(allowedVars);

    /* Skip Variables not in careVars */
    if (auxv != topVar) {
      continue;
    }

    Ddi_VarsetNextAcc (allowedVars); 
    
    switch (partitionMethod) {
      case Part_MethodCofactor_c:
	exactCofactor (f, topVar, &thenSize, &elseSize);
        costNew = estimateCost (bddSize, thenSize, elseSize, 0, 0);
	break;
      case Part_MethodEstimate_c:
	estimateCofactor (f, topVar, &thenSize, &elseSize);
        costNew = estimateCost (bddSize, thenSize, elseSize, i, suppSize);
	break;
      case Part_MethodEstimateComplex_c:
	estimateCofactorComplex (f, topVar, &thenSize, &elseSize);
        costNew = estimateCost (bddSize, thenSize, elseSize, i, suppSize);
	break;            
      case Part_MethodEstimateFreeOrder_c:
        for (j=0; j<suppSize; j++) {
          thenArray[j] = elseArray[j] = 0;
        }
	estimateCofactorFreeOrder (f, topVar, &thenSize, &elseSize,
	  thenArray, elseArray);
        costNew = estimateCostFreeOrder (bddSize, thenSize, elseSize,
	  thenArray, elseArray, i, suppSize);
	break;            
      case Part_MethodComparison_c:
	comparisonRoutine (f, topVar, &thenSize, &elseSize);
        costNew = estimateCost (bddSize, thenSize, elseSize, 0, 0);
	break;
      default:
	fprintf (stderr, "Error: Decomposition choice.\n");
	return (NULL);
	break;
    }

    if (costNew < costRef) {
      costRef = costNew;
      savev = topVar;
      fail = 0;                           
    } else {
      fail++;
    }
    
#if 0
    if (fail > 8) {      /* introduced to reduce iterations */
      break;
    }
#endif
  }
  
#if DISPLAY_WEIGHTS
  (void) printf("\n selected: {%d}\n ", Ddi_VarIndex(savev));
  fflush(stdout);
#endif
  
  Ddi_Free (supp); 
  Ddi_Free (allowedVars); 

  return (savev);
}


/**Function********************************************************************

  Synopsis    [Manual selection of best splitting variable]

  Description []

  SideEffects [none]

  SeeAlso     [partPartitionDisjSet]

******************************************************************************/

static Ddi_Var_t *
varSelectionManual (
  Ddi_Bdd_t *f      /* input BDD */
  )
{
  int i, flag, n;
  Ddi_Varset_t *supp;
  Ddi_Var_t *topVar = NULL, *v; 
  char row[100], name[100];
  Ddi_Mgr_t *dd = Ddi_ReadMgr(f);    /* dd manager */

  /*-------------  Read Index Position or Name of the variable --------------*/

  n = (-1);
  do {
    fprintf (stdout, "Choose Variable by (0) Position, (1) Index, (2) Name: ");
    fgets (row, 99, stdin);
    flag = atoi (row);
   
    switch (flag) {
      case 0:
        fprintf (stdout, "Which Position (0=top var, ...): ");
        fgets (row, 99, stdin);
        n = atoi (row);
        break;
      case 1:
        fprintf (stdout, "Which index: ");
        fgets (row, 99, stdin);
        n = atoi (row);
        break;
      case 2:
        fprintf (stdout, "Which name: ");
        fgets (row, 99, stdin);
        sscanf (row, "%s", name);
        v = Ddi_VarFromName (dd, name);
        if (v == NULL) {
          fprintf (stdout, "Variable name not found: %s\n", name);
          n = -1;
        }
        else {
          n = Ddi_VarIndex(v);
	}
        break;
      default:
        Pdtutil_Warning (1, "Wrong choice.");
        break;
    }
  } while (n<0);

  /*------------------------  Select the Variable ----------------------------*/

  supp = Ddi_BddSupp (f);
  
  i=0;
  while (!Ddi_VarsetIsVoid (supp)) {
    topVar = Ddi_VarsetTop(supp);
    Ddi_VarsetNextAcc(supp); 
    
    if (flag == 0 && i == n) {
      break;
    }
    if (flag == 1 && Ddi_VarIndex (topVar) == n) {
      break;
    }
    if (flag == 2 && Ddi_VarIndex (topVar) == n) {
      break;
    }
    
    i++;
  }
  
  return (topVar);
}
