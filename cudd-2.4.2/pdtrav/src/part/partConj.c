/**CFile**********************************************************************

  FileName    [partConj.c]

  PackageName [part]

  Synopsis    [Conjunctively partitioned functions]

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
#include "ddiInt.h"

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


/**AutomaticEnd***************************************************************/

  
/*----------------------------------------------------------------------------*/
/* Definition of internal function                                            */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Definition of External function                                            */
/*----------------------------------------------------------------------------*/

/**Function*******************************************************************

  Synopsis    [Compute the multiway and-exist over a conjunctively partitioned
    function]

  Description []

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/ 

Ddi_Bdd_t *
Part_BddMultiwayLinearAndExist (
  Ddi_Bdd_t *FPart           /* Input partitioned function */,
  Ddi_Varset_t *smoothV     /* Var Set */,
  int threshold             /* Size threshold for result factor */
)
{
  Ddi_Bdd_t *Ris;
  Ddi_Bdd_t *F,                /* an element in the BDDs'array */
           *P,*Pnew;          /* product of BDDs */
  Ddi_Varset_t *smoothF, *smoothPF, *suppF, *suppP, *suppRis,
    *suppNext, *smooth;
  Ddi_Varset_t **EarlySm;   /* early smoothed variable sets */
  Ddi_Mgr_t *dd;
  int i, size, n;
  int extRef;
  Pdtutil_VerbLevel_e verbosity;

  /* dd Manager */
  dd = Ddi_ReadMgr(FPart);
  Pdtutil_Assert(dd==Ddi_ReadMgr(smoothV), 
    "Bug: different DDI managers in multiway and-exist");

  /*Ddi_MgrPrintExtRef(dd);*/

  verbosity = Ddi_MgrReadVerbosity(dd);
  extRef = Ddi_MgrReadExtRef(dd);
  Ddi_MgrPeakProdLocalReset(dd);

  if (Ddi_BddIsMono(FPart)) {
    return (Ddi_BddExist (FPart, smoothV));
  }

  n = Ddi_BddPartNum (FPart);
  Pdtutil_Assert (n>=1, "Bug: no partitions in multiway and-exist");

  /*
   *  Generate vector of early smoothable variable sets, analyzing
   *  Partitions in reverse order.
   */

  EarlySm = Pdtutil_Alloc (Ddi_Varset_t *, n);

  suppNext = Ddi_VarsetVoid (dd);

  for (i=n-1; i>=0; i--) {

    /* take the support of i-th function */
    suppF = Ddi_BddSupp (Ddi_BddPartRead(FPart,i));
     
    Ddi_VarsetIntersectAcc(suppF,smoothV);

    /* Isolate variables not in the following functions for early smoothing */
    EarlySm[i] = Ddi_VarsetDiff(suppF,suppNext);

    /* Update support of following functions */
     
    Ddi_VarsetUnionAcc(suppNext,suppF);

    Ddi_Free (suppF);
  } /* End for i */

  Ddi_Free (suppNext);

  Ris = Ddi_BddMakePartConjVoid(dd);
  P = Ddi_BddDup(Ddi_MgrReadOne(dd));
  suppP = Ddi_VarsetVoid(dd);
  suppRis = Ddi_VarsetVoid(dd);

  for (i=0; i<n; i++) {

    F = Ddi_BddDup (Ddi_BddPartRead (FPart, i));

    if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
      fprintf (stdout, "{%d", Ddi_BddSize(F));
      fflush (stdout);
    }

    /* compute smooth varsets */
    smooth = Ddi_VarsetDiff (EarlySm[i], suppRis);

    /*Ddi_VarsetPrint(smooth, 0, NULL, stdout);*/

    smoothF = Ddi_VarsetDiff (smooth, suppP);
    smoothPF = Ddi_VarsetIntersect(smooth, suppP);
    Ddi_Free (smooth);
    Ddi_Free (EarlySm[i]);

    /* Early smoothing on F */
    if (!(Ddi_VarsetIsVoid (smoothF))) {
      Ddi_BddExistAcc (F, smoothF);

      size = Ddi_BddSize (F);
      Ddi_MgrPeakProdUpdate(dd,size);

      if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
        fprintf (stdout, "<%d>%d}", Ddi_VarsetNum(smoothF), size);
        fflush (stdout);
      }

    } else {
      if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
        fprintf (stdout, "}");
        fflush (stdout);
      }
    }

    Ddi_Free (smoothF);

    size = Ddi_BddSize (F);
    Ddi_MgrPeakProdUpdate(dd,size);

    if ((size>threshold)&&(threshold>=0)) {
      Ddi_Free (smoothPF);
      Ddi_Free (EarlySm[i]);
      Pnew = Ddi_BddDup(P);
    }
    else {

#if 0
    if (!(Ddi_VarsetIsVoid (smoothPF))) {
#else
      if (1) {
#endif
      /* Early smoothing on P and F */
      if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
        fprintf (stdout, "(%d)", Ddi_VarsetNum(smoothPF));
        fflush (stdout);
      }
      /*
      Ddi_ProfileSetCurrentPart (F, i-1);
      fprintf (stdout, "##%d-%d##", i, Ddi_ProfileReadCurrentPart (F));
      */
      if (threshold>=0)
        Ddi_MgrAbortOnSiftEnable(dd);
      Pnew = Ddi_BddAndExist (P, F, smoothPF);
      if (dd->abortedOp) {
        size = threshold+1;
      }
      else {
        size = Ddi_BddSize (Pnew);
      }
      if (threshold>=0)
        Ddi_MgrAbortOnSiftDisable(dd);
#if 0
      if (Ddi_MetaActive(dd)/*&&(Ddi_BddSize(P)>800000)*/) {
        Ddi_Bdd_t *PM = Ddi_BddMakeMeta(P);
        DdiMetaAndExistAcc(PM,F,smoothPF);
        Ddi_BddFromMeta(PM);
        Pdtutil_Assert(Ddi_BddEqual(PM,Pnew),
          "Wrong result of META-AND-EXIST");
        Ddi_Free(PM);
      }
#endif
    } else {
      /* No early smoothing on P and F */  
      /*Ddi_ProfileSetCurrentPart (F, i-1);*/
      if (threshold>=0)
        Ddi_MgrAbortOnSiftEnable(dd);
      Pnew = Ddi_BddAnd (P, F);
      if (dd->abortedOp) {
        size = threshold+1;
      }
      else {
        size = Ddi_BddSize (Pnew);
      }
      if (threshold>=0)
        Ddi_MgrAbortOnSiftDisable(dd);
    }

    Ddi_Free (smoothPF);
    }

    if ((size>threshold)&&(threshold>=0)) {

      Ddi_VarsetUnionAcc(suppRis,suppP);

      if (!Ddi_BddIsOne(P)) {
        Ddi_BddPartInsertLast(Ris,P);
      }

      Ddi_Free (Pnew);
      Ddi_Free (P);
      P = Ddi_BddDup(F);

      if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
        fprintf (stdout, "(+%d)-", size-threshold);
        fflush (stdout);
      }

    } else { 

      Ddi_Free (P);
      P = Pnew;

      Ddi_MgrPeakProdUpdate(dd,size);

      if (verbosity >= Pdtutil_VerbLevelAppMed_c) {
        fprintf (stdout, "=%d*", size);
        fflush (stdout);

#if 0
        if (Ddi_MetaActive(dd)/*&&(Ddi_BddSize(P)>800000)*/) {
          Ddi_BddSetMeta(P);
          fprintf (stdout, "$M:%d$", Ddi_BddSize (P));
          fflush (stdout);
          Ddi_BddFromMeta(P);
        }
#endif
      }

    }

    suppP = Ddi_VarsetEvalFree(Ddi_BddSupp(P), suppP);

    Ddi_Free (F);

  } /* end-for(i) */

  Pdtutil_Free (EarlySm);

  Ddi_BddPartInsertLast(Ris,P);
  Ddi_Free (P);
  Ddi_Free (suppP);
  Ddi_Free (suppRis);

