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
 * consolidate_complex_tracks.c
 *
 * Storms which merge and/or split may have different complex track numbers.
 * These complex tracks must be combined.
 *
 * This routine searches for groups of storms which are part of a
 * common merger/split. The complex tracks in this group are
 * consolidated.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * April 1996
 *
 ****************************************************************************/

#include "storm_track.h"

/*
 * file scope variables
 */

static si32 *Complex_nums = NULL;
static si32 *Storm1_nums = NULL;
static si32 N_complex_alloc = 0;
static si32 N_complex_nums;

/*
 * file scope prototypes
 */

static void alloc_complex_nums(si32 n_complex_needed);

static int compare_si32s(const void *, const void *);

static void consolidate(track_file_handle_t *t_handle,
			storm_status_t *storms1,
			storm_status_t *storms2,
			si32 nstorms1,
			si32 nstorms2,
			si32 lower_track_num,
			si32 higher_track_num,
			track_utime_t *track_utime);

static void process_storms1_entry(storm_status_t *storms1,
				  storm_status_t *storms2,
				  si32 nstorms1,
				  si32 nstorms2,
				  si32 storm1_num,
				  si32 storm2_num);

static void process_storms2_entry(storm_status_t *storms1,
				  storm_status_t *storms2,
				  si32 nstorms1,
				  si32 nstorms2,
				  si32 storm1_num,
				  si32 storm2_num);

/*
 * principal routine
 */

void consolidate_complex_tracks(track_file_handle_t *t_handle,
				storm_status_t *storms1,
				storm_status_t *storms2,
				si32 nstorms1,
				si32 nstorms2,
				track_utime_t *track_utime)
     
{

  int i, jstorm, kstorm;
  si32 min_complex;
  storm_status_t *storm;
  track_match_t *match;
  /*
   * initialize storms2 array for checking
   */
  
  storm = storms2;
  for (jstorm = 0; jstorm < nstorms2; jstorm++, storm++) {
    storm->checked = FALSE;
  }
  
  /*
   * loop through each storm2 entry
   */
  
  storm = storms2;
  for (jstorm = 0; jstorm < nstorms2; jstorm++, storm++) {
    
    if (!storm->checked &&
	(storm->has_merger || storm->has_split)) {
      
      storm->checked = TRUE;
      N_complex_nums = 0;
      
      match = storm->match_array;
      for (kstorm = 0; kstorm < storm->n_match; kstorm++, match++) {

	/*
	 * search recursively to find all complex tracks in the
	 * merger / split
	 */
	
	if (Glob->params.debug >= DEBUG_EXTRA) {
	  fprintf(stderr, "STARTING_SEARCH ....\n");
	}

	process_storms1_entry(storms1, storms2, nstorms1, nstorms2,
			      match->storm_num, jstorm);
      } /* kstorm */
      
      if (Glob->params.debug >= DEBUG_EXTRA) {
	fprintf(stderr, "#### COMMON COMPLEX NUMS - Storm2 array %ld ####\n",
		(long) jstorm);
	for (i = 0; i < N_complex_nums; i++) {
	  fprintf(stderr, "%ld/%ld",
		  (long) Storm1_nums[i], (long) Complex_nums[i]);
	  if (i == N_complex_nums - 1) {
	    fprintf(stderr, "\n");
	  } else {
	    fprintf(stderr, " ");
	  }
	} /* i */
      }
      
      /*
       * find min complex num
       */

      min_complex = 999999999;
      for (i = 0; i < N_complex_nums; i++) {
	min_complex = MIN(min_complex, Complex_nums[i]);
      }

      /*
       * consolidate the tracks, using the min complex number
       */

      for (i = 0; i < N_complex_nums; i++) {
	if (Complex_nums[i] != min_complex) {
	  consolidate(t_handle,
		      storms1, storms2,
		      nstorms1, nstorms2,
		      min_complex,
		      Complex_nums[i],
		      track_utime);
	  
	}
      } /* i */
      
    } /*  if (!storm->checked) */

  } /* jstorm */
  
 return;

}

/**********************
 * alloc_complex_nums()
 *
 * Memory allocation
 */

