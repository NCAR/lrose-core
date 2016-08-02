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
 * dva_ingest.h
 *
 * Header file for dva_ingest
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

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <rapformats/bprp.h>
#include <tdrp/tdrp.h>
#include <toolsa/sockutil.h>
#include <toolsa/pmu.h>

#include "dva_ingest_tdrp.h"

typedef struct {
  double viphi, viplo;
  double dbzhi, dbzlo;
  int mus;
  double start_range, gate_spacing;
  int ngates;
} dva_rdas_cal_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */
  TDRPtable *table;               /* TDRP parsing table */
  dva_ingest_tdrp_struct params;  /* parameter struct */

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

extern int check_calib(bprp_beam_t *beam, dva_rdas_cal_t *cal);

extern int close_beam_file(void);

extern int handle_beam(bprp_beam_t *beam, dva_rdas_cal_t *cal);

extern int handle_status(bprp_beam_t *beam, int nbeams_last_vol);

extern int new_beam_file(void);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void read_radar(dva_rdas_cal_t *cal);

extern int read_rdas_cal(dva_rdas_cal_t *cal);

extern void tidy_and_exit(int sig);

extern int write_beam(bprp_beam_t *beam);

extern int write_rdas_cal(dva_rdas_cal_t *cal);

#ifdef __cplusplus
}
#endif
