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
/***************************************************************************
 * storm_ident.c
 *
 * Identifies storms in a radar data file, computes the storm parameters
 * and writes the parameters to a file.
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * January 1991
 *
 ****************************************************************************/

#define MAIN
#include "storm_ident.h"
#undef MAIN

static storm_file_handle_t *S_Handle;
static vol_file_handle_t *V_Handle;

/*
 * file scope prototypes
 */

static char *header_file_path(date_time_t *ftime);

static void init_indices(vol_file_handle_t *v_handle,
			 storm_file_handle_t *s_handle);

static void print_startup_message(void);

static void run_archive(vol_file_handle_t *v_handle,
			storm_file_handle_t *s_handle);

static void run_realtime(vol_file_handle_t *v_handle,
			 storm_file_handle_t *s_handle);

/*
 * main
 */

int main(int argc, char **argv)

{

  char *params_file_path = NULL;
  int check_params;
  int print_params;
  path_parts_t progname_parts;
  tdrp_override_t override;
  vol_file_handle_t v_handle;
  storm_file_handle_t s_handle;

  /*
   * register function to trap termination and interrupts
   */

  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *)
    ucalloc((ui32) 1, (ui32) sizeof(global_t));

  /*
   * set argc and argv in the global struct - these are used in
   * tidy_and_exit if a restart is needed
   */

  Glob->argc = argc;
  Glob->argv = argv;

  /*
   * initialize flags
   */

  Glob->shmem_available = FALSE;
  Glob->sems_available = FALSE;

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
	     &params_file_path);
    
  /*
   * load up parameters
   */
  
  Glob->table = storm_ident_tdrp_init(&Glob->params);
  
  if (FALSE == TDRP_read(params_file_path,
			 Glob->table,
			 &Glob->params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    exit(-1);
  }
  TDRP_free_override(&override);

  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    exit(0);
  }

  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    exit(0);
  }

  set_derived_params();

  /*
   * initialize process registration
   */

  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);

  /*
   * create lock file
   */

  if (create_lock_file()) {
    exit(-1);
  }
  
  /*
   * start up storm_track if required
   */
  
  if (Glob->params.tracking) {
    create_shmem();
  }
  if (Glob->params.start_storm_track) {
    system(Glob->params.storm_track_command_line);
    sleep(STORM_TRACK_START_SECS);
  }
    
  /*
   * initialize file indices
   */
  
  init_indices(&v_handle, &s_handle);

  /*
   * start cpu clock in debug mode
   */
  
  if (Glob->params.debug >= DEBUG_VERBOSE) {
    clock();
  }

  /*
   * print message
   */

  print_startup_message();

  /*
   * call archive or realtime routines
   */
  
  if (Glob->params.mode == TDATA_ARCHIVE) {
    
    run_archive(&v_handle, &s_handle);

  } else {

    run_realtime(&v_handle, &s_handle);

  }

  tidy_and_exit (0);

  return (0);

}

/*-------------------------------------------------
 * get header file path
 */

static char *header_file_path(date_time_t *ftime)

{

  static char *header_file_path = NULL;
  si32 nbytes_path;
  
  /*
   * malloc space for path
   */
  
  nbytes_path = (strlen(Glob->params.storm_data_dir) + 
		 strlen(PATH_DELIM) +
		 strlen(STORM_HEADER_FILE_EXT) + 10);
  
  if (header_file_path == NULL) {
    header_file_path = umalloc((ui32) nbytes_path);
  } else {
    header_file_path = urealloc((char *) header_file_path,
				(ui32) nbytes_path);
  }
  
  /*
   * set file path
   */

  sprintf(header_file_path, "%s%s%.4d%.2d%.2d.%s",
	  Glob->params.storm_data_dir,
	  PATH_DELIM,
	  ftime->year, ftime->month,
	  ftime->day,
	  STORM_HEADER_FILE_EXT);
  
  return (header_file_path);

}

/*-------------------------------------------------
 * initialize the file indices
 */

static void init_indices(vol_file_handle_t *v_handle,
			 storm_file_handle_t *s_handle)

