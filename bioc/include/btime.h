/* 
 * File:   btime.h
 * Author: roberto
 *
 * Created on April 15, 2014, 1:37 PM
 */

#ifndef BTIME_H
#define	BTIME_H

#ifdef	__cplusplus
extern "C" {
#endif
    /**
     * Calculate the difference in time in nanoseconds
     * 
     * @param stop last time 
     * @param start first time
     * @return the time in nanoseconds
     */
    extern float timespecDiff(struct timespec *stop, struct timespec *start);
    
    /**
     * Calculate the difference in time in seconds
     * 
     * @param stop last time 
     * @param start first time
     * @return the time in seconds
     */
    extern float timespecDiffSec(struct timespec *stop, struct timespec *start);

#ifdef	__cplusplus
}
#endif

#endif	/* BTIME_H */

