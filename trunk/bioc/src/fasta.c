/* 
 * File:   fasta.c
 * Author: roberto
 *
 * Created on April 14, 2014, 11:59 AM
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "memory.h"
#include "bstring.h"
#include "fasta.h"
#include "error.h"

#define SIZE 3072
#define _CHECK_SELF_P(s) checkPointerError(s, "Null self pointer", __FILE__, __LINE__, -1)

typedef struct thread_param {
    int number;
    void * self;
    char *out;
    int start;
    int end;
    int length;
    int offset;
    int lineLength;
} thread_param_t;

/**
 * Print in fasta format with a line length of lineLength 
 * 
 * @param self the container object
 * @param out the output file
 * @param lineLength the length of the fasta line
 */
void toFile(void * self, FILE *out, int lineLength) {
    _CHECK_SELF_P(self);
    int i = 0;
    int j = 0;
    fprintf(out, ">%s\n", ((fasta_l) self)->header);
    while (i < ((fasta_l) self)->length(self)) {
        if (j == lineLength) {
            fprintf(out, "\n");
            j = 0;
        }
        fprintf(out, "%c", *(((fasta_l) self)->seq + i));
        j++;
        i++;
    }
    fprintf(out, "\n");
}

/**
 * Print in fasta file to the STDOUT
 * 
 * @param self the container object
 * @param lineLength the length of the fasta line
 */
void toString(void * self, int lineLength) {
    _CHECK_SELF_P(self);
    toFile(self, stdout, lineLength);
}

/**
 * Return the length of the fasta sequence
 * 
 * @param self the container object
 * @return the length of the fasta sequence
 */
int length(void *self) {
    _CHECK_SELF_P(self);
    return strlen(((fasta_l) self)->seq);
}

/**
 * Extract and print a segments from the start position with length
 * 
 * @param self the container object
 * @param out the output file 
 * @param header the fasta header
 * @param start the start position 
 * @param length the segment length
 * @param lineLength the length of the fasta line
 */
void printSegment(void * self, FILE *out, char *header, int start, int length, int lineLength) {
    _CHECK_SELF_P(self);
    int i = start;
    int j = 0;
    fprintf(out, ">%s\n", header);
    while (i < ((fasta_l) self)->length(self) && (i - start) < length) {
        if (j == lineLength) {
            fprintf(out, "\n");
            j = 0;
        }
        fprintf(out, "%c", *(((fasta_l) self)->seq + i));
        j++;
        i++;
    }
    fprintf(out, "\n");
}

/**
 * Creates segments of length with an overlap of offset
 * 
 * @param self the container object
 * @param out the output file 
 * @param length the length of the segments
 * @param offset the offset of the segments
 * @param lineLength the length of the fasta line
 */
void printOverlapSegments(void * self, FILE *out, int length, int offset, int lineLength) {
    _CHECK_SELF_P(self);
    int i, index;
    char **ids;
    int ids_number;
    size_t size = (strlen(((fasta_l) self)->header)) + 1000;
    char *header = allocate(sizeof (char) * size, __FILE__, __LINE__);

    memset(header, 0, size);
    ids_number = splitString(&ids, ((fasta_l) self)->header, "|");
    if (ids_number % 2 == 0) {
        strcpy(header, ((fasta_l) self)->header);
        strcat(header, "|from-to|");
    } else {
        for (i = 0; i < ids_number - 1; i++) {
            strcat(header, ids[i]);
            strcat(header, "|");
        }
        strcat(header, "from-to|");
    }

    index = strlen(header);
    for (i = 0;; i += offset) {
        header[index] = '\0';
        if (ids_number % 2 == 0) {
            sprintf(header, "%s%d-%d", header, i, i + length);
        } else {
            sprintf(header, "%s%d-%d|%s", header, i, i + length, ids[ids_number - 1]);
        }
        printSegment(self, out, header, i, length, lineLength);
        if (i + length >= ((fasta_l) self)->length(self)) break;
    }

    freeString(ids, ids_number);
    free(header);
}

void *thread_function(void *arg) {
    thread_param_t *parms = ((thread_param_t*) arg);
    void * self = parms->self;
    int i, index;
    char **ids;
    int ids_number;
    size_t size = (strlen(((fasta_l) self)->header)) + 1000;
    char *header = allocate(sizeof (char) * size, __FILE__, __LINE__);
    FILE *fd = checkPointerError(fopen(parms->out, "w"), "Can't open temporal file", __FILE__, __LINE__, -1);

    memset(header, 0, size);
    ids_number = splitString(&ids, ((fasta_l) self)->header, "|");
    if (ids_number % 2 == 0) {
        strcpy(header, ((fasta_l) self)->header);
        strcat(header, "|from-to|");
    } else {
        for (i = 0; i < ids_number - 1; i++) {
            strcat(header, ids[i]);
            strcat(header, "|");
        }
        strcat(header, "from-to|");
    }

    index = strlen(header);

    index = strlen(header);
    for (i = parms->start; i < parms->end; i += parms->offset) {
        header[index] = '\0';
        if (ids_number % 2 == 0) {
            sprintf(header, "%s%d-%d", header, i, i + parms->length);
        } else {
            sprintf(header, "%s%d-%d|%s", header, i, i + parms->length, ids[ids_number - 1]);
        }
        printSegment(self, fd, header, i, parms->length, parms->lineLength);
        if (i + parms->length >= ((fasta_l) self)->length(self)) break;
    }

    freeString(ids, ids_number);
    free(header);
    fclose(fd);
}

