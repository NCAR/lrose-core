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

/*********************************************************************
 * test2gate.h
 *
 * The header for test2gate program
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * September 1995
 *********************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>

#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <titan/GateData.h>
#include <rapformats/gate_data.h>
#include <tdrp/tdrp.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>

#include "test2gate_tdrp.h"

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                  /* program name */
  char *params_path_name;           /* params file path name */

  TDRPtable  *table;                /* TDRP parsing table */
  test2gate_tdrp_struct  params;
                                    /* TDRP parameter structure */

  int nfields;

} global_t;

/*
 * globals
 */

#ifdef MAIN

global_t *Glob = NULL;

/*
 * if not main, declare global struct as extern
 */

#else

extern global_t *Glob;

#endif

/*
 * function prototypes
 */

extern void calc_sampling_geom(scan_table_t *scan_table);

extern int create_beam_buffer(gate_data_beam_header_t *beam_buffer);

extern void create_data_stream(void);

extern ui08 *create_param_buffer(int *buffer_size);

extern vol_file_handle_t *get_sampling_vol_index(void);

extern void init_sampling(void);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params,
		       int *print_params,
		       char **params_file_path_p,
		       tdrp_override_t *override);

extern void print_radar_params(ui08 *param_data, FILE *out);

extern void sample_dbz_and_vel(ui08 *byte_ptr,
			       scan_table_t *scan_table,
			       int curr_elev, int curr_az);

extern void setup_scan(scan_table_t *scan_table);

extern void tidy_and_exit(int sig);

#ifdef __cplusplus
}
#endif
