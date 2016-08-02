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
/*********************************************************************
 * write_basic_data.c
 *
 * Routines to write basic track data set to client
 *
 * RAP, NCAR, Boulder CO
 *
 * May 1993
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <math.h>
#include "track_server.h"
#include <toolsa/xdru.h>

static void load_entry(tdata_basic_header_t *header,
		       tdata_basic_track_entry_t *basic_entry,
		       track_file_entry_t *file_entry,
		       storm_file_params_t *sparams,
		       track_file_params_t *tparams,
		       storm_file_scan_header_t *scan,
		       storm_file_global_props_t *gprops,
		       int runs_included);

static void load_header(tdata_basic_header_t *header,
			storm_file_handle_t *s_handle,
			si32 dtime,
			si32 n_current_tracks,
			si32 n_entries,
			int runs_included);

/*********************************************************************
 * write_basic_with_params()
 *
 * Writes basic track data set to client, along with the complex
 * and simple params
 *
 * The data is placed in the packet buffer, which is sent whenever
 * it grows so large that adding another struct would make it
 * overflow.
 *
 * returns 0 on success, -1 on failure
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

int write_basic_with_params(tdata_request_t *request,
			    int sockfd,
			    storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle,
			    si32 dtime,
			    si32 n_current_tracks,
			    si32 *track_set)

{
  
  char *run_buffer;

  int entry_found;

  si32 icomplex, isimple, ientry;
  si32 nbytes;
  si32 nbytes_runs;
  si32 start_entry, end_entry, n_entries;
  si32 start_time, end_time, entry_time;
  si32 duration_before_request;
  si32 duration_after_request;
  si32 complex_num, simple_num;
  si32 *simples_per_complex;

  tdata_basic_header_t header, tmp_header;
  tdata_basic_complex_params_t basic_ct_params;
  tdata_basic_simple_params_t basic_st_params;
  tdata_basic_track_entry_t basic_entry;
  
  complex_track_params_t *ct_params;
  simple_track_params_t *st_params;
  storm_file_params_t *sparams;
  storm_file_scan_header_t *scan;
  storm_file_global_props_t *gprops;
  storm_file_run_t *runs;
  track_file_params_t *tparams;
  track_file_entry_t *file_entry;

  sparams = &s_handle->header->params;
  tparams = &t_handle->header->params;

  /*
   * data header
   */

  load_header(&header,
	      s_handle,
	      dtime,
	      n_current_tracks,
	      0L,
	      request->runs_included);
  
  tmp_header = header;
  BE_from_array_32((ui32 *) &tmp_header,
		   (ui32) sizeof(tdata_basic_header_t));
  
  if (write_to_buffer(sockfd, (char *) &tmp_header,
		      (si32) sizeof(tdata_basic_header_t),
		      TDATA_BASIC_HEADER_ID))
    return(-1);
  
  /*
   * loop through the complex tracks
   */
  
  for (icomplex = 0; icomplex < n_current_tracks; icomplex++) {
    
    if(RfReadComplexTrackParams(t_handle, track_set[icomplex], TRUE,
				"write_basic_with_params") != R_SUCCESS) {
      return (-1);
    }
    
    ct_params = t_handle->complex_params;
    complex_num = ct_params->complex_track_num;
    
    basic_ct_params.complex_track_num = complex_num;
    basic_ct_params.n_simple_tracks = ct_params->n_simple_tracks;
    
    basic_ct_params.start_time = ct_params->start_time;
    basic_ct_params.end_time = ct_params->end_time;
    
    basic_ct_params.start_scan = ct_params->start_scan;
    basic_ct_params.end_scan = ct_params->end_scan;
    
    basic_ct_params.duration_in_scans = ct_params->duration_in_scans;
    basic_ct_params.duration_in_secs = ct_params->duration_in_secs;
    basic_ct_params.n_top_missing = ct_params->n_top_missing;
    basic_ct_params.n_range_limited = ct_params->n_range_limited;
    basic_ct_params.start_missing = ct_params->start_missing;
    basic_ct_params.end_missing = ct_params->end_missing;
    basic_ct_params.volume_at_start_of_sampling =
      ct_params->volume_at_start_of_sampling;
    basic_ct_params.volume_at_end_of_sampling =
      ct_params->volume_at_end_of_sampling;
    
    BE_from_array_32((ui32 *) &basic_ct_params,
		     (ui32) sizeof(tdata_basic_complex_params_t));
    
    if (write_to_buffer(sockfd, (char *) &basic_ct_params,
			(si32) sizeof(tdata_basic_complex_params_t),
			TDATA_BASIC_COMPLEX_PARAMS_ID)) {
      return(-1);
    }

    /*
     * write simples_per_complex
     */

    nbytes = ct_params->n_simple_tracks * sizeof(si32);
    simples_per_complex = (si32 *) umalloc(nbytes);

    memcpy ((void *) simples_per_complex,
            (void *) t_handle->simples_per_complex[complex_num], nbytes);
    
    BE_from_array_32((ui32 *) simples_per_complex, nbytes);

    if (write_to_buffer(sockfd, (char *) simples_per_complex, nbytes,
			TDATA_BASIC_SIMPLES_PER_COMPLEX_ID)) {
      ufree((char *) simples_per_complex);
      return(-1);
    }

    ufree((char *) simples_per_complex);

    /*
     * loop through simple tracks
     */
    
    for (isimple = 0;
	 isimple < ct_params->n_simple_tracks; isimple++) {

      simple_num = t_handle->simples_per_complex[complex_num][isimple];
      
      /*
       * determine which entries are to be sent
       */
      
      if(RfRewindSimpleTrack(t_handle, simple_num,
			     "write_basic_with_params") != R_SUCCESS)
	return (-1);
      
      st_params = t_handle->simple_params;
      
      if (request->target_entries == TDATA_CURRENT_ENTRIES_ONLY ||
	  request->target_entries == TDATA_ENTRIES_IN_TIME_WINDOW) {

	if (request->target_entries == TDATA_CURRENT_ENTRIES_ONLY) {

	  /*
	   * nominal margin (secs) around the request time
	   */

	  duration_before_request = 10;
	  duration_after_request = 10;

	} else {

	  /*
	   * requested margin (secs) around the request time
	   */

	  duration_before_request = request->duration_before_request;
	  duration_after_request = request->duration_after_request;

	} /* if (request->target_entries .... */
	
	start_time = dtime - duration_before_request;
	end_time = dtime + duration_after_request;

	entry_found = FALSE;
	
	for (ientry = 0;
	     ientry < st_params->duration_in_scans; ientry++) {
	  
	  if (RfReadTrackEntry(t_handle,
			       "write_basic_with_params") != R_SUCCESS) {
	    return (-1);
	  }
	  
	  file_entry = t_handle->entry;
	  entry_time = file_entry->time;

	  if (entry_time >= start_time &&
	      entry_time <= end_time) {
	    if (!entry_found)
	      start_entry = ientry;
	    end_entry = ientry;
	    entry_found = TRUE;
	  } /* if (entry->time >= start_time ... */

	  if (entry_time >= end_time)
	    break;

	} /* ientry */

	if (entry_found) {

	  n_entries = end_entry - start_entry + 1;

	} else {
	  
	  start_entry = 0;
	  end_entry = -1;
	  n_entries = 0;

	} /* if (entry_found) */
	
      } else {

	start_entry = 0;
	end_entry = st_params->duration_in_scans - 1;
	n_entries = st_params->duration_in_scans;

      } /* if (request->target_entries ... */

      basic_st_params.simple_track_num = st_params->simple_track_num;
      basic_st_params.complex_track_num = st_params->complex_track_num;
      basic_st_params.start_time = st_params->start_time;
      basic_st_params.end_time = st_params->end_time;
      basic_st_params.start_scan = st_params->start_scan;
      basic_st_params.end_scan = st_params->end_scan;
      
      basic_st_params.duration_in_scans = n_entries;
      basic_st_params.duration_in_secs =
	st_params->duration_in_secs;
      
      basic_st_params.nparents = st_params->nparents;
      basic_st_params.nchildren = st_params->nchildren;
      
      memcpy ((void *) basic_st_params.parent,
              (void *)  st_params->parent,
              (size_t) (MAX_PARENTS * sizeof(si32)));
      
      memcpy ((void *) basic_st_params.child,
              (void *)  st_params->child,
              (size_t) (MAX_CHILDREN * sizeof(si32)));
      
      BE_from_array_32((ui32 *) &basic_st_params,
		       (ui32) sizeof(tdata_basic_simple_params_t));
      
      if (write_to_buffer(sockfd, (char *) &basic_st_params,
			  (si32) sizeof(tdata_basic_simple_params_t),
			  TDATA_BASIC_SIMPLE_PARAMS_ID))
	return(-1);

      /*
       * position at start_entry
       */
      
      if(RfRewindSimpleTrack(t_handle, simple_num,
			     "write_basic_with_params") != R_SUCCESS)
	return (-1);
      
      for (ientry = 0; ientry < start_entry; ientry++)
	if (RfReadTrackEntry(t_handle,
			     "write_basic_with_params") != R_SUCCESS)
	  return (-1);

      /*
       * loop through the track entries
       */
      
      for (ientry = start_entry; ientry <= end_entry; ientry++) {
	
	if (RfReadTrackEntry(t_handle,
			     "write_basic_with_params") != R_SUCCESS)
	  return (-1);
	
	file_entry = t_handle->entry;
	
	/*
	 * storm file scan header
	 */
	
	if (RfReadStormScan(s_handle,
			    file_entry->scan_num,
			    "write_basic_with_params") != R_SUCCESS)
	  exit(-1);
	
	scan = s_handle->scan;
	
	/*
	 * read in storm props
	 */

	if (request->runs_included)
	  if (RfReadStormProps(s_handle,
			       file_entry->storm_num,
			       "write_basic_with_params") != R_SUCCESS)
	    return(-1);
	
	gprops = s_handle->gprops + file_entry->storm_num;
	
	/*
	 * load up basic entry struct
	 */
	
	load_entry(&header,
		   &basic_entry,
		   file_entry,
		   sparams, tparams,
		   scan,
		   gprops,
		   request->runs_included);
    
	BE_from_array_32((ui32 *) &basic_entry,
			 (ui32) sizeof(tdata_basic_track_entry_t));
	
	if (write_to_buffer(sockfd, (char *) &basic_entry,
			    (si32) sizeof(tdata_basic_track_entry_t),
			    TDATA_BASIC_TRACK_ENTRY_ID))
	  return(-1);

	/*
	 * if requested, write storms runs
	 */
	
	if (request->runs_included) {

	  nbytes_runs = gprops->n_runs * sizeof(storm_file_run_t);
	  runs = s_handle->runs;
  
	  run_buffer = (char *) umalloc ((ui32) nbytes_runs);

	  memcpy ((void *) run_buffer,
		  (void *) runs,
		  (size_t) nbytes_runs);

	  BE_from_array_16((ui16 *) run_buffer,
			   (ui32) nbytes_runs);
	  
	  if (write_to_buffer(sockfd, run_buffer,
			      (si32) nbytes_runs,
			      TDATA_RUNS_ID)) {
	    ufree((char *) run_buffer);
	    return(-1);
	  }

	  ufree((char *) run_buffer);

	} /* if (request->runs_included) */

      } /* ientry */
      
    } /* isimple */
    
  } /* icomplex */
  
  /*
   * make sure all of the data in the buffer has been
   * sent to the client
   */
  
  if (flush_write_buffer(sockfd))
    return (-1);
  
  return (0);
  
}

