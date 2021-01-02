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
 * process_storms.c
 *
 * Opens the files, reads in the headers
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 *******************************************************************************/

#include "tracks_to_ascii.h"

void process_storms(storm_file_handle_t *s_handle,
		    track_file_handle_t *t_handle,
		    si32 *n_complex,
		    si32 *n_simple,
		    autocorr_t *autocorr_volume,
		    autocorr_t *autocorr_vol_two_thirds,
		    autocorr_t *autocorr_precip_area,
		    autocorr_t *autocorr_proj_area)

{
  
  int track_required;
  si32 icomplex, isimple, ientry;
  si32 nentries;
  si32 complex_track_num;
  si32 simple_track_num;
  
  /*
   * loop through the complex tracks
   */
  
  for (icomplex = 0;
       icomplex < t_handle->header->n_complex_tracks; icomplex++) {
    
    /*
     * read in the complex track params
     */
   
    complex_track_num = t_handle->complex_track_nums[icomplex];
    if(RfReadComplexTrackParams(t_handle, complex_track_num,
				TRUE,
				"process_storms") != R_SUCCESS)
      tidy_and_exit(-1);
    
    /*
     * check if this track is required for analysis
     */
    
    if (t_handle->complex_params->n_simple_tracks == 1 &&
	Glob->use_simple_tracks) {
      track_required = TRUE;
    } else if (t_handle->complex_params->n_simple_tracks > 1 && 
	       Glob->use_complex_tracks) {
      track_required = TRUE;
    } else {
      track_required = FALSE;
    }
    
    if (t_handle->complex_params->duration_in_secs <
	Glob->min_duration) {
      
      if (Glob->debug)
	fprintf(stderr, "Track %ld rejected - duration = %ld scans\n",
		(long) t_handle->complex_params->complex_track_num,
		(long) t_handle->complex_params->duration_in_scans);
      
      track_required = FALSE;
      
    }
    
    if (t_handle->complex_params->n_top_missing >
	Glob->max_top_missing) {
      
      if (Glob->debug)
	fprintf(stderr, "Track %ld rejected - n_top_missing = %ld\n",
		(long) t_handle->complex_params->complex_track_num,
		(long) t_handle->complex_params->n_top_missing);
      
      track_required = FALSE;
      
    }
    
    if (t_handle->complex_params->n_range_limited >
	Glob->max_range_limited) {
      
      if (Glob->debug)
	fprintf(stderr, "Track %ld rejected - n_range_limited = %ld\n",
		(long) t_handle->complex_params->complex_track_num,
		(long) t_handle->complex_params->n_range_limited);
      
      track_required = FALSE;
      
    }
    
    if (t_handle->complex_params->volume_at_start_of_sampling >
	Glob->max_vol_at_start_of_sampling) {
      
      if (Glob->debug)
	fprintf(stderr,
		"Track %ld rejected - vol_at_start_of_sampling = %ld\n",
		(long) t_handle->complex_params->complex_track_num,
		(long) t_handle->complex_params->volume_at_start_of_sampling);
      
      track_required = FALSE;
      
    }
    
    if (t_handle->complex_params->volume_at_end_of_sampling >
	Glob->max_vol_at_end_of_sampling) {
      
      if (Glob->debug)
	fprintf(stderr,
		"Track %ld rejected - vol_at_end_of_sampling = %ld\n",
		(long) t_handle->complex_params->complex_track_num,
		(long) t_handle->complex_params->volume_at_end_of_sampling);
      
      track_required = FALSE;
      
    }

    if (track_required) {

      if (t_handle->complex_params->n_simple_tracks == 1)
	(*n_simple)++;
      else
	(*n_complex)++;

      /*
       * skip now if count only is required
       */

      if (Glob->storm_count_only)
	continue;
	
      if (Glob->target_entity == TRENDS) {
	
	trend_analysis(s_handle, t_handle);

      } else {
	
	if (Glob->target_entity == COMPLETE_TRACK)
	  initialize_track_comps(s_handle, t_handle);
	
	/*
	 * read in simple tracks
	 */
	
	for (isimple = 0;
	     isimple < t_handle->complex_params->n_simple_tracks; isimple++) {
	  
	  simple_track_num =
	    t_handle->simples_per_complex[complex_track_num][isimple];
	  
	  /*
	   * read in simple track params and prepare entries for reading
	   */
	  
	  if(RfRewindSimpleTrack(t_handle, simple_track_num,
				 "process_storms") != R_SUCCESS)
	    tidy_and_exit(-1);
	  
	  nentries = t_handle->simple_params->duration_in_scans;
	  
	  /*
	   * loop through the entries
	   */
	  
	  for (ientry = 0; ientry < nentries; ientry++) {
	    
	    /*
	     * read in track entry
	     */
	    
	    if (RfReadTrackEntry(t_handle, "process_storms") != R_SUCCESS)
	      tidy_and_exit(-1);
	    
	    if (RfReadStormScan(s_handle, t_handle->entry->scan_num,
				"process_storms") != R_SUCCESS)
	      tidy_and_exit(-1);
	    
	    if (Glob->target_entity == COMPLETE_TRACK) {
	      
	      update_track_comps(s_handle, t_handle);
	      
	    } else if (Glob->target_entity == TRACK_ENTRY) {
	      
	      entry_comps(s_handle, t_handle);
	      
	    }
	    
	  } /* ientry */
	  
	} /* isimple */
	
	if (Glob->target_entity == COMPLETE_TRACK)
	  finalize_track_comps(s_handle, t_handle,
			       autocorr_volume,
			       autocorr_vol_two_thirds,
			       autocorr_precip_area,
			       autocorr_proj_area);
	
      } /* if (Glob->target_entity ... */
      
    } /* if (track_required) */
    
  } /* icomplex */
  
}
