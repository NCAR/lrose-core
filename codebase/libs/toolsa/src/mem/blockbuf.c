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
/*
 *  Module BLOCK, handles block-based memory allocation.
 *
 * See blobkbuf.h for details.
 *
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000, USA
 *
 * Sept 1996
 */

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
 
#include <toolsa/globals.h>
#include <toolsa/blockbuf.h>

#define BLOCK_NALLOC 8

/*
 * file scope prototypes
 */

static void *alloc_from_seg(BLOCKseg_t *seg, size_t n_needed,
			    int n_align, size_t *n_alloc_p);
static size_t alloc_seg(BLOCKseg_t *seg, size_t n_alloc, int n_align);
static void free_seg(BLOCKseg_t *seg);
static void reset_seg(BLOCKseg_t *seg);

/*************************
 * BLOCKinit()
 *
 * Initialize BLOCK buffer with n_init bytes.
 */

void BLOCKinit(BLOCKbuf_t *buf, size_t n_init)

{

  size_t n_alloc;

  buf->nsegs_alloc = BLOCK_NALLOC;
  buf->segs = umalloc(buf->nsegs_alloc * sizeof(BLOCKseg_t));

  buf->nsegs = 1;
  buf->current = buf->segs;
  buf->sum_used = 0;
  buf->n_align = sizeof(double);
  n_alloc = alloc_seg(buf->current, n_init, buf->n_align);
  buf->sum_alloc = n_alloc;

}

/********************************************************
 * BLOCKalign()
 *
 * Set block alignment - default is to align for doubles.
 *
 * Note that this routine should only be used when there
 * is no data in the buffer (right after BLOCKinit() or
 * BLOCKreset().  If there is already data in the buffer,
 * the alignment may be off.
 */

void BLOCKalign(BLOCKbuf_t *buf, int n_align)

{
  
  buf->n_align = n_align;
  
}

/****************************************
 * BLOCKmalloc()
 *
 * malloc from block
 *
 */

void *BLOCKmalloc(BLOCKbuf_t *buf, size_t n_needed)

{

  void *user_ptr;
  size_t n_requested;
  size_t n_alloc;

  /*
   * try to allocate from current segment
   */
  
  user_ptr = alloc_from_seg(buf->current, n_needed,
			    buf->n_align, &n_alloc);

  if (user_ptr != NULL) {

    /*
     * allocate from current segment
     */
    
    buf->sum_used += n_alloc;
    return (user_ptr);

  } else {

    /*
     * Not enough memory in current segment
     */

    /*
     * realloc segment pointer array as needed.  Set the current
     * seg ptr to the last segment in the previously allocated seg
     * array.  This pointer will be incremented before allocating
     * the segment, just as it is if no new segment needed to be
     * allocated.
     */

    if (buf->nsegs >= buf->nsegs_alloc) {
      buf->nsegs_alloc += BLOCK_NALLOC;
      buf->segs = urealloc(buf->segs,
			   buf->nsegs_alloc * sizeof(BLOCKseg_t));
      buf->current = buf->segs + buf->nsegs - 1;
    }

    /*
     * alloc new seg to be size sum of all previous segs used plus
     * n_needed. This always at least doubles the segment size.
     */
    
    n_requested = n_needed + buf->sum_used;

    buf->current++;
    n_alloc = alloc_seg(buf->current, n_requested, buf->n_align);
    buf->sum_alloc += n_alloc;
    buf->nsegs++;
    
    /*
     * allocate from new segment
     */

    user_ptr = alloc_from_seg(buf->current, n_needed,
			      buf->n_align, &n_alloc);

    buf->sum_used += n_alloc;

    return (user_ptr);

  } /* if (user_ptr != NULL) */
  
}

/****************************************
 * BLOCKcalloc()
 *
 * calloc from block
 *
 */

void *BLOCKcalloc(BLOCKbuf_t *buf, size_t n_needed)

{

  void *user_ptr;

  user_ptr = BLOCKmalloc(buf, n_needed);
  memset(user_ptr, 0, n_needed);

  return (user_ptr);

}

/****************************************
 * BLOCKreset()
 *
 * Reset block for reuse. If nsegs > 1, consolidates
 * segments into a single segment which is as large
 * as the previously-used memory.
 */

void BLOCKreset(BLOCKbuf_t *buf)

{

  int i;
  size_t n_alloc;
  BLOCKseg_t *seg;

  if (buf->nsegs > 1) {

    /*
     * free up all segs
     */

    seg = buf->segs;
    for (i = 0; i < buf->nsegs; i++, seg++) {
      free_seg(seg);
    }

    /*
     * allocate single segment
     */

    buf->nsegs = 1;
    buf->current = buf->segs;
    n_alloc = alloc_seg(buf->current, buf->sum_used, buf->n_align);
    buf->sum_used = 0;
    buf->sum_alloc = n_alloc;

  } else {

    reset_seg(buf->current);
    buf->sum_used = 0;

  }

}

/****************************************
 * BLOCKfree()
 *
 * Free up all memory associated with buf
 */

void BLOCKfree(BLOCKbuf_t *buf)

{

  int i;
  BLOCKseg_t *seg;

  seg = buf->segs;
  
  for (i = 0; i < buf->nsegs; i++, seg++) {
    free_seg(seg);
  }

  ufree(buf->segs);

}

/********************************************************
 * alloc_from_seg()
 *
 * Allocate section of memory from a segment.
 * Allocated block is aligned.
 *
 * Returns pointer to memory allocated, NULL if there
 * is not enough space in segment.
 *
 * Sets *n_alloc_p to the number of bytes allocated.
 */

static void *alloc_from_seg(BLOCKseg_t *seg, size_t n_needed,
			    int n_align, size_t *n_alloc_p)

{
  
  void *user_ptr;
  size_t n_aligned;
  
  n_aligned = (((n_needed - 1) / n_align) + 1) * n_align;

  if (seg->unused >= n_aligned) {

    user_ptr = ((char *) seg->array + seg->used);
    seg->used += n_aligned;
    seg->unused -= n_aligned;
    *n_alloc_p = n_aligned;
    return (user_ptr);

  } else {

    *n_alloc_p = 0;
    return (NULL);

  }

}

/********************************************************
 * alloc_seg()
 *
 * Allocate memory for a segment, set up segment counters.
 * Allocated block is aligned.
 *
 * Returns number of bytes allocated.
 */

static size_t alloc_seg(BLOCKseg_t *seg, size_t n_alloc, int n_align)

{

  size_t n_aligned;

  n_aligned = (((n_alloc - 1) / n_align) + 1) * n_align;

  seg->array = umalloc(n_aligned);
  seg->len = n_aligned;
  seg->used = 0;
  seg->unused = n_aligned;

  return (n_aligned);

}

/****************************
 * free_seg()
 *
 * Free segment memory
 */

static void free_seg(BLOCKseg_t *seg)

{

  ufree(seg->array);

}

/****************************
 * reset_seg()
 *
 * Reset segment counters
 */

static void reset_seg(BLOCKseg_t *seg)

{

  seg->used = 0;
  seg->unused = seg->len;

}


