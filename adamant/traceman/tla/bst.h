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


/*** File bst.h - Binary Search Tree ***/
/*
 *   Shane Saunders
 */
#ifndef BST_H
#define BST_H
#include "dict_info.h"  /* Defines the universal dictionary structure type. */


#define BST_STACK_SIZE 1000


/* Structure type for nodes in the binary search tree. */
typedef struct bst_node {
    void *item;
    struct bst_node *left, *right;
} bst_node_t;

/* Structure type for the binary search tree. */
typedef struct bst {
    bst_node_t *root;
    int n;
    int (* compar)(const void *, const void *);
} bst_t;


/* bst_alloc() - Allocates space for a binary search tree and returns a pointer
 * to it.  The function compar compares they keys of two items, and returns a
 * negative, zero, or positive integer depending on whether the first item is
 * less than, equal to, or greater than the second.
 */
bst_t *bst_alloc(int (* compar)(const void *, const void *));

/* bst_free() - Frees space used by the binary search tree pointed to by t. */
void bst_free(bst_t *t);

/* bst_insert() - Inserts an item into the binary search tree pointed to by t,
 * according the the value its key.  The key of an item in the binary search
 * tree must be unique among items in the tree.  If an item with the same key
 * already exists in the tree, a pointer to that item is returned.  Otherwise,
 * NULL is returned, indicating insertion was successful.
 */
void *bst_insert(bst_t *t, void *item);

/* bst_find() - Find an item in the binary search tree with the same key as the
 * item pointed to by `key_item'.  Returns a pointer to the item found, or NULL
 * if no item was found.
 */
void *bst_find(bst_t *t, void *key_item);

/* bst_find_min() - Returns a pointer to the minimum item in the binary search
 * tree pointed to by t.  If there are no items in the tree a NULL pointer is
 * returned.
 */
void *bst_find_min(bst_t *t);

/* bst_delete() - Delete an item in the binary search tree with the same key as
 * the item pointed to by `key_item'.  Returns a pointer to the  deleted item,
 * and NULL if no item was found.
 */
void *bst_delete(bst_t *t, void *key_item);

/* bst_delete_min() - Deletes the item with the smallest key from the binary
 * search tree pointed to by t.  Returns a pointer to the deleted item.
 * Returns a NULL pointer if there are no items in the tree.
 */
void *bst_delete_min(bst_t *t);

/*** Alternative interface via the universal dictionary structure type. ***/
extern const dict_info_t BST_info;

#endif


