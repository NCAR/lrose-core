/**********************************************************************
 * TDRP params for Params
 **********************************************************************/

//======================================================================
//
// Colide.
//
// This program does the COlide algorithm.
//
//======================================================================
 
///////////// fixed_const /////////////////////////////
//
// Fixed constant strings that are interpreted by user functions.
//
//
// Type: string
// 1D array - variable length.
//

fixed_const = {
};

///////////// user_data ///////////////////////////////
//
// Non gridded data names.
//
//
// Type: string
// 1D array - variable length.
//

user_data = {
};

///////////// volume_before_filter ////////////////////
//
// Volume Filters to apply before the sweep filters.
//
//
// Type: string
// 1D array - variable length.
//

volume_before_filter = {
};

///////////// filter //////////////////////////////////
//
// Filters (sweep) to apply.
//
//
// Type: string
// 1D array - variable length.
//

filter = {
 "line_dir = LineDir($(DBZ_NAME), 12, 5, 1)",
 "line_det = LineDet($(DBZ_NAME), line_dir, 12, 5, 1)",
 "shear_dir = ShearDir($(VEL_NAME), line_det, 12, 5, 30)",
 "shear_det = ShearDet($(VEL_NAME), shear_dir, line_det, 12, 5, 30)",
 "raw_comb = maximum(line_det, shear_det)",
 "line_ellip_orient = EllipOrient(line_det, 12, 2, 30)",
 "line_ellip_orient2 = median(line_ellip_orient, 5, 5, 0, 180, 5)",
 "line_ellip = Ellip(line_det, line_ellip_orient, 12, 2, 30)",
 "line_ellip_conf = EllipConf(line_det, line_ellip_orient, 12, 2, 30)",
 "shear_ellip_orient = EllipOrient(shear_det, 12, 2, 30)",
 "shear_ellip_orient2 = median(shear_ellip_orient, 5, 5, 0, 180, 5)",
 "shear_ellip = Ellip(shear_det, shear_ellip_orient, 12, 2, 30)",
 "shear_ellip_conf = EllipConf(shear_det, shear_ellip_orient, 12, 2, 30)",
 "ellip_comb = weighted_average(0, line_ellip, 0.25, shear_ellip, 0.75)",
 "ellip_comb_dilate = max_expand(ellip_comb, 1, 1)",
 "ellip_orient_comb = weighted_angle_average(0, line_ellip_orient, 0.5, shear_ellip_orient, 0.5)",
 "enhanced0 = Enhance(raw_comb, 20, 16, 18, 30)",
 "enhanced0_orient = EnhanceDir(raw_comb, 20, 16, 18, 30)",
 "enhanced1 = Enhance(raw_comb, 25, 14, 18, 30)",
 "enhanced1_orient = EnhanceDir(raw_comb, 25, 14, 18, 30)",
 "enhanced_out = weighted_average(0, enhanced0, 0.5, raw_comb, 0.5)",
 "ellip_orient = EllipOrient(enhanced0, 12, 2, 30)",
 "ellip = Ellip(enhanced0, ellip_orient, 12, 2, 30)",
 "ellip_conf = EllipConf(enhanced0, ellip_orient, 12, 2, 30)",
 "dilate = max_expand(ellip, 2, 2)",
 "line_hotspot = 0",
 "line_medium = 0",
 "line_full = 0",
 "if (line_ellip >= 0.55) then line_hotspot = 1",
 "if (line_ellip >= 0.45) then line_medium = 1",
 "if (line_ellip >= 0.40) then line_full = 1",
 "shear_hotspot = 0",
 "shear_medium = 0",
 "shear_full = 0",
 "if (shear_ellip >= 0.5) then shear_hotspot = 1",
 "if (shear_ellip >= 0.4) then shear_medium = 1",
 "if (shear_ellip >= 0.35) then shear_full = 1",
 "hot = 0",
 "medium = 0",
 "full = 0",
 "if (ellip_comb_dilate >= 0.55) then hot = 1",
 "if (ellip_comb_dilate >= 0.42) then medium = 1",
 "if (ellip_comb_dilate >= 0.38) then full = 1",
 "line_reg = Region(line_hotspot,line_medium,line_full,10,20,0,80,20)",
 "shear_reg=Region(shear_hotspot,shear_medium,shear_full,10,20,0,80,20)",
 "reg=Region(hot,medium,full,10,20,0,80,20)",
 "eocm = median(ellip_orient_comb, 5, 5, 0, 180, 10)",
 "omask = mask_missing_to_missing(eocm, reg)",
 "ellip_orient_expand = expand_angles_laterally(omask, 15)",
 "regComb = RegComb(reg, eocm, 15, 30, 5)",
 "ecf = ellip_comb",
 "if (ecf < 0.4) then ecf = missing",
 "ellipD = max_expand(ecf, 5, 5)",
 "ellipHist = History(ellipD, 300, 0.9, 600, 0.7, 1000, 0.5, 2000, 0.2)",
 "ellipMaxAgeHist = MaxAgeMinutes(ellipD)",
 "eof = enhanced_out",
 "if (eof < 0.3) then eof = missing",
 "expandEof = max_expand(eof, 5, 5)",
 "enhHist = History(expandEof, 300, 0.9, 600, 0.7, 1000, 0.5, 2000, 0.2)",
 "enhMaxAgeHist = MaxAgeMinutes(expandEof)"
};

