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
#ifndef tdata_server_h
#define tdata_server_h

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************
 * tdata_server.h
 *
 * Structures and defines for track data server and clients.
 *
 * COMMUNICATIONS MODES
 * ====================
 *
 * There is support for both one-way and two-way communications. The
 * one-way mode is intended for use with the AWPG display system,
 * and is used along with the 'product_mode' of operations in
 * which very small simple structs are sent to the product server.
 * The two-way mode is used for all other ops, and allows for both
 * real-time and archive requests.
 *
 * ONE-WAY MODE
 * ------------
 *
 * Client connects, but no messsages received from the client.
 * Data is sent to the client as available, using product mode, 
 *
 * TWO-WAY MODE
 * ------------
 *
 * Client requests / server responses
 *
 * CONNECT VERIFICATION:
 *
 *   Client: tdata_request_t: request = TDATA_VERIFY_CONNECT
 *   Server: tdata_reply_t: TDATA_SUCCESS / TDATA_FAILURE
 *
 * SET MAX PACKET LENGTH
 *
 *   Client: tdata_request_t: request = TDATA_SET_MAX_PACKET_LEN
 *   Server: tdata_reply_t: TDATA_SUCCESS / TDATA_FAILURE
 *
 * REQUEST SERVER TO NOTIFY CLIENT WHEN NEW DATA IS AVAILABLE
 *
 *   Client: tdata_request_t: request = TDATA_REQUEST_NOTIFY
 *   Server: tdata_reply_t: TDATA_SUCCESS / TDATA_FAILURE
 *
 * When new data is available the server sends the message:
 *
 *   Server: tdata_reply_t: TDATA_NEW_AVAILABLE
 *
 * REQUEST SERVER TO STOP NOTIFICATION
 *
 *   Client: tdata_request_t: request = TDATA_STOP_NOTIFY
 *   Server: tdata_reply_t: TDATA_SUCCESS / TDATA_FAILURE
 *
 * REQUEST SERVER TO HANGUP CONNECTION
 *
 *   Client: tdata_request_t: request = TDATA_HANGUP
 *   Server: tdata_reply_t: TDATA_SUCCESS / TDATA_FAILURE
 *
 * REQUEST SERVER TO SEND DATA
 *
 *   Client: tdata_request_t: request = TDATA_REQUEST_DATA
 *                            other fields set accordingly
 * 
 *   Server:
 *
 *        tdata_reply_t: TDATA_SUCCESS / TDATA_FAILURE
 *
 * SENDING DATA TO THE CLIENT
 * --------------------------
 *
 *   The data will be sent as a sequence of structs as shown
 *   below. The structs are packeed into a buffer, each
 *   preceded by a tdata_struct_header_t identifying the
 *   struct type. The buffer is sent when the addition
 *   of another struct would make it overflow.
 *
 *     If (mode == TDATA_COMPLETE)
 * 
 *        tdata_reply_t: TDATA_SUCCESS / TDATA_FAILURE
 *
 *        if TDATA_SUCCESS {
 *          tdata_complete_header_t
 *          for each complex track {
 *            complex_track_params_t (track.h)
 *            si32 simples_per_complex[nsimples]
 *            for each simple track {
 *              simple_track_params_t (track.h)
 *              for each entry {
 *               track_file_entry_t (track.h)
 *                storm_file_scan_header_t (storm.h)
 *                storm_file_global_props_t (storm.h)
 *                for each layer {
 *                  storm_file_layer_props_t (storm.h)
 *                }
 *                for each histogram interval {
 *                  storm_file_dbz_hist_t (storm.h)
 *                }
 *              } - entry
 *            } - simple track
 *          } - complex track
 *        } - if TDATA_SUCCESS
 *
 *     If (mode == TDATA_BASIC_WITH_PARAMS)
 * 
 *        if TDATA_SUCCESS {
 *          tdata_basic_header_t
 *          for each complex track {
 *            tdata_basic_complex_params_t
 *            si32 simples_per_complex[nsimples]
 *            for each simple track {
 *              tdata_basic_simple_params_t
 *              for each entry {
 *                tdata_basic_track_entry_t
 *                if (runs_included == TRUE) {
 *                  entry.nruns * storm_file_run_t
 *                } - runs
 *              } - entry
 *            } - simple track
 *          } - complex track
 *        } - if TDATA_SUCCESS
 *
 *     If (mode == TDATA_BASIC_WITHOUT_PARAMS)
 * 
 *        if TDATA_SUCCESS {
 *          tdata_basic_header_t
 *          for each entry in header.n_entries {
 *            tdata_basic_track_entry_t
 *          } - entry
 *        } - if TDATA_SUCCESS
 *
 *     If (mode == TDATA_PRODUCT)
 * 
 *        if TDATA_SUCCESS {
 *          tdata_product_header_t
 *          for each entry in header.n_entries {
 *            tdata_product_entry_t
 *            if (header.ellipse_included) {
 *              tdata_product_ellipse_t
 *            }
 *            if (header.polygon_included) {
 *              tdata_product_polygon_t, with the
 *              polygon array filled with header.n_poly_sides
 *              members.
 *            }
 *          } - entry
 *        } - if TDATA_SUCCESS
 *
 * PACKET HEADERS
 *
 * Note: refer to sockutil.h for struct defs
 *
 * All packets are preceded by a header. There are two header types:
 *
 *   Old type : sockutil_old_header_t
 *   New type : sockutil_code_t followed by sockutil_header_t
 *
 * The client may use either header type, although the new type is
 * preferred because the is no limitation on packet length.
 *
 * The server will always use the header type which was used most recently
 * by the client.
 *
 * TIME FORMAT
 *
 * All times are passed as longs, and are to be interpreted as the number of
 * seconds since Jan 1 1970 GMT. (see time(3V))
 *
 ***********************************************************************/
   
