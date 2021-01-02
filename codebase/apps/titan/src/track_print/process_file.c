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
 * process_file.c
 *
 * Process the track file
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1996
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_print.h"

static void print_gprops(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 simple_track_params_t *st_params);

static void print_layers(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 simple_track_params_t *st_params);
     
static void print_simple(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 si32 complex_num,
			 si32 simple_num);

void process_file(char *file_path)
     
{

  static int first_call = TRUE;
  static track_file_handle_t t_handle;
  static storm_file_handle_t s_handle;

  char storm_file_path[MAX_PATH_LEN];
  path_parts_t track_path_parts;

  int i, j;
  int complex_found, simple_found;
  si32 complex_num;
  
  if (first_call) {

    /*
     * initialize
     */
    
    RfInitStormFileHandle(&s_handle, Glob->prog_name);

    RfInitTrackFileHandle(&t_handle, Glob->prog_name);

    first_call = FALSE;

  }

  fprintf(stdout, "Printing file %s\n", file_path);
  
  /*
   * open track properties files
   */
  
  if (RfOpenTrackFiles (&t_handle, "r",
			file_path,
			(char *) NULL,
			"process_file")) {
    
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "Opening track file '%s'.\n", file_path);
    tidy_and_exit(-1);
    
  }
  
  /*
   * read in track properties file header
   */
  
  if (RfReadTrackHeader(&t_handle, "process_file") != R_SUCCESS)
    tidy_and_exit(-1);
  
  if (RfReadSimplesPerComplex(&t_handle, "process_file") != R_SUCCESS)
    tidy_and_exit(-1);
  
  /*
   * open storm file
   */

  uparse_path(file_path, &track_path_parts);
  
  sprintf(storm_file_path, "%s%s",
	  track_path_parts.dir,
	  t_handle.header->storm_header_file_name);
  
  /*
   * open storm properties files
   */
  
  if (RfOpenStormFiles (&s_handle, "r",
			storm_file_path,
			(char *) NULL,
			"process_file")) {
    
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "Opening storm file.\n");
    tidy_and_exit(-1);
    
  }
  
  /*
   * read in storm properties file header
   */
  
  if (RfReadStormHeader(&s_handle, "storm_view") != R_SUCCESS) {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "Reading file %s.\n", storm_file_path);
    tidy_and_exit(-1);
  }
  
  if (Glob->print_complex) {

    /*
     * search for complex tracks
     */
    
    complex_found = FALSE;
    for (i = 0; i < t_handle.header->n_complex_tracks; i++) {
      if (t_handle.complex_track_nums[i] == Glob->complex_num) {
	complex_found = TRUE;
	break;
      }
    } /* i */
    
    if (complex_found) {

      if (Glob->print_simple) {

	/*
	 * search for simple track
	 */
	
	simple_found = FALSE;
	for (i = 0; i < t_handle.nsimples_per_complex[Glob->complex_num]; i++) {
	  if (Glob->simple_num ==
	      t_handle.simples_per_complex[Glob->complex_num][i]) {
	    simple_found = TRUE;
	    break;
	  }
	} /* i */
	
	/*
	 * print simple track if found
	 */
	
	if (simple_found) {
	  print_simple(&s_handle, &t_handle,
		       Glob->complex_num, Glob->simple_num);
	} else {
	  fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
	  fprintf(stderr, "Simple track num %ld not valid\n",
		  Glob->simple_num);
	}
	
      } else { /* if (Glob->print_simple) */
	
	/*
	 * print all simple tracks in this complex track
	 */
	
	for (i = 0; i < t_handle.nsimples_per_complex[Glob->complex_num]; i++) {
	  print_simple(&s_handle, &t_handle,
		       Glob->complex_num,
		       t_handle.simples_per_complex[Glob->complex_num][i]);
	}
	
      } /* if (Glob->print_simple) */
      
    } else {
      
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Complex track num %ld not valid\n",
	      Glob->complex_num);
    }
   
  } else if (Glob->print_simple) { /* if (Glob->print_complex) */

    /*
     * find complex num
     */
    
    simple_found = FALSE;
    for (i = 0; i < t_handle.header->n_complex_tracks; i++) {
      complex_num = t_handle.complex_track_nums[i];
      for (j = 0; j < t_handle.nsimples_per_complex[complex_num]; j++) {
	if (Glob->simple_num ==
	    t_handle.simples_per_complex[complex_num][j]) {
	  simple_found = TRUE;
	  break;
	}
      } /* j */
      if (simple_found) {
	break;
      }
    } /* i */
    
    /*
     * if found, print simple track
     */
    
    if (simple_found) {
      print_simple(&s_handle, &t_handle,
		   complex_num, Glob->simple_num);
    } else {
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Simple track num %ld not valid\n",
	      Glob->simple_num);
    }

  } /* if (Glob->print_complex) */
  
  /*
   * close files
   */

  RfCloseStormFiles(&s_handle, "process_file");
  RfCloseTrackFiles(&t_handle, "process_file");

  return;
  
}

/*---------------------------------------------------
 */

static void print_simple(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 si32 complex_num,
			 si32 simple_num)

