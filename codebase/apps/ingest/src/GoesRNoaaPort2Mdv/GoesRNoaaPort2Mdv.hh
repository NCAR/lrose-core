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
 * @file GoesRNoaaPort2Mdv.hh
 *
 * @class GoesRNoaaPort2Mdv
 *
 * GoesRNoaaPort2Mdv is the top level application class.
 *  
 * @date 3/21/2018
 *
 */

#ifndef GoesRNoaaPort2Mdv_HH
#define GoesRNoaaPort2Mdv_HH

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
 * @class GoesRNoaaPort2Mdv
 */

class GoesRNoaaPort2Mdv {
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

  ~GoesRNoaaPort2Mdv(void);


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv Command line arguments.
   */

  static GoesRNoaaPort2Mdv *Inst(int argc, char **argv);


  /**
   * @brief Retrieve the singleton instance of this class.
   */

  static GoesRNoaaPort2Mdv *Inst();


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
    PRODUCT_SECTORIZED_IMAGERY,
  } product_type_t;

  typedef enum {
    SCAN_FULL_DISK,
    SCAN_CONUS,
    SCAN_MESOSCALE_1,
    SCAN_MESOSCALE_2
  } scan_type_t;

  typedef enum {
    PROJECTION_LAMBERT_CONFORMAL,
    PROJECTION_MERCATOR,
    PROJECTION_FIXED_GRID
  } projection_type_t;

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

  static GoesRNoaaPort2Mdv *_instance;

  /**
   * @brief string constant names of attributues, dimensions, and variables
   */

  // Start of epoch for Noaa Port GOES-R data is 2017-01-01 00:00:00
  static const time_t J2017_EPOCH_START;

  // projection parameters not in header
  static const float INVERSE_FLATTENING;

  // conversion from micro radians to radians
  static const float MICRORAD_TO_RAD;

  static const float MISSING_DATA_VALUE;
  static const float BAD_DATA_VALUE;

  static const int EMISSIVE_BAND_START;

  // header string for logging messages
  static const char *ERROR_STR;
  static const char *WARN_STR;
  static const char *INFO_STR;

  // product Dimensions
  static const char *X_DIM;
  static const char *Y_DIM;

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

  // Variables
  static const char *TIME;
  static const char *SECTORIZED_CMI;
  static const char *Y_COORD;
  static const char *X_COORD;
  static const char *LAMBERT_PROJECTION;
  static const char *MERCATOR_PROJECTION;
  static const char *FIXEDGRID_PROJECTION;


  // Variable Attributes
  static const char *FILL_VALUE;
  static const char *STANDARD_NAME;
  static const char *UNITS;
  static const char *GRID_MAPPING;
  static const char *ADD_OFFSET;
  static const char *SCALE_FACTOR;
  static const char *VALID_MIN;
  static const char *VALID_MAX;
  static const char *COORDINATES;
  static const char *GRID_MAPPING_NAME;
  static const char *STANDARD_PARALLEL;
  static const char *LONGITUDE_OF_PROJECTION_ORIGIN;
  static const char *LONGITUDE_OF_CENTRAL_MERIDIAN;
  static const char *LATITUDE_OF_PROJECTION_ORIGIN;
  static const char *SEMI_MAJOR_AXIS;
  static const char *SEMI_MINOR_AXIS;
  static const char *PERSPECTIVE_POINT_HEIGHT;

  // strings to identify type of scan
  static const char *FULL_DISK_SCAN_NAME;
  static const char *CONUS_SCAN_NAME;
  static const char *MESOSCALE_1_SCAN_NAME;
  static const char *MESOSCALE_2_SCAN_NAME;

  //strings to identify projection type
  static const char *LAMBERT;
  static const char *MERCATOR;
  static const char *FIXED_GRID;

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
   * @brief List of domains we need to process.
   */

  MdvxProj _outputDomain;

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

  projection_type_t _projectionType;

  std::string _errStr;

  std::vector<float> _Sectorized_CMI;
  float _ncRadMissingVal;
  int _bandID;
  float _bandWavelength;

  std::time_t _beginTime;

  //  Satellite location
  float _nominalSatSubpointLat;
  float _nominalSatSubpointLon;
  float _nominalSatHeight;

  // projection information
  MdvxRemapLut _remapLut;

  int _nx;
  int _ny;
  float _dx;
  float _dy;

  std::string _gridMappingName;
  float _standardParallel;
  float _projectionOriginLongitude;
  float _projectionOriginLatitude;
  float _semiMajorAxis;
  float _semiMinorAxis;
  float _perspectivePointHeight;
  float _ecc;
  float _radiusRatio2;
  float _invRadiusRatio2;
  float _H;
  float _dxRad;
  float _dyRad;
  float _minxRad;
  float _minyRad;


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

  GoesRNoaaPort2Mdv(int argc, char **argv);


  /**
   * @brief Clear and reset internal state.
   *
   * @param[in] None
   *
   * @return None
   */

  void _clearAndReset();

  /**
   * @brief Initialize the list of output domains.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initDomain();


  /**
   * @brief Convert the units of the data in the given field using the
   *        conversion specified in the parameter file.
   *
   * @param[in,out] field    The field to convert.
   */

  void _convertUnits(MdvxField *field) const;

  /**
   * @brief Initialize the output vertical level type.
   *
   * @return Returns true on success, false on failure.
   */

  bool _initOutputVlevelType();

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

  void _readVariables();

  void _readGlobalAttributes();

  void _readDimensions();

  void _readCoordinateVars();

  void _readTimeVars();

  void _readProjectionVars();

  void _ReadSectorized_CMI();

  void _setProductType(const std::string &product_type);

  void _setScanType(const std::string &scan_id);

  void _setProjectionType(const std::string &projection);

  void _addData(float *out_data);

  MdvxField *_createField(const std::string &field_name,
                          const std::string &long_field_name,
                          const std::string &units);

  void _updateMasterHeader(DsMdvx &mdvx);

  bool _latLon2XY(float lat, float lon, int &col_num, int &line_num);

  std::string _getErrStr() { return _errStr; }

  void _addErrStr(std::string label, std::string strarg = "", bool cr = true);
};


#endif
