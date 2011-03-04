/**CHeaderFile*****************************************************************

  FileName    [testInt.h]

  PackageName [test]

  Synopsis    [Internal header file for pdtrav test package]

  Description [Header file for pdtrav test package. 
    C files in this package provide various tests for pdtrav packages, used
    as a library. In the meanwhile, test programd can be vieved as usage
    examples. No external objects are defined in this package, all functions
    are internal. All testPkg.c (Pkg is one of ddi, fsm, tr, trav, ...) 
    provide a TestPkg() routine, which is called by the package main in 
    test.c]
              
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

#ifndef _TESTINT
#define _TESTINT

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include "ddi.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int main(int argc, char *argv[]);
EXTERN int TestDdi();
EXTERN int TestFsm();
EXTERN int TestTr();
EXTERN int TestTrav();

/**AutomaticEnd***************************************************************/

#endif /* _TESTINT */

