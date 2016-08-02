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

#include "forecast_monitor.h"
#include "sys/types.h"
#include "sys/stat.h"

/****************************************************************************
 * score_forecast.c
 *
 * Score the forecast for the given scan pair
 *
 * This is carried out for each complex track.
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
 * July 1995
 *
 ****************************************************************************/

void score_forecast(storm_file_handle_t *s_handle,
		    track_file_handle_t *t_handle,
		    vol_file_handle_t *v_handle,
		    si32 verify_scan_num,
		    si32 generate_scan_num,
		    si32 n_scans,
		    double actual_lead_time,
		    date_time_t *scan_times)

{

  si32 i, k, nadjacent;
  si32 tag;
  si32 icomplex, isimple;
  si32 complex_track_num;
  si32 simple_track_num;
  si32 n_simple_tracks;
  si32 n_vertices;
  si32 n_sub_trees;
  fm_simple_track_t *stracks, *strack;
  tree_vertex_t *vertices, *vertex;
  
  /*
   * allocate index array for vertices (one per simple track)
   */
  
  alloc_vertex_index(t_handle->header->n_simple_tracks);
  
  /*
   * complex tracks
   */
  
  for (icomplex = 0;
       icomplex < t_handle->header->n_complex_tracks; icomplex++) {
    
    complex_track_num = t_handle->complex_track_nums[icomplex];
    if(RfReadComplexTrackParams(t_handle, complex_track_num, TRUE,
				"score_forecasts") != R_SUCCESS) {
      tidy_and_exit(-1);
    }
    
    if (t_handle->complex_params->start_scan < verify_scan_num &&
	t_handle->complex_params->end_scan > generate_scan_num) {
      
      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stdout,"  --> Complex track %ld, %s - %s\n",
		(long) t_handle->complex_track_nums[icomplex],
		utimstr(t_handle->complex_params->start_time),
		utimstr(t_handle->complex_params->end_time));
      }

      n_simple_tracks = t_handle->complex_params->n_simple_tracks;

      /*
       * alloc for simple tracks array
       */
      
      alloc_simple_array(n_simple_tracks, &stracks);
      vertices = alloc_vertices(n_simple_tracks);

      /*
       * simple tracks in this complex track
       */
    
      n_vertices = 0;
      strack = stracks;
      vertex = vertices;
      
      for (isimple = 0; isimple < n_simple_tracks; isimple++) {
	
	simple_track_num =
	  t_handle->simples_per_complex[complex_track_num][isimple];
	
	if(RfRewindSimpleTrack(t_handle, simple_track_num,
			       "score_forecasts") != R_SUCCESS)
	  tidy_and_exit(-1);
	
	/*
	 * Get the relevant set of simple tracks -
	 * we only consider simple tracks which start on or before
	 * the verify scan number and end after or on the generate
	 * scan number
	 */
	
	if (t_handle->simple_params->start_scan <= verify_scan_num &&
	    t_handle->simple_params->end_scan >= generate_scan_num) {
	  
	  strack->params = *t_handle->simple_params;
	  load_vertex_index(simple_track_num, n_vertices);
	  load_storm(s_handle, t_handle,
		     verify_scan_num, generate_scan_num, strack);
	  
	  /*
	   * load up vertex with adjacency information
	   */

	  nadjacent = 0;
	  if (strack->params.start_scan > generate_scan_num) {
	    for (i = 0; i < strack->params.nparents; i++) {
	      vertex->adjacent[nadjacent] = strack->params.parent[i];
	      nadjacent++;
	    }
	  }
	  if (strack->params.end_scan < verify_scan_num) {
	    for (i = 0; i < strack->params.nchildren; i++) {
	      vertex->adjacent[nadjacent] = strack->params.child[i];
	      nadjacent++;
	    }
	  }

	  vertex->nadjacent = nadjacent;
	  vertex->name = simple_track_num;
	  
	  strack++;
	  vertex++;
	  n_vertices++;
	  
	} /* if (t_handle->simple_params->start_scan ... */

      } /* isimple */

      if (Glob->params.debug >= DEBUG_NORM) {

	strack = stracks;
	for (isimple = 0; isimple < n_vertices; isimple++, strack++) {
	  
	  simple_track_num = stracks->params.simple_track_num;
	  
	  fprintf(stdout,"        Simple track %ld, %s - %s\n",
		  (long) simple_track_num,
		  utimstr(strack[isimple].params.start_time),
		  utimstr(strack[isimple].params.end_time));
	  
	  fprintf(stdout, "              Adjacent array: ");
	  for (i = 0; i < vertices[isimple].nadjacent; i++) {
	    fprintf(stdout, "%d", vertices[isimple].adjacent[i]);
	    if (i < vertices[isimple].nadjacent - 1) {
	      fprintf(stdout, ", ");
	    }
	  } /* i */
	  fprintf(stdout, "\n");

	} /* isimple */

      } /* if (Glob->params.debug >= DEBUG_NORM) */
	
      /*
       * tag the members of the set to indicate which are
       * members of the same sub-tree
       */

      n_sub_trees = tag_sub_trees(vertices, n_vertices);
      
      if (n_sub_trees < 0) {
	fprintf(stderr, "ERROR in %s:score_forecast:tag_sub_trees\n",
		Glob->prog_name);
	tidy_and_exit(-1);
      }

      if (Glob->params.debug >= DEBUG_NORM) {
	fprintf(stdout, "n_sub_trees: %ld\n", (long) n_sub_trees);
	vertex = vertices;
	for (k = 0; k < n_vertices; k++, vertex++) {
	  fprintf(stdout, "Vertex %d, tag %d, ", k, vertex->tag);
	  fprintf(stdout, "%ld, ", (long) vertex->name);
	  for (i = 0; i < vertex->nadjacent; i++) {
	    fprintf(stdout, "%d", vertex->adjacent[i]);
	    if (i < vertex->nadjacent - 1) {
	      fprintf(stdout, ", ");
	    }
	  } /* i */
	  fprintf(stdout, "\n");
	} /* k */
	
      } /* if */

      /*
       * for each sub-tree, compute the forecast performance
       * statistics
       */

      for (tag = 0; tag < n_sub_trees; tag++) {

	analyze_sub_tree(s_handle, t_handle, v_handle,
			 verify_scan_num, generate_scan_num,
			 n_scans, actual_lead_time, scan_times,
			 tag, stracks, vertices, n_vertices);

      } /* tag */

    } /* if (t_handle->complex_params->start_scan ... */

  } /* icomplex */

  return;

}

