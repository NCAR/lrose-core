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
 * process_stream.c
 *
 * Process the incoming data stream
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * June 1996
 *
 ****************************************************************************/

#include "wmi_ac_ingest.h"
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <symprod/spdb_client.h>

#define NFIELDS 20
#define FIELD_LEN {4, 7, 8, 3, 4, 4, 4, 3, 4, 4, 1, 1, 1, 1, 3, 3, 2, 4, 3, 4}

static FILE *Archive_file = NULL;
static FILE *Realtime_file = NULL;
static char Archive_path[MAX_PATH_LEN];
static spdb_handle_t *Db_array = NULL;

static void add_spaces(char *line, char *spaced_line);
static int ac_index(char *callsign);

static void store_line(date_time_t *ptime,
		       double lat,
		       double lon,
		       double alt,
		       char *callsign,
		       int ispd,
		       double tdry,
		       double lwjw,
		       double dewpt,
		       int left_burn,
		       int right_burn,
		       int flare_state,
		       int snowmax_state);

static void store_single_spdb(date_time_t *ptime,
			      double lat,
			      double lon,
			      double alt,
			      char *callsign);

static void store_mult_spdb(date_time_t *ptime,
			    double lat,
			    double lon,
			    double alt,
			    char *callsign);

void process_realtime_stream(FILE *tty_file)

{

  int forever = TRUE;
  char line[BUFSIZ];
  char spaced_line[BUFSIZ];

  char callsign[8];

  int ilat, ilon;
  int itas, igps_alt, ipres_alt;
  int itdry, ilwjw, idewpt;
  int ice_count;
  int left_burn, right_burn, flare_state, snowmax_state;
  int igps_var, igps_err, iarnav_warn;

  date_time_t gps_time, wall_time;
  time_t last_store = 0;
  
  while (forever) {
    
    /*
     * wait for data, registering once per second while waiting
     */

    while (ta_read_select(tty_file, 1000) != 1) {
      PMU_auto_register("Waiting for data");
    }

    PMU_auto_register("Got data");

    if (fgets(line, BUFSIZ, tty_file) == NULL) {
      fprintf(stderr, "ERROR - %s:process_stream\n", Glob->prog_name);
      perror(Glob->params.input_device);
      return;
    }

    add_spaces(line, spaced_line);

    if (Glob->params.debug) {
      fprintf(stderr, "Input line:\n%s", line);
      fprintf(stderr, "Spaced line:\n%s\n", spaced_line);
    }
    
    if (sscanf(spaced_line,
	       "%s %d %d %d %d %d %d %d %d %d"
	       "%d %d %d %d %2d %2d %2d %d %d %d",
	       callsign, &ilat, &ilon, &itas, &igps_alt, &ipres_alt,
	       &itdry, &ilwjw, &idewpt, &ice_count,
	       &left_burn, &right_burn, &flare_state, &snowmax_state,
	       &gps_time.hour, &gps_time.min, &gps_time.sec,
	       &igps_var, &igps_err, &iarnav_warn) == 20) {

      if (Glob->params.debug) {

	fprintf(stderr, "Decoding   : %s", line);
	fprintf(stderr, "Callsign   : %s\n", callsign);
	fprintf(stderr, "lat        : %g\n", ilat / 10000.0);
	fprintf(stderr, "lon        : %g\n", ilon / 10000.0);
	fprintf(stderr, "tas        : %d\n", itas);
	fprintf(stderr, "gps_alt    : %g\n", igps_alt / 100.0);
	fprintf(stderr, "pres_alt   : %g\n", ipres_alt / 100.0);
	fprintf(stderr, "tdry       : %g\n", itdry / 10.0);
	fprintf(stderr, "lwJW       : %g\n", ilwjw / 100.0);
	fprintf(stderr, "dewpt      : %g\n", idewpt / 10.0);
	fprintf(stderr, "left burn  : %d\n", left_burn);
	fprintf(stderr, "right burn : %d\n", right_burn);
	fprintf(stderr, "flare state: %d\n", flare_state);
	fprintf(stderr, "snow state : %d\n", snowmax_state);
	fprintf(stderr, "time       : %.2d:%.2d:%.2d\n",
		gps_time.hour, gps_time.min, gps_time.sec);
	fprintf(stderr, "gps var    : %d\n", igps_var);
	fprintf(stderr, "gps err    : %d\n", igps_err);
	fprintf(stderr, "arnav code : %d\n", iarnav_warn);
	
      }

      /*
       * substitute wall clock time for gps time
       */

      wall_time.unix_time = time(NULL);
      uconvert_from_utime(&wall_time);

      if((wall_time.unix_time - last_store) >= Glob->params.data_interval) {

	store_line(&wall_time,
		   (double) ilat / 10000.0,
		   (double) ilon / 10000.0,
		   (double) ipres_alt / 100.0,
		   callsign,
		   itas,
		   (double) itdry / 10.0,
		   (double) ilwjw / 100.0,
		   (double) idewpt / 10.0,
		   left_burn,
		   right_burn,
		   flare_state,
		   snowmax_state);

	if (Glob->params.use_spdb) {

	  if (Glob->params.single_database) {

	    store_single_spdb(&wall_time,
			      (double) ilat / 10000.0,
			      (double) ilon / 10000.0,
			      (double) ipres_alt / 100.0,
			      callsign);
	    
	  } else {

	    store_mult_spdb(&wall_time,
			    (double) ilat / 10000.0,
			    (double) ilon / 10000.0,
			    (double) ipres_alt / 100.0,
			    callsign);

	  }

	} /* if (Glob->params.use_spdb) */

	last_store = wall_time.unix_time;

      } /* if((wall_time.unix_time - last_store) ... */

    } else {
      
      if (Glob->params.debug) {
	fprintf(stderr, "WARNING - %s:process_stream\n", Glob->prog_name);
	fprintf(stderr, "Cannot decode: %s\n", (char *) spaced_line);
      }
      
    }

  } /* while (forever) */

}

