/**CHeaderFile*****************************************************************

  FileName    [expr.h]

  PackageName [expr]

  Synopsis    [expression management]

  Description []

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

#ifndef _EXPR
#define _EXPR

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum************************************************************************
  Synopsis    [CTL expression codes.]
******************************************************************************/

typedef enum {
  Expr_Ctl_SPEC_c,
  Expr_Ctl_INVARSPEC_c,

  Expr_Ctl_TERMINAL_c,

  Expr_Ctl_TRUE_c,
  Expr_Ctl_FALSE_c,

  Expr_Ctl_AND_c,
  Expr_Ctl_OR_c,
  Expr_Ctl_NOT_c,
  Expr_Ctl_IFF_c,
  Expr_Ctl_IMPLIES_c,

  Expr_Ctl_EX_c,
  Expr_Ctl_AX_c,
  Expr_Ctl_EF_c,
  Expr_Ctl_AG_c,
  Expr_Ctl_AF_c,
  Expr_Ctl_EG_c,
  /* 
  Expr_Ctl_EU_c,
  Expr_Ctl_AU_c,
  Expr_Ctl_EBU_c,
  Expr_Ctl_ABU_c,
  Expr_Ctl_EBF_c,
  Expr_Ctl_ABF_c,
  Expr_Ctl_EBG_c,
  Expr_Ctl_ABG_c,
  */
  Expr_Ctl_Null_code_c
}
Expr_Ctl_Code_e;

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

EXTERN Ddi_Expr_t * Expr_ExprLoad(Ddi_BddMgr *dd, char *filename, FILE *fp);
EXTERN void Expr_WriteTree(FILE *fout, Ddi_Expr_t *tnode);

/**AutomaticEnd***************************************************************/

#endif /* _EXPR */








