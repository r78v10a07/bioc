/* 
 * File:   main.c
 * Author: roberto
 *
 * Created on May 7, 2014, 2:46 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>
#include "btree.h"
#include "btime.h"
#include "berror.h"
#include "bmemory.h"
#include "bstring.h"
#include "taxonomy.h"

char *program_name;

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "\n********************************************************************************\n");
    fprintf(stream, "\nUsage: %s \n", program_name);
    fprintf(stream, "\n\n%s options:\n\n", program_name);
    fprintf(stream, "-v,   --verbose                     Print info\n");
    fprintf(stream, "-h,   --help                        Display this usage information.\n");
    fprintf(stream, "-d,   --dir                         The directory to the NCBI Taxonomy database\n");
    fprintf(stream, "-o,   --output                      The output fasta file\n");
    fprintf(stream, "-t,   --taxid                       The file with the taxids\n");
    fprintf(stream, "********************************************************************************\n");
    fprintf(stream, "\n            Roberto Vera Alvarez (e-mail: r78v10a07@gmail.com)\n\n");
    fprintf(stream, "********************************************************************************\n");
    exit(0);
}

int getPrintIndex(char *rank, int useNoRank) {
    if (strcmp(rank, "no rank") == 0 && useNoRank) {
        return 0;
    } else if (strcmp(rank, "species") == 0) {
        return 1;
    } else if (strcmp(rank, "genus") == 0) {
        return 2;
    } else if (strcmp(rank, "family") == 0) {
        return 3;
    } else if (strcmp(rank, "order") == 0) {
        return 4;
    } else if (strcmp(rank, "class") == 0) {
        return 5;
    } else if (strcmp(rank, "phylum") == 0) {
        return 6;
    } else if (strcmp(rank, "superkingdom") == 0) {
        return 7;
    }
    return -1;
}

/*
 * 
 */
int main(int argc, char** argv) {
    taxonomy_l tax;
    struct timespec start, stop, mid;
    int i, next_option, verbose, taxId;
    const char* const short_options = "vhd:o:t:";
    char *dir, *output, *tmp, *taxIdsName;
    FILE *nodes, *names, *taxids;
    FILE *fd;
    node *taxDB = NULL;
    node *foundTax = NULL;
    record *rec;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int *lineage = NULL;
    int lineage_count = 0;
    char **lineToPrint = NULL;

    clock_gettime(CLOCK_MONOTONIC, &start);
    program_name = argv[0];

    const struct option long_options[] = {
        { "help", 0, NULL, 'h'},
        { "verbose", 0, NULL, 'v'},
        { "dir", 1, NULL, 'd'},
        { "output", 1, NULL, 'o'},
        { "taxid", 1, NULL, 't'},
        { NULL, 0, NULL, 0} /* Required at end of array.  */
    };

    verbose = 0;
    dir = output = taxIdsName = NULL;
    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);

        switch (next_option) {
            case 'h':
                print_usage(stdout, 0);

            case 'v':
                verbose = 1;
                break;

            case 'o':
                output = strdup(optarg);
                break;

            case 'd':
                dir = strdup(optarg);
                break;

            case 't':
                taxIdsName = strdup(optarg);
                break;
        }
    } while (next_option != -1);

    if (!dir || !output || !taxIdsName) {
        print_usage(stderr, -1);
    }

    tmp = allocate(sizeof (char) * (strlen(dir) + 11), __FILE__, __LINE__);
    sprintf(tmp, "%s/nodes.dmp", dir);
    nodes = checkPointerError(fopen(tmp, "r"), "Can't open the nodes file", __FILE__, __LINE__, -1);
    sprintf(tmp, "%s/names.dmp", dir);
    names = checkPointerError(fopen(tmp, "r"), "Can't open the names file", __FILE__, __LINE__, -1);
    fd = checkPointerError(fopen(output, "w"), "Can't open output file", __FILE__, __LINE__, -1);

    taxids = checkPointerError(fopen(taxIdsName, "r"), "Can't open the Gi file", __FILE__, __LINE__, -1);

    clock_gettime(CLOCK_MONOTONIC, &mid);
    if (verbose) printf("Reading the Taxonomy database ... ");
    fflush(stdout);
    taxDB = TaxonomyDBIndex(nodes, names);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    if (verbose) printf("%lu sec\n", timespecDiffSec(&stop, &mid));
    fflush(stdout);

    printf("The Btree has a height of %d\n", height(taxDB));

    lineToPrint = allocate(sizeof (char *) * 8, __FILE__, __LINE__);
    for (i = 0; i < 8; i++) {
        lineToPrint[i] = allocate(sizeof (char) * 1000, __FILE__, __LINE__);
        lineToPrint[i][0] = '\t';
        lineToPrint[i][1] = '\0';
    }
    fprintf(fd, "strain\tspecies\tgenus\tfamily\torder\tclass\tphylum\tsuperkingdom\n");
    while ((read = getline(&line, &len, taxids)) != -1) {
        sscanf(line, "%d", &taxId);
        if ((rec = find(taxDB, taxId, false)) != NULL) {
            tax = (taxonomy_l) rec->value;
            if (find(foundTax, tax->taxId, false) == NULL) {
                foundTax = insert(foundTax, tax->taxId, tax);
                for (i = 0; i < 8; i++) {
                    lineToPrint[i][0] = '\t';
                    lineToPrint[i][1] = '\0';
                }
                next_option = getPrintIndex(tax->rank, 1);
                if (next_option != -1) {
                    sprintf(lineToPrint[next_option], "%s (%d)\t", tax->name, tax->taxId);
                }
                if ((rec = find(taxDB, tax->parentTaxId, false)) != NULL) {
                    tax = (taxonomy_l) rec->value;
                    tax->getLineage(tax, &lineage, &lineage_count, taxDB);
                    for (i = 0; i < lineage_count; i++) {
                        if ((rec = find(taxDB, lineage[i], false)) != NULL) {
                            tax = (taxonomy_l) rec->value;
                            next_option = getPrintIndex(tax->rank, 0);
                            if (next_option != -1) {
                                sprintf(lineToPrint[next_option], "%s (%d)\t", tax->name, tax->taxId);
                            }
                        }
                    }
                    for (i = 0; i < 8; i++) {
                        fprintf(fd, "%s", lineToPrint[i]);
                    }
                    fprintf(fd, "\n");
                    free(lineage);
                    lineage = NULL;
                    lineage_count = 0;
                }
            }
        }else{
            printf("%d\n",taxId);
        }
    }

    tax = CreateTaxonomy();
    destroy_tree(taxDB, tax->free);
    destroy_tree(foundTax, NULL);
    tax->free(tax);

    freeString(lineToPrint, 8);
    if (fd) fclose(fd);
    if (line) free(line);
    if (taxIdsName) free(taxIdsName);
    if (taxids) fclose(taxids);
    if (tmp) free(tmp);
    if (nodes) fclose(nodes);
    if (names) fclose(names);
    if (dir) free(dir);
    if (output) free(output);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    printf("\n\tThe total time was %lu sec\n\n", timespecDiffSec(&stop, &start));
    return (EXIT_SUCCESS);
}

