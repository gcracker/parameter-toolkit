/**CFile***********************************************************************

  FileName    [cuplusAndAbs.c]

  PackageName [cuplus]

  Synopsis    [Optimized AndAbstract.]

  Description []

  SeeAlso     []

  Author      [Gianpiero Cabodi]

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

#include "cuplusInt.h"

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


/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Takes the AND of two BDDs and simultaneously abstracts the
  variables in cube.]

  Description [Takes the AND of two BDDs and simultaneously abstracts
  the variables in cube. The variables are existentially abstracted.
  Returns a pointer to the result is successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_bddAndAbstract]

******************************************************************************/
DdNode *
Cuplus_bddAndAbstract(
  DdManager * manager,
  DdNode * f,
  DdNode * g,
  DdNode * cube)
{
    DdNode *res;

    do {
	manager->reordered = 0;
	res = cuplusBddAndAbstractRecur(manager, f, g, cube);
    } while (manager->reordered == 1);
    return(res);

} /* end of Cuplus_bddAndAbstract */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Takes the AND of two BDDs and simultaneously abstracts the
  variables in cube.]

  Description [Takes the AND of two BDDs and simultaneously abstracts
  the variables in cube. The variables are existentially abstracted.
  Returns a pointer to the result is successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_bddAndAbstract]

