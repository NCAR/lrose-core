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
/******************************************************************
 * storm_ident.h : header file for storm_ident program
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1991
 *
 * Mike Dixon
 ******************************************************************/

/*
 ************************ includes *****************************
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/radar.h>
#include <titan/storm.h>
#include <titan/tdata_server.h>
#include <toolsa/pmu.h>
#include <euclid/clump.h>
#include <mdv/mdv_macros.h>

#include <limits.h>
#include <time.h>

#include <tdrp/tdrp.h>
#include "storm_ident_tdrp.h"

/*
 *************************** defines ****************************
 */

/*
 * delay on starting storm_track
 */

#define STORM_TRACK_START_SECS 1

/*
 * restart flag
 */
   
#define EXIT_AND_RESTART -2

/*
 * permissions for the shared memory and semaphores
 */

#define S_PERMISSIONS 0666

/*
 * general-purpose constants
 */

/*
 * FUDGE_X is the distance x is moved to avoid errors from
 * computing arctan when the points are coincident
 */

#define FUDGE_X (0.1)

/*
 * MAX_EIG_DIM is the maximum dimension which will be used in
 * eigenvector analysis - used for array allocation
 */
   
#define MAX_EIG_DIM 3

/*
 * missing floating point flag value
 */

#define MISSING_VAL -9999.0

/*
 * vorticity model assumes the storm has a 
 * circular structure
 */

#define VORTICITY_MODEL_FACTOR 2.0
		      
/*
 ************************** structures *****************************
 */

/*
 * struct for all the stats which are computed for each layer
 */
     
typedef struct {
       
  si32 n;
  si32 n_vorticity;
  si32 sum_x;
  si32 sum_y;
       
  double sum_refl_x;
  double sum_refl_y;
  double sum_refl;
  double sum_mass;
  double sum_vel;
  double sum_vel2;
  double sum_vorticity;
  
  double area;
  double vol_centroid_x;
  double vol_centroid_y;
  double refl_centroid_x;
  double refl_centroid_y;
  double mass;
  double dbz_mean;
  double dbz_max;
  double vel_mean;
  double vel_sd;
  
  double vol_centroid_az;
  double vol_centroid_range;
  double vorticity;
  
} layer_stats_t;

/*
 * struct for the dbz histogram entries
 */

typedef struct {
  si32 n_area;
  si32 n_vol;
  double percent_area;
  double percent_vol;
} dbz_hist_entry_t;

/*
 * struct for those variables which are summed
 */

typedef struct {
  si32 n, n_vorticity;
  si32 x, y, z;
  double refl, refl_x, refl_y, refl_z;
  double precip, mass;
  double vel, vel2, vorticity;
} sum_stats_t;

/*
 * struct for grid params
 */

typedef struct {

  si32 nx, ny, nz;              /* number of points in grid */

  si32 start_ix, start_iy, start_iz; /* start index relative to
				      * the data - usually 0 */
  
  si32 end_ix, end_iy, end_iz; /* end index relative to
				* the data - usually 0 */
  
  double min_x, min_y, min_z; /* start coords, SW corner,
			       * bottom plane (* scale) */
  
  double dx, dy, dz;       /* spacing in each dirn */
  

} storm_ident_grid_t;

/*
 ************************** globals structure *************************
 */

