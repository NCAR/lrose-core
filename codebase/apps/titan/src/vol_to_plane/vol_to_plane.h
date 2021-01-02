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

/**************************************************************************
 * vol_to_plane.h - header file for vol_to_plane program
 *
 * Mike Dixon RAP NCAR July 1992
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <titan/radar.h>

#define NX 100L
#define NY 100L

#define MINX -50.0
#define MINY -50.0

#define MINZ 0.0
#define MAXZ 20.0

#define DX 1.0
#define DY 1.0

#define TRACK_FILE_PATH " "

#define DEBUG_STR "false"

/*
 * global struct
 */

typedef struct {

  char *prog_name;          /* program name */
  char *params_path_name;   /* params file path name */

  long nx, ny;              /* cartesian grid size */
  double dx, dy;            /* cartesian grid deltas */
  double minx, miny;        /* cartesian grid start values */
  double minz, maxz;        /* lower and upper plane heights */

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
 * function prototypes
 */

extern void tidy_and_exit(int sig);
extern void open_files();
extern void parse_args(int argc,
		       char **argv,
		       long *n_files,
		       char ***file_paths);
extern void read_params(void);
extern void remap_file(char *file_path);

#ifdef __cplusplus
}
#endif
