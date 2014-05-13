/* 
 * File:   memory.h
 * Author: roberto
 *
 * Created on April 14, 2014, 11:54 AM
 */

#ifndef MEMORY_H
#define	MEMORY_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _CHECK_SELF_P(s) checkPointerError(s, "Null self pointer", __FILE__, __LINE__, -1)

    /**
     * The function allocates memory of size bytes
     * 
     * @param size size in bytes
     * @param file the source code file (__FILE__) or NULL to not print this info
     * @param line the source code line (__LINE__) or 0
     * @return return a pointer to the allocated memory
     */
    extern void *allocate(size_t size, char *file, int line);

    /**
     * The function reallocate memory for a 
     * @param self the pointer to be reallocated
     * @param size size in bytes
     * @param file the source code file (__FILE__) or NULL to not print this info
     * @param line the source code line (__LINE__) or 0
     * @return return a pointer to the reallocated memory
     */
    extern void *reallocate(void *self, size_t size, char *file, int line);

    /**
     * Free an array of pointers
     * 
     * @param str the pointer to be free
     * @param index the number of strings
     */
    extern void freeArrayofPointers(void **pointer, int index);

#ifdef	__cplusplus
}
#endif

#endif	/* MEMORY_H */

