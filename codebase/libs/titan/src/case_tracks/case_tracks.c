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
/************************************************************
 * case_tracks.c
 *
 * Seeding case track handling
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * November 1997
 *
 ************************************************************/

#include <titan/case_tracks.h>
#include <toolsa/str.h>
#include <assert.h>

/**************
 * Constructor
 */

void init_case_track_handle(case_track_handle_t *handle,
			    char *prog_name,
			    int debug)
  
{

  MEM_zero(*handle);

  handle->prog_name = STRdup(prog_name);
  handle->debug = debug;

  handle->mbuf = MEMbufCreate();

  handle->init_flag =  CASE_TRACK_HANDLE_INIT_FLAG;

}

/************
 * Destructor
 */

void free_case_track_handle(case_track_handle_t *handle)

{

  STRfree(handle->prog_name);
  MEMbufDelete(handle->mbuf);
  handle->init_flag = 0;

}

/***************
 * Read in cases
 *
 * Returns 0 on success, -1 on failure.
 */

int read_case_track_file(case_track_handle_t *handle,
			 char *file_path)

{

  char line[BUFSIZ];
  char seed_flag[8];

  int num;
  int num_flares;
  int seed_duration;
  int complex_track_num;
  int simple_track_num;
  int ref_minus_start;
  int end_minus_ref;

  double cloud_base;
  double mixing_ratio;
  double temp_ccl;
  double deltat_500mb;

  date_time_t ref_time;
  case_track_t this_case;
  
  FILE *case_file;

  assert(handle->init_flag = CASE_TRACK_HANDLE_INIT_FLAG);
  
  /*
   * open case file
   */
  
  if ((case_file = fopen(file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:read_case_track_file\n",
	    handle->prog_name);
    fprintf(stderr, "Cannot open case file\n");
    perror (file_path);
    return (-1);
  }

  if (handle->debug) {
    fprintf(stderr, "Program %s\n", handle->prog_name);
    fprintf(stderr, "Reading case file '%s'\n", file_path);
  }
  
  /*
   * initialize
   */
  
  MEMbufReset(handle->mbuf);
  handle->n_cases = 0;

  /*
   * read in cases
   */
  
  while (fgets(line, BUFSIZ, case_file) != NULL) {
    
    if (line[0] == '#') {
      continue;
    }
    
    if (sscanf(line,
	       "%d "
	       "%s %d %d"
	       "%d %d %d "
	       "%d %d %d "
	       "%d %d "
	       "%d %d "
	       "%lg %lg "
	       "%lg %lg",
	       &num,
	       seed_flag, &num_flares, &seed_duration,
	       &ref_time.year, &ref_time.month, &ref_time.day,
	       &ref_time.hour, &ref_time.min, &ref_time.sec,
	       &ref_minus_start, &end_minus_ref,
	       &complex_track_num, &simple_track_num,
	       &cloud_base, &mixing_ratio,
	       &temp_ccl, &deltat_500mb) != 18) {
      continue;
    }

    /*
     * ignore negative track numbers
     */

    if (complex_track_num < 0 || simple_track_num < 0) {
      continue;
    }
    
    this_case.num = num;

    if (seed_flag[0] == 'Y' || seed_flag[0] == 'y') {
      this_case.seed_flag = TRUE;
    } else {
      this_case.seed_flag = FALSE;
    }

    this_case.num_flares = num_flares;
    this_case.seed_duration = seed_duration * 60;

    uconvert_to_utime(&ref_time);
    this_case.ref_time = ref_time.unix_time;
    this_case.ref_minus_start = ref_minus_start * 60;
    this_case.end_minus_ref = end_minus_ref * 60;

    this_case.start_time = this_case.ref_time - this_case.ref_minus_start;
    this_case.end_time = this_case.ref_time + this_case.end_minus_ref;
    
    this_case.complex_track_num = complex_track_num;
    this_case.simple_track_num = simple_track_num;

    this_case.cloud_base = cloud_base;
    this_case.mixing_ratio = mixing_ratio;
    this_case.temp_ccl = temp_ccl;
    this_case.deltat_500mb = deltat_500mb;

    if (handle->debug) {

      fprintf(stderr, "Case %3d, ", this_case.num);
      fprintf(stderr, "  Ref_time   %s\n", utimstr(this_case.start_time));
      fprintf(stderr, "  Start_time %s\n", utimstr(this_case.ref_time));
      fprintf(stderr, "  End_time   %s\n", utimstr(this_case.end_time));
      fprintf(stderr,
	      "  seed %s, nflares %2d, dur %2d, "
	      "complex %4d, simple %4d, "
	      "base %5.1f, mratio %5.1f, tccl"
	      "%5.1f, dt500 %5.1f\n",
	      this_case.seed_flag? "Y" : "N",
	      this_case.num_flares,
	      this_case.seed_duration,
	      this_case.complex_track_num,
	      this_case.simple_track_num,
	      this_case.cloud_base,
	      this_case.mixing_ratio,
	      this_case.temp_ccl,
	      this_case.deltat_500mb);

    } /* if (_debug) */

    handle->cases = (case_track_t *)
      MEMbufAdd(handle->mbuf, &this_case, sizeof(case_track_t));
    handle->n_cases++;

  } /* while */

  fclose(case_file);

  if (handle->n_cases > 0) {
    return (0);
  } else {
    return (-1);
  }

}

/*****************
 * Find track case
 *
 * Look through the case tracks for a match.
 *
 * Returns 0 on success, -1 on failure.
 */

int case_tracks_find_case(case_track_handle_t *handle,
			  time_t scan_time,
			  int complex_track_num,
			  int simple_track_num,
			  case_track_t **this_case_p)

{

  int i;
  case_track_t *this_case;

  assert(handle->init_flag = CASE_TRACK_HANDLE_INIT_FLAG);

  this_case = handle->cases;
  for (i = 0; i < handle->n_cases; i++, this_case++) {

    if (this_case->start_time <= scan_time &&
	this_case->end_time >= scan_time) {

      /* time is good */
      
      if (this_case->complex_track_num == complex_track_num &&
	  this_case->simple_track_num == simple_track_num) {

	/* track numbers are good */
	
	*this_case_p = this_case;
	return (0);
	
      }
      
    }
    
  }

  return (-1);

}

/********************
 * case_track_print()
 *
 * Prints out struct.
 *
 * Returns 0 on success, -1 on failure.
 */

void case_track_print(FILE *out,
		      char *spacer,
		      case_track_t *this_case)

{

  fprintf(out, "%sCASE TRACK DATA\n", spacer);
  
  fprintf(out, "%s  num: %d\n", spacer, this_case->num);

  fprintf(out, "%s  seed_flag: %d\n", spacer, this_case->seed_flag);
  fprintf(out, "%s  num_flares: %d\n", spacer, this_case->num_flares);
  fprintf(out, "%s  seed_duration: %d\n", spacer, this_case->seed_duration);

  fprintf(out, "%s  ref_time: %s\n", spacer,
	  utimstr(this_case->ref_time));
  
  fprintf(out, "%s  ref_minus_start: %d\n", spacer,
	  this_case->ref_minus_start);
  fprintf(out, "%s  end_minus_ref: %d\n", spacer,
	  this_case->end_minus_ref);

  fprintf(out, "%s  start_time: %s\n", spacer,
	  utimstr(this_case->start_time));
  fprintf(out, "%s  end_time: %s\n", spacer,
	  utimstr(this_case->end_time));
  
  fprintf(out, "%s  complex_track_num: %d\n", spacer,
	  this_case->complex_track_num);
  fprintf(out, "%s  simple_track_num: %d\n", spacer,
	  this_case->simple_track_num);
  
  fprintf(out, "%s  cloud_base: %g\n", spacer, this_case->cloud_base);
  fprintf(out, "%s  mixing_ratio: %g\n", spacer, this_case->mixing_ratio);
  fprintf(out, "%s  temp_ccl: %g\n", spacer, this_case->temp_ccl);
  fprintf(out, "%s  deltat_500mb: %g\n", spacer, this_case->deltat_500mb);

}
