#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "multimap.h"

/*============================================================================
 * TYPES
 *
 *   These types are defined in the implementation file so that they can
 *   be kept hidden to code outside this source file.  This is not for any
 *   security reason, but rather just so we can enforce that our testing
 *   programs are generic and don't have any access to implementation details.
 *============================================================================*/

/* Represents a key and its associated values in the multimap, as well as
 * pointers to the left and right child nodes in the multimap. */
typedef struct multimap_node {
    /* The key-value that this multimap node represents. */
    int key;

    /* A linked list of the values associated with this key in the multimap. */

    /* An array of the values associated with this key in the multimap. */
    int *values;

    /* The current size of the values array. */
    unsigned short curr_size;

    /* The max size of the values array. */
    unsigned short max_size;

    /* The left child of the multimap node.  This will reference nodes that
     * hold keys that are strictly less than this node's key.
     */
    struct multimap_node *left_child;

    /* The right child of the multimap node.  This will reference nodes that
     * hold keys that are strictly greater than this node's key.
     */
    struct multimap_node *right_child;
} multimap_node;


/* The entry-point of the multimap data structure. */
struct multimap {
    multimap_node *root;
};

/* Variables to be used for slab allocation */
void *memory_bank = NULL;
int memory_bank_num_nodes = 0;
int memory_bank_max_size = 0;


/*============================================================================
 * HELPER FUNCTION DECLARATIONS
 *
 *   Declarations of helper functions that are local to this module.  Again,
 *   these are not visible outside of this module.
 *============================================================================*/

multimap_node * alloc_mm_node();

multimap_node * find_mm_node(multimap_node *root, int key,
                             int create_if_not_found);

void remove_mm_node(multimap *mm, multimap_node *to_remove);
int remove_mm_node_helper(multimap_node *node, multimap_node *to_remove);

void free_multimap_node(multimap_node *node);
void resize_values();
void resize_memory_bank();


/*============================================================================
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/* Allocates a multimap node, and zeros out its contents so that we know what
 * the initial value of everything will be.
 */
multimap_node * alloc_mm_node() {
    if (memory_bank_num_nodes * sizeof(multimap_node) >= memory_bank_max_size)
        resize_memory_bank();
    multimap_node *node = memory_bank + memory_bank_num_nodes * sizeof(multimap_node);
    memory_bank_num_nodes++;
    bzero(node, sizeof(multimap_node));
    /*printf("initially, curr_size = %d and max_size = %d\n", node->curr_size, node->max_size);*/

    return node;
}


/* This helper function searches for the multimap node that contains the
 * specified key.  If such a node doesn't exist, the function can initialize
 * a new node and add this into the structure, or it will simply return NULL.
 * The one exception is the root - if the root is NULL then the function will
 * return a new root node.
 */
multimap_node * find_mm_node(multimap_node *root, int key,
                             int create_if_not_found) {
    multimap_node *node;

    /* If the entire multimap is empty, the root will be NULL. */
    if (root == NULL) {
        if (create_if_not_found) {
            root = alloc_mm_node();
            root->key = key;
        }
        return root;
    }

    /* Now we know the multimap has at least a root node, so start there. */
    node = root;
    while (1) {
        if (node->key == key)
            break;

        if (node->key > key) {   /* Follow left child */
            if (node->left_child == NULL && create_if_not_found) {
                /* No left child, but caller wants us to create a new node. */
                multimap_node *new = alloc_mm_node();
                new->key = key;

                node->left_child = new;
            }
            node = node->left_child;
        }
        else {                   /* Follow right child */
            if (node->right_child == NULL && create_if_not_found) {
                /* No right child, but caller wants us to create a new node. */
                multimap_node *new = alloc_mm_node();
                new->key = key;

                node->right_child = new;
            }
            node = node->right_child;
        }

        if (node == NULL)
            break;
    }

    return node;
}


/* Removes the specified multimap node from the multimap.  The root is checked
 * separately so that the multimap structure itself can be updated; otherwise,
 * the multimap is recursively scanned to make it easier.
 */