#include <dataport/port_types.h>
#include <titan/track.h>
#include <rapformats/titan_grid.h>

#define TDATA_TEXT_LEN 128
#define TDATA_HOST_LEN 128

#define TDATA_TRUE 1
#define TDATA_FALSE 0

/*
 * packet ids
 */

#define TDATA_REQUEST_PACKET_ID 7500
#define TDATA_REPLY_PACKET_ID 7501
#define TDATA_DATA_PACKET_ID 7502
#define TDATA_NOTIFY_PACKET_ID 7503

/*
 * data struct ids
 */

#define TDATA_COMPLETE_HEADER_ID 7510

#define TDATA_COMPLEX_TRACK_PARAMS_ID 7511
#define TDATA_SIMPLE_TRACK_PARAMS_ID 7512

#define TDATA_TRACK_FILE_ENTRY_ID 7513

#define TDATA_STORM_FILE_SCAN_HEADER_ID 7514
#define TDATA_STORM_FILE_GLOBAL_PROPS_ID 7515
#define TDATA_STORM_FILE_LAYER_PROPS_ID 7516
#define TDATA_STORM_FILE_DBZ_HIST_ID 7517

#define TDATA_BASIC_HEADER_ID 7518
#define TDATA_BASIC_COMPLEX_PARAMS_ID 7519
#define TDATA_BASIC_SIMPLE_PARAMS_ID 7520
#define TDATA_BASIC_TRACK_ENTRY_ID 7521
#define TDATA_RUNS_ID 7522
#define TDATA_COMPLETE_SIMPLES_PER_COMPLEX_ID 7523
#define TDATA_BASIC_SIMPLES_PER_COMPLEX_ID 7524
   
/*
 * message sizes
 */

#define TDATA_DEFAULT_MAX_MESSAGE_LEN (si32) 65536
#define TDATA_LOWEST_MAX_MESSAGE_LEN (si32) 512
   
/*
 * array sizes
 */

/* must match MAX_PARENTS in track.h */
#define TDATA_MAX_PARENTS 8
					 
/* must match MAX_CHILDREN in track.h */
#define TDATA_MAX_CHILDREN 8
   
/*
 * requests
 */
   
#define TDATA_VERIFY_CONNECT           0
#define TDATA_REQUEST_NOTIFY           1
#define TDATA_STOP_NOTIFY              2
#define TDATA_REQUEST_DATA             3
#define TDATA_SET_MAX_MESSAGE_LEN      4
#define TDATA_HANGUP                   5
#define TDATA_REQUEST_DATA_BEFORE      6
#define TDATA_REQUEST_DATA_NEXT_SCAN   7
#define TDATA_REQUEST_DATA_PREV_SCAN   8

/*
 * reply status
 */

