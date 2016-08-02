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
 * bd_tree.c
 *
 * Bi-directional Tree module
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1995
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <rapmath/bd_tree.h>
#include <rapmath/RMmalloc.h>

#define TREE_STACK_INCR 4096

/*
 * file scope protptypes
 */

static void alloc_index_array(bd_tree_handle_t *handle,
			      int max_index);

static void clear_vertex_stack(bd_tree_handle_t *handle);

static bd_tree_vertex_t *pop_vertex(bd_tree_handle_t *handle);

static void push_vertex(bd_tree_handle_t *handle,
			bd_tree_vertex_t *v);

static int vertex_stack_empty(bd_tree_handle_t *handle);

static void visit(bd_tree_handle_t *handle, int k, int tag);

/***********************
 * initialize the handle
 */

void BD_TREE_init_handle(bd_tree_handle_t *handle)

{
  handle->top = 0;

  handle->index = NULL;
  handle->n_index_alloc = 0;

  handle->vertices = NULL;
  handle->n_vert_alloc = 0;
  handle->n_vertices = 0;

  handle->stack = NULL;
  handle->n_stack_alloc = TREE_STACK_INCR;

  handle->stack = (bd_tree_vertex_t **)
    RMmalloc(handle->n_stack_alloc *sizeof(*handle->stack));
}

/*****************
 * free the handle
 */

void BD_TREE_free_handle(bd_tree_handle_t *handle)

{

  int i;
  bd_tree_vertex_t *v;

  if (handle->stack != NULL)
    RMfree(handle->stack);
  handle->stack = NULL;

  if (handle->index != NULL)
    RMfree(handle->index);
  handle->index = NULL;

  v = handle->vertices;
  
  for (i = 0; i < handle->n_vert_alloc; i++, v++) {
    if (v->adjacent != NULL) {
      RMfree(v->adjacent);
    }
  }

  if (handle->vertices != NULL) {
    RMfree(handle->vertices);
    handle->vertices = NULL;
  }

}

/**************************
 * alloc array for vertices
 */

void BD_TREE_alloc_vertices(bd_tree_handle_t *handle,
			    int max_vertices,
			    int max_adjacent)
     
{
  
  int i;
  bd_tree_vertex_t *v;

  if (max_vertices > handle->n_vert_alloc) {

    if (handle->vertices == NULL) {
      handle->vertices = (bd_tree_vertex_t *)
	RMmalloc (max_vertices * sizeof(*handle->vertices));
      memset(handle->vertices, 0,
	     max_vertices * sizeof(*handle->vertices));
    } else {
      handle->vertices = (bd_tree_vertex_t *)
	RMrealloc (handle->vertices,
		    max_vertices * sizeof(*handle->vertices));
      memset(handle->vertices + handle->n_vert_alloc, 0,
	     (max_vertices - handle->n_vert_alloc) *
	     sizeof(*handle->vertices));
    }
    
    handle->n_vert_alloc = max_vertices;

    v = handle->vertices;

    for (i = 0; i < handle->n_vert_alloc; i++, v++) {
      if (v->n_adj_alloc < max_adjacent) {
	if (v->adjacent == NULL) {
	  v->adjacent =
	    (int *) RMmalloc(max_adjacent * sizeof(int));
	} else {
	  v->adjacent =
	    (int *) RMrealloc(v->adjacent,
			       max_adjacent * sizeof(int));
	}
	v->n_adj_alloc = max_adjacent;
      }
    } /* i */
    
  }
  
}

/******************************
 * BD_TREE_start()
 *
 * Start the tree - initialize.
 */

void BD_TREE_start(bd_tree_handle_t *handle)

{
  int i;
  int *ip;
  bd_tree_vertex_t *v;

  v = handle->vertices;
  for (i = 0; i < handle->n_vert_alloc; i++, v++) {
    v->nadjacent = 0;
  }

  ip = handle->index;
  
  for (i = 0; i < handle->n_index_alloc; i++, ip++) {
    *ip = -1;
  }
    
  handle->n_vertices = 0;

}

/*******************
 * add adjacent node
 *
 * Applies to current vertex.
 */

#include <stdio.h>

void BD_TREE_add_adjacent(bd_tree_handle_t *handle,
			  int id)

