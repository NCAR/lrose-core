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

/**********************************************************************
 * storms_to_ascii.h - header file for storms_to_ascii program
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **********************************************************************/

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/track.h>

#define DEBUG_STR "false"

/*
 * global struct
 */

typedef struct {

  char *prog_name;          /* program name */
  char *params_path_name;   /* params file path name */

  long gmt_offset;          /* number of secs for GMT */
  int debug;                /* debug flag */

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
 * functions
 */

extern date_time_t* load_scan_times(storm_file_handle_t *s_handle);

extern void tidy_and_exit(int sig);

extern void init_indices(storm_file_handle_t *s_handle,
			 track_file_handle_t *t_handle);

extern void open_files(storm_file_handle_t *s_handle,
		       track_file_handle_t *t_handle,
		       char **track_file_paths,
		       si32 file_num);

extern void parse_args(int argc,
		       char **argv,
		       si32 *n_track_files,
		       char ***track_file_paths);

extern void print_comments(FILE *out);

extern void process_track_file(storm_file_handle_t *s_handle,
			       track_file_handle_t *t_handle,
			       date_time_t *scan_time);

#ifdef __cplusplus
}
#endif
