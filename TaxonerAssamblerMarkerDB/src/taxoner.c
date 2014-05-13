/* 
 * File:   taxoner.c
 * Author: roberto
 *
 * Created on May 8, 2014, 11:05 AM
 */

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zlib.h>
#include "berror.h"
#include "bmemory.h"
#include "bstring.h"
#include "btree.h"
#include "btime.h"
#include "fasta.h"
#include "taxonomy.h"
#include "taxoner.h"

/**
 * Free the Taxoner Gi container
 * 
 * @param self the container object
 */
void freeTaxonerGi(void *self) {
    _CHECK_SELF_P(self);
    int i;
    for (i = 0; i < ((taxoner_gi_l) self)->fromTo_number; i++) {
        if (((taxoner_gi_l) self)->fromTo[i]) free(((taxoner_gi_l) self)->fromTo[i]);
    }
    if (((taxoner_gi_l) self)->fromTo) free(((taxoner_gi_l) self)->fromTo);
}

/**
 * Free the Taxoner Tax container
 * 
 * @param self the container object
 */
void freeTaxonerTax(void *self) {
    int i;
    _CHECK_SELF_P(self);
    for (i = 0; i < ((taxoner_tax_l) self)->gis_numbers; i++) {
        freeTaxonerGi(&(((taxoner_tax_l) self)->gis[i]));
    }
    free(((taxoner_tax_l) self)->gis);
    if (((taxoner_tax_l) self)->gisIndex) BTreeFree(((taxoner_tax_l) self)->gisIndex, free);
    free(((taxoner_tax_l) self));
}

static int cmpFromTo(const void *p1, const void *p2) {
    return *((int * const *) p1)[0] - *((int * const *) p2)[0];
}

/**
 * Sort the From-To values by the Front 
 * 
 * @param self the container object
 */
void sortFromTo(void *self) {
    _CHECK_SELF_P(self);
    if (((taxoner_gi_l) self)->fromTo) {
        qsort(((taxoner_gi_l) self)->fromTo, ((taxoner_gi_l) self)->fromTo_number, sizeof (int *), cmpFromTo);
    }
}

/**
 * Internal function to initialize the taxoner_gi_l pointer
 * 
 * @param self the pointer to be initialized
 */
void InitTaxonerGi(taxoner_gi_l self) {
    self->gi = -1;
    self->fromTo = NULL;
    self->fromTo_number = 0;
    self->sortFromTo = &sortFromTo;
    self->free = &freeTaxonerGi;
}

/**
 * Create the Taxoner Gi and initialized the pointers to the methods
 * 
 * @return a taxoner_gi_l object
 */
taxoner_gi_l CreateTaxonerGi() {
    taxoner_gi_l self = allocate(sizeof (struct taxoner_gi_s), __FILE__, __LINE__);
    InitTaxonerGi(self);
    return self;
}

/**
 * Create the Taxoner Tax and initialized the pointers to the methods
 * 
 * @return a taxoner_tax_l object
 */
taxoner_tax_l CreateTaxonerTax() {
    taxoner_tax_l self = allocate(sizeof (struct taxoner_tax_s), __FILE__, __LINE__);
    self->taxId = -1;
    self->gis = NULL;
    self->gis_numbers = 0;
    self->gisIndex = NULL;
    self->free = &freeTaxonerTax;
    return self;
}

/**
 * Print the assambled result if the input tax has GI
 * 
 * @param outs array with the outputs files. [0] summary, [1] error
 * @param tax2 the input taxoner option
 * @param fBtree the fasta btree index
 * @param fFasta the fasta file
 * @param taxDB the NCBI Taxonomy db * 
 * @param readLenght length of the reads
 * @param readOffset offset used to overlap the reads
 * @param verbose 1 to print info
 */
