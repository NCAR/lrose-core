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
 * verify_tracks.h - header file for verify_tracks program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/track.h>

#define STORM_ELLIPSE 1
#define STORM_POLYGON 2
#define STORM_RUNS 3

#define POINT_ACTIVITY 1
#define AREA_ACTIVITY 2

#define NX (long) 100
#define NY (long) 100

#define MINX -50.0
#define MINY -50.0

#define DX 1.0
#define DY 1.0

#define TRACK_FILE_PATH " "

#define FORECAST_LEAD_TIME 30.0
#define FORECAST_LEAD_TIME_MARGIN 3.0
#define FORECAST_MIN_HISTORY 20.0
#define FORECAST_SCALE_FACTOR 1.0

#define ZERO_GROWTH "false"
#define PARABOLIC_GROWTH "false"
#define FORECAST_GROWTH_PERIOD 30.0

#define DEBUG_STR "false"
#define VERIFY_BEFORE_FORECAST_TIME "false"
#define VERIFY_AFTER_TRACK_DIES "true"
#define FORCE_XY_FIT "false"

#define FORECAST_METHOD "ellipse"
#define VERIFY_METHOD "runs"

#define ACTIVITY_CRITERION "point"
#define ACTIVITY_RADIUS 5.0
#define ACTIVITY_FRACTION 0.25

typedef struct {
  double x, y;
} point_t;

/*
 * struct for storm entry in a track
 */

typedef struct {

  track_file_entry_t entry;
  storm_file_global_props_t gprops;
  storm_file_run_t *runs;
  long *x_lookup;          /* lookup table to relate the scan cart grid
			    * to the verification grid */
  long *y_lookup;          /* ditto */

} vt_storm_t;

/*
 * struct for simple track and its storms
 */

typedef struct {

  simple_track_params_t params;
  vt_storm_t *storms;

} vt_simple_track_t;

/*
 * struct for index to a track entry
 */

typedef struct {

  long isimple;
  long istorm;

} vt_entry_index_t;

/*
 * struct for contingency data
 */

typedef struct {

  double n_success;
  double n_failure;
  double n_false_alarm;
  double n_non_event;

} vt_count_t;

/*
 * struct for rmse computations
 */

typedef struct {

  track_file_forecast_props_t sum_sq_error;
  track_file_forecast_props_t sum_error;
  track_file_forecast_props_t rmse;
  track_file_forecast_props_t bias;

  /*  
   * distance error btn forecast and verify cells 
  */ 

  double sum_dist_error ;
  double sum_sq_dist_error ;

  /*
   * correlation coefficient
   */

  track_file_forecast_props_t sumx;
  track_file_forecast_props_t sumy;
  track_file_forecast_props_t sumx2;
  track_file_forecast_props_t sumy2;
  track_file_forecast_props_t sumxy;
  track_file_forecast_props_t corr;

  /*
   * norm refers to error normalized relative to the
   * data value, i.e. as a fraction of the correct val
   */

  track_file_forecast_props_t norm_sum_sq_error;
  track_file_forecast_props_t norm_sum_error;
  track_file_forecast_props_t norm_rmse;
  track_file_forecast_props_t norm_bias;

  double n_movement;
  double n_growth;
  
} vt_stats_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;          /* program name */
  char *params_path_name;   /* params file path name */

  double forecast_lead_time; /* input as mins, stored as secs */

  int zero_growth;     /* if true, only storm movement is used for the
			* forecast, no storm growth */

  int parabolic_growth;      /* TRUE or FALSE - if set then
			      * growth is parabolic - see below */

  double forecast_growth_period; /* input as mins, stored as secs.
				  * The period of growth - at the end
				  * of the period the rate of growth is
				  * zero, and after that decay begins.
				  * This results in a parabolic growth
				  * curve */

  double forecast_lead_time_margin; /* margin of error around the forecast
				     * time, within which a scan time must
				     * fall for it to be applicable for
				     * the forecast - input as mins,
				     * stored as secs */

  long forecast_min_history; /* min history before forecasts
			      * are considered valid -
			      * input as mins, stored as secs */

  double forecast_scale_factor; /* the factor by which the shape
				 * dimensions are multiplied when
				 * setting the verification grids */

  long nx, ny;                  /* cartesian grid size */
  double dx, dy;                /* cartesian grid deltas */
  double minx, miny;            /* cartesian grid start values */

  int debug;                    /* debug flag */
  
  int forecast_method;          /* STORM_ELLIPSE or STORM_POLYGON */

  int verify_method;            /* STORM_ELLIPSE, STORM_POLYGON or
				 * STORM_RUNS */

  int verify_before_forecast_time; /* flag - if set to true, the
				    * verification is ignored until
				    * the track is old enough for
				    * a forecast to be applicable to
				    * this scan. Default is false */

  int verify_after_track_dies;     /* if true, the verification is 
				    * performed for forecasts which
				    * extend beyond the real lifetime
				    * of the track. Default is true */

  int force_xy_fit;    /* if true, the forecast grid data is offset in (x, y)
			* to match the truth data as closely as possible
			* before the contingency table computations
			* are performed. This has the effect of ignoring
			* errors in position, and assesses in errors in
			* shape and size only */

  int activity_criterion; /* POINT_ACTIVITY or AREA_ACTIVITY -
			   * criterion used to decide on hit, miss or
			   * false alarm */

  double activity_radius;
  double activity_fraction;

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

