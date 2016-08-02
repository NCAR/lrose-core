/**********************************************************************
 * TDRP params for RadxMergeFields
 **********************************************************************/

//======================================================================
//
// Merges fields from multiple CfRadial files into a single file.
//
//======================================================================
 
//======================================================================
//
// DEBUGGING.
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
//     DEBUG_EXTRA
//

debug = DEBUG_OFF;

///////////// instance ////////////////////////////////
//
// Program instance for process registration.
// This application registers with procmap. This is the instance used 
//   for registration.
// Type: string
//

instance = "cmd";

//======================================================================
//
// DATA INPUT.
//
//======================================================================
 
///////////// mode ////////////////////////////////////
//
// Operating mode.
// In REALTIME mode, the program waits for a new input file.  In ARCHIVE 
//   mode, it moves through the data between the start and end times set 
//   on the command line. In FILELIST mode, it moves through the list of 
//   file names specified on the command line. Paths (in FILELIST mode, at 
//   least) MUST contain a day-directory below the data file -- 
//   ./data_file.ext will not work as a file path.
//
// Type: enum
// Options:
//     REALTIME
//     ARCHIVE
//     FILELIST
//

mode = ARCHIVE;

///////////// max_realtime_data_age_secs //////////////
//
// Maximum age of realtime data (secs).
// Only data less old than this will be used.
// Type: int
//

max_realtime_data_age_secs = 300;

//======================================================================
//
// DATA LOCATIONS.
//
//======================================================================
 
///////////// input_datasets //////////////////////////
//
// Index of directories containing the data files.
// The indices are used by output_fields to indicate where each field is 
//   located. The lowest index is used as the primary input. The geometry 
//   of non-primary files will be converted to match the primary files. 
//   The file_match_time_offset_sec and file_match_time_tolerance_sec are 
//   used to search for the best secondary file to merge. The offset is 
//   ignored for the primary field (index 1). The ray tolerances are used 
//   to match rays in the secondary file with those in the primary file.
//
// Type: struct
//   typedef struct {
//      int index;
//      string dir;
//      double file_match_time_offset_sec;
//      double file_match_time_tolerance_sec;
//      double ray_match_elevation_tolerance_deg;
//      double ray_match_azimuth_tolerance_deg;
//      double ray_match_time_tolerance_sec;
//   }
//
// 1D array - variable length.
//

input_datasets = {
  {
    index = 1,
    dir = "$(HOME)/data/CmdVerify/cfradial/weather",
    file_match_time_offset_sec = 0,
    file_match_time_tolerance_sec = 60,
    ray_match_elevation_tolerance_deg = 0.25,
    ray_match_azimuth_tolerance_deg = 0.1,
    ray_match_time_tolerance_sec = 60
  }
  ,
  {
    index = 2,
    dir = "$(HOME)/data/CmdVerify/cfradial/clutter",
    file_match_time_offset_sec = 0,
    file_match_time_tolerance_sec = 60,
    ray_match_elevation_tolerance_deg = 0.25,
    ray_match_azimuth_tolerance_deg = 0.1,
    ray_match_time_tolerance_sec = 60
  }
  ,
  {
    index = 3,
    dir = "$(HOME)/data/CmdVerify/cfradial/combined",
    file_match_time_offset_sec = 0,
    file_match_time_tolerance_sec = 60,
    ray_match_elevation_tolerance_deg = 0.25,
    ray_match_azimuth_tolerance_deg = 0.1,
    ray_match_time_tolerance_sec = 60
  }
};

//======================================================================
//
// SPECIFYING FIELDS TO COPY FROM EACH SOURCE.
//
//======================================================================
 
///////////// output_fields ///////////////////////////
//
// Specs of fields to be copied from input to output.
// input_index: indicates which input_dir to use.
// input_field_name: name of field in input file.
// output_field_name: name of field in output file.
// encoding: output encoding for the field.
//
// Type: struct
//   typedef struct {
//      int input_index;
//      string input_field_name;
//      string output_field_name;
//      output_encoding_t output_encoding;
//        Options:
//          ENCODING_FLOAT32
//          ENCODING_INT32
//          ENCODING_INT16
//          ENCODING_INT08
//   }
//
// 1D array - variable length.
//

