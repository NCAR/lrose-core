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
 * tracks_to_ascii.h - header file for tracks_to_ascii program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/track.h>

#define TRACK_FILE_PATH " "
#define DEBUG_STR "false"

#define STORM_COUNT_ONLY "false"
#define USE_SIMPLE_TRACKS "true"
#define USE_COMPLEX_TRACKS "true"
#define USE_BOX_LIMITS "false"
#define NONZERO_VERIFICATION_ONLY "false"
#define TARGET_ENTITY "track_entry"
#define PRINT_POLYGONS "false"

#define BOX_MIN_X -150.0
#define BOX_MIN_Y -150.0
#define BOX_MAX_X 150.0
#define BOX_MAX_Y 150.0

#define MIN_PERCENT_IN_BOX 100.0
#define MIN_NSTORMS_IN_BOX (si32) 1

#define SAMPLE_INTERVAL 10.0
#define SCAN_INTERVAL 6.0
#define MIN_DURATION 1L

#define NSCANS_PRE_TREND 5L
#define MIN_NSCANS_PRE_TREND 2L
#define NSCANS_POST_TREND 5L
#define MIN_NSCANS_POST_TREND 2L
#define MIN_NSCANS_MONOTONIC 2L

#define VOL_PERCENTILE 100.0
#define DBZ_FOR_MAX_HT 45.0
#define DBZ_FOR_PERCENT_VOL_ABOVE 45.0

#define MAX_TOP_MISSING 0L
#define MAX_RANGE_LIMITED 0L
#define MAX_VOL_AT_START_OF_SAMPLING 0L
#define MAX_VOL_AT_END_OF_SAMPLING 0L

#define COMPLETE_TRACK 0
#define TRACK_ENTRY 1
#define TRENDS 2

/*
 * structs for correlation coefficients
 */

#define NLAGS 6

typedef struct {

  double sumx;
  double sumx2;
  double sumy;
  double sumy2;
  double sumxy;
  double n;
  double corr;
  double val;
  int val_available;

} corr_t;

typedef struct {
  corr_t lag[NLAGS];
} autocorr_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;          /* program name */
  char *params_path_name;   /* params file path name */

  int target_entity;        /* COMPLETE_TRACK, TRACK_ENTRY or
			     * TRENDS  */

  int debug;                /* debug flag */

  int storm_count_only;     /* TRUE or FALSE */

  int use_simple_tracks;    /* TRUE or FALSE */
  int use_complex_tracks;   /* TRUE or FALSE */
  int use_box_limits;       /* TRUE or FALSE */

  int nonzero_verification_only; /* TRUE or FALSE - if TRUE,
				  * only tracks which have reasonable
				  * pod, csi and far values will
				  * be printed out */

  int print_polygons;       /* option to add polygon data to the
			     * entry printout */

  si32 box_min_x;           /* box coordinate limit */
  si32 box_min_y;           /* box coordinate limit */
  si32 box_max_x;           /* box coordinate limit */
  si32 box_max_y;           /* box coordinate limit */

  si32 min_duration;        /* the min duration_in_secs for a track
			     * to be included in the anlaysis */

  double min_percent_in_box; /* the percent of a track which must be
			      * in the box for the track to be
			      * included - target_entity set to
			      * COMPLETE_TRACK only */

  si32 min_nstorms_in_box;   /* the number of storms of a track which
			      * must be in the box for the track to be
			      * included - target_entity set to
			      * COMPLETE_TRACK only */

  si32 max_top_missing;     /* max number of scans in track for which 
			     * storm top was missing */
  si32 max_range_limited;   /* max number of scans in track for which
			     * storm was range limited */

  si32 max_vol_at_start_of_sampling; /* max allowable volume for 
				      * storm in track at start
				      * of radar operations */
  
  si32 max_vol_at_end_of_sampling;   /* max allowable volume for 
				      * storm in track at end
				      * of radar operations */

  si32 sample_interval;     /* input as mins, stored as secs -
			     * used by track_entry mode only  - 
			     * if -1 then all entries are printed */

  si32 scan_interval;       /* input as mins, stored as secs -
			       used by track_entry mode only */

  si32 nscans_post_trend;
  si32 nscans_pre_trend;    /* number of scans used to determine the
			     * trend in the data, unless the scan is
			     * close to the start or end of the
			     * track, in which case min_scans_trend
			     * is used */

  si32 min_nscans_post_trend;
  si32 min_nscans_pre_trend; /* min number of scans used to determine the
			      * trend when close to start or end of track */

  si32 min_nscans_monotonic; /* min number of scans over which the
			      * variable must be either monotonically
			      * increasing or decreasing for the
			      * trend to be valid */

  double vol_percentile;
  double dbz_for_max_ht;
  double dbz_for_percent_vol_above;

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
 * function prototypes
 */

extern void compute_autocorrelation(autocorr_t *autocorr);

extern void entry_comps(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle);

extern void finalize_track_comps(storm_file_handle_t *s_handle,
				 track_file_handle_t *t_handle,
				 autocorr_t *autocorr_volume,
				 autocorr_t *autocorr_vol_two_thirds,
				 autocorr_t *autocorr_precip_area,
				 autocorr_t *autocorr_proj_area);

extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle);

extern void initialize_track_comps(storm_file_handle_t *s_handle,
				   track_file_handle_t *t_handle);

extern void open_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       char **track_file_paths,
		       si32 file_num);

extern void parse_args(int argc,
		       char **argv,
		       si32 *n_track_files,
		       char ***track_file_paths);

extern void print_autocorrelation(char *label,
				  autocorr_t *autocorr);

extern void print_header(si32 n_track_files,
			 char **track_file_paths);

extern void process_storms(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle,
			   si32 *n_complex,
			   si32 *n_simple,
			   autocorr_t *autocorr_volume,
			   autocorr_t *autocorr_vol_two_thirds,
			   autocorr_t *autocorr_precip_area,
			   autocorr_t *autocorr_proj_area);

extern void read_params(void);

extern void tidy_and_exit(int sig);

extern void trend_analysis(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle);

extern void update_track_comps(storm_file_handle_t *s_handle,
			       track_file_handle_t *t_handle);
#ifdef __cplusplus
}
#endif
