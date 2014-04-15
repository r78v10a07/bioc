/* 
 * File:   bstring.h
 * Author: roberto
 *
 * Created on April 15, 2014, 3:05 PM
 */

#ifndef BSTRING_H
#define	BSTRING_H

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * Split a string into a array of string using the delimiter
     * 
     * @param dest the destination pointer
     * @param src the source pointer
     * @param delimiter the delimiter
     * @return a array with the splited strings
     */
    extern size_t splitString(char ***dest, char *src, char *delimiter);

    /**
     * Free an array of strings
     * 
     * @param str the pointer to be free
     * @param index the number of strings
     */
    void freeString(char **str, int index);


#ifdef	__cplusplus
}
#endif

#endif	/* BSTRING_H */

