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
/***************************************************************************
 * DIDSS_COMPRESS.H: Defines and Globals for didss_compress program
 */

#include <toolsa/umisc.h>

#include <time.h>
#include <dirent.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#define PATH_DELIM "/"

/*
 * global data struct
 */

typedef struct {
  
  char *prog_name;              /* the applications name */
  
  char **top_dirs;              /* Top dirs for searches */

  int n_dirs;                   /* number of top dirs */

  int min_age;                  /* secs - the min age of the file since
				 * last touch for compression to be 
				 * performed
				 */

  int verbose;                  /* verbose mode? TRUE or FALSE */

  int sleep_factor;             /* factor to determine sleep interval.
				 * Sleep interval is this factor multiplied
				 * by time taken for last compress */

  int gzip;                     /* use gzip? TRUE or FALSE */

  int date_format;              /* check for date format - TRUE or FALSE */

  int time_format;              /* check for time format - TRUE or FALSE */

  int check_ext;                /* check extension - TRUE or FALSE */

  char *ext;                    /* extension to check for */

  int recursive;                /* recursive directory search? T/F */

  int n_compressed;             /* number of files compressed in a 
				 * given cycle */

  char *procmap_instance;       /* instance used when registering with
				 * the process mapper.  If NULL, doesn't
				 * register */

  int procmap_register_interval;
  
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

extern void check_dir(char *dir_path);
extern int compress_if_required(char *file_path);
extern void parse_args(int argc, char **argv);
extern void tidy_and_exit(int sig);


