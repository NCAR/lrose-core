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
/**************************************************************************
 * dix_storm.v3.h - header file for storm structures v3
 *
 * needs dix_util.h and dix_radar.h
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#ifndef dix_storm_v3_h
#define dix_storm_v3_h

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <limits.h>
#include <sys/types.h>
#include <toolsa/pjg.h>

typedef struct {
  si32 year, month, day, hour, min, sec;
} dattim_t;
		  

/*
 * defines
 */

#define V3_MISSING_VALUE (si32) -9999

#define STORM_V3_DATA_MULT 10000.0 /* most floating point values are
				 * multiplied by mult before being
				 * written to file */

#define STORM_V3_VORTICITY_MULT 1000000000.0 /* vorticity values are
					   * multiplied by vort_mult
					   * before being
					   * written to file */
				 
#define V3_N_POLY_SIDES 36 /* the number of sides to the polygon
			 * describing storm area */

/*
 * definitions for projective geometry used in storm
 * computations - before STORM_V3_MINOR_REV_2. After this
 * PJG_FLAT and PJG_LATLON are used
 */

#define OLD_STORM_V3_GRIDTYP_LATLON -1
#define OLD_STORM_V3_GRIDTYP_FLAT 0

/*
 * constants for verification
 */
  
#define N_VERIFICATION_FIELDS 2

#define ALL_STORMS_V3_FIELD 0
#define VALID_STORMS_V3_FIELD 1

/*
 * Storm file formats.
 *
 * There are two files for each date - a header file and a data file.
 *
 * Header file.
 * -----------
 *
 * file label - char[R_FILE_LABEL_LEN] (see dix_rfutil.h)
 *
 * header - struct storm_v3_file_header_t
 *
 * si32 scan_offset[header->n_scans] - file offset of each scan header
 *
 * Data file.
 * ---------
 *
 * file label - char[R_FILE_LABEL_LEN] (see dix_rfutil.h)
 *
 * for each scan :-
 *
 *  struct storm_v3_file_scan_header_t
 *
 *  array of structs storm_v3_file_global_props_t[nstorms], where there are
 *  nstorms in this scan. The offset for this array is given by 
 *  gprops_offset in the scan header
 *
 *  followed by
 *
 *  for each storm in the scan :-
 *
 *    array of structs storm_v3_file_layer_props_t[n_layers], where n_layers is in
 *      the global props (at file pos layer_props_offset)
 * 
 *    array of structs storm_v3_file_dbz_hist_t[n_dbz_intervals],
 *      where n_dbz_intervals is in the global props
 *      (at file pos dbz_hist_offset)
 *
 *    array of structs storm_v3_file_run_t[n_runs],
 *      where n_runs is in the global props (at file pos runs_offset)
 *    
 * Notes.
 *
 * 1.
 * 
 * The offsets of scan headers in the file are stored in the array
 * scan_offset[].
 *
 * The global properties for the storms are in the 'storm_v3_file_global_props_t'
 * array, which are pointed to by gprops_offset in the scan header.
 *
 * The position of the two arrays of types 'storm_v3_file_layer_props_t'
 * and 'storm_v3_file_dbz_hist_t' are given by the entries 'layer_props_offset'
 * and 'dbz_hist_offset' in the scan header.
 *
 * It is therefore possible to move reasonably directly to the data
 * for a particular storm, knowing the scan number and the storm number.
 *
 * 2.
 *
 * All data values in the storm properties files are longs.
 * In most cases, these longs represent floating point values which
 * have been multiplied by a factor to preserve their significance.
 * The multipliers are stored as 'mult' and 'vort_mult'
 * in the storm file header.
 *
 * 3.
 *
 * The data grids upon which the storm identification is based are
 * of two types, LATLON and FLAT.
 * The LATLON projection has dx and dy in longitude and latitude
 * respectivley. The FLAT grid has dx and dy in km. The position
 * and shape properties of the storms will be either in km or
 * degrees, depending upon which grid was used. This is indicated
 * by the comment (km or deg) next to entries in this structs in
 * this file. The area, volume and speed properties are all in
 * km units.
 */