{

  /*
   * initialize volume index
   */

  RfInitVolFileHandle(v_handle, 
		      Glob->prog_name,
		      (char *) NULL,
		      (FILE *) NULL);

  /*
   * initialize storm file handle
   */
  
  memset ((void *) s_handle,
          (int) 0, (size_t) sizeof(storm_file_handle_t));

  RfInitStormFileHandle(s_handle, Glob->prog_name);
  
  V_Handle = v_handle;
  S_Handle = s_handle;

  return;

}

/*-------------------------------------------------
 * free the file indices
 */

void free_indices(void)

{

  /*
   * close storm file
   */

  RfCloseStormFiles(S_Handle, "tidy_and_exit");

  /*
   * free the indices
   */
  
  RfFreeStormFileHandle(S_Handle);
  RfFreeVolFileHandle(V_Handle);

  return;

}

/*-------------------------------------------------
 * starting message
 */

#define BOOL_STR(a) (a == 0? "false" : "true")

static void print_startup_message(void)

{

  fprintf(stdout, "\n");
  fprintf(stdout, "STORM_IDENT\n");
  fprintf(stdout, "===========\n");
  fprintf(stdout, "\n");
  
  fprintf(stdout, "Low dBZ threshold : %g\n",
	  Glob->params.low_dbz_threshold);
  fprintf(stdout, "High dBZ threshold : %g\n",
	  Glob->params.high_dbz_threshold);
  fprintf(stdout, "dBZ hist interval : %g\n",
	  Glob->params.dbz_hist_interval);
  fprintf(stdout, "hail dBZ threshold : %g\n",
	  Glob->params.hail_dbz_threshold);
  fprintf(stdout, "Base threshold (km) : %g\n",
	  Glob->params.base_threshold);
  fprintf(stdout, "Top threshold (km) : %g\n",
	  Glob->params.top_threshold);

  if (Glob->nz <= 1)
  {
    fprintf(stdout, "Min storm area (km2) : %g\n",
	    Glob->params.min_storm_size);
    fprintf(stdout, "Max storm area (km2) : %g\n",
	    Glob->params.max_storm_size);
  }
  else
  {
    fprintf(stdout, "Min storm volume (km3) : %g\n",
	    Glob->params.min_storm_size);
    fprintf(stdout, "Max storm volume (km3) : %g\n",
	    Glob->params.max_storm_size);
  }
  fprintf(stdout, "Z-P coefficient : %g\n", Glob->params.ZR.coeff);
  fprintf(stdout, "Z-P exponent : %g\n", Glob->params.ZR.expon);
  fprintf(stdout, "Z-M coefficient : %g\n", Glob->params.ZM.coeff);
  fprintf(stdout, "Z-M exponent : %g\n", Glob->params.ZM.expon);
  fprintf(stdout, "2nd trip vert aspect : %g\n",
	  Glob->params.sectrip_vert_aspect);
  fprintf(stdout, "2nd trip horiz aspect : %g\n",
	  Glob->params.sectrip_horiz_aspect);
  fprintf(stdout, "2nd trip orientation error : %g\n",
	  Glob->params.sectrip_orientation_error);
  fprintf(stdout, "Velocity data available? : %s\n",
	  BOOL_STR(Glob->params.vel_available));
  
  fprintf(stdout, "\n\n");

}
  
/*-------------------------------------------------
 * archive mode
 */

static void run_archive(vol_file_handle_t *v_handle,
			storm_file_handle_t *s_handle)

