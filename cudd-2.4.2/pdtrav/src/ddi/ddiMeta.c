/**CFile***********************************************************************

  FileName    [ddiMeta.c]

  PackageName [ddi]

  Synopsis    [Functions working on Meta BDDs]

  Description [Functions working on Meta BDDs]

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

#include "ddiInt.h"
#include "part.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define META_REVERSE 0
#define HEURISTIC1 0
#define HEURISTIC2 1
#define HEURISTIC3 1

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************
  Synopsis    [Meta convert direction.]
******************************************************************************/

typedef enum {
  Meta2Bdd_c,
  Bdd2Meta_c
}
Meta_Convert_e;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static int enableUpdate = 1;
static int strongSimplify = 1;
  
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define META_MAX(a,b) (((a)<(b))?(b):(a))
#define META_MIN(a,b) (((a)<(b))?(a):(b))


#define LOG_BDD(f,msg) \
\
    {\
      Ddi_Varset_t *supp;\
\
      printf(msg);\
      printf("\n");\
      if (f==NULL) \
        printf("NULL!\n");\
      else if (Ddi_BddIsZero(f)) \
        printf("ZERO!\n");\
      else if (Ddi_BddIsOne(f)) \
        printf("ONE!\n");\
      else {\
        printf("Size: %d\nSupport\n",Ddi_BddSize(f));\
        /* print support variables */\
        supp = Ddi_BddSupp(f);\
        Ddi_VarsetPrint(supp, 0, NULL, stdout);\
        Ddi_Free(supp);\
      }\
    }

#define CHECK_META(f_meta,f_bdd) {\
  Ddi_Bdd_t *tmp;\
  tmp = Ddi_BddMakeFromMeta(f_meta);\
  Pdtutil_Assert(Ddi_BddEqual(tmp,f_bdd),\
    "Re-converted Meta BDD does not match original one");\
  Ddi_Free(tmp);\
}
  
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static Ddi_Generic_t * MetaConvert(Ddi_Generic_t *f, Meta_Convert_e sel);
static void MetaConvertBdd(Ddi_Generic_t *f, Meta_Convert_e sel);
static void MetaFromMono(Ddi_Bdd_t *f);
static void MetaReduce(Ddi_Bdd_t *f, int init);
static void MetaSimplify(Ddi_Bdd_t *f, int init, int end);
static void MetaSetConj(Ddi_Bdd_t *f);
static void MetaToMono(Ddi_Bdd_t *f);
static int MetaUpdate(Ddi_Mgr_t *ddm);
static Ddi_Bdd_t * MetaLinearAndExistAcc(Ddi_Bdd_t *fMeta, Ddi_Bdd_t *gBdd, Ddi_Varset_t *smooth);
static Ddi_Bdd_t * MetaConstrainOpt(Ddi_Bdd_t *f, Ddi_Bdd_t *g);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************
  Synopsis    [Free meta struct and pointed arrays: one, zero and dc.]
  Description [Free meta struct and pointed arrays: one, zero and dc.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
void
DdiMetaFree (
  Ddi_Meta_t *m
)
{
  Ddi_Unlock(m->one);
  Ddi_Unlock(m->zero);
  Ddi_Unlock(m->dc);
  Ddi_Free(m->one);
  Ddi_Free(m->zero);
  Ddi_Free(m->dc);
  Pdtutil_Free(m);
}

/**Function********************************************************************
  Synopsis    [Duplicate meta struct]
  Description [Duplicate meta struct]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Meta_t *
DdiMetaDup (
  Ddi_Meta_t *m
)
{
  Ddi_Meta_t *newm;

  newm = Pdtutil_Alloc(Ddi_Meta_t,1);
  newm->one = Ddi_BddarrayDup(m->one);
  newm->zero = Ddi_BddarrayDup(m->zero);
  newm->dc = Ddi_BddarrayDup(m->dc);
  newm->metaId = m->metaId;
  Ddi_Lock(newm->one);
  Ddi_Lock(newm->zero);
  Ddi_Lock(newm->dc);

  return(newm);
}

/**Function********************************************************************
  Synopsis    [Return support of Meta BDD]
  Description [Return support of Meta BDD]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Varset_t *
DdiMetaSupp (
  Ddi_Meta_t *m
)
{
  Ddi_Varset_t *supp, *aux;

  supp = Ddi_BddarraySupp(m->one);
  aux = Ddi_BddarraySupp(m->zero);
  Ddi_VarsetUnionAcc(supp,aux);
  Ddi_Free(aux);
  aux = Ddi_BddarraySupp(m->dc);
  Ddi_VarsetUnionAcc(supp,aux);
  Ddi_Free(aux);
  return(supp);

}


/**Function********************************************************************
  Synopsis    [Complement (not) meta BDD. Result accumulated]
  Description [Complement (not) meta BDD. Result accumulated. Operation is
    achieved by swapping the one/zero arrays]
  SideEffects []
  SeeAlso     []
******************************************************************************/
void
DdiMetaDoCompl (
  Ddi_Meta_t *m
)
{
  Ddi_Bddarray_t *tmp;

  tmp = m->one;
  m->one = m->zero;
  m->zero = tmp;
}


/**Function********************************************************************
  Synopsis    [Return true if meta BDD is constant]
  Description [Return true if meta BDD is constant. Phase = 1 for one, 0 for
    zero constant.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
int
DdiMetaIsConst (
  Ddi_Bdd_t *fMeta,
  int phase
)
{
  Ddi_Bddarray_t *tmp;
  int i;
  Ddi_Mgr_t *ddm =  Ddi_ReadMgr(fMeta);

  if (phase == 0) {
    tmp = fMeta->data.meta->one;
  }
  else {
    tmp = fMeta->data.meta->zero;
  }

  for (i=0; i<ddm->meta.groupNum; i++) {
    if (!Ddi_BddIsZero(Ddi_BddarrayRead(tmp,i))) {
      return(0);
    }
  }
  return(1);
}



/**Function********************************************************************
  Synopsis    [Operate And-Exist between Meta BDD and monolithic BDD]
  Description [Operate And-Exist between Meta BDD and monolithic BDD]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
DdiMetaAndExistAcc (
  Ddi_Bdd_t *fMeta,
  Ddi_Bdd_t *gBdd,
  Ddi_Varset_t *smooth
)
{
  int i, j, ng, peak=0, size, initSize;
  Ddi_Bdd_t *dc, *gg, *ff, *tmp, *up;
  Ddi_Varset_t *metaGrp, *smooth_i, *downTot, *smooth_down, 
    *smooth_down_up, *aux, *range;
  Ddi_Varsetarray_t *downGroups;
  Ddi_Mgr_t *ddm; 
  Ddi_Meta_t *m;
  int extRef, nodeId;
  Pdtutil_VerbLevel_e verbosity;

  Pdtutil_Assert(fMeta->common.code == Ddi_Bdd_Meta_c,
    "Operation requires Meta BDD!");
#if 0
  Pdtutil_Assert((gBdd == NULL)||(gBdd->common.code == Ddi_Bdd_Mono_c)
    ||(gBdd->common.code == Ddi_Bdd_Part_Conj_c),
    "Operation requires monolithic or cpart BDD!");
#endif
  ddm = Ddi_ReadMgr(fMeta);

  if (fMeta->data.meta->metaId != ddm->meta.id) {
    fMeta->data.meta->metaId = ddm->meta.id;
    DdiMetaAndExistAcc (fMeta,NULL,NULL);
  }

  initSize = Ddi_BddSize(fMeta);

#if 0
  if ((gBdd!=NULL)&&(Ddi_BddIsPartConj(gBdd))) {
    return (MetaLinearAndExistAcc (fMeta,gBdd,smooth));
  }
#endif

  verbosity = Ddi_MgrReadVerbosity(ddm);

  extRef = Ddi_MgrReadExtRef(ddm);
  nodeId = Ddi_MgrReadCurrNodeId(ddm);
  ng = ddm->meta.groupNum;

  if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "$AE-M:");
    fflush(stdout);
  }

  if (smooth == NULL) {
    smooth = Ddi_VarsetVoid(ddm);
  }
  else {
    smooth = Ddi_VarsetDup(smooth);
  }

  if (gBdd == NULL) {
    gg = Ddi_BddMakeConst(ddm,1);
  }
  else {
    if (Ddi_BddIsMeta(gBdd)) {
      Ddi_Bddarray_t *aa;
      MetaSetConj(gBdd);
      gg = Ddi_BddMakePartConjVoid(ddm);
      aa = gBdd->data.meta->zero;
      for (i=0; i<ng; i++) {
        dc = Ddi_BddNot(Ddi_BddarrayRead(aa,i));
        Ddi_BddPartInsertLast(gg,dc);
        Ddi_Free(dc);
      }
    }
    else {
      gg = Ddi_BddDup(gBdd);
    }
  }

  MetaSetConj(fMeta);
  m = fMeta->data.meta;

#if HEURISTIC1
  if (Ddi_VarsetNum(smooth) == 0) {
#else
#if HEURISTIC2
    if (0) {
#else
    if (gBdd!=NULL) {
#endif
#endif
    if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
      fprintf (stdout, "[MA(%d):", Ddi_VarsetNum(smooth));
      fflush(stdout);
    }
    dc = Ddi_BddNot(Ddi_BddarrayRead(m->zero,ng-1));
    dc = Ddi_BddAndAcc(dc,gg);
    if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
      fprintf (stdout, "%d]", Ddi_BddSize(dc));
      fflush(stdout);
    }
    Ddi_BddarrayWrite(m->dc,ng-1,dc);
    Ddi_BddarrayWrite(m->zero,ng-1,Ddi_BddNotAcc(dc));
    Ddi_Free(dc);
    Ddi_Free(gg);

    if (Ddi_VarsetNum(smooth) == 0) {
      MetaReduce(fMeta,-1);
      MetaSimplify(fMeta,0,ng-1);
      Ddi_Free(smooth);

      Ddi_MgrCheckExtRef(ddm,extRef);

      return (fMeta);
    }
    else {
#if 0
      if (MetaUpdate(ddm)) {
        DdiMetaAndExistAcc (fMeta,NULL,NULL);
      }
#endif
      /*Ddi_VarsetPrint(smooth, 0, NULL, stdout);*/
      MetaReduce(fMeta,-1);
      /*MetaSimplify(fMeta,0);*/
    }
    gg = Ddi_BddMakeConst(ddm,1);
  }

  ff = Ddi_BddMakePartConjVoid(ddm);

  range = Ddi_BddSupp(fMeta);
  aux = Ddi_BddSupp(gg);
  Ddi_VarsetUnionAcc(range,aux);
  Ddi_Free(aux);
  Ddi_VarsetDiffAcc(range,smooth);

  downGroups = Ddi_VarsetarrayAlloc(ddm,ng);
  downTot = Ddi_VarsetVoid(ddm);


  for (i=ng-1; i>0; i--) {
    Ddi_VarsetarrayWrite(downGroups,i,downTot);
    metaGrp = Ddi_VarsetarrayRead(ddm->meta.groups,i);
    Ddi_VarsetUnionAcc(downTot,metaGrp);

#if 0
    zero = Ddi_BddarrayRead(m->zero,i);
    aux = Ddi_VarsetIntersect(metaGrp,smooth);
    printf ("/SM[%d]=%d|f|=%d:",i,Ddi_VarsetNum(aux),Ddi_BddSize(zero));
    Ddi_Free(aux);
    aux = Ddi_BddSupp(zero);
    Ddi_VarsetIntersectAcc(aux,smooth);
    printf ("%d|",Ddi_VarsetNum(aux));
    Ddi_Free(aux);
    aux = Ddi_BddSupp(zero);
    Ddi_VarsetPrint(aux, 0, NULL, stdout);
    Ddi_Free(aux);
#endif
  }
  Ddi_VarsetarrayWrite(downGroups,0,downTot);
  Ddi_Free(downTot);

  for (i=0; i<ng; i++) {
    dc = Ddi_BddNot(Ddi_BddarrayRead(m->zero,i));
    Ddi_BddPartInsertLast(ff,dc);
    Ddi_Free(dc);
  }

  for (i=0; i<ng; i++) {

    if (verbosity >= Pdtutil_VerbLevelDevMin_c) {
      fprintf (stdout, " MAE[%d] ", i);
      fflush(stdout);
    }

#if 1
    if ((enableUpdate)&&/*(i<ng-1)&&*/(MetaUpdate(ddm))) {
      Ddi_Bdd_t *fMeta2;
      int k;
      Ddi_Meta_t *m2;

      printf("START UPDATE\n");

      /*enableUpdate = 0;*/
      if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
        fprintf (stdout, "(%d)", peak);
        fflush(stdout);
      }

      fMeta2 = Ddi_BddDup(fMeta);
      m2 = fMeta2->data.meta;

      for (k=0;k<i;k++) {
        Ddi_BddarrayWrite(m2->one,k,Ddi_MgrReadZero(ddm));
        Ddi_BddarrayWrite(m2->zero,k,Ddi_MgrReadZero(ddm));
        Ddi_BddarrayWrite(m2->dc,k,Ddi_MgrReadOne(ddm));
      }
      for (k=i;k<ng-1;k++) {
        Ddi_BddarrayWrite(m2->one,k,Ddi_MgrReadZero(ddm));
        dc = Ddi_BddPartExtract(ff,0);
        Ddi_BddarrayWrite(m2->dc,k,dc);
        Ddi_BddarrayWrite(m2->zero,k,Ddi_BddNotAcc(dc));
        Ddi_Free(dc);
        Ddi_BddarrayWrite(m->one,k,Ddi_MgrReadZero(ddm));
        Ddi_BddarrayWrite(m->zero,k,Ddi_MgrReadZero(ddm));
        Ddi_BddarrayWrite(m->dc,k,Ddi_MgrReadOne(ddm));
      }
      dc = Ddi_BddPartExtract(ff,0);
      Ddi_BddarrayWrite(m2->one,k,dc);
      Ddi_BddarrayWrite(m2->dc,k,Ddi_MgrReadZero(ddm));
      Ddi_BddarrayWrite(m2->zero,k,Ddi_BddNotAcc(dc));
      Ddi_Free(dc);
      Ddi_BddarrayWrite(m->dc,k,Ddi_MgrReadZero(ddm));
      Ddi_BddarrayWrite(m->zero,k,Ddi_MgrReadZero(ddm));
      Ddi_BddarrayWrite(m->one,k,Ddi_MgrReadOne(ddm));

      Pdtutil_Assert(Ddi_BddPartNum(ff)==0,"ff is not void");
      Ddi_Free(ff);
      Ddi_Free(range);
      Ddi_Free(downGroups);

      /*DdiMetaAndExistAcc (fMeta2,NULL,NULL);*/

      DdiMetaAndExistAcc (fMeta2,gg,smooth);
      Ddi_Free(gg);
      Ddi_Free(smooth);
      /*DdiMetaAndExistAcc (fMeta,NULL,NULL);*/

      Ddi_BddAndAcc(fMeta,fMeta2);
      Ddi_Free(fMeta2);

      printf("\nEND UPDATE\n");

      enableUpdate = 1;
      return;
    }
