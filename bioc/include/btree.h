/* 
 * File:   btree.h
 * Author: roberto
 *
 * Created on May 7, 2014, 9:55 AM
 */

#ifndef BTREE_H
#define	BTREE_H

#ifdef	__cplusplus
extern "C" {
#endif

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

    typedef struct record {
        void *value;
    } record;

    /* Type representing a node in the B+ tree.
     * This type is general enough to serve for both
     * the leaf and the internal node.
     * The heart of the node is the array
     * of keys and the array of corresponding
     * pointers.  The relation between keys
     * and pointers differs between leaves and
     * internal nodes.  In a leaf, the index
     * of each key equals the index of its corresponding
     * pointer, with a maximum of order - 1 key-pointer
     * pairs.  The last pointer points to the
     * leaf to the right (or NULL in the case
     * of the rightmost leaf).
     * In an internal node, the first pointer
     * refers to lower nodes with keys less than
     * the smallest key in the keys array.  Then,
     * with indices i starting at 0, the pointer
     * at i + 1 points to the subtree with keys
     * greater than or equal to the key in this
     * node at index i.
     * The num_keys field is used to keep
     * track of the number of valid keys.
     * In an internal node, the number of valid
     * pointers is always num_keys + 1.
     * In a leaf, the number of valid pointers
     * to data is always num_keys.  The
     * last leaf pointer points to the next leaf.
     */
    typedef struct node {
        void ** pointers;
        int * keys;
        struct node * parent;
        bool is_leaf;
        int num_keys;
        struct node * next; // Used for queue.
    } node;

    // Default order is 10.
#define DEFAULT_ORDER 20

    // Minimum order is necessarily 3.  We set the maximum
    // order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 20    

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
    extern node *insert(node * root, int key, void *value);

    /**
     * Finds and returns the record to which a key refers.
     * 
     * @param root the root node
     * @param key the key of the object to find
     * @param verbose true to print info
     * @return 
     */
    extern record *find(node * root, int key, bool verbose);

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
    extern void print_tree(node * root);

    /**
     * Utility function to give the height
     * of the tree, which length in number of edges
     * of the path from the root to any leaf.
     * 
     * @param root the root node
     * @return the height of the tree
     */
    extern int height(node * root);

    /**
     * Destroy the tree using the record specific function
     * 
     * @param root the root node
     * @param freeRecord the record specific function
     * @return NULL;
     */
    extern node *destroy_tree(node * root, void freeRecord(void *));

#ifdef	__cplusplus
}
#endif

#endif	/* BTREE_H */

