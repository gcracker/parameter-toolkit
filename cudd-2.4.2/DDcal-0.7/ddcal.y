%{
/* $Id: ddcal.y,v 1.26 2009/02/21 19:58:06 fabio Exp fabio $ */
#include <string.h>
#include <math.h>
#define DDCAL_MAIN
#include "ddcal.h"
  static void usage(char const *);
%}

%union {
    DdNode * dval;
    struct symtab *symp;
    void *vptr;
}
%token <symp> NAME
%token <dval> NODE
%token <vptr> XNOR IMPLIES GETS
%left IMPLIES
%left '?' '!'
%left '^' XNOR
%left '+'
%left '*'
%left '/' GETS
%left '_'
%nonassoc '\''

%type <dval> expression
%%
program:
	/* nothing */
	| statement_list
	;

statement_list:
	statement
	| error '\n' {
	    yyclearin;
	    if (graphics) {
		(void) fprintf(stdout,"error\n");
	    }
	}
	| statement_list statement
	| statement_list error '\n' {
	    yyclearin;
	    if (graphics) {
		(void) fprintf(stdout,"error\n");
	    }
	}
	;

statement:
	'\n' {
	    if (graphics) {
		(void) fprintf(stdout,"stop\n");
	    }
	}
	| NAME '=' expression '\n' {
	    if ($1->value != NULL) {
		Cudd_RecursiveDeref(manager,$1->value);
	    }
	    $1->value = $3;
	    if (graphics) {
		(void) fprintf(stdout,"stop\n");
	    }
	}
	| expression '\n' {
	    noutputs = 1;
	    onames[0] = defname[0];
	    if (graphics) {
		DdNode *local = $1;
		if (!bddDump(manager,&local,NULL,0)) {
		    yyerror("bddDump failed!\n");
		    YYERROR;
		}
	    } else {
		if (!Cudd_PrintDebug(manager,$1,Cudd_ReadSize(manager),2)) {
		    yyerror("Cudd_PrintDebug failed!\n");
		    YYERROR;
		}
	    }
	    Cudd_RecursiveDeref(manager,lastexp);
	    lastexp = $1;
	}
	| '[' {noutputs = 0; ddTypeSave = ddType; ddType = BDD_TYPE + ADD_TYPE;} namelist ']' '\n' {
	    ddType = ddTypeSave;
	    if (graphics) {
		if (!bddDump(manager,nodes,NULL,0)) {
		    yyerror("bddDump failed!\n");
		    YYERROR;
		}
	    }
	}
	| command '\n'
	;

