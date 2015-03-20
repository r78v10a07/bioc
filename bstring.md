# Functions to manage the strings #

[bstring.h](https://bioc.googlecode.com/svn/trunk/bioc/include/bstring.h)  [bstring.c](https://bioc.googlecode.com/svn/trunk/bioc/src/bstring.c)

```
#include <stdio.h>
#include <string.h>
#include "bstring.h"
```

## Split a string into an array of strings using the delimiter ##

```
    /**
     * @param dest the destination pointer
     * @param src the source pointer
     * @param delimiter the delimiter
     * @return a array with the splited strings
     */
    extern size_t splitString(char ***dest, char *src, char *delimiter);
```

Usage:

```
    int i;
    char *line = "string1 to cut|string2 to cut|string 3 to cut";
    char **ids;
    int ids_number;

    ids_number = splitString(&ids, line, "|");
    for (i = 0; i < ids_number; i++) {
        printf("%d %s\n",i,ids[i]);
    }
     
    freeArrayofPointers((void **) ids, ids_number);

/*
 This will print out:

0 string1 to cut
1 string2 to cut
2 string 3 to cut
*/
```

## Compare if the string ends with the sufix ##

```
    /**
     * @param haystack the string to be analyzed
     * @param needle the suffix string
     * @return 0 if the string haystack ends with the string needle, 1 if not
     */
    int strbcmp(const char *haystack, const char *needle);

```

Usage:

```
   char *fileName = "myfile.gz";
   char *ext = ".gz";

   int result = strbcmp(fileName, ext);

/*
The result variable will be 0 because fileName ends with the .gz 
*/
```