/**CFile***********************************************************************

  FileName    [testDdi.c]

  PackageName [test]

  Synopsis    [Functions to test the DDI package]

  Description [Various functions to test the correct operation of the DDI
    routines. This program is developed as a regression test, and as usage
    example for DDI functions.]

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
#include "ddi.h"

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
static int TestCnt(int nbits);
static int TestLShift(int nbits);
static int TestLRShift(int nbits);
static int TestExpr();

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
TestDdi (
)
{

  if (TestMisc() == 0)
    return (0);
  if (TestCnt(3) == 0)
    return (0);
  if (TestLShift(4) == 0)
    return (0);
  if (TestLRShift(4) == 0)
    return (0);
  if (TestExpr() == 0)
    return (0);
  return (1);

}

/**Function*******************************************************************
  Synopsis    [general DDI tests]
  Description [general DDI tests. Creates a DDI manager, variables
    and functions, performs various operations and checks, finally closes
    the manager.
    Return 0 upon failure.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static int
TestMisc (
)
{
  int ret=1;
  Ddi_Mgr_t *ddm; 
  int i, j, varNum, slotNum, cacheSize, memorySizeMax;
  Ddi_Var_t *v;
  Ddi_Varset_t *vs1, *vs2, *vs3;
  Ddi_Vararray_t *va;
  Ddi_Bdd_t *f, *g, *h, *part;
  char buf[PDTUTIL_MAX_STR_LEN];

  /**********************************************************************/
  /*                           Start test                               */
  /**********************************************************************/

  printf("***************** DDI MISC TEST start **********************\n");

  /**********************************************************************/
  /*                        Create DDI manager                          */
  /**********************************************************************/

  printf("\n*** Creating DDI manager...");fflush(stdout);

  /* 
   * initial number of variables. Set low to test variable 
   * creation later on
   */
  varNum = 3;
  /*
   * these are set to default constants
   * See related Cudd_Init parameters for explanation
   */
  slotNum = DDI_UNIQUE_SLOTS;
  cacheSize = DDI_CACHE_SLOTS;
  memorySizeMax = 0;

  ddm = Ddi_MgrInit("DDI_test_manager",NULL,varNum,slotNum,cacheSize,
    memorySizeMax);

  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }

  printf("done\n");

  /**********************************************************************/
  /*                        Variable management                         */
  /**********************************************************************/

  printf("\n*** Variable handling:\n");

  v = Ddi_VarNew(ddm);
  v = Ddi_VarNew(ddm);
  v = Ddi_VarNewAtLevel(ddm,0);

  for (i=0;i<6;i++) {
    sprintf(buf,"V%d",i);
    Ddi_VarAttachName(Ddi_IthVar(ddm,i),buf);
    Ddi_VarAttachAuxid(Ddi_IthVar(ddm,i),1000+i);
  }

  printf("\n==========================================\n"); 
  printf("\n>> Variable table\n"); 
  printf("------------------------------------------\n"); 
  printf("%6s %6s %6s   %s\n", "id","ord","auxid","name"); 
  for (i=0;i<6;i++) {
    v = Ddi_IthVar(ddm,i);
    printf("%6d %6d %6d   %s\n", i, 
      Ddi_VarCurrPos(v),Ddi_VarAuxid(v),Ddi_VarName(v));
  }
  printf("==========================================\n\n"); 

  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }
  Ddi_MgrCheckExtRef(ddm,0);

  /**********************************************************************/
  /*                Variable set and array management                   */
  /**********************************************************************/

  printf("\n*** Variable set/array handling:\n");

  varNum = Ddi_ReadNumVars(ddm);

  printf("\n>> Creating va var array including %d variables\n", varNum);

  va = Ddi_VararrayAlloc(ddm,varNum);
  for (i=0; i<varNum; i++) {
    Ddi_VararrayWrite(va,i,Ddi_IthVar(ddm,i));
  }
  Ddi_MgrCheckExtRef(ddm,1);

  printf("\n>> Creating vs1 var set with 1 variable\n");

  vs1 = Ddi_VarsetMakeFromVar(Ddi_IthVar(ddm,0));
  Ddi_MgrCheckExtRef(ddm,2);
  Ddi_Free(vs1);

  printf("\n>> Creating vs1 var set with 3 variables\n");

  vs1 = Ddi_VarsetVoid(ddm);
  for (i=0; i<3; i++) {
    Ddi_VarsetAddAcc(vs1,Ddi_IthVar(ddm,i));
  }
  Ddi_MgrCheckExtRef(ddm,2);

  printf("\n>> Creating vs2 var set from va var array\n");

  vs2 = Ddi_VarsetMakeFromArray(va);
  Ddi_MgrCheckExtRef(ddm,3);

  printf("\n>> Printing vs1 in single row\n");
  Ddi_VarsetPrint(vs1,100,NULL,stdout);

  printf("\n>> Printing vs2 in rows of 2\n");
  Ddi_VarsetPrint(vs2,2,NULL,stdout);

  printf("\n>> vs3 = Diff(vs2,vs1)\n");
  vs3 = Ddi_VarsetDiff(vs2,vs1);
  Ddi_MgrCheckExtRef(ddm,4);
  Ddi_VarsetPrint(vs3,100,NULL,stdout);

  printf("\n>> vs1 = Add(vs1,V4)\n");
  Ddi_VarsetAddAcc(vs1,Ddi_VarFromName(ddm,"V4"));
  Ddi_MgrCheckExtRef(ddm,4);
  Ddi_VarsetPrint(vs1,100,NULL,stdout);

  printf("\n>> vs2 = Remove(vs2,V2)\n");
  Ddi_VarsetRemoveAcc(vs2,Ddi_VarFromName(ddm,"V2"));
  Ddi_MgrCheckExtRef(ddm,4);
  Ddi_VarsetPrint(vs2,100,NULL,stdout);

  printf("\n>> v3 = Intersect(vs2,vs1)\n");
  Ddi_Free(vs3);
  vs3 = Ddi_VarsetIntersect(vs2,vs1);
  Ddi_MgrCheckExtRef(ddm,4);
  Ddi_VarsetPrint(vs3,100,NULL,stdout);

  printf("\n>> Freeing vs1 var set\n");
  Ddi_Free(vs1);
  Ddi_MgrCheckExtRef(ddm,3);

  printf("\n>> Freeing vs2 var set\n");
  Ddi_Free(vs2);
  Ddi_MgrCheckExtRef(ddm,2);

  printf("\n>> Freeing vs3 var set\n");
  Ddi_Free(vs3);
  Ddi_MgrCheckExtRef(ddm,1);

  printf("\n>> Printing va var array\n");
  for (i=0; i<Ddi_VararrayNum(va); i++) {
    v = Ddi_VararrayRead(va,i);
    printf("va[%d]: id=%d - name=%s\n",i, Ddi_VarIndex(v),Ddi_VarName(v));
  }

  printf("\n>> Reverting order of va by rewriting\n");
  for (i=0; i<Ddi_VararrayNum(va)/2; i++) {
    v = Ddi_VararrayRead(va,i);
    j = Ddi_VararrayNum(va)-i-1;
    Ddi_VararrayWrite(va,i,Ddi_VararrayRead(va,j));
    Ddi_VararrayWrite(va,j,v);
  }

  for (i=0; i<Ddi_VararrayNum(va); i++) {
    v = Ddi_VararrayRead(va,i);
    printf("va[%d]: id=%d - name=%s\n",i, Ddi_VarIndex(v),Ddi_VarName(v));
  }

  printf("\n>> Freeing va var array\n");
  Ddi_Free(va);
  Ddi_MgrCheckExtRef(ddm,0);

  Ddi_MgrPrintAllocStats(ddm,stdout);
  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }
  Ddi_MgrCheckExtRef(ddm,0);

  /**********************************************************************/
  /*                       monolithic BDDs                              */
  /**********************************************************************/

  printf("\n*** Monolithic BDD handling ");
  printf("(BDDs are printed as cubes):\n");

  varNum = Ddi_ReadNumVars(ddm);

  printf("\n>> f = !%s&V5\n",Ddi_VarName(Ddi_IthVar(ddm,2)));

  v = Ddi_IthVar(ddm,2);
  f = Ddi_BddMakeLiteral(v,0);
  v = Ddi_VarFromName(ddm,"V5");
  h = Ddi_BddMakeLiteral(v,1);

  Ddi_BddAndAcc(f,h);

  Ddi_Free(h);
  Ddi_MgrCheckExtRef(ddm,1);

  Ddi_BddSuppAttach(f);
  Ddi_VarsetPrint(Ddi_BddSuppRead(f),100,NULL,stdout);
  Ddi_BddPrintCubes(f,NULL,100,0,NULL,stdout);

  printf("\n>> g = %s|V4\n",Ddi_VarName(Ddi_IthVar(ddm,0)));

  v = Ddi_IthVar(ddm,0);
  g = Ddi_BddMakeLiteral(v,1);
  v = Ddi_VarFromName(ddm,"V4");
  h = Ddi_BddMakeLiteral(v,1);

  Ddi_BddOrAcc(g,h);

  Ddi_Free(h);
  Ddi_MgrCheckExtRef(ddm,2);

  Ddi_BddSuppAttach(g);
  Ddi_VarsetPrint(Ddi_BddSuppRead(g),100,NULL,stdout);
  Ddi_BddPrintCubes(g,NULL,100,0,NULL,stdout);

  printf("\n>> h = f&g\n");

  h = Ddi_BddAnd(f,g);
  Ddi_MgrCheckExtRef(ddm,3);

  Ddi_BddSuppAttach(h);
  Ddi_VarsetPrint(Ddi_BddSuppRead(h),100,NULL,stdout);
  Ddi_BddPrintCubes(h,NULL,100,0,NULL,stdout);

  printf("\n>> h = f|g\n");

  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }

  Ddi_Free(h);
  h = Ddi_BddOr(f,g);
  Ddi_MgrCheckExtRef(ddm,3);

  Ddi_BddSuppAttach(h);
  Ddi_VarsetPrint(Ddi_BddSuppRead(h),100,NULL,stdout);
  Ddi_BddPrintCubes(h,NULL,100,0,NULL,stdout);

  printf("\n>> h = ~(~(f^g)|f)\n");

  Ddi_Free(h);
  h = Ddi_BddXnor(f,g);
  Ddi_BddOrAcc(h,f);
  Ddi_BddNotAcc(f);
  Ddi_MgrCheckExtRef(ddm,3);

  Ddi_BddSuppAttach(h);
  Ddi_VarsetPrint(Ddi_BddSuppRead(h),100,NULL,stdout);
  Ddi_BddPrintCubes(h,NULL,100,0,NULL,stdout);

  printf("\n>> h = Constrain(h,V5)\n");

  Ddi_Free(g);
  g = Ddi_BddMakeLiteral(Ddi_VarFromName(ddm,"V5"),1);
  /* freeing old h with EvalFree instead of accumulate OP */
  h = Ddi_BddEvalFree(Ddi_BddConstrain(h,g),h);

  Ddi_BddSuppAttach(h);
  Ddi_VarsetPrint(Ddi_BddSuppRead(h),100,NULL,stdout);
  Ddi_BddPrintCubes(h,NULL,100,0,NULL,stdout);

  printf("\n>> Printing h in DDDMP text format\n");
  Ddi_BddStore(f,"h",DDDMP_MODE_TEXT,NULL,stdout);

  printf("\n>> Extracting cube\n");
  Ddi_Free(g);
  g = Ddi_BddPickOneCube(h);

  printf("selected cube - expressed in terms of variables: ");
  Ddi_VarsetPrint(Ddi_BddSuppRead(h),100,NULL,stdout);
  if (Ddi_BddPrintCubeToString(g,Ddi_BddSuppRead(h),buf)==0) {
    printf("Error printing cube");
  }
  else {
    printf("%s\n", buf);
  }

  printf("\n>> Freeing f\n");
  Ddi_Free(f);
  Ddi_MgrCheckExtRef(ddm,2);

  printf("\n>> Freeing g\n");
  Ddi_Free(g);
  Ddi_MgrCheckExtRef(ddm,1);

  printf("\n>> Freeing h\n");
  Ddi_Free(h);
  Ddi_MgrCheckExtRef(ddm,0);

  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }
  Ddi_MgrCheckExtRef(ddm,0);


  /**********************************************************************/
  /*                      partitioned BDDs                              */
  /**********************************************************************/

  printf("\n*** Partitioned BDD handling ");
  printf("(BDDs are printed as cubes):\n");

  varNum = Ddi_ReadNumVars(ddm);

  printf("\n>> part = CPART(V1,!V3,V5)\n");

  f = Ddi_BddMakeLiteral(Ddi_VarFromName(ddm,"V1"),1);
  g = Ddi_BddMakeLiteral(Ddi_VarFromName(ddm,"V3"),0);
  h = Ddi_BddMakeLiteral(Ddi_VarFromName(ddm,"V5"),1);

  part = Ddi_BddMakePartConjFromMono(f);
  Ddi_MgrCheckExtRef(ddm,4);

  Ddi_BddPartInsertLast(part,g);
  Ddi_BddPartInsert(part,1,g);
  Ddi_MgrCheckExtRef(ddm,4);

  Ddi_Free(f);
  Ddi_Free(g);
  Ddi_Free(h);

  printf("\n>> printing part\n");
  printf("\n>> %d partitions\n", Ddi_BddPartNum(part));
  for (i=0; i<Ddi_BddPartNum(part); i++) {
    f = Ddi_BddPartRead(part,i);
    printf("\n>>* part[%d]\n", i);
    Ddi_BddSuppAttach(f);
    Ddi_VarsetPrint(Ddi_BddSuppRead(f),100,NULL,stdout);
    Ddi_BddPrintCubes(f,NULL,100,0,NULL,stdout); 
  }

  printf("\n>> Freeing part\n");
  Ddi_Free(part);
  Ddi_MgrCheckExtRef(ddm,0);

  /**********************************************************************/
  /*                       Close DDI manager                            */
  /**********************************************************************/

  Ddi_MgrPrintAllocStats(ddm,stdout);
  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }
  Ddi_MgrCheckExtRef(ddm,0);

  printf("\n*** Freeing DDI manager...");fflush(stdout);

  Ddi_MgrQuit(ddm);

  printf("done\n");

  /**********************************************************************/
  /*                             End test                               */
  /**********************************************************************/

  printf("***************** DDI MISC TEST end ************************\n");

  return(ret);
}


