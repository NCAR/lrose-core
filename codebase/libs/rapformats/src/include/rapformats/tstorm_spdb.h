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
#ifndef tstorm_spdb_h
#define tstorm_spdb_h

/**************************************************
 * tstorm_spdb.h
 *
 * Structures and defines for TITAN track SPDB data
 **************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#include <dataport/port_types.h>
#include <rapformats/titan_grid.h>

/**********************************************************
 * A typical tstorm buffer consists of a header followed
 * by nstorms storms.
 */

/***********************
 * tstorm_spdb_header_t
 */

typedef struct {

  si32 time;                 /* time of closest scan to time requested */

  titan_grid_t grid;           /* cartesian params for grid from which
			      * this data was derived */

  fl32 low_dbz_threshold;    /* reflectivity threshold for storms (dbz) */
  
  si32 n_entries;             /* number of storm entries */

  si32 n_poly_sides;         /* number of sides in polygon */

  fl32 poly_start_az;        /* (deg) azimuth from T.N. for
			      * first polygon point */

  fl32 poly_delta_az;        /* (deg) azimuth delta between
			      * polygon points (pos is clockwise) */

  fl32 spare[11];

} tstorm_spdb_header_t;

/*********************
 * tstorm_spdb_entry_t
 */

#define TSTORM_SPDB_ENTRY_NBYTES_32 60

typedef struct {

  fl32 longitude; /* deg */
  fl32 latitude;  /* deg */

  fl32 direction; /* deg T */
  fl32 speed;     /* km/h */

  si32 simple_track_num;
  si32 complex_track_num;

  fl32 area;      /* km2 */
  fl32 darea_dt;  /* forecast rate of change of area - km2/hr */

  fl32 top;       /* km MSL */

  fl32 ellipse_orientation;  /* deg T */
  fl32 ellipse_minor_radius; /* km or deg */
  fl32 ellipse_major_radius; /* km or deg */

  fl32 polygon_scale;        /* scale to compute km or deg from byte vals
			      * for the polygon rays */
  fl32 algorithm_value;      /* a value that can be associated with each
			      * storm.  This value would be added by a
			      * downstream algorithm process. */
  
  fl32 spare;

  ui08 polygon_radials[N_POLY_SIDES];

  si08 forecast_valid;  /* T/F */
  si08 dbz_max;         /* dBZ */

  si08 intensity_trend; /* -1 decreasing
			 * 0 steady
			 * 1 increasing */

  si08 size_trend;      /* -1 decreasing
			 * 0 steady
			 * 1 increasing */

} tstorm_spdb_entry_t;

/*
 * polygon
 */

typedef struct {

  fl32 lat;                   /* latitude in degrees */
  fl32 lon;                   /* longitude in degrees */

} tstorm_pt_t;

typedef struct {

  tstorm_pt_t pts[N_POLY_SIDES + 1];

} tstorm_polygon_t;

/*
 * product definition
 */

typedef struct {

  si32 type;                  /* product type value as defined above */
  si32 subtype;               /* product subtype value as defined above */
  si32 sequence_num;          /* product counter */
  si32 group_id;              /* group id number */
  si32 generate_time;         /* time of product generation */
  si32 data_time;             /* time of data used to create */
  si32 valid_time;            /* time product is valid */
  si32 expire_time;           /* time product becomes invalid */
  si32 line_type;             /* line type */
  si32 num_polygons;          /* number of polygons in this product */

  si32 spare_int[2];

  tstorm_spdb_header_t header;

  tstorm_spdb_entry_t *entries;

  tstorm_polygon_t *polygons; /* array of structures for all of the
			       * polylines making up this product
			       * num_polylines of these */

} tstorm_product_t;

/*********************
 * function prototypes
 */

/*********************************************
 * function to load forecast centroid location
 */

extern void tstorm_spdb_load_centroid(tstorm_spdb_header_t *header,
				      tstorm_spdb_entry_t *entry,
				      double *lat_p, double *lon_p,
				      double lead_time);

/****************************************
 * function to load ellipse latlon points -
 *   grow argument specifies whether to 
 *   use lineal growth when extrapolating
 *   the storm
 */

void tstorm_spdb_load_growth_ellipse(tstorm_spdb_header_t *header,
			             tstorm_spdb_entry_t *entry,
			             tstorm_polygon_t *polygon,
			             double lead_time,
                                     int grow);
     
/****************************************
 * function to load ellipse latlon points
 */

extern void tstorm_spdb_load_ellipse(tstorm_spdb_header_t *header,
				     tstorm_spdb_entry_t *entry,
				     tstorm_polygon_t *polygon,
				     double lead_time);

/****************************************
 * function to load polygon latlon points
 * from ray data -
 *   the grow argument specifies whether
 *   to use lineal growth when extrapolating
 *   the storm
 */

void tstorm_spdb_load_growth_polygon(const tstorm_spdb_header_t *header,
			             const tstorm_spdb_entry_t *entry,
			             tstorm_polygon_t *polygon,
			             const double lead_time,
                                     const int grow);
     
/****************************************
 * function to load polygon latlon points
 * from ray data.
 */

extern void tstorm_spdb_load_polygon(const tstorm_spdb_header_t *header,
				     const tstorm_spdb_entry_t *entry,
				     tstorm_polygon_t *polygon,
				     const double lead_time);

/*******************************************************************
 * buffer_len()
 *
 * Compute tstorm_spdb buffer len
 *
 */

extern int tstorm_spdb_buffer_len(tstorm_spdb_header_t *header);

/*****************************
 * BigEndian swapping routines
 */

/****************************
 * tstorm_spdb_header_to_BE
 */

extern void tstorm_spdb_header_to_BE(tstorm_spdb_header_t *header);

/****************************
 * tstorm_spdb_header_from_BE
 */

extern void tstorm_spdb_header_from_BE(tstorm_spdb_header_t *header);

/*************************
 * tstorm_spdb_entry_to_BE
 */

extern void tstorm_spdb_entry_to_BE(tstorm_spdb_entry_t *entry);

/***************************
 * tstorm_spdb_entry_from_BE
 */

extern void tstorm_spdb_entry_from_BE(tstorm_spdb_entry_t *entry);

/****************************
 * tstorm_spdb_buffer_to_BE
 *
 * Swap on entire tstorm buffer to big-endian ordering
 */

extern void tstorm_spdb_buffer_to_BE(ui08 *buffer);

/****************************
 * tstorm_spdb_buffer_from_BE
 *
 * Swap on entire tstorm buffer to host byte ordering
 */

extern void tstorm_spdb_buffer_from_BE(ui08 *buffer);

/****************************
 * tstorm_spdb_print_header()
 *
 * Print out header
 */

extern void tstorm_spdb_print_header(FILE *out,
				     const char *spacer,
				     tstorm_spdb_header_t *header);

/****************************
 * tstorm_spdb_print_entry()
 *
 * Print out entry
 */

extern void tstorm_spdb_print_entry(FILE *out,
				    const char *spacer,
				    tstorm_spdb_header_t *header,
				    tstorm_spdb_entry_t *entry);
     
/************************
 * tstorm_print_polygon()
 *
 * Print out polygon
 */

extern void tstorm_spdb_print_polygon(FILE *out,
				      const char *spacer,
				      tstorm_polygon_t *polygon,
				      int n_poly_sides);

     
extern void tstorm_spdb_print_buffer(FILE *out,
				     const char *spacer,
				     ui08 *buffer);

#ifdef __cplusplus
}
#endif

#endif

