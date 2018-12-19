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
 * @file GoesRGLM2Spdb.hh
 *
 * @class GoesRGLM2Spdb
 *
 * GoesRGLM2Spdb is the top level application class.
 *  
 * @date 4/12/2018
 *
 */

#ifndef GoesRGLM2Spdb_HH
#define GoesRGLM2Spdb_HH

#include <dsdata/DsTrigger.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Ncxx/Ncxx.hh>
#include <didss/DsInputPath.hh>

#include "Args.hh"
#include "Params.hh"


/** 
 * @class GoesRGLM2Spdb
 */

class GoesRGLM2Spdb {
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

  ~GoesRGLM2Spdb(void);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static GoesRGLM2Spdb *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static GoesRGLM2Spdb *Inst();


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


  /////////////////////
  // Private classes //
  /////////////////////

  // string content of global attributes
  class GlobalAttributes {
  public:
    std::string title;
    std::string ICD_version;
    std::string conventions;
    int channelID;
    float centralWavelength;
    int abi_mode;
    std::string sourceScene;
    float periodicity;
    std::string productionLocation;
    std::string productName;
    std::string satelliteID;
    float productCenterLat;
    float productCenterLon;
    std::string projection;
    int bitDepth;
    float sourceSpatialRes;
    float requestSpatialRes;
    std::string startDateTime;
    int numberProductTiles;
    int productTileWidth;
    int productTileHeight;
    int productRows;
    int productColumns;
    float pixelXsize;
    float pixelYsize;
    float satelliteLat;
    float satelliteLon;
    float satelliteAlt;
    std::string createdBy;
    int productTilesReceived;
  };


  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static GoesRGLM2Spdb *_instance;

  /**
   * @brief string constant names of attributues, dimensions, and variables
   */

  // start of epoch for GOES-R is 2000-01-01 12:00:00, the J2000 epoch
  static const time_t J2000_EPOCH_START;

  // header string for logging messages
  static const char *ERROR_STR;
  static const char *WARN_STR;
  static const char *INFO_STR;

  // Global Attributes

  static const char *TITLE;
  static const char *ICD_VERSION;
  static const char *CONVENTIONS;
  static const char *CHANNEL_ID;
  static const char *CENTRAL_WAVELENGTH;
  static const char *ABI_MODE;
  static const char *SOURCE_SCENE;
  static const char *PERIODICITY;
  static const char *PRODUCTION_LOCATION;
  static const char *PRODUCT_NAME;
  static const char *SATELLITE_ID;
  static const char *PRODUCT_CENTER_LAT;
  static const char *PRODUCT_CENTER_LON;
  static const char *PROJECTION;
  static const char *BIT_DEPTH;
  static const char *SOURCE_SPATIAL_RES;
  static const char *REQUEST_SPATIAL_RES;
  static const char *START_DATE_TIME;
  static const char *NUMBER_PRODUCT_TILES;
  static const char *PRODUCT_TILE_WIDTH;
  static const char *PRODUCT_TILE_HEIGHT;
  static const char *PRODUCT_ROWS;
  static const char *PRODUCT_COLUMNS;
  static const char *PIXEL_X_SIZE;
  static const char *PIXEL_Y_SIZE;
  static const char *SATELLITE_LAT;
  static const char *SATELLITE_LON;
  static const char *SATELLITE_ALT;
  static const char *CREATED_BY;
  static const char *PRODUCT_TILES_RECEIVED;

  // product Dimensions
  static const char *NUMBER_OF_EVENTS;
  static const char *NUMBER_OF_GROUPS;
  static const char *NUMBER_OF_FLASHES;

  // Count Variables
  static const char *EVENT_COUNT;
  static const char *GROUP_COUNT;
  static const char *FLASH_COUNT;

  // Time Variables
  static const char *PRODUCT_TIME;
  static const char *EVENT_TIME_OFFSET;
  static const char *GROUP_TIME_OFFSET;
  static const char *FLASH_TIME_OFFSET_LAST_EVENT;
  static const char *FLASH_TIME_OFFSET_FIRST_EVENT;

  // Location Variables
  static const char *EVENT_LAT;
  static const char *EVENT_LON;
  static const char *GROUP_LAT;
  static const char *GROUP_LON;
  static const char *FLASH_LAT;
  static const char *FLASH_LON;

  // ID Variables
  static const char *FLASH_ID;
  static const char *GROUP_ID;
  static const char *EVENT_ID;
  static const char *GROUP_PARENT_FLASH_ID;
  static const char *EVENT_PARENT_GROUP_ID;

  // Data Quality Variables
  static const char *MISSING_QUALITY_FLAG;
  static const char *GROUP_QUALITY_FLAG;
  static const char *FLASH_QUALITY_FLAG;

  // Variable Attributes
  static const char *ADD_OFFSET;
  static const char *SCALE_FACTOR;
  static const char *VALID_RANGE;


  struct flash_properties {
    unsigned long flash_id;
    vector<unsigned long> group_ids;
    vector<size_t> events_in_group;
    size_t calculatedFlashQuality; // 0 = good 1 = bad
  };

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
  DsInputPath *fileTrigger;

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

  std::string _errStr;

  std::time_t _beginTime;

  // seconds

  std::vector<time_t> _productTime;


  std::vector<float>_productLat;
  std::vector<float>_productLon;
  std::vector<short>_productQualityFlag;
  std::vector<unsigned long>_productFlashId;
  std::vector<unsigned long>_productGroupId;
  std::vector<unsigned long>_productEventId;
  std::vector<unsigned long>_productGroupParentFlashId;
  std::vector<unsigned long>_productEventParentGroupId;

  size_t _productCount;
  size_t _eventCount;
  size_t _groupCount;
  size_t _flashCount;

  size_t _Dim_number_of_events;
  size_t _Dim_number_of_groups;
  size_t _Dim_number_of_flashes;

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

  GoesRGLM2Spdb(int argc, char **argv);


  /**
   * @brief Clear and reset internal state.
   *
   * @param[in] None
   *
   * @return None
   */

  void _clearAndReset();

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

  bool _readFile(const std::string &file_path);

  bool _readVariables();

  void _readGlobalAttributes();

  void _readDimensions();

  void _readCoordinateVars(const char *LatVar, const char *LonVar);

  bool _readTimeVars(const char *TimeOffsetVar);

  bool _readCountVars(const char *CountVar, size_t &ProductCount);

  void _readDataQualityFlag(const char *DataQaulityVar);

  void _readIdVars(const char *Id, size_t count, vector<unsigned long>&ID);

  std::string _getErrStr() { return _errStr; }
};


#endif