expression:
	expression '+' expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddOr(manager,$1,$3);
	    else
		$$ = Cudd_addApply(manager,Cudd_addOr,$1,$3);
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression '*' expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddAnd(manager,$1,$3);
	    else
		$$ = Cudd_addApply(manager,Cudd_addTimes,$1,$3);
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression '^' expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddXor(manager,$1,$3);
	    else
		$$ = Cudd_addApply(manager,Cudd_addXor,$1,$3);
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression XNOR expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddXnor(manager,$1,$3);
	    else
		$$ = Cudd_addApply(manager,Cudd_addXnor,$1,$3);
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression IMPLIES expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddOr(manager,Cudd_Not($1),$3);
	    else
		$$ = Cudd_addIte(manager,$1,$3,Cudd_ReadOne(manager));
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression '_' expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddConstrain(manager,$1,$3);
	    else
		$$ = Cudd_addConstrain(manager,$1,$3);
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression '?' expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddExistAbstract(manager,$1,$3);
	    else
		$$ = Cudd_addOrAbstract(manager,$1,$3);
	    if ($$ == NULL) {
		Cudd_RecursiveDeref(manager,$1);
		Cudd_RecursiveDeref(manager,$3);
		YYERROR;
	    }
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression '!' expression {
	    if (isBDD(ddType))
		$$ = Cudd_bddUnivAbstract(manager,$1,$3);
	    else
		$$ = Cudd_addUnivAbstract(manager,$1,$3);
	    if ($$ == NULL) {
		Cudd_RecursiveDeref(manager,$1);
		Cudd_RecursiveDeref(manager,$3);
		YYERROR;
	    }
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$3);
	}
	| expression '/' NAME GETS expression {
	    if (!isBDD(ddType)) {
		DdNode *var = Cudd_addIthVar(manager,$3->value->index);
		Cudd_Ref(var);
		if (Cudd_IsConstant($3->value) || $3->value != var) {
		    Cudd_RecursiveDeref(manager,$1);
		    Cudd_RecursiveDeref(manager,$5);
		    Cudd_RecursiveDeref(manager,var);
		    yyerror("invalid variable name");
		    YYERROR;
		}
		Cudd_RecursiveDeref(manager,var);
		$$ = Cudd_addCompose(manager,$1,$5,$3->value->index);
	    } else {
		if (Cudd_IsConstant($3->value) ||
		    $3->value != Cudd_bddIthVar(manager,$3->value->index)) {
		    Cudd_RecursiveDeref(manager,$1);
		    Cudd_RecursiveDeref(manager,$5);
		    yyerror("invalid variable name");
		    YYERROR;
		}
		$$ = Cudd_bddCompose(manager,$1,$5,$3->value->index);
	    }
	    if ($$ == NULL) {
		Cudd_RecursiveDeref(manager,$1);
		Cudd_RecursiveDeref(manager,$5);
		YYERROR;
	    }
	    Cudd_Ref($$);
	    Cudd_RecursiveDeref(manager,$1);
	    Cudd_RecursiveDeref(manager,$5);
	}
	| expression '\'' {
	    if (isBDD(ddType))
		$$ = Cudd_Not($1);
	    else {
		$$ = Cudd_addCmpl(manager,$1);
		Cudd_Ref($$);
		Cudd_RecursiveDeref(manager,$1);
	    }
	}
	| '(' expression ')' {
	    $$ = $2;
	}
	| NODE {
	    $$ = $1;
	    Cudd_Ref($$);
	}
	| NAME {
	    if ($1->value == NULL) {
		struct symtab *auxp;
		if (isBDD(ddType))
		    if ((auxp = exists($1->name,ADD_TYPE)) != NULL) {
			$1->value = Cudd_bddIthVar(manager,auxp->value->index);
		    } else {
			$1->value = Cudd_bddNewVar(manager);
		    }
		else
		    if ((auxp = exists($1->name,BDD_TYPE)) != NULL) {
			$1->value = Cudd_addIthVar(manager,auxp->value->index);
		    } else {
			$1->value = Cudd_addNewVar(manager);
		    }
		Cudd_Ref($1->value);
	    }
	    $$ = $1->value;
	    Cudd_Ref($$);
	}
	;
namelist:
	NAME {
	    if ($1->value == NULL) {
		YYERROR;
	    }
	    nodes[noutputs] = $1->value;
	    onames[noutputs] = $1->name;
	    noutputs++;
	}
	| namelist NAME {
	    if ($2->value == NULL) {
		YYERROR;
	    }
	    nodes[noutputs] = $2->value;
	    onames[noutputs] = $2->name;
	    noutputs++;
	}
	;