{

  char *file_path;
  si32 start_time, end_time;
  si32 itime, ntimes_in_list;
  si32 scan_num, prev_scan_num;
  date_time_t *time_list;
  date_time_t ftime;
  date_time_t latest_data_time;
  storm_file_header_t header;

  /*
   * set up the start and end times for the analysis
   */

  get_startup_time_limits(&start_time, &end_time);
  latest_data_time.unix_time = end_time;
  uconvert_from_utime(&latest_data_time);
  
  /*
   * get the list of times for the analysis
   */
  
  ntimes_in_list = get_time_list (start_time, end_time, &time_list);

  if (ntimes_in_list < 1) {
    fprintf(stderr, "WARNING - %s:run_archive\n", Glob->prog_name);
    fprintf(stderr, "No times found\n");
    tidy_and_exit(-1);
  }

  /*
   * get date on which file name is based
   */

  get_storm_file_date(&ftime.unix_time);
  uconvert_from_utime(&ftime);
  
  /*
   * get header file path
   */

  file_path = header_file_path(&ftime);

  /*
   * load file header
   */
  
  load_header(&header);

  /*
   * check storm file
   */

  if (open_and_check_storm_file(s_handle, file_path, &header) == 0) {

    /*
     * get prev scan number analyzed - this also modifies
     * time_list and ntimes_in_list so that the list 
     * contains only entries which have not yet been
     * analyzed.
     */
    
    prev_scan_num = get_prev_scan(s_handle,
				  time_list,
				  &ntimes_in_list);

  } else {

    prev_scan_num = -1;

  }
    
  if (prev_scan_num < 0) {
    
    prepare_new_file(s_handle,
                     file_path,
                     &header);
    
  } else {
    
    prepare_old_file(s_handle,
                     file_path,
                     prev_scan_num);
    
  }
  
  /*
   * If tracking is required, set up shared memory
   */
  
  if (Glob->params.tracking) {

    init_shmem(file_path);
    
    /*
     * if the storm file already has analysed scans in it,
     * prepare the storm track program for appending
     */
    
    if (prev_scan_num >= 0) {
      perform_tracking(PREPARE_FOR_APPEND);
    }
    
  }
  
  /*
   * loop through the radar data files
   */

  scan_num = prev_scan_num + 1;
  
  for (itime = 0; itime < ntimes_in_list; itime++) {

    PMU_auto_register("Processing archive data");
    
    /*
     * read in dobson data
     */
      
    if (read_dobson(v_handle, &time_list[itime]) == 0) {

      /*
       * if required, mask out areas with low tops
       */
      
      if (Glob->params.check_tops) {
	mask_low_tops(v_handle);
      }
      
      /*
       * identify storms
       */
      
      identify(v_handle, s_handle, scan_num);
      scan_num++;
      
      /*
       * write index file
       */
      
      write_file_index(s_handle, file_path);
      
      umalloc_map();

    } /* if (read_dobson() ... */
      
  } /* itime */
  
  return;

}

/*-------------------------------------------------
 * realtime mode - dobson data
 */

static void run_realtime(vol_file_handle_t *v_handle,
			 storm_file_handle_t *s_handle)
     