/**Function*******************************************************************
  Synopsis    [generate TR of a counter and compute reachable states]
  Description [generate TR of a counter and compute reachable states.
    Return 0 upon failure.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static int
TestCnt (
  int nbits /* number of counter bits */
)
{
  int ret=1;
  Ddi_Mgr_t *ddm; 
  int i, step, nl, ni;
  int varNum, slotNum, cacheSize, memorySizeMax;
  Ddi_Var_t *env, *clv, *v;
  Ddi_Vararray_t *psv, *nsv;
  Ddi_Bdd_t *TR, *TRcnt, *TRcl, *ti, *en, *yb, *sb, *y, *s;
  Ddi_Bdd_t *s0, *from, *to, *reached;
  Ddi_Varset_t *quantify;
  char buf[PDTUTIL_MAX_STR_LEN];

  nl = nbits;
  ni = 2; /* en, cl */
  varNum = 2*nl + ni;


  /**********************************************************************/
  /*                           Start test                               */
  /**********************************************************************/

  printf("***************** DDI COUNTER TEST start **********************\n");

  /**********************************************************************/
  /*                        Create DDI manager                          */
  /**********************************************************************/

  printf("\n*** Creating DDI manager with %d variables...", varNum);
  fflush(stdout);

  slotNum = DDI_UNIQUE_SLOTS;
  cacheSize = DDI_CACHE_SLOTS;
  memorySizeMax = 0;

  ddm = Ddi_MgrInit("DDI_counter_manager",NULL,varNum,slotNum,cacheSize,
    memorySizeMax);

  printf("done\n");

  /**********************************************************************/
  /*                        Variable management                         */
  /**********************************************************************/

  clv = Ddi_IthVar(ddm,0);
  env = Ddi_IthVar(ddm,1);
  Ddi_VarAttachName(clv,"Cl");
  Ddi_VarAttachName(env,"En");
  psv = Ddi_VararrayAlloc(ddm,nl);
  nsv = Ddi_VararrayAlloc(ddm,nl);

  for (i=0; i<nl; i++) {
    v = Ddi_IthVar(ddm,2+2*(i));
    Ddi_VararrayWrite(psv,i,v);
    sprintf(buf,"s[%d]",i);
    Ddi_VarAttachName(v,buf);
    printf("ps[%d]: id=%d - name=%s\n",i, Ddi_VarIndex(v),Ddi_VarName(v));

    v = Ddi_IthVar(ddm,2+2*(i)+1);
    Ddi_VararrayWrite(nsv,i,v);
    sprintf(buf,"y[%d]",i);
    Ddi_VarAttachName(v,buf);
    printf("ns[%d]: id=%d - name=%s\n",i, Ddi_VarIndex(v),Ddi_VarName(v));

  }


  printf("\n*** Creating Transition Relation\n");

  TRcnt = Ddi_BddMakeLiteral(clv,0);

  for (i=0; i<nl; i++) {
    en = Ddi_BddMakeLiteral(env,1);
    if (i>0) {
      y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i-1),1);
      s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i-1),1);
      Ddi_BddAndAcc(en,Ddi_BddNotAcc(y));
      Ddi_BddAndAcc(en,s);
      Ddi_Free(y);
      Ddi_Free(s);
    }
    y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),1);
    s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i),1);
    ti = Ddi_BddXor(y,s);
    Ddi_BddXnorAcc(ti,en);

    Ddi_BddAndAcc(TRcnt,ti);

    Ddi_Free(en);
    Ddi_Free(y);
    Ddi_Free(s);
    Ddi_Free(ti);
  }  

  TRcl = Ddi_BddMakeLiteral(clv,1);

  for (i=0; i<nl; i++) {
    yb = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),0);
    Ddi_BddAndAcc(TRcl,yb);
    Ddi_Free(yb);
  }  

  TR = Ddi_BddOr(TRcnt,TRcl);
  Ddi_Free(TRcl);
  Ddi_Free(TRcnt);

  /**********************************************************************/
  /*                           Start traversal                          */
  /**********************************************************************/

  printf("\n*** Computing reachable states\n");

  /* initial state */
  s0 = Ddi_BddMakeConst(ddm,1);
  for (i=0; i<nl; i++) {
    sb = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i),0);
    Ddi_BddAndAcc(s0,sb);
    Ddi_Free(sb);
  }  

  /* quantification set */
  quantify = Ddi_VarsetMakeFromArray(psv);
  Ddi_VarsetAddAcc(quantify,clv);
  Ddi_VarsetAddAcc(quantify,env);

  from = Ddi_BddDup(s0);
  reached = Ddi_BddDup(s0);
  for (step=0; !Ddi_BddIsZero(from); step++) {

    printf("Traversal iteration %d\n", step);
    Ddi_BddPrintDbg("-- From --\n ",from,stdout);

    /* image computation */ 
    to = Ddi_BddAndExist(from,TR,quantify);

    Ddi_BddPrintDbg("-- To --\n",to,stdout);
    
    /* here we use var subst instead of swap */
    Ddi_BddSubstVarsAcc(to,nsv,psv);

    Ddi_BddPrintDbg("-- To(y<-s) --\n",to,stdout);

    Ddi_Free(from);
    from = Ddi_BddDiff(to,reached);
    Ddi_BddOrAcc(reached,to);

    Ddi_Free(to);

    Ddi_BddPrintDbg("-- Reached --\n",reached,stdout);
  }

  Ddi_Free(s0);
  Ddi_Free(from);
  Ddi_Free(reached);
  Ddi_Free(TR);

  Ddi_Free(quantify);
  Ddi_Free(psv);
  Ddi_Free(nsv);

  Ddi_MgrQuit(ddm);

  printf("***************** DDI COUNTER TEST end   **********************\n");

  return(ret);

}

