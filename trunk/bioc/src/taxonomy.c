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
void toFile(void * self, FILE *out) {
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
void toString(void * self) {
    _CHECK_SELF_P(self);
    toFile(self, stdout);
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
void getLineage(void *self, int **lineage, int *count, node *taxDB) {
    _CHECK_SELF_P(self);
    record *rec;

    *lineage = (int *) realloc(*lineage, sizeof (int) * (*count + 1));
    (*lineage)[*count] = ((taxonomy_l) self)->taxId;
    (*count)++;
    if (((taxonomy_l) self)->parentTaxId != 1) {
        if ((rec = find(taxDB, ((taxonomy_l) self)->parentTaxId, false)) != NULL) {
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
    tax->toFile = &toFile;
    tax->toString = &toString;
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
    off_t pos;
    char **ids;
    int ids_number;

    if (getline(&line, &len, nodes) != -1) {
        self = CreateTaxonomy();
        ids_number = splitString(&ids, line, "\t|");
        self->setTaxId(self, atoi(ids[0]));
        self->setParentTaxId(self, atoi(ids[1]));
        self->setRank(self, ids[2]);

        freeString(ids, ids_number);
        while ((read = getline(&line, &len, names)) != -1) {
            i = atoi(line);
            if (i == self->taxId) {
                if (strstr(line, "scientific name") != NULL) {
                    ids_number = splitString(&ids, line, "\t|");
                    self->setName(self, ids[1]);
                    freeString(ids, ids_number);
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
 * @param nodes the nodes.dmp NCBI Taxonomy file
 * @param names the names.dmp NCBI Taxonomy file
 * @return the NCBI Taxonomy db in a Btree index
 */
node *TaxonomyDBIndex(FILE *nodes, FILE *names) {
    node *root = NULL;
    taxonomy_l tax;
    while ((tax = ReadTaxonomy(nodes, names)) != NULL) {
        root = insert(root, tax->taxId, tax);
    }
    return root;
}

/**
 * Read the gi_taxid_nucl.dmp.gz file from NCBI Taxonomy and return a Btree 
 * index ober the Gi
 * 
 * @param filename the gi_taxid_nucl.dmp.gz complete path
 * @param verbose 1 to print a verbose info
 * @return 
 */
node *TaxonomyNuclIndex(char *gi_taxid_nucl, int verbose) {
    struct timespec start, stop;
    gzFile gFile;
    node *root = NULL;
    char *buffer = NULL;
    int gi;
    int *taxid;
    int count = 0;

    if (verbose) printf("\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    gFile = checkPointerError(gzopen(gi_taxid_nucl, "rb"), "Can't open the gi_taxid_nucl.dmp.gz file", __FILE__, __LINE__, -1);

    buffer = allocate(sizeof (char) * 101, __FILE__, __LINE__);
    while (!gzeof(gFile)) {
        gzgets(gFile, buffer, 100);
        taxid = (int *) malloc(sizeof (int));
        sscanf(buffer, "%d\t%d\n", &gi, taxid);
        root = insert(root, gi, taxid);
        if (verbose && count % 10000 == 0) {
            clock_gettime(CLOCK_MONOTONIC, &stop);
            printf("\tReading GIs: Total: %10d\t\tTime: %lu   \r", count, timespecDiffSec(&stop, &start));
        }
        count++;
        if (count == 10000000) break;
    }

    if (buffer) free(buffer);
    gzclose(gFile);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    if (verbose) printf("\n\tThere are %d GIs into the B+Tree. Elapsed time: %lu sec\n\n", count, timespecDiffSec(&stop, &start));
    fflush(NULL);
    return root;
}