void process_test_stream(void)

{

  int forever = TRUE;
  char *callsign;
  double lat, lon, alt;

  date_time_t wall_time;
  time_t last_store = 0;

  init_test_data();

  while (forever) {
    
    PMU_auto_register("Got data");
    
    get_test_data(&callsign, &lat, &lon, &alt);
    
    /*
     * substitute wall clock time for gps time
     */
    
    wall_time.unix_time = time(NULL);
    uconvert_from_utime(&wall_time);

    if((wall_time.unix_time - last_store) >= Glob->params.data_interval) {

      store_line(&wall_time, lat, lon, alt, callsign,
		 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0);

      if (Glob->params.use_spdb) {
	if (Glob->params.single_database) {
	  store_single_spdb(&wall_time, lat, lon, alt, callsign);
	} else {
	  store_mult_spdb(&wall_time, lat, lon, alt, callsign);
	}
      } /* if (Glob->params.use_spdb) */
      
      last_store = wall_time.unix_time;

    } /* if((wall_time.unix_time - last_store) ... */
    
    sleep(1);
    
  } /* while (forever) */

  free_test_data();

}

/******************************************************
 * space the fields in the input line for later parsing
 */

static void add_spaces(char *line, char *spaced_line)

{

  int i, ifield;
  int pos = 0;
  int spaced_pos = 0;
  int field_len[NFIELDS] = FIELD_LEN;

  for (ifield = 0; ifield < NFIELDS; ifield++) {

    for (i = 0; i < field_len[ifield]; i++) {
      if (line[pos] != ':') {
	spaced_line[spaced_pos] = line[pos];
	spaced_pos++;
      }
      pos++;
    }

    spaced_line[spaced_pos] = ' ';
    spaced_pos++;
    
  }
  
  spaced_line[spaced_pos] = '\0';

}

/**************
 * store_line()
 */

static void store_line(date_time_t *ptime,
		       double lat,
		       double lon,
		       double alt,
		       char *callsign,
		       int ispd,
		       double tdry,
		       double lwjw,
		       double dewpt,
		       int left_burn,
		       int right_burn,
		       int flare_state,
		       int snowmax_state)

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

    /*
     * open fresh realtime file for writing
     */

    if ((Realtime_file = fopen(Glob->params.realtime_file_path, "w"))
	== NULL) {
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot open realtime file for writing\n");
      perror(Glob->params.realtime_file_path);
      return;
    }
    strcpy(Archive_path, arch_path);
  }

  /*
   * write to file
   */

  fprintf(Archive_file, "%8ld %9.4f %9.4f %6.3f %5s %4d "
	  "%6.2f %5.2f %6.2f %d %d %d %d\n",
	  ptime->unix_time, lat, lon,
	  alt, callsign, ispd,
	  tdry, lwjw, dewpt,
	  left_burn, right_burn,
	  flare_state, snowmax_state); 

  fflush(Archive_file);

  fprintf(Realtime_file, "%8ld %9.4f %9.4f %6.3f %5s %4d "
	  "%6.2f %6.3f %6.2f %d %d %d %d\n",
	  ptime->unix_time, lat, lon,
	  alt, callsign, ispd,
	  tdry, lwjw, dewpt,
	  left_burn, right_burn,
	  flare_state, snowmax_state); 

  fflush(Realtime_file);

  return;
  
}