{

  int vnum, anum;
  bd_tree_vertex_t *v;

  alloc_index_array(handle, id);

  vnum = handle->n_vertices - 1;
  v = handle->vertices + vnum;
  anum = v->nadjacent;
  
  v->adjacent[anum] = id;
  v->nadjacent++;

}

/************************
 * add vertex of given id
 */

void BD_TREE_add_vertex(bd_tree_handle_t *handle,
			int id,
			void *user_data)

{
  alloc_index_array(handle, id);
  handle->index[id] = handle->n_vertices;
  handle->vertices[handle->n_vertices].user_data = user_data;
  handle->vertices[handle->n_vertices].id = id;
  handle->n_vertices++;
}

/**********************
 * BD_TREE_tag_sub_trees()
 */

int BD_TREE_tag_sub_trees(bd_tree_handle_t *handle)
     
{

  int k;
  int tag;
  bd_tree_vertex_t *vertex;
  
  /*
   * clear the stack
   */
  
  clear_vertex_stack(handle);

  /*
   * set visited flags off
   */
  
  vertex = handle->vertices;
  for (k = 0; k < handle->n_vertices; k++, vertex++) {
    vertex->visited = 0;
  } /* k */

  /*
   * initialize Tag
   */

  tag = 0;

  vertex = handle->vertices;
  for (k = 0; k < handle->n_vertices; k++, vertex++) {
    if (vertex->visited == 0) {
      visit(handle, k, tag);
      tag++;
    }
  } /* k */

  return (tag);

}

/***********************************
 * allocate index array for vertices
 *
 * Initialize to -1
 *
 */

static void alloc_index_array(bd_tree_handle_t *handle,
			      int max_index)
     
{
  
  int i, *ip;
  int nneeded = max_index + 1;

  if (nneeded > handle->n_index_alloc) {
    if (handle->index == NULL) {
      handle->index = (int *) RMmalloc
	(nneeded * sizeof(int));
      ip = handle->index;
      for (i = 0; i < nneeded; i++, ip++) {
	*ip = -1;
      }
    } else {
      handle->index = (int *) RMrealloc
	(handle->index, nneeded * sizeof(int));
      ip = handle->index + handle->n_index_alloc;
      for (i = 0; i < nneeded - handle->n_index_alloc; i++, ip++) {
	*ip = -1;
      }
    }
    handle->n_index_alloc = nneeded;
  }
  
}

static void clear_vertex_stack(bd_tree_handle_t *handle)
{
  handle->top = 0;
}

static bd_tree_vertex_t *pop_vertex(bd_tree_handle_t *handle)
{
  if (handle->top > 0) {
    handle->top -= 1;
    return (handle->stack[handle->top]);
  } else {
    clear_vertex_stack(handle);
    return(NULL);
  }
}

static void push_vertex(bd_tree_handle_t *handle,
			bd_tree_vertex_t *v)
{
  if (handle->top > handle->n_stack_alloc - 2) {
    /*
     * realloc
     */
    handle->n_stack_alloc += TREE_STACK_INCR;
    handle->stack = (bd_tree_vertex_t **)
      RMrealloc(handle->stack,
	       handle->n_stack_alloc * sizeof(*handle->stack));
  }
  handle->stack[handle->top++] = v;
}

static int vertex_stack_empty(bd_tree_handle_t *handle)
{
  if (handle->top == 0) {
    return (1);
  } else {
    return (0);
  }
}

/*
 * visit vertex
 */

static void visit(bd_tree_handle_t *handle,
		  int k, int tag)

{

  int i;
  bd_tree_vertex_t *v;

  push_vertex(handle, handle->vertices + k);
  
  while (!vertex_stack_empty(handle)) {
    v = pop_vertex(handle);
    if (v == NULL) {
      fprintf(stderr, "ERROR - tree:visit, popped NULL v\n");
      return;
    }
    if (!v->visited) {
      v->tag = tag;
      v->visited = 1;
      for (i = 0; i < v->nadjacent; i++) {
	if (v->adjacent[i] > handle->n_index_alloc - 1) {
	  fprintf(stderr, "ALLOC ERROR - tree.c:visit()\n");
	  fprintf(stderr, "n_alloc: %d, n_needed: %d\n",
		  handle->n_index_alloc, v->adjacent[i]);
	}
	if (handle->index[v->adjacent[i]] >= 0) {
	  push_vertex(handle, (handle->vertices +
			       handle->index[v->adjacent[i]]));
	}
      } /* i */
    }
  } /* while */

}
