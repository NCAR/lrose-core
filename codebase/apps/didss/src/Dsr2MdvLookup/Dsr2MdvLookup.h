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
 * Dsr2MdvLookup.h : header file for Dsr2MdvLookup program
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
#include <Mdv/mdv/mdv_radar.h>
#include <rapformats/polar2mdv_lookup.h>
#include <tdrp/tdrp.h>
#include "Dsr2MdvLookup_tdrp.h"

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
 ******************************* globals struct ******************************
 */

typedef struct {
  
  char *prog_name;                          /* program name */

  TDRPtable *table;                         /* TDRP parsing table */

  Dsr2MdvLookup_tdrp_struct params;  /* parameter struct */

  si32 nx, ny, nz;
  double minx, miny, minz;
  double dx, dy, dz;

  double radarx, radary, radarz;
  double rotation_at_radar;

  P2mdv_geom_t geom;

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

extern si32 cappi_index_irregular_azs(double azimuth, double range,
				      double x, double y,
				      double z, double *cosphi,
				      double **beam_ht, double **gnd_range,
				      radar_scan_table_t *scan_table);

extern si32 cappi_index_regular_azs(double azimuth, double range,
				    double z, double *cosphi,
				    double **beam_ht, double **gnd_range,
				    radar_scan_table_t *scan_table);

extern double get_error_irregular_azs(si32 az_index, si32 elev_index,
				      double range, double x,
				      double y, double z,
				      si32 *range_index_p, double *cosphi,
				      double **beam_ht, double **gnd_range,
				      radar_scan_table_t *scan_table);

extern double get_error_regular_azs(si32 elev_index,
				    double range, double z,
				    si32 *range_index_p, double *cosphi,
				    double **beam_ht, double **gnd_range,
				    radar_scan_table_t *scan_table);

extern si32 elev_index_below (double range, double z,
			      double *cosphi, double **beam_ht,
			      radar_scan_table_t *scan_table);

extern void calc_geom(si32 n_ext_elev, double *cosphi,
		      double **beam_ht, double **gnd_range,
		      radar_scan_table_t *scan_table);

extern int header_size(radar_scan_table_t *scan_table);

extern void load_params(P2mdv_lookup_params_t *params,
			radar_scan_table_t *scan_table,
			si32 list_offset,
			si32 header_size,
			si32 index_size);

extern void make_table_cart(si32 *rindex_table,
			    double *cosphi,
			    double **beam_ht,
			    double **gnd_range,
			    radar_scan_table_t *scan_table);

extern void make_table_ppi(si32 *rindex_table,
			   double *cosphi,
			   double **beam_ht,
			   double **gnd_range,
			   radar_scan_table_t *scan_table);

extern void make_table_polar(si32 *rindex_table);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern si32 ppi_index_irregular_azs(double azimuth,
				    double range,
				    int ielev,
				    double *cosphi,
				    double **gnd_range,
				    radar_scan_table_t *scan_table);
  
extern si32 ppi_index_regular_azs(double azimuth,
				  double range,
				  int ielev,
				  double *cosphi,
				  double **gnd_range,
				  radar_scan_table_t *scan_table);
  
extern void set_derived_params(radar_scan_table_t *scan_table);

extern void setup_scan(radar_scan_table_t *scan_table);

extern void tidy_and_exit(int sig);

extern void write_cart_or_ppi(si32 *rindex_table,
			      radar_scan_table_t *scan_table);

extern void write_header(P2mdv_lookup_params_t *params,
			 radar_scan_table_t *scan_table,
			 char *label,
			 FILE *out_file,
			 char *file_path);

extern void write_polar(si32 *rindex_table,
			radar_scan_table_t *scan_table);

#ifdef __cplusplus
}
#endif
