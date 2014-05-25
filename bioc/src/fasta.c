/* 
 * File:   fasta.c
 * Author: roberto
 *
 * Created on April 14, 2014, 11:59 AM
 */

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#define SIZE 3072

typedef struct thread_param {
    int number;
    void * self;
    char *out;
    char *parser;
    int start;
    int end;
    int length;
    int offset;
    int lineLength;
    void **res;
    int resNumber;
} thread_param_t;

/**
 * Print in fasta format with a line length of lineLength 
 * 
 * @param self the container object
 * @param out the output file
 * @param lineLength the length of the fasta line
 */
void toFileFasta(void * self, FILE *out, int lineLength) {
    _CHECK_SELF_P(self);
    int i = 0;
    char *tmp = allocate(sizeof (char) * (lineLength + 1), __FILE__, __LINE__);
    fprintf(out, ">%s\n", ((fasta_l) self)->header);
    for (i = 0; i < ((fasta_l) self)->len; i += lineLength) {
        memset(tmp, 0, sizeof (char) * (lineLength + 1));
        if (i + lineLength < ((fasta_l) self)->len) {
            strncpy(tmp, (((fasta_l) self)->seq + i), lineLength);
        } else {
            strncpy(tmp, (((fasta_l) self)->seq + i), ((fasta_l) self)->len - i);
        }
        fprintf(out, "%s\n", tmp);
    }
    free(tmp);
}

/**
 * Print in fasta file to the STDOUT
 * 
 * @param self the container object
 * @param lineLength the length of the fasta line
 */
