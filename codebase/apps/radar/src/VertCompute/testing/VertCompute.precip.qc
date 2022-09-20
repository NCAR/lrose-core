/**********************************************************************
 * TDRP params for ./VertCompute
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
//
// If set, debug messages will be printed appropriately.
//
//
// Type: enum
// Options:
//     DEBUG_OFF
//     DEBUG_NORM
//     DEBUG_VERBOSE
//     DEBUG_EXTRA
//

debug = DEBUG_NORM;

///////////// instance ////////////////////////////////
//
// Process instance.
//
// Used for registration with procmap.
//
//
// Type: string
//

instance = "sband.sim";

///////////// register_with_procmap ///////////////////
//
// Option to register with the process mapper.
//
// If true, this application will try to register with procmap once per 
//   minute. (If unable to do so, no error occurs.).
//
//
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
//
// DSR_MOMENTS_INPUT: read moments data in DSR format. TS_FMQ_INPUT: 
//   read time series from a file message queue and process the pulses as 
//   they come in. TS_FILE_INPUT: read time series from files specified on 
//   the command line.
//
//
// Type: enum
// Options:
//     RADX_MOMENTS_INPUT
//     DSR_MOMENTS_INPUT
//     TS_FILE_INPUT
//     TS_FMQ_INPUT
//

// input_mode = TS_FILE_INPUT;
input_mode = RADX_MOMENTS_INPUT;

//======================================================================
//
// REALTIME - DSR_MOMENTS_INPUT, TS_FMQ_INPUT.
//
//======================================================================
 
///////////// input_fmq_name //////////////////////////
//
// FMQ name. For DSR_MOMENTS_INPUT and TS_FMQ_INPUT.
//
// Path to FMQ files. There are 2 files, one with a .buf extension and 
//   one with a .stat extention. This path does not include the 
//   extensions.
//
//
// Type: string
//

input_fmq_name = "$(DATA_DIR)/fmq/moments/sband/shmem_20000";

///////////// seek_to_start_of_input //////////////////
//
// Option to seek to the start of the input FMQ.
//
// Applies to DSR_MOMENTS_INPUT and TS_FMQ_INPUT. If TRUE, the program 
//   will seek to the start of the fmq and read in data from the entire 
//   queue. If FALSE, it will only read new data as it is added to the 
//   FMQ.
//
//
// Type: boolean
//

seek_to_start_of_input = FALSE;

//======================================================================
//
// ARCHIVE - RADX_MOMENTS_INPUT, TS_FILE_INPUT.
//
//======================================================================
 
///////////// input_dir ///////////////////////////////
//
// Input directory for searching for files between start_time and 
//   end_time.
//
// RADX_MOMENTS_INPUT, TS_FILE_INPUT: files will be searched for in this 
//   directory. This is ignored if the list of files is  specified on the 
//   command line using the -f option.
//
//
// Type: string
//

input_dir = ".";

///////////// start_time //////////////////////////////
//
// Set the start time for ARCHIVE mode analysis.
//
//  Applies to RADX_MOMENTS_INPUT and TS_FILE_INPUT. Format is 'yyyy mm 
//   dd hh mm ss'.
//
//
// Type: string
//

start_time = "1970 01 01 00 00 00";

///////////// end_time ////////////////////////////////
//
// Set the end time for ARCHIVE mode analysis.
//
//  Applies to RADX_MOMENTS_INPUT and TS_FILE_INPUT. Format is 'yyyy mm 
//   dd hh mm ss'.
//
//
// Type: string
//

end_time = "1970 01 01 00 00 00";

//======================================================================
//
// COMPUTING MOMENTS FROM TIME SERIES.
//
// TS_FILE_INPUT and TS_FMQ_INPUT modes.
//
//======================================================================
 
///////////// n_samples ///////////////////////////////
//
// Number of pulse samples in a beam.
//
//
// Type: int
//

n_samples = 128;

///////////// invert_hv_flag //////////////////////////
//
// Option to invert H/V flag in pulse data.
//
// Normally, the HV flag is set to 1 for H, 0 for V. If the flag in the 
//   data has the opposite sense, set this to true.
//
//
// Type: boolean
//

invert_hv_flag = FALSE;

///////////// cal_xml_file_path ///////////////////////
//
// Path for XML calibration file.
//
// At a minimum, the calibration file should contain noiseDbm, 
//   receiverGainDbm, dbz0 and radarConstant values for each channel.
//
//
// Type: string
//

cal_xml_file_path = "$(HOME)/git/spol-configuration/projDir/mgen/params/sband_cal.xml";

///////////// adjust_dbz_for_measured_xmit_power //////
//
// Option to adjust DBZ based on measured transmitter power.
//
// If true, and the measured transmitter power is available, the 
//   difference between the measured power and calibration power will be 
//   used to adjust the computed DBZ fields.
//
//
// Type: boolean
//

adjust_dbz_for_measured_xmit_power = FALSE;

///////////// adjust_zdr_for_measured_xmit_power //////
//
// Option to adjust ZDR based on measured transmitter power.
//
// If true, and the measured transmitter power is available, the 
//   difference between the measured power and calibration power will be 
//   used to adjust the computed ZDR fields.
//
//
// Type: boolean
//

adjust_zdr_for_measured_xmit_power = FALSE;

///////////// xmit_rcv_mode ///////////////////////////
//
// Transmit/Receive mode for radar.
//
// Mode for xmit and receive for polarization diversity
// DP_ALT_HV_CO_ONLY: Dual pol, alternating transmission, copolar 
//   receiver only (e.g. CP2 S-band)
//  DP_ALT_HV_CO_CROSS: Dual pol, alternating transmission, co-polar and 
//   cross-polar receivers (e.g. SPOL with Mitch Switch and receiver in 
//   switching mode, CHILL)
//  DP_ALT_HV_FIXED_HV: Dual pol, alternating transmission, fixed H and 
//   V receivers (e.g. SPOL with Mitch Switch and receivers in fixed mode)
//  DP_SIM_HV_FIXED_HV: Dual pol, simultaneous transmission, fixed H and 
//   V receivers (e.g. NEXRAD upgrade, SPOL with T and receivers in fixed 
//   mode)
//  DP_SIM_HV_SWITCHED_HV: Dual pol, simultaneous transmission, 
//   switching H and V receivers (e.g. SPOL with T and receivers in 
//   switching mode).
//
//
// Type: enum
// Options:
//     DP_ALT_HV_CO_ONLY
//     DP_ALT_HV_CO_CROSS
//     DP_ALT_HV_FIXED_HV
//     DP_SIM_HV_FIXED_HV
//     DP_SIM_HV_SWITCHED_HV
//

xmit_rcv_mode = DP_SIM_HV_FIXED_HV;

//======================================================================
//
// READING MOMENTS FROM FMQ.
//
// DSR_MOMENTS_INPUT mode only.
//
//======================================================================
 
///////////// min_elevation ///////////////////////////
//
// Minimum elevation angle (deg).
//
// Beams below this elevation angle will be ignored.
//
//
// Type: double
//

min_elevation = 88;

///////////// input_fields ////////////////////////////
//
// List of input fields for moments.
//
// Applies to RADX_MOMENTS_INPUT and DSR_MOMENTS_INPUT.
//
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
//          ZDRM
//          LDRH
//          LDRV
//          PHIDP
//          RHOHV
//      string moments_name;
//   }
//
// 1D array - variable length.
//

input_fields = {
  {
    id = SNR,
    moments_name = "SNRHC"
  }
  ,
  {
    id = SNRHC,
    moments_name = "SNRHC"
  }
  ,
  {
    id = SNRVC,
    moments_name = "SNRVC"
  }
  ,
  {
    id = DBM,
    moments_name = "DBMHC"
  }
  ,
  {
    id = DBMHC,
    moments_name = "DBMHC"
  }
  ,
  {
    id = DBMVC,
    moments_name = "DBMVC"
  }
  ,
  {
    id = DBZ,
    moments_name = "DBZ"
  }
  ,
  {
    id = VEL,
    moments_name = "VEL"
  }
  ,
  {
    id = WIDTH,
    moments_name = "WIDTH"
  }
  ,
  {
    id = ZDRM,
    moments_name = "ZDRM"
  }
  ,
  {
    id = LDRH,
    moments_name = "LDRH"
  }
  ,
  {
    id = LDRV,
    moments_name = "LDRV"
  }
  ,
  {
    id = PHIDP,
    moments_name = "PHIDP"
  }
  ,
  {
    id = RHOHV,
    moments_name = "RHOHV"
  }
};

//======================================================================
//
// VERTICAL LAYERS.
//
//======================================================================
 
///////////// n_layers ////////////////////////////////
//
// Number of layers for analysis.
//
//
// Type: int
//

n_layers = 16;

///////////// delta_height ////////////////////////////
//
// Height of each layer (km).
//
//
// Type: double
//

delta_height = 0.5;

///////////// start_height ////////////////////////////
//
// Start height of lowest layer (km).
//
//
// Type: double
//

start_height = 0.5;

//======================================================================
//
// COMPUTING STATISTICS.
//
//======================================================================
 
///////////// cumulative_azimuth_moved_for_stats //////
//
// Cumulative delta az for computing stats (deg).
//
// We only compute stats after the antenna has scanned this number of 
//   degrees.
//
//
// Type: double
//

cumulative_azimuth_moved_for_stats = 1080;

///////////// cumulative_azimuth_moved_for_debug_print 
//
// Cumulative delta az for printing debug stats (deg).
//
//
// Type: double
//

cumulative_azimuth_moved_for_debug_print = 1080;

///////////// max_time_gap_for_stats //////////////////
//
// Max time gap in data for stats (s).
//
// If we find a gap that exceeds this, we clear the stats and start 
//   again.
//
//
// Type: double
//

max_time_gap_for_stats = 3600;

///////////// min_snr /////////////////////////////////
//
// Minimum SNR for analysis.
//
// Gates with SNR below this will be ignored.
//
//
// Type: double
//

min_snr = 15;

///////////// max_snr /////////////////////////////////
//
// Maximum SNR for analysis.
//
// Gates with SNR above this will be ignored.
//
//
// Type: double
//

max_snr = 70;

///////////// min_vel /////////////////////////////////
//
// Minimum VEL for analysis.
//
// Gates with VEL below this will be ignored.
//
//
// Type: double
//

min_vel = -100;

///////////// max_vel /////////////////////////////////
//
// Maximum VEL for analysis.
//
// Gates with VEL above this will be ignored.
//
//
// Type: double
//

max_vel = 100;

///////////// min_rhohv ///////////////////////////////
//
// Minimum RHOHV for analysis.
//
// Gates with RHOHV below this will be ignored.
//
//
// Type: double
//

min_rhohv = 0.99;

///////////// max_ldr /////////////////////////////////
//
// Max LDR for analysis.
//
// Gates with LDR above this will be ignored.
//
//
// Type: double
//

max_ldr = 9999;

///////////// zdr_n_sdev //////////////////////////////
//
// Number of standard deviations for data QA purposes.
//
// QA is applied to the ZDR data by ignoring values outside a certain 
//   range around the mean. This is the number of standard deviations 
//   within which data is accepted.
//
//
// Type: double
//

zdr_n_sdev = 2;

///////////// min_ht_for_stats ////////////////////////
//
// Min ht for computing stats (km).
//
// A minimum ht is required to be clear of problems caused by 
//   differential recovery of the TR limiters.
//
//
// Type: double
//

min_ht_for_stats = 2.4;

///////////// max_ht_for_stats ////////////////////////
//
// Max ht for computing stats (km).
//
// A max ht is sometimes required to be below bright band.
//
//
// Type: double
//

max_ht_for_stats = 10.0;

//======================================================================
//
// OUTPUT STATISTICS.
//
//======================================================================
 
///////////// write_stats_to_text_file ////////////////
//
// Option to write statistics to a text file.
//
// If true, stats for the entire run will be writted to a text file.
//
//
// Type: boolean
//

write_stats_to_text_file = TRUE;

///////////// text_output_dir /////////////////////////
//
// Dir for output text files.
//
// The results will be written to sub-directories named from the data 
//   time.
//
//
// Type: string
//

// text_output_dir = "/tmp/output/VertCompute.precip.qc";
text_output_dir = "/tmp/output/VertCompute.precip.qc.moments";

///////////// write_zdr_point_values_to_text_file /////
//
// Option to write individual ZDR values to a text file.
//
// If true, the individual zdrm values, plus metadata such as height, is 
//   written to a text file.
//
//
// Type: boolean
//

write_zdr_point_values_to_text_file = FALSE;

///////////// zdr_points_output_dir ///////////////////
//
// Dir for output text files.
//
// The results will be written to sub-directories named from the data 
//   time.
//
//
// Type: string
//

zdr_points_output_dir = "./output/zdr_points";

///////////// write_stats_to_spdb /////////////////////
//
// Option to write out stats results to SPDB.
//
// The results will be written in XML, stored in SPDB. The data can then 
//   be retrieved for plotting or other purposes.
//
//
// Type: boolean
//

write_stats_to_spdb = FALSE;

///////////// spdb_output_url /////////////////////////
//
// URL for SPDB XML.
//
// For local writes, specify the directory. For remote writes, specify 
//   the full url: spdbp:://host::dir.
//
//
// Type: string
//

spdb_output_url = "/tmp/spdb/vert/sband.sim";

///////////// radar_name_for_spdb /////////////////////
//
// Radar name for tagging SPDB output data.
//
// This is used to set the data_type in SPDB.
//
//
// Type: string
//

radar_name_for_spdb = "SPOL";

