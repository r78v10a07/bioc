/* 
 * File:   fasta.h
 * Author: roberto
 *
 * Created on April 14, 2014, 11:58 AM
 */

#ifndef FASTA_H
#define	FASTA_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct fasta_s {
        /*
         * Members          
         */
        char *header;
        char *seq;
        int len;

        /*
         * Methods
         */

        /**
         * Set the fasta header
         * 
         * @param self the container object
         * @param string the header
         */
        void (*setHeader)(void *self, char *string);

        /**
         * Set the sequence
         * 
         * @param self the container object
         * @param string the sequence
         */
        void (*setSeq)(void *self, char *string);

        /**
         * Creates segments of length with an overlap of offset
         * 
         * @param self the container object
         * @param out the output file 
         * @param length the length of the segments
         * @param offset the offset of the segments
         * @param lineLength the length of the fasta line
         */
        void (*printOverlapSegments)(void * self, FILE *out, int length, int offset, int lineLength);

        /**
         * Creates segments of length with an overlap of offset using threads
         * 
         * @param self the container object
         * @param out the output file 
         * @param length the length of the segments
         * @param offset the offset of the segments
         * @param lineLength the length of the fasta line
         * @param threads_number Number of threads
         * @param inMem du the generation in memory
         */
        void (*printOverlapSegmentsPthread)(void * self, FILE *out, int length, int offset, int lineLength, int threads_number, int inMem);

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
        void (*printSegment)(void * self, FILE *out, char *header, int start, int length, int lineLength);

        /**
         * Print in fasta file to the STDOUT
         * 
         * @param self the container object
         * @param lineLength the length of the fasta line
         */
        void (*toString)(void *self, int lineLength);

        /**
         * Print in fasta format with a line length of lineLength 
         * 
         * @param self the container object
         * @param out the output file
         * @param lineLength the length of the fasta line
         */
        void (*toFile)(void * self, FILE *out, int lineLength);

        /**
         * Return the length of the fasta sequence
         * 
         * @param self the container object
         * @return the length of the fasta sequence
         */
        int (*length)(void *self);

        /**
         * Free the fasta container
         * 
         * @param self the container object
         */
        void (*free)(void *self);
    };

    typedef struct fasta_s *fasta_l;

    extern fasta_l CreateFasta();

    extern fasta_l ReadFasta(FILE *fp);

#ifdef	__cplusplus
}
#endif

#endif	/* FASTA_H */