/*********************************************************************
 * write_basic_without_params()
 *
 * Writes basic track data set to client
 *
 * The data is placed in the packet buffer, which is sent whenever
 * it grows so large that adding another struct would make it
 * overflow.
 *
 * returns 0 on success, -1 on failure
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

int write_basic_without_params(tdata_request_t *request,
			       int sockfd,
			       storm_file_handle_t *s_handle,
			       track_file_handle_t *t_handle,
			       si32 dtime)
     
{

  int runs_included;
  
  si32 n_entries, n_scans;
  si32 scan_num;
  si32 iscan, ientry;
  
  double time_diff, min_time_diff;
  
  tdata_basic_header_t header, tmp_header;
  tdata_basic_track_entry_t basic_entry;
  
  track_file_entry_t *file_entry;
  storm_file_params_t *sparams;
  storm_file_scan_header_t *scan;
  storm_file_global_props_t *gprops;
  track_file_params_t *tparams;

  sparams = &s_handle->header->params;
  tparams = &t_handle->header->params;

  /*
   * determine which scan is closest to the request time
   */

  n_scans = s_handle->header->n_scans;

  if (request->source == TDATA_LATEST ||
      request->source == TDATA_REALTIME) {

    scan_num = n_scans - 1;

  } else {
    
    min_time_diff = LARGE_DOUBLE;
  
    for (iscan = 0; iscan < n_scans; iscan++) {

      time_diff = fabs((double) dtime -
		       (double) t_handle->scan_index[iscan].utime);

      if (time_diff > min_time_diff)
	break;

      min_time_diff = time_diff;
      scan_num = iscan;

    } /* i */

  } /* if (request->source == TDATA_LATEST || ...*/

  /*
   * read in track file scan entries
   */

  if (RfReadTrackScanEntries(t_handle,
			     scan_num,
			     "write_basic_data") != R_SUCCESS)
    return (-1);
  
  /*
   * read in storm file scan
   */

  if (RfReadStormScan(s_handle,
		      scan_num,
		      "write_basic_data") != R_SUCCESS)
    return (-1);
  
  scan = s_handle->scan;
	
  /*
   * set data header
   */

  n_entries = t_handle->scan_index[scan_num].n_entries,
  runs_included = FALSE;
  
  load_header(&header,
	      s_handle,
	      dtime,
	      n_entries,
	      n_entries,
	      runs_included);
  
  /*
   * write data header
   */

  tmp_header = header;
  BE_from_array_32((ui32 *) &tmp_header,
		   (ui32) sizeof(tdata_basic_header_t));
  
  if (write_to_buffer(sockfd, (char *) &tmp_header,
		      (si32) sizeof(tdata_basic_header_t),
		      TDATA_BASIC_HEADER_ID))
    return(-1);

  /*
   * loop through the entries
   */
  
  file_entry = t_handle->scan_entries;

  for (ientry = 0; ientry < n_entries; ientry++) {
    
    gprops = s_handle->gprops + file_entry->storm_num;

    /*
     * load up basic entry struct
     */
    
    load_entry(&header,
	       &basic_entry,
	       file_entry,
	       sparams, tparams,
	       scan,
	       gprops,
	       runs_included);
    
    BE_from_array_32((ui32 *) &basic_entry,
		     (ui32) sizeof(tdata_basic_track_entry_t));
    
    if (write_to_buffer(sockfd, (char *) &basic_entry,
			(si32) sizeof(tdata_basic_track_entry_t),
			TDATA_BASIC_TRACK_ENTRY_ID))
      return(-1);

    file_entry++;
    
  } /* ientry */
  
  /*
   * make sure all of the data in the buffer has been
   * sent to the client
   */
  
  if (flush_write_buffer(sockfd))
    return (-1);

  return (0);
  
}

