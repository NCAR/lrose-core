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
/***********************************************************************
 * track_grid_stats.c
 *
 * Produces a grid of stats on storm tracks
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * August 1993
 *
 ************************************************************************/

#define MAIN
#include "track_grid_stats.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char **track_file_paths;
  char *params_file_path = NULL;

  int check_params;
  int print_params;

  si32 n_track_files = 0;
  si32 ifile;
  si32 n_scans_total = 0;

  date_time_t data_start, data_end;

  path_parts_t progname_parts;
  storm_file_handle_t s_handle;
  track_file_handle_t t_handle;
  vol_file_handle_t v_handle;
  grid_stats_t **stats;
  tdrp_override_t override;
  
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
   * parse command line arguments
   */

  parse_args(argc, argv,
             &check_params, &print_params,
	     &override,
             &params_file_path,
             &n_track_files, &track_file_paths);

  /*
   * load up parameters
   */
  
  Glob->table = track_grid_stats_tdrp_init(&Glob->params);

  if (FALSE == TDRP_read(params_file_path,
			 Glob->table,
			 &Glob->params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    tidy_and_exit(-1);
  }
  
  TDRP_free_override(&override);

  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(0);
  }
  
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(0);
  }
  
  if (Glob->params.malloc_debug_level > 0)
    umalloc_debug(Glob->params.malloc_debug_level);
  
  /*
   * initialize
   */

  init_indices(&s_handle, &t_handle, &v_handle);
  data_start.unix_time = 2000000000;
  data_end.unix_time = 0;

  /*
   * allocate grid for stats
   */

  stats = (grid_stats_t **) ucalloc2
    ((ui32) Glob->params.grid.ny,
     (ui32) Glob->params.grid.nx,
     sizeof(grid_stats_t));
  
  /*
   * loop through the track files
   */

  if (n_track_files == 0) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr,
	    "For archive mode you must specify files using -f arg\n");
    tidy_and_exit(-1);
  }
    
  for (ifile = 0; ifile < n_track_files; ifile++) {
      
    process_file(&s_handle, &t_handle,
		 track_file_paths[ifile],
		 stats,
		 &data_start, &data_end,
		 &n_scans_total);

  } /* ifile */

  /*
   * compute the stats fields
   */

  compute_stats(&v_handle, stats, n_scans_total);

  /*
   * write the stats file
   */

  write_stats_file(&v_handle, &data_start, &data_end);

  /*
   * quit
   */
  
  tidy_and_exit(0);
  return(0);

}

