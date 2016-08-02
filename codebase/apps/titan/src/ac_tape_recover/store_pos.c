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
 * store_pos.c
 *
 * Store the aircraft position
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * August 1997
 *
 ****************************************************************************/

#include "ac_tape_recover.h"
#include <symprod/spdb.h>
#include <toolsa/str.h>

static FILE *Archive_file = NULL;
static char Archive_path[MAX_PATH_LEN];
static spdb_handle_t Db;

static void store_line(date_time_t *ptime,
		       double lat,
		       double lon,
		       double alt,
		       char *callsign);

static void store_spdb(date_time_t *ptime,
		       double lat,
		       double lon,
		       double alt,
		       char *callsign);

void store_pos(date_time_t *ptime,
	       double lat,
	       double lon,
	       double alt,
	       char *callsign)
     
{

  static time_t last_store = 0;
  
  if((ptime->unix_time - last_store) >= Glob->params.data_interval) {

    if (Glob->params.debug || !Glob->store) {
      fprintf(stdout,"%s %g %g %g %s \n",
	      utimestr(ptime),lat,lon,alt,callsign);
      fflush(stdout);
    }

    if (Glob->store) {

      store_line(ptime, lat, lon, alt, callsign);
      
      if (Glob->params.use_spdb) {
	store_spdb(ptime, lat, lon, alt, callsign);
      }

    }

    last_store = ptime->unix_time;
  
  }

}

/**************
 * store_line()
 */

static void store_line(date_time_t *ptime,
		       double lat,
		       double lon,
		       double alt,
		       char *callsign)
     
{

  static int first_call = TRUE;
  char arch_path[MAX_PATH_LEN];
  
  /*
   * on first call, init Archive_path
   */

  if (first_call) {
    strcpy(Archive_path, "");
    first_call = FALSE;
  }

  /*
   * compute file path
   */

  sprintf(arch_path, "%s%s%.4d%.2d%.2d", 
	  Glob->params.archive_dir_path,
	  PATH_DELIM,
	  ptime->year, ptime->month, ptime->day);
  
  if (Archive_file != NULL) {

    if (strcmp(arch_path, Archive_path)) {

      /*
       * names differ, so close the opened files
       */

      close_output_files();

    }
    
  } /* if (Archive_file != NULL) */
  
  /*
   * if output file is not open, open it and store path
   * for future use
   */
  
  if (Archive_file == NULL) {

    /*
     * open archive file for appending
     */
    
    if ((Archive_file = fopen(arch_path, "a")) == NULL) {
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot open archive file for appending\n");
      perror(arch_path);
      return;
    }

    strcpy(Archive_path, arch_path);
  }

  /*
   * write to file
   */

  fprintf(Archive_file, "%8ld %9.4f %9.4f %6.3f %5s\n",
	  ptime->unix_time, lat, lon,
	  alt, callsign);
  
  fflush(Archive_file);
  
  return;
  
}

/**************
 * store_spdb()
 */

static void store_spdb(date_time_t *ptime,
		       double lat,
		       double lon,
		       double alt,
		       char *callsign)

{

  static int first_call = TRUE;
  char db_path[MAX_PATH_LEN];
  ac_posn_t posn;

  if (first_call) {

    sprintf(db_path, "%s%s%s", Glob->params.db_dir,
	    PATH_DELIM, Glob->params.callsign);
      
    if (SPDB_init(&Db,
		  AC_POSN_PROD_LABEL, AC_POSN_PROD_CODE,
		  db_path)) {
      fprintf(stderr, "WARNING %s:store_spdb\n", Glob->prog_name);
      fprintf(stderr, "Cannot open ac_posn data base %s\n", db_path);
      exit(-1);
    }
    
    first_call = FALSE;

  } /* first_call */

  posn.lat = lat;
  posn.lon = lon;
  posn.alt = alt;
  STRncopy(posn.callsign, callsign, AC_POSN_N_CALLSIGN);

  /*
   * set to bigend format
   */
  
  BE_from_ac_posn(&posn);

  /*
   * store in data base
   */
    
  if (SPDB_store_over(&Db, 0,
		      ptime->unix_time,
		      ptime->unix_time + Glob->params.valid_period,
		      (void *) &posn, sizeof(ac_posn_t))) {
    fprintf(stderr, "WARNING %s:store_spdb\n", Glob->prog_name);
    fprintf(stderr, "Could not store posn for ac callsign %s\n", callsign);
  }

  return;
  
}

/**********************
 * close_output_files()
 *
 * cleanup
 */

void close_output_files(void)

{

  if (Archive_file != NULL) {
    fclose (Archive_file);
    Archive_file = NULL;
  }

  SPDB_free(&Db);

  return;

}

