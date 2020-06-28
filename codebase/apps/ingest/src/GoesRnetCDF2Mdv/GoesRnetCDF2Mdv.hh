// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 *
 * @file GoesRnetCDF2Mdv.hh
 *
 * @class GoesRnetCDF2Mdv
 *
 * GoesRnetCDF2Mdv is the top level application class.
 *  
 * @date 1/30/2012
 *
 */

#ifndef GoesRnetCDF2Mdv_HH
#define GoesRnetCDF2Mdv_HH

#include <dsdata/DsTrigger.hh>
#include <toolsa/DateTime.hh>
#include <euclid/SunAngle.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Ncxx/Ncxx.hh>

#include "Args.hh"
#include "Params.hh"


/** 
 * @class GoesRnetCDF2Mdv
 */

class GoesRnetCDF2Mdv
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Flag indicating whether the program status is currently okay.
   */

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Destructor
   */

  ~GoesRnetCDF2Mdv(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static GoesRnetCDF2Mdv *Inst(int argc, char **argv);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static GoesRnetCDF2Mdv *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /**
   * @brief Run the program.
   */

  void run();
  

 private:

  /////////////////////
  // Private types   //
  /////////////////////

  typedef enum {
    PRODUCT_LEVEL1B_RADIANCES,
    PRODUCT_CLOUD_AND_MOISTURE_IMAGERY,
    PRODUCT_AEROSOL_DETECTION,
    PRODUCT_AEROSOL_OPTICAL_DEPTH,
    PRODUCT_CLOUD_TOP_PHASE,
    PRODUCT_CLOUD_TOP_HEIGHT,
    PRODUCT_CLOUD_TOP_TEMPERATURE,
    PRODUCT_CLOUD_TOP_PRESSURE,
    PRODUCT_DERIVED_STABILITY_INDICES,
    PRODUCT_TOTAL_PRECIPITABLE_WATER,
    PRODUCT_CLEAR_SKY_MASK,
    PRODUCT_FIRE_CHARACTERIZATION,
    PRODUCT_LAND_SURFACE_TEMPERATURE,
    PRODUCT_CLOUD_OPTICAL_DEPTH,
    PRODUCT_CLOUD_PARTICLE_SIZE,
    PRODUCT_DERIVED_MOTION_WINDS,
    PRODUCT_GLOBAL_LIGHTNING,
    PRODUCT_TYPE_UNKNOWN
  } product_type_t;
   
  typedef enum {
    SCAN_FULL_DISK,
    SCAN_CONUS,
    SCAN_MESOSCALE
  } scan_type_t;

  /////////////////////
  // Private classes //
  /////////////////////

  // string content of global attributes
  class GlobalAttributes {
  public:
    std::string ncProperties;
    std::string namingAuthority;
    std::string conventions;
    std::string metadataConventions;
    std::string standardNameVocabulary;
    std::string institution;
    std::string project;
    std::string productionSite;
    std::string productionEnvironment;
    std::string spatialResolution;
    std::string orbitalSlot;
    std::string platformID;
    std::string instrumentType;
    std::string sceneID;
    std::string instrumentID;
    std::string datasetName;
    std::string isoSeriesMetadataID;
    std::string title;
    std::string summary;
    std::string keywords;
    std::string keywordsVocabulary;
    std::string license;
    std::string processingLevel;
    std::string dateCreated;
    std::string cdmDataType;
    std::string timelineID;
    std::string timeCoverageStart;
    std::string timeCoverageEnd;
    std::string createdBy;
    std::string productionDataSource;
  };


  
  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static GoesRnetCDF2Mdv *_instance;
  
  /**
   * @brief string constant names of attributues, dimensions, and variables
   */

  // start of epoch for GOES-R is 2000-01-01 12:00:00, the J2000 epoch
  static const time_t J2000_EPOCH_START;

  static const float MISSING_DATA_VALUE;

  static const int EMISSIVE_BAND_START;
  
  // header string for logging messages
  static const char* ERROR_STR;
  static const char* WARN_STR;
  static const char* INFO_STR;
  
  // Level-1 and Level-2 product Dimensions
  static const char* X_DIM;
  static const char* Y_DIM;
  static const char* NUMBER_OF_TIME_BOUNDS;
  static const char* BAND_DIM;
  static const char* NUMBER_OF_IMAGE_BOUNDS;

  // Radiance specific Dimensions
  static const char* NUM_STAR_LOOKS;

  // Level-2 specific Dimensions
  static const char* NUMBER_OF_LZA_BOUNDS;
  static const char* NUMBER_OF_SZA_BOUNDS;
  static const char* NUMBER_OF_SUNGLINT_ANGLE_BOUNDS;
  static const char* LAND_SENSOR_BANDS;
  static const char* SEA_SENSOR_BANDS;
  static const char* LATITUDE_BANDS;
  static const char* NUMBER_OF_LATITUDE_BAND_BOUNDS;
  static const char* NUM_AREA_TYPES;
  static const char* MAX_AREA_TYPE_LEN;
  static const char* NUMBER_OF_LAT_BOUNDS;
  static const char* SOUNDING_EMISSIVE_BANDS;
  static const char* RTM_BT_COMPARISON_BANDS;
  
  // Level-1 and Level-2 product Global Attributes
  static const char* NC_PROPERTIES;
  static const char* NAMING_AUTHORITY;
  static const char* CONVENTIONS;
  static const char* METADATA_CONVENTIONS;
  static const char* STANDARD_NAME_VOCABULARY;
  static const char* INSTITUTION;
  static const char* PROJECT;
  static const char* PRODUCTION_SITE;
  static const char* PRODUCTION_ENVIRONMENT;
  static const char* SPATIAL_RESOLUTION;
  static const char* ORBITAL_SLOT;
  static const char* PLATFORM_ID;
  static const char* INSTRUMENT_TYPE;
  static const char* SCENE_ID;
  static const char* INSTRUMENT_ID;
  static const char* TITLE;
  static const char* SUMMARY;
  static const char* KEYWORDS;
  static const char* KEYWORDS_VOCABULARY;
  static const char* ISO_SERIES_METADATA_ID;
  static const char* LICENSE;
  static const char* PROCESSING_LEVEL;
  static const char* CDM_DATA_TYPE;
  static const char* DATASET_NAME;
  static const char* PRODUCTION_DATA_SOURCE;
  static const char* TIMELINE_ID;
  static const char* DATE_CREATED;
  static const char* TIME_COVERAGE_START;
  static const char* TIME_COVERAGE_END;
  static const char* CREATED_BY;
  

  // Level-1 and Level-2 Product Variables
  static const char* X_COORD;
  static const char* Y_COORD;
  static const char* DATA_QUALITY_FLAG;
  static const char* TIME;
  static const char* TIME_BOUNDS;
  static const char* GOES_IMAGER_PROJECTION;
  static const char* X_IMAGE;
  static const char* Y_IMAGE;
  static const char* X_IMAGE_BOUNDS;
  static const char* Y_IMAGE_BOUNDS;
  static const char* NOMINAL_SATELLITE_SUBPOINT_LAT;
  static const char* NOMINAL_SATELLITE_SUBPOINT_LON;
  static const char* NOMINAL_SATELLITE_HEIGHT;
  static const char* GEOSPATIAL_LAT_LON_EXTENT;
  static const char* ALGORITHM_DYNAMIC_INPUT_DATA_CONTAINER;
  static const char* PROCESSING_PARM_VERSION_CONTAINER;
  static const char* ALGORITHM_PRODUCT_VERSION_CONTAINER;
  static const char* PERCENT_UNCORRECTABLE_L0_ERRORS;

  // Level-1 Variables
  static const char* RADIANCE;
  static const char* CMI;
  static const char* YAW_FLIP_FLAG;
  static const char* BAND_ID;
  static const char* BAND_WAVELENGTH;
  static const char* ESUN;
  static const char* KAPPA0;
  static const char* PLANCK_FK1;
  static const char* PLANCK_FK2;
  static const char* PLANCK_BC1;
  static const char* PLANCK_BC2;
  static const char* VALID_PIXEL_COUNT;
  static const char* MISSING_PIXEL_COUNT;
  static const char* SATURATED_PIXEL_COUNT;
  static const char* UNDERSATURATED_PIXEL_COUNT;
  static const char* MIN_RADIANCE_VALUE_OF_VALID_PIXELS;
  static const char* MAX_RADIANCE_VALUE_OF_VALID_PIXELS;
  static const char* MEAN_RADIANCE_VALUE_OF_VALID_PIXELS;
  static const char* STD_DEV_RADIANCE_VALUE_OF_VALID_PIXELS;
  static const char* EARTH_SUN_DISTANCE_ANOMALY_IN_AU;
  static const char* T_STAR_LOOK;
  static const char* BAND_WAVELENGTH_STAR_LOOK;
  static const char* STAR_ID;
  
  // Level-2 Product Shared Variables
  static const char* RETRIEVAL_LOCAL_ZENITH_ANGLE;
  static const char* QUANTITATIVE_LOCAL_ZENITH_ANGLE;
  static const char* RETRIEVAL_LOCAL_ZENITH_ANGLE_BOUNDS;
  static const char* QUANTITATIVE_LOCAL_ZENITH_ANGLE_BOUNDS;
  static const char* RETRIEVAL_SOLAR_ZENITH_ANGLE;
  static const char* QUANTITATIVE_SOLAR_ZENITH_ANGLE;
  static const char* RETRIEVAL_SOLAR_ZENITH_ANGLE_BOUNDS;
  static const char* QUANTITATIVE_SOLAR_ZENITH_ANGLE_BOUNDS;
  static const char* SOLAR_ZENITH_ANGLE;
  static const char* SOLAR_ZENITH_ANGLE_BOUNDS;
  static const char* LOCAL_ZENITH_ANGLE;
  static const char* LOCAL_ZENITH_ANGLE_BOUNDS;
  static const char* TWILIGHT_SOLAR_ZENITH_ANGLE;
  static const char* TWILIGHT_SOLAR_ZENITH_ANGLE_BOUNDS;
  static const char* SUNGLINT_ANGLE;
  static const char* SUNGLINT_ANGLE_BOUNDS;
  static const char* DAY_RETRIEVAL_LOCAL_ZENITH_ANGLE;
  static const char* NIGHT_RETRIEVAL_LOCAL_ZENITH_ANGLE;
  static const char* DAY_RETRIEVAL_LOCAL_ZENITH_ANGLE_BOUNDS;
  static const char* NIGHT_RETRIEVAL_LOCAL_ZENITH_ANGLE_BOUNDS;
  static const char* DAY_SOLAR_ZENITH_ANGLE;
  static const char* NIGHT_SOLAR_ZENITH_ANGLE;
  static const char* DAY_ALGORITHM_SOLAR_ZENITH_ANGLE;
  static const char* NIGHT_ALGORITHM_SOLAR_ZENITH_ANGLE;
  static const char* DAY_SOLAR_ZENITH_ANGLE_BOUNDS;
  static const char* NIGHT_SOLAR_ZENITH_ANGLE_BOUNDS;
  static const char* DATA_QUALITY_FLAG_OVERALL;
  static const char* DATA_QUALITY_FLAG_RETRIEVAL;
  static const char* DATA_QUALITY_FLAG_SKINTEMP;
  static const char* PERCENT_UNCORRECTABLE_GRB_ERRORS;
  static const char* OUTLIER_PIXELS;
  static const char* CLOUD_PIXELS;
  static const char* OUTLIER_PIXEL_COUNT;
  static const char* DAY_CLOUD_PIXELS;
  static const char* NIGHT_CLOUD_PIXELS;
  static const char* PERCENT_DAY_PIXEL;
  static const char* PERCENT_NIGHT_PIXELS;
  static const char* PERCENT_TERMINATOR_PIXELS;
  static const char* LATITUDE;
  static const char* LATITUDE_BOUNDS;
  static const char* SOUNDING_EMISSIVE_WAVELENGTHS;
  static const char* SOUNDING_EMISSIVE_BAND_IDS;
  static const char* TOTAL_ATTEMPTED_RETRIEVALS;
  static const char* MEAN_OBS_MODELED_DIFF_SOUNDING_EMISSIVE_BANDS;
  static const char* STD_DEV_OBS_MODELED_DIFF_SOUNDING_EMISSIVE_BANDS;

  
  // Level-2 Aerosol Variables (A99) 
  static const char* AEROSOL;
  static const char* SMOKE;
  static const char* DUST;
  static const char* NUMBER_GOOD_LZA_PIXELS;
  static const char* NUMBER_GOOD_SZA_PIXELS;
  static const char* NUMBER_OF_GOOD_SMOKE_RETRIEVALS;
  static const char* NUMBER_OF_GOOD_DUST_RETRIEVALS;
  static const char* NUMBER_OF_GOOD_RETRIEVALS_WHERE_SMOKE_DETECTED;
  static const char* NUMBER_OF_GOOD_RETRIEVALS_WHERE_DUST_DETECTED;

  
  // Level-2 Aerosol Optical Depth File (B99)
  static const char* AEROSOL_OPTICAL_DEPTH;
  static const char* AOD_PRODUCT_WAVELENGTH;
  static const char* LAND_SENSOR_BAND_WAVELENGTHS;
  static const char* SEA_SENSOR_BAND_WAVELENGTHS;
  static const char* LAND_SENSOR_BAND_IDS;
  static const char* SEA_SENSOR_BAND_IDS;
  static const char* SNOW_FREE_LAND_AND_ICE_FREE_SEA;
  static const char* GOES_LAT_LON_PROJECTION;
  static const char* AOD550_RETRIEVALS_ATTEMPTED_LAND;
  static const char* AOD550_RETRIEVALS_ATTEMPTED_SEA;
  static const char* AOD550_GOOD_LZA_RETRIEVALS_ATTEMPTED;
  static const char* AOD550_OUTLIER_PIXEL_COUNT;
  static const char* MIN_AOD550_LAND;
  static const char* MAX_AOD550_LAND;
  static const char* MEAN_AOD550_LAND;
  static const char* STD_DEV_AOD550_LAND;
  static const char* MIN_AOD550_SEA;
  static const char* MAX_AOD550_SEA;
  static const char* MEAN_AOD550_SEA;
  static const char* STD_DEV_AOD550_SEA;
  static const char* SENSOR_BAND_MIN_AOD_LAND;
  static const char* SENSOR_BAND_MAX_AOD_LAND;
  static const char* SENSOR_BAND_MEAN_AOD_LAND;
  static const char* SENSOR_BAND_STD_DEV_AOD_LAND;
  static const char* SENSOR_BAND_MIN_AOD_SEA;
  static const char* SENSOR_BAND_MAX_AOD_SEA;
  static const char* SENSOR_BAND_MEAN_AOD_SEA;
  static const char* SENSOR_BAND_STD_DEV_AOD_SEA;
  static const char* LAT_BAND_AOD550_RETRIEVALS_ATTEMPTED_LAND;
  static const char* LAT_BAND_AOD550_RETRIEVALS_ATTEMPTED_SEA;
  static const char* LAT_BAND_AOD550_GOOD_LZA_RETRIEVALS_ATTEMPTED_LAND;
  static const char* LAT_BAND_AOD550_GOOD_LZA_RETRIEVALS_ATTEMPTED_SEA;
  static const char* LAT_BAND_AOD550_PERCENT_GOOD_RETRIEVALS_LAND;
  static const char* LAT_BAND_AOD550_PERCENT_BAD_RETRIEVALS_LAND;
  static const char* LAT_BAND_AOD550_PERCENT_GOOD_RETRIEVALS_SEA;
  static const char* LAT_BAND_AOD550_PERCENT_BAD_RETRIEVALS_SEA;
  static const char* LAT_BAND_MIN_AOD550_LAND;
  static const char* LAT_BAND_MAX_AOD550_LAND;
  static const char* LAT_BAND_MEAN_AOD550_LAND;
  static const char* LAT_BAND_STD_DEV_AOD550_LAND;
  static const char* LAT_BAND_MIN_AOD550_SEA;
  static const char* LAT_BAND_MAX_AOD550_SEA;
  static const char* LAT_BAND_MEAN_AOD550_SEA;
  static const char* LAT_BAND_STD_DEV_AOD550_SEA;
  static const char* LAT_BAND_SENSOR_BAND_MIN_AOD_LAND;
  static const char* LAT_BAND_SENSOR_BAND_MAX_AOD_LAND;
  static const char* LAT_BAND_SENSOR_BAND_MEAN_AOD_LAND;
  static const char* LAT_BAND_SENSOR_BAND_STD_DEV_AOD_LAND;
  static const char* LAT_BAND_SENSOR_BAND_MIN_AOD_SEA;
  static const char* LAT_BAND_SENSOR_BAND_MAX_AOD_SEA;
  static const char* LAT_BAND_SENSOR_BAND_MEAN_AOD_SEA;
  static const char* LAT_BAND_SENSOR_BAND_STD_DEV_AOD_SEA;
  static const char* LAT_BAND_SENSOR_BAND_MIN_SURFACE_REFLECTIVITY_LAND;
  static const char* LAT_BAND_SENSOR_BAND_MAX_SURFACE_REFLECTIVITY_LAND;
  static const char* LAT_BAND_SENSOR_BAND_MEAN_SURFACE_REFLECTIVITY_LAND;
  static const char* LAT_BAND_SENSOR_BAND_STD_DEV_SURFACE_REFLECTIVITY_LAND;
  static const char* LAT_BAND_SENSOR_BAND_MIN_SURFACE_REFLECTIVITY_SEA;
  static const char* LAT_BAND_SENSOR_BAND_MAX_SURFACE_REFLECTIVITY_SEA;
  static const char* LAT_BAND_SENSOR_BAND_MEAN_SURFACE_REFLECTIVITY_SEA;
  static const char* LAT_BAND_SENSOR_BAND_STD_DEV_SURFACE_REFLECTIVITY_SEA;


  // Level-2 Cloud Top Phase File (D99)
  static const char* CLOUD_TOP_PHASE;
  static const char* TOTAL_NUMBER_CLOUDY_PIXELS;


  // Level-2 Cloud Top Height File (G99)
  static const char* CLOUD_TOP_HEIGHT;
  static const char* MIN_CLOUD_TOP_HEIGHT;
  static const char* MAX_CLOUD_TOP_HEIGHT;
  static const char* MEAN_CLOUD_TOP_HEIGHT;
  static const char* STD_DEV_CLOUD_TOP_HEIGHT;


  // Level-2 Clear Sky Mask File (H99)
  static const char* CLEAR_SKY_MASK;
  static const char* TOTAL_NUMBER_OF_CLOUD_MASK_POINTS;
  static const char* NUMBER_OF_CLEAR_PIXELS;
  static const char* NUMBER_OF_PROBABLY_CLEAR_PIXELS;
  static const char* NUMBER_OF_PROBABLY_CLOUDY_PIXELS;
  static const char* NUMBER_OF_CLOUDY_PIXELS;
  static const char* PERCENT_CLEAR_PIXELS;
  static const char* PERCENT_PROBABLY_CLEAR_PIXELS;
  static const char* PERCENT_PROBABLY_CLOUDY_PIXELS;
  static const char* MIN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY;
  static const char* MAX_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY;
  static const char* MEAN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY;
  static const char* STD_DEV_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY;
  static const char* MIN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY;
  static const char* MAX_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY;
  static const char* MEAN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY;
  static const char* STD_DEV_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY;
  static const char* RTM_BT_COMPARISON_WAVELENGTHS;
  static const char* RTM_BT_COMPARISON_BAND_IDS;

  // Level-2 Cloud Top Temperature File (I99)
  static const char* CLOUD_TOP_TEMP;
  static const char* MIN_CLOUD_TOP_TEMPERATURE;
  static const char* MAX_CLOUD_TOP_TEMPERATURE;
  static const char* MEAN_CLOUD_TOP_TEMPERATURE;
  static const char* STD_DEV_CLOUD_TOP_TEMPERATURE;

  // Level-2 Fire Weather File (J99)
  static const char* FIRE_AREA;
  static const char* FIRE_TEMP;
  static const char* FIRE_MASK;
  static const char* FIRE_POWER;
  static const char* TOTAL_NUMBER_OF_PIXELS_WITH_FIRES_DETECTED;
  static const char* TOTAL_NUMBER_OF_PIXELS_WITH_FIRE_TEMPERATURE_AND_AREA;
  static const char* TOTAL_NUMBER_OF_PIXELS_WITH_FIRE_RADIATIVE_POWER;
  static const char* FIRE_TEMPERATURE_OUTLIER_PIXEL_COUNT;
  static const char* FIRE_AREA_OUTLIER_PIXEL_COUNT;
  static const char* FIRE_RADIATIVE_POWER_OUTLIER_PIXEL_COUNT;
  static const char* MIN_FIRE_TEMPERATURE;
  static const char* MAX_FIRE_TEMPERATURE;
  static const char* MEAN_FIRE_TEMPERATURE;
  static const char* STD_DEV_FIRE_TEMPERATURE;
  static const char* MIN_FIRE_AREA;
  static const char* MAX_FIRE_AREA;
  static const char* MEAN_FIRE_AREA;
  static const char* STD_DEV_FIRE_AREA;
  static const char* MIN_FIRE_RADIATIVE_POWER;
  static const char* MAX_FIRE_RADIATIVE_POWER;
  static const char* MEAN_FIRE_RADIATIVE_POWER;
  static const char* STD_DEV_FIRE_RADIATIVE_POWER;

  // Level-2 Land surface Temp File (K99)
  static const char* LAND_SURFACE_TEMP;
  static const char* TOTAL_PIXELS_WHERE_LST_IS_RETRIEVED;
  static const char* NUMBER_GOOD_RETRIEVALS;
  static const char* MIN_LAND_SURFACE_TEMP;
  static const char* MAX_LAND_SURFACE_TEMP;
  static const char* MEAN_LAND_SURFACE_TEMP;
  static const char* STD_DEV_LAND_SURFACE_TEMP;

  // Level-2 Stability Indicies File (N99)
  static const char* LIFTED_INDEX;
  static const char* CAPE;
  static const char* TOTAL_TOTALS;
  static const char* SHOWALTER_INDEX;
  static const char* K_INDEX;
  static const char* FINAL_AIR_PRESSURE;
  static const char* CAPE_OUTLIER_PIXEL_COUNT;
  static const char* LIFTED_INDEX_OUTLIER_PIXEL_COUNT;
  static const char* SHOWALTER_INDEX_OUTLIER_PIXEL_COUNT;
  static const char* TOTAL_TOTALS_INDEX_OUTLIER_PIXEL_COUNT;
  static const char* K_INDEX_OUTLIER_PIXEL_COUNT;
  static const char* MIN_CAPE;
  static const char* MAX_CAPE;
  static const char* MEAN_CAPE;
  static const char* STD_DEV_CAPE;
  static const char* MIN_LIFTED_INDEX;
  static const char* MAX_LIFTED_INDEX;
  static const char* MEAN_LIFTED_INDEX;
  static const char* STD_DEV_LIFTED_INDEX;
  static const char* MIN_TOTAL_TOTALS_INDEX;
  static const char* MAX_TOTAL_TOTALS_INDEX;
  static const char* MEAN_TOTAL_TOTALS_INDEX;
  static const char* STD_DEV_TOTAL_TOTALS_INDEX;
  static const char* MIN_SHOWALTER_INDEX;
  static const char* MAX_SHOWALTER_INDEX;
  static const char* MEAN_SHOWALTER_INDEX;
  static const char* STD_DEV_SHOWALTER_INDEX;
  static const char* MIN_K_INDEX;
  static const char* MAX_K_INDEX;
  static const char* MEAN_K_INDEX;
  static const char* STD_DEV_K_INDEX;

  // Level-2 Total Precipitable Water File (O99)
  static const char* TOTAL_PRECIPITABLE_WATER;
  static const char* MIN_TOTAL_PRECIPITABLE_WATER;
  static const char* MAX_TOTAL_PRECIPITABLE_WATER;
  static const char* MEAN_TOTAL_PRECIPITABLE_WATER;
  static const char* STD_DEV_TOTAL_PRECIPITABLE_WATER;

  // Level-2 Derived Motion Winds File (U99)
  // Not implementing ingest of this product for now
  
  // Level-2 Cloud Particle Size File (W01)
  static const char* CLOUD_PARTICLE_SIZE;
  static const char* OUTLIER_PSD_DAY;
  static const char* OUTLIER_PSD_NIGHT;
  static const char* MIN_PSD_DAY;
  static const char* MAX_PSD_DAY;
  static const char* MEAN_PSD_DAY;
  static const char* STD_DEV_PSD_DAY;
  static const char* MIN_PSD_NIGHT;
  static const char* MAX_PSD_NIGHT;
  static const char* MEAN_PSD_NIGHT;
  static const char* STD_DEV_PSD_NIGHT;


  // Level-2 Cloud Top Pressure File (X01)
  static const char* CLOUD_TOP_PRESSURE;
  static const char* MIN_CLOUD_TOP_PRESSURE;
  static const char* MAX_CLOUD_TOP_PRESSURE;
  static const char* MEAN_CLOUD_TOP_PRESSURE;
  static const char* STD_DEV_CLOUD_TOP_PRESSURE;

  // Level-2 Cloud Optical Depth File (Y01)
  static const char* CLOUD_OPTIC_DEPTH;
  static const char* MIN_COD_DAY;
  static const char* MAX_COD_DAY;
  static const char* MEAN_COD_DAY;
  static const char* STD_DEV_COD_DAY;
  static const char* MIN_COD_NIGHT;
  static const char* MAX_COD_NIGHT;
  static const char* MEAN_COD_NIGHT;
  static const char* STD_DEV_COD_NIGHT;
  static const char* COD_PRODUCT_WAVELENGTH;
  static const char* OUTLIER_COD_DAY;
  static const char* OUTLIER_COD_NIGHT;
  
  // Variable Attributes
  static const char* SCALE_FACTOR;
  static const char* ADD_OFFSET;
  static const char* UNITS;
  static const char* AXIS;
  static const char* LONG_NAME;
  static const char* STANDARD_NAME;
  static const char* FILL_VALUE;
  static const char* UNSIGNED;
  static const char* SENSOR_BAND_BIT_DEPTH;
  static const char* VALID_RANGE;
  static const char* RESOLUTION;
  static const char* COORDINATES;
  static const char* GRID_MAPPING;
  static const char* CELL_METHODS;
  static const char* ANCILLARY_VARIABLES;
  static const char* FLAG_VALUES;
  static const char* FLAG_MEANINGS;
  static const char* FLAG_MASKS;
  static const char* NUMBER_OF_QF_VALUES;
  static const char* PERCENT_GOOD_PIXEL_QF;
  static const char* PERCENT_CONDITIONALLY_USABLE_PIXEL_QF;
  static const char* PERCENT_OUT_OF_RANGE_PIXEL_QF;
  static const char* PERCENT_NO_VALUE_PIXEL_QF;
  static const char* BOUNDS;
  static const char* GRID_MAPPING_NAME;
  static const char* PERSPECTIVE_POINT_HEIGHT;
  static const char* SEMI_MAJOR_AXIS;
  static const char* SEMI_MINOR_AXIS;
  static const char* INVERSE_FLATTENING;
  static const char* LATITUDE_OF_PROJECTION_ORIGIN;
  static const char* LONGITUDE_OF_PROJECTION_ORIGIN;
  static const char* SWEEP_ANGLE_AXIS;
  static const char* GEOSPATIAL_WESTBOUND_LONGITUDE;
  static const char* GEOSPATIAL_NORTHBOUND_LATITUDE;
  static const char* GEOSPATIAL_EASTBOUND_LONGITUDE;
  static const char* GEOSPATIAL_SOUTHBOUND_LATITUDE;
  static const char* GEOSPATIAL_LAT_CENTER;
  static const char* GEOSPATIAL_LON_CENTER;
  static const char* GEOSPATIAL_LAT_NADIR;
  static const char* GEOSPATIAL_LON_NADIR;
  static const char* GEOSPATIAL_LAT_UNITS;
  static const char* GEOSPATIAL_LON_UNITS;
  static const char* INPUT_ABI_L0_DATA;
  static const char* L1B_PROCESSING_PARM_VERSION;
  static const char* ALGORITHM_VERSION;
  static const char* PRODUCT_VERSION;
  
  // strings to identify product type
  static const char* TITLE_LEVEL1B_RADIANCES;
  static const char* TITLE_CLOUD_AND_MOISTURE_IMAGERY;
  static const char* TITLE_AEROSOL_DETECTION;
  static const char* TITLE_AEROSOL_OPTICAL_DEPTH;
  static const char* TITLE_CLOUD_TOP_PHASE;
  static const char* TITLE_CLOUD_TOP_HEIGHT;
  static const char* TITLE_CLOUD_TOP_TEMPERATURE;
  static const char* TITLE_CLOUD_TOP_PRESSURE;
  static const char* TITLE_DERIVED_STABILITY_INDICES;
  static const char* TITLE_TOTAL_PRECIPITABLE_WATER;
  static const char* TITLE_CLEAR_SKY_MASK;
  static const char* TITLE_FIRE_CHARACTERIZATION;
  static const char* TITLE_LAND_SURFACE_TEMPERATURE;
  static const char* TITLE_CLOUD_OPTICAL_DEPTH;
  static const char* TITLE_CLOUD_PARTICLE_SIZE;
  static const char* TITLE_DERIVED_MOTION_WINDS;
  static const char* TITLE_GLOBAL_LIGHTNING;

  // strings to identify type of scan
  static const char* FULL_DISK_SCAN_NAME;
  static const char* CONUS_SCAN_NAME;
  static const char* MESOSCALE_SCAN_NAME;

    /**
   * @brief Program name.
   */

  std::string _progName;

  /**
   * @brief Command line arguments.
   */

  Args *_args;

  /**
   * @brief Parameter file parameters.
   */

  Params *_params;
  
  /**
   * @brief The data trigger object.
   */

  DsTrigger *_dataTrigger;
  
  /**
   * @brief List of domains we need to process.
   */

  MdvxProj  _outputDomain;
  
  /**
   * @brief The vertical level type to use in the output file.
   */

  Mdvx::vlevel_type_t _outputVlevelType;


  /** 
   * @brief The NcxxFile object for the netCDF file
   */

  NcxxFile _file;


  /** 
   * @brief The global attributes object container
   */

  GlobalAttributes _globalAtts;

  /** 
   * @brief the type of product (i.e. radiances, GLM, or other level-2 products)
   */

  product_type_t _productType;


  /** 
   * @brief the type of scan (i.e. full disk, CONUS, or mesoscale)
   */

  scan_type_t _scanType;

  std::string _errStr; 

  /**
   * @brief the dimensions
   *
   */
  

  size_t _numX;
  size_t _numY;
  size_t _numBands;
  size_t _numImageBounds;
  size_t _numLZABounds;
  size_t _numSZABounds;
  size_t _numStarLooks;
  size_t _numSunglintAngleBounds;
  size_t _numLandSensorBands;
  size_t _numSeaSensorBands;
  size_t _numLatitudeBands;
  size_t _numLatitudeBandBounds;
  size_t _numAreaTypes;
  size_t _maxAreaTypeLen;
  size_t _numLatBounds;
  size_t _numSoundingEmissiveBands;
  size_t _numRtmBtComparisonBands;
  
  /**
   * @brief the variables
   *
   */

  std::vector< float > _xCoord;
  std::vector< float > _yCoord;
  float _dxRad;
  float _dyRad;

  std::vector< float > _outData;
  std::vector< float > _outQC;
  std::vector< float > _radiance;
  std::vector< float > _cmi;
  std::vector< std::int8_t > _dataQuality;
  std::map< int, std::string > _qualityFlags;

  float _ncRadMissingVal;
  float _ncCmiMissingVal;
  int8_t _ncDqfMissingVal;
  bool _yawFlipFlag;
  int _bandID;
  float _bandWavelength;
  
  string _dqfName, _dqfLongName, _dqfStandardName, _dqfUnits;
  string _outName, _outLongName, _outStandardName, _outUnits;
  string _radName, _radLongName, _radStandardName, _radUnits;
  string _cmiName, _cmiLongName, _cmiStandardName, _cmiUnits;

  std::time_t _beginTime;
  std::time_t _midpointTime;
  std::time_t _endTime;
  
  //  GoesImagerProjection _goesImagerProjection;
  double _nominalSatSubpointLat; 
  double _nominalSatSubpointLon;
  double _nominalSatHeight;
  double _westLongitude;
  double _northLatitude;
  double _eastLongitude;
  double _southLatitude;
  double _centerLongitude;
  double _centerLatitude;
  double _nadirLongitude;
  double _nadirLatitude;
  double _xImageCenter;
  double _yImageCenter;
  std::vector< double > _xImageBounds;
  std::vector< double > _yImageBounds;

  std::string _gridMappingName;
  double _perspectivePointHeight;
  double _semiMajorAxis;
  double _semiMinorAxis;
  double _inverseFlattening;
  double _projectionOriginLongitude;
  double _projectionOriginLatitude;
  double _ecc;
  double _radiusRatio2;
  double _invRadiusRatio2;
  double _H;
  int _xSubSatIdx;
  int _ySubSatIdx;

  
  // variables for converting radiances to relfectances and
  // brightness temperatures

  double _eSun; 
  double _kappa0; 
  double _planckFk1; 
  double _planckFk2; 
  double _planckBc1; 
  double _planckBc2; 

  // quality control variables
  
  int _validPixelCount;
  int _missingPixelCount;
  int _saturatedPixelCount;
  int _undersaturatedPixelCount;
  float _minRadValueValidPixels;
  float _maxRadValueValidPixels;
  float _meanRadValueValidPixels;
  float _stdDevRadValueValidPixels;
  float _percentUncorrectableL0Errors;
  double _earthSunDistAnomalyAU;
  std::string _inputAbiL0Data;
  std::string _l1bProcessingParamVersion;
  std::string _algorithmVersion;
  std::string _algoProductVersion;
  std::vector< std::time_t > _timeStarLook;
  std::vector< float > _bandWavelengthStarLook;
  std::vector< int > _starID;

  // sun angle computations for albedo correction

  SunAngle _sunAngle;
  
  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   *
   * @note The constructor is private because this is a singleton object.
   */

  GoesRnetCDF2Mdv(int argc, char **argv);
  

  /**
   * @brief Clear and reset internal state.
   *
   * @param[in] None
   *
   * @return None
   */
  
  void _clearAndReset();
  

  /**
   * @brief Convert the units of the data in the given field using the
   *        conversion specified in the parameter file.
   *
   * @param[in,out] field    The field to convert.
   */

  void _convertUnits(MdvxField *field) const;

  /**
   * @brief Initialize the list of output domains.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initDomain();
  

  /**
   * @brief Initialize the output vertical level type.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputVlevelType();
  

  /**
   * @brief Initialize the program trigger object.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /**
   * @brief Process data for the given trigger.
   *
   * @param[in] 
   *
   * @return Returns true on success, false on failure.
   */

  bool _processData();
  
  /**
   * @brief Read the netCDF file.
   *
   * @param[in] file_path    The file path.
   *
   * @return Returns true on success, false on failure.
   */

  bool _readFile(const std::string& file_path);


  // TODO: add comment blocks for methods
  // These methods will be part of GOES-R reader library
  void _readVariables();
  void _readGlobalAttributes();
  void _readOptionalGlobalAttr(const string &name, string &dest);
  int _readRequiredGlobalAttr(const string &name, string &dest);
  int _getVarAtt(const NcxxVar &var, const string &attName, NcxxVarAtt &att);
  string _getAsStr(NcxxVarAtt &att);
  int _getAsInt(NcxxVarAtt &att);
  vector<int>_getAsInts(NcxxVarAtt &att);
  float _getAsFloat(NcxxVarAtt &att);
  double _getAsDouble(NcxxVarAtt &att);
  void _printGlobalAttributes(ostream &out);
  void _readDimensions();
  void _readZenithAngleDims();
  void _readCoordinateVars();
  void _readTimeVars();
  void _readProjectionVars();
  void _readRadianceConstantsVars();
  void _readQualityControlVars();
  void _readFieldVars();
  void _readCMI();
  void _readRadiance();
  void _readDataQualFlag();
  void _setProductType(const std::string &product_type);
  void _setScanType(const std::string &scan_id);

  
  float _radianceToBrightTemp(float rad);
  float _radianceToAlbedo(float rad, 
                          double lat = -9999.0,
                          double lon = -9999.0);
  bool _latLon2XY(double lat, double lon, int& col_num,  int& line_num);  
  void _addData(float *out_data, float *qc_data, float *rad_data);
  MdvxField* _createField(const std::string &field_name,
			  const std::string &long_field_name,
			  const std::string &units);
  void _updateMasterHeader(DsMdvx &mdvx);
  std::string _getErrStr() { return _errStr; }
  void _addErrStr(std::string label, std::string strarg = "", bool cr = true);
};


#endif
