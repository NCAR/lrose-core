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
 * NAME
 *	alloc
 *
 * PURPOSE
 *	header for EG_ allocation routines
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Nov 25, 1992: Created.
 */

#ifndef EG_ALLOC_H
#define EG_ALLOC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/types.h>

struct mem_ptr
{
  void *p;
  int size;
};

void EG_free_mem(struct mem_ptr *ptr_array, int ptr_ct);
void *EG_calloc(size_t nelem, size_t  elsize);
void *EG_malloc(size_t size);
int EG_malloc_amt();
void *EG_realloc(void *ptr, size_t size);
void EG_free(void *ptr);
char **EG_st2_alloc(int rows, int cols, int size);
void EG_st2_free(char **m, int rows, int cols, int size);

#ifdef __cplusplus
}
#endif

#endif /* EG_ALLOC_H */