void printTaxwithReads(FILE **outs, char **ids, int ids_number, taxoner_tax_l tax2, BtreeNode_t *fBtree, FILE * fFasta, BtreeNode_t *taxDB, int readLength, int readOffset, int verbose) {
    int j, k, i;
    taxoner_gi_l tmpTaxGi;
    taxonomy_l taxon;
    fasta_l fna;
    BtreeRecord_t *rec;
    off_t offset;
    int nt, reads;
    int headerSize = 1000;

    char *header = allocate(sizeof (char) * headerSize, __FILE__, __LINE__);

    nt = reads = 0;
    if (tax2->gis_numbers != 0) {
        for (j = 0; j < tax2->gis_numbers; j++) {
            tmpTaxGi = &(tax2->gis[j]);
            taxon = NULL;
            fna = NULL;
            if ((rec = BTreeFind(taxDB, tax2->taxId, false)) != NULL) {
                taxon = ((taxonomy_l) rec->value);
                if ((rec = BTreeFind(fBtree, tmpTaxGi->gi, false)) != NULL) {
                    offset = *((off_t *) rec->value);
                    fseeko(fFasta, offset, SEEK_SET);
                    fna = ReadFasta(fFasta, 0);
                    nt = reads = 0;
                    for (k = 0; k < tmpTaxGi->fromTo_number; k++) {
                        nt += (tmpTaxGi->fromTo[k][1] - tmpTaxGi->fromTo[k][0]);
                        reads += ((tmpTaxGi->fromTo[k][1] - tmpTaxGi->fromTo[k][0] - readLength) / readOffset + 1);
                        for (i = 0; i < ids_number; i++) {
                            if (strcmp(taxon->rank, ids[i]) == 0) {                                
                                if (strlen(fna->header) > headerSize - 100) {
                                    headerSize = strlen(fna->header);
                                    header = reallocate(header, sizeof (char) * headerSize, __FILE__, __LINE__);
                                }
                                sprintf(header, "%d|%d-%d", tmpTaxGi->gi, tmpTaxGi->fromTo[k][0], tmpTaxGi->fromTo[k][1]);
                                fna->printSegment(fna, outs[i + 2], header, tmpTaxGi->fromTo[k][0], tmpTaxGi->fromTo[k][1] - tmpTaxGi->fromTo[k][0], 80);
                                fflush(outs[i + 2]);
                                break;
                            }
                        }
                    }
                    if (verbose) {
                        printf("%10d\t%6d\t%50s\t%15s\t%10d\t%12d\t%18d\n",
                                tmpTaxGi->gi, tax2->taxId, taxon->name, taxon->rank,
                                fna->len, nt, reads);
                        fflush(stdout);
                    }
                    fprintf(outs[0], "%10d\t%6d\t%50s\t%15s\t%10d\t%12d\t%18d\n",
                            tmpTaxGi->gi, tax2->taxId, taxon->name, taxon->rank,
                            fna->len, nt, reads);
                    fna->free(fna);
                    fflush(outs[0]);
                } else {
                    if (verbose) {
                        printf("The GI %d does not have a fasta seq\n", tmpTaxGi->gi);
                        fflush(stdout);
                    }
                    fprintf(outs[1], "fasta\t%d\n", tmpTaxGi->gi);
                }
            } else {
                if (verbose) {

                    printf("Taxa %d is not in the current NCBI Taxonomy DB\n", tax2->taxId);
                    fflush(stdout);
                }
                fprintf(outs[1], "taxa\t%d\n", tax2->taxId);
            }
        }
    }
    free(header);
}

