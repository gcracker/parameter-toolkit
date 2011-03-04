/**CFile***********************************************************************

  FileName    [exprRead.c]

  PackageName [expr]

  Synopsis    [Input of PdTrav expressions]

  Description [Interface routine to expr parsing]

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

#include "exprInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

FILE *ExprYyin;
Ddi_Mgr_t *ExprYyDdmgr;
Ddi_Expr_t *ExprYySpecif;

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
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Synopsis    [Load EXPRESSION from file]
  SideEffects [None]
  SeeAlso     []
******************************************************************************/

Ddi_Expr_t *
Expr_ExprLoad (
  Ddi_BddMgr *dd      /* dd manager */,
  char *filename     /* file name */,  
  FILE *fp           /* file pointer */
  )             
{
  Ddi_Expr_t *f;
  int flagFile;

  ExprYyDdmgr = dd;

  f = NULL;
  
  fp = Pdtutil_FileOpen (fp, filename, "r", &flagFile);
  if (fp == NULL) {
    return (0);
  }

  ExprYyin = fp;

  ExprYyparse();
  f = ExprYySpecif;

  Expr_WriteTree(stdout,f);
		 
  Pdtutil_FileClose (fp, &flagFile);

  return (f);
}

/**Function********************************************************************
  Synopsis    [Print EXPRESSION to file]
  SideEffects [None]
  SeeAlso     []
******************************************************************************/


void
Expr_WriteTree(
  FILE *fout,
  Ddi_Expr_t *tnode
)
{
  int i,j;

  i = Ddi_GenericReadCode((Ddi_Generic_t *)tnode);
  if(i == Ddi_Expr_String_c){
    fprintf(fout,"%s",Ddi_ExprToString(tnode));
  }
  else if(i == Ddi_Expr_Ctl_c ){
    i = Ddi_ExprSubNum(tnode);
    printf("(");
    Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0));
    
    for(j=1;j<i;j++){
      printf(" op_ctl ");
      Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,j));
    }
    printf(")");
  }

/*      i = Ddi_ExprReadOpcode(tnode); */
/*      switch(i){ */
/*      case SPEC:  */
/*        fprintf(fout,"SPEC\n\t"); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout,"\n"); */
/*        break; */
/*      case ATOM:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout,"%s",tnode->data.char); */
/*      case IMPLIES:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," -> "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case IFF:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," <-> "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case OR:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," | "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case AND:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," & "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case PLUS:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," + "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case MINUS:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," - "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case TIMES:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," * "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case DIVIDE:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," / "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case MOD:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," mod "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case EQUAL:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," = "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case LT:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," < "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case GT:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," > "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case LE:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," <= "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case GE:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," >= "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case UNION:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," union "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case SETIN:   */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," in "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        break; */
/*      case EX:   */
/*        fprintf(fout,"EX "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        break; */
/*      case AX:   */
/*        fprintf(fout,"AX "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        break; */
/*      case EF:   */
/*        fprintf(fout,"EF "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        break; */
/*      case AF:   */
/*        fprintf(fout,"AF "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        break; */
/*      case EG:   */
/*        fprintf(fout,"EG "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        break; */
/*      case AG:   */
/*        fprintf(fout,"AG "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        break; */
/*      case AU:   */
/*        fprintf(fout,"AA ( "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," U "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        fprintf(fout,")"); */
/*        break; */
/*      case EU:   */
/*        fprintf(fout,"EE ( "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,0)); */
/*        fprintf(fout," U "); */
/*        Expr_WriteTree(fout,Ddi_ExprReadSub(tnode,1)); */
/*        fprintf(fout,")"); */
/*        break; */
/*      }     */
  else printf("Ddi code unknown");

  return;
}








