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
 * mem.c
 *
 * Dynamic memory allocation routines with error checking and
 * debugging facilities.
 *
 * For no debug, just use umalloc(), urealloc(), ucalloc() and
 * ufree() in place of malloc(), realloc(), calloc(), and free().
 *
 * umalloc_debug ((int) level) : sets debug level - only to be
 *                                 invoked once per program, normally
 *                                 close to the beginning
 *
 * umalloc_verify () :           check integrity of each block
 *
 * umalloc_map () :              print out details of each block
 *
 * For debugging:
 *
 * At start of prog, call umalloc_debug((int) level), where
 *
 *    level 0 gives no debug
 *
 *    level 1 gives corruption checking by inspecting the ID at
 *            start and end of malloc'd block
 * 
 *    level 2 adds facility for recording all malloc'd blocks and
 *            verifying integrity of all blocks 
 *
 *    level 3 adds prints to stderr
 *
 * If level exceeds 0, then 24 extra bytes are added to the malloc'd
 * block.
 *
 * Bytes 0 - 3 : magic cookie 0x0ff10ff2
 * Bytes 4 - 7 : ID (increments with each malloc, calloc or realloc)
 * Bytes 8 - 11: size (including extra 12 bytes)
 * Bytes 12 - 15: 0's padding so that doubles are correctly aligned
 * Bytes 7 to 4 from end: 0's padding
 * Bytes 3 to 0 from end: ID - should be same as first 4 bytes.
 *
 * For level 1+, memory is allocated in full word sizes only,
 * to avoid alignment problems for the ID at the end of the block.
 * The number of bytes per word is defined by NBYTES_WORD in this file.
 *
 * For level 2+, umalloc_verify() will check integrity of all blocks.
 * Performed automatically by umalloc and urealloc for levels 2+.
 *
 * For level 3+, umalloc_map() prints malloc'd table to stderr.
 *
 * On errors, the routines will cause a segmentation violation, so 
 * that the debugger may be used to trace the call stack.
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * September 1991
 *
 * Copied over to toolsa, July 1996
 *
 **********************************************************************/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h> 
#include <signal.h>
#include <math.h>
#include <sys/types.h>

#include <dataport/port_types.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>

#define N_ALLOCATE 100
#define NBYTES_WORD 8
#define UMALLOC_CODE 0x0ff10ff2
#define START_ID 987654321

static void alloc_check_space(void);
static void alloc_check_block(char *addr, int64_t *ret_start_id,
			      int64_t *ret_end_id, u_int64_t *ret_size);
static void umalloc_segv(void);

static char **Malloc_addr;

static int64_t Narray = 0;
static int64_t Nmalloc = 0;
static int64_t Id = START_ID;

static int Debug_level = 0;
static int Nshift = 4;
static int Moffset = 0;
static int Nbytes_extra = 0;


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

void umalloc_debug(int level)
{

  Debug_level = level;

  if (level > 0) {
    Nbytes_extra = 6 * sizeof(int64_t);
    Moffset = 4 * sizeof(int64_t);
  }

}

/*******************************************************************
 * umalloc()
 *
 * normal malloc
 *
 *******************************************************************/

void *(umalloc)(size_t size)

{

  int slot_found;
  int64_t imalloc;
  int64_t *laddr;
  u_int64_t msize;
  char *addr, *user_addr;

  /*
   * return NULL if size is 0
   */

  if (size == 0) {
    return ((void *) NULL);
  }

  if (Debug_level > 1) {
    umalloc_verify();
  }

  /*
   * compute size to keep aligned with word boundaries
   */

  if (Debug_level > 0) {
    msize = ((((size + NBYTES_WORD * 2 - 1) >> Nshift) << Nshift) +
	     Nbytes_extra);
  } else {
    msize = size;
  }

  /*
   * allocate
   */

  if ((addr = (char *) malloc(msize)) == NULL) {
    fprintf(stderr, "ERROR - umalloc\n");
    fprintf(stderr, "Cannot perform malloc, size = %d\n", (int) size);
    umalloc_segv();
    return ((void*)NULL);
  }

  user_addr = addr + Moffset;

  /*
   * if debug level is 1+, store the code, id and size at the
   * start, and the id at the end
   */

  if (Debug_level > 0) {
    laddr = (int64_t *) addr;
    *laddr = UMALLOC_CODE;
    laddr++;
    *laddr = Id;
    laddr++;
    *laddr = (int64_t) msize;
    laddr++;
    *laddr = 0L;
    *((int64_t *) (addr + msize - 2 * sizeof(int64_t))) = 0L;
    *((int64_t *) (addr + msize - sizeof(int64_t))) = Id;
    Id++;
  }

  /*
   * if debug level is 2+, store the allocation info
   * in the Malloc_addr array
   */

  if (Debug_level > 1) {

    slot_found = FALSE;
    
    for (imalloc = 0; imalloc < Nmalloc; imalloc++) {
      if (Malloc_addr[imalloc] == (char *) NULL) {
	Malloc_addr[imalloc] = addr;
	slot_found = TRUE;
	break;
      }
    } /* imalloc */

    if (slot_found == FALSE) {

      alloc_check_space();
      Malloc_addr[Nmalloc] = addr;
      Nmalloc++;

    } /* if (slot_found == FALSE) */

  } /* if (Debug_level > 1) */

  if (Debug_level > 2) {

    fprintf(stderr,
	    "%s %10p(%10p), %10d, %10d(%10d)\n",
	    "umalloc   : addr(user), id, size(user) =",
	    addr, user_addr,
	    (int) (Id - 1), (int) msize,(int) (msize - Nbytes_extra));

  } /* if (Debug_level > 2) */

  /*
   * return offset pointer
   */

  return((void *) user_addr);

}

