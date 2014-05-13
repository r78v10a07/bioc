/* 
 * File:   btree.c
 * Author: roberto
 *
 * Created on May 7, 2014, 9:55 AM
 */

/**
 *    This code was taken and modified from:
 * 
 *    http://www.amittai.com/prose/bplustree.html
 * 
 *    Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *    Senior Software Engineer
 *    MathWorks, Inc.
 *    3 Apple Hill Drive
 *    Natick, MA 01760 
 *  Original Date:  26 June 2010
 *  Last modified: 15 April 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "btree.h"
#include "berror.h"
#include "bmemory.h"

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
int order = DEFAULT_ORDER;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
BtreeNode_t * queue = NULL;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
bool verbose_output = false;

BtreeNode_t * insert_into_parent(BtreeNode_t * root, BtreeNode_t * left, int key, BtreeNode_t * right);

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut(int length) {
    if (length % 2 == 0)
        return length / 2;
    else
        return length / 2 + 1;
}

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
BtreeNode_t * find_leaf(BtreeNode_t * root, int key, bool verbose) {
    int i = 0;
    BtreeNode_t * c = root;
    if (c == NULL) {
        if (verbose)
            printf("Empty tree.\n");
        return c;
    }
    while (!c->is_leaf) {
        if (verbose) {
            printf("[");
            for (i = 0; i < c->num_keys - 1; i++)
                printf("%d ", c->keys[i]);
            printf("%d] ", c->keys[i]);
        }
        i = 0;
        while (i < c->num_keys) {
            if (key >= c->keys[i]) i++;
            else break;
        }
        if (verbose)
            printf("%d ->\n", i);
        c = (BtreeNode_t *) c->pointers[i];
    }
    if (verbose) {
        printf("Leaf [");
        for (i = 0; i < c->num_keys - 1; i++)
            printf("%d ", c->keys[i]);
        printf("%d] ->\n", c->keys[i]);
    }
    return c;
}

/**
 * Finds and returns the record to which a key refers.
 * 
 * @param root the root node
 * @param key the key of the object to find
 * @param verbose true to print info
 * @return 
 */
BtreeRecord_t * BTreeFind(BtreeNode_t * root, int key, bool verbose) {
    int i = 0;
    BtreeNode_t * c = find_leaf(root, key, verbose);
    if (c == NULL) return NULL;
    for (i = 0; i < c->num_keys; i++)
        if (c->keys[i] == key) break;
    if (i == c->num_keys)
        return NULL;
    else
        return (BtreeRecord_t *) c->pointers[i];
}

/* Creates a new record to hold the value
 * to which a key refers.
 */
BtreeRecord_t * make_record(void *value) {
    BtreeRecord_t * new_record = (BtreeRecord_t *) allocate(sizeof (BtreeRecord_t), __FILE__, __LINE__);
    new_record->value = value;
    return new_record;
}

/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
BtreeNode_t * make_node(void) {
    BtreeNode_t * new_node;
    new_node = allocate(sizeof (BtreeNode_t), __FILE__, __LINE__);
    new_node->keys = allocate((order - 1) * sizeof (int), __FILE__, __LINE__);    
    new_node->pointers = allocate(order * sizeof (void *), __FILE__, __LINE__);
    
    new_node->is_leaf = false;
    new_node->num_keys = 0;
    new_node->parent = NULL;
    new_node->next = NULL;
    return new_node;
}

/* Creates a new leaf by creating a node
 * and then adapting it appropriately.
 */
BtreeNode_t * make_leaf(void) {
    BtreeNode_t * leaf = make_node();
    leaf->is_leaf = true;
    return leaf;
}

/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index(BtreeNode_t * parent, BtreeNode_t * left) {
    int left_index = 0;
    while (left_index <= parent->num_keys && parent->pointers[left_index] != left)
        left_index++;
    return left_index;
}

/* First insertion:
 * start a new tree.
 */
BtreeNode_t * start_new_tree(int key, BtreeRecord_t * pointer) {
    BtreeNode_t * root = make_leaf();
    root->keys[0] = key;
    root->pointers[0] = pointer;
    root->pointers[order - 1] = NULL;
    root->parent = NULL;
    root->num_keys++;
    return root;
}

/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
BtreeNode_t * insert_into_leaf(BtreeNode_t * leaf, int key, BtreeRecord_t * pointer) {
    int i, insertion_point;

    insertion_point = 0;
    while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] < key)
        insertion_point++;

    for (i = leaf->num_keys; i > insertion_point; i--) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->pointers[i] = leaf->pointers[i - 1];
    }
    leaf->keys[insertion_point] = key;
    leaf->pointers[insertion_point] = pointer;
    leaf->num_keys++;
    return leaf;
}

/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
BtreeNode_t * insert_into_new_root(BtreeNode_t * left, int key, BtreeNode_t * right) {
    BtreeNode_t * root = make_node();
    root->keys[0] = key;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->num_keys++;
    root->parent = NULL;
    left->parent = root;
    right->parent = root;
    return root;
}

