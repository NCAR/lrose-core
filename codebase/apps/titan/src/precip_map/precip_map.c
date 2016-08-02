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
 * precip_map.c
 *
 * Produces a precipitation map, based either on storm tracking
 * forecasts or verification data
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Jan 1992
 *
 ************************************************************************/

#define MAIN
#include "precip_map.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char **track_file_paths;
  char *params_file_path = NULL;
  char header_file_path[MAX_PATH_LEN];
  int forever = TRUE;
  int check_params;
  int print_params;
  si32 n_track_files = 0;
  si32 ifile;
  path_parts_t progname_parts;
  storm_file_handle_t s_handle;
  track_file_handle_t t_handle;
  vol_file_handle_t v_handle, map_v_handle;
  tdrp_override_t override;
  LDATA_handle_t ldata;

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);

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
  
  Glob->table = precip_map_tdrp_init(&Glob->params);

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
  
  if (Glob->params.map_type == VERIFY)
    Glob->params.mode = ARCHIVE;
  
  /*
   * initialize process registration
   */

  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * initialize
   */

  init_indices(&s_handle, &t_handle, &v_handle, &map_v_handle);
  
  LDATA_init_handle(&ldata,
		    Glob->prog_name,
		    Glob->params.debug);

  if (Glob->params.mode == REALTIME) {

    /*
     * realtime mode
     */

    while (forever) {

      /*
       * get latest data info, set header file path.
       * blocks until new data available.
       */
      
      LDATA_info_read_blocking(&ldata,
			       Glob->params.storms_dir,
			       Glob->params.max_realtime_valid_age,
			       2000,
			       PMU_auto_register);
      
      sprintf(header_file_path, "%s%s%s.%s",
	      Glob->params.storms_dir, PATH_DELIM,
	      ldata.info.user_info_1, TRACK_HEADER_FILE_EXT);
      
      process_file(&s_handle, &t_handle, &v_handle, &map_v_handle,
		   header_file_path, &ldata);
      
      umalloc_count();
      
    } /* while (forever) */
    
  } else {

    /*
     * archive mode
     *
     * loop through the track files
     */

    if (n_track_files == 0) {
      fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
      fprintf(stderr,
	      "For archive mode you must specify files using -f arg\n");
      tidy_and_exit(-1);
    }
    
    for (ifile = 0; ifile < n_track_files; ifile++) {
      
      process_file(&s_handle, &t_handle, &v_handle, &map_v_handle,
		   track_file_paths[ifile], &ldata);

      umalloc_count();
      
    } /* ifile */

    tidy_and_exit(0);

  } /* if (Glob->params.mode == REALTIME) */

  /*
   * quit
   */
  
  return(0);

}

