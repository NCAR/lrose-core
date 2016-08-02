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
 * tdata_partial_track.c
 *
 * Identifies the partial track requested.
 *
 * A complex track represents a storm which may or may not exhibit
 * mergers or splits during its lifetime. It is made up of one or
 * more simple tracks. A simple track represents a part of the
 * complex track for which no mergers or splits occurred. A merger
 * is represented by two or more simple tracks ending and one simple
 * track starting at the following scan time. The connections between
 * the simple tracks are recorded by noting the parents and children
 * of each simple track. Similarly a split is represented by one simple
 * track ending and two or more simple tracks starting at the next
 * scan time.
 *
 * In this module we represent the complex track as a tree of vertices
 * and edges, in which each simple track is a vertex, and the edges are the
 * connections between the simple tracks, represented by the list of
 * parents and children of each simple track.
 * 
 * We only consider simple tracks which start on or before
 * the verify scan number and end after or on the generate
 * scan number. This breaks the tree into a forest of sub-trees,
 * and deletes some of the edges in the complex tree.
 *
 * The first major step is to determine the connectivity of the forest,
 * thereby identifying the sub-trees.
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * Jan 1997
 *
 ****************************************************************************/

#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <titan/tdata_partial_track.h>
#include <assert.h>

static void alloc_sparams_basic(tdata_partial_track_t *part,
				si32 n_simple_tracks);

static void alloc_sparams_complete(tdata_partial_track_t *part,
				   si32 n_simple_tracks);

/****************************
 * tdata_init_partial_track()
 */

void tdata_init_partial_track(tdata_partial_track_t *part,
			      char *prog_name,
			      int debug)

{

  part->debug = debug;

  if (part->init_flag == TDATA_PARTIAL_INIT_FLAG) {
    return;
  }

  BD_TREE_init_handle(&part->tree);
  part->n_basic_alloc = 0;
  part->n_complete_alloc = 0;
  part->sparams_basic = NULL;
  part->sparams_complete = NULL;
  part->prog_name = STRdup(prog_name);
  part->init_flag = TDATA_PARTIAL_INIT_FLAG;

}

/****************************
 * tdata_free_partial_track()
 */

void tdata_free_partial_track(tdata_partial_track_t *part)

{

  if (part->init_flag != TDATA_PARTIAL_INIT_FLAG) {
    return;
  }

  BD_TREE_free_handle(&part->tree);
  if (part->sparams_basic != NULL) {
    ufree(part->sparams_basic);
  }
  part->n_basic_alloc = 0;
  part->sparams_basic = NULL;

  if (part->sparams_complete != NULL) {
    ufree(part->sparams_complete);
  }
  part->sparams_complete = NULL;
  part->n_complete_alloc = 0;

  ufree(part->prog_name);
  part->init_flag = 0;

}

/************************************
 * tdata_find_basic_partial_track()
 *
 * Find the partial track,
 * based on basic data from the server.
 *
 * returns 0 on success, -1 on failure
 */

int
tdata_find_basic_partial_track (tdata_partial_track_t *part,
				time_t partial_time,
				int past_period,
				int future_period,
			        int target_complex_num,
				int target_simple_num,
				tdata_basic_with_params_index_t *tdata_index)

