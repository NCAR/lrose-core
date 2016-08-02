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
/************************************************************************
 * mdv2ascii.h : header file for mdv2ascii program
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1994
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <titan/radar.h>

enum {
  TOP,
  BOT
};

/*
 * global structure
 */

typedef struct {

  char *prog_name;   /* program name */
  char *file_name;   /* name of file for view */
  si32 plane_num;    /* number of plane in grid */
  si32 field_num;    /* number of field in file */
  si32 ncolumns;     /* number of cols in the printout */
  char *format;      /* printout format */
  int start_row;     /* TOP or BOT */
  int composite;     /* flag to indicate composite data */
  int vol;           /* flag to print entire volume */
  double max_output_val;
  double min_output_val;
  double bad;        /* Value to use when data bad/missing. */
  int badSet;        /* 0 => no -bad option set, 1 => value set. */
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

extern void parse_args(int argc, char **argv);

extern int
retrieve_next(double *val);

extern void
init_retrieval(vol_file_handle_t *v_handle,
	       si32 plane_num,
	       si32 field_num);

extern void
init_retrieval_comp(vol_file_handle_t *v_handle,
		    si32 field_num);

extern void
init_retrieval_vol(vol_file_handle_t *v_handle,
		   si32 field_num);

#ifdef __cplusplus
}
#endif
