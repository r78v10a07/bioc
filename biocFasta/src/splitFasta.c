/* 
 * File:   splitFasta.c
 * Author: roberto
 *
 * Created on April 14, 2014, 7:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "fasta.h"
#include "btime.h"
#include "error.h"

char *program_name;

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "\n********************************************************************************\n");
    fprintf(stream, "\nUsage: %s \n", program_name);
    fprintf(stream, "\n\n%s options:\n\n", program_name);
    fprintf(stream, "-h,   --help                        Display this usage information.\n");
    fprintf(stream, "-i,   --input                       The input fasta file\n");
    fprintf(stream, "-o,   --output                      The output fasta file\n");
    fprintf(stream, "-l,   --length                      The reads length\n");
    fprintf(stream, "-f,   --offset                      The overlaping offset\n");
    fprintf(stream, "-s,   --size                        The fasta line size (default: 80)\n");
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

    struct timespec start, stop;
    int next_option, verbose, write;
    const char* const short_options = "hi:o:l:f:s:";
    char *input, *output;
    int length, offset, size;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "input", 1, NULL, 'i'},
        { "output", 1, NULL, 'o'},
        { "length", 1, NULL, 'l'},
        { "offset", 1, NULL, 'f'},
        { "size", 1, NULL, 's'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    write = verbose = 0;
    input = output = NULL;
    size = 80;
    length = offset = 0;
    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        switch (next_option) {
            case 'h':
                print_usage(stdout, 0);

            case 'o':
                output = strdup(optarg);
                break;

            case 'i':
                input = strdup(optarg);
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
        }
    } while (next_option != -1);

    if (!input || !output || length <= 0 || offset <= 0 || size <= 0) {
        print_usage(stderr, -1);
    }

    FILE *fd = checkPointerError(fopen(input, "r"),"Can't open input file",__FILE__,__LINE__, -1);
    FILE *fo = checkPointerError(fopen(output, "w"),"Can't open output file",__FILE__,__LINE__, -1);
    
    while ((fasta = ReadFasta(fd)) != NULL) {
        fasta->printOverlapSegments(fasta, fo, length, offset, size);

        fasta->free(fasta);
    }

    fclose(fd);
    fclose(fo);
    if (input) free(input);
    if (output) free(output);clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %lu sec\n\n", timespecDiff(&stop, &start) / 1000000000);
    return (EXIT_SUCCESS);
}

