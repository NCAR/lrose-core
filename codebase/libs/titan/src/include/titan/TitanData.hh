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

#include <cstdint>
#include <Mdv/Mdvx.hh>
#include <titan/storm.h>
#include <titan/track.h>

class TitanData {

public:

  // missing values
  
  static constexpr double missingFl64 = -999999.0;
  static constexpr float missingFl32 = -999999.0f;
  static constexpr int64_t missingInt64 = -999999;
  static constexpr int32_t missingInt32 = -999999;
  static constexpr int16_t missingInt16 = -32768;
  static constexpr int8_t missingInt08 = -128;
  
  typedef enum {
    FCAST_BY_TREND = 1,
    FCAST_BY_PARABOLA = 2,
    FCAST_BY_REGRESSION  = 3
  } forecast_t;

  static std::string BOOL_STR(bool a) {
    if (a) {
      return "true";
    } else {
      return "false";
    }
  }
  
  static std::string FORECAST_TYPE_STR(forecast_t a) {
    switch (a) {
      case FCAST_BY_TREND:
      default:
        return "trend";
      case FCAST_BY_PARABOLA:
        return "parabola";
      case FCAST_BY_REGRESSION:
        return "regression";
    }
  }
  
  /////////////////////////////////////////////////////////////
  // storm data params
  // these are the parameters that govern the operations of the
  // identification algorithm
  
  class StormParams {
    
  public:

    // methods
    
    StormParams();
    void initialize();
    void setFromLegacy(const storm_file_params_t &params);
    storm_file_params_t convertToLegacy() const;
    void convertToLegacy(storm_file_params_t &params) const;
    void print(FILE *out, const char *spacer);

    // mode for computing precip
    // TITAN_PRECIP_FROM_COLUMN_MAX: precip computed from col-max dBZ
    // TITAN_PRECIP_AT_SPECIFIED_HT: precip computed at specified height
    // TITAN_PRECIP_AT_LOWEST_VALID_HT: precip computed at lowest valid CAPPI
    // TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL:
    //   precip computed from lowest level with non-missing dbz

    typedef enum {
      PRECIP_FROM_COLUMN_MAX = 0,
      PRECIP_AT_SPECIFIED_HT = 1,
      PRECIP_AT_LOWEST_VALID_HT = 2,
      PRECIP_FROM_LOWEST_AVAILABLE_REFL = 3
    } precip_mode_t;

    // data fields
    
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
    
    int32_t check_morphology;	/* check_morphology flag */
    
    int32_t check_tops;		/* check_tops flag */
    
    int32_t vel_available;		/* vel data availability flag */
    
    int32_t n_poly_sides;	/* number of sides in storm shape
				 * polygons */
    
    fl32 ltg_count_time;        /* number of seconds over which the ltg
                                 * strikes are counted. */
    
    fl32 ltg_count_margin_km;   /* margin from storm edge to describe
                                 * outer region for counting ltg strikes */
    
    fl32 hail_z_m_coeff;        /* Z - mass coeficient */
    
    fl32 hail_z_m_exponent;     /* Z - mass exponent */
    
    fl32 hail_mass_dbz_threshold; /* dbz - threshold above which
                                   * hail mass is calculated */
    
    int32_t gprops_union_type;  /* interpretation of add_on union in gprops */

    fl32 tops_dbz_threshold;	/* dbz threshold for computing storm tops */
    
    fl32 precip_plane_ht;  /* CAPPI ht for which precip is computed (km MSL)
                            * See precip_computation_mode */
    
    fl32 low_convectivity_threshold;   /* used if convectivity thresholds applied */
    fl32 high_convectivity_threshold;  /* otherwise missing */

    precip_mode_t precip_computation_mode;
    
  }; // StormParams

  /////////////////////////////////////////////////////////////
  // storm data header

  class StormHeader {
    
  public:

    // methods
    
    StormHeader();
    void initialize();
    void setFromLegacy(const storm_file_header_t &hdr);
    storm_file_header_t convertToLegacy() const;
    void convertToLegacy(storm_file_header_t &hdr) const;
    void print(FILE *out, const char *spacer);