{

  int i, j, k;
  int icomplex, isimple;
  int simple_num;
  int n_simple_tracks;
  int n_sub_trees;
  
  bd_tree_vertex_t *vertex;
  tdata_basic_header_t *header;
  tdata_basic_complex_params_t *ct_params = NULL;
  tdata_basic_simple_params_t *st_params;
  tdata_basic_simple_params_t *sparams;

  assert(part->init_flag == TDATA_PARTIAL_INIT_FLAG);
  
  header = &tdata_index->header;

  part->past_period = past_period;
  part->future_period = future_period;
  part->start_time = partial_time - past_period;
  part->end_time = partial_time + future_period;
  part->complex_num = target_complex_num;
  part->simple_num = target_simple_num;
  
  /*
   * loop through the complex tracks, looking for the selected
   * one
   */

  if (part->debug) {
    fprintf(stderr, "partial complex_track: %d\n", target_complex_num);
    fprintf(stderr, "partial simple_track: %d\n", target_simple_num);
    fprintf(stderr, "partial start time: %s\n",
	    utimstr(part->start_time));
    fprintf(stderr, "partial end time: %s\n",
	    utimstr(part->end_time));
  }

  for (icomplex = 0;
       icomplex < header->n_complex_tracks; icomplex++) {
    
    ct_params = tdata_index->complex_params + icomplex;
    if (ct_params->complex_track_num == target_complex_num) {
      break;
    }

  }

  /*
   * check that the track was found
   */
  
  if (icomplex == header->n_complex_tracks) {
    if (part->debug) {
      fprintf(stderr, "WARNING - %s:partial_track\n", part->prog_name);
      fprintf(stderr, "Cannot find complex_track_num %d\n",
	      target_complex_num);
    }
    return (-1);
  }
  
  /*
   * allocate index array for vertices (one per simple track)
   */
  
  n_simple_tracks = ct_params->n_simple_tracks;

  /*
   * alloc for simple tracks array
   */
      
  alloc_sparams_basic(part, n_simple_tracks);
  BD_TREE_alloc_vertices(&part->tree, n_simple_tracks,
			 TDATA_MAX_PARENTS + TDATA_MAX_CHILDREN);

  /*
   * simple tracks in this complex track
   */

  BD_TREE_start(&part->tree);

  sparams = part->sparams_basic;
  vertex = part->tree.vertices;
  
  st_params = tdata_index->simple_params[icomplex];
  
  for (isimple = 0; isimple < n_simple_tracks; isimple++,
	 st_params++, sparams++) {
	
    simple_num = st_params->simple_track_num;

    /*
     * copy simple params into array
     */

    *sparams = *st_params;
    
    /*
     * Get the relevant set of simple tracks -
     * we only consider simple tracks which overlap with the
     * partial track period
     */
	
    if (st_params->start_time <= part->end_time &&
	st_params->end_time >= part->start_time) {

      BD_TREE_add_vertex(&part->tree, simple_num, sparams);
      
      /*
       * load up vertex with adjacency information
       */
      
      if (sparams->start_time >= part->start_time) {
	for (i = 0; i < sparams->nparents; i++) {
	  BD_TREE_add_adjacent(&part->tree, sparams->parent[i]);
	}
      }
      if (sparams->end_time <= part->end_time) {
	for (i = 0; i < sparams->nchildren; i++) {
	  BD_TREE_add_adjacent(&part->tree, sparams->child[i]);
	}
      }
      
    } /* if (st_params->start_time <= part->end_time ... */
    
  } /* isimple */

  if (part->debug) {

    for (i = 0; i < part->tree.n_vertices; i++) {
      
      sparams = (tdata_basic_simple_params_t *) part->tree.vertices[i].user_data;
      
      fprintf(stderr,"        Simple track %ld, %s - %s\n",
	      (long) sparams->simple_track_num,
	      utimstr(sparams->start_time),
	      utimstr(sparams->end_time));
      
      fprintf(stderr, "              Adjacent array: ");
      for (j = 0; j < part->tree.vertices[i].nadjacent; j++) {
	fprintf(stderr, "%d", part->tree.vertices[i].adjacent[j]);
	if (j < part->tree.vertices[i].nadjacent - 1) {
	  fprintf(stderr, ", ");
	}
      } /* j */
      fprintf(stderr, "\n");
    
    } /* i */

  } /* if (part->debug) */
  
  /*
   * tag the members of the set to indicate which are
   * members of the same sub-tree
   */
  
  n_sub_trees = BD_TREE_tag_sub_trees(&part->tree);
  
  if (n_sub_trees < 0) {
    fprintf(stderr, "ERROR in %s:partial_track:tag_sub_trees\n",
	    part->prog_name);
    return(-1);
  }

  /*
   * find the tag for the chosen track
   */

  part->tag = -1;
  vertex = part->tree.vertices;
  for (k = 0; k < part->tree.n_vertices; k++, vertex++) {
    sparams = (tdata_basic_simple_params_t *) vertex->user_data;
    if (sparams->simple_track_num == target_simple_num &&
	sparams->complex_track_num == target_complex_num) {
      part->tag = vertex->tag;
      break;
    }
  }

  if (part->debug) {
    fprintf(stderr, "n_sub_trees: %d\n", n_sub_trees);
    fprintf(stderr, "Chosen_tag: %d\n", part->tag);
    vertex = part->tree.vertices;
    for (k = 0; k < part->tree.n_vertices; k++, vertex++) {
      if (vertex->tag == part->tag) {
	fprintf(stderr, "----> ");
      } else {
	fprintf(stderr, "      ");
      }
      fprintf(stderr, "Vertex %d, tag %d, ", k, vertex->tag);
      sparams = (tdata_basic_simple_params_t *) vertex->user_data;
      fprintf(stderr, "simple num %d, ", (int) sparams->simple_track_num);
      fprintf(stderr, "complex num %d, ", (int) sparams->complex_track_num);
      for (i = 0; i < vertex->nadjacent; i++) {
	fprintf(stderr, "%d", vertex->adjacent[i]);
	if (i < vertex->nadjacent - 1) {
	  fprintf(stderr, ", ");
	}
      } /* i */
      fprintf(stderr, "\n");
    } /* k */
  }
  
  if (part->tag < 0) {
    return (-1);
  }
  
  return (0);

}

