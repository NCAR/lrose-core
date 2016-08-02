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

/****************************************************************************
 * tag_sub_trees.c
 *
 * Tag the subtrees, i.e. those vertices which are connected are
 * given the same tag number.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1995
 *
 ****************************************************************************/

#include "forecast_monitor.h"
#include "sys/types.h"
#include "sys/stat.h"

#define INIT_MAX_STACK 4096
#define STACK_INCR 4096

static int Top = 0;
static si32 *Index = NULL;
static tree_vertex_t **Stack = NULL;
static int Max_stack = INIT_MAX_STACK;

static int init_vertex_stack(void)

{
  if (Stack == NULL) {
    Stack =
      (tree_vertex_t **)umalloc(Max_stack*sizeof(*Stack));
    if (Stack == NULL)
      return(-1);
  }
  Top = 0;
  return (0);
}

#ifdef NOTNOW
static void free_vertex_stack(void)
{
  if (Stack != NULL)
    free(Stack);
  Stack = NULL;
}
#endif

static void clear_vertex_stack(void)
{
  Top = 0;
}

static int push_vertex(tree_vertex_t *v)
{
  void *ptr;
  if (Top < Max_stack - 1) {
    Stack[Top++] = v;
    return(0);
  } else {
    /* try to realloc */
    Max_stack += STACK_INCR;
    ptr = urealloc((char *) Stack, Max_stack * sizeof(*Stack));
    Stack = ptr;
    if (ptr == NULL) {
      printf("vertex stack overflow -- out of memory\n");
      clear_vertex_stack();
      return(-1);
    } else {
      Stack[Top++] = v;
      return(0);
    }
  }
}

static tree_vertex_t *pop_vertex(void)
{
  if (Top > 0) {
    return (Stack[--Top]);
  } else {
    clear_vertex_stack();
    return(NULL);
  }
}

static int vertex_stack_empty(void)
{
  if (Top == 0) {
    return (1);
  } else {
    return (0);
  }
}

/*
 * alloc array for vertices
 */

tree_vertex_t *alloc_vertices(si32 n_vertices)

{

  static int n_alloc = 0;
  static tree_vertex_t *vertices;
  
  if (n_vertices > n_alloc) {

    if (vertices == NULL) {
      vertices = (tree_vertex_t *)
	umalloc ((ui32) (n_vertices * sizeof(*vertices)));
    } else {
      vertices = (tree_vertex_t *)
	urealloc ((char *) vertices,
		  (ui32) (n_vertices * sizeof(*vertices)));
    }

    n_alloc = n_vertices;

  }

  return (vertices);

}

/*
 * allocate index array for vertices
 */

void alloc_vertex_index(si32 max_index)

{
  static si32 n_alloc = 0;
  
  if (max_index > n_alloc) {
    if (Index == NULL) {
      Index = (si32 *) umalloc
	((ui32) (max_index * sizeof(*Index)));
    } else {
      Index = (si32 *) urealloc
	((char *) Index,
	 (ui32) (max_index * sizeof(*Index)));
    }
  }

  return;

}

/*
 * load index
 */

void load_vertex_index(si32 vertex_num, si32 loc)

{
  Index[vertex_num] = loc;
}

static si32 vertex_index(si32 vertex_num)

{
  return (Index[vertex_num]);
}

static void visit(int k,
		  tree_vertex_t *vertices,
		  si32 tag)

{

  int i;
  tree_vertex_t *v;

  push_vertex(vertices + k);

  while (!vertex_stack_empty()) {
    v = pop_vertex();
    if (!v->visited) {
      v->tag = tag;
      v->visited = 1;
      for (i = 0; i < v->nadjacent; i++) {
	push_vertex(vertices + vertex_index(v->adjacent[i]));
      }
    } /* i */
  } /* while */

}

int tag_sub_trees(tree_vertex_t *vertices,
		  si32 n_vertices)

{

  int k;
  si32 tag;
  tree_vertex_t *vertex;
  
  /*
   * initialize the stack
   */
  
  if (init_vertex_stack()) {
    fprintf(stderr, "ERROR: tag_sub_trees\n");
    fprintf(stderr, "Cannot malloc for stack\n");
    return (-1);
  }

  /*
   * set visited flags off
   */
  
  vertex = vertices;
  for (k = 0; k < n_vertices; k++, vertex++) {
    vertex->visited = 0;
  } /* k */

  /*
   * initialize Tag
   */

  tag = 0;

  vertex = vertices;
  for (k = 0; k < n_vertices; k++, vertex++) {
    if (vertex->visited == 0) {
      visit(k, vertices, tag);
      tag++;
    }
  } /* k */

  return (tag);

}








