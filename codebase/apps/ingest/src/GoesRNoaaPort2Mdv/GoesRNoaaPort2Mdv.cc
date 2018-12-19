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
 * @file GoesRNoaaPort2Mdv.cc
 *
 * @class GoesRNoaaPort2Mdv
 *
 * GoesRNoaaPort2Mdv is the top level application class.
 *  
 * @date 3/21/2018
 *
 */

#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>

#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/TriggerInfo.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapmath/math_macros.h>
#include <toolsa/os_config.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

#include "GoesRNoaaPort2Mdv.hh"
#include "Params.hh"

using namespace std;


// Global variables

GoesRNoaaPort2Mdv *GoesRNoaaPort2Mdv::_instance = (GoesRNoaaPort2Mdv *) NULL;

// start of epoch for GOES-R NoaaPort data is 2017-01-01 00:00:00
const time_t GoesRNoaaPort2Mdv::J2017_EPOCH_START = 1483228800;

// projection parameters not in header
const float GoesRNoaaPort2Mdv::INVERSE_FLATTENING = 298.257222096;

// conversion from micro radians to radians
const float GoesRNoaaPort2Mdv::MICRORAD_TO_RAD = 0.000001;

// missing and and bad values used in MDV files.
const float GoesRNoaaPort2Mdv::MISSING_DATA_VALUE = 999999.0;
const float GoesRNoaaPort2Mdv::BAD_DATA_VALUE = 888888.0;
const int GoesRNoaaPort2Mdv::EMISSIVE_BAND_START = 7;


/*********************************************************************
 * Constructor
 */

GoesRNoaaPort2Mdv::GoesRNoaaPort2Mdv(int argc, char **argv) :
  _outputVlevelType(Mdvx::VERT_SATELLITE_IMAGE) {
  static const string method_name = "GoesRNoaaPort2Mdv::GoesRNoaaPort2Mdv()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (GoesRNoaaPort2Mdv *) NULL);

  // Initialize the okay flag.

  okay = true;

  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = progname_parts.base;

  // Display ucopyright message.

  ucopyright(_progName.c_str());

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  // Get TDRP parameters.

  _params = new Params();
  // Params class responsible for params_path 
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");

  if (_params->loadFromArgs(argc, argv,
                            _args->override.list,
                            &params_path)) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;

    okay = false;
  }

}

/*********************************************************************
 * Destructor
 */

GoesRNoaaPort2Mdv::~GoesRNoaaPort2Mdv() {
  // Unregister process

  PMU_auto_unregister();

  // // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;

}


/*********************************************************************
 * Inst()
 */

GoesRNoaaPort2Mdv *GoesRNoaaPort2Mdv::Inst(int argc, char **argv) {
  if (_instance == (GoesRNoaaPort2Mdv *) NULL)
    new GoesRNoaaPort2Mdv(argc, argv);

  return (_instance);
}

GoesRNoaaPort2Mdv *GoesRNoaaPort2Mdv::Inst() {
  assert(_instance != (GoesRNoaaPort2Mdv *) NULL);

  return (_instance);
}

/*********************************************************************
 * init()
 */

bool GoesRNoaaPort2Mdv::init() {
  static const string method_name = "GoesRNoaaPort2Mdv::init()";

  // Initialize the output vertical level type

  if (!_initOutputVlevelType())
    return false;

  // initialize process registration

  PMU_auto_init(_progName.c_str(), _params->instance,
                _params->procmap_reg_interval);

  return true;
}

void GoesRNoaaPort2Mdv::run() {
  static const string method_name = "GoesRNoaaPort2Mdv::run()";

  //
  // Initialize file trigger
  //
  if (_params->mode == Params::REALTIME) {
    if (_params->debug)
      cerr << "FileInput::init: Initializing realtime input from "
           << _params->input_dir << endl;

    bool debug = false;
    if (_params->debug == Params::DEBUG_VERBOSE) {
      debug = true;
    }

    fileTrigger = new DsInputPath(_progName, debug,
                                  _params->input_dir,
                                  _params->max_valid_realtime_age_min * 60,
                                  PMU_auto_register,
                                  _params->ldata_info_avail,
                                  false);

    //
    // set max recursion depth
    //
    fileTrigger->setMaxRecursionDepth(_params->Dir_search_depth);

    //
    // Set wait for file to be written to disk before being served out.
    //
    fileTrigger->setFileQuiescence(_params->file_quiescence_sec);

    //
    // Set time to wait before looking for new files.
    //
    fileTrigger->setDirScanSleep(_params->check_input_sec);

    // Set file naming requirements
    if (strlen(_params->DomainString) > 0) {
      fileTrigger->setSubString(_params->DomainString);
    }
    if (strlen(_params->File_extension) > 0) {
      fileTrigger->setSearchExt(_params->File_extension);
    }

  }

  if (_params->mode == Params::FILE_LIST) {
    //
    // Archive mode.
    //
    const vector<string> &fileList = _args->getFileList();

    if (fileList.size() == 0) {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify file paths on command line in FILE_LIST mode." << endl;

      return;
    }

    if (fileList.size()) {
      if (_params->debug)
        cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;

      fileTrigger = new DsInputPath(_progName, _params->debug, fileList);
    }
  }

  //
  //
  // process data
  //
  char *inputPath;

  while ((inputPath = fileTrigger->next()) != NULL) {

    if (_readFile(inputPath) == false) {
      // Need better info on the reason for the failure

      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading file: " << inputPath << endl;

      continue;
    }


    if (_processData() == false) {
      // Need better info on the reason for the failure

      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing file: " << inputPath << endl;

      continue;
    }
  } // while

  delete fileTrigger;

  return;

}
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addData()
 */