/*
 * storm file label
 *
 * Type 2 is same as type 1 with following features added (10/92)
 *
 *   storm shape polygons
 *   second trip flags
 *   merge height
 *   top threshold
 *   volume limit
 *   hail threshold
 *   low and high dbz thresholds
 *   grid projections
 *
 * Type 3 has same data as type 2. However, the type 3 files have
 * a header and data file pair, so that there is no maximum number
 * of scans, (7/93)
 */

#define STORM_V3_HEADER_FILE_TYPE "Storm header type 3"
#define STORM_V3_DATA_FILE_TYPE "Storm data type 3"

/*
 * the minor revision for the file type
 */

#define STORM_V3_MINOR_REV_2 2

/*
 * Format notes: the following notes apply as indicated in the
 * structs which follow.
 *
 * F1: stored as (val * mult)
 * 
 * F2: stored as (val * mult) for minor_rev < 2
 *     stored as (val) for minor_rev >= 2
 *
 * F3: stored as (deg * mult) for minor_rev < 2
 *     stored as (deg * 1000000) for minor_rev >= 2
 *
 * F4: units are km  for projection == FLAT,
 *               deg for projection == LATLON.
 *     for minor_rev < 2
 *       stored as ((km or deg) * mult)
 *     for minor_rev >= 2
 *       for projection == FLAT, stored as (km * mult)
 *       for projection == LATLON, stored as (deg * 1000000)
 *
 * F5: stored as (km3 * mult) for minor_rev < 2
 *     stored as (km3 * vol_mult) for minor_rev >= 2
 *
 * F6: stored as (km2 * mult) for minor_rev < 2
 *     stored as (km2 * area_mult) for minor_rev >= 2
 *
 * F7: stored as (m3/s * mult) for minor_rev < 2
 *     stored as (m3/s * flux_mult) for minor_rev >= 2
 *
 * F8: stored as (ktons * mult) for minor_rev < 2
 *     stored as (ktons * mass_mult) for minor_rev >= 2
 *
 * F9: stored as (val * vort_mult)
 *
 * F10:
 *   For minor rev < 2, used:
 *                      OLD_STORM_GRIDTYP_LATLON or
 *                      OLD_STORM_GRIDTYP_FLAT
 *   For minor rev >= 2, used:
 *                       PJG_LATLON or PJG_FLAT (toolsa/pjg.h)
 */

/*
 * storm file params - these are the parameters set by the user via
 * the environment file and govern the operations of the
 * identification algorithm
 */

typedef struct {

  si32 low_dbz_threshold;         /* dbz (F1) - low limit for dbz 
				   * values */

  si32 high_dbz_threshold;        /* dbz (F1) - high limit for dbz 
				   * values */

  si32 dbz_hist_interval;         /* dbz (F1) */
  si32 hail_dbz_threshold;        /* dbz (F1) - threshold above which
				   * precip is assumed to be hail */

  si32 base_threshold;            /* km (F1) - min ht for storm base -
				   * echo below this is ignored */

  si32 top_threshold;             /* km (F1) - max ht for storm top -
				   * echo above this is ignored */

  si32 merge_ht_threshold;        /* km (F1) - storms are only 
				   * considered merged of they connect
				   * above this ht */

  si32 min_storm_size;            /* km2 or km3 (F2) - min size for
			 	   * storm definition.  km2 used for
				   * 2D data; km3 used for 3D data. */

  si32 max_storm_size;            /* km2 or km3 (F2) - max size for
				   * storm definition.  km2 used for
				   * 2D data; km3 used for 3D data. */

  si32 z_p_coeff;                 /* Z - precip coeficient (F1) */
  si32 z_p_exponent;              /* Z - precip exponent (F1) */
  si32 z_m_coeff;                 /* Z - mass coeficient (F1) */
  si32 z_m_exponent;              /* Z - mass exponent (F1) */

  si32 sectrip_vert_aspect;       /* (F1) - vertical aspect ratio
				   * threshold above which storm
				   * is considered to be second trip */

  si32 sectrip_horiz_aspect;      /* (F1) - horizontal aspect ratio
				   * threshold above which storm
				   * is considered to be second trip */

  si32 sectrip_orientation_error; /* deg (F3) - error in ellipse
				   * orientation from radar within which
				   * storm is considered decond trip */

  si32 grid_type;                 /* type of projection used for grid
				   * from which storm props are computed
				   * (F10) */
 
  si32 vel_available;             /* vel data availability flag */

#ifdef STORM_V3_MINOR_REV_2
  si32 minor_rev;                 /* file type minor revision -
				   * before minor_rev 2, this slot
				   * was used for width_available
				   * flag */
#else
  si32 width_available;           /* width data availability flag */
#endif

  si32 n_poly_sides;              /* number of sides in storm shape
				   * polygons */

  si32 poly_start_az;             /* deg (F3) azimuth from T.N. for
				   * first polygon point */

  si32 poly_delta_az;             /* deg (F3) azimuth delta between
				   * polygon points (pos is
				   * counterclockwise) */

  si32 mult;                      /* multiplier - floating point values are
				   * multiplied by mult before being written
				   * to file (See F1) */

  si32 vort_mult;                 /* vorticity multiplier (see F9) */

} storm_v3_file_params_t;

