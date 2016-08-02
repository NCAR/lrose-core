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
 * ufnexrad2gate.h
 *
 * The header for ufnexrad2gate program
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * March 1995
 *
 * Based on rdata_to_socket written by Mike Dixon, Sept. 1992.
 *********************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>

#include <toolsa/umisc.h>
#include <titan/radar.h>
#include <rapformats/gate_data.h>
#include <tdrp/tdrp.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>

#include "ufnexrad2gate_tdrp.h"



#define MAX_DISK_REC_SIZE  65536

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                  /* program name */
  char *params_path_name;           /* params file path name */

  TDRPtable  *table;                /* TDRP parsing table */
  ufnexrad2gate_tdrp_struct  params;
                                    /* TDRP parameter structure */
  
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

extern void parse_args(int argc,
		       char **argv,
		       int *check_params,
		       int *print_params,
		       char **params_file_path_p,
		       tdrp_override_t *override);

extern void print_beam_buffer(ui08 *beam_data);

extern void print_header(ui08 *beam_data);

extern void print_radar_params(ui08 *param_data);

extern void print_summary(ui08 *beam_data, ui08 *radar_data);

extern void process_data_stream(void);

extern si32 read_disk_ufnexrad(ui08 *beam_buffer, ui08 *param_buffer);

extern void read_params(void);

extern int set_beam_flags(ui08 *gate_data_pkt,
			  ui08 *prev_gate_data_pkt);

extern int update_params(ui08 *beam_buffer,
			 ui08 *gate_params_pkt,
			 si32 nbytes_params_pkt);

extern void tidy_and_exit(int sig);

#ifdef __cplusplus
}
#endif
