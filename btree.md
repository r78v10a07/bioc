# Integer key based Btree #

This code was taken and modified from [here](http://www.amittai.com/prose/bplustree.html). Original code wrote by [Amittai Aviram](http://www.amittai.com).

[btree.h](https://bioc.googlecode.com/svn/trunk/bioc/include/btree.h)  [btree.c](https://bioc.googlecode.com/svn/trunk/bioc/src/btree.c)

```
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "btree.h"
```

Two important structures:

The record and the node structure:

```
    typedef struct BtreeRecord_t {
        void *value;
    } BtreeRecord_t;

    typedef struct BtreeNode_t {
        void ** pointers;
        int * keys;
        struct BtreeNode_t * parent;
        bool is_leaf;
        int num_keys;
        struct BtreeNode_t * next; // Used for queue.
    } BtreeNode_t;
```

## Master insertion function ##

```
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
    extern BtreeNode_t *BtreeInsert(BtreeNode_t * root, int key, void *value)
```

Usage:

```
   FILE *fd;
   BtreeNode_t *root = NULL;
   fasta_l fasta;
   int gi;
  
   // Read the fasta from the file fd and create an index with them 
   while ((fasta = ReadFasta(fd, 0)) != NULL) {
        // Get the fasta Gi parsing the fasta header
        fasta->getGi(fasta, &gi);        
        
        /* Insert the position in the file where the fasta 
         * object start using the gi as key
         */
        root = BtreeInsert(root, gi, fasta);
    }
```

## Free the Btree ##

```
    /**
     * Free the tree using the record specific function
     * 
     * @param root the root node
     * @param freeRecord the record specific function
     * @return NULL;
     */
    extern BtreeNode_t *BTreeFree(BtreeNode_t * root, void freeRecord(void *));
```

Usage:

```
   // Here the fasta->free is a void * to the fasta_l free function
   BTreeFree(root, fasta->free);
```

## Finds and returns the record to which a key refers ##

```
    /**
     * @param root the root node
     * @param key the key of the object to find
     * @param verbose true to print info
     * @return 
     */
    extern BtreeRecord_t *BTreeFind(BtreeNode_t * root, int key, bool verbose);
```

Usage:

```
   BtreeRecord_t *rec = BTreeFind(root,10, false);
 
   fasta_l fasta = (fasta_l) rec->value;
```

## Print the BTree ##

```
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
    extern void void BtreePrintTree(BtreeNode_t * root);
```

Usage:

```
   BtreePrintTree(root);
```

## Utility function to give the height of the tree ##

```
    /**
     * Utility function to give the height
     * of the tree, which length in number of edges
     * of the path from the root to any leaf.
     * 
     * @param root the root node
     * @return the height of the tree
     */
    extern int BTreeHeight(BtreeNode_t * root);
```

Usage:

```
   int h = BTreeHeight(root);
```