#if 0
  if ((!Ddi_BddIsMono(Ris))&&(Ddi_BddPartNum(Ris)==1)) {
    Ddi_BddSetMono (Ris);
  }
#endif

  
  /*Ddi_MgrPrintExtRef(dd);*/
  Ddi_MgrCheckExtRef(dd,extRef+1);


  return (Ris);
}


/**Function*******************************************************************

  Synopsis    []

  Description []

  SideEffects [none]

  SeeAlso     []

*****************************************************************************/ 

Ddi_Bdd_t *
Part_BddDisjSuppPart (
  Ddi_Bdd_t *f              /* a BDD */,
  Ddi_Bdd_t *TR             /* a Clustered Transition Relation */,
  Ddi_Vararray_t *psv      /* array of present state variables */,
  Ddi_Vararray_t *nsv      /* array of next state variables */,
  int verbosity            /* level of verbosity */
)
{
  int i, n;
  Ddi_Bdd_t *f2, *fh, *fl;
  Ddi_Varset_t *suppf, *suppl, *supph;

  Pdtutil_Assert (Ddi_BddIsPartConj(TR),
    "Conjuntion of BDDs expected");

  suppf = Ddi_BddSupp(f);
  suppl = Ddi_VarsetDup(suppf);

  fprintf (stdout, "%d support vars found\nHow many keep ? ",
    Ddi_VarsetNum(suppf));
  fflush(stdout);
  scanf ("%d",&n);
  fprintf (stdout, "-> %d\n", n);
  for(i=0;i<n;i++) {
    suppl = Ddi_VarsetEvalFree(Ddi_VarsetRemove(suppl,
             Ddi_VarsetTop(suppl)),suppl);
  }
  supph = Ddi_VarsetDiff(suppf,suppl);


  fh = Ddi_BddExist(f,suppl);
  fl = Ddi_BddExist(f,supph);
  printf ("FH: [%d] - FL: [%d]\n", Ddi_BddSize(fh),Ddi_BddSize(fl));
  f2 = Ddi_BddConstrain(f,fh);
  printf ("F|FH: [%d]\n", Ddi_BddSize(f2));
  Ddi_BddConstrainAcc(f2,fl);
  printf ("F|FL: [%d]\n", Ddi_BddSize(f2));
  Ddi_Free (f2);

  f2 = Ddi_BddNot(f);

  fh = Ddi_BddExist(f2,suppl);
  fl = Ddi_BddExist(f2,supph);
  Ddi_Free (f2);
  printf ("~FH: [%d] - ~FH: [%d]\n", Ddi_BddSize(fh), Ddi_BddSize(fl));
  f2 = Ddi_BddConstrain(f,fh);
  printf ("F|~FH: [%d]\n", Ddi_BddSize(f2));
  Ddi_Free (f2);
  f2 = Ddi_BddConstrain(f,fl);
  printf ("F|~FL: [%d]\n", Ddi_BddSize(f2));
  Ddi_Free (f2);

  Ddi_Free (f);

#if 0
  f2 = Ddi_BddDup(f);

  for (i=0;i<Ddi_BddPartNum(TR);i++) {
    t = Ddi_BddSwapVars(Ddi_BddPartRead(TR,i),nsv,psv);
    supp = Ddi_BddSupp(t);
    smooth = Ddi_VarsetDiff(suppf,supp);
    Ddi_Free (t);
    part = Ddi_BddExist(f2,smooth);
    Ddi_BddConstrainAcc(f2,part);
    printf ("Partition %d: (%d) - [%d]\n", i,
            Ddi_BddSize(part),Ddi_BddSize(f2));
    Ddi_Free (part);
    Ddi_Free (supp);
    Ddi_Free (smooth);
  }

  printf ("Trying Disj on ~F\n");

  f2 = Ddi_BddNot(f);

  for (i=0;i<Ddi_BddPartNum(TR);i++) {
    t = Ddi_BddSwapVars(Ddi_BddPartRead(TR,i),nsv,psv);
    supp = Ddi_BddSupp(t);
    smooth = Ddi_VarsetDiff(suppf,supp);
    Ddi_Free (t);
    part = Ddi_BddExist(f2,smooth);
    Ddi_BddConstrainAcc(f2,part);
    printf ("Partition %d: (%d) - [%d]\n", i,
            Ddi_BddSize(part),Ddi_BddSize(f2));
    Ddi_Free (part);
    Ddi_Free (supp);
    Ddi_Free (smooth);
  }
#endif

  Ddi_Free (suppf);
  return (NULL);
}



