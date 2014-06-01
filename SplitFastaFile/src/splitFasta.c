/* 
 * File:   splitFasta.c
 * Author: roberto
 *
 * Created on April 14, 2014, 7:16 PM
 */
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <zlib.h>
#include <stdbool.h>
#include "btree.h"
#include "btime.h"
#include "berror.h"
#include "bmemory.h"
#include "bstring.h"
#include "fasta.h"
#include "taxonomy.h"

char *program_name;

/**
 * Creates segments of length with an overlap of offset
 * 
 * @param self the container object
 * @param out the output file 
 * @param length the length of the segments
 * @param offset the offset of the segments
 * @param lineLength the length of the fasta line
 * @param threads_number Number of threads
 * @param inMem du the generation in memory
 */
void splitInSegmentsLocal(void * self, FILE *out, int length, int offset, int lineLength, int threads_number, int inMem, int tax);

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "\n********************************************************************************\n");
    fprintf(stream, "\nUsage: %s \n", program_name);
    fprintf(stream, "\n\n%s options:\n\n", program_name);
    fprintf(stream, "-v,   --verbose                     Print info\n");
    fprintf(stream, "-h,   --help                        Display this usage information.\n");
    fprintf(stream, "-i,   --input                       The input fasta file\n");
    fprintf(stream, "-o,   --output                      The output fasta file\n");
    fprintf(stream, "-l,   --length                      The reads length\n");
    fprintf(stream, "-f,   --offset                      The overlaping offset\n");
    fprintf(stream, "-s,   --size                        The fasta line size (default: 80)\n");
    fprintf(stream, "-p,   --pthread                     The number of threads (default: 2)\n");
    fprintf(stream, "-t,   --split                       Split the result fasta file. Value in Gb (Ex: --split 2, not set for not split)\n");
    fprintf(stream, "-m,   --mem                         Do the pthread work in memory\n");
    fprintf(stream, "-n,   --name                        Just rename fasta file\n");
    fprintf(stream, "-r,   --parser                      Sscanf format to parse the fasta header (Don't use it for default fasta header)\n");
    fprintf(stream, "-g,   --gi                          The GenBank Gi files\n");
    fprintf(stream, "********************************************************************************\n");
    fprintf(stream, "\n            Roberto Vera Alvarez (e-mail: r78v10a07@gmail.com)\n\n");
    fprintf(stream, "********************************************************************************\n");
    exit(0);
}

/*
 * Main function
 */