command:
	':' NAME {
	    if (strcmp($2->name, "reorder") == 0) {
		int option = (Cudd_ReadSize(manager) < 10) ?
		    CUDD_REORDER_EXACT : CUDD_REORDER_SIFT_CONVERGE;
		if (!Cudd_ReduceHeap(manager,option,1)) {
		    yyerror("Cudd_ReduceHeap failed!");
		    YYERROR;
		}
		if (graphics) {
		    (void) fprintf(stdout,"stop\n");
		}
	    } else if (strcmp($2->name, "last") == 0) {
		if (!do_last(NULL,0)) YYERROR;
	    } else if (strcmp($2->name, "concentrate") == 0) {
		concentrate = 1;
		if (graphics) {
		    (void) fprintf(stdout,"stop\n");
		}
	    } else if (strcmp($2->name, "noconcentrate") == 0) {
		concentrate = 0;
		if (graphics) {
		    (void) fprintf(stdout,"stop\n");
		}
	    } else if (strcmp($2->name, "complementarcs") == 0) {
		ddType = BDD_TYPE;
		if (graphics) {
		    (void) fprintf(stdout,"stop\n");
		}
	    } else if (strcmp($2->name, "nocomplementarcs") == 0) {
		ddType = ADD_TYPE;
		if (graphics) {
		    (void) fprintf(stdout,"stop\n");
		}
	    } else {
		yyerror("unrecognized command");
		YYERROR;
	    }
	}
	| ':' NAME NAME {
	    if (strcmp($2->name, "down") == 0) {
		int i,x,y,level,retval;
		int *permutation;
		x = $3->value->index;
		if (isBDD(ddType)) {
		    if (Cudd_IsConstant($3->value) ||
			$3->value != Cudd_bddIthVar(manager,x)) {
			yyerror("invalid variable name");
			YYERROR;
		    }
		} else {
		    DdNode *var = Cudd_addIthVar(manager,x);
		    Cudd_Ref(var);
		    if (Cudd_IsConstant($3->value) || $3->value != var) {
			yyerror("invalid variable name");
			Cudd_RecursiveDeref(manager,var);
			YYERROR;
		    }
		    Cudd_RecursiveDeref(manager,var);
		}
		level = Cudd_ReadIndex(manager,x);
		if (level < Cudd_ReadSize(manager) - 1) {
		    y = Cudd_ReadInvPerm(manager,level+1);
		    permutation = ALLOC(int,Cudd_ReadSize(manager));
		    for (i = 0; i < Cudd_ReadSize(manager); i++) {
			permutation[i] = Cudd_ReadInvPerm(manager,i);
		    }
		    permutation[level] = y;
		    permutation[level+1] = x;
		    retval = Cudd_ShuffleHeap(manager,permutation);
		    FREE(permutation);
		    if (!retval) {
			yyerror("Cudd_ShuffleHeap failed!");
			YYERROR;
		    }
		    if (!do_last(NULL,0)) YYERROR;
		} else if (graphics) {
		    (void) fprintf(stdout,"beep\n");
		}
	    } else if (strcmp($2->name, "up") == 0) {
		int i,x,y,level,retval;
		int *permutation;
		y = $3->value->index;
		if (isBDD(ddType)) {
		    if (Cudd_IsConstant($3->value) ||
			$3->value != Cudd_bddIthVar(manager,y)) {
			yyerror("invalid variable name");
			YYERROR;
		    }
		} else {
		    DdNode *var = Cudd_addIthVar(manager,y);
		    Cudd_Ref(var);
		    if (Cudd_IsConstant($3->value) || $3->value != var) {
			yyerror("invalid variable name");
			Cudd_RecursiveDeref(manager,var);
			YYERROR;
		    }
		    Cudd_RecursiveDeref(manager,var);
		}
		level = Cudd_ReadIndex(manager,y);
		if (level > 0) {
		    x = Cudd_ReadInvPerm(manager,level-1);
		    permutation = ALLOC(int,Cudd_ReadSize(manager));
		    for (i = 0; i < Cudd_ReadSize(manager); i++) {
			permutation[i] = Cudd_ReadInvPerm(manager,i);
		    }
		    permutation[level] = x;
		    permutation[level-1] = y;
		    retval = Cudd_ShuffleHeap(manager,permutation);
		    FREE(permutation);
		    if (!retval) {
			yyerror("Cudd_ShuffleHeap failed!");
			YYERROR;
		    }
		    if (!do_last(NULL,0)) YYERROR;
		} else if (graphics) {
		    (void) fprintf(stdout,"beep\n");
		}
	    } else if (strcmp($2->name, "dot") == 0) {
		if (!do_last($3->name,0)) YYERROR;
		if (graphics) {
		    (void) fprintf(stdout,"stop\n");
		}
	    } else if (strcmp($2->name, "blif") == 0) {
		if (!do_last($3->name,1)) YYERROR;
		if (graphics) {
		    (void) fprintf(stdout,"stop\n");
		}
	    } else {
		yyerror("unrecognized command");
		YYERROR;
	    }
	}
	;