/************************************
 * tdata_find_partial_track(),
 *
 * Find the partial track,
 * based on complete data from the server.
 *
 * returns 0 on success, -1 on failure
 */

int
tdata_find_partial_track (tdata_partial_track_t *part,
			  time_t partial_time,
			  int past_period,
			  int future_period,
			  int target_complex_num,
			  int target_simple_num,
			  tdata_complete_index_t *tdata_index)

{

  int i, j, k;
  int icomplex, isimple;
  int simple_num;
  int n_simple_tracks;
  int n_sub_trees;
  
  bd_tree_vertex_t *vertex;
  tdata_complete_header_t *header;
  complex_track_params_t *ct_params = NULL;
  simple_track_params_t *st_params;
  simple_track_params_t *sparams;

  assert(part->init_flag == TDATA_PARTIAL_INIT_FLAG);
  
  header = &tdata_index->header;

  part->past_period = past_period;
  part->future_period = future_period;
  part->start_time = partial_time - past_period;
  part->end_time = partial_time + future_period;
  part->complex_num = target_complex_num;
  part->simple_num = target_simple_num;

  /*
   * loop through the complex tracks, looking for the selected
   * one
   */

  if (part->debug) {
    fprintf(stderr, "partial complex_track: %d\n", target_complex_num);
    fprintf(stderr, "partial simple_track: %d\n", target_simple_num);
    fprintf(stderr, "partial start time: %s\n",
	    utimstr(part->start_time));
    fprintf(stderr, "partial end time: %s\n",
	    utimstr(part->end_time));
  }

  for (icomplex = 0;
       icomplex < header->n_complex_tracks; icomplex++) {
    
    ct_params = tdata_index->complex_params + icomplex;
    if (ct_params->complex_track_num == target_complex_num) {
      break;
    }

  }

  /*
   * check that the track was found
   */
  
  if (icomplex == header->n_complex_tracks) {
    if (part->debug) {
      fprintf(stderr, "WARNING - %s:partial_track\n", part->prog_name);
      fprintf(stderr, "Cannot find complex_track_num %d\n",
	      target_complex_num);
    }
    return (-1);
  }
  
  /*
   * allocate index array for vertices (one per simple track)
   */
  
  n_simple_tracks = ct_params->n_simple_tracks;

  /*
   * alloc for simple tracks array
   */
      
  alloc_sparams_complete(part, n_simple_tracks);
  BD_TREE_alloc_vertices(&part->tree, n_simple_tracks,
			 TDATA_MAX_PARENTS + TDATA_MAX_CHILDREN);

  /*
   * simple tracks in this complex track
   */

  BD_TREE_start(&part->tree);

  sparams = part->sparams_complete;
  vertex = part->tree.vertices;
  
  st_params = tdata_index->simple_params[icomplex];
  
  for (isimple = 0; isimple < n_simple_tracks; isimple++,
	 st_params++, sparams++) {
	
    simple_num = st_params->simple_track_num;

    /*
     * copy simple params into array
     */

    *sparams = *st_params;
    
    /*
     * Get the relevant set of simple tracks -
     * we only consider simple tracks which overlap with the
     * partial track period
     */
	
    if (st_params->start_time <= part->end_time &&
	st_params->end_time >= part->start_time) {

      BD_TREE_add_vertex(&part->tree, simple_num, sparams);
      
      /*
       * load up vertex with adjacency information
       */
      
      if (sparams->start_time >= part->start_time) {
	for (i = 0; i < sparams->nparents; i++) {
	  BD_TREE_add_adjacent(&part->tree, sparams->parent[i]);
	}
      }
      if (sparams->end_time <= part->end_time) {
	for (i = 0; i < sparams->nchildren; i++) {
	  BD_TREE_add_adjacent(&part->tree, sparams->child[i]);
	}
      }
      
    } /* if (st_params->start_time <= part->end_time ... */
    
  } /* isimple */

  if (part->debug) {

    for (i = 0; i < part->tree.n_vertices; i++) {
      
      sparams = (simple_track_params_t *) part->tree.vertices[i].user_data;
      
      fprintf(stderr,"        Simple track %ld, %s - %s\n",
	      (long) sparams->simple_track_num,
	      utimstr(sparams->start_time),
	      utimstr(sparams->end_time));
      
      fprintf(stderr, "              Adjacent array: ");
      for (j = 0; j < part->tree.vertices[i].nadjacent; j++) {
	fprintf(stderr, "%d", part->tree.vertices[i].adjacent[j]);
	if (j < part->tree.vertices[i].nadjacent - 1) {
	  fprintf(stderr, ", ");
	}
      } /* j */
      fprintf(stderr, "\n");
    
    } /* i */

  } /* if (part->debug) */
  
  /*
   * tag the members of the set to indicate which are
   * members of the same sub-tree
   */
  
  n_sub_trees = BD_TREE_tag_sub_trees(&part->tree);
  
  if (n_sub_trees < 0) {
    fprintf(stderr, "ERROR in %s:partial_track:tag_sub_trees\n",
	    part->prog_name);
    return(-1);
  }

  /*
   * find the tag for the chosen track
   */

  part->tag = -1;
  vertex = part->tree.vertices;
  for (k = 0; k < part->tree.n_vertices; k++, vertex++) {
    sparams = (simple_track_params_t *) vertex->user_data;
    if (sparams->simple_track_num == target_simple_num &&
	sparams->complex_track_num == target_complex_num) {
      part->tag = vertex->tag;
      break;
    }
  }

  if (part->debug) {
    fprintf(stderr, "n_sub_trees: %d\n", n_sub_trees);
    fprintf(stderr, "Chosen_tag: %d\n", part->tag);
    vertex = part->tree.vertices;
    for (k = 0; k < part->tree.n_vertices; k++, vertex++) {
      if (vertex->tag == part->tag) {
	fprintf(stderr, "----> ");
      } else {
	fprintf(stderr, "      ");
      }
      fprintf(stderr, "Vertex %d, tag %d, ", k, vertex->tag);
      sparams = (simple_track_params_t *) vertex->user_data;
      fprintf(stderr, "simple num %d, ", (int) sparams->simple_track_num);
      fprintf(stderr, "complex num %d, ", (int) sparams->complex_track_num);
      for (i = 0; i < vertex->nadjacent; i++) {
	fprintf(stderr, "%d", vertex->adjacent[i]);
	if (i < vertex->nadjacent - 1) {
	  fprintf(stderr, ", ");
	}
      } /* i */
      fprintf(stderr, "\n");
    } /* k */
  }
  
  if (part->tag < 0) {
    return (-1);
  }
  
  return (0);

}

