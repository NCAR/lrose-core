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
 * verify_day.h - header file for verify_day program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <titan/track.h>
#include <time.h>


#define NX (si32) 100
#define NY (si32) 100

#define MINX -50.0
#define MINY -50.0

#define DX 1.0
#define DY 1.0

#define VERIFY_DIR "/home/dixon/verify"
#define TRACK_FILE_PATH " "

#define VERIFY_FILE_EXT "dob"

#define MIN_VALID_HISTORY (si32) 3

#define FORECAST_LEAD_TIME 30.0
#define FORECAST_LEAD_TIME_MARGIN 1.0

#define ELLIPSE_RADIUS_RATIO 1.0

#define DEBUG_STR "false"

#define MODE_STR "ellipse"
#define ELLIPSE_MODE 0
#define RUNS_MODE 1

#define VERIFICATION_FIELD "valid_storms"

#define N_VERIFICATION_FIELDS 2
#define ALL_STORMS_FIELD 0
#define VALID_STORMS_FIELD 1

/*
 * global struct
 */

typedef struct {

  char *prog_name;          /* program name */
  char *params_path_name;   /* params file path name */

  char *verify_dir;         /* verification directory path */
  char *verify_file_ext;    /* extension for the verification
			     * file */

  si32 min_valid_history;   /* min number of scans in history before the
			     * forecast is considered valid */

  double forecast_lead_time;  /* input as mins, stored as secs */

  double forecast_lead_time_margin; /* margin of error around the forecast
				     * time, within which a scan time must fall
				     * for it to be applicable for the
				     * forecast - input as mins, stored
				     * as secs */

  si32 verification_field;  /* ALL_STORMS_FIELD or VALID_STORM_FIELD */

  si32 nx, ny;              /* cartesian grid size */

  double dx, dy;            /* cartesian grid deltas */

  double minx, miny;        /* cartesian grid start values */

  double ellipse_radius_ratio; /* the ratio of the radius of the ellipse
				* used for verification to the
				* projected area ellipse */

  int mode;                  /* ELLIPSE_MODE or RUNS_MODE */

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

extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle);

extern void load_cont_data(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle,
			   time_t *scan_time,
			   ucont_table_t *cont_data);
     
extern time_t* load_scan_times(storm_file_handle_t *s_handle);

extern void open_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       char **track_file_paths,
		       si32 file_num);

extern void parse_args(int argc,
		       char **argv,
		       si32 *n_track_files,
		       char ***track_file_paths);

extern void read_params(void);

extern int read_verification_file(storm_file_scan_header_t *scan,
				  time_t scan_time,
				  ui08 **verify_grid);

extern void tidy_and_exit(int sig);

extern void update_ellipse_grid(double ellipse_x,
				double ellipse_y,
				double major_radius,
				double minor_radius,
				double axis_rotation,
				ui08 **forecast_grid);

extern void update_runs_grid(storm_file_handle_t *s_handle,
			     si32 n_runs,
			     double forecast_dx,
			     double forecast_dy,
			     ui08 **forecast_grid);
#ifdef __cplusplus
}
#endif