#define TDATA_SUCCESS       0
#define TDATA_FAILURE       1
#define TDATA_NEW_AVAILABLE 2

/*
 * request mode
 */

#define TDATA_COMPLETE 0
#define TDATA_BASIC_WITH_PARAMS  1
#define TDATA_BASIC_WITHOUT_PARAMS 2
#define TDATA_PRODUCT 3
   
/*
 * data source
 *
 * If TDATA_LATEST is used, realtime data is returned if available,
 * otherwise the latest file data is used.
 *
 * If TDATA_REALTIME is used, realtime data is returned only if available,
 * and an error if not.
 */

#define TDATA_ARCHIVE  0
#define TDATA_REALTIME 1
#define TDATA_LATEST 2

/*
 * track_set - if > 0 the track_set is the number of the track
 * requested.
 */

#define TDATA_ALL_AT_TIME -1
#define TDATA_ALL_IN_FILE -2

/*
 * target_entries
 */

#define TDATA_ALL_ENTRIES 0
#define TDATA_CURRENT_ENTRIES_ONLY 1
#define TDATA_ENTRIES_IN_TIME_WINDOW 2

/*
 * number of sides in polygon
 */

#ifndef N_POLY_SIDES
#define N_POLY_SIDES 72
#endif

/*
 * The following structures are passed across the network in
 * network-byte-order
 */

/*
 * The client request
 */
   
typedef struct {

  si32 request;     /* The primary command */

  si32 mode;        /* TDATA_COMPLETE or
		     * TDATA_BASIC_WITH_PARAMS or
		     * TDATA_BASIC_WITHOUT_PARAMS or
		     * TDATA_PRODUCT */ 

  si32 source;      /* TDATA_ARCHIVE, TDATA_REALTIME or TDATA_LATEST */

  si32 track_set;   /* track set
		     *
		     * If >= 0, track number
		     *
		     * TDATA_ALL_AT_TIME for all entries of all
		     * tracks active at the request time.
		     *
		     * TDATA_ALL_IN_FILE for all entries for all
		     * tracks in the relvant file.
		     */

  si32 target_entries; /* TDATA_ALL_ENTRIES for all entries from
			* the track set
			*
			* DATA_CURRENT_ENTRIES_ONLY for track entries
			* at the current time only - only  used with
			* TDATA_BASIC_ONLY.
			*
			* TDATA_ENTRIES_IN_TIME_WINDOW for track entries
			* in time window relative to the request time -
			* in this mode the 'duration' fields in this
			* struct must be filled in - only used with
			* TDATA_BASIC_ONLY.
			*/

  si32 runs_included;   /* Set to TRUE if runs should be sent,
			 * 0 otherwise. Only implemented for
			 * TDATA_BASIC_WITH_PARAMS */

  si32 time; /* Unix time value
	      * (seconds since 1/1/1970 00:00:00) */

  si32 time_margin; /* Search margin on either side of target (secs) */

  si32 duration_before_request; /* duration in secs before request time
				 * for which track entries will be sent
				 * when using TDATA_ENTRIES_IN_TIME_WINDOW */

  si32 duration_after_request;  /* duration in secs after request time
				 * for which track entries will be sent
				 * when using TDATA_ENTRIES_IN_TIME_WINDOW */

  /*
   * for TDATA_ENTRIES_IN_TIME_WINDOW data will be sent for those track
   * entries between (time - duration_before_request) and 
   *                 (time + duration_after_request).
   */

  si32 max_message_len;

  si32 spare;

} tdata_request_t;

/*
 * The server reply
 */

typedef struct {

  si32 status;                /* TDATA_SUCCESS or TDATA_FAILURE or
			       * TDATA_NEW_AVAILABLE */

  si32 spare;

  char text[TDATA_TEXT_LEN];  /* message as appropriate */

} tdata_reply_t;

/*
 * data struct header, for keeping track of different struct types
 * in the data stream
 */

typedef struct {
  si32 id;
  si32 spare;
} tdata_struct_header_t;

/*
 * track basic data header struct
 */