/*******************************************
 * store_single_spdb()
 *
 * Store data in single database directory, db_dir.
 *
 * The chunk type is computed by hashing the callsign.
 */

static void store_single_spdb(date_time_t *ptime,
			      double lat,
			      double lon,
			      double alt,
			      char *callsign)

{

  ac_posn_t posn;
  si32 chunk_type;
  spdb_chunk_ref_t chunk_ref;

  /*
   * load struct
   */
  
  posn.lat = lat;
  posn.lon = lon;
  posn.alt = alt;
  STRncopy(posn.callsign, callsign, AC_POSN_N_CALLSIGN);

  /*
   * set to bigend format
   */
  
  BE_from_ac_posn(&posn);

  /*
   * compute chunk type
   */

  chunk_type = SPDB_4chars_to_int32(callsign);

  /*
   * load chunk ref
   */

  chunk_ref.valid_time = ptime->unix_time;
  chunk_ref.expire_time = ptime->unix_time + Glob->params.valid_period;
  chunk_ref.data_type = chunk_type;
  chunk_ref.offset = 0;
  chunk_ref.len = sizeof(ac_posn_t);
    
  /*
   * store in data base
   */
  
  if (SPDB_put_over(Glob->params.db_dir,
		    AC_POSN_PROD_CODE, AC_POSN_PROD_LABEL, 1,
		    &chunk_ref,
		    &posn, sizeof(ac_posn_t))) {
    fprintf(stderr, "WARNING %s:store_single_spdb\n", Glob->prog_name);
    fprintf(stderr, "Could not store posn for ac callsign %s\n", callsign);
  }
  
  return;
  
}

/*******************************************
 * store_mult_spdb()
 *
 * Store data in multiple database directories.
 * The directory is computed as 'db_dir/callsign'
 */

static void store_mult_spdb(date_time_t *ptime,
			    double lat,
			    double lon,
			    double alt,
			    char *callsign)

{

  static int first_call = TRUE;
  char db_path[MAX_PATH_LEN];
  int i, index;
  ac_posn_t posn;

  if (first_call) {

    Db_array = (spdb_handle_t *)
      ucalloc (Glob->params.callsigns.len, sizeof(spdb_handle_t));

    for (i = 0; i < Glob->params.callsigns.len; i++) {
      
      sprintf(db_path, "%s%s%s", Glob->params.db_dir,
	      PATH_DELIM, Glob->params.callsigns.val[i]);
      
      if (SPDB_init(&Db_array[i],
		    AC_POSN_PROD_LABEL, AC_POSN_PROD_CODE,
		    db_path)) {
	fprintf(stderr, "WARNING %s:store_mult_spdb\n", Glob->prog_name);
	fprintf(stderr, "Cannot open ac_posn data base %s\n", db_path);
	tidy_and_exit(-1);
      }
      
    } /* i */

    first_call = FALSE;

  } /* first_call */

  /*
   * get database index
   */

  index = ac_index(callsign);

  if (index < 0) {
    fprintf(stderr, "WARNING %s:store_mult_spdb\n", Glob->prog_name);
    fprintf(stderr, "Ac callsign %s not recognized\n", callsign);
    return;
  }

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
    
  if (SPDB_store_over(&Db_array[index], 0,
		      ptime->unix_time,
		      ptime->unix_time + Glob->params.valid_period,
		      (void *) &posn, sizeof(ac_posn_t))) {
    fprintf(stderr, "WARNING %s:store_mult_spdb\n", Glob->prog_name);
    fprintf(stderr, "Could not store posn for ac callsign %s\n", callsign);
  }

  return;
  
}

/******************************************
 * ac_index() - gets ac index from callsign
 *
 * Returns index on success, -1 on failure
 */

static int ac_index(char *callsign)

{
  
  int i;

  for (i = 0; i < Glob->params.callsigns.len; i++) {
    if (!strcmp(Glob->params.callsigns.val[i], callsign)) {
      return (i);
    }
  }

  return (-1);

}

/**********************
 * close_output_files()
 *
 * cleanup
 */

void close_output_files(void)

{

  int i;

  if (Archive_file != NULL) {
    fclose (Archive_file);
    Archive_file = NULL;
  }

  if (Realtime_file != NULL) {
    fclose (Realtime_file);
    Realtime_file = NULL;
  }

  if (Db_array != NULL) {
    for (i = 0; i < Glob->params.callsigns.len; i++) {
      SPDB_free(&Db_array[i]);
    }
  }

  return;

}






