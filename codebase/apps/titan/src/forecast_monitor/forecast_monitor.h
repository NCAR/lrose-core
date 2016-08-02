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
 * forecast_monitor.h - header file for forecast_monitor program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <rapmath/umath.h>
#include <Mdv/mdv/mdv_grid.h>
#include <titan/radar.h>
#include <titan/track.h>
#include "tdrp/tdrp.h"
#include "forecast_monitor_tdrp.h"

#define MAX_ADJACENT (MAX_PARENTS + MAX_CHILDREN)

typedef struct {
  int tag;
  int name;
  int nadjacent;
  int adjacent[MAX_ADJACENT];
  char visited;
} tree_vertex_t;

/*
 * struct for storm entry in a track (from verify_tracks)
 */

typedef struct {

  track_file_entry_t entry;
  storm_file_scan_header_t scan;
  storm_file_global_props_t gprops;
  si32 n_runs;
  si32 n_runs_alloc;
  storm_file_run_t *runs;

} fm_storm_t;

/*
 * struct for simple track and its entries
 */

typedef struct {

  simple_track_params_t params;
  int have_generate;
  int have_verify;
  fm_storm_t generate;
  fm_storm_t verify;

} fm_simple_track_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */

  TDRPtable *table;               /* TDRP parsing table */

  forecast_monitor_tdrp_struct params;  /* parameter struct */

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

extern void alloc_runs(fm_storm_t *fm_entry);

extern void alloc_simple_array(si32 n_simple_tracks,
			       fm_simple_track_t **simple_array_p);

extern void alloc_vertex_index(si32 max_index);

extern tree_vertex_t *alloc_vertices(si32 n_vertices);

extern void analyze_sub_tree(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle,
			     vol_file_handle_t *v_handle,
			     si32 verify_scan_num,
			     si32 generate_scan_num,
			     si32 n_scans,
			     double actual_lead_time,
			     date_time_t *scan_times,
			     si32 tag,
			     fm_simple_track_t *stracks,
			     tree_vertex_t *vertices,
			     si32 n_vertices);

extern void clear_grids(void);

extern int compute_grid_stats(double *pod_p,
			      double *far_p, double *csi_p);

extern void init_grids(void);

extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle);

extern void load_forecast_grid(storm_file_handle_t *s_handle,
			       track_file_handle_t *t_handle,
			       fm_simple_track_t *strack,
			       double lead_time);

extern void init_output_module(storm_file_handle_t *s_handle);

extern si32 load_scan_times(storm_file_handle_t *s_handle,
			    date_time_t **scan_times_p);

extern void load_output_entry(storm_file_handle_t *s_handle,
			      fm_simple_track_t *strack,
			      double pod, double far, double csi);

extern void load_storm(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       si32 verify_scan_num,
		       si32 generate_scan_num,
		       fm_simple_track_t *strack);

extern void load_verify_grid(fm_simple_track_t *strack);

extern void load_vertex_index(si32 vertex_num, si32 loc);

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

extern void print_cont(void);

extern void print_grids(void);

extern void process_file(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle,
			 char *track_file_path);

extern void process_scan(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle,
			 vol_file_handle_t *v_handle,
			 si32 verify_scan_num,
			 si32 n_scans,
			 date_time_t *scan_times);

extern void reset_output_module(si32 scan_time);

extern void score_forecast(storm_file_handle_t *s_handle,
			   track_file_handle_t *t_handle,
			   vol_file_handle_t *v_handle,
			   si32 verify_scan_num,
			   si32 generate_scan_num,
			   si32 n_scans,
			   double actual_lead_time,
			   date_time_t *scan_times);

extern int tag_sub_trees(tree_vertex_t *vertices,
			 si32 n_vertices);

extern void tidy_and_exit(int sig);

extern void write_output_file(void);

#ifdef __cplusplus
}
#endif
