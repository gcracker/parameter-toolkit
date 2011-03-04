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


#ifndef DICT_INFO_H
#define DICT_INFO_H
/*** dict_info.h - Defines the universal dict_info_t data structure which each
 *** dictionary can provide, and algorithms can use.  An algorithm which uses
 *** the universal definition of a dictionary data structure can use different
 *** dictionaries interchangeably for testing or comparison purposes.
 ***/

/* Structure that is provided to algorithm which uses a dictionary. */
typedef struct dict_info {
    void *(*alloc)(int (* compar)(const void *, const void *),
		   unsigned int (* getval)(const void *));
    void (*free)(void *t);
    void *(*insert)(void *t, void *item);
    void *(*delete)(void *t, void *key_item);
    void *(*delete_min)(void *t);
    void *(*find)(void *t, void *key_item);
    void *(*find_min)(void *t);
} dict_info_t;

/* A pointer to a compar() function, which compares two items in the
 * dictionary, is normally supplied as an argument to a dictionaries alloc()
 * function.  However, some dictionaries instead use a getval() function, which
 * evaluates an item in the dictionary to an integer.  To accommodate this, the
 * universal alloc() function in the dict_info structure has two parameters:
 * One for compar() and one for getval().
 *
 * Either the compar() or getval() parameter will be passed as an argument to
 * the actual alloc() function of the dictionary, while the other parameter
 * will be ignored.
 *
 * If an algorithm will only ever use comparison based dictionaries then the a
 * NULL pointer argument can be passed for the getval() parameter when the
 * algorithm calls the universal alloc() function.
 */

#endif


