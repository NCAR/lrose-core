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
 *	These functions correspond to the usual memory allocation
 *      and freeing functions, but exit on failure
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Feb 16, 1993: Created.
 */


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <euclid/alloc.h>

extern int kill(pid_t, int);
static void cause_segv(void);

void *EG_malloc(size_t size)
     
{

  void *addr;
  
  if ((addr = (void *) malloc(size)) == NULL) {
    if (size != 0) {
      fprintf(stderr, "ERROR - EG_malloc\n");
      fprintf(stderr, "Cannot perform malloc, size = %d\n", (int) size);
      fprintf(stderr, "Program will now crash for debugging.\n");
      cause_segv();
    }
  }
  
  return (addr);

}

void *EG_calloc (size_t nelem, size_t elsize)
     
{

  void *addr;
  
  if ((addr = (void *) calloc(nelem, elsize)) == NULL) {
    if (nelem * elsize != 0) {
      fprintf(stderr, "ERROR - EG_calloc\n");
      fprintf(stderr, "Cannot perform calloc, nelem, elsize = %d, %d\n",
	      (int) nelem, (int) elsize);
      fprintf(stderr, "Program will now crash for debugging.\n");
      cause_segv();
    }
  }
  
  return (addr);

}

void *EG_realloc (void *ptr, size_t size)

{

  void *addr;
  
  if (ptr == NULL) {
    addr = (void *) malloc(size);
  } else {
    addr = (void *) realloc(ptr, size);
  }

  if (addr == NULL) {

    if (size != 0) {
      fprintf(stderr, "ERROR - EG_realloc\n");
      fprintf(stderr, "Cannot perform realloc, size = %d\n", (int) size);
      fprintf(stderr, "Program will now crash for debugging.\n");
      cause_segv();
    }
    
  }
  
  return (addr);
  
}

void EG_free (void *ptr)

{

  free (ptr);

}


char **EG_st2_alloc(int rows, int cols, int size)
{
  int i;
  char **m;
  char *ptr;
  int sum = 0;

  /* allocate m to be an array of "rows" pointers */
  m = (char **) EG_malloc(rows * sizeof(char *));
  if (m == (char **) NULL)
    return((char **) NULL);

  /*
   * allocate an array containing enough space for rows * cols structures
   * of size "size"
   */
  ptr = (char *)EG_malloc(rows * cols * size * (sizeof(char)));
  if (ptr == (char *)NULL)
    return((char **) NULL);

  /* set the pointers in m to point to the beginning of each row */
  for(i = 0; i < rows; i++)
    {
      m[i] = &ptr[sum];
      sum += cols * size;
    }
  return((char **) m);
}

void EG_st2_free(char **m, int rows, int cols, int size)
{
  EG_free((char*) m[0]);
  EG_free((char*) (m ));
  return;
}

/* free memory that has been allocated */
void EG_free_mem(struct mem_ptr *ptr_array, int ptr_ct)
{
  int i;

  for (i=0; i<ptr_ct; i++)
    {
      EG_free(ptr_array[i].p);
    }
}

/************************************************************************
 * cause_segv()
 *
 * cause segmentation violation, thereby crashing program
 * this facilitates debugging at point of error
 *
 *************************************************************************/

static void cause_segv(void)

{

  kill(getpid(), SIGSEGV);

}
