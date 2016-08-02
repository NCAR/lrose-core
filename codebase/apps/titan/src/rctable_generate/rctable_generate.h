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
 * rctable_generate.h : header file for rctable_generate program
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990
 *
 * Mike Dixon
 ************************************************************************/

/*
 **************************** includes *********************************
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>

/*
 ******************************** structures *****************************
 */

/*
 * the ppi_table is not part of the lookup table. It is used to accumulate data
 * about each elevation ppi from which the rc_table etc is constructed
 */

typedef struct {

  ui32 npoints;
  ui32 *index;

} ppi_table_t;

/*
 ******************************** defines ********************************
 */

/*
 * defaults
 */

#define NELEVATIONS (si32) 11
#define ELEV_TARGET "0.5 1.2 2.5 4.0 5.5 7.0 8.5 10.0 13.0 17.0 22.0"
#define NAZIMUTHS (si32) 360
#define DELTA_AZIMUTH 1.0
#define START_AZIMUTH 0.0

#define BEAM_WIDTH 0.95
#define NGATES (si32) 664
#define GATE_SPACING 225.0
#define START_RANGE 112.0

#define RADARZ 0.0
#define RADAR_LATITUDE 0.0
#define RADAR_LONGITUDE 0.0
#define CART_LATITUDE 0.0
#define CART_LONGITUDE 0.0
#define CART_ROTATION 0.0

#define NX (si32) 240
#define NY (si32) 240
#define NZ (si32) 20

#define MINX -149.375
#define MINY -149.375
#define MINZ 2.0

#define DX 1.5
#define DY 1.5
#define DZ 1.0

#define KM_SCALEX 1000.0
#define KM_SCALEY 1000.0
#define KM_SCALEZ 1000.0

#define UNITSX "km"
#define UNITSY "km"
#define UNITSZ "km"

#define CREATE_SLAVE_TABLE "false"
#define USE_AZIMUTH_TABLE "false"
#define EXTEND_BELOW "false"

typedef enum {
  CartMode, PpiMode, PolarMode, NModes
} ModeType;
  
#define PPI_MODE "false"

/*
 ******************************* globals struct ******************************
 */

typedef struct {
  
  char *prog_name;                        /* program name */
  char *params_path_name;                 /* parameters file path name */
  char *rc_table_file_path;               /* radar to cart table file */
  char *slave_table_file_path;            /* slave table file */
  char *azimuth_table_file_path;

  int debug;
  int malloc_debug_level;

  int create_slave_table;   /* If set TRUE, slave table is generated.
			     * Otherwise, only the rc_table is generated */

  int use_azimuth_table;    /* set TRUE if use azimuth table,
			     * sequence, FALSE if use regular az strategy */

  int mode;                 /* CartMode, PpiMode, PolarMode */

  si32 nx, ny, nz;

  double radar_latitude;
  double radar_longitude;
  double cart_latitude;
  double cart_longitude;
  double cart_rotation;

  double minx, miny, minz;
  double dx, dy, dz;
  double radarx, radary, radarz;
  double rotation_at_radar;

} global_t;

/*
 * declare the global structure locally in the main,
 * and as an extern in all other routines
 */

#ifdef MAIN

global_t *Glob;

#else

extern global_t *Glob;

#endif

/*
 *********************** function definitions ****************************
 */

extern si32 cappi_index_irregular_azs(double azimuth,
				      double range,
				      double x,
				      double y,
				      double z,
				      double *cosphi,
				      double **beam_ht,
				      double **gnd_range,
				      scan_table_t *scan_table);

extern si32 cappi_index_regular_azs(double azimuth,
				    double range,
				    double z,
				    double *cosphi,
				    double **beam_ht,
				    double **gnd_range,
				    scan_table_t *scan_table);

extern double get_error_irregular_azs(si32 az_index,
				      si32 elev_index,
				      double range,
				      double x,
				      double y,
				      double z,
				      si32 *range_index_p,
				      double *cosphi,
				      double **beam_ht,
				      double **gnd_range,
				      scan_table_t *scan_table);

extern double get_error_regular_azs(si32 elev_index,
				    double range, double z,
				    si32 *range_index_p,
				    double *cosphi,
				    double **beam_ht,
				    double **gnd_range,
				    scan_table_t *scan_table);

extern si32 elev_index_below (double range,
			      double z,
			      double *cosphi,
			      double **beam_ht,
			      scan_table_t *scan_table);

extern void calc_geom(si32 n_ext_elev,
		      double *cosphi,
		      double **beam_ht,
		      double **gnd_range,
		      scan_table_t *scan_table);

extern void invert_table_and_write_files(si32 *rindex_table,
					 scan_table_t *scan_table);

extern void make_table_cart(si32 *rindex_table,
			    double *cosphi,
			    double **beam_ht,
			    double **gnd_range,
			    scan_table_t *scan_table);

extern void make_table_ppi(si32 *rindex_table,
			   double *cosphi,
			   double **beam_ht,
			   double **gnd_range,
			   scan_table_t *scan_table);

extern void make_table_polar(si32 *rindex_table);

extern void parse_args(int argc,
		       char **argv);

extern si32 ppi_index_irregular_azs(double azimuth,
				    double range,
				    int ielev,
				    double *cosphi,
				    double **gnd_range,
				    scan_table_t *scan_table);
  
extern si32 ppi_index_regular_azs(double azimuth,
				  double range,
				  int ielev,
				  double *cosphi,
				  double **gnd_range,
				  scan_table_t *scan_table);
  
extern void read_params(void);

extern void setup_scan(scan_table_t *scan_table);

#ifdef __cplusplus
}
#endif
