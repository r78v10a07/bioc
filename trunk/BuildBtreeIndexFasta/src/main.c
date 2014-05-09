/* 
 * File:   main.c
 * Author: roberto
 *
 * Created on May 7, 2014, 3:41 PM
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

char *program_name;

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "\n********************************************************************************\n");
    fprintf(stream, "\nUsage: %s \n", program_name);
    fprintf(stream, "\n\n%s options:\n\n", program_name);
    fprintf(stream, "-v,   --verbose                     Print info\n");
    fprintf(stream, "-h,   --help                        Display this usage information.\n");
    fprintf(stream, "-i,   --input                       The input fasta file\n");
    fprintf(stream, "-o,   --output                      The output binary file as index\n");
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
    int next_option, verbose, gzip;
    const char* const short_options = "vhi:o:";
    char *input, *output;
    FILE *fo;
    FILE *fd = NULL;
    gzFile gFile = NULL;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "input", 1, NULL, 'i'},
        { "output", 1, NULL, 'o'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    verbose = gzip = 0;
    input = output = NULL;
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
                gzip = 1 - strbcmp(input, ".gz");
                break;
        }
    } while (next_option != -1);

    if (!input || !output) {
        print_usage(stderr, -1);
    }

    if (!gzip) {
        fd = checkPointerError(fopen(input, "r"), "Can't open input file", __FILE__, __LINE__, -1);
    } else {
        gFile = checkPointerError(gzopen(input, "rb"), "Can't open the input file", __FILE__, __LINE__, -1);
    }
    fo = checkPointerError(fopen(output, "wb"), "Can't open output file", __FILE__, __LINE__, -1);
    if (!gzip) {
        CreateFastaIndexToFile(fd, fo, verbose);
    } else {
        CreateFastaIndexGzipToFile(gFile, fo, verbose);
    }
    fclose(fo);

    if (!gzip) {
        fclose(fd);
    } else {
        gzclose(gFile);
    }
    if (input) free(input);
    if (output) free(output);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %lu sec\n\n", timespecDiffSec(&stop, &start));
    return (EXIT_SUCCESS);
}

