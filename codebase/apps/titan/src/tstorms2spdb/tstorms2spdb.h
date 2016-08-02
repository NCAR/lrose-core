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
 * tstorms2spdb.h - header file for tstorms2spdb program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <rapmath/umath.h>
#include <mdv/mdv_grid.h>
#include <titan/radar.h>
#include <titan/track.h>
#include <rapformats/tstorm_spdb.h>
#include "tdrp/tdrp.h"
#include "tstorms2spdb_tdrp.h"

/*
 * global struct
 */

typedef struct {

  char *prog_name;                  /* program name */

  TDRPtable *table;                 /* TDRP parsing table */

  tstorms2spdb_tdrp_struct params;  /* parameter struct */

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

extern void close_track_files(storm_file_handle_t *s_handle,
			      track_file_handle_t *t_handle);

extern void init_output_module(storm_file_handle_t *s_handle);

extern si32 load_scan_times(storm_file_handle_t *s_handle,
			    date_time_t **scan_times_p);

extern void open_track_files(storm_file_handle_t *s_handle,
			     track_file_handle_t *t_handle,
			     char *track_file_path);

extern void output_close(void);

extern int output_init(void);
     
extern int output_write(si32 valid_time, si32 expire_time,
			ui08 *message, int messlen);
     
extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p,
		       int *n_track_files_p,
		       char ***track_file_paths_p);

extern void process_file(char *track_file_path);

extern int process_scan(storm_file_handle_t *s_handle,
			track_file_handle_t *t_handle,
			int this_scan_num,
			int n_scans,
			date_time_t *scan_times);

extern void reset_output_module(si32 scan_time);

extern void tidy_and_exit(int sig);

#ifdef __cplusplus
}
#endif
