/**CFile***********************************************************************

  FileName    [travTrav.c]

  PackageName [trav]

  Synopsis    [Functions to traverse a FSM]

  Description [External procedures included in this file are:
    <ul>
    <li>Trav_Traversal()
    </ul> 
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

#include "travInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define PRUNING_NUMBER_MAX 20
#define THRESHOLD_FACTOR 2
#define TIME_COST_FACTOR 0.5
#define MEM_COST_FACTOR 0.25

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

/* if 0 allows don't cares within input patterns */
#define NO_PI_DONT_CARES 0

/*
 *  If 0 allows don't cares within initial state
 *  At present no don't cares allowed.
 *  So don't change next definition for now: not yet supported
 */
#define NO_IS_DONT_CARES 1

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [FSM traversal given a transition relation]

  Description [The pseudo-code of traversal algorithm is:<br>
    <code>
    traverse ( delta , S0 )<br>
      {<br>
      reached = from = S0;<br> 
      do<br> 
        {<br>
        to = Img  ( delta , from );<br>
        new = to - reached;<br>
        reached = reached + new;<br>
        from = new;<br>
        }<br>
      while ( new!= 0 )<br>
      <p>
      return reached ;<br>
    }<br>
    </code>
    We use the following notations:
    <ul>
    <li>from = initial set of states
    <li>to  = set of reached state in one step from current states
    <li>new  = new reached states 
    <li>reached  = set of reached states from 0 up to current
    iteration
    </ul>]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

Ddi_Bdd_t *
Trav_Traversal (
  Trav_Mgr_t *travMgr  /* Traversal Manager */
  )
{
  Ddi_Mgr_t *dd, *ddR;
  Ddi_Vararray_t *psv, *nsv;
  Ddi_Varset_t *quantify, *inputs;
  Ddi_Varset_t *part_vars = NULL;
  Ddi_Bdd_t *newr, *from, *to, *reached;
  Tr_Tr_t *trActive;
  Pdtutil_VerbLevel_e verbosity, verbositySave;
  double nStates;
  long initialTime, imgTime, currTime1, currTime2;
  long currMem1, currMem2;
  int step, iterNumberMax, travContinue, toIncluded,
      logPeriod, savePeriod, toSize, l;
  char *periodNameSave, *nameTmp;
  int extRef, nodeId;

  /*
   *  Check for Correctness and Get Basic Objects
   */

  Pdtutil_Assert (travMgr!=NULL, "NULL traversal manager");

  dd = travMgr->ddiMgrTr;
  ddR = travMgr->ddiMgrR;

  extRef = Ddi_MgrReadExtRef(ddR);

  trActive = Tr_TrDup (Trav_MgrReadTr (travMgr));

  psv = Trav_MgrReadPS (travMgr);
  nsv = Trav_MgrReadNS (travMgr);

  quantify = Ddi_VarsetMakeFromArray (psv);
  if ((Trav_MgrReadSmoothVar (travMgr) == TRAV_SMOOTH_SX) 
    && (Trav_MgrReadI (travMgr) != NULL)) {
    inputs = Ddi_VarsetMakeFromArray (Trav_MgrReadI (travMgr));
    Ddi_VarsetUnionAcc (quantify, inputs);
    Ddi_Free(inputs);
  }

  verbosity = Trav_MgrReadVerbosity (travMgr);
  iterNumberMax = Trav_MgrReadMaxIter (travMgr);
  logPeriod = Trav_MgrReadLogPeriod (travMgr);

  savePeriod = Trav_MgrReadSavePeriod (travMgr);
  if (Trav_MgrReadSavePeriodName (travMgr) != NULL) {
    periodNameSave = Pdtutil_StrDup (Trav_MgrReadSavePeriodName (travMgr));
  } else {
    periodNameSave = Pdtutil_StrDup ("");
  }

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    if (ddR != dd) {
      fprintf (stdout, "Two dd managers used for traversal\n");
      fflush (stdout);
    }
  }

  reached = Ddi_BddCopy (ddR, travMgr->reached);
  from = Ddi_BddCopy (ddR, travMgr->from);
  Ddi_Free(travMgr->from);
  Ddi_Free(travMgr->reached);
  travMgr->from = from;
  travMgr->reached = reached;

  /*
   *  Initialize frontier set array if option enabled
   */

  if ((travMgr->settings.keepNew) && (travMgr->newi==NULL)) {
    travMgr->newi = Ddi_BddarrayAlloc (ddR,1);
    Pdtutil_Assert(travMgr->newi!=NULL, "NULL newi array in trav manager");

    Ddi_BddarrayWrite (travMgr->newi, 0, travMgr->reached);
  }

  /*
   *  Final Settings Before Cycle
   */

  initialTime = util_cpu_time ();
  verbositySave = verbosity;
  step = 1;
  travContinue = (iterNumberMax<0) || (step<=iterNumberMax);