    // data

    time_t file_time;    /* filetime - time of last write */
    time_t start_time;   /* start_time - time of first scan */
    time_t end_time;     /* end_time - time of last scan */
    int32_t n_scans;     /* number of completed scans in file */
    StormParams params;  /* see above */
    
  };

  /////////////////////////////////////////////////////////////
  // scan header
  
  class ScanHeader {
    
  public:
    
    // methods
    
    ScanHeader();
    void initialize();
    void setFromLegacy(const storm_file_scan_header_t &hdr);
    storm_file_scan_header_t convertToLegacy() const;
    void convertToLegacy(storm_file_scan_header_t &hdr) const;
    static void setFromLegacy(const storm_file_scan_header_t *legacyHdrs,
                              vector<TitanData::ScanHeader> &scans);
    static void convertToLegacy(const vector<TitanData::ScanHeader> &scans,
                                storm_file_scan_header_t *legacyHdrs);
    void print(FILE *out, const char *spacer);
    
    // data
    
    time_t time;

    int64_t gprops_offset;	/* file offset for the gprops array */
    int64_t first_offset;	/* offset of first byte for data for
				 * this scan */
    int64_t last_offset;	/* offset of last byte for data for
				 * this scan */
    
    fl32 min_z;			/* km - msl ht of lowest layer in storm */
    fl32 delta_z;		/* km - layer thickness */
    
    int32_t scan_num;
    int32_t nstorms;
    
    fl32 ht_of_freezing;          /* Height of freezing in km */

    Mdvx::coord_t grid;           /* cartesian params for this scan */
    
  };

  /////////////////////////////////////////////////////////////
  // storm global properties

  class StormGprops {
    
  public:

    // methods
    
    StormGprops();
    void initialize();
    void setFromLegacy(const storm_file_params_t &params,
                       const storm_file_global_props_t &gprops);
    storm_file_global_props_t convertToLegacy() const;
    void convertToLegacy(storm_file_global_props_t &gprops) const;
    static void setFromLegacy(const storm_file_params_t &params,
                              const storm_file_global_props_t *legacyGprops,
                              vector<TitanData::StormGprops> &gprops);
    static void convertToLegacy(const vector<TitanData::StormGprops> &gprops,
                                storm_file_global_props_t *legacyGprops);
    void print(FILE *out, const char *spacer,
               const StormParams &params, const ScanHeader &scan);
     

    // data
    
    static constexpr int32_t GPROPS_N_POLY_SIDES = 72;
    
    int64_t layer_props_offset;	/* posn in file of layer props data */
    int64_t dbz_hist_offset;	/* posn in file of dbz hist data */
    int64_t runs_offset;	/* posn in file of runs data */
    int64_t proj_runs_offset;	/* posn in file of proj area runs data */

    fl32 vol_centroid_x;	/* (km or deg) (NOTE 3) */
    fl32 vol_centroid_y;	/* (km or deg) (NOTE 3) */
    fl32 vol_centroid_z;	/* km */

    fl32 refl_centroid_x;	/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_y;	/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_z;	/* km */

    fl32 top;			/* km */
    fl32 base;			/* km */
    fl32 volume;		/* km3 */
    fl32 area_mean;		/* km2 */
    fl32 precip_flux;		/* m3/s */
    fl32 mass;			/* ktons */

    fl32 tilt_angle;		/* deg */
    fl32 tilt_dirn;		/* degT */

    fl32 dbz_max;		/* dbz */
    fl32 dbz_mean;		/* dbz */
    fl32 dbz_max_gradient;	/* dbz/km */
    fl32 dbz_mean_gradient;	/* dbz/km */
    fl32 ht_of_dbz_max;		/* km */

    fl32 rad_vel_mean;		/* m/s */
    fl32 rad_vel_sd;		/* m/s */
    fl32 vorticity;		/* /s */

