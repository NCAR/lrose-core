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
 * verify_day.c
 *
 * Verifies the track forecast against grids prepared by storm_ident
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * October 1991
 *
 *******************************************************************************/

#define MAIN
#include "verify_day.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;

  si32 n_track_files, ifile;
  char **track_file_paths;

  time_t *scan_time;

  storm_file_handle_t s_handle;
  track_file_handle_t t_handle;

  ucont_table_t cont_table;

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

  memset ((void *) &cont_table,
          (int) 0, (size_t) sizeof(ucont_table_t));

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
     * read in track utime array
     */

    if (RfReadTrackUtime(&t_handle, "main") != R_SUCCESS)
      tidy_and_exit(1);

    /*
     * load contingency data
     */

    load_cont_data(&s_handle, &t_handle, scan_time, &cont_table);

    /*
     * close files
     */

    RfCloseStormFiles(&s_handle, "main");
    RfCloseTrackFiles(&t_handle, "main");

  }

  /*
   * compute contingency table
   */

  ucompute_cont(&cont_table);

  /*
   * printout
   */

  fprintf(stderr, "\n");
  fprintf(stderr, "VERIFY_DAY\n");
  fprintf(stderr, "==========\n\n");
  
  fprintf(stderr, "    Forecast lead time: %g mins\n",
	  Glob->forecast_lead_time / 60.0);
  fprintf(stderr, "    Forecast lead time_margin: %g mins\n",
	  Glob->forecast_lead_time_margin / 60.0);

  if (Glob->mode == ELLIPSE_MODE) {
    fprintf(stdout, "    Ellipses used.\n");
  } else {
    fprintf(stdout, "    Runs used.\n");
  }
  
  if (Glob->verification_field == ALL_STORMS_FIELD) {
    fprintf(stderr, "    All storms used.\n");
  } else {
    fprintf(stderr, "    Only valid storms used.\n");
  }
  
  fprintf(stderr, "    Ellipse radius ratio: %g\n",
	  Glob->ellipse_radius_ratio);
  
  fprintf(stderr, "    Min valid history: %ld scans\n",
	  (long) Glob->min_valid_history);

  fprintf(stdout, "\n");

  uprint_cont(&cont_table, stdout, Glob->prog_name,
	      "    ", TRUE, TRUE);

  /*
   * quit
   */
  
  tidy_and_exit(0);

  return(0);

}

