/* 
 * File:   splitFasta.c
 * Author: roberto
 *
 * Created on April 14, 2014, 7:16 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "fasta.h"

/*
 * 
 */
int main(int argc, char** argv) {
    fasta_l fasta;
    FILE *fd = fopen(argv[1], "r");
    FILE *fo = fopen(argv[2], "w");

    while ((fasta = ReadFasta(fd)) != NULL) {
        fasta->printOverlapSegments(fasta, fo, 100, 50, 80);

        fasta->free(fasta);
    }
    
    fclose(fd);
    fclose(fo);
    return (EXIT_SUCCESS);
}