#endif

    dc = Ddi_BddPartExtract(ff,0);

    if (Ddi_BddIsOne(dc)&&(i<ng-1)) {
      Ddi_BddarrayWrite(m->one,i,Ddi_MgrReadZero(ddm));
      Ddi_BddarrayWrite(m->zero,i,Ddi_MgrReadZero(ddm));
      Ddi_BddarrayWrite(m->dc,i,Ddi_MgrReadOne(ddm));
      Ddi_Free(dc);
      continue;
    }

    if (i<ng-1) {
      downTot = Ddi_BddSupp(ff);
    }
    else {
      downTot = Ddi_VarsetVoid(ddm);
    }
    smooth_i = Ddi_VarsetDiff(smooth,downTot);
    /* Ddi_VarsetDiffAcc(smooth,smooth_i); */

    smooth_down = Ddi_VarsetarrayRead(downGroups,i);
    smooth_down = Ddi_VarsetUnion(smooth_down,smooth);

#if 0
    tmp = Ddi_BddAndExist(gg,dc,smooth_i);
    if (((size=Ddi_BddSize(tmp)) < 3*Ddi_BddSize(gg))||(size<3*initSize)) {
      Ddi_Free(gg);
      gg = tmp;
      Ddi_Free(dc);
      dc = Ddi_BddMakeConst(ddm,1);
printf("#");fflush(stdout);
    }
    else {
      Ddi_Free(tmp);
#else 
    {
#endif
#if 0
      Ddi_MgrAutodynDisable(ddm);
      tmp = Ddi_BddCofexist(gg,dc,smooth);
      if (((size=Ddi_BddSize(tmp)) < 5*Ddi_BddSize(gg))||(size<1*initSize)) {
        Ddi_BddExistAcc(dc,smooth);
        Ddi_Free(gg);
        gg = tmp;
        Ddi_BddConstrainAcc(ff,dc); /* !!! ???*/
	/*printf("@");fflush(stdout);*/
      }
      else {
      printf("<@%d(%d)>",Ddi_BddSize(tmp)/Ddi_BddSize(gg),Ddi_BddSize(tmp));
        Ddi_Free(tmp);
        Ddi_BddSetPartConj(gg);
        Ddi_BddPartInsert(gg,0,dc);
        Ddi_Free(dc);
        dc = Ddi_BddMakeConst(ddm,1);
printf("?");fflush(stdout);
      }
      Ddi_MgrAutodynEnable(ddm, Ddi_ReorderingMethodString2Enum("sift"));
#else
      Ddi_BddSetPartConj(gg);
      if (!Ddi_BddIsOne(dc)) {
        Ddi_BddPartInsert(gg,1,dc);
        Ddi_Free(dc);
        dc = Ddi_BddMakeConst(ddm,1);
      }
#if 1

#if HEURISTIC3
      /*if (strongSimplify) {*/
        for (j=0; j<i; j++) {
          tmp = Ddi_BddarrayRead(m->dc,j);
          gg = Ddi_BddEvalFree(MetaConstrainOpt(gg,tmp),gg);
        }
      /*}*/
#endif
      /*Ddi_MgrAutodynSuspend(ddm);*/
      Ddi_MgrSetExistClustThresh(ddm,META_MAX(4*Ddi_BddSize(gg),4*initSize));
      Ddi_MgrSetVerbosity(ddm,Pdtutil_VerbLevelUsrMin_c);
      Ddi_BddExistAcc(gg,smooth_i);
      Ddi_MgrSetVerbosity(ddm,verbosity);
      Ddi_MgrSetExistClustThresh(ddm,-1);
      /*Ddi_MgrAutodynResume(ddm);*/
      if (Ddi_BddIsMono(gg)) {
        if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
          printf("#");fflush(stdout);
        }
#if HEURISTIC3
        for (j=0; j<i; j++) {
          tmp = Ddi_BddarrayRead(m->dc,j);
          gg = Ddi_BddEvalFree(MetaConstrainOpt(gg,tmp),gg);
        }
#endif
      }
      else {
        int j;
        if ((i<ng-1)&&(i>(3*ng)/6)) {
          if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
            printf("!");fflush(stdout);
          }
          for (j=0; j<META_MIN(ng/2,Ddi_BddPartNum(ff)); j++) {
            tmp = Ddi_BddPartExtract(ff,j);
            Ddi_BddPartInsert(ff,j,Ddi_MgrReadOne(ddm));
            Ddi_BddPartInsert(gg,1,tmp);
            Ddi_Free(tmp);
  	  }
	}
        else {
          if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
            printf("&");fflush(stdout);
          }
        }
      }
