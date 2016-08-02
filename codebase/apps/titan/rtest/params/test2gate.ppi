/****************************************************
 * TDRP params for test2gate
 ****************************************************/

/*
 * debug flag.
 * TRUE if running in debug mode; FALSE otherwise.
 *
 * Type: boolean
 * Default: TRUE 
 */
debug = FALSE;

/*
 * malloc debug level.
 * 0 - none, 1 - corruption checking, 2 - records all malloc blocks
 *   and checks, 3 - printout of all mallocs etc.
 *
 * Type: long
 * Default: 0 
 * Min value: 0 
 * Max value: 3 
 */
malloc_debug_level = 1;

/*
 * output port.
 * Port used for sending radar gate data to clients.
 *
 * Type: long
 * Default: 60000 
 */
output_port = 49000;

/*
 * Option to output veloocity field.
 * If FALSE, only dBZ is output. If TRUE, velocity is also output.
 *
 * Type: boolean
 * Default: TRUE 
 */
output_vel_field = TRUE;

/*
 * Option to output geometry fields.
 * If TRUE, elevation, azimuth and range fields are also output.
 *
 * Type: boolean
 * Default: FALSE 
 */
output_geom_fields = TRUE;

/*
 * radar parameters.
 * Default radar parameters.
 *
 * Type: struct
 * Default: <structure> 
 */
radar_params = {
  0, /* radar_id:long */
  1604, /* altitude:long */
  39.87823, /* latitude:double */
  -104.759, /* longitude:double */
  660, /* num_gates:long */
  225, /* gate_spacing:double */
  112.5, /* start_range:double */
  0.95, /* beam_width:double */
  45, /* samples_per_beam:long */
  6000, /* pulse_width:double */
  1000, /* prf:double */
  10  /* wavelength:double */
};

/*
 * Use scan table.
 * If set, use scan azimuth table. If not, construct table from
 *   elevation angles, start azimuth and delta azimuth.
 *
 * Type: boolean
 * Default: TRUE 
 */
use_scan_table = FALSE;

/*
 * scan table path.
 *
 * Type: string
 * Default: "./tass_scan_table" 
 */
scan_table_path = "./dtass_scan_table";

/*
 * Noise dBZ level at 100 km range.
 * Used to compute noise reflectivity at all ranges.
 *
 * Type: double
 * Default: 0 
 */
noise_dbz_at_100km = 0;

/*
 * Elevation angle list.
 * If use_scan_table is false, these elevation angles are used
 *   to construct the scan table.
 *
 * Type: double
 * Default: 1 
 * Array elem size: 8 bytes
 * Array has no max number of elements
 */
elev_angles = {0.5, 1.2, 2.5, 4.0, 5.5, 7.0, 8.5, 10.0, 13.0, 17.0, 22.0};

/*
 * Number of aziimuths.
 * If use_scan_table is false, this is used to construct the scan
 *   table.
 *
 * Type: long
 * Default: 360 
 */
nazimuths = 360;

/*
 * Start azimuth angle.
 * If use_scan_table is false, this is used to construct the scan
 *   table.
 *
 * Type: double
 * Default: 0 
 */
start_azimuth = 0;

/*
 * Delta azimuth angle.
 * If use_scan_table is false, this is used to construct the scan
 *   table.
 *
 * Type: double
 * Default: 1 
 */
delta_azimuth = 1;

/*
 * Radar sample file path.
 * Path for file from which radar data are sampled.
 *
 * Type: string
 * Default: "null" 
 */
radar_sample_file_path = "$(RTEST_HOME)/test_data/235716.dob";

/*
 * Sample dbz field num.
 *
 * Type: long
 * Default: 0 
 */
sample_dbz_field = 0;

/*
 * Sample vel field num.
 *
 * Type: long
 * Default: 0 
 */
sample_vel_field = 1;

/*
 * Radar data sampling origin.
 * Origin from which the dobson radar data are sampled.
 *
 * Type: struct
 * Default: <structure> 
 */
sampling_origin = {
  0, /* start_x:double */
  0, /* start_y:double */
  50, /* speed:double */
  240  /* dirn:double */
};

/*
 * Wait per beam (milli-secs).
 *
 * Type: long
 * Default: 50 
 */
beam_wait_msecs = 10;

/*
 * Wait per vol (secs).
 *
 * Type: long
 * Default: 30 
 */
vol_wait_secs = 5;