extern int load_props(storm_file_handle_t *s_handle,
		      track_file_handle_t *t_handle,
		      track_file_entry_t *entry,
		      storm_file_global_props_t *gprops,
		      double lead_time,
		      track_file_forecast_props_t *props);

extern void tidy_and_exit(int sig);

extern int uline_through_ellipse(double major_radius,
				 double minor_radius,
				 double slope,
				 double intercept,
				 double *xx1,
				 double *yy1,
				 double *xx2,
				 double *yy2);

extern void compute_contingency_data(ui08 **forecast_grid,
				     ui08 **truth_grid,
				     long nbytes_grid,
				     vt_count_t *count);

extern void compute_errors(storm_file_handle_t *s_handle,
			   long nverify,
			   track_file_forecast_props_t *props_current,
			   track_file_forecast_props_t *props_forecast,
			   track_file_forecast_props_t *props_verify,
			   vt_stats_t *complex_stats,
			   vt_stats_t *file_stats,
			   vt_stats_t *total_stats);

extern void compute_lookup(titan_grid_t *grid,
			   long *x_lookup,
			   long *y_lookup);

extern void compute_stats(vt_stats_t *stats);

extern void debug_print(track_file_handle_t *t_handle,
			vt_simple_track_t *stracks,
			long ncurrent,
			vt_entry_index_t *current,
			ui08 **forecast_grid,
			ui08 **runs_truth_grid,
			vt_count_t *count);

extern void find_last_descendant(track_file_handle_t *t_handle,
				 int level);

extern void increment_count(vt_count_t *general,
			    vt_count_t *specific);

extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle);

extern void load_file_stats(track_file_params_t *tparams,
			    vt_stats_t *stats,
			    si32 *n_samples,
			    track_file_forecast_props_t *bias,
			    track_file_forecast_props_t *rmse);

extern void load_forecast_grid(storm_file_handle_t *s_handle,
			       track_file_handle_t *t_handle,
			       track_file_entry_t *entry,
			       storm_file_global_props_t *gprops,
			       double lead_time,
			       ui08 **grid);

extern void load_truth_grid(storm_file_handle_t *s_handle,
			    vt_storm_t *storm,
			    ui08 **grid);

extern void open_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       char **track_file_paths,
 		       long file_num);

extern void parse_args(int argc,
		       char **argv,
		       long *n_track_files,
		       char ***track_file_paths);

extern void perform_verification(storm_file_handle_t *s_handle,
				 track_file_handle_t *t_handle,
				 date_time_t *scan_time,
				 vt_count_t *total_count,
				 vt_stats_t *total_stats);

extern void print_contingency_table(FILE *fout,
				    const char *label,
				    vt_count_t *count);

extern void print_stats(FILE *fout,
			const char *label,
			vt_stats_t *stats);

extern void read_params(void);

extern void save_verification_parameters(storm_file_handle_t *s_handle,
					 track_file_handle_t  *t_handle);

extern void set_counts_to_longs(vt_count_t *double_count,
				track_file_contingency_data_t *long_count);

extern void triangle_pts(point_t *pt0,
			 point_t *pt1,
			 point_t *pt2,
			 ui08 **grid,
			 long nx,
			 long ny,
			 double minx,
			 double miny,
			 double dx,
			 double dy);

extern void verify_tracks();

#ifdef __cplusplus
}
#endif
