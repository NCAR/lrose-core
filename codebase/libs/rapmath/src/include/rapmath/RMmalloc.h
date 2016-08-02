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
/************************************************
 * umath.h : header file for math utility routines
 ************************************************/

#ifndef _rmmalloc_h
#define _rmmalloc_h

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define RM_mem_zero(a) memset(&(a), 0, sizeof((a)))

/*
 * RMmalloc
 */

/*******************************************************************
 * RMmalloc()
 *
 * normal malloc
 *
 *******************************************************************/

extern void *RMmalloc(size_t size);

/*******************************************************************
 * RMcalloc()
 *
 * malloc of buffer initialized with zeros
 *
 ********************************************************************/

extern void *RMcalloc(size_t num, size_t size);

/*******************************************************************
 * RMrealloc()
 *
 * reallocation
 *
 ********************************************************************/

extern void *RMrealloc(void *ptr, size_t size);

/*******************************************************************
 * RMfree()
 *
 * normal free
 *
 *******************************************************************/

extern void RMfree(void *ptr);

/************************************************************
 * RMfree_non_null()
 *
 * Frees pointer if non_null, sets pointer to null after free.
 *
 * The arg passed in is a pointer to the pointer to be freed.
 * 
 ************************************************************/

extern void RMfree_non_null(void **ptr_p);

/*******************************************************************
 * RMmalloc2()
 *
 * malloc of two-dimensional array
 *
 * The data array is contiguously malloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

extern void **RMmalloc2(size_t m, size_t n, size_t item_size);

/*******************************************************************
 * RMmalloc3()
 *
 * malloc of three-dimensional array
 *
 * The data array is contiguously malloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 * The three_d_pointer points to different parts of the two_d_pointer
 * array.
 *
 *******************************************************************/

extern void ***RMmalloc3(size_t l, size_t m, size_t n, size_t item_size);

/*******************************************************************
 * RMcalloc2()
 *
 * calloc of two-dimensional array
 *
 * The data array is contiguously calloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

extern void **RMcalloc2(size_t m, size_t n, size_t item_size);

/*******************************************************************
 * RMcalloc3()
 *
 * calloc of three-dimensional array
 *
 * The data array is contiguously calloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 * The three_d_pointer points to different parts of the two_d_pointer
 * array.
 *
 *******************************************************************/

extern void ***RMcalloc3(size_t l, size_t m, size_t n, size_t item_size);

/*******************************************************************
 * RMrealloc2()
 *
 * realloc of two-dimensional array
 *
 * The data structure used in RMmalloc2() is realloc'd to include
 * a different number for the m dimension.  Note that if you try
 * to change the n dimension, this will mess up all of your pointers
 * because of the data structures used.
 *
 * Can only be used on arrays allocated using RMmalloc2() or RMcalloc2().
 *
 *******************************************************************/

extern void **RMrealloc2(void **user_addr,
			size_t m, size_t n, size_t item_size);

/*******************************************************************
 * RMfree2()
 *
 * frees 2 dimensional arrau allocated with RMmalloc2 or RMcalloc2
 *
 *******************************************************************/

extern void RMfree2(void **two_d_pointers);

/*******************************************************************
 * RMfree3()
 *
 * frees 3 dimensional arrau allocated with RMmalloc3 or RMcalloc3
 *
 *******************************************************************/

extern void RMfree3(void ***three_d_pointers);

#ifdef __cplusplus
}
#endif

#endif /* _rmmalloc_h */