******************************************************************************/
DdNode *
cuplusBddAndAbstractRecur(
  DdManager * manager,
  DdNode * f,
  DdNode * g,
  DdNode * cube)
{
    DdNode *F, *ft, *fe, *G, *gt, *ge;
    DdNode *one, *zero, *r, *t, *e;
    unsigned int topf, topg, topcube, top, index;

    statLine(manager);
    one = DD_ONE(manager);
    zero = Cudd_Not(one);

    /* Terminal cases. */
    if (f == zero || g == zero || f == Cudd_Not(g)) return(zero);
    if (f == one && g == one)	return(one);

    if (cube == one) {
	return(cuddBddAndRecur(manager, f, g));
    }
    if (f == one || f == g) {
	return(cuddBddExistAbstractRecur(manager, g, cube));
    }
    if (g == one) {
	return(cuddBddExistAbstractRecur(manager, f, cube));
    }
    /* At this point f, g, and cube are not constant. */

    if (f > g) { /* Try to increase cache efficiency. */
	DdNode *tmp = f;
	f = g;
	g = tmp;
    }

    /* Here we can skip the use of cuddI, because the operands are known
    ** to be non-constant.
    */
    F = Cudd_Regular(f);
    G = Cudd_Regular(g);
    topf = manager->perm[F->index];
    topg = manager->perm[G->index];
    top = ddMin(topf, topg);
    topcube = manager->perm[cube->index];

    while (topcube < top) {
	cube = cuddT(cube);
	if (cube == one) {
	    return(cuddBddAndRecur(manager, f, g));
	}
	topcube = manager->perm[cube->index];
    }
    /* Now, topcube >= top. */

    /* Check cache. */
    if (F->ref != 1 || G->ref != 1) {
	r = cuddCacheLookup(manager, DD_BDD_AND_ABSTRACT_TAG, f, g, cube);
	if (r != NULL) {
	    return(r);
	}
    }

    /* GP PATCH */

#if QUANTIFY_BEFORE_RECUR

    if (topcube == top) {
      if ((topf != top)&&(topg == top)) {
	DdNode *tmp = f;
	f = g;
	g = tmp;
        F = Cudd_Regular(f);
        G = Cudd_Regular(g);
        topf = manager->perm[F->index];
        topg = manager->perm[G->index];
      }
      if ((topf == top)&&(topg != top)) {
        /* quantify top var from f */
	DdNode *fabs, *Cube;
	ft = cuddT(F);
	fe = cuddE(F);
	if (Cudd_IsComplement(f)) {
	    ft = Cudd_Not(ft);
	    fe = Cudd_Not(fe);
	}
        fabs = cuddBddAndRecur(manager, Cudd_Not(ft), Cudd_Not(fe));
	if (fabs == NULL) {
 	  return(NULL);
	}
	fabs = Cudd_Not(fabs);
	cuddRef(fabs);

        /* recur */
	Cube = cuddT(cube);
	r = cuddBddAndAbstractRecur(manager, fabs, g, Cube);
	if (r == NULL) return(NULL);
	cuddRef(r);

	Cudd_DelayedDerefBdd(manager, fabs);
	cuddDeref(r);

        if (F->ref != 1 || G->ref != 1)
   	  cuddCacheInsert(manager, DD_BDD_AND_ABSTRACT_TAG, f, g, cube, r);
        return (r);
      }
    }
#endif
    /* end GP PATCH */

    if (topf == top) {
	index = F->index;
	ft = cuddT(F);
	fe = cuddE(F);
	if (Cudd_IsComplement(f)) {
	    ft = Cudd_Not(ft);
	    fe = Cudd_Not(fe);
	}
    } else {
	index = G->index;
	ft = fe = f;
    }

    if (topg == top) {
	gt = cuddT(G);
	ge = cuddE(G);
	if (Cudd_IsComplement(g)) {
	    gt = Cudd_Not(gt);
	    ge = Cudd_Not(ge);
	}
    } else {
	gt = ge = g;
    }

    if (topcube == top) {	/* quantify */
	DdNode *Cube = cuddT(cube);
	t = cuddBddAndAbstractRecur(manager, ft, gt, Cube);
	if (t == NULL) return(NULL);
	/* Special case: 1 OR anything = 1. Hence, no need to compute
	** the else branch if t is 1. Likewise t + t * anything == t.
	** Notice that t == fe implies that fe does not depend on the
	** variables in Cube. Likewise for t == ge.
	*/
	if (t == one || t == fe || t == ge) {
	    if (F->ref != 1 || G->ref != 1)
		cuddCacheInsert(manager, DD_BDD_AND_ABSTRACT_TAG,
				f, g, cube, t);
	    return(t);
	}
	cuddRef(t);
	/* Special case: t + !t * anything == t + anything. */
	if (t == Cudd_Not(fe)) {
	    e = cuddBddExistAbstractRecur(manager, ge, Cube);
	} else if (t == Cudd_Not(ge)) {
	    e = cuddBddExistAbstractRecur(manager, fe, Cube);
	} else {
	    e = cuddBddAndAbstractRecur(manager, fe, ge, Cube);
	}
	if (e == NULL) {
	    Cudd_IterDerefBdd(manager, t);
	    return(NULL);
	}
	if (t == e) {
	    r = t;
	    cuddDeref(t);
	} else {
	    cuddRef(e);
	    r = cuddBddAndRecur(manager, Cudd_Not(t), Cudd_Not(e));
	    if (r == NULL) {
		Cudd_IterDerefBdd(manager, t);
		Cudd_IterDerefBdd(manager, e);
		return(NULL);
	    }
	    r = Cudd_Not(r);
	    cuddRef(r);
	    Cudd_DelayedDerefBdd(manager, t);
	    Cudd_DelayedDerefBdd(manager, e);
	    cuddDeref(r);
	}
    } else {
	t = cuddBddAndAbstractRecur(manager, ft, gt, cube);
	if (t == NULL) return(NULL);
	cuddRef(t);
	e = cuddBddAndAbstractRecur(manager, fe, ge, cube);
	if (e == NULL) {
	    Cudd_IterDerefBdd(manager, t);
	    return(NULL);
	}
	if (t == e) {
	    r = t;
	    cuddDeref(t);
	} else {
	    cuddRef(e);
	    if (Cudd_IsComplement(t)) {
		r = cuddUniqueInter(manager, (int) index,
				    Cudd_Not(t), Cudd_Not(e));
		if (r == NULL) {
		    Cudd_IterDerefBdd(manager, t);
		    Cudd_IterDerefBdd(manager, e);
		    return(NULL);
		}
		r = Cudd_Not(r);
	    } else {
		r = cuddUniqueInter(manager,(int)index,t,e);
		if (r == NULL) {
		    Cudd_IterDerefBdd(manager, t);
		    Cudd_IterDerefBdd(manager, e);
		    return(NULL);
		}
	    }
	    cuddDeref(e);
	    cuddDeref(t);
	}
    }

    if (F->ref != 1 || G->ref != 1)
	cuddCacheInsert(manager, DD_BDD_AND_ABSTRACT_TAG, f, g, cube, r);
    return (r);

} /* end of cuddBddAndAbstractRecur */

