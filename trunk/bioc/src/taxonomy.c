/* 
 * File:   taxonomy.c
 * Author: roberto
 *
 * Created on May 7, 2014, 8:55 AM
 */

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

/**
 * Free the fasta container
 * 
 * @param self the container object
 */
void freeTaxonomy(void *self) {
    _CHECK_SELF_P(self);
    if (((taxonomy_l) self)->name) free(((taxonomy_l) self)->name);
    if (((taxonomy_l) self)->rank) free(((taxonomy_l) self)->rank);
    free(((taxonomy_l) self));
}

/**
 * Print the taxonomy to a file 
 * 
 * @param self the container object
 * @param out the output file
 */
void toFileTaxonomy(void * self, FILE *out) {
    _CHECK_SELF_P(self);
    fprintf(out, "%d %d %s %s\n",
            ((taxonomy_l) self)->taxId,
            ((taxonomy_l) self)->parentTaxId,
            ((taxonomy_l) self)->rank,
            ((taxonomy_l) self)->name);
}

/**
 * Print the Taxonomy to the STDOUT
 * 
 * @param self the container object
 */
void toStringTaxonomy(void * self) {
    _CHECK_SELF_P(self);
    ((taxonomy_l) self)->toFile(self, stdout);
}

/**
 * Set the TaxId
 * 
 * @param self the container object
 * @param int the taxId
 */
void setTaxId(void *self, int taxId) {
    _CHECK_SELF_P(self);
    ((taxonomy_l) self)->taxId = taxId;
}

/**
 * Set the parentTaxId
 * 
 * @param self the container object
 * @param int the parentTaxId
 */
void setParentTaxId(void *self, int parentTaxId) {
    _CHECK_SELF_P(self);
    ((taxonomy_l) self)->parentTaxId = parentTaxId;
}

/**
 * Set the name
 * 
 * @param self the container object
 * @param string the sequence
 */
void setName(void *self, char *name) {
    _CHECK_SELF_P(self);
    ((taxonomy_l) self)->name = strdup(name);
}

/**
 * Set the rank
 * 
 * @param self the container object
 * @param string the rank
 */
void setRank(void *self, char *rank) {
    _CHECK_SELF_P(self);
    ((taxonomy_l) self)->rank = strdup(rank);
}

/**
 * Return an array of TaxIds of the lineage for the taxonomy
 * 
 * @param self the container object
 * @param an array of TaxIds of the lineage for the taxonomy
 * @param count the number of elements in the leneage array
 * @param taxDB the NCBI Taxonomy database in a bTree index
 */
void getLineage(void *self, int **lineage, int *count, BtreeNode_t *taxDB) {
    _CHECK_SELF_P(self);
    BtreeRecord_t *rec;

    *lineage = (int *) realloc(*lineage, sizeof (int) * (*count + 1));
    (*lineage)[*count] = ((taxonomy_l) self)->taxId;
    (*count)++;
    if (((taxonomy_l) self)->parentTaxId != 1) {
        if ((rec = BTreeFind(taxDB, ((taxonomy_l) self)->parentTaxId, false)) != NULL) {
            getLineage(rec->value, lineage, count, taxDB);
        }
    }
}

/**
 * Create the Taxonomy object and initialized the pointers to the methods
 * 
 * @return a taxonomy_l object
 */
taxonomy_l CreateTaxonomy() {
    taxonomy_l tax = allocate(sizeof (struct taxonomy_s), __FILE__, __LINE__);

    tax->taxId = -1;
    tax->parentTaxId = -1;
    tax->name = NULL;
    tax->rank = NULL;

    tax->free = &freeTaxonomy;
    tax->toFile = &toFileTaxonomy;
    tax->toString = &toStringTaxonomy;
    tax->setTaxId = &setTaxId;
    tax->setParentTaxId = &setParentTaxId;
    tax->setName = &setName;
    tax->setRank = &setRank;
    tax->getLineage = &getLineage;
    return tax;
}

/**
 * Read a taxonomy entry from the files
 * 
 * @param nodes the nodes.dmp NCBI Taxonomy file
 * @param names the names.dmp NCBI Taxonomy file
 * @return the taxonomy entry
 */
