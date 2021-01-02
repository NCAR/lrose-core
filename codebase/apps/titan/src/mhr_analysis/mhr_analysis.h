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
 * mhr_analysis.h
 *
 * The header for mhr_analysis program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1993
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>

#include <rapformats/rp7.h>

#define TRUE 1
#define FALSE 0

/*
 * max record size for tape and beam buffers
 */

#define MAX_TAPE_REC_SIZE 65536
#define MAX_BEAM_REC_SIZE 32768

/*
 * printout params
 */

#define HEADER_INTERVAL 360L
#define SUMMARY_INTERVAL 1L

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                     /* program name */
  char *tape_name;

  int filelist;                        /* only list the files on tape
					* TRUE or FALSE */
  si32 summary_interval;                /* interval between prints */
  int summary_print;                    /* TRUE or FALSE */

  si32 header_interval;                 /* interval between prints */
  int header_print;                     /* TRUE or FALSE */

  int analyze;                          /* TRUE or FALSE */

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

extern int fwd_space_file(int tape,
			  si32 nfiles);

extern int fwd_space_record(int tape,
			    si32 nrec);

extern int rewind_tape(int tape);

extern si32 read_tape_beam(char **beam_buffer);

extern struct mtget *get_tape_status(int tape);

extern void list_tape_files(void);

extern void parse_args(int argc,
		       char **argv);

extern void print_header(char *beam_buffer);

extern void print_summary(char *beam_buffer);

extern void perform_analysis (char *beam_buffer);

extern void process_data_stream(void);

extern void read_rp7_enet(char **beam_buffer);

extern void tidy_and_exit(int sig);

#ifdef __cplusplus
}
#endif
