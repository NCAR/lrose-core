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
 * add_overlap.c
 *
 * Add overlap to match list
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * March 1996
 *
 ****************************************************************************/

#include "storm_track.h"

void add_overlap(storm_status_t *storms1,
		 storm_status_t *storms2,
		 si32 istorm, si32 jstorm,
		 double area_overlap)

{

  int i;
  int complex_already_present;
  
  si32 merging_complex_num;
  si32 n_simple_after_merger;

  storm_status_t *storm1;
  storm_status_t *storm2;
  track_match_t *match;
  
  storm1 = storms1 + istorm;
  storm2 = storms2 + jstorm;

  /*
   * check if complex num has already been added to merger
   */
  
  merging_complex_num = storm1->track->complex_track_num;
  match = storm2->match_array;

  complex_already_present = FALSE;
  for (i = 0; i < storm2->n_match; i++, match++) {
    if (match->complex_track_num == merging_complex_num) {
      complex_already_present = TRUE;
      break;
    }
  }

  if (complex_already_present) {
    n_simple_after_merger = storm2->sum_simple_tracks;
  } else {
    n_simple_after_merger =
      storm2->sum_simple_tracks + storm1->track->n_simple_tracks;
  }

  /*
   * add data to storm1 array - potential splits
   */
  
  alloc_match_array(storm1->n_match + 1, storm1);
  match = storm1->match_array + storm1->n_match;
  
  match->storm_num = jstorm;
  match->overlap = area_overlap;

  storm1->sum_overlap += area_overlap;
  storm1->n_match++;
    
  /*
   * add data to storm2 array - potential mergers
   */
  
  alloc_match_array(storm2->n_match + 1, storm2);
  match = storm2->match_array + storm2->n_match;
  
  match->complex_track_num = merging_complex_num;
  match->storm_num = istorm;
  match->overlap = area_overlap;
  match->n_simple_tracks = storm1->track->n_simple_tracks;

  storm2->sum_overlap += area_overlap;
  storm2->n_match++;

  if (!complex_already_present) {
    storm2->sum_simple_tracks = n_simple_after_merger;
  }
  
  return;
  
}

#ifdef JUNK

static void add_overlap_(storm_status_t *storm,
			 si32 match_num,
			 double area_overlap,
			 int type)
     
{

  int already_stored = FALSE;
  int i;
  track_match_t *match = storm->match_array;

  /*
   * check if match has already been stored
   */

  for (i = 0; i < storm->n_match; i++, match++) {
    if (match_num == match->storm_num) {
      already_stored = TRUE;
      break;
    }
  }

  /*
   * if not already stored, store it
   */

  if (!already_stored) {
  
    alloc_match_array(storm->n_match + 1, storm);
  
    storm->match_array[storm->n_match].storm_num = match_num;
    storm->match_array[storm->n_match].overlap = area_overlap;
    storm->match_array[storm->n_match].type = type;
    storm->sum_overlap += area_overlap;
    storm->n_match++;
    
  }

  return;

}

#endif