#endif
#endif
    }

    Ddi_Free(downTot);

    if (strongSimplify&&(i>ng/2)) {
      int size1, size2;
      size = Ddi_BddSize(gg);

      smooth_down_up = Ddi_VarsetarrayRead(downGroups,i-1);
      smooth_down_up = Ddi_VarsetUnion(smooth_down_up,smooth);

      Ddi_MgrSetVerbosity(ddm,Pdtutil_VerbLevelUsrMin_c);
      up = Ddi_BddExist(gg,smooth_down_up);
#if 1
      for (j=0; j<i-1; j++) {
        tmp = Ddi_BddarrayRead(m->dc,j);
        up = Ddi_BddEvalFree(MetaConstrainOpt(up,tmp),up);
      }
#endif
      Ddi_MgrSetVerbosity(ddm,verbosity);
      tmp = MetaConstrainOpt(gg,up);

      size1 = Ddi_BddSize(up);
      size2 = Ddi_BddSize(tmp);

      if ((size2<0.7*size)||(size1+size2<0.9*size)) {
        if (verbosity >= Pdtutil_VerbLevelDevMin_c) {
          fprintf(stdout,"\nREDUCED UP: %d > (%d,%d)\n", size, size1, size2); 
        }
        Ddi_Free(gg); gg = tmp;
        Ddi_BddAndAcc(Ddi_BddarrayRead(m->dc,i-1),up);
#if HEURISTIC3
        MetaSimplify(fMeta,0,i-1);
#endif
        Ddi_Free(up);
        up = Ddi_BddNot(Ddi_BddarrayRead(m->dc,i-1));
        Ddi_BddarrayWrite(m->zero,i-1,up);
      }
      else {
        Ddi_Free(tmp);
      }
      Ddi_Free(up);
      Ddi_Free(smooth_down_up);
    }

    Ddi_MgrSetVerbosity(ddm,Pdtutil_VerbLevelUsrMin_c);
    tmp = Ddi_BddExist(gg,smooth_down);
    Ddi_MgrSetVerbosity(ddm,verbosity);

    Ddi_BddAndAcc(dc,tmp);
    Ddi_Free(tmp);

#if HEURISTIC3
    for (j=0; j<i; j++) {
      tmp = Ddi_BddarrayRead(m->dc,j);
      dc = Ddi_BddEvalFree(MetaConstrainOpt(dc,tmp),dc);
    }
#endif

    if (i<ng-1) {

      /*Ddi_MgrAutodynSuspend(ddm);*/
      Ddi_BddarrayWrite(m->dc,i,dc);
      Ddi_BddarrayWrite(m->one,i,Ddi_MgrReadZero(ddm));
#if !HEURISTIC3

#if 0
      tmp = Ddi_BddRestrict(ff,dc);
      if (Ddi_BddSize(tmp) < Ddi_BddSize(ff)) {
        Ddi_Free(ff);
        ff = tmp;
      }
      else {
        Ddi_Free(tmp);
      }
#endif

#if 1
      for (j=i/2; j<=i; j++) {
        tmp = Ddi_BddarrayRead(m->dc,j);
        gg = Ddi_BddEvalFree(MetaConstrainOpt(gg,tmp),gg);
      }
#else
      gg = Ddi_BddEvalFree(MetaConstrainOpt(gg,dc),gg);
#endif

#endif
      
      /*Ddi_MgrAutodynResume(ddm);*/

#if 0
#if 0
      Ddi_BddSetMono(gg);
#else
      Ddi_MgrSetExistClustThresh(ddm,META_MAX(2*Ddi_BddSize(gg),1*initSize));
      Ddi_MgrSetVerbosity(ddm,Pdtutil_VerbLevelUsrMin_c);
      Ddi_BddExistAcc(gg,smooth_i);
      Ddi_MgrSetVerbosity(ddm,verbosity);
      Ddi_MgrSetExistClustThresh(ddm,-1);
      if (Ddi_BddIsMono(gg)) {
printf("#");fflush(stdout);
      }
      else {
printf("&");fflush(stdout);
      }
#endif
#endif
    }
    else {
      Ddi_BddarrayWrite(m->one,i,dc);
      Ddi_BddarrayWrite(m->dc,i,Ddi_MgrReadZero(ddm));
    }

    Ddi_Free(smooth_i);
    Ddi_BddarrayWrite(m->zero,i,Ddi_BddNotAcc(dc));

    Ddi_Free(dc);
    Ddi_Free(smooth_down);

    if ((size = Ddi_BddSize(gg))>peak) {
      peak = size;
    }
    if (verbosity >= Pdtutil_VerbLevelDevMin_c) {
      fprintf (stdout, "<%d>", size);
      fflush(stdout);
    }

  }

  if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "(max:%d)%d->", peak, Ddi_BddSize(fMeta));
    fflush(stdout);
  }

#if 0
  if (Ddi_BddSize(fMeta)>100000) {
    Ddi_MgrSetVerbosity(ddm,Pdtutil_VerbLevelDevMin_c);
  }
#endif
  MetaReduce(fMeta,-1);
  Ddi_MgrSetVerbosity(ddm,verbosity);

#if 1
  if (strongSimplify) {
    if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
      fprintf (stdout, "%d|", Ddi_BddSize(fMeta));
      fflush(stdout);
    }

    MetaSimplify(fMeta,0,ng-1);
  }
#endif

  if (verbosity >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "%d$", Ddi_BddSize(fMeta));
    fflush(stdout);
  }

  Ddi_Free(range);
  Ddi_Free(ff);
  Ddi_Free(gg);
  Ddi_Free(smooth);
  Ddi_Free(downGroups);
#if 0
  if (Ddi_MgrCheckExtRef(ddm,extRef)==0) {
    Ddi_MgrPrintExtRef(ddm,nodeId);
  }
#endif
  return(fMeta);

}

