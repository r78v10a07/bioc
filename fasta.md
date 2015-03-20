# Functions manipulate FASTA files #

[fasta.h](https://bioc.googlecode.com/svn/trunk/bioc/include/fasta.h)  [fasta.c](https://bioc.googlecode.com/svn/trunk/bioc/src/fasta.c)


```
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
```

The structure:

```
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
        void (*getSegment)(void **outvoid, void * self, char *header, int start, int length);

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
        void (*splitInSegments)(void * self, FILE *out, int length, int offset, int lineLength, int threads_number, int inMem);

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
```

# Independent functions #

## Create the Fasta object and initialized the pointers to the methods ##

```
    /**
     * @return a fasta_l object
     */
    extern fasta_l CreateFasta();
```

## Read a fasta entry from the file ##

```
    /**
     * @param fp the input file
     * @param excludeSeq 1 if you want to exclude the sequence and read only the header 
     * @return the fasta entry
     */
    extern fasta_l ReadFasta(FILE *fp, int excludeSeq);

```

## Read a fasta entry from the file starting from the offset ##

```
    /**
     * @param fp the input file
     * @param offset the offset to start reading
     * @param excludeSeq 1 if you want to exclude the sequence and read only the header 
     * @return the fasta entry
     */
    extern fasta_l ReadFastaFromOffset(FILE *fp, off_t offset, int excludeSeq);
```

## Read a fasta entry from a gzipped file ##

```
    /**
     * @param fp the gzipped input file
     * @param excludeSeq 1 if you want to exclude the sequence and read only the header 
     * @return the fasta entry
     */
    extern fasta_l ReadFastaGzip(gzFile fp, int excludeSeq);

```

## Create a fasta binary index file which include the gi and the offset position ##

```
    /**
     * @param fd the input fasta file
     * @param fo the output binary file
     * @param verbose 1 to print info
     * @return the number of elements read
     */
    extern int CreateFastaIndexToFile(FILE *fd, FILE *fo, int verbose);
```

## Create a Btree index which include the gi and the offset position ##

```
    /**
     * @param fd the input fasta file
     * @param verbose 1 to print info
     * @return the Btree index
     */
    extern node * CreateBtreeFromFasta(FILE *fd, int verbose);
```

## Create a fasta binary index file which include the gi and the offset position ##

```
    /**
     * @param fd the input fasta gzip file
     * @param fo the output binary file
     * @param verbose 1 to print info
     * @return the number of elements read
     */
    extern int CreateFastaIndexGzipToFile(gzFile fd, FILE *fo, int verbose);
```

## Create a Btree index which include the gi and the offset position ##

```
    /**
     * @param fd the input fasta file
     * @param verbose 1 to print info
     * @return the Btree index
     */
    extern node * CreateBtreeFromFastaGzip(gzFile fd, int verbose);
```

## Create a Btree index from a fasta index file ##

```
    /**
     * @param fi the fasta index file
     * @param verbose 1 to print info
     * @return the Btree index
     */
    extern node *CreateBtreeFromIndex(FILE *fi, int verbose);
```

Usage:

Create a binary index file for a big fasta file. Then, read randomly fasta entries from a GI list and print the fasta entry. In the last program only the GI and offset are in memory.

Program to create the index (see source code [here](https://code.google.com/p/bioc/source/browse/trunk/BuildBtreeIndexFasta/)):
```
FILE *fd = checkPointerError(fopen("myfastabigfile.fna", "r"), "Can't open input file", __FILE__, __LINE__, -1);
FILE *fo = checkPointerError(fopen("mybinaryindexfile.ind", "wb"), "Can't open output file", __FILE__, __LINE__, -1);

CreateFastaIndexToFile(fd, fo, verbose);
```

Program to read the binary index file and play around:
```
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zlib.h>
#include "berror.h"
#include "btree.h"
#include "fasta.h"

int main(int argc, char *argv[]) {
    fasta_l fasta;
    int gi, verbose;

    off_t offset;
    BtreeRecord_t * rec;

    // The Gis file, one Gi per line
    FILE *giFile = checkPointerError(fopen(argv[1], "r"), "Can't open the GI file", __FILE__, __LINE__, -1);

    // The big fasta file
    FILE *fFasta = checkPointerError(fopen(argv[2], "r"), "Can't open the fasta file", __FILE__, __LINE__, -1);

    // The fasta file index
    FILE *fIndex = checkPointerError(fopen(argv[3], "r"), "Can't open index file", __FILE__, __LINE__, -1);

    verbose = 0;
    
    // Create the Btree from the fasta binary index file
    BtreeNode_t *fBtree = CreateBtreeFromIndex(fIndex, verbose);

    while (fscanf(giFile, "%d", &gi) != EOF) {
        /* Get the offset in the fasta file for the GI
         * Return NULL if the Gi is not in the fasta file
         */
        if ((rec = BTreeFind(fBtree, gi, false)) != NULL) {
            // Get the fasta entry offset in the big fasta file
            offset = *((off_t *) rec->value);
            // Put the file stream in the correct position in the big fasta file
            fasta = ReadFastaFromOffset(fFasta, offset, 0);

            //Print the fasta entry
            fasta->toString(fasta, 80);

            //Free the fasta entry
            fasta->free(fasta);
        }else{
            fprintf(stderr,"Gi: %d is not in the fasta file\n",gi);
        }
    }

    /* Free the Btree using the system free function (#include <stdlib.h>)
     * because the btree are offsets
     */
    BTreeFree(fBtree, free);
    fclose(giFile);
    fclose(fIndex);
    fclose(fFasta);
    
    return EXIT_SUCCESS;
}
```

Compile the program like:

```
gcc -g main.c -I ./bioc/include/ -L ./bioc/dist/Debug/GNU-Linux-x86/ -lbioc -lpthread -lz
```

Run it like this:

```
./a.out gisFile complete_bacteria.fna complete_bacteria.ind
```