void remove_mm_node(multimap *mm, multimap_node *to_remove) {
    assert(mm != NULL);
    assert(to_remove != NULL);

    if (mm->root == to_remove) {
        /* The root of the multimap is the node being removed. */

        multimap_node *left, *right;

        left = to_remove->left_child;
        right = to_remove->right_child;

        /* If there is a right child, it must take the place of the node being
         * removed; the left child becomes the left child of the promoted node.
         * Otherwise, if there is no right child, the left child is promoted.
         */
        if (right != NULL) {
            mm->root = right;
            right->left_child = left;
        }
        else {
            mm->root = left;
        }
    }
    else {
        /* The node being removed is not the root, so recursively scan
         * the tree.
         */
#ifndef NDEBUG
        /* Wrap this in an #ifndef since the compiler will complain that the
         * variable is unused otherwise.
         */
        int found =
#endif
            remove_mm_node_helper(mm->root, to_remove);
        assert(found);
    }

    /* Presumably, the node to remove has been found, so extract it from the
     * tree and call the free-node helper.
     */
    to_remove->left_child = NULL;
    to_remove->right_child = NULL;
    free_multimap_node(to_remove);
}


/* This helper function recursively scans the multimap tree to identify the
 * node being removed, and update the tree structure to maintain the ordering
 * property of the structure.  (It does not try to keep the tree balanced.)
 *
 * The helper also reports whether or not the node was found in the subtree
 * rooted at node, so that we can minimize the amount of the tree that is
 * scanned.
 */
int remove_mm_node_helper(multimap_node *node, multimap_node *to_remove) {
    multimap_node **to_change;
    multimap_node *left, *right;

    if (node == NULL)
        return 0;

    assert(node != NULL);
    assert(to_remove != NULL);

    /* See if the node to remove is a child of this node.  If so, figure out
     * if it's the left or right child, and take it from there.
     */
    to_change = NULL;
    if (node->left_child == to_remove)
        to_change = &(node->left_child);
    else if (node->right_child == to_remove)
        to_change = &(node->right_child);

    if (to_change != NULL) {
        /* If there is a right child, it must take the place of the node being
         * removed; the left child becomes the left child of the promoted node.
         * Otherwise, if there is no right child, the left child is promoted.
         */

        left = to_remove->left_child;
        right = to_remove->right_child;

        if (right != NULL) {
            *to_change = right;
            right->left_child = left;
        }
        else {
            *to_change = left;
        }

        /* Found it! */
        return 1;
    }
    else {
        /* This node doesn't have the node-to-remove as a child, so recursively
         * descend into the part of the tree that will have the node to remove.
         */

        multimap_node *child;
        assert(to_remove->key != node->key);

        if (to_remove->key < node->key)
            child = node->left_child;
        else
            child = node->right_child;

        return remove_mm_node_helper(child, to_remove);
    }
}


/* This helper function frees a multimap node, including its children and
 * value-list.
 */
void free_multimap_node(multimap_node *node) {
    if (node == NULL)
        return;

    /* Free the children first. */
    free_multimap_node(node->left_child);
    free_multimap_node(node->right_child);

    /* Free the array of values. */
    free(node->values);

#ifdef DEBUG_ZERO
    /* Clear out what we are about to free, to expose issues quickly. */
    bzero(node, sizeof(multimap_node));
#endif
    free(node);
}


/* Initialize a multimap data structure. */
multimap * init_multimap() {
    multimap *mm = malloc(sizeof(multimap));
    mm->root = NULL;
    return mm;
}


/* Release all dynamically allocated memory associated with the multimap
 * data structure.
 */
void clear_multimap(multimap *mm) {
    assert(mm != NULL);

    /* Free the memory bank, reset information variables */
    free(memory_bank);
    memory_bank = NULL;
    memory_bank_max_size = 0;
    memory_bank_num_nodes = 0;

    mm->root = NULL;
}