/*
 * storm_v3_float_params_t - floating point params struct
 */

typedef struct {

  double low_dbz_threshold;
  double high_dbz_threshold;
  double dbz_hist_interval;
  double hail_dbz_threshold;
  double base_threshold;
  double top_threshold;
  double merge_ht_threshold;
  double min_storm_size;
  double max_storm_size;
  double z_p_coeff;
  double z_p_exponent;
  double z_m_coeff;
  double z_m_exponent;
  double sectrip_vert_aspect;
  double sectrip_horiz_aspect;
  double sectrip_orientation_error;
  double poly_start_az;
  double poly_delta_az;
  
} storm_v3_float_params_t;

/*
 * storm_v3_file_header_t - structure for header in storm properties
 * file
 */

#define N_STORM_V3_HEADER_LABELS 2

typedef struct {

  storm_v3_file_params_t params;   /* see above */

  si32 n_scans;                 /* number of completed scans in file */

  si32 data_file_size;          /* current file size in bytes - this refers
				 * to completed scans - if the program fails 
				 * while  writing a scan, the actual file
				 * may be longer, but the data for the 
				 * last scan will be unusable. Note that
				 * n_scans, file_size and end_timer are
				 * updated after the full scan has been
				 * written to the file */

  dattim_t file_time; 		/* file time - time of last write */
  dattim_t start_time;          /* start_time - time of first scan */
  dattim_t end_time;            /* end_time - time of last scan */

  si32 nbytes_char;             /* number of char bytes at end of
				 * the struct =
				 * N_STORM_V3_HEADER_LABELS * R_LABEL_LEN */
  
  char header_file_name[R_LABEL_LEN];
  char data_file_name[R_LABEL_LEN];

} storm_v3_file_header_t;

/*
 * storm_v3_file_scan_t - structure for header for a scan in a storm properties
 * file - there is one of these headers per scan. The position of this
 * struct in the file is given by scan_offset in the storm_v3_file_header (above)
 */

typedef struct {

  si32 nbytes_char;    /* number of characters at the end of this 
			* struct - R_LABEL_LEN * N_CART_PARAMS_LABELS
			* in the cart params struct */
  
  si32 scan_num;
  si32 nstorms;

  dattim_t time;

  si32 datum_longitude; /* (F3) long. of point from which storm locations
			 * are referenced */
  si32 datum_latitude;  /* (F3) lat. of point from which storm locations
			 * are referenced */

  si32 min_z;           /* km (F1) - msl ht of lowest layer in storm */
  si32 delta_z;         /* km (F1) - layer thickness */

  si32 gprops_offset;   /* file offset for the gprops array */
  si32 first_offset;    /* offset of first byte for data for this scan */
  si32 last_offset;     /* offset of last byte for data for this scan */
  
  cart_params_t cart;   /* cartesian params for this scan */

} storm_v3_file_scan_header_t;