void GoesRNoaaPort2Mdv::_addData(float *out_data) {
  Mdvx::coord_t coords = _outputDomain.getCoord();

  float min_val = float(999999999.0);
  float max_val = float(-999999999.0);

  for (int yOutIdx = 0; yOutIdx < coords.ny; ++yOutIdx) {
    for (int xOutIdx = 0; xOutIdx < coords.nx; ++xOutIdx) {

      // Get the lat/lon location of the grid point

      double lat, lon;
      int xSatIdx, ySatIdx;

      _outputDomain.xyIndex2latlon(xOutIdx, yOutIdx, lat, lon);

      // Convert lat/lon to satellite space x/y

      if (!_latLon2XY( float(lat), float(lon), xSatIdx, ySatIdx)) {
        continue;
      }

      int satIndex = ySatIdx * static_cast<int>(_nx) + xSatIdx;

      if (satIndex < 0) {
        continue;
      }
      size_t outDataIndex = size_t(yOutIdx * coords.nx + xOutIdx);

      if (_bandID < EMISSIVE_BAND_START) {

        // reflective bands 1 - 6 are in reflectance factor

        if (_Sectorized_CMI[satIndex] != MISSING_DATA_VALUE) {
          out_data[outDataIndex] = _Sectorized_CMI[satIndex];
        }

      } else {
        // emissive bands 7 - 16 are in brightness temperature
        // no conversion needed.

        out_data[outDataIndex] = _Sectorized_CMI[satIndex];
      }
      max_val = fmax(out_data[outDataIndex], max_val);
      min_val = fmin(out_data[outDataIndex], min_val);

    } /* endfor - x_index */

  } /* endfor - y_index */

  _Sectorized_CMI.clear();
  std::vector<float>(_Sectorized_CMI).swap(_Sectorized_CMI);

}

/*********************************************************************
 * _convertUnits()
 */

void GoesRNoaaPort2Mdv::_convertUnits(MdvxField *field) const {
  static const string method_name = "GoesRNoaaPort2Mdv::_convertUnits()";

  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  fl32 *data = (fl32 *) field->getVol();

  for (int i = 0; i < field_hdr.nx * field_hdr.ny * field_hdr.nz; ++i) {
    // Skip missing data values

    if (data[i] == field_hdr.bad_data_value ||
        data[i] == field_hdr.missing_data_value)
      continue;

    switch (_params->units_convert_type) {
      case Params::CONVERT_K_TO_C :
        data[i] = TEMP_K_TO_C(data[i]);
        break;
      case Params::CONVERT_REFLECTANCE_FACTOR_TO_ALBEDO :
        data[i] = data[i] * float(100.0);
        break;
    } /* endswitch - _params->units_convert_type */
  } /* endfor - i */

  // Update the units string in the field header

  switch (_params->units_convert_type) {
    case Params::CONVERT_K_TO_C :
      STRcopy(field_hdr.units, "C", MDV_UNITS_LEN);
      break;
    case Params::CONVERT_REFLECTANCE_FACTOR_TO_ALBEDO :
      STRcopy(field_hdr.units, "Percent", MDV_UNITS_LEN);
      break;
  } /* endswitch - _params->units_convert_type */

  field->setFieldHeader(field_hdr);
}


/*********************************************************************
 * _createField()
 */

MdvxField *GoesRNoaaPort2Mdv::_createField(const string &field_name,
                                           const string &long_field_name,
                                           const string &units) {
  static const string method_name = "JmaHimawari8toMdv::_createField()";

  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));

  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = _outputVlevelType;
  field_hdr.vlevel_type = _outputVlevelType;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = BAD_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(field_hdr.field_name_long, long_field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  _outputDomain.syncToFieldHdr(field_hdr);

  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));

  vlevel_hdr.type[0] = _outputVlevelType;
  vlevel_hdr.level[0] = 0.0;

  return new MdvxField(field_hdr, vlevel_hdr, (void *) NULL, true);
}


/*********************************************************************
 * _initDomains()
 */

