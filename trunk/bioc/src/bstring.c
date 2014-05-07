/* 
 * File:   bstring.c
 * Author: roberto
 *
 * Created on April 15, 2014, 3:05 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "berror.h"

/**
 * Split a string into a array of string using the delimiter
 * 
 * @param dest the destination pointer
 * @param src the source pointer
 * @param delimiter the delimiter
 * @return a array with the splited strings
 */
size_t splitString(char ***dest, char *src, char *delimiter) {
    size_t count = 0;
    char *token;
    char *srccpy = strdup(src);

    *dest = NULL;
    token = strtok(srccpy, delimiter);
    while (token != NULL) {
        *dest = (char **) realloc(*dest, sizeof (char **) * (count + 1));
        checkPointerError(*dest, "Can't reallocate memory", __FILE__, __LINE__, -1);
        (*dest)[count++] = strdup(token);
        token = strtok(NULL, delimiter);
    }
    if (srccpy)free(srccpy);
    return count;
}

/**
 * Free an array of strings
 * 
 * @param str the pointer to be free
 * @param index the number of strings
 */
void freeString(char **str, int index) {
    int i = 0;
    if (str != NULL) {
        for (i = 0; i < index; i++) {
            if (str[i]) free(str[i]);
        }
        free(str);
    }
}