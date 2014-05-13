/* 
 * File:   btime.c
 * Author: roberto
 *
 * Created on April 15, 2014, 1:36 PM
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Calculate the difference in time in nanoseconds
 * 
 * @param stop last time 
 * @param start first time
 * @return the time in nanoseconds
 */
float timespecDiff(struct timespec *stop, struct timespec *start) {
    return (float) ((stop->tv_sec * 1000000000) + stop->tv_nsec) -
            ((start->tv_sec * 1000000000) + start->tv_nsec);
}

/**
 * Calculate the difference in time in seconds
 * 
 * @param stop last time 
 * @param start first time
 * @return the time in seconds
 */
float timespecDiffSec(struct timespec *stop, struct timespec *start) {
    return (float) (((stop->tv_sec * 1000000000) + stop->tv_nsec) -
            ((start->tv_sec * 1000000000) + start->tv_nsec)) / (CLOCKS_PER_SEC * 1000);
}