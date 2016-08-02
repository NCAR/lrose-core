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
 * fmq2tape.h
 *
 * The header for gate_data2fmq program
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * May 1997
 *
 *************************************************************************/

#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/membuf.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <toolsa/fmq.h>
#include <toolsa/ttape.h>
#include <tdrp/tdrp.h>
#include "fmq2tape_tdrp.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>


/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                     /* program name */

  TDRPtable *table;                    /* TDRP parsing table */

  fmq2tape_tdrp_struct params;         /* parameter struct */

  int fmq_file_open;                   /* has the fmq been openned -
                                          TRUE or FALSE */
  int fmq_handle_set;                  /* has the fmq handle been set - 
                                          TRUE or FALSE */
  FMQ_handle_t *fmq_handle;            /* fmq handle */

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
                       int *check_params_p,
                       int *print_params_p,
                       tdrp_override_t *override,
                       char **params_file_path_p);

extern void save_tape_id(int id);

extern void send_fmq_to_tape(void);

extern void setup_fmq(void);

extern void tidy_and_exit(int sig);

#ifdef __cplusplus
}
#endif

