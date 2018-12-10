/**********************************************************************
 * TDRP params for NcarPidParams
 **********************************************************************/

//======================================================================
//
// The NCAR PID method is based on a fuzzy logic approach.
//
// This is documented in the following reference: Cloud Microphysics 
//   Retrieval Using S-Band Dual-Polarization Radar Measurements: J. 
//   Vivekanandan, D. S. Zrnic, S. M. Ellis, R. Oye, A. V. Ryzhkov, and J. 
//   Straka. Bulletin of the American Meteorological Society, 1999.
//
//======================================================================
 
///////////// PID_thresholds_file_path ////////////////
//
// File path for fuzzy logic thresholds for PID.
//
// This file contains the thresholds and weights for computing particle 
//   ID.
//
//
// Type: string
//

PID_thresholds_file_path = "./pid_thresholds.nexrad";

///////////// PID_snr_threshold ///////////////////////
//
// Minimum SNR for valid PID.
//
// If the SNR at a gate is below this, the PID is censored.
//
//
// Type: double
//

PID_snr_threshold = 3;

///////////// PID_snr_upper_threshold /////////////////
//
// Maximum SNR for valid PID.
//
// If the SNR at a gate is above this value, the PID will be set to 
//   SATURATED_SNR = 18.
//
//
// Type: double
//

PID_snr_upper_threshold = 9999;

///////////// PID_min_valid_interest //////////////////
//
// Minimum valid interest value for identifying a particle.
//
// If the computed interest value is below this, the PID is set to 
//   missing.
//
//
// Type: double
//

PID_min_valid_interest = 0.5;

///////////// PID_apply_median_filter_to_DBZ //////////
//
// Option to filter DBZ with median filter.
//
// The filter is computed in range.
//
//
// Type: boolean
//

PID_apply_median_filter_to_DBZ = TRUE;

///////////// PID_DBZ_median_filter_len ///////////////
//
// Length of median filter for DBZ - gates.
//
// See 'PID_apply_median_filter_to_DBZ'.
//
//
// Type: int
//

PID_DBZ_median_filter_len = 5;

///////////// PID_apply_median_filter_to_ZDR //////////
//
// Option to filter ZDR with median filter.
//
// The filter is computed in range.
//
//
// Type: boolean
//

PID_apply_median_filter_to_ZDR = TRUE;

///////////// PID_ZDR_median_filter_len ///////////////
//
// Length of median filter for ZDR - gates.
//
// See 'PID_apply_median_filter_to_ZDR'.
//
//
// Type: int
//

PID_ZDR_median_filter_len = 5;

///////////// PID_apply_median_filter_to_RHOHV ////////
//
// Option to filter RHOHV with median filter.
//
// The filter is computed in range.
//
//
// Type: boolean
//

PID_apply_median_filter_to_RHOHV = TRUE;

///////////// PID_RHOHV_median_filter_len /////////////
//
// Length of median filter for RHOHV - gates.
//
// See 'PID_apply_median_filter_to_RHOHV'.
//
//
// Type: int
//

PID_RHOHV_median_filter_len = 5;

///////////// PID_apply_median_filter_to_LDR //////////
//
// Option to filter LDR with median filter.
//
// The filter is computed in range.
//
//
// Type: boolean
//

PID_apply_median_filter_to_LDR = TRUE;

///////////// PID_LDR_median_filter_len ///////////////
//
// Length of median filter for LDR - gates.
//
// See 'PID_apply_median_filter_to_LDR'.
//
//
// Type: int
//

PID_LDR_median_filter_len = 5;

///////////// PID_replace_missing_LDR /////////////////
//
// For PID, option to replace missing LDR values with a specified value.
//
// When the SNR gets low, LDR is unreliable since there is not 
//   sufficient dynamic range to provide an accurate cross-polar power 
//   measurement. In these cases, it makes sense to replace LDR with a 
//   neutral value, such as 0.0, so that we do not reject gates at which 
//   valuable data is available.
//
//
// Type: boolean
//

PID_replace_missing_LDR = TRUE;

///////////// PID_LDR_replacement_value ///////////////
//
// Value to which LDR will be set if missing.
//
//
// Type: double
//

PID_LDR_replacement_value = 0;

///////////// PID_ngates_for_sdev /////////////////////
//
// Number of gates for computing standard deviations.
//
// This applies to computing the standard deviation of zdr and phidp.
//
//
// Type: int
//

PID_ngates_for_sdev = 9;

///////////// PID_output_particle_interest_fields /////
//
// Option to output the individual interest fields.
//
// If TRUE, the interest field for each particle type will be written to 
//   the output FMQ, in addition to the list in 'output_fields'.
//
//
// Type: boolean
//

PID_output_particle_interest_fields = FALSE;

///////////// PID_locate_melting_layer ////////////////
//
// Option to locate the melting layer.
//
// If true, the melting layer will be located using the WET_SNOW 
//   category. In addition, the ML_INTEREST field will be computed. 
//   Otherwise it will be missing. Follows Giangrande et al. - Automatic 
//   Designation of the Melting Layer with Polarimitric Prototype of 
//   WSR-88D Radar - AMS JAMC, Vol47, 2008.
//
//
// Type: boolean
//

PID_locate_melting_layer = TRUE;

