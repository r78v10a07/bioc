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
#include "memory.h"

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
    fprintf(stream, "-p,   --pthread                     The number of threads (default: 2)\n");
    fprintf(stream, "-t,   --split                       Split the result fasta file. Value in Gb (Ex: --split 2, not set for not split)\n");
    fprintf(stream, "-m,   --mem                         Do the pthread work in memory\n");
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
    const char* const short_options = "hi:o:l:f:s:p:t:m";
    char *input, *output, *tmp;
    int length, offset, size, threads, split, count, countWords, mem;
    FILE *fo;
    FILE *fd;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "input", 1, NULL, 'i'},
        { "output", 1, NULL, 'o'},
        { "length", 1, NULL, 'l'},
        { "offset", 1, NULL, 'f'},
        { "size", 1, NULL, 's'},
        { "pthread", 1, NULL, 'p'},
        { "split", 1, NULL, 't'},
        { "mem", 0, NULL, 'm'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    write = verbose = split = countWords = count = mem = 0;
    input = output = tmp = NULL;
    size = 80;
    length = offset = 0;
    threads = 2;
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

            case 'p':
                threads = atoi(optarg);
                break;

            case 't':
                split = atoi(optarg);
                break;

            case 'm':
                mem = 1;
                break;
        }
    } while (next_option != -1);

    if (!input || !output || length <= 0 || offset <= 0 || size <= 0) {
        print_usage(stderr, -1);
    }

    fd = checkPointerError(fopen(input, "r"), "Can't open input file", __FILE__, __LINE__, -1);

    if (split == 0) {
        fo = checkPointerError(fopen(output, "w"), "Can't open output file", __FILE__, __LINE__, -1);
    } else {
        tmp = allocate(sizeof (char) * (strlen(output) + 100), __FILE__, __LINE__);
        sprintf(tmp, "%s_%d.fna", output, count);
        fo = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);
    }
    while ((fasta = ReadFasta(fd)) != NULL) {
        printf("Read sequence of size: %d\n", fasta->len);
        if (split != 0) {
            countWords += fasta->length(fasta);
            if (countWords >= (split * 1024 * 1024 * 1024)) {
                count++;
                countWords = fasta->length(fasta);
                fclose(fo);
                sprintf(tmp, "%s_%d.fna", output, count);
                fo = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);
            }
        }
        if (threads != 0) {
            fasta->printOverlapSegmentsPthread(fasta, fo, length, offset, size, threads, mem);
        } else {
            fasta->printOverlapSegments(fasta, fo, length, offset, size);
        }
        fasta->free(fasta); 
    }

    fclose(fd);
    fclose(fo);
    if (tmp) free(tmp);
    if (input) free(input);
    if (output) free(output);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %lu sec\n\n", timespecDiff(&stop, &start) / CLOCKS_PER_SEC);
    return (EXIT_SUCCESS);
}

