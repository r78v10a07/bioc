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
#include <pthread.h>
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

typedef struct thread_param {
    int number;
    char *out;
    char *in;
    off_t start;
    off_t end;
    BtreeNode_t *taxIn;
    BtreeNode_t *gi_tax;
    int verbose;
} thread_param_t;

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
    fprintf(stream, "-p,   --threads                     NUmber of threads (default: 2)\n");
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

void *pthreadTaxFilter(void *arg) {
    thread_param_t *parms = ((thread_param_t*) arg);

    FILE *fd1 = checkPointerError(fopen(parms->in, "r"), "Can't open include file", __FILE__, __LINE__, -1);
    FILE *fd2 = checkPointerError(fopen(parms->out, "w"), "Can't open include file", __FILE__, __LINE__, -1);
    BtreeNode_t *taxIn = parms->taxIn;
    BtreeNode_t *gi_tax = parms->gi_tax;
    BtreeRecord_t *rec;

    int bufferSize, excludeSeq, gi, ends;
    size_t buf_size;
    char *buffer, *line, *str;
    int numLines, header;

    off_t pos;

    pos = parms->start;
    fseeko(fd1, parms->start, SEEK_SET);

    bufferSize = 8000000;
    if (parms->end - parms->start < bufferSize)
        bufferSize = parms->end - parms->start;

    excludeSeq = 1;
    buf_size = sizeof (char) * bufferSize;
    buffer = allocate(sizeof (char) * (bufferSize + 1), __FILE__, __LINE__);
    numLines = ends = header = 0;
    gi = -1;
    while (!feof(fd1)) {
        memset(buffer, 0, buf_size);
        if (fread(buffer, buf_size, 1, fd1) == 1) {
            buffer[bufferSize] = '\0';
        }
        str = buffer;
        while (1) {
            line = strchr(str, '\n');
            if (line) *line = '\0';
            if (*str != '\0') {
                if (*str == '>' || header) {
                    if (ends == 2) {
                        ends = 3;
                        break;
                    }
                    if (!header) {
                        excludeSeq = 1;
                        sscanf(str, ">gi|%d|", &gi);
                        if ((rec = BTreeFind(gi_tax, gi, false)) != NULL) {
                            if ((rec = BTreeFind(taxIn, *((int *) rec->value), false)) != NULL) {
                                fprintf(fd2, ">%d;%d\n", gi, *((int *) rec->value));
                                excludeSeq = 0;
                            }
                        }
                    }
                    if (line) {
                        header = 0;
                    } else {
                        header = 1;
                    }
                } else {
                    if (gi != -1 && !excludeSeq) {
                        fprintf(fd2, "%s", str);
                        if (line) {
                            fprintf(fd2, "\n");
                        }
                    }
                }
            }
            if (!line) break;
            str = line + 1;
            numLines++;
        }
        if (ends == 1) ends = 2;
        pos = ftello(fd1);
        if (buf_size >= parms->end - pos && ends == 0) {
            bufferSize = parms->end - pos;
            buf_size = sizeof (char) * bufferSize;
            ends = 1;
        }
        if (ends == 3) break;
    }
    free(buffer);
    fclose(fd1);
    fclose(fd2);
    return NULL;
}

/*
 * 
 */
int main(int argc, char** argv) {
    struct timespec start, stop, mid;
    int i, next_option, verbose, count, pthreads;
    const char* const short_options = "vhn:o:t:d:s:i:p:";
    char *ntName, *output, *taxgiName, *tmp, *dirName, *skipName, *includeName;

    FILE *fd1, *fd2;
    off_t tot, perThread;

    pthread_t *threads;
    thread_param_t *tp;
    int thread_join_res;

    BtreeNode_t *taxIn = NULL;
    BtreeNode_t *gi_tax = NULL;
    long long int countWords;

    int bufferSize;
    size_t buf_size;
    char *buffer, *line, *str;

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
        { "threads", 1, NULL, 'p'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    pthreads = 2;
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

            case 'p':
                pthreads = atoi(optarg);
                break;
        }
    } while (next_option != -1);

    if (!dirName || !output || !ntName || !taxgiName || !includeName) {
        print_usage(stderr, -1);
    }

    if (pthreads < 2) pthreads = 2;

    threads = allocate(sizeof (pthread_t) * pthreads, __FILE__, __LINE__);
    tp = allocate(sizeof (thread_param_t) * pthreads, __FILE__, __LINE__);

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
    sprintf(tmp, "%s_%d.fasta", output, count);
    if (verbose) printf("Creating a new file: %s\n", tmp);
    fd2 = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);

    fseeko(fd1, 0, SEEK_END);
    tot = ftello(fd1);
    fseeko(fd1, 0, SEEK_SET);
    fclose(fd1);

    perThread = tot / pthreads;

    if (verbose)
        printf("Fasta file's size: %lu\tSplit in: %lu\n", tot, perThread);

    for (i = 0; i < pthreads; i++) {
        tp[i].number = i;
        tp[i].out = allocate(sizeof (char) * (strlen(output) + 20), __FILE__, __LINE__);
        sprintf(tp[i].out, "%s_%d_p.fasta", output, i);
        tp[i].in = ntName;
        tp[i].gi_tax = gi_tax;
        tp[i].taxIn = taxIn;
        tp[i].verbose = verbose;
        tp[i].start = i * perThread;
        if (i == pthreads - 1) {
            tp[i].end = tot;
        } else {
            tp[i].end = (i + 1) * perThread;
        }
        pthread_create(&threads[i], NULL, pthreadTaxFilter, (void*) &(tp[i]));
    }

    bufferSize = 8000000;
    buf_size = sizeof (char) * bufferSize;
    buffer = allocate(sizeof (char) * (bufferSize + 1), __FILE__, __LINE__);

    for (i = 0; i < pthreads; i++) {
        thread_join_res = pthread_join(threads[i], NULL);
        if (thread_join_res != 0) {
            checkPointerError(NULL, "JOIN ERROR", __FILE__, __LINE__, -1);
        } else {
            printf("Thread %d ends\n", i);
            fd1 = checkPointerError(fopen(tp[i].out, "r"), "Can't open include file", __FILE__, __LINE__, -1);
            while (!feof(fd1)) {
                memset(buffer, 0, buf_size);
                if (fread(buffer, buf_size, 1, fd1) == 1) {
                    buffer[bufferSize] = '\0';
                }

                str = buffer;
                while (1) {
                    line = strchr(str, '\n');
                    if (line) *line = '\0';
                    if (*str != '\0') {
                        if (*str == '>') {
                            if (countWords > 4294967296) {
                                count++;
                                countWords = 0;
                                fclose(fd2);
                                sprintf(tmp, "%s_%d.fasta", output, count);
                                if (verbose) printf("Creating a new file: %s\n", tmp);
                                fd2 = checkPointerError(fopen(tmp, "w"), "Can't open output file", __FILE__, __LINE__, -1);
                            }
                            countWords += strlen(str);
                            fprintf(fd2, "%s", str);
                            if (line) {
                                countWords++;
                                fprintf(fd2, "\n");
                            }
                        } else {
                            countWords += strlen(str);
                            fprintf(fd2, "%s", str);
                            if (line) {
                                countWords++;
                                fprintf(fd2, "\n");
                            }
                        }
                    }
                    if (!line) break;
                    str = line + 1;
                }
            }
            fclose(fd1);
        }
        free(tp[i].out);
    }
    free(tp);
    if (threads) free(threads);
    if (tmp) free(tmp);
    BTreeFree(gi_tax, free);
    BTreeFree(taxIn, free);
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

