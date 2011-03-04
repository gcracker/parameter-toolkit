/**CHeaderFile*****************************************************************

  FileName    [pdtutil.h]

  PackageName [pdtutil] 

  Synopsis    [Utility functions for the PdTRAV package.]

  Description [This package contains a set of <em>utility functions</em>
    shared in the <b>PdTRAV</b> package.<br>
    In this package are contained functions to allocate and free memory,
    open and close files, deal with strings, read and write variable orders,
    etc.
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

#ifndef _PDTUTIL
#define _PDTUTIL
#define PDTRAV_DEBUG

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include "cuddInt.h"
#include "dddmp.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define PDTRAV_VERSION "2.0"

#define PDTUTIL_MAX_STR_LEN 1000
#define PDTUTIL_INITIAL_ARRAY_SIZE 3

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************

  Synopsis    [Type for verbosity levels.]

  Description [The basic idea is to have verbosity level divided into three
    categories: user, application, and developers. For each of this category
    three levels are defined: minimun, medium, and average.
    ]

******************************************************************************/

typedef enum
  {
  Pdtutil_VerbLevelSame_c,
  Pdtutil_VerbLevelNone_c,
  Pdtutil_VerbLevelUsrMin_c,
  Pdtutil_VerbLevelUsrMed_c,
  Pdtutil_VerbLevelUsrMax_c,
  Pdtutil_VerbLevelAppMin_c,
  Pdtutil_VerbLevelAppMed_c,
  Pdtutil_VerbLevelAppMax_c,
  Pdtutil_VerbLevelDevMin_c,
  Pdtutil_VerbLevelDevMed_c,
  Pdtutil_VerbLevelDevMax_c
  }
Pdtutil_VerbLevel_e;

/**Enum************************************************************************

  Synopsis    [Type to Distinguishing Different Operations of Managers]

  Description [Type to Distinguishing Different Operations of Managers.
    For each package, fsm, tr, trav, etc., a manager is defined.
    On each manager a certain number of operations are permitted and encoded
    as defined by this enumeration type.
    ]

******************************************************************************/

typedef enum {
  Pdtutil_MgrOpNone_c,
  /*
   * Generic Operations (Usually on a Bdd o an Array)
   * used whenever type is not known in advance
   */
  Pdtutil_MgrOpDelete_c,
  Pdtutil_MgrOpRead_c,
  Pdtutil_MgrOpSet_c,
  /*
   * Operation on BDDs
   */
  Pdtutil_MgrOpBddDelete_c,
  Pdtutil_MgrOpBddRead_c,
  Pdtutil_MgrOpBddSet_c,
  /*
   * Operation on Array of BDDs
   */
  Pdtutil_MgrOpArrayDelete_c,
  Pdtutil_MgrOpArrayRead_c,
  Pdtutil_MgrOpArraySet_c,
  /*
   * Operation on Managers
   */
  Pdtutil_MgrOpMgrDelete_c,
  Pdtutil_MgrOpMgrRead_c,
  Pdtutil_MgrOpMgrSet_c,
  /*
   * Set, Show, Stats
   */
  Pdtutil_MgrOpOptionSet_c,
  Pdtutil_MgrOpOptionShow_c,
  Pdtutil_MgrOpStats_c
  }
Pdtutil_MgrOp_t;

/**Enum************************************************************************

  Synopsis    [Type to Establishing The Type Returned]

  Description []

******************************************************************************/

typedef enum {
  Pdtutil_MgrRetNone_c,
  Pdtutil_MgrRetBdd_c,
  Pdtutil_MgrRetBddarray_c,
  Pdtutil_MgrRetDdMgr_c
  }
Pdtutil_MgrRet_t;

/**Enum************************************************************************

  Synopsis    [Type for Variable Order Format.]

  Description [Defines the format for the variable order file.
    For compatibility reasons, with the cudd/nanotrav package and with
    vis, different formats are defined.
    ]

******************************************************************************/

typedef enum
  {
  Pdtutil_VariableOrderDefault_c,
  Pdtutil_VariableOrderOneRow_c,
  Pdtutil_VariableOrderPiPs_c,
  Pdtutil_VariableOrderPiPsNs_c,
  Pdtutil_VariableOrderIndex_c,
  Pdtutil_VariableOrderComment_c
  }
Pdtutil_VariableOrderFormat_e;

/**Enum************************************************************************

  Synopsis    [Operation Codes for DD Operations.]

  Description [Define operation (AND, OR, etc.) on DD types, on operations
    between two BDD, one array of BDDs and on BDD, two arrays of BDDs, etc. 
    ]

******************************************************************************/