typedef struct {

  fl32 dbz_threshold;    /* reflectivity threshold for storms
			  * (dbz) */

  fl32 min_storm_size;   /* minimum storm size (km2 for 2D data;
			  * km3 for 3D data) */

  fl32 poly_start_az;    /* deg azimuth from T.N. for
			  * first polygon point */

  fl32 poly_delta_az;    /* deg * azimuth delta between
			  * polygon points (pos is
			  * counterclockwise) */

  si32 n_poly_sides;     /* number of sides in storm shape
			  * polygons */

  si32 n_complex_tracks; /* number of complex tracks */

  si32 time;             /* time of closest scan to time requested */

  si32 data_start_time;  /* the start time for track data */
  si32 data_end_time;    /* the end time for track data */    

  si32 n_entries;        /* number of entries following - used in mode
			  * TDATA_BASIC_WITHOUT_PARAMS only */

  si32 runs_included;   /* Set to TRUE if runs should be sent,
			 * 0 otherwise. */

  si32 spare;

  titan_grid_t grid;      /* grid upon which storms are based */

} tdata_basic_header_t;

/*
 * parameters for complex track
 */

typedef struct {

  fl32 volume_at_start_of_sampling;  /* km3 */
  fl32 volume_at_end_of_sampling;    /* km3 */

  si32 complex_track_num;

  si32 start_scan;
  si32 end_scan;

  si32 duration_in_scans;
  si32 duration_in_secs;

  si32 start_time; /* start time of track */
  si32 end_time;   /* end time of track */

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

} tdata_basic_complex_params_t;

/*
 * parameters for simple track
 */

typedef struct {

  si32 start_scan;
  si32 end_scan;

  si32 simple_track_num;
  si32 complex_track_num;

  si32 start_time; /* start time of track */
  si32 end_time;   /* end time of track */

  si32 duration_in_scans;
  si32 duration_in_secs;

  si32 nchildren;            /* number of children */
  si32 nparents;             /* number of parents */

  si32 parent[TDATA_MAX_PARENTS];  /* simple track number for each parent */
  si32 child[TDATA_MAX_CHILDREN];  /* simple track number for each child */

} tdata_basic_simple_params_t;

/*
 * track entry for basic data
 *
 * NOTE1: stored as km  if grid_type == TITAN_PROJ_FLAT
 *        stored as deg if grid_type == TITAN_PROJ_LATLON
 */

typedef struct {

  si32 simple_track_num;
  si32 complex_track_num;

  si32 time;
  si32 scan_num;

  si32 history_in_scans;
  si32 history_in_secs;

  si32 forecast_valid;  /* TRUE (1) or FALSE (0) */
  si32 n_runs;          /* number of runs to follow if mode
			 * is TDATA_BASIC_WITH_RUNS */
  
  /*
   * storm data
   */

  fl32 vol_centroid_x;              /* km or deg (NOTE1) */
  fl32 vol_centroid_y;              /* km or deg (NOTE1) */
  fl32 vol_centroid_z;              /* km */

  fl32 refl_centroid_x;              /* km or deg (NOTE1) */
  fl32 refl_centroid_y;              /* km or deg (NOTE1) */
  fl32 refl_centroid_z;              /* km */

  fl32 top;                          /* km */
  fl32 base;                         /* km */
  fl32 volume;                       /* km3 */
  fl32 area_mean;                    /* km2 */
  fl32 precip_flux;                  /* m3/s */
  fl32 mass;                         /* ktons */
  fl32 vil_from_maxz;                /* kg/m2 */

  fl32 tilt_angle;		     /* deg */
  fl32 tilt_dirn;		     /* degT */

  fl32 dbz_max;			     /* dbz */
  fl32 dbz_mean;		     /* dbz */
  fl32 dbz_max_gradient;	     /* dbz/km */
  fl32 dbz_mean_gradient;	     /* dbz/km */
  fl32 ht_of_dbz_max;		     /* km */

  fl32 proj_area;                    /* km2 */
  fl32 proj_area_centroid_x;         /* km or deg (NOTE1) */
  fl32 proj_area_centroid_y;         /* km or deg (NOTE1) */
  fl32 proj_area_orientation;        /* degT (* DEG2LONG) */
  fl32 proj_area_minor_radius;       /* km or deg (NOTE1)
				      * minor radius of proj.
				      * area ellipse */

  fl32 proj_area_major_radius;       /* km or deg (NOTE1) - standard dev of 
				      * proj area along the major axis of
				      * the proj area shape */

  /*
   * forecast rates of change
   */

  fl32 proj_area_centroid_dx_dt;      /* km/hr */
  fl32 proj_area_centroid_dy_dt;      /* km/hr */
  fl32 dvolume_dt;                    /* km3/hr */
  fl32 dproj_area_dt;                 /* km2/hr */
  fl32 speed;                         /* km/hr */
  fl32 speed_knots;                   /* knots */
  fl32 direction;                     /* deg T */

  fl32 proj_area_polygon[N_POLY_SIDES];  /* km or deg (NOTE1)
					  * polygon rays relative
					  * to the proj area centroid */

} tdata_basic_track_entry_t;

