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

/*************************************************************************
 * polar2mdv.h
 *
 * The header for polar2mdv program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1990
 *
 *************************************************************************/


#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <rapmath/umath.h>
#include <titan/radar.h>
#include <toolsa/sockutil.h>
#include <toolsa/pmu.h>

/*
 * default defines
 */

/*
 * output socket objects
 */

#define OUT_OBJ_NAME "cartesian_radar_data"
#define N_OUT_OBJS 1
#define OUT_OBJ 0

/*
 * server mapper
 */

#define SERVMAP_HOST1_STR "local"
#define SERVMAP_HOST2_STR "none"

#define INFO "Cartesian slave server"
#define INSTANCE "Test"

/*
 * flags
 */

#define AUTO_MID_TIME "false"
#define OVERRIDE_TABLE_LATLON "false"
#define CHECK_MISSING_BEAMS "false"
#define REPORT_MISSING_BEAMS "false"
#define DEBUG_STR "false"
#define REMOVE_CLUTTER "false"
#define RUN_LENGTH_ENCODE "false"
#define TRANSMIT_CART_DATA "false"
#define USE_REPEATED_ELEVATIONS "true"
#define USE_MAX_ON_REPEAT "false"
#define CHECK_SN "true"
#define SWAP_TIMES_IF_INCORRECT "false"

/*
 * default values
 */

#define AZIMUTH_OFFSET 0.5
#define AGE_AT_END_OF_VOLUME 180L
#define TIMESTAMP_AT_START_OF_TILT -1L
#define CHECK_CLIENTS_INTERVAL 1.0
#define CLUTTER_TABLE_PATH "clutter_table"
#define OUTPUT_FILE_EXT "mdv"
#define FIELD_POSITIONS "1 2 3 4 5 6"
#define RDATA_DIR "rdata"
#define MAX_MISSING_BEAMS 50L
#define MAX_PACKET_LENGTH 1400L
#define MIN_SN_THRESHOLD -28.0
#define MIN_VALID_RUN 5L
#define MISSING_DATA_VAL 0x00
#define NFIELDS_PROCESSED 6L
#define PORT 50001L
#define RC_TABLE_PATH "rc_table"
#define HOSTNAME "local"
#define SHMEM_KEY 66666L
#define SHMEM_READ_WAIT 0.1
#define SN_THRESHOLD 0.0

/*
 * global structure
 */

