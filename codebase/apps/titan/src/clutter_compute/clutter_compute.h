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
 * clutter_compute.h : header file for clutter_compute program
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

#include <titan/file_io.h>
#include <titan/radar.h>

/*
 ******************************** defines ********************************
 */

#define CLUTTER_DIR "../clutter"
#define CLUTTER_FILE_PATH "clutter.median"

/*
 ******************************* global structure ******************************
 */

typedef struct {
  
  char *prog_name;                        /* program name */
  char *params_path_name;                 /* parameters file path name */

  char *clutter_dir;                      /* clutter data directory */
  char *clutter_file_path;                /* name of file with clutter data */

  int debug;

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

extern void compute_median(si32 nfiles,
			   char **clutter_file_names,
			   vol_file_handle_t *vol_index,
			   vol_file_handle_t *clut_vol_index);

extern void get_clutter_files(si32 *nfiles,
			      char ***clutter_file_names);

extern void parse_args(int argc,
		       char **argv);

extern void read_params(void);

extern void initialize(si32 nfiles,
		       char **clutter_file_names,
		       vol_file_handle_t *vol_index,
		       vol_file_handle_t *clut_vol_index);

extern void setup_stats_files();
#ifdef __cplusplus
}
#endif
