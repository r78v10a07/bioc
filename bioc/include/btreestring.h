/* 
 * File:   btreestring.h
 * Author: roberto
 *
 * Created on May 16, 2014, 11:06 AM
 */

#ifndef BTREESTRING_H
#define	BTREESTRING_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct BtreeNodeString_t {
        void ** pointers;
        char **keys;
        struct BtreeNodeString_t * parent;
        bool is_leaf;
        int num_keys;
        struct BtreeNodeString_t * next; // Used for queue.
    } BtreeNodeString_t;


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
    extern BtreeNodeString_t *BtreeInsertString(BtreeNodeString_t * root, char *key, void *value);

    /**
     * Finds and returns the record to which a key refers.
     * 
     * @param root the root node
     * @param key the key of the object to find
     * @param verbose true to print info
     * @return 
     */
    extern BtreeRecord_t * BTreeFindString(BtreeNodeString_t * root, char *key, bool verbose);
    
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
    extern void BtreeStringPrintTree(BtreeNodeString_t * root);

    /**
     * Utility function to give the height
     * of the tree, which length in number of edges
     * of the path from the root to any leaf.
     * 
     * @param root the root node
     * @return the height of the tree
     */
    extern int BTreeStringHeight(BtreeNodeString_t * root);

    /**
     * Destroy the tree using the record specific function
     * 
     * @param root the root node
     * @param freeRecord the record specific function
     * @return NULL;
     */
    extern BtreeNodeString_t *BTreeStringFree(BtreeNodeString_t * root, void freeRecord(void *));

#ifdef	__cplusplus
}
#endif

#endif	/* BTREESTRING_H */