void checkTaxForContReads(FILE **outs, char **ids, int ids_number, taxoner_tax_l tax, BtreeNode_t *fBtree, FILE * fFasta, BtreeNode_t *taxDB, int readLength, int readOffset, int verbose) {
    taxoner_tax_l tax2;
    taxoner_gi_l tmpTaxGi;
    BtreeRecord_t *rec;
    int *index;

    int i, taxGi, gi, from, to;
    tax2 = CreateTaxonerTax();
    tax2->taxId = tax->taxId;
    tmpTaxGi = NULL;

    gi = from = to = 0;
    for (taxGi = 0; taxGi < tax->gis_numbers; taxGi++) {
        tmpTaxGi = &(tax->gis[taxGi]);
        tmpTaxGi->sortFromTo(tmpTaxGi);
        gi = 1;
        from = tmpTaxGi->fromTo[0][0];
        to = tmpTaxGi->fromTo[0][1];
        for (i = 1; i < tmpTaxGi->fromTo_number; i++) {
            if (tmpTaxGi->fromTo[i][0] < to) {
                gi++;
                to = tmpTaxGi->fromTo[i][1];
            } else {
                if (gi > 1) {
                    if ((rec = BTreeFind(tax2->gisIndex, tmpTaxGi->gi, false)) != NULL) {
                        index = (int *) rec->value;
                        tax2->gis[*index].fromTo = checkPointerError(realloc(tax2->gis[*index].fromTo, sizeof (int **) * (tax2->gis[*index].fromTo_number + 1)), "Can't reallocate memory", __FILE__, __LINE__, -1);
                        tax2->gis[*index].fromTo[tax2->gis[*index].fromTo_number] = allocate(sizeof (int) * 2, __FILE__, __LINE__);
                        tax2->gis[*index].fromTo[tax2->gis[*index].fromTo_number][0] = from;
                        tax2->gis[*index].fromTo[tax2->gis[*index].fromTo_number][1] = to;
                        tax2->gis[*index].fromTo_number++;
                    } else {
                        tax2->gis = checkPointerError(realloc(tax2->gis, sizeof (struct taxoner_gi_s) * (tax2->gis_numbers + 1)), "Can't reallocate memory", __FILE__, __LINE__, -1);

                        InitTaxonerGi(&(tax2->gis[tax2->gis_numbers]));
                        tax2->gis[tax2->gis_numbers].gi = tmpTaxGi->gi;
                        tax2->gis[tax2->gis_numbers].fromTo = allocate(sizeof (int **) * 1, __FILE__, __LINE__);
                        tax2->gis[tax2->gis_numbers].fromTo[0] = allocate(sizeof (int) * 2, __FILE__, __LINE__);
                        tax2->gis[tax2->gis_numbers].fromTo[0][0] = from;
                        tax2->gis[tax2->gis_numbers].fromTo[0][1] = to;
                        tax2->gis[tax2->gis_numbers].fromTo_number = 1;

                        index = allocate(sizeof (int), __FILE__, __LINE__);
                        *index = tax2->gis_numbers;
                        tax2->gisIndex = BtreeInsert(tax2->gisIndex, tax2->gis[tax2->gis_numbers].gi, index);
                        tax2->gis_numbers++;
                    }
                }
                gi = 1;
                from = tmpTaxGi->fromTo[i][0];
                to = tmpTaxGi->fromTo[i][1];
            }
        }
    }
    if (gi > 1) {
        if ((rec = BTreeFind(tax2->gisIndex, tmpTaxGi->gi, false)) != NULL) {
            index = (int *) rec->value;
            tax2->gis[*index].fromTo = checkPointerError(realloc(tax2->gis[*index].fromTo, sizeof (int **) * (tax2->gis[*index].fromTo_number + 1)), "Can't reallocate memory", __FILE__, __LINE__, -1);
            tax2->gis[*index].fromTo[tax2->gis[*index].fromTo_number] = allocate(sizeof (int) * 2, __FILE__, __LINE__);
            tax2->gis[*index].fromTo[tax2->gis[*index].fromTo_number][0] = from;
            tax2->gis[*index].fromTo[tax2->gis[*index].fromTo_number][1] = to;
            tax2->gis[*index].fromTo_number++;
        } else {
            tax2->gis = checkPointerError(realloc(tax2->gis, sizeof (struct taxoner_gi_s) * (tax2->gis_numbers + 1)), "Can't reallocate memory", __FILE__, __LINE__, -1);
            InitTaxonerGi(&(tax2->gis[tax2->gis_numbers]));
            tax2->gis[tax2->gis_numbers].gi = tmpTaxGi->gi;
            tax2->gis[tax2->gis_numbers].fromTo = allocate(sizeof (int **) * 1, __FILE__, __LINE__);
            tax2->gis[tax2->gis_numbers].fromTo[0] = allocate(sizeof (int) * 2, __FILE__, __LINE__);
            tax2->gis[tax2->gis_numbers].fromTo[0][0] = from;
            tax2->gis[tax2->gis_numbers].fromTo[0][1] = to;
            tax2->gis[tax2->gis_numbers].fromTo_number = 1;

            index = allocate(sizeof (int), __FILE__, __LINE__);
            *index = tax2->gis_numbers;
            tax2->gisIndex = BtreeInsert(tax2->gisIndex, tax2->gis[tax2->gis_numbers].gi, index);
        }
    }
    printTaxwithReads(outs, ids, ids_number, tax2, fBtree, fFasta, taxDB, readLength, readOffset, verbose);
    tax2->free(tax2);
}