{

  char *file_path;

  int forever = TRUE;
  int first_pass;

  si32 itime;
  si32 elapsed_time;
  si32 start_time, end_time;
  si32 ntimes_in_list;
  si32 scan_num, prev_scan_num;

  date_time_t *time_list;
  date_time_t ftime;
  date_time_t prev_time;
  date_time_t latest_data_time;
  storm_file_header_t header;

  /*
   * set up the start and end times for prep analysis
   */

  get_startup_time_limits(&start_time, &end_time);
  
  /*
   * get the list of times for the prep analysis
   */

  ntimes_in_list = get_time_list (start_time, end_time, &time_list);
  latest_data_time.unix_time = end_time;
  uconvert_from_utime(&latest_data_time);
  
  /*
   * get date on which file name is based
   */

  get_storm_file_date(&ftime.unix_time);
  uconvert_from_utime(&ftime);
  
  /*
   * get header file path
   */

  file_path = header_file_path(&ftime);

  /*
   * load file header
   */
  
  load_header(&header);

  /*
   * check storm file
   */

  if (open_and_check_storm_file(s_handle, file_path, &header) == 0) {

    /*
     * get prev scan number analyzed - this also modifies
     * time_list and ntimes_in_list so that the list 
     * contains only entries which have not yet been
     * analyzed.
     */
    
    prev_scan_num = get_prev_scan(s_handle,
				  time_list,
				  &ntimes_in_list);

  } else {

    prev_scan_num = -1;

  }
    
  if (prev_scan_num < 0) {
    
    prepare_new_file(s_handle,
		     file_path,
		     &header);
    
  } else {
    
    prepare_old_file(s_handle,
		     file_path,
		     prev_scan_num);
    
  }
  
  /*
   * If tracking is required, set up shared memory
   */
  
  if (Glob->params.tracking) {

    init_shmem(file_path);
    
    /*
     * if the storm file already has analysed scans in it,
     * prepare the storm track program for appending
     */
    
    if (prev_scan_num >= 0) {
      perform_tracking(PREPARE_FOR_APPEND);
    }
    
  }
  
  /*
   * initialize
   */

  scan_num = prev_scan_num + 1;
  first_pass = TRUE;
  memset((void *) &prev_time, 0, (int) sizeof(date_time_t));
    
  /*
   * loop forever, getting a new times list each loop - under normal
   * circumstances the list will contain only a single entry
   */

  while (forever) {
    
    PMU_auto_register("Processing realtime data");
    
    /*
     * process the times list
     */
    
    for (itime = 0; itime < ntimes_in_list; itime++) {

      char pmu_message[BUFSIZ];
      
      sprintf(pmu_message, "Processing time %d\n", itime);
      PMU_auto_register(pmu_message);
      
      /*
       * on subsequent passes, check for restart conditions
       */
      
      if (first_pass) {
	
	first_pass = FALSE;
	
      } else {
	
	elapsed_time =
	  time_list[itime].unix_time - prev_time.unix_time;
	
	/*
	 * if elapsed time is negative exit and restart
	 */
	
	if (elapsed_time < 0) {
	  fprintf(stderr, "NOTE: %s\n", Glob->prog_name);
	  fprintf(stderr, "Restarting - time moved into the past\n");
	  fprintf(stderr, "Prev time: %s\n",
		  utimestr(&prev_time));
	  fprintf(stderr, "This time: %s\n",
		  utimestr((time_list + itime)));
	  tidy_and_exit(EXIT_AND_RESTART);
	}
	
	/*
	 * if elapsed time exceeds max_gap, exit and restart
	 */
	
	if (Glob->params.mode == TDATA_ARCHIVE &&
	    (elapsed_time > Glob->params.max_missing_data_gap)) {
	  fprintf(stderr, "NOTE: %s\n", Glob->prog_name);
	  fprintf(stderr, "Restarting - data time gap too large\n");
	  fprintf(stderr, "Prev time: %s\n",
		  utimestr(&prev_time));
	  fprintf(stderr, "This time: %s\n",
		  utimestr((time_list + itime)));
	  tidy_and_exit(EXIT_AND_RESTART);
	}
	
	/*
	 * if program is in auto-restart mode and the restart time
	 * has passed, restart
	 */

	if (Glob->params.auto_restart &&
	    time_list[itime].unix_time >= get_restart_time()) {

   	  if (restart(s_handle) && Glob->params.debug >= DEBUG_NORM) {
	    fprintf(stderr, "WARNING: %s:run_realtime\n",
		    Glob->prog_name);
	    fprintf(stderr, "Cannot restart - continuing\n");
	  }
	  
	} /* if (Glob->params.auto_restart ... */
	
      } /* if (first_pass) */
      
      /*
       * read in dobson data
       */
      
      if (read_dobson(v_handle, &time_list[itime]) == 0) {
      
	/*
	 * if required, mask out areas with low tops
	 */
	
	if (Glob->params.check_tops) {
	  mask_low_tops(v_handle);
	}
	
	/*
	 * identify storms
	 */
	
	identify(v_handle, s_handle, scan_num);
	scan_num++;
	
	/*
	 * write index file
	 */
	
	write_file_index(s_handle, file_path);
	
	/*
	 * store prev time
	 */
	
	memcpy ((void *) &prev_time,
		(void *) &time_list[itime],
		(size_t) sizeof(date_time_t));
	
	fflush(stdout);

      } /* if (read_dobson() ... */
      
    } /* itime */
  
    /*
     * get a new times list each loop - under normal
     * circumstances the list will contain only a single entry
     */
    
    start_time = end_time + 1;
    end_time = get_dobson_data_time();
    ntimes_in_list = get_time_list (start_time, end_time, &time_list);

    umalloc_map();

  } /* while (forever) */

}
    
