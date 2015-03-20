# Functions to manage the time #

[btime.h](https://bioc.googlecode.com/svn/trunk/bioc/include/btime.h)  [btime.c](https://bioc.googlecode.com/svn/trunk/bioc/src/btime.c)

```
#include <time.h>
#include "btime.h"
```

## Calculate the time difference in nanoseconds ##

```
    /**
     * @param stop last time 
     * @param start first time
     * @return the time in nanoseconds
     */
    extern float timespecDiff(struct timespec *stop, struct timespec *start);
```

Usage:

```
   struct timespec start, stop;

   ...

   clock_gettime(CLOCK_MONOTONIC, &start);

   ...

   clock_gettime(CLOCK_MONOTONIC, &stop);
   printf("The total time was %.1f nanosec\n", timespecDiff(&stop, &start));
```

## Calculate the time difference in seconds ##

```
    /**
     * @param stop last time 
     * @param start first time
     * @return the time in seconds
     */
    extern float timespecDiffSec(struct timespec *stop, struct timespec *start);
```

Usage:

```
   struct timespec start, stop;

   ...

   clock_gettime(CLOCK_MONOTONIC, &start);

   ...

   clock_gettime(CLOCK_MONOTONIC, &stop);
   printf("The total time was %.1f sec\n", timespecDiffSec(&stop, &start));
```