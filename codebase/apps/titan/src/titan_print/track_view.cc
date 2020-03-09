// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*********************************************************************
 * track_view.c
 *
 * prints info about a track file
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include <math.h>
#include "titan_print.h"

#define BOOL_STR(a) (a == FALSE ? "false" : "true")

static void print_full(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle);

static void print_summary(storm_file_handle_t *s_handle,
			  track_file_handle_t *t_handle);


void track_view(void)

{

  char storm_file_path[MAX_PATH_LEN];
  
  track_file_handle_t t_handle;
  storm_file_handle_t s_handle;
  path_parts_t track_path_parts;
  
  /*
   * initialize
   */

  RfInitTrackFileHandle(&t_handle, Glob->prog_name);

  /*
   * open track properties files
   */
  
  if (RfOpenTrackFiles (&t_handle, "r",
			Glob->file_name,
			(char *) NULL,
			"track_view")) {
    
    fprintf(stderr, "ERROR - %s:track_view\n", Glob->prog_name);
    fprintf(stderr, "Opening track file '%s'.\n", Glob->file_name);
    exit(-1);

  }

  /*
   * read in track properties file header
   */
  
  if (RfReadTrackHeader(&t_handle, "track_view") != R_SUCCESS)
    exit(-1);
  
  if (RfReadSimplesPerComplex(&t_handle, "track_view") != R_SUCCESS)
    exit(-1);
  
  /*
   * print out header
   */
  
  printf("STORM TRACK FILE\n");
  printf("================\n");
  printf("\n");
  
  printf("Header file label : %s\n\n", t_handle.header_file_label);
  
  RfPrintTrackHeader(stdout, "  ",
		     t_handle.header);

  if (Glob->full) {
    RfPrintTrackHeaderArrays(stdout, "  ",
			     t_handle.header,
			     t_handle.complex_track_nums,
			     t_handle.complex_track_offsets,
			     t_handle.simple_track_offsets,
			     t_handle.nsimples_per_complex,
			     t_handle.simples_per_complex_offsets,
			     (const si32 **) t_handle.simples_per_complex,
			     t_handle.scan_index);
    
  }

  /*
   * open storm file
   */

  uparse_path(Glob->file_name, &track_path_parts);
  
  sprintf(storm_file_path, "%s%s",
	  track_path_parts.dir,
	  t_handle.header->storm_header_file_name);
  
  RfInitStormFileHandle(&s_handle, Glob->prog_name);
  
  /*
   * open storm properties files
   */
  
  if (RfOpenStormFiles (&s_handle, "r",
			storm_file_path,
			(char *) NULL,
			"track_view")) {
    
    fprintf(stderr, "ERROR - %s:track_view\n", Glob->prog_name);
    fprintf(stderr, "Opening storm file.\n");
    exit(-1);
    
  }
  
  /*
   * read in storm properties file header
   */
  
  if (RfReadStormHeader(&s_handle, "storm_view") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:track_view\n", Glob->prog_name);
    fprintf(stderr, "Reading file %s.\n", storm_file_path);
    exit(-1);
  }
  
  /*
   * if full listing, print out track info
   */
  
  if (Glob->full) {
    print_full(&s_handle, &t_handle);
  } else if (Glob->summary) {
    print_summary(&s_handle, &t_handle);
  }
    
  /*
   * close files
   */

  RfCloseStormFiles(&s_handle, "track_view");
  RfCloseTrackFiles(&t_handle, "track_view");
  
}

/*------------------------------------------------
 */

static void print_full(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle)

