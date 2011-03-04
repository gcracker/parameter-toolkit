/**CFile***********************************************************************

  FileName    [testTr.c]

  PackageName [test]

  Synopsis    [Functions to test the TR package]

  Description [Various functions to test the correct operation of the TR
    routines. This program is developed as a regression test, and as usage
    example for TR functions.]

  SeeAlso   []  

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
  
  Revision  []

******************************************************************************/

#include "testInt.h"
#include "tr.h"

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

static int TestMisc();
static Ddi_Bddarray_t * s27Delta(Ddi_Vararray_t *pi, Ddi_Vararray_t *ps, Ddi_Vararray_t *ns);
static Ddi_Bdd_t * traversal(Tr_Tr_t *tr, Ddi_Bdd_t *init, Ddi_Vararray_t *ps);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************
  Synopsis    [Main test function for DDI]
  Description [Main test function for DDI. Calls specific DDI test functions.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
int
TestTr (
)
{

  if (TestMisc() == 0)
    return (0);
#if 0
  if (TestCnt(3) == 0)
    return (0);
  if (TestLShift(4) == 0)
    return (0);
#endif
  return (1);

}

/**Function*******************************************************************
  Synopsis    [general TR tests]
  Description [general TR tests. Creates a DDI and a TR manager, 
    transition relations, performs various operations and checks, 
    finally closes the managers. s27 ISCAS89 benchmark is used.
    Return 0 upon failure.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static int
TestMisc (
)
{
  int i, ret=1;
  Ddi_Mgr_t *ddm; 
  Ddi_Vararray_t *pi, *ps, *ns;
  Tr_Mgr_t *trMgr;
  Tr_Tr_t *tr;
  Ddi_Bdd_t *init, *reached, *lit;
  Ddi_Bddarray_t *delta;

  /**********************************************************************/
  /*                           Start test                               */
  /**********************************************************************/

  printf("***************** TR MISC TEST start **********************\n");

  /**********************************************************************/
  /*                        Create DDI manager                          */
  /**********************************************************************/

  printf("\n*** DDI manager created\n");fflush(stdout);

  ddm = Ddi_MgrInit("DDI_manager",NULL,10,
    DDI_UNIQUE_SLOTS,DDI_CACHE_SLOTS,0);

  pi = Ddi_VararrayAlloc(ddm,4);
  Ddi_VararrayWrite(pi,0,Ddi_IthVar(ddm,0));
  Ddi_VararrayWrite(pi,1,Ddi_IthVar(ddm,1));
  Ddi_VararrayWrite(pi,2,Ddi_IthVar(ddm,2));
  Ddi_VararrayWrite(pi,3,Ddi_IthVar(ddm,3));

  ps = Ddi_VararrayAlloc(ddm,3);
  Ddi_VararrayWrite(ps,0,Ddi_IthVar(ddm,4));
  Ddi_VararrayWrite(ps,1,Ddi_IthVar(ddm,5));
  Ddi_VararrayWrite(ps,2,Ddi_IthVar(ddm,6));

  ns = Ddi_VararrayAlloc(ddm,3);
  Ddi_VararrayWrite(ns,0,Ddi_IthVar(ddm,7));
  Ddi_VararrayWrite(ns,1,Ddi_IthVar(ddm,8));
  Ddi_VararrayWrite(ns,2,Ddi_IthVar(ddm,9));

  delta = s27Delta(pi,ps,ns);

  /**********************************************************************/
  /*                       generate TR manager                          */
  /**********************************************************************/

  trMgr = Tr_MgrInit ("TR-manager", ddm);
  /* Iwls95 sorting method */
  Tr_MgrSetSortMethod (trMgr, Tr_SortWeight_c);
  /* enable smoothing PIs while clustering */
  Tr_MgrSetClustSmoothPi(trMgr,1);
  Tr_MgrSetClustThreshold (trMgr,5);
  Tr_MgrSetVerbosity(trMgr,Pdtutil_VerbLevelAppMin_c);

  Tr_MgrSetI (trMgr, pi);
  Tr_MgrSetPS (trMgr, ps);
  Tr_MgrSetNS (trMgr, ns);

  tr = Tr_TrMakePartConjFromFuns(trMgr,delta,ns);

  Tr_TrSortIwls95(tr); 

  Tr_TrSetClustered(tr);

  /* reset state: all latches set to 0 */
  init = Ddi_BddMakeConst(ddm,1);
  for (i=0; i<Ddi_VararrayNum(ps); i++) {
    lit = Ddi_BddMakeLiteral(Ddi_VararrayRead(ps,i),0);
    Ddi_BddAndAcc(init,lit);
    Ddi_Free(lit);
  }

  printf ("\nTraversal starting from all 0 state\n");

  reached = traversal(tr,init,ps);

  printf ("\nTraversal using transitive closure\n");

  /*  Tr_MgrSetClustThreshold (trMgr,500);
  Tr_TrSetClustered(tr); */
  Tr_TrSetMono(tr); 

  {
    Ddi_Bdd_t *trBdd = Tr_TrBdd(tr);
    Ddi_BddSuppAttach(trBdd);
    printf ("TR before closure\n");
    Ddi_VarsetPrint(Ddi_BddSuppRead(trBdd),100,NULL,stdout);
    Ddi_BddPrintCubes(trBdd,NULL,100,0,NULL,stdout);
    Ddi_BddSuppDetach(trBdd);
  }

  Tr_TransClosure(tr);

  {
    Ddi_Bdd_t *trBdd = Tr_TrBdd(tr);
    Ddi_BddSuppAttach(trBdd);
    printf ("TR after closure\n");
    Ddi_VarsetPrint(Ddi_BddSuppRead(trBdd),100,NULL,stdout);
    Ddi_BddPrintCubes(trBdd,NULL,100,0,NULL,stdout);
    Ddi_BddSuppDetach(trBdd);
  }

  Ddi_Free(reached);

  reached = traversal(tr,init,ps);

  Ddi_Free(reached);
  Ddi_Free(init);

  Tr_TrFree(tr);
  Tr_MgrQuit(trMgr);

  Ddi_Free(delta);
  Ddi_Free(pi);
  Ddi_Free(ps);
  Ddi_Free(ns);

  if (Ddi_MgrCheckExtRef(ddm,0)==0) {
    Ddi_MgrPrintExtRef(ddm,0);
  }

  /**********************************************************************/
  /*                       Close DDI manager                            */
  /**********************************************************************/

  Ddi_MgrQuit(ddm);

  printf("\n*** DDI manager freed\n");fflush(stdout);

  /**********************************************************************/
  /*                             End test                               */
  /**********************************************************************/

  printf("***************** DDI MISC TEST end ************************\n");

  return(ret);
}


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*******************************************************************
  Synopsis    [generate NS functions of s27]
  Description []
  SideEffects []
  SeeAlso     []
