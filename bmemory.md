# Functions to manage the memory allocation #

[bmemory.h](https://bioc.googlecode.com/svn/trunk/bioc/include/bmemory.h)  [bmemory.c](https://bioc.googlecode.com/svn/trunk/bioc/src/bmemory.c)

```
#include <stdio.h>
#include <stdlib.h>
#include "bmemory.h"
```

## The function allocates memory of size bytes ##

```
    /**
     * @param size size in bytes
     * @param file the source code file (__FILE__) or NULL to not print this info
     * @param line the source code line (__LINE__) or 0
     * @return return a pointer to the allocated memory
     */
    extern void *allocate(size_t size, char *file, int line);
```

Usage:

```
  /* Allocate the memory
   * If error exit the program and print the source file and line
   */ 
  char * a = allocate(sizeof(char) * 10, __FILE__, __LINE__);
```

## The function reallocate memory of size bytes ##

```
    /**
     * @param self the pointer to be reallocated
     * @param size size in bytes
     * @param file the source code file (__FILE__) or NULL to not print this info
     * @param line the source code line (__LINE__) or 0
     * @return return a pointer to the reallocated memory
     */
    extern void *reallocate(void *self, size_t size, char *file, int line);
```

Usage:

```
  /* Reallocate the memory
   * If error exit the program and print the source file and line
   */ 
  a = reallocate(a, sizeof(char) * 20, __FILE__, __LINE__);
```

## Free an array of pointers ##

```
    /**
     * @param str the pointer to be free
     * @param index the number of strings
     */
    extern void freeArrayofPointers(void **pointer, int index);

```

Usage: see [String management](bstring.md)