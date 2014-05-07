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

    extern int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p);
    
    extern int64_t timespecDiffSec(struct timespec *timeA_p, struct timespec *timeB_p);

#ifdef	__cplusplus
}
#endif

#endif	/* BTIME_H */

