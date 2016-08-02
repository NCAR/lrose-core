// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**********************************************************************
 * umalloc.cc
 *
 * Dynamic memory allocation routines for 2D and 3D arrays
 *
 **********************************************************************/

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "umalloc.hh"

/*******************************************************************
 * umalloc()
 *
 * normal malloc
 *
 *******************************************************************/

void *Umalloc::umalloc(size_t size)

{

  void *addr;

  /*
   * return NULL if size is 0
   */
  
  if (size == 0) {
    return NULL;
  }

  /*
   * allocate
   */

  if ((addr = (char *) malloc(size)) == NULL) {
    fprintf(stderr, "ERROR - umalloc\n");
    fprintf(stderr, "Cannot perform malloc, size = %d\n", (int) size);
    return NULL;
  }

  return addr;

}

/*******************************************************************
 * umalloc2()
 *
 * malloc of two-dimensional array
 *
 * The data array is contiguously malloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

void **Umalloc::umalloc2(size_t m, size_t n, size_t item_size)

{

  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  int i;

  /*
   * allocate the pointer array
   */

  two_d_pointers = (char **) umalloc (m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) umalloc (m * n * item_size);

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

void ***Umalloc::umalloc3(size_t l, size_t m, size_t n, size_t item_size)

{

  char ***three_d_pointers, ***tmp_3d_ptr;
  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  int i;

  /*
   * allocate the 3-d pointer array
   */

  three_d_pointers = (char ***) umalloc (l * sizeof(char **));

  /*
   * allocate the 2-d pointer array
   */

  two_d_pointers = (char **) umalloc (l * m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) umalloc (l * m * n * item_size);

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
 * ucalloc()
 *
 * malloc of buffer initialized with zeros
 *
 ********************************************************************/

void *Umalloc::ucalloc(size_t num, size_t size)
{

  unsigned int csize;
  void *addr;

  /*
   * compute size in bytes
   */

  csize = num * size;

  /*
   * normal malloc
   */

  addr = umalloc(csize);

  /*
   * initialize area with zeros
   */

  memset ((void *) addr, 0, csize);

  return((void *) addr);

}

/*******************************************************************
 * ucalloc2()
 *
 * calloc of two-dimensional array
 *
 * The data array is contiguously calloc'd in a buffer, and the
 * two_d_pointer array points to different parts of the data array.
 *
 *******************************************************************/

void **Umalloc::ucalloc2(size_t m, size_t n, size_t item_size)

{

  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  int i;

  /*
   * allocate the pointer array
   */

  two_d_pointers = (char **) umalloc (m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) ucalloc ((m * n), item_size);

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

void ***Umalloc::ucalloc3(size_t l, size_t m, size_t n, size_t item_size)

{

  char ***three_d_pointers, ***tmp_3d_ptr;
  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  int i;

  /*
   * allocate the 3-d pointer array
   */

  three_d_pointers = (char ***) umalloc (l * sizeof(char **));

  /*
   * allocate the 2-d pointer array
   */

  two_d_pointers = (char **) umalloc (l * m * sizeof(char *));

  /*
   * allocate the buffer
   */

  buffer = (char *) ucalloc ((l * m * n), item_size);

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
 * urealloc()
 *
 * reallocation
 *
 ********************************************************************/

void *Umalloc::urealloc(void *user_addr, size_t size)

{

  void *new_addr;

  /*
   * return NULL if size is 0
   */
  
  if (size == 0) {
      return NULL;
  }

  if (user_addr == NULL) {

    /*
     * use malloc instead
     */

    void *addr = umalloc(size);
    return addr;
    
  }
  
  /*
   * reallocate
   */

  if ((new_addr = (char *) realloc(user_addr, size)) == NULL) {
    fprintf(stderr, "ERROR - urealloc\n");
    fprintf(stderr, 
	    "Cannot perform realloc, addr, size = %p, %d\n",
	    user_addr, (int) size);
    return NULL;
  }

  return new_addr;

}

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

void **Umalloc::urealloc2(void **user_addr, size_t m, size_t n, size_t item_size)
{
  char **two_d_pointers, **tmp_2d_ptr;
  char *buffer, *tmp_buf;
  int i;

  /*
   * re-allocate the pointer array
   */

  two_d_pointers = (char **)user_addr;
  
  two_d_pointers = (char **)urealloc(two_d_pointers,
				     m * sizeof(char *));

  /*
   * re-allocate the buffer
   */

  buffer = *two_d_pointers;
  
  buffer = (char *)urealloc(buffer,
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
 * ufree()
 *
 * normal free
 *
 *******************************************************************/

void Umalloc::ufree(void *addr)

{

  if (addr == NULL) {

    /*
     * nothing to free
     */
    
    return;
    
  } /* if (match_found == FALSE)  */

  free(addr);
  
}

/************************************************************
 * ufree_non_null()
 *
 * Frees pointer if non_null, sets pointer to null after free.
 *
 * The arg passed in is a pointer to the pointer to be freed.
 * 
 ************************************************************/

void Umalloc::ufree_non_null(void **user_addr_p)

{

  if (*user_addr_p != NULL) {
    ufree((void *) *user_addr_p);
    *user_addr_p = NULL;
  }

  return;
  
}

/*******************************************************************
 * ufree2()
 *
 * frees 2 dimensional arrau allocated with umalloc2 or ucalloc2
 *
 *******************************************************************/

void Umalloc::ufree2(void **two_d_pointers)

{

  /*
   * free up data buffer
   */

  ufree((void *) *two_d_pointers);
  
  /*
   * free up 2d-pointer array
   */

  ufree((void *) two_d_pointers);

}

/*******************************************************************
 * ufree3()
 *
 * frees 3 dimensional arrau allocated with umalloc3 or ucalloc3
 *
 *******************************************************************/

void Umalloc::ufree3(void ***three_d_pointers)

{

  /*
   * free up data buffer
   */

  ufree((void *) **three_d_pointers);
  
  /*
   * free up 2d-pointer array
   */

  ufree((void *) *three_d_pointers);

  /*
   * free up 3d-pointer array
   */

  ufree((void *) three_d_pointers);

}