bool GoesRNoaaPort2Mdv::_initDomain(void) {
  static const string methodName = "GoesRnetCDF2Mdv::_initDomain()";


  // set the projection object

  if (_params->set_earth_radius == true) {
    Pjg::setEarthRadiusKm(_params->earth_radius);
  }

  _outputDomain.setGrid(_params->out_grid.nx, _params->out_grid.ny,
                        _params->out_grid.dx, _params->out_grid.dy,
                        _params->out_grid.minx, _params->out_grid.miny);

  switch (_params->out_projection) {
    case Params::PROJ_LATLON : {
      _outputDomain.initLatlon(_params->out_origin_lon);
      break;
    }

    case Params::PROJ_FLAT : {
      _outputDomain.initFlat(_params->out_origin_lat,
                             _params->out_origin_lon,
                             _params->out_rotation);
      break;
    }

    case Params::PROJ_LAMBERT_CONF : {
      _outputDomain.initLambertConf(_params->out_origin_lat,
                                    _params->out_origin_lon,
                                    _params->out_lat1,
                                    _params->out_lat2);
      break;
    }

    case Params::PROJ_LAMBERT_AZIM : {
      _outputDomain.initLambertAzim(_params->out_origin_lat,
                                    _params->out_origin_lon);
      break;
    }

    case Params::PROJ_MERCATOR : {
      _outputDomain.initMercator(_params->out_origin_lat,
                                 _params->out_origin_lon);
      break;
    }

    case Params::PROJ_TRANS_MERCATOR : {
      _outputDomain.initTransMercator(_params->out_origin_lat,
                                      _params->out_origin_lon,
                                      _params->out_central_scale);
      break;
    }

    case Params::PROJ_POLAR_STEREO : {
      _outputDomain.initPolarStereo(_params->out_tangent_lon,
        //static_cast<Mdvx::pole_type_t>(_params->out_pole_type),
                                    Mdvx::POLE_NORTH,
                                    _params->out_central_scale);

      _outputDomain.setOffsetOrigin(_params->out_origin_lat,
                                    _params->out_origin_lon);
      break;
    }

    case Params::PROJ_OBLIQUE_STEREO : {
      _outputDomain.initStereographic(_params->out_origin_lat,
                                      _params->out_origin_lon,
                                      _params->out_central_scale);
      break;
    }

    case Params::PROJ_ALBERS : {
      _outputDomain.initAlbers(_params->out_origin_lat,
                               _params->out_origin_lon,
                               _params->out_lat1,
                               _params->out_lat2);
      break;
    }

    default :
      cerr << ERROR_STR << methodName << endl;
      cerr << "  Unsupported map projection." << endl;
      return false;
  } // endswitch

  return true;
}

/*********************************************************************
 * _initOutputVlevelType()
 */

bool GoesRNoaaPort2Mdv::_initOutputVlevelType(void) {
  static const string method_name = "GoesRNoaaPort2Mdv::_initOutputVlevelType()";

  switch (_params->output_vlevel_type) {
    case Params::VERT_TYPE_SURFACE :
      _outputVlevelType = Mdvx::VERT_TYPE_SURFACE;
      break;

    case Params::VERT_TYPE_Z :
      _outputVlevelType = Mdvx::VERT_TYPE_Z;
      break;

    case Params::VERT_SATELLITE_IMAGE :
      _outputVlevelType = Mdvx::VERT_SATELLITE_IMAGE;
      break;

    case Params::VERT_FLIGHT_LEVEL :
      _outputVlevelType = Mdvx::VERT_FLIGHT_LEVEL;
      break;
  }

  return true;
}


/*********************************************************************
 * _processData()
 */

bool GoesRNoaaPort2Mdv::_processData() {
  static const string methodName = "GoesRNoaaPort2Mdv::_processData()";

  // Initialize the output domains

  if (!_initDomain())
    return false;

  // Create the output file

  DsMdvx mdvx;
  _updateMasterHeader(mdvx);

  // Create the output field and add it to the file

  MdvxField *outDataField;
  if ((outDataField = _createField(_params->out_data_prep.short_name,
                                   _params->out_data_prep.long_name,
                                   _params->out_data_prep.units)) == 0) {
    return false;
  }

  float *outData = (float *) outDataField->getVol();

  _addData(outData);

  // Convert the units in the field, if requested

  if (_params->convert_units)
    _convertUnits(outDataField);

  // Add the field to the output file

  if (outDataField->convertType(static_cast<Mdvx::encoding_type_t>(_params->out_data_prep.encoding_type),
                                static_cast<Mdvx::compression_type_t>(_params->out_data_prep.compression_type),
                                static_cast<Mdvx::scaling_type_t>(_params->out_data_prep.scaling_type),
                                _params->out_data_prep.scale,
                                _params->out_data_prep.bias) != 0) {
    cerr << WARN_STR << methodName << endl;
    cerr << "  convertType failed. message: " << outDataField->getErrStr() << endl;
  }

  mdvx.addField(outDataField);

  // Write the output file

  mdvx.printMasterHeader(cerr);

  mdvx.getField(0)->printHeaders(cerr);

  cerr << "Writing output to: " << _params->output_url << " for time "
       << DateTime::str(mdvx.getMasterHeader().time_centroid) << endl
       << endl;

  if (mdvx.writeToDir(_params->output_url) != 0) {
    cerr << ERROR_STR << methodName << endl;
    cerr << "Error writing MDV file to output URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;

    return false;
  }

  return true;
}


/*********************************************************************
 * _readFile()
 */

bool GoesRNoaaPort2Mdv::_readFile(const std::string &file_path) {
  static const string method_name = "GoesRNoaaPort2Mdv::_readFile()";

  // clear out and reset things before reading another file

  _clearAndReset();

  // open file and read contents

  try {
    _file.open(file_path, NcxxFile::read);

    _readGlobalAttributes();
    _readDimensions();
    _readVariables();

  } catch (NcxxException &e) {
    _file.close();
    cerr << _getErrStr();
    return false;
  }

  // successfully read the file
  _file.close();

  return true;
}


/*********************************************************************
 * _readGlobalAttributes()
 *
 * For the most part the global attributes only have informational 
 * value. There are a few that must be in the file and are used in 
 * subsequesnt processing. They are:
 *  SCENE_ID, TITLE, ...
 *
 * For these attributes the method will return false.
 *
 */