///////////// volume_after_filter /////////////////////
//
// Volume Filters to apply after the sweep filters.
//
//
// Type: string
// 1D array - variable length.
//

volume_after_filter = {
};

///////////// line_detect_side_fuzzy //////////////////
//
// Line detection side fuzzy function.
//
//
// Type: struct
//   typedef struct {
//      double x;
//      double y;
//   }
//
// 1D array - variable length.
//

line_detect_side_fuzzy = {
  {
    x = 0.75,
    y = -1
  }
  ,
  {
    x = 5.25,
    y = 1
  }
};

///////////// line_detect_center_fuzzy ////////////////
//
// Line detection center fuzzy function.
//
//
// Type: struct
//   typedef struct {
//      double x;
//      double y;
//   }
//
// 1D array - variable length.
//

line_detect_center_fuzzy = {
  {
    x = -10,
    y = 0
  }
  ,
  {
    x = 6,
    y = 1
  }
  ,
  {
    x = 15,
    y = 1
  }
  ,
  {
    x = 30,
    y = 0
  }
};

///////////// line_detect_std_fuzzy ///////////////////
//
// Line detection std fuzzy function.
//
//
// Type: struct
//   typedef struct {
//      double x;
//      double y;
//   }
//
// 1D array - variable length.
//

line_detect_std_fuzzy = {
  {
    x = 2,
    y = 1.25
  }
  ,
  {
    x = 70,
    y = 0
  }
};

///////////// shear_detect_side_fuzzy /////////////////
//
// Shear detection side fuzzy function.
//
//
// Type: struct
//   typedef struct {
//      double x;
//      double y;
//   }
//
// 1D array - variable length.
//

shear_detect_side_fuzzy = {
  {
    x = -2.5,
    y = -2
  }
  ,
  {
    x = -2,
    y = -1
  }
  ,
  {
    x = 3.5,
    y = 1
  }
};

///////////// ellip_conf_fuzzy ////////////////////////
//
// Elliptical filter confidence fuzzy function.
//
//
// Type: struct
//   typedef struct {
//      double x;
//      double y;
//   }
//
// 1D array - variable length.
//

ellip_conf_fuzzy = {
  {
    x = 0,
    y = 1
  }
  ,
  {
    x = 0.5,
    y = 0.9
  }
  ,
  {
    x = 1,
    y = 0.1
  }
};

///////////// enhance_fuzzy ///////////////////////////
//
// Enhance filter fuzzy function.
//
//
// Type: struct
//   typedef struct {
//      double x;
//      double y;
//   }
//
// 1D array - variable length.
//

enhance_fuzzy = {
  {
    x = 0.15,
    y = 0
  }
  ,
  {
    x = 0.5,
    y = 1
  }
};

///////////// old_data ////////////////////////////////
//
// Fields for which we want to get and use older data.
//
// BOOST.
//
//
// Type: struct
//   typedef struct {
//      string fieldName;
//      int maxSecondsBack;
//   }
//
// 1D array - variable length.
//

old_data = {
  {
    fieldName = "ellipD",
    maxSecondsBack = 3600
  }
  ,
  {
    fieldName = "expandEof",
    maxSecondsBack = 3600
  }
};

