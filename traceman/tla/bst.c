/*** File bst.c - Binary Search Tree ***/
/*
 *   Shane Saunders
 */
#include <stdlib.h>
#include "bst.h"


/* bst_alloc() - Allocates space for a binary search tree and returns a pointer
 * to it.  The function compar compares they keys of two items, and returns a
 * negative, zero, or positive integer depending on whether the first item is
 * less than, equal to, or greater than the second.
 */
bst_t *bst_alloc(int (* compar)(const void *, const void *))
{
    bst_t *t;


    t = malloc(sizeof(bst_t));
    t->root = NULL;
    t->compar = compar;
    t->n = 0;

    return t;
}


/* bst_free() - Frees space used by the binary search tree pointed to by t. */
void bst_free(bst_t *t)
{
    bst_node_t *p, **stack;
    int tos;

    /* In order to free all nodes in the tree a depth first search is performed
     * This is implemented using a stack.
     */
    
    if(t->root) {
        stack = malloc(t->n * sizeof(bst_node_t *));
	stack[0] = t->root;
        tos = 1;
	while(tos) {
	    p = stack[--tos];
	    if(p->left) {
		stack[tos++] = p->left;
	    }
	    if(p->right) {
		stack[tos++] = p->right;
	    }
	    free(p);
	}
	free(stack);
    }
    
    free(t);
}


/* bst_insert() - Inserts an item into the binary search tree pointed to by t,
 * according the the value its key.  The key of an item in the binary search
 * tree must be unique among items in the tree.  If an item with the same key
 * already exists in the tree, a pointer to that item is returned.  Otherwise,
 * NULL is returned, indicating insertion was successful.
 */
void *bst_insert(bst_t *t, void *item)
{
    bst_node_t *x, *p, *next_p, **attach_x;
    int (* compar)(const void *, const void *);
    int cmp_result;

    
    if((next_p = t->root)) {
        compar = t->compar;
	
	/* Repeatedly explore either the left branch or the right branch
	 * depending on the value of the key, until an empty branch is chosen.
	 */
        for(;;) {
	    p = next_p;
	    cmp_result = compar(item, p->item);
	    if(cmp_result < 0) {
                next_p = p->left;
		if(!next_p) {
		    attach_x = &p->left;
		    break;
		}
	    }
	    else if(cmp_result > 0) {
		next_p = p->right;
		if(!next_p) {
		    attach_x = &p->right;
		    break;
		}
	    }
	    else {
	        return p->item;
	    }
	}
    }
    else {
	attach_x = &t->root;
    }

    x = malloc(sizeof(bst_node_t));
    x->left = x->right = NULL;
    x->item = item;

    *attach_x = x;
    t->n++;
    
    return NULL;
}


/* bst_find() - Find an item in the binary search tree with the same key as the
 * item pointed to by `key_item'.  Returns a pointer to the item found, or NULL
 * if no item was found.
 */
void *bst_find(bst_t *t, void *key_item)
{
    bst_node_t *p, *next_p;
    int (* compar)(const void *, const void *);
    int cmp_result;

	
    if((next_p = t->root)) {
        compar = t->compar;
	
	/* Repeatedly explore either the left or right branch, depending on the
	 * value of the key, until the correct item is found.
	 */
        do {
	    p = next_p;
	    cmp_result = compar(key_item, p->item);
	    if(cmp_result < 0) {
                next_p = p->left;
	    }
	    else if(cmp_result > 0) {
		next_p = p->right;
	    }
	    else {
		/* Item found. */
                return p->item;
	    }
	} while(next_p);
    }

    return NULL;
}


/* bst_find_min() - Returns a pointer to the minimum item in the binary search
 * tree pointed to by t.  If there are no items in the tree a NULL pointer is
 * returned.
 */
void *bst_find_min(bst_t *t)
{
    bst_node_t *p, *next_p;


    if((next_p = t->root)) {
        /* To locate the minimum, the left branches is repeatedly chosen until
	 * we can explore no further.
         */
        do {
	    p = next_p;
            next_p = p->left;
	} while(next_p);
    }
    else {
	return NULL;
    }
    
    return p->item;
}


/* bst_delete() - Delete an item in the binary search tree with the same key as
 * the item pointed to by `key_item'.  Returns a pointer to the deleted item,
 * and NULL if no item was found.
 */