#if 1
  if (Ddi_MetaActive(dd)) {
    Ddi_BddSetMeta(travMgr->reached);
    Ddi_BddSetMeta(travMgr->from);
    Trav_MgrSetFromSelect (travMgr,Trav_FromSelectNew_c);
  }
#endif
  /*---------------------- Traversal Cycle ... Start ------------------------*/

  nodeId = Ddi_MgrReadCurrNodeId(ddR);

  while (travContinue) {

    /*
     *  Cope with logPeriod through verbosity level
     */

    if ((step%logPeriod) == 0) {
      verbosity = travMgr->settings.verbosity = verbositySave;
    } else { 
      verbosity = travMgr->settings.verbosity = Pdtutil_VerbLevelNone_c;
    }

    /*
     *  Check Assertion
     */

    if (Trav_MgrReadAssert (travMgr) != NULL) {
      if (!Ddi_BddIncluded (travMgr->reached, Trav_MgrReadAssert (travMgr))) {
        if (verbositySave >= Pdtutil_VerbLevelUsrMax_c) {
          fprintf (stdout, "TravLevel %d: Assertion failed!\n",
            travMgr->level);
          fflush (stdout);
        }
        Trav_MgrSetAssertFlag (travMgr, 1);
        break;
      }
    }

    /* 
     *  Print Statistics for this Iteration - Part 1
     */

    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "TravLevel %d: ", travMgr->level);
      fprintf (stdout, "[|Tr|: %d] ", Ddi_BddSize (Tr_TrBdd(trActive)));
      fprintf (stdout, "[|From|: %d]", Ddi_BddSize (travMgr->from));
      if (verbosity >= Pdtutil_VerbLevelAppMed_c) { 
        fprintf (stdout, "\n");
      }
      fflush (stdout);
    }

    /*
     *  Compute image of next state function: to = Img (TR, from)
     */

    currMem1 = Ddi_ReadMemoryInUse (dd) / 1024;
    currTime1 = util_cpu_time ();
    to = Tr_Img (trActive, travMgr->from);
    currTime2 = util_cpu_time ();
    imgTime = currTime2 - currTime1;
    currMem2 = Ddi_ReadMemoryInUse (dd) / 1024;
    toSize = Ddi_BddSize (to);

    /*
     *  Check Fixt Point
     */

    if (Ddi_BddIncluded (to, travMgr->reached)) {
      toIncluded = 1;
    } else {
      toIncluded = 0;
    }

    /*
     *  Prepare Sets for the New Iteration
     */

    Ddi_MgrAutodynSuspend(dd);

    /* Compute new from according to the selection option
       and transfer to main manager */
    Ddi_Free (travMgr->from);
    travMgr->from = TravFromCompute (to, travMgr->reached,
      Trav_MgrReadFromSelect (travMgr));

    if (Ddi_MetaActive(dd)) {
      if (Ddi_BddIsZero (travMgr->from)) {
        toIncluded = 1;
      } else {
        toIncluded = 0;
      }
    }

#if 0
    if (Ddi_MetaActive(dd)) {
      Ddi_Varset_t *novars = Ddi_VarsetVoid(dd);
      fprintf (stdout, "[|R(meta)|: %d]", 
        Ddi_BddSize (travMgr->reached));
      fflush(stdout);
      Ddi_BddExistAcc(travMgr->reached,novars);
      fprintf (stdout, "[|R(meta2)|: %d]", 
        Ddi_BddSize (travMgr->reached));
      fflush(stdout);
      Ddi_Free(novars);
    }
#endif

    /* Keep frontier set if option enabled */
    if (toIncluded==0 && travMgr->settings.keepNew) {
      Pdtutil_Assert (travMgr->newi!=NULL, "NULL newi array in trav manager");
      newr = TravFromCompute (to, travMgr->reached, Trav_FromSelectNew_c);
      Ddi_BddSetMono(newr);
      Ddi_BddarrayWrite (travMgr->newi, travMgr->level, newr);
      Ddi_Free (newr);
    }

#if 0
    Ddi_BddSetMono(to);
#endif

    /* Update the Set of Reached States */	
    travMgr->reached = Ddi_BddEvalFree (Ddi_BddOr (travMgr->reached, to), 
      travMgr->reached);

