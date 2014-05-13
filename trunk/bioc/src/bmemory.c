/* 
 * File:   memory.c
 * Author: roberto
 *
 * Created on April 14, 2014, 11:54 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "berror.h"

/**
 * The function allocates memory of size bytes
 * 
 * @param size size in bytes
 * @param file the source code file (__FILE__) or NULL to not print this info
 * @param line the source code line (__LINE__) or 0
 * @return return a pointer to the allocated memory
 */
void *allocate(size_t size, char *file, int line) {
    return checkPointerError(malloc(size), "Can't allocate memory", file, line, -1);
}

/**
 * The function reallocate memory for a 
 * @param self the pointer to be reallocated
 * @param size size in bytes
 * @param file the source code file (__FILE__) or NULL to not print this info
 * @param line the source code line (__LINE__) or 0
 * @return return a pointer to the reallocated memory
 */
void *reallocate(void *self, size_t size, char *file, int line){
    return checkPointerError(realloc(self,size), "Can't reallocate memory", file, line, -1);
}

/**
 * Free an array of pointers
 * 
 * @param str the pointer to be free
 * @param index the number of strings
 */
void freeArrayofPointers(void **pointer, int index) {
    int i = 0;
    if (pointer) {
        for (i = 0; i < index; i++) {
            if (pointer[i]) free(pointer[i]);
        }
        free(pointer);
    }
}