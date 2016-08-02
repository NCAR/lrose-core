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
#ifdef __cplusplus
 extern "C" {
#endif

/**************************************************************************
 * grid_forecast.h - header file for grid_forecast program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <titan/storm.h>
#include <titan/tdata_index.h>

#define DOBSON_FILE_EXT "dob"
#define ORIGINAL_RDATA_DIR "null"
#define FORECAST_RDATA_DIR "null"
#define TRACK_FILE_PATH "null"
#define MIN_HISTORY_IN_SECS 0L
#define FORECAST_LEAD_TIME 0L
#define DBZ_FIELD 0L

#define DEBUG_STR "false"

#define DATA_MOVED_FLAG 255

/*
 * global struct
 */

typedef struct {

  char *prog_name;          /* program name */
  char *params_path_name;   /* params file path name */

  char *original_rdata_dir;
  char *forecast_rdata_dir;
  char *dobson_file_ext;

  si32 dbz_field;            /* dbz field number */

  si32 min_history_in_secs;  /* min number of secs in history before the
			      * forecast is considered valid */

  si32 forecast_lead_time;   /* margin of error around the forecast
			      * time, within which a scan time must fall
			      * for it to be applicable for the
			      * forecast - secs */

  int debug;                 /* debug flag */

} global_t;

/*
 * declare the global structure locally in the main,
 * and as an extern in all other routines
 */

#ifdef MAIN

global_t *Glob = NULL;

#else

extern global_t *Glob;

#endif

/*
 * functions
 */

extern date_time_t* load_scan_times(storm_file_handle_t *s_handle);

extern void tidy_and_exit(int sig);

extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle);

extern void open_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       char **track_file_paths,
		       si32 file_num);

extern void parse_args(int argc,
		       char **argv,
		       si32 *n_track_files,
		       char ***track_file_paths);

extern void process_track_file(storm_file_handle_t *s_handle,
			       track_file_handle_t *t_handle,
			       vol_file_handle_t *v_handle,
			       date_time_t *scan_time);

extern void read_params(void);

extern void read_volume(vol_file_handle_t *v_handle,
			date_time_t *stime);

extern void update_forecast_grid(storm_file_handle_t *s_handle,
				 vol_file_handle_t *v_handle,
				 ui08 *forecast_grid,
				 si32 n_runs,
				 double forecast_dx,
				 double forecast_dy);

extern void write_volume(vol_file_handle_t *v_handle,
			 date_time_t *stime);
#ifdef __cplusplus
}
#endif