int main(int argc, char** argv) {
    fasta_l fasta;

    struct timespec start, stop, mid;
    int i, next_option, verbose;
    const char* const short_options = "vhi:o:l:f:s:p:t:mnr:g:";
    char *input, *output, *tmp, *headerParser, *giName;
    int length, offset, size, threads, count, mem, name;
    FILE *fo;
    FILE *fd;
    char **ids = NULL;
    int ids_number, gi, fromTo, tax;
    long long int countWords;
    long long int split;
    BtreeNode_t *gi_tax = NULL;
    BtreeRecord_t *rec;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "verbose", 0, NULL, 'v'},
        { "input", 1, NULL, 'i'},
        { "output", 1, NULL, 'o'},
        { "length", 1, NULL, 'l'},
        { "offset", 1, NULL, 'f'},
        { "size", 1, NULL, 's'},
        { "pthread", 1, NULL, 'p'},
        { "split", 1, NULL, 't'},
        { "mem", 0, NULL, 'm'},
        { "name", 0, NULL, 'n'},
        { "tax", 0, NULL, 'x'},
        { "parser", 0, NULL, 'r'},
        { "gi", 1, NULL, 'g'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    verbose = split = countWords = count = mem = 0;
    input = output = tmp = headerParser = giName = NULL;
    size = 80;
    length = offset = name = 0;
    threads = 1;
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

            case 'i':
                input = strdup(optarg);
                break;

            case 'g':
                giName = strdup(optarg);
                break;

            case 'l':
                length = atoi(optarg);
                break;

            case 'f':
                offset = atoi(optarg);
                break;

            case 's':
                size = atoi(optarg);
                break;

            case 'p':
                threads = atoi(optarg);
                break;

            case 't':
                split = atoi(optarg);
                break;

            case 'm':
                mem = 1;
                break;

            case 'n':
                name = 1;
                break;

            case 'r':
                headerParser = strdup(optarg);
                break;
        }
    } while (next_option != -1);

    if (!input || !output) {
        print_usage(stderr, -1);
    }

    fd = checkPointerError(fopen(input, "r"), "Can't open input file", __FILE__, __LINE__, -1);

    if (split == 0) {
        fo = checkPointerError(fopen(output, "w"), "Can't open output file", __FILE__, __LINE__, -1);
    } else {
        tmp = allocate(sizeof (char) * (strlen(output) + 100), __FILE__, __LINE__);
        sprintf(tmp, "%s_%d.fna", output, count);
        if (verbose) printf("Creating a new file: %s\n", tmp);
        fo = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);
    }

    if (!headerParser) {
        headerParser = allocate(sizeof (char) * 10, __FILE__, __LINE__);
        sprintf(headerParser, "gi|%%d|");
    }

    if (giName) {
        clock_gettime(CLOCK_MONOTONIC, &mid);
        if (verbose) printf("Reading the Taxonomy-Nucleotide database ... ");
        fflush(stdout);
        gi_tax = TaxonomyNuclIndex(giName, verbose);
        clock_gettime(CLOCK_MONOTONIC, &stop);
        if (verbose) printf("%.1f sec\n", timespecDiffSec(&stop, &mid));
        fflush(stdout);
    }

    clock_gettime(CLOCK_MONOTONIC, &mid);
    while ((fasta = ReadFasta(fd, 0)) != NULL) {
        clock_gettime(CLOCK_MONOTONIC, &stop);
        if (verbose) printf("Read sequence of size: %8d in %4.2f sec\n", fasta->len, timespecDiffSec(&stop, &mid));
        if (name == 0 && !giName) {
            if (split != 0) {
                countWords += fasta->length(fasta);
                if (countWords >= split * 1024 * 1024 * 1024) {
                    count++;
                    countWords = fasta->length(fasta);
                    fclose(fo);
                    sprintf(tmp, "%s_%d.fna", output, count);
                    if (verbose) printf("Creating a new file: %s\n", tmp);
                    fo = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);
                }
            }
            fasta->splitInSegments(fasta, fo, headerParser, length, offset, size, threads, mem);
        } else if (name && !giName){
            gi = fromTo = -1;
            ids_number = splitString(&ids, ((fasta_l) fasta)->header, "|");
            for (i = 0; i < ids_number; i++) {
                if (strcmp(ids[i], "gi") == 0) {
                    gi = i + 1;
                }
                if (strcmp(ids[i], "from-to") == 0) {
                    fromTo = i + 1;
                }
            }
            if (gi == -1 || gi >= ids_number || fromTo == -1 || fromTo >= ids_number) {
                fasta->toString(fasta, size);
                checkPointerError(NULL, "Error reading header", __FILE__, __LINE__, -1);
            }
            memset(fasta->header, 0, strlen(fasta->header));
            sprintf(fasta->header, "%s|%s", ids[gi], ids[fromTo]);
            fasta->toFile(fasta, fo, size);

            freeArrayofPointers((void **) ids, ids_number);
        }else if(giName){
            if (sscanf(fasta->header,headerParser,&gi) != 1){
                fasta->toString(fasta, size);
                checkPointerError(NULL, "Error reading header", __FILE__, __LINE__, -1);
            }
            if ((rec = BTreeFind(gi_tax, gi, false)) != NULL) {
                tax = *((int *) rec->value);
                fasta->header = reallocate(fasta->header, sizeof(char) * (strlen(fasta->header) + 50),__FILE__, __LINE__);
                sprintf(fasta->header,"%s;%d",fasta->header, tax);
                fasta->toFile(fasta, fo, size);
            }
        }
        fasta->free(fasta);
        clock_gettime(CLOCK_MONOTONIC, &mid);
    }
    
    if (giName) {
        BTreeFree(gi_tax, free);
        free(giName);
    }

    fclose(fd);
    fclose(fo);
    if (headerParser) free(headerParser);
    if (tmp) free(tmp);
    if (input) free(input);
    if (output) free(output);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %.1f sec\n\n", timespecDiffSec(&stop, &start));
    return (EXIT_SUCCESS);
}

