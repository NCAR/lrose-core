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
 * TitanData.hh
 *
 * Classes for Titan storm and track dataK - header file for storm track data
 *
 * Mike Dixon
 *
 * EOL NCAR Boulder CO USA
 *
 * July 2025
 *
 **************************************************************************/

#ifndef TitanData_hh
#define TitanData_hh

#include <titan/storm.h>

/*
 * StormFileParams
 * these are the parameters that govern the operations of the
 * identification algorithm
 */

class StormFileParams {

public:
  
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
    
}; // StormFileParams

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
 * major rev 5, Jan 6 1996
 * minor rev 1, Jan 6 1996
 */

#define TRACK_FILE_MAJOR_REV_V6 6
#define TRACK_FILE_MINOR_REV_V6 1

#define TRACK_HEADER_FILE_TYPE_V6 "Track header type 6"
#define TRACK_DATA_FILE_TYPE_V6 "Track data type 6"

#define TRACK_HEADER_FILE_EXT_V6 "th6"
#define TRACK_DATA_FILE_EXT_V6 "td6"

  /*
   * max number of simple tracks in a complex track
   */

#define MAX_PARENTS_V6 8  /* max number of parents allowed per track */
#define MAX_CHILDREN_V6 8 /* max number of children allowed per track */

  /*
   * the max number of weights for the linear forecasts
   */
  
#define MAX_NWEIGHTS_FORECAST_V6 10

  /* 
   * TRACK FILE FORMATS
   * ------------------
   *
   * There are two files for each date - a header file and a data file.
   *
   * Header file.
   * -----------
   *
   * file label - char[R_FILE_LABEL_LEN]
   *
   * header - struct track_file_header_t
   *
   * si32 complex_track[header->n_complex_tracks]
   *   this stores the numbers of the complex tracks, which are in
   *   ascending order but not necessarily contiguous.
   *
   * si32 complex_track_offset[header->n_complex_tracks]
   *   these are the file offsets to the complex_track_params_t struct
   *   for each complex track.
   *
   * si32 simple_track_offset[header->n_simple_tracks]
   *   these are the file offsets to the simple_track_params_t struct
   *   for each simple track.
   *
   * track_file_scan_index_t scan_index[header->n_scans]
   *   info about the track entry list for a given scan number
   *
   * si32 n_simples_per_complex[header->n_simple_tracks]
   *   This array lists the number of simple tracks per complex track
   *
   * si32 simples_per_complex_offsets[header->n_simple_tracks]
   *   This array lists the offsets for the arrays which contain
   *   the simple track numbers for each complex track
   *
   * For each complex track:
   *
   *   si32 simples_per_complex[n_simples_per_complex[icomplex]]
   *     This array holds the simple track numbers for the
   *     relevant complex track number. The array offsets are
   *     stored in simples_per_complex_offsets.
   *
   * Data file.
   * ---------
   *
   * file label - char[R_FILE_LABEL_LEN] followed by
   * track_data (described below).
   *
   * The track data is made up of the following structs:
   *
   *   complex_track_params_t
   *   simple_track_params_t
   *   track_file_entry_t
   *
   * Each complex track has a complex_track_params_t struct.
   * The offset of this struct is obtained from the complex_track_offset
   * array in the header file.
   * 
   * Each complex track is made up of one or more simple tracks.
   * Each of these has a simple_track_params_t struct pointed at by the
   * simple_track_offset array in the header file.
   *
   * Each simple track is made up of one or more track entries.
   * The entries for a simple track are stored as a doubly-linked list.
   * The offset to the first entry in the list is obtained from the
   * simple_track_params_t struct (first_entry_offset).
   *
   * The entries for a given scan are also stored as a singly-linked list.
   * The offset to the first entry in the list is obtained from the
   * scan index array in the header file. Using this list allows one
   * to get data on all of the entries at a given time (scan).
   *
   * DATA RETRIEVAL
   * --------------
   *
   * A track is made up of a complex track and one or more simple tracks.
   * A simple track is a part of a complex track which has no mergers or
   * splits in it. A complex track is therefore made up of a number of 
   * simple tracks.
   *
   * To retrieve a complex track, find its position in the array by searching
   * for the track number in complex_track_num[].
   *
   * Then read the complex_track_params_t struct at the offset given by
   * complex_track_offset[].
   *
   * Then for each of the simple tracks in the complex track
   * read the simple_track_params_t at the offset
   * given by simple_track_offset[].
   *
   * The first entry in a simple track is pointed to by first_entry_offset
   * in the simple params. Read in the first entry. The entries form a
   * linked list, with the next_entry_offset pointing to the next
   * entry in the simple track.
   *
   * You may also retrieve the entries relevant to a particular scan.
   * The scan_index array contains the number of entries and the offset to
   * the first entry for that scan. Read in the first track entry, and then
   * follow a (second) linked  list to other entries in the scan by using the
   * next_scan_entry_offset values in the entry.
   */

