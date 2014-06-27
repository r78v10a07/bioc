/* 
 * File:   main.c
 * Author: roberto
 *
 * Created on June 26, 2014, 10:23 AM
 */

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <zlib.h>
#include <time.h>
#include "btree.h"
#include "btime.h"
#include "berror.h"
#include "bmemory.h"
#include "bstring.h"
#include "taxonomy.h"
#include "fasta.h"

char *program_name;

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "\n********************************************************************************\n");
    fprintf(stream, "\nUsage: %s \n", program_name);
    fprintf(stream, "\n\n%s options:\n\n", program_name);
    fprintf(stream, "-v,   --verbose                     Print info\n");
    fprintf(stream, "-h,   --help                        Display this usage information.\n");
    fprintf(stream, "-n,   --nt                          NT fasta file\n");
    fprintf(stream, "-o,   --output                      Output fasta file prefix\n");
    fprintf(stream, "-t,   --taxgi                       File with the gi-taxids (like: gi_taxid_nucl.dmp)\n");
    fprintf(stream, "-d,   --dir                         NCBI Taxonomy db dir\n");
    fprintf(stream, "-s,   --skip                        File with the TaxId to skip\n");
    fprintf(stream, "-i,   --include                     File with the TaxId to include. All children will be included\n");
    fprintf(stream, "-l,   --line                        The fasta line size (default: 80)\n");
    fprintf(stream, "********************************************************************************\n");
    fprintf(stream, "\n            Roberto Vera Alvarez (e-mail: r78v10a07@gmail.com)\n\n");
    fprintf(stream, "********************************************************************************\n");
    exit(0);
}

BtreeNode_t *TaxsToInclude(char *dirName, char *include, char *skip, int verbose) {
    FILE *fd1, *fd2;

    int *lineage, lineage_number, *value;
    void **records = NULL;
    int i, j, records_number = 0;
    BtreeNode_t *taxDB = NULL;
    BtreeNode_t *taxIn = NULL;
    BtreeNode_t *toInTaxId = NULL;
    BtreeNode_t *toSkTaxId = NULL;
    taxonomy_l tax;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    taxDB = TaxonomyDBIndex(dirName, verbose);

    fd2 = NULL;
    fd1 = checkPointerError(fopen(include, "r"), "Can't open include file", __FILE__, __LINE__, -1);
    if (skip)
        fd2 = checkPointerError(fopen(skip, "r"), "Can't open skip file", __FILE__, __LINE__, -1);

    if (verbose) {
        printf("Extracting the records to array\n");
        fflush(stdout);
    }

    while ((read = getline(&line, &len, fd1)) != -1) {
        value = allocate(sizeof (int), __FILE__, __LINE__);
        sscanf(line, "%d", value);
        toInTaxId = BtreeInsert(toInTaxId, *value, value);
    }

    if (fd2) {
        while ((read = getline(&line, &len, fd2)) != -1) {
            value = allocate(sizeof (int), __FILE__, __LINE__);
            sscanf(line, "%d", value);
            toSkTaxId = BtreeInsert(toSkTaxId, *value, value);
        }
    }

    BtreeRecordsToArray(&records, &records_number, taxDB);
    for (i = 0; i < records_number; i++) {
        lineage = NULL;
        lineage_number = 0;
        tax = ((taxonomy_l) records[i]);

        if (BTreeFind(toSkTaxId, tax->taxId, false) == NULL) {

            tax->getLineage(tax, &lineage, &lineage_number, taxDB);
            for (j = 0; j < lineage_number; j++) {
                if (BTreeFind(toInTaxId, lineage[j], false) != NULL) {
                    value = allocate(sizeof (int), __FILE__, __LINE__);
                    *value = tax->taxId;
                    taxIn = BtreeInsert(taxIn, tax->taxId, value);
                    break;
                }
            }

            free(lineage);
        }
    }

    tax = CreateTaxonomy();
    BTreeFree(taxDB, tax->free);
    tax->free(tax);

    BTreeFree(toInTaxId, free);
    BTreeFree(toSkTaxId, free);
    if (records) {
        free(records);
    }
    if (line) free(line);
    fclose(fd1);
    if (fd2)fclose(fd2);
    return taxIn;
}

/*
 * 
 */
