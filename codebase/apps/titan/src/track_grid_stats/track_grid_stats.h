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
 * track_grid_stats.h - header file for track_grid_stats program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <titan/track.h>
#include <tdrp/tdrp.h>
#include "track_grid_stats_tdrp.h"

/*
 * struct for grid stats
 */

#define N_STATS_FIELDS 20

#define N_EVENTS_POS 0
#define N_WEIGHTED_POS 1
#define N_COMPLEX_POS 2
#define PERCENT_ACTIVITY_POS 3
#define N_START_POS 4
#define N_MID_POS 5
#define PRECIP_POS 6
#define VOLUME_POS 7
#define DBZ_MAX_POS 8
#define TOPS_POS 9
#define SPEED_POS 10
#define U_POS 11
#define V_POS 12
#define DISTANCE_POS 13
#define DX_POS 14
#define DY_POS 15
#define DUR_MAX_PRECIP_POS 16
#define AREA_POS 17
#define DURATION_POS 18
#define LN_AREA_POS 19

typedef struct {

  double n_events;
  double n_weighted;
  double n_complex;
  double percent_activity;
  double n_start;
  double n_mid;
  double precip;
  double volume;
  double dbz_max;
  double tops;
  double speed;
  double u;
  double v;
  double distance;
  double dx;
  double dy;
  double dur_max_precip;
  double area;
  double duration;
  double ln_area;

} grid_stats_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */
  TDRPtable *table;               /* TDRP parsing table */
  track_grid_stats_tdrp_struct params;  /* parameter struct */

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

extern void compute_stats(vol_file_handle_t *v_handle,
			  grid_stats_t **stats,
			  si32 n_scans_total);

extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle);

extern void load_stats_grid(storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle,
			    grid_stats_t **stats);

extern void open_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       char *track_file_path);

extern void parse_args(int argc,
                       char **argv,
                       int *check_params_p,
                       int *print_params_p,
                       tdrp_override_t *override,
                       char **params_file_path_p,
                       si32 *n_track_files_p,
                       char ***track_file_paths_p);

extern void process_file(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 char *track_file_path,
			 grid_stats_t **stats,
			 date_time_t *data_start,
			 date_time_t *data_end,
			 si32 *n_scans_total);

extern void tidy_and_exit(int sig);

extern void write_stats_file(vol_file_handle_t *v_handle,
			     date_time_t *data_start,
			     date_time_t *data_end);


#ifdef __cplusplus
}
#endif