#if 0
  if (Ddi_MetaActive(dd)) {
    Ddi_Varset_t *novars = Ddi_VarsetVoid(dd);
    fprintf (stdout, "[|F,R(meta)|: %d,%d]", 
      Ddi_BddSize (travMgr->from), Ddi_BddSize (travMgr->reached));
    fflush(stdout);
#if 1
    Ddi_BddExistAcc(travMgr->reached,novars);
    Ddi_BddExistAcc(travMgr->from,novars);
    fprintf (stdout, "[|F,R(meta2)|: %d,%d]", 
      Ddi_BddSize (travMgr->from), Ddi_BddSize (travMgr->reached));
    fflush(stdout);
#endif
#if 0
    Ddi_BddSetMono(travMgr->reached);
    Ddi_BddSetMono(travMgr->from);
    nStates = Ddi_CountMinterm(travMgr->reached, Ddi_VararrayNum(psv));
    fprintf (stdout, "[|F,R(mono)|: %d,%d]", 
      Ddi_BddSize (travMgr->from), Ddi_BddSize (travMgr->reached));
    Ddi_BddSetMeta(travMgr->reached);
    Ddi_BddSetMeta(travMgr->from);
    fprintf (stdout, "[|F,R(meta)|: %d,%d]\n", 
      Ddi_BddSize (travMgr->from), Ddi_BddSize (travMgr->reached));
    fflush(stdout);
    fprintf (stdout, "[#ReachedStates: %g]", nStates);
#endif
    fprintf (stdout, "\n");
    Ddi_Free(novars);
  }
#endif

    Ddi_MgrAutodynResume(dd);

    /* 
     *  Print Statistics for this Iteration - Part 2
     */

    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
        fprintf (stdout, "\n");
      }
      fprintf (stdout, "[|To|: %d]", Ddi_BddSize (to));
      fprintf (stdout, "[|Reached|: %d]", Ddi_BddSize (travMgr->reached));

      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        nStates = Ddi_CountMinterm(travMgr->reached, Ddi_VararrayNum(psv));
        fprintf (stdout, "[#ReachedStates: %g]", nStates);
      }

      fprintf (stdout, "\n");
      fprintf (stdout, "(ImgTime: %s)", util_print_time (imgTime));
      fprintf (stdout, "(TotalTime: %s)\n",
        util_print_time (currTime2-initialTime));

      if (verbosity >= Pdtutil_VerbLevelAppMin_c) {
        fprintf (stdout,
          "{ImgMemory: %ld Kbytes} {TotalMemory: %ld Kbytes} {DDI-DD Num: %d}\n",
          currMem2-currMem1, currMem2, Ddi_MgrReadExtRef (dd));
        fprintf (stdout, 
          "{Peak live: %u nodes} {Unique Table: %u nodes} {Cache: %u slots}\n",
          Ddi_ReadPeakLiveNodeCount(dd), 
          Ddi_ReadKeys(dd), Ddi_ReadCacheSlots(dd));
      }

      fflush (stdout);
    }

    /*
     *  Free the to set
     */

    Ddi_Free (to);

    /*
     * Save BDDs and Variable Orders every "savePeriod" cycles
     */

    if (savePeriod!=(-1) && (step%savePeriod) == 0) {
      l = strlen (periodNameSave) + 20;
      nameTmp = Pdtutil_Alloc (char, l);

      /* Save Ord */
      sprintf (nameTmp, "%sord-l%d.ord", periodNameSave, travMgr->level);
      Ddi_MgrOrdWrite (dd, nameTmp, NULL, Pdtutil_VariableOrderComment_c);

      /* Save From */
      sprintf (nameTmp, "%sfrom-l%d.bdd", periodNameSave, travMgr->level);
      Ddi_BddStore (travMgr->from, nameTmp, 
        DDDMP_MODE_BINARY/*DDDMP_MODE_TEXT*/, nameTmp, NULL);
 
      /* Save Reached */
      sprintf (nameTmp, "%sreached-l%d.bdd", periodNameSave, travMgr->level);
      Ddi_BddStore (travMgr->reached, nameTmp, 
        DDDMP_MODE_BINARY/*DDDMP_MODE_TEXT*/, nameTmp, NULL);
      
      Pdtutil_Free (nameTmp);
    }

    /*
     *  Check End Of Cycle: Fix Point for Closure, etc.
     */

    travContinue = (iterNumberMax<0) || (step<iterNumberMax);

    if (toIncluded == 1) {
      travContinue = 0;
    }

    step++;
    travMgr->level++;

  }

  /*----------------------- Traversal Cycle ... End -------------------------*/

  verbosity = travMgr->settings.verbosity = verbositySave;

  Pdtutil_Free (periodNameSave);

  if (part_vars != NULL) {
    Ddi_Free(part_vars);
  }

  if (travMgr->ddiMgrAux != NULL) {
    Ddi_MgrQuit (travMgr->ddiMgrAux);
    travMgr->ddiMgrAux = NULL;
  }

  Ddi_Free (quantify);
  Tr_TrFree (trActive);

  if (travMgr->newi != NULL) {
    Ddi_MgrCheckExtRef(ddR,extRef+1);
  }
  else {
    Ddi_MgrCheckExtRef(ddR,extRef);
#if 0
    Ddi_MgrPrintExtRef(ddR,nodeId);
#endif
  }

  if (Ddi_MetaActive(dd)) {
    Ddi_BddFromMeta(travMgr->reached);
  }

  return (travMgr->reached);
}

