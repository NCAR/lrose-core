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
/**********************************************************************
 * RMmalloc.c
 *
 * Dynamic memory allocation routines with error checking.
 *
 * Use RMmalloc(), RMrealloc(), RMcalloc() and
 * RMfree() in place of malloc(), realloc(), calloc(), and free().
 *
 * Copied over from toolsa umalloc module.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * Nov 2001
 *
 **********************************************************************/

#include <rapmath/RMmalloc.h>
#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <unistd.h> 
#include <math.h>

static void RMmalloc_exit(void);

/*******************************************************************
 * RMmalloc()
 *
 * normal malloc
 *
 *******************************************************************/

void *RMmalloc(size_t size)

{

  void *addr;

  /*
   * return NULL if size is 0
   */

  if (size == 0) {
    return ((void *) NULL);
  }

  /*
   * allocate
   */

  if ((addr = malloc(size)) == NULL) {
    fprintf(stderr, "ERROR - RMmalloc\n");
    fprintf(stderr, "Cannot perform malloc, size = %d\n", (int) size);
    RMmalloc_exit();
  }

  /*
   * return pointer
   */

  return(addr);

}

/*******************************************************************
 * RMmalloc2()
 *
 * malloc of two-dimensional array
 *
 * The data array is contiguously malloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

void **RMmalloc2(size_t m, size_t n, size_t item_size)

{

  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  size_t i;

  /*
   * allocate the pointer array
   */

  two_d_pointers = (char **) RMmalloc (m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) RMmalloc (m * n * item_size);

  /*
   * load up the pointer array to point to different parts of
   * the buffer
   */

  tmp_2d_ptr = two_d_pointers;
  tmp_buf = buffer;

  for (i = 0; i < m; i++) {
    *tmp_2d_ptr = tmp_buf;
    tmp_2d_ptr++;
    tmp_buf += (n * item_size);
  }

  /*
   * return pointer array address
   */

  return ((void **)two_d_pointers);

}

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

void ***RMmalloc3(size_t l, size_t m, size_t n, size_t item_size)

{

  char ***three_d_pointers, ***tmp_3d_ptr;
  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  size_t i;

  /*
   * allocate the 3-d pointer array
   */

  three_d_pointers = (char ***) RMmalloc (l * sizeof(char **));

  /*
   * allocate the 2-d pointer array
   */

  two_d_pointers = (char **) RMmalloc (l * m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) RMmalloc (l * m * n * item_size);

  /*
   * load up 3-d pointer array to point to different parts of
   * the 2-d array
   */

  tmp_3d_ptr = three_d_pointers;
  tmp_2d_ptr = two_d_pointers;

  for (i = 0; i < l; i++) {
    *tmp_3d_ptr = tmp_2d_ptr;
    tmp_3d_ptr++;
    tmp_2d_ptr += m;
  }
  
  /*
   * load up the 2d pointer array to point to different parts of
   * the buffer
   */

  tmp_2d_ptr = two_d_pointers;
  tmp_buf = buffer;

  for (i = 0; i < l * m; i++) {
    *tmp_2d_ptr = tmp_buf;
    tmp_2d_ptr++;
    tmp_buf += (n * item_size);
  }

  /*
   * return 3d pointer array address
   */

  return ((void ***)three_d_pointers);

}

/*******************************************************************
 * RMcalloc()
 *
 * malloc of buffer initialized with zeros
 *
 ********************************************************************/

void *RMcalloc(size_t num, size_t size)
{

  void *addr;

  /*
   * return NULL if size is 0
   */
  
  if (num == 0 || size == 0) {
    return ((void *) NULL);
  }
  
  /*
   * allocate
   */
  
  if ((addr = calloc(num, size)) == NULL) {
    fprintf(stderr, "ERROR - RMcalloc\n");
    fprintf(stderr, "Cannot perform calloc, num = %d, size = %d\n",
	    (int) num, (int) size);
    RMmalloc_exit();
  }

  /*
   * return pointer
   */

  return(addr);

}

/*******************************************************************
 * RMcalloc2()
 *
 * calloc of two-dimensional array
 *
 * The data array is contiguously calloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

void **RMcalloc2(size_t m, size_t n, size_t item_size)

{

  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  size_t i;

  /*
   * allocate the pointer array
   */

  two_d_pointers = (char **) RMmalloc (m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) RMcalloc ((m * n), item_size);

  /*
   * load up the pointer array to point to different parts of
   * the buffer
   */

  tmp_2d_ptr = two_d_pointers;
  tmp_buf = buffer;

  for (i = 0; i < m; i++) {
    *tmp_2d_ptr = tmp_buf;
    tmp_2d_ptr++;
    tmp_buf += (n * item_size);
  }

  /*
   * return pointer array address
   */

  return ((void **) two_d_pointers);

}

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

