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
#include "memory.h"
#include "bstring.h"
#include "fasta.h"

#define _CHECK_SELF_P(s) checkPointerError(s, "Null self pointer", __FILE__, __LINE__, -1)

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
    char *header = allocate(sizeof (char) * size,__FILE__, __LINE__);

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