/********************************
 * tdata_basic_entry_in_partial()
 *
 * Is this entry in the partial track,
 * based on basic data from the server?
 */

int tdata_basic_entry_in_partial(tdata_partial_track_t *part,
				 tdata_basic_track_entry_t *entry)

{

  int k;
  si32 simple_track_num = entry->simple_track_num;
  si32 complex_track_num = entry->complex_track_num;
  tdata_basic_simple_params_t *sparams;
  bd_tree_vertex_t *vertex;
  
  assert(part->init_flag == TDATA_PARTIAL_INIT_FLAG);

  if (entry->time < part->start_time ||
      entry->time > part->end_time) {
    return (FALSE);
  }

  vertex = part->tree.vertices;
  for (k = 0; k < part->tree.n_vertices; k++, vertex++) {
    if (vertex->tag == part->tag) {
      sparams = (tdata_basic_simple_params_t *) vertex->user_data;
      if (sparams->simple_track_num == simple_track_num &&
	  sparams->complex_track_num == complex_track_num)
	return (TRUE);
    }
  } /* k */

  return (FALSE);

}

/********************************
 * tdata_entry_in_partial()
 *
 * Is this entry in the partial track,
 * based on complete data from the server?
 */

int tdata_entry_in_partial(tdata_partial_track_t *part,
			   track_file_entry_t *entry)