/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
BtreeNode_t * insert_into_node(BtreeNode_t * root, BtreeNode_t * n,
        int left_index, int key, BtreeNode_t * right) {
    int i;

    for (i = n->num_keys; i > left_index; i--) {
        n->pointers[i + 1] = n->pointers[i];
        n->keys[i] = n->keys[i - 1];
    }
    n->pointers[left_index + 1] = right;
    n->keys[left_index] = key;
    n->num_keys++;
    return root;
}

/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
BtreeNode_t * insert_into_node_after_splitting(BtreeNode_t * root, BtreeNode_t * old_node, int left_index,
        int key, BtreeNode_t * right) {

    int i, j, split, k_prime;
    BtreeNode_t * new_node, * child;
    int * temp_keys;
    BtreeNode_t ** temp_pointers;

    /* First create a temporary set of keys and pointers
     * to hold everything in order, including
     * the new key and pointer, inserted in their
     * correct places. 
     * Then create a new node and copy half of the 
     * keys and pointers to the old node and
     * the other half to the new.
     */

    temp_pointers = allocate((order + 1) * sizeof (BtreeNode_t *), __FILE__, __LINE__);
    temp_keys = allocate(order * sizeof (int), __FILE__, __LINE__);

    for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
        if (j == left_index + 1) j++;
        temp_pointers[j] = old_node->pointers[i];
    }

    for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
        if (j == left_index) j++;
        temp_keys[j] = old_node->keys[i];
    }

    temp_pointers[left_index + 1] = right;
    temp_keys[left_index] = key;

    /* Create the new node and copy
     * half the keys and pointers to the
     * old and half to the new.
     */
    split = cut(order);
    new_node = make_node();
    old_node->num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old_node->pointers[i] = temp_pointers[i];
        old_node->keys[i] = temp_keys[i];
        old_node->num_keys++;
    }
    old_node->pointers[i] = temp_pointers[i];
    k_prime = temp_keys[split - 1];
    for (++i, j = 0; i < order; i++, j++) {
        new_node->pointers[j] = temp_pointers[i];
        new_node->keys[j] = temp_keys[i];
        new_node->num_keys++;
    }
    new_node->pointers[j] = temp_pointers[i];
    free(temp_pointers);
    free(temp_keys);
    new_node->parent = old_node->parent;
    for (i = 0; i <= new_node->num_keys; i++) {
        child = new_node->pointers[i];
        child->parent = new_node;
    }

    /* Insert a new key into the parent of the two
     * nodes resulting from the split, with
     * the old node to the left and the new to the right.
     */

    return insert_into_parent(root, old_node, k_prime, new_node);
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
BtreeNode_t * insert_into_parent(BtreeNode_t * root, BtreeNode_t * left, int key, BtreeNode_t * right) {

    int left_index;
    BtreeNode_t * parent;

    parent = left->parent;

    /* Case: new root. */

    if (parent == NULL)
        return insert_into_new_root(left, key, right);

    /* Case: leaf or node. (Remainder of
     * function body.)  
     */

    /* Find the parent's pointer to the left 
     * node.
     */

    left_index = get_left_index(parent, left);


    /* Simple case: the new key fits into the node. 
     */

    if (parent->num_keys < order - 1)
        return insert_into_node(root, parent, left_index, key, right);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    return insert_into_node_after_splitting(root, parent, left_index, key, right);
}

/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
BtreeNode_t * insert_into_leaf_after_splitting(BtreeNode_t * root, BtreeNode_t * leaf, int key, BtreeRecord_t * pointer) {

    BtreeNode_t * new_leaf;
    int * temp_keys;
    void ** temp_pointers;
    int insertion_index, split, new_key, i, j;

    new_leaf = make_leaf();

    temp_keys = allocate(order * sizeof (int), __FILE__, __LINE__);
    temp_pointers = allocate(order * sizeof (void *), __FILE__, __LINE__);

    insertion_index = 0;
    while (insertion_index < order - 1 && leaf->keys[insertion_index] < key)
        insertion_index++;

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index) j++;
        temp_keys[j] = leaf->keys[i];
        temp_pointers[j] = leaf->pointers[i];
    }

    temp_keys[insertion_index] = key;
    temp_pointers[insertion_index] = pointer;

    leaf->num_keys = 0;

    split = cut(order - 1);

    for (i = 0; i < split; i++) {
        leaf->pointers[i] = temp_pointers[i];
        leaf->keys[i] = temp_keys[i];
        leaf->num_keys++;
    }

    for (i = split, j = 0; i < order; i++, j++) {
        new_leaf->pointers[j] = temp_pointers[i];
        new_leaf->keys[j] = temp_keys[i];
        new_leaf->num_keys++;
    }

    free(temp_pointers);
    free(temp_keys);

    new_leaf->pointers[order - 1] = leaf->pointers[order - 1];
    leaf->pointers[order - 1] = new_leaf;

    for (i = leaf->num_keys; i < order - 1; i++)
        leaf->pointers[i] = NULL;
    for (i = new_leaf->num_keys; i < order - 1; i++)
        new_leaf->pointers[i] = NULL;

    new_leaf->parent = leaf->parent;
    new_key = new_leaf->keys[0];

    return insert_into_parent(root, leaf, new_key, new_leaf);
}

/**
 * Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 * 
 * @param root the root nodes
 * @param key the key to be used to identify the object
 * @param value the pointer to the object
 * @return the new root node
 */
BtreeNode_t * BtreeInsert(BtreeNode_t * root, int key, void *value) {

    BtreeRecord_t * pointer;
    BtreeNode_t * leaf;

    /* The current implementation ignores
     * duplicates.
     */

    if (BTreeFind(root, key, false) != NULL)
        return root;

    /* Create a new record for the
     * value.
     */
    pointer = make_record(value);


    /* Case: the tree does not exist yet.
     * Start a new tree.
     */

    if (root == NULL)
        return start_new_tree(key, pointer);


    /* Case: the tree already exists.
     * (Rest of function body.)
     */

    leaf = find_leaf(root, key, false);

    /* Case: leaf has room for key and pointer.
     */

    if (leaf->num_keys < order - 1) {
        leaf = insert_into_leaf(leaf, key, pointer);
        return root;
    }


    /* Case:  leaf must be split.
     */

    return insert_into_leaf_after_splitting(root, leaf, key, pointer);
}

/* Helper function for printing the
 * tree out.  See print_tree.
 */
void enqueue(BtreeNode_t * new_node) {
    BtreeNode_t * c;
    if (queue == NULL) {
        queue = new_node;
        queue->next = NULL;
    } else {
        c = queue;
        while (c->next != NULL) {
            c = c->next;
        }
        c->next = new_node;
        new_node->next = NULL;
    }
}

/* Helper function for printing the
 * tree out.  See print_tree.
 */
BtreeNode_t * dequeue(void) {
    BtreeNode_t * n = queue;
    queue = queue->next;
    n->next = NULL;
    return n;
}

/* Utility function to give the length in edges
 * of the path from any node to the root.
 */
int path_to_root(BtreeNode_t * root, BtreeNode_t * child) {
    int length = 0;
    BtreeNode_t * c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}

/**
 * Prints the B+ tree in the command
 * line in level (rank) order, with the 
 * keys in each node and the '|' symbol
 * to separate nodes.
 * With the verbose_output flag set.
 * the values of the pointers corresponding
 * to the keys also appear next to their respective
 * keys, in hexadecimal notation.
 * 
 * @param root the root nodes
 */
void BtreePrintTree(BtreeNode_t * root) {
    BtreeNode_t * n = NULL;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    queue = NULL;
    enqueue(root);
    while (queue != NULL) {
        n = dequeue();
        if (n->parent != NULL && n == n->parent->pointers[0]) {
            new_rank = path_to_root(root, n);
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        if (verbose_output)
            printf("(%lx)", (unsigned long) n);
        for (i = 0; i < n->num_keys; i++) {
            if (verbose_output)
                printf("%lx ", (unsigned long) n->pointers[i]);
            printf("%d ", n->keys[i]);
        }
        if (!n->is_leaf)
            for (i = 0; i <= n->num_keys; i++)
                enqueue(n->pointers[i]);
        if (verbose_output) {
            if (n->is_leaf)
                printf("%lx ", (unsigned long) n->pointers[order - 1]);
            else
                printf("%lx ", (unsigned long) n->pointers[n->num_keys]);
        }
        printf("| ");
    }
    printf("\n");
}

/**
 * Utility function to give the height
 * of the tree, which length in number of edges
 * of the path from the root to any leaf.
 * 
 * @param root the root node
 * @return the height of the tree
 */
int BTreeHeight(BtreeNode_t * root) {
    int h = 0;
    BtreeNode_t * c = root;
    while (!c->is_leaf) {
        c = c->pointers[0];
        h++;
    }
    return h;
}

void destroy_tree_nodes(BtreeNode_t * root, void freeRecord(void *)) {
    int i;
    if (root == NULL) return;
    if (root->is_leaf) {
        for (i = 0; i < root->num_keys; i++) {
            if (freeRecord != NULL) {
                freeRecord(((BtreeRecord_t*) root->pointers[i])->value);
            }
            free(root->pointers[i]);
        }
    } else {
        for (i = 0; i < root->num_keys + 1; i++) {
            destroy_tree_nodes(root->pointers[i], freeRecord);
        }
    }
    free(root->pointers);
    free(root->keys);
    free(root);
}

/**
 * Free the tree using the record specific function
 * 
 * @param root the root node
 * @param freeRecord the record specific function
 * @return NULL;
 */
BtreeNode_t * BTreeFree(BtreeNode_t * root, void freeRecord(void *)) {
    destroy_tree_nodes(root, freeRecord);
    return NULL;
}