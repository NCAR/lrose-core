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
 * kavm2mdv.h - header file for km2mdv program
 *
 * Nancy Rehak RAP NCAR
 *
 * January 1997
 *
 * Based on kav2dobson.h by Mike Dixon
 *
 **************************************************************************/

#include <rapformats/kav_grid.h>
#include <rapformats/km.h>
#include <rapmath/umath.h>
#include <tdrp/tdrp.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "kavm2mdv_tdrp.h"


/*
 * global struct
 */

typedef struct
{
  char *prog_name;                   /* program name */
  TDRPtable *table;                  /* TDRP parsing table */
  kavm2mdv_tdrp_struct params;         /* parameter struct */
  char *input_file_path;
  char *output_dir;

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

extern int convert_param_mdv_encoding_type(int param_encoding_type);

extern char *get_next_file(void);

extern void init_vol_index(vol_file_handle_t *v_handle);

extern void load_data_grid(vol_file_handle_t *v_handle,
			   char *file_buf,
			   int file_size);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params,
		       int *print_params,
		       char **params_file_path_p,
		       tdrp_override_t *override,
		       si32 *n_input_files_p,
		       char ***input_file_list);

extern void process_file(vol_file_handle_t *v_handle,
			 char *input_file_path);

extern void set_vol_time(vol_file_handle_t *v_handle, KM_header_t *header);

extern void tidy_and_exit(int sig);

extern void write_output(vol_file_handle_t *v_handle);

#ifdef __cplusplus
}
#endif