    fl32 precip_area; /* km2 */
    fl32 precip_area_centroid_x; /* (km or deg) (NOTE 3) */
    fl32 precip_area_centroid_y; /* (km or deg) (NOTE 3) */
    fl32 precip_area_orientation; /* degT */
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
    
    fl32 proj_area_polygon[GPROPS_N_POLY_SIDES]; /* in grid units, the length of
                                                  * rays from the proj area
                                                  * centroid to the polygon
                                                  * vertices (NOTE 3) */

    int32_t storm_num;

    int32_t n_layers;		/* the number of layers in this storm */

    int32_t base_layer;		/* the layer number of the storm base */

    int32_t n_dbz_intervals;	/* the number of intervals in the
                                 * dbz distribution histograms */

    int64_t n_runs;		/* number of runs above threshold for
                                 * this storm */

    int64_t n_proj_runs;	/* number of runs in the projected area
				 * for this storm */

    int32_t top_missing;		/* flag to indicate that top of storm was
                                         * not sampled: set to 1 of top missing,
                                         * 0 otherwise */

    int32_t range_limited;		/* flag to indicate that storm was not
                                         * fully sampled because of range limitations:
                                         * set to 1 of range-limited,
                                         * 0 otherwise */

    int32_t second_trip;		/* flag to indicate that storm is probably
                                         * second trip */

    int32_t hail_present;		/* flag to indicate dBZ values above the
                                         * hail threshold */

    int32_t anom_prop;               /* flag to indicate anomalous propagation */
    
    int32_t bounding_min_ix;         /* proj_area (x,y) bounding box */
    int32_t bounding_min_iy;         /* in grid coords */
    int32_t bounding_max_ix;
    int32_t bounding_max_iy;
  
    fl32 vil_from_maxz;
    fl32 ltg_count;               /* ltg strike count in x minutes before storm
                                   * time with x km of storm */

    fl32 convectivity_median;     /* used if convectivity thresholds applied */

    // hail metrics
    
    int32_t hailFOKRcategory;      /*  category 0-4 */
    fl32 hailWaldvogelProb;        /* 0 <= probability <= 1.0 */
    fl32 hailMassAloft;            /* ktons */
    fl32 hailVertIntgMass;         /* kg/m2 (from maxz) */
    
    fl32 hda_poh; /* Waldvogel Probability (%) */
    fl32 hda_shi; /* Severe Hail Index (J.m-1.s-1) */
    fl32 hda_posh; /* probability of severe hail (%) */
    fl32 hda_mehs; /* Maximum Expected Hail Size (mm) */
    
  };  // StormGprops

  /////////////////////////////////////////////////////////////
  // storm layer properties

  class StormLprops {
    
  public:
    
    // methods
    
    StormLprops();
    void initialize();
    void setFromLegacy(const storm_file_layer_props_t &lprops);
    storm_file_layer_props_t convertToLegacy() const;
    void convertToLegacy(storm_file_layer_props_t &lprops) const;
    static void setFromLegacy(const storm_file_layer_props_t *legacyLprops,
                              vector<TitanData::StormLprops> &lprops);
    static void convertToLegacy(const vector<TitanData::StormLprops> &lprops,
                                storm_file_layer_props_t *legacyLprops);
    static void print(FILE *out, const char *spacer,
                      const ScanHeader &scan,
                      const StormGprops &gprops,
                      const vector<StormLprops> &lprops);

    // data
    
    fl32 vol_centroid_x;	/* (km or deg) (NOTE 3) */
    fl32 vol_centroid_y;	/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_x;	/* (km or deg) (NOTE 3) */
    fl32 refl_centroid_y;	/* (km or deg) (NOTE 3) */
    fl32 area;		/* km2 */
    fl32 dbz_max;		/* dbz */
    fl32 dbz_mean;		/* dbz */
    fl32 mass;			/* ktons */
    fl32 rad_vel_mean;		/* m/s */
    fl32 rad_vel_sd;		/* m/s */
    fl32 vorticity;		/* /s */
    fl32 convectivity_median;   /* used if convectivity thresholds applied */
    
  };  // StormLprops