/**Function*******************************************************************
  Synopsis    [generate TR of a left shifter and compute reachable states]
  Description [generate TR of a left shifter and compute reachable states.
    Return 0 upon failure.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static int
TestLShift (
  int nbits /* number of shifter bits */
)
{
  int ret=1;
  Ddi_Mgr_t *ddm; 
  int i, step, nl, ni;
  int varNum, slotNum, cacheSize, memorySizeMax;
  Ddi_Var_t *env, *sinlv, *v;
  Ddi_Vararray_t *psv, *nsv;
  Ddi_Bdd_t *TR, *TRlsh, *TRhold, *ti, *y, *s;
  Ddi_Bdd_t *s0, *from, *to, *reached;
  Ddi_Varset_t *quantify;
  char buf[PDTUTIL_MAX_STR_LEN];

  nl = nbits;
  ni = 2; /* en, s_in_l */
  varNum = 2*nl + ni;


  /**********************************************************************/
  /*                           Start test                               */
  /**********************************************************************/

  printf("\n*************** DDI LEFT SHIFT TEST start *****************\n");

  /**********************************************************************/
  /*                        Create DDI manager                          */
  /**********************************************************************/

  printf("\n*** Creating DDI manager with %d variables...", varNum);
  fflush(stdout);

  slotNum = DDI_UNIQUE_SLOTS;
  cacheSize = DDI_CACHE_SLOTS;
  memorySizeMax = 0;

  ddm = Ddi_MgrInit("DDI_counter_manager",NULL,varNum,slotNum,cacheSize,
    memorySizeMax);
  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }

  printf("done\n");
  Ddi_MgrCheckExtRef(ddm,0);

  /**********************************************************************/
  /*                        Variable management                         */
  /**********************************************************************/

  env = Ddi_IthVar(ddm,0);
  Ddi_VarAttachName(env,"En");
  sinlv = Ddi_IthVar(ddm,1);
  Ddi_VarAttachName(sinlv,"SInL");
  psv = Ddi_VararrayAlloc(ddm,nl);
  nsv = Ddi_VararrayAlloc(ddm,nl);
  Ddi_MgrCheckExtRef(ddm,2);

  for (i=0; i<nl; i++) {

    v = Ddi_IthVar(ddm,2+2*(nl-i-1));
    Ddi_VararrayWrite(psv,i,v);
    sprintf(buf,"s[%d]",i);
    Ddi_VarAttachName(v,buf);

    v = Ddi_IthVar(ddm,2+2*(nl-i-1)+1);
    Ddi_VararrayWrite(nsv,i,v);
    sprintf(buf,"y[%d]",i);
    Ddi_VarAttachName(v,buf);

  }

  printf("\n*** Creating Transition Relation...");

  TRhold = Ddi_BddMakeLiteral(env,0);

  for (i=0; i<nl; i++) {
    y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),1);
    s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i),1);
    ti = Ddi_BddXnor(y,s);

    Ddi_BddAndAcc(TRhold,ti);

    Ddi_Free(y);
    Ddi_Free(s);
    Ddi_Free(ti);
  }  

  TRlsh = Ddi_BddMakeLiteral(env,1);

  for (i=0; i<nl; i++) {

    y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),1);
    if (i>0) 
      s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i-1),1);
    else 
      s = Ddi_BddMakeLiteral(sinlv,1);

    ti = Ddi_BddXnor(s,y);
      
    Ddi_BddAndAcc(TRlsh,ti);

    Ddi_Free(y);
    Ddi_Free(s);
    Ddi_Free(ti);
    }


  TR = Ddi_BddOr(TRlsh,TRhold);
  Ddi_Free(TRlsh);
  Ddi_Free(TRhold);

  printf("done\n");
  Ddi_MgrCheckExtRef(ddm,3);

  /**********************************************************************/
  /*                           Start traversal                          */
  /**********************************************************************/

  printf("\n*** Computing reachable states\n");

  /* initial state */
  s0 = Ddi_BddMakeConst(ddm,1);
  for (i=0; i<nl; i++) {
    s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i),0);
    Ddi_BddAndAcc(s0,s);
    Ddi_Free(s);
  }  

  /* quantification set */
  quantify = Ddi_VarsetMakeFromArray(psv);
  Ddi_VarsetAddAcc(quantify,env);
  Ddi_VarsetAddAcc(quantify,sinlv);
  Ddi_MgrCheckExtRef(ddm,5);

  from = Ddi_BddDup(s0);
  reached = Ddi_BddDup(s0);
  Ddi_Free(s0);
  Ddi_MgrCheckExtRef(ddm,6);
  for (step=0; !Ddi_BddIsZero(from); step++) {

    printf("\nTraversal iteration %d\n", step);
    Ddi_BddPrintDbg("-- From --\n ",from,stdout);

    /* image computation */ 
    to = Ddi_BddAndExist(from,TR,quantify);

    Ddi_BddPrintDbg("-- To --\n",to,stdout);

    Ddi_BddSwapVarsAcc(to,psv,nsv);
/*
    Ddi_BddPrintDbg("-- To(Sw) --\n",to,stdout);
*/
    Ddi_Free(from);
    from = Ddi_BddDiff(to,reached);
    Ddi_BddOrAcc(reached,to);

    Ddi_Free(to);

    Ddi_BddPrintDbg("-- Reached --\n",reached,stdout);
  }

  Ddi_Free(from);
  Ddi_Free(reached);
  Ddi_Free(TR);
  Ddi_Free(quantify);
  Ddi_Free(psv);
  Ddi_Free(nsv);
  i=Ddi_MgrReadExtRef(ddm);
  printf("Number of Ext. Ref. found: %d\n",i);

  /**********************************************************************/
  /*                       Close DDI manager                            */
  /**********************************************************************/