{

  long isimple, icomplex, ientry;
  si32 simple_track_num;
  si32 complex_track_num;
  track_file_params_t *tparams;
  track_file_entry_t *entry;
  track_file_verify_t *verify;
  storm_file_params_t *sparams;
  storm_file_scan_header_t *scan;
  
  sparams = &s_handle->header->params;
  tparams = &t_handle->header->params;
  verify = &t_handle->header->verify;

  /*
   * complex tracks
   */
  
  for (icomplex = 0;
       icomplex < t_handle->header->n_complex_tracks; icomplex++) {
  
    complex_track_num = t_handle->complex_track_nums[icomplex];
    
    if (Glob->track_num >= 0 &&
	Glob->track_num != complex_track_num) {
      continue;
    }
    
    if(RfReadComplexTrackParams(t_handle, complex_track_num, TRUE,
				"track_view") != R_SUCCESS) {
      exit(-1);
    }
    
    RfPrintComplexTrackParams(stdout, "    ",
			      verify->verification_performed,
			      tparams,
			      t_handle->complex_params,
			      t_handle->simples_per_complex[complex_track_num]);
    
    /*
     * simple tracks in this complex track
     */
    
    for (isimple = 0;
	 isimple < t_handle->complex_params->n_simple_tracks; isimple++) {
      
      simple_track_num =
	t_handle->simples_per_complex[complex_track_num][isimple];
      
      if(RfRewindSimpleTrack(t_handle, simple_track_num,
			     "track_view") != R_SUCCESS)
	exit(-1);
      
      RfPrintSimpleTrackParams(stdout, "      ",
			       t_handle->simple_params);
      
      /*
       * loop through the track entries
       */
      
      for (ientry = 0;
	   ientry < t_handle->simple_params->duration_in_scans; ientry++) {
	
	if (RfReadTrackEntry(t_handle, "track_view") != R_SUCCESS)
	  exit(-1);
	
	entry = t_handle->entry;
	
	/*
	 * check that simple and complex track numbers are
	 * correct
	 */
	
	if (entry->complex_track_num !=
	    t_handle->complex_params->complex_track_num) {
	  fprintf(stderr, "\aERROR: complex track num "
		  "incorrect in entry\n");
	  fprintf(stderr, "complex_params->complex_track_num : %ld\n",
		  (long) t_handle->complex_params->complex_track_num);
	  fprintf(stderr, "entry->complex_track_num : %ld\n",
		  (long) entry->complex_track_num);
	}
	
	if (entry->simple_track_num !=
	    t_handle->simple_params->simple_track_num) {
	  fprintf(stderr, "\aERROR: simple track num "
		  "incorrect in entry\n");
	  fprintf(stderr, "simple_params->simple_track_num : %ld\n",
		  (long) t_handle->simple_params->simple_track_num);
	  fprintf(stderr, "entry->simple_track_num : %ld\n",
		  (long) entry->simple_track_num);
	}
	
	RfPrintTrackEntry(stdout, "        ",
			  ientry, tparams, entry);
	
	if (Glob->track_num >= 0) {
	  
	  /*
	   * read in storm props
	   */
	  
	  if (RfReadStormScan(s_handle, entry->scan_num,
			      "track_view") != R_SUCCESS) {
	    exit(-1);
	  }
	  
	  scan = s_handle->scan;
	  
	  /*
	   * print out s_handle->scan info
	   */
	  
	  RfPrintStormScan(stdout, "    ", sparams, scan);
	  
	  if (RfReadStormProps(s_handle, entry->storm_num,
			       "print_track") != R_SUCCESS) {
	    exit(-1);
	  }
	  
	  RfPrintStormProps(stdout, "      ", sparams,
			    s_handle->scan,
			    s_handle->gprops + entry->storm_num);
	  
	  RfPrintStormLayer(stdout, "      ", sparams,
			    s_handle->scan,
			    s_handle->gprops + entry->storm_num,
			    s_handle->layer);
	  
	  RfPrintStormHist(stdout, "      ", sparams,
			   s_handle->gprops + entry->storm_num,
			   s_handle->hist);
	  
	  
	  RfPrintStormRuns(stdout, "      ",
			   s_handle->gprops + entry->storm_num,
			   s_handle->runs);
	  
	  
	} /* if (Glob->track_num >= 0) */
	
      } /* ientry */
      
    } /* isimple */
    
    printf("\n");
    
  } /* icomplex */

}

/*---------------------------------------------------
 */
  
static void print_summary(storm_file_handle_t *s_handle,
			  track_file_handle_t *t_handle)

