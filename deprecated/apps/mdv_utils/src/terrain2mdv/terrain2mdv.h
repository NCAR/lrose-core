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
# include <stdlib.h>	/* for atof */
# include <stdio.h>
# include <errno.h>
# include <math.h>
# include <netcdf.h>

#include <toolsa/umisc.h>
#include <toolsa/globals.h>
#include <rapmath/umath.h>
#include <tdrp/tdrp.h>
#include <toolsa/pmu.h>
#include <cidd/cdata_util.h>
#include <toolsa/file_io.h>
#include <toolsa/pjg.h>
#include <mdv/mdv_grid.h>
#include <ctype.h>

# include "terrain2mdv_tdrp.h"


# define BAD_VALUE 255 

/*
 * global struct
 */

typedef struct {

  char *prog_name;                   /* program name */
  TDRPtable *table;                  /* TDRP parsing table */
  terrain2mdv_tdrp_struct params;     /* parameter struct */
  char *input_file_path;
  char *output_dir;
  int nx, ny;
  double dx, dy;

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


struct _Map
{
    int	nc_id;
    int	alt_var;
    int nlats, nlons;
    /* all the following are in arc seconds */
    int	lat_spacing, lon_spacing;
    int north, south, east, west;
} Map;


void	InitMap (char *fname);
void	InitMapMdv (char *fname);
void	ProcessFiles (char *fnames[], int nfiles);
void	ProcessFilesMdv (char *fnames[], int nfiles);
void	ReadHeader (FILE *infile, int *lat_spacing, int *lon_spacing,
		    int *ssec, int *nsec, int *wsec, int *esec);
short*	ReadColumn (FILE *infile, int firstneeded, int lastneeded, int step);
short*	ReadRow    (FILE *infile, int firstneeded, int lastneeded, int step);

extern void LoadMdvFieldHdr(time_t image_time);

extern void LoadMdvMasterHdr(time_t time);

extern int WriteMdv(date_time_t *image_time, ui08 *data);

extern void parse_args(int argc,
                       char **argv,
                       int *check_params,
                       int *print_params,
                       char **params_file_path_p,
                       tdrp_override_t *override,
                       char **input_file_list_p);

extern void tidy_and_exit(int sig);


