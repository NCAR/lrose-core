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
 *	membuf 
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

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define _MEMBUF_INTERNAL

/*
 * MemBuf struct is private - it should not  be accessed by
 * external routines
 */

typedef struct {

  char *buf;           /* pointer to allocated buffer */
  size_t len;          /* number of bytes currently used */
  size_t nalloc;       /* allocated size of buffer */
  
} MEMbuf;

#include <toolsa/umisc.h>

#define MEMBUF_START_SIZE 512

/*
 * NOTE ON USE:
 *
 * The following functions return the pointer to the user
 * buffer:
 *
 * MEMbufAlloc, MEMbufGrow
 * MEMbufLoad, MEMbufAdd, MEMbufConcat
 *
 * When using these, you must set your local buffer pointer
 * to the return value. This allows for the fact that the
 * buffer position may change during a realloc.
 *
 * MEMbufPtr() may also be used at any time to get the
 * user buffer location.
 * 
 * After using MEMbufDup(), MEMbufPtr() must be used to
 * get the pointer to the user buffer.
 *
 */

/*__________________________________________________________________________
 *
 * Default constructor
 *__________________________________________________________________________
 */ 

MEMbuf * 
MEMbufCreate(void) 

{ 

  /*	 
   *	Variables. 
   */ 
  
  MEMbuf *newBuf = umalloc(sizeof(MEMbuf));
  
  /* 
   *	Processing. 
   */ 

  newBuf->buf = (void *)ucalloc(1, MEMBUF_START_SIZE);
  newBuf->len = 0;
  newBuf->nalloc = MEMBUF_START_SIZE;
    
  return(newBuf); 

} 

/*__________________________________________________________________________
 *
 * Copy constructor
 *__________________________________________________________________________
 */ 

MEMbuf * 
MEMbufCreateCopy(MEMbuf *rhs) 

{ 

  MEMbuf *newBuf = MEMbufCreate();
  MEMbufAdd(newBuf, MEMbufPtr(rhs), MEMbufLen(rhs));
    
  return(newBuf); 

} 

/*___________________________________________________________________________
 * 
 * Destructor
 *___________________________________________________________________________
 */ 

void
MEMbufDelete(MEMbuf * thisbuf)

{

  ufree(thisbuf->buf);
  ufree(thisbuf);
  return;

}

/*___________________________________________________________________________
 * 
 * Delete handle, but do not delete the buffer
 * so that the memory ownership can be transferred.
 * The memory must be freed by the calling routine to avoid a leak.
 * Returns a pointed to the buffer.
 *___________________________________________________________________________
 */ 

void *
MEMbufDeleteHandle(MEMbuf *thisbuf)

{

  char *bufPtr = thisbuf->buf; /* pointer to allocated buffer */
  ufree(thisbuf); /* free the handle */
  return bufPtr; /* return pointer to buffer, to be freed by caller */
}

/*___________________________________________________________________________
 *
 * Reset the memory buffer - sets current length to 0
 * Zero's out allocated buffer memory.
 *___________________________________________________________________________
 */

void
MEMbufReset(MEMbuf * thisbuf)

{

  thisbuf->len = 0;
  memset(thisbuf->buf, 0, thisbuf->nalloc);
  return;

}

/*___________________________________________________________________________
 *
 * Free up memory allocated for data. MEMbuf still valid.
 *___________________________________________________________________________
 */

void
MEMbufFree(MEMbuf * thisbuf)

{
  
  thisbuf->buf = urealloc(thisbuf->buf, MEMBUF_START_SIZE);
  thisbuf->len = 0;
  thisbuf->nalloc = MEMBUF_START_SIZE;
  return;

}

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

void *
MEMbufPrepare(MEMbuf * thisbuf, size_t numbytes)

{
  
  MEMbufAlloc(thisbuf, numbytes);
  thisbuf->len = numbytes;
  return(thisbuf->buf);

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
MEMbufLoad(MEMbuf * target, void * source, size_t numbytes)

{

  MEMbufReset(target);
  return(MEMbufAdd(target, source, numbytes));

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
MEMbufAdd(MEMbuf * target, void * source, size_t numbytes)

{

  MEMbufGrow(target, numbytes);
  memcpy(target->buf + target->len, source, numbytes);
  target->len += numbytes;
  return(target->buf);

}

/*___________________________________________________________________________
 *
 * Concat the contents of one membuf onto end of another one.
 *
 * Target buffer is automatically resized as necessary.
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
MEMbufConcat(MEMbuf * target, MEMbuf *src)

{

  return (MEMbufAdd(target, src->buf, src->len));

}

/*___________________________________________________________________________
 * 
 * Duplicates the mem buffer and its contents.
 *
 * Returns pointer to new message buffer.
 *___________________________________________________________________________
 */

MEMbuf *
MEMbufDup(MEMbuf * thisbuf)

{

  MEMbuf *newbuf = MEMbufCreate();
  MEMbufLoad(newbuf, thisbuf->buf, thisbuf->nalloc);
  return(newbuf);

}

/*___________________________________________________________________________
 * 
 * Get the currently-used length of the current buffer - in bytes
 *___________________________________________________________________________
 */

size_t
MEMbufLen(MEMbuf * thisbuf)

{
  return(thisbuf->len);
}

/*___________________________________________________________________________
 * 
 * Get a pointer to the start of the usable buffer
 *___________________________________________________________________________
 */

void *
MEMbufPtr(MEMbuf * thisbuf)

{
  return(thisbuf->buf);
}

/*___________________________________________________________________________
 *
 * Print out an MEMbuf struct. For internal debugging
 *___________________________________________________________________________
 */

void 
MEMbufPrint(MEMbuf * thisbuf, FILE * fp)

{

    fprintf(fp, "buf        = 0x%p\n", thisbuf->buf);
    fprintf(fp, "len        = %d\n", (int) thisbuf->len);
    fprintf(fp, "nalloc     = %d\n", (int) thisbuf->nalloc);
    return;

}

/*___________________________________________________________________________
 *
 * Check available space, alloc as needed
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
MEMbufAlloc(MEMbuf * thisbuf, size_t nbytes_total)

{
  
  size_t new_alloc;

  if(nbytes_total > thisbuf->nalloc) {

    new_alloc = MAX(thisbuf->nalloc * 2, nbytes_total);
    thisbuf->buf = (char*)urealloc(thisbuf->buf, new_alloc);
    thisbuf->nalloc = new_alloc;
    
  } else if (nbytes_total < thisbuf->nalloc / 2) {
    
    new_alloc = thisbuf->nalloc / 2;
    thisbuf->buf = (char*)urealloc(thisbuf->buf, new_alloc);
    thisbuf->nalloc = new_alloc;
    if (thisbuf->len > thisbuf->nalloc) {
      thisbuf->len = thisbuf->nalloc;
    }
    
  }
  
  return (thisbuf->buf);

}

/*___________________________________________________________________________
 *
 * Check available space, grow if needed
 *
 * Returns pointer to user buffer.
 *___________________________________________________________________________
 */

void *
MEMbufGrow(MEMbuf * thisbuf, size_t nbytes_needed)

{

  size_t nbytes_total;

  nbytes_total = thisbuf->len + nbytes_needed;
  MEMbufAlloc(thisbuf, nbytes_total);
  return (thisbuf->buf);

}

