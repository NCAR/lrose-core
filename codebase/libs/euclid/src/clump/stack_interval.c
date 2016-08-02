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
#include <euclid/clump.h>
#include <euclid/alloc.h>

#define INIT_MAX_STACK 4096
#define STACK_INCR 4096

static int top_2d = 0;
static int *stack_2d = NULL;
static int top_3d = 0;
static int *stack_3d = NULL;
static int max_stack = INIT_MAX_STACK;

int
EG_init_stack_2d()
{
  if (stack_2d == NULL)
    {
      stack_2d = (int *)EG_malloc(max_stack*sizeof(int));
      if (stack_2d == NULL)
	return(-1);
    }
  top_2d = 0;
  
  return(0);
}

void
EG_free_stack_2d()
{
  if (stack_2d != NULL)
    EG_free(stack_2d);
  stack_2d = NULL;
}

void EG_clear_stack_2d()
{
  top_2d = 0;
}

int EG_push_2d(int x, int y)
{
  void *ptr;

  if (top_2d < max_stack - 1)
    {
      stack_2d[top_2d++] = x;
      stack_2d[top_2d++] = y;
      return(1);
    }
  else
    {
      /* try to realloc */
      max_stack += STACK_INCR;
      ptr = EG_realloc(stack_2d, max_stack * sizeof(int));
      stack_2d = ptr;
      if (ptr == NULL)
	{
	  printf("stack overflow -- out of memory\n");
	  EG_clear_stack_2d();
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

int EG_pop_2d(int *x, int *y)
{
  if (top_2d > 1)
    {
      *y = stack_2d[--top_2d];
      *x = stack_2d[--top_2d];
      return(1);
    }
  else
    {
      EG_clear_stack_2d();
      return(0);
    }
}

int
EG_init_stack_3d()
{
  if (stack_3d == NULL)
    {
      stack_3d = (int *)EG_malloc(max_stack*sizeof(int));
      if (stack_3d == NULL)
	return(-1);
    }
  top_3d = 0;
  return(0);
}

void
EG_free_stack_3d()
{
  if (stack_3d != NULL)
    EG_free(stack_3d);

  stack_3d = NULL;
}

void EG_clear_stack_3d()
{
  top_3d = 0;
}

int EG_push_3d(int x, int y, int z)
{
  void *ptr;

  if (top_3d < max_stack - 2)
    {
      stack_3d[top_3d++] = x;
      stack_3d[top_3d++] = y;
      stack_3d[top_3d++] = z;
      return(1);
    }
  else
    {
      /* try to realloc */
      max_stack += STACK_INCR;
      ptr = EG_realloc(stack_3d, max_stack * sizeof(int));
      stack_3d = ptr;
      if (ptr == NULL)
	{
	  printf("stack overflow -- out of memory\n");
	  EG_clear_stack_3d();
	  return(0);
	}
      else
	{
	  stack_3d[top_3d++] = x;
	  stack_3d[top_3d++] = y;
 	  stack_3d[top_3d++] = z;
	  return(1);
	}
    }
}

int EG_pop_3d(int *x, int *y, int *z)
{
  if (top_3d > 2)
    {
      *z = stack_3d[--top_3d];
      *y = stack_3d[--top_3d];
      *x = stack_3d[--top_3d];
      return(1);
    }
  else
    {
      EG_clear_stack_3d();
      return(0);
    }
}
