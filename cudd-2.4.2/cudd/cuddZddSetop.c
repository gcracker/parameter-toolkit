/**CFile***********************************************************************

   FileName    [cuddZddSetop.c]

   PackageName [cudd]

   Synopsis    [Set operations on ZDDs.]

   Description [External procedures included in this module:
   <ul>
   <li> Cudd_zddIte()
   <li> Cudd_zddUnion()
   <li> Cudd_zddIntersect()
   <li> Cudd_zddDiff()
   <li> Cudd_zddDiffConst()
   <li> Cudd_zddSubset1()
   <li> Cudd_zddSubset0()
   <li> Cudd_zddChange()
   </ul>
   Internal procedures included in this module:
   <ul>
   <li> cuddZddIte()
   <li> cuddZddUnion()
   <li> cuddZddIntersect()
   <li> cuddZddDiff()
   <li> cuddZddChangeAux()
   <li> cuddZddSubset1()
   <li> cuddZddSubset0()
   </ul>
   Static procedures included in this module:
   <ul>
   <li> zdd_subset1_aux()
   <li> zdd_subset0_aux()
   <li> zddVarToConst()
   </ul>
   ]

   SeeAlso     []

   Author      [Hyong-Kyoon Shin, In-Ho Moon]

   Copyright   [Copyright (c) 1995-2004, Regents of the University of Colorado

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   Neither the name of the University of Colorado nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.]

******************************************************************************/

#include "util.h"
#include "cuddInt.h"

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