%%
/* look up a symbol table entry, add if not present */
struct symtab *
symlook(char const *s, int type)
{
    struct symtab *sp;
    
    for (sp = symtab; sp < &symtab[NSYMS]; sp++) {
	/* is it already here? */
	if (sp->name && !strcmp(sp->name, s) && (sp->type & type))
	    return sp;
	
	/* is it free? */
	if (!sp->name) {
	    sp->name = strdup(s);
	    sp->type = type;
	    sp->value = NULL;
	    return sp;
	}
	/* otherwise continue to next */
    }
    yyerror("Too many symbols");
    exit(1);	/* cannot continue */

} /* end of symlook */

struct symtab *
exists(char const *s, int type)
{
    struct symtab *sp;
    
    for (sp = symtab; sp < &symtab[NSYMS]; sp++) {
	/* is it already here? */
	if (!sp->name)
	    return NULL;
	if (!strcmp(sp->name, s) && sp->type == type)
	    return sp;
    }
    return NULL;

} /* end of exists */

/**Function********************************************************************

  Synopsis    [Main program for the BDD calculator.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
main(int argc, char **argv)
{
    int c;	/* variable to read in options */

    setbuf(stdin,NULL);
    setbuf(stdout,NULL);
    manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,1024,1048576);
    graphics = 0;
    concentrate = 0;
    ddType = BDD_TYPE;
    lastexp = Cudd_Not(Cudd_ReadOne(manager));
    Cudd_Ref(lastexp);
    noutputs = 0;

    while ((c = getopt(argc, argv, "gh")) != EOF) {
	switch(c) {
	case 'g':
	    graphics = 1;
	    break;
	case 'h':
	default:
	    usage(argv[0]);
	    break;
	}
    }

    yyparse();

    exit(0);

} /* end of main */


/**Function********************************************************************

  Synopsis	[Error reporting routine.]

  Description	[Error reporting routine.]

  SideEffects	[None]

  SeeAlso	[]

******************************************************************************/
int
yyerror(char const *msg)
{
    if (!graphics) {
	(void) fprintf(stderr, "%s\n", msg);
    }
    return(1);

} /* end of yyerror */