typedef struct {

  double datum_longitude;
  double datum_latitude;
  double min_z;
  double delta_z;
  cart_float_params_t cart;

} storm_v3_float_scan_header_t;

/*
 * storm_v3_file_global_props_t - structure to store the global properties of the
 * storm, including offsets to the layer properties and the dbz histogram.
 * There is an array of 'nstorms' of these structures following each
 * scan header.
 */


typedef struct {

  si32 storm_num;

  si32 n_layers;                 /* the number of layers in this storm */
  si32 base_layer;               /* the layer number of the storm base */
  si32 n_dbz_intervals;          /* the number of intervals in the
				  * dbz distribution histograms */
  si32 n_runs;                   /* number of runs above threshold for
				  * this storm */

  si32 top_missing;              /* flag to indicate that top of storm was
				  * not sampled: set to 1 of top missing,
				  * 0 otherwise */

  si32 range_limited;            /* flag to indicate that storm was not
				  * fully sampled because of range limitations:
				  * set to 1 of range-limited,
				  * 0 otherwise */

  si32 second_trip;              /* flag to indicate that storm is probably
				  * second trip */

  si32 hail_present;             /* flag to indicate dBZ values above the
				  * hail threshold */

  si32 spare1;

  si32 vol_centroid_x;           /* (km or deg) (F4) */
  si32 vol_centroid_y;           /* (km or deg) (F4) */
  si32 vol_centroid_z;           /* km (F1) */
  si32 refl_centroid_x;          /* (km or deg) (F4) */
  si32 refl_centroid_y;          /* (km or deg) (F4) */
  si32 refl_centroid_z;          /* km (F1) */

  si32 top;                      /* km (F1) */
  si32 base;                     /* km (F1) */
  si32 volume;                   /* km3 (F5) */
  si32 area_mean;                /* km2 (F6) */
  si32 precip_flux;              /* m3/s (F7) */
  si32 mass;                     /* ktons (F8) */

  si32 tilt_angle;               /* deg (F3) */
  si32 tilt_dirn;                /* degT (F3) */

  si32 dbz_max;                  /* dbz (F1) */
  si32 dbz_mean;                 /* dbz (F1) */
  si32 ht_of_dbz_max;            /* km (F1) */

#ifdef STORM_V3_MINOR_REV_2
  si32 vol_mult;
  si32 area_mult;
  si32 mass_mult;
  si32 flux_mult;
#else
  si32 vel_mean;                 /* m/s (F1) */
  si32 vel_sd;                   /* m/s (F1) */
  si32 width_mean;               /* m/s (F1) */
  si32 width_sd;                 /* m/s (F1) */
#endif

  si32 vorticity;                /* /s (F9) */

  si32 precip_area;              /* km2 (F6) */
  si32 precip_area_centroid_x;   /* (km or deg) (F4) */
  si32 precip_area_centroid_y;   /* (km or deg) (F4) */
  si32 precip_area_orientation;  /* degT (F3) */
  si32 precip_area_minor_sd;     /* (km or deg) (F4) - standard dev of precip area
			          * along the minor axis of the precip area
				  * shape */
  si32 precip_area_major_sd;     /* (km or deg) (F4)  - standard dev of precip area
			          * along the major axis of the precip area
				  * shape */
  si32 precip_area_minor_radius; /* (km or deg) (F4) - minor radius of precip
				  * area ellipse */
  si32 precip_area_major_radius; /* (km or deg) (F4)  - standard dev of precip
				  * area along the major axis of the
				  * precip area shape */

  si32 proj_area;             /* km2 (F6) */
  si32 proj_area_centroid_x;  /* (km or deg) (F4) */
  si32 proj_area_centroid_y;  /* (km or deg) (F4) */
  si32 proj_area_orientation; /* degT (F3) */
  si32 proj_area_minor_sd;    /* (km or deg) (F4) - standard dev of proj
			       * area along the minor axis of the
			       * proj area shape */
  si32 proj_area_major_sd;    /* (km or deg) (F4)  - standard dev of proj
			       * area along the major axis of the
			       * proj area shape */
  si32 proj_area_minor_radius; /* (km or deg) (F4) - minor radius of proj
				* area ellipse */
  si32 proj_area_major_radius; /* (km or deg) (F4)  - standard dev of proj
				* area along the major axis of the
				* proj area shape */
  
  si32 spare2[36];             /* was precip_area_polygon - 
				* removed April 26 1995 */
  
  si32 proj_area_polygon[V3_N_POLY_SIDES]; /* in grid units, the length of
					 * rays from the proj area
					 * centroid to the polygon
					 * vertices (F4) */
  
  si32 layer_props_offset;       /* posn in file of layer props data */
  si32 dbz_hist_offset;          /* posn in file of dbz hist data */
  si32 runs_offset;              /* posn in file of runs data */

} storm_v3_file_global_props_t;

