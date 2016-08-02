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
 * dva_mklookup.c
 *
 * Reads Bethlehem data, outputs ray and status files.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * Nov 1996
 *
 ************************************************************************/

#define MAIN
#include "dva_mklookup.h"
#undef MAIN

int main(int argc, char **argv)

{

  /*
   * basic declarations
   */

  char *params_file_path = NULL;
  char grid_file_path[MAX_PATH_LEN];
  char lookup_file_path[MAX_PATH_LEN];
  int check_params;
  int print_params;
  int iz, igate;
  int npts_cappi, nlookup;
  path_parts_t progname_parts;
  dva_grid_t grid;
  dva_rdas_cal_t cal;
  dva_lookup_t *lookup;
  float *range;
  dva_position_t *pos;
  FILE *gridfile;
  FILE *lookupfile;
  tdrp_override_t override;

  /*
   * allocate space for the global structure
   */
  
  Glob = (global_t *) umalloc((unsigned int) sizeof(global_t));

  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((unsigned int) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * set signal traps
   */
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
 
  /*
   * ignore SIGPIPE - disconnect from client
   */
  
  signal(SIGPIPE, SIG_IGN);

  /*
   * parse command line arguments
   */
  
  parse_args(argc, argv,
	     &check_params, &print_params,
	     &override, &params_file_path);

  /*
   * load up parameters
   */
  
  Glob->table = dva_mklookup_tdrp_init(&Glob->params);
  
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
  
  if (Glob->params.malloc_debug_level > 0) {
    umalloc_debug(Glob->params.malloc_debug_level);
  }
  
  /*
   * initialize process registration
   */

  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);

  /*
   * write out elevations array
   */

  if (write_elevations()) {
    exit (-1);
  }

  /*
   * read in grid params
   */

  if (read_grid_params(&grid)) {
    exit (-1);
  }
  
  /*
   * read in cal data
   */
  
  if (read_rdas_cal(&cal)) {
    exit (-1);
  }

  /*
   * generate displace table
   */

  if (generate_displace_table(&cal)) {
    exit(-1);
  }
  
  /*
   * generate range correction and vip2byte tables
   */

  if (generate_range_corr(&cal)) {
    exit(-1);
  }
  
  /*
   * set up range array. This is the range to the start of each gate.
   * This will be used as a 1-based array, hence the (igate-1.5) factor.
   */
  
  range = (float *) umalloc((cal.ngates+1) * sizeof(float));
  for (igate = 0; igate <= cal.ngates; igate++) {
    range[igate] = (cal.start_range + (float) (igate-1.5) * cal.gate_spacing);
  }
  
  /*
   * alloc position array
   */
  
  npts_cappi = grid.nx * grid.ny;
  pos = (dva_position_t *) umalloc (npts_cappi * sizeof(dva_position_t));
  
  /*
   * alloc lookup array
   */
  
  nlookup = npts_cappi / 4;
  lookup = (dva_lookup_t *) umalloc (nlookup * sizeof(dva_lookup_t));
  
  /*
   * loop through cappis
   */
  
  for (iz = 0; iz < grid.nz; iz ++)  {
    
    /*
     * compute grid position file path
     */
    
    sprintf (grid_file_path, "%s%sgrid_cappi_%02d",
	     Glob->params.grid_files_dir, PATH_DELIM, iz);

    if (Glob->params.debug) {
      fprintf(stderr, "Reading grid file %s\n", grid_file_path);
    }

    /*
     * open position array file and read in
     */
    
    if ((gridfile = fopen(grid_file_path, "rb")) == NULL) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Cannot open grid file\n");
      perror(grid_file_path);
      return (-1);
    }
    if (fread ( pos, sizeof(dva_position_t), npts_cappi, gridfile)
	!= npts_cappi) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Cannot read position array to file %s\n",
	      grid_file_path);
      perror(grid_file_path);
      return (-1);
    }
    fclose(gridfile);

    /*
     * compute lookup table, load up lookup array
     */
    
    compute_lookup(iz, nlookup, &cal, &grid, pos, range, lookup);
    
    /*
     * open lookup array file and write out
     */
    
    sprintf (lookup_file_path, "%s%slookup_cappi_%02d",
	     Glob->params.lookup_files_dir, PATH_DELIM, iz);

    if ((lookupfile = fopen(lookup_file_path, "wb")) == NULL) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Cannot open lookup file for writing\n");
      perror(lookup_file_path);
      return (-1);
    }
    if (fwrite(lookup, sizeof(dva_lookup_t),
	       nlookup, lookupfile) != nlookup) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Cannot write lookup array to file\n");
      perror(lookup_file_path);
      return (-1);
    }
    fclose(lookupfile);
    if (Glob->params.debug) {
      fprintf(stderr, "Lookup file %s written\n", lookup_file_path);
    }
    
  } /* iz */

  /*
   * quit
   */
  
  tidy_and_exit(0);
  return(0);

}