///////////// PID_melting_layer_percentile_for_bottom_limit 
//
// Percentile value for estimating the bottom of the melting layer.
//
// To locate the melting layer limits, we rank the heights of all gates 
//   containing WET_SNOW, from bottom to top. This is the percentile value 
//   for the bottom of the layer.
//
//
// Type: double
//

PID_melting_layer_percentile_for_bottom_limit = 25;

///////////// PID_melting_layer_percentile_for_top_limit 
//
// Percentile value for estimating the top of the melting layer.
//
// To locate the melting layer limits, we rank the heights of all gates 
//   containing WET_SNOW, from bottom to top. This is the percentile value 
//   for the top of the layer.
//
//
// Type: double
//

PID_melting_layer_percentile_for_top_limit = 75;

///////////// PID_melting_layer_write_results_to_spdb /
//
// Option to save melting layer properties to spdb.
//
// If true, the melting layer properties will be saved to SPDB using XML 
//   encoding. See 'melting_layer_spdb_output_url'.
//
//
// Type: boolean
//

PID_melting_layer_write_results_to_spdb = FALSE;

///////////// PID_melting_layer_spdb_output_url ///////
//
// URL for writing melting layer results to SPDB XML.
//
// For local writes, specify the directory. For remote writes, specify 
//   the full url: spdbp:://host::dir.
//
//
// Type: string
//

PID_melting_layer_spdb_output_url = "/tmp/spdb/melting_layer";

//======================================================================
//
// SOUNDINGS FOR PID TEMPERATURES.
//
//======================================================================
 
///////////// PID_use_soundings_from_spdb /////////////
//
// Option to read sounding data from SPDB.
//
// If TRUE, the program will read the closest (in time) available 
//   sounding from an SPDB data base. The temperature profile from the 
//   sounding will be used to override the temperature profile in the PID 
//   thresholds config file.
//
//
// Type: boolean
//

PID_use_soundings_from_spdb = FALSE;

///////////// PID_sounding_spdb_url ///////////////////
//
// SPDB URL for sounding data.
//
//
// Type: string
//

PID_sounding_spdb_url = "/scr/rain1/rsfdata/projects/pecan/spdb/sounding/ruc";

///////////// PID_sounding_search_time_margin_secs ////
//
// Time margin for retrieving sounding, in secs.
//
// This is the total size of the output FMQ buffer. Some of this buffer 
//   will be used for control bytes (12 bytes per message).
//
//
// Type: int
//

PID_sounding_search_time_margin_secs = 86400;

///////////// PID_sounding_location_name //////////////
//
// Name of sounding location.
//
// If set, we request a profile just for that sounding. If empty, all 
//   soundings in the data base are considered valid.
//
//
// Type: string
//

PID_sounding_location_name = "KDDC";

///////////// PID_sounding_check_pressure_range ///////
//
// Option to check that pressure covers the required range.
//
// If TRUE, we will check that pressure range in the sounding meets or 
//   exceeds the min and max specified.
//
//
// Type: boolean
//

PID_sounding_check_pressure_range = FALSE;

///////////// PID_sounding_required_pressure_range_hpa 
//
// Required pressure range for sounding to be valid (hPa).
//
// This is used to provide a quality check on the sounding. If the 
//   pressure data does not fully cover this range, the sounding is 
//   rejected and we look back for the next available one.
//
//
// Type: struct
//   typedef struct {
//      double min_val;
//      double max_val;
//   }
//
//

PID_sounding_required_pressure_range_hpa = {
    min_val = 300,
    max_val = 950
};

///////////// PID_sounding_check_height_range /////////
//
// Option to check that height covers the required range.
//
// If TRUE, we will check that height range in the sounding meets or 
//   exceeds the min and max specified.
//
//
// Type: boolean
//

PID_sounding_check_height_range = FALSE;

///////////// PID_sounding_required_height_range_m ////
//
// Required height range for sounding to be valid (m).
//
// This is used to provide a quality check on the sounding. If the 
//   height data does not fully cover this range, the sounding is rejected 
//   and we look back for the next available one.
//
//
// Type: struct
//   typedef struct {
//      double min_val;
//      double max_val;
//   }
//
//

PID_sounding_required_height_range_m = {
    min_val = 500,
    max_val = 15000
};

///////////// PID_sounding_check_pressure_monotonically_decreasing 
//
// Option to check that pressure decreases monotonically.
//
// If TRUE, we will check that pressure decreases monotonically. If not, 
//   the sounding is rejected and we look back for the next available one.
//
//
// Type: boolean
//

PID_sounding_check_pressure_monotonically_decreasing = FALSE;

///////////// PID_sounding_height_correction_km ///////
//
// Correction to the heights read in with the sounding (km).
//
// This correction is ADDED to the heights read in. So if the freezing 
//   level seems low, the correction should be positive. If the freezing 
//   level appears high, it should be negative.
//
//
// Type: double
//

PID_sounding_height_correction_km = 0;

///////////// PID_sounding_use_wet_bulb_temp //////////
//
// Option to use wet bulb temperature profile.
//
// If TRUE, we use the wet bulb temperature profile from the sounding 
//   instead of the dry bulb. This is thought to more closely track the 
//   temperature of melting ice.
//
//
// Type: boolean
//

PID_sounding_use_wet_bulb_temp = TRUE;

