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

/*
 * dva_cart.h
 *
 * Header file for dva_cart
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapformats/bprp.h>
#include <tdrp/tdrp.h>
#include <titan/radar.h>

#include "dva_cart_tdrp.h"

#define MAXVIP 5000

typedef struct {
  int nx, ny, nz;
  int dx, dy;
  float dz;
  int minx, miny;
  float minz;
} dva_grid_t;

typedef struct {
  double viphi, viplo;
  double dbzhi, dbzlo;
  int mus;
  double start_range, gate_spacing;
  int ngates;
} dva_rdas_cal_t;

typedef struct
{
  unsigned short z, x1, y1, x2, y2, x3, y3, x4, y4;
  unsigned short ii, a1, a2, a3, a4, ll;
  short	rd1, rd2, ed1, ed2, ad1, ad2, noise;
} dva_lookup_t;

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */
  TDRPtable *table;               /* TDRP parsing table */
  dva_cart_tdrp_struct params;  /* parameter struct */

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

extern int cartesianize(int nbeams,
			bprp_beam_t *beam_array,
			dva_grid_t *grid,
			dva_rdas_cal_t *cal,
			int n_elev,
			double *elevations);
     
extern void init_dobson(dva_grid_t *grid,
			dva_rdas_cal_t *cal,
			int n_elev,
			double *elevations);

extern void load_dobson(int iz, ui08 *cappi);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void process(dva_grid_t *grid,
		    dva_rdas_cal_t *cal,
		    int n_elev,
		    double *elevations);
     
extern int read_elevations(int *n_elev_p,
			   double **elevations_p);

extern int read_grid_params(dva_grid_t *grid);

extern int read_rdas_cal(dva_rdas_cal_t *cal);

extern void tidy_and_exit(int sig);

extern int write_dobson(int radar_id, si32 start_time, si32 end_time);

#ifdef __cplusplus
}
#endif
