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
 * verify_tracks.c
 *
 * Verifies the track forecast against grids prepared by storm_ident.
 * Also, computes and inserts the 'last-descendant' data into each
 * simple track's params.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Jan 1992
 *
 *******************************************************************************/

#define MAIN
#include "verify_tracks.h"
#undef MAIN

#define BOOL_STR(a) (a == 0? "false" : "true")

#define METHOD_STR(a) (a == STORM_ELLIPSE? "ellipse" : \
		       (a == STORM_POLYGON? "polygon" : "runs"))

#define ACTIVITY_STR(a) (a == AREA_ACTIVITY? "area" : "point")

#define KM_PER_DEG_AT_EQUATOR 111.12

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char **track_file_paths;
  long n_track_files, ifile;
  path_parts_t progname_parts;
  date_time_t *scan_time;
  storm_file_handle_t s_handle;
  track_file_handle_t t_handle;
  vt_count_t total_count;
  vt_stats_t total_stats;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * load up parameters data base
   */
  
  Glob->params_path_name = uparams_read(argv, argc, Glob->prog_name);

  /*
   * set variables from data base
   */

  read_params();

  /*
   * parse command line arguments
   */

  parse_args(argc, argv, &n_track_files, &track_file_paths);

  /*
   * initialize
   */

  init_indices(&s_handle, &t_handle);

  memset ((void *)  &total_count,
          (int) 0, (size_t) sizeof(vt_count_t));
  memset ((void *)  &total_stats,
          (int) 0, (size_t) sizeof(vt_stats_t));
  
  /*
   * loop through the track files
   */

  for (ifile = 0; ifile < n_track_files; ifile++) {

    fprintf(stderr, "\nProcessing file %s\n",
	    track_file_paths[ifile]);

    /*
     * open track and storm files
     */
    
    open_files(&s_handle, &t_handle, track_file_paths, ifile);
  
    /*
     * load scan time array
     */

    scan_time = load_scan_times(&s_handle);

    /*
     * perform the verification
     */

    perform_verification(&s_handle, &t_handle,
			 scan_time,
			 &total_count,
			 &total_stats);

    /*
     * save the verification parameters
     */

    save_verification_parameters(&s_handle, &t_handle);
    
    /*
     * close files
     */

    RfCloseStormFiles(&s_handle, "main");
    RfCloseTrackFiles(&t_handle, "main");

    umalloc_count();

  } /* ifile */

  /*
   * print out
   */

  fprintf(stdout, "\nVERIFY_TRACKS\n-------------\n\n");

  fprintf(stdout, "Track files used:\n");

  for (ifile = 0; ifile < n_track_files; ifile++)
    fprintf(stdout, "  %s\n",
	    track_file_paths[ifile]);
  fprintf(stdout, "\n");
  
  fprintf(stdout, "Verification grid:\n");
  fprintf(stdout, "  dx, dy                          : %g, %g\n",
	  Glob->dx, Glob->dy);
  fprintf(stdout, "  minx, miny                      : %g, %g\n",
	  Glob->minx, Glob->miny);
  fprintf(stdout, "  nx, ny                          : %ld, %ld\n",
	  Glob->nx, Glob->ny);
  fprintf(stdout, "\n");

  fprintf(stdout, "Forecast method                   : %s\n", 
	  METHOD_STR(Glob->forecast_method));

  fprintf(stdout, "Verify method                     : %s\n", 
	  METHOD_STR(Glob->verify_method));
  
  fprintf(stdout, "Forecast lead time (mins)         : %g\n", 
	  Glob->forecast_lead_time / 60.0);

  fprintf(stdout, "Forecast lead time margin (mins)  : %g\n", 
	  Glob->forecast_lead_time_margin / 60.0);

  fprintf(stdout, "Forecast min history (mins)       : %g\n", 
	  Glob->forecast_min_history / 60.0);

  fprintf(stdout, "Parabolic_Growth                  : %s\n", 
	  BOOL_STR(Glob->parabolic_growth));

  fprintf(stdout, "Forecast growth period (mins)     : %g\n", 
	  Glob->forecast_growth_period / 60.0);

  fprintf(stdout, "Verify_before_forecast_time       : %s\n", 
	  BOOL_STR(Glob->verify_before_forecast_time));

  fprintf(stdout, "Verify_after_track_dies           : %s\n", 
	  BOOL_STR(Glob->verify_after_track_dies));

  fprintf(stdout, "Force_xy_fit                      : %s\n", 
	  BOOL_STR(Glob->force_xy_fit));

  fprintf(stdout, "Forecast_scale_factor             : %g\n",
	  Glob->forecast_scale_factor);

  fprintf(stdout, "Activity_criterion                : %s\n", 
	  ACTIVITY_STR(Glob->activity_criterion));

  if (Glob->activity_criterion == AREA_ACTIVITY) {

    fprintf(stdout, "Activity_radius                   : %g\n", 
	    Glob->activity_radius);

    fprintf(stdout, "Activity_fraction                 : %g\n", 
	    Glob->activity_fraction);
    
  } /* if (Glob->activity_criterion == AREA_ACTIVITY) */
  
  print_contingency_table(stdout,
			  "Contingency data.\n----------------",
			  &total_count);
  
  compute_stats(&total_stats);
  
  print_stats(stdout,
	      "Stats for entire analysis.\n-------------------------",
	      &total_stats);

  /*
   * quit
   */
  
  tidy_and_exit(0);

  return(0);

}