typedef enum
  {
  /*
   *  Nothing to do
   */
  Pdtutil_BddOpNone_c,
  /*
   *  Unary Operators
   */
  /* Insert a New Element */
  Pdtutil_BddOpInsert_c,
  /* Insert a New Element After Freeing the Previous One (if not NULL) */
  Pdtutil_BddOpReplace_c,
  Pdtutil_BddOpNot_c,
  /*
   *  Binary Operators
   */
  Pdtutil_BddOpAnd_c,
  Pdtutil_BddOpOr_c,
  Pdtutil_BddOpNand_c,
  Pdtutil_BddOpNor_c,
  Pdtutil_BddOpXor_c,
  Pdtutil_BddOpXnor_c,
  Pdtutil_BddOpExist_c,
  Pdtutil_BddOpAndExist_c,
  Pdtutil_BddOpConstrain_c,
  Pdtutil_BddOpRestrict_c,
  /*
   *  Ternary Operators
   */
  Pdtutil_BddOpIte_c
  }
Pdtutil_BddOp_e;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis    [Checks for fatal bugs]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

#ifdef PDTRAV_DEBUG
#  define Pdtutil_Assert(expr,errMsg) \
     { \
     if ((expr) == 0) { \
       fprintf (stderr, "FATAL ERROR: %s\n", errMsg); \
       fprintf (stderr, "             File %s -> Line %d\n", \
         __FILE__, __LINE__); \
       fflush (stderr); \
       exit (1); \
       } \
     }
#else
#  define Pdtutil_Assert(expr,errMsg) \
     {}
#endif

/**Macro***********************************************************************

  Synopsis    [Checks for Warnings: If expr==1 print out the warning on
    stderr]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

#define Pdtutil_Warning(expr,errMsg) \
  { \
  if ((expr) == 1) { \
    fprintf (stderr, "WARNING: %s\n", errMsg); \
    fprintf (stderr, "         File %s -> Line %d\n", \
      __FILE__, __LINE__); \
    fflush (stderr); \
    } \
  }

/**Macro***********************************************************************

  Synopsis    []

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

#define Pdtutil_Alloc(type,num) \
    ((type *)Pdtutil_AllocCheck(ALLOC(type, num)))

/**Macro***********************************************************************

  Synopsis    []

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

#define Pdtutil_Realloc(type,obj,num) \
    ((type *)Pdtutil_AllocCheck(REALLOC(type, obj, num)))

/**Macro***********************************************************************

  Synopsis    []

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

#define Pdtutil_Free(obj) \
    (FREE(obj))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void * Pdtutil_AllocCheck(void * pointer);
EXTERN void Pdtutil_ChrPrint(FILE *fp, char c, int n);
EXTERN Pdtutil_VerbLevel_e Pdtutil_VerbosityString2Enum(char *string);
EXTERN char * Pdtutil_VerbosityEnum2String(Pdtutil_VerbLevel_e enumType);
EXTERN Pdtutil_VariableOrderFormat_e Pdtutil_VariableOrderFormatString2Enum(char *string);
EXTERN char * Pdtutil_VariableOrderFormatEnum2String(Pdtutil_VariableOrderFormat_e enumType);
EXTERN char * Pdtutil_FileName(char *filename, char *attribute, char *extension, int overwrite);
EXTERN FILE * Pdtutil_FileOpen(FILE *fp, char *filename, char *mode, int *flag);
EXTERN void Pdtutil_FileClose(FILE *fp, int *flag);
EXTERN void Pdtutil_ReadSubstring(char *stringIn, char **stringFirstP, char **stringSecondP);
EXTERN void Pdtutil_ReadName(char *extName, int *nstr, char **names, int maxNstr);
EXTERN char * Pdtutil_StrDup(char *str);
EXTERN int Pdtutil_OrdRead(char ***pvarnames, int **pvarauxids, char *filename, FILE *fp, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN int Pdtutil_OrdWrite(char **varnames, int *varauxids, int *sortedIds, int nvars, char *filename, FILE *fp, Pdtutil_VariableOrderFormat_e ordFileFormat);
EXTERN char ** Pdtutil_StrArrayDup(char **array, int n);
EXTERN int Pdtutil_StrArrayRead(char ***parray, FILE *fp);
EXTERN int Pdtutil_StrArrayWrite(FILE *fp, char **array, int n);
EXTERN void Pdtutil_StrArrayFree(char **array, int n);
EXTERN int * Pdtutil_IntArrayDup(int *array, int n);
EXTERN int Pdtutil_IntArrayRead(int **parray, FILE *fp);
EXTERN int Pdtutil_IntArrayWrite(FILE *fp, int *array, int n);

/**AutomaticEnd***************************************************************/

#endif /* _PDTUTIL */