/**Function********************************************************************

  Synopsis    [Generation of a mismatch input sequence]

  Description [
              ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

Ddi_Bddarray_t *
Trav_MismatchPat (
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  Tr_Tr_t *TR               /* Transition relation */,
  Ddi_Bdd_t *firstC         /* constrain for start set */,
  Ddi_Bdd_t *lastC          /* constrain for last set */,
  Ddi_Bdd_t **startp        /* Pointer to start set */,
  Ddi_Bdd_t **endp          /* Pointer to end set */,
  Ddi_Bddarray_t *newi      /* Frontier sets */,
  Ddi_Vararray_t *psv      /* Array of present state variables */,
  Ddi_Vararray_t *nsv      /* Array of next state variables */,
  Ddi_Varset_t *pivars      /* Set of pattern (input) variables */
  )
{
  int i, depth;
  Ddi_Bdd_t *from, *Bset, *Bcube, *currpat, *newr;
  Ddi_Bddarray_t *totpat;
  Ddi_Varset_t *psvars, *auxvars;
  long startTime;

  Pdtutil_VerbLevel_e verbosity;

  Ddi_Mgr_t *dd;                /* TR DD manager */
  Ddi_Mgr_t *ddR;               /* reached DD manager */

  Pdtutil_Assert (travMgr!=NULL, "NULL traversal manager");
  Pdtutil_Assert(newi!=NULL,"NULL newi array in trav manager");

  dd = travMgr->ddiMgrTr;
  ddR = travMgr->ddiMgrR;

  /* keep PIs while computing image */
  Tr_MgrSetImgSmoothPi(Tr_TrMgr(TR),0);

  if (Trav_MgrReadAux (travMgr) == NULL) {
    auxvars = Ddi_VarsetVoid (ddR);
  } else {
    auxvars = Ddi_VarsetMakeFromArray (Trav_MgrReadAux (travMgr));
  }

  depth = Ddi_BddarrayNum(newi);

  verbosity = Trav_MgrReadVerbosity (travMgr);

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    fprintf (stdout, "Generating a mismatch sequence of length %d\n", depth);
    fprintf (stdout, "From last to first frontier set\n");
    if (ddR != dd) {
      fprintf (stdout, "Two dd managers used\n");
    }
    fflush (stdout);
  }

  startTime = util_cpu_time();

  /*
   * Image keeps input values to tie next states and inputs
   * so only present state vars will be quantified out by image
   */

  psvars = Ddi_VarsetMakeFromArray (psv);

  if (endp!=NULL) {
    /*
     *  Save end set if required
     */

     *endp = Ddi_BddDup(Ddi_MgrReadZero(ddR));
  }
  if (startp!=NULL) {
    /*
     *  Save start set if required
     */

     *startp = Ddi_BddDup(Ddi_MgrReadZero(ddR));
  }


  /*----------------------- Generate START set  -----------------------*/

  if (firstC != NULL) {
    Bset  = Ddi_BddCopy (ddR, firstC);
  } else {
    Bset = Ddi_BddDup (Ddi_MgrReadOne (ddR));
  }

  /*----------------------------   START   ----------------------------*/


  totpat = Ddi_BddarrayAlloc(ddR,0);

  for (i=depth-1; i>=0; i--) {

    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "level %d: ", i);
      fprintf (stdout, "[|Bset|: %d]\n", Ddi_BddSize(Bset));
      fflush (stdout);
    }

    /* 
     * get current frontier set 
     */

    newr   = Ddi_BddarrayRead (newi, i);

    /* 
     * AND backward reached set with current frontier 
     */

    Ddi_BddAndAcc(Bset,newr);
    if (Ddi_BddIsZero(Bset)) {
      fprintf (stdout, "VOID set found: no mismatch sequence possible\n");
      fflush (stdout);
      return (NULL);
    }

    Pdtutil_Assert(!Ddi_BddIsZero(Bset),"VOID set found computing mismatch");

    /* 
     *  Pick up one cube randomly
     */

    Bcube = Ddi_BddPickOneCube(Bset);
    Ddi_Free(Bset);

    /* 
     *  Remove non input vars for current pattern
     *  and save current pattern in result array
     *  don't do if first iteration and firstC not specified
     */

    if ((firstC != NULL)||(i<(depth-1))) {
      currpat = Ddi_BddExist(Bcube,psvars);
#if NO_PI_DONT_CARES
      currpat = Ddi_BddEvalFree(Ddi_BddPickOneMinterm(currpat,pivars),currpat);
#endif
      if (verbosity >= Pdtutil_VerbLevelUsrMed_c) {
        Ddi_BddPrintCubes (currpat, pivars, 1, 0, NULL, stdout);
      }
      Ddi_BddarrayInsertLast(totpat,currpat);
      Ddi_Free(currpat);
    }

    if ((startp!=NULL)&&(i==depth-1)) {
      /*
       * Save start set if required
       */
      Ddi_Free(*startp); /* free dup of ZERO */
      *startp = Ddi_BddExist(Bcube,pivars);
#if NO_IS_DONT_CARES
      *startp = Ddi_BddEvalFree(Ddi_BddPickOneMinterm(*startp,psvars),*startp);
      Ddi_BddAndAcc(Bcube,*startp);
      if (verbosity >= Pdtutil_VerbLevelUsrMed_c) {
        fprintf (stdout, "startpoint\n");
        Ddi_BddPrintCubes (*startp, psvars, 1, 0, NULL, stdout);
      }
#endif
      /* remove auxvars */
      *startp = Ddi_BddEvalFree (Ddi_BddExist (*startp, auxvars), *startp);
    }

    /*
     * Compute new Bset for next iteration
     */

    Bset = Ddi_BddExist(Bcube,pivars);
    Ddi_Free(Bcube);
    if (i>0) {
      from = Bset;
      Bset = Tr_Img (TR, from);
      Ddi_BddSetMono(Bset);
      Ddi_Free(from);
    }

  }

  if (lastC!=NULL) {
    /* 
     * a constrain is specified for the last step 
     * select proper cube and generate pattern
     */
    Bcube = Ddi_BddAnd(Bset,lastC);
    Bcube = Ddi_BddEvalFree(Ddi_BddPickOneCube(Bcube),Bcube);
    currpat = Ddi_BddExist(Bcube,psvars);
#if NO_PI_DONT_CARES
    currpat = Ddi_BddEvalFree(Ddi_BddPickOneMinterm(currpat,pivars),currpat);
#endif
    if (verbosity >= Pdtutil_VerbLevelUsrMed_c) {
      fprintf (stdout, "Input for last constrain\n");
      fflush (stdout);
      Ddi_BddPrintCubes (currpat, pivars, 1, 0, NULL, stdout);
    }
    Ddi_BddarrayInsertLast(totpat,currpat);
    Ddi_Free(currpat);  
    Ddi_Free(Bset);
    Bset = Ddi_BddExist(Bcube,pivars);
    Ddi_Free(Bcube);
  }
  if (endp!=NULL) {
    /*
     * Save start set if required
     */
    Ddi_Free(*endp); /* free dup of ZERO */
    *endp = Ddi_BddExist(Bset,pivars);
#if NO_IS_DONT_CARES
    *endp = Ddi_BddEvalFree(Ddi_BddPickOneMinterm(*endp,psvars),*endp);
    if (verbosity >= Pdtutil_VerbLevelUsrMed_c) {
      fprintf (stdout, "endpoint\n");
      fflush (stdout);
      Ddi_BddPrintCubes (*endp, psvars, 1, 0, NULL, stdout);
    }
#endif
  }

  Ddi_Free (psvars);
  Ddi_Free (Bset);
  Ddi_Free (auxvars);

  /*-------------------- Mismatch generation ... End ------------------------*/

  return (totpat);
}

