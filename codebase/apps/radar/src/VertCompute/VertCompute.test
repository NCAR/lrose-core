/**********************************************************************
 * TDRP params for VertCompute
 **********************************************************************/

//======================================================================
//
// VertCompute analyses data from tsarchive time series files taken 
//   during vertical pointing scans.
//
//======================================================================
 
//======================================================================
//
// DEBUGGING AND PROCESS CONTROL.
//
//======================================================================
 
///////////// debug ///////////////////////////////////
//
// Debug option.
// If set, debug messages will be printed appropriately.
//
// Type: enum
// Options:
//     DEBUG_OFF
//     DEBUG_NORM
//     DEBUG_VERBOSE
//

debug = DEBUG_OFF;

///////////// instance ////////////////////////////////
//
// Process instance.
// Used for registration with procmap.
// Type: string
//

instance = "test";

///////////// register_with_procmap ///////////////////
//
// Option to register with the process mapper.
// If true, this application will try to register with procmap once per 
//   minute. (If unable to do so, no error occurs.).
// Type: boolean
//

register_with_procmap = FALSE;

//======================================================================
//
// DATA INPUT.
//
//======================================================================
 
///////////// input_mode //////////////////////////////
//
// Input mode - files or time series API.
// DSR_MOMENTS_INPUT: read moments data in DSR format. TS_FMQ_INPUT: 
//   read time series from a file message queue and process the pulses as 
//   they come in. TS_FILE_INPUT: read time series from files specified on 
//   the command line.
//
// Type: enum
// Options:
//     DSR_MOMENTS_INPUT
//     TS_FILE_INPUT
//     TS_FMQ_INPUT
//

input_mode = TS_FILE_INPUT;

///////////// input_fmq_name //////////////////////////
//
// FMQ name. For DSR_MOMENTS_INPUT and TS_FMQ_INPUT.
// Path to FMQ files. There are 2 files, one with a .buf extension and 
//   one with a .stat extention. This path does not include the 
//   extensions.
// Type: string
//

input_fmq_name = "/tmp/fmq/ts";

///////////// seek_to_start_of_input //////////////////
//
// Option to seek to the start of the input FMQ.
// If TRUE, the program will seek to the start of the fmq and read in 
//   data from the entire queue. If FALSE, it will only read new data as 
//   it is added to the FMQ.
// Type: boolean
//

seek_to_start_of_input = FALSE;

//======================================================================
//
// INPUT MOMENTS.
//
// Names of input fields in the Dsr queue.
//
//======================================================================
 
///////////// input_fields ////////////////////////////
//
// List of input field from moments FMQ.
// Only applies to DSR_MOMENTS_INPUT.
//
// Type: struct
//   typedef struct {
//      moments_id_t id;
//        Options:
//          SNR
//          SNRHC
//          SNRHX
//          SNRVC
//          SNRVX
//          DBM
//          DBMHC
//          DBMHX
//          DBMVC
//          DBMVX
//          DBZ
//          VEL
//          WIDTH
//          ZDRC
//          ZDRM
//          LDRH
//          LDRV
//          PHIDP
//          RHOHV
//      string dsr_name;
//   }
//
// 1D array - variable length.
//

input_fields = {
  { SNR, "SNR"},
  { SNRHC, "SNRHC"},
  { SNRHX, "SNRHX"},
  { SNRVC, "SNRVC"},
  { SNRVX, "SNRVX"},
  { DBM, "DBM"},
  { DBMHC, "DBMHC"},
  { DBMHX, "DBMHX"},
  { DBMVC, "DBMVC"},
  { DBMVX, "DBMVX"},
  { DBZ, "DBZ"},
  { VEL, "VEL"},
  { WIDTH, "WIDTH"},
  { ZDRC, "ZDRC"},
  { ZDRM, "ZDRM"},
  { LDRH, "LDRH"},
  { LDRV, "LDRV"},
  { PHIDP, "PHIDP"},
  { RHOHV, "RHOHV"}
};

//======================================================================
//
// VERTICAL LAYERS.
//
//======================================================================
 
///////////// n_layers ////////////////////////////////
//
// Number of layers for analysis.
// Type: int
//

n_layers = 20;

///////////// delta_height ////////////////////////////
//
// Height of each layer (km).
// Type: double
//

delta_height = 0.5;

///////////// start_height ////////////////////////////
//
// Start height of lowest layer (km).
// Type: double
//

start_height = 0.5;

///////////// n_samples ///////////////////////////////
//
// Number of pulse samples in a beam.
// Type: int
//

n_samples = 64;

