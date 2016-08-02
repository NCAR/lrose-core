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
 * mdv_radar.h
 *
 * header file for mdv radar structs
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * April 1997
 *
 **************************************************************************/

#ifndef mdv_radar_h
#define mdv_radar_h
      
#ifdef __cplusplus
extern "C" {
#endif
  
#include <toolsa/os_config.h>
#include <stdio.h>
#include <math.h>
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_macros.h>
#include <Mdv/mdv/mdv_file.h>

/*
 * labels
 */
  
#define MDV_RADAR_LABEL_LEN 40

/*
 * radar parameters
 */

#define N_MDV_RADAR_PARAMS_LABELS 1

typedef struct {
  
  si32 nbytes_char;		/* number of bytes of character data
				 * at end of this struct */
  
  si32 nelevations;
  si32 nazimuths;
  si32 ngates;
  si32 radar_id;
  si32 samples_per_beam;

  si32 spare_int[2];

  fl32 altitude;		/* km */
  fl32 latitude;		/* degrees */
  fl32 longitude;		/* degrees */

  fl32 gate_spacing;		/* km */
  fl32 start_range;		/* km */

  fl32 delta_azimuth;		/* degrees */
  fl32 start_azimuth;		/* degrees */

  fl32 beam_width;		/* degrees */

  fl32 pulse_width;		/* micro-seconds */
  fl32 prf;			/* pulse repitition freq */
  fl32 wavelength;		/* cm */
  fl32 nyquist_freq;            /* s-1 */

  fl32 spare_float[2];

  char name[MDV_RADAR_LABEL_LEN]; /* radar name */
  
} MDV_radar_params_t;

/*
 * parameters for data field
 */

#define N_MDV_RADAR_FIELD_LABELS 3

typedef struct {
  
  si32 nbytes_char;	/* number of bytes of character data at
			 * the end of this struct */
  
  si32 encoded;		/* run-length encoded? TRUE or FALSE */
  
  si32 missing_val;	/* missing data value */
  si32 noise;		/* measured noise value */

  fl32 scale;		/* gain of the data */
  fl32 bias;		/* offset of zero value */
  
  fl32 spare[2];
  
  char transform[MDV_RADAR_LABEL_LEN];  /* description of any data transform
					 * eg. dbz */

  char name[MDV_RADAR_LABEL_LEN];	/* field name */
  
  char units[MDV_RADAR_LABEL_LEN];	/* field units */
  
} MDV_radar_field_t;

/*
 * cartesian radar grid parameters
 */

#define N_MDV_RADAR_GRID_LABELS 3

typedef struct {
  
  si32 nbytes_char;		/* number of bytes of character data
				 * at end of this struct */
  
  si32 nx, ny, nz;		/* number of points in cartesian grid */
  
  si32 dz_constant;		/* flag to indicate regularly spaced planes.
				 * Set to 1 if regularly-spaced planes
				 * (constant dz), 0 otherwise. 
				 * In all cases, the actual
				 * limits for the plane heights
				 * (factored by km_factor) are stored in
				 * the plane_limits array */

  si32 spare_int[3];
  
  fl32 latitude;		/* lat of origin, degrees */
  fl32 longitude;		/* long of origin, degrees */
  
  fl32 rotation;		/* rotation clockwise from true north,
				 * degrees */
  
  fl32 minx, miny, minz;	/* start value, SW corner,
				 * bottom plane (* scale)
				 * minz set to -1 if dz_constant is FALSE */
  
  fl32 dx, dy, dz;		/* cartesian spacing in each dirn (* scale)
				 * dz set to -1 if dz_constant is FALSE */
  
  fl32 radarx, radary, radarz;  /* radar coords in each dirn (* scale) */

  fl32 spare_float[2];
  
  char unitsx[MDV_RADAR_LABEL_LEN]; /* units in x dirn */
  char unitsy[MDV_RADAR_LABEL_LEN]; /* units in y dirn */
  char unitsz[MDV_RADAR_LABEL_LEN]; /* units in z dirn */
  
} MDV_radar_grid_t;

/*
 * vol_file_index_t is a convenience structure which may be used for
 * referring to any or all component(s) of the radar volume file
 */

typedef struct {
  
  char *prog_name;
  char *vol_file_path;
  char *vol_file_label;
  FILE *vol_file;
  int index_initialized;

  si32 nfields;

  MDV_radar_params_t params;
  MDV_radar_grid_t grid;
  MDV_radar_field_t *fields;

  fl32 *radar_elevations;
  ui08 ***field_plane;
  
  /*
   * the following are used to keep track of allocated memory
   */
  
  si32 nelevations_allocated;
  si32 nfields_allocated;
  si32 nplanes_allocated;
  si32 **plane_allocated;
  
} MDV_radar_index_t;

/*
 * prototypes
 */

extern void MDVPrintRadarGrid(FILE *out,
			      char *spacer,
			      MDV_radar_grid_t *grid);

extern void MDVPrintRadarElevations(FILE *out,
				    char *spacer,
				    char *label,
				    si32 nelevations,
				    fl32 *radar_elevations);

extern void MDVPrintRadarField(FILE *out,
			       char *spacer,
			       si32 field_num,
			       MDV_radar_field_t *field);

extern void MDVPrintRadarParams(FILE *out,
				char *spacer,
				MDV_radar_params_t *rparams);

extern void BE_to_MDV_radar_params(MDV_radar_params_t *rparams);
extern void BE_from_MDV_radar_params(MDV_radar_params_t *rparams);
extern void BE_to_MDV_radar_field(MDV_radar_field_t *rfield);
extern void BE_from_MDV_radar_field(MDV_radar_field_t *rfield);
extern void BE_to_MDV_radar_grid(MDV_radar_grid_t *rgrid);
extern void BE_from_MDV_radar_grid(MDV_radar_grid_t *rgrid);

#ifdef __cplusplus
}
#endif

#endif