void toStringFasta(void * self, int lineLength) {
    _CHECK_SELF_P(self);
    ((fasta_l) self)->toFile(self, stdout, lineLength);
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
 * Get the Gi parsing the fasta header
 * 
 * @param self the container object
 * @param gi the return gi, -1 if not Gi is present
 */
void getGi(void *self, int *gi) {
    char **ids;
    int i, ids_number;

    *gi = -1;
    ids_number = splitString(&ids, ((fasta_l) self)->header, "|");
    for (i = 0; i < ids_number; i++) {
        if (strcmp(ids[i], "gi") == 0 && i < ids_number - 1) {
            *gi = atoi(ids[i + 1]);
            if (*gi <= 0) {
                fprintf(stderr, "%s Trying to do atoi in:  %s\n", ids[i], ids[i + 1]);
                fprintf(stderr, "Can't find Gi %d on: %s\nThe format have to be gi|ginumber: >gi|12345\n", *gi, ((fasta_l) self)->header);
                exit(1);
            }
            break;
        }
    }
    freeArrayofPointers((void **) ids, ids_number);
    if (*gi <= 0) {
        fprintf(stderr, "Can't find Gi %d on: %s\nThe format have to be gi|ginumber: >gi|12345\n", *gi, ((fasta_l) self)->header);
        exit(1);
    }
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
    ((fasta_l) self)->len = strlen(string);
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
 * Extract and print a segments from the start position with length
 * 
 * @param outvoid the fasta_l object to return
 * @param self the container object
 * @param out the output file 
 * @param header the fasta header
 * @param start the start position 
 * @param length the segment length
 * @param lineLength the length of the fasta line
 */
void getSegment(void **outvoid, void * self, char *header, int start, int length) {
    _CHECK_SELF_P(self);
    fasta_l out;
    int size;

    *outvoid = NULL;
    if (start < ((fasta_l) self)->len) {
        out = CreateFasta();
        if (start + length < ((fasta_l) self)->len) {
            size = length;
        } else {
            size = ((fasta_l) self)->len - start;
        }
        out->header = strdup(header);
        out->seq = allocate(sizeof (char) * (size + 1), __FILE__, __LINE__);
        memset(out->seq, 0, sizeof (char) * (size + 1));
        strncpy(out->seq, (((fasta_l) self)->seq + start), size);
        out->len = size;
        *outvoid = out;
    }
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
    fasta_l outf;
    int size;
    
    if (start < ((fasta_l) self)->len) {
        if (start + length < ((fasta_l) self)->len) {
            size = length;
        } else {
            size = ((fasta_l) self)->len - start;
        }
        outf = CreateFasta();
        outf->setHeader(outf, header);
        outf->seq = allocate(sizeof (char) * (size + 1), __FILE__, __LINE__);
        memset(outf->seq, 0, sizeof (char) * (size + 1));
        strncpy(outf->seq, (((fasta_l) self)->seq + start), size);
        outf->len = size;
        outf->toFile(outf, out, lineLength);
        outf->free(outf);
    }
}

void *pthreadSplitInSegments(void *arg) {
    thread_param_t *parms = ((thread_param_t*) arg);
    void * self = parms->self;
    int i, gi;
    size_t size = (strlen(((fasta_l) self)->header)) + 1000;
    char *header = allocate(sizeof (char) * size, __FILE__, __LINE__);
    FILE *fd = checkPointerError(fopen(parms->out, "w"), "Can't open temporal file", __FILE__, __LINE__, -1);

    for (i = parms->start; i < parms->end; i += parms->offset) {
        ((fasta_l) self)->getGi(self, &gi);
        sprintf(header, "%d|%d-%d", gi, i, i + parms->length);
        printSegment(self, fd, header, i, parms->length, parms->lineLength);
        if (i + parms->length >= ((fasta_l) self)->len) break;
    }

    free(header);
    fclose(fd);
    return NULL;
}

void *pthreadSplitInSegmentsInMem(void *arg) {
    thread_param_t *parms = ((thread_param_t*) arg);
    void * self = parms->self;
    int i, gi;
    size_t size = (strlen(((fasta_l) self)->header)) + 1000;
    char *header = allocate(sizeof (char) * size, __FILE__, __LINE__);
    void **res = NULL;
    int resNumber = 0;

    memset(header, 0, size);
    for (i = parms->start; i < parms->end; i += parms->offset) {
        sscanf(((fasta_l) self)->header, parms->parser, &gi);
        if (gi <= 0) {
            fprintf(stderr, "Bad GI %d on header: %s\n", gi, ((fasta_l) self)->header);
            exit(-1);
        }
        sprintf(header, "%d|%d-%d", gi, i, i + parms->length);
        res = realloc(res, sizeof (void **) * (resNumber + 1));
        checkPointerError(res, "Can't allocate memory", __FILE__, __LINE__, -1);
        getSegment((void **) &(res[resNumber]), self, header, i, parms->length);
        resNumber++;
        if (i + parms->length >= ((fasta_l) self)->len) break;
    }

    parms->res = res;
    parms->resNumber = resNumber;
    free(header);
    return NULL;
}

/**
 * Creates segments of length with an overlap of offset
 * 
 * @param self the container object
 * @param out the output file 
 * @param headerParser sscanf format to parse the fasta header
 * @param length the length of the segments
 * @param offset the offset of the segments
 * @param lineLength the length of the fasta line
 * @param threads_number Number of threads
 * @param inMem du the generation in memory
 */
void splitInSegments(void * self, FILE *out, char *headerParser, int length, int offset, int lineLength, int threads_number, int inMem) {
    _CHECK_SELF_P(self);
    int i, j, reads, numPerThread, thread_cr_res, thread_join_res;
    pthread_t *threads = allocate(sizeof (pthread_t) * threads_number, __FILE__, __LINE__);
    thread_param_t *tp = allocate(sizeof (thread_param_t) * threads_number, __FILE__, __LINE__);
    char **tFiles = NULL;
    pid_t pid = getpid();
    size_t bytes;
    char buffer[SIZE];
    FILE *fd;

    if (inMem == 0) {
        tFiles = allocate(sizeof (char **) * threads_number, __FILE__, __LINE__);
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

        tp[i].parser = headerParser;
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
                    ((fasta_l) tp[i].res[j])->toFile(tp[i].res[j], out, lineLength);
                }
                ((fasta_l) tp[i].res[j])->free(tp[i].res[j]);
            }
            free(tp[i].res);
        }
    }
    if (tp) free(tp);
    if (tFiles)free(tFiles);
    if (threads)free(threads);
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
    self->len = 0;
    self->toString = &toStringFasta;
    self->length = &length;
    self->free = &freeFasta;
    self->setHeader = &setHeader;
    self->setSeq = &setSeq;
    self->splitInSegments = &splitInSegments;
    self->printSegment = &printSegment;
    self->toFile = &toFileFasta;
    self->getSegment = &getSegment;
    self->getGi = &getGi;

    return self;
}

/**
 * Read the fasta entry using a buffer of characters
 * 
 * @param fp the input file
 * @param bufferSize the number of characters in the buffer
 * @param excludeSeq 1 if you want to exclude the sequence and read only the header 
 * @return the fasta entry
 */
