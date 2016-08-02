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

/****************************************************************************
 * output.c
 *
 * output module - creates output buffers, writes to file
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1995
 *
 ****************************************************************************/

#include "forecast_monitor.h"
#include <toolsa/pjg.h>
#include <dataport/bigend.h>
#include <titan/tdata_server.h>
#include <sys/stat.h>

#define ALLOC_INCR 100

static int Grid_type;
static int N_entries_alloc = 0;

static double Datum_longitude;
static double Datum_latitude;

static storm_file_params_t Sparams;
static tdata_monitor_header_t Header;
static tdata_monitor_entry_t *Entries = NULL;

static void check_entry_buffer(void);

void init_output_module(storm_file_handle_t *s_handle)

{

  Grid_type = s_handle->scan->grid.proj_type;
  Sparams = s_handle->header->params;

  Datum_longitude = s_handle->scan->grid.proj_origin_lon;
  Datum_latitude = s_handle->scan->grid.proj_origin_lat;

  Header.dbz_threshold = (si32) (Sparams.low_dbz_threshold + 0.5);
  Header.min_storm_size = (si32) (Sparams.min_storm_size + 0.5);
  Header.forecast_lead_time = Glob->params.forecast_lead_time;
  Header.angle_scale = DEG_FACTOR;
  
}

void reset_output_module(si32 scan_time)

{

  Header.time = scan_time;
  Header.n_entries = 0;
  
}

void load_output_entry(storm_file_handle_t *s_handle,
		       fm_simple_track_t *strack,
		       double pod, double far, double csi)

{

  double xx, yy;
  double latitude, longitude;
  storm_file_global_props_t *gprops;
  tdata_monitor_entry_t *entry;

  gprops = &strack->verify.gprops;

  /*
   * lat / lon
   */

  xx = gprops->proj_area_centroid_x;
  yy = gprops->proj_area_centroid_y;

  if (Grid_type == MDV_PROJ_FLAT) {
    PJGLatLonPlusDxDy(Datum_latitude, Datum_longitude,
		      xx, yy,
		      &latitude, &longitude);
  } else {
    longitude = xx;
    latitude = yy;
  }

  check_entry_buffer();

  entry = Entries + Header.n_entries;

  entry->longitude = (si32) (longitude * Header.angle_scale + 0.5);
  entry->latitude = (si32) (latitude * Header.angle_scale + 0.5);
  entry->pod = (char) (pod * 100.0 + 0.5);
  entry->far = (char) (far * 100.0 + 0.5);
  entry->csi = (char) (csi * 100.0 + 0.5);

  Header.n_entries++;

  return;

}

void write_output_file(void)

{

  char output_path[MAX_PATH_LEN];
  char tmp_path[MAX_PATH_LEN];
  char tmp_dir[MAX_PATH_LEN];
  char call_str[BUFSIZ];
  int ientry;
  date_time_t scan_time;
  tdata_monitor_header_t hdr;
  tdata_monitor_entry_t entry;
  FILE *tmp_file;

  scan_time.unix_time = Header.time;
  uconvert_from_utime(&scan_time);

  /*
   * make tmp dir
   */

  sprintf(tmp_dir, "%s%stmp",
	  Glob->params.output_dir,
	  PATH_DELIM);

  mkdir (tmp_dir, S_IRWXU | S_IRGRP | S_IROTH);

  /*
   * compute file paths
   */

  sprintf(tmp_path, "%s%s%.4d%.2d%.2d_%.2d%.2d%.2d.%s",
	  tmp_dir,
	  PATH_DELIM,
	  scan_time.year,
	  scan_time.month,
	  scan_time.day,
	  scan_time.hour,
	  scan_time.min,
	  scan_time.sec,
	  Glob->params.output_file_ext);
  
  sprintf(output_path, "%s%s%.4d%.2d%.2d_%.2d%.2d%.2d.%s",
	  Glob->params.output_dir,
	  PATH_DELIM,
	  scan_time.year,
	  scan_time.month,
	  scan_time.day,
	  scan_time.hour,
	  scan_time.min,
	  scan_time.sec,
	  Glob->params.output_file_ext);

  /*
   * open tmp file
   */
	  
  if ((tmp_file = fopen(tmp_path, "w")) == NULL) {
    fprintf(stderr, "ERROR - %s:write_output_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot open tmp file for writing\n");
    perror(tmp_path);
    tidy_and_exit(-1);
  }

  /*
   * write header
   */

  hdr = Header;
  BE_from_array_32((ui32 *) &hdr, (ui32) sizeof(hdr));
  
  if (ufwrite((void *) &Header, sizeof(tdata_monitor_header_t),
	      1, tmp_file) != 1) {
    fprintf(stderr, "ERROR - %s:write_output_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot write header\n");
    perror(tmp_path);
    tidy_and_exit(-1);
  }

  /*
   * write entries
   */

  for (ientry = 0; ientry < Header.n_entries; ientry++) {

    entry = Entries[ientry];

    BE_from_array_32((ui32 *) &entry,
		     (ui32) (NLONGS_TDATA_MONITOR_ENTRY * sizeof(si32)));
    
    if (ufwrite((void *) &entry, sizeof(tdata_monitor_entry_t),
		1, tmp_file) != 1) {
      fprintf(stderr, "ERROR - %s:write_output_file\n", Glob->prog_name);
      fprintf(stderr, "Cannot write entry\n");
      perror(tmp_path);
      tidy_and_exit(-1);
    }

  }

  /*
   * close file
   */

  fclose(tmp_file);

  /*
   * move tmp file to proper place
   */

  sprintf(call_str, "/bin/mv %s %s", tmp_path, output_path);

  if (Glob->params.debug) {
    fprintf(stderr, "%s\n", call_str);
  }

  usystem_call(call_str);

  return;

}

static void check_entry_buffer(void)

{

  if (N_entries_alloc < (Header.n_entries + 1)) {

    N_entries_alloc += ALLOC_INCR;

    if (Entries == NULL) {
      Entries = (tdata_monitor_entry_t *) umalloc
	((ui32) (N_entries_alloc * sizeof(tdata_monitor_entry_t)));
    } else {
      Entries = (tdata_monitor_entry_t *) urealloc
	((char *) Entries,
	 (ui32) (N_entries_alloc * sizeof(tdata_monitor_entry_t)));
    }

  }

}