/**Function********************************************************************
  Synopsis    [Operate compose]
  Description [Operate compose]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
DdiMetaComposeAcc (
  Ddi_Bdd_t *fMeta,
  Ddi_Vararray_t *v,
  Ddi_Bddarray_t *g
)
{
  Ddi_BddarrayOp(Ddi_BddCompose_c,fMeta->data.meta->one,v,g);
  Ddi_BddarrayOp(Ddi_BddCompose_c,fMeta->data.meta->zero,v,g);
  Ddi_BddarrayOp(Ddi_BddCompose_c,fMeta->data.meta->dc,v,g);

  return(fMeta);
}


/**Function********************************************************************
  Synopsis    [Operate variable swap]
  Description [Operate variable swap]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
DdiMetaSwapVarsAcc (
  Ddi_Bdd_t *fMeta,
  Ddi_Vararray_t *v1,
  Ddi_Vararray_t *v2
)
{
  Ddi_BddarrayOp(Ddi_BddSwapVars_c,fMeta->data.meta->one,v1,v2);
  Ddi_BddarrayOp(Ddi_BddSwapVars_c,fMeta->data.meta->zero,v1,v2);
  Ddi_BddarrayOp(Ddi_BddSwapVars_c,fMeta->data.meta->dc,v1,v2);

  return(fMeta);
}


/**Function********************************************************************
  Synopsis    [Operate variable substitution]
  Description [Operate variable substitution]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
DdiMetaSubstVarsAcc (
  Ddi_Bdd_t *fMeta,
  Ddi_Vararray_t *v1,
  Ddi_Vararray_t *v2
)
{
  Ddi_BddarrayOp(Ddi_BddSubstVars_c,fMeta->data.meta->one,v1,v2);
  Ddi_BddarrayOp(Ddi_BddSubstVars_c,fMeta->data.meta->zero,v1,v2);
  Ddi_BddarrayOp(Ddi_BddSubstVars_c,fMeta->data.meta->dc,v1,v2);

  return(fMeta);
}

/**Function********************************************************************
  Synopsis    [Operate And between two Meta BDDs]
  Description [Operate And between two Meta BDDs]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
DdiMetaAndAcc (
  Ddi_Bdd_t *fMeta,
  Ddi_Bdd_t *gMeta
)
{
#if 0
  if (gMeta->common.code != Ddi_Bdd_Meta_c) {
    return(DdiMetaAndExistAcc(fMeta,gMeta,NULL));
  }
#endif
  Ddi_BddNotAcc(fMeta);
  Ddi_BddNotAcc(gMeta);
  DdiMetaOrAcc(fMeta,gMeta);
  Ddi_BddNotAcc(fMeta);
  Ddi_BddNotAcc(gMeta);

  return(fMeta);
}

/**Function********************************************************************
  Synopsis    [Operate Or between two Meta BDDs]
  Description [Operate Or between two Meta BDDs]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
DdiMetaOrAcc (
  Ddi_Bdd_t *fMeta,
  Ddi_Bdd_t *gMeta
)
{
  int i, j, ng;
  Ddi_Bdd_t *fg0, *f0, *g0, *fg1, *f1, *g1, *dcup, *f0up, *g0up, *dc;
  Ddi_Mgr_t *ddm; 
  Ddi_Meta_t *mf, *mg;

  Pdtutil_Assert(fMeta->common.code == Ddi_Bdd_Meta_c,
    "Operation requires Meta BDD!");
#if 0
  Pdtutil_Assert(gMeta->common.code == Ddi_Bdd_Meta_c,
    "Operation requires Meta BDD!");
#endif

  ddm = Ddi_ReadMgr(fMeta);

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "$OR-M:");
    fflush(stdout);
  }

  if (fMeta->data.meta->metaId != ddm->meta.id) {
    fMeta->data.meta->metaId = ddm->meta.id;
    DdiMetaAndExistAcc (fMeta,NULL,NULL);
  }
  if (Ddi_BddIsMeta(gMeta)&&(gMeta->data.meta->metaId != ddm->meta.id)) {
    gMeta->data.meta->metaId = ddm->meta.id;
    DdiMetaAndExistAcc (gMeta,NULL,NULL);
  }
#if 0
  Ddi_MgrAutodynSuspend(ddm);
  DdiMetaAndExistAcc (fMeta,NULL,NULL);
  if (!Ddi_BddIsMono(gMeta)) {
    DdiMetaAndExistAcc (gMeta,NULL,NULL);
  }
  Ddi_MgrAutodynResume(ddm);
#endif

#if 0
  MetaSetConj(fMeta);
  MetaSetConj(gMeta);
#endif

  ng = ddm->meta.groupNum;
  mf = fMeta->data.meta;
  if (Ddi_BddIsMono(gMeta)) {
    mg = NULL;
  }
  else {
    mg = gMeta->data.meta;
  }

  f0up = Ddi_BddMakeConst(ddm,0);
  g0up = Ddi_BddMakeConst(ddm,0);
  dcup = Ddi_BddMakeConst(ddm,1);

  for (i=0; i<ng; i++) {

    if ((i<ng-1)&&Ddi_BddIsOne(Ddi_BddarrayRead(mf->dc,i))
        &&(mg!=NULL)&&Ddi_BddIsOne(Ddi_BddarrayRead(mg->dc,i))) {
      Ddi_BddarrayWrite(mf->one,i,Ddi_MgrReadZero(ddm));
      Ddi_BddarrayWrite(mf->zero,i,Ddi_MgrReadZero(ddm));
      Ddi_BddarrayWrite(mf->dc,i,Ddi_MgrReadOne(ddm));
      continue;
    }

    f1 = Ddi_BddDup(Ddi_BddarrayRead(mf->one,i));
    if (mg == NULL) {  
      if (i<(ng-1)) {
	 g1 = Ddi_BddMakeConst(ddm,0);
      }
      else {
	 g1 = Ddi_BddDup(gMeta);
      }
    }
    else {
      g1 = Ddi_BddDup(Ddi_BddarrayRead(mg->one,i));
    }

    for (j=0; j<i; j++) {
      dc = Ddi_BddarrayRead(mf->dc,j);
      f1 = Ddi_BddEvalFree(MetaConstrainOpt(f1,dc),f1);
      g1 = Ddi_BddEvalFree(MetaConstrainOpt(g1,dc),g1);
    }

    Ddi_BddDiffAcc(f1,f0up);
    Ddi_BddDiffAcc(g1,g0up);
    fg1 = Ddi_BddOr(f1,g1);
    Ddi_Free(f1);
    Ddi_Free(g1);

    f0 = Ddi_BddDup(Ddi_BddarrayRead(mf->zero,i));
    if (mg == NULL) {  
      if (i<(ng-1)) {
	 g0 = Ddi_BddMakeConst(ddm,0);
      }
      else {
	 g0 = Ddi_BddNot(gMeta);
      }
    }
    else {
      g0 = Ddi_BddDup(Ddi_BddarrayRead(mg->zero,i));
    }

    for (j=0; j<i; j++) {
      dc = Ddi_BddarrayRead(mf->dc,j);
      f0 = Ddi_BddEvalFree(MetaConstrainOpt(f0,dc),f0);
      g0 = Ddi_BddEvalFree(MetaConstrainOpt(g0,dc),g0);
    }
    Ddi_BddOrAcc(f0,f0up);
    Ddi_BddOrAcc(g0,g0up);
    fg0 = Ddi_BddAnd(f0,g0);

    Ddi_Free(dcup);
    dcup = Ddi_BddNotAcc(Ddi_BddOr(fg0,fg1));

    Ddi_Free(f0up);
    Ddi_Free(g0up);
    f0up = MetaConstrainOpt(f0,dcup);
    g0up = MetaConstrainOpt(g0,dcup);

    Ddi_Free(f0);
    Ddi_Free(g0);

    Ddi_BddarrayWrite(mf->zero,i,fg0);
    if (i<ng-1) {
      Ddi_BddarrayWrite(mf->dc,i,dcup);
      Ddi_BddarrayWrite(mf->one,i,fg1);
    }
    else {
      Ddi_BddarrayWrite(mf->one,i,Ddi_BddNotAcc(fg0));
      Ddi_BddarrayWrite(mf->dc,i,Ddi_MgrReadZero(ddm));
    }

    Ddi_Free(fg0);
    Ddi_Free(fg1);
  }

  Ddi_Free(f0up);
  Ddi_Free(g0up);
  Ddi_Free(dcup);

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "%d->", Ddi_BddSize(fMeta));
    fflush(stdout);
  }

  MetaReduce(fMeta,-1);

#if 1
  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "%d|", Ddi_BddSize(fMeta));
    fflush(stdout);
  }

  MetaSimplify(fMeta,0,ng-1);
#endif

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "%d$", Ddi_BddSize(fMeta));
    fflush(stdout);
  }

  return(fMeta);

}

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*
#define DdiMetaCopy(m,f) \
  (fprintf(stderr,"DdiMetaCopy still to implement\n"),NULL)
#define DdiMetaAnd(f,g) \
  (fprintf(stderr,"DdiMetaAnd still to implement\n"),NULL)
#define DdiMetaConstrain(f,g) \
  (fprintf(stderr,"DdiMetaConstrain still to implement\n"),NULL)
#define DdiMetaRestrict(f,g) \
  (fprintf(stderr,"DdiMetaRestrict still to implement\n"),NULL)
#define DdiMetaExist(f,g) \
  (fprintf(stderr,"DdiMetaExist still to implement\n"),NULL)
#define DdiMetaSwapVars(f,g,h) \
  (fprintf(stderr,"DdiMetaAnd still to implement\n"),NULL)
#define DdiMetaSupp(f) \
  (fprintf(stderr,"DdiMetaSupp still to implement\n"),NULL)
*/


/**Function********************************************************************
  Synopsis    [Return true if Meta handling active (Ddi_MetaInit done)]
  Description [Return true if Meta handling active (Ddi_MetaInit done)]
  SideEffects []
  SeeAlso     []
******************************************************************************/
int
Ddi_MetaActive (
  Ddi_Mgr_t *ddm
)
{
  return (ddm->meta.groups != NULL);
}

