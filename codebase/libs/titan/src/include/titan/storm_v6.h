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
 * storm_v6.h - header file for Version 5 storm structures
 *
 * Mike Dixon RAP NCAR
 * March 2014
 *
 **************************************************************************/

#ifndef titan_storm_v6_h
#define titan_storm_v6_h

#ifdef __cplusplus
extern "C" {
#endif

#include <titan/file_io.h>
#include <rapformats/titan_grid.h>
#include <titan/titan_hail.h>

#define STORM_HEADER_FILE_TYPE_V6 "Storm header type 6"
#define STORM_DATA_FILE_TYPE_V6 "Storm data type 6"

#define STORM_HEADER_FILE_EXT_V6 "sh6"
#define STORM_DATA_FILE_EXT_V6 "sd6"

#define STORM_FILE_MAJOR_REV_V6 6
#define STORM_FILE_MINOR_REV_V6 1

  /*
   * Storm file formats.
   *
   * There are two files for each date - a header file and a data file.
   *
   * Header file.
   * -----------
   *
   * file label - char[R_FILE_LABEL_LEN]
   *
   * header - struct storm_file_header_t
   *
   * si32 scan_offset[header->n_scans] - file offset of each scan header
   *
   * Data file.
   * ---------
   *
   * file label - char[R_FILE_LABEL_LEN]
   *
   * for each scan :
   *
   *  at scan_offset[iscan]:
   *    struct storm_file_scan_header_t
   *
   *  followed by
   *
   *  at scan_header->gprops_offset:
   *    array of structs storm_file_global_props_t[nstorms],
   *      where nstorms is scan_header.nstorms
   *
   *  followed by
   *
   *  for each storm in the scan :
   *
   *    at gprops[istorm].layer_props_offset:
   *      array of structs storm_file_layer_props_t[n_layers],
   *        where n_layers is gprops[istorm].n_layers
   * 
   *    at gprops[istorm].dbz_hist_offset:
   *      array of structs storm_file_dbz_hist_t[n_dbz_intervals],
   *        where n_dbz_intervals is gprops[istorm].n_dbz_intervals
   *
   *    at gprops[istorm].runs_offset:
   *      array of structs storm_file_run_t[n_runs],
   *        where n_runs is gprops[istorm].n_runs
   *
   *    at gprops[istorm].proj_runs_offset:
   *      array of structs storm_file_run_t[n_proj_runs],
   *        where n_runs is gprops[istorm].n_proj_runs
   *
   * NOTE 1.
   * 
   * The offsets of scan headers in the file are stored in the array
   * scan_offset[].
   *
   * The global props for the storms are in the 'storm_file_global_props_t'
   * array, which are pointed to by gprops_offset in the scan header.
   *
   * The position of the two arrays of types 'storm_file_layer_props_t',
   * 'storm_file_dbz_hist_t' and storm_file_run_t are given by the entries
   * 'layer_props_offset', 'dbz_hist_offset', 'runs_offset' and
   * 'proj_runs_offset'.
   *
   * It is therefore possible to move reasonably directly to the data
   * for a particular storm, knowing the scan number and the storm number.
   *
   * NOTE 2.
   *
   * Integer values are stored as IEEE 32-bit intergers.
   * Floating point values are stored as IEEE 32-bit floats.
   * All ints and floats are stored in big-endian format.
   *
   * NOTE 3.
   *
   * The storm data is based on two types of data grid, a local-area
   * flat grid, and a lot-lon grid. The grid type is either TITAN_PROJ_FLAT
   * or TITAN_PROJ_LATLON.
   *
   * The position and shape properties of the storms are in either
   * in km or degrees, depending upon which grid was used.
   * Units are in km for flat grid, degrees for lat-lon grid.
   *
   * The area, volume and speed properties are always in km units.
   */

  /*
   * storm file params - these are the parameters set by the user via
   * the environment file and govern the operations of the
   * identification algorithm
   */

  typedef struct {

    fl32 low_dbz_threshold;	/* dbz - low limit for dbz 
				 * values */

    fl32 high_dbz_threshold;	/* dbz - high limit for dbz 
				 * values */

    fl32 dbz_hist_interval;	/* dbz */

    fl32 hail_dbz_threshold;	/* dbz - threshold above which
				 * precip is assumed to be hail */

    fl32 base_threshold;	/* km - min ht for storm base -
                                 * echo below this is ignored */

    fl32 top_threshold;		/* km - max ht for storm top -
				 * echo above this is ignored */

    fl32 min_storm_size;   	/* km2 or km3 - min size for
				 * storm definition (km2 for 2D
				 * data; km3 for 3D data) */

    fl32 max_storm_size;	/* km2 or km3 - max size for
                                 * storm definition (km2 for 2D
                                 * data; km3 for 3D data) */

    fl32 morphology_erosion_threshold; /* threshold to which morphology
                                        * erosion is performed (km) */

    fl32 morphology_refl_divisor; /* The morphology field is obtained
                                   * by adding the euclidean distance
                                   * to storm edge (km) to the
                                   * reflectivity excess (above
                                   * threshold) divided by this
                                   * value (dbz/km) */
  
    fl32 min_radar_tops;	/* (km) min tops for valid radar data -
                                 * if checked */

    fl32 tops_edge_margin;	/* (km) margin placed on min_tops
				 * field to allow for tilted
				 * storms */

    fl32 z_p_coeff;		/* Z - precip coeficient */
    fl32 z_p_exponent;		/* Z - precip exponent */
    fl32 z_m_coeff;		/* Z - mass coeficient */
    fl32 z_m_exponent;		/* Z - mass exponent */

    fl32 sectrip_vert_aspect;	/* - vertical aspect ratio
				 * threshold above which storm
				 * is considered to be second trip */

    fl32 sectrip_horiz_aspect;	/* - horizontal aspect ratio
				 * threshold above which storm
				 * is considered to be second trip */

    fl32 sectrip_orientation_error; /* deg - error in ellipse
                                     * orientation from radar within which
                                     * storm is considered decond trip */

    fl32 poly_start_az;		/* deg azimuth from T.N. for
				 * first polygon point */

    fl32 poly_delta_az;		/* deg azimuth delta between
				 * polygon points (pos is
				 * counterclockwise) */

    si32 check_morphology;	/* check_morphology flag */

    si32 check_tops;		/* check_tops flag */

    si32 vel_available;		/* vel data availability flag */

    si32 n_poly_sides;		/* number of sides in storm shape
				 * polygons */

    fl32 ltg_count_time;        /* number of seconds over which the ltg
                                 * strikes are counted. */

    fl32 ltg_count_margin_km;   /* margin from storm edge to describe
                                 * outer region for counting ltg strikes */

    fl32 hail_z_m_coeff;        /* Z - mass coeficient */

    fl32 hail_z_m_exponent;     /* Z - mass exponent */

    fl32 hail_mass_dbz_threshold; /* dbz - threshold above which
                                   * hail mass is calculated */

    fl32 tops_dbz_threshold;	/* dbz threshold for computing storm tops */

    si32 precip_computation_mode; /* mode for computing precip */
    /* TITAN_PRECIP_FROM_COLUMN_MAX: precip computed from col-max dBZ */
    /* TITAN_PRECIP_AT_SPECIFIED_HT: precip computed at specified height */
    /* TITAN_PRECIP_AT_LOWEST_VALID_HT: precip computed at lowest valid CAPPI */
    /* TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL: precip computed from lowest non-missing dbz */

    fl32 precip_plane_ht;  /* CAPPI ht for which precip is computed (km MSL)
                            * See precip_computation_mode */

    si32 spare[31];
    
  } storm_file_params_v6_t;

  /*
   * storm_file_header_t - structure for header in storm properties
   * file
   */

#define STORM_FILE_HEADER_NBYTES_CHAR_V6 (2 * RF_FILE_NAME_LEN)

  typedef struct {

    si64 file_time;    /* filetime - time of last write */
    si64 start_time;   /* start_time - time of first scan */
    si64 end_time;     /* end_time - time of last scan */

    si64 data_file_size;  /* current file size in bytes - this refers
                           * to completed scans - if the program fails 
                           * while  writing a scan, the actual file
                           * may be longer, but the data for the 
                           * last scan will be unusable. Note that
                           * n_scans, file_size and end_timer are
                           * updated after the full scan has been
                           * written to the file */

    si64 spare64[4];

    storm_file_params_v6_t params; /* see above */

    si32 major_rev;
    si32 minor_rev;
    
    si32 n_scans;     /* number of completed scans in file */

    si32 nbytes_char;	/* number of char bytes at end of
                         * the struct =
                         * N_STORM_HEADER_LABELS * R_LABEL_LEN */
  
    si32 spare32[132];
    
    char header_file_name[RF_FILE_NAME_LEN];
    char data_file_name[RF_FILE_NAME_LEN];

  } storm_file_header_v6_t;

  /*
   * storm_file_scan_t - structure for header for a scan in a storm props
   * file - there is one of these headers per scan. The position of this
   * struct in the file is given by scan_offset in the storm_file_header
   * (see above)
   */

  typedef struct {

    si64 time;

    si64 gprops_offset;		/* file offset for the gprops array */
    si64 first_offset;		/* offset of first byte for data for
				 * this scan */
    si64 last_offset;		/* offset of last byte for data for
				 * this scan */

    si64 spare64[4];

    fl32 min_z;			/* km - msl ht of lowest layer in storm */
    fl32 delta_z;			/* km - layer thickness */

    si32 scan_num;
    si32 nstorms;

    si32 nbytes_char;		/* number of characters at the end of this 
				 * struct,
				 * TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN
				 * in the storm_grid_t struct */

    fl32 ht_of_freezing;          /* Height of freezing in km */

    si32 spare32[42];
  
    titan_grid_t grid;            /* cartesian params for this scan */

  } storm_file_scan_header_v6_t;

  /*
   * storm_file_global_props_t - structure to store the global props of the
   * storm, including offsets to the layer properties and the dbz histogram.
   * There is an array of 'nstorms' of these structures following each
   * scan header.
   */

  typedef struct {

    si64 layer_props_offset;	/* posn in file of layer props data */
    si64 dbz_hist_offset;		/* posn in file of dbz hist data */
    si64 runs_offset;		/* posn in file of runs data */
    si64 proj_runs_offset;	/* posn in file of proj area runs data */

    si64 spare[4];

    fl32 vol_centroid_x;		/* (km or deg) (NOTE 3) */
    fl32 vol_centroid_y;		/* (km or deg) (NOTE 3) */
    fl32 vol_centroid_z;		/* km */

    fl32 refl_centroid_x;		/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_y;		/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_z;		/* km */

    fl32 top;			/* km */
    fl32 base;			/* km */
    fl32 volume;			/* km3 */
    fl32 area_mean;		/* km2 */
    fl32 precip_flux;		/* m3/s */
    fl32 mass;			/* ktons */

    fl32 tilt_angle;		/* deg */
    fl32 tilt_dirn;		/* degT */

    fl32 dbz_max;			/* dbz */
    fl32 dbz_mean;		/* dbz */
    fl32 dbz_max_gradient;	/* dbz/km */
    fl32 dbz_mean_gradient;	/* dbz/km */
    fl32 ht_of_dbz_max;		/* km */

    fl32 rad_vel_mean;		/* m/s */
    fl32 rad_vel_sd;		/* m/s */
    fl32 vorticity;		/* /s */

    fl32 precip_area;		/* km2 */
    fl32 precip_area_centroid_x;	/* (km or deg) (NOTE 3) */
    fl32 precip_area_centroid_y;	/* (km or deg) (NOTE 3) */
    fl32 precip_area_orientation;	/* degT */
    fl32 precip_area_minor_radius; /* (km or deg)(NOTE 3) - minor radius
                                    * of precip area ellipse */
    fl32 precip_area_major_radius; /* (km or deg) (NOTE 3)  - standard dev
                                    * of precip area along the major axis
                                    * of the precip area shape */
    
    fl32 proj_area;		/* km2 */
    fl32 proj_area_centroid_x;	/* (km or deg) (NOTE 3) */
    fl32 proj_area_centroid_y;	/* (km or deg) (NOTE 3) */
    fl32 proj_area_orientation;	/* degT */
    fl32 proj_area_minor_radius;	/* (km or deg) (NOTE 3) - minor radius
                                         * of proj area ellipse */
    fl32 proj_area_major_radius;	/* (km or deg) (NOTE 3)  - standard dev
                                         * of proj area along the major axis of the
                                         * proj area shape */
  
    fl32 proj_area_polygon[N_POLY_SIDES]; /* in grid units, the length of
                                           * rays from the proj area
                                           * centroid to the polygon
                                           * vertices (NOTE 3) */

    si32 storm_num;

    si32 n_layers;		/* the number of layers in this storm */

    si32 base_layer;		/* the layer number of the storm base */

    si32 n_dbz_intervals;		/* the number of intervals in the
                                         * dbz distribution histograms */

    si32 n_runs;			/* number of runs above threshold for
                                         * this storm */

    si32 n_proj_runs;		/* number of runs in the projected area
				 * for this storm */

    si32 top_missing;		/* flag to indicate that top of storm was
				 * not sampled: set to 1 of top missing,
				 * 0 otherwise */

    si32 range_limited;		/* flag to indicate that storm was not
				 * fully sampled because of range limitations:
				 * set to 1 of range-limited,
				 * 0 otherwise */

    si32 second_trip;		/* flag to indicate that storm is probably
				 * second trip */

    si32 hail_present;		/* flag to indicate dBZ values above the
				 * hail threshold */

    si32 anom_prop;               /* flag to indicate anomalous propagation */

    si32 bounding_min_ix;         /* proj_area (x,y) bounding box */
    si32 bounding_min_iy;         /* in grid coords */
    si32 bounding_max_ix;
    si32 bounding_max_iy;
  
    fl32 vil_from_maxz;
    fl32 ltg_count;               /* ltg strike count in x minutes before storm
                                   * time with x km of storm */
    fl32 user_data[4];           /* user-specified purpose */
    
    titan_hail_t hail_metrics;  /* hail indicies from titan/titan_hail.h */

    si32 spare32[108];

  } storm_file_global_props_v6_t;

  /*
   * storm_file_layer_props_t
   * structure which holds the storm layer properties,
   * i.e. those which are not layer dependent.
   */

  typedef struct {

    fl32 vol_centroid_x;		/* (km or deg) (NOTE 3) */
    fl32 vol_centroid_y;		/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_x;		/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_y;		/* (km or deg) (NOTE 3) */
    fl32 area;			/* km2 */
    fl32 dbz_max;			/* dbz */
    fl32 dbz_mean;		/* dbz */
    fl32 mass;			/* ktons */
    fl32 rad_vel_mean;		/* m/s */
    fl32 rad_vel_sd;		/* m/s */
    fl32 vorticity;		/* /s */
    fl32 spare[13];

  } storm_file_layer_props_v6_t;

  /*
   * storm_file_dbz_hist_t - percentages of volume and precip area which
   * fall in the dbz histogram intervals
   */

  typedef struct {

    fl32 percent_volume;
    fl32 percent_area;
    fl32 spare[2];

  } storm_file_dbz_hist_v6_t;

  /*
   * storm_file_run_t - struct type for a run of storm data above threshold
   */

  typedef struct {

    ui16 ix;
    ui16 iy;
    ui16 iz;
    ui16 n;

  } storm_file_run_v6_t;

  /*
   * prototypes
   */
 
  extern void RfPrintStormHistV6(FILE *out,
                                 const char *spacer,
                                 const storm_file_params_v6_t *params,
                                 const storm_file_global_props_v6_t *gprops,
                                 const storm_file_dbz_hist_v6_t *hist);
  
  extern void RfPrintStormHistXMLV6(FILE *out,
                                    const char *spacer,
                                    const storm_file_params_v6_t *params,
                                    const storm_file_global_props_v6_t *gprops,
                                    const storm_file_dbz_hist_v6_t *hist);
  
  extern void RfPrintStormLayerV6(FILE *out,
                                  const char *spacer,
                                  const storm_file_params_v6_t *params,
                                  const storm_file_scan_header_v6_t *scan,
                                  const storm_file_global_props_v6_t *gprops,
                                  const storm_file_layer_props_v6_t *layer);
  
  extern void RfPrintStormLayerXMLV6(FILE *out,
                                     const char *spacer,
                                     const storm_file_params_v6_t *params,
                                     const storm_file_scan_header_v6_t *scan,
                                     const storm_file_global_props_v6_t *gprops,
                                     const storm_file_layer_props_v6_t *layer);
  
  extern void RfPrintStormParamsV6(FILE *out,
                                   const char *spacer,
                                   const storm_file_params_v6_t *params);
  
  extern void RfPrintStormPropsV6(FILE *out,
                                  const char *spacer,
                                  const storm_file_params_v6_t *params,
                                  const storm_file_scan_header_v6_t *scan,
                                  const storm_file_global_props_v6_t *gprops);
  
  extern void RfPrintStormPropsXMLV6(FILE *out,
                                     const char *spacer,
                                     const storm_file_params_v6_t *params,
                                     const storm_file_scan_header_v6_t *scan,
                                     const storm_file_global_props_v6_t *gprops);
  
  extern void RfPrintStormProjRunsV6(FILE *out,
                                     const char *spacer,
                                     const storm_file_global_props_v6_t *gprops,
                                     const storm_file_run_v6_t *proj_runs);
  
  extern void RfPrintStormProjRunsXMLV6(FILE *out,
                                        const char *spacer,
                                        const storm_file_global_props_v6_t *gprops,
                                        const storm_file_run_v6_t *proj_runs);
  
  extern void RfPrintStormRunsV6(FILE *out,
                                 const char *spacer,
                                 const storm_file_global_props_v6_t *gprops,
                                 const storm_file_run_v6_t *runs);
  
  extern void RfPrintStormRunsXMLV6(FILE *out,
                                    const char *spacer,
                                    const storm_file_global_props_v6_t *gprops,
                                    const storm_file_run_v6_t *runs);
  
  extern void RfPrintStormScanV6(FILE *out,
                                 const char *spacer,
                                 const storm_file_params_v6_t *params,
                                 const storm_file_scan_header_v6_t *scan);
  
  extern void RfPrintStormScanXMLV6(FILE *out,
                                    const char *spacer,
                                    const storm_file_params_v6_t *params,
                                    const storm_file_scan_header_v6_t *scan);
  
  extern void RfStormGpropsEllipses2KmV6(storm_file_scan_header_v6_t *scan,
                                         storm_file_global_props_v6_t *gprops);
  
  extern void RfStormGpropsXY2LatLonV6(storm_file_scan_header_v6_t *scan,
                                       storm_file_global_props_v6_t *gprops);
  
#ifdef __cplusplus
}
#endif

#endif