#ifndef lint
static char rcsid[] DD_UNUSED = "$Id: cuddZddSetop.c,v 1.25 2004/08/13 18:04:54 fabio Exp $";
#endif

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

    /**AutomaticStart*************************************************************/

    /*---------------------------------------------------------------------------*/
    /* Static function prototypes                                                */
    /*---------------------------------------------------------------------------*/

    static DdNode * zdd_subset1_aux (DdManager *zdd, DdNode *P, DdNode *zvar);
    static DdNode * zdd_subset0_aux (DdManager *zdd, DdNode *P, DdNode *zvar);
    static void zddVarToConst (DdNode *f, DdNode **gp, DdNode **hp, DdNode *base, DdNode *empty);
    static DdNode * mg_extraZddPermuteRecur  (DdManager * dd, DdHashTable * table, DdNode * N, 
					      int * permut);
    static void     cuddHashTableQuitZdd2(DdHashTable * hash);

    /**AutomaticEnd***************************************************************/

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis [Computes the ITE of three ZDDs.]

   Description [Computes the ITE of three ZDDs. Returns a pointer to the
   result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
Cudd_zddIte(
            DdManager * dd,
            DdNode * f,
            DdNode * g,
            DdNode * h)
{
    DdNode *res;

    do {
        dd->reordered = 0;
        res = cuddZddIte(dd, f, g, h);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddIte */


/**Function********************************************************************

   Synopsis [Computes the ITE of three ZDDs.]

   Description [Custom version by Manish and Graham.Computes the ITE of three 
   ZDDs. Returns a pointer to the result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
mg_Cudd_zddIte(
               DdManager * dd,
               DdNode * f,
               DdNode * g,
               DdNode * h)
{
    DdNode *res;

    do {
        dd->reordered = 0;
        res = cuddZddIte(dd, f, g, h);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddIte */

/**Function********************************************************************

   Synopsis [Computes the ITE of three ZDDs.]

   Description [Custom version by Manish and Graham.Computes the ITE of three 
   ZDDs. Returns a pointer to the result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
int
mg_Cudd_zddGetRefCount(
		       DdNode * n)
{
    DdNode *res;

    return((int)(n->ref));

} /* end of Cudd_zddGetRefCount */



/**Function********************************************************************

  Synopsis [Existentially abstracts all the variables in cube from f.]

  Description [Existentially abstracts all the variables in cube from f.
  Returns the abstracted ZDD if successful; NULL otherwise. 
  Copied from the Extra20 dd package.]

  SideEffects [None]

  SeeAlso     [Cudd_bddExistAbstract Cudd_bddUnivAbstract Cudd_addExistAbstract]

******************************************************************************/
DdNode *
mg_Extra_zddExistAbstract(
  DdManager * dd,
  DdNode * f,
  DdNode * cube)
{
    DdNode *res;

    /* // Debug */
    /* if(Cudd_CheckKeys(dd) != 0) */
    /*   { */
    /* 	(void) fprintf(dd->err, "ExistAbstract received messed up keys\n"); */
    /*   } */
    /* else */
    /*   { */
    /* 	(void) fprintf(dd->err, "ExistAbstract received Good keys\n"); */
    /*   } */


    /* if (zddCheckPositiveCube(dd, cube) == 0)  */
    /* { */
    /*     (void) fprintf(dd->err, "Error: Extra_zddExistAbstract() can only abstract positive cubes\n"); */
    /*     dd->errorCode = CUDD_INVALID_ARG; */
    /*     return(NULL); */
    /* } */

    do {
    dd->reordered = 0;
    res = mg_extraZddExistAbstractRecur(dd, f, cube);
    } while (dd->reordered == 1);


    /* // Debug */
    /* cuddRef(res); */
    /* if(Cudd_CheckKeys(dd) != 0) */
    /*   { */
    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys\n"); */
    /*   } */
    /* else */
    /*   { */
    /* 	(void) fprintf(dd->err, "ExistAbstract returned good keys\n"); */
    /*   } */
    /* cuddDeref(res); */
    
    return(res);

} /* end of mg_Extra_zddExistAbstract */

/**Function********************************************************************

  Synopsis    [Permutes the variables of a ZDD.]

  Description [Given a permutation in array permut, creates a new ZDD
  with permuted variables. There should be an entry in array permut
  for each variable in the manager. The i-th entry of permut holds the
  index of the variable that is to substitute the i-th variable.
  Returns a pointer to the resulting ZDD if successful; NULL
  otherwise. Copied from the Extra20 dd package.]

  SideEffects [None]

  SeeAlso     [Cudd_addPermute Cudd_bddPermute Cudd_bddSwapVariables]

******************************************************************************/
DdNode *
mg_Extra_zddPermute(
  DdManager * dd,
  DdNode * node,
  int * permut)
{
    DdHashTable     *table;
    DdNode          *res;

    do {

    dd->reordered = 0;
    table = cuddHashTableInit(dd,1,2);
    if (table == NULL) return(NULL);
    res = mg_extraZddPermuteRecur(dd,table,node,permut);
    if (res != NULL) cuddRef(res);
    /* Dispose of local cache. */
    cuddHashTableQuitZdd2(table); 

    } while (dd->reordered == 1);

    if (res != NULL) cuddDeref(res);
    return(res);

} /* end of mg_Extra_zddPermute */



/**Function********************************************************************

   Synopsis [Computes the union of two ZDDs.]

   Description [Computes the union of two ZDDs. Returns a pointer to the
   result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
Cudd_zddUnion(
              DdManager * dd,
              DdNode * P,
              DdNode * Q)
{
    DdNode *res;

    do {
        dd->reordered = 0;
        res = cuddZddUnion(dd, P, Q);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddUnion */


/**Function********************************************************************

   Synopsis [Computes the intersection of two ZDDs.]

   Description [Computes the intersection of two ZDDs. Returns a pointer to
   the result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
Cudd_zddIntersect(
                  DdManager * dd,
                  DdNode * P,
                  DdNode * Q)
{
    DdNode *res;

    do {
        dd->reordered = 0;
        res = cuddZddIntersect(dd, P, Q);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddIntersect */


/**Function********************************************************************

   Synopsis [Computes the difference of two ZDDs.]

   Description [Computes the difference of two ZDDs. Returns a pointer to the
   result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     [Cudd_zddDiffConst]

******************************************************************************/
DdNode *
Cudd_zddDiff(
             DdManager * dd,
             DdNode * P,
             DdNode * Q)
{
    DdNode *res;

    do {
        dd->reordered = 0;
        res = cuddZddDiff(dd, P, Q);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddDiff */


/**Function********************************************************************

   Synopsis [Performs the inclusion test for ZDDs (P implies Q).]

   Description [Inclusion test for ZDDs (P implies Q). No new nodes are
   generated by this procedure. Returns empty if true;
   a valid pointer different from empty or DD_NON_CONSTANT otherwise.]

   SideEffects [None]

   SeeAlso     [Cudd_zddDiff]

******************************************************************************/
DdNode *
Cudd_zddDiffConst(
                  DdManager * zdd,
                  DdNode * P,
                  DdNode * Q)
{
    int		p_top, q_top;
    DdNode	*empty = DD_ZERO(zdd), *t, *res;
    DdManager	*table = zdd;
    statLine(zdd);
    if (P == empty)
        return(empty);
    if (Q == empty)
        return(P);
    if (P == Q)
        return(empty);

    /* Check cache.  The cache is shared by cuddZddDiff(). */
    res = cuddCacheLookup2Zdd(table, cuddZddDiff, P, Q);
    if (res != NULL)
        return(res);

    if (cuddIsConstant(P))
        p_top = P->index;
    else
        p_top = zdd->permZ[P->index];
    if (cuddIsConstant(Q))
        q_top = Q->index;
    else
        q_top = zdd->permZ[Q->index];
    if (p_top < q_top) {
        res = DD_NON_CONSTANT;
    } else if (p_top > q_top) {
        res = Cudd_zddDiffConst(zdd, P, cuddE(Q));
    } else {
        t = Cudd_zddDiffConst(zdd, cuddT(P), cuddT(Q));
        if (t != empty)
            res = DD_NON_CONSTANT;
        else
            res = Cudd_zddDiffConst(zdd, cuddE(P), cuddE(Q));
    }

    cuddCacheInsert2(table, cuddZddDiff, P, Q, res);

    return(res);

} /* end of Cudd_zddDiffConst */


/**Function********************************************************************

   Synopsis [Computes the positive cofactor of a ZDD w.r.t. a variable.]

   Description [Computes the positive cofactor of a ZDD w.r.t. a
   variable. In terms of combinations, the result is the set of all
   combinations in which the variable is asserted. Returns a pointer to
   the result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     [Cudd_zddSubset0]

******************************************************************************/
DdNode *
Cudd_zddSubset1(
                DdManager * dd,
                DdNode * P,
                int  var)
{
    DdNode	*r;

    do {
        dd->reordered = 0;
        r = cuddZddSubset1(dd, P, var);
    } while (dd->reordered == 1);

    return(r);

} /* end of Cudd_zddSubset1 */


/**Function********************************************************************

   Synopsis [Computes the negative cofactor of a ZDD w.r.t. a variable.]

   Description [Computes the negative cofactor of a ZDD w.r.t. a
   variable. In terms of combinations, the result is the set of all
   combinations in which the variable is negated. Returns a pointer to
   the result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     [Cudd_zddSubset1]

******************************************************************************/
DdNode *
Cudd_zddSubset0(
                DdManager * dd,
                DdNode * P,
                int  var)
{
    DdNode	*r;

    do {
        dd->reordered = 0;
        r = cuddZddSubset0(dd, P, var);
    } while (dd->reordered == 1);

    return(r);

} /* end of Cudd_zddSubset0 */


/**Function********************************************************************

   Synopsis [Substitutes a variable with its complement in a ZDD.]

   Description [Substitutes a variable with its complement in a ZDD.
   returns a pointer to the result if successful; NULL otherwise.]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
Cudd_zddChange(
               DdManager * dd,
               DdNode * P,
               int  var)
{
    DdNode	*res;

    if ((unsigned int) var >= CUDD_MAXINDEX - 1) return(NULL);
    
    do {
        dd->reordered = 0;
        res = cuddZddChange(dd, P, var);
    } while (dd->reordered == 1);
    return(res);

} /* end of Cudd_zddChange */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Shuts down a hash table.]

  Description [Shuts down a hash table, dereferencing all the values.
  
  Basically the same as cuddHashTableQuit(), only uses Cudd_RecursiveDerefZdd
  instead of Cudd_RecursiveDeref. Pulled from the extra20 dd package.]

  SideEffects [None]

  SeeAlso     [cuddHashTableInit]

******************************************************************************/
void
cuddHashTableQuitZdd2(
  DdHashTable * hash)
{
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    unsigned int i;
    DdManager *dd = hash->manager;
    DdHashItem *bucket;
    DdHashItem **memlist, **nextmem;
    unsigned int numBuckets = hash->numBuckets;

    for (i = 0; i < numBuckets; i++) {
    bucket = hash->bucket[i];
    while (bucket != NULL) {
        Cudd_RecursiveDerefZdd(dd, bucket->value);
        bucket = bucket->next;
    }
    }

    memlist = hash->memoryList;
    while (memlist != NULL) {
    nextmem = (DdHashItem **) memlist[0];
    FREE(memlist);
    memlist = nextmem;
    }

    FREE(hash->bucket);
    FREE(hash);
#ifdef __osf__
#pragma pointer_size restore
#endif

    return;

} /* end of cuddHashTableQuitZdd2 */

/**Function********************************************************************

  Synopsis    [Implements the recursive step of Extra_zddPermute.]

  Description [ Recursively puts the ZDD in the order given in the array permut.
  Checks for trivial cases to terminate recursion, then splits on the
  children of this node.  Once the solutions for the children are
  obtained, it puts into the current position the node from the rest of
  the ZDD that should be here. Then returns this ZDD.
  The key here is that the node being visited is NOT put in its proper
  place by this instance, but rather is switched when its proper position
  is reached in the recursion tree.<p>
  The DdNode * that is returned is the same ZDD as passed in as node,
  but in the new order. Copied from the Extra20 DD package.]

  SideEffects [None]

  SeeAlso     [Extra_zddPermute cuddBddPermuteRecur]

******************************************************************************/
static DdNode *
mg_extraZddPermuteRecur(
  DdManager * dd /* DD manager */,
  DdHashTable * table /* computed table */,
  DdNode * N /* ZDD to be reordered */,
  int * permut /* permutation array */)
{
    DdNode  *T,*E, *res;

    /*  statLine(dd); */
    /* Check for terminal case of constant node. */
    if (cuddIsConstant(N)) 
        return(N);

    /* If problem already solved, look up answer and return. */
    if (N->ref != 1 && (res = cuddHashTableLookup1(table,N)) != NULL) 
        return (res);
 
    /* Split and recur on children of this node. */
    T = mg_extraZddPermuteRecur(dd,table,cuddT(N),permut);
    if (T == NULL) return(NULL);
    cuddRef(T);
    E = mg_extraZddPermuteRecur(dd,table,cuddE(N),permut);
    if (E == NULL) {
        Cudd_RecursiveDerefZdd(dd, T);
        return(NULL);
    }
    cuddRef(E);

    /* Move variable that should be in this position to this position
    ** by retrieving the single var ZDD for that variable, and calling
    ** cuddZddGetNodeIVO with the T and E we just created.
    */
    res = cuddZddGetNodeIVO(dd,permut[N->index],T,E);
    if (res == NULL) {
        Cudd_RecursiveDerefZdd(dd, T);
        Cudd_RecursiveDerefZdd(dd, E);
        return(NULL);
    }
    cuddRef(res);
    Cudd_RecursiveDerefZdd(dd, T);
    Cudd_RecursiveDerefZdd(dd, E);

    /* Do not keep the result if the reference count is only 1, since
    ** it will not be visited again.
    */
    if (N->ref != 1) {
        ptrint fanout = (ptrint) N->ref;
        cuddSatDec(fanout);
        if (!cuddHashTableInsert1(table,N,res,fanout)) {
            Cudd_RecursiveDerefZdd(dd, res);
            return(NULL);
        }
    }
    cuddDeref(res);
    return(res);

} /* end of mg_extraZddPermuteRecur */



/**Function********************************************************************

  Synopsis    [Performs the recursive steps of mg_Extra_zddExistAbstract.]

  Description [Performs the recursive steps of mg_Extra_zddExistAbstract.
  Returns the ZDD obtained by removing the variables of cube from f 
  if successful; NULL otherwise. Copied from the Extra20 dd package.]

  SideEffects [None]

  SeeAlso     [Cudd_bddExistAbstract Cudd_bddUnivAbstract]

******************************************************************************/
DdNode *
mg_extraZddExistAbstractRecur(
  DdManager * dd,
  DdNode * F,
  DdNode * cube)
{
    DdNode *zRes;

    /* statLine(dd); */
    /* Cube is guaranteed to be a cube at this point. */    
    if (F == dd->one || F == dd->zero || cube == dd->one )   
        return(F);    
    /* From now on, F and cube are non-constant. */

    /* Abstract a variable that does not appear in F. */
    while ( dd->permZ[F->index] > dd->permZ[cube->index] ) 
    {
        cube = cuddT(cube);
        if (cube == dd->one) 
            return(F);
    }

    /* Check the cache. */
    if ((F->ref != 1) && ((zRes = cuddCacheLookup2Zdd(dd, mg_extraZddExistAbstractRecur, F, cube)) != NULL)) 
      {
	return(zRes);
      }
    else
    {
        DdNode *zRes0, *zRes1;

        /* If the two indices are the same, so are their levels. */
        if (F->index == cube->index) 
        {
            /* solve the problem for the else branch */
            zRes0 = mg_extraZddExistAbstractRecur(dd, cuddE(F), cuddT(cube));

            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* solve the problem for the then branch */
            zRes1 = mg_extraZddExistAbstractRecur(dd, cuddT(F), cuddT(cube));
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

	    /* // Debug */
	    /* if(Cudd_CheckKeys(dd) != 0) */
	    /*   { */
	    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys before union zRes1\n"); */
	    /* 	return NULL; */
	    /*   } */

            /* find the result by computing the union of these two sets */
            zRes = cuddZddUnion( dd, zRes0, zRes1 );
	    cuddRef( zRes );
	    /* // Debug */
	    /* if(Cudd_CheckKeys(dd) != 0) */
	    /*   { */
	    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys after union\n"); */
	    /* 	return NULL; */
	    /*   } */

	    Cudd_RecursiveDerefZdd( dd, zRes0 ); 
	    Cudd_RecursiveDerefZdd( dd, zRes1 ); 
	    
	    /* // Debug */
	    /* if(Cudd_CheckKeys(dd) != 0) */
	    /*   { */
	    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys after deref after union\n"); */
	    /* 	return NULL; */
	    /*   } */
	    cuddDeref( zRes );

            /* if ( zRes == NULL ) */
            /* { */
            /*     Cudd_RecursiveDerefZdd( dd, zRes0 ); */
            /*     Cudd_RecursiveDerefZdd( dd, zRes1 ); */
            /*     return NULL; */
            /* } */
	    /* if(zRes == zRes0) */
	    /*   { */
	    /* 	cuddRef( zRes ); */
	    /* 	Cudd_RecursiveDerefZdd( dd, zRes0 ); */
	    /* 	cuddDeref( zRes ); */
	    /*   }  */
	    /* if(zRes == zRes1) */
	    /*   { */


	    /*   }  */
	    /* if((zRes != zRes1) && (zRes != zRes0)) */
	    /*   { */
	    /* 	cuddRef( zRes ); */
	    /* 	//	    cuddDeref( zRes0 ); */
	    /* 	//	    cuddDeref( zRes1 ); */
	    /* 	Cudd_RecursiveDerefZdd( dd, zRes0 ); */
	    /* 	Cudd_RecursiveDerefZdd( dd, zRes1 ); */
	    /* 	cuddDeref( zRes );		 */
	    /*   } */

	    /* // Debug */
	    /* Cudd_Ref(zRes); */
	    /* if(Cudd_CheckKeys(dd) != 0) */
	    /*   { */
	    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys in Union\n"); */
	    /*   } */
	    /* cuddDeref( zRes ); */
        }
        else /* if (cuddIZ(dd,F->index) < cuddIZ(dd,cube->index)) */
        {
            /* solve the problem for the else branch */
            zRes0 = mg_extraZddExistAbstractRecur(dd, cuddE(F), cube);
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* solve the problem for the then branch */
            zRes1 = mg_extraZddExistAbstractRecur(dd, cuddT(F), cube);
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            /* combine the solutions using the current variable */
            zRes = cuddZddGetNode( dd, F->index, zRes1, zRes0 );

	    /* cuddRef(zRes); */
	    /* // Debug */
	    /* if(Cudd_CheckKeys(dd) != 0) */
	    /*   { */
	    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys after getnode\n"); */
	    /* 	return NULL; */
	    /*   } */
	    /* cuddDeref(zRes); */

            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
	    //            cuddRef( zRes );
	    cuddDeref( zRes0 );
	    cuddDeref( zRes1 );

	    /* cuddRef(zRes); */
	    /* if(Cudd_CheckKeys(dd) != 0) */
	    /*   { */
	    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys after getnode after deref\n"); */
	    /* 	printf("f %p zRes %p zRes0 %p zRes1 %p fr %d zResR %d zRes0R %d zRes1R %d\n", */
	    /* 	       F, F->ref, zRes, zRes->ref, zRes0, zRes0->ref, zRes1, zRes1->ref); */
	    /* 	return NULL; */
	    /*   } */
	    /* cuddDeref(zRes); */

	    //	    Cudd_RecursiveDerefZdd( dd, zRes0 );
	    //	    Cudd_RecursiveDerefZdd( dd, zRes1 );
	    //            cuddDeref( zRes );

	    /* // Debug */
	    /* Cudd_Ref(zRes); */
	    /* if(Cudd_CheckKeys(dd) != 0) */
	    /*   { */
	    /* 	(void) fprintf(dd->err, "ExistAbstract messed up the keys at GetNode\n"); */
	    /*   } */
	    /* cuddDeref(zRes); */
        }

        /* insert the result into cache */
        if ((F->ref != 1))
	  {
	    cuddCacheInsert2(dd, mg_extraZddExistAbstractRecur, F, cube, zRes);
	  } 
        return zRes;
    }
} /* end of extraZddExistAbstractRecur */


/**Function********************************************************************

   Synopsis    [Performs the recursive step of Cudd_zddIte.]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
cuddZddIte(
           DdManager * dd,
           DdNode * f,
           DdNode * g,
           DdNode * h)
{
    DdNode *tautology, *empty;
    DdNode *r,*Gv,*Gvn,*Hv,*Hvn,*t,*e;
    unsigned int topf,topg,toph,v,top;
    int index;

    statLine(dd);
    /* Trivial cases. */
    /* One variable cases. */
    if (f == (empty = DD_ZERO(dd))) {	/* ITE(0,G,H) = H */
        return(h);
    }
    topf = cuddIZ(dd,f->index);
    topg = cuddIZ(dd,g->index);
    toph = cuddIZ(dd,h->index);
    v = ddMin(topg,toph);
    top  = ddMin(topf,v);

    tautology = (top == CUDD_MAXINDEX) ? DD_ONE(dd) : dd->univ[top];
    if (f == tautology) {			/* ITE(1,G,H) = G */
    	return(g);
    }

    /* From now on, f is known to not be a constant. */
    zddVarToConst(f,&g,&h,tautology,empty);

    /* Check remaining one variable cases. */
    if (g == h) {			/* ITE(F,G,G) = G */
        return(g);
    }

    if (g == tautology) {			/* ITE(F,1,0) = F */
        if (h == empty) return(f);
    }

    /* Check cache. */
    r = cuddCacheLookupZdd(dd,DD_ZDD_ITE_TAG,f,g,h);
    if (r != NULL) {
        return(r);
    }

    /* Recompute these because they may have changed in zddVarToConst. */
    topg = cuddIZ(dd,g->index);
    toph = cuddIZ(dd,h->index);
    v = ddMin(topg,toph);

    if (topf < v) {
        r = cuddZddIte(dd,cuddE(f),g,h);
        if (r == NULL) return(NULL);
    } else if (topf > v) {
        if (topg > v) {
            Gvn = g;
            index = h->index;
        } else {
            Gvn = cuddE(g);
            index = g->index;
        }
        if (toph > v) {
            Hv = empty; Hvn = h;
        } else {
            Hv = cuddT(h); Hvn = cuddE(h);
        }
        e = cuddZddIte(dd,f,Gvn,Hvn);
        if (e == NULL) return(NULL);
        cuddRef(e);
        r = cuddZddGetNode(dd,index,Hv,e);
        if (r == NULL) {
            Cudd_RecursiveDerefZdd(dd,e);
            return(NULL);
        }
        cuddDeref(e);
    } else {
        index = f->index;
        if (topg > v) {
            Gv = empty; Gvn = g;
        } else {
            Gv = cuddT(g); Gvn = cuddE(g);
        }
        if (toph > v) {
            Hv = empty; Hvn = h;
        } else {
            Hv = cuddT(h); Hvn = cuddE(h);
        }
        e = cuddZddIte(dd,cuddE(f),Gvn,Hvn);
        if (e == NULL) return(NULL);
        cuddRef(e);
        t = cuddZddIte(dd,cuddT(f),Gv,Hv);
        if (t == NULL) {
            Cudd_RecursiveDerefZdd(dd,e);
            return(NULL);
        }
        cuddRef(t);
        r = cuddZddGetNode(dd,index,t,e);
        if (r == NULL) {
            Cudd_RecursiveDerefZdd(dd,e);
            Cudd_RecursiveDerefZdd(dd,t);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert(dd,DD_ZDD_ITE_TAG,f,g,h,r);

    return(r);

} /* end of cuddZddIte */


/**Function********************************************************************

   Synopsis    [Performs the recursive step of Cudd_zddIte.]

   Description [Modified by Manish and Graham]

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
mg_cuddZddIte(
              DdManager * dd,
              DdNode * f,
              DdNode * g,
              DdNode * h)
{
    DdNode *tautology, *empty;
    DdNode *r,*Gv,*Gvn,*Hv,*Hvn,*t,*e;
    unsigned int topf,topg,toph,v,top;
    int index;

    statLine(dd);
    /* Trivial cases. */
    /* One variable cases. */
    if (f == (empty = DD_ZERO(dd))) {	/* ITE(0,G,H) = H */
        return(h);
    }
    topf = cuddIZ(dd,f->index);
    topg = cuddIZ(dd,g->index);
    toph = cuddIZ(dd,h->index);
    v = ddMin(topg,toph);
    top  = ddMin(topf,v);

    tautology = (top == CUDD_MAXINDEX) ? DD_ONE(dd) : dd->univ[top];
    if (f == tautology) {			/* ITE(1,G,H) = G */
    	return(g);
    }

    /* From now on, f is known to not be a constant. */
    zddVarToConst(f,&g,&h,tautology,empty);

    /* Check remaining one variable cases. */
    if (g == h) {			/* ITE(F,G,G) = G */
        return(g);
    }

    if (g == tautology) {			/* ITE(F,1,0) = F */
        if (h == empty) return(f);
    }

    /* Check cache. */
    r = cuddCacheLookupZdd(dd,DD_ZDD_ITE_TAG,f,g,h);
    if (r != NULL) {
        return(r);
    }

    /* Recompute these because they may have changed in zddVarToConst. */
    topg = cuddIZ(dd,g->index);
    toph = cuddIZ(dd,h->index);
    v = ddMin(topg,toph);

    if (topf < v) {
        r = cuddZddIte(dd,cuddE(f),g,h);
        if (r == NULL) return(NULL);
    } else if (topf > v) {
        if (topg > v) {
            Gvn = g;
            index = h->index;
        } else {
            Gvn = cuddE(g);
            index = g->index;
        }
        if (toph > v) {
            Hv = empty; Hvn = h;
        } else {
            Hv = cuddT(h); Hvn = cuddE(h);
        }
        e = cuddZddIte(dd,f,Gvn,Hvn);
        if (e == NULL) return(NULL);
        cuddRef(e);
        r = cuddZddGetNode(dd,index,Hv,e);
        if (r == NULL) {
            Cudd_RecursiveDerefZdd(dd,e);
            return(NULL);
        }
        cuddDeref(e);
    } else {
        index = f->index;
        if (topg > v) {
            Gv = empty; Gvn = g;
        } else {
            Gv = cuddT(g); Gvn = cuddE(g);
        }
        if (toph > v) {
            Hv = empty; Hvn = h;
        } else {
            Hv = cuddT(h); Hvn = cuddE(h);
        }
        e = cuddZddIte(dd,cuddE(f),Gvn,Hvn);
        if (e == NULL) return(NULL);
        cuddRef(e);
        t = cuddZddIte(dd,cuddT(f),Gv,Hv);
        if (t == NULL) {
            Cudd_RecursiveDerefZdd(dd,e);
            return(NULL);
        }
        cuddRef(t);
        r = cuddZddGetNode(dd,index,t,e);
        if (r == NULL) {
            Cudd_RecursiveDerefZdd(dd,e);
            Cudd_RecursiveDerefZdd(dd,t);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert(dd,DD_ZDD_ITE_TAG,f,g,h,r);

    return(r);

} /* end of mg_cuddZddIte */


/**Function********************************************************************

   Synopsis [Performs the recursive step of Cudd_zddUnion.]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
cuddZddUnion(
             DdManager * zdd,
             DdNode * P,
             DdNode * Q)
{
    int		p_top, q_top;
    DdNode	*empty = DD_ZERO(zdd), *t, *e, *res;
    DdManager	*table = zdd;

    statLine(zdd);
    if (P == empty)
        return(Q);
    if (Q == empty)
        return(P);
    if (P == Q)
        return(P);

    /* Check cache */
    res = cuddCacheLookup2Zdd(table, cuddZddUnion, P, Q);
    if (res != NULL)
        return(res);

    if (cuddIsConstant(P))
        p_top = P->index;
    else
        p_top = zdd->permZ[P->index];
    if (cuddIsConstant(Q))
        q_top = Q->index;
    else
        q_top = zdd->permZ[Q->index];
    if (p_top < q_top) {
        e = cuddZddUnion(zdd, cuddE(P), Q);
        if (e == NULL) return (NULL);
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, cuddT(P), e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(table, e);
            return(NULL);
        }
        cuddDeref(e);
    } else if (p_top > q_top) {
        e = cuddZddUnion(zdd, P, cuddE(Q));
        if (e == NULL) return(NULL);
        cuddRef(e);
        res = cuddZddGetNode(zdd, Q->index, cuddT(Q), e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(table, e);
            return(NULL);
        }
        cuddDeref(e);
    } else {
        t = cuddZddUnion(zdd, cuddT(P), cuddT(Q));
        if (t == NULL) return(NULL);
        cuddRef(t);
        e = cuddZddUnion(zdd, cuddE(P), cuddE(Q));
        if (e == NULL) {
            Cudd_RecursiveDerefZdd(table, t);
            return(NULL);
        }
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, t, e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(table, t);
            Cudd_RecursiveDerefZdd(table, e);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert2(table, cuddZddUnion, P, Q, res);

    return(res);

} /* end of cuddZddUnion */


/**Function********************************************************************

   Synopsis [Performs the recursive step of Cudd_zddIntersect.]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
cuddZddIntersect(
                 DdManager * zdd,
                 DdNode * P,
                 DdNode * Q)
{
    int		p_top, q_top;
    DdNode	*empty = DD_ZERO(zdd), *t, *e, *res;
    DdManager	*table = zdd;

    statLine(zdd);
    if (P == empty)
        return(empty);
    if (Q == empty)
        return(empty);
    if (P == Q)
        return(P);

    /* Check cache. */
    res = cuddCacheLookup2Zdd(table, cuddZddIntersect, P, Q);
    if (res != NULL)
        return(res);

    if (cuddIsConstant(P))
        p_top = P->index;
    else
        p_top = zdd->permZ[P->index];
    if (cuddIsConstant(Q))
        q_top = Q->index;
    else
        q_top = zdd->permZ[Q->index];
    if (p_top < q_top) {
        res = cuddZddIntersect(zdd, cuddE(P), Q);
        if (res == NULL) return(NULL);
    } else if (p_top > q_top) {
        res = cuddZddIntersect(zdd, P, cuddE(Q));
        if (res == NULL) return(NULL);
    } else {
        t = cuddZddIntersect(zdd, cuddT(P), cuddT(Q));
        if (t == NULL) return(NULL);
        cuddRef(t);
        e = cuddZddIntersect(zdd, cuddE(P), cuddE(Q));
        if (e == NULL) {
            Cudd_RecursiveDerefZdd(table, t);
            return(NULL);
        }
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, t, e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(table, t);
            Cudd_RecursiveDerefZdd(table, e);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert2(table, cuddZddIntersect, P, Q, res);

    return(res);

} /* end of cuddZddIntersect */


/**Function********************************************************************

   Synopsis [Performs the recursive step of Cudd_zddDiff.]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
cuddZddDiff(
            DdManager * zdd,
            DdNode * P,
            DdNode * Q)
{
    int		p_top, q_top;
    DdNode	*empty = DD_ZERO(zdd), *t, *e, *res;
    DdManager	*table = zdd;

    statLine(zdd);
    if (P == empty)
        return(empty);
    if (Q == empty)
        return(P);
    if (P == Q)
        return(empty);

    /* Check cache.  The cache is shared by Cudd_zddDiffConst(). */
    res = cuddCacheLookup2Zdd(table, cuddZddDiff, P, Q);
    if (res != NULL && res != DD_NON_CONSTANT)
        return(res);

    if (cuddIsConstant(P))
        p_top = P->index;
    else
        p_top = zdd->permZ[P->index];
    if (cuddIsConstant(Q))
        q_top = Q->index;
    else
        q_top = zdd->permZ[Q->index];
    if (p_top < q_top) {
        e = cuddZddDiff(zdd, cuddE(P), Q);
        if (e == NULL) return(NULL);
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, cuddT(P), e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(table, e);
            return(NULL);
        }
        cuddDeref(e);
    } else if (p_top > q_top) {
        res = cuddZddDiff(zdd, P, cuddE(Q));
        if (res == NULL) return(NULL);
    } else {
        t = cuddZddDiff(zdd, cuddT(P), cuddT(Q));
        if (t == NULL) return(NULL);
        cuddRef(t);
        e = cuddZddDiff(zdd, cuddE(P), cuddE(Q));
        if (e == NULL) {
            Cudd_RecursiveDerefZdd(table, t);
            return(NULL);
        }
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, t, e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(table, t);
            Cudd_RecursiveDerefZdd(table, e);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert2(table, cuddZddDiff, P, Q, res);

    return(res);

} /* end of cuddZddDiff */


/**Function********************************************************************

   Synopsis [Performs the recursive step of Cudd_zddChange.]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
DdNode *
cuddZddChangeAux(
                 DdManager * zdd,
                 DdNode * P,
                 DdNode * zvar)
{
    int		top_var, level;
    DdNode	*res, *t, *e;
    DdNode	*base = DD_ONE(zdd);
    DdNode	*empty = DD_ZERO(zdd);

    statLine(zdd);
    if (P == empty)
        return(empty);
    if (P == base)
        return(zvar);

    /* Check cache. */
    res = cuddCacheLookup2Zdd(zdd, cuddZddChangeAux, P, zvar);
    if (res != NULL)
        return(res);

    top_var = zdd->permZ[P->index];
    level = zdd->permZ[zvar->index];

    if (top_var > level) {
        res = cuddZddGetNode(zdd, zvar->index, P, DD_ZERO(zdd));
        if (res == NULL) return(NULL);
    } else if (top_var == level) {
        res = cuddZddGetNode(zdd, zvar->index, cuddE(P), cuddT(P));
        if (res == NULL) return(NULL);
    } else {
        t = cuddZddChangeAux(zdd, cuddT(P), zvar);
        if (t == NULL) return(NULL);
        cuddRef(t);
        e = cuddZddChangeAux(zdd, cuddE(P), zvar);
        if (e == NULL) {
            Cudd_RecursiveDerefZdd(zdd, t);
            return(NULL);
        }
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, t, e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(zdd, t);
            Cudd_RecursiveDerefZdd(zdd, e);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert2(zdd, cuddZddChangeAux, P, zvar, res);

    return(res);

} /* end of cuddZddChangeAux */


/**Function********************************************************************

   Synopsis [Computes the positive cofactor of a ZDD w.r.t. a variable.]

   Description [Computes the positive cofactor of a ZDD w.r.t. a
   variable. In terms of combinations, the result is the set of all
   combinations in which the variable is asserted. Returns a pointer to
   the result if successful; NULL otherwise. cuddZddSubset1 performs
   the same function as Cudd_zddSubset1, but does not restart if
   reordering has taken place. Therefore it can be called from within a
   recursive procedure.]

   SideEffects [None]

   SeeAlso     [cuddZddSubset0 Cudd_zddSubset1]

******************************************************************************/
DdNode *
cuddZddSubset1(
               DdManager * dd,
               DdNode * P,
               int  var)
{
    DdNode	*zvar, *r;
    DdNode	*base, *empty;

    base = DD_ONE(dd);
    empty = DD_ZERO(dd);

    zvar = cuddUniqueInterZdd(dd, var, base, empty);
    if (zvar == NULL) {
        return(NULL);
    } else {
        cuddRef(zvar);
        r = zdd_subset1_aux(dd, P, zvar);
        if (r == NULL) {
            Cudd_RecursiveDerefZdd(dd, zvar);
            return(NULL);
        }
        cuddRef(r);
        Cudd_RecursiveDerefZdd(dd, zvar);
    }

    cuddDeref(r);
    return(r);

} /* end of cuddZddSubset1 */


/**Function********************************************************************

   Synopsis [Computes the negative cofactor of a ZDD w.r.t. a variable.]

   Description [Computes the negative cofactor of a ZDD w.r.t. a
   variable. In terms of combinations, the result is the set of all
   combinations in which the variable is negated. Returns a pointer to
   the result if successful; NULL otherwise. cuddZddSubset0 performs
   the same function as Cudd_zddSubset0, but does not restart if
   reordering has taken place. Therefore it can be called from within a
   recursive procedure.]

   SideEffects [None]

   SeeAlso     [cuddZddSubset1 Cudd_zddSubset0]

******************************************************************************/
DdNode *
cuddZddSubset0(
               DdManager * dd,
               DdNode * P,
               int  var)
{
    DdNode	*zvar, *r;
    DdNode	*base, *empty;

    base = DD_ONE(dd);
    empty = DD_ZERO(dd);

    zvar = cuddUniqueInterZdd(dd, var, base, empty);
    if (zvar == NULL) {
        return(NULL);
    } else {
        cuddRef(zvar);
        r = zdd_subset0_aux(dd, P, zvar);
        if (r == NULL) {
            Cudd_RecursiveDerefZdd(dd, zvar);
            return(NULL);
        }
        cuddRef(r);
        Cudd_RecursiveDerefZdd(dd, zvar);
    }

    cuddDeref(r);
    return(r);

} /* end of cuddZddSubset0 */


/**Function********************************************************************

   Synopsis [Substitutes a variable with its complement in a ZDD.]

   Description [Substitutes a variable with its complement in a ZDD.
   returns a pointer to the result if successful; NULL
   otherwise. cuddZddChange performs the same function as
   Cudd_zddChange, but does not restart if reordering has taken
   place. Therefore it can be called from within a recursive
   procedure.]

   SideEffects [None]

   SeeAlso     [Cudd_zddChange]

******************************************************************************/
DdNode *
cuddZddChange(
              DdManager * dd,
              DdNode * P,
              int  var)
{
    DdNode	*zvar, *res;

    zvar = cuddUniqueInterZdd(dd, var, DD_ONE(dd), DD_ZERO(dd));
    if (zvar == NULL) return(NULL);
    cuddRef(zvar);

    res = cuddZddChangeAux(dd, P, zvar);
    if (res == NULL) {
        Cudd_RecursiveDerefZdd(dd,zvar);
        return(NULL);
    }
    cuddRef(res);
    Cudd_RecursiveDerefZdd(dd,zvar);
    cuddDeref(res);
    return(res);

} /* end of cuddZddChange */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

   Synopsis [Performs the recursive step of Cudd_zddSubset1.]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
static DdNode *
zdd_subset1_aux(
                DdManager * zdd,
                DdNode * P,
                DdNode * zvar)
{
    int		top_var, level;
    DdNode	*res, *t, *e;
    DdNode	*empty;

    statLine(zdd);
    empty = DD_ZERO(zdd);

    /* Check cache. */
    res = cuddCacheLookup2Zdd(zdd, zdd_subset1_aux, P, zvar);
    if (res != NULL)
        return(res);

    if (cuddIsConstant(P)) {
        res = empty;
        cuddCacheInsert2(zdd, zdd_subset1_aux, P, zvar, res);
        return(res);
    }

    top_var = zdd->permZ[P->index];
    level = zdd->permZ[zvar->index];

    if (top_var > level) {
        res = empty;
    } else if (top_var == level) {
        res = cuddT(P);
    } else {
        t = zdd_subset1_aux(zdd, cuddT(P), zvar);
        if (t == NULL) return(NULL);
        cuddRef(t);
        e = zdd_subset1_aux(zdd, cuddE(P), zvar);
        if (e == NULL) {
            Cudd_RecursiveDerefZdd(zdd, t);
            return(NULL);
        }
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, t, e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(zdd, t);
            Cudd_RecursiveDerefZdd(zdd, e);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert2(zdd, zdd_subset1_aux, P, zvar, res);

    return(res);

} /* end of zdd_subset1_aux */


/**Function********************************************************************

   Synopsis [Performs the recursive step of Cudd_zddSubset0.]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
static DdNode *
zdd_subset0_aux(
                DdManager * zdd,
                DdNode * P,
                DdNode * zvar)
{
    int		top_var, level;
    DdNode	*res, *t, *e;

    statLine(zdd);

    /* Check cache. */
    res = cuddCacheLookup2Zdd(zdd, zdd_subset0_aux, P, zvar);
    if (res != NULL)
        return(res);

    if (cuddIsConstant(P)) {
        res = P;
        cuddCacheInsert2(zdd, zdd_subset0_aux, P, zvar, res);
        return(res);
    }

    top_var = zdd->permZ[P->index];
    level = zdd->permZ[zvar->index];

    if (top_var > level) {
        res = P;
    }
    else if (top_var == level) {
        res = cuddE(P);
    }
    else {
        t = zdd_subset0_aux(zdd, cuddT(P), zvar);
        if (t == NULL) return(NULL);
        cuddRef(t);
        e = zdd_subset0_aux(zdd, cuddE(P), zvar);
        if (e == NULL) {
            Cudd_RecursiveDerefZdd(zdd, t);
            return(NULL);
        }
        cuddRef(e);
        res = cuddZddGetNode(zdd, P->index, t, e);
        if (res == NULL) {
            Cudd_RecursiveDerefZdd(zdd, t);
            Cudd_RecursiveDerefZdd(zdd, e);
            return(NULL);
        }
        cuddDeref(t);
        cuddDeref(e);
    }

    cuddCacheInsert2(zdd, zdd_subset0_aux, P, zvar, res);

    return(res);

} /* end of zdd_subset0_aux */


/**Function********************************************************************

   Synopsis    [Replaces variables with constants if possible (part of
   canonical form).]

   Description []

   SideEffects [None]

   SeeAlso     []

******************************************************************************/
static void
zddVarToConst(
              DdNode * f,
              DdNode ** gp,
              DdNode ** hp,
              DdNode * base,
              DdNode * empty)
{
    DdNode *g = *gp;
    DdNode *h = *hp;

    if (f == g) { /* ITE(F,F,H) = ITE(F,1,H) = F + H */
        *gp = base;
    }

    if (f == h) { /* ITE(F,G,F) = ITE(F,G,0) = F * G */
        *hp = empty;
    }

} /* end of zddVarToConst */

