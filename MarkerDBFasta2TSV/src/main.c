/* 
 * File:   main.c
 * Author: roberto
 *
 * Created on May 20, 2014, 5:08 PM
 */

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <zlib.h>
#include "bmemory.h"
#include "bstring.h"
#include "berror.h"
#include "btree.h"
#include "btime.h"
#include "fasta.h"

char *program_name;

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "\n********************************************************************************\n");
    fprintf(stream, "\nUsage: %s \n", program_name);
    fprintf(stream, "\n\n%s options:\n\n", program_name);
    fprintf(stream, "-v,   --verbose                     Print info\n");
    fprintf(stream, "-h,   --help                        Display this usage information.\n");
    fprintf(stream, "-i,   --input                       The fasta file\n");
    fprintf(stream, "-o,   --output                      The output TSV file\n");
    fprintf(stream, "-a,   --add                         Add to the output\n");
    fprintf(stream, "********************************************************************************\n");
    fprintf(stream, "\n            Roberto Vera Alvarez (e-mail: r78v10a07@gmail.com)\n\n");
    fprintf(stream, "********************************************************************************\n");
    exit(0);
}

/*
 * 
 */
int main(int argc, char** argv) {

    struct timespec start, stop;
    int i, next_option, verbose, add, gi, from, to;
    const char* const short_options = "vhi:o:a";
    char *input, *output;
    FILE *in;
    FILE *out;
    fasta_l fasta;
    off_t pos, length;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "verbose", 0, NULL, 'v'},
        { "input", 1, NULL, '1'},
        { "output", 1, NULL, 'o'},
        { "add", 0, NULL, 'a'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    verbose = add = 0;
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
                break;

            case 'a':
                add = 1;
                break;
        }
    } while (next_option != -1);

    if (!input || !output) {
        print_usage(stderr, -1);
    }

    in = checkPointerError(fopen(input, "r"), "Can't open output file", __FILE__, __LINE__, -1);
    if (add) {
        out = checkPointerError(fopen(output, "a"), "Can't open output file", __FILE__, __LINE__, -1);
    } else {
        out = checkPointerError(fopen(output, "w"), "Can't open output file", __FILE__, __LINE__, -1);
    }

    pos = 0;
    fseeko(in, 0, SEEK_END);
    length = ftello(in);
    fseeko(in, 0, SEEK_SET);

    i = 1;
    while ((fasta = ReadFasta(in, 0)) != NULL) {
        if (sscanf(fasta->header, "%d|%d-%d\n", &gi, &from, &to) != 3) {
            fprintf(stderr, "Bad header format:\n>gi|from-to\n%s", fasta->header);
            checkPointerError(NULL, "ERORR!!", __FILE__, __LINE__, -1);
        }
        if (verbose) {
            printf("%6.2f %%\t\t%6d\t%10d\t%5d\t%5d\r", (float) ((float) pos * (float) 100 / (float) length), i, gi, from, to);
            fflush(stdout);
        }
        fprintf(out, "%d\t%d\t%d\t%s\n", gi, from, to, fasta->seq);
        fasta->free(fasta);
        i++;
        pos = ftello(in);
    }
    if (verbose) {
        printf("100.0 %%\t\t%6d\t%10d\t%5d\t%5d\n", i, gi, from, to);
        fflush(stdout);
    }

    fclose(in);
    fclose(out);
    free(input);
    free(output);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %.1f sec\n\n", timespecDiffSec(&stop, &start));
    return (EXIT_SUCCESS);
}

