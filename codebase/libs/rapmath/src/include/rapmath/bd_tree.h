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
/************************************************
 * bd_tree.h : header file for bi-directional
 *             tree module
 ************************************************/

#ifndef bd_tree_h
#define bd_tree_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

  int id;
  int tag;
  int nadjacent;
  int visited;
  int n_adj_alloc;
  int *adjacent;

  /*
   * the following is set by the user
   * to point to data related to the vertex
   */
  
  void *user_data;

} bd_tree_vertex_t;

typedef struct {

  int n_vert_alloc;
  int n_index_alloc;
  int n_stack_alloc;
  int n_vertices;

  int top;
  int *index;
  bd_tree_vertex_t *vertices;
  bd_tree_vertex_t **stack;

} bd_tree_handle_t;

/*
 * prototypes
 */

extern void BD_TREE_init_handle(bd_tree_handle_t *handle);

extern void BD_TREE_free_handle(bd_tree_handle_t *handle);

extern void BD_TREE_alloc_vertices(bd_tree_handle_t *handle,
				   int n_vertices,
				   int max_adjacent);

extern void BD_TREE_start(bd_tree_handle_t *handle);

extern void BD_TREE_add_adjacent(bd_tree_handle_t *handle,
				 int id);

extern void BD_TREE_add_vertex(bd_tree_handle_t *handle,
			       int id,
			       void *user_data);

extern int BD_TREE_tag_sub_trees(bd_tree_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif




