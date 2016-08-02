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
/************************************************************************
 * storms_to_tifs.c
 *
 * Prints out storm and track data for the track files in the
 * command line. 
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1992
 * 
 * (rjp, 7 Oct 2002) Revised version of storms_to_ascii.c code 
 * to print output in format for TIFS. 
 * 
 ************************************************************************/

#define MAIN
#include "storms_to_tifs.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  path_parts_t progname_parts;

  si32 n_track_files, ifile;
  char **track_file_paths;

  date_time_t *scan_time;

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
   * parse command line arguments
   */

  parse_args(argc, argv, &n_track_files, &track_file_paths);

  /*
   * initialize
   */

  init_indices(&s_handle, &t_handle);

  /*
   * print comments to start of file
   * (rjp 7 Oct 2002) Header printed in process_track_file. 
   */

  /*  print_comments(stdout); */

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
      tidy_and_exit(-1);

    /*
     * process the track file, producing forecast grids
     */

    process_track_file(&s_handle, &t_handle, scan_time);

    /*
     * close files
     */

    RfCloseStormFiles(&s_handle, "main");
    RfCloseTrackFiles(&t_handle, "main");

  }

  /*
   * quit
   */
  
  tidy_and_exit(0);

  return(0);

}