static void alloc_complex_nums(si32 n_complex_needed)

{

  if (n_complex_needed > N_complex_alloc) {
    if (Complex_nums == NULL) {
      Complex_nums = (si32 *) umalloc (n_complex_needed * sizeof(si32));
      Storm1_nums = (si32 *) umalloc (n_complex_needed * sizeof(si32));
    } else {
      Complex_nums = (si32 *) urealloc
	((char *) Complex_nums, n_complex_needed * sizeof(si32));
      Storm1_nums = (si32 *) urealloc
	((char *) Storm1_nums, n_complex_needed * sizeof(si32));
    }
    N_complex_alloc = n_complex_needed;
  }
  
  return;

}

/****************************************************************************
 * consolidate()
 *
 * Consolidates the file entries for two complex storm tracks - this
 * is required when two complex tracks merge, forming one complex
 * track. The consolidation is done in favor of the first track, which
 * is the older of the two.
 *
 * The complex tracks (if any) with a higher number than the vacated
 * position are moved down to fill the gap. One entry for a complex
 * track (the highest one) will be left vacant in the file until an
 * additional complex track is required.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1991
 *
 ****************************************************************************/

static void consolidate(track_file_handle_t *t_handle,
			storm_status_t *storms1,
			storm_status_t *storms2,
			si32 nstorms1,
			si32 nstorms2,
			si32 lower_track_num,
			si32 higher_track_num,
			track_utime_t *track_utime)