{
  
  complex_track_params_t *ct_params;
  simple_track_params_t *st_params;

  /*
   * read in complex tracks
   */
  
  if(RfReadComplexTrackParams(t_handle, complex_num, TRUE,
			      "print_simple") != R_SUCCESS) {
    tidy_and_exit(-1);
  }
  ct_params = t_handle->complex_params;

  /*
   * continue to next track if this one is too short
   */
  
  if (ct_params->duration_in_secs < Glob->min_duration) {
    return;
  }
    
  /*
   * read in simple track params,
   * prepare the track entries for reading
   */
    
  if(RfRewindSimpleTrack(t_handle, simple_num,
			 "process_file") != R_SUCCESS) {
    tidy_and_exit(-1);
  }
  
  st_params = t_handle->simple_params;

  /*
   * loop through the track entries
   */
  
  fprintf(stdout,
	  "\n*****************************************\n");
  fprintf(stdout, "Complex_track number: %ld\n", (long) complex_num);
  fprintf(stdout, "Simple_track number : %ld\n", (long) simple_num);
  fprintf(stdout, "Start time          : %s\n",
	  utimstr(st_params->start_time));
  fprintf(stdout, "End time            : %s\n",
	  utimstr(st_params->end_time));
  fprintf(stdout, "Number of Scans     : %ld\n",
	  (long) st_params->duration_in_scans);
  fprintf(stdout, "\n");

  if (Glob->print_layers) {

    print_layers(s_handle, t_handle, st_params);

  } else {
    
    print_gprops(s_handle, t_handle, st_params);
    
  }
  
  printf("\n");
  
}

static void print_gprops(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 simple_track_params_t *st_params)

{
  
  long istorm, ientry;
  storm_file_global_props_t *gprops;

  fprintf(stdout, "%4s %10s %8s %7s %7s %7s %7s %7s %7s %8s %8s\n",
	  "scan", "date", "time", "refl-x", "refl-y", "refl-z", "delta-z",
	  "volume", "mass", "av area", "max dbz");
  
  fprintf(stdout, "%4s %10s %8s %7s %7s %7s %7s %7s %7s %8s %8s\n",
	  " ", " ", " ", "(km)", "(km)", "(km)", "(km)",
	  "(km3)", "(ktons)", "(km2)", "(dbz)");
  
  for (ientry = 0;
       ientry < st_params->duration_in_scans; ientry++) {
    
    if (RfReadTrackEntry(t_handle, "print_gprops") != R_SUCCESS) {
      tidy_and_exit(-1);
    }
    
    if (RfReadStormScan(s_handle, t_handle->entry->scan_num,
			"print_gprops") != R_SUCCESS) {
      tidy_and_exit(-1);
    }
    
    istorm = t_handle->entry->storm_num;
    gprops = s_handle->gprops + istorm;
    
    /*
     * print out storm data
     */
    
    fprintf(stdout,
	    "%4d %s "
	    "%7.1f %7.1f %7.1f %7.1f %7.1f"
	    " %7.1f %8.1f %8.1f\n", 
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

}

static void print_layers(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 simple_track_params_t *st_params)
     
{

  char *spacer = "";
  char *loc_label;
  long istorm, ientry, ilayer;
  storm_file_global_props_t *gprops;
  track_file_entry_t *entry;
  storm_file_layer_props_t *l;
  
  for (ientry = 0;
       ientry < st_params->duration_in_scans; ientry++) {
    
    if (RfReadTrackEntry(t_handle, "print_layers") != R_SUCCESS) {
      tidy_and_exit(-1);
    }
    
    entry = t_handle->entry;
    
    if (RfReadStormScan(s_handle, t_handle->entry->scan_num,
			"print_layers") != R_SUCCESS) {
      tidy_and_exit(-1);
    }
    
    istorm = t_handle->entry->storm_num;
    gprops = s_handle->gprops + istorm;
    
    if (RfReadStormProps(s_handle, istorm,
			 "print_layers") != R_SUCCESS) {
      tidy_and_exit(-1);
    }
    
    fprintf(stdout, "\nTime of scan %ld: %s\n\n", ientry,
	    utimstr(entry->time));
    
    if (s_handle->scan->grid.proj_type == TITAN_PROJ_FLAT) {
      loc_label = " (km)";
    } else {
      loc_label = "(deg)";
    }
    
    /*
     * layer properties
     */
    
    fprintf(stdout, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	    spacer,
	    "Layer", "z", "x-cent", "y-cent", "x-Zcent", "y-Zcent", "area",
	    "max Z", "mean Z");
    fprintf(stdout, "%s%5s %5s %7s %7s %7s %7s %6s %6s %7s\n",
	    spacer, " ", "(km)", loc_label, loc_label, loc_label, loc_label,
	    "(km2)", "(dbz)", "(dbz)");
    
    l = s_handle->layer;
    
    for (ilayer = gprops->base_layer;
	 ilayer < (gprops->base_layer + gprops->n_layers);
	 ilayer++, l++) {
      
      fprintf(stdout, "%s%5ld %5g %7.1f %7.1f %7.1f %7.1f "
	      "%6.1f %6.1f %7.1f\n",
	      spacer, ilayer,
	      (s_handle->scan->min_z + (double) ilayer * s_handle->scan->delta_z),
	      l->vol_centroid_x,
	      l->vol_centroid_y,
	      l->refl_centroid_x,
	      l->refl_centroid_y,
	      l->area,
	      l->dbz_max,
	      l->dbz_mean);
      
    } /* ilayer */
    
    fprintf(stdout, "\n");
    
/*    RfPrintStormLayer(stdout, "", &s_handle->header->params,
		      s_handle->scan, gprops,
		      s_handle->layer); */
    
  } /* ientry */

  return;

}