/*
  Ddi_MgrPrintAllocStats(ddm,stdout);
*/
  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }

  printf("\n*** Freeing DDI manager...");fflush(stdout);

  Ddi_MgrQuit(ddm);

  printf("done\n\n");

  printf("*************** DDI LEFT SHIFT TEST end  ********************\n");

  return(ret);
}

/**Function*******************************************************************
 Synopsis [generate TR of a l/r shifter and compute reachable states]
 Description [generate TR of a l/r shifter and compute reachable
              states. Return 0 upon failure.]   
 SideEffects []   
 SeeAlso     []
******************************************************************************/ 
static int TestLRShift (
  int nbits /* number of shifter bits */
)
{
  int ret=1;
  Ddi_Mgr_t *ddm; 
  int i, step, nl, ni;
  int varNum, slotNum, cacheSize, memorySizeMax;
  Ddi_Var_t *enlv, *enrv, *clv, *silv, *sirv, *v;
  Ddi_Vararray_t *psv, *nsv;
  Ddi_Bdd_t *TR, *TRlsh, *TRhold, *TRrsh, *TRcl, *ti, *y, *s;
  Ddi_Bdd_t *s0, *from, *to, *reached;
  Ddi_Varset_t *quantify;
  char buf[PDTUTIL_MAX_STR_LEN];

  nl = nbits;
  ni = 5; /* cl, enls, enrs, SIL, SIR */
  varNum = 2*nl + ni;


  /**********************************************************************/
  /*                           Start test                               */
  /**********************************************************************/

  printf("\n************* DDI LEFT-RIGHT SHIFT TEST start **************\n");

  /**********************************************************************/
  /*                        Create DDI manager                          */
  /**********************************************************************/

  printf("\n*** Creating DDI manager with %d variables...", varNum);
  fflush(stdout);

  slotNum = DDI_UNIQUE_SLOTS;
  cacheSize = DDI_CACHE_SLOTS;
  memorySizeMax = 0;

  ddm = Ddi_MgrInit("DDI_counter_manager",NULL,varNum,slotNum,cacheSize,
    memorySizeMax);
  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }

  printf("done\n");
  Ddi_MgrCheckExtRef(ddm,0);

  /**********************************************************************/
  /*                        Variable management                         */
  /**********************************************************************/

  clv = Ddi_IthVar(ddm,0);
  Ddi_VarAttachName(clv,"Cl");
  enlv = Ddi_IthVar(ddm,1);
  Ddi_VarAttachName(enlv,"ELS");
  silv = Ddi_IthVar(ddm,2);
  Ddi_VarAttachName(silv,"SIL");
  enrv = Ddi_IthVar(ddm,3);
  Ddi_VarAttachName(enrv,"ERS");
  sirv = Ddi_IthVar(ddm,4);
  Ddi_VarAttachName(sirv,"SIR");
  psv = Ddi_VararrayAlloc(ddm,nl);
  nsv = Ddi_VararrayAlloc(ddm,nl);
  Ddi_MgrCheckExtRef(ddm,2);

  for (i=0; i<nl; i++) {

    v = Ddi_IthVar(ddm,ni+2*(nl-i-1));
    Ddi_VararrayWrite(psv,i,v);
    sprintf(buf,"s[%d]",i);
    Ddi_VarAttachName(v,buf);

    v = Ddi_IthVar(ddm,ni+2*(nl-i-1)+1);
    Ddi_VararrayWrite(nsv,i,v);
    sprintf(buf,"y[%d]",i);
    Ddi_VarAttachName(v,buf);

  }

  printf("\n*** Creating Transition Relation...");

  TRhold = Ddi_BddMakeLiteral(enlv,0);
  ti = Ddi_BddMakeLiteral(enrv,0);
  Ddi_BddAndAcc(TRhold,ti);
  Ddi_Free(ti);

  for (i=0; i<nl; i++) {
    y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),1);
    s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i),1);
    ti = Ddi_BddXnor(y,s);

    Ddi_BddAndAcc(TRhold,ti);

    Ddi_Free(y);
    Ddi_Free(s);
    Ddi_Free(ti);
  }  

  TRlsh = Ddi_BddMakeLiteral(enlv,1);
  ti = Ddi_BddMakeLiteral(enrv,0);
  Ddi_BddAndAcc(TRlsh,ti);
  Ddi_Free(ti);

  for (i=0; i<nl; i++) {

    y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),1);
    if (i>0) 
      s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i-1),1);
    else 
      s = Ddi_BddMakeLiteral(silv,1);

    ti = Ddi_BddXnor(s,y);
      
    Ddi_BddAndAcc(TRlsh,ti);

    Ddi_Free(y);
    Ddi_Free(s);
    Ddi_Free(ti);
  }

  TRrsh = Ddi_BddMakeLiteral(enrv,1);
  ti = Ddi_BddMakeLiteral(enlv,0);
  Ddi_BddAndAcc(TRrsh,ti);
  Ddi_Free(ti);

  for (i=0; i<nl; i++) {

    y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),1);
    if (i<nl-1) 
      s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i+1),1);
    else 
      s = Ddi_BddMakeLiteral(sirv,1);

    ti = Ddi_BddXnor(s,y);
      
    Ddi_BddAndAcc(TRrsh,ti);

    Ddi_Free(y);
    Ddi_Free(s);
    Ddi_Free(ti);
  }

  TR = Ddi_BddOr(TRlsh,TRhold);
  Ddi_BddOrAcc(TR,TRrsh);

  Ddi_Free(TRlsh);
  Ddi_Free(TRhold);
  Ddi_Free(TRrsh);

  ti = Ddi_BddMakeLiteral(clv,0);
  Ddi_BddAndAcc(TR,ti);
  Ddi_Free(ti);

  TRcl = Ddi_BddMakeLiteral(clv,1);

  for (i=0; i<nl; i++) {
    y = Ddi_BddMakeLiteral(Ddi_VararrayRead(nsv,i),0);
    Ddi_BddAndAcc(TRcl,y);
    Ddi_Free(y);
  }  

  Ddi_BddOrAcc(TR,TRcl);
  Ddi_Free(TRcl);

  printf("done\n");
  Ddi_MgrCheckExtRef(ddm,3);

  /**********************************************************************/
  /*                           Start traversal                          */
  /**********************************************************************/

  printf("\n*** Computing reachable states\n");

  /* initial state */
  s0 = Ddi_BddMakeConst(ddm,1);
  for (i=0; i<nl; i++) {
    s = Ddi_BddMakeLiteral(Ddi_VararrayRead(psv,i),0);
    Ddi_BddAndAcc(s0,s);
    Ddi_Free(s);
  }  

  /* quantification set */
  quantify = Ddi_VarsetMakeFromArray(psv);
  Ddi_VarsetAddAcc(quantify,clv);
  Ddi_VarsetAddAcc(quantify,enlv);
  Ddi_VarsetAddAcc(quantify,silv);
  Ddi_VarsetAddAcc(quantify,enrv);
  Ddi_VarsetAddAcc(quantify,sirv);
  Ddi_MgrCheckExtRef(ddm,5);

  from = Ddi_BddDup(s0);
  reached = Ddi_BddDup(s0);
  Ddi_Free(s0);
  Ddi_MgrCheckExtRef(ddm,6);

  for (step=0; !Ddi_BddIsZero(from); step++) {

    printf("\nTraversal iteration %d\n", step);
    Ddi_BddPrintDbg("-- From --\n ",from,stdout);

    /* image computation */ 
    to = Ddi_BddAndExist(from,TR,quantify);

    Ddi_BddPrintDbg("-- To --\n",to,stdout);

    Ddi_BddSwapVarsAcc(to,psv,nsv);
/*
    Ddi_BddPrintDbg("-- To(Sw) --\n",to,stdout);
*/
    Ddi_Free(from);
    from = Ddi_BddDiff(to,reached);
    Ddi_BddOrAcc(reached,to);

    Ddi_Free(to);

    Ddi_BddPrintDbg("-- Reached --\n",reached,stdout);
  }

  Ddi_Free(from);
  Ddi_Free(reached);
  Ddi_Free(TR);
  Ddi_Free(quantify);
  Ddi_Free(psv);
  Ddi_Free(nsv);
  i=Ddi_MgrReadExtRef(ddm);
  printf("Number of Ext. Ref. found: %d\n",i);

  /**********************************************************************/
  /*                       Close DDI manager                            */
  /**********************************************************************/
