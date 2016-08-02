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
 * titan_grid.h
 *
 * header file for titan grid struct
 * Modified from mdv_grid.h.
 *
 * Mike Dixon RAP NCAR Dec 1999
 *
 **************************************************************************/

#ifndef titan_grid_h
#define titan_grid_h
      
#ifdef __cplusplus
extern "C" {
#endif
  
#include <toolsa/os_config.h>
#include <stdio.h>
#include <math.h>
#include <dataport/port_types.h>
#include <toolsa/toolsa_macros.h>
  
/* the number of sides to the polygon describing storm area */

#define N_POLY_SIDES 72

#define TITAN_N_GRID_LABELS 3
#define TITAN_GRID_UNITS_LEN 32

#define TITAN_PROJ_LATLON           0   /* x,y in degrees. 
                                         z defined by vert proj type */
#define TITAN_PROJ_STEREOGRAPHIC    2   /* x,y in km */
#define TITAN_PROJ_LAMBERT_CONF     3   /* x,y in km */
#define TITAN_PROJ_MERCATOR         4   /* x,y in km */
#define TITAN_PROJ_POLAR_STEREO     5   /* x,y in km */
#define TITAN_PROJ_POLAR_ST_ELLIP   6   /* x,y in km */
#define TITAN_PROJ_CYL_EQUIDIST     7   /* x,y in km */
#define TITAN_PROJ_FLAT             8   /* Cartesian, x,y in km. 
                                         z defined by vert proj type*/
#define TITAN_PROJ_POLAR_RADAR      9   /* Radial range, Azimuth angle,
					 * x is gate spacing in km .
					 * y is azimuth in degrees. from
					 * true north + is clockwise
					 * z is elevation angle in degrees. 
					 */
#define TITAN_PROJ_RADIAL          10   /* x = Radius, Meters,
					 * y = azimuth in degrees
					 * z = Defined by TITAN_VERT_TYPE...
					 */
#define TITAN_PROJ_OBLIQUE_STEREO  12   /* x,y in km */
#define TITAN_PROJ_TRANS_MERCATOR  15   /* x,y in km */
#define TITAN_PROJ_ALBERS          16   /* x,y in km */
#define TITAN_PROJ_LAMBERT_AZIM    17   /* x,y in km */

#define TITAN_PROJ_UNKNOWN         99

typedef struct {

  fl32 rotation;
  fl32 spare[9];
  
} titan_flat_params_t;
 
typedef struct {
  
  fl32 spare[10];
  
} titan_ll_params_t;

typedef struct {

  fl32 lat1;
  fl32 lat2;
  fl32 SW_lat;
  fl32 SW_lon;
  fl32 origin_x; /* Number of cells from SW corner to grid lon. */
  fl32 origin_y; /* Number of cells from SW corner to grid lat. */
  fl32 spare[4];

} titan_lc2_params_t;
 
typedef struct {
  
  /*
   * projection params
   */
  
  fl32 proj_origin_lat;         /* lat of origin, degrees */
  fl32 proj_origin_lon;         /* long of origin, degrees */

  union {
    titan_flat_params_t flat;
    titan_ll_params_t ll;
    titan_lc2_params_t lc2;
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
				 * TITAN_PROJ_FLAT, TITAN_PROJ_LATLON etc. */
 
  si32 dz_constant;		/* flag to indicate regularly spaced planes.
				 * Set to 1 if regularly-spaced planes
				 * (constant dz), 0 otherwise. 
				 */

  si32 nx, ny, nz;		/* number of points in each dirn */
  
  si32 nbytes_char;		/* number of bytes of character data
				 * at end of this struct */
  
  char unitsx[TITAN_GRID_UNITS_LEN]; /* units in x dirn */
  char unitsy[TITAN_GRID_UNITS_LEN]; /* units in y dirn */
  char unitsz[TITAN_GRID_UNITS_LEN]; /* units in z dirn */
    
} titan_grid_t;

struct titan_grid_comps_s;

typedef void (*TITAN_latlon2xy_t)(const struct titan_grid_comps_s *comps,
				  double lat, double lon,
				  double  *x, double *y);

typedef void (*TITAN_xy2latlon_t)(const struct titan_grid_comps_s *comps,
				  double x, double y,
				  double *lat, double *lon);

/*
   * struct for titan_grid computations
   */

typedef struct titan_grid_comps_s {

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
  double lc2_sin0;
  double lc2_tan0;
  int lc2_2_tan_lines;
  
  TITAN_latlon2xy_t latlon2xy;
  TITAN_xy2latlon_t xy2latlon;

} titan_grid_comps_t;

/*
   * prototypes
   */
 
extern void TITAN_init_flat(double origin_lat,
			    double origin_lon,
			    double rotation,
			    titan_grid_comps_t *comps);

extern void TITAN_init_latlon(titan_grid_comps_t *comps);

extern void TITAN_init_lc2(double origin_lat,
			   double origin_lon,
			   double lat1,
			   double lat2,
			   titan_grid_comps_t *comps);

extern void TITAN_init_proj(const titan_grid_t *grid,
			    titan_grid_comps_t *comps);

extern void TITAN_latlon2xy(const titan_grid_comps_t *comps,
			    double lat, double lon,
			    double *x, double *y);

extern void TITAN_print_grid(FILE *out, const char *spacer,
			     const titan_grid_t *grid);

extern void  TITAN_print_gridXML(FILE *out, const char *spacer,
				 const titan_grid_t *grid);
     
extern void TITAN_xy2latlon(const titan_grid_comps_t *comps,
			    double x, double y,
			    double *lat, double *lon);

#ifdef __cplusplus
}
#endif

#endif

