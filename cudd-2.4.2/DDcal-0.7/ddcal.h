/* $Id: ddcal.h,v 1.9 1996/11/09 02:53:21 fabio Exp fabio $ */

/*
**	Header for calculator program
**/
#include "util.h"
#include "cudd.h"

/*
**	Masks for diagram types.
*/
#define BDD_TYPE 1
#define ADD_TYPE 2

#define isBDD(x)	((x) & BDD_TYPE)
#define isADD(x)	((x) & ADD_TYPE)

#ifdef DDCAL_MAIN
DdManager *manager;
DdNode *lastexp;
int graphics;
int concentrate;
int ddType;
int ddTypeSave;
char *defname[] = {" "};
int noutputs;
char *onames[100];
DdNode *nodes[100];
#else
extern DdManager *manager;
extern DdNode *lastexp;
extern int graphics;
extern int concentrate;
extern int ddType;
extern int ddTypeSave;
extern char **defname;
extern int noutputs;
extern char **onames;
extern DdNode **nodes;
#endif

#define NSYMS 1000	/* maximum number of symbols */

struct symtab {
    char *name;
    DdNode * value;
    int type;
} symtab[NSYMS];

struct symtab *symlook();
struct symtab *exists();
int bddDump();
int do_last();