  /////////////////////////////////////////////////////////////
  // storm dbz histograms

  class StormDbzHist {
    
  public:
    
    // methods
    
    StormDbzHist();
    void initialize();
    void setFromLegacy(const storm_file_dbz_hist_t &hist);
    storm_file_dbz_hist_t convertToLegacy() const;
    void convertToLegacy(storm_file_dbz_hist_t &hist) const;
    static void setFromLegacy(const storm_file_dbz_hist_t *legacyHist,
                              vector<TitanData::StormDbzHist> &hist);
    static void convertToLegacy(const vector<TitanData::StormDbzHist> &hist,
                                storm_file_dbz_hist_t *legacyHist);
    static void print(FILE *out, const char *spacer,
                      const StormParams &params,
                      const StormGprops &gprops,
                      const vector<StormDbzHist> &hist);
      

    // data
    
    fl32 percent_volume;
    fl32 percent_area;
    
  };  // StormDbzHist

  /////////////////////////////////////////////////////////////
  // storm runs - contiguous grid cells in a row

  class StormRun {
    
  public:
    
    // methods
    
    StormRun();
    void initialize();
    void setFromLegacy(const storm_file_run_t &run);
    storm_file_run_t convertToLegacy() const;
    void convertToLegacy(storm_file_run_t &run) const;
    static void setFromLegacy(const storm_file_run_t *legacyRun,
                              vector<TitanData::StormRun> &run);
    static void convertToLegacy(const vector<TitanData::StormRun> &run,
                                storm_file_run_t *legacyRun);
    static void print(FILE *out, const char *spacer, const char *label,
                      const StormGprops &gprops, const vector<StormRun> &runs);
     

    // data
    
    int32_t run_ix;
    int32_t run_iy;
    int32_t run_iz;
    int32_t run_len;
    
  };  // StormRun

  /////////////////////////////////////////////////////////////
  // track forecast properties

  class TrackFcastProps {
    
  public:
    
    // methods
    
    TrackFcastProps();
    void initialize();
    void setFromLegacy(const track_file_forecast_props_t &fprops);
    track_file_forecast_props_t convertToLegacy() const;
    void convertToLegacy(track_file_forecast_props_t &fprops) const;
    void print(FILE *out, const char *label, const char *spacer);
  
    // data
    
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
    
  };  // TrackFcastProps

  /////////////////////////////////////////////////////////////
  // track forecast verification

  class TrackVerify {
    
  public:
    
    // methods
    
    TrackVerify();
    void initialize();
    void setFromLegacy(const track_file_verify_t &verify);
    track_file_verify_t convertToLegacy() const;
    void convertToLegacy(track_file_verify_t &verify) const;
    void print(FILE *out, const char *label, const char *spacer);
     
    // data
    
    time_t end_time;		/* last scan time for which verification
				 * has been performed */
    
    int32_t verification_performed;	/* set to 1 if true, i.e. if
                                         * verification has been performed,
                                         * 0 if false */

    int32_t forecast_lead_time;	/* lead time for forecast 
				 * verification (secs) */

    int32_t forecast_lead_time_margin; /* the accuracy with which the forecast
                                        * time is used (secs). For a scan to
                                        * be used, its time must fall within
                                        * this margin of the forecast time */

    int32_t forecast_min_history;	/* min history for valid forecasts -
                                         * (secs) - when track duration is less
                                         * than this, forecast is not
                                         * considered valid */

    int32_t verify_before_forecast_time; /* include in the analysis those
                                          * scans in a track which have
                                          * a duration less than the 
                                          * forecast lead time - the
                                          * default should be false */

    int32_t verify_after_track_dies; /* include in the analysis those
                                      * scans after a track has died -
                                      * the default should be true */
  
    Mdvx::coord_t grid;          /* describes the cartesian grid
                                  * used for verification */

  };  // TrackVerify

  /////////////////////////////////////////////////////////////
  // contingency data for track forecast verification

