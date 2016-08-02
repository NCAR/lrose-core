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
/*********************************************************************
 * process_file()
 *
 * process ac posn file
 *
 * RAP, NCAR, Boulder CO
 *
 * November 1995
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "acmg_acpos_filter.h"

FILE *Out = NULL;
static char Opened_path[MAX_PATH_LEN];

static void store_line(date_time_t *ptime,
		       double lat, double lon,
		       int ispd, int seed_code);

static void close_output_file(void);

void process_file(char *file_path)

{

  char *file_name;
  char line[BUFSIZ];
  char code1[8], code2[8], code3[8];
  char ns_hemi[8], ew_hemi[8];

  int ilat, ilat_dec_min;
  int ilon, ilon_dec_min;
  int ispd, seed_code;

  double lat, lon;

  date_time_t ptime;
  FILE *fp;

  /*
   * get file name from path
   */

  if ((file_name = strrchr(file_path, '/')) != NULL) {
    file_name++;
  } else {
    file_name = file_path;
  }

  if (sscanf(file_name, "%d_%d_%d",
	     &ptime.day, &ptime.month, &ptime.year) != 3) {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "File name must have form like '03_07_95.v02'\n");
    fprintf(stderr, "File name is : '%s'\n", file_name);
    return;
  }

  if (ptime.year < 1900) {
    if (ptime.year < 50) {
      ptime.year += 2000;
    } else {
      ptime.year += 1900;
    }
  }
  
  if ((fp = fopen(file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot open ac posn file for reading\n");
    perror(file_path);
    return;
  }

  while (fgets(line, BUFSIZ, fp) != NULL) {

    if (Glob->params.debug) {
      fprintf(stderr, "%s", line);
    }

    /*
     * line has form:
     *
     * 20:25:10AN 43 5474BE 000 1634D1230
     */
    
    if (sscanf(line, "%2d:%2d:%2d%1s%1s%3d%5d%1s%1s%3d%5d%1s%3d%d",
	       &ptime.hour, &ptime.min, &ptime.sec,
	       code1, ns_hemi, &ilat, &ilat_dec_min,
	       code2, ew_hemi, &ilon, &ilon_dec_min,
	       code3, &ispd, &seed_code) == 14) {
      
      uconvert_to_utime(&ptime);
      
      
      lat = (double) ilat + (double) ilat_dec_min / 6000.0;
      lon = (double) ilon + (double) ilon_dec_min / 6000.0;

      if (!strcmp(ns_hemi, "S")) {
	lat *= -1.0;
      }
      if (!strcmp(ew_hemi, "W")) {
	lon *= -1.0;
      }

      if (Glob->params.debug) {
	fprintf(stderr, "%s %ld %g %g  %d  %d\n",
		utimestr(&ptime),
		ptime.unix_time,
		lat, lon, ispd, seed_code);
      }
      
      store_line(&ptime, lat, lon, ispd, seed_code);
      
    } /* if (sscanf( .... */
    
  } /* while */

  close_output_file();
  fclose(fp);

}
      
static void store_line(date_time_t *ptime,
		       double lat, double lon,
		       int ispd, int seed_code)

{

  static int first_call = TRUE;
  char this_path[MAX_PATH_LEN];

  /*
   * on first call, init Opened_path
   */

  if (first_call) {
    strcpy(Opened_path, "");
    first_call = FALSE;
  }

  /*
   * compute file path
   */

  sprintf(this_path, "%s%s%.4d%.2d%.2d%s", 
	  Glob->params.output_dir,
	  PATH_DELIM,
	  ptime->year, ptime->month, ptime->day,
	  Glob->params.output_file_ext);

  if (Out != NULL) {

    if (strcmp(this_path, Opened_path)) {

      /*
       * names differ close the opened file
       */

      fclose(Out);
      Out = NULL;

    }
    
  } /* if (Out != NULL) */
  
  /*
   * if output file is not open, open it and store path
   * for future use
   */
  
  if (Out == NULL) {
    if ((Out = fopen(this_path, "a")) == NULL) {
      fprintf(stderr, "ERROR - %s:process_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot open output file for appending\n");
      perror(this_path);
      return;
    }
    strcpy(Opened_path, this_path);
  }

  /*
   * write to file
   */

  fprintf(Out, "%8ld %10.5f %10.5f   0.0 %5s %4d %3d\n",
	  ptime->unix_time, lat, lon,
	  Glob->params.call_sign, ispd, seed_code);

  if (Glob->params.debug) {
    fprintf(stderr, "%8ld %10.5f %10.5f   0.0 %5s %4d %3d\n",
	    ptime->unix_time, lat, lon,
	    Glob->params.call_sign, ispd, seed_code);
  }

  return;
  
}

static void close_output_file(void)

{
  fclose (Out);
  return;
}






