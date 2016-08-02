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
 * handle_status.c
 *
 * Handle status data. Write out status file.
 *
 * Mike Dixon  RAP NCAR Boulder CO USA
 *
 * November 1996
 *
 * returns 0 on success, -1 on failure.
 *
 ************************************************************************/

#include "dva_ingest.h"

static void write_status(FILE *out, bprp_status_t *stat,
			 int nbeams_last_vol);

int handle_status(bprp_beam_t *beam, int nbeams_last_vol)

{

  char file_path[MAX_PATH_LEN];
  FILE *status_file;
  date_time_t now;

  PMU_auto_register("In handle_status");

  now.unix_time = time(NULL);
  uconvert_from_utime(&now);
  
  sprintf(file_path, "%s%s%s.%.4d%.2d%.2d.%.2d%.2d%.2d",
	  Glob->params.status_dir, PATH_DELIM,
	  Glob->params.status_file_name,
	  now.year, now.month, now.day,
	  now.hour, now.min, now.sec);
  
  if ((status_file = fopen(file_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:handle_status\n", Glob->prog_name);
    fprintf(stderr, "Cannot open file for status\n");
    perror(file_path);
    return(-1);
  }

  write_status(status_file, (bprp_status_t *) beam, nbeams_last_vol);

  if (Glob->params.debug) {
    write_status(stderr, (bprp_status_t *) beam, nbeams_last_vol);
  }

  fclose(status_file);

  return (0);

}

static void write_status(FILE *out, bprp_status_t *stat,
			 int nbeams_last_vol)

{
  
  ui08 flag0;
  time_t now;
  struct tm *tms;
  
  now = time(NULL);
  tms = localtime(&now);

  fprintf(out, "RADAR STATUS\n");
  fprintf(out, "============\n\n");
  fprintf(out, "Radar name: %s\n", Glob->params.radar_name);
  fprintf(out, "Time: %s", asctime(tms));
  fprintf(out, "Nbeams last vol: %d\n", nbeams_last_vol);
  fprintf(out, "\n");
  
  flag0 = stat->flags[1];
  
  fprintf(out, "%-40s: %s\n",
	  "UPS INVERTER",
	  (flag0 & 1)? "ON" : "OFF");

  fprintf(out, "%-40s: %s\n",
	  "UPS BYPASS",
	  ((flag0 >> 1) & 1)? "ON" : "OFF");

  fprintf(out, "%-40s: %s\n",
	  "DIESEL GENERATOR A CONTACTOR",
	  ((flag0 >> 2) & 1)? "OFF" : "ON");

  fprintf(out, "%-40s: %s\n",
	  "DIESEL GENERATOR B CONTACTOR",
	  ((flag0 >> 3) & 1)? "OFF" : "ON");

  fprintf(out, "%-40s: %s\n",
	  "CH2 WAVEGUIDE PRESSURE",
	  ((flag0 >> 4) & 1)? "OK" : "LOW");

  fprintf(out, "%-40s: %s\n",
	  "CH1 WAVEGUIDE PRESSURE",
	  ((flag0 >> 5) & 1)? "OK" : "LOW");

  fprintf(out, "%-40s: %s\n",
	  "CH2 OVERALL STATUS",
	  ((flag0 >> 6) & 1)? "FAULT" : "GOOD");

  fprintf(out, "%-40s: %s\n",
	  "CH1 OVERALL STATUS",
	  ((flag0 >> 7) & 1)? "FAULT" : "GOOD");

}
