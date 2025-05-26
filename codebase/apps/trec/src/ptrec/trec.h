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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cidd/cidd_files.h>
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_grid.h>
#include <Mdv/mdv/mdv_macros.h>                           
#include <Mdv/mdv/mdv_print.h>                                     
#include <Mdv/mdv/mdv_user.h>                                                
#include <Mdv/mdv/mdv_utils.h>           
#include <Mdv/mdv/mdv_handle.h>           
#include <rapmath/umath.h>
#include <rapformats/lincoln.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <tdrp/tdrp.h>
#include "ptrec_tdrp.h"

/*
 * defines
 */

#define DTR 0.01745329  // degrees to radian
#ifndef PI
#define PI 3.141593
#endif

#define BAD      999    // unhappy trec calculation
#define NAMESIZE 128

//
// just an enumeration -- the numbers don't matter 
// as long as they are unique
//
#define DBZ_FIELD 8
#define VEL_FIELD 9

#define SUCCESS 0    
#define FAILURE -1

#define WAIT_TIME 10000  /* wait time for new file */

/*
 * dimensions struct
 */

typedef struct {
  int nx, ny, nz;
  double max_x, min_x,del_x;
  double max_y, min_y,del_y;
  double max_z, min_z,del_z;
} dimension_data_t;

//
// output fields from trec 
// to be written to mdv file
//
#define NFIELDS_OUT  5                         // number of output fields

typedef struct {
   float ***data;                              // result of trec analysis
   char     name[NAMESIZE];                    // name of output field
   char     units[MDV_UNITS_LEN];              // units of output field
} trec_field_t;

/*
 * globals struct
 */

typedef struct {
  
  char  *prog_name;                     /* program name */
  char  *params_file_name;              /* full name of parameter file */
  char  *input_file_path;               /* path to data files */
  char **input_file_list;               /* list of archive mode data files */
  int    n_input_files;                 /* number of files in the input list */
  float  radar_altitude;                /* read in from mdv file */

  ptrec_tdrp_struct  params;            /* parameter data */
  TDRPtable *table;                     /* TDRP parsing table */
} global_t;


#ifdef MAIN
   global_t *Glob;
#else
   extern global_t *Glob;
#endif

/*
 * function prototypes
 */

extern void adjust_time(ll_params_t *beam, long time_correction);

extern void archive_loop();
extern void do_archive();

extern void azindx(double az2,
		   float *azim,
		   int idistb,
		   int numaz,
		   int *ib3,
		   int *ib4,
		   int isizb);

extern int check_geometry( MDV_field_header_t *hdr );

extern char *create_mdv_input_fname( long file_time, char *file_name );

extern char *create_mdv_output_fname(char *file_name,
				     MDV_master_header_t *mmh);

extern void deci(dimension_data_t *dim_data,
		 float ***data,
		 int nz,
		 int dec_rad,
		 double dec_npts);

extern void fill3d(dimension_data_t *dim_data,
		   float ***data,
		   int nzst,
		   int nz,
		   int fill_rad,
		   int num_oct,
		   float fill_npts);

extern int find_field( int field, MDV_handle_t *mdv );

extern int get_data_times( char *rdata_dir, long *first_data_time,
                                            long *latest_data_time );

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

extern int input_select(const struct dirent *dirInfo );                         
extern int topdir_select(const struct dirent *dirInfo );                         
extern void lstsq(dimension_data_t *dim_data,
		  float *u,
		  float *x,
		  float *y,
		  float ***ucart,
		  int *iaz1,
		  int *iaz2,
                  float *el);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_brief_p,
		       int *print_params_full_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern int process_files( char* file0, char* file1 );

extern void read_params(char *params_file_path,
			tdrp_override_t *override,
			int check_params,
			int print_params_brief,
			int print_params_full);

extern void real_loop();

extern void set_missing_data();

extern void set_synthetic_data();

extern void setup_data( MDV_handle_t *mdv, int field_num );

extern void tidy_and_exit(int sig);

void tkcomp( unsigned char ***dbz0, unsigned char ***dbz1,     
             unsigned char ***vel1,
             float dbz_scale0, float dbz_bias0,
             float vel_scale0, float vel_bias0,
             float max_distance, float delta_time, 
             float delta_azim, float delta_gate, 
             float *azim, int num_azim, float *elev, int num_elev,
             float *range, int num_gate, float radar_altitude,
             dimension_data_t *dim_data, int max_vec, int *iaz1, int *iaz2,
             float *u, float *v, float *x, float *y, float *z, float *dop,
             int *num_vec );

extern int update_cindex( long file_time );

extern void vcterp(float **store_cor,
		   float *range,
		   int iamx,
		   float *azim,
		   int ibmx,
		   int ia1,
		   int ib3,
		   int ioffa,
		   int ioffb,
		   float *xpa,
		   float *xpb);

extern void wair(dimension_data_t *dim_data,
		 float ***ucart,
		 float ***vcart,
		 float ***wcart,
		 float ***conv);

extern int write_mdv_file( MDV_handle_t *mdv,
                           dimension_data_t *dim_data,
                           char* file0, char* file1, 
                           trec_field_t *results );