int main(int argc, char** argv) {
    struct timespec start, stop, mid;
    int next_option, verbose, count, lineSize, gi;
    const char* const short_options = "vhn:o:t:d:s:i:";
    char *ntName, *output, *taxgiName, *tmp, *dirName, *skipName, *includeName;

    FILE *fd1, *fd2;
    off_t pos, tot;
    float percent;

    BtreeNode_t *taxIn = NULL;
    BtreeNode_t *gi_tax = NULL;
    BtreeRecord_t *rec;

    fasta_l fasta;
    long long int countWords;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "verbose", 0, NULL, 'v'},
        { "nt", 1, NULL, 'n'},
        { "output", 1, NULL, 'o'},
        { "taxgi", 1, NULL, 't'},
        { "dir", 1, NULL, 'd'},
        { "skip", 1, NULL, 's'},
        { "include", 1, NULL, 'i'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    lineSize = 80;
    verbose = 0;
    ntName = output = taxgiName = dirName = skipName = includeName = NULL;
    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        switch (next_option) {
            case 'h':
                print_usage(stdout, 0);

            case 'v':
                verbose = 1;
                break;

            case 'o':
                output = strdup(optarg);
                break;

            case 'n':
                ntName = strdup(optarg);
                break;

            case 't':
                taxgiName = strdup(optarg);
                break;

            case 'd':
                dirName = strdup(optarg);
                break;

            case 's':
                skipName = strdup(optarg);
                break;

            case 'i':
                includeName = strdup(optarg);
                break;

            case 'l':
                lineSize = atoi(optarg);
                break;
        }
    } while (next_option != -1);

    if (!dirName || !output || !ntName || !taxgiName || !includeName) {
        print_usage(stderr, -1);
    }

    taxIn = TaxsToInclude(dirName, includeName, skipName, verbose);

    clock_gettime(CLOCK_MONOTONIC, &mid);
    if (verbose) {
        printf("Reading the Taxonomy-Nucleotide database ... ");
        fflush(stdout);
    }
    gi_tax = TaxonomyNuclIndex(taxgiName, verbose);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    if (verbose) {
        printf("%.1f sec\n", timespecDiffSec(&stop, &mid));
        fflush(stdout);
    }

    count = countWords = 0;
    fd1 = checkPointerError(fopen(ntName, "r"), "Can't open include file", __FILE__, __LINE__, -1);
    tmp = allocate(sizeof (char) * (strlen(output) + 100), __FILE__, __LINE__);
    sprintf(tmp, "%s_%d.fna", output, count);
    if (verbose) printf("Creating a new file: %s\n", tmp);
    fd2 = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);
    
    fseeko(fd1, SEEK_END, 0);
    tot = ftello(fd1);
    fseeko(fd1, SEEK_SET, 0);

    pos = 0;
    while ((fasta = ReadFasta(fd1, 0)) != NULL) {
        fasta->getGi(fasta, &gi);
        if ((rec = BTreeFind(gi_tax, gi, false)) != NULL) {
            if ((rec = BTreeFind(taxIn, *((int *) rec->value), false)) != NULL) {
                sprintf(fasta->header, "%d;%d", gi, *((int *) rec->value));
                countWords += (fasta->length(fasta) + strlen(fasta->header) + 3);
                if (countWords > 4294967296) {
                    count++;
                    countWords = fasta->length(fasta);
                    fclose(fd2);
                    sprintf(tmp, "%s_%d.fna", output, count);
                    if (verbose) printf("Creating a new file: %s\n", tmp);
                    fd2 = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);
                }
                fasta->toFile(fasta, fd2, lineSize);
            }
        }
        percent = (pos * 100) / tot;

        if (verbose) {
            printf("\t%6.2f%% \r", percent);
        }

        fasta->free(fasta);
        pos = ftello(fd1);
    }

    if (verbose) {
        printf("\t%6.2f%% \n", atof("100.00"));
    }

    BTreeFree(gi_tax, free);
    BTreeFree(taxIn, free);
    if (fd1) fclose(fd1);
    if (fd2) fclose(fd2);
    if (dirName) free(dirName);
    if (output) free(output);
    if (ntName) free(ntName);
    if (taxgiName) free(taxgiName);
    if (includeName) free(includeName);
    if (skipName) free(skipName);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %.1f sec\n\n", timespecDiffSec(&stop, &start));
    return (EXIT_SUCCESS);
}