/****************************
 * track product data structs
 *
 * A tdata_prod message is made up of the following:
 *
 * {
 *   tdata_product_header_t (header)
 *
 *   For each entry in (header.n_entries) {
 *     tdata_product_entry_t
 *     if (header.ellipse_included) {
 *       tdata_product_ellipse_t
 *     }
 *     if (header.polygon_included) {
 *       tdata_product_polygon_t with N_POLY_SIDES
 *     }
 *   }
 * }
 */

#define ANGLE_SCALE 1000000
#define SHORT_ANGLE_SCALE 100
#define SPEED_SCALE 100
#define DAREA_DT_SCALE 1000
#define FLAT_PROJ_ELLIPSE_SCALE 10
#define LATLON_PROJ_ELLIPSE_SCALE 1000
#define RADIAL_MULTIPLIER_SCALE 1000000

typedef struct {

  si32 time;                 /* time of closest scan to time requested */

  si32 grid_type;            /* TITAN_PROJ_FLAT (grid units in km) or
			      * TITAN_PROJ_LATLON (grid units in deg) */
                      
  si32 grid_dx;              /* grid delta x (units) * grid_scalex */
  si32 grid_dy;              /* grid delta y (units) * grid_scaley */
  si32 grid_scalex;
  si32 grid_scaley;

  si32 dbz_threshold;        /* reflectivity threshold for storms
			      * (dbz) */

  si32 min_storm_size;       /* minimum size for storms (km2 for 2D
			      * data; km3 for 3D data) */

  si32 forecast_lead_time;   /* lead time (secs) for forecast */

  si32 n_complex_tracks;     /* number of complex tracks */

  si32 n_entries;            /* number of entries following */

  si32 plot_current;         /* plot current storm shape (T/F) */
  si32 plot_forecast;        /* plot forecast storm shape (T/F) */
  si32 plot_ellipse;         /* plot storm shape as ellipse (T/F) */
  si32 plot_polygon;         /* plot storm shape as polygon (T/F) */
  si32 plot_trend;           /* plot storm intensity trend (T/F) */

  si32 ellipse_included;     /* ellipse data struct included (T/F) */
  si32 polygon_included;     /* polygon data struct included (T/F) */

  si32 speed_knots;          /* plot speed in knots (T/F) - default
			      * in km/h */
  si32 speed_round;          /* round speed to nearest 5 */
  si32 fixed_length_arrows;  /* plot arrows as fixed length (T/F) */

  si32 n_poly_sides;         /* number of sides in polygon */
  si32 poly_start_az;        /* (deg * angle_scale) azimuth from T.N. for
			      * first polygon point */
  si32 poly_delta_az;        /* (deg * angle_scale) azimuth delta between
			      * polygon points (pos is counterclockwise) */
  si32 angle_scale;
  si32 short_angle_scale;
  si32 speed_scale;
  si32 darea_dt_scale;
  si32 flat_proj_ellipse_scale;
  si32 latlon_proj_ellipse_scale;
  si32 radial_multiplier_scale;

  si32 spare;

} tdata_product_header_t;

/*
 * track entry for product data
 */

typedef struct {

  si32 longitude; /* deg * angle_scale */
  si32 latitude;  /* deg * angle_scale */

  ui16 direction;             /* (deg T * short_angle_scale) */
  ui16 speed;                 /* (km/h * speed_scale) */

  ui16 simple_track_num;
  ui16 complex_track_num;

  ui08 forecast_valid;         /* (T/F) */

  ui08 top;                    /* km */

  char intensity_trend;                 /* -1 decreasing
					 * 0 steady
					 * 1 increasing */

  char size_trend;                      /* -1 decreasing
					 * 0 steady
					 * 1 increasing */
} tdata_product_entry_t;

/*
 * track entry for product ellipse
 */

