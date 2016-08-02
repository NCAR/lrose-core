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
/**************************************************************
 * resolve_matches.c
 *
 * Matches are derived in two ways:
 *
 *  1. find_overlap() matches storms based on overlap
 *     between current shape and the previous forecast shape.
 *  2. For those storms which were not matched in step 1,
 *     match_storms() finds matches between storms
 *     based on distance apart, similar attributes etc.
 *
 * This routine adds the step 2 matches with those from step 1.
 *
 */

#include "storm_track.h"

void resolve_matches(storm_status_t *storms1,
		     storm_status_t *storms2,
		     si32 nstorms1, si32 nstorms2)
     
{

  int i, j, istorm, jstorm;
  storm_status_t *storm1;
  storm_status_t *storm2;
  track_match_t *match1, *match2;

  storm1 = storms1;
  for (istorm = 0; istorm < nstorms1; istorm++, storm1++) {
    storm1->starts = FALSE;
    storm1->stops = FALSE;
    storm1->continues = FALSE;
    storm1->has_split = FALSE;
    storm1->has_merger = FALSE;
  }
    
  storm2 = storms2;
  for (jstorm = 0; jstorm < nstorms2; jstorm++, storm2++) {
    storm2->starts = FALSE;
    storm2->stops = FALSE;
    storm2->continues = FALSE;
    storm2->has_split = FALSE;
    storm2->has_merger = FALSE;
  }
    
  storm2 = storms2;
  for (jstorm = 0; jstorm < nstorms2; jstorm++, storm2++) {
    
    if (storm2->match >= 0) {

      storm2->continues = TRUE;

    } else if (storm2->n_match == 0) {

      storm2->starts = TRUE;

    } else if (storm2->n_match == 1 &&
	       storms1[storm2->match_array[0].storm_num].n_match == 1) {

      storm2->continues = TRUE;
      storm2->match = storm2->match_array[0].storm_num;

    } else {

      if (storm2->n_match > 1) {
	storm2->has_merger = TRUE;
      }

      match2 = storm2->match_array;
      for (j = 0; j < storm2->n_match; j++, match2++) {
	storm1 = storms1 + match2->storm_num;
	if (storm1->n_match > 1) {
	  storm2->has_split = TRUE;
	  match1 = storm1->match_array;
	  for (i = 0; i < storm1->n_match; i++, match1++) {
	    if (storms2[match1->storm_num].n_match > 1) {
	      storm2->has_merger = TRUE;
	    }
	  } /* i */
	}
      } /* j */

    } /* if (storm2->match > 0) */

  } /* jstorm */

  storm1 = storms1;
  for (istorm = 0; istorm < nstorms1; istorm++, storm1++) {
    
    if (storm1->match >= 0) {

      storm1->continues = TRUE;

    } else if (storm1->n_match == 0) {

      storm1->stops = TRUE;

    } else if (storm1->n_match == 1 &&
	       storms2[storm1->match_array[0].storm_num].n_match == 1) {
      
      storm1->continues = TRUE;
      storm1->match = storm1->match_array[0].storm_num;

    } else {

      if (storm1->n_match > 1) {
	storm1->has_split = TRUE;
      }

      match1 = storm1->match_array;
      for (i = 0; i < storm1->n_match; i++, match1++) {
	storm2 = storms2 + match1->storm_num;
	if (storm2->n_match > 1) {
	  storm1->has_merger = TRUE;
	  match2 = storm2->match_array;
	  for (j = 0; j < storm2->n_match; j++, match2++) {
	    if (storms1[match2->storm_num].n_match > 1) {
	      storm1->has_split = TRUE;
	    }
	  } /* i */
	}
      } /* j */

      for (i = 0; i < storm1->n_match; i++) {
	if (storms2[storm1->match_array[i].storm_num].n_match > 1) {
	  storm1->has_merger = TRUE;
	}
      } /* i */

    } /* if (storm1->match > 0) */

  } /* istorm */

  return;

}