output_fields = {
  {
    input_index = 1,
    input_field_name = "DBZ",
    output_field_name = "DBZ_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "VEL",
    output_field_name = "VEL_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "WIDTH",
    output_field_name = "WIDTH_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "ZDR",
    output_field_name = "ZDR_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "RHOHV",
    output_field_name = "RHOHV_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "PHIDP",
    output_field_name = "PHIDP_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "SNRHC",
    output_field_name = "SNRHC_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "SNRVC",
    output_field_name = "SNRVC_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "DBMHC",
    output_field_name = "DBMHC_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "DBMVC",
    output_field_name = "DBMVC_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "NCP",
    output_field_name = "NCP_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "CPA",
    output_field_name = "CPA_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "TDBZ",
    output_field_name = "TDBZ_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "SPIN",
    output_field_name = "SPIN_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "ZDR_SDEV",
    output_field_name = "ZDR_SDEV_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "PHIDP_SDEV",
    output_field_name = "PHIDP_SDEV_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "PHIDP_SDEV_4KDP",
    output_field_name = "PHIDP_SDEV_4KDP_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "PHIDP_SDEV_NEW",
    output_field_name = "PHIDP_SDEV_NEW_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "CMD",
    output_field_name = "CMD_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "CMD_FLAG",
    output_field_name = "CMD_FLAG_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 1,
    input_field_name = "CLUT",
    output_field_name = "CLUT_WX",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "DBZ",
    output_field_name = "DBZ_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "VEL",
    output_field_name = "VEL_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "WIDTH",
    output_field_name = "WIDTH_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "ZDR",
    output_field_name = "ZDR_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "RHOHV",
    output_field_name = "RHOHV_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "PHIDP",
    output_field_name = "PHIDP_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "SNRHC",
    output_field_name = "SNRHC_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "SNRVC",
    output_field_name = "SNRVC_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "DBMHC",
    output_field_name = "DBMHC_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "DBMVC",
    output_field_name = "DBMVC_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "NCP",
    output_field_name = "NCP_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "CPA",
    output_field_name = "CPA_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "TDBZ",
    output_field_name = "TDBZ_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "SPIN",
    output_field_name = "SPIN_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "ZDR_SDEV",
    output_field_name = "ZDR_SDEV_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "PHIDP_SDEV",
    output_field_name = "PHIDP_SDEV_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "PHIDP_SDEV_4KDP",
    output_field_name = "PHIDP_SDEV_4KDP_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "PHIDP_SDEV_NEW",
    output_field_name = "PHIDP_SDEV_NEW_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "CMD",
    output_field_name = "CMD_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "CMD_FLAG",
    output_field_name = "CMD_FLAG_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 2,
    input_field_name = "CLUT",
    output_field_name = "CLUT_CLUT",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "DBZ",
    output_field_name = "DBZ_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "DBZ_F",
    output_field_name = "DBZ_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "VEL",
    output_field_name = "VEL_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "VEL_F",
    output_field_name = "VEL_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "WIDTH",
    output_field_name = "WIDTH_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "WIDTH_F",
    output_field_name = "WIDTH_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "ZDR",
    output_field_name = "ZDR_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "ZDR_F",
    output_field_name = "ZDR_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "RHOHV",
    output_field_name = "RHOHV_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "RHOHV_F",
    output_field_name = "RHOHV_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "PHIDP",
    output_field_name = "PHIDP_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "PHIDP_F",
    output_field_name = "PHIDP_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "SNRHC",
    output_field_name = "SNRHC_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "SNRHC_F",
    output_field_name = "SNRHC_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "SNRVC",
    output_field_name = "SNRVC_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "SNRVC_F",
    output_field_name = "SNRVC_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "DBMHC",
    output_field_name = "DBMHC_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "DBMHC_F",
    output_field_name = "DBMHC_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "DBMVC",
    output_field_name = "DBMVC_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "DBMVC_F",
    output_field_name = "DBMVC_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "NCP",
    output_field_name = "NCP_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "NCP_F",
    output_field_name = "NCP_F_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "CPA",
    output_field_name = "CPA_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "TDBZ",
    output_field_name = "TDBZ_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "SPIN",
    output_field_name = "SPIN_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "ZDR_SDEV",
    output_field_name = "ZDR_SDEV_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "PHIDP_SDEV",
    output_field_name = "PHIDP_SDEV_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "PHIDP_SDEV_4KDP",
    output_field_name = "PHIDP_SDEV_4KDP_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "PHIDP_SDEV_NEW",
    output_field_name = "PHIDP_SDEV_NEW_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "CMD",
    output_field_name = "CMD_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "CMD_FLAG",
    output_field_name = "CMD_FLAG_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "CLUT",
    output_field_name = "CLUT_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "TDBZ_INT",
    output_field_name = "TDBZ_INT_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "SPIN_INT",
    output_field_name = "SPIN_INT_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "CPA_INT",
    output_field_name = "CPA_INT_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "ZDR_SDEV_INT",
    output_field_name = "ZDR_SDEV_INT_COMB",
    output_encoding = ENCODING_FLOAT32
  }
  ,
  {
    input_index = 3,
    input_field_name = "PHIDP_SDEV_INT",
    output_field_name = "PHIDP_SDEV_INT_COMB",
    output_encoding = ENCODING_FLOAT32
  }
};