/* Adds the specified (key, value) pair to the multimap. */
void mm_add_value(multimap *mm, int key, int value) {
    multimap_node *node;

    assert(mm != NULL);

    /* Look up the node with the specified key.  Create if not found. */
    node = find_mm_node(mm->root, key, /* create */ 1);
    if (mm->root == NULL)
        mm->root = node;

    assert(node != NULL);
    assert(node->key == key);

    /* If the node's value array is too small, resize it */
    if (node->curr_size >= node->max_size) {
        resize_values(node);
    }

    /* Add the new value to the multimap node. */
    node->values[node->curr_size] = value;
    node->curr_size++;
}


/* Returns nonzero if the multimap contains the specified key-value, zero
 * otherwise.
 */
int mm_contains_key(multimap *mm, int key) {
    return find_mm_node(mm->root, key, /* create */ 0) != NULL;
}


/* Returns nonzero if the multimap contains the specified (key, value) pair,
 * zero otherwise.
 */
int mm_contains_pair(multimap *mm, int key, int value) {
    multimap_node *node;
    int i;

    node = find_mm_node(mm->root, key, /* create */ 0);
    if (node == NULL)
        return 0;

    // Search through values
    for (i = 0; i < node->curr_size; i++) {
        if (node->values[i] == value)
            return 1;
    }

    return 0;
}


/* Removes the specified (key, value) pair from the multimap.  Returns 1 if
 * the specified pair was found, or 0 if the pair was not found.
 */
int mm_remove_pair(multimap *mm, int key, int value) {
    multimap_node *node;
    int found = 0;
    int i;
    int index;

    assert(mm != NULL);

    /* Look up the node with the specified key.  DO NOT create if not found. */
    node = find_mm_node(mm->root, key, /* create */ 0);
    if (node == NULL)
        return 0;      /* Pair already doesn't appear in the multiset. */

    /* If we got here, we found a node with the specified key.  Now we need to
     * remove one instance of the specified value from the node's value-list.
     */

    assert(node->key == key);

    /* Find the index of the value to delete */
    for (i = 0; i < node->curr_size; i++) {
        if (node->values[i] == value) {
            found = 1;
            index = i;
        }
    }

    /* Delete the value, adjust curr_size */
    if (found) {
        for (i = index; i < node->curr_size - 1; i++) {
            node->values[i] = node->values[i + 1];
        }
        node->curr_size--;
    }

    return found;
}


/* This helper function is used by mm_traverse() to traverse every pair within
 * the multimap.
 */
void mm_traverse_helper(multimap_node *node, void (*f)(int key, int value)) {
    int i;

    if (node == NULL)
        return;

    mm_traverse_helper(node->left_child, f);

    for (i = 0; i < node->curr_size; i++) {
        f(node->key, node->values[i]);
    }

    mm_traverse_helper(node->right_child, f);
}


/* Performs an in-order traversal of the multimap, passing each (key, value)
 * pair to the specified function.
 */
void mm_traverse(multimap *mm, void (*f)(int key, int value)) {
    mm_traverse_helper(mm->root, f);
}

 /* Resize the values array to be double the current max_size. Or, if max_size
  * is currently 0, just sets max_size = 10.
 */
void resize_values(multimap_node *node) {
    int i;
    unsigned short old_max_size = node->max_size;
    if (node->max_size == 0)
        node->max_size = 2;
    else
        node->max_size *= 2;
    int *old = node->values;
    int *new = malloc(node->max_size * sizeof(int));
    /* Copy over old values */
    for (i = 0; i < old_max_size; ++i) {
        new[i] = old[i];
    }
    /* Free old array b/c we are no longer using it */
    free(node->values);
    node->values = new;
}

/* Resizes the memory bank to be double the current max size. Or, if max size
 * is currently 0, just sets max size to be 100000 * sizeof(multimap_node)
 */
void resize_memory_bank() {
    if (memory_bank_max_size == 0)
        memory_bank_max_size = 100000 * sizeof(multimap_node);
    else
        memory_bank_max_size *= 2;
    memory_bank = realloc(memory_bank, memory_bank_max_size);
}
