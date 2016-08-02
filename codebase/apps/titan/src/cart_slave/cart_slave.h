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

/*************************************************************************
 * cart_slave.h
 *
 * The header for cart_slave program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1990
 *
 *************************************************************************/

#include <titan/file_io.h>
#include <titan/radar.h>
#include <toolsa/xdru.h>
#include <toolsa/sockutil.h>
#include <toolsa/pmu.h>
#include <dataport/bigend.h>
#include <tdrp/tdrp.h>
#include "cart_slave_tdrp.h"

#include <sys/socket.h>
#include <netinet/in.h>

/*
 * restart signal - tells the exit routine that cart_slave
 * should be restarted
 */

#define RESTART_SIG -9999

/*
 * read returns
 */

#define CS_SUCCESS 0
#define CS_FAILURE -1

/*
 * global struct
 */

typedef struct {

  int argc;
  char **argv;

  char *prog_name;

  TDRPtable *table;               /* TDRP parsing table */

  cart_slave_tdrp_struct params;  /* parameter struct */

  char *packet;                           /* packet buffer */

  int sock_dev;                           /* socket descriptor */

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

extern void check_geom(void);

extern void connect_to_server(void);

extern void initialize(void);

extern void init_output(void);

extern void load_beam(ui08 *packet);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);


extern int read_beams(void);

extern int read_field_params(void);

extern void read_slave_table(void);

extern int read_packet(si32 *packet_id);

extern int read_volparams(void);

extern void setup_field_params(field_params_t *field_param_array);

extern void setup_volume(vol_params_t *vol_params,
			 si32 *radar_elevations,
			 si32 *plane_heights);

extern void tidy_and_exit(int sig);

extern int write_output(void);

#ifdef __cplusplus
}
#endif