{

  si32 istorm, ientry;
  si32 simple_num;
  si32 isimple, icomplex, jcomplex;
  si32 ntracks1, ntracks2, n_simple_tracks;
  si32 nentries;
  si32 start_scan, end_scan;
  si32 start_time, end_time;
  si32 *simples1, *simples2;
  complex_track_params_t ctparams1, ctparams2;

  if (Glob->params.debug >= DEBUG_EXTRA)
    fprintf(stderr, "\nConsolidating complex tracks %ld and %ld\n",
	    (long) lower_track_num, (long) higher_track_num);

  /*
   * check that first track is older
   */

  if (lower_track_num >= higher_track_num) {

    fprintf(stderr, "ERROR - %s:consolidate_complex_tracks.\n",
	    Glob->prog_name);
    fprintf(stderr, "Tracks given in wrong order.\n");
    fprintf(stderr, "First track num %ld\n", (long) lower_track_num);
    fprintf(stderr, "Second track num %ld\n", (long) higher_track_num);
    tidy_and_exit(-1);
    
  }

  /*
   * read in the two track structs
   */

  if (RfReadComplexTrackParams(t_handle, lower_track_num, FALSE,
			       "consolidate_complex_tracks"))
    tidy_and_exit(-1);

  ctparams1 = *t_handle->complex_params;
  ntracks1 = ctparams1.n_simple_tracks;

  if (RfReadComplexTrackParams(t_handle, higher_track_num, FALSE,
			       "consolidate_complex_tracks"))
    tidy_and_exit(-1);

  ctparams2 = *t_handle->complex_params;
  ntracks2 = ctparams2.n_simple_tracks;
  
  n_simple_tracks = ntracks1 + ntracks2;

  /*
   * realloc the simples_per_complex array
   */
  
  t_handle->simples_per_complex[lower_track_num] = (si32 *) urealloc
    ((char *) t_handle->simples_per_complex[lower_track_num],
     ((ntracks1 + ntracks2) * sizeof(si32)));
  
  /*
   * compute scan and time values for the consolidated track
   */

  if (ctparams1.start_scan < ctparams2.start_scan) {

    start_scan = ctparams1.start_scan;
    start_time = ctparams1.start_time;

  } else {

    start_scan = ctparams2.start_scan;
    start_time = ctparams2.start_time;

  } /* if (ctparams1.start_scan < ctparams2.start_scan) */

  if (ctparams1.end_scan > ctparams2.end_scan) {

    end_scan = ctparams1.end_scan;
    end_time = ctparams1.end_time;

  } else {

    end_scan = ctparams2.end_scan;
    end_time = ctparams2.end_time;

  } /* if (ctparams1.end_scan > ctparams2.end_scan) */

  /*
   * put consolidated values into track number 1
   */

  ctparams1.start_scan = start_scan;

  ctparams1.duration_in_scans = end_scan - start_scan + 1;
  
  ctparams1.start_time = start_time;
  ctparams1.end_time = end_time;

  if (ctparams1.duration_in_scans > 1) {
    ctparams1.duration_in_secs =
      (si32) ((end_time - start_time) * 86400.0
	      * (ctparams1.duration_in_scans /
		 (ctparams1.duration_in_scans - 1.0)) + 0.5);
  } else {
    ctparams1.duration_in_secs = 0;
  }
  
  ctparams1.n_simple_tracks = n_simple_tracks;
  t_handle->nsimples_per_complex[lower_track_num] =
    ctparams1.n_simple_tracks;

  track_utime[lower_track_num].start_complex = start_time;
  track_utime[lower_track_num].end_complex = end_time;
  
  /*
   * append to the simple track number list and sort it
   */

  simples1 = t_handle->simples_per_complex[lower_track_num];
  simples2 = t_handle->simples_per_complex[higher_track_num];
  
  for (isimple = 0; isimple < ntracks2; isimple++) {
    simples1[ntracks1 + isimple] = simples2[isimple];
  } /* isimple */
  
  qsort((char *) simples1, (int) (n_simple_tracks),
	sizeof(si32), compare_si32s);

  /*
   * write out amended complex track params
   */

  memcpy ((void *) t_handle->complex_params,
          (void *) &ctparams1,
          (size_t) sizeof(complex_track_params_t));
  
  if (RfWriteComplexTrackParams(t_handle, lower_track_num,
				"consolidate_complex_tracks"))
    tidy_and_exit(-1);
  
  /*
   * loop through the simple tracks which make up complex track 2
   * changing the complex track number to track 1
   */

  for (isimple = 0; isimple < ntracks2; isimple++) {

    simple_num = simples2[isimple];

    /*
     * read in simple params
     */

    if(RfRewindSimpleTrack(t_handle, simple_num,
			   "consolidate_complex_tracks"))
      tidy_and_exit(-1);
    
    /*
     * change complex track num
     */
    
    t_handle->simple_params->complex_track_num = lower_track_num;

    /*
     * write amended simple params
     */

    if (RfWriteSimpleTrackParams(t_handle, simple_num,
				 "consolidate_complex_tracks"))
      tidy_and_exit(-1);

    /*
     * amend track number in each track entry
     */

    nentries = t_handle->simple_params->duration_in_scans;
    
    for (ientry = 0; ientry < nentries; ientry++) {
      
      if (RfReadTrackEntry(t_handle, "consolidate_complex_tracks"))
	tidy_and_exit(-1);

      t_handle->entry->complex_track_num = lower_track_num;

      if (RfRewriteTrackEntry(t_handle, "consolidate_complex_tracks"))
	tidy_and_exit(-1);
      
    } /* ientry */
    
  } /* isimple */

  /*
   * search for the position of the higher complex track number in
   * the complex track num array
   */

  for (icomplex = 0;
       icomplex < t_handle->header->n_complex_tracks; icomplex++) {

    if (t_handle->complex_track_nums[icomplex] == higher_track_num)
      break;

  } /* icomplex */

  if (icomplex == t_handle->header->n_complex_tracks) {

    fprintf(stderr, "ERROR - %s:consolidate_complex_tracks.\n",
	    Glob->prog_name);
    fprintf(stderr, "Problem with complex_track_nums array.\n");
    fprintf(stderr,
	    "Complex track num %d not found.\n", higher_track_num);
    tidy_and_exit(-1);
    
  }

  /*
   * for all complex tracks with numbers greater than higher_track_num,
   * move their entries down in the complex_track_num array
   */

  for (jcomplex = icomplex;
       jcomplex < t_handle->header->n_complex_tracks - 1; jcomplex++) {

    t_handle->complex_track_nums[jcomplex] = 
      t_handle->complex_track_nums[jcomplex + 1];

  } /* jcomplex */

  t_handle->complex_track_nums[t_handle->header->n_complex_tracks - 1] = 0;
  
  /*
   * free up the complex params for the higher track num
   */
  
  if (RfReuseTrackComplexSlot(t_handle,
			      higher_track_num,
			      "consolidate_complex_tracks")) {
    tidy_and_exit(-1);
  }
  
  /*
   * free up simples_per_complex for higher track num
   */

  ufree((char *) t_handle->simples_per_complex[higher_track_num]);
  t_handle->simples_per_complex[higher_track_num] = NULL;
      
  /*
   * reduce number of complex tracks by 1
   */

  t_handle->header->n_complex_tracks--;

  /*
   * write header
   */

  if (RfWriteTrackHeader(t_handle, "consolidate_complex_tracks"))
    tidy_and_exit(-1);

  /*
   * update storms entries for complex track number
   */

  for (istorm = 0; istorm < nstorms1; istorm++) {
    if (storms1[istorm].track->complex_track_num == higher_track_num) {
      storms1[istorm].track->complex_track_num = lower_track_num;
      storms1[istorm].track->n_simple_tracks = n_simple_tracks;
    }
  }

}

