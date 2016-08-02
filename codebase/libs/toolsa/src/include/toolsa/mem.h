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

#ifndef MEM_WAS_INCLUDED
#define MEM_WAS_INCLUDED

#include <sys/types.h>
#include <memory.h>

#define MEM_zero(a) memset(&(a), 0, sizeof((a)))

extern void *MEM_alloc(size_t size);
extern void *MEM_calloc (size_t nelem, size_t elsize);
extern void *MEM_realloc (void *ptr, size_t size);
extern void MEM_free (void *ptr);

#include <stdlib.h>

/*******************************************************************
 * umalloc()
 *
 * normal malloc
 *
 *******************************************************************/

extern void *umalloc(size_t size);

/*******************************************************************
 * umalloc_min_1()
 *
 * umalloc, but if size == 0, at least 1 bytes is allocated
 * so that the returned pointer is not null
 *
 *******************************************************************/

extern void *umalloc_min_1(size_t size);

/*******************************************************************
 * ucalloc()
 *
 * malloc of buffer initialized with zeros
 *
 ********************************************************************/

extern void *ucalloc(size_t num, size_t size);

/*******************************************************************
 * ucalloc_min_1()
 *
 * ucalloc, but if num == 0 or size == 0, at least 1 byte
 * is allocated so that the returned pointer is not null
 *
 *******************************************************************/

void *(ucalloc_min_1)(size_t num, size_t size);

/*******************************************************************
 * urealloc()
 *
 * reallocation
 *
 ********************************************************************/

extern void *urealloc(void *ptr, size_t size);

/*******************************************************************
 * ufree()
 *
 * normal free
 *
 *******************************************************************/

extern void ufree(void *ptr);

/************************************************************
 * ufree_non_null()
 *
 * Frees pointer if non_null, sets pointer to null after free.
 *
 * The arg passed in is a pointer to the pointer to be freed.
 * 
 ************************************************************/

extern void ufree_non_null(void **ptr_p);

/*******************************************************************
 * umalloc_debug()
 *
 * sets the debug level
 *
 * 0+ no debugging
 * 1+ check for block corruption on free
 * 2+ as above plus keep malloc addr list, verify all entries
 *    on each malloc, calloc or realloc
 * 3+ as above plus prints to stderr
 *
 *******************************************************************/

extern void umalloc_debug(int level);

/*******************************************************************
 * umalloc2()
 *
 * malloc of two-dimensional array
 *
 * The data array is contiguously malloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

extern void **umalloc2(size_t m, size_t n, size_t item_size);

/*******************************************************************
 * umalloc3()
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

extern void ***umalloc3(size_t l, size_t m, size_t n, size_t item_size);

/*******************************************************************
 * ucalloc2()
 *
 * calloc of two-dimensional array
 *
 * The data array is contiguously calloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

extern void **ucalloc2(size_t m, size_t n, size_t item_size);

/*******************************************************************
 * ucalloc3()
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

extern void ***ucalloc3(size_t l, size_t m, size_t n, size_t item_size);

/*******************************************************************
 * urealloc2()
 *
 * realloc of two-dimensional array
 *
 * The data structure used in umalloc2() is realloc'd to include
 * a different number for the m dimension.  Note that if you try
 * to change the n dimension, this will mess up all of your pointers
 * because of the data structures used.
 *
 * Can only be used on arrays allocated using umalloc2() or ucalloc2().
 *
 *******************************************************************/

extern void **urealloc2(void **user_addr,
			size_t m, size_t n, size_t item_size);

/*******************************************************************
 * ufree2()
 *
 * frees 2 dimensional arrau allocated with umalloc2 or ucalloc2
 *
 *******************************************************************/

extern void ufree2(void **two_d_pointers);

/*******************************************************************
 * ufree3()
 *
 * frees 3 dimensional arrau allocated with umalloc3 or ucalloc3
 *
 *******************************************************************/

extern void ufree3(void ***three_d_pointers);

/*******************************************************************
 * umalloc_map()
 *
 * print out table of malloc'd blocks
 *
 *******************************************************************/

extern void umalloc_map(void);

/*******************************************************************
 * umalloc_count()
 *
 * count size of malloc'd blocks
 *
 *******************************************************************/

extern int umalloc_count(void);

/*******************************************************************
 * umalloc_verify()
 *
 * verify malloc'd entries
 *
 *******************************************************************/

extern void umalloc_verify(void);

#include <toolsa/membuf.h>

#endif /* MEM_WAS_INCLUDED */

#ifdef __cplusplus
}
#endif