typedef struct {
  
  int argc;                              
  char **argv;
  
  char *prog_name;                        /* program name */
  
  int projection;                         /* MDV_PROJ_FLAT or
					   * MDV_PROJ_LATLON */

  si32 nx, ny, nz;                        /* cartesian dimensions */
  double min_x, min_y, min_z;             /* cartesian limits */
  double delta_x, delta_y, delta_z;       /* cartesian grid spacings */
  double radar_x, radar_y, radar_z;       /* radar coords */
  
  si32 low_dbz_byte;                      /* threshold with scale and bias */
  si32 high_dbz_byte;                     /* threshold with scale and bias */
  si32 nfields;                           /* number of radar data fields */
  
  si32 min_valid_layer;                   /* lowest cartesian layer in which
					   * data is considered valid */
  
  si32 max_valid_layer;                   /* highest cartesian layer in which
					   * data is considered valid */
  
  si32 n_dbz_hist_intervals;              /* number of intervals in the
					   * dbz histograms */
  
  si32 dbz_interval[N_BYTE_DATA_VALS];    /* index array which gives the 
					   * hist interval number given
					   * a dbz byte value */
  
  si32 ref_time;                           /* reference time for archive
					    * analysis - work back from this
					    * time (to be phased out)
					    * Unix time */
  
  si32 start_time;                         /* start time for archive
					    * analysis Unix time */
  
  si32 end_time;                           /* end time for archive
					    * analysis Unix time */
  
  storm_tracking_shmem_t *shmem;           /* shared memory - communicates 
					    * with storm_track and
					    * track_server */
  
  int shmem_available;                     /* flag to indicate whether
					    * shmem has been made
					    * available */
  
  int sems_available;                      /* flag to indicate whether
					    * semaphores have been made
					    * available */
  
  int sem_id;                              /* semaphore id */
  
  TDRPtable *table;                 /* TDRP parsing table */

  storm_ident_tdrp_struct params;   /* parameter struct */

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
 ********************* function prototypes **************************
 */

extern void alloc_area_coords(const si32 n_coords,
			      double ***coords_p,
			      si32 *n_alloc_p);

extern void alloc_morphology_grids(void);

extern void alloc_verification_grids(void);

extern void area_comps(const Clump_order *clump,
		       storm_ident_grid_t *grid_params,
		       storm_file_global_props_t *gprops,
		       double darea_at_centroid,
		       double darea_ellipse);

extern int check_second_trip(double top,
			     double base,
			     double centroid_x,
			     double centroid_y,
			     double major_radius,
			     double minor_radius,
			     double orientation);

extern void compute_verification(void);

extern int create_lock_file(void);

extern void create_comp_grid(vol_file_handle_t *v_handle);

extern void create_shmem(void);

extern void dbz_gradient_compute(si32 nz,
				 si32 n_layers,
				 si32 base_layer,
				 si32 top_layer,
				 double min_valid_z,
				 layer_stats_t *layer,
				 fl32 *dbz_max_gradient,
				 fl32 *dbz_mean_gradient);

extern void ellipse_compute(storm_ident_grid_t *grid_params,
			    ui08 *grid,
			    double darea_at_centroid,
			    double darea_ellipse,
			    fl32 *area,
			    fl32 *area_centroid_x,
			    fl32 *area_centroid_y,
			    fl32 *area_orientation,
			    fl32 *area_major_radius,
			    fl32 *area_minor_radius);
     
extern ui08 *erode_proj_area(vol_file_handle_t *v_handle,
			     storm_ident_grid_t *grid_params,
			     si32 nplanes,
			     const Clump_order *clump,
			     double darea_at_centroid);

extern void free_indices(void);

extern ui08 *get_comp_grid(void);

extern si32 get_dobson_data_time(void);

extern int get_next_file_time(date_time_t *prev_file_time,
			      date_time_t *next_file_time,
			      date_time_t *latest_file_time,
			      int depth);

extern si32 get_prev_scan(storm_file_handle_t *s_handle,
			  date_time_t *time_list,
			  si32 *ntimes_in_list);
     
extern si32 get_restart_time(void);

extern void get_startup_time_limits(si32 *start_time_p,
				    si32 *end_time_p);
     
extern void get_storm_file_date(time_t *storm_file_date_p);

extern si32 get_time_list(si32 u_start_time,
			  si32 u_end_time,
			  date_time_t **time_list_p);

extern double km_per_grid_unit(si32 min_iy, si32 max_iy);

extern void identify(vol_file_handle_t *v_handle,
		     storm_file_handle_t *s_handle,
		     si32 scan_num);

extern void init_area_comps(double dbz_scale,
			    double dbz_bias,
			    dbz_hist_entry_t *Dbz_hist,
			    double z_p_inverse_coeff,
			    double z_p_inverse_exponent);

extern void init_props_compute(vol_file_handle_t *v_handle,
			       storm_file_handle_t *s_handle,
			       double *min_valid_z_p);

extern void init_shmem(char *storm_file_path);

extern void init_vol_and_area_comps(void);

extern void load_dbz_hist(dbz_hist_entry_t *dbz_hist,
			  storm_file_dbz_hist_t *hist);

extern void load_gprops(storm_file_global_props_t *gprops,
			si32 storm_num,
			si32 n_layers,
			si32 base_layer,
			si32 n_dbz_intvls,
			int range_limited,
			int top_missing,
			int hail_present,
			int second_trip);

extern void load_header(storm_file_header_t *file_header);

extern void load_lprops(layer_stats_t *layer,
			storm_file_layer_props_t *lprops);

extern void mask_low_tops(vol_file_handle_t *v_handle);

extern int mosaic_get_next_line(ui08 **line_p);

extern void mosaic_read(double min_lon,
			double max_lon,
			double min_lat,
			double max_lat,
			si32 *valid_time_p,
			storm_ident_grid_t *sub_grid);

extern int mosaic_reset(void);
     
extern void mosaic_server_destroy(void);

extern int mosaic_server_init(void);

extern int open_and_check_storm_file(storm_file_handle_t *s_handle,
				     char *header_file_path,
				     storm_file_header_t *file_header);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void perform_tracking(int tracking_mode);

extern void prepare_new_file(storm_file_handle_t *s_handle,
			     char *header_file_path,
			     storm_file_header_t *file_header);

extern int prepare_restart_storm_file(storm_file_handle_t *old_s_handle,
				      time_t restart_time,
				      time_t new_start_time);
     
extern void prepare_old_file(storm_file_handle_t *s_handle,
			     char *header_file_path,
			     si32 current_scan_num);

extern void prepare_storm_file(storm_file_handle_t *s_handle,
			       char *header_file_path,
			       si32 *next_file_num,
			       si32 *next_scan_num,
			       date_time_t *data_time,
			       date_time_t *prev_time,
			       date_time_t *time_list,
			       si32 nfiles_in_list);

extern void
  process_clumps(vol_file_handle_t *v_handle,
		 storm_file_handle_t *s_handle,
		 si32 scan_num,
		 si32 nplanes,
		 si32 nclumps,
		 Clump_order *clumps);
     
extern si32
  process_this_clump(si32 nstorms,
		     Clump_order *clump,
		     vol_file_handle_t *v_handle,
		     storm_file_handle_t *s_handle,
		     si32 nplanes,
		     double clump_vol,
		     double dvol_at_centroid,
		     double darea_at_centroid,
		     double darea_ellipse);
      
extern si32
  props_compute(si32 nstorms,
		Clump_order *clump,
		vol_file_handle_t *v_handle,
		storm_file_handle_t *s_handle,
		storm_ident_grid_t *grid_params,
		double clump_vol,
		double dvol_at_centroid,
		double darea_at_centroid,
		double darea_ellipse);

extern int read_dobson(vol_file_handle_t *v_handle,
		       date_time_t *data_time);

extern void read_mosaic(vol_file_handle_t *v_handle,
			date_time_t *data_time);

extern void remove_lock_file(void);

extern int restart(storm_file_handle_t *s_handle);

extern void set_derived_params(void);

extern void set_storm_file_date(si32 date);

extern si32 store_proj_runs(storm_file_handle_t *s_handle,
			    storm_ident_grid_t *gparams);

extern void tidy_and_exit(int sig);

extern void tilt_compute(si32 nz,
			 si32 n_layers,
			 si32 base_layer,
			 si32 top_layer,
			 double min_valid_z,
			 layer_stats_t *layer,
			 fl32 *tilt_dirn,
			 fl32 *tilt_angle);
     
extern void update_morphology_grids(storm_ident_grid_t *grid_params,
				    ui08 *edm, ui08 *refl_margin,
				    ui08 *morph, ui08 *eroded,
				    int min_ix, int min_iy);

extern void update_all_storms_grid(Clump_order *clump);

extern void update_valid_storms_grid(Clump_order *clump,
				     storm_ident_grid_t *grid_params);

extern void vol_and_area_comps(const Clump_order *clump,
			       double *volume_p,
			       double *dvol_at_centroid_p,
			       double *darea_at_centroid_p,
			       double *darea_ellipse_p);

extern void write_file_index(storm_file_handle_t *s_handle,
			     char *storm_header_file_path);

extern void write_morphology_file(vol_file_handle_t *v_handle);

extern void write_verification_file(vol_file_handle_t *v_handle);

#ifdef __cplusplus
}
#endif