******************************************************************************/
static Ddi_Bddarray_t *
s27Delta (
  Ddi_Vararray_t *pi, 
  Ddi_Vararray_t *ps,
  Ddi_Vararray_t *ns
)
{
  Ddi_Bddarray_t *delta;
  Ddi_Bdd_t *g0, *g1, *g2, *g3, *g5, *g6, *g7, *g8,
    *g9, *g10, *g11, *g12, *g13, *g14, *g15, *g16, *g17;

  Ddi_Mgr_t *ddm=Ddi_ReadMgr(ps); 

  /* inputs */
  
  g0 = Ddi_BddMakeLiteral(Ddi_VararrayRead(pi,0), 1);
  Ddi_SetName(Ddi_VararrayRead(pi,0),"g0");
  g1 = Ddi_BddMakeLiteral(Ddi_VararrayRead(pi,1), 1);
  Ddi_SetName(Ddi_VararrayRead(pi,1),"g1");
  g2 = Ddi_BddMakeLiteral(Ddi_VararrayRead(pi,2), 1);
  Ddi_SetName(Ddi_VararrayRead(pi,2),"g2");
  g3 = Ddi_BddMakeLiteral(Ddi_VararrayRead(pi,3), 1);
  Ddi_SetName(Ddi_VararrayRead(pi,3),"g3");

  /* present states */

  g5 = Ddi_BddMakeLiteral(Ddi_VararrayRead(ps,0), 1);
  Ddi_SetName(Ddi_VararrayRead(ps,0),"g5");
  g6 = Ddi_BddMakeLiteral(Ddi_VararrayRead(ps,1), 1);
  Ddi_SetName(Ddi_VararrayRead(ps,1),"g6");
  g7 = Ddi_BddMakeLiteral(Ddi_VararrayRead(ps,2), 1);
  Ddi_SetName(Ddi_VararrayRead(ps,2),"g7");

  /* next states */

  Ddi_SetName(Ddi_VararrayRead(ns,0),"g5$NS");
  Ddi_SetName(Ddi_VararrayRead(ns,1),"g6$NS");
  Ddi_SetName(Ddi_VararrayRead(ns,2),"g7$NS");

  /* gates */

  g12 = Ddi_BddNor(g1,g7);
  g13 = Ddi_BddNor(g2,g12);
  g14 = Ddi_BddNot(g0);
  g8 = Ddi_BddAnd(g14,g6);
  g15 = Ddi_BddOr(g12,g8);
  g16 = Ddi_BddOr(g3,g8);
  g9 = Ddi_BddNand(g16,g15);
  g11 = Ddi_BddNor(g5,g9);
  g10 = Ddi_BddNor(g14,g11);
  g17 = Ddi_BddNot(g11);

  delta = Ddi_BddarrayAlloc(ddm,3);

  Ddi_BddarrayWrite(delta,0,g10);
  Ddi_BddarrayWrite(delta,1,g11);
  Ddi_BddarrayWrite(delta,2,g13);

  Ddi_Free(g0);
  Ddi_Free(g1);
  Ddi_Free(g2);
  Ddi_Free(g3);

  Ddi_Free(g5);
  Ddi_Free(g6);
  Ddi_Free(g7);
  Ddi_Free(g8);
  Ddi_Free(g9);
  Ddi_Free(g10);
  Ddi_Free(g11);
  Ddi_Free(g12);
  Ddi_Free(g13);
  Ddi_Free(g14);
  Ddi_Free(g15);
  Ddi_Free(g16);
  Ddi_Free(g17);

  return(delta);

}