/**Function********************************************************************
  Synopsis    [Initialize Meta BDD handling in DDI manager]
  Description [Initialize Meta BDD handling in DDI manager]
  SideEffects []
  SeeAlso     []
******************************************************************************/
void
Ddi_MetaInit (
  Ddi_Mgr_t *ddm,
  Ddi_Meta_Method_e method,
  Ddi_Bdd_t *ref,
  Ddi_Varset_t *firstGroup,
  int sizeMin
)
{
  int nvar, ng, i, np, min;
  Ddi_Var_t *v;
  Ddi_Varset_t *metaGrp, *selectedVars; 
  int *used;
  Ddi_Varsetarray_t *groups;

  if (ddm->meta.groups!=NULL) {
    Ddi_Free(ddm->meta.groups);
    Pdtutil_Free(ddm->meta.ord);
  }

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    printf("Meta-Init\n");
  }

  ddm->meta.id++;

  nvar = Ddi_ReadNumVars(ddm);
  if (sizeMin > 0) {
    ddm->settings.meta.groupSizeMin = sizeMin;
  }
  ddm->settings.meta.method = method;

  ddm->meta.nvar = nvar;
  ddm->meta.ord = Pdtutil_Alloc(int,nvar);
  for (i=0; i<nvar; i++) {
    ddm->meta.ord[i] = Ddi_ReadInvPerm(ddm,i);
  }
  ng = 0;
  groups = Ddi_VarsetarrayAlloc(ddm,0); 

  switch (method) {
  case Ddi_Meta_Size:
  case Ddi_Meta_McM:
    used = Pdtutil_Alloc(int,nvar);
    for (i=0; i<nvar; i++) {
      used[i]=0;
    }
    for (i=0; i<nvar; i++) {
      v = Ddi_VarAtLevel(ddm,i);
      if (used[i]==0) {
        used[i]=1;
        if (Ddi_VarIsGrouped(v)) {
          /* if variable is in variable group take entire group */
          metaGrp = Ddi_VarReadGroup(v);
#if META_REVERSE
          Ddi_VarsetarrayInsert(groups,0,metaGrp);ng++;
#else
          Ddi_VarsetarrayWrite(groups,ng++,metaGrp);
#endif
	}
        else {
          /* variable is not grouped: add to group */
          metaGrp = Ddi_VarsetVoid(ddm);
          Ddi_VarsetAddAcc(metaGrp,v);
#if META_REVERSE
          Ddi_VarsetarrayInsert(groups,0,metaGrp);ng++;
#else
          Ddi_VarsetarrayWrite(groups,ng++,metaGrp);
#endif
          Ddi_Free(metaGrp);
	}
      }
    }
    Pdtutil_Free(used);
    break;
  case Ddi_Meta_EarlySched:
    Pdtutil_Assert(Ddi_BddIsPartConj(ref)||Ddi_BddIsPartDisj(ref),
      "Partitioned BDD required to bias early-sched");
    selectedVars = Ddi_VarsetVoid(ddm);
    if (firstGroup != NULL) {
      Ddi_VarsetUnionAcc(selectedVars,firstGroup);
      Ddi_VarsetarrayWrite(groups,ng++,firstGroup);
    }
    np = Ddi_BddPartNum(ref);
    for (i=0;i<np;i++) {
      metaGrp = Ddi_BddSupp(Ddi_BddPartRead(ref,np-1-i));
      Ddi_VarsetDiffAcc(metaGrp,selectedVars);
      Ddi_VarsetUnionAcc(selectedVars,metaGrp);
#if META_REVERSE
      Ddi_VarsetarrayInsert(groups,0,metaGrp);ng++;
#else
      Ddi_VarsetarrayWrite(groups,ng++,metaGrp);
#endif
      Ddi_Free(metaGrp);
    }
    Ddi_Free(selectedVars);
    break;
  default:
    Pdtutil_Assert(0,"unknown or not supported meta BDD method");
    break;
  }
  
  ddm->meta.groups = Ddi_VarsetarrayAlloc(ddm,0); 

  metaGrp = Ddi_VarsetVoid(ddm);
  min = ddm->settings.meta.groupSizeMin;
  for (i=Ddi_VarsetarrayNum(groups)-1; i>=0; i--) {
    Ddi_VarsetUnionAcc(metaGrp,Ddi_VarsetarrayRead(groups,i));
    if (Ddi_VarsetNum(metaGrp) >= min) {
      if ((ddm->meta.groupNum <= 0) || 
          (Ddi_VarsetarrayNum(ddm->meta.groups) < ddm->meta.groupNum-1)) {
        Ddi_VarsetarrayInsert(ddm->meta.groups,0,metaGrp);
        Ddi_Free(metaGrp);
        metaGrp = Ddi_VarsetVoid(ddm);
        min *= 1.0;
      }
    }
  }
  if (!Ddi_VarsetIsVoid(metaGrp)) {
    Ddi_VarsetarrayInsert(ddm->meta.groups,0,metaGrp);
  }

  Ddi_Free(metaGrp);
  Ddi_Free(groups);

  if (ddm->meta.groupNum >0) {
    while (Ddi_VarsetarrayNum(ddm->meta.groups) < ddm->meta.groupNum) {
      metaGrp = Ddi_VarsetVoid(ddm);
      Ddi_VarsetarrayInsertLast(ddm->meta.groups,metaGrp);
      Ddi_Free(metaGrp);
    }
  } 

  ddm->meta.groupNum = Ddi_VarsetarrayNum(ddm->meta.groups);

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    int i;
    printf("Meta-Groups\n");
    printf ("Meta Variable Groups:\n");
    for (i=0; i<Ddi_VarsetarrayNum(ddm->meta.groups); i++){
    printf("Group[%d]\n",i);
      Ddi_VarsetPrint(
        Ddi_VarsetarrayRead(ddm->meta.groups,i), 20, NULL, stdout);
    }
  }

}

/**Function********************************************************************
  Synopsis    [Close Meta BDD handling in DDI manager]
  Description [Close Meta BDD handling in DDI manager. This enables further
    opening of Meta BDD management with different method/parameters]
  SideEffects []
  SeeAlso     []
******************************************************************************/
void
Ddi_MetaQuit (
  Ddi_Mgr_t *ddm
)
{
  if (ddm->meta.groups != NULL) {

    Ddi_Free(ddm->meta.groups);
    Pdtutil_Free(ddm->meta.ord);
    ddm->meta.nvar = 0;
    ddm->meta.groupNum = 0;
    /* check if Meta BDDs are still present (not freed) */
    if (ddm->meta.bddNum != 0) {
      char buf[300];
      sprintf(buf, 
        "%d Non freed Meta BDDs present when quitting Meta handling\n",
        ddm->meta.bddNum);
      strcat(buf,"This may cause wrong Meta BDD operations\n");
      strcat(buf,"with new Meta BDDs\n");
      strcat(buf,"if meta Handling re-opened with different settings");
      Pdtutil_Warning(1,buf);
    }
  }
}

/**Function********************************************************************
  Synopsis    [Transform a BDD to Meta BDD. Result generated]
  Description [Transform a BDD to Meta BDD. Result generated]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakeMeta (
  Ddi_Bdd_t *f
)
{
  return((Ddi_Bdd_t *)MetaConvert(
    DdiGenericDup((Ddi_Generic_t *)f),Bdd2Meta_c));
}

/**Function********************************************************************
  Synopsis    [Transform a BDD to Meta BDD. Result accumulated]
  Description [Transform a BDD to Meta BDD. Result accumulated]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddSetMeta (
  Ddi_Bdd_t *f
)
{
  return((Ddi_Bdd_t *)MetaConvert((Ddi_Generic_t *)f,Bdd2Meta_c));
}


/**Function********************************************************************
  Synopsis    [Transform a BDD array to Meta BDD. Result generated]
  Description [Transform a BDD array to Meta BDD. Result generated]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bddarray_t *
Ddi_BddarrayMakeMeta (
  Ddi_Bddarray_t *f
)
{
  return((Ddi_Bddarray_t *)MetaConvert(
    DdiGenericDup((Ddi_Generic_t *)f),Bdd2Meta_c));
}

/**Function********************************************************************
  Synopsis    [Transform a BDD array to Meta BDD. Result accumulated]
  Description [Transform a BDD array to Meta BDD. Result accumulated]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bddarray_t *
Ddi_BddArraySetMeta (
  Ddi_Bddarray_t *f
)
{
  return((Ddi_Bddarray_t *)MetaConvert((Ddi_Generic_t *)f,Bdd2Meta_c));
}


/**Function********************************************************************
  Synopsis    [Transform a BDD to Meta BDD. Result generated]
  Description [Transform a BDD to Meta BDD. Result generated]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddMakeFromMeta (
  Ddi_Bdd_t *f
)
{
  return((Ddi_Bdd_t *)MetaConvert(
    DdiGenericDup((Ddi_Generic_t *)f),Meta2Bdd_c));
}

/**Function********************************************************************
  Synopsis    [Transform a BDD to Meta BDD. Result accumulated]
  Description [Transform a BDD to Meta BDD. Result accumulated]
  SideEffects []
  SeeAlso     []
******************************************************************************/
Ddi_Bdd_t *
Ddi_BddFromMeta (
  Ddi_Bdd_t *f
)
{
  return((Ddi_Bdd_t *)MetaConvert((Ddi_Generic_t *)f,Meta2Bdd_c));
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [Transform between generic DDI node and Meta BDD. 
    Result accumulated]
  Description [Transform between generic DDI node and Meta BDD. 
    Result accumulated. The transformation is applied to leaf BDDs, 
    whereas the DDI structure is kept as it is]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static Ddi_Generic_t *
MetaConvert (
  Ddi_Generic_t *f,
  Meta_Convert_e sel
)
{
  Ddi_Mgr_t *ddm; 
  Ddi_ArrayData_t *array;
  int i;

  ddm = f->common.mgr;

  switch (f->common.type) {

  case Ddi_Bdd_c:
    switch (f->common.code) {
    case Ddi_Bdd_Mono_c:
    case Ddi_Bdd_Meta_c:
      MetaConvertBdd(f,sel);
      break;
    case Ddi_Bdd_Part_Conj_c:
    case Ddi_Bdd_Part_Disj_c:
      array = f->Bdd.data.part;
      for (i=0;i<DdiArrayNum(array);i++) {
        MetaConvertBdd(DdiArrayRead(array,i),sel);
      }
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
    }
    break;
  case Ddi_Var_c:
  case Ddi_Vararray_c:
    Pdtutil_Assert (0, "Variables cannot be transformed to meta");
    break;
  case Ddi_Varset_c:
  case Ddi_Varsetarray_c:
    Pdtutil_Assert (0, "Varsets cannot be transformed to meta");
    break;
  case Ddi_Expr_c:
    switch (f->common.code) {
    case Ddi_Expr_Bdd_c:
      MetaConvertBdd(f->Expr.data.bdd,sel);
      break;
    case Ddi_Expr_String_c:
      break;
    case Ddi_Expr_Bool_c:
    case Ddi_Expr_Ctl_c:
      array = f->Expr.data.sub;
      for (i=0;i<DdiArrayNum(array);i++) {
        MetaConvertBdd(DdiArrayRead(array,i),sel);
      }
      break;
    default:
      Pdtutil_Assert (0, "Wrong DDI node type");
      break;
    }
    break;
  case Ddi_Bddarray_c:
  case Ddi_Exprarray_c:
    array = f->Bddarray.array;
    for (i=0;i<DdiArrayNum(array);i++) {
      MetaConvertBdd(DdiArrayRead(array,i),sel);
    }
    break;
  default:
    Pdtutil_Assert (0, "Wrong DDI node type");
  }

  return (f);
}

/**Function********************************************************************
  Synopsis    [Conversion between BDD and Meta BDD]
  Description [Conversion between BDD and Meta BDD]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static void
MetaConvertBdd (
  Ddi_Generic_t *f,
  Meta_Convert_e sel
)
{
  switch (sel) {
  case Meta2Bdd_c:
    if (f->common.code == Ddi_Bdd_Meta_c) {
      MetaToMono((Ddi_Bdd_t *)f);
    }
    else {
       /* Pdtutil_Warning(1,"BDD is already in BDD format"); */
    }
    break;
  case Bdd2Meta_c:
    if (f->common.code == Ddi_Bdd_Mono_c) {
      MetaFromMono((Ddi_Bdd_t *)f);
    }
    else {
       /* Pdtutil_Warning(1,"BDD is already in Meta format"); */
    }
    break;
  default:
    Pdtutil_Assert(0,"invalid conversion selection btw. BDD and Meta");
  }
}

