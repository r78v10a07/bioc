/* 
 * File:   split.c
 * Author: roberto
 *
 * Created on May 23, 2014, 1:53 PM
 */
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <zlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "btree.h"
#include "btime.h"
#include "berror.h"
#include "bmemory.h"
#include "bstring.h"
#include "fasta.h"

#define SIZE 3072

typedef struct thread_param {
    int number;
    void * self;
    char *out;
    int start;
    int end;
    int length;
    int offset;
    int lineLength;
    void **res;
    int resNumber;
} thread_param_t;

extern void *pthreadSplitInSegments(void *arg);
extern void *pthreadSplitInSegmentsInMem(void *arg);

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
void splitInSegmentsLocal(void * self, FILE *out, int length, int offset, int lineLength, int threads_number, int inMem, int tax) {
    _CHECK_SELF_P(self);
    int i, j, reads, numPerThread, thread_cr_res, thread_join_res;
    pthread_t *threads = allocate(sizeof (pthread_t) * threads_number, __FILE__, __LINE__);
    thread_param_t *tp = allocate(sizeof (thread_param_t) * threads_number, __FILE__, __LINE__);
    char **tFiles = NULL;
    pid_t pid = getpid();
    size_t bytes;
    char buffer[SIZE];
    FILE *fd;
    int gi, from, taxId;

    if (inMem == 0) {
        tFiles = allocate(sizeof (char **) * threads_number, __FILE__, __LINE__);
    }

    if (tax) {
        ((fasta_l) self)->header = reallocate(((fasta_l) self)->header,
                sizeof (char) * (strlen(((fasta_l) self)->header) + 100), __FILE__, __LINE__);
        sscanf(((fasta_l) self)->header, "%d;%d", &gi, &taxId);
        sprintf(((fasta_l) self)->header, "gi|%d", gi);
    }

    reads = ((fasta_l) self)->len / offset;
    numPerThread = reads / threads_number;

    for (i = 0; i < threads_number; i++) {
        fflush(NULL);
        if (inMem == 0) {
            tFiles[i] = allocate(sizeof (char *) * 150, __FILE__, __LINE__);
            sprintf(tFiles[i], "%d_%d.fna", pid, i);
            tp[i].out = tFiles[i];
        }

        tp[i].number = i;
        tp[i].length = length;
        tp[i].lineLength = lineLength;
        tp[i].offset = offset;
        tp[i].self = self;
        tp[i].start = i * numPerThread * offset;
        tp[i].res = NULL;
        tp[i].resNumber = 0;
        if (i < threads_number - 1) {
            tp[i].end = (i + 1) * numPerThread * offset;
        } else {
            tp[i].end = ((fasta_l) self)->len;
        }

        if (inMem == 0) {
            thread_cr_res = pthread_create(&threads[i],
                    NULL,
                    pthreadSplitInSegments,
                    (void*) &(tp[i]));
        } else {
            thread_cr_res = pthread_create(&threads[i],
                    NULL,
                    pthreadSplitInSegmentsInMem,
                    (void*) &(tp[i]));
        }
        if (thread_cr_res != 0) {
            checkPointerError(NULL, "THREAD CREATE ERROR", __FILE__, __LINE__, -1);
        }
    }

    for (i = 0; i < threads_number; i++) {
        fflush(NULL);
        thread_join_res = pthread_join(threads[i], NULL);
        if (thread_join_res != 0) {
            checkPointerError(NULL, "JOIN ERROR", __FILE__, __LINE__, -1);
        }
        if (inMem == 0) {
            fd = checkPointerError(fopen(tFiles[i], "r"), "Can't open temporal file", __FILE__, __LINE__, -1);

            while (0 < (bytes = fread(buffer, 1, sizeof (buffer), fd))) {
                fwrite(buffer, 1, bytes, out);
            }
            fclose(fd);
            remove(tFiles[i]);
            if (tFiles[i]) free(tFiles[i]);
        } else {
            for (j = 0; j < tp[i].resNumber; j++) {
                if (strstr(((fasta_l) tp[i].res[j])->seq, "NNNNN") == NULL) {
                    if (tax) {
                        sscanf(((fasta_l) tp[i].res[j])->header, "%d|%d", &gi, &from);
                        ((fasta_l) tp[i].res[j])->header = reallocate(((fasta_l) tp[i].res[j])->header, sizeof (char) * (strlen(((fasta_l) tp[i].res[j])->header) + 100), __FILE__, __LINE__);
                        sprintf(((fasta_l) tp[i].res[j])->header, "%d-%d;%d", gi, from, taxId);
                    }
                    ((fasta_l) tp[i].res[j])->toFile(tp[i].res[j], out, lineLength);
                    ((fasta_l) tp[i].res[j])->free(tp[i].res[j]);
                }
            }
            free(tp[i].res);
        }
    }
    if (tp) free(tp);
    if (tFiles)free(tFiles);
    if (threads)free(threads);
}