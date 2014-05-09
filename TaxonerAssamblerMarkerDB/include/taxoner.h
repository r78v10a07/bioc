/* 
 * File:   taxoner.h
 * Author: roberto
 *
 * Created on May 8, 2014, 11:01 AM
 */

#ifndef TAXONER_H
#define	TAXONER_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct taxoner_gi_s {
        int gi;
        int **fromTo;
        int fromTo_number;

        /**
         * Sort the From-To values by the Front 
         * @param self the container object
         */
        void (*sortFromTo)(void *self);

        /**
         * Free the Taxoner Gi container
         * 
         * @param self the container object
         */
        void (*free)(void *self);
    };

    typedef struct taxoner_gi_s *taxoner_gi_l;

    /**
     * Create the Taxoner Gi and initialized the pointers to the methods
     * 
     * @return a taxoner_gi_l object
     */
    extern taxoner_gi_l CreateTaxonerGi();

    struct taxoner_tax_s {
        int taxId;
        node *gisIndex;
        taxoner_gi_l gis;
        int gis_numbers;

        /**
         * Free the Taxoner Tax container
         * 
         * @param self the container object
         */
        void (*free)(void *self);
    };

    typedef struct taxoner_tax_s *taxoner_tax_l;

    /**
     * Create the Taxoner Tax and initialized the pointers to the methods
     * 
     * @return a taxoner_tax_l object
     */
    extern taxoner_tax_l CreateTaxonerTax();

    /**
     * Parse the Taxoner file and print into the output dir the results
     * 
     * @param output the name of the ouput dir
     * @param rankToPrint coma separated list of taxonomy rank to print fasta
     * @param fd the input file
     * @param score the score to be used as cutoff
     * @param fBtree the fasta btree index
     * @param fFasta the fasta file
     * @param taxDB the NCBI Taxonomy db
     * @param readLenght length of the reads
     * @param readOffset offset used to overlap the reads
     * @param verbose 1 to print info
     */
    extern void ParseTaxonerResult(char *output, char *rankToPrint, FILE *fd, float score, node *fBtree, FILE * fFasta, node *taxDB, int readLength, int readOffset, int verbose);


#ifdef	__cplusplus
}
#endif

#endif	/* TAXONER_H */

