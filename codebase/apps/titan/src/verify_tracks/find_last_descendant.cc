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
/*******************************************************************************
 * find_last_descendant.c
 *
 * Finds the last descendant for the given simple track, and loads up
 * the last_descendant values in the simple_params_t struct.
 *
 * Recursive routine
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * January 1992
 *
 *******************************************************************************/

#include "verify_tracks.h"

void find_last_descendant(track_file_handle_t *t_handle,
			  int level)

{

  long i;
  long child_track_num;
  static simple_track_params_t last_params;
  static ui08 *visited = NULL;
  static int nvisited_alloc = 0;
  simple_track_params_t params_copy;

  /*
   * make local copy of the simple params
   */

  params_copy = *t_handle->simple_params;

  /*
   * if level 0, copy simple params to last_params, which is static
   * and initialize visited array
   */

  if (level == 0) {

    last_params = *t_handle->simple_params;

    if (t_handle->header->n_simple_tracks > nvisited_alloc) {
      if (visited == NULL) {
	visited = (ui08*) umalloc(t_handle->header->n_simple_tracks);
      } else {
	visited = (ui08 *) urealloc(visited, t_handle->header->n_simple_tracks);
      }
      nvisited_alloc = t_handle->header->n_simple_tracks;
    }

    memset(visited, 0, nvisited_alloc);

  }
  
  if (Glob->debug)
    fprintf(stderr, "find_last_descendent, level %d, "
	    "simple_track_num %ld, end_scan %ld\n",
	    level, (long) params_copy.simple_track_num,
	    (long) params_copy.end_scan);

  if (params_copy.nchildren > 0) {

    /*
     * for each child, read in the simple params, and find the
     * last descendant for that child
     */

    if (Glob->debug) {
      fprintf(stderr, "===> simple_track %d has children: ",
	      t_handle->simple_params->simple_track_num);
      for (i = 0; i < params_copy.nchildren; i++) {
	fprintf(stderr, "%d ", params_copy.child[i]);
      }
      fprintf(stderr, " <===\n");
    }

    for (i = 0; i < params_copy.nchildren; i++) {
      
      child_track_num = params_copy.child[i];

      if (visited[child_track_num] == FALSE) {
      
	if (Glob->debug)
	  fprintf(stderr, "---> simple_track %d, recursing to child %ld\n",
		  t_handle->simple_params->simple_track_num,
		  child_track_num);
	
	if(RfReadSimpleTrackParams(t_handle, child_track_num,
				   "find_last_descendant")) {
	  fprintf(stderr, "Error on recursive level %d\n", level);
	  tidy_and_exit(-1);
	}
	
	find_last_descendant(t_handle, level + 1);

	visited[child_track_num] = TRUE;

      }

    } /* i */

  } else {

    /*
     * no children, so check to see if the end scan of this simple
     * track is later than the static last_params end scan. If so
     * copy to last_params
     */

    if (t_handle->simple_params->end_scan > last_params.end_scan) {
      last_params = *t_handle->simple_params;
      if (Glob->debug)
	fprintf(stderr, "Updating last params, last_scan = %ld\n",
		(long) last_params.end_scan);
    }
    
  } /* if (params_copy.nchildren > 0) */

  /*
   * if level 0, set the last_descendant params, copy the params back
   */

  if (level == 0) {

    params_copy.last_descendant_simple_track_num =
      last_params.simple_track_num;
    params_copy.last_descendant_end_scan = last_params.end_scan;
    params_copy.last_descendant_end_time = last_params.end_time;

    *t_handle->simple_params = params_copy;
    
    if(RfWriteSimpleTrackParams(t_handle,
				t_handle->simple_params->simple_track_num,
				"find_last_descendant")) {
      fprintf(stderr, "Error on recursive level %d\n", level);
      tidy_and_exit(-1);
    }

    if (Glob->debug)
      fprintf(stderr, "EXITING, last_scan = %ld\n",
	      (long) t_handle->simple_params->last_descendant_end_scan);

  }

}
