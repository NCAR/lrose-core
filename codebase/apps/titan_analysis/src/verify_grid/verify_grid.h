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
 * verify_grid.h - header file for verify_grid program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_write.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <tdrp/tdrp.h>
#include "verify_grid_tdrp.h"

/*
 * struct for contingency data
 */

typedef struct {
  
  double n_detect;
  double n_truth;
  double n_success;
  double n_failure;
  double n_false_alarm;
  double n_non_event;

  double pod;
  double far;
  double csi;
  double hss;

} contingency_t;

/*
 * struct for stats
 */

typedef struct {

  double n_total;
  double sumx;
  double sum2x;
  double mean;
  double sd;

  si32 hist_n_intervals;
  double hist_low_limit;
  double hist_interval_size;

  double *n_per_interval;
  double *percent_per_interval;

} statistics_t;

/*
 * struct for mean computations at grid points
 */

typedef struct {

  si32 n;
  double sum;
  double mean;
  
} mean_stats_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */

  TDRPtable *table;               /* TDRP parsing table */

  verify_grid_tdrp_struct params;  /* parameter struct */

  MDV_dataset_t inter_dataset;     /* dataset for outputting intermediate */
                                   /*   grids */
  
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

extern char *get_truth_path(char *detect_file_path);

extern void tidy_and_exit(int sig);

extern void init_indices(vol_file_handle_t *truth_index,
			 vol_file_handle_t *detect_index);

extern void init_intermediate_grid(void);

extern void parse_args(int argc,
		       char **argv,
		       char **params_file_path_p,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       si32 *n_track_files,
		       char ***track_file_paths);

extern double parse_threshold(vol_file_handle_t *v_handle);

extern void print_cart_params(cart_params_t *cart);

extern void print_contingency_table(contingency_t *cont,
				    FILE *fout);

extern void print_header(si32 n_track_files,
			 char **track_file_paths,
			 FILE *fout);

extern void print_stats(statistics_t *stats,
			FILE *fout);

extern void read_params(statistics_t *stats);

extern void update_cont(char *detect_file_path,
			contingency_t *cont);

extern void update_regression(char *detect_file_path,
			      FILE *fout);

extern void update_stats(char *detect_file_path,
			 statistics_t *stats);

#ifdef __cplusplus
}
#endif
