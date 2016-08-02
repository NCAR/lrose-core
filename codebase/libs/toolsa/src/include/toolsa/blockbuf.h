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

#ifndef BLOCKBUF_WAS_INCLUDED
#define BLOCKBUF_WAS_INCLUDED 

/* 
 * NAME: 
 *	blockbuf.h 
 * 
 * SYNOPSIS: 
 *	#include <toolsa/blockbuf.h>
 * 
 * DESCRIPTION: 
 *	Header file for "blockbuf.c"
 *	Block-allocated memory module.
 *
 *	See below for a description of function arguments
 *	 
 * AUTHOR: 
 *	Mike Dixon
 * 
 */ 
 
#include <toolsa/mem.h>

/*
 * Block-allocated memory is intended for applications which allocate
 * a very large number of small memory regions, and then needs to free
 * all of them at once, or reuse the allocated memory.
 *
 * Behavior.
 * --------
 *
 * BLOCKinit() allocates a memory segment of size n_init, and
 * intializes the BLOCKbuf_t struct.
 *
 * As the BLOCKmalloc() or BLOCKcalloc() routines are repeatedly used
 * a pointer is returned to sections of the segment. These sections
 * are allocated in order. The start of each section is aligned,.
 * By default alignment is set for double types. A call to
 * BLOCKalign will set the alignment byte number.
 *
 * If the current segment runs out of space, another segment is
 * allocated equal in size to all of the block memory used so 
 * far plus the size requested. This always more than doubles
 * the allocated space.
 * Then, allocations are made from the new segment.
 *
 * A call to BLOCKreset() behaves as follows:
 *
 *   If only 1 segment has been allocated, that segment is reset
 *   for use.
 *
 *   If more than one segment exists, all are freed and a single
 *   segment is allocated in their place, equal in size to the
 *   size of memory previously used.
 *
 * A call to BLOCKfree() frees up all allocated memory associated with
 * this block.
 */

/*
 * structure types
 */

typedef struct {

  void *array;       /* allocated memory */
  size_t len;        /* lenth of array */
  size_t used;       /* number of bytes used */
  size_t unused;     /* number of bytes unused */

} BLOCKseg_t;

typedef struct {

  int nsegs;              /* number of segments */
  int nsegs_alloc;        /* length of segs array */
  int n_align;            /* byte-alignment boundary */
  BLOCKseg_t *current;    /* current segment */
  BLOCKseg_t *segs;       /* segment array */
  size_t sum_used;        /* total number of bytes used */
  size_t sum_alloc;       /* total number of bytes allocated */

} BLOCKbuf_t;

/*
 * function prototypes
 */

/*************************
 * BLOCKinit()
 *
 * Initialize BLOCK buffer with n_init bytes.
 */

extern void BLOCKinit(BLOCKbuf_t *buf, size_t n_init);

/********************************************************
 * BLOCKalign()
 *
 * Set block alignment - default is to align for doubles
 *
 * Note that this routine should only be used when there
 * is no data in the buffer (right after BLOCKinit() or
 * BLOCKreset().  If there is already data in the buffer,
 * the alignment may be off.
 */

extern void BLOCKalign(BLOCKbuf_t *buf, int n_align);

/****************************************
 * BLOCKmalloc()
 *
 * malloc from block
 *
 */

extern void *BLOCKmalloc(BLOCKbuf_t *buf, size_t n_needed);
/****************************************
 * BLOCKcalloc()
 *
 * calloc from block
 *
 */

extern void *BLOCKcalloc(BLOCKbuf_t *buf, size_t n_needed);

/****************************************
 * BLOCKreset()
 *
 * Reset block for reuse. If nsegs > 1, consolidates
 * segments into a single segment which is as large
 * as the previously-used memory.
 */

extern void BLOCKreset(BLOCKbuf_t *buf);

/****************************************
 * BLOCKfree()
 *
 * Free up all memory associated with buf
 */

extern void BLOCKfree(BLOCKbuf_t *buf);

#endif 
#ifdef __cplusplus
}
#endif