/**
 * Parse the Taxoner file and print into the output dir the results
 * 
 * @param output the name of the ouput dir
 * @param rankToPrint coma separated list of taxonomy rank to print fasta
 * @param fd the input file
 * @param score the score to be used as cutoff
 * @param fBtree the fasta btree index
 * @param fFasta the fasta file
 * @param taxDB the NCBI Taxonomy db
 * @param readLenght length of the reads
 * @param readOffset offset used to overlap the reads
 * @param verbose 1 to print info
 */
void ParseTaxonerResult(char *output, char *rankToPrint, FILE *fd, float score, BtreeNode_t *fBtree, FILE * fFasta, BtreeNode_t *taxDB, int readLength, int readOffset, int verbose) {
    int *index, lastTaxId;
    taxoner_tax_l tax;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int rgi, taxId, taxGi, rfrom, rto;
    float rScore;
    BtreeRecord_t *rec;
    char **ids = NULL;
    int ids_number = 0;
    FILE **outs;

    if (rankToPrint) {
        ids_number = splitString(&ids, rankToPrint, ",");
    } else {
        ids_number = 0;
    }
    outs = allocate(sizeof (FILE *) * (ids_number + 2), __FILE__, __LINE__);

    len = sizeof (char) * (strlen(output) + 150);
    line = allocate(len, __FILE__, __LINE__);
    sprintf(line, "%s/summary.txt", output);
    outs[0] = checkPointerError(fopen(line, "w"), "Can't open the summary file", __FILE__, __LINE__, -1);
    sprintf(line, "%s/error.txt", output);
    outs[1] = checkPointerError(fopen(line, "w"), "Can't open the error file", __FILE__, __LINE__, -1);
    for (rgi = 2; rgi < ids_number + 2; rgi++) {
        sprintf(line, "%s/%s.fna", output, ids[rgi - 2]);
        outs[rgi] = checkPointerError(fopen(line, "w"), "Can't open the error file", __FILE__, __LINE__, -1);
    }

    lastTaxId = -1;
    tax = NULL;

    fprintf(outs[0], "%10s\t%6s\t%50s\t%15s\t%10s\t%12s\t%18s\n"
            , "gi", "taxid", "tax name", "rank",
            "total bp",
            "assigned bp",
            "consecutive reads");
    if (verbose) {
        printf("Starting the Taxonomy.txt file parser\n");
        printf("\n%10s\t%6s\t%50s\t%15s\t%10s\t%12s\t%18s\n"
                , "gi", "taxid", "tax name", "rank",
                "total bp",
                "assigned bp",
                "consecutive reads");
        fflush(stdout);
    }
    while ((read = getline(&line, &len, fd)) != -1) {
        if (sscanf(line, "%d|%d-%d\t%d\t%d\t%f", &rgi, &rfrom, &rto, &taxId, &taxGi, &rScore) != 6) {
            fprintf(stderr, "LINE: %s\n", line);
            checkPointerError(NULL, "Bad Taxoner line", __FILE__, __LINE__, -1);
        }
        if (rScore >= score) {
            if (lastTaxId != taxId) {
                if (tax) {
                    checkTaxForContReads(outs, ids, ids_number, tax, fBtree, fFasta, taxDB, readLength, readOffset, verbose);
                    tax->free(tax);
                }
                tax = CreateTaxonerTax();
                tax->taxId = taxId;
                tax->gis = CreateTaxonerGi();
                tax->gis_numbers = 1;
                tax->gis->gi = rgi;
                tax->gis->fromTo = allocate(sizeof (int **) * 1, __FILE__, __LINE__);
                tax->gis->fromTo[0] = allocate(sizeof (int) * 2, __FILE__, __LINE__);
                tax->gis->fromTo[0][0] = rfrom;
                tax->gis->fromTo[0][1] = rto;
                tax->gis->fromTo_number = 1;

                index = allocate(sizeof (int), __FILE__, __LINE__);
                *index = 0;
                tax->gisIndex = BtreeInsert(tax->gisIndex, rgi, index);
                lastTaxId = taxId;
            } else {
                if ((rec = BTreeFind(tax->gisIndex, rgi, false)) != NULL) {
                    index = (int *) rec->value;
                    tax->gis[*index].fromTo = checkPointerError(realloc(tax->gis[*index].fromTo, sizeof (int **) * (tax->gis[*index].fromTo_number + 1)), "Can't reallocate memory", __FILE__, __LINE__, -1);

                    tax->gis[*index].fromTo[tax->gis[*index].fromTo_number] = allocate(sizeof (int) * 2, __FILE__, __LINE__);
                    tax->gis[*index].fromTo[tax->gis[*index].fromTo_number][0] = rfrom;
                    tax->gis[*index].fromTo[tax->gis[*index].fromTo_number][1] = rto;
                    tax->gis[*index].fromTo_number++;
                } else {
                    tax->gis = checkPointerError(realloc(tax->gis, sizeof (struct taxoner_gi_s) * (tax->gis_numbers + 1)), "Can't reallocate memory", __FILE__, __LINE__, -1);
                    InitTaxonerGi(&(tax->gis[tax->gis_numbers]));
                    tax->gis[tax->gis_numbers].gi = rgi;
                    tax->gis[tax->gis_numbers].fromTo = allocate(sizeof (int **) * 1, __FILE__, __LINE__);
                    tax->gis[tax->gis_numbers].fromTo[0] = allocate(sizeof (int) * 2, __FILE__, __LINE__);
                    tax->gis[tax->gis_numbers].fromTo[0][0] = rfrom;
                    tax->gis[tax->gis_numbers].fromTo[0][1] = rto;
                    tax->gis[tax->gis_numbers].fromTo_number = 1;

                    index = allocate(sizeof (int), __FILE__, __LINE__);
                    *index = tax->gis_numbers;
                    tax->gisIndex = BtreeInsert(tax->gisIndex, tax->gis[tax->gis_numbers].gi, index);
                    tax->gis_numbers++;
                }
            }
        }
    }
    if (tax) {
        checkTaxForContReads(outs, ids, ids_number, tax, fBtree, fFasta, taxDB, readLength, readOffset, verbose);
        tax->free(tax);
    }
    if (verbose) {
        printf("\n");
        fflush(stdout);
    }
    for (rgi = 0; rgi < ids_number + 2; rgi++) {
        fclose(outs[rgi]);
    }
    freeArrayofPointers((void **) ids, ids_number);
    free(outs);
    if (line) free(line);
}