{

  int k;
  si32 simple_track_num = entry->simple_track_num;
  si32 complex_track_num = entry->complex_track_num;
  simple_track_params_t *sparams;
  bd_tree_vertex_t *vertex;
  
  assert(part->init_flag == TDATA_PARTIAL_INIT_FLAG);

  if (entry->time < part->start_time ||
      entry->time > part->end_time) {
    return (FALSE);
  }

  vertex = part->tree.vertices;
  for (k = 0; k < part->tree.n_vertices; k++, vertex++) {
    if (vertex->tag == part->tag) {
      sparams = (simple_track_params_t *) vertex->user_data;
      if (sparams->simple_track_num == simple_track_num &&
	  sparams->complex_track_num == complex_track_num)
	return (TRUE);
    }
  } /* k */

  return (FALSE);

}

static void alloc_sparams_basic(tdata_partial_track_t *part,
				si32 n_simple_tracks)
     
{
  
  int n_extra_needed;

  assert(part->init_flag == TDATA_PARTIAL_INIT_FLAG);

  /*
   * allocate array for simple tracks
   */
  
  n_extra_needed = n_simple_tracks - part->n_basic_alloc;
  
  if (n_extra_needed > 0) {
    
    if (part->sparams_basic == NULL) {
      
      part->sparams_basic = (tdata_basic_simple_params_t *) umalloc
	(n_simple_tracks * sizeof(tdata_basic_simple_params_t));

    } else {
      
      part->sparams_basic = (tdata_basic_simple_params_t *) urealloc
	(part->sparams_basic,
	 n_simple_tracks * sizeof(tdata_basic_simple_params_t));

    }
    
    part->n_basic_alloc = n_simple_tracks;

  }

  return;

}

static void alloc_sparams_complete(tdata_partial_track_t *part,
				   si32 n_simple_tracks)

{
  
  int n_extra_needed;

  assert(part->init_flag == TDATA_PARTIAL_INIT_FLAG);

  /*
   * allocate array for simple tracks
   */
  
  n_extra_needed = n_simple_tracks - part->n_complete_alloc;
  
  if (n_extra_needed > 0) {
    
    if (part->sparams_complete == NULL) {
      
      part->sparams_complete = (simple_track_params_t *) umalloc
	(n_simple_tracks * sizeof(simple_track_params_t));

    } else {
      
      part->sparams_complete = (simple_track_params_t *) urealloc
	(part->sparams_complete,
	 n_simple_tracks * sizeof(simple_track_params_t));

    }
    
    part->n_complete_alloc = n_simple_tracks;

  }

  return;

}

