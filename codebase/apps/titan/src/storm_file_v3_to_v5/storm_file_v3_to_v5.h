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
/******************************************************************
 * storm_file_v3_to_v5.h
 *
 * header file for storm_file_v3_to_v5 program
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1996
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
#include <tdrp/tdrp.h>
#include "dix_storm.v3.h"
#include "storm_file_v3_to_v5_tdrp.h"

/*
 * global struct
 */

typedef struct {

  char *prog_name;		/* program name */

  TDRPtable *table;		/* TDRP parsing table */

  storm_file_v3_to_v5_tdrp_struct params; /* parameter struct */

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

extern void add_proj_runs(storm_file_handle_t *s_handle,
			  storm_file_global_props_t *gprops);

extern void convert_file(storm_v3_file_index_t *v3_s_handle,
			 storm_file_handle_t *s_handle,
			 char *file_path);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p,
		       si32 *n_track_files_p,
		       char ***track_file_paths_p);

extern void print_file(storm_v3_file_index_t *v3_s_handle,
		       char *file_path);

extern void tidy_and_exit(int sig);

extern int Rfv3AllocStormHeader(storm_v3_file_index_t *s_handle,
				char *calling_routine);

extern int Rfv3AllocStormProps(storm_v3_file_index_t *s_handle,
			       si32 n_layers,
			       si32 n_dbz_intervals,
			       si32 n_runs,
			       char *calling_routine);

extern int Rfv3AllocStormScan(storm_v3_file_index_t *s_handle,
			      si32 nstorms,
			      char *calling_routine);

extern int Rfv3AllocStormScanOffsets(storm_v3_file_index_t *s_handle,
				     si32 nscans_needed,
				     char *calling_routine);

extern int Rfv3CloseStormFiles(storm_v3_file_index_t *s_handle,
			       char *calling_routine);

extern int Rfv3CopyStormScan(storm_v3_file_index_t *s_handle1,
			     storm_v3_file_index_t *s_handle2);

extern void Rfv3DecodeStormHist(storm_v3_file_params_t *params,
				storm_v3_file_dbz_hist_t *hist,
				storm_v3_float_dbz_hist_t *fl_hist);

extern void Rfv3DecodeStormLayer(storm_v3_file_params_t *params,
				 storm_v3_file_global_props_t *gprops,
				 storm_v3_file_layer_props_t *layer,
				 storm_v3_float_layer_props_t *fl_layer);

extern void Rfv3DecodeStormParams(storm_v3_file_params_t *params,
				  storm_v3_float_params_t *fl_params);

extern void Rfv3DecodeStormProps(storm_v3_file_params_t *params,
				 storm_v3_file_global_props_t *gprops,
				 storm_v3_float_global_props_t *fl_gprops,
				 int mode);

extern void Rfv3DecodeStormScan(storm_v3_file_params_t *params,
				storm_v3_file_scan_header_t *scan,
				storm_v3_float_scan_header_t *fl_scan);

extern int Rfv3FlushStormFiles(storm_v3_file_index_t *s_handle,
			       char *calling_routine);

extern int Rfv3FreeStormHeader(storm_v3_file_index_t *s_handle,
			       char *calling_routine);

extern int Rfv3FreeStormProps(storm_v3_file_index_t *s_handle,
			      char *calling_routine);

extern int Rfv3FreeStormScan(storm_v3_file_index_t *s_handle,
			     char *calling_routine);

extern int Rfv3FreeStormScanOffsets(storm_v3_file_index_t *s_handle,
				    char *calling_routine);

extern void Rfv3PrintStormHeader(FILE *out,
				 char *spacer,
				 storm_v3_file_header_t *header);

extern void Rfv3PrintStormHist(FILE *out,
			       char *spacer,
			       storm_v3_file_params_t *params,
			       storm_v3_file_global_props_t *gprops,
			       storm_v3_file_dbz_hist_t *hist);

extern void Rfv3PrintStormLayer(FILE *out,
				char *spacer,
				storm_v3_file_params_t *params,
				storm_v3_file_scan_header_t *scan,
				storm_v3_file_global_props_t *gprops,
				storm_v3_file_layer_props_t *layer);

extern void Rfv3PrintStormParams(FILE *out,
				 char *spacer,
				 storm_v3_file_params_t *params);

extern void Rfv3PrintStormProps(FILE *out,
				char *spacer,
				storm_v3_file_params_t *params,
				storm_v3_file_global_props_t *gprops);

extern void Rfv3PrintStormRuns(FILE *out,
			       char *spacer,
			       storm_v3_file_global_props_t *gprops,
			       storm_v3_file_run_t *runs);

extern void Rfv3PrintStormScan(FILE *out,
			       char *spacer,
			       storm_v3_file_params_t *params,
			       storm_v3_file_scan_header_t *scan);

extern int Rfv3OpenStormFiles(storm_v3_file_index_t *s_handle,
			      char *mode,
			      char *header_file_path,
			      char *data_file_ext,
			      char *calling_routine);

extern int Rfv3ReadStormHeader(storm_v3_file_index_t *s_handle,
			       char *calling_routine);

extern int Rfv3ReadStormProps(storm_v3_file_index_t *s_handle,
			      si32 storm_num,
			      char *calling_routine);

extern int Rfv3ReadStormScan(storm_v3_file_index_t *s_handle,
			     si32 scan_num,
			     char *calling_routine);

extern int Rfv3SeekEndStormData(storm_v3_file_index_t *s_handle,
				char *calling_routine);

extern int Rfv3SeekStartStormData(storm_v3_file_index_t *s_handle,
				  char *calling_routine);

extern int Rfv3StormGridType(storm_v3_file_params_t *params);

extern int Rfv3WriteStormHeader(storm_v3_file_index_t *s_handle,
				char *calling_routine);

extern int Rfv3WriteStormProps(storm_v3_file_index_t *s_handle,
			       si32 storm_num,
			       char *calling_routine);

extern int Rfv3WriteStormScan(storm_v3_file_index_t *s_handle,
			      si32 scan_num,
			      char *calling_routine);