taxonomy_l ReadTaxonomy(FILE *nodes, FILE *names) {
    taxonomy_l self = NULL;
    int i;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    off_t pos = 0;
    char **ids;
    int ids_number;

    if (getline(&line, &len, nodes) != -1) {
        self = CreateTaxonomy();
        ids_number = splitString(&ids, line, "\t|");
        if (ids_number < 3) {
            fprintf(stderr, "LINE: %s", line);
            fprintf(stderr, "ids number: %d\n", ids_number);
            checkPointerError(NULL, "Can't parse the nodes.dmp file", __FILE__, __LINE__, -1);
        }
        self->setTaxId(self, atoi(ids[0]));
        self->setParentTaxId(self, atoi(ids[1]));
        self->setRank(self, ids[2]);

        freeArrayofPointers((void **) ids, ids_number);
        while ((read = getline(&line, &len, names)) != -1) {
            i = atoi(line);
            if (i == self->taxId) {
                if (strstr(line, "scientific name") != NULL) {
                    ids_number = splitString(&ids, line, "\t|");
                    self->setName(self, ids[1]);
                    freeArrayofPointers((void **) ids, ids_number);
                }
            }
            if (i > self->taxId) {
                fseeko(names, pos, SEEK_SET);
                break;
            }
            pos = ftello(names);
        }
    }
    if (line) free(line);
    return self;
}

/**
 * Read the NCBI Taxonomy nodes.dmp and names.dmp files an return a Btree index with 
 * the data
 * 
 * @param dir the NCBI Taxonomy DB directory
 * @param verbose 1 to print a verbose info
 * @return the NCBI Taxonomy db in a Btree index
 */
BtreeNode_t *TaxonomyDBIndex(char *dir, int verbose) {
    struct timespec stop, mid;
    char *tmp;
    FILE *nodes, *names;
    BtreeNode_t *root = NULL;
    taxonomy_l tax;

    clock_gettime(CLOCK_MONOTONIC, &mid);
    if (verbose) {
        printf("Reading the Taxonomy database ... ");
        fflush(stdout);
    }

    tmp = allocate(sizeof (char) * (strlen(dir) + 11), __FILE__, __LINE__);
    sprintf(tmp, "%s/nodes.dmp", dir);
    nodes = checkPointerError(fopen(tmp, "r"), "Can't open the nodes file", __FILE__, __LINE__, -1);
    sprintf(tmp, "%s/names.dmp", dir);
    names = checkPointerError(fopen(tmp, "r"), "Can't open the names file", __FILE__, __LINE__, -1);

    while ((tax = ReadTaxonomy(nodes, names)) != NULL) {
        root = BtreeInsert(root, tax->taxId, tax);
    }

    fclose(nodes);
    fclose(names);
    free(tmp);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    if (verbose) {
        printf("%.2f sec\n", timespecDiffSec(&stop, &mid));
        fflush(stdout);
    }
    return root;
}

/**
 * Read the gi_taxid_nucl.dmp file from NCBI Taxonomy and return a Btree 
 * index over the Gi
 * 
 * @param filename the gi_taxid_nucl.dmp complete path
 * @param verbose 1 to print a verbose info
 * @return 
 */
BtreeNode_t *TaxonomyNuclIndex(char *gi_taxid_nucl, int verbose) {
    struct timespec start, stop;
    FILE *fi;
    BtreeNode_t *root = NULL;
    int gi;
    int *taxid;
    int count = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if (verbose) printf("\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    fi = checkPointerError(fopen(gi_taxid_nucl, "r"), "Can't open the gi_taxid_nucl.dmp.gz file", __FILE__, __LINE__, -1);

    while ((read = getline(&line, &len, fi)) != -1) {
        taxid = (int *) malloc(sizeof (int));
        sscanf(line, "%d\t%d\n", &gi, taxid);
        root = BtreeInsert(root, gi, taxid);
        if (verbose && count % 10000 == 0) {
            clock_gettime(CLOCK_MONOTONIC, &stop);
            printf("\tReading GIs: Total: %10d\t\tTime: %.2f   \r", count, timespecDiffSec(&stop, &start));
        }
        count++;
    }

    if (line) free(line);
    fclose(fi);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    if (verbose) printf("\n\tThere are %d GIs into the B+Tree. Elapsed time: %.2f sec\n\n", count, timespecDiffSec(&stop, &start));
    fflush(NULL);
    return root;
}