#define N_MOVEMENT_PROPS_FORECAST_V6 5
#define N_GROWTH_PROPS_FORECAST_V6 5
#define N_TOTAL_PROPS_FORECAST_V6 14

  typedef struct {

    fl32 proj_area_centroid_x;	/* km/hr or deg/hr */
    fl32 proj_area_centroid_y;	/* km/hr or deg/hr */
    fl32 vol_centroid_z;          /* km/hr */
    fl32 refl_centroid_z;         /* km/hr */
    fl32 top;			/* km/hr */
    fl32 dbz_max;			/* dbz/hr */
    fl32 volume;			/* km3/hr */
    fl32 precip_flux;             /* (m3/s)/hr */
    fl32 mass;                    /* ktons/hr */
    fl32 proj_area;		/* km2/hr */
    fl32 smoothed_proj_area_centroid_x; /* km/hr or deg/hr */
    fl32 smoothed_proj_area_centroid_y; /* km/hr or deg/hr */
    fl32 smoothed_speed;
    fl32 smoothed_direction;
    fl32 spare[4];

  } track_file_forecast_props_v6_t;

  /*
   * track file params - some of these are the parameters set by the user via
   * the environment file
   */

  typedef struct {

    /*
     * weights for computing forecasts
     */

    fl32 forecast_weights[MAX_NWEIGHTS_FORECAST_V6];
  
    fl32 weight_distance;		      /* weight for distance moved
                                               * in tracking algorithm */

    fl32 weight_delta_cube_root_volume; /* weight for delta of
                                         * cube root of volume in
                                         * tracking algorithm */

    fl32 merge_split_search_ratio;      /* ratio of search radii used
                                         * when searching for mergers or
                                         * splits */

    fl32 max_tracking_speed;	      /* km/hr */

    fl32 max_speed_for_valid_forecast;  /* km/hr */

    fl32 parabolic_growth_period;  /* number of seconds for positive growth -
                                    * this is used with the PARABOLA
                                    * trend growth forecast */

    fl32 smoothing_radius;	      /* km */

    fl32 min_fraction_overlap;     /* This is the min individual fraction
                                    * overlap for a valid match */

    fl32 min_sum_fraction_overlap; /* When detetmining the overlap of a
                                    * storm at successive times, we sum
                                    * the overlap as a fraction of the
                                    * storm area at time1 and time2. This is
                                    * the min sum for a valid match */

    fl32 spare[9];

    si32 scale_forecasts_by_history;  /* flag for scaling the forecasts
                                       * based on the ratio of the history
                                       * to min_history_for_valid_forecast */

    si32 use_runs_for_overlaps;   /* flag for using runs to find the overlaps
                                   * for mergers and splits */

    si32 grid_type;		/* TITAN_PROJ_FLAT or TITAN_PROJ_LATLON */

    si32 nweights_forecast;	/* number of weighted points to be used in
				 * the forecast data */
  
    si32 forecast_type;		/* FORECAST_BY_TREND,
				 * FORECAST_BY_PARABOLA, or
				 * FORECAST_BY_REGRESSION */

    si32 max_delta_time;		/* secs */

    si32 min_history_for_valid_forecast; /* secs */

    si32 spatial_smoothing;	/* TRUE or FALSE */

  } track_file_params_v6_t;

  /*
   * track_file_verify_t - structure for the data used for forecast
   * verification
   */

  typedef struct {

    si64 end_time;		/* last scan time for which verification
				 * has been performed */

    si64 spare64[3];

    si32 verification_performed;	/* set to 1 if true, i.e. if
                                         * verification has been performed,
                                         * 0 if false */

    si32 forecast_lead_time;	/* lead time for forecast 
				 * verification (secs) */

    si32 forecast_lead_time_margin; /* the accuracy with which the forecast
                                     * time is used (secs). For a scan to
                                     * be used, its time must fall within
                                     * this margin of the forecast time */

    si32 forecast_min_history;	/* min history for valid forecasts -
				 * (secs) - when track duration is less
				 * than this, forecast is not
				 * considered valid */

    si32 verify_before_forecast_time; /* include in the analysis those
                                       * scans in a track which have
                                       * a duration less than the 
                                       * forecast lead time - the
                                       * default should be false */

    si32 verify_after_track_dies; /* include in the analysis those
                                   * scans after a track has died -
                                   * the default should be true */
  
    si32 spare[3];

    titan_grid_t grid;           /* describes the cartesian grid
                                  * used for verification -
                                  * has character data at the end - 
                                  * GRID_LABEL_LEN * N_GRID_STRUCT_LABELS
                                  * bytes */

  } track_file_verify_v6_t;

  /*
   * contingency data for forecast verification
   */

  typedef struct {

    fl32 n_success;
    fl32 n_failure;
    fl32 n_false_alarm;
    fl32 spare;

  } track_file_contingency_data_v6_t;

  /*
   * track_file_header_t - structure for header in track data file
   */