fasta_l ReadFastaBuffer(FILE *fp, int bufferSize, int excludeSeq) {
    fasta_l self = NULL;

    size_t buf_size, seq_size;
    char *buffer, *seq, *line, *str;
    off_t pos;
    int numLines, readTotal, out;

    seq_size = buf_size = sizeof (char) * bufferSize;
    buffer = allocate(sizeof (char) * (bufferSize + 1), __FILE__, __LINE__);
    if (!excludeSeq) {
        seq = allocate(sizeof (char) * (bufferSize + 1), __FILE__, __LINE__);
        seq[0] = '\0';
    } else {
        seq = NULL;
    }
    pos = ftello(fp);
    readTotal = out = 0;
    numLines = 0;
    while (!feof(fp)) {
        memset(buffer, 0, buf_size);
        if (fread(buffer, buf_size, 1, fp) == 1) buffer[bufferSize] = '\0';
        str = buffer;
        while (1) {
            line = strchr(str, '\n');
            if (line) *line = '\0';
            if (*str != '\0' && *str != '\n') {
                if (*str == '>') {
                    if (self == NULL) {
                        readTotal += strlen(str);
                        self = CreateFasta();
                        self->header = strndup(str + 1, strlen(str) - 1);
                    } else {
                        if (!excludeSeq) self->setSeq(self, seq);
                        pos = pos + sizeof (char) * (readTotal + numLines);
                        fseeko(fp, pos, SEEK_SET);
                        break;
                    }
                } else {
                    checkPointerError(self, "The fasta file does not start with the header (>)", __FILE__, __LINE__, -1);
                    readTotal += strlen(str);
                    if (!excludeSeq) {
                        if (readTotal >= seq_size) {
                            seq_size += seq_size;
                            seq = reallocate(seq, seq_size, __FILE__, __LINE__);
                        }
                        strcat(seq, str);
                    }
                }
            }
            if (!line) break;
            str = line + 1;
            numLines++;
        }
        if (*str == '>') break;
    }
    if (feof(fp) && self && !self->seq && !excludeSeq) self->setSeq(self, seq);

    if (!excludeSeq) free(seq);
    free(buffer);
    return self;
}

/**
 * Read a fasta entry from the file
 * 
 * @param fp the input file
 * @param excludeSeq 1 if you want to exclude the sequence and read only the header 
 * @return the fasta entry
 */
fasta_l ReadFasta(FILE *fp, int excludeSeq) {
    if (!excludeSeq)
        return ReadFastaBuffer(fp, 7000000, excludeSeq);
    else
        return ReadFastaBuffer(fp, 7000, excludeSeq);
}

/**
 * Read a fasta entry from the file starting from the offset
 * 
 * @param fp the input file
 * @param offset the offset to start reading
 * @param excludeSeq 1 if you want to exclude the sequence and read only the header 
 * @return the fasta entry
 */
fasta_l ReadFastaFromOffset(FILE *fp, off_t offset, int excludeSeq) {
    fseeko(fp, offset, SEEK_SET);
    return ReadFasta(fp, excludeSeq);
}

/**
 * Read a fasta entry from the file
 * 
 * @param fp the input file
 * @param excludeSeq 1 if you want to exclude the sequence and read only the header 
 * @return the fasta entry
 */
fasta_l ReadFastaGzip(gzFile fp, int excludeSeq) {
    fasta_l self = NULL;

    char *line = NULL;
    z_off_t pos = 0;

    line = allocate(sizeof (char) * 1000, __FILE__, __LINE__);
    while (!gzeof(fp)) {
        if (strncmp(line, ">", 1) == 0) {
            if (self == NULL) {
                self = CreateFasta();
                self->header = strndup(line + 1, strlen(line) - 2);
            } else {
                gzseek(fp, pos, SEEK_SET);
                break;
            }
        } else if (excludeSeq != 1) {
            checkPointerError(self, "The fasta file does not start with the header (>)", __FILE__, __LINE__, -1);
            if (self->seq == NULL) {
                self->seq = strndup(line, strlen(line) - 1);
            } else {
                self->seq = reallocate(self->seq, strlen(self->seq) + strlen(line) + 1, __FILE__, __LINE__);
                strncat(self->seq, line, strlen(line) - 1);
            }
        }
        pos = gztell(fp);
    }


    if (self != NULL) {
        self->len = self->length(self);
    }
    if (line) free(line);
    return self;
}

/**
 * Create a fasta binary index file which include the gi and the offset position
 * 
 * @param fd the input fasta file
 * @param fo the output binary file
 * @param verbose 1 to print info
 * @return the number of elements read
 */