/**Function********************************************************************
  Synopsis    [Transform a BDD to Meta BDD]
  Description [Transform a BDD to Meta BDD]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static void
MetaFromMono (
  Ddi_Bdd_t *f
)
{
  int ng, i;
  Ddi_Meta_t *m;
  Ddi_Mgr_t *ddm;
  Ddi_Bdd_t *one, *zero;
  Ddi_Bdd_t *fBdd;

  ddm = Ddi_ReadMgr(f);
  ng = ddm->meta.groupNum;
  one = Ddi_MgrReadOne(ddm);
  zero = Ddi_MgrReadZero(ddm);

  fBdd = Ddi_BddDup(f);

  m = Pdtutil_Alloc(Ddi_Meta_t,1);
  m->one = Ddi_BddarrayAlloc(ddm,ng);
  m->zero = Ddi_BddarrayAlloc(ddm,ng);
  m->dc = Ddi_BddarrayAlloc(ddm,ng);
  m->metaId = ddm->meta.id;

  for (i=0;i<ng-1;i++) {
    Ddi_BddarrayWrite(m->one,i,zero);
    Ddi_BddarrayWrite(m->zero,i,zero);
    Ddi_BddarrayWrite(m->dc,i,one);
  }

  Ddi_BddarrayWrite(m->one,ng-1,f);
  Ddi_BddarrayWrite(m->zero,ng-1,Ddi_BddNotAcc(f));
  Ddi_BddNotAcc(f);
  Ddi_BddarrayWrite(m->dc,ng-1,zero);

  Ddi_Lock(m->one);
  Ddi_Lock(m->zero);
  Ddi_Lock(m->dc);

  Cudd_RecursiveDeref (Ddi_MgrReadMgrCU(ddm),f->data.bdd);
  f->common.code = Ddi_Bdd_Meta_c;
  f->data.meta = m;

  /* apply bottom-up reduction */
  MetaReduce(f,-1);

  CHECK_META(f,fBdd);

  MetaSimplify(f,0,ng-1);

  CHECK_META(f,fBdd);

#if 0

  MetaSetConj(f);
  CHECK_META(f,fBdd);
#endif

  Ddi_Free(fBdd);

}

/**Function********************************************************************
  Synopsis    [Apply bottom-up reduction process to meta BDD]
  Description [Apply bottom-up reduction process to meta BDD]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static void
MetaReduce (
  Ddi_Bdd_t *f,
  int        init  /* start from this layer: < 0 for full reduction */
)
{
  int i, j, k, ng, oldSize, newSize, sizeJoin, noReduce, aborted;
  Ddi_Bdd_t *zero, *one, *dc, *tmp, *tmp2, *dcup, *zeroup, *oneup, 
            *onejoin, *zerojoin, *dcjoin;
  Ddi_Varset_t *smooth;
  Ddi_Mgr_t *ddm; 
  Ddi_Meta_t *m;
  Ddi_Bddarray_t *auxReduce, *auxJoin;
  Pdtutil_VerbLevel_e verbosity;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  Pdtutil_Assert(f->common.code == Ddi_Bdd_Meta_c,
    "Operation requires Meta BDD!");

  ddm = Ddi_ReadMgr(f);
  MetaUpdate(ddm);

  verbosity = Ddi_MgrReadVerbosity(ddm);

  /* Ddi_MgrAutodynSuspend(ddm); */

  m = f->data.meta;
  ng = Ddi_BddarrayNum(m->one);

  while (ng > ddm->meta.groupNum) {
    Ddi_Varset_t *voidGrp;
    ddm->meta.groupNum++;
    voidGrp = Ddi_VarsetVoid(ddm);
    Ddi_VarsetarrayInsertLast(ddm->meta.groups,voidGrp);
    Ddi_Free(voidGrp);
  }
  while (ng < ddm->meta.groupNum) {
    Ddi_BddarrayWrite(m->dc,ng,Ddi_MgrReadOne(ddm));
    Ddi_BddarrayWrite(m->one,ng,Ddi_MgrReadZero(ddm));
    Ddi_BddarrayWrite(m->zero,ng,Ddi_MgrReadZero(ddm));
    ng++;
  }

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    printf("Meta-Reduce\nInitial size: %d\n",Ddi_BddSize(f));
  }

  smooth = Ddi_VarsetVoid(ddm);

  Pdtutil_Assert(ng>1,"At least 2 groups required for meta reduce");

  for (j=ng-2; j>=0; j--) {

    Ddi_VarsetUnionAcc(smooth,Ddi_VarsetarrayRead(ddm->meta.groups,j+1));
    if ((init>=0)&&(j>=init)) {
      continue;
    }

    i = j+1;
    while ((i<ng-1)&&(Ddi_BddIsOne(Ddi_BddarrayRead(m->dc,i)))) {
      i++;
    }

    noReduce = 1;

    auxReduce = Ddi_BddarrayAlloc(ddm,4);

    one = Ddi_BddarrayRead(m->one,i);
    zero = Ddi_BddarrayRead(m->zero,i);
    dc = Ddi_BddarrayRead(m->dc,i);

    oneup = Ddi_BddarrayRead(m->one,j);
    zeroup = Ddi_BddarrayRead(m->zero,j);

    Ddi_BddarrayWrite(auxReduce,0,one);
    Ddi_BddarrayWrite(auxReduce,1,zero);
    Ddi_BddarrayWrite(auxReduce,2,oneup);
    Ddi_BddarrayWrite(auxReduce,3,zeroup);
    oldSize = Ddi_BddarraySize(auxReduce);
    
    if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
      printf("Meta-Reduce[%d] (%d|%d|%d) -> ",i,
        Ddi_BddSize(one),Ddi_BddSize(zero),Ddi_BddSize(dc)); 
    }

    dcup = Ddi_BddarrayRead(m->dc,j);

    Ddi_MgrAbortOnSiftEnable(ddm);
    tmp = Ddi_BddForall(one,smooth);
    aborted = ddm->abortedOp;
    Ddi_MgrAbortOnSiftDisable(ddm);
    noReduce = noReduce && Ddi_BddIsZero(tmp);

    if (aborted) {
      oneup = Ddi_BddDup(Ddi_BddarrayRead(m->one,j));
    }
    else {
      Ddi_MgrAbortOnSiftEnable(ddm);
      oneup = Ddi_BddIte(dcup,tmp,Ddi_BddarrayRead(m->one,j));
      aborted = ddm->abortedOp;
      Ddi_MgrAbortOnSiftDisable(ddm);
      if (aborted) {
        Ddi_Free(oneup);
        oneup = Ddi_BddDup(Ddi_BddarrayRead(m->one,j));
      }
      else {
        for (k=0; k<j; k++) {
          tmp2 = Ddi_BddarrayRead(m->dc,k);
          oneup = Ddi_BddEvalFree(MetaConstrainOpt(oneup,tmp2),oneup);
        }
      }
    }
    Ddi_Free(tmp);

    Ddi_MgrAbortOnSiftEnable(ddm);
    tmp = Ddi_BddForall(zero,smooth);
    aborted = ddm->abortedOp;
    Ddi_MgrAbortOnSiftDisable(ddm);

    noReduce = noReduce && Ddi_BddIsZero(tmp);

    if (aborted) {
      zeroup = Ddi_BddDup(Ddi_BddarrayRead(m->zero,j));
    }
    else {
      Ddi_MgrAbortOnSiftEnable(ddm);
      zeroup = Ddi_BddIte(dcup,tmp,Ddi_BddarrayRead(m->zero,j));
      aborted = ddm->abortedOp;
      Ddi_MgrAbortOnSiftDisable(ddm);
      if (aborted) {
        Ddi_Free(zeroup);
        zeroup = Ddi_BddDup(Ddi_BddarrayRead(m->zero,j));
      }
      else {
        for (k=0; k<j; k++) {
          tmp2 = Ddi_BddarrayRead(m->dc,k);
          zeroup = Ddi_BddEvalFree(MetaConstrainOpt(zeroup,tmp2),zeroup);
        }
      }
    }
    Ddi_Free(tmp);

    dcup = Ddi_BddNotAcc(Ddi_BddOr(oneup,zeroup));

    zero = MetaConstrainOpt(zero,dcup);
    one = MetaConstrainOpt(one,dcup);
    dc = Ddi_BddNotAcc(Ddi_BddOr(one,zero));

    Ddi_BddarrayWrite(auxReduce,0,one);
    Ddi_BddarrayWrite(auxReduce,1,zero);
    Ddi_BddarrayWrite(auxReduce,2,oneup);
    Ddi_BddarrayWrite(auxReduce,3,zeroup);
    newSize = Ddi_BddarraySize(auxReduce);
    Ddi_Free(auxReduce);
    
