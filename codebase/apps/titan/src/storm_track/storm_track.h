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

/************************************************************************
 * storm_track.h : header file for storm_track program
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1991
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/track.h>
#include <titan/tdata_server.h>
#include <euclid/geometry.h>
#include <toolsa/pmu.h>
#include <tdrp/tdrp.h>
#include "storm_track_tdrp.h"

/*
 ******************************** defines ********************************
 */

#define ALL_SCANS 1
#define LAST_SCAN_ONLY 2

#define S_PERMISSIONS 0666
#define STATE_FILE_NAME "storm_track.state"

/*
 ********************************* structures *********************************
 */

/*
 * structures for storm and track status
 */

typedef struct {

  si32 min_ix, min_iy, max_ix, max_iy;

} bounding_box_t;

typedef struct {

  double proj_area_centroid_x;
  double proj_area_centroid_y;
  double vol_centroid_z;      
  double refl_centroid_z;     
  double top;			
  double dbz_max;		
  double volume;		
  double precip_flux;           
  double mass;                  
  double proj_area;		
  double smoothed_proj_area_centroid_x;
  double smoothed_proj_area_centroid_y;
  double smoothed_speed;
  double smoothed_direction;
  double proj_area_rays[N_POLY_SIDES];
  bounding_box_t bound;
  date_time_t time;

} storm_track_props_t;

typedef struct {

  double smoothed_history_in_secs;
  storm_track_props_t dval_dt;
  storm_track_props_t *history;

  double forecast_x;
  double forecast_y;
  double forecast_area;
  double forecast_length_ratio;

  date_time_t time_origin;

  si32 scan_origin;
  si32 simple_track_num;
  si32 complex_track_num;
  si32 n_simple_tracks;
  si32 duration_in_scans;
  si32 duration_in_secs;
  si32 entry_offset;
  si32 history_in_scans;
  si32 history_in_secs;

} track_status_t;

typedef struct {

  fl32 overlap;
  si32 storm_num;
  si32 complex_track_num;
  si32 n_simple_tracks;

} track_match_t;

typedef struct {

  bounding_box_t box_for_overlap;

  double sum_overlap;
  si32 sum_simple_tracks;

  si32 n_match_alloc;
  si32 n_match;
  si32 match;
  track_match_t *match_array;
  
  int starts;
  int stops;
  int continues;
  int has_split;
  int has_merger;
  
  int checked;
  si32 consolidated_complex_num;

  si32 n_proj_runs_alloc;
  si32 n_proj_runs;

  storm_track_props_t current;
  storm_file_run_t *proj_runs;
  track_status_t *track;

} storm_status_t;

/*
 *********************** globals structure *************************
 */

