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
 * cart_convert.h : header file for cart_convert program
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <titan/radar.h>

/*
 ******************************** defines ********************************
 */

#define RC_TABLE "rc_table"
#define RDATA_FILE "rdata_file"

/*
 * global struct
 */

typedef struct {
  
  char *prog_name;                        /* program name */
  char *params_path_name;                 /* params file path name */
  char *rdata_path;                       /* r_theta_file */
  char *rc_table_path;                    /* lookup table file */

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

extern void check_geom(vol_file_handle_t *rv_handle,
		       rc_table_file_handle_t *rc_handle);

extern void parse_args(int argc,
		       char **argv);

extern void setup_cart_volume(vol_file_handle_t *rv_handle,
			      vol_file_handle_t *cv_handle,
			      rc_table_file_handle_t *rc_handle);

extern void transform(vol_file_handle_t *rv_handle,
		      vol_file_handle_t *cv_handle,
		      rc_table_file_handle_t *rc_handle);
#ifdef __cplusplus
}
#endif