typedef struct {

  char *prog_name;                       /* program name */
  char *params_path_name;                /* parameters file path name */

  /*
   * server port and server mapper
   */

  int port;                     /* port number for cart service */

  char *info;                   /* server mapper info for this
				 * server */

  char *instance;               /* instance - matched on requests to
				   servmap and procmap */

  char *servmap_host1;          /* server mapper hosts */
  char *servmap_host2;
  
  si32 latest_data_time;        /* times for registering with server */
  si32 latest_request_time;	/* mapper */

  /*
   * files, directories and hosts
   */

  char *rdata_dir;
  char *local_tmp_dir;
  char *hostname;
  char *output_file_ext;

  /*
   * clutter and cartesian lookup tables
   */
  
  char *clutter_table_path;
  char *rc_table_path;

  /*
   * flag to indicate whether to overwrite the array with data
   * from later tilts at the same target elevation
   */

  int use_repeated_elevations;

  /*
   * flag to indicate whether to take max value if an elevation is repeated.
   * The use of this option depends upon using 0 for the missing value.
   */

  int use_max_on_repeat;

  /*
   * flag to indicate whether start and end times should be swapped if
   * the start time is after the end time.  We need to do this if the
   * beams are written to the file backwards or something, but don't
   * want to do this in general in case the incorrect times are indicative
   * of a big problem.
   */

  int swap_times_if_incorrect;
  
  /*
   * age at end of volume (secs) - this is the estimated mean age of the
   * data when the volume is complete. If auto_mid_time is set TRUE, then
   * the mid time is computed as the mean of the start and end times of
   * the data. Otherwise, it is computed as the end time minus the
   * age at the end of the volume
   */

  int auto_mid_time;              /* TRUE or FALSE */
  si32 age_at_end_of_volume;

  /*
   * timestamp_at_start_of_tilt - this is the tiltnumber to use for
   * setting the volume_timestamp used for timestamping the output file.  
   * If timestamp_at_start_of_tilt parameter is not set by the user,
   * the default value (-1) indicates that the old timestamping
   * scheme should be used
   */
  si32 timestamp_at_start_of_tilt;
  si32 volume_timestamp;

  /*
   * option to override the lat/lon in the lookup table and use the
   * radar lat/lon instead. This is useful for radar which are located
   * on a moving platform, such as a ship.
   */

  int override_table_latlon;
  fl32 radar_lat; /* used to pass lat to mdv file if override is on */
  fl32 radar_lon; /* used to pass lat to mdv file if override is on */

  /*
   * shared memory
   */

  key_t shmem_key;
  si32 shmem_read_wait;            /* wait on the read if no data is
				    * ready - input as secs, stored
				    * as micro-secs */

  double azimuth_offset;           /* offset to use when computing
				    * the azimuth number */

  /*
   * Max allowable vol interval
   */

  si32 max_vol_duration;           /* max allowed vol interval (secs) */

  /*
   * allowable number of missing beams
   */

  si32 max_missing_beams;          /* max allowed number of missing beams
				    * per volume */

  /*
   * noise removal
   */

  double sn_threshold;             /* signal/noise threshold */
  si32 min_valid_run;              /* min run length (in a beam) of data
				    * above the signal/noise threshold for
				    * the data to be considered valid */

  /*
   * number of fields to be processed, and their positions in the
   * data stream
   */

  si32 nfields_processed;
  si32 *field_positions;

  /*
   * socket parameters
   */

  si32 max_packet_length;
  double check_clients_interval;
  char *service_name;

  /*
   * decision flags
   */

  int debug;
  int run_length_encode;
  int remove_clutter;
  int transmit_cart_data;
  int check_missing_beams;
  int report_missing_beams;
  int check_sn;

} global_t;

/*
 * globals
 */

#ifdef MAIN

global_t *Glob = NULL;

/*
 * if not main, declare global struct as extern
 */

#else

extern global_t *Glob;

#endif

/*
 * function prototypes
 */

extern si32 elev_index(double elev, scan_table_t *scan_table);

extern void check_for_new_clients();

extern void check_geom( rc_table_file_handle_t *rc_handle,
		       clutter_table_file_handle_t *clutter_handle);

extern void handle_end_of_volume(void);

extern void init_cart_vol_index(vol_file_handle_t *v_handle,
				rc_table_file_handle_t *rc_handle,
				clutter_table_file_handle_t *clutter_handle);

extern void parse_args(int argc,
		       char **argv);

extern void process_beam (rdata_shmem_beam_header_t *bhdr,
			  ui08 *field_data,
			  vol_file_handle_t *v_handle,
			  rc_table_file_handle_t *rc_handle,
			  clutter_table_file_handle_t *clutter_handle);

extern void read_params(void);

extern void read_shmem(vol_file_handle_t *v_handle,
		       rc_table_file_handle_t *rc_handle,
		       clutter_table_file_handle_t *clutter_handle);

extern void register_server(void);

extern void setup_shmem(scan_table_t *scan_table);

extern void setup_sockets();

extern void tidy_and_exit(int sig);

extern void unlink_tmp_files(void);

extern void write_volume(vol_file_handle_t *v_handle,
			 si32 nscan,
			 si32 nrays_target,
			 si32 nrays,
			 si32 nnoise,
			 si32 *sum_noise);

extern char *get_shmem_buffer(void);
extern rdata_shmem_header_t *get_shmem_header(void);
extern int get_sem_id(void);
extern field_params_t *get_field_params(void);
extern rdata_shmem_beam_header_t *get_beam_headers(void);


#ifdef __cplusplus
}
#endif