//======================================================================
//
// SPECIFYING FIELDS TO BE COMBINED.
//
//======================================================================
 
///////////// add_combined_fields /////////////////////
//
// Option to combine some fields to produce derived fields.
// We can combine pairs of fields using simple algorithms. This created 
//   derived fields. The fields to be combined must already have been 
//   specified as 'output_fields' above.
// Type: boolean
//

add_combined_fields = TRUE;

///////////// combined_fields /////////////////////////
//
// Specs of fields to be combined.
// field_name_1 and field_name_2 must be in the output_fields list. If 
//   'require_both' is TRUE, both fields needs to be non-missing to 
//   produce an output field. output_encoding - see 'output_fields' above. 
//   MEAN: arimthmetic mean. UNBIASED_MEAN: first remove any consistent 
//   bias between field1 and field2, by adjusting field2 to field 1. Then 
//   compute the arithmetic mean. This reduces noise in the fields without 
//   changing the value of field1. GEOM_MEAN: geometric mean. MAX: maximum 
//   of the two fields. MIN: minimum of the two fields.
//
// Type: struct
//   typedef struct {
//      string field_name_1;
//      string field_name_2;
//      string combined_name;
//      string long_name;
//      combine_method_t combine_method;
//        Options:
//          COMBINE_MEAN
//          COMBINE_UNBIASED_MEAN
//          COMBINE_GEOM_MEAN
//          COMBINE_MAX
//          COMBINE_MIN
//          COMBINE_SUM
//          COMBINE_DIFF
//      boolean require_both;
//      output_encoding_t output_encoding;
//        Options:
//          ENCODING_FLOAT32
//          ENCODING_INT32
//          ENCODING_INT16
//          ENCODING_INT08
//   }
//
// 1D array - variable length.
//

combined_fields = {
  {
    field_name_1 = "DBMHC_CLUT",
    field_name_2 = "DBMHC_WX",
    combined_name = "CSR",
    long_name = "clutter_to_signal_ratio",
    combine_method = COMBINE_DIFF,
    require_both = TRUE,
    output_encoding = ENCODING_FLOAT32
  }
};

//======================================================================
//
// OPTIONAL FIXED ANGLE OR SWEEP NUMBER LIMITS.
//
// Fixed angles are elevation in PPI mode and azimuth in RHI mode.
//
//======================================================================
 
///////////// set_fixed_angle_limits //////////////////
//
// Option to set fixed angle limits.
// Only use sweeps within the specified fixed angle limits.
// Type: boolean
//

set_fixed_angle_limits = FALSE;

///////////// lower_fixed_angle_limit /////////////////
//
// Lower fixed angle limit - degrees.
// Type: double
//

