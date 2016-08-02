/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef tdrpBuf_WAS_INCLUDED
#define tdrpBuf_WAS_INCLUDED 

/* 
 * NAME: 
 *	tdrpbuf.h 
 * 
 * SYNOPSIS: 
 *	#include <tdrp/tdrpbuf.h>
 * 
 * DESCRIPTION: 
 *	Header file for "tdrpbuf.c"
 *	Automatically-resizing accumulating buffer.
 *      Resizes as necessary.
 *	See below for a description of function arguments
 *	 
 * AUTHOR: 
 *	Thomas Wilsher (wilsher@rap.ucar.edu)
 *      Copied to toolsa by Mike Dixon.
 *      Copied from toolsa to tdrp by Mike Dixon.
 * 
 */ 
 
#include <stdio.h> 
#include <sys/types.h> 

typedef struct {

  char *buf;           /* pointer to allocated buffer */
  size_t len;          /* number of bytes currently used */
  size_t nalloc;       /* allocated size of buffer */
  
} tdrpBuf;

/*
 * NOTES ON USE:
 *
 * The following functions return the pointer to the user
 * buffer:
 *
 * tdrpBufAlloc, tdrpBufGrow
 * tdrpBufLoad, tdrpBufAdd, tdrpBufConcat
 *
 * When using these, you must set your local buffer pointer
 * to the return value. This allows for the fact that the
 * buffer position may change during a realloc.
 *
 * tdrpBufPtr() may also be used at any time to get the
 * user buffer location.
 * 
 * After using tdrpBufDup(), tdrpBufPtr() must be used to
 * get the pointer to the user buffer.
 *
 */

/*__________________________________________________________________________
 *
 * Constructor
 *__________________________________________________________________________
 */ 

extern tdrpBuf * 
tdrpBufCreate(void);

/*___________________________________________________________________________
 * 
 * Destructor
 *___________________________________________________________________________
 */ 

extern void
tdrpBufDelete(tdrpBuf * bufp);

/*___________________________________________________________________________
 *
 * Reset the memory buffer - sets current length to 0
 * Zero's out allocated buffer memory.
 *___________________________________________________________________________
 */

extern void
tdrpBufReset(tdrpBuf * bufp);

/*___________________________________________________________________________
 * 
 * Prepare a buffer by allocating or reallocating a starting size.
 * This is done if you want to read data directly into the buffer.
 * Note that this routine sets things up so that the internal buffer
 * looks like the data has already been added, although that is left
 * up to the calling routine (i.e. the buffer length is numbytes
 * after the call to this routine).
 *
 * This routine does not change the existing parts of the buffer,
 * only adjusts the size.
 *
 * Buffer is resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

extern void *
tdrpBufPrepare(tdrpBuf * bufp, size_t numbytes);

/*___________________________________________________________________________
 * 
 * Load numbytes from source array into start of target buffer.
 *
 * Buffer is resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

extern void *
tdrpBufLoad(tdrpBuf * target, void * source, size_t numbytes);

/*___________________________________________________________________________
 *
 * Add numbytes from source array onto end of buffer.
 *
 * Buffer is resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

extern void *
tdrpBufAdd(tdrpBuf * target, void * source, size_t numbytes);

/*___________________________________________________________________________
 *
 * Concat the contents of one tdrpbuf onto end of another one.
 *
 * Target buffer is automatically resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

extern void *
tdrpBufConcat(tdrpBuf * target, tdrpBuf *src);

/*___________________________________________________________________________
 * 
 * Duplicates the message buffer and its contents.
 *
 * Returns pointer to new message buffer.
 *___________________________________________________________________________
 */

extern tdrpBuf *
tdrpBufDup(tdrpBuf * bufp);

/*___________________________________________________________________________
 * 
 * Get the currently-used length of the current buffer - in bytes
 *___________________________________________________________________________
 */

extern size_t
tdrpBufLen(tdrpBuf * bufp);

/*___________________________________________________________________________
 * 
 * Get a pointer to the start of the usable buffer
 *___________________________________________________________________________
 */

extern void *
tdrpBufPtr(tdrpBuf * bufp);

/*___________________________________________________________________________
 *
 * Check available space, grow or shrink as needed
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

extern void *
tdrpBufAlloc(tdrpBuf * bufp, size_t nbytes_total);

/*___________________________________________________________________________
 *
 * Check available space, grow if needed
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

extern void *
tdrpBufGrow(tdrpBuf * bufp, size_t nbytes_needed);

/*___________________________________________________________________________
 *
 * Print out an tdrpBuf struct. For internal debugging
 *___________________________________________________________________________
 */

extern void 
tdrpBufPrint(tdrpBuf * bufp, FILE * fp);


#endif 
#ifdef __cplusplus
}
#endif
