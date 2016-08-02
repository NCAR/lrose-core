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
/**************************************************************************
 * mdv_grid.h
 *
 * header file for mdv grid struct
 *
 * Mike Dixon RAP NCAR Feb 1996
 *
 **************************************************************************/

#ifndef mdv_grid_h
#define mdv_grid_h
      
#ifdef __cplusplus
extern "C" {
#endif
  
#include <toolsa/os_config.h>
#include <stdio.h>
#include <math.h>
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_macros.h>
#include <toolsa/toolsa_macros.h>
#include <Mdv/mdv/mdv_file.h>
  
#define MDV_N_GRID_LABELS 3
#define MDV_GRID_UNITS_LEN 32

typedef struct {

  fl32 rotation;
  fl32 spare[9];

} mdv_flat_params_t;
 
typedef struct {

  fl32 spare[10];

} mdv_ll_params_t;
 
typedef struct {

  fl32 lat1;
  fl32 lat2;
  fl32 SW_lat;
  fl32 SW_lon;
  fl32 origin_x; /* Number of cells from SW corner to grid lon. */
  fl32 origin_y; /* Number of cells from SW corner to grid lat. */
  fl32 spare[4];

} mdv_lc2_params_t;
 
typedef struct {
  
  /*
   * projection params
   */
  
  fl32 proj_origin_lat;         /* lat of origin, degrees */
  fl32 proj_origin_lon;         /* long of origin, degrees */

  union {
    mdv_flat_params_t flat;
    mdv_ll_params_t ll;
    mdv_lc2_params_t lc2;
  } proj_params;
  
  fl32 minx, miny, minz;	/* start value, SW corner,
				 * bottom plane (* scale)
				 * minz set to -1 if dz_constant is FALSE */
  
  fl32 dx, dy, dz;		/* cartesian spacing in each dirn (* scale)
				 * dz set to -1 if dz_constant is FALSE */
  
  fl32 sensor_x;                /* sensor coords */
  fl32 sensor_y;
  fl32 sensor_z;
  fl32 sensor_lat;
  fl32 sensor_lon;

  si32 spare[11];
    
  si32 proj_type;               /* type of projection used for grid
				 * MDV_PROJ_FLAT, MDV_PROJ_LATLON etc. */
 
  si32 dz_constant;		/* flag to indicate regularly spaced planes.
				 * Set to 1 if regularly-spaced planes
				 * (constant dz), 0 otherwise. 
				 */

  si32 nx, ny, nz;		/* number of points in each dirn */
  
  si32 nbytes_char;		/* number of bytes of character data
				 * at end of this struct */
  
  char unitsx[MDV_GRID_UNITS_LEN]; /* units in x dirn */
  char unitsy[MDV_GRID_UNITS_LEN]; /* units in y dirn */
  char unitsz[MDV_GRID_UNITS_LEN]; /* units in z dirn */
    
} mdv_grid_t;

struct mdv_grid_comps_s;

typedef void (*MDV_latlon2xy_t)(struct mdv_grid_comps_s *comps,
				double lat, double lon,
				double  *x, double *y);

typedef void (*MDV_xy2latlon_t)(struct mdv_grid_comps_s *comps,
				double x, double y,
				double *lat, double *lon);

/*
 * struct for mdv_grid computations
 */

typedef struct mdv_grid_comps_s {

  si32 proj_type;
  
  double origin_lat;
  double origin_lon;
  double rotation;

  double origin_lat_rad;
  double origin_lon_rad;
  double rotation_rad;

  double origin_colat;
  double sin_origin_colat;
  double cos_origin_colat;

  /* lambert stuff. */
  double lc2_lat1_rad;
  double lc2_lat2_rad;
  double lc2_n;
  double lc2_F;
  double lc2_rho;
  
  MDV_latlon2xy_t latlon2xy;
  MDV_xy2latlon_t xy2latlon;
  

} mdv_grid_comps_t;

/*
 * prototypes
 */
 
extern void MDV_init_flat(double origin_lat,
			  double origin_lon,
			  double rotation,
			  mdv_grid_comps_t *comps);

extern void MDV_init_latlon(mdv_grid_comps_t *comps);

extern void MDV_init_lc2(double origin_lat,
                         double origin_lon,
                         double lat1,
                         double lat2,
                         mdv_grid_comps_t *comps);

extern void MDV_init_proj(mdv_grid_t *grid,
			  mdv_grid_comps_t *comps);

extern void MDV_latlon2xy(mdv_grid_comps_t *comps,
			  double lat, double lon,
			  double *x, double *y);

extern void MDV_print_grid(FILE *out, char *spacer,
			   mdv_grid_t *grid);

extern void MDV_xy2latlon(mdv_grid_comps_t *comps,
			  double x, double y,
			  double *lat, double *lon);

/****************************
 * MDV_load_grid_from_hdrs()
 *
 * Load up mdv_grid_t from
 * MDV_master_header_t and MDV_field_header_t
 */

extern void MDV_load_grid_from_hdrs(MDV_master_header_t *mhdr,
				    MDV_field_header_t *fhdr,
				    mdv_grid_t *grid);

/***************************
 * MDV_load_hdrs_from_grid()
 *
 * Load up MDV_master_header_t and MDV_field_header_t
 * from mdv_grid_t
 */

extern void MDV_load_hdrs_from_grid(mdv_grid_t *grid,
				    MDV_master_header_t *mhdr,
				    MDV_field_header_t *fhdr);
  
/***************************
 * MDV_latlon2index_xy()
 *
 * Returns the data x, y indices for the given lat/lon location.
 * Returns -1 and prints an error message on error.  The current
 # errors are:
 *    - data is compressed
 *    - projection not yet handled
 *
 * Currently only handles the MDV_PROJ_LATLON projection.
 */

extern int MDV_latlon2index_xy(MDV_master_header_t *master_hdr,
			       MDV_field_header_t *field_hdr,
			       double lat, double lon,
			       int *x_index, int *y_index);

/***************************
 * MDV_latlon2index()
 *
 * Returns the data index for the given lat/lon location.
 * Returns -1 and prints an error message on error.  The
 * current errors are:
 *    - data is compressed
 *    - projection not yet handled
 *    - location outside of grid (no error message for this one)
 *
 * Currently only handles the MDV_PROJ_LATLON projection.
 */

int MDV_latlon2index(MDV_master_header_t *master_hdr,
		     MDV_field_header_t *field_hdr,
		     double lat, double lon);


#ifdef __cplusplus
}
#endif

#endif

