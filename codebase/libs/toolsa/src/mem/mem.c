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
   Module MEM, handles memory allocation.
   Handles our special form for structures, and handles arrays.

   KEYWORD: malloc cover, memory allocation

   Creation:      2/15/90 JCaron
		  8/17/92 : ansi only version

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
 
#include <toolsa/globals.h>
#include <toolsa/err.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>

static void alloc_segv(void);

void *MEM_alloc(size_t size)
     
{

  void *addr;
  
  if ((addr = (void *) malloc(size)) == NULL) {
    if (size != 0) {
      fprintf(stderr, "ERROR - MEM_alloc\n");
      fprintf(stderr, "Cannot perform malloc, size = %d\n", (int) size);
      alloc_segv();
    }
  }
  
  return (addr);

}


void *MEM_calloc (size_t nelem, size_t elsize)
     
{

  void *addr;
  
  if ((addr = (void *) calloc(nelem, elsize)) == NULL) {
    if (nelem * elsize != 0) {
      fprintf(stderr, "ERROR - MEM_calloc\n");
      fprintf(stderr, "Cannot perform calloc, nelem, elsize = %d, %d\n",
	      (int) nelem, (int) elsize);
      alloc_segv();
    }
  }
  
  return (addr);

}

void *MEM_realloc (void *ptr, size_t size)

{

  void *addr;
  
  if ((addr = (void *) realloc(ptr, size)) == NULL) {
    if (size != 0) {
      fprintf(stderr, "ERROR - MEM_realloc\n");
      fprintf(stderr, "Cannot perform realloc, size = %d\n", (int) size);
      alloc_segv();
    }
  }
  
  return (addr);

}

void MEM_free (void *ptr)

{

  free (ptr);

}

/************************************************************************
 * alloc_segv()
 *
 * cause segmentation violation, thereby crashing program
 *
 *************************************************************************/

#include <signal.h>
extern int kill(pid_t, int);

static void alloc_segv(void)

{

  fprintf(stderr, "Program will now segv for debugging.\n");
  kill(getpid(), SIGSEGV);

}