/**Function********************************************************************

  Synopsis [simplifies first factor of AndAbstract.]
  Description [simplifies first factor of AndAbstract.]

  SideEffects [None]

  SeeAlso     [Cuplus_bddAndAbstract]

******************************************************************************/
DdNode *
cuplusBddAndAbstractSimplify(
  DdManager * manager,
  DdNode * f,
  DdNode * g,
  DdNode * cube)
{
    DdNode *F, *ft, *fe, *G, *gt, *ge;
    DdNode *one, *zero, *r, *t, *e;
    unsigned int topf, topg, topcube, top, index;

    statLine(manager);
    one = DD_ONE(manager);
    zero = Cudd_Not(one);

    /* Terminal cases. */
    if (f == zero || f == one || f == Cudd_Not(g)) || f == g) return(f);
    if (g == zero) return(one);
    if (cube == one) {
	return(f);
    }
    if (g == one) {
	return(cuddBddExistAbstractRecur(manager, f, cube));
    }
@@@
    /* At this point f, g, and cube are not constant. */

    if (f > g) { /* Try to increase cache efficiency. */
	DdNode *tmp = f;
	f = g;
	g = tmp;
    }

    /* Here we can skip the use of cuddI, because the operands are known
    ** to be non-constant.
    */
    F = Cudd_Regular(f);
    G = Cudd_Regular(g);
    topf = manager->perm[F->index];
    topg = manager->perm[G->index];
    top = ddMin(topf, topg);
    topcube = manager->perm[cube->index];

    while (topcube < top) {
	cube = cuddT(cube);
	if (cube == one) {
	    return(cuddBddAndRecur(manager, f, g));
	}
	topcube = manager->perm[cube->index];
    }
    /* Now, topcube >= top. */

    /* Check cache. */
    if (F->ref != 1 || G->ref != 1) {
	r = cuddCacheLookup(manager, DD_BDD_AND_ABSTRACT_TAG, f, g, cube);
	if (r != NULL) {
	    return(r);
	}
    }

    /* GP PATCH */

#if QUANTIFY_BEFORE_RECUR

    if (topcube == top) {
      if ((topf != top)&&(topg == top)) {
	DdNode *tmp = f;
	f = g;
	g = tmp;
        F = Cudd_Regular(f);
        G = Cudd_Regular(g);
        topf = manager->perm[F->index];
        topg = manager->perm[G->index];
      }
      if ((topf == top)&&(topg != top)) {
        /* quantify top var from f */
	DdNode *fabs, *Cube;
	ft = cuddT(F);
	fe = cuddE(F);
	if (Cudd_IsComplement(f)) {
	    ft = Cudd_Not(ft);
	    fe = Cudd_Not(fe);
	}
        fabs = cuddBddAndRecur(manager, Cudd_Not(ft), Cudd_Not(fe));
	if (fabs == NULL) {
 	  return(NULL);
	}
	fabs = Cudd_Not(fabs);
	cuddRef(fabs);

        /* recur */
	Cube = cuddT(cube);
	r = cuddBddAndAbstractRecur(manager, fabs, g, Cube);
	if (r == NULL) return(NULL);
	cuddRef(r);

	Cudd_DelayedDerefBdd(manager, fabs);
	cuddDeref(r);

        if (F->ref != 1 || G->ref != 1)
   	  cuddCacheInsert(manager, DD_BDD_AND_ABSTRACT_TAG, f, g, cube, r);
        return (r);
      }
    }
