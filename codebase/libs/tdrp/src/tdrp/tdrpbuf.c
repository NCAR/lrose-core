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

/*_____________________________________________________________________________
 *
 * NAME: 
 *	tdrpBuf 
 *
 * 	
 * DESCRIPTION: 
 *	Automatically-resizing accumulating buffer.
 *      Resizes as necessary.
 *	See below for a description of function arguments
 * 
 * ARGUMENTS: 
 *	Describe any function arguments. 
 * 
 * AUTHOR: 
 *	Thomas Wilsher (wilsher@rap.ucar.edu)
 *      Copied to toolsa and revised by Mike Dixon
 * 
 * BUGS: 
 *	None noticed. 
 * 
 * REVISIONS: 
 * 
 */ 

#include <sys/types.h>
#include <ctype.h>  
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <tdrp/tdrp.h>

#define TDRPBUF_START_SIZE 512

/*
 * NOTE ON USE:
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

tdrpBuf * 
tdrpBufCreate(void) 

{ 

  /*	 
   *	Variables. 
   */ 
  
  tdrpBuf *newBuf =(tdrpBuf*) tdrpMalloc(sizeof(tdrpBuf));
  
  /* 
   *	Processing. 
   */ 

  newBuf->buf = (char *)tdrpCalloc(1, TDRPBUF_START_SIZE);
  newBuf->len = 0;
  newBuf->nalloc = TDRPBUF_START_SIZE;
    
  return(newBuf); 

} 

/*___________________________________________________________________________
 * 
 * Destructor
 *___________________________________________________________________________
 */ 

void
tdrpBufDelete(tdrpBuf * thisbuffer)

{

  tdrpFree(thisbuffer->buf);
  tdrpFree(thisbuffer);
  return;

}

/*___________________________________________________________________________
 *
 * Reset the memory buffer - sets current length to 0
 * Zero's out allocated buffer memory.
 *___________________________________________________________________________
 */

void
tdrpBufReset(tdrpBuf * thisbuffer)

{

  thisbuffer->len = 0;
  memset(thisbuffer->buf, 0, thisbuffer->nalloc);
  return;

}

/*___________________________________________________________________________
 * 
 * Prepare a buffer by allocating or reallocating a starting size.
 * This is done if you want to read data directly into the buffer.
 * Note that thisbuffer routine sets things up so that the internal buffer
 * looks like the data has already been added, although that is left
 * up to the calling routine (i.e. the buffer length is numbytes
 * after the call to thisbuffer routine).
 *
 * This routine does not change the existing parts of the buffer,
 * only adjusts the size.
 *
 * Buffer is resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
tdrpBufPrepare(tdrpBuf * thisbuffer, size_t numbytes)

{
  
  tdrpBufAlloc(thisbuffer, numbytes);
  thisbuffer->len = numbytes;
  return(thisbuffer->buf);

}

/*___________________________________________________________________________
 * 
 * Load numbytes from source array into start of target buffer.
 *
 * Buffer is resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
tdrpBufLoad(tdrpBuf * target, void * source, size_t numbytes)

{

  tdrpBufReset(target);
  return(tdrpBufAdd(target, source, numbytes));

}

/*___________________________________________________________________________
 *
 * Add numbytes from source array onto end of buffer.
 *
 * Buffer is resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
tdrpBufAdd(tdrpBuf * target, void * source, size_t numbytes)

{

  tdrpBufGrow(target, numbytes);
  memcpy(target->buf + target->len, source, numbytes);
  target->len += numbytes;
  return(target->buf);

}

/*___________________________________________________________________________
 *
 * Concat the contents of one tdrpBuf onto end of another one.
 *
 * Target buffer is automatically resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
tdrpBufConcat(tdrpBuf * target, tdrpBuf *src)

{

  return (tdrpBufAdd(target, src->buf, src->len));

}

/*___________________________________________________________________________
 * 
 * Duplicates the mem buffer and its contents.
 *
 * Returns pointer to new message buffer.
 *___________________________________________________________________________
 */

tdrpBuf *
tdrpBufDup(tdrpBuf * thisbuffer)

{

  tdrpBuf *newbuf = tdrpBufCreate();
  tdrpBufLoad(newbuf, thisbuffer->buf, thisbuffer->nalloc);
  return(newbuf);

}

/*___________________________________________________________________________
 * 
 * Get the currently-used length of the current buffer - in bytes
 *___________________________________________________________________________
 */

size_t
tdrpBufLen(tdrpBuf * thisbuffer)

{
  return(thisbuffer->len);
}

/*___________________________________________________________________________
 * 
 * Get a pointer to the start of the usable buffer
 *___________________________________________________________________________
 */

void *
tdrpBufPtr(tdrpBuf * thisbuffer)

{
  return(thisbuffer->buf);
}

/*___________________________________________________________________________
 *
 * Print out an tdrpBuf struct. For internal debugging
 *___________________________________________________________________________
 */

void 
tdrpBufPrint(tdrpBuf * thisbuffer, FILE * fp)

{

    fprintf(fp, "buf        = 0x%p\n", thisbuffer->buf);
    fprintf(fp, "len        = %d\n", (int) thisbuffer->len);
    fprintf(fp, "nalloc     = %d\n", (int) thisbuffer->nalloc);
    return;

}

/*___________________________________________________________________________
 *
 * Check available space, alloc as needed
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

void *
tdrpBufAlloc(tdrpBuf * thisbuffer, size_t nbytes_total)

{
  
  size_t new_alloc;

  if(nbytes_total > thisbuffer->nalloc) {
    
    new_alloc = MAX(thisbuffer->nalloc * 2, nbytes_total);
    thisbuffer->buf = (char*)tdrpRealloc(thisbuffer->buf, new_alloc);
    thisbuffer->nalloc = new_alloc;
    
  } else if (nbytes_total < thisbuffer->nalloc / 2) {
    
    new_alloc = thisbuffer->nalloc / 2;
    thisbuffer->buf = (char*)tdrpRealloc(thisbuffer->buf, new_alloc);
    thisbuffer->nalloc = new_alloc;
    if (thisbuffer->len > thisbuffer->nalloc) {
      thisbuffer->len = thisbuffer->nalloc;
    }
    
  }
  
  return (thisbuffer->buf);

}

/*___________________________________________________________________________
 *
 * Check available space, grow if needed
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
tdrpBufGrow(tdrpBuf * thisbuffer, size_t nbytes_needed)

{

  size_t nbytes_total;

  nbytes_total = thisbuffer->len + nbytes_needed;
  tdrpBufAlloc(thisbuffer, nbytes_total);
  return (thisbuffer->buf);

}

