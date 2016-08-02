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
/* stack_interval.c - functions to manipulate a stack for seed.c */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <rdi/clump.h>

extern char *prog_name;

static int top_2d = 0;
static int *stack_2d = NULL;
static int max_stack = MAX_STACK;

int init_stack()
{
  if (stack_2d == NULL)
    {
      stack_2d = (int *)malloc_new(MAX_STACK*sizeof(int));
      if (stack_2d == NULL)
	return(-1);
    }
  top_2d = 0;
  
  return(0);
}

void free_stack()
{
  if (stack_2d != NULL)
    free_new(stack_2d, max_stack*sizeof(int));
  stack_2d = NULL;
}

void clear_stack_2d()
{
  top_2d = 0;
}

int push_2d(x, y)
     int x;
     int y;
{
  void *ptr;

/*  printf("pushing %d, %d, top_2d is %d\n", x, y, top_2d);  */
  if (top_2d < max_stack - 2)
    {
      stack_2d[top_2d++] = x;
      stack_2d[top_2d++] = y;
/*      printf("stack_2d[%d] = %d, stack_2d[%d] = %d\n", top_2d-1, stack_2d[top_2d-1], top_2d-2, stack_2d[top_2d-2]); */
      return(1);
    }
  else
    {
      /* try to realloc */
      max_stack += STACK_INCR;
      ptr = realloc_new(stack_2d, max_stack, STACK_INCR);
      stack_2d = ptr;
      if (ptr == NULL)
	{
	  printf("stack overflow -- out of memory\n");
	  clear_stack_2d();
	  return(0);
	}
      else
	{
	  stack_2d[top_2d++] = x;
	  stack_2d[top_2d++] = y;
	  return(1);
	}
    }
}

int pop_2d(x, y)
     int *x;
     int *y;
{
  if (top_2d > 1)
    {
      *y = stack_2d[--top_2d];
      *x = stack_2d[--top_2d];
      return(1);
    }
  else
    {
      clear_stack_2d();
      return(0);
    }
}