void *bst_delete(bst_t *t, void *key_item)
{
    bst_node_t *p, *next_p, *prev_p;
    bst_node_t *m, *next_m, *prev_m;
    void *return_item;
    int (* compar)(const void *, const void *);
    int cmp_result;


    /* Attempt to locate the item to be deleted. */
    if((next_p = t->root)) {
        compar = t->compar;
        p = NULL;
        for(;;) {
	    prev_p = p;
	    p = next_p;
	    cmp_result = compar(key_item, p->item);
	    if(cmp_result < 0) {
                next_p = p->left;
	    }
	    else if(cmp_result > 0) {
		next_p = p->right;
	    }
	    else {
		/* Item found. */
                break;
	    }
            if(!next_p) return NULL;
	}
    }
    else {
        return NULL;
    }

    /* p points to the node to be deleted. */
    if(!p->left) {
	/* Right child replaces p. */
	if(!prev_p) {
            t->root = p->right;
	}
	else if(p == prev_p->left) {
	    prev_p->left = p->right;
	}
	else {
	    prev_p->right = p->right;
	}
    }
    else if(!p->right) {
	/* Left child replaces p. */
	if(!prev_p) {
            t->root = p->left;
	}
	else if(p == prev_p->left) {
	    prev_p->left = p->left;
	}
	else {
	    prev_p->right = p->left;
	}
    }
    else {
        /* Minimum child, m, in the right subtree replaces p. */
	m = p;
	next_m = p->right;
        do {
	    prev_m = m;
	    m = next_m;
            next_m = m->left;
	} while(next_m);

	/* Update either the left or right child pointers of prev_p. */
	if(!prev_p) {
            t->root = m;
	}
        else if(p == prev_p->left) {
	    prev_p->left = m;
	}
	else {
	    prev_p->right = m;
        }

	/* Update the tree part m was removed from, and assign m the child
	 * pointers of p (only if m is not the right child of p).
	 */
        if(prev_m != p) {
	    prev_m->left = m->right;
	    m->right = p->right;
	}
        m->left = p->left;
    }

    /* Get return value and free space used by node p. */
    return_item = p->item;
    free(p);

    t->n--;
    
    return return_item;
}


/* bst_delete_min() - Deletes the item with the smallest key from the binary
 * search tree pointed to by t.  Returns a pointer to the deleted item.
 * Returns a NULL pointer if there are no items in the tree.
 */
void *bst_delete_min(bst_t *t)
{
    bst_node_t *p, *next_p, *prev_p;
    void *return_item;

    /* Attempt to locate the item to be deleted. */
    if((next_p = t->root)) {
	p = NULL;
        /* To locate the minimum, the left branches is repeatedly chosen until
	 * we can explore no further.
         */
        do {
	    prev_p = p;
	    p = next_p;
            next_p = p->left;
	} while(next_p);
    }
    else {
	return NULL;
    }

    /* Right child replaces p. */
    if(!prev_p) {
        t->root = p->right;
    }
    else {
        prev_p->left = p->right;
    }

    /* Get return value and free space used by node p. */
    return_item = p->item;
    free(p);

    t->n--;
    
    return return_item;
}


/*** Implement the universal dictionary structure type ***/

/*** Binary search tree wrapper functions. ***/

void *_bst_alloc(int (* compar)(const void *, const void *),
		 unsigned int (* getval)(const void *)) {
    return bst_alloc(compar);
}

void _bst_free(void *t) {
    bst_free((bst_t *)t);
}

void *_bst_insert(void *t, void *item) {
    return bst_insert((bst_t *)t, item);
}

void *_bst_delete(void *t, void *key_item) {
    return bst_delete((bst_t *)t, key_item);
}

void *_bst_delete_min(void *t) {
    return bst_delete_min((bst_t *)t);
}

void *_bst_find(void *t, void *key_item) {
    return bst_find((bst_t *)t, key_item);
}

void *_bst_find_min(void *t) {
    return bst_find_min((bst_t *)t);
}

/* Binary search tree info. */
const dict_info_t BST_info = {
    _bst_alloc,
    _bst_free,
    _bst_insert,
    _bst_delete,
    _bst_delete_min,
    _bst_find,
    _bst_find_min
};
