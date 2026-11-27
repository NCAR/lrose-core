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
 * dsr2mmq.h
 *
 * The header for the dsr2mmq program
 *
 * Jaimi Yee
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * April 1998
 *
 *************************************************************************/

#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <rdi/mmq.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <tdrp/tdrp.h>
#include "dsr2mmq_tdrp.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>

typedef struct {
   
   char *prog_name;                     /* program name */ 

   TDRPtable *table;                    /* TDRP parsing table */

   dsr2mmq_tdrp_struct params;          /* parameter struct */

   DsRadarQueue *radarQueue;            /* DS format radar queue */
   
   DsRadarMsg   *radarMsg;              /* Ds format radar message */

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

extern void close_mmq();

extern int open_mmq();

extern void parse_args(int argc,
		       char **argv,
                       int *check_params_p,
                       int *print_params_p,
                       tdrp_override_t *override,
                       char **params_file_path_p);

extern void process_data();

extern void tidy_and_exit(int sig);