  class TrackContingency {
    
  public:
    
    // methods
    
    TrackContingency();
    void initialize();
    void setFromLegacy(const track_file_contingency_data_t &cont);
    track_file_contingency_data_t convertToLegacy() const;
    void convertToLegacy(track_file_contingency_data_t &cont) const;
    void print(FILE *out, const char *label, const char *spacer);

    // data
    
    fl32 n_success;
    fl32 n_failure;
    fl32 n_false_alarm;

  }; // TrackContingency

  /////////////////////////////////////////////////////////////
  // tracking parameters

  class TrackingParams {
    
  public:
    
    // methods
    
    TrackingParams();
    void initialize();
    void setFromLegacy(const track_file_params_t &params);
    track_file_params_t convertToLegacy() const;
    void convertToLegacy(track_file_params_t &params) const;
    void print(FILE *out, const char *spacer);
    
    // data
    
    static constexpr int32_t MAX_NWEIGHTS_FCAST = 10;
    static constexpr int32_t N_MOVEMENT_PROPS_FCAST = 5;
    static constexpr int32_t N_GROWTH_PROPS_FCAST = 5;
    static constexpr int32_t N_TOTAL_PROPS_FCAST = 14;
    
    fl32 forecast_weights[MAX_NWEIGHTS_FCAST];
  
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

    int32_t scale_forecasts_by_history;  /* flag for scaling the forecasts
                                          * based on the ratio of the history
                                          * to min_history_for_valid_forecast */

    int32_t use_runs_for_overlaps;   /* flag for using runs to find the overlaps
                                      * for mergers and splits */

    int32_t grid_type;		/* TITAN_PROJ_FLAT or TITAN_PROJ_LATLON */

    int32_t nweights_forecast;	/* number of weighted points to be used in
				 * the forecast data */
  
    int32_t forecast_type;	/* FORECAST_BY_TREND,
				 * FORECAST_BY_PARABOLA, or
				 * FORECAST_BY_REGRESSION */

    int32_t max_delta_time;		/* secs */

    int32_t min_history_for_valid_forecast; /* secs */

    int32_t spatial_smoothing;	/* TRUE or FALSE */
    
  };  // TrackingParams

  /////////////////////////////////////////////////////////////
  // track data header

  class TrackHeader {
    
  public:

    // methods
    
    TrackHeader();
    void initialize();
    void setFromLegacy(const track_file_header_t &hdr);
    track_file_header_t convertToLegacy() const;
    void convertToLegacy(track_file_header_t &hdr) const;
    void print(FILE *out, const char *spacer);

    // data
    
    int32_t file_valid;		/* set to 0 when file is under
				 * modification, 1 when modification is
				 * successfully complete */
    
    int32_t modify_code;	/* a unique code inserted when file
				 * modification begins  - used for 
				 * restarting tracking after program
				 * is stopped for any reason */

    int32_t n_simple_tracks;	/* number of simple tracks in file */
    int32_t n_complex_tracks;	/* number of complex tracks in file */

    int32_t n_samples_for_forecast_stats;

    int32_t n_scans;			/* the number of scans in the file */

    int32_t last_scan_num;		/* the number of the last scan
                                         * in the file = n_scans - 1 */

    int32_t max_simple_track_num;	/* the highest simple track num in
                                         * the file */

    int32_t max_complex_track_num;	/* the highest complex track num in
                                         * the file */

    int32_t max_parents;
    int32_t max_children;
    int32_t max_nweights_forecast;

    TrackingParams params;	/* see above */

    TrackVerify verify; /* see above - has character data at the end -
                         * GRID_LABEL_LEN * N_GRID_STRUCT_LABELS
                         * bytes */
    
    TrackContingency ellipse_verify; /* contingency table data for
                                      * forecast verification 
                                      * using ellipses */
    
    TrackContingency polygon_verify; /* contingency table data for
                                      * forecast verification 
                                      * using polygons */
    
    TrackFcastProps forecast_bias; /* bias error for
                                    * forecast props */
  