/*******************************************************************
 * umalloc_min_1()
 *
 * umalloc, but if size == 0, at least 1 byte is allocated
 * so that the returned pointer is not null
 *
 *******************************************************************/

void *(umalloc_min_1)(size_t size)

{
  if (size < 1) {
    size = 1;
  }
  return umalloc(size);
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

void **umalloc2(size_t m, size_t n, size_t item_size)

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

void ***umalloc3(size_t l, size_t m, size_t n, size_t item_size)

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

void *(ucalloc)(size_t num, size_t size)
{

  u_int64_t csize;
  char *user_addr;

  /*
   * compute size in bytes
   */

  csize = num * size;

  /*
   * normal malloc
   */

  if (Debug_level > 2) {
    fprintf(stderr, "ucalloc invoking umalloc\n");
  }

  user_addr = umalloc(csize);

  /*
   * initialize area with zeros
   */

  memset ((void *) user_addr, 0, csize);

  return((void *) user_addr);

}

/*******************************************************************
 * ucalloc_min_1()
 *
 * ucalloc, but if num == 0 or size == 0, at least 1 byte
 * is allocated so that the returned pointer is not null
 *
 *******************************************************************/

void *(ucalloc_min_1)(size_t num, size_t size)
{
  if (num == 0 || size == 0) {
    num = 1;
    size = 1;
  }
  return ucalloc(num, size);
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

void **ucalloc2(size_t m, size_t n, size_t item_size)

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

void ***ucalloc3(size_t l, size_t m, size_t n, size_t item_size)

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

void *(urealloc)(void *user_addr, size_t size)

{

  char *addr;
  char *new_addr;
  int64_t *laddr;
  int64_t imalloc = 0;
  u_int64_t msize;
  int match_found;

  /*
   * return NULL if size is 0
   */

  if (size == 0) {
      return ((char *) NULL);
  }

  if (Debug_level > 1) {
    umalloc_verify();
  }

  if (user_addr == NULL) {

    /*
     * use malloc instead
     */

    addr = umalloc(size);
    return ((void *) addr);
    
  } /* if (match_found == FALSE)  */
  
  addr = (char *) user_addr - Moffset;

  /*
   * if debug level is 2+,
   * verify that the malloc'd blocks are OK and that
   * check that the address is currently active
   */

  if (Debug_level > 1) {

    match_found = FALSE;

    for (imalloc = 0; imalloc < Nmalloc; imalloc++) {
	  
      if (addr == Malloc_addr[imalloc]) {
	match_found = TRUE;
	break;
      } /* if (addr == Malloc_addr[imalloc]) */

    } /* imalloc */
	
    if (match_found == FALSE) {
	
      fprintf(stderr, "ERROR - urealloc\n");
      fprintf(stderr, "Trying to realloc block at addr %p.\n", addr);
      fprintf(stderr, "This block not currently allocated.\n");
      umalloc_segv();
      return((void*)NULL);
	
    } /* if (match_found == FALSE)  */

  } /* if (Debug_level > 1) */
      
  /*
   * compute size to preserve word boundary alignment
   */

  if (Debug_level > 0) {
    msize = ((((size + NBYTES_WORD * 2 - 1) >> Nshift) << Nshift) +
	     Nbytes_extra);
  } else {
    msize = size;
  }

  /*
   * reallocate
   */

  if ((new_addr = (char *) realloc(addr, msize)) == NULL) {
    fprintf(stderr, "ERROR - urealloc\n");
    fprintf(stderr, 
	    "Cannot perform realloc, addr, size = %p, %d\n",
	    addr, (int) size);
    umalloc_segv();
    return((void*)NULL);
  }

  user_addr = (void *) (new_addr + Moffset);

  /*
   * if debug level is 1+, store the code, id and size at the
   * start, and the id at the end
   */
  
  if (Debug_level > 0) {
    laddr = (int64_t *) new_addr;
    *laddr = UMALLOC_CODE;
    laddr++;
    *laddr = Id;
    laddr++;
    *laddr = (int64_t) msize;
    laddr++;
    *laddr = 0L;
    *((int64_t *) (new_addr + msize - 2 * sizeof(int64_t))) = 0L;
    *((int64_t *) (new_addr + msize - sizeof(int64_t))) = Id;
    Id++;
  }

  /*
   * if debug level is 2+, and pointer has changed,
   * store the allocation info in the malloc_addr array
   */

  if (Debug_level > 1 && new_addr != addr) {

    Malloc_addr[imalloc] = new_addr;

    if (Debug_level > 2) {
      fprintf(stderr,
	      "urealloc: changing addr, old, new    = %7p, %7p\n",
	      addr, new_addr);
    } /* if (Debug_level > 2) */

  } /* if (Debug_level > 1 && new_addr != addr) */
  
  if (Debug_level > 2) {

    fprintf(stderr,
	    "%s %10p(%10p), %10d, %10d(%10d)\n",
	    "urealloc : addr(user), id, size(user) =",
	    new_addr, user_addr,
	    (int) (Id - 1), (int) msize, (int) msize - Nbytes_extra);

  } /* if (Debug_level > 2) */

  /*
   * return offset pointer
   */

  return((void *) user_addr);

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

void **urealloc2(void **user_addr, size_t m, size_t n, size_t item_size)
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

void (ufree)(void *user_addr)

{

  char *addr;
  int64_t match_found;
  int64_t start_id;
  int64_t end_id;
  u_int64_t msize;
  int64_t imalloc;

  if (user_addr == NULL) {

    /*
     * nothing to free
     */

    return;
    
  } /* if (match_found == FALSE)  */

  /*
   * compute pointer to start of actual block
   */
  
  addr = (char *) user_addr - Moffset;
  
  /*
   * if not allocated under umalloc, free and return
   */

  if ((Debug_level == 0) ||
      (*(int64_t *) addr != UMALLOC_CODE)) {
    free ((char *) user_addr);
    return;
  }

  /*
   * if debug level is 1+, check block integrity
   */
  
  if (Debug_level > 0) {
    alloc_check_block (addr, &start_id, &end_id, &msize);
  }
  
  /*
   * if debug level is 2+, set entry in address array to NULL
   */
  
  if (Debug_level > 1) {
    
    match_found = FALSE;
    
    for (imalloc = 0; imalloc < Nmalloc; imalloc++) {
      
      if (addr == Malloc_addr[imalloc]) {
	
	if (Debug_level > 2) {
	  
	  fprintf(stderr,
		  "%s %10p(%10p), %10d, %10d(%10d)\n",
		  "ufree    : addr(user), id, size(user) =",
		  addr, addr + Moffset,
		  (int) start_id,
		  (int) msize, (int) msize - Nbytes_extra);
	  
	} /* if (Debug_level > 2) */
	
	Malloc_addr[imalloc] = (char *) NULL;
	
	match_found = TRUE;
	
	break;
	
      } /* if (addr == Malloc_addr[imalloc]) */
      
    } /* imalloc */
    
    if (match_found == FALSE) {
      
      fprintf(stderr, "ERROR - ufree\n");
      fprintf(stderr, "Trying to free block at addr %p.\n", addr);
      fprintf(stderr, "This block not currently allocated.\n");
      umalloc_segv();
      return;
    } /* if (match_found == FALSE)  */
    
  } /* if (Debug_level > 1) */
  
  free((char *) addr);
  
}

/************************************************************
 * ufree_non_null()
 *
 * Frees pointer if non_null, sets pointer to null after free.
 *
 * The arg passed in is a pointer to the pointer to be freed.
 * 
 ************************************************************/

void ufree_non_null(void **user_addr_p)

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

void ufree2(void **two_d_pointers)

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

void ufree3(void ***three_d_pointers)

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

/*******************************************************************
 * umalloc_map()
 *
 * print out table of malloc'd blocks
 *
 *******************************************************************/

void umalloc_map(void)

{

  int64_t mtotal = 0;
  int64_t imalloc;
  int64_t start_id;
  int64_t end_id;
  u_int64_t msize;

  if (Debug_level > 2) {

    fprintf(stderr, "\n");
    fprintf(stderr, "umalloc_map - memory allocation details.\n");
    fprintf(stderr, "----------------------------------------\n");

    for (imalloc = 0; imalloc < Nmalloc; imalloc++) {

      if (Malloc_addr[imalloc] != NULL) {

	alloc_check_block (Malloc_addr[imalloc],
			   &start_id, &end_id, &msize);

	fprintf(stderr,
		"%s %10p(%10p), %10d, %10d(%10d)\n",
		"addr(user), id, size(user) =",
		Malloc_addr[imalloc],
		Malloc_addr[imalloc] + Moffset,
		(int) start_id,
		(int) msize, (int) (msize - Nbytes_extra));

	mtotal += msize - Nbytes_extra;

      } /* if (Malloc_addr[imalloc != NULL) */

    } /* imalloc */

    fprintf(stderr, "\nTotal malloc'd = %d\n\n", (int) mtotal);

  } /* if (debug_level > 1) */

}

/*******************************************************************
 * umalloc_count()
 *
 * count size of malloc'd blocks
 *
 *******************************************************************/

int umalloc_count(void)

{

  int64_t mtotal = 0;
  int64_t imalloc;
  int64_t start_id;
  int64_t end_id;
  u_int64_t msize;

  if (Debug_level > 1) {

    for (imalloc = 0; imalloc < Nmalloc; imalloc++) {

      if (Malloc_addr[imalloc] != NULL) {

	alloc_check_block (Malloc_addr[imalloc],
			   &start_id, &end_id, &msize);

	mtotal += msize - Nbytes_extra;

      } /* if (Malloc_addr[imalloc != NULL) */

    } /* imalloc */

    fprintf(stderr, "\nTotal malloc'd = %d\n\n", (int) mtotal);

  } /* if (debug_level ... */

  return ((int) mtotal);

}

/*******************************************************************
 * umalloc_verify()
 *
 * verify malloc'd entries
 *
 *******************************************************************/

void umalloc_verify(void)
{

  int64_t imalloc;
  int64_t start_id;
  int64_t end_id;
  u_int64_t msize;

  if (Debug_level > 1) {

    for (imalloc = 0; imalloc < Nmalloc; imalloc++) {

      alloc_check_block (Malloc_addr[imalloc],
			 &start_id, &end_id, &msize);

    } /* imalloc */

  } /* if (debug_level > 1) */

}

/*******************************************************************
 * alloc_check_block()
 *
 * check that the id at start and end of block is the same - 
 * if not, block has been corrupted.
 *
 ********************************************************************/

static void alloc_check_block(char *addr, int64_t *ret_start_id,
			      int64_t *ret_end_id, u_int64_t *ret_size)

{

  int64_t *start_id;
  int64_t *end_id;
  int64_t *msize;

  if (addr != NULL) {

    start_id = (int64_t *) addr + 1;
    msize = start_id + 1;
    end_id = start_id + *msize / sizeof(int64_t) - 2;

    if (*start_id <= 0 || *end_id <= 0 ||
	*msize < 0 || *start_id != *end_id) {

	  
      fprintf(stderr, "ERROR - alloc_check_block\n");
      fprintf(stderr, "Malloc block corrupted\n");
      fprintf(stderr,
	      "%s %10p(%10p), %10d, %10d(%10d)\n",
	      "            : addr(user), id, size(user) =",
	      addr, addr + Moffset,
	      (int) *start_id,
	      (int) *msize, (int) (*msize - Nbytes_extra));
      umalloc_segv();
      return;
    } /* if (*start_id != *end_id) */

    *ret_start_id = *start_id;
    *ret_end_id = *end_id;
    *ret_size = *msize;

   } /* if (addr != NULL) */

}

/************************************************************************
 * alloc_check_space()
 *
 * checks that there is enough memory for another entry, and
 * adds to the allocation if necessary
 *
 ************************************************************************/

static void alloc_check_space(void)

{

  char **m_addr;
  int64_t n_array;

  if (Nmalloc + 1 > Narray) {

    /*
     * allocate another block of space
     */

    n_array = Narray + N_ALLOCATE;

    if ((m_addr = (char **) malloc
	 ((u_int64_t) n_array * sizeof(char *))) == NULL) {
      fprintf(stderr, "ERROR - umalloc_alloc_addr_array\n");
      fprintf(stderr, "Cannot perform malloc for debug table\n");
      umalloc_segv();
      return;
    }

    memset ((void *)  m_addr,
            (int) 0, (size_t)  (n_array * sizeof(char *)));

    if (Narray > 0) {

      memcpy ((void *)  m_addr,
              (void *)  Malloc_addr,
              (size_t)  (Nmalloc * sizeof(char *)));

      free((char *) Malloc_addr);

    } /* if (Narray > 0) */

    Malloc_addr = m_addr;

    Narray = n_array;

  } /* if (Nmalloc + 1 > Narray) */

}

/************************************************************************
 * umalloc_segv()
 *
 * cause segmentation violation, thereby crashing program
 *
 *************************************************************************/

extern int kill(pid_t, int);

static void umalloc_segv(void)

{

  fprintf(stderr, "Out of memory - exiting.\n");
  /* kill(getpid(), SIGSEGV); */

}