#if 1
    if ((!noReduce)&&(newSize < 0.9*oldSize)) {
#else
    if (1) {
#endif

      Ddi_BddarrayWrite(m->one,j,oneup);
      Ddi_BddarrayWrite(m->zero,j,zeroup);
      Ddi_BddarrayWrite(m->dc,j,dcup);

      Ddi_BddarrayWrite(m->one,i,one);
      Ddi_BddarrayWrite(m->zero,i,zero);
      Ddi_BddarrayWrite(m->dc,i,dc);

      if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
        printf ("  1: %5d - 0: %5d - dc: %5d \n",
          Ddi_BddSize(one),Ddi_BddSize(zero),Ddi_BddSize(dc));
      }
      else if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
        printf("-");fflush(stdout);
      }

      Ddi_Free(oneup);
      Ddi_Free(zeroup);
      Ddi_Free(dcup);
      Ddi_Free(one);
      Ddi_Free(zero);
      Ddi_Free(dc);

    }
    else {

      Ddi_Free(one);
      Ddi_Free(zero);
      Ddi_Free(dc);
      Ddi_Free(oneup);
      Ddi_Free(zeroup);
      Ddi_Free(dcup);

      dcup = Ddi_BddarrayRead(m->dc,j);
      one = Ddi_BddarrayRead(m->one,i);
      zero = Ddi_BddarrayRead(m->zero,i);
      dc = Ddi_BddarrayRead(m->dc,i);

      Ddi_MgrAbortOnSiftEnable(ddm);
      onejoin = Ddi_BddIte(dcup,one,Ddi_BddarrayRead(m->one,j));
      if (ddm->abortedOp) {
        Ddi_Free(onejoin);
        onejoin = NULL;
      }
      Ddi_MgrAbortOnSiftDisable(ddm);
      
      Ddi_MgrAbortOnSiftEnable(ddm);
      zerojoin = Ddi_BddIte(dcup,zero,Ddi_BddarrayRead(m->zero,j));
      if (ddm->abortedOp) {
        Ddi_Free(zerojoin);
        zerojoin = NULL;
      }
      Ddi_MgrAbortOnSiftDisable(ddm);
      
      sizeJoin = 2*oldSize;
      if ((onejoin != NULL)&&(zerojoin != NULL)) {
        auxJoin = Ddi_BddarrayAlloc(ddm,2);
        dcjoin = Ddi_BddNotAcc(Ddi_BddOr(onejoin,zerojoin));
        Ddi_BddarrayWrite(auxJoin,0,onejoin);
        Ddi_BddarrayWrite(auxJoin,1,zerojoin);
        sizeJoin = Ddi_BddarraySize(auxJoin);
        Ddi_Free(auxJoin);
      }
      else {
        dcjoin = NULL;
      }

      if (/*(!noReduce)&&*/(sizeJoin < 1.1*oldSize)) {
        Ddi_BddarrayWrite(m->one,i,onejoin);
        Ddi_BddarrayWrite(m->zero,i,zerojoin);
        Ddi_BddarrayWrite(m->dc,i,dcjoin);

        Ddi_BddarrayWrite(m->one,j,Ddi_MgrReadZero(ddm));
        Ddi_BddarrayWrite(m->zero,j,Ddi_MgrReadZero(ddm));
        Ddi_BddarrayWrite(m->dc,j,Ddi_MgrReadOne(ddm));

        if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
          printf("_");fflush(stdout);
        }
      }
      else {
        if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
          printf(".");fflush(stdout);
        }
      }

      Ddi_Free(onejoin);
      Ddi_Free(zerojoin);
      Ddi_Free(dcjoin);

    }


  }

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    printf("Meta-Reduce[%d] (%d|%d|%d) ",0,
      Ddi_BddSize(Ddi_BddarrayRead(m->one,0)),
      Ddi_BddSize(Ddi_BddarrayRead(m->zero,0)),
      Ddi_BddSize(Ddi_BddarrayRead(m->dc,0))); 
  }

  Ddi_Free(smooth);

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    printf("Final size: %d\n",Ddi_BddSize(f));
  }

  /* Ddi_MgrAutodynResume(ddm); */

}

/**Function********************************************************************
  Synopsis    [Apply top-down cofactor based simplification]
  Description [Apply top-down cofactor based simplification]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static void
MetaSimplify (
  Ddi_Bdd_t *f,
  int  init  /* apply simplification starting from this layer */,
  int  end
)
{
  int i, ng, limit;
  Ddi_Bdd_t *dc, *zero_i, *one_i, *dc_i;
  Ddi_Mgr_t *ddm; 
  Ddi_Meta_t *m;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  Pdtutil_Assert(f->common.code == Ddi_Bdd_Meta_c,
    "Operation requires Meta BDD!");

  m = f->data.meta;
  ddm = Ddi_ReadMgr(f);
  ng = Ddi_BddarrayNum(m->one);
  limit = META_MIN(end,ng-1);

  Pdtutil_Assert(ng == ddm->meta.groupNum,
    "Wrong number of groups in Meta BDD");
  Pdtutil_Assert(init>=0,
    "Wrong initial layer in meta-simplify");

  if (init>=limit) 
    return;

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    printf("Meta-Simplify (init: %d)\nInitial size: %d\n",
      init,Ddi_BddSize(f));
  }

  dc = Ddi_BddarrayRead(m->dc,init);

  /* Ddi_MgrAutodynSuspend(ddm); */

  for (i=init+1; i<=limit; i++) {
    Ddi_Bdd_t *tmp;
    int size;

    one_i = Ddi_BddarrayRead(m->one,i);
    zero_i = Ddi_BddarrayRead(m->zero,i);
    dc_i = Ddi_BddarrayRead(m->dc,i);

    if (Ddi_BddIsOne(dc_i)) {
      continue;
    }
    
    tmp = Ddi_BddMakePartConjVoid(ddm);
    Ddi_BddPartInsert(tmp,0,one_i);
    Ddi_BddPartInsert(tmp,1,zero_i);
    Ddi_BddPartInsert(tmp,2,dc_i);
    size = Ddi_BddSize(tmp);
    /* Ddi_BddConstrainAcc(tmp,dc); */                                 
    tmp = Ddi_BddEvalFree(MetaConstrainOpt(tmp,dc),tmp); 
    if (Ddi_BddSize(tmp) < size) {
      one_i = Ddi_BddPartRead(tmp,0);
      Ddi_BddarrayWrite(m->one,i,one_i);
      zero_i = Ddi_BddPartRead(tmp,1);
      Ddi_BddarrayWrite(m->zero,i,zero_i);
      dc_i = Ddi_BddPartRead(tmp,2);
      Ddi_BddarrayWrite(m->dc,i,dc_i);
    }
    Ddi_Free(tmp);

  }

  /* Ddi_MgrAutodynResume(ddm); */

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    printf("Meta-Simplify (init: %d)\nFinal size: %d\n",
      init,Ddi_BddSize(f));
  }

  MetaSimplify (f,init+1,end);

}
/**Function********************************************************************
  Synopsis    [Apply top-down reduction of ones to bottom layer]
  Description [Apply top-down reduction of ones to bottom layer. This reduces
    a Meta BDD to a conjunctive form]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static void
MetaSetConj (
  Ddi_Bdd_t *f
)
{
  int i, j, ng;
  Ddi_Bdd_t *zero, *zero_j, *one, *dc, *dc_j, *dcnew, *dccof, *zeroConst;
  Ddi_Mgr_t *ddm; 
  Ddi_Meta_t *m;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  Pdtutil_Assert(f->common.code == Ddi_Bdd_Meta_c,
    "Operation requires Meta BDD!");

  m = f->data.meta;
  ddm = Ddi_ReadMgr(f);
  ng = Ddi_BddarrayNum(m->one);
  zeroConst = Ddi_MgrReadZero(ddm);

  Pdtutil_Assert(ng == ddm->meta.groupNum,
    "Wrong number of groups in Meta BDD");

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    printf("Meta-Set-Conj\nInitial size: %d\n",Ddi_BddSize(f));
  }

  for (i=0; i<ng-1; i++) {

    zero = Ddi_BddarrayRead(m->zero,i);
    dc = Ddi_BddarrayRead(m->dc,i);

    /* one is pushed down. So new dc is !zero */
    dcnew = Ddi_BddNot(zero);
    dccof = Ddi_BddConstrain(dc,dcnew);

    Ddi_BddarrayWrite(m->one,i,zeroConst);
    Ddi_BddarrayWrite(m->dc,i,dcnew);

    for (j=i+1; j<ng; j++) {
      zero_j = Ddi_BddConstrain(Ddi_BddarrayRead(m->zero,j),dcnew);
      Ddi_BddAndAcc(zero_j,dccof);
      dc_j = Ddi_BddConstrain(Ddi_BddarrayRead(m->dc,j),dcnew);
      Ddi_BddAndAcc(dc_j,dccof);
      Ddi_BddarrayWrite(m->zero,j,zero_j);
      Ddi_BddarrayWrite(m->dc,j,dc_j);
      Ddi_Free(zero_j);
      Ddi_Free(dc_j);
    }

    Ddi_Free(dcnew);
    Ddi_Free(dccof);

  }
  zero = Ddi_BddarrayRead(m->zero,ng-1);
  one = Ddi_BddNot(zero);
  Ddi_BddarrayWrite(m->dc,ng-1,zeroConst);
  Ddi_BddarrayWrite(m->one,ng-1,one);
  Ddi_Free(one);

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelDevMin_c) {
    printf("Final size: %d\n",Ddi_BddSize(f));
  }

}



