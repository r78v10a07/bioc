/* 
 * File:   error.h
 * Author: roberto
 *
 * Created on April 14, 2014, 11:52 AM
 */

#ifndef ERROR_H
#define	ERROR_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define PRINTFILELINE printFileLine(__FILE__, __LINE__)

    /**
     * Print a message in the file and exit if the exit_status is different of zero
     * 
     * @param out the file to printout the message
     * @param msg the message to be printed out
     * @param file the source code file (__FILE__) or NULL to not print this info
     * @param line the source code line (__LINE__) or 0
     * @param exit_status the exit status (Exit the program is this number is different of zero)
     */
    extern void printLog(FILE *out, char *msg, char *file, int line, int exit_status);

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
    extern void *checkPointerError(void *data, char *msg, char *file, int line, int exit_status);

    /**
     * Print the FILE and LINE info
     * 
     * @param file the file name (__FILE__)
     * @param line the line name (__LINE__)
     */
    extern void printFileLine(char *file, int line);

#ifdef	__cplusplus
}
#endif

#endif	/* ERROR_H */

