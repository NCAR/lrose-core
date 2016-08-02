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

/*
 * bprp2gate.h
 *
 * Header file for bprp2gate
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <dataport/bigend.h>
#include <rapformats/gate_data.h>
#include <rapformats/bprp.h>
#include <tdrp/tdrp.h>
#include <toolsa/sockutil.h>
#include <toolsa/pmu.h>

#include "bprp2gate_tdrp.h"

#define DBZ_SCALE (0.5)
#define DBZ_BIAS (-30.0)
#define DBZ_FACTOR (10000)

typedef struct {
  ui32 code1;
  ui32 code2;
  ui32 id;
  ui32 len;
  ui32 seqno;
} filter_header_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */
  TDRPtable *table;               /* TDRP parsing table */
  bprp2gate_tdrp_struct params;  /* parameter struct */

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

extern void client_comm(bprp_beam_t *beam);

extern void handle_response(bprp_response_t *response);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void read_radar(void);

extern int send_beam(int client_fd, bprp_beam_t *beam);

extern int send_params(int client_fd, int radar_id, bprp_beam_t *beam);

extern void tidy_and_exit(int sig);

#ifdef __cplusplus
}
#endif
