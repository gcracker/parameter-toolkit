/**CFile***********************************************************************

  FileName    [trav.c]

  PackageName [trav]

  Synopsis    [Main module for a simple traversal of finite state machine]

  Description [External procedure included in this file is:
    <ul>
    <li>TravReadOptions()
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

#define MAX_ROW_LEN 120
#define MAX_OPT_LEN 15

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Temporary main program to be used as interface for cmd.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

void
Trav_Main (
  Trav_Mgr_t *travMgr   /* Traversal manager */
)
{
  Ddi_Bdd_t *reached;
  double nstates;
  Ddi_Mgr_t *dd;
  long travTime;
  Pdtutil_VerbLevel_e verbosity;

  dd = travMgr->ddiMgrTr;
  verbosity = Trav_MgrReadVerbosity (travMgr);

  /*-------------------------------- Traversal ------------------------------*/

  travTime = util_cpu_time();

  reached = Trav_Traversal (travMgr);

  travTime = util_cpu_time() - travTime;
  travMgr->travTime += travTime;

  if (reached == NULL) {
    Pdtutil_Warning (1, "Traversal is failed.");
    return;
  }

  /*------------------------- Print Final Statistics -----------------------*/

  if (verbosity >= Pdtutil_VerbLevelUsrMax_c) {
    Pdtutil_ChrPrint (stdout, '*', 50);
    fprintf (stdout, "\n");
    fprintf (stdout, "Reachability Analysis Results Summary:\n");
    fprintf (stdout, "Traversal depth       : %d\n",
      Trav_MgrReadLevel (travMgr) - 1);
    fprintf (stdout, "# REACHED size        : %d\n", Ddi_BddSize(reached));

    if (verbosity >=  Pdtutil_VerbLevelAppMed_c) {
     /* Total number of reached states */
      nstates = Ddi_CountMinterm (reached, 
        Ddi_VararrayNum (Trav_MgrReadPS (travMgr)));
      fprintf (stdout, "# REACHED states      : %g\n", nstates);

      fprintf (stdout, "Max. Internal Product : %d\n",
        Trav_MgrReadProductPeak (travMgr));

      fprintf (stdout, "CPU Time              : %s\n",
        util_print_time (travTime));
    }
    Pdtutil_ChrPrint (stdout, '*', 50);
    fprintf (stdout, "\n");

    fflush (stdout);
  }

  /*--------------------- Partitioning Reached ----------------------------*/

#if 0
  if (Trav_MgrReadVerbosity (travMgr) > 2) {
    Part_BddDisjSuppPart (reached, Trav_MgrReadTr(travMgr),
      Trav_MgrReadPS(travMgr), Trav_MgrReadNS(travMgr),
      Trav_MgrReadVerbosity (travMgr));
  }
#endif

  /*------------------------------ Exit -----------------------------------*/

}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