void ***RMcalloc3(size_t l, size_t m, size_t n, size_t item_size)

{

  char ***three_d_pointers, ***tmp_3d_ptr;
  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  size_t i;

  /*
   * allocate the 3-d pointer array
   */

  three_d_pointers = (char ***) RMmalloc (l * sizeof(char **));

  /*
   * allocate the 2-d pointer array
   */

  two_d_pointers = (char **) RMmalloc (l * m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) RMcalloc ((l * m * n), item_size);

  /*
   * load up 3-d pointer array to point to different parts of
   * the 2-d array
   */

  tmp_3d_ptr = three_d_pointers;
  tmp_2d_ptr = two_d_pointers;

  for (i = 0; i < l; i++) {
    *tmp_3d_ptr = tmp_2d_ptr;
    tmp_3d_ptr++;
    tmp_2d_ptr += m;
  }
  
  /*
   * load up the 2d pointer array to point to different parts of
   * the buffer
   */

  tmp_2d_ptr = two_d_pointers;
  tmp_buf = buffer;

  for (i = 0; i < l * m; i++) {
    *tmp_2d_ptr = tmp_buf;
    tmp_2d_ptr++;
    tmp_buf += (n * item_size);
  }

  /*
   * return 3d pointer array address
   */

  return ((void ***) three_d_pointers);

}

/*******************************************************************
 * RMrealloc()
 *
 * reallocation
 *
 ********************************************************************/

void *RMrealloc(void *ptr, size_t size)

{

  void *addr;

  /*
   * return NULL if size is 0
   */

  if (size == 0) {
    return ((void *) NULL);
  }

  /*
   * allocate
   */
  
  if ((addr = realloc(ptr, size)) == NULL) {
    fprintf(stderr, "ERROR - RMrealloc\n");
    fprintf(stderr, "Cannot perform realloc, ptr = %p, size = %d\n",
	    ptr, (int) size);
    RMmalloc_exit();
  }

  /*
   * return pointer
   */

  return(addr);

}

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

void **RMrealloc2(void **user_addr, size_t m, size_t n, size_t item_size)
{
  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  size_t i;

  /*
   * re-allocate the pointer array
   */

  two_d_pointers = (char **)user_addr;
  
  two_d_pointers = (char **)RMrealloc(two_d_pointers,
				     m * sizeof(char *));

  /*
   * re-allocate the buffer
   */

  buffer = *two_d_pointers;
  
  buffer = (char *)RMrealloc(buffer,
			    m * n * item_size);

  /*
   * load up the pointer array to point to different parts of
   * the buffer
   */

  tmp_2d_ptr = two_d_pointers;
  tmp_buf = buffer;

  for (i = 0; i < m; i++)
  {
    *tmp_2d_ptr = tmp_buf;
    tmp_2d_ptr++;
    tmp_buf += (n * item_size);
  }

  /*
   * return pointer array address
   */

  return ((void **) two_d_pointers);

}

/*******************************************************************
 * RMfree()
 *
 * normal free
 *
 *******************************************************************/

void RMfree(void *user_addr)

{
  free(user_addr);
}

/************************************************************
 * RMfree_non_null()
 *
 * Frees pointer if non_null, sets pointer to null after free.
 *
 * The arg passed in is a pointer to the pointer to be freed.
 * 
 ************************************************************/

void RMfree_non_null(void **user_addr_p)

{

  if (*user_addr_p != NULL) {
    RMfree(*user_addr_p);
    *user_addr_p = NULL;
  }

  return;
  
}

/*******************************************************************
 * RMfree2()
 *
 * frees 2 dimensional arrau allocated with RMmalloc2 or RMcalloc2
 *
 *******************************************************************/

void RMfree2(void **two_d_pointers)

{

  /*
   * free up data buffer
   */

  RMfree((void *) *two_d_pointers);
  
  /*
   * free up 2d-pointer array
   */

  RMfree((void *) two_d_pointers);

}

/*******************************************************************
 * RMfree3()
 *
 * frees 3 dimensional arrau allocated with RMmalloc3 or RMcalloc3
 *
 *******************************************************************/

void RMfree3(void ***three_d_pointers)

{

  /*
   * free up data buffer
   */

  RMfree((void *) **three_d_pointers);
  
  /*
   * free up 2d-pointer array
   */

  RMfree((void *) *three_d_pointers);

  /*
   * free up 3d-pointer array
   */

  RMfree((void *) three_d_pointers);

}

/************************************************************************
 * RMmalloc_exit()
 *
 *************************************************************************/

static void RMmalloc_exit(void)

{

  fprintf(stderr, "Program will now exit for debugging.\n");

}