/**********************************************************************
 * TDRP params for AlgorithmParams
 **********************************************************************/

//======================================================================
//
// Algorithm Parameters.
//
// Parameters used to run the Algorithm class.
//
//======================================================================
 
///////////// instance ////////////////////////////////
//
// instance.
//
// for PMU.
//
//
// Type: string
//

instance = "$(INSTANCE)";

///////////// debug_mode //////////////////////////////
//
// debugging mode.
//
// NONE=as quiet as possible, DEBUG = minimal debug output, 
//   DEBUG_VERBOSE=more.
//
//
// Type: enum
// Options:
//     NONE
//     DEBUG
//     DEBUG_VERBOSE
//

debug_mode = DEBUG;

///////////// debug_threads ///////////////////////////
//
// threads debug.
//
// set to TRUE to debug threading.
//
//
// Type: boolean
//

debug_threads = FALSE;

///////////// num_threads /////////////////////////////
//
// Number of threads.
//
// Threading is done on sweeps. (More than one sweep processed by 
//   individual threads).  Set to 1 or 0 for no threading.
//
//
// Type: int
//

num_threads = 3;

///////////// input ///////////////////////////////////
//
// Inputs to the algorithm.
//
// Names of the fields that are inputs to the algorithm.
//
//
// Type: string
// 1D array - variable length.
//

// input = {
//  "$(DBZ_NAME)",
//  "$(VEL_NAME)"
// };

///////////// output //////////////////////////////////
//
// Fields/values to export from the algorithm.
//
//
// Type: string
// 1D array - variable length.
//

// output = {
//  "line_det",
//  "line_dir",
//  "shear_det",
//  "shear_dir",
//  "line_ellip",
//  "line_ellip_conf",
//  "line_ellip_orient",
//  "line_ellip_orient2",
//  "shear_ellip",
//  "shear_ellip_conf",
//  "shear_ellip_orient",
//  "shear_ellip_orient2",
//  "ellip_comb",
//  "ellip_orient_comb",
//  "raw_comb",
//  "enhanced0",
//  "enhanced1",
//  "enhanced0_orient",
//  "enhanced1_orient",
//  "enhanced_out",
//  "ellip_orient",
//  "ellip",
//  "ellip_conf",
//  "dilate",
//  "line_hotspot",
//  "line_full",
//  "shear_hotspot",
//  "shear_full",
//  "line_reg",
//  "shear_reg",
//  "reg",
//  "ellip_orient_expand"
// };

/**********************************************************************
 * TDRP params for VirtVolParams
 **********************************************************************/

//======================================================================
//
// VirtVol Parameters.
//
// Triggering new data, reading in new data, writing data.
//
//======================================================================
 
///////////// debug_triggering ////////////////////////
//
// triggering debug.
//
// set to TRUE to debug triggering.
//
//
// Type: boolean
//

debug_triggering = FALSE;

///////////// trigger_url /////////////////////////////
//
// trigger url.
//
// URL to trigger off.
//
//
// Type: string
//

trigger_url = "mdvp:://$(RADAR_HOST)::front_porch/mdv/radar/$(RADAR)/ppi500m";

///////////// restrict_vertical_levels ////////////////
//
// Set TRUE to restrict vertical levels to a range of values, FALSE to 
//   use all the vertical levels.
//
//
// Type: boolean
//

restrict_vertical_levels = TRUE;

///////////// vertical_level_range ////////////////////
//
// If restrict_vertical_levels = TRUE, this is the range of vlevels.
//
// If set to -1, -1 it is not used, and there is no restriction.
//
//
// Type: double
// 1D array - fixed length - 2 elements.
//

vertical_level_range = {
 0.3,
 1.5
};

///////////// restrict_max_range //////////////////////
//
// Set TRUE to restrict maximum range to a parameter, FALSE to use all
//   the data.
//
//
// Type: boolean
//

restrict_max_range = FALSE;

///////////// max_range ///////////////////////////////
//
// If restrict_max_range = TRUE, this is the max range index.
//
// Not used when restrict_max_range = FALSE, or if the value is >=0.
//
//
// Type: double
//

max_range = -1;

///////////// input_url ///////////////////////////////
//
// names of param files for input URLS, one per URL.
//
// The param files are read and parsed.  The names should match filter
//   descriptions.
//
//
// Type: string
// 1D array - variable length.
//

