/**CHeaderFile*****************************************************************

  FileName    [cuplus.h]

  PackageName [cuplus] 

  Synopsis    [Extensions of the CU package]

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

#ifndef _CUPLUS
#define _CUPLUS

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include "cudd.h"
#include "st.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure & type declarations                                             */
/*---------------------------------------------------------------------------*/

/* only accessed by pointer */
typedef struct profile_info_summary cuplus_profile_info_t; 
typedef struct profile_info_item2 profile_info_item2_t;

/**Enum************************************************************************

  Synopsis    [Type for selecting pruning heuristics.]

  Description []

******************************************************************************/

typedef enum
  {
  Cuplus_ActiveRecursions,
  Cuplus_Recursions,
  Cuplus_RecursionsWithSharing,
  Cuplus_NormSizePrune,
  Cuplus_NormSizePruneLight,
  Cuplus_NormSizePruneHeavy,
  Cuplus_SizePrune,
  Cuplus_SizePruneLight,
  Cuplus_SizePruneHeavy,
  Cuplus_SizePruneLightNoRecur,
  Cuplus_SizePruneHeavyNoRecur,
  Cuplus_None
  }
Cuplus_PruneHeuristic_e;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN DdNode * Cuplus_bddAndAbstract(DdManager * manager, DdNode * f, DdNode * g, DdNode * cube);
EXTERN DdNode * Cuplus_CofexistMask(DdManager *manager, DdNode *f, DdNode *cube);
EXTERN void Cuplus_bddOpEnableAbortOnSift(DdManager * manager);
EXTERN void Cuplus_bddOpDisableAbortOnSift(DdManager * manager);
EXTERN int Cuplus_DoAbortHookFun(DdManager * manager, char *str, void *heuristic);
EXTERN DdNode * Cuplus_ProfileAndAbstract(DdManager *manager, DdNode *f, DdNode *g, DdNode *cube, cuplus_profile_info_t *profile);
EXTERN void Cuplus_ProfileSetCurrentPart(cuplus_profile_info_t *profile, int currentPart);
EXTERN int Cuplus_ProfileReadCurrentPart(cuplus_profile_info_t *profile);
EXTERN DdNode * Cuplus_ProfilePrune(DdManager *manager, DdNode *f, cuplus_profile_info_t *profile, Cuplus_PruneHeuristic_e pruneHeuristic, int threshold);
EXTERN cuplus_profile_info_t * Cuplus_ProfileInfoGen(unsigned char dac99Compatible, int nVar, int nPart);
EXTERN void Cuplus_ProfileInfoFree(cuplus_profile_info_t *profile);
EXTERN void Cuplus_ProfileInfoPrint(cuplus_profile_info_t *profile);
EXTERN void Cuplus_ProfileInfoPrintOriginal(cuplus_profile_info_t *profile);
EXTERN void Cuplus_ProfileInfoPrintByVar(cuplus_profile_info_t *profile);

/**AutomaticEnd***************************************************************/

#endif /* _CUPLUS */

 