/**Function********************************************************************

  Synopsis    [Find a universal alignment sequence]

  Description [Compute a universal alignment sequence using Pixley's
               algorithm: ICCAD'91, DAC'92.
               The algorithm works knowing the set of rings or frontier
               sets.
               The goal is the innermost ring. The outermost one must be 0.
              ]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

Ddi_Bddarray_t *
Trav_UnivAlignPat (
  Trav_Mgr_t *travMgr      /* Traversal Manager */,
  Tr_Tr_t   *TR             /* Transition relation */,
  Ddi_Bdd_t *goal           /* Destination set */,
  Ddi_Bdd_t **endp          /* Pointer to end set */,
  Ddi_Bddarray_t *rings     /* Frontier or ring sets */,
  Ddi_Vararray_t *psv      /* Array of present state variables */,
  Ddi_Vararray_t *nsv      /* Array of next state variables */,
  Ddi_Varset_t *pivars     /* Set of pattern (input) variables */,
  int maxDepth             /* maximum depth allowed for the sequence */
)
{
  int i, minDepth, currDepth;
  Ddi_Bdd_t *ring, *from, *to, *sum, *Bset, *Bcube, 
    *currpat, *prevpat, *inner, *advance, *regress, *covered, *leftover, 
    *goalBar;
  Tr_Tr_t *TrAux;
  Ddi_Bddarray_t *totpat, *ringsX;
  Ddi_Varset_t *aux, *quantify, *psvars;
  long startTime;

  Pdtutil_VerbLevel_e verbosity;

  Ddi_Mgr_t *dd;                /* TR DD manager */
  Ddi_Mgr_t *ddR;               /* reached DD manager */

  Pdtutil_Assert (travMgr!=NULL, "NULL traversal manager");
  Pdtutil_Assert (rings!=NULL,"NULL rings array in trav manager");

  dd = travMgr->ddiMgrTr;
  ddR = travMgr->ddiMgrR;

  minDepth = Ddi_BddarrayNum (rings);
  Pdtutil_Assert (minDepth>0,"# of rings is <= 0!");

  verbosity = Trav_MgrReadVerbosity (travMgr);

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    fprintf (stdout, "Generating an alignment sequence (minimum length %d)\n",
      minDepth);
    fprintf (stdout, "From outer to inner frontier sets\n");
    if (ddR != dd) {
      fprintf (stdout, "Two dd managers used\n");
    }
    fflush (stdout);
  }

  startTime = util_cpu_time();

  /*
   * Image keeps input values to tie next states and inputs
   * so only present state vars will be quantified out by image
   */
  quantify = Ddi_VarsetMakeFromArray(psv);
  Ddi_VarsetUnionAcc(quantify,pivars);

  /*----------------------------   START   ----------------------------*/

  sum = Ddi_BddDup(Ddi_MgrReadZero(ddR));
  for (i=0; i<minDepth; i++)
    sum = Ddi_BddEvalFree(Ddi_BddOr(sum,Ddi_BddarrayRead(rings,i)),sum);
  if (!Ddi_BddIsOne(sum)) {
    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "There is NO universal alignment sequence\n");
