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
 * rapformats/polar2mdv_lookup.h
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * April 1997
 *
 **********************************************************************/

#ifndef polar2mdv_lookup_h
#define polar2mdv_lookup_h

#ifdef __cplusplus
extern "C" {
#endif

#include <rapformats/radar_scan_table.h>

#define POLAR2MDV_LOOKUP_LABEL "Polar2mdv_lookup_file"

typedef enum {
  P2MDV_CART, P2MDV_PPI, P2MDV_POLAR, P2MDV_NGEOM
} P2mdv_geom_t;
  
/*
 * cartesian radar grid parameters
 */

#define _MDV_RADAR_LABEL_LEN 40
#define _N_MDV_RADAR_GRID_LABELS 3

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
  
  char unitsx[_MDV_RADAR_LABEL_LEN]; /* units in x dirn */
  char unitsy[_MDV_RADAR_LABEL_LEN]; /* units in y dirn */
  char unitsz[_MDV_RADAR_LABEL_LEN]; /* units in z dirn */
  
} _MDV_radar_grid_t;

/*
 * lookup table format
 *
 * file_label   : char[DS_FILE_LABEL_LEN]
 *
 * table_params : P2mdv_lookup_params_t
 *
 * elevations : fl32[]
 *  For each elevation: elev angles (deg) (nelevations)
 *
 * If (table_params.use_scan_table == TRUE) {
 *   nazimuths array: si32[]
 *     For each elevation: nazimuths
 *   azimuths array: fl32[]
 *     array of azimuths: azimuths[]
 * }
 *
 * index array: P2mdv_lookup_index_t[][]
 *   For each elevation:
 *     For each azimith: P2mdv_lookup_index_t
 *
 * entry array: P2mdv_lookup_entry_t[]
 *   For each entry in the table: P2mdv_lookup_entry_t
 */

/*
 * lookup table params
 */

typedef struct {

  si32 nbytes_char;		/* number of character bytes at the
				 * end of this struct */

  si32 geom;                    /* P2MDV_CART, P2MDV_PPI, P2MDV_POLAR */
  
  si32 use_azimuth_table;	/* flag to indicate whether a
				 * variable azimuth table is used */

  si32 extend_below;		/* flag to indicate whether the
				 * data are extended below the bottom
				 * elevation angle */

  si32 missing_data_index;

  si32 nelevations;
  si32 nazimuths;
  si32 ngates;
  si32 nbeams_vol;

  fl32 delta_azimuth;		/* degrees */
  fl32 start_azimuth;		/* degrees */
  fl32 beam_width;		/* degrees */

  fl32 start_range;		/* km */
  fl32 gate_spacing;		/* km */

  fl32 radar_latitude;	        /* deg */
  fl32 radar_longitude;	        /* deg */

  si32 ndata;			/* total number of points in grid */
  si32 nlist;			/* number of bytes in entry list */

  si32 index_offset;		/* byte offset of table_index array */
  si32 list_offset;		/* byte offset of list array */

  si32 spare[2];

  _MDV_radar_grid_t grid;        /* grid parameters */

} P2mdv_lookup_params_t;

/*
 * the P2mdv_lookup entries hold the number of points (npoints),
 * the gate number and the offset relative to the start of the
 * P2mdv_lookup start (offset) of each set of cartesian offsets
 * mapped to by the radar point
 * used for translating radar to cartesian coords
 */

typedef struct {

  ui32 gate;
  ui32 index;

} P2mdv_lookup_entry_t;

/*
 * P2mdv_lookup_index array holds an index of the P2mdv_lookup entries
 * in the file used for translating radar to cartesian coords
 */

typedef struct {

  ui16 npoints;
  ui16 last_gate_active;

  union {

    ui32 offset; /* For P2MDV_CART or P2MDV_PPI, this is the offset to
		  * the relevant point in the entry list.
		  * For P2MDV_POLAR, this is the offset to the start
		  * of the ray in the grid */

    P2mdv_lookup_entry_t *entry;

  } u;

} P2mdv_lookup_index_t;

/*
 * P2mdv_lookup_file_index_t is a convenience structure which may be used for
 * referring to any or all component(s) of the lookup table file
 */

typedef struct {

  char *prog_name;
  char *file_path;
  char *file_label;
  FILE *file;
  int index_initialized;

  P2mdv_lookup_params_t *lookup_params;
  P2mdv_lookup_index_t **lookup_index;
  radar_scan_table_t *scan_table;
  char *list;
  
} P2mdv_lookup_file_index_t;

/*
 * prototypes
 */

extern int FreeP2mdvIndex(P2mdv_lookup_file_index_t *index,
			  char *calling_routine);

extern int FreeP2mdvLookup(P2mdv_lookup_file_index_t *index,
			   char *calling_routine);

extern int InitP2mdvIndex(P2mdv_lookup_file_index_t *index,
			  int size,
			  char *prog_name,
			  char *file_path,
			  char *file_label,
			  FILE *file,
			  char *calling_routine);

extern int ReadP2mdvLookup(P2mdv_lookup_file_index_t *index,
			   char *file_path,
			   char *calling_routine);


#ifdef __cplusplus
}
#endif

#endif
