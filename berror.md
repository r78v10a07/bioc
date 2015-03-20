# Functions to manage the errors #

[berror.h](https://bioc.googlecode.com/svn/trunk/bioc/include/berror.h)  [berror.c](https://bioc.googlecode.com/svn/trunk/bioc/src/berror.c)

```
#include <stdio.h>
#include <stdlib.h>
#include "berror.h"
```

## Print a message in the file and exit if the exit\_status is different of zero ##
```
    /**
     * @param out the file to printout the message
     * @param msg the message to be printed out
     * @param file the source code file (__FILE__) or NULL to not print this info
     * @param line the source code line (__LINE__) or 0
     * @param exit_status the exit status (Exit the program is this number is different of zero)
     */
    extern void printLog(FILE *out, char *msg, char *file, int line, int exit_status);
```

Usage:

```
   // Print this log to the stdout adding the source file name and line
   printLog(stdout,"This is a log message",__FILE__, __LINE__, 0);
```

## Check if the void pointer is NULL and print out a message ##

If the exit\_status is different of zero it exit the program

```
    /**
     * @param data the pointer to check
     * @param msg the message to be printed out
     * @param file the source code file (__FILE__) or NULL to not print this info
     * @param line the source code line (__LINE__) or 0
     * @param exit_status the exit status (Exit the program is this number is different of zero)
     * @return the same pointer if everything is fine
     */
    extern void *checkPointerError(void *data, char *msg, char *file, int line, int exit_status);
```

Usage:

```
   /* Open the file myTestfile.txt
    * If an error occur The message "Can't open the file" is printed
    * The program ends due to the last option -1
    */  
   FILE *fd = checkPointerError(fopen("myTestfile.txt","r"), "Can't open the file", __FILE__, __LINE__, -1);

   /* For allocate memory */
   char *a = checkPointerError(malloc(sizeof(char) * 10), "Can't allocate memory", __FILE__, __LINE__, -1);
  
```

## Print the source FILE and LINE info ##

```
     #define PRINTFILELINE printFileLine(__FILE__, __LINE__)

    /**
     * Print the FILE and LINE info
     * 
     * @param file the file name (__FILE__)
     * @param line the line name (__LINE__)
     */
    extern void printFileLine(char *file, int line);
```

Usage:

```
   //For debugging porpoises print the source file name and line
   PRINTFILELINE;
```