typedef struct {
  
  char *prog_name;                  /* program name */
  
  int exit_signal;                  /* set by tidy_and_exit to indicate that
				     * an exit signal has been received - 
				     * allows the file writes to complete
				     * before exiting */

  int write_in_progress;            /* indicates to tidy_and_exit that a
				     * file write is in progress */

  storm_tracking_shmem_t *shmem;    /* shared memory */

  int forecast_type;

  int sem_id;                       /* semaphore id */

  TDRPtable *table;                 /* TDRP parsing table */

  storm_track_tdrp_struct params;   /* parameter struct */

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
 *********************** function definitions ****************************
 */

extern void add_overlap(storm_status_t *storms1,
			storm_status_t *storms2,
			si32 istorm, si32 jstorm,
			double area_overlap);

extern void alloc_match_array(si32 n_match, storm_status_t *storm);

extern si32 alloc_new_track(track_file_handle_t *t_handle,
			    date_time_t *dtime,
			    track_utime_t *track_utime,
			    storm_status_t *storm,
			    si32 scan_num,
			    int new_complex_track,
			    si32 complex_track_num,
			    si32 history_in_scans,
			    si32 scan_origin,
			    date_time_t *time_origin);

extern si32 augment_complex_track(track_file_handle_t *t_handle,
				  track_status_t *track,
				  si32 simple_track_num,
				  si32 complex_track_num);

extern void check_matches(storm_status_t *storms1,
			  storm_status_t *storms2,
			  si32 nstorms1,
			  si32 nstorms2);
     
extern void compute_forecast(storm_file_handle_t *s_handle,
			     storm_status_t *storms,
			     si32 storm_num);

extern si32 compute_history_in_secs(track_status_t *track,
				    date_time_t *dtime,
				    si32 history_in_scans);

extern void compute_speed_and_dirn(storm_file_handle_t *s_handle,
				   storm_status_t *storms,
				   si32 nstorms);

extern void consolidate_complex_tracks(track_file_handle_t *t_handle,
				       storm_status_t *storms1,
				       storm_status_t *storms2,
				       si32 nstorms1,
				       si32 nstorms2,
				       track_utime_t *track_utime);
     
extern int create_lock_file(char *storm_data_dir);

extern void free_complex_params(track_file_handle_t *t_handle,
				si32 complex_track_num);

extern void find_overlaps(storm_file_handle_t *s_handle,
			  si32 nstorms1,
			  si32 nstorms2,
			  storm_status_t *storms1,
			  storm_status_t *storms2,
			  double d_hours);

extern void init_file_header(storm_file_handle_t *s_handle,
			     track_file_header_t *theader);

extern void load_bounds(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			si32 nstorms1,
			si32 nstorms2,
			storm_status_t *storms1,
			storm_status_t *storms2,
			double d_hours);
     
extern void load_storm_props(storm_file_handle_t *s_handle,
			     si32 nstorms,
			     storm_status_t *storms,
			     date_time_t *scan_time);

extern void match_storms(storm_file_handle_t *s_handle,
			 si32 nstorms1, si32 nstorms2,
			 storm_status_t *storms1,
			 storm_status_t *storms2,
			 double d_hours);

extern void merge_and_split_search(storm_file_handle_t *s_handle,
				   track_file_handle_t *t_handle,
				   si32 nstorms1,
				   si32 nstorms2,
				   storm_status_t *storms1,
				   storm_status_t *storms2,
				   double d_hours,
				   date_time_t *datetime1,
				   date_time_t *datetime2);

extern int open_files(storm_file_handle_t *s_handle,
		      track_file_handle_t *t_handle,
		      char *access_mode,
		      char *storm_header_path);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p,
		       si32 *n_files_p,
		       char ***file_paths_p);

extern int perform_tracking(storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle,
			    int call_mode,
			    char *storm_header_path);

extern int point_in_polygon(double centroid_x,
			    double centroid_y,
			    double *radials,
			    double start_az,
			    double delta_az,
			    si32 n_sides,
			    double grid_dx,
			    double grid_dy,
			    double search_x,
			    double search_y,
			    double search_radius_ratio);

extern void read_params(void);

extern void remove_current_state_file(void);

extern void remove_lock_file(void);

extern void remove_track_files(void);

extern void resolve_matches(storm_status_t *storms1,
			    storm_status_t *storms2,
			    si32 nstorms1, si32 nstorms2);
     
extern void set_derived_params(int n_files);

extern si32 setup_scan(storm_file_handle_t *s_handle,
		       si32 scan_num,
		       si32 *nstorms,
		       date_time_t *scan_time,
		       char *storm_header_path);

extern void setup_shmem(void);

extern void smooth_spatial_forecasts(storm_file_handle_t *s_handle,
				     storm_status_t *storms,
				     si32 nstorms);

extern si32 start_complex_track(track_file_handle_t *t_handle,
				track_status_t *track,
				si32 simple_track_num,
				track_utime_t *track_utime);

extern void tidy_and_exit(int sig);

extern void update_times(date_time_t *dtime,
			 track_utime_t *track_utime,
			 storm_status_t *storms,
			 si32 storm_num);

extern void update_tracks(storm_file_handle_t *s_handle,
			  track_file_handle_t *t_handle,
			  date_time_t *dtime,
			  si32 scan_num,
			  double d_hours,
			  track_utime_t *track_utime,
			  si32 nstorms1,
			  si32 nstorms2,
			  storm_status_t *storms1,
			  storm_status_t *storms2,
			  si32 *track_continues);

extern si32 write_track(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			date_time_t *dtime,
			track_utime_t *track_utime,
			storm_status_t *storms,
			si32 storm_num,
			si32 scan_num);
#ifdef __cplusplus
}
#endif
