/* 
 * File:   taxonomy.h
 * Author: roberto
 *
 * Created on May 7, 2014, 8:47 AM
 */

#ifndef TAXONOMY_H
#define	TAXONOMY_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct taxonomy_s {
        int taxId;
        int parentTaxId;
        char *name;
        char *rank;

        /*
         * Methods
         */

        /**
         * Set the TaxId
         * 
         * @param self the container object
         * @param int the taxId
         */
        void (*setTaxId)(void *self, int taxId);

        /**
         * Set the parentTaxId
         * 
         * @param self the container object
         * @param int the parentTaxId
         */
        void (*setParentTaxId)(void *self, int parentTaxId);

        /**
         * Set the name
         * 
         * @param self the container object
         * @param string the sequence
         */
        void (*setName)(void *self, char *name);

        /**
         * Set the rank
         * 
         * @param self the container object
         * @param string the rank
         */
        void (*setRank)(void *self, char *rank);

        /**
         * Return an array of TaxIds of the lineage for the taxonomy
         * 
         * @param self the container object
         * @param an array of TaxIds of the lineage for the taxonomy
         * @param count the number of elements in the leneage array
         * @param taxDB the NCBI Taxonomy database in a bTree index
         */
        void (*getLineage)(void *self, int **lineage, int *count, BtreeNode_t *taxDB);


        /**
         * Print the Taxonomy to the STDOUT
         * 
         * @param self the container object
         */
        void (*toString)(void *self);

        /**
         * Print the taxonomy to a file 
         * 
         * @param self the container object
         * @param out the output file
         */
        void (*toFile)(void * self, FILE *out);

        /**
         * Free the fasta container
         * 
         * @param self the container object
         */
        void (*free)(void *self);
    };

    typedef struct taxonomy_s *taxonomy_l;

    /**
     * Create the Taxonomy object and initialized the pointers to the methods
     * 
     * @return a taxonomy_l object
     */
    extern taxonomy_l CreateTaxonomy();

    /**
     * Read a taxonomy entry from the files
     * 
     * @param nodes the nodes.dmp NCBI Taxonomy file
     * @param names the names.dmp NCBI Taxonomy file
     * @return the taxonomy entry
     */
    extern taxonomy_l ReadTaxonomy(FILE *nodes, FILE *names);

    /**
     * Read the NCBI Taxonomy nodes.dmp and names.dmp files an return a Btree index with 
     * the data
     * 
     * @param dir the NCBI Taxonomy DB directory
     * @param verbose 1 to print a verbose info
     * @return the NCBI Taxonomy db in a Btree index
     */
    extern BtreeNode_t *TaxonomyDBIndex(char *dir, int verbose);

    /**
     * Read the gi_taxid_nucl.dmp.gz file from NCBI Taxonomy and return a Btree 
     * index of the Gi
     * 
     * @param filename the gi_taxid_nucl.dmp.gz complete path
     * @param verbose 1 to print a verbose info
     * @return 
     */
    extern BtreeNode_t *TaxonomyNuclIndex(char *gi_taxid_nucl, int verbose);


#ifdef	__cplusplus
}
#endif

#endif	/* TAXONOMY_H */