#define TRACK_FILE_HEADER_NBYTES_CHAR_V6 (3 * RF_FILE_NAME_LEN)

  typedef struct {

    si64 file_time;		/* file time - time of last write */
    si64 spare64[3];

    si32 major_rev;
    si32 minor_rev;

    si32 file_valid;		/* set to 0 when file is under
				 * modification, 1 when modification is
				 * successfully complete */

    si32 modify_code;		/* a unique code inserted when file
				 * modification begins  - used for 
				 * restarting tracking after program
				 * is stopped for any reason */

    track_file_params_v6_t params;	/* see above */

    si32 n_simple_tracks;		/* number of simple tracks in file */
    si32 n_complex_tracks;	/* number of complex tracks in file */
    
    track_file_contingency_data_v6_t ellipse_verify; /* contingency table data for
                                                      * forecast verification 
                                                      * using ellipses */
    
    track_file_contingency_data_v6_t polygon_verify; /* contingency table data for
                                                      * forecast verification 
                                                      * using polygons */

    track_file_forecast_props_v6_t forecast_bias; /* bias error for
                                                   * forecast props */
  
    track_file_forecast_props_v6_t forecast_rmse; /* root mean squared error for
                                                   * forecast props */

    si32 n_samples_for_forecast_stats;

    si32 n_scans;			/* the number of scans in the file */

    si32 last_scan_num;		/* the number of the last scan
				 * in the file = n_scans - 1 */

    si32 max_simple_track_num;	/* the highest simple track num in
				 * the file */

    si32 max_complex_track_num;	/* the highest complex track num in
				 * the file */

    si32 data_file_size;		/* current data file size in bytes */

    si32 max_parents;
    si32 max_children;
    si32 max_nweights_forecast;

    si32 spare[9];
  
    si32 nbytes_char;		/* number of character bytes at the
				 * end of this struct */

    track_file_verify_v6_t verify; /* see above - has character data at the end -
                                    * GRID_LABEL_LEN * N_GRID_STRUCT_LABELS
                                    * bytes */

    char header_file_name[RF_FILE_NAME_LEN]; /* name of the track header file */
    char data_file_name[RF_FILE_NAME_LEN];   /* name of the track data file */
    
    char storm_header_file_name[RF_FILE_NAME_LEN]; /* name of storm header file
                                                    * with which the track files
                                                    * are paired */

  } track_file_header_v6_t;

  /*
   * simple track parameters
   */

  typedef struct {

    si64 start_time;		/* for the simple track only */
    si64 end_time;		/* for the simple track only */

    si64 last_descendant_end_time; /* end time for the last descendant
                                    * of this storm */

    si64 time_origin;		/* time for first storm in any branch which
				 * led to this entry */

    si64 first_entry_offset;	/* file offset of first track entry struct */

    si64 spare64[3];
    
    si32 simple_track_num;
    si32 last_descendant_simple_track_num; /* simple track num of last
                                            * descendant of this storm */

    si32 start_scan;
    si32 end_scan;
    si32 last_descendant_end_scan; /* end scan of last descendant
                                    * of this storm */

    si32 scan_origin;		/* scan for first storm in any branch which
				 * led to this simple track */

    si32 history_in_scans;	/* number of scans of history data for
				 * making forecast - in the case of a merger
				 * or a split, this will be longer than the
				 * duration in scans */

    si32 history_in_secs;       /* time in secs since the start of the
                                 * earliest branch of this storm */
    
    si32 duration_in_scans;	/* duration of this simple track */
    si32 duration_in_secs;	/* duration of this simple track */

    si32 nparents, nchildren;
    si32 parent[MAX_PARENTS_V6]; /* track numbers of parents */
    si32 child[MAX_CHILDREN_V6]; /* track numbers of children */

    si32 complex_track_num;	/* number of track complex if track is part
				 * of a complex */

  } simple_track_params_v6_t;

  /*
   * complex track params
   */

  typedef struct {

    si64 start_time;
    si64 end_time;

    si64 spare64[6];

    fl32 volume_at_start_of_sampling; /* km3 */
    fl32 volume_at_end_of_sampling;   /* km3 */

    fl32 spare[3];
    
    si32 complex_track_num;

    si32 start_scan;
    si32 end_scan;

    si32 duration_in_scans;
    si32 duration_in_secs;

    si32 n_simple_tracks;		/* number of simple tracks in this
                                         * complex track */

    /*
     * the following are flags to indicate status:
     * a) number of scans for which top was not completely sampled
     * b) number of scans for which storm sampling was incomplete
     *    due to range
     * c) storm existed when sampling began - start volume stored
     * d) storm existed when sampling ended - end volume stored
     */

    si32 n_top_missing;
    si32 n_range_limited;
    si32 start_missing;
    si32 end_missing;
    si32 n_samples_for_forecast_stats;

    track_file_contingency_data_v6_t ellipse_verify; /* contingency table data for
                                                      * forecast verification 
                                                      * using ellipses */
    
    track_file_contingency_data_v6_t polygon_verify; /* contingency table data for
                                                      * forecast verification 
                                                      * using polygons */

    track_file_forecast_props_v6_t forecast_bias; /* bias error for
                                                   * forecast props */
    
    track_file_forecast_props_v6_t forecast_rmse; /* root mean squared error for
                                                   * forecast props */
    
  } complex_track_params_v6_t;

  /*
   * track entry in file
   */

  typedef struct {

    si64 time;			/* time for this entry */
    si64 time_origin;		/* time for first storm in any branch which
				 * led to this entry */

    si64 prev_entry_offset;	/* track file offset to previous entry in
				 * in track - doubly linked list */

    si64 this_entry_offset;	/* track file offset to this entry */

    si64 next_entry_offset;	/* track file offset to next entry in
				 * track - doubly linked list */

    si64 next_scan_entry_offset;	/* offset to next entry in this scan */

    si64 spare64[6];
    
    si32 scan_origin;		/* scan for first storm in any branch which
				 * led to this entry */

    si32 scan_num;		/* scan number in storm file */
    si32 storm_num;		/* storm number in scan */

    si32 simple_track_num;	/* simple track number - should match the
				 * number in the simple_params struct */
    si32 complex_track_num;

    si32 history_in_scans;	/* number of scans of history data for
				 * making forecast - in the case of a merger
				 * or a split, this will be longer than the
				 * duration in scans */

    si32 history_in_secs;		/* time in secs since the start of the
                                         * earliest branch of this storm */

    si32 duration_in_scans;	/* duration of the simple track */
    si32 duration_in_secs;	/* duration of the simple track */

    si32 forecast_valid;		/* flag to indicate if the forecast is
                                         * valid */

    track_file_forecast_props_v6_t dval_dt; /* rate of change of each forecast
                                             * value */

  } track_file_entry_v6_t;

  /*
   * structure for keeping track of entries which exist for a given
   * scan
   */

  typedef struct {

    si64 utime;
    si64 first_entry_offset;
    si64 spare64[2];

    si32 n_entries;
    si32 spare32[7];

  } track_file_scan_index_v6_t;

  /*
   * prototypes
   */

  extern void RfPrintComplexTrackParamsV6(FILE *out,
                                          const char *spacer,
                                          int verification_performed,
                                          const track_file_params_v6_t *params,
                                          const complex_track_params_v6_t *cparams,
                                          const si32 *simples_per_complex);
  
  extern void RfPrintComplexTrackParamsXMLV6(FILE *out,
                                             const char *spacer,
                                             int verification_performed,
                                             const track_file_params_v6_t *params,
                                             const complex_track_params_v6_t *cparams,
                                             const si32 *simples_per_complex);
  
  extern void RfPrintContTableV6(FILE *out,
                                 const char *label,
                                 const char *spacer,
                                 const track_file_contingency_data_v6_t *count);
  
  extern void RfPrintForecastPropsV6(FILE *out,
                                     const char *label,
                                     const char *space,
                                     const track_file_params_v6_t *params,
                                     const track_file_forecast_props_v6_t *forecast);
  
  extern void RfPrintForecastPropsXMLV6(FILE *out,
                                        const char *label,
                                        const char *space,
                                        const track_file_params_v6_t *params,
                                        const track_file_forecast_props_v6_t *forecast);
  
  extern void RfPrintSimpleTrackParamsV6(FILE *out,
                                         const char *spacer,
                                         const simple_track_params_v6_t *sparams);
  
  extern void RfPrintSimpleTrackParamsXMLV6(FILE *out,
                                            const char *spacer,
                                            const simple_track_params_v6_t *sparams);
  
  extern void RfPrintTrackEntryV6(FILE *out,
                                  const char *spacer,
                                  si32 entry_num,
                                  const track_file_params_v6_t *params,
                                  const track_file_entry_v6_t *entry);
  
  extern void RfPrintTrackEntryXMLV6(FILE *out,
                                     const char *spacer,
                                     si32 entry_num,
                                     const track_file_params_v6_t *params,
                                     const track_file_entry_v6_t *entry);
  
  extern void RfPrintTrackHeaderV6(FILE *out,
                                   const char *spacer,
                                   const track_file_header_v6_t *header);
  
  extern void RfPrintTrackHeaderArraysV6(FILE *out,
                                         const char *spacer,
                                         const track_file_header_v6_t *header,
                                         const si32 *complex_track_nums,
                                         const si32 *complex_track_offsets,
                                         const si32 *simple_track_offsets,
                                         const si32 *nsimples_per_complex,
                                         const si32 *simples_per_complex_offsets,
                                         const si32 **simples_per_complex,
                                         const track_file_scan_index_v6_t *scan_index);
     
  extern void RfPrintTrackParamsV6(FILE *out,
                                    const char *spacer,
                                   const track_file_params_v6_t *params);

  extern void RfPrintTrackVerifyV6(FILE *out,
                                   const char *spacer,
                                   const track_file_verify_v6_t *verify);

#endif