/**Function********************************************************************
  Synopsis    [Transform a Meta BDD to monolitic BDD]
  Description [Transform a Meta BDD to monolitic BDD]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static void
MetaToMono (
  Ddi_Bdd_t *f
)
{
  int ng, i;
  Ddi_Meta_t *m;
  Ddi_Mgr_t *ddm;
  Ddi_Bdd_t *one, *zero, *dc, *fBdd;

  DdiConsistencyCheck(f,Ddi_Bdd_c);
  Pdtutil_Assert(f->common.code == Ddi_Bdd_Meta_c,
    "Operation requires MEta BDD!");

  m = f->data.meta;
  ddm = Ddi_ReadMgr(f);
  ng = Ddi_BddarrayNum(m->one);

  Pdtutil_Assert(ng == ddm->meta.groupNum,
    "Wrong number of groups in Meta BDD");

  one = Ddi_BddarrayRead(m->one,ng-1);
  fBdd = Ddi_BddDup(one);

  for (i=ng-2; i>=0; i--) {

    one = Ddi_BddarrayRead(m->one,i);
    zero = Ddi_BddarrayRead(m->zero,i);
    dc = Ddi_BddarrayRead(m->dc,i);

    fBdd = Ddi_BddEvalFree(Ddi_BddIte(dc,fBdd,one),fBdd);

  }

  DdiMetaFree(f->data.meta);
  f->data.bdd = fBdd->data.bdd;
  Cudd_Ref (f->data.bdd);
  f->common.code = Ddi_Bdd_Mono_c;
  Ddi_Free(fBdd);

}




/**Function********************************************************************
  Synopsis    [Update Meta handling by calling Init]
  Description [Update Meta handling by calling Init. This is activated if 
    new variables have been created or reordering has taken place from 
    previous. Return true (non 0) if activated]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static int
MetaUpdate (
  Ddi_Mgr_t *ddm
)
{
  int i, nvar, doInit;

  doInit = 0;
  if (ddm->settings.meta.method != Ddi_Meta_Size)
    return(doInit);

  nvar = Ddi_ReadNumVars(ddm);
  if (nvar > ddm->meta.nvar) {
    doInit = 1;
  }
  else {
    for (i=0; i<nvar; i++) {
      if (ddm->meta.ord[i] != Ddi_ReadInvPerm(ddm,i)) {
	doInit = 1;
        break;
      }
    }
  }

  if (doInit) {
    Ddi_MetaInit(ddm,Ddi_Meta_Size,NULL,NULL,
      ddm->settings.meta.groupSizeMin);
  }

  return(doInit);

}



/**Function********************************************************************
  Synopsis    [Operate And-Exist between Meta BDD and conj. part. BDD]
  Description [Operate And-Exist between Meta BDD and conj. part. BDD]
  SideEffects []
  SeeAlso     []
******************************************************************************/
static Ddi_Bdd_t *
MetaLinearAndExistAcc (
  Ddi_Bdd_t *fMeta,
  Ddi_Bdd_t *gBdd,
  Ddi_Varset_t *smooth
)
{
  int i, ng, peak=0, size;
  Ddi_Bdd_t *dc, *gg, *ff, *tmp;
  Ddi_Varset_t *metaGrp, *downTot, *smooth_down, *aux, *range;
  Ddi_Varsetarray_t *downGroups;
  Ddi_Mgr_t *ddm; 
  Ddi_Meta_t *m;

  Pdtutil_Assert(fMeta->common.code == Ddi_Bdd_Meta_c,
    "Operation requires Meta BDD!");
  Pdtutil_Assert(gBdd->common.code == Ddi_Bdd_Part_Conj_c,
    "Operation requires cpart BDD!");

  ddm = Ddi_ReadMgr(fMeta);

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "$LAE-M:");
    fflush(stdout);
  }

  if (smooth == NULL) {
    smooth = Ddi_VarsetVoid(ddm);
  }
  else {
    smooth = Ddi_VarsetDup(smooth);
  }

  MetaSetConj(fMeta);
  ff = Ddi_BddMakeFromMeta(fMeta);
  ng = ddm->meta.groupNum;
  m = fMeta->data.meta;

  range = Ddi_BddSupp(fMeta);
  aux = Ddi_BddSupp(gBdd);
  Ddi_VarsetUnionAcc(range,aux);
  Ddi_Free(aux);
  Ddi_VarsetDiffAcc(range,smooth);

  downGroups = Ddi_VarsetarrayAlloc(ddm,ng);
  downTot = Ddi_VarsetVoid(ddm);
  gg = Ddi_BddDup(gBdd);

#if 0
  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "\nRepartitioning [%d]->[", Ddi_BddSize(gBdd));
    fflush(stdout);
  }
#endif
  for (i=ng-1; i>=0; i--) {
    Ddi_VarsetarrayWrite(downGroups,i,downTot);
    metaGrp = Ddi_VarsetarrayRead(ddm->meta.groups,i);
    Ddi_VarsetUnionAcc(downTot,metaGrp);
#if 0
    smooth_i = Ddi_VarsetDiff(range,metaGrp);
    /*    p = Ddi_BddExist(gBdd,smooth_i); */
    Ddi_VarsetPrint(smooth_i, 0, NULL, stdout);
    p = Part_BddMultiwayLinearAndExist(gBdd,smooth_i,100);
    Ddi_BddPartInsert(gg,0,p);
    if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
      if (i>0) {
        fprintf (stdout, "[%d,", Ddi_BddSize(p));
      }
      else {
        fprintf (stdout, "%d]=[(%d):%d]", Ddi_BddSize(p), 
          Ddi_BddPartNum(gg), Ddi_BddSize(gg));
      }
      fflush(stdout);
    }
    Ddi_Free(p);
    Ddi_Free(smooth_i);
#endif
  }
  Ddi_Free(downTot);

  Ddi_BddPartInsert(gg,0,ff);
  Ddi_Free(ff);

  for (i=0; i<ng; i++) {

    if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
      fprintf (stdout, "\nMLAE[%d] ", i);
      fflush(stdout);
    }

    if (MetaUpdate(ddm)) {
      if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
        fprintf (stdout, "(%d)", peak);
        fflush(stdout);
      }
      Ddi_Free(downGroups);
      DdiMetaAndExistAcc (fMeta,NULL,NULL);
      DdiMetaAndExistAcc (fMeta,gg,smooth);
      Ddi_Free(gg);
      Ddi_Free(smooth);
      return(fMeta);
    }

    smooth_down = Ddi_VarsetarrayRead(downGroups,i);
    smooth_down = Ddi_VarsetUnion(smooth_down,smooth);

    dc = Ddi_BddExist(gg,smooth_down);

    if (i<ng-1) {
 
#if 1
      tmp = Ddi_BddConstrain(gg,dc);
      if (Ddi_BddSize(tmp) < Ddi_BddSize(gg)) {
        Ddi_Free(gg);
        gg = tmp;
        printf("|-|"); fflush(stdout);
      }
      else {
        Ddi_Free(tmp);
        Ddi_BddPartInsert(gg,0,dc);
        Ddi_Free(dc);
        dc = Ddi_BddMakeConst(ddm,1);
        printf("|+|"); fflush(stdout);
      }
#endif
      Ddi_BddarrayWrite(m->dc,i,dc);
    }
    else {
      /*Ddi_VarsetPrint(smooth_down, 0, NULL, stdout);*/
      Ddi_BddarrayWrite(m->one,i,dc);
    }

    if ((size = Ddi_BddSize(gg))>peak) {
      peak = size;
    }

    Ddi_BddarrayWrite(m->zero,i,Ddi_BddNotAcc(dc));

    Ddi_Free(dc);
    Ddi_Free(smooth_down);
  }

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "(%d)%d->", peak, Ddi_BddSize(fMeta));
    fflush(stdout);
  }

  MetaReduce(fMeta,-1);

#if 1
  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "%d|", Ddi_BddSize(fMeta));
    fflush(stdout);
  }

  MetaSimplify(fMeta,0,ng-1);
#endif

  if (Ddi_MgrReadVerbosity(ddm) >= Pdtutil_VerbLevelAppMax_c) {
    fprintf (stdout, "%d$", Ddi_BddSize(fMeta));
    fflush(stdout);
  }

  Ddi_Free(range);
  Ddi_Free(ff);
  Ddi_Free(gg);
  Ddi_Free(smooth);
  Ddi_Free(downGroups);

  return(fMeta);

}


/**Function********************************************************************
  Synopsis    [Optimized constrain cofactor.]
  Description [return constrain only if size reduced]
  SideEffects [none]
  SeeAlso     []
******************************************************************************/
static Ddi_Bdd_t *
MetaConstrainOpt (
  Ddi_Bdd_t  *f,
  Ddi_Bdd_t  *g
)
{
  Ddi_Bdd_t *r, *p, *pc;
  int i, siftThresh, abortThresh;
  Ddi_Mgr_t *ddm = Ddi_ReadMgr(f);

  siftThresh = Ddi_MgrReadDynordThresh(ddm);
  abortThresh = Ddi_ReadLiveNodes(ddm)+2*Ddi_BddSize(f);
  Ddi_MgrSetDynordThresh(ddm,abortThresh);

  Ddi_MgrAbortOnSiftEnable(ddm);
  if (Ddi_BddIsPartConj(f)) {
    r = Ddi_BddDup(f);
    for (i=0; i<Ddi_BddPartNum(f); i++) {
      p = Ddi_BddPartRead(r,i);
      pc = Ddi_BddConstrain(p,g);
      if (Ddi_BddSize(pc)<Ddi_BddSize(p)) {
        Ddi_BddPartWrite(r,i,pc);
      }
      Ddi_Free(pc);
    }
  }
  else {
    r = Ddi_BddConstrain(f,g);
    if (Ddi_BddSize(r)>Ddi_BddSize(r)) {
      Ddi_Free(r);
      r = Ddi_BddDup(f);
    }
  }

  Ddi_MgrSetDynordThresh(ddm,siftThresh);

  Ddi_MgrAbortOnSiftDisable(ddm);

  return (r);
}