/*
 * storm_v3_float_global_props_t - floating point equivalent
 *
 * the flags are used to control which parts of the struct are
 * computed using RfDecodeStormProps()
 */

#define STORM_V3_POSITION 1
#define STORM_V3_SIZE 2
#define STORM_V3_PRECIP_AREA 4
#define STORM_V3_PROJ_AREA 8
#define STORM_V3_PROJ_POLYGON 16
#define STORM_V3_ALL_PROPS 31
 
typedef struct {

  /* STORM_V3_POSITION */

  double vol_centroid_x;
  double vol_centroid_y;
  double vol_centroid_z;
  double refl_centroid_x;
  double refl_centroid_y;
  double refl_centroid_z;
  double tilt_angle;
  double tilt_dirn;

  /*
   * STORM_V3_SIZE
   */

  double top;
  double base;
  double volume;
  double area_mean;
  double precip_flux;
  double mass;
  double dbz_max;
  double dbz_mean;
  double ht_of_dbz_max;
  double vorticity;

  /*
   * STORM_V3_PRECIP_AREA
   */

  double precip_area;
  double precip_area_centroid_x;
  double precip_area_centroid_y;
  double precip_area_orientation;
  double precip_area_minor_sd;
  double precip_area_major_sd;
  double precip_area_minor_radius;
  double precip_area_major_radius;
  
  /*
   * STORM_V3_PROJ_AREA
   */

  double proj_area;
  double proj_area_centroid_x;
  double proj_area_centroid_y;
  double proj_area_orientation;
  double proj_area_minor_sd;
  double proj_area_major_sd;
  double proj_area_minor_radius;
  double proj_area_major_radius;

  /*
   * STORM_V3_PROJ_POLYGON
   */

  double proj_area_polygon[V3_N_POLY_SIDES];

} storm_v3_float_global_props_t;

/*
 * storm_v3_file_layer_props_t - structure which holds the storm layer properties,
 * i.e. those which are not layer dependent.
 */

typedef struct {

  si32 vol_centroid_x;                        /* (km or deg) (F4) */
  si32 vol_centroid_y;                        /* (km or deg) (F4) */
  si32 refl_centroid_x;                       /* (km or deg) (F4) */
  si32 refl_centroid_y;                       /* (km or deg) (F4) */
  si32 area;                                  /* km2 (F6) */
  si32 dbz_max;                               /* dbz (F1) */
  si32 dbz_mean;                              /* dbz (F1) */
  si32 mass;                                  /* ktons (F8) */
#ifdef STORM_V3_MINOR_REV_2
  si32 spare[4];
#else
  si32 vel_mean, vel_sd;                      /* m/s (F1) */
  si32 width_mean, width_sd;                  /* m/s (F1) */
#endif
  si32 vorticity;                             /* /s (F9) */

} storm_v3_file_layer_props_t;

typedef struct {

  double vol_centroid_x;
  double vol_centroid_y;
  double refl_centroid_x;
  double refl_centroid_y;
  double area;
  double dbz_max;
  double dbz_mean;
  double mass;
  double vorticity;

} storm_v3_float_layer_props_t;

