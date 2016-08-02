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
/**********************************************************************
 * dobson.h : radar data headers for dobson format file
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, Sept 1990
 *
 **********************************************************************/

#ifndef dobson_h
#define dobson_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <toolsa/udatetime.h>

/*
 * labels
 */
  
#ifndef R_FILE_LABEL_LEN
#define R_FILE_LABEL_LEN 40
#endif

#ifndef R_LABEL_LEN
#define R_LABEL_LEN 40
#endif
  
#define N_RADAR_PARAMS_LABELS 1
#define N_CART_PARAMS_LABELS 3
#define N_FIELD_PARAMS_LABELS 3

#define N_VOL_PARAMS_TIMES 4
#define VOL_PARAMS_NOTE_LEN 400

/*
 * plane heights
 */

#define N_PLANE_HEIGHT_VALUES 3

#define PLANE_BASE_INDEX 0
#define PLANE_MIDDLE_INDEX 1
#define PLANE_TOP_INDEX 2

/*
 * factor by which degree values are multiplied before
 * being stored as longs
 */

#define DEG_FACTOR 1000000.0

/*
 * scan modes
 */
   
#define DIX_PPI_MODE 0
#define DIX_RHI_MODE 1
#define DIX_SECTOR_MODE 2
#define DIX_UNKNOWN_MODE 3


/*
 * simple date and time struct
 */

typedef struct {
  si32 year, month, day, hour, min, sec;
} radtim_t;

/*
 * volume scan dobson file format
 *
 * label : char[R_FILE_LABEL_LEN]
 * vol_params : vol_params_t (1)
 * elevations : si32[] - elev angles degrees * 1000000 (nelevations)
 * plane_heights : si32[] - heights of lower and upper plane limits
 *                 (nz * N_PLANE_HEIGHT_VALUES). These are scaled by
 *                 cart->scalez for units indicated by cart->units,
 *                 or cart->km_scalez for km
 * field_params_offsets : file offsets for field params structs (nfields)
 * data : field_params and data - offsets to this data are provided
 *        in the file
 */

/*
 * radar parameters
 */

typedef struct {
  
  si32 nbytes_char;		/* number of bytes of character data
				 * at end of this struct */
  
  si32 radar_id;
  si32 altitude;		/* meters */
  si32 latitude;		/* degrees * 1000000 */
  si32 longitude;		/* degrees * 1000000 */
  si32 nelevations;
  si32 nazimuths;
  si32 ngates;
  si32 gate_spacing;		/* millimeters */
  si32 start_range;		/* millimeters */
  si32 delta_azimuth;		/* degrees * 1000000 */
  si32 start_azimuth;		/* degrees * 1000000 */
  si32 beam_width;		/* degrees * 1000000 */
  si32 samples_per_beam;
  si32 pulse_width;		/* nano-seconds */
  si32 prf;			/* pulse repitition freq. * 1000 */
  si32 wavelength;		/* micro-meters */
  si32 nmissing;		/* number of missing rays */
  
  char name[R_LABEL_LEN];	/* radar name max 40 chars */
  
} radar_params_t;

/*
 * cartesian grid parameters
 */

typedef struct {
  
  si32 nbytes_char;		/* number of bytes of character data
				 * at end of this struct */
  
  si32 latitude;		/* lat of origin, degrees * 1000000 */
  si32 longitude;		/* long of origin, degrees * 1000000 */
  
  si32 rotation;		/* rotation clockwise from true north,
				 * degrees * 1000000 */
  
  si32 nx, ny, nz;		/* number of points in cartesian grid */
  
  si32 minx, miny, minz;	/* start value, SW corner,
				 * bottom plane (* scale)
				 * minz set to -1 if dz_constant is FALSE */
  
  si32 dx, dy, dz;		/* cartesian spacing in each dirn (* scale)
				 * dz set to -1 if dz_constant is FALSE */
  
  si32 radarx, radary, radarz; /* radar coords in each dirn (* scale) */
  
  si32 scalex, scaley, scalez; /* factor by which the file values for
				* min, d and radar must be divided to 
				* be in the units given */
  
  si32 km_scalex, km_scaley, km_scalez;

  /* factor by which the file values for
   * min, d and radar must be divided to 
   * be in kilometers. Set to -1 if
   * not applicable */
  
  si32 dz_constant;		/* flag to indicate regularly spaced planes.
				 * Set to 1 if regularly-spaced planes
				 * (constant dz), 0 otherwise. 
				 * In all cases, the actual
				 * limits for the plane heights
				 * (factored by km_factor) are stored in
				 * the plane_limits array */
  
  char unitsx[R_LABEL_LEN];	/* units in x dirn */
  char unitsy[R_LABEL_LEN];	/* units in y dirn */
  char unitsz[R_LABEL_LEN];	/* units in z dirn */
  
} cart_params_t;

typedef struct {
  
  double latitude;		/* lat of origin, degrees */
  double longitude;		/* long of origin, degrees */
  
  double rotation;		/* rotation clockwise from true north,
				 * degrees */
  
  si32 nx, ny, nz;		/* number of points in cartesian grid */
  
  double minx, miny, minz;	/* start value, SW corner,
				 * bottom plane */
  
  double dx, dy, dz;		/* cartesian spacing in each dirn */
  
  double radarx, radary, radarz; /* radar coords in each dirn (* scale) */
  
} cart_float_params_t;

/*
 * volume scan parameters
 */

typedef struct {
  
  char note[VOL_PARAMS_NOTE_LEN]; /* any relevant note  - max 400 chars */
  
  radtim_t file_time;
  radtim_t start_time;
  radtim_t mid_time;
  radtim_t end_time;
  
  radar_params_t radar;	/* radar parameters */
  cart_params_t cart;		/* cartesian parameters */
  
  si32 nfields;		/* number of data fields */
  
} vol_params_t;

/*
 * parameters for data field
 */

typedef struct {
  
  si32 nbytes_char;	/* number of bytes of character data at
			 * the end of this struct */
  
  si32 encoded;		/* run-length encoded? TRUE or FALSE */
  
  si32 factor;		/* scale and bias values are multiplied by this
			 * factor before being written */
  
  si32 scale;		/* gain of the data */
  si32 bias;		/* offset of zero value */
  
  si32 missing_val;	/* missing data value */
  
  si32 noise;		/* measured noise value */
  
  char transform[R_LABEL_LEN]; /* description of any data transform
				* eg. dbz */
  char name[R_LABEL_LEN];	/* field name */
  char units[R_LABEL_LEN];	/* field units */
  
} field_params_t;

#ifdef __cplusplus
}
#endif

#endif
