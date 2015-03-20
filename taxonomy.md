# NCBI Taxonomy database #

This module is designed to work with the [NCBI Taxonomy](https://www.ncbi.nlm.nih.gov/taxonomy) database.

The module parse the nodes.dmp and names.dmp files on [taxdmp.zip](ftp://ftp.ncbi.nih.gov/pub/taxonomy/taxdmp.zip) file and creates a [Btree](btree.md) structure.

The Taxonomy [Btree](btree.md) structure can be used to get the names, ranks, parent taxid and the taxonomy lineage.

[taxonomy.h](https://bioc.googlecode.com/svn/trunk/bioc/include/taxonomy.h)  [taxonomy.c](https://bioc.googlecode.com/svn/trunk/bioc/src/taxonomy.c)

```
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zlib.h>
#include <time.h>
#include "bmemory.h"
#include "bstring.h"
#include "berror.h"
#include "btree.h"
#include "btime.h"
#include "taxonomy.h"
```

The structure:

```
    struct taxonomy_s {
        int taxId;
        int parentTaxId;
        char *name;
        char *rank;

        /*
         * Methods
         */

        /**
         * Set the TaxId
         * 
         * @param self the container object
         * @param int the taxId
         */
        void (*setTaxId)(void *self, int taxId);

        /**
         * Set the parentTaxId
         * 
         * @param self the container object
         * @param int the parentTaxId
         */
        void (*setParentTaxId)(void *self, int parentTaxId);

        /**
         * Set the name
         * 
         * @param self the container object
         * @param string the sequence
         */
        void (*setName)(void *self, char *name);

        /**
         * Set the rank
         * 
         * @param self the container object
         * @param string the rank
         */
        void (*setRank)(void *self, char *rank);

        /**
         * Return an array of TaxIds of the lineage for the taxonomy
         * 
         * @param self the container object
         * @param an array of TaxIds of the lineage for the taxonomy
         * @param count the number of elements in the leneage array
         * @param taxDB the NCBI Taxonomy database in a bTree index
         */
        void (*getLineage)(void *self, int **lineage, int *count, BtreeNode_t *taxDB);


        /**
         * Print the Taxonomy to the STDOUT
         * 
         * @param self the container object
         */
        void (*toString)(void *self);

        /**
         * Print the taxonomy to a file 
         * 
         * @param self the container object
         * @param out the output file
         */
        void (*toFile)(void * self, FILE *out);

        /**
         * Free the fasta container
         * 
         * @param self the container object
         */
        void (*free)(void *self);
    };

    typedef struct taxonomy_s *taxonomy_l;

```

# Independent functions #

## Read a taxonomy entry from the files ##

```
    /**
     * @param nodes the nodes.dmp NCBI Taxonomy file
     * @param names the names.dmp NCBI Taxonomy file
     * @return the taxonomy entry
     */
    extern taxonomy_l ReadTaxonomy(FILE *nodes, FILE *names);
```

## Read the NCBI Taxonomy nodes.dmp and names.dmp files an return a Btree index with the data ##

```
    /**
     * @param dir the NCBI Taxonomy DB directory
     * @param verbose 1 to print a verbose info
     * @return the NCBI Taxonomy db in a Btree index
     */
    extern BtreeNode_t *TaxonomyDBIndex(char *dir, int verbose);
```

## Read the gi\_taxid\_nucl.dmp file from NCBI Taxonomy and return a Btree index ##

This function reads the [gi\_taxid\_nucl.dmp.gz](ftp://ftp.ncbi.nih.gov/pub/taxonomy/gi_taxid_nucl.dmp.gz) uncompressed file and create a [Btree](btree.md) structure. This index can be used to assign the taxId to the GenBank Gi.

The function require around 32 GB RAM due to the size of the gi\_taxid\_nucl.dmp.gz file.

```
    /**
     * @param filename the gi_taxid_nucl.dmp.gz complete path
     * @param verbose 1 to print a verbose info
     * @return 
     */
    extern BtreeNode_t *TaxonomyNuclIndex(char *gi_taxid_nucl, int verbose);
```

Examples:

Print the taxonomy lineage from a list of GenBank Gis. The source code [here](https://code.google.com/p/bioc/source/browse/trunk/TaxLineageFromGi/)

Print the taxonomy lineage from a list of TaxIds. The source code [here](https://code.google.com/p/bioc/source/browse/trunk/TaxLineageFromTaxId/)