void GoesRNoaaPort2Mdv::_readGlobalAttributes() {
  static const string methodName = "GoesRNoaaPort2Mdv::_readGlobalAttributes()";

  // Unused Global attributes
  try {
    _file.readGlobAttr(TITLE, _globalAtts.title);
    _file.readGlobAttr(ICD_VERSION, _globalAtts.ICD_version);
    _file.readGlobAttr(CONVENTIONS, _globalAtts.conventions);
    _file.readGlobAttr(ABI_MODE, _globalAtts.abi_mode);
    _file.readGlobAttr(PERIODICITY, _globalAtts.periodicity);
    _file.readGlobAttr(PRODUCTION_LOCATION, _globalAtts.productionLocation);
    _file.readGlobAttr(SATELLITE_ID, _globalAtts.satelliteID);
    _file.readGlobAttr(PRODUCT_CENTER_LAT, _globalAtts.productCenterLat);
    _file.readGlobAttr(PRODUCT_CENTER_LON, _globalAtts.productCenterLon);
    _file.readGlobAttr(BIT_DEPTH, _globalAtts.bitDepth);
    _file.readGlobAttr(SOURCE_SPATIAL_RES, _globalAtts.sourceSpatialRes);
    _file.readGlobAttr(REQUEST_SPATIAL_RES, _globalAtts.requestSpatialRes);
    _file.readGlobAttr(START_DATE_TIME, _globalAtts.startDateTime);
    _file.readGlobAttr(NUMBER_PRODUCT_TILES, _globalAtts.numberProductTiles);
    _file.readGlobAttr(PRODUCT_TILE_WIDTH, _globalAtts.productTileWidth);
    _file.readGlobAttr(PRODUCT_TILE_HEIGHT, _globalAtts.productTileHeight);
    _file.readGlobAttr(PRODUCT_ROWS, _globalAtts.productRows);
    _file.readGlobAttr(PRODUCT_COLUMNS, _globalAtts.productColumns);
    _file.readGlobAttr(CREATED_BY, _globalAtts.createdBy);
    _file.readGlobAttr(PRODUCT_TILES_RECEIVED, _globalAtts.productTilesReceived);
  } catch (NcxxException &e) {
    cerr << endl;
    cerr << WARN_STR << methodName << endl;
    cerr << "        - Didn't find unused attribute" << endl;
    cerr << endl;
    cerr << "  " << e.what() << endl;
  }

  // Used Global attributes
  try {
    _file.readGlobAttr(CHANNEL_ID, _globalAtts.channelID);
    _file.readGlobAttr(CENTRAL_WAVELENGTH, _globalAtts.centralWavelength);
    _file.readGlobAttr(SOURCE_SCENE, _globalAtts.sourceScene);
    _file.readGlobAttr(PRODUCT_NAME, _globalAtts.productName);
    _file.readGlobAttr(PROJECTION, _globalAtts.projection);
    _file.readGlobAttr(PIXEL_X_SIZE, _globalAtts.pixelXsize);
    _file.readGlobAttr(PIXEL_Y_SIZE, _globalAtts.pixelYsize);
    _file.readGlobAttr(SATELLITE_LAT, _globalAtts.satelliteLat);
    _file.readGlobAttr(SATELLITE_LON, _globalAtts.satelliteLon);
    _file.readGlobAttr(SATELLITE_ALT, _globalAtts.satelliteAlt);
  } catch (NcxxException e) {
    NcxxErrStr err;
    ostringstream info;
    info << ERROR_STR << methodName << endl;
    info << "        - Didn't find required attribute" << endl;
    err.addErrStr(info.str());
    err.addErrStr("  ", e.what());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  _bandID = _globalAtts.channelID;
  _bandWavelength = _globalAtts.centralWavelength;
  _dx = _globalAtts.pixelXsize;
  _dy = _globalAtts.pixelYsize;
  _nominalSatSubpointLat = _globalAtts.satelliteLat;
  _nominalSatSubpointLon = _globalAtts.satelliteLon;
  _nominalSatHeight = _globalAtts.satelliteAlt;
  _setProjectionType(_globalAtts.projection);

  _setScanType(_globalAtts.sourceScene);
  _setProductType(_globalAtts.productName);

  if (_params->debug == Params::DEBUG_VERBOSE) {
    cerr << "NATIVE DOMAIN: " << endl;
    cerr << "  DX: " << _dx << endl;
    cerr << "  DY: " << _dy << endl;
  }

}


/*********************************************************************
 * _readDimensions()
 */

void GoesRNoaaPort2Mdv::_readDimensions() {
  static const string methodName = "GoesRNoaaPort2Mdv::_readDimensions()";


  try {

    switch (_productType) {
      case PRODUCT_SECTORIZED_IMAGERY: {
        break;
      }
      default: {
        NcxxErrStr err;
        stringstream info;
        info << ERROR_STR << methodName << endl;
        info << "unknown product type. should not be possible." << endl;
        err.addErrStr(info.str());
        cerr << endl;
        cerr << err.getErrStr() << endl;
        throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
    }
  }
  catch (NcxxException e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}

/*********************************************************************
 * _clearAndReset()
 */

void GoesRNoaaPort2Mdv::_clearAndReset() {
  static const string methodName = "GoesRNoaaPort2Mdv::_clearAndReset()";

}

/*********************************************************************
 * _readVariables()
 */

void GoesRNoaaPort2Mdv::_readVariables() {
  static const string methodName = "GoesRNoaaPort2Mdv::_readVariables()";

  try {
    _readCoordinateVars();
    _readTimeVars();
    _readProjectionVars();
    _ReadSectorized_CMI();
  }
  catch (NcxxException e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  Cannot read all the variables." << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}

/*********************************************************************
 * _readCoordinateVars()
 */

void GoesRNoaaPort2Mdv::_readCoordinateVars() {
  static const string methodName = "GoesRNoaaPort2Mdv::_readCoordinateVars()";

  NcxxDim xDim;
  NcxxDim yDim;
  NcxxVar xVar;
  NcxxVar yVar;
  string xDimStr = X_DIM;
  string yDimStr = Y_DIM;
  _file.getCoordVar(xDimStr, xDim, xVar);
  _file.getCoordVar(yDimStr, yDim, yVar);

  if ((xDim.isNull() == true) || (xVar.isNull() == true)) {
    NcxxErrStr err;
    string info = ERROR_STR + methodName;
    err.addErrStr(info);
    cerr << "  X coordinate is missing. dimension: " << X_DIM << " variable: "
         << X_COORD << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  std::map<std::string, NcxxVarAtt> xCoordAtts;
  _nx = int(xDim.getSize());
  xCoordAtts = xVar.getAtts();
  float scaleFactor;
  float offset;
  try {
    xCoordAtts[SCALE_FACTOR].getValues(&scaleFactor);
    xCoordAtts[ADD_OFFSET].getValues(&offset);
  }
  catch (NcxxException &e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  _dxRad = fabs(scaleFactor * MICRORAD_TO_RAD);
  _minxRad = offset * MICRORAD_TO_RAD;

  if (yDim.isNull() || yVar.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  Y coordinate is missing. dimension: " << Y_DIM << " variable: "
         << Y_COORD << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  std::map<std::string, NcxxVarAtt> yCoordAtts;
  _ny = int(yDim.getSize());
  yCoordAtts = yVar.getAtts();
  try {
    yCoordAtts[SCALE_FACTOR].getValues(&scaleFactor);
    yCoordAtts[ADD_OFFSET].getValues(&offset);
  }
  catch (NcxxException e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr("  exception: ", e.what());
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  _dyRad = fabs(scaleFactor * MICRORAD_TO_RAD);
  _minyRad = offset * MICRORAD_TO_RAD;

  if (_params->debug == Params::DEBUG_VERBOSE) {
    cerr << "  NX: " << _nx << endl;
    cerr << "  NY: " << _ny << endl;
    cerr << "  minxRad: " << _minxRad << endl;
    cerr << "  minyRad: " << _minyRad << endl;
  }

}


/*********************************************************************
 * _readTimeVars()
 */

void GoesRNoaaPort2Mdv::_readTimeVars() {
  static const string methodName = "GoesRNoaaPort2Mdv::_readTimeVars()";

  NcxxVar varT = _file.getVar(TIME);
  if (varT.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << TIME << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  double val;
  varT.getVal(&val);
  _beginTime = J2017_EPOCH_START + static_cast< time_t >(round(val));

}

/*********************************************************************
 * _readProjectionVars()
 */

void GoesRNoaaPort2Mdv::_readProjectionVars() {
  static const string methodName = "GoesRNoaaPort2Mdv::_readProjectionVars()";

  switch (_projectionType) {
    case PROJECTION_LAMBERT_CONFORMAL: {

      NcxxVar varG = _file.getVar(LAMBERT_PROJECTION);
      if (varG.isNull()) {
        NcxxErrStr err;
        stringstream info;
        info << ERROR_STR << methodName << endl;
        info << "  cannot find variable, name: " << LAMBERT_PROJECTION << endl;
        err.addErrStr(info.str());
        cerr << endl;
        cerr << err.getErrStr() << endl;
        throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }

      // pull out attributes from this variable
      std::map<std::string, NcxxVarAtt> imgProjAtts;
      imgProjAtts = varG.getAtts();
      try {
        imgProjAtts[GRID_MAPPING_NAME].getValues(_gridMappingName);
        imgProjAtts[STANDARD_PARALLEL].getValues(&_standardParallel);
        imgProjAtts[SEMI_MAJOR_AXIS].getValues(&_semiMajorAxis);
        imgProjAtts[SEMI_MINOR_AXIS].getValues(&_semiMinorAxis);
        imgProjAtts[LATITUDE_OF_PROJECTION_ORIGIN].getValues(&_projectionOriginLatitude);
        imgProjAtts[LONGITUDE_OF_CENTRAL_MERIDIAN].getValues(&_projectionOriginLongitude);
      }
      catch (NcxxException &e) {
        NcxxErrStr err;
        stringstream info;
        info << ERROR_STR << methodName << endl;
        info << "   exception: " << e.what() << endl;
        err.addErrStr(info.str());
        cerr << endl;
        cerr << err.getErrStr() << endl;
        throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      break;

    }

    case PROJECTION_MERCATOR: {
      NcxxVar varG = _file.getVar(MERCATOR_PROJECTION);
      if (varG.isNull()) {
        NcxxErrStr err;
        stringstream info;
        info << ERROR_STR << methodName << endl;
        info << "  cannot find variable, name: " << MERCATOR_PROJECTION << endl;
        err.addErrStr(info.str());
        cerr << endl;
        cerr << err.getErrStr() << endl;
        throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }

      // pull out attributes from this variable
      std::map<std::string, NcxxVarAtt> imgProjAtts;
      imgProjAtts = varG.getAtts();
      try {
        imgProjAtts[GRID_MAPPING_NAME].getValues(_gridMappingName);
        imgProjAtts[STANDARD_PARALLEL].getValues(&_standardParallel);
        imgProjAtts[SEMI_MAJOR_AXIS].getValues(&_semiMajorAxis);
        imgProjAtts[SEMI_MINOR_AXIS].getValues(&_semiMinorAxis);
        imgProjAtts[LONGITUDE_OF_PROJECTION_ORIGIN].getValues(&_projectionOriginLongitude);
      }
      catch (NcxxException &e) {
        NcxxErrStr err;
        stringstream info;
        info << ERROR_STR << methodName << endl;
        info << "   exception: " << e.what() << endl;
        err.addErrStr(info.str());
        cerr << endl;
        cerr << err.getErrStr() << endl;
        throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
      break;

    }

    case PROJECTION_FIXED_GRID: {
      NcxxVar varG = _file.getVar(FIXEDGRID_PROJECTION);
      if (varG.isNull()) {
        NcxxErrStr err;
        stringstream info;
        info << ERROR_STR << methodName << endl;
        info << "  cannot find variable, name: " << FIXEDGRID_PROJECTION << endl;
        err.addErrStr(info.str());
        cerr << endl;
        cerr << err.getErrStr() << endl;
        throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }

      // pull out attributes from this variable
      std::map<std::string, NcxxVarAtt> imgProjAtts;
      imgProjAtts = varG.getAtts();
      try {
        imgProjAtts[GRID_MAPPING_NAME].getValues(_gridMappingName);
        imgProjAtts[SEMI_MAJOR_AXIS].getValues(&_semiMajorAxis);
        imgProjAtts[SEMI_MINOR_AXIS].getValues(&_semiMinorAxis);
        imgProjAtts[PERSPECTIVE_POINT_HEIGHT].getValues(&_perspectivePointHeight);
        imgProjAtts[LATITUDE_OF_PROJECTION_ORIGIN].getValues(&_projectionOriginLatitude);
        imgProjAtts[LONGITUDE_OF_PROJECTION_ORIGIN].getValues(&_projectionOriginLongitude);
      }
      catch (NcxxException &e) {
        NcxxErrStr err;
        stringstream info;
        info << ERROR_STR << methodName << endl;
        info << "   exception: " << e.what() << endl;
        err.addErrStr(info.str());
        cerr << endl;
        cerr << err.getErrStr() << endl;
        throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }

      float flatten = float( 1.0 ) / INVERSE_FLATTENING;
      _ecc = float( sqrt(2.0 * flatten - flatten * flatten) );
      _radiusRatio2 = float( pow((_semiMajorAxis / _semiMinorAxis), 2.0) );
      _invRadiusRatio2 = float( 1.0 / _radiusRatio2 );
      _H = _perspectivePointHeight + _semiMajorAxis;

      break;
    }
  }

  if (_params->debug == Params::DEBUG_VERBOSE) {
    cerr << "  PROJECTION: " << _gridMappingName << endl;
    cerr << "  LATITUDE_OF_PROJECTION_ORIGIN: " << _projectionOriginLatitude << endl;
    cerr << "  LONGITUDE_OF_PROJECTION_ORIGIN: " << _projectionOriginLongitude << endl;
    cerr << "  STANDARD_PARALLEL: " << _standardParallel << endl;
    cerr << "  EARTH RADIUS: " << _semiMajorAxis * 0.001 << endl;
  }
}

/*********************************************************************
 * _readVars()
 */

void GoesRNoaaPort2Mdv::_ReadSectorized_CMI() {
  static const string methodName = "GoesRNoaaPort2Mdv::_ReadSectorized_CMI()";

  NcxxVar varR = _file.getVar(SECTORIZED_CMI);
  if (varR.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << SECTORIZED_CMI << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  std::map<std::string, NcxxVarAtt> radAtts;
  radAtts = varR.getAtts();
  float scaleFactor;
  float offset;
  try {
    radAtts[SCALE_FACTOR].getValues(&scaleFactor);
    radAtts[ADD_OFFSET].getValues(&offset);
    radAtts[FILL_VALUE].getValues(&_ncRadMissingVal);
  }
  catch (NcxxException &e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "   exception: " << e.what() << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  size_t nPts = size_t(_nx * _ny);
  short *svals = new short[nPts];
  varR.getVal(svals);

  if (_params->init_zero == true) {
    _Sectorized_CMI.assign(nPts, 0.0);
  } else {
    _Sectorized_CMI.assign(nPts, MISSING_DATA_VALUE);
  }

  for (size_t i = 0; i < nPts; i++) {
    if (svals[i] != _ncRadMissingVal) {
      _Sectorized_CMI[i] = static_cast<float>(svals[i]) * scaleFactor + offset;

      // somehow small negative radiances values can be computed for svals that are
      // less than offset/scaleFactor. 
      if (_Sectorized_CMI[i] < 0.0) {
        _Sectorized_CMI[i] = 0.0;
      }
    } else {
      //      cerr << "got a missing value here" << endl;
    }
  }
  delete[] svals;

}

/*********************************************************************
 * _setProductType()
 */

void GoesRNoaaPort2Mdv::_setProductType(const string &title) {
  static const string methodName = "GoesRNoaaPort2Mdv::_setProductType()";

  _productType = PRODUCT_SECTORIZED_IMAGERY;

  if (_params->debug == Params::DEBUG_VERBOSE) {
    cerr << endl;
    cerr << "PRODUCT_NAME: " << title << endl;
    cerr << endl;
  }

}

/*********************************************************************
 * _setScanType()
 */

void GoesRNoaaPort2Mdv::_setScanType(const string &scan_id) {
  static const string methodName = "GoesRNoaaPort2Mdv::_setScanType()";

  if (scan_id == FULL_DISK_SCAN_NAME) {
    _scanType = SCAN_FULL_DISK;
  } else if (scan_id == CONUS_SCAN_NAME) {
    _scanType = SCAN_CONUS;
  } else if (scan_id == MESOSCALE_1_SCAN_NAME) {
    _scanType = SCAN_MESOSCALE_1;
  } else if (scan_id == MESOSCALE_2_SCAN_NAME) {
    _scanType = SCAN_MESOSCALE_2;
  } else {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  Unknown scan ID." << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}

/*********************************************************************
 * _setProjectionType()
 */

void GoesRNoaaPort2Mdv::_setProjectionType(const string &projection) {
  static const string methodName = "GoesRNoaaPort2Mdv::_setProjectionType()";

  if (projection == LAMBERT) {
    _projectionType = PROJECTION_LAMBERT_CONFORMAL;
  } else if (projection == MERCATOR) {
    _projectionType = PROJECTION_MERCATOR;
  } else if (projection == FIXED_GRID) {
    _projectionType = PROJECTION_FIXED_GRID;
  } else {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  Unknown projection: " << projection << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}

/*********************************************************************
 * _updateMasterHeader()
 */

void GoesRNoaaPort2Mdv::_updateMasterHeader(DsMdvx &mdvx) {
  static const string method_name = "GoesRNoaaPort2Mdv::_updateMasterHeader()";

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));

  master_hdr.time_gen = si32( time(NULL) );
  master_hdr.time_begin = si32( _beginTime );
  master_hdr.time_end = si32( _beginTime );
  master_hdr.time_centroid = si32( _beginTime );

  master_hdr.time_expire = si32( master_hdr.time_end + static_cast< time_t >(_params->expire_offset) );
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = _outputVlevelType;
  master_hdr.vlevel_type = _outputVlevelType;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.sensor_lon = _nominalSatSubpointLon;
  master_hdr.sensor_lat = _nominalSatSubpointLat;
  master_hdr.sensor_alt = _nominalSatHeight;
  STRcopy(master_hdr.data_set_info, _progName.c_str(), MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, _progName.c_str(), MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _progName.c_str(), MDV_NAME_LEN);

  mdvx.setMasterHeader(master_hdr);
}

/*********************************************************************
 * _latLon2XY()
 */

bool GoesRNoaaPort2Mdv::_latLon2XY(float lat, float lon,
                                   int &x_idx, int &y_idx) {
  static const string methodName = "GoesRnetCDF2Mdv::_latLon2XY()";

  float c_lat = float( atan(_invRadiusRatio2 * tan(lat * DEG_TO_RAD)) );
  float cos_clat = cos(c_lat);

  float rc = float( _semiMinorAxis / sqrt(1.0 - pow(_ecc * cos_clat, 2.0)) );

  float del_lon_angle = float( (lon - _projectionOriginLongitude) * DEG_TO_RAD );

  float sx = _H - (rc * cos_clat * cos(del_lon_angle));
  float sy = -rc * cos_clat * sin(del_lon_angle);
  float sz = rc * sin(c_lat);

  // // check that point is on disk of the earth
  if ((_H * (_H - sx)) < (sy * sy + _radiusRatio2 * sz * sz)) {
    x_idx = -1;
    y_idx = -1;
    return false;
  }

  float rl = sqrt((sx * sx + sy * sy + sz * sz));
  float xx = asin((-sy / rl));
  float yy = atan((sz / sx));


  x_idx = round((xx - _minxRad) / _dxRad);
  y_idx = round((_minyRad - yy) / _dyRad);

  return true;
}



/////////////////////////////////////////////////////////////////////////////////////
// string constant instantiation
//

const char *GoesRNoaaPort2Mdv::ERROR_STR = "ERROR - ";
const char *GoesRNoaaPort2Mdv::WARN_STR = "WARNING - ";
const char *GoesRNoaaPort2Mdv::INFO_STR = "INFO - ";

// Product Dimensions
const char *GoesRNoaaPort2Mdv::Y_DIM = "y";
const char *GoesRNoaaPort2Mdv::X_DIM = "x";


// Global Attributes
const char *GoesRNoaaPort2Mdv::TITLE = "title";
const char *GoesRNoaaPort2Mdv::ICD_VERSION = "ICD_version";
const char *GoesRNoaaPort2Mdv::CONVENTIONS = "Conventions";
const char *GoesRNoaaPort2Mdv::CHANNEL_ID = "channel_id";
const char *GoesRNoaaPort2Mdv::CENTRAL_WAVELENGTH = "central_wavelength";
const char *GoesRNoaaPort2Mdv::ABI_MODE = "abi_mode";
const char *GoesRNoaaPort2Mdv::SOURCE_SCENE = "source_scene";
const char *GoesRNoaaPort2Mdv::PERIODICITY = "periodicity";
const char *GoesRNoaaPort2Mdv::PRODUCTION_LOCATION = "production_location";
const char *GoesRNoaaPort2Mdv::PRODUCT_NAME = "product_name";
const char *GoesRNoaaPort2Mdv::SATELLITE_ID = "satellite_id";
const char *GoesRNoaaPort2Mdv::PRODUCT_CENTER_LAT = "product_center_latitude";
const char *GoesRNoaaPort2Mdv::PRODUCT_CENTER_LON = "product_center_longitude";
const char *GoesRNoaaPort2Mdv::PROJECTION = "projection";
const char *GoesRNoaaPort2Mdv::BIT_DEPTH = "bit_depth";
const char *GoesRNoaaPort2Mdv::SOURCE_SPATIAL_RES = "source_spatial_resolution";
const char *GoesRNoaaPort2Mdv::REQUEST_SPATIAL_RES = "request_spatial_resolution";
const char *GoesRNoaaPort2Mdv::START_DATE_TIME = "start_date_time";
const char *GoesRNoaaPort2Mdv::NUMBER_PRODUCT_TILES = "number_product_tiles";
const char *GoesRNoaaPort2Mdv::PRODUCT_TILE_WIDTH = "product_tile_width";
const char *GoesRNoaaPort2Mdv::PRODUCT_TILE_HEIGHT = "product_tile_height";
const char *GoesRNoaaPort2Mdv::PRODUCT_ROWS = "product_rows";
const char *GoesRNoaaPort2Mdv::PRODUCT_COLUMNS = "product_columns";
const char *GoesRNoaaPort2Mdv::PIXEL_X_SIZE = "pixel_x_size";
const char *GoesRNoaaPort2Mdv::PIXEL_Y_SIZE = "pixel_y_size";
const char *GoesRNoaaPort2Mdv::SATELLITE_LAT = "satellite_latitude";
const char *GoesRNoaaPort2Mdv::SATELLITE_LON = "satellite_longitude";
const char *GoesRNoaaPort2Mdv::SATELLITE_ALT = "satellite_altitude";
const char *GoesRNoaaPort2Mdv::CREATED_BY = "created_by";
const char *GoesRNoaaPort2Mdv::PRODUCT_TILES_RECEIVED = "product_tiles_received";

// Variables
const char *GoesRNoaaPort2Mdv::X_COORD = "x";
const char *GoesRNoaaPort2Mdv::Y_COORD = "y";
const char *GoesRNoaaPort2Mdv::TIME = "time";
const char *GoesRNoaaPort2Mdv::LAMBERT_PROJECTION = "lambert_projection";
const char *GoesRNoaaPort2Mdv::MERCATOR_PROJECTION = "mercator_projection";
const char *GoesRNoaaPort2Mdv::FIXEDGRID_PROJECTION = "fixedgrid_projection";
const char *GoesRNoaaPort2Mdv::SECTORIZED_CMI = "Sectorized_CMI";

// Variable Attributes
const char *GoesRNoaaPort2Mdv::FILL_VALUE = "_FillValue";
const char *GoesRNoaaPort2Mdv::STANDARD_NAME = "standard_name";
const char *GoesRNoaaPort2Mdv::UNITS = "units";
const char *GoesRNoaaPort2Mdv::GRID_MAPPING = "grid_mapping";
const char *GoesRNoaaPort2Mdv::ADD_OFFSET = "add_offset";
const char *GoesRNoaaPort2Mdv::SCALE_FACTOR = "scale_factor";
const char *GoesRNoaaPort2Mdv::VALID_MIN = "valid_min";
const char *GoesRNoaaPort2Mdv::VALID_MAX = "valid_max";
const char *GoesRNoaaPort2Mdv::COORDINATES = "coordinates";
const char *GoesRNoaaPort2Mdv::GRID_MAPPING_NAME = "grid_mapping_name";
const char *GoesRNoaaPort2Mdv::STANDARD_PARALLEL = "standard_parallel";
const char *GoesRNoaaPort2Mdv::LONGITUDE_OF_CENTRAL_MERIDIAN = "longitude_of_central_meridian";
const char *GoesRNoaaPort2Mdv::LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
const char *GoesRNoaaPort2Mdv::LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
const char *GoesRNoaaPort2Mdv::SEMI_MAJOR_AXIS = "semi_major_axis";
const char *GoesRNoaaPort2Mdv::SEMI_MINOR_AXIS = "semi_minor_axis";
const char *GoesRNoaaPort2Mdv::PERSPECTIVE_POINT_HEIGHT = "perspective_point_height";

// strings to identify type of scan
const char *GoesRNoaaPort2Mdv::FULL_DISK_SCAN_NAME = "FullDisk";
const char *GoesRNoaaPort2Mdv::CONUS_SCAN_NAME = "CONUS";
const char *GoesRNoaaPort2Mdv::MESOSCALE_1_SCAN_NAME = "Mesoscale-1";
const char *GoesRNoaaPort2Mdv::MESOSCALE_2_SCAN_NAME = "Mesoscale-2";

// strings to identify projection
const char *GoesRNoaaPort2Mdv::LAMBERT = "Lambert Conformal";
const char *GoesRNoaaPort2Mdv::MERCATOR = "Mercator";
const char *GoesRNoaaPort2Mdv::FIXED_GRID = "Fixed Grid";