#if 0
      fprintf (stdout, "Generating a sequence for all possible states\n");
#endif
      fflush (stdout);
    }
#if 1
    return (NULL);
#endif
  }

  totpat = Ddi_BddarrayAlloc(ddR,0);
  leftover = Ddi_BddDup(sum);
  Ddi_Free(sum);

  psvars = Ddi_VarsetMakeFromArray(psv);
  TrAux = Tr_TrReverse(TR);

  /* keep PIs while computing image */
  Tr_MgrSetImgSmoothPi(Tr_TrMgr(TR),0);

  inner = Ddi_BddDup(Ddi_BddarrayRead(rings,0));
  advance = Ddi_BddDup(goal);

  ringsX = Ddi_BddarrayAlloc(ddR,minDepth);
  Ddi_BddarrayWrite(ringsX,0,goal);

  for (i=1; i<minDepth; i++) {
    ring = Ddi_BddarrayRead(rings,i);
    /* do preimage of inner ring, keeping input vars */

    if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
      fprintf (stdout, "\n\nUniv alignment: computing full ring %d\n", i);
      fflush (stdout);
    }

#if 0
    /* now partitioned image done using library img func */
    to = Ddi_BddMakePartDisjVoid(ddR);
    for (j=0;j<Ddi_BddPartNum(TrAux);j++) {
      if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
        fprintf (stdout, "\nTR Partition %d\n", j);
        fflush (stdout);
      }
      TR_i = Ddi_BddPartRead(TrAux,j);
      /* keep inactive PIs unchanged: remove from inner only active PIs */
#if 0
      pi_i = Ddi_BddSupp(TR_i);
      Ddi_VarsetIntersectAcc(pivars,pi_i);
      from = Ddi_BddExist(inner,pi_i);
#else
      pi_i = Ddi_VarsetDup(pivars);
      from = Ddi_BddExist(inner,pi_i);
