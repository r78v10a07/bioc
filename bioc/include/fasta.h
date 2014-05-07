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
         * Get the Gi parsing the fasta header
         * 
         * @param self the container object
         * @param gi the return gi
         */
        void (*getGi)(void *self, int *gi);

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

    /**
     * Create the Fasta object and initialized the pointers to the methods
     * 
     * @return a fasta_l object
     */
    extern fasta_l CreateFasta();

    /**
     * Read a fasta entry from the file
     * 
     * @param fp the input file
     * @param excludeSeq 1 if you want to exclude the sequence 
     * @return the fasta entry
     */
    extern fasta_l ReadFasta(FILE *fp, int excludeSeq);

    /**
     * Read a fasta entry from a gzipped file
     * 
     * @param fp the gzipped input file
     * @param excludeSeq 1 if you want to exclude the sequence 
     * @return the fasta entry
     */
    extern fasta_l ReadFastaGzip(gzFile fp, int excludeSeq);

    /**
     * Create the fasta index file which include the gi and the offset position
     * 
     * @param fd the input fasta file
     * @param fo the output binary file
     * @param verbose 1 to print info
     * @return the number of elements read
     */
    extern int CreateFastaIndex(FILE *fd, FILE *fo, int verbose);

    /**
     * Create the fasta index file which include the gi and the offset position
     * 
     * @param fd the input fasta gzip file
     * @param fo the output binary file
     * @param verbose 1 to print info
     * @return the number of elements read
     */
    extern int CreateFastaIndexGzip(gzFile fd, FILE *fo, int verbose);

    /**
     * Create a Btree index from a fasta index file
     * 
     * @param fi the fasta index file
     * @param verbose 1 to print info
     * @return the Btree index
     */
    extern node *CreateBtreeFromIndex(FILE *fi, int verbose);

#ifdef	__cplusplus
}
#endif

#endif	/* FASTA_H */