    TrackFcastProps forecast_rmse; /* root mean squared error for
                                    * forecast props */
    
  };

  /////////////////////////////////////////////////////////////
  // simple track parameters

  class SimpleTrackParams {
    
  public:
    
    // methods
    
    SimpleTrackParams();
    void initialize();
    void setFromLegacy(const simple_track_params_t &params);
    simple_track_params_t convertToLegacy() const;
    void convertToLegacy(simple_track_params_t &params) const;
    static void setFromLegacy(const simple_track_params_t *legacyParams,
                              vector<TitanData::SimpleTrackParams> &params);
    static void convertToLegacy(const vector<TitanData::SimpleTrackParams> &params,
                                simple_track_params_t *legacyParams);
    void print(FILE *out, const char *spacer);
    
    // data
    
    static constexpr int32_t MAX_PARENTS_ = 8;
    static constexpr int32_t MAX_CHILDREN_ = 8;
    
    time_t start_time;		/* for the simple track only */
    time_t end_time;		/* for the simple track only */

    time_t last_descendant_end_time; /* end time for the last descendant
                                      * of this storm */

    time_t time_origin;		/* time for first storm in any branch which
				 * led to this entry */

    int64_t first_entry_offset;	/* file offset of first track entry struct */

    int32_t simple_track_num;
    int32_t complex_track_num;	/* number of track complex if track is part
				 * of a complex */

    int32_t last_descendant_simple_track_num; /* simple track num of last
                                               * descendant of this storm */

    int32_t start_scan;
    int32_t end_scan;
    int32_t last_descendant_end_scan; /* end scan of last descendant
                                       * of this storm */

    int32_t scan_origin;		/* scan for first storm in any branch which
                                         * led to this simple track */

    int32_t history_in_scans;	/* number of scans of history data for
				 * making forecast - in the case of a merger
				 * or a split, this will be longer than the
				 * duration in scans */

    int32_t history_in_secs;       /* time in secs since the start of the
                                    * earliest branch of this storm */
    
    int32_t duration_in_scans;	/* duration of this simple track */
    int32_t duration_in_secs;	/* duration of this simple track */

    int32_t nparents;
    int32_t nchildren;
    
    int32_t parent[MAX_PARENTS_]; /* track numbers of parents */
    int32_t child[MAX_CHILDREN_];   /* track numbers of children */
    
  };  // SimpleTrackParams

  /////////////////////////////////////////////////////////////
  // complex track parameters

  class ComplexTrackParams {
    
  public:
    
    // methods
    
    ComplexTrackParams();
    void initialize();
    void setFromLegacy(const complex_track_params_t &params);
    complex_track_params_t convertToLegacy() const;
    void convertToLegacy(complex_track_params_t &params) const;
    static void setFromLegacy(const complex_track_params_t *legacyParams,
                              vector<TitanData::ComplexTrackParams> &params);
    static void convertToLegacy(const vector<TitanData::ComplexTrackParams> &params,
                                complex_track_params_t *legacyParams);
    void print(FILE *out, const char *spacer,
               bool verification_performed,
               const vector<int> &simples_per_complex);
     
    
    // data
    
    time_t start_time;
    time_t end_time;
    
    int32_t complex_track_num;

    int32_t n_simple_tracks; /* number of simple tracks in this
                              * complex track */

    int32_t start_scan;
    int32_t end_scan;

    int32_t duration_in_scans;
    int32_t duration_in_secs;

    fl32 volume_at_start_of_sampling; /* km3 */
    fl32 volume_at_end_of_sampling;   /* km3 */
    
    /*
     * the following are flags to indicate status:
     * a) number of scans for which top was not completely sampled
     * b) number of scans for which storm sampling was incomplete
     *    due to range
     * c) storm existed when sampling began - start volume stored
     * d) storm existed when sampling ended - end volume stored
     */

    int32_t n_top_missing;
    int32_t n_range_limited;
    int32_t start_missing;
    int32_t end_missing;
    int32_t n_samples_for_forecast_stats;

