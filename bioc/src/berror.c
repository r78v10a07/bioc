/* 
 * File:   error.c
 * Author: roberto
 *
 * Created on April 14, 2014, 11:52 AM
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * Print a message in the file and exit if the exit_status is different of zero
 * 
 * @param out the file to printout the message
 * @param msg the message to be printed out
 * @param file the source code file (__FILE__) or NULL to not print this info
 * @param line the source code line (__LINE__) or 0
 * @param exit_status the exit status (Exit the program is this number is different of zero)
 */
void printLog(FILE *out, char *msg, char *file, int line, int exit_status) {
    if (file) {
        fprintf(out, "%s. File: %s Line: %d\t", msg, file, line);
    } else {
        fprintf(out, "%s", msg);
    }
    if (exit_status != 0) {
        fprintf(out, "\n");
        exit(exit_status);
    }
}

/**
 * Check if the void pointer is NULL and print out a message. If the exit_status
 * is different of zero it exit the program 
 * 
 * @param data the pointer to check
 * @param msg the message to be printed out
 * @param file the source code file (__FILE__) or NULL to not print this info
 * @param line the source code line (__LINE__) or 0
 * @param exit_status the exit status (Exit the program is this number is different of zero)
 * @return the same pointer if everything is fine
 */
void *checkPointerError(void *data, char *msg, char *file, int line, int exit_status) {
    if (!data) {
        printLog(stderr, msg, file, line, exit_status);
    }
    return data;
}

/**
 * Print the FILE and LINE info
 * 
 * @param file the file name (__FILE__)
 * @param line the line name (__LINE__)
 */
void printFileLine(char *file, int line) {
    printf("%s %d\n", file, line);
    fflush(stdout);
}