/**Function********************************************************************

  Synopsis    [Writes BDDs to a file in dot format.]

  Description [Writes BDDs to a file in dot format. If parameter filename
  is non-NULL, the output is in not laid out; is in standard format; and
  goes to filename. If filename is NULL, the output is piped through dot
  and sent to stdout.  Returns 1 in case of success; 0 otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
bddDump(
  DdManager *dd		/* DD manager */,
  DdNode **dnodes	/* nodes whose BDDs should be dumped */,
  char *filename	/* name of file for dot format output */,
  int dumpBlif		/* format for file dump (1:blif, 0:dot) */
)
{
    FILE *toCommand;		/* file descriptor for pipe */
    FILE *fromCommand;		/* file descriptor for pipe */
    FILE *fp;			/* file descriptor for file output */
    char s[512];		/* buffer for communications through pipe */
    char *args[4];		/* arguments for call to dot */
    char **inames = NULL;	/* array of input names */
    int retval = 0;		/* 0 -> failure; 1 -> success */
    struct symtab *sp;
    int index;
    int pid;

    /* Find the input names. */
    inames = ALLOC(char *,Cudd_ReadSize(dd));
    if (inames == NULL) {
	(void) fprintf(stderr,"Panic: malloc returned NULL!\n");
	exit(2);
    }
    /* Initialize to NULL all entries of inames, so that we can tell
    ** whether we find synonims for variables. The first we find in the
    ** table is the original name. The others are ignored.
    */
    for (index = 0; index < Cudd_ReadSize(dd); index++) {
	inames[index] = NULL;
    }
    for (sp = symtab; sp < &symtab[NSYMS]; sp++) {
	if (!sp->name) break;
	if (sp->value == NULL) continue;
	if (Cudd_IsComplement(sp->value)) continue;
	if (Cudd_IsConstant(sp->value)) continue;
	index = Cudd_NodeReadIndex(sp->value);
	if (isBDD(sp->type)) {
	    if ((inames[index] == NULL) &&
		(sp->value == Cudd_bddIthVar(dd,index))) {
		inames[index] = sp->name;
	    }
	} else {
	    DdNode *var = Cudd_addIthVar(dd,index);
	    Cudd_Ref(var);
	    if (inames[index] == NULL && sp->value == var) {
		inames[index] = sp->name;
	    }
	    Cudd_RecursiveDeref(dd,var);
	}
    }

    if (filename != NULL) {
	fp = fopen(filename,"w");
	if (fp == NULL) return(0);
	if (dumpBlif) {
	    retval = Cudd_DumpBlif(dd,noutputs,dnodes,inames,onames,
				   filename,fp,0);
	} else {
	    retval = Cudd_DumpDot(dd,noutputs,dnodes,inames,onames,fp);
	}
	fclose(fp);
	return(retval);
    }

    /* Set up args to call "dot -Tplain." */
    args[0] = util_strsav("dot");
    args[1] = util_strsav("-Tplain");
    if (concentrate) {
	args[2] = util_strsav("-Gconcentrate=true");
    } else {
	args[2] = 0;
    }
    args[3] = 0;

    /* Set up bidirectional pipe to/from dot. */
    /* A unidirectional pipe should suffice. We'll try it some day. */
    retval = util_pipefork(args,&toCommand,&fromCommand,&pid);
    if (retval == 0) {
	(void) fprintf(stderr,"Panic: util_pipefork returned 0!\n");
	exit(2);
    }

    retval = Cudd_DumpDot(dd,noutputs,dnodes,inames,onames,toCommand);
    if (retval == 0) {
	(void) fprintf(stderr,"Panic: Cudd_DumpDot returned 0!\n");
	exit(2);
    }
    fclose(toCommand);
    FREE(inames);

    while (!feof(fromCommand)) {
	fgets(s,100,fromCommand);
	if (!feof(fromCommand)) (void) fprintf(stdout,"%s",s);
    }

    return(1);

} /* end of bddDump */


/**Function********************************************************************

  Synopsis    [Prints usage info.]

  Description [Prints usage info and exits.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
usage(char const *prog)
{
    (void) fprintf(stderr, "usage: %s [options]\n", prog);
    (void) fprintf(stderr, "   -g\t\tproduce BDD layout\n");
    (void) fprintf(stderr, "   -h\t\tprints this message\n");
    exit(2);

} /* end of usage */


/**Function********************************************************************

  Synopsis    [Prints the last expression.]

  Description [Prints the last expression.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
do_last(char *filename, int dumpBlif)
{
    if (noutputs == 0) {
	if (graphics) {
	    (void) fprintf(stdout,"stop\n");
	}
	return(1);
    }
    if (graphics || filename != NULL) {
	if (onames[0] == defname[0]) {
	    if (!bddDump(manager,&lastexp,filename,dumpBlif)) {
		yyerror("bddDump failed!\n");
		return(0);
	    }
	} else {
	    if (!bddDump(manager,nodes,filename,dumpBlif)) {
		yyerror("bddDump failed!\n");
		return(0);
	    }
	}
    } else {
	if (!Cudd_PrintDebug(manager,lastexp,Cudd_ReadSize(manager),2)) {
	    yyerror("Cudd_PrintDebug failed!\n");
	    return(0);
	}
    }
    return(1);

} /* end of do_last */