/*
 * storm_v3_file_dbz_hist_t - percentages of volume and precip area which
 * fall in the dbz histogram intervals
 */

typedef struct {

  si32 percent_volume;                        /* (F1) */
  si32 percent_precip_area;                   /* (F1) */

} storm_v3_file_dbz_hist_t;

typedef struct {

  double percent_volume;
  double percent_precip_area;

} storm_v3_float_dbz_hist_t;

/*
 * storm_v3_file_run_t - struct type for a run of storm data above threshold
 */

typedef struct {

  si16 ix;
  si16 iy;
  si16 iz;
  si16 n;

} storm_v3_file_run_t;

/*
 * storm_v3_file_index_t is a convenience structure which may be used for
 * referring to any or all component(s) of the storm properties file
 */

typedef struct {

  char *prog_name;

  /*
   * the  following must match rf_dual_index_t
   */

  char *header_file_path;
  char *header_file_label;
  FILE *header_file;
  char *data_file_path;
  char *data_file_label;
  FILE *data_file;
  int index_initialized;

  /*
   * memory-mapped IO
   */

  int use_mmio;

  /*
   * end of match with rf_dual_index_t
   */

  char *header_mmio_buf;
  size_t header_mmio_len;
  char *data_mmio_buf;
  size_t data_mmio_len;

  storm_v3_file_header_t *header;
  si32 *scan_offsets;
  storm_v3_file_scan_header_t *scan;
  storm_v3_file_global_props_t *gprops;
  storm_v3_file_layer_props_t *layer;
  storm_v3_file_dbz_hist_t *hist;
  storm_v3_file_run_t *runs;
  si32 storm_num;

  /*
   * the following are used to keep track of allocated memory
   */

  int header_allocated;
  int scan_allocated;
  int props_allocated;
  si32 n_scans_allocated;
  si32 max_storms;
  si32 max_layers;
  si32 max_dbz_intervals;
  si32 max_runs;

} storm_v3_file_index_t;

/*
 * semaphore constants for storm_ident, storm_track and
 * track_server programs
 */

#define N_STORM_V3_IDENT_SEMS 6

#define STORM_V3_IDENT_SETUP_IN_PROGRESS_SEM 0
#define STORM_V3_IDENT_ACTIVE_SEM 1
#define STORM_V3_TRACK_ACTIVE_SEM 2
#define STORM_V3_FILE_READABLE_SEM 3
#define STORM_V3_IDENT_QUIT_SEM 4
#define STORM_V3_FILES_BUSY_SEM 5

/*
 * flags for tracking mode
 */

#define RETRACK -1
#define PREPARE_NEW_FILE -2
#define PREPARE_FOR_APPEND -3
#define TRACK_LAST_SCAN -4

/*
 * shared memory for storm_ident, storm_track and track_server
 */

typedef struct {

  int realtime_data_available; /* set by storm_ident to let server
				* know when data is available
				* see also STORM_V3_AND_TRACK_FILE_SEM */
  
  int remove_track_file;       /* set by storm_ident on exit - if set
				* to TRUE, storm_track will remove the
				* track file before exiting */

  int tracking_mode;           /* set by storm_ident to indicate the
				* tracking mode to storm_track
				* PREPARE_NEW_FILE or
				* PREPARE_FOR_APPEND or
				* TRACK_LAST_SCAN
				*
				* Note: RETRACK mode is set by
				* storm_track itself, since storm_ident
				* is not run in retrack mode.
				*/
  
  char storm_header_file_path[MAX_PATH_LEN];
  char track_header_file_path[MAX_PATH_LEN];

  si32 time; /* latest time for storm and track files */

  si32 time_last_request; /* latest time track server was requested
			   * data by client */
  
  si32 seq_num;

  si32 n_scans_for_forecast; /* the number of scans used in computing
			      * the trends for the forecast.
			      * Set by storm_track.
			      * Used by storm_ident to determine the 
			      * number of scans to copy to the new file
			      * at 0Z - kav tracks only */

} storm_v3_tracking_shmem_t;

#endif
 
#ifdef __cplusplus
}
#endif