/**Function*******************************************************************
  Synopsis    [traversal]
  Description []
  SideEffects []
  SeeAlso     []
******************************************************************************/
static Ddi_Bdd_t *
traversal (
  Tr_Tr_t *tr,
  Ddi_Bdd_t *init,
  Ddi_Vararray_t *ps
)
{
  int i;
  Ddi_Bdd_t *from, *reached, *to;

  from = Ddi_BddDup(init);
  reached = Ddi_BddDup(init);

  for (i=0; !Ddi_BddIsZero(from); i++) {
    printf ("\nTraversal iteration: %d\nFROM\n",i+1);

    Ddi_BddSuppAttach(from);
    Ddi_VarsetPrint(Ddi_BddSuppRead(from),100,NULL,stdout);
    Ddi_BddPrintCubes(from,NULL,100,0,NULL,stdout);
    Ddi_BddSuppDetach(from);

    to = Tr_Img(tr,from);
    Ddi_Free(from);
    from = Ddi_BddDiff(to,reached);
    Ddi_BddOrAcc(reached,to);
    printf ("|To|=%d, |Reached|=%d\n",Ddi_BddSize(to),Ddi_BddSize(reached));
    printf ("# Reached states = %g\n", 
      Ddi_CountMinterm(reached, Ddi_VararrayNum(ps)));
    Ddi_Free(to);
  }

  Ddi_Free(from);

  printf ("\nREACHED\n");
  Ddi_BddSuppAttach(reached);
  Ddi_VarsetPrint(Ddi_BddSuppRead(reached),100,NULL,stdout);
  Ddi_BddPrintCubes(reached,NULL,100,0,NULL,stdout);
  Ddi_BddSuppDetach(reached);

  return(reached);
}