lower_fixed_angle_limit = 0;

///////////// upper_fixed_angle_limit /////////////////
//
// Upper fixed angle limit - degrees.
// Type: double
//

upper_fixed_angle_limit = 90;

///////////// set_sweep_num_limits ////////////////////
//
// Option to set sweep number limits.
// Only read sweeps within the specified sweep number limits.
// Type: boolean
//

set_sweep_num_limits = FALSE;

///////////// lower_sweep_num /////////////////////////
//
// Lower sweep number limit.
// Type: int
//

lower_sweep_num = 0;

///////////// upper_sweep_num /////////////////////////
//
// Upper sweep number limit.
// Type: int
//

upper_sweep_num = 0;

//======================================================================
//
// OPTION TO CHECK FOR CONSTANT GEOMETRY.
//
//======================================================================
 
///////////// check_constant_geometry /////////////////
//
// Option to check that the ray geometry does not change.
// If true, will only merge rays with the same geometry as the primary 
//   volume. If false, secondary rays will be remapped to the primary ray 
//   geometry.
// Type: boolean
//

check_constant_geometry = FALSE;

//======================================================================
//
// SWEEP FILE AGGREGATION.
//
//======================================================================
 
///////////// aggregate_sweep_files_on_read ///////////
//
// Option to aggregate sweep files into a volume on read.
// If false, and the input data is in sweeps rather than volumes (e.g. 
//   DORADE), the sweep files from a volume will be aggregated into a 
//   volume.
// Type: boolean
//

aggregate_sweep_files_on_read = TRUE;

//======================================================================
//
// OUTPUT FORMAT.
//
//======================================================================
 
///////////// output_format ///////////////////////////
//
// Format for the output files.
//
// Type: enum
// Options:
//     OUTPUT_FORMAT_CFRADIAL
//     OUTPUT_FORMAT_DORADE
//     OUTPUT_FORMAT_FORAY
//     OUTPUT_FORMAT_NEXRAD
//     OUTPUT_FORMAT_UF
//     OUTPUT_FORMAT_MDV_RADIAL
//

output_format = OUTPUT_FORMAT_CFRADIAL;

///////////// netcdf_style ////////////////////////////
//
// NetCDF style - if output_format is CFRADIAL.
// netCDF classic format, netCDF 64-bit offset format, netCDF4 using 
//   HDF5 format, netCDF4 using HDF5 format but only netCDF3 calls.
//
// Type: enum
// Options:
//     CLASSIC
//     NC64BIT
//     NETCDF4
//     NETCDF4_CLASSIC
//

netcdf_style = NETCDF4;

//======================================================================
//
// OUTPUT BYTE-SWAPPING and COMPRESSION.
//
//======================================================================
 
///////////// output_native_byte_order ////////////////
//
// Option to leave data in native byte order.
// If false, data will be byte-swapped as appropriate on output.
// Type: boolean
//

output_native_byte_order = FALSE;

///////////// output_compressed ///////////////////////
//
// Option to compress data fields on output.
// Applies to netCDF and Dorade. UF does not support compression.
// Type: boolean
//

output_compressed = TRUE;

//======================================================================
//
// OUTPUT OPTIONS FOR CfRadial FILES.
//
//======================================================================
 
///////////// output_force_ngates_vary ////////////////
//
// Option to force the use of ragged arrays for CfRadial files.
// Only applies to CfRadial. If true, forces the use of ragged arrays 
//   even if the number of gates for all rays is constant.
// Type: boolean
//

output_force_ngates_vary = TRUE;

///////////// compression_level ///////////////////////
//
// Compression level for output, if compressed.
// Applies to netCDF only. Dorade compression is run-length encoding, 
//   and has not options..
// Type: int
//

compression_level = 4;

//======================================================================
//
// OUTPUT DIRECTORY AND FILE NAME.
//
//======================================================================
 
///////////// output_dir //////////////////////////////
//
// Output directory path.
// Files will be written to this directory.
// Type: string
//

output_dir = "$(HOME)/data/CmdVerify/cfradial/merged";