/*****************************************************************************
 * define function to be used for sorting (lowest to highest)
 */

static int compare_si32s(const void *v1, const void *v2)


{
    si32 *l1, *l2;

    l1 = (si32 *) v1;
    l2 = (si32 *) v2;

    return (*l1 - *l2);

}

/*************************
 * process_storms1_entry()
 *
 * Process storms1 entry for complex nums
 */

static void process_storms1_entry(storm_status_t *storms1,
				  storm_status_t *storms2,
				  si32 nstorms1,
				  si32 nstorms2,
				  si32 storm1_num,
				  si32 storm2_num)

{

  int already_in_list;
  si32 i, istorm;
  storm_status_t *storm1;
  track_match_t *match;

  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr,
	    "PROCESS_STORMS1_ENTRY: storm1_num, storm2_num: %d, %d\n",
	    (int) storm1_num, (int) storm2_num);
  }
    
  storm1 = storms1 + storm1_num;

  already_in_list = FALSE;
  for (i = 0; i < N_complex_nums; i++) {
    if (Complex_nums[i] == storm1->track->complex_track_num) {
      already_in_list = TRUE;
      break;
    }
  } /* i */

  if (!already_in_list) {
    alloc_complex_nums(N_complex_nums + 1);
    Complex_nums[N_complex_nums] = storm1->track->complex_track_num;
    Storm1_nums[N_complex_nums] = storm1_num;
    N_complex_nums += 1;
  }

  match = storm1->match_array;
  for (istorm = 0; istorm < storm1->n_match; istorm++, match++) {
    if (match->storm_num != storm2_num) {
      process_storms2_entry(storms1, storms2, nstorms1, nstorms2,
			    storm1_num, match->storm_num);
    }
  } /* istorm */

  return;

}

/*************************
 * process_storms2_entry()
 *
 * Process storms2 entry for complex nums
 */

static void process_storms2_entry(storm_status_t *storms1,
				  storm_status_t *storms2,
				  si32 nstorms1,
				  si32 nstorms2,
				  si32 storm1_num,
				  si32 storm2_num)

{

  si32 jstorm;
  storm_status_t *storm2;
  track_match_t *match;
  
  if (Glob->params.debug >= DEBUG_EXTRA) {
    fprintf(stderr,
	    "PROCESS_STORMS2_ENTRY: storm1_num, storm2_num: %d, %d\n",
	    (int) storm1_num, (int) storm2_num);
  }
	  
  storm2 = storms2 + storm2_num;

  if (!storm2->checked) {

    storm2->checked = TRUE;

    match = storm2->match_array;
    for (jstorm = 0; jstorm < storm2->n_match; jstorm++, match++) {
      if (match->storm_num != storm1_num) {
	process_storms1_entry(storms1, storms2, nstorms1, nstorms2,
			      match->storm_num, storm2_num);
      }
    } /* jstorm */

  } /* if (!storm2->checked) */

  return;

}