typedef struct {

  si16 norm_darea_dt;         /* rate of change of area per hour
				* normalized wrt current area */
  ui16 orientation;  /* (deg T * short_angle_scale) */
  ui16 minor_radius; /* (km * flat_proj_ellipse_scale) or
				* (deg * latlon_proj_ellipse_scale) */
  ui16 major_radius; /* (km * flat_proj_ellipse_scale) or
				* (deg * latlon_proj_ellipse_scale) */

} tdata_product_ellipse_t;
  
/*
 * track entry for product polygon
 */

typedef struct {

  si32 radial_multiplier;  /* Scale factor for polygon
			    * radials (* radial_multiplier_scale).
			    * Radials are in grid units.
			    * To retrieve from ui08 array, use:
			    * length (grid units) =
			    * (radials[i] * scale) / radial_multiplier_scale.
			    */

  ui08 radials[1]; /* (grid_units / radial_multiplier)
		      * This is used as a pointer to the radials array.
		      * The number of radials is passed as n_poly_sides in
		      * tdata_product_header_t */

} tdata_product_polygon_t;
  
/*
 * track monitor header struct
 */

typedef struct {

  si32 time;                 /* time of latest scan */

  si32 dbz_threshold;        /* reflectivity threshold for storms
			      * (dbz) */

  si32 min_storm_size;       /* minimum size for storms (km2 for
			      * 2D data; km3 for 3D data) */

  si32 forecast_lead_time;   /* lead time (secs) for forecast */

  si32 angle_scale;          /* scale factor for lat/long angles */

  si32 n_entries;            /* number of entries following */

} tdata_monitor_header_t;

/*
 * entry for monitor data
 */

#define NLONGS_TDATA_MONITOR_ENTRY 2

typedef struct {

  si32 longitude; /* deg * angle_scale */
  si32 latitude;  /* deg * angle_scale */
  ui08 pod;     /* percent */
  ui08 far;     /* percent */
  ui08 csi;     /* percent */
  ui08 spare;   /* percent */

} tdata_monitor_entry_t;

/*******************************
 * prod_mode function prototypes
 */

/*******************
 * tdata_prod_init()
 *
 * initialize file scope variables for product generation 
 */

extern void tdata_prod_init(int zero_growth_forecast,
			    int send_invalid_forecasts,
			    int forecast_lead_time,
			    int plot_current,
			    int plot_forecast,
			    int plot_trend,
			    int speed_knots,
			    int speed_round,
			    int fixed_length_arrows);

/*******************************************************************
 * tdata_prod_load_header()
 *
 * loads up product header struct
 */

extern void tdata_prod_load_header(tdata_product_header_t *header,
				   storm_file_params_t *sparams,
				   titan_grid_t *grid,
				   track_file_handle_t *t_handle,
				   si32 dtime,
				   si32 n_sent,
				   int plot_ellipse,
				   int plot_polygon,
				   int ellipse_included,
				   int polygon_included,
				   si32 n_poly_sides);

/*******************************************************************
 * tdata_prod_load_entry()
 *
 * loads up product entry struct
 *
 * Returns 1 if entry valid, 0 otherwise
 */

extern int tdata_prod_load_entry(tdata_product_entry_t *entry,
				 track_file_entry_t *file_entry,
				 storm_file_global_props_t *gprops,
				 track_file_forecast_props_t *fprops,
				 titan_grid_comps_t *comps,
				 double lead_time_hr,
				 double vol_threshold);

/*******************************************************************
 * tdata_prod_load_ellipse()
 *
 * loads up product ellipse struct
 *
 */

extern void tdata_prod_load_ellipse(tdata_product_ellipse_t *ellipse,
				    storm_file_global_props_t *gprops,
				    track_file_forecast_props_t *fprops,
				    titan_grid_comps_t *comps);

/*******************************************************************
 * tdata_prod_load_polygon()
 *
 * loads up product polygon struct
 *
 */

extern void tdata_prod_load_polygon(tdata_product_polygon_t *polygon,
				    storm_file_global_props_t *gprops,
				    si32 n_poly_sides);
  
/*******************************************************************
 * buffer_len()
 *
 * Compute tdata_product buffer len
 *
 */

extern int tdata_prod_buffer_len(int plot_ellipse, int plot_polygon,
				 int n_poly_sides, int n_entries);

#ifdef __cplusplus
}
#endif

#endif