/**
 * Creates segments of length with an overlap of offset
 * 
 * @param self the container object
 * @param out the output file 
 * @param length the length of the segments
 * @param offset the offset of the segments
 * @param lineLength the length of the fasta line
 * @param threads_number Number of threads
 */
void printOverlapSegmentsPthread(void * self, FILE *out, int length, int offset, int lineLength, int threads_number) {
    _CHECK_SELF_P(self);
    int i, reads, numPerThread, thread_cr_res, thread_join_res;
    pthread_t *threads = allocate(sizeof (pthread_t) * threads_number, __FILE__, __LINE__);
    thread_param_t *tp = allocate(sizeof (thread_param_t) * threads_number, __FILE__, __LINE__);
    char **tFiles = allocate(sizeof (char **) * threads_number, __FILE__, __LINE__);
    pid_t pid = getpid();
    size_t bytes;
    char buffer[SIZE];
    FILE *fd;

    reads = ((fasta_l) self)->length(self) / offset;
    numPerThread = reads / threads_number;

    for (i = 0; i < threads_number; i++) {
        printf("Creating thread: %d\n", i);
        fflush(NULL);
        tFiles[i] = allocate(sizeof (char *) * 150, __FILE__, __LINE__);
        sprintf(tFiles[i], "%d_%d.fna", pid, i);

        tp[i].number = i;
        tp[i].length = length;
        tp[i].lineLength = lineLength;
        tp[i].offset = offset;
        tp[i].out = tFiles[i];
        tp[i].self = self;
        tp[i].start = i * numPerThread * offset;
        if (i < threads_number - 1) {
            tp[i].end = (i + 1) * numPerThread * offset;
        } else {
            tp[i].end = ((fasta_l) self)->length(self);
        }

        thread_cr_res = pthread_create(&threads[i],
                NULL,
                thread_function,
                (void*) &(tp[i]));
        if (thread_cr_res != 0) {
            checkPointerError(NULL, "THREAD CREATE ERROR", __FILE__, __LINE__, -1);
        }
    }

    for (i = 0; i < threads_number; i++) {
        printf("Waiting for thread: %d\n", i);
        fflush(NULL);
        thread_join_res = pthread_join(threads[i], NULL);
        if (thread_join_res != 0) {
            checkPointerError(NULL, "JOIN ERROR", __FILE__, __LINE__, -1);
        }
        fd = fopen(tFiles[i], "r");
        checkPointerError(fd, "Can't open temporal file", __FILE__, __LINE__, -1);

        while (0 < (bytes = fread(buffer, 1, sizeof (buffer), fd))) {
            fwrite(buffer, 1, bytes, out);
        }
        fclose(fd);
        remove(tFiles[i]);
        if (tFiles[i]) free(tFiles[i]);
    }
    if (tp) free(tp);
    if (tFiles)free(tFiles);

    if (threads)free(threads);
}

/**
 * Set the fasta header
 * 
 * @param self the container object
 * @param string the header
 */
void setHeader(void *self, char *string) {

    _CHECK_SELF_P(self);
    ((fasta_l) self)->header = strdup(string);
}

/**
 * Set the sequence
 * 
 * @param self the container object
 * @param string the sequence
 */
void setSeq(void *self, char *string) {

    _CHECK_SELF_P(self);
    ((fasta_l) self)->seq = strdup(string);
}

/**
 * Free the fasta container
 * 
 * @param self the container object
 */
void freeFasta(void *self) {
    _CHECK_SELF_P(self);
    if (((fasta_l) self)->header) free(((fasta_l) self)->header);

    if (((fasta_l) self)->seq) free(((fasta_l) self)->seq);
    free(((fasta_l) self));
}

/**
 * Create the Fasta object and initialized the pointers to the methods
 * 
 * @return a fasta_l object
 */
fasta_l CreateFasta() {
    fasta_l self = allocate(sizeof (struct fasta_s), __FILE__, __LINE__);

    self->header = NULL;
    self->seq = NULL;
    self->toString = &toString;
    self->length = &length;
    self->free = &freeFasta;
    self->setHeader = &setHeader;
    self->setSeq = &setSeq;
    self->printOverlapSegments = &printOverlapSegments;
    self->printSegment = &printSegment;
    self->printOverlapSegmentsPthread = &printOverlapSegmentsPthread;

    return self;
}

fasta_l ReadFasta(FILE *fp) {
    fasta_l self = NULL;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    off_t pos;

    while ((read = getline(&line, &len, fp)) != -1) {
        if (strncmp(line, ">", 1) == 0) {
            if (self == NULL) {
                self = CreateFasta();
                self->header = strndup(line + 1, strlen(line) - 2);
            } else {
                fseeko(fp, pos, SEEK_SET);
                break;
            }
        } else {
            checkPointerError(self, "The fasta file does not start with the header (>)", __FILE__, __LINE__, -1);
            if (self->seq == NULL) {
                self->seq = strndup(line, strlen(line) - 1);
            } else {
                self->seq = reallocate(self->seq, strlen(self->seq) + strlen(line) + 1, __FILE__, __LINE__);
                strncat(self->seq, line, strlen(line) - 1);
            }
        }
        pos = ftello(fp);
    }

    if (line) free(line);
    return self;
}

