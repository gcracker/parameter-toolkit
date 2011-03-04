/*#*STARTLICENCE*#
Copyright (c) 2005-2009, Regents of the University of Colorado

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

Neither the name of the University of Colorado nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
#*ENDLICENCE*#*/

/* Version 0.0
** From "Wayne's Little DSA Library" (DSA == Data Structures and
** Algorithms) Feel free to change, modify, or burn these sources, but if
** you modify them please don't distribute your changes without a clear
** indication that you've done so.  If you think your change is spiffy,
** send it to me and maybe I'll include it in the next release.
**
** Wayne Hayes, wayne@csri.utoronto.ca (preffered), or wayne@csri.toronto.edu
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>   /*getrusage doesn't seem to work.*/
#include <sys/resource.h>
#include <unistd.h>
/*#include <../ucbinclude/sys/rusage.h>*/

#include "misc.h"

const foint ABSTRACT_ERROR = {(1 << (8*sizeof(void*)-1))};

static FILE *tty;

void Warning(const char *fmt, ...)
{
    va_list ap;
    fflush(stdout);
    fprintf(stderr, "Warning: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    if(!isatty(fileno(stderr)))
    {
	if(!tty)
	    if(!(tty = fopen("/dev/tty", "w")))
		return;
	fprintf(tty, "Warning: ");
	va_start(ap, fmt);
	vfprintf(tty, fmt, ap);
	va_end(ap);
	fprintf(tty, "\n");
    }
}

void Apology(const char *fmt, ...)
{
    va_list ap;
    fflush(stdout);
    fprintf(stderr, "Sorry, fatal limitation: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    if(!isatty(fileno(stderr)))
    {
	if(!tty)
	    tty = fopen("/dev/tty", "w");
	fprintf(tty, "Sorry, fatal limitation: ");
	va_start(ap, fmt);
	vfprintf(tty, fmt, ap);
	va_end(ap);
	fprintf(tty, "\n");
    }
    assert(false);
    exit(1);
}

void Fatal(const char *fmt, ...)
{
    va_list ap;
    fflush(stdout);
    fprintf(stderr, "Fatal Error: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    if(!isatty(fileno(stderr)))
    {
	if(!tty)
	    tty = fopen("/dev/tty", "w");
	fprintf(tty, "Fatal Error: ");
	va_start(ap, fmt);
	vfprintf(tty, fmt, ap);
	va_end(ap);
	fprintf(tty, "\n");
    }
    assert(false);
    exit(1);
}

/* A malloc that exits if system calloc fails.
*/
void *Malloc(int n)
{
    void *p;
    assert(n>=0);
    p = (void*)malloc(n);
    if(!p && n)
	Fatal("malloc failed");
    return p;
}
void *Calloc(int n, int m)
{
    void *p;
    assert(n>=0 && m>=0);
    p = (void*)calloc(n, m);
    if(!p && n && m)
	Fatal("calloc failed");
    return p;
}

void *Realloc(void *ptr, int newSize)
{
    void *p;
    assert(newSize>=0);
    p = (void*) realloc(ptr, newSize);
    if(!p)
	Fatal("realloc failed");
    return p;
}

void *Memdup(void *v, int n)
{
    void *r = Malloc(n);
    memcpy(r, v, n);
    return r;
}


/* return current user time used in seconds */
double uTime(void)
{
#if 1
	struct rusage rUsage;
	getrusage(RUSAGE_SELF, &rUsage);
	return rUsage.ru_utime.tv_sec + 1e-6*rUsage.ru_utime.tv_usec;
#else
	return -1;
#endif
}

char *Int2BitString(char word[33], unsigned i)
{
    int b, k = 0;
    assert(sizeof(unsigned) == 4);
    for(b=31; b >= 0; b--)
	word[k++] = '0' + !!(i & (1<<b));
    word[k] = '\0';
    return word;
}


void PrintArray(FILE *fp, int n, int *array)
{
    int i;
    if(!n)
	return;
    for(i=0; i<n; i++)
	fprintf(fp, "%d ", array[i]);
    fprintf(fp, "\n");
}

double IntPow(double a, int n)
{
    double result;
    if(n == 1)
	return a;
    if(n == 0)
	return a == 0.0 ? 0.0/0.0 : 1;
    if(n < 0)
	return 1/IntPow(a,-n);

    result = IntPow(a, n/2);
    if(n & 1)
	return a * result * result;
    else
	return result * result;
}


int Log2(int n)
{
    int log2 = 0;
    assert(n != 0);
    while((n /= 2))
	++log2;
    return log2;
}