/*
  Ddi_MgrPrintAllocStats(ddm,stdout);
*/
  if (Ddi_MgrConsistencyCheck(ddm)==0) {
    fprintf (stderr, "Error found in manager check");
    return (0);
  }

  printf("\n*** Freeing DDI manager...");fflush(stdout);

  Ddi_MgrQuit(ddm);

  printf("done\n\n");

  printf("************* DDI LEFT-RIGHT SHIFT TEST end *****************\n");

  return(ret);
}

/**Function*******************************************************************
  Synopsis    [Read expression from file and test expression handling]
  Description [Read expression from file and test expression handling]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static int
TestExpr (
)
{
  int ret=1;
  Ddi_Mgr_t *ddm; 
  int varNum, slotNum, cacheSize, memorySizeMax;
  Ddi_Expr_t *e, *e1, *e2;

  /**********************************************************************/
  /*                           Start test                               */
  /**********************************************************************/

  printf("***************** EXPR TEST start **********************\n");

  /**********************************************************************/
  /*                        Create DDI manager                          */
  /**********************************************************************/

  printf("\n*** Creating DDI manager with 0 variables...");
  fflush(stdout);

  varNum = 0;
  slotNum = DDI_UNIQUE_SLOTS;
  cacheSize = DDI_CACHE_SLOTS;
  memorySizeMax = 0;

  ddm = Ddi_MgrInit("DDI_counter_manager",NULL,varNum,slotNum,cacheSize,
    memorySizeMax);

  printf("done\n");

  /**********************************************************************/
  /*                       Create expression                            */
  /**********************************************************************/

  printf("\n*** Creatring expression...\n");
  fflush(stdout);

  /**********************************************************************/
  /*                        Load expression from file                   */
  /**********************************************************************/

  printf("\n*** Loading expression from file expr.txt...");
  fflush(stdout);

  e = Ddi_ExprLoad(ddm,"expr.txt",NULL);
  if (e != NULL) {
    printf("done\n*** Printing expression...\n");
    Ddi_ExprPrint(e,stdout);
    Ddi_Free(e);
    printf("\n...done\n");
  }
  else {
    printf("The expression is NULL\n");
  }
  /**********************************************************************/
  /*                        Generate expression                         */
  /**********************************************************************/

  printf("\n*** Generating expression...");
  fflush(stdout);
  e1 = Ddi_ExprMakeFromString(ddm,"A");
  Ddi_SetName(e1,"op-A");
  e2 = Ddi_ExprMakeFromString(ddm,"B");
  Ddi_SetName(e2,"op-B");

  e = Ddi_ExprCtlMake(ddm,0,e1,e2,NULL);
  Ddi_SetName(e,"A UNTIL B");

  printf("done\n*** Printing expression...\n");
  Ddi_ExprPrint(e,stdout);
  Ddi_Free(e);
  printf("\n...done\n");

  Ddi_Free(e1);
  Ddi_Free(e2);


  Ddi_MgrQuit(ddm);

  printf("***************** DDI EXPR TEST end   **********************\n");

  return(ret);

}

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/







