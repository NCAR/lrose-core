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
 * process.c
 *
 * Processing loop
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Nov 1996
 *
 ****************************************************************************/

#include "dva_cart.h"
#include <toolsa/ldata_info.h>
#include <sys/stat.h>

static int read_beam_file(char *beam_file_path,
			  int *nbeams_p,
			  bprp_beam_t **beam_array_p);
     
void process(dva_grid_t *grid,
	     dva_rdas_cal_t *cal,
	     int n_elev,
	     double *elevations)
     
{
  
  static int first_call = TRUE;
  static LDATA_handle_t ldata;

  char beam_file_path[MAX_PATH_LEN];
  int nbeams;
  bprp_beam_t *beam_array = NULL;
 
  int forever = TRUE;

  if (first_call) {
    LDATA_init_handle(&ldata,
		      Glob->prog_name,
		      (Glob->params.debug > DEBUG_NORM));
    first_call = FALSE;
  }
  
  while (forever) {
    
    /*
     * get latest data info, set time.
     * blocks until new data available.
     */
    
    LDATA_info_read_blocking(&ldata,
			     Glob->params.beam_dir,
			     Glob->params.max_realtime_valid_age,
			     5000,
			     PMU_auto_register);
      
    /*
     * new data, compute latest beam file path
     */
    
    sprintf(beam_file_path, "%s%s%s.%s",
	    Glob->params.beam_dir, PATH_DELIM,
	    ldata.info.user_info_1, ldata.info.file_ext);

    if (Glob->params.debug) {
      fprintf(stderr, "Processing file %s \n", beam_file_path);
    }

    if (read_beam_file(beam_file_path, &nbeams, &beam_array) == 0) {
      if (cartesianize(nbeams, beam_array, grid, cal, n_elev, elevations)) {
	tidy_and_exit(-1);
      }
    }
    
  } /* while */

}

/******************
 * read_beam_file()
 *
 * reads in beam file, allocating array space as required.
 *
 * returns 0 on success, -1 on failure.
 */

static int read_beam_file(char *beam_file_path,
			  int *nbeams_p,
			  bprp_beam_t **beam_array_p)
     
{
 
  struct stat file_stat;
  FILE *bfile;
 
  /*
   * get file status
   */
 
  if (stat(beam_file_path, &file_stat) < 0) {
    fprintf(stderr, "ERROR - %s:process\n", Glob->prog_name);
    fprintf(stderr, "Cannot stat beam file\n");
    perror(beam_file_path);
    return (-1);
  }

  /*
   * allocate beam array
   */

  *beam_array_p = urealloc(*beam_array_p, file_stat.st_size);

  /*
   * open file and read in
   */

  if ((bfile = fopen(beam_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:process\n", Glob->prog_name);
    fprintf(stderr, "Cannot open beam file\n");
    perror(beam_file_path);
    return (-1);
  }

  if (fread(*beam_array_p, file_stat.st_size, 1, bfile) != 1) {
    fprintf(stderr, "ERROR - %s:process\n", Glob->prog_name);
    fprintf(stderr, "Cannot read beam file array\n");
    perror(beam_file_path);
    return (-1);
  }

  fclose(bfile);

  if ((file_stat.st_size % sizeof(bprp_beam_t)) != 0) {
    fprintf(stderr, "WARNING - %s:process\n", Glob->prog_name);
    fprintf(stderr, "Beam file does not end on exact beam boundary,\n");
    fprintf(stderr, "%s\n", beam_file_path);
  }


  *nbeams_p = file_stat.st_size / sizeof(bprp_beam_t);

  if (Glob->params.debug) {
    fprintf(stderr, "nbeams: %d\n", *nbeams_p);
  }

  return (0);

}