{

  long istorm, icomplex, isimple, ientry;
  si32 simple_track_num;
  si32 complex_track_num;
  track_file_verify_t *verify;
  // storm_file_params_t *sparams;
  storm_file_global_props_t *gprops;
  // track_file_params_t *tparams;
  complex_track_params_t *ct_params;
  simple_track_params_t *st_params;
  // track_file_entry_t *entry;

  // sparams = &s_handle->header->params;
  // tparams = &t_handle->header->params;

  /*
   * complex tracks
   */
  
  if (t_handle->header->n_complex_tracks > 0)
    printf("COMPLEX TRACKS\n\n");
  
  verify = &t_handle->header->verify;

  for (icomplex = 0; icomplex < t_handle->header->n_complex_tracks;
       icomplex++) {
    
    complex_track_num = t_handle->complex_track_nums[icomplex];
    if (Glob->track_num >= 0 &&
	Glob->track_num != complex_track_num) {
      continue;
    }
    
    if(RfReadComplexTrackParams(t_handle, complex_track_num, TRUE,
				"print_tracks") != R_SUCCESS)
      exit(-1);
    
    ct_params = t_handle->complex_params;

    /*
     * continue to next track if this one is too short
     */

    if (ct_params->duration_in_secs < Glob->min_duration)
      continue;
    
    RfPrintComplexTrackParams(stdout, "    ",
			      verify->verification_performed,
			      &t_handle->header->params,
			      t_handle->complex_params,
			      t_handle->simples_per_complex[complex_track_num]);

    
    /*
     * loop through simple tracks in this complex track
     */
  
    for (isimple = 0;
	 isimple < ct_params->n_simple_tracks; isimple++) {
    
      complex_track_num =
	ct_params->complex_track_num;
      
      simple_track_num =
	t_handle->simples_per_complex[complex_track_num][isimple];
    
      if(RfRewindSimpleTrack(t_handle, simple_track_num,
			     "track_view") != R_SUCCESS)
	exit(-1);
    
      st_params = t_handle->simple_params;

      /*
       * loop through the track entries
       */
    
      printf("\nTRACK NUMBER %ld/%ld\n\n",
	     (long) complex_track_num, (long) simple_track_num);
      
      printf("%4s %10s %8s %7s %7s %7s %7s %7s %7s %7s %7s\n",
	     "scan", "date", "time", "refl-x", "refl-y", "refl-z", "delta-z",
	     "volume", "mass", "av area", "max dbz");
      
      printf("%4s %10s %8s %7s %7s %7s %7s %7s %7s %7s %7s\n",
	     " ", " ", " ", "(km)", "(km)", "(km)", "(km)",
	     "(km3)", "(ktons)", "(km2)", "(dbz)");
      
      for (ientry = 0;
	   ientry < st_params->duration_in_scans; ientry++) {
      
	if (RfReadTrackEntry(t_handle, "load_stats_grid") != R_SUCCESS)
	  exit(-1);
      

	if (RfReadStormScan(s_handle, t_handle->entry->scan_num,
			    "print_tracks") != R_SUCCESS) {
	  fprintf(stderr, "ERROR - %s:print_tracks\n", Glob->prog_name);
	  fprintf(stderr, "Reading storm scan\n");
	  exit(-1);
	}
	
	istorm = t_handle->entry->storm_num;
	gprops = s_handle->gprops + istorm;
	
	/*
	 * print out storm data
	 */
	
	printf(
	       "%4d %s "
	       "%7.1f %7.1f %7.1f %7.1f %7.1f"
	       "%7.1f %7.1f %7.1f\n", 
	       s_handle->scan->scan_num,
	       utimstr(s_handle->scan->time),
	       gprops->refl_centroid_x,
	       gprops->refl_centroid_y,
	       gprops->refl_centroid_z,
	       gprops->refl_centroid_z - gprops->vol_centroid_z,
	       gprops->volume,
	       gprops->mass,
	       gprops->area_mean,
	       gprops->dbz_max);
	
      } /* ientry */

    } /* isimple */
	
  } /* icomplex */
  
  printf("\n");
  
}