///////////// invert_hv_flag //////////////////////////
//
// Option to invert H/V flag in pulse data.
// Normally, the HV flag is set to 1 for H, 0 for V. If the flag in the 
//   data has the opposite sense, set this to true.
// Type: boolean
//

invert_hv_flag = FALSE;

//======================================================================
//
// MOMENTS COMPUTATION.
//
//======================================================================
 
///////////// hc_receiver /////////////////////////////
//
// Properties of horiz co-polar receiver.
// bsky_noise_at_ifd: the blue-sky noise measured at the ifd, in dBm. 
//   gain: the gain from the measurement plane to the ifd, in dB.
//
// Type: struct
//   typedef struct {
//      double bsky_noise_at_ifd;
//      double gain;
//      double radar_constant;
//   }
//
//

hc_receiver = { -77.7722, 36.8883, -70.72 };

///////////// hx_receiver /////////////////////////////
//
// Properties of horiz cross-polar receiver.
//
// Type: struct
//   typedef struct {
//      double bsky_noise_at_ifd;
//      double gain;
//      double radar_constant;
//   }
//
//

hx_receiver = { -77.8385, 36.8704, -71.18 };

///////////// vc_receiver /////////////////////////////
//
// Properties of vertical co-polar receiver.
//
// Type: struct
//   typedef struct {
//      double bsky_noise_at_ifd;
//      double gain;
//      double radar_constant;
//   }
//
//

vc_receiver = { -77.9013, 36.4709, -71.18 };

///////////// vx_receiver /////////////////////////////
//
// Properties of vertical cross-polar receiver.
//
// Type: struct
//   typedef struct {
//      double bsky_noise_at_ifd;
//      double gain;
//      double radar_constant;
//   }
//
//

vx_receiver = { -77.9904, 36.3969, -70.72 };

///////////// atmos_attenuation ///////////////////////
//
// Atmospheric attenuation (dB/km).
// Reflectivity is corrected for this.
// Type: double
//

atmos_attenuation = 0.016;

///////////// min_snr /////////////////////////////////
//
// Minimum SNR for analysis.
// Gates with SNR below this will be ignored.
// Type: double
//

min_snr = 30;

///////////// max_snr /////////////////////////////////
//
// Maximum SNR for analysis.
// Gates with SNR above this will be ignored.
// Type: double
//

max_snr = 60;

///////////// min_vel /////////////////////////////////
//
// Minimum VEL for analysis.
// Gates with VEL below this will be ignored.
// Type: double
//

min_vel = -100;

///////////// max_vel /////////////////////////////////
//
// Maximum VEL for analysis.
// Gates with VEL above this will be ignored.
// Type: double
//

max_vel = 100;

///////////// min_rhohv ///////////////////////////////
//
// Minimum RHOHV for analysis.
// Gates with RHOHV below this will be ignored.
// Type: double
//

min_rhohv = 0.95;

///////////// max_ldr /////////////////////////////////
//
// Max LDR for analysis.
// Gates with LDR above this will be ignored.
// Type: double
//

max_ldr = -20;

///////////// zdr_correction //////////////////////////
//
// Correction applied to calibrated ZDR.
// Type: double
//

zdr_correction = 0.50;

///////////// ldr_correction //////////////////////////
//
// Correction applied to LDR.
// Type: double
//

ldr_correction = 0;

//======================================================================
//
// ANALYSIS DETAILS.
//
//======================================================================
 
///////////// zdr_n_sdev //////////////////////////////
//
// Number of standard deviations for data QA purposes.
// QA is applied to the ZDR data by ignoring values outside a certain 
//   range around the mean. This is the number of standard deviations 
//   within which data is accepted.
// Type: double
//

zdr_n_sdev = 2;

///////////// min_ht_for_stats ////////////////////////
//
// Min ht for computing stats (km).
// A minimum ht is required to be clear of problems caused by 
//   differential recovery of the TR limiters.
// Type: double
//

min_ht_for_stats = 2;

///////////// max_ht_for_stats ////////////////////////
//
// Max ht for computing stats (km).
// A max ht is sometimes required to be below bright band.
// Type: double
//

max_ht_for_stats = 3.75;

//======================================================================
//
// OUTPUT RESULTS.
//
//======================================================================
 
///////////// output_dir //////////////////////////////
//
// Dir for output files.
// The results will be written to sub-directories named from the data 
//   time.
// Type: string
//

output_dir = "$(PROJ_DIR)/data/analysis/vert";

///////////// nrevs_for_global_stats //////////////////
//
// Number of revolutions for global stats.
// A global stats file will be output each time the number of 
//   revolutions reaches this value.
// Type: int
//

nrevs_for_global_stats = 4;

