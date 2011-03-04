/**CFile***********************************************************************

  FileName    [test.c]

  PackageName [test]

  Synopsis    [Test package main]

  Description [Test package main. Calls various pdtrav package test routines.]

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

static void Usage(char *prog);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [Main program of test package.]
  Description [Main program of test package.]
  SideEffects []
  SeeAlso     []
******************************************************************************/
int
main (
  int argc,
  char *argv[]
)
{
  char buf[PDTUTIL_MAX_STR_LEN+1];
  char *pkg;
  int i;

  if (argc != 2) {
    Usage(argv[0]);
    return(1);
  }

  pkg = argv[1];

  /*
   *  Print Out Command and Version and etc.
   */

  fprintf (stdout, "# Running Date: ");
  fflush (stdout); 
  system ("date");
  if (gethostname (buf, PDTUTIL_MAX_STR_LEN) == 0) {
    fprintf (stdout, "# HostName: %s\n", buf);
  }
  fflush (stdout);
  fprintf (stdout, "# Command: ");
  for (i=0; i<argc; i++) {
    fprintf (stdout, "%s ", argv[i]);
  }
  fprintf (stdout, "\n");
  fprintf (stdout, "# PdTrav Version: %s\n", PDTRAV_VERSION);
  fprintf (stdout, "# DDI Version: ");
  Ddi_PrintCuddVersion (stdout);

  /*
   *  select package and call relates test
   */

  if (strcmp(pkg,"ddi")==0) {
    TestDdi();
  }
  else if (strcmp(pkg,"tr")==0) {
    TestTr();
  }
  else if (strcmp(pkg,"trav")==0) {
    TestTrav();
  }
  else if (strcmp(pkg,"fsm")==0) {
    TestFsm();
  }
  else {
    fprintf(stderr, "Wrong package selection: %s\n", pkg);
    Usage(argv[0]);
  }

  return (0);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [Print usage message.]
  SideEffects []
******************************************************************************/
static void 
Usage (
  char *prog
)
{
  fprintf(stderr, "USAGE: %s <package>\n", prog);
  fprintf(stderr, "Currently testable packages: ddi tr fsm trav\n");
}