#endif
    /* end GP PATCH */

    if (topf == top) {
	index = F->index;
	ft = cuddT(F);
	fe = cuddE(F);
	if (Cudd_IsComplement(f)) {
	    ft = Cudd_Not(ft);
	    fe = Cudd_Not(fe);
	}
    } else {
	index = G->index;
	ft = fe = f;
    }

    if (topg == top) {
	gt = cuddT(G);
	ge = cuddE(G);
	if (Cudd_IsComplement(g)) {
	    gt = Cudd_Not(gt);
	    ge = Cudd_Not(ge);
	}
    } else {
	gt = ge = g;
    }

    if (topcube == top) {	/* quantify */
	DdNode *Cube = cuddT(cube);
	t = cuddBddAndAbstractRecur(manager, ft, gt, Cube);
	if (t == NULL) return(NULL);
	/* Special case: 1 OR anything = 1. Hence, no need to compute
	** the else branch if t is 1. Likewise t + t * anything == t.
	** Notice that t == fe implies that fe does not depend on the
	** variables in Cube. Likewise for t == ge.
	*/
	if (t == one || t == fe || t == ge) {
	    if (F->ref != 1 || G->ref != 1)
		cuddCacheInsert(manager, DD_BDD_AND_ABSTRACT_TAG,
				f, g, cube, t);
	    return(t);
	}
	cuddRef(t);
	/* Special case: t + !t * anything == t + anything. */
	if (t == Cudd_Not(fe)) {
	    e = cuddBddExistAbstractRecur(manager, ge, Cube);
	} else if (t == Cudd_Not(ge)) {
	    e = cuddBddExistAbstractRecur(manager, fe, Cube);
	} else {
	    e = cuddBddAndAbstractRecur(manager, fe, ge, Cube);
	}
	if (e == NULL) {
	    Cudd_IterDerefBdd(manager, t);
	    return(NULL);
	}
	if (t == e) {
	    r = t;
	    cuddDeref(t);
	} else {
	    cuddRef(e);
	    r = cuddBddAndRecur(manager, Cudd_Not(t), Cudd_Not(e));
	    if (r == NULL) {
		Cudd_IterDerefBdd(manager, t);
		Cudd_IterDerefBdd(manager, e);
		return(NULL);
	    }
	    r = Cudd_Not(r);
	    cuddRef(r);
	    Cudd_DelayedDerefBdd(manager, t);
	    Cudd_DelayedDerefBdd(manager, e);
	    cuddDeref(r);
	}
    } else {
	t = cuddBddAndAbstractRecur(manager, ft, gt, cube);
	if (t == NULL) return(NULL);
	cuddRef(t);
	e = cuddBddAndAbstractRecur(manager, fe, ge, cube);
	if (e == NULL) {
	    Cudd_IterDerefBdd(manager, t);
	    return(NULL);
	}
	if (t == e) {
	    r = t;
	    cuddDeref(t);
	} else {
	    cuddRef(e);
	    if (Cudd_IsComplement(t)) {
		r = cuddUniqueInter(manager, (int) index,
				    Cudd_Not(t), Cudd_Not(e));
		if (r == NULL) {
		    Cudd_IterDerefBdd(manager, t);
		    Cudd_IterDerefBdd(manager, e);
		    return(NULL);
		}
		r = Cudd_Not(r);
	    } else {
		r = cuddUniqueInter(manager,(int)index,t,e);
		if (r == NULL) {
		    Cudd_IterDerefBdd(manager, t);
		    Cudd_IterDerefBdd(manager, e);
		    return(NULL);
		}
	    }
	    cuddDeref(e);
	    cuddDeref(t);
	}
    }

    if (F->ref != 1 || G->ref != 1)
	cuddCacheInsert(manager, DD_BDD_AND_ABSTRACT_TAG, f, g, cube, r);
    return (r);

} /* end of cuddBddAndAbstractRecur */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