#endif
      to_i = Trav_ImgConjPartTr (travMgr, TR_i, psv, nsv, from, psvars);
      Ddi_Free (from);
      Ddi_Free(pi_i);
      if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
        fprintf (stdout, "[%d]", Ddi_BddSize (to_i));
        fflush (stdout);
      }
      Ddi_BddPartInsertLast(to, to_i);
      Ddi_Free (to_i);
    }
#else
    from = Ddi_BddExist(inner,pivars);
    to = Tr_Img(TrAux,from);
    Ddi_Free (from);
#endif

    if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
      fprintf (stdout, "\n|to|: %d\n", Ddi_BddSize (to));
      fflush (stdout);
    }

    Ddi_Free(inner);
    /* set new inner */
    inner = to;

    /* AND with ring */
    ring = Ddi_BddAnd(to,ring);
    Ddi_BddSetMono(ring);
    if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
      fprintf (stdout, "\n|ring[%d]|: %d\n", i, Ddi_BddSize (ring));
      fflush (stdout);
    }
    /* add to advance and ringsX */
    Ddi_BddarrayWrite(ringsX,i,ring);
    Ddi_BddOrAcc(ring,advance);
    Ddi_Free(ring);
  }

  Tr_TrFree(TrAux);
  regress = Ddi_BddNot(advance);

  for (currDepth=0; (!Ddi_BddIsZero(leftover)); currDepth++) {

    if ((maxDepth>0)&&(currDepth>maxDepth)) {
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "Max requested depth reached: %d\n", maxDepth);
        fprintf (stdout, "Process interrupted and solution non complete\n");
        fflush (stdout);
      }
      break;
    }

#if 0
    fprintf (stdout, "\nLeftover: ");
    aux = Ddi_BddSupp(leftover);
    Ddi_VarsetPrint (aux, -1, NULL, stdout);
    Ddi_Free(aux);
    Ddi_BddPrintCubes (leftover, NULL, 100, 0, NULL, stdout);
#endif

#if 0

    if (i==0) {
      /* only check goal on inner ring */
      Ddi_BddAndAcc(Bset,goal);
    }
    else {

      /* 
       * get inner frontier set in the next state space
       */
      inner = Ddi_BddSwapVars(Ddi_BddarrayRead(rings,i-1),psv,nsv);

      /* 
       * add input variables
       */
      TrBdd = Ddi_BddDup(Tr_TrBdd(TR));
      Ddi_BddSetPartConj(TrBdd);
      Ddi_BddPartInsert(TrBdd,0,inner);
      Ddi_BddPartInsert(TrBdd,0,Bset);
      aux = Ddi_VarsetMakeFromArray(nsv);

#if 0
      fprintf (stdout, "\nTr-aux: ");
      aux = Ddi_BddSupp(TrBdd);
      Ddi_VarsetPrint (aux, -1, NULL, stdout);
      Ddi_Free(aux);
#endif

      Ddi_Free(Bset);
      Bset = Ddi_BddExist(TrBdd,aux);

      Ddi_Free(inner);
      Ddi_Free(TrBdd);
      Ddi_Free(aux);
    }

