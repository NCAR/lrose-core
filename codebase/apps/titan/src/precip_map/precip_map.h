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
 * precip_map.h - header file for precip_map program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include <toolsa/ldata_info.h>
#include <titan/radar.h>
#include <titan/zr.h>
#include <titan/track.h>
#include <mdv/mdv_grid.h>
#include "tdrp/tdrp.h"
#include "precip_map_tdrp.h"

typedef struct {

  double u, v, wt;
  int dx, dy;

} precip_map_motion_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */

  TDRPtable *table;               /* TDRP parsing table */

  precip_map_tdrp_struct params;  /* parameter struct */

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

extern time_t accum_generate(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle,
			     vol_file_handle_t *v_handle,
			     vol_file_handle_t *map_v_handle,
			     date_time_t *scan_times,
			     si32 scan_num);

extern int cart_comp_base(cart_params_t *cart);
extern int cart_comp_top(cart_params_t *cart);

extern void cart_params_to_mdv_grid(cart_params_t *cart,
				    mdv_grid_t *grid,
				    si32 grid_type);
     
extern int check_cart_geom(cart_params_t *radar_cart,
			   titan_grid_t *storm_grid);

extern int check_write(char *file_path);

extern void clear_overwrite(void);
  
extern void compute_precip_lookup(double *precip_lookup,
				  field_params_t *dbz_fparams,
				  double coeff, double expon);

extern void create_composite(int npoints, int base, int top,
			     ui08 **vol, ui08 *comp);
     
extern time_t forecast_generate(storm_file_handle_t *s_handle,
				track_file_handle_t *t_handle,
				vol_file_handle_t *v_handle,
				vol_file_handle_t *map_v_handle,
				date_time_t *scan_times,
				si32 scan_num,
				double lead_time_requested);
    
extern time_t generate(storm_file_handle_t *s_handle,
		     track_file_handle_t *t_handle,
		     vol_file_handle_t *v_handle,
		     vol_file_handle_t *map_v_handle,
		     date_time_t *scan_times,
		     si32 scan_num);

extern int get_mean_motion(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle,
			   si32 iscan,
			   double *mean_dx_dt_p,
			   double *mean_dy_dt_p);
     
extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle,
			 vol_file_handle_t *map_v_handle);

extern void init_map_index(vol_file_handle_t *map_v_handle,
			   vol_file_handle_t *radar_v_handle,
			   date_time_t *ftimeyy);

extern void init_motion_v_handle(vol_file_handle_t *map_v_handle);

extern int load_motion_grid(storm_file_handle_t *s_handle,
			    track_file_handle_t *t_handle,
			    cart_params_t *cart,
			    si32 iscan,
			    double lead_time_hr);
     
extern void load_refl_motion_forecast(ui08 *comp_grid,
				      ui08 *forecast_grid,
				      field_params_t *dbz_fparams);

extern date_time_t* load_scan_times(storm_file_handle_t *s_handle);

extern void lock_and_read_headers(storm_file_handle_t *s_handle,
				  track_file_handle_t *t_handle);

extern void lock_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle);

extern int mdv_comp_base(mdv_grid_t *grid);
extern int mdv_comp_top(mdv_grid_t *grid);

extern int new_data(char *header_file_path);

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

extern time_t persistence_generate(storm_file_handle_t *s_handle,
				   vol_file_handle_t *v_handle,
				   vol_file_handle_t *map_v_handle,
				   date_time_t *scan_times,
				   si32 scan_num,
				   double lead_time);

extern void process_file(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle,
			 vol_file_handle_t *map_v_handle,
			 char *track_file_path,
			 LDATA_handle_t *ldata);

extern time_t refl_forecast_generate(storm_file_handle_t *s_handle,
				     track_file_handle_t *t_handle,
				     vol_file_handle_t *v_handle,
				     vol_file_handle_t *map_v_handle,
				     date_time_t *scan_times,
				     si32 scan_num,
				     double lead_time_requested);

extern void scale_data(double *dprecip,
		       ui08 *uprecip,
		       si32 npoints,
		       si32 factor,
		       si32 *scale_p,
		       si32 *bias_p);

extern void set_overwrite(void);
  
extern void tidy_and_exit(int sig);

extern void unlock_files(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle);

extern time_t verify_generate(storm_file_handle_t *s_handle,
			      vol_file_handle_t *v_handle,
			      vol_file_handle_t *map_v_handle,
			      date_time_t *scan_time,
			      si32 scan_num,
			      double lead_time);
    
extern void write_files(vol_file_handle_t *map_v_handle,
			date_time_t *stime,
			char *map_file_dir,
			char *map_file_path);

extern void  write_motion_grid_file(void);

#ifdef __cplusplus
}
#endif
