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
/****************************************************************************
 * trec.h
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * May 1996
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

#include <dataport/port_types.h>
#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <rapformats/lincoln.h>
#include <toolsa/pmu.h>
#include <tdrp/tdrp.h>
#include "trec_tdrp.h"

/*
 * defines
 */

#define DTR 0.01745329
#ifndef PI
#define PI 3.141593
#endif

#define AZ_OFF 25

/*
 * dimensions struct
 */

typedef struct {
  int nx, ny, nz;
  double max_x, min_x,del_x;
  double max_y, min_y,del_y;
  double max_z, min_z,del_z;
} dimension_data_t;

/*
 * boundary layer data type
 */

typedef struct {
  long year, month, day, hour, min, sec;
  long scan_num;
  long threshold_gate;
  double azimuth, elevation;
  double x, y, z;
} blayer_data_t;
  
/*
 * default settings
 */

#define COMMUNICATION_HDR_LEN 8

#define NBEAMS_SAMPLE (long) 7
#define NGATES_SAMPLE (long) 11

#define MAX_POINTS_SAMPLED (long) 500
#define MAX_SAMPLE_TIME 6.0

#define NQUAD 3
#define PTS   15

/*
 * globals struct
 */

typedef struct {
  
  char *prog_name;               /* program name */

  TDRPtable *table;              /* TDRP parsing table */

  trec_tdrp_struct  params;

  long ngates[90];               /* number of gates for each integral
				  * elevation angle - used to determine
				  * how many gates are below the max
				  * height set */
  
  double grid_nx;                /* number of elems. along x for user
				  * defined grid */

  double grid_ny;                /* number of elems. along y for user
				  * defined grid */

  double grid_nz;                /* number of elems. along z for user
				  * defined grid */


} global_t;

global_t *Glob;


/*
 * function prototypes
 */

extern void adjust_time(ll_params_t *beam, long time_correction);

extern void azindx(double az2,
		   float ***az,
		   int idistb,
		   int iel,
		   int k1,
		   int numaz,
		   int *ib3,
		   int *ib4,
		   int isizb);

extern void deci(dimension_data_t *dim_data,
		 float ***data,
		 int nz,
		 int dec_rad,
		 double dec_npts);

extern void despike(dimension_data_t *dim_data,
		    int k1, int nel, int naz, int ngates,
		    double vel_scale,
		    ui08 ****vel);

extern void fill3d(dimension_data_t *dim_data,
		   float ***data,
		   int nzst,
		   int nz,
		   int fill_rad,
		   int num_oct,
		   float fill_npts);

extern void init_ls(dimension_data_t *dim_data,
		    float ***ucones,
		    int iel,
		    float **npts,
		    float **sumx,
		    float **sumy,
		    float **sumx2,
		    float **sumy2,
		    float **sumxy,
		    float **sumu,
		    float **sumux,
		    float **sumuy,
		    short ***ioct,
		    float **meanu);

extern void lstsq(dimension_data_t *dim_data,
		  float *u,
		  float *x,
		  float *y,
		  float *z,
		  int nvec,
		  float ***ucart,
		  int *iaz1,
		  int *iaz2,
		  int nel,
		  float ***el);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_brief_p,
		       int *print_params_full_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void print_beam(ll_params_t *beam);

extern void process_beams(void);

extern void radius(int **el_time,
		   int iel,
		   int k1,
		   int k2,
		   float *tkmove,
		   float *dt);

extern char *read_ll_udp_beam (int udp_port, int debug, char *prog_name);

extern char *read_ncar_udp_beam (int udp_port, int debug, char *prog_name);

extern void read_params(char *params_file_path,
			tdrp_override_t *override,
			int check_params,
			int print_params_brief,
			int print_params_full);

extern void tidy_and_exit(int sig);

extern void tkcomp(dimension_data_t *dim_data,
		   float ***az,
		   float ***el,
		   int nel,
		   int **el_time,
		   int k1,
		   int k2,
		   int **num_az,
		   double vel_scale,
		   double vel_bias,
		   double dbz_scale,
		   double dbz_bias,
		   float *u,
		   float *v,
		   float *x,
		   float *y,
		   float *z,
		   float *dop,
		   int max_vec,
		   int *num_vec,
		   float *rng,
		   int num_gates,
		   int *iaz1,
		   int *iaz2,
		   int date,
		   ui08 ****vel,
		   ui08 ****dbz,
		   ui08 ***flg);

extern void vcterp(float **store_cor,
		   float *rng,
		   int iamx,
		   float ***az,
		   int ibmx,
		   int ia1,
		   int ib3,
		   int ioffa,
		   int ioffb,
		   int iel,
		   int k1,
		   int k2,
		   float *xpa,
		   float *xpb);

extern void wair(dimension_data_t *dim_data,
		 float ***ucart,
		 float ***vcart,
		 float ***wcart,
		 float ***conv);

extern void write_ced_file(date_time_t *sample_time,
			   ll_params_t *beam,
			   dimension_data_t *dim_data,
			   float ***ucart,
			   float ***vcart,
			   float ***wcart,
			   float ***conv,
			   float ***dcart);





