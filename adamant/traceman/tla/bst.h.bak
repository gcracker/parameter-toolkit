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
