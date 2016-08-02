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
 * clutter_table_generate.h : header file for clutter_table_generate program
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1990
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <titan/file_io.h>
#include <titan/radar.h>

/*
 ******************************** defines ********************************
 */

/*
 * command line arg positions (from end of command line)
 */

#define DBZ_MARGIN 3.0
#define DBZ_THRESHOLD 10.0
#define DBZ_FIELD (si32) 1
#define CLUTTER_FILE_PATH "clutter.median"
#define RC_TABLE_PATH "rc_table"
#define CLUTTER_TABLE_PATH "clutter_table"

/*
 ******************************* global struct ******************************
 */

typedef struct {
  
  char *prog_name;                        /* program name */
  char *params_path_name;                 /* parameters file path name */
  
  char *clutter_table_path;               /* output file */
  char *rc_table_path;                    /* cart lookup table file */
  char *clutter_file_path;                /* file with clutter data */

  int debug;                              /* debug flag */

  si32 dbz_field;                         /* dbz field number */

  double dbz_margin;                      /* extra margin to be placed
					   * on clutter dbz vals */

  double dbz_threshold;                   /* threshold below which vals
					   * are not considered to be
					   * clutter */

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
 *********************** function definitions ****************************
 */

extern void check_geom(vol_file_handle_t *clut_vol_index,
		       rc_table_file_handle_t *rc_handle);

extern void parse_args(int argc,
		       char **argv);

extern void read_clutter();

extern void read_params(void);

extern void write_table(vol_file_handle_t *clut_vol_index,
			rc_table_file_handle_t *rc_handle);
#ifdef __cplusplus
}
#endif