///////////// output_filename_mode ////////////////////
//
// Mode for computing output file name.
// START_AND_END_TIMES: include both start and end times in file name. 
//   START_TIME_ONLY: include only start time in file name. END_TIME_ONLY: 
//   include only end time in file name.
//
// Type: enum
// Options:
//     START_AND_END_TIMES
//     START_TIME_ONLY
//     END_TIME_ONLY
//     SPECIFY_FILE_NAME
//

output_filename_mode = START_AND_END_TIMES;

///////////// output_filename /////////////////////////
//
// Name of output file.
// Applies only if output_filename_mode is SPECIFY_FILE_NAME. File of 
//   this name will be written to output_dir.
// Type: string
//

output_filename = "cfradial.test.nc";

///////////// append_day_dir_to_output_dir ////////////
//
// Add the day directory to the output directory.
// Path will be output_dir/yyyymmdd/filename.
// Type: boolean
//

append_day_dir_to_output_dir = TRUE;

///////////// append_year_dir_to_output_dir ///////////
//
// Add the year directory to the output directory.
// Path will be output_dir/yyyy/yyyymmdd/filename.
// Type: boolean
//

append_year_dir_to_output_dir = FALSE;

//======================================================================
//
// OPTION TO OVERRIDE MISSING VALUES.
//
// Missing values are applicable to both metadata and field data. The 
//   default values should be satisfactory for most purposes. However, you 
//   can choose to override these if you are careful with the selected 
//   values.

// The default values for metadata are:
// 	missingMetaDouble = -9999.0
// 	missingMetaFloat = -9999.0
// 	missingMetaInt = -9999
// 	missingMetaChar = -128

// The default values for field data are:
// 	missingFl64 = -9.0e33
// 	missingFl32 = -9.0e33
// 	missingSi32 = -2147483647
// 	missingSi16 = -32768
// 	missingSi08 = -128.
//
//======================================================================
 
///////////// override_missing_metadata_values ////////
//
// Option to override the missing values for meta-data.
// See following parameter options.
// Type: boolean
//

override_missing_metadata_values = FALSE;

///////////// missing_metadata_double /////////////////
//
// Missing value for metadata of type double.
// Only applies if override_missing_metadata_values is TRUE.
// Type: double
//

missing_metadata_double = -9999;

///////////// missing_metadata_float //////////////////
//
// Missing value for metadata of type float.
// Only applies if override_missing_metadata_values is TRUE.
// Type: float
//

missing_metadata_float = -9999;

///////////// missing_metadata_int ////////////////////
//
// Missing value for metadata of type int.
// Only applies if override_missing_metadata_values is TRUE.
// Type: int
//

missing_metadata_int = -9999;

///////////// missing_metadata_char ///////////////////
//
// Missing value for metadata of type char.
// Only applies if override_missing_metadata_values is TRUE.
// Type: int
//

missing_metadata_char = -128;

///////////// override_missing_field_values ///////////
//
// Option to override the missing values for field data.
// See following parameter options.
// Type: boolean
//

override_missing_field_values = FALSE;

///////////// missing_field_fl64 //////////////////////
//
// Missing value for field data of type 64-bit float.
// Only applies if override_missing_field_values is TRUE.
// Type: double
//

missing_field_fl64 = -9e+33;

///////////// missing_field_fl32 //////////////////////
//
// Missing value for field data of type 32-bit float.
// Only applies if override_missing_field_values is TRUE.
// Type: double
//

missing_field_fl32 = -9e+33;

///////////// missing_field_si32 //////////////////////
//
// Missing value for field data of type 32-bit integer.
// Only applies if override_missing_field_values is TRUE.
// Type: int
//

missing_field_si32 = -2147483647;

///////////// missing_field_si16 //////////////////////
//
// Missing value for field data of type 16-bit integer.
// Only applies if override_missing_field_values is TRUE.
// Type: int
//

missing_field_si16 = -232768;

///////////// missing_field_si08 //////////////////////
//
// Missing value for field data of type 8-bit integer.
// Only applies if override_missing_field_values is TRUE.
// Type: int
//

missing_field_si08 = -128;

