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
 * write_complete_data.c
 *
 * Writes complete track data set to client
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

#include "track_server.h"
#include <toolsa/xdru.h>

/*ARGSUSED*/

int write_complete_data(int sockfd,
			storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			si32 dtime,
			si32 n_current_tracks,
			si32 *track_set)
     
{
  
  si32 icomplex, isimple, ientry, ilayer, interval;
  si32 nbytes;
  si32 complex_num, simple_num;
  si32 *simples_per_complex;
  
  tdata_complete_header_t header;
  complex_track_params_t local_ct_params, *ct_params;
  simple_track_params_t local_st_params, *st_params;
  track_file_entry_t local_entry, *entry;
  storm_file_scan_header_t local_scan, *scan;
  storm_file_global_props_t local_gprops, *gprops;
  storm_file_layer_props_t local_layer, *layer;
  storm_file_dbz_hist_t local_hist, *hist;
  
  /*
   * complete data header
   */
  
  header.sparams = s_handle->header->params;
  header.tparams = t_handle->header->params;
  header.n_complex_tracks = n_current_tracks;
  header.time = dtime;
  header.data_start_time = s_handle->header->start_time;
  header.data_end_time = s_handle->header->end_time;
  
  BE_from_array_32((ui32 *) &header,
		   (ui32) sizeof(tdata_complete_header_t));
  
  if (write_to_buffer(sockfd, (char *) &header,
		      (si32) sizeof(tdata_complete_header_t),
		      TDATA_COMPLETE_HEADER_ID))
    return(-1);
  
  /*
   * loop thourgh the complex tracks
   */
  
  for (icomplex = 0; icomplex < n_current_tracks; icomplex++) {
    
    if(RfReadComplexTrackParams(t_handle, track_set[icomplex], TRUE,
				"write_complete_data") != R_SUCCESS)
      return (-1);
    
    ct_params = t_handle->complex_params;
    complex_num = ct_params->complex_track_num;
    
    memcpy ((void *) &local_ct_params,
            (void *) ct_params,
            (size_t) sizeof(complex_track_params_t));
    
    BE_from_array_32((ui32 *) &local_ct_params,
		     (ui32) sizeof(complex_track_params_t));
    
    if (write_to_buffer(sockfd, (char *) &local_ct_params,
			(si32) sizeof(complex_track_params_t),
			TDATA_COMPLEX_TRACK_PARAMS_ID))
      return(-1);
    
    /*
     * write simples_per_complex
     */

    nbytes = ct_params->n_simple_tracks * sizeof(si32);
    simples_per_complex = (si32 *) umalloc(nbytes);

    memcpy ((void *) simples_per_complex,
            (void *) t_handle->simples_per_complex[complex_num], nbytes);
    
    BE_from_array_32((ui32 *) simples_per_complex, nbytes);

    if (write_to_buffer(sockfd, (char *) simples_per_complex, nbytes,
			TDATA_COMPLETE_SIMPLES_PER_COMPLEX_ID)) {
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
      
      if(RfRewindSimpleTrack(t_handle, simple_num,
			     "write_complete_data") != R_SUCCESS)
	return (-1);
      
      st_params = t_handle->simple_params;
      
      memcpy ((void *) &local_st_params,
              (void *) st_params,
              (size_t) sizeof(simple_track_params_t));
      
      BE_from_array_32((ui32 *) &local_st_params,
		       (ui32) sizeof(simple_track_params_t));
      
      if (write_to_buffer(sockfd, (char *) &local_st_params,
			  (si32) sizeof(simple_track_params_t),
			  TDATA_SIMPLE_TRACK_PARAMS_ID))
	return(-1);
      
      /*
       * loop through the track entries
       */
      
      for (ientry = 0;
	   ientry < st_params->duration_in_scans; ientry++) {
	
	if (RfReadTrackEntry(t_handle, "write_complete_data") != R_SUCCESS)
	  return (-1);
	
	entry = t_handle->entry;
	
	memcpy ((void *) &local_entry,
		(void *)  entry,
		(size_t) sizeof(track_file_entry_t));
	
	BE_from_array_32((ui32 *) &local_entry,
			 (ui32) sizeof(track_file_entry_t));
	
	if (write_to_buffer(sockfd, (char *) &local_entry,
			    (si32) sizeof(track_file_entry_t),
			    TDATA_TRACK_FILE_ENTRY_ID))
	  return(-1);
	
	/*
	 * storm file scan header
	 */
	
	if (RfReadStormScan(s_handle, entry->scan_num,
			    "write_complete_data") != R_SUCCESS)
	  exit(-1);
	
	scan = s_handle->scan;
	
	memcpy ((void *) &local_scan,
		(void *)  scan,
		(size_t) sizeof(storm_file_scan_header_t));
	
	BE_from_array_32((ui32 *) &local_scan,
			 (ui32) (sizeof(storm_file_scan_header_t) -
				 scan->nbytes_char));
	
	if (write_to_buffer(sockfd, (char *) &local_scan,
			    (si32) sizeof(storm_file_scan_header_t),
			    TDATA_STORM_FILE_SCAN_HEADER_ID))
	  return(-1);
	
	/*
	 * read in storm props
	 */
	
	if (RfReadStormProps(s_handle, entry->storm_num,
			     "write_complete_data") != R_SUCCESS)
	  return(-1);
	
	gprops = s_handle->gprops + entry->storm_num;
	
	memcpy ((void *) &local_gprops,
		(void *)  gprops,
		(size_t) sizeof(storm_file_global_props_t));
	
	BE_from_array_32((ui32 *) &local_gprops,
			 (ui32) (sizeof(storm_file_global_props_t)));
	
	if (write_to_buffer(sockfd, (char *) &local_gprops,
			    (si32) sizeof(storm_file_global_props_t),
			    TDATA_STORM_FILE_GLOBAL_PROPS_ID))
	  return(-1);
	
	/*
	 * layer properties
	 */
	
	layer = s_handle->layer;

	for (ilayer = gprops->base_layer;
	     ilayer < (gprops->base_layer + gprops->n_layers);
	     ilayer++, layer++) {
	  
	  memcpy ((void *) &local_layer,
		  (void *)  layer,
		  (size_t) sizeof(storm_file_layer_props_t));
	  
	  BE_from_array_32((ui32 *) &local_layer,
			   (ui32) (sizeof(storm_file_layer_props_t)));
	  
	  if (write_to_buffer(sockfd, (char *) &local_layer,
			      (si32) sizeof(storm_file_layer_props_t),
			      TDATA_STORM_FILE_LAYER_PROPS_ID))
	    return(-1);
	  
	} /* ilayer */
	
	/*
	 * dbz histograms
	 */
	
	for (interval = 0;
	     interval < gprops->n_dbz_intervals; interval++) {
	  
	  hist = s_handle->hist + interval;
	  
	  memcpy ((void *) &local_hist,
		  (void *)  hist,
		  (size_t) sizeof(storm_file_dbz_hist_t));
	  
	  BE_from_array_32((ui32 *) &local_hist,
			   (ui32) (sizeof(storm_file_dbz_hist_t)));
	  
	  if (write_to_buffer(sockfd, (char *) &local_hist,
			      (si32) sizeof(storm_file_dbz_hist_t),
			      TDATA_STORM_FILE_DBZ_HIST_ID))
	    return(-1);
	  
	} /* interval */
	
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

