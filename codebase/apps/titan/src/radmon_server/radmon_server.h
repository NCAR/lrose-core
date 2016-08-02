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

/*************************************************************************
 * radmon_server.h
 *
 * The header for radmon_server program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 *************************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#include <toolsa/umisc.h>
#include <titan/radar.h>
#include <toolsa/pmu.h>
#include <tdrp/tdrp.h>
#include "radmon_server_tdrp.h"

/*
 * permissions for shared mem
 */

#define S_PERMISSIONS 0666

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                     /* program name */

  TDRPtable *table;                    /* TDRP parsing table */
 
  radmon_server_tdrp_struct params;    /* parameter struct */

  time_t latest_data_time;
  time_t latest_request_time;

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

extern char *get_shmem_buffer(void);

extern rdata_shmem_header_t *get_shmem_header(void);

extern int get_sem_id(void);

extern field_params_t *get_field_params(void);

extern rdata_shmem_beam_header_t *get_beam_headers(void);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void read_shmem(void);

extern void register_server(void);

extern void setup_shmem(void);

extern void tidy_and_exit(int sig);

extern void unregister_server(void);

#ifdef __cplusplus
}
#endif