int CreateFastaIndexToFile(FILE *fd, FILE *fo, int verbose) {
    fasta_l fasta;
    int count, gi;
    count = 0;
    off_t pos = 0;

    if (verbose) {
        printf("Creating the fasta index\n");
        fflush(stdout);
    }
    while ((fasta = ReadFasta(fd, 1)) != NULL) {
        fasta->getGi(fasta, &gi);
        if (verbose) {
            printf("Total: %10d \r", count);
            fflush(stdout);
        }
        fwrite(&gi, sizeof (int), 1, fo);
        fwrite(&(pos), sizeof (off_t), 1, fo);
        fasta->free(fasta);
        pos = ftello(fd);
        count++;
    }
    if (verbose) {
        printf("Total: %10d \n", count);
        fflush(stdout);
    }
    return count;
}

/**
 * Create a Btree index which include the gi and the offset position
 * 
 * @param fd the input fasta file
 * @param verbose 1 to print info
 * @return the Btree index
 */
BtreeNode_t * CreateBtreeFromFasta(FILE *fd, int verbose) {
    fasta_l fasta;
    BtreeNode_t *root = NULL;
    off_t *value;
    int count, gi;
    count = 0;
    off_t pos = 0;

    if (verbose) {
        printf("Creating the fasta index\n");
        fflush(stdout);
    }
    while ((fasta = ReadFasta(fd, 1)) != NULL) {
        value = malloc(sizeof (off_t));
        fasta->getGi(fasta, &gi);
        if (verbose) {
            printf("Total: %10d \r", count);
            fflush(stdout);
        }
        *value = pos;
        root = BtreeInsert(root, gi, value);
        fasta->free(fasta);
        pos = ftello(fd);
        count++;
    }
    if (verbose) {
        printf("Total: %10d \n", count);
        fflush(stdout);
    }
    return root;
}

/**
 * Create the fasta index file which include the gi and the offset position
 * 
 * @param fd the input fasta gzip file
 * @param fo the output binary file
 * @param verbose 1 to print info
 * @return the number of elements read
 */
int CreateFastaIndexGzipToFile(gzFile fd, FILE *fo, int verbose) {
    fasta_l fasta;
    int count, gi;
    count = 0;
    off_t pos = 0;
    while ((fasta = ReadFastaGzip(fd, 1)) != NULL) {
        fasta->getGi(fasta, &gi);
        if (verbose) {
            printf("Total: %10d\tGi: %10d \r", count, gi);
            fflush(stdout);
        }
        fwrite(&gi, sizeof (int), 1, fo);
        fwrite(&(pos), sizeof (off_t), 1, fo);
        fasta->free(fasta);
        pos = gztell(fd);
        count++;
    }
    return count;
}

/**
 * Create a Btree index which include the gi and the offset position
 * 
 * @param fd the input fasta file
 * @param verbose 1 to print info
 * @return the Btree index
 */
BtreeNode_t * CreateBtreeFromFastaGzip(gzFile fd, int verbose) {
    fasta_l fasta;
    BtreeNode_t *root = NULL;
    off_t *value;
    int count, gi;
    count = 0;
    off_t pos = 0;

    if (verbose) {
        printf("Creating the fasta index\n");
        fflush(stdout);
    }
    while ((fasta = ReadFastaGzip(fd, 1)) != NULL) {
        value = malloc(sizeof (off_t));
        fasta->getGi(fasta, &gi);
        if (verbose) {
            printf("Total: %10d \r", count);
            fflush(stdout);
        }
        *value = pos;
        root = BtreeInsert(root, gi, value);
        fasta->free(fasta);
        pos = gztell(fd);
        count++;
    }
    if (verbose) {
        printf("Total: %10d \n", count);
        fflush(stdout);
    }
    return root;
}

/**
 * Create a Btree index from a fasta index file
 * 
 * @param fi the fasta index file
 * @param verbose 1 to print info
 * @return the Btree index
 */
BtreeNode_t *CreateBtreeFromIndex(FILE *fi, int verbose) {
    struct timespec start, stop;
    BtreeNode_t *root = NULL;
    off_t fileLen;
    off_t pos;
    off_t *value;
    int gi;
    int count = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    fseeko(fi, 0, SEEK_END);
    fileLen = ftello(fi);
    fseeko(fi, 0, SEEK_SET);

    pos = 0;
    while (pos < fileLen) {
        value = malloc(sizeof (off_t));
        fread(&gi, sizeof (int), 1, fi);
        fread(value, sizeof (off_t), 1, fi);

        root = BtreeInsert(root, gi, value);
        pos = ftell(fi);
        count++;
    }
    clock_gettime(CLOCK_MONOTONIC, &stop);
    if (verbose)
        printf("\n\tThere are %d GIs into the B+Tree. Elapsed time: %.2f sec\n\n", count, timespecDiffSec(&stop, &start));
    fflush(NULL);
    return root;
}

