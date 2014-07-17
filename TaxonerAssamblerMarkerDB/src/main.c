/* 
 * File:   main.c
 * Author: roberto
 *
 * Created on May 8, 2014, 8:51 AM
 */

#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <zlib.h>
#include <stdbool.h>
#include "btime.h"
#include "berror.h"
#include "bmemory.h"
#include "bstring.h"
#include "btree.h"
#include "fasta.h"
#include "taxonomy.h"
#include "taxoner.h"

char *program_name;

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "\n********************************************************************************\n");
    fprintf(stream, "\nUsage: %s \n", program_name);
    fprintf(stream, "\n\n%s options:\n\n", program_name);
    fprintf(stream, "-v,   --verbose                     Print info\n");
    fprintf(stream, "-h,   --help                        Display this usage information.\n");
    fprintf(stream, "-i,   --input                       The input Taxoner out file (Taxonomy.txt)\n");
    fprintf(stream, "-o,   --output                      The output directory\n");
    fprintf(stream, "-t,   --tax                         The NCBI Taxonomy DB directory\n");
    fprintf(stream, "-f,   --fasta                       Fasta file with the sequences\n");
    fprintf(stream, "-n,   --index                       Fasta file index file (optional, it can be created by this program)\n");
    fprintf(stream, "-s,   --score                       Cutoff score to use the read (default: 0.90)\n");
    fprintf(stream, "-l,   --readlength                  The length of the reads (default: 100)\n");
    fprintf(stream, "-z,   --readOffset                  The offset used to overlap the reads (default: 75)\n");
    fprintf(stream, "-p,   --print                       Coma separated list of taxonomy rank to print fasta (example: \"no rank,species,genus\", default not printing)\n");
    fprintf(stream, "-a,   --pattern                     Pattern to extract the gi from the fasta header (\">%%d;\")\n");
    fprintf(stream, "********************************************************************************\n");
    fprintf(stream, "\n            Roberto Vera Alvarez (e-mail: r78v10a07@gmail.com)\n\n");
    fprintf(stream, "********************************************************************************\n");
    exit(0);
}

/*
 * Main function
 */
int main(int argc, char** argv) {

    struct timespec start, stop;
    int next_option, verbose, gInputFlag, gFastaFlag;
    const char* const short_options = "vhi:o:f:n:s:t:l:z:p:a:";
    char *input, *output, *fasta, *index, *taxDir, *giPattern;
    float score;
    FILE *fInput, *fFasta, *fIndex;
    gzFile gInput, gFasta;
    BtreeNode_t *taxDB = NULL;
    BtreeNode_t *fBtree = NULL;
    taxonomy_l tax;
    int readLength, readOffset;
    char *rankToPrint;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "verbose", 0, NULL, 'v'},
        { "input", 1, NULL, 'i'},
        { "output", 1, NULL, 'o'},
        { "tax", 1, NULL, 't'},
        { "fasta", 1, NULL, 'f'},
        { "index", 1, NULL, 'n'},
        { "score", 1, NULL, 's'},
        { "readlength", 1, NULL, 'l'},
        { "readOffset", 1, NULL, 'z'},
        { "print", 1, NULL, 'p'},
        { "pattern", 1, NULL, 'a'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    score = 0.90;
    readLength = 100;
    readOffset = 75;
    verbose = gInputFlag = gFastaFlag = 0;
    input = output = fasta = index = taxDir = giPattern = NULL;
    fInput = fFasta = fIndex = NULL;
    gInput = gFasta = NULL;
    rankToPrint = NULL;
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

            case 't':
                taxDir = strdup(optarg);
                break;

            case 'i':
                input = strdup(optarg);
                gInputFlag = 1 - strbcmp(input, ".gz");
                break;

            case 'f':
                fasta = strdup(optarg);
                gFastaFlag = 1 - strbcmp(fasta, ".gz");
                break;

            case 'p':
                rankToPrint = strdup(optarg);
                break;

            case 'n':
                index = strdup(optarg);
                break;

            case 's':
                score = atof(optarg);
                break;

            case 'l':
                readLength = atoi(optarg);
                break;

            case 'z':
                readOffset = atoi(optarg);
                break;

            case 'a':
                giPattern = strdup(optarg);
                break;
        }
    } while (next_option != -1);

    if (!input || !output) {
        print_usage(stderr, -1);
    }

    if (!gInputFlag) {
        fInput = checkPointerError(fopen(input, "r"), "Can't open input file", __FILE__, __LINE__, -1);
    } else {
        gInput = checkPointerError(gzopen(input, "rb"), "Can't open the input file", __FILE__, __LINE__, -1);
    }

    if (!gFastaFlag) {
        fFasta = checkPointerError(fopen(fasta, "r"), "Can't open input file", __FILE__, __LINE__, -1);
    } else {
        gFasta = checkPointerError(gzopen(fasta, "rb"), "Can't open the input file", __FILE__, __LINE__, -1);
    }
    if (index) {
        fIndex = checkPointerError(fopen(index, "r"), "Can't open input file", __FILE__, __LINE__, -1);
        fBtree = CreateBtreeFromIndex(fIndex, verbose);
        fclose(fIndex);
    } else {
        if (!gFastaFlag) {
            fBtree = CreateBtreeFromFastawithPattern(fFasta, giPattern, verbose);
        } else {
            fBtree = CreateBtreeFromFastaGzip(gFasta, verbose);
        }
    }
    taxDB = TaxonomyDBIndex(taxDir, verbose);

    ParseTaxonerResult(output, rankToPrint, fInput, score, fBtree, fFasta, taxDB, readLength, readOffset, verbose);

    if (!gInputFlag) {
        fclose(fInput);
    } else {
        gzclose(gInput);
    }
    if (!gFastaFlag) {
        fclose(fFasta);
    } else {
        gzclose(gFasta);
    }

    BTreeFree(fBtree, free);

    tax = CreateTaxonomy();
    BTreeFree(taxDB, tax->free);
    tax->free(tax);

    if (giPattern) free(giPattern);
    if (rankToPrint) free(rankToPrint);
    if (input) free(input);
    if (output) free(output);
    if (fasta) free(fasta);
    if (index) free(index);
    if (taxDir) free(taxDir);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %.1f sec\n\n", timespecDiffSec(&stop, &start));
    return (EXIT_SUCCESS);
}

