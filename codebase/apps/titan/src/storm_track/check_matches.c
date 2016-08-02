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
 * check_matches.c
 *
 * Check the matches to make sure the max number of parents and
 * children is not exceeded.
 *
 * In cases in which the max is exceeded, the storms with the smallest
 * overlaps are deleted from the match array.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * March 1996
 *
 ****************************************************************************/

#include "storm_track.h"

/*
 * file scope prototypes
 */

static void delete_match(storm_status_t *storm, si32 storm_num);

static void delete_smallest_overlap(si32 this_storm_num,
				    storm_status_t *these_storms,
				    storm_status_t *other_storms);

/*
 * main
 */

void check_matches(storm_status_t *storms1,
		   storm_status_t *storms2,
		   si32 nstorms1,
		   si32 nstorms2)
     
{

  int istorm, jstorm;
  storm_status_t *storm;

  /*
   * check on MAX_CHILDREN limit for storms1 entries
   */

  storm = storms1;
  for (istorm = 0; istorm < nstorms1; istorm++, storm++) {
    
    while (storm->n_match > MAX_CHILDREN) {

      if (Glob->params.debug >= DEBUG_EXTRA) {
	fprintf(stderr,
		"MAX_CHILDREN limit - deleting smallest from storm1 %d\n",
		istorm);
      }

      delete_smallest_overlap(istorm, storms1, storms2);
      
    } /* while */

  } /* istorm */

  /*
   * check on MAX_PARENTS limit for storms2 entries
   */

  storm = storms2;
  for (jstorm = 0; jstorm < nstorms2; jstorm++, storm++) {
    
    while (storm->n_match > MAX_PARENTS) {

      if (Glob->params.debug >= DEBUG_EXTRA) {
	fprintf(stderr,
		"MAX_PARENTS limit - deleting smallest from storm2 %d\n",
		jstorm);
      }

      delete_smallest_overlap(jstorm, storms2, storms1);
      
    } /* while */

  } /* jstorm */

 return;

}

static void delete_smallest_overlap(si32 this_storm_num,
				    storm_status_t *these_storms,
				    storm_status_t *other_storms)
				    
{

  int i;
  int match_num;
  double min_overlap = 1.0e99;
  track_match_t *match;
  storm_status_t *this_storm;
  storm_status_t *match_storm;

  this_storm = these_storms + this_storm_num;
  
  /*
   * delete smallest overlap from this storm's matches
   */

  match = this_storm->match_array;

  for (i = 0; i < this_storm->n_match; i++, match++) {
    if (match->overlap < min_overlap) {
      min_overlap = match->overlap;
      match_num = match->storm_num;
    }
  } /* i */

  delete_match(this_storm, match_num);

  /*
   * delete the matching entry in the other storms array
   */

  match_storm = other_storms + match_num;
  delete_match(match_storm, this_storm_num);

}

static void delete_match(storm_status_t *storm, si32 storm_num)

{

  int i;
  int  index = -1;
  int complex_used_elsewhere;
  si32 complex_track_num;
  track_match_t *match, *match_1;

  /*
   * find the index of the match to be removed
   */
  
  match = storm->match_array;
  for (i = 0; i < storm->n_match; i++, match++) {
    if (match->storm_num == storm_num) {
      index = i;
      complex_track_num = match->complex_track_num;
      break;
    }
  } /* i */

  if (index < 0) {
    fprintf(stderr, "ERROR - %s:check_matches\n", Glob->prog_name);
    fprintf(stderr, "Cannot find storm_num %d to remove\n",
	    storm_num);
    return;
  }

  /*
   * Check if complex num for index is used by a different match.
   * If not, subtract from sum_simple_tracks.
   */

  if (storm->sum_simple_tracks > 0) {
    complex_used_elsewhere = FALSE;
    match = storm->match_array;
    for (i = 0; i < storm->n_match; i++, match++) {
      if (i != index) {
	if (match->complex_track_num == complex_track_num) {
	  complex_used_elsewhere = TRUE;
	  break;
	} 
      }
    }
    if (!complex_used_elsewhere) {
      storm->sum_simple_tracks -= match->n_simple_tracks;
    }
  }

  /*
   * subtract from sum_overlap
   */
  
  storm->sum_overlap -= match->overlap;

  /*
   * move all of the entries above the index down by 1
   */

  match = storm->match_array + index;
  match_1 = match + 1;
  
  for (i = index; i < storm->n_match - 1;
       i++, match++, match_1++) {
    *match = *match_1;
  } /* i */

  /*
   * decrement the number of matches
   */

  storm->n_match--;

  return;

}