input_url = {
  "UrlInput.parms"
};

///////////// output_url //////////////////////////////
//
// names of param files for output URLS, one per URL.
//
// The param files are read and parsed.  The names should match filter
//   descriptions.
//
//
// Type: string
// 1D array - variable length.
//

output_url = {
  "UrlOutput.parms"
};


///////////// virtvol_input ///////////////////////////
//
// inputs.
//
// The fields read in externally
// internal_name = name used in filtering
// external_name = external field name.
//
//
// Type: struct
//   typedef struct {
//      string internal_name;
//      string external_name;
//      string url;
//      Url_t url_type;
//        Options:
//          VIRTUAL_VOLUME
//          DATABASE
//          ASCII
//      Data_t data_type;
//        Options:
//          GRID
//          VALUE
//          NOT_SET
//   }
//
// 1D array - variable length.
//

virtvol_input = {
  {
    internal_name = "$(DBZ_NAME)",
    external_name = "$(DBZ_NAME)",
    url = "mdvp:://$(RADAR_HOST):0:front_porch/mdv/radar/$(RADAR)/ppi500m",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "$(VEL_NAME)",
    external_name = "$(VEL_NAME)",
    url = "mdvp:://$(RADAR_HOST):0:front_porch/mdv/radar/$(RADAR)/ppi500m",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
};

///////////// virtvol_output //////////////////////////
//
// Fields/values to write out.
//
//
// Type: struct
//   typedef struct {
//      string internal_name;
//      string external_name;
//      string url;
//      Url_t url_type;
//        Options:
//          VIRTUAL_VOLUME
//          DATABASE
//          ASCII
//      Data_t data_type;
//        Options:
//          GRID
//          VALUE
//          NOT_SET
//   }
//
// 1D array - variable length.
//

virtvol_output = {
  {
    internal_name = "line_det",
    external_name = "line_det",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_dir",
    external_name = "line_dir",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_det",
    external_name = "shear_det",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_dir",
    external_name = "shear_dir",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_ellip",
    external_name = "line_ellip",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_ellip_orient",
    external_name = "line_ellip_orient",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_ellip_orient2",
    external_name = "line_ellip_orient2",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_ellip_conf",
    external_name = "line_ellip_conf",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_ellip",
    external_name = "shear_ellip",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_ellip_orient",
    external_name = "shear_ellip_orient",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_ellip_orient2",
    external_name = "shear_ellip_orient2",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_ellip_conf",
    external_name = "shear_ellip_conf",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellip_comb",
    external_name = "ellip_comb",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellip_orient_comb",
    external_name = "ellip_orient_comb",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "raw_comb",
    external_name = "raw_comb",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "enhanced0",
    external_name = "enhanced0",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "enhanced1",
    external_name = "enhanced1",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "enhanced0_orient",
    external_name = "enhanced0_orient",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "enhanced1_orient",
    external_name = "enhanced1_orient",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "enhanced_out",
    external_name = "enhanced_out",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellip",
    external_name = "ellip",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellip_orient",
    external_name = "ellip_orient",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellip_conf",
    external_name = "ellip_conf",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "dilate",
    external_name = "dilate",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_hotspot",
    external_name = "line_hotspot",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_full",
    external_name = "line_full",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_hotspot",
    external_name = "shear_hotspot",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_full",
    external_name = "shear_full",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "hot",
    external_name = "hot",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "full",
    external_name = "full",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "shear_reg",
    external_name = "shear_reg",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "line_reg",
    external_name = "line_reg",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "reg",
    external_name = "reg",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellip_orient_expand",
    external_name = "ellip_orient_expand",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "regComb",
    external_name = "regComb",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellipHist",
    external_name = "ellipHist",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellipMaxAgeHist",
    external_name = "ellipMaxAgeHist",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "ellipD",
    external_name = "ellipD",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "enhHist",
    external_name = "enhHist",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "enhMaxAgeHist",
    external_name = "enhMaxAgeHist",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
  ,
  {
    internal_name = "expandEof",
    external_name = "expandEof",
    url = "mdvp:://localhost::$(COLIDE_DATA_DIR)/front_porch/mdv/radar/$(RADAR)/$(COLIDE)",
    url_type = VIRTUAL_VOLUME,
    data_type = GRID
  }
};

