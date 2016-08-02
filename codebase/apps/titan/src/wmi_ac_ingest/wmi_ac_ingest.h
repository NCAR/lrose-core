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
 * wmi_ac_ingest.h
 *
 * Header file for wmi_ac_ingest
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>

#include <toolsa/port.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <rapformats/ac_posn.h>
#include <symprod/spdb.h>

#include <tdrp/tdrp.h>
#include "wmi_ac_ingest_tdrp.h"

/*
 * global struct
 */

typedef struct {

  char *prog_name;                   /* program name */
  TDRPtable *table;                  /* TDRP parsing table */
  wmi_ac_ingest_tdrp_struct params;  /* parameter struct */

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

extern void close_output_files(void);

extern void free_test_data(void);

extern void get_test_data(char **callsign_p,
			  double *lat_p, double *lon_p,
			  double *alt_p);

extern void init_test_data(void);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void process_realtime_stream(FILE *tty_file);

extern void process_test_stream(void);

extern void tidy_and_exit(int sig);

extern void tty_close(void);

extern FILE *tty_open(char *ttyname, int baudrate);

extern char tty_read_char(void);

#ifdef __cplusplus
}
#endif
