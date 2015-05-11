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
} multimap_node;


/* The entry-point of the multimap data structure. */
struct multimap {
    /* We use an array to store the binary tree of nodes. */
    multimap_node **tree_list;

    /* The current size of the tree array. */
    int curr_size;

    /* The max size of the tree array. */
    int max_size;
};


/*============================================================================
 * HELPER FUNCTION DECLARATIONS
 *
 *   Declarations of helper functions that are local to this module.  Again,
 *   these are not visible outside of this module.
 *============================================================================*/

multimap_node * alloc_mm_node();

multimap_node * find_mm_node(multimap *mm, int key,
                             int create_if_not_found);

void remove_mm_node(multimap *mm, multimap_node *to_remove);
int remove_mm_node_helper(multimap *mm, multimap_node *to_remove, \
        int curr_size);
void free_multimap_node(multimap_node *node);

void resize_values();
void resize_multimap(multimap *mm);
multimap_node *get_min_node(multimap *mm, int i);
multimap_node *get_left_child(multimap *mm, int i);
multimap_node *get_right_child(multimap *mm, int i);
multimap_node *get_parent(multimap *mm, int i);
int get_min_index(multimap *mm, int i);
int get_left_index(int i);
int get_right_index(int i);
int get_parent_index(int i);
void add_node(multimap *mm, multimap_node *node, int index);


/*============================================================================
 * FUNCTION IMPLEMENTATIONS
 *============================================================================*/

/* Allocates a multimap node, and zeros out its contents so that we know what
 * the initial value of everything will be.
 */
multimap_node * alloc_mm_node() {
    multimap_node *node = malloc(sizeof(multimap_node));
    bzero(node, sizeof(multimap_node));
    /*printf("initially, curr_size = %d and max_size = %d\n", node->curr_size, node->max_size);*/

    return node;
}


/* This helper function searches for the multimap node that contains the
 * specified key. If such a node doesn't exist, the function can initialize
 * a new node and add this into the structure, or it will simply return NULL.
 * The one exception is the root - if the root is NULL then the function will
 * return a new root node.
 */
multimap_node * find_mm_node(multimap *mm, int key,
                             int create_if_not_found) {
    multimap_node **tree_list = mm->tree_list;
    multimap_node *node;
    int curr_index = 0;
    int left_index;
    int right_index;

    /* If the entire multimap is empty, the root will be NULL. */
    if (mm->curr_size == 0) {
        multimap_node *root;
        if (create_if_not_found) {
            root = alloc_mm_node();
            root->key = key;
        }
        return root;
    }

    /* Now we know the multimap has at least a root node, so start there. */
    node = tree_list[curr_index];
    while (1) {
        if (node->key == key)
            break;

        if (node->key > key) {    /* Follow left child */
            node = get_left_child(mm, curr_index);
            if (node == NULL && create_if_not_found) {
                /* No left child, but caller wants us to create a new node. */
                multimap_node *new = alloc_mm_node();
                new->key = key;
                add_node(mm, new, left_index);
            }
            curr_index = left_index;
        } else {                  /* Follow right child */
            node = get_right_child(mm, curr_index);
            if (node == NULL && create_if_not_found) {
                /* No right child, but caller wants us to create a new node. */
                multimap_node *new = alloc_mm_node();
                new->key = key;
                add_node(mm, new, right_index);
            }
            curr_index = right_index;
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

    int left_index, right_index;

    multimap_node **tree_list = mm->tree_list;
    if (mm->curr_size > 0 && tree_list[0] == to_remove) {
        /* The root of the multimap is the node being removed. */

        multimap_node *left, *right;

        right_index = get_right_index(0);
        left = get_left_child(mm, 0);
        right = get_right_child(mm, 0);

        /* If there is a right child, replace the root with the minimum-valued
         * node in the right subtree (and delete that node).
         * Otherwise, if there is no right child, the left child is promoted.
         */
        if (right != NULL) {
            int min_index = get_min_index(mm, right_index);
            // Replace root
            tree_list[0] = tree_list[min_index];
            // Delete min node
            free_multimap_node(tree_list[min_index]);
            tree_list[min_index] = NULL;
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
        int found = 5;
#endif
        assert(found);
    }

    /* Presumably, the node to remove has been found, so extract it from the
     * tree and call the free-node helper.
     */
    to_remove->left_child = NULL;
    to_remove->right_child = NULL;
    free_multimap_node(to_remove);
}


int remove_mm_node_helper(multimap *mm, multimap_node *to_remove,\
        int curr_index) {
    multimap_node *curr_node = mm->tree_list[curr_index];
    multimap_node *left_child = get_left_child(mm, curr_index);
    multimap_node *right_child = get_right_child(mm, curr_index);
    int left_index = get_left_index(curr_index);
    int right_index = get_right_index(curr_index);

    if (to_remove->key < curr_node->key) {
        if (left_child != NULL)
            return remove_mm_node_helper(mm, to_remove, left_index);
        else
            return 0;
    } else if (to_remove->key > curr_node->key) {
        if (right_child != NULL)
            return remove_mm_node_helper(mm, to_remove, right_index);
        else
            return 0;
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
    free_multimap_node(mm->root);
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

/* Resizes the tree_list of the passed-in multimap.
 */
void resize_multimap(multimap *mm) {
    int i;
    int old_max_size = mm->max_size;
    if (mm->max_size == 0)
        mm->max_size = 2;
    else
        mm->max_size *= 2;
    multimap_node **old = mm->tree_list;
    multimap_node **new = calloc(mm->max_size, sizeof(multimap *));
    /* Copy over old values */
    for (i = 0; i < old_max_size; ++i) {
        new[i] = old[i];
    }
    /* Free old array b/c we are no longer using it */
    free(mm->tree_list);
    mm->tree_list = new;
}

/* Adds a node into the multimap add the specificed index.
 */
void add_node(multimap *mm, multimap_node *node, int index) {
    /* Make sure the multimap is large enough so that we can actually add the
     * node
     */
    while (index > mm->max_size)
        resize_multimap(mm);
    multimap_node **tree_list = mm->tree_list;
    // Add the node
    tree_list[index] = node;
    // Make sure curr_size stays up to date
    if (index >= mm->curr_size)
        mm->curr_size = index + 1;
}

multimap_node *get_min_node(multimap *mm, int i) {
    int min_index = get_min_index(mm, i);
    return mm->tree_list[min_index];
}

multimap_node *get_left_child(multimap *mm, int i) {
    int left_index = get_left_index(i);
    if (left_index >= mm->curr_size || left_index < 0)
        return NULL;
    else
        return mm->tree_list[left_index];
}

multimap_node *get_right_child(multimap *mm, int i) {
    int right_index = get_right_index(i);
    if (right_index >= mm->curr_size || right_index < 0)
        return NULL;
    else
        return mm->tree_list[right_index];
}

multimap_node *get_parent(multimap *mm, int i) {
    int parent_index = get_parent_index(i);
    if (parent_index >= mm->curr_size || parent_index < 0)
        return NULL;
    else
        return mm->tree_list[parent_index];
}

int get_min_index(multimap *mm, int i) {
    // Keep looking at the left child
    while (get_left_child(mm, i) != NULL) {
        i = get_left_index(i);
    }
    return i;
}

/* Gets index of left child (for binary tree array).
 */
int get_left_index(int i) {
    return (2 * i + 1);
}

/* Gets index of right child (for binary tree array).
 */
int get_right_index(int i) {
    return (2 * i + 2);
}

/* Gets index of parent node (for binary tree array).
 */
int get_parent_index(int i) {
    return (i - 1) / 2;
}