#endif

    /* check for an input making progress for all states in leftover */
    Bset = Ddi_BddAndExist(leftover,regress,psvars);
    Ddi_BddNotAcc(Bset);

    /* try to advance part of leftover */
    if (Ddi_BddIsZero(Bset)) {

      if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
        fprintf (stdout, "PARTIAL!\n");
        fflush (stdout);
      }

      /* 
       *  Find inner non null intersection btw leftover and rings 
       */

      Bset = Ddi_BddDup(Ddi_MgrReadZero(ddR));
      for (i=0; i<minDepth; i++) {
        Ddi_Free(Bset);
        Bset = Ddi_BddAnd(leftover,Ddi_BddarrayRead(ringsX,i));
        if (!Ddi_BddIsZero(Bset))
          break;
      }

      Pdtutil_Assert (!Ddi_BddIsZero(Bset), "ZERO Bset found in alignment");
  
      if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
        fprintf (stdout, "iteration %d - progress for ring %d - ", 
          currDepth, i);
        fprintf (stdout, "[|set|: %d]\n", Ddi_BddSize(Bset));
        fflush (stdout);
      }

    }
    else {
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "iteration %d - progress for all states - ", 
          currDepth);
        fprintf (stdout, "[|set|: %d]\n", Ddi_BddSize(Bset));
        fflush (stdout);
      }
    }
    /* 
     * pick up one cube trying to maximize improvement
     */

    Bcube = Ddi_BddPickOneCube(Bset);

    /* 
     *  remove non input vars for current pattern
     *  pick one minterm
     *  and save current pattern in result array
     */

    aux = Ddi_BddSupp(Bcube);

    if (currDepth==0) {
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "NO previous PATTERN.\n");
        fflush (stdout);
      }
      prevpat = Ddi_MgrReadOne(ddR);
    }
    else {
      if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
        fprintf (stdout, "Previous PATTERN exists.\n");
        fflush (stdout);
      }
      prevpat = Ddi_BddarrayRead(totpat,currDepth-1);
    }

    /* Minimize input transitions */
    /* keep previous pattern vars free in Bcube */
    prevpat = Ddi_BddExist(prevpat,aux);
    /* set free vars in Bcube according to prevpat */
    Ddi_BddAndAcc(Bcube,prevpat);
    Ddi_Free(prevpat);

    Ddi_VarsetDiffAcc(aux,pivars);
    currpat = Ddi_BddExist(Bcube,aux);
    Ddi_Free(aux);

    currpat = Ddi_BddEvalFree(Ddi_BddPickOneMinterm(currpat,pivars),currpat);
    Ddi_BddarrayInsertLast(totpat,currpat);

    if (verbosity >= Pdtutil_VerbLevelUsrMed_c) {
      fprintf (stdout, "pat[%3d]: ",currDepth);
      Ddi_BddPrintCubes (currpat, pivars, 1, 0, NULL, stdout);
    }

    /*
     * Compute states where goal is satisfied
     * remove them from leftover
     */

    covered = Ddi_BddAndExist(goal,currpat,pivars);
    if (!Ddi_BddIsZero(covered)) {
      goalBar = Ddi_BddNot(covered);
      Ddi_BddAndAcc(leftover,goalBar);
      Ddi_Free(goalBar);
    }

    Ddi_Free(covered);
    Ddi_Free(Bset);
   
    if (Ddi_BddIsZero(leftover)) {
      continue; /* avoid doing image */
    }

    /*
     * Compute new leftover for next iteration
     */
    from = Ddi_BddAnd(leftover,currpat);
    Ddi_Free(currpat);

    /* smooth PIs while computing forward image */
    Tr_MgrSetImgSmoothPi(Tr_TrMgr(TR),1);
    to = Tr_Img (TR, from);
    Ddi_BddSetMono(to);

    Ddi_Free(from);
    Ddi_Free(leftover);
    leftover = Ddi_BddDup(to);
    Ddi_Free(to);

  }

  Ddi_Free(psvars);
  Ddi_Free(advance);
  Ddi_Free(leftover);
  Ddi_Free(quantify);

  /*----------------- Univ Alignment generation ... End ---------------------*/

  return (totpat);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

#if 0
static void
TraceFrozenLatches(
  Trav_Mgr_t *travMgr        /* traversal manager */,
  Ddi_Mgr_t *dd              /* dd manager */,
  Ddi_Vararray_t *s          /* present state vars */,
  Ddi_Vararray_t *y          /* next state vars */,
  Ddi_Varset_t *quantify     /* set of quantifying variables */,
  Ddi_Bdd_t *TR               /* Transition Relation */,
  Ddi_Bdd_t *from             /* result of image computation */,
  Ddi_Bdd_t *to               /* result of image computation */,
  Ddi_Bdd_t *reached          /* old reached */,
  int *frozen_latches        /* array of frozen latches */,
  Pdtutil_VerbLevel_e verbosity
)
{
  int i, ns;
  Ddi_Var_t *vs, *vy;
  Ddi_Bdd_t *constr_TR, *tmp, *eq, *newY, *reachedY;

  /*  assert(Ddi_BddIsMono(TR)); */

  ns = Ddi_VararrayNum(s);

  reachedY = Ddi_BddSwapVars(reached,s,y);
  newY = Ddi_BddAnd(to,(tmp=Ddi_BddNot(reachedY)));

  Ddi_Free(tmp);
  Ddi_Free(reachedY);

  for (i=0;i<ns;i++) {
    if (frozen_latches[i]==1) {
      vs = Ddi_VararrayRead(s,i);
      vy = Ddi_VararrayRead(y,i);
      eq = Ddi_BddXnor(Ddi_BddMakeLiteral(vs,1),Ddi_BddMakeLiteral(vy,1));

      constr_TR = Ddi_BddAnd(TR,eq);

      to = Trav_ImgConjPartTr(travMgr,constr_TR,y,y,from,quantify);

      if (!(Ddi_BddIncluded(newY,to)))
        frozen_latches[i]=0;

      Ddi_Free(eq);
      Ddi_Free(constr_TR);
      Ddi_Free(to);
    }
  }

  return;
}
#endif