    TrackContingency ellipse_verify; /* contingency table data for
                                      * forecast verification 
                                      * using ellipses */
    
    TrackContingency polygon_verify; /* contingency table data for
                                      * forecast verification 
                                      * using polygons */
    
    TrackFcastProps forecast_bias; /* bias error for
                                    * forecast props */
  
    TrackFcastProps forecast_rmse; /* root mean squared error for
                                    * forecast props */
    
  };  // ComplexTrackParams

  /////////////////////////////////////////////////////////////
  // track entry

  class TrackEntry {
    
  public:
    
    // methods
    
    TrackEntry();
    void initialize();
    void setFromLegacy(const track_file_entry_t &entry);
    track_file_entry_t convertToLegacy() const;
    void convertToLegacy(track_file_entry_t &entry) const;
    static void setFromLegacy(const track_file_entry_t *legacyEntry,
                              vector<TitanData::TrackEntry> &entry);
    static void convertToLegacy(const vector<TitanData::TrackEntry> &entry,
                                track_file_entry_t *legacyEntry);
    void print(FILE *out, const char *spacer, int entry_num);
    
    // data
    
    time_t time;		/* time for this entry */
    time_t time_origin;		/* time for first storm in any branch which
				 * led to this entry */

    int64_t prev_entry_offset;	/* track file offset to previous entry in
				 * in track - doubly linked list */

    int64_t this_entry_offset;	/* track file offset to this entry */

    int64_t next_entry_offset;	/* track file offset to next entry in
				 * track - doubly linked list */

    int64_t next_scan_entry_offset; /* offset to next entry in this scan */

    int32_t scan_origin;	/* scan for first storm in any branch which
                                 * led to this entry */

    int32_t scan_num;		/* scan number in storm file */
    int32_t storm_num;		/* storm number in scan */

    int32_t simple_track_num;	/* simple track number - should match the
				 * number in the simple_params struct */
    int32_t complex_track_num;

    int32_t history_in_scans;	/* number of scans of history data for
				 * making forecast - in the case of a merger
				 * or a split, this will be longer than the
				 * duration in scans */

    int32_t history_in_secs;		/* time in secs since the start of the
                                         * earliest branch of this storm */

    int32_t duration_in_scans;	/* duration of the simple track */
    int32_t duration_in_secs;	/* duration of the simple track */

    int32_t forecast_valid;	/* flag to indicate if the forecast is
                                 * valid */

    TrackFcastProps dval_dt; /* rate of change of each forecast
                              * value */
  };  // TrackEntry

  /////////////////////////////////////////////////////////////
  // index of track in scan 

  class TrackScanIndex {
    
  public:
    
    // methods
    
    TrackScanIndex();
    void initialize();
    void setFromLegacy(const track_file_scan_index_t &index);
    track_file_scan_index_t convertToLegacy() const;
    void convertToLegacy(track_file_scan_index_t &index) const;
    static void setFromLegacy(const track_file_scan_index_t *legacyIndex,
                              vector<TitanData::TrackScanIndex> &index);
    static void convertToLegacy(const vector<TitanData::TrackScanIndex> &index,
                                track_file_scan_index_t *legacyIndex);
    static void print(FILE *out, const char *spacer,
                      const vector<TrackScanIndex> &indexes);
    
    // data
    
    time_t utime;
    int64_t first_entry_offset;
    int32_t n_entries;

  };  // TrackScanIndex

  // print track arrays
  
  static void printTrackArrays(FILE *out, const char *spacer,
                               const TrackHeader &header,
                               const vector<int> &complex_track_nums,
                               const vector<int> &nsimples_per_complex,
                               const vector<int> &simples_per_complex_offsets,
                               const vector<vector<int> > &simples_per_complex,
                               const vector<TrackScanIndex> &scan_index);

  // print coord grid struct
  
  static void printMdvCoord(FILE *out, const char *spacer, const Mdvx::coord_t &coord);

}; // TitanData

#endif // TitanData_hh
