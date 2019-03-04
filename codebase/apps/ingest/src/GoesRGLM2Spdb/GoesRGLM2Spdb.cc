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
 * @file GoesRGLM2Spdb.cc
 *
 * @class GoesRGLM2Spdb
 *
 * GoesRGLM2Spdb is the top level application class.
 *  
 * @date 4/12/2018
 *
 */

#include <iostream>
#include <sstream>
#include <cmath>

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
#include <Spdb/DsSpdb.hh>
#include <rapformats/ltg.h>


#include "GoesRGLM2Spdb.hh"
#include "Params.hh"

using namespace std;


// Global variables

GoesRGLM2Spdb *GoesRGLM2Spdb::_instance = (GoesRGLM2Spdb *) NULL;

// start of epoch for GOES-R is 2000-01-01 12:00:00, the J2000 epoch
// value determined by date --date='2000-01-01 12:00:00 UTC' +%s
const time_t GoesRGLM2Spdb::J2000_EPOCH_START = 946728000;


/*********************************************************************
 * Constructor
 */

GoesRGLM2Spdb::GoesRGLM2Spdb(int argc, char **argv) :
  _outputVlevelType(Mdvx::VERT_SATELLITE_IMAGE) {
  static const string method_name = "GoesRGLM2Spdb::GoesRGLM2Spdb()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (GoesRGLM2Spdb *) NULL);

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

GoesRGLM2Spdb::~GoesRGLM2Spdb() {
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

GoesRGLM2Spdb *GoesRGLM2Spdb::Inst(int argc, char **argv) {
  if (_instance == (GoesRGLM2Spdb *) NULL)
    new GoesRGLM2Spdb(argc, argv);

  return (_instance);
}

GoesRGLM2Spdb *GoesRGLM2Spdb::Inst() {
  assert(_instance != (GoesRGLM2Spdb *) NULL);

  return (_instance);
}

/*********************************************************************
 * init()
 */

bool GoesRGLM2Spdb::init() {
  static const string method_name = "GoesRGLM2Spdb::init()";

  // initialize process registration

  PMU_auto_init(_progName.c_str(), _params->instance,
                _params->procmap_reg_interval);

  return true;
}

void GoesRGLM2Spdb::run() {
  static const string method_name = "GoesRGLM2Spdb::run()";

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
 * _processData()
 */

bool GoesRGLM2Spdb::_processData() {
  static const string methodName = "GoesRGLM2Spdb::_processData()";

  // load up strike
  int numAdded = 0;
  MemBuf strikeBuffer;
  unsigned short product_remove_count = 0;
  unsigned long quality_control_remove_count = 0;

  int good_count = 0;
  int bad_count = 0;

  vector<flash_properties> flash_info;

  // If user defined quality control is requested, then figure out
  // which flashes and corresponding groups and events are of low
  // quality.
  if (_params->quality_control.do_user_defined_quality_control) {
    for (size_t i = 0; i < _flashCount; i++) {

      flash_properties thisFlash;
      thisFlash.flash_id = _productFlashId[i];

      // loop through the group parent flash ID's and get the
      // groups that are a part of this flash
      for (size_t gID = 0; gID < _groupCount; gID++) {
        if (_productGroupParentFlashId[gID] == _productFlashId[i]) {
          thisFlash.group_ids.push_back(_productGroupId[gID]);
        }
      }

      thisFlash.events_in_group.assign(thisFlash.group_ids.size(), 0);

      // loop through the groups that are contained in this flash
      // and get number of events for each group
      for (size_t gif = 0; gif < thisFlash.group_ids.size(); gif++) {
        thisFlash.events_in_group[gif] = 0;
        for (size_t x = 0; x < _eventCount; x++) {
          if (_productEventParentGroupId[x] == thisFlash.group_ids[gif]) {
            thisFlash.events_in_group[gif]++;
          }
        }

      }

      // get the number of groups in the flash that has less than or equal to the number
      // of events allowed in the quality control parameter.
      size_t groups_below_threshold = 0;
      for (size_t x = 0; x < thisFlash.group_ids.size(); x++) {
        if ((int) thisFlash.events_in_group[x] <= _params->quality_control.number_of_events) {
          groups_below_threshold++;
        }
      }

      // flag the flash as low quality if the flash has as many or more groups than the
      // number allowed in the quality control parameter that are less than the
      // number of events allowed in the quality control parameter
      // OR
      // There are fewer groups in the flash than the quality control number of groups
      // AND all of these groups have fewer events than the that allowed in the quality
      // control parameter.
/*
      if (groups_below_threshold >= _params->quality_control.number_of_groups ||
          (thisFlash.events_in_group.size() < _params->quality_control.number_of_groups &&
           groups_below_threshold == thisFlash.events_in_group.size() )
        ) {
        thisFlash.calculatedFlashQuality = 1;
        bad_count++;
*/
      // flag the flash as low quality if the flash has not more than the number_of_groups
      // set in the quality control parameter, and all of these groups have not more than
      // the number_of_events set in the quality control parameter.
      if ((int) thisFlash.group_ids.size() <= _params->quality_control.number_of_groups &&
           groups_below_threshold == thisFlash.group_ids.size()) {
        thisFlash.calculatedFlashQuality = 1;
        bad_count++;
      } else {
        thisFlash.calculatedFlashQuality = 0;
        good_count++;
      }

      if (_params->debug == Params::DEBUG_VERBOSE) {
        cerr << "FLASH INFO:" << endl;
        cerr << " ID: " << thisFlash.flash_id << endl;
        for (size_t w = 0; w < thisFlash.group_ids.size(); w++) {
          cerr << " GROUP ID: " << thisFlash.group_ids[w] << endl;
          cerr << " GROUP Event count: " << thisFlash.events_in_group[w] << endl;
        }
        if (thisFlash.calculatedFlashQuality == 0) {
          cerr << " Flash quality: GOOD quality\n";
        }
        else{
          cerr << " Flash quality: POOR quality\n";
        }
        cerr << endl;
      }

      // add this flash to the vector of flashes
      flash_info.push_back(thisFlash);
    }
  }

  bool remove_strike = false;

  for (size_t i = 0; i < _productCount; i++) {

    // remove strikes based on quality flags in the data file
    for (ssize_t dq = 0; dq < _params->quality_values_to_remove_n; dq++) {
      if (_productQualityFlag[i] == _params->_quality_values_to_remove[dq]) {
        remove_strike = true;
        product_remove_count++;
        if (_params->debug >= Params::DEBUG_NORM) {
          cerr << "Removed strike with quality flag " << _productQualityFlag[i] << endl;
        }
        break;
      }
    }

    // If user defined quality control is requested, then skip flashes, groups
    // or events that where flagged as low quality and not removed by quality flag.
    if (_params->quality_control.do_user_defined_quality_control &&
        remove_strike == false) {
      if (_params->data_type == Params::FLASHES) {
        // skip flashes that have been flagged as low quality
        if (flash_info[i].calculatedFlashQuality == 1) {
          remove_strike = true;
          quality_control_remove_count++;
        }
      } else if (_params->data_type == Params::GROUPS) {

        // Skip groups that are a part of flashes that have been flagged as low quality
        for (size_t x = 0; x < flash_info.size(); x++) {
          if (remove_strike) {
            break;
          }
          if (_productGroupParentFlashId[i] == flash_info[x].flash_id &&
              flash_info[x].calculatedFlashQuality == 1) {
            remove_strike = true;
            quality_control_remove_count++;
            break;
          }
        }
      } else { // Params::EVENTS

        // skip events that are a part of flashes that have been flagged as bad
        for (size_t x = 0; x < flash_info.size(); x++) {
          if (remove_strike) {
            break;
          }
          for (size_t y = 0; y < flash_info[x].group_ids.size(); y++) {
            if (_productEventParentGroupId[i] == flash_info[x].group_ids[y] &&
                flash_info[x].calculatedFlashQuality == 1) {
              remove_strike = true;
              quality_control_remove_count++;
              break;
            }
          }
        }
      }
    }

    if (remove_strike) {

      remove_strike = false;
      continue;

    }

    // check location

    double latitude = _productLat[i];
    double longitude = _productLon[i];

    bool validLocation = true;
    
    // check bounding box
    
    if (_params->limit_location_to_bounding_box) {
      if (latitude < _params->bounding_box.min_lat ||
          latitude > _params->bounding_box.max_lat) {
        validLocation = false;
      }
      if (longitude < _params->bounding_box.min_lon ||
          longitude > _params->bounding_box.max_lon) {
        validLocation = false;
      }
    }

    // also check for reasonable values

    if (fabs(latitude) > 90.0) {
      validLocation = false;
    }
    if (fabs(longitude) > 90.0) {
      validLocation = false;
    }
    if (latitude != 0 && fabs(latitude) < 1.0e-6) {
      validLocation = false;
    }
    if (longitude != 0 && fabs(longitude) < 1.0e-6) {
      validLocation = false;
    }

    if (validLocation) {

      time_t utime = _productTime[i];
      LTG_extended_t strike;
      LTG_init_extended(&strike);
      strike.time = (si32) utime;
      strike.latitude = (fl32) _productLat[i];
      strike.longitude = (fl32) _productLon[i];
      strike.altitude = 0.0;
      strike.amplitude = 0.0;
      
      if (_params->debug >= Params::DEBUG_VERBOSE) {
        LTG_print_extended(stderr, &strike);
      }
      
      LTG_extended_t copy = strike;
      LTG_extended_to_BE(&copy);
      
      strikeBuffer.add(&copy, sizeof(copy));
      numAdded++;

    }
    
  }

  if (_params->debug >= Params::DEBUG_NORM){
    if (_params->data_type == Params::EVENTS){
      cerr << endl;
      cerr << "Processed " << _productCount << " Events." << endl;
      if (_params->quality_control.do_user_defined_quality_control ) {
        cerr << "- Removed " << quality_control_remove_count << " low quality events." << endl;
        cerr << "- Total removed: " << quality_control_remove_count << endl;
      }
      cerr << "- Total written: " << _productCount - quality_control_remove_count << endl;
      cerr << endl;
    }
    else if (_params->data_type == Params::GROUPS){
      cerr << endl;
      cerr << "Processed " << _productCount << " Groups." << endl;
      if (_params->quality_values_to_remove_n > 0 ) {
        cerr << "- Removed " << product_remove_count << " groups based on the quality flag." << endl;
      }
      if (_params->quality_control.do_user_defined_quality_control ) {
        cerr << "- Removed " << quality_control_remove_count << " low quality groups." << endl;
      }
      if (_params->quality_control.do_user_defined_quality_control || _params->quality_values_to_remove_n > 0) {
        cerr << "- Total removed: " << product_remove_count + quality_control_remove_count << endl;
      }
      cerr << "- Total written: " << _productCount - (product_remove_count + quality_control_remove_count) << endl;
      cerr << endl;
    }
    else if (_params->data_type == Params::FLASHES){
      cerr << endl;
      cerr << "Processed " << _productCount << " Flashes." << endl;
      if (_params->quality_values_to_remove_n > 0 ) {
        cerr << "- Removed " << product_remove_count << " flashes based on the quality flag." << endl;
      }
      if (_params->quality_control.do_user_defined_quality_control ) {
        cerr << "- Removed " << quality_control_remove_count << " low quality flashes." << endl;
      }
      if (_params->quality_control.do_user_defined_quality_control || _params->quality_values_to_remove_n > 0) {
        cerr << "- Total removed: " << product_remove_count + quality_control_remove_count << endl;
      }
      cerr << "- Total written: " << _productCount - (product_remove_count + quality_control_remove_count) << endl;
      cerr << endl;
    }

  }

  // Send the data to the database

  DsSpdb spdb;
  cerr << "Writing data for time: " << utimstr(_beginTime) << endl;

  if (spdb.put(_params->output_url,
               SPDB_LTG_ID, SPDB_LTG_LABEL,
               0,
               _beginTime,
               _beginTime + _params->expire_seconds,
               int(strikeBuffer.getLen()), strikeBuffer.getPtr())) {
    fprintf(stderr, "ERROR: GoesRGLM2Spdb::_processData\n");
    fprintf(stderr, "  Error writing ltg to URL <%s>\n", _params->output_url);
    fprintf(stderr, "%s\n", spdb.getErrStr().c_str());
    return false;
  }
  strikeBuffer.free();
  _clearAndReset();

  return true;
}


/*********************************************************************
 * _readFile()
 */

bool GoesRGLM2Spdb::_readFile(const std::string &file_path) {
  static const string method_name = "GoesRGLM2Spdb::_readFile()";

  // open file and read contents

  try {
    _file.open(file_path, NcxxFile::read);

    cerr << endl;
    cerr << "Read file " << file_path << endl;

//    _readGlobalAttributes();
    _readDimensions();
    if (!_readVariables() ){
      cerr << "ERROR reading variables." << endl;
      return false;
    }

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

void GoesRGLM2Spdb::_readGlobalAttributes() {
  static const string methodName = "GoesRGLM2Spdb::_readGlobalAttributes()";

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
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}


/*********************************************************************
 * _clearAndReset()
 */

void GoesRGLM2Spdb::_clearAndReset() {
  static const string methodName = "GoesRGLM2Spdb::_clearAndReset()";

  // clear memory held by vectors
  _productQualityFlag.clear();
  std::vector<short>(_productQualityFlag).swap(_productQualityFlag);

  _productLat.clear();
  std::vector<float>(_productLat).swap(_productLat);

  _productLon.clear();
  std::vector<float>(_productLon).swap(_productLon);

  _productTime.clear();
  std::vector<time_t>(_productTime).swap(_productTime);

  _productFlashId.clear();
  std::vector<unsigned long>(_productFlashId).swap(_productFlashId);

  _productGroupId.clear();
  std::vector<unsigned long>(_productGroupId).swap(_productGroupId);

  _productEventId.clear();
  std::vector<unsigned long>(_productEventId).swap(_productEventId);

  _productGroupParentFlashId.clear();
  std::vector<unsigned long>(_productGroupParentFlashId).swap(_productGroupParentFlashId);

  _productEventParentGroupId.clear();
  std::vector<unsigned long>(_productEventParentGroupId).swap(_productEventParentGroupId);

}
/*********************************************************************
 * _readDimensions()
 */

void GoesRGLM2Spdb::_readDimensions() {
  static const string methodName = "GoesRGLM2Spdb::_readDimensions()";

  try {
    NcxxDim dim = _file.getDim(NUMBER_OF_EVENTS);
    if (dim.isNull() == true) {
      NcxxErrStr err;
      stringstream info;
      info << ERROR_STR << methodName << endl;
      info << "  dimension is missing, name: " << NUMBER_OF_EVENTS << endl;
      err.addErrStr(info.str());
      throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));

    }
    _Dim_number_of_events = dim.getSize();
  }
  catch (NcxxException e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  exception: " << e.what() << endl;
    err.addErrStr(info.str());
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  try {
    NcxxDim dim = _file.getDim(NUMBER_OF_GROUPS);
    if (dim.isNull() == true) {
      NcxxErrStr err;
      stringstream info;
      info << ERROR_STR << methodName << endl;
      info << "  dimension is missing, name: " << NUMBER_OF_EVENTS << endl;
      err.addErrStr(info.str());
      throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));

    }
    _Dim_number_of_groups = dim.getSize();
  }
  catch (NcxxException e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  exception: " << e.what() << endl;
    err.addErrStr(info.str());
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  try {
    NcxxDim dim = _file.getDim(NUMBER_OF_FLASHES);
    if (dim.isNull() == true) {
      NcxxErrStr err;
      stringstream info;
      info << ERROR_STR << methodName << endl;
      info << "  dimension is missing, name: " << NUMBER_OF_EVENTS << endl;
      err.addErrStr(info.str());
      throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));

    }
    _Dim_number_of_flashes = dim.getSize();
  }
  catch (NcxxException e) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  exception: " << e.what() << endl;
    err.addErrStr(info.str());
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}

/*********************************************************************
 * _readVariables()
 */

bool GoesRGLM2Spdb::_readVariables() {
  static const string methodName = "GoesRGLM2Spdb::_readVariables()";

  try {

    // For quality control all counts and ID's are needed
    // Check if flag set before doing this?

    if (! _readCountVars(EVENT_COUNT, _eventCount) ) {
      cerr << "ERROR: reading count variable."<< endl;
      return false;
    }
    else if (_eventCount != _Dim_number_of_events){
//      cerr << "\nWARNING: number_of_events dimension does not match variable event_count.\n";
      cerr << "\nERROR: number_of_events dimension does not match variable event_count.\n";
      cerr << "- number_of_events = " << _Dim_number_of_events << endl;
      cerr << "- event_count = " << _eventCount << endl;
      cerr << endl;
//      _eventCount = _Dim_number_of_events;
      return false;
    }

    if (! _readCountVars(GROUP_COUNT, _groupCount) ) {
      cerr << "ERROR reading count variable."<< endl;
      return false;
    }
    else if (_groupCount != _Dim_number_of_groups){
//      cerr << "\nWARNING: number_of_events dimension does not match variable event_count.\n";
      cerr << "\nERROR: number_of_events dimension does not match variable event_count.\n";
      cerr << "- number_of_events = " << _Dim_number_of_events << endl;
      cerr << "- event_count = " << _eventCount << endl;
      cerr << endl;
//      _groupCount = _Dim_number_of_groups;
      return false;
    }

    if (! _readCountVars(FLASH_COUNT, _flashCount) ) {
      cerr << "ERROR reading count variable."<< endl;
      return false;
    }
    else if (_flashCount != _Dim_number_of_flashes){
//      cerr << "\nWARNING: number_of_events dimension does not match variable event_count.\n";
      cerr << "\nERROR: number_of_events dimension does not match variable event_count.\n";
      cerr << "- number_of_events = " << _Dim_number_of_events << endl;
      cerr << "- event_count = " << _eventCount << endl;
      cerr << endl;
//      _flashCount = _Dim_number_of_flashes;
      return false;
    }

    _readIdVars(FLASH_ID, _flashCount, _productFlashId);
    _readIdVars(GROUP_ID, _groupCount, _productGroupId);
    _readIdVars(EVENT_ID, _eventCount, _productEventId);
    _readIdVars(EVENT_PARENT_GROUP_ID, _eventCount, _productEventParentGroupId);
    _readIdVars(GROUP_PARENT_FLASH_ID, _groupCount, _productGroupParentFlashId);


    switch (_params->data_type) {
      case Params::EVENTS : {

        _productCount = _eventCount;

        if (! _readTimeVars(EVENT_TIME_OFFSET) ){
          cerr << "ERROR reading product time." << endl;
          return false;
        }
        _readCoordinateVars(EVENT_LAT, EVENT_LON);

        // No data quality flag for events so set to 0 or good data quality
        _readDataQualityFlag(MISSING_QUALITY_FLAG);

        break;
      }
      case Params::GROUPS : {

        _productCount = _groupCount;

        if (! _readTimeVars(GROUP_TIME_OFFSET) ){
          cerr << "ERROR reading product time." << endl;
          return false;
        }
        _readCoordinateVars(GROUP_LAT, GROUP_LON);
        _readDataQualityFlag(GROUP_QUALITY_FLAG);

        break;
      }
      case Params::FLASHES : {

        _productCount = _flashCount;

        if (_params->debug >= Params::DEBUG_NORM) {
          cerr << endl;
          cerr << "Processing Flashes\n";
          cerr << "Number of flashes = " << _productCount << endl;
          cerr << endl;
        }

        if (! _readTimeVars(FLASH_TIME_OFFSET_LAST_EVENT) ){
          cerr << "ERROR reading product time." << endl;
          return false;
        }
        _readCoordinateVars(FLASH_LAT, FLASH_LON);
        _readDataQualityFlag(FLASH_QUALITY_FLAG);

        break;
      }
    }

  return true;
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

void GoesRGLM2Spdb::_readCoordinateVars(const char *LatVar, const char *LonVar) {
  static const string methodName = "GoesRGLM2Spdb::_readCoordinateVars()";

  NcxxVar varLAT = _file.getVar(LatVar);
  if (varLAT.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << EVENT_LAT << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  NcxxVar varLON = _file.getVar(LonVar);
  if (varLON.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << EVENT_LON << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  if (_params->data_type == Params::EVENTS){
    std::map<std::string, NcxxVarAtt> radAtts;
    radAtts = varLAT.getAtts();
    float scaleFactor;
    float offset;
    try {
      radAtts[SCALE_FACTOR].getValues(&scaleFactor);
      radAtts[ADD_OFFSET].getValues(&offset);
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

    short *latVals = new short[_productCount];
    varLAT.getVal(latVals);
    _productLat.assign(_productCount, 0);

    for (size_t i = 0; i < _productCount; i++) {
      _productLat[i] = static_cast<short>(latVals[i]) * scaleFactor + offset;
    }
    delete[] latVals;

    radAtts = varLON.getAtts();
    try {
      radAtts[SCALE_FACTOR].getValues(&scaleFactor);
      radAtts[ADD_OFFSET].getValues(&offset);
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

    short *lonVals = new short[_productCount];
    varLON.getVal(lonVals);
    _productLon.assign(_productCount, 0);

    for (size_t i = 0; i < _productCount; i++) {
      _productLon[i] = static_cast<short>(lonVals[i]) * scaleFactor + offset;
    }
    delete[] lonVals;

  }
  else {

    float *latVals = new float[_productCount];
    varLAT.getVal(latVals);
    _productLat.assign(_productCount, 0);
    for (size_t i = 0; i < _productCount; i++) {
      _productLat[i] = static_cast<float>(latVals[i]);
    }
    delete[] latVals;

    float *lonVals = new float[_productCount];
    varLON.getVal(lonVals);
    _productLon.assign(_productCount, 0);

    for (size_t i = 0; i < _productCount; i++) {
      _productLon[i] = static_cast<float>(lonVals[i]);
    }
    delete[] lonVals;

  }

}

/*********************************************************************
 * _readDataQualityVars()
 */

void GoesRGLM2Spdb::_readDataQualityFlag(const char *DataQualityVar) {
  static const string methodName = "GoesRGLM2Spdb::_readDataQualityVars()";

  // EVENTS don't have a quality flag so will set all values to 0 or good data quality.
  if (! strcmp(DataQualityVar,"none") ) {
    _productQualityFlag.assign(_productCount, 0);
  }
  else {

    // Get the time offset
    NcxxVar varPTO = _file.getVar(DataQualityVar);
    if (varPTO.isNull()) {
      NcxxErrStr err;
      stringstream info;
      info << ERROR_STR << methodName << endl;
      info << "  cannot find variable, name: " << DataQualityVar << endl;
      err.addErrStr(info.str());
      cerr << endl;
      cerr << err.getErrStr() << endl;
      throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }

    short *svals = new short[_productCount];
    varPTO.getVal(svals);
    _productQualityFlag.assign(_productCount, 0);

    for (size_t i = 0; i < _productCount; i++) {

      // convert from milliseconds to seconds.
      _productQualityFlag[i] = static_cast<short>(svals[i]);
    }
    delete[] svals;
  }
}

/*********************************************************************
 * _readIdVars()
 */

void GoesRGLM2Spdb::_readIdVars(const char *Id, size_t count, vector<unsigned long>&ID) {
  static const string methodName = "GoesRGLM2Spdb::_readDataQualityVars()";

  // Get the time offset
  NcxxVar varPTO = _file.getVar(Id);
  if (varPTO.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << Id << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  long *svals = new long[count];
  varPTO.getVal(svals);
  ID.assign(count, 0);


  for (size_t i = 0; i < count; i++) {

    // convert from milliseconds to seconds.
    ID[i] = static_cast<unsigned long>(svals[i]);
  }
  delete[] svals;
}

/*********************************************************************
 * _readCountVars()
 */

bool GoesRGLM2Spdb::_readCountVars(const char *CountVar, size_t &ProductCount) {
  static const string methodName = "GoesRGLM2Spdb::_readCountVars()";

  NcxxVar varPC = _file.getVar(CountVar);
  if (varPC.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << CountVar << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  std::map<std::string, NcxxVarAtt> radAtts;
  radAtts = varPC.getAtts();
  vector<int> validRange;
  try {
    radAtts[VALID_RANGE].getValues(validRange);
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

  ssize_t count = 0;
  varPC.getVal(&count);
  if (count < validRange[0] || count > validRange[1]){
    cerr << endl;
    cerr << "Product count " << count << " outside valid range." << endl;
    cerr << "  Valid Range: " << validRange[0] << " - " << validRange[1] << endl;
    cerr << "  skipping this file." << endl;
    return false;
  }
  if (_params->debug >= Params::DEBUG_NORM) {
    cerr << CountVar << " = " << count << endl;
  }
  ProductCount = count;
  return true;
}

/*********************************************************************
 * _readTimeVars()
 */

  bool GoesRGLM2Spdb::_readTimeVars(const char *TimeOffsetVar) {
    static const string methodName = "GoesRGLM2Spdb::_readTimeVars()";

  NcxxVar varT = _file.getVar(PRODUCT_TIME);
  if (varT.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << PRODUCT_TIME << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  double val;
  varT.getVal(&val);
  if (val <= -999.0 ){
    cerr << "ERROR: BAD product time." << endl;
    return false;
  }
  _beginTime = J2000_EPOCH_START + static_cast< time_t >(round(val));

  // Get the time offset
  NcxxVar varPTO = _file.getVar(TimeOffsetVar);
  if (varPTO.isNull()) {
    NcxxErrStr err;
    stringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << TimeOffsetVar << endl;
    err.addErrStr(info.str());
    cerr << endl;
    cerr << err.getErrStr() << endl;
    throw (NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  std::map<std::string, NcxxVarAtt> radAtts;
  radAtts = varPTO.getAtts();
  float scaleFactor;
  float offset;
  try {
    radAtts[SCALE_FACTOR].getValues(&scaleFactor);
    radAtts[ADD_OFFSET].getValues(&offset);
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

  short *svals = new short[_productCount];
  varPTO.getVal(svals);
  _productTime.assign(_productCount, 0);

  for (size_t i = 0; i < _productCount; i++) {

    _productTime[i] = _beginTime + static_cast<time_t>(svals[i] * scaleFactor + offset);
  }
  delete[] svals;

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// string constant instantiation
//

const char *GoesRGLM2Spdb::ERROR_STR = "ERROR - ";
const char *GoesRGLM2Spdb::WARN_STR = "WARNING - ";
const char *GoesRGLM2Spdb::INFO_STR = "INFO - ";

// Global Attributes
const char *GoesRGLM2Spdb::TITLE = "title";
const char *GoesRGLM2Spdb::ICD_VERSION = "ICD_version";
const char *GoesRGLM2Spdb::CONVENTIONS = "Conventions";
const char *GoesRGLM2Spdb::CHANNEL_ID = "channel_id";
const char *GoesRGLM2Spdb::CENTRAL_WAVELENGTH = "central_wavelength";
const char *GoesRGLM2Spdb::ABI_MODE = "abi_mode";
const char *GoesRGLM2Spdb::SOURCE_SCENE = "source_scene";
const char *GoesRGLM2Spdb::PERIODICITY = "periodicity";
const char *GoesRGLM2Spdb::PRODUCTION_LOCATION = "production_location";
const char *GoesRGLM2Spdb::PRODUCT_NAME = "product_name";
const char *GoesRGLM2Spdb::SATELLITE_ID = "satellite_id";
const char *GoesRGLM2Spdb::PRODUCT_CENTER_LAT = "product_center_latitude";
const char *GoesRGLM2Spdb::PRODUCT_CENTER_LON = "product_center_longitude";
const char *GoesRGLM2Spdb::PROJECTION = "projection";
const char *GoesRGLM2Spdb::BIT_DEPTH = "bit_depth";
const char *GoesRGLM2Spdb::SOURCE_SPATIAL_RES = "source_spatial_resolution";
const char *GoesRGLM2Spdb::REQUEST_SPATIAL_RES = "request_spatial_resolution";
const char *GoesRGLM2Spdb::START_DATE_TIME = "start_date_time";
const char *GoesRGLM2Spdb::NUMBER_PRODUCT_TILES = "number_product_tiles";
const char *GoesRGLM2Spdb::PRODUCT_TILE_WIDTH = "product_tile_width";
const char *GoesRGLM2Spdb::PRODUCT_TILE_HEIGHT = "product_tile_height";
const char *GoesRGLM2Spdb::PRODUCT_ROWS = "product_rows";
const char *GoesRGLM2Spdb::PRODUCT_COLUMNS = "product_columns";
const char *GoesRGLM2Spdb::PIXEL_X_SIZE = "pixel_x_size";
const char *GoesRGLM2Spdb::PIXEL_Y_SIZE = "pixel_y_size";
const char *GoesRGLM2Spdb::SATELLITE_LAT = "satellite_latitude";
const char *GoesRGLM2Spdb::SATELLITE_LON = "satellite_longitude";
const char *GoesRGLM2Spdb::SATELLITE_ALT = "satellite_altitude";
const char *GoesRGLM2Spdb::CREATED_BY = "created_by";
const char *GoesRGLM2Spdb::PRODUCT_TILES_RECEIVED = "product_tiles_received";

// Dimensions
const char *GoesRGLM2Spdb::NUMBER_OF_EVENTS = "number_of_events";
const char *GoesRGLM2Spdb::NUMBER_OF_GROUPS = "number_of_groups";
const char *GoesRGLM2Spdb::NUMBER_OF_FLASHES = "number_of_flashes";

// count Variables
const char *GoesRGLM2Spdb::EVENT_COUNT = "event_count";
const char *GoesRGLM2Spdb::GROUP_COUNT = "group_count";
const char *GoesRGLM2Spdb::FLASH_COUNT = "flash_count";

// TIME Variables
const char *GoesRGLM2Spdb::PRODUCT_TIME = "product_time";
const char *GoesRGLM2Spdb::EVENT_TIME_OFFSET = "event_time_offset";
const char *GoesRGLM2Spdb::GROUP_TIME_OFFSET = "group_time_offset";
const char *GoesRGLM2Spdb::FLASH_TIME_OFFSET_LAST_EVENT = "flash_time_offset_of_last_event";
const char *GoesRGLM2Spdb::FLASH_TIME_OFFSET_FIRST_EVENT = "flash_time_offset_of_first_event";

// Location Variables
const char *GoesRGLM2Spdb::EVENT_LAT = "event_lat";
const char *GoesRGLM2Spdb::EVENT_LON = "event_lon";
const char *GoesRGLM2Spdb::GROUP_LAT = "group_lat";
const char *GoesRGLM2Spdb::GROUP_LON = "group_lon";
const char *GoesRGLM2Spdb::FLASH_LAT = "flash_lat";
const char *GoesRGLM2Spdb::FLASH_LON = "flash_lon";

// ID Variables
const char *GoesRGLM2Spdb::FLASH_ID = "flash_id";
const char *GoesRGLM2Spdb::GROUP_ID = "group_id";
const char *GoesRGLM2Spdb::EVENT_ID = "event_id";
const char *GoesRGLM2Spdb::GROUP_PARENT_FLASH_ID = "group_parent_flash_id";
const char *GoesRGLM2Spdb::EVENT_PARENT_GROUP_ID = "event_parent_group_id";

// Data Quality Flags
const char *GoesRGLM2Spdb::MISSING_QUALITY_FLAG = "none";
const char *GoesRGLM2Spdb::GROUP_QUALITY_FLAG = "group_quality_flag";
const char *GoesRGLM2Spdb::FLASH_QUALITY_FLAG = "flash_quality_flag";

// Variable Attributes
const char *GoesRGLM2Spdb::ADD_OFFSET = "add_offset";
const char *GoesRGLM2Spdb::SCALE_FACTOR = "scale_factor";
const char *GoesRGLM2Spdb::VALID_RANGE = "valid_range";


