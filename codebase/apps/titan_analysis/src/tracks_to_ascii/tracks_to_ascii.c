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
/**********************************************************
 * tracks_to_ascii.c
 *
 * reads in tracks and storm files, outputs data to an
 * ascii file in column format
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * December 1992
 *
 **********************************************************/

#define MAIN
#include "tracks_to_ascii.h"
#undef MAIN

int main(int argc, char **argv)

{

  si32 n_complex = 0;
  si32 n_simple = 0;
  si32 n_total;
  autocorr_t autocorr_volume;
  autocorr_t autocorr_vol_two_thirds;
  autocorr_t autocorr_precip_area;
  autocorr_t autocorr_proj_area;
  
  /*
   * basic declarations
   */

  path_parts_t progname_parts;

  si32 n_track_files, ifile;
  char **track_file_paths;

  storm_file_handle_t s_handle;
  track_file_handle_t t_handle;

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

  memset ((void *) &autocorr_volume,
	  (int) 0, sizeof(autocorr_t));

  memset ((void *) &autocorr_vol_two_thirds,
	  (int) 0, sizeof(autocorr_t));

  memset ((void *) &autocorr_precip_area,
	  (int) 0, sizeof(autocorr_t));

  memset ((void *) &autocorr_proj_area,
	  (int) 0, sizeof(autocorr_t));

  /* 
   * print header
   */

  print_header(n_track_files, track_file_paths);

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
     * perform the conversion
     */

    process_storms(&s_handle, &t_handle, &n_complex, &n_simple,
		   &autocorr_volume,
		   &autocorr_vol_two_thirds,
		   &autocorr_precip_area,
		   &autocorr_proj_area);

    /*
     * close files
     */

    RfCloseStormFiles(&s_handle, "main");
    RfCloseTrackFiles(&t_handle, "main");

  } /* ifile */

  /*
   * computations
   */

  compute_autocorrelation(&autocorr_volume);
  compute_autocorrelation(&autocorr_vol_two_thirds);
  compute_autocorrelation(&autocorr_precip_area);
  compute_autocorrelation(&autocorr_proj_area);

  n_total = n_simple + n_complex;
  
  fprintf(stderr, "n_complex : %ld\n", (long) n_complex);
  fprintf(stderr, "n_simple : %ld\n", (long) n_simple);
  fprintf(stderr, "n_total : %ld\n", (long) n_total);

  print_autocorrelation("Volume", &autocorr_volume);
  print_autocorrelation("Vol_two_thirds", &autocorr_vol_two_thirds);
  print_autocorrelation("Precip_Area", &autocorr_precip_area);
  print_autocorrelation("Proj_Area", &autocorr_proj_area);

  /*
   * quit
   */
  
  tidy_and_exit(0);

  return(0);

}