/*******************************************************************
 * load_entry()
 *
 * loads up basic entry struct
 */

static void load_entry(tdata_basic_header_t *header,
		       tdata_basic_track_entry_t *basic_entry,
		       track_file_entry_t *file_entry,
		       storm_file_params_t *sparams,
		       track_file_params_t *tparams,
		       storm_file_scan_header_t *scan,
		       storm_file_global_props_t *gprops,
		       int runs_included)

{
  
  int i;
  track_file_forecast_props_t *fprops;
  
  fprops = &file_entry->dval_dt;

  basic_entry->scan_num = file_entry->scan_num;
  basic_entry->simple_track_num = file_entry->simple_track_num;
  basic_entry->complex_track_num = file_entry->complex_track_num;
  
  basic_entry->time = file_entry->time;
  basic_entry->history_in_scans = file_entry->history_in_scans;
  basic_entry->history_in_secs = file_entry->history_in_secs;
  
  basic_entry->forecast_valid = file_entry->forecast_valid;

  basic_entry->vol_centroid_x = gprops->vol_centroid_x;
  basic_entry->vol_centroid_y = gprops->vol_centroid_y;
  basic_entry->refl_centroid_x = gprops->refl_centroid_x;
  basic_entry->refl_centroid_y = gprops->refl_centroid_y;
  basic_entry->vol_centroid_z = gprops->vol_centroid_z;
  basic_entry->refl_centroid_z = gprops->refl_centroid_z;

  basic_entry->top = gprops->top;
  basic_entry->base = gprops->base;
  basic_entry->volume = gprops->volume;
  basic_entry->area_mean = gprops->area_mean;
  basic_entry->precip_flux = gprops->precip_flux;
  basic_entry->mass = gprops->mass;
  basic_entry->vil_from_maxz = gprops->vil_from_maxz;

  basic_entry->tilt_angle = gprops->tilt_angle;
  basic_entry->tilt_dirn = gprops->tilt_dirn;

  basic_entry->dbz_max = gprops->dbz_max;
  basic_entry->dbz_mean = gprops->dbz_mean;
  basic_entry->dbz_max_gradient = gprops->dbz_max_gradient;
  basic_entry->dbz_mean_gradient = gprops->dbz_mean_gradient;
  basic_entry->ht_of_dbz_max = gprops->ht_of_dbz_max;

  basic_entry->proj_area = gprops->proj_area;
  basic_entry->proj_area_orientation = gprops->proj_area_orientation;
  basic_entry->proj_area_centroid_x = gprops->proj_area_centroid_x;
  basic_entry->proj_area_centroid_y = gprops->proj_area_centroid_y;
  basic_entry->proj_area_minor_radius = gprops->proj_area_minor_radius;
  basic_entry->proj_area_major_radius = gprops->proj_area_major_radius;
  
  for (i = 0; i < N_POLY_SIDES; i++) {
    basic_entry->proj_area_polygon[i]= gprops->proj_area_polygon[i];
  } /* i */
  
  if (runs_included) {
    basic_entry->n_runs = gprops->n_runs;
  } else {
    basic_entry->n_runs = 0;
  }
  
  basic_entry->proj_area_centroid_dx_dt =
    fprops->smoothed_proj_area_centroid_x;
  basic_entry->proj_area_centroid_dy_dt =
    fprops->smoothed_proj_area_centroid_y;
  basic_entry->dvolume_dt = fprops->volume;
  basic_entry->dproj_area_dt = fprops->proj_area;
  basic_entry->speed = fprops->smoothed_speed;
  basic_entry->speed_knots = fprops->smoothed_speed / 1.852;
  basic_entry->direction = fprops->smoothed_direction;

  return;

}

/*******************************************************************
 * load_header()
 *
 * loads up basic header struct
 */

static void load_header(tdata_basic_header_t *header,
			storm_file_handle_t *s_handle,
			si32 dtime,
			si32 n_current_tracks,
			si32 n_entries,
			int runs_included)
     
{

  storm_file_params_t *sparams = &s_handle->header->params;

  header->grid = s_handle->scan->grid;
  
  header->dbz_threshold = sparams->low_dbz_threshold;
  header->min_storm_size = sparams->min_storm_size;
  header->n_poly_sides = sparams->n_poly_sides;
  header->poly_start_az = sparams->poly_start_az;
  header->poly_delta_az = sparams->poly_delta_az;

  header->n_complex_tracks = n_current_tracks;
  header->n_entries = n_entries;
  header->runs_included = runs_included;
  header->time = dtime;
  header->data_start_time = s_handle->header->start_time;
  header->data_end_time = s_handle->header->end_time;

}  

