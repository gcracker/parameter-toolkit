%{
/* $Id: ddcal.l,v 1.10 1997/09/23 19:24:20 fabio Exp fabio $ */
#include "ddcal.h"
#include "y.tab.h"
#include <math.h>

#define YY_INPUT(buf,result,max_size) { \
    int c = getchar(); \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
    }
%}

%%
[ \t]	;		/* ignore white space */

#.*	;		/* comment */

\\[ \t]*\n	;	/* line continuation */

1	{
		yylval.dval = Cudd_ReadOne(manager);
		return NODE;
	}

0	{
		if (isBDD(ddType)) {
		    yylval.dval = Cudd_Not(Cudd_ReadOne(manager));
		} else {
		    yylval.dval = Cudd_ReadZero(manager);
		}
		return NODE;
	}

==	return XNOR;

=>	return IMPLIES;

"<-"	return GETS;

[A-Za-z][A-Za-z0-9\.]*	{	/* return symbol pointer */
		struct symtab *sp = symlook(yytext,ddType);
		yylval.symp = sp;
		return NAME;
	}

"$"	{ return 0; }

\n	|
.	return yytext[0];
%%
