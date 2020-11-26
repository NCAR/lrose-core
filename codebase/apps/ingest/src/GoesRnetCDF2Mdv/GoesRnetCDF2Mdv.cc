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
 * @file GoesRnetCDF2Mdv.cc
 *
 * @class GoesRnetCDF2Mdv
 *
 * GoesRnetCDF2Mdv is the top level application class.
 *  
 * @date 10/10/2017
 *
 */

#include <iostream>
#include <sstream>
#include <cmath>
#include <cassert>

#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputPathTrigger.hh>
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
#include <toolsa/TaStr.hh>

#include "GoesRnetCDF2Mdv.hh"
#include "Params.hh"

using namespace std;


// Global variables

GoesRnetCDF2Mdv *GoesRnetCDF2Mdv::_instance = (GoesRnetCDF2Mdv *)NULL;

// start of epoch for GOES-R is 2000-01-01 12:00:00, the J2000 epoch
// value determined by date --date='2000-01-01 12:00:00 UTC' +%s
const time_t GoesRnetCDF2Mdv::J2000_EPOCH_START = 946728000;
  
// missing and and bad values used in MDV files.
const float GoesRnetCDF2Mdv::MISSING_DATA_VALUE = 999999.0;
const int GoesRnetCDF2Mdv::EMISSIVE_BAND_START = 7;


/*********************************************************************
 * Constructor
 */

GoesRnetCDF2Mdv::GoesRnetCDF2Mdv(int argc, char **argv) :
  _outputVlevelType(Mdvx::VERT_SATELLITE_IMAGE)
{
  static const string method_name = "GoesRnetCDF2Mdv::GoesRnetCDF2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (GoesRnetCDF2Mdv *)NULL);
  
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
    cerr << ERROR_STR << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
  }

}

/*********************************************************************
 * Destructor
 */

GoesRnetCDF2Mdv::~GoesRnetCDF2Mdv()
{
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

GoesRnetCDF2Mdv *GoesRnetCDF2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (GoesRnetCDF2Mdv *)NULL)
    new GoesRnetCDF2Mdv(argc, argv);
  
  return(_instance);
}

GoesRnetCDF2Mdv *GoesRnetCDF2Mdv::Inst()
{
  assert(_instance != (GoesRnetCDF2Mdv *)NULL);
  
  return(_instance);
}

/*********************************************************************
 * init()
 */

bool GoesRnetCDF2Mdv::init()
{
  static const string method_name = "GoesRnetCDF2Mdv::init()";
  
  // Initialize the input trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the output domains

  if (!_initDomain())
    return false;
  
  // Initialize the output vertical level type

  if (!_initOutputVlevelType())
    return false;
  
  // initialize process registration
  if(_params->trigger_mode != Params::FILE_LIST) {
    PMU_auto_init(_progName.c_str(), _params->instance,
		  PROCMAP_REGISTER_INTERVAL);
  }

  return true;
}
  


/*********************************************************************
 * run()
 */

void GoesRnetCDF2Mdv::run()
{
  static const string methodName = "GoesRnetCDF2Mdv::run()";
  
  // register with procmap
  PMU_auto_register("Running");

  TriggerInfo triggerInfo;

  while (!_dataTrigger->endOfData()) {

    PMU_auto_register("In main loop");

    if (_params->debug) {
      cerr << INFO_STR << methodName << endl;
      cerr << "    Begin waiting for new file" << endl;
    }
    
    if (_dataTrigger->next(triggerInfo) != 0) {
      cerr << ERROR_STR << methodName << endl;
      cerr << "    Error getting next trigger time" << endl;      
      continue;
    }
  
    string filePath = triggerInfo.getFilePath();

    if (_readFile(filePath) == false) {
      PMU_auto_register("Failed to read input file");
      cerr << ERROR_STR << methodName << endl;
      cerr << "    Error reading file: " << filePath << endl;
      
      continue;
    }
    

    if (_processData() == false) {
      PMU_auto_register("Failed to process input file");
      cerr << ERROR_STR << methodName << endl;
      cerr << "    Error processing file: " << filePath << endl;
      
      continue;
    }
    
  } // endwhile - !_dataTrigger->endOfData() 

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addData()
 */
void GoesRnetCDF2Mdv::_addData(float *out_data, float *qc_data, float *rad_data)
{

  Mdvx::coord_t coords =  _outputDomain.getCoord();

  float min_val = 999999999.0;
  float max_val = -999999999.0;
   
  for (int yOutIdx = 0; yOutIdx < coords.ny; ++yOutIdx) {
    for (int xOutIdx = 0; xOutIdx < coords.nx; ++xOutIdx) {

      // Get the lat/lon location of the grid point

      double lat, lon;
      int xSatIdx, ySatIdx;
            
      _outputDomain.xyIndex2latlon(xOutIdx, yOutIdx, lat, lon);
      
      // cerr << "yOutIdx: " << yOutIdx << "  xOutIdx: "
      //      << xOutIdx << "  lat: " << lat << "  lon: " << lon << endl;

      // Convert lat/lon to satellite space x/y
      
      if(!_latLon2XY(lat, lon, xSatIdx, ySatIdx)) {
	//	cerr << "in a bad place" << endl;
	continue;
      }

      // cerr << "in a good place   " << xSatIdx << "  "
      //      << ySatIdx << "   " << _numX << "   "
      //      << (ySatIdx*static_cast<int>(_numX) + xSatIdx) << endl;

      int satIndex =  ySatIdx*static_cast<int>(_numX) + xSatIdx;

      //      cerr << "made it a little farther  " <<  satIndex << endl;

      if(satIndex < 0) {
        // cerr << "satIndex: " << satIndex
        //      << " for (lat,lon) of (" << lat << ","
        //      << lon << ")" << endl;
	continue;
      }
      size_t outDataIndex = yOutIdx*coords.nx + xOutIdx;

      if(qc_data != NULL) {
	qc_data[outDataIndex] = _dataQuality[satIndex];
      }
      
      if (_productType == PRODUCT_CLOUD_AND_MOISTURE_IMAGERY) {

        out_data[outDataIndex] = _cmi[satIndex];

      } else {

        // radiance

        if(_dataQuality[satIndex] == 0 || !_params->check_quality_field) {
          if(rad_data != NULL) {
            rad_data[outDataIndex] = _radiance[satIndex];
          }
          if(_bandID < EMISSIVE_BAND_START) {
            // reflective bands 1 - 6
            if((_radiance[satIndex] < 0.0) &&
               (_params->debug == Params::DEBUG_VERBOSE)) {
              cerr << "radiance < 0" << endl;
            }
            out_data[outDataIndex] = _radianceToAlbedo(_radiance[satIndex], lat, lon);
          } else {
            // emissive bands 7 - 16
            out_data[outDataIndex] = _radianceToBrightTemp(_radiance[satIndex]);
          }
        }

      } // if(_dataQuality[satIndex] == 0)

      max_val = fmax(out_data[outDataIndex], max_val);
      min_val = fmin(out_data[outDataIndex], min_val);

    } /* endfor - x_index */

  } /* endfor - y_index */

  if (_params->debug) {
    cerr << "==>> min_val: " << min_val << endl;
    cerr << "==>> max_val: " << max_val << endl;
  }

}

/*********************************************************************
 * _convertUnits()
 */

void GoesRnetCDF2Mdv::_convertUnits(MdvxField *field) const
{
  static const string method_name = "GoesRnetCDF2Mdv::_convertUnits()";
  
  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  fl32 *data = (fl32 *)field->getVol();

  for (int i = 0; i < field_hdr.nx * field_hdr.ny * field_hdr.nz; ++i)
  {
    // Skip missing data values

    if (data[i] == field_hdr.bad_data_value ||
	data[i] == field_hdr.missing_data_value)
      continue;

    switch (_params->units_convert_type)
    {
    case Params::CONVERT_K_TO_C :
      data[i] = TEMP_K_TO_C(data[i]);
      break;
    } /* endswitch - _params->units_convert_type */
  } /* endfor - i */

  // Update the units string in the field header

  switch (_params->units_convert_type)
  {
  case Params::CONVERT_K_TO_C :
    STRcopy(field_hdr.units, "C", MDV_UNITS_LEN);
    break;
  } /* endswitch - _params->units_convert_type */

  field->setFieldHeader(field_hdr);
}


/*********************************************************************
 * _createField()
 */

MdvxField *GoesRnetCDF2Mdv::_createField(const string &field_name,
                                         const string &long_field_name,
                                         const string &units)

{
  static const string method_name = "GoesRnetCDF2Mdv::_createField()";
  
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
  field_hdr.bad_data_value =  MISSING_DATA_VALUE;
  field_hdr.missing_data_value =  MISSING_DATA_VALUE;

  STRcopy(field_hdr.field_name_long, long_field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  _outputDomain.syncToFieldHdr(field_hdr);

  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = _outputVlevelType;
  vlevel_hdr.level[0] = 0.0;
  
  return new MdvxField(field_hdr, vlevel_hdr, (void *)NULL, true);
}


/*********************************************************************
 * _initDomains()
 */

bool GoesRnetCDF2Mdv::_initDomain(void)
{
  static const string methodName = "GoesRnetCDF2Mdv::_initDomain()";
  

  // set the projection object

  if(_params->set_earth_radius == true) {
    Pjg::setEarthRadiusKm(_params->earth_radius);
  }
  
  _outputDomain.setGrid(_params->out_grid.nx, _params->out_grid.ny,
			_params->out_grid.dx, _params->out_grid.dy,
			_params->out_grid.minx, _params->out_grid.miny);

  switch (_params->out_projection) {
  case Params::PROJ_LATLON :
  {
    _outputDomain.initLatlon(_params->out_origin_lon);
    break;
  }

  case Params::PROJ_FLAT :
  {
    _outputDomain.initFlat(_params->out_origin_lat,
			   _params->out_origin_lon,
			   _params->out_rotation);
    break;
  }
    
  case Params::PROJ_LAMBERT_CONF :
  {
    _outputDomain.initLambertConf(_params->out_origin_lat,
				  _params->out_origin_lon,
				  _params->out_lat1,
				  _params->out_lat2);
    break;
  }

  case Params::PROJ_LAMBERT_AZIM :
  {
    _outputDomain.initLambertAzim(_params->out_origin_lat,
				  _params->out_origin_lon);
    break;
  }
  
  case Params::PROJ_MERCATOR :
  {
    _outputDomain.initMercator(_params->out_origin_lat,
			       _params->out_origin_lon);
    break;
  }

  case Params::PROJ_TRANS_MERCATOR :
  {
    _outputDomain.initTransMercator(_params->out_origin_lat,
				    _params->out_origin_lon,
				    _params->out_central_scale);
      break;
  }
    
  case Params::PROJ_POLAR_STEREO :
  {
    _outputDomain.initPolarStereo(_params->out_tangent_lon,
				  //static_cast<Mdvx::pole_type_t>(_params->out_pole_type),
				  Mdvx::POLE_NORTH,
				  _params->out_central_scale);
    
    _outputDomain.setOffsetOrigin(_params->out_origin_lat,
				  _params->out_origin_lon);
    break;
  }
    
  case Params::PROJ_OBLIQUE_STEREO :
  {
    _outputDomain.initStereographic(_params->out_origin_lat,
				    _params->out_origin_lon,
				    _params->out_central_scale);
    break;
  }

  case Params::PROJ_ALBERS :
  {
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

bool GoesRnetCDF2Mdv::_initOutputVlevelType(void)
{
  static const string method_name = "GoesRnetCDF2Mdv::_initOutputVlevelType()";
  
  switch (_params->output_vlevel_type)
  {
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
 * _initTrigger()
 */

bool GoesRnetCDF2Mdv::_initTrigger(void)
{
  static const string method_name = "GoesRnetCDF2Mdv::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA:
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << ERROR_STR << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger:" << endl;
      cerr << "   input dir = " << _params->input_dir << endl;
      cerr << "   max valid secs = " << _params->max_valid_secs << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::REALTIME:
  {
    DsInputPathTrigger *trigger = new DsInputPathTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register,
		      false, true ) != 0)
    {
      cerr << ERROR_STR << method_name << endl;
      cerr << "Error initializing REALTIME trigger:" << endl;
      cerr << "   input dir = " << _params->input_dir << endl;
      cerr << "   max valid secs = " << _params->max_valid_secs << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::FILE_LIST:
  {
    const vector< string > file_list = _args->getFileList();
    
    if (file_list.size() == 0)
    {
      cerr << ERROR_STR << method_name << endl;
      cerr << "Must specify file paths on command line" << endl;
      
      return false;
    }
    
    if (_params->debug >= Params::DEBUG_NORM)
    {
      cerr << INFO_STR << method_name << endl;
      cerr << "Initializing FILE_LIST trigger: " << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
    }
    
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(file_list) != 0)
    {
      cerr << ERROR_STR << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger:" << endl;
      vector< string >::const_iterator file;
      for (file = file_list.begin(); file != file_list.end(); ++file)
	cerr << "   " << *file << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool GoesRnetCDF2Mdv::_processData()
{
  static const string methodName = "GoesRnetCDF2Mdv::_processData()";
  
  PMU_auto_register("Processing data");
  if (_params->debug) {
    cerr << INFO_STR << methodName << endl;
    cerr << "    Processing data" << endl;
  }
  
  // Create the output file
  DsMdvx mdvx;
  _updateMasterHeader(mdvx);
    
  // Create the output field and add it to the file
  // the output field data array is empty at this stage
  
  MdvxField *outDataField = NULL;
  if (_productType == PRODUCT_CLOUD_AND_MOISTURE_IMAGERY) {
    if ((outDataField = _createField(_cmiName, _cmiLongName, _cmiUnits)) == 0) {
      return false;
    }
  } else {
    if ((outDataField = _createField(_outName, _outLongName, _outUnits)) == 0) {
      return false;
    }
  }
  float *outData = (float*)outDataField->getVol();

  // the processing of the QC is controlled in the parameter file
  float *qcData = NULL;  
  MdvxField *outQcField = NULL;
  if (_params->include_qc_field == true) {
    if ((outQcField = _createField(_dqfName, _dqfLongName, _dqfUnits)) == 0) {
      return false;
    }
    // the qcData field data array is empty at this stage
    qcData = (float*)outQcField->getVol();
  }

  // create a field for the radiance values if requested
  float *radData = NULL;
  MdvxField *radField = NULL;
  if ((_params->include_rad_field == true) &&
      (_productType != PRODUCT_CLOUD_AND_MOISTURE_IMAGERY)) {
    if ((radField = _createField(_radName, _radLongName, _radUnits)) == 0) {
      return false;
    }
    // the radData field data array is empty at this stage
    radData = (float*)radField->getVol();
  }

  // fill out the data arrays as appropriate
  // and add to the MDV output object

  _addData(outData, qcData, radData);
    
  // Convert the units in the field, if requested

  if (_params->convert_units &&
      _productType != PRODUCT_CLOUD_AND_MOISTURE_IMAGERY) {
    _convertUnits(outDataField);
  }
  
  // Add the field to the output file
  
  if(outDataField->convertType(static_cast<Mdvx::encoding_type_t>(_params->out_data_prep.encoding_type),
			       static_cast<Mdvx::compression_type_t>(_params->out_data_prep.compression_type),
			       static_cast<Mdvx::scaling_type_t>(_params->out_data_prep.scaling_type),
			       _params->out_data_prep.scale, 
			       _params->out_data_prep.bias) != 0) {
    cerr << WARN_STR << methodName << endl;
    cerr << "  convertType failed. message: " << outDataField->getErrStr() << endl;
  }
  mdvx.addField(outDataField);

  if (_params->include_qc_field == true) {
    if(outQcField->convertType(static_cast<Mdvx::encoding_type_t>(_params->qc_data_prep.encoding_type),
			       static_cast<Mdvx::compression_type_t>(_params->qc_data_prep.compression_type),
			       static_cast<Mdvx::scaling_type_t>(_params->qc_data_prep.scaling_type),
			       _params->qc_data_prep.scale, 
			       _params->qc_data_prep.bias) != 0) {
      cerr << WARN_STR << methodName << endl;
      cerr << "  convertType failed. message: " << outQcField->getErrStr() << endl;
    }    
    mdvx.addField(outQcField);
  }
    
  if ((_params->include_rad_field == true) &&
      (_productType != PRODUCT_CLOUD_AND_MOISTURE_IMAGERY)) {
    if(radField->convertType(static_cast<Mdvx::encoding_type_t>(_params->rad_data_prep.encoding_type),
                             static_cast<Mdvx::compression_type_t>(_params->rad_data_prep.compression_type),
                             static_cast<Mdvx::scaling_type_t>(_params->rad_data_prep.scaling_type),
                             _params->rad_data_prep.scale, 
                             _params->rad_data_prep.bias) != 0) {
      cerr << WARN_STR << methodName << endl;
      cerr << "  convertType failed. message: " << radField->getErrStr() << endl;
    }    
    mdvx.addField(radField);
  }
    
  // Write the output file
  if(_params->debug == Params::DEBUG_VERBOSE) {
    cerr << INFO_STR << methodName << endl;
    mdvx.printMasterHeader(cerr);
    mdvx.getField(0)->printHeaders(cerr);
  }
    
  if (_params->debug) {
    cerr << INFO_STR << methodName << endl;
    cerr << "    Writing output to: " << _params->output_url << " for time "
	 << DateTime::str(mdvx.getMasterHeader().time_centroid) << endl;
  }
    
  mdvx.setWriteLdataInfo();
  if (mdvx.writeToDir(_params->output_url) != 0)
    {
      cerr << ERROR_STR << methodName << endl;
      cerr << "Error writing MDV file to output URL: " << _params->output_url << endl;
      cerr << mdvx.getErrStr() << endl;
      
      return false;
    }

  if (_params->debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }
    
  return true;
}


/*********************************************************************
 * _readFile()
 */

bool GoesRnetCDF2Mdv::_readFile(const std::string& file_path)
{
  static const string methodName = "GoesRnetCDF2Mdv::_readFile()";
  
  PMU_auto_register("Reading file");

  if (_params->debug) {
    cerr << INFO_STR << methodName << endl;
    cerr << "    Reading file." << endl;
  }

  // clear out and reset things before reading another file

  _clearAndReset();

   // open file and read contents

  try {
    
    if (_params->debug) {
      string msg = INFO_STR + methodName + "--   Reading from " + file_path;
      cerr << msg << endl;
    }

    _file.open(file_path, NcxxFile::read);

    _readGlobalAttributes();

    _readDimensions();

    _readVariables();

  } catch (NcxxException& e) {

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

void GoesRnetCDF2Mdv::_readGlobalAttributes()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readGlobalAttributes()";

  if (_params->debug) {
    cerr << "Reading in non-essential global properties" << endl;
  }

  _readOptionalGlobalAttr(NC_PROPERTIES, _globalAtts.ncProperties);
  _readOptionalGlobalAttr(NAMING_AUTHORITY, _globalAtts.namingAuthority);
  _readOptionalGlobalAttr(CONVENTIONS, _globalAtts.conventions);
  _readOptionalGlobalAttr(METADATA_CONVENTIONS, _globalAtts.metadataConventions);
  _readOptionalGlobalAttr(STANDARD_NAME_VOCABULARY, _globalAtts.standardNameVocabulary);
  _readOptionalGlobalAttr(INSTITUTION, _globalAtts.institution);
  _readOptionalGlobalAttr(PROJECT, _globalAtts.project);
  _readOptionalGlobalAttr(PRODUCTION_SITE, _globalAtts.productionSite);
  _readOptionalGlobalAttr(PRODUCTION_ENVIRONMENT, _globalAtts.productionEnvironment);
  _readOptionalGlobalAttr(SPATIAL_RESOLUTION, _globalAtts.spatialResolution);
  _readOptionalGlobalAttr(ORBITAL_SLOT, _globalAtts.orbitalSlot);
  _readOptionalGlobalAttr(PLATFORM_ID, _globalAtts.platformID);
  _readOptionalGlobalAttr(INSTRUMENT_TYPE, _globalAtts.instrumentType);
  _readOptionalGlobalAttr(INSTRUMENT_ID, _globalAtts.instrumentID);
  _readOptionalGlobalAttr(SUMMARY, _globalAtts.summary);
  _readOptionalGlobalAttr(KEYWORDS, _globalAtts.keywords);
  _readOptionalGlobalAttr(KEYWORDS_VOCABULARY, _globalAtts.keywordsVocabulary);
  _readOptionalGlobalAttr(ISO_SERIES_METADATA_ID, _globalAtts.isoSeriesMetadataID);
  _readOptionalGlobalAttr(LICENSE, _globalAtts.license);
  _readOptionalGlobalAttr(PROCESSING_LEVEL, _globalAtts.processingLevel);
  _readOptionalGlobalAttr(DATASET_NAME, _globalAtts.datasetName);
  _readOptionalGlobalAttr(PRODUCTION_DATA_SOURCE, _globalAtts.productionDataSource);
  _readOptionalGlobalAttr(TIMELINE_ID, _globalAtts.timelineID);
  _readOptionalGlobalAttr(DATE_CREATED, _globalAtts.dateCreated);
  _readOptionalGlobalAttr(TIME_COVERAGE_START, _globalAtts.timeCoverageStart);
  _readOptionalGlobalAttr(TIME_COVERAGE_END, _globalAtts.timeCoverageEnd);
  _readOptionalGlobalAttr(CREATED_BY, _globalAtts.createdBy);

  if (_params->debug) {
    _printGlobalAttributes(cerr);
  }

  // read in CDM data type
  // check that the value is 'Image'

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading in " << CDM_DATA_TYPE << endl;
  }
  try {
    _file.readGlobAttr(CDM_DATA_TYPE, _globalAtts.cdmDataType);
  } catch (NcxxException &e) {
    NcxxErrStr err;
    ostringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  didn't find " << CDM_DATA_TYPE << " in global attributes" << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  if (_globalAtts.cdmDataType != "Image") {
    NcxxErrStr err;
    ostringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  " << CDM_DATA_TYPE << " must be 'Image'" << endl;
    info << "  value found is: " << _globalAtts.cdmDataType << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading in " << SCENE_ID << endl;
  }
  try {
    _file.readGlobAttr(SCENE_ID, _globalAtts.sceneID);
  } catch (NcxxException &e) {
    NcxxErrStr err;
    ostringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  didn't find " << SCENE_ID << " in global attributes" << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading in " << TITLE << endl;
  }
  try {
    _file.readGlobAttr(TITLE, _globalAtts.title);
  } catch (NcxxException &e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  didn't find " << TITLE << " in global attributes" << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Setting scan id to sceneID: " << _globalAtts.sceneID << endl;
  }
  try {
    _setScanType(_globalAtts.sceneID); 
  } catch (NcxxException &e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Setting productType to title: " << _globalAtts.title << endl;
  }
  try {
    _setProductType(_globalAtts.title);
  } catch (NcxxException &e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
}

///////////////////////////////////////////////////////
// read an optional global attribute, store in dest
// print warning on failure

void GoesRnetCDF2Mdv::_readOptionalGlobalAttr(const string &name, string &dest)
{

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading optional global attr: " << name << endl;
  }

  try {
    _file.readGlobAttr(name, dest);
  } catch (NcxxException& e) {
    if (_params->debug) {
      cerr << "WARNING - cannot find optional global attr: " << name << endl;
    }
  }

}

///////////////////////////////////////////////////////
// read a requried global attribute, store in dest
// returns 0 on success, -1 on failure

int GoesRnetCDF2Mdv::_readRequiredGlobalAttr(const string &name, string &dest)
{
  
  static const string methodName = "GoesRnetCDF2Mdv::_readRequiredGlobalAttr()";
  
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading in required global attr: " << name << endl;
  }
  
  try {
    _file.readGlobAttr(name, dest);
  } catch (NcxxException &e) {
    NcxxErrStr err;
    ostringstream info;
    info << ERROR_STR << methodName << endl;
    info << "  didn't find " << name << " in global attributes" << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    cerr << err.getErrStr() << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////
// get a variable attribute, given the name
// returns 0 on success, -1 on failure

int GoesRnetCDF2Mdv::_getVarAtt(const NcxxVar &var, const string &attName, 
                                NcxxVarAtt &att)
{
  try {
    att = var.getAtt(attName);
  } catch (NcxxException& e) {
    return -1;
  }
  return 0;
}

///////////////////////////////////////////////////////
// get string value of an attribute
// returns empty string on failure

string GoesRnetCDF2Mdv::_getAsStr(NcxxVarAtt &att)
{
  string val;
  try {
    att.getValues(val);
  } catch (NcxxException& e) {
    val.clear();
  }
  return val;
}

///////////////////////////////////////////////////////
// get int value of an attribute
// returns -9999 on failure

int GoesRnetCDF2Mdv::_getAsInt(NcxxVarAtt &att)
{
  int val;
  try {
    att.getValues(&val);
  } catch (NcxxException& e) {
    val = -9999;
  }
  return val;
}

///////////////////////////////////////////////////////
// get int values of an attribute
// returns -9999 on failure

vector<int> GoesRnetCDF2Mdv::_getAsInts(NcxxVarAtt &att)
{
  vector<int> vals;
  try {
    att.getValues(vals);
  } catch (NcxxException& e) {
    vals.clear();
  }
  return vals;
}

///////////////////////////////////////////////////////
// get float value of an attribute
// returns -9999.0 on failure

float GoesRnetCDF2Mdv::_getAsFloat(NcxxVarAtt &att)
{
  float val;
  try {
    att.getValues(&val);
  } catch (NcxxException& e) {
    val = -9999.0;
  }
  return val;
}

///////////////////////////////////////////////////////
// get doublea value of an attribute
// returns -9999.0 on failure

double GoesRnetCDF2Mdv::_getAsDouble(NcxxVarAtt &att)
{
  double val;
  try {
    att.getValues(&val);
  } catch (NcxxException& e) {
    val = -9999.0;
  }
  return val;
}

/*********************************************************************
 * print the global attributes
 */

void GoesRnetCDF2Mdv::_printGlobalAttributes(ostream &out)
{

  out << "============= Global Attributes ================" << endl;
  out << "  ncProperties: " << _globalAtts.ncProperties << endl;
  out << "  namingAuthority: " << _globalAtts.namingAuthority << endl;
  out << "  conventions: " << _globalAtts.conventions << endl;
  out << "  metadataConventions: " << _globalAtts.metadataConventions << endl;
  out << "  standardNameVocabulary: " << _globalAtts.standardNameVocabulary << endl;
  out << "  institution: " << _globalAtts.institution << endl;
  out << "  project: " << _globalAtts.project << endl;
  out << "  productionSite: " << _globalAtts.productionSite << endl;
  out << "  productionEnvironment: " << _globalAtts.productionEnvironment << endl;
  out << "  spatialResolution: " << _globalAtts.spatialResolution << endl;
  out << "  orbitalSlot: " << _globalAtts.orbitalSlot << endl;
  out << "  platformID: " << _globalAtts.platformID << endl;
  out << "  instrumentType: " << _globalAtts.instrumentType << endl;
  out << "  sceneID: " << _globalAtts.sceneID << endl;
  out << "  instrumentID: " << _globalAtts.instrumentID << endl;
  out << "  datasetName: " << _globalAtts.datasetName << endl;
  out << "  isoSeriesMetadataID: " << _globalAtts.isoSeriesMetadataID << endl;
  out << "  title: " << _globalAtts.title << endl;
  out << "  summary: " << _globalAtts.summary << endl;
  out << "  keywords: " << _globalAtts.keywords << endl;
  out << "  keywordsVocabulary: " << _globalAtts.keywordsVocabulary << endl;
  out << "  license: " << _globalAtts.license << endl;
  out << "  processingLevel: " << _globalAtts.processingLevel << endl;
  out << "  dateCreated: " << _globalAtts.dateCreated << endl;
  out << "  cdmDataType: " << _globalAtts.cdmDataType << endl;
  out << "  timelineID: " << _globalAtts.timelineID << endl;
  out << "  timeCoverageStart: " << _globalAtts.timeCoverageStart << endl;
  out << "  timeCoverageEnd: " << _globalAtts.timeCoverageEnd << endl;
  out << "  createdBy: " << _globalAtts.createdBy << endl;
  out << "  productionDataSource: " << _globalAtts.productionDataSource << endl;
  out << "================================================" << endl;

}

/*********************************************************************
 * _readDimensions()
 */

void GoesRnetCDF2Mdv::_readDimensions()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readDimensions()";


  try {

    switch(_productType) {
    case PRODUCT_LEVEL1B_RADIANCES:
    case PRODUCT_CLOUD_AND_MOISTURE_IMAGERY:
    case PRODUCT_TYPE_UNKNOWN:
      {
	try {   
	  NcxxDim dim = _file.getDim(BAND_DIM);
	  if(dim.isNull() == true) {
	    NcxxErrStr err;
	    stringstream  info;
	    info << ERROR_STR << methodName << endl;
	    info << "  dimension is missing, name: " << BAND_DIM << endl;
	    err.addErrStr(info.str());
	    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));

	  }
	  _numBands = dim.getSize();    
	}
	catch  (NcxxException &e) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  exception: " << e.what() << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}

	break;
      }
      case PRODUCT_AEROSOL_DETECTION:
      case PRODUCT_CLOUD_TOP_PHASE:
      case PRODUCT_CLOUD_TOP_HEIGHT:
      case PRODUCT_CLOUD_TOP_TEMPERATURE:
      case PRODUCT_CLOUD_TOP_PRESSURE:
      case PRODUCT_LAND_SURFACE_TEMPERATURE:
      case PRODUCT_CLOUD_OPTICAL_DEPTH:
      case PRODUCT_CLOUD_PARTICLE_SIZE:   
      {
	_readZenithAngleDims(); // allow the 8 types above to fall through to this point
	break;
      }
    case PRODUCT_AEROSOL_OPTICAL_DEPTH:
      {
	_readZenithAngleDims();
	
	NcxxDim dim = _file.getDim(NUMBER_OF_SUNGLINT_ANGLE_BOUNDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << " dimension is missing, name: " <<
	    NUMBER_OF_SUNGLINT_ANGLE_BOUNDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numSunglintAngleBounds = dim.getSize();

	dim = _file.getDim(LAND_SENSOR_BANDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << LAND_SENSOR_BANDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numLandSensorBands = dim.getSize();

	dim = _file.getDim(SEA_SENSOR_BANDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << SEA_SENSOR_BANDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numSeaSensorBands = dim.getSize();
	
	dim = _file.getDim(LATITUDE_BANDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << LATITUDE_BANDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numLatitudeBands = dim.getSize();
    
	dim = _file.getDim(NUMBER_OF_LATITUDE_BAND_BOUNDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " <<
	    NUMBER_OF_LATITUDE_BAND_BOUNDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numLatitudeBandBounds = dim.getSize();
	
	dim = _file.getDim(NUM_AREA_TYPES);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << NUM_AREA_TYPES << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numAreaTypes = dim.getSize();
    
	dim = _file.getDim(MAX_AREA_TYPE_LEN);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << MAX_AREA_TYPE_LEN<< endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_maxAreaTypeLen = dim.getSize();

	break;
      }
    case PRODUCT_DERIVED_STABILITY_INDICES:
    case PRODUCT_TOTAL_PRECIPITABLE_WATER:
      {
	_readZenithAngleDims();
	
	NcxxDim dim = _file.getDim(NUMBER_OF_LAT_BOUNDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << NUMBER_OF_LAT_BOUNDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numLatBounds = dim.getSize();
	
	dim = _file.getDim(SOUNDING_EMISSIVE_BANDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << SOUNDING_EMISSIVE_BANDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numSoundingEmissiveBands = dim.getSize();
	
	break;
      }
    case PRODUCT_CLEAR_SKY_MASK:
      {
	_readZenithAngleDims();
	
	NcxxDim dim = _file.getDim(RTM_BT_COMPARISON_BANDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
 	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " << RTM_BT_COMPARISON_BANDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numRtmBtComparisonBands = dim.getSize();
	
	break;
      }
    case PRODUCT_FIRE_CHARACTERIZATION:
      {
	_readZenithAngleDims();
	
	NcxxDim dim = _file.getDim(NUMBER_OF_SUNGLINT_ANGLE_BOUNDS);      
	if(dim.isNull() == true) {
	  NcxxErrStr err;
	  stringstream  info;
	  info << ERROR_STR << methodName << endl;
	  info << "  dimension is missing, name: " <<
	    NUMBER_OF_SUNGLINT_ANGLE_BOUNDS << endl;
	  err.addErrStr(info.str());
	  throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
	}
	_numSunglintAngleBounds = dim.getSize();
	
	break;
      }
    case PRODUCT_DERIVED_MOTION_WINDS:
      {
	NcxxErrStr err;
	stringstream  info;
	info << ERROR_STR << methodName << endl;
	info << "Processing derived motion winds not implemented." << endl;
	err.addErrStr(info.str());
	throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
    case PRODUCT_GLOBAL_LIGHTNING:
      {
	NcxxErrStr err;
	stringstream  info;
	info << ERROR_STR << methodName << endl;
	info << "Processing global lightning not implemented." << endl;
	err.addErrStr(info.str());
	throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
    default:
      {
	NcxxErrStr err;
	stringstream  info;
	info << ERROR_STR << methodName << endl;
	info << "unknown product type. should not be possible." << endl;
	err.addErrStr(info.str());
	throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
      }
    }
  }
  catch  (NcxxException &e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
}

/*********************************************************************
 * _readZenithAngleDims()
 */

void GoesRnetCDF2Mdv::_readZenithAngleDims()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readZenithAngleDims()";

  try {
    NcxxDim dim = _file.getDim(NUMBER_OF_LZA_BOUNDS);
    if(dim.isNull() == true) {
      NcxxErrStr err;
      stringstream  info;
      info << ERROR_STR << methodName << endl;
      info << "  dimension is missing, name: " << NUMBER_OF_LZA_BOUNDS << endl;
      err.addErrStr(info.str());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
    _numLZABounds = dim.getSize();
    
    dim = _file.getDim(NUMBER_OF_SZA_BOUNDS);      
    if(dim.isNull() == true) {
      NcxxErrStr err;
      stringstream  info;
      info << ERROR_STR << methodName << endl;
      info << "  dimension is missing, name: " << NUMBER_OF_SZA_BOUNDS << endl;
      err.addErrStr(info.str());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
    _numSZABounds = dim.getSize();

  }
  catch  (NcxxException &e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr(info.str());
    info << "  exception: " << e.what() << endl;
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}

 

/*********************************************************************
 * _clearAndReset()
 */

void GoesRnetCDF2Mdv::_clearAndReset()
{
  static const string methodName = "GoesRnetCDF2Mdv::_clearAndReset()";
  
}

/*********************************************************************
 * _readVariables()
 */

void GoesRnetCDF2Mdv::_readVariables()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readVariables()";

  try {
    _readCoordinateVars();
    _readTimeVars();
    _readProjectionVars();
    _readRadianceConstantsVars();
    _readQualityControlVars();
    _readFieldVars();
  }
  catch (NcxxException &e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  Cannot read all the variables." << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}

/*********************************************************************
 * _readCoordinateVars()
 */

void GoesRnetCDF2Mdv::_readCoordinateVars()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readCoordinateVars()";
  
  NcxxDim xDim;
  NcxxDim yDim;
  NcxxVar xVar;
  NcxxVar yVar;
  string xDimStr = X_DIM;
  string yDimStr = Y_DIM;
  _file.getCoordVar(xDimStr, xDim, xVar);
  _file.getCoordVar(yDimStr, yDim, yVar);

  if((xDim.isNull() == true) || (xVar.isNull() == true)) {
    NcxxErrStr err;
    string info = ERROR_STR + methodName;
    err.addErrStr(info);
    cerr << "  X coordinate is missing. dimension: " << X_DIM << " variable: "
	 << X_COORD << endl;
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  std::map< std::string, NcxxVarAtt > xCoordAtts;
  _numX = xDim.getSize();
  xCoordAtts = xVar.getAtts();
  float scaleFactor;
  float offset;
  try{
    xCoordAtts[SCALE_FACTOR].getValues(&scaleFactor);
    xCoordAtts[ADD_OFFSET].getValues(&offset);
  }
  catch (NcxxException& e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr(info.str());
    err.addErrStr("  exception: ", e.what());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  short *xVals = new short[_numX];
  xVar.getVal(xVals);
  for (size_t i = 0; i < _numX; i++) {
    _xCoord.push_back((static_cast<float>(xVals[i])*scaleFactor + offset));
  }
  delete [] xVals;

  if((yDim.isNull() == true) || (yVar.isNull() == true)) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  Y coordinate is missing. dimension: " << Y_DIM << " variable: "
	 << Y_COORD << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  std::map< std::string, NcxxVarAtt > yCoordAtts;
  _numY = yDim.getSize();
  yCoordAtts = yVar.getAtts();
  try{
    yCoordAtts[SCALE_FACTOR].getValues(&scaleFactor);
    yCoordAtts[ADD_OFFSET].getValues(&offset);
  }
  catch (NcxxException &e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    err.addErrStr("  exception: ", e.what());
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  short *yVals = new short[_numY];
  yVar.getVal(yVals);
  for (size_t i = 0; i < _numY; i++) {
    _yCoord.push_back((static_cast<float>(yVals[i])*scaleFactor + offset));
  }
  delete [] yVals;
  	    
}


/*********************************************************************
 * _readTimeVars()
 */

void GoesRnetCDF2Mdv::_readTimeVars()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readTimeVars()";

  NcxxVar varT = _file.getVar(TIME);
  if(varT.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<  TIME << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  double val;
  varT.getVal(&val);
  _midpointTime = J2000_EPOCH_START + static_cast< time_t >(round(val));
  if (_params->correct_albedo_for_sun_angle) {
    _sunAngle.initForTime(_midpointTime);
  }

  NcxxDim dim = _file.getDim(NUMBER_OF_TIME_BOUNDS);
  if(dim.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find dimension, name: " << NUMBER_OF_TIME_BOUNDS << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  size_t numBounds = dim.getSize();

  NcxxVar varTb = _file.getVar(TIME_BOUNDS);
  if(varTb.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << TIME_BOUNDS << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  assert(numBounds == 2);
  double *vals = new double[numBounds];
  varTb.getVal(vals);
  _beginTime = J2000_EPOCH_START + static_cast< time_t >(round(vals[0]));
  _endTime = J2000_EPOCH_START + static_cast< time_t >(round(vals[1]));
  delete [] vals;
  
}
  
/*********************************************************************
 * _readProjectionVars()
 */

void GoesRnetCDF2Mdv::_readProjectionVars()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readProjectionVars()";

  NcxxVar varG = _file.getVar(GOES_IMAGER_PROJECTION);
  if(varG.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<  GOES_IMAGER_PROJECTION << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  // pull out attributes from this variable
  std::map< std::string, NcxxVarAtt > imgProjAtts;
  imgProjAtts = varG.getAtts();
  try{
    imgProjAtts[GRID_MAPPING_NAME].getValues(_gridMappingName);
    imgProjAtts[PERSPECTIVE_POINT_HEIGHT].getValues(&_perspectivePointHeight);
    imgProjAtts[SEMI_MAJOR_AXIS].getValues(&_semiMajorAxis);
    imgProjAtts[SEMI_MINOR_AXIS].getValues(&_semiMinorAxis);
    imgProjAtts[INVERSE_FLATTENING].getValues(&_inverseFlattening);
    imgProjAtts[LATITUDE_OF_PROJECTION_ORIGIN].getValues(&_projectionOriginLatitude);
    imgProjAtts[LONGITUDE_OF_PROJECTION_ORIGIN].getValues(&_projectionOriginLongitude);
    // TEST
    //    _projectionOriginLongitude = -75.0;
  }
  catch (NcxxException& e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "   exception: " << e.what() << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }


 NcxxVar varLa = _file.getVar(NOMINAL_SATELLITE_SUBPOINT_LAT);
  if(varLa.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<  NOMINAL_SATELLITE_SUBPOINT_LAT << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varLa.getVal(&_nominalSatSubpointLat);

  NcxxVar varLo = _file.getVar(NOMINAL_SATELLITE_SUBPOINT_LON);
  if(varLo.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<  NOMINAL_SATELLITE_SUBPOINT_LON << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varLo.getVal(&_nominalSatSubpointLon);

  NcxxVar varH = _file.getVar(NOMINAL_SATELLITE_HEIGHT);
  if(varH.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << NOMINAL_SATELLITE_HEIGHT  << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varH.getVal(&_nominalSatHeight);

  NcxxVar varLL = _file.getVar(GEOSPATIAL_LAT_LON_EXTENT);
  if(varLL.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << GEOSPATIAL_LAT_LON_EXTENT  << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  // pull out attributes from this variable
  std::map< std::string, NcxxVarAtt > latLonExtAtts;
  latLonExtAtts = varLL.getAtts();
  try{
    latLonExtAtts[GEOSPATIAL_WESTBOUND_LONGITUDE].getValues(&_westLongitude);
    latLonExtAtts[GEOSPATIAL_NORTHBOUND_LATITUDE].getValues(&_northLatitude);
    latLonExtAtts[GEOSPATIAL_EASTBOUND_LONGITUDE].getValues(&_eastLongitude);
    latLonExtAtts[GEOSPATIAL_SOUTHBOUND_LATITUDE].getValues(&_southLatitude);
    latLonExtAtts[GEOSPATIAL_LAT_CENTER].getValues(&_centerLatitude);
    latLonExtAtts[GEOSPATIAL_LON_CENTER].getValues(&_centerLongitude);
    latLonExtAtts[GEOSPATIAL_LAT_NADIR].getValues(&_nadirLatitude);
    latLonExtAtts[GEOSPATIAL_LON_NADIR].getValues(&_nadirLongitude);
  }
  catch (NcxxException& e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "   exception: " << e.what() << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  NcxxVar varX = _file.getVar(X_IMAGE);
  if(varX.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << X_IMAGE << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }  
  varX.getVal(&_xImageCenter);

  NcxxVar varY = _file.getVar(Y_IMAGE);
  if(varY.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << Y_IMAGE << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varY.getVal(&_yImageCenter);

  NcxxVar varXb = _file.getVar(X_IMAGE_BOUNDS);
  if(varXb.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << X_IMAGE_BOUNDS << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  NcxxDim dimI = _file.getDim(NUMBER_OF_IMAGE_BOUNDS); 
  if(dimI.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  dimension is missing, name: " << NUMBER_OF_IMAGE_BOUNDS << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  size_t numBounds = dimI.getSize();
  assert(numBounds == 2);

  _xImageBounds.assign(numBounds, 0.0);
  varXb.getVal(_xImageBounds.data());

  // TODO: get _dxRad from resolution attribute in radiance var
  // TEST
  //_xImageBounds[0] = -0.10136;
  //_xImageBounds[1] = 0.03864;
  
  //_dxRad = (_xImageBounds[1] - _xImageBounds[0])/2500;
    _dxRad = (_xImageBounds[1] - _xImageBounds[0])/_numX;

  NcxxVar varYb = _file.getVar(Y_IMAGE_BOUNDS);
  if(varYb.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << Y_IMAGE_BOUNDS << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  
  _yImageBounds.assign(numBounds, 0.0);
  varYb.getVal(_yImageBounds.data());

  // TODO: get _dyRad from resolution attribute in radiance var
  // TEST
  //_yImageBounds[0] = 0.12824;
  //_yImageBounds[1] = 0.04424;
  //_dyRad = (_yImageBounds[0] - _yImageBounds[1])/1500;
  _dyRad = (_yImageBounds[0] - _yImageBounds[1])/_numY;

  
  // initialize coeffiecients for the projective geometry 
  double flatten = 1.0/_inverseFlattening;
  _ecc = sqrt(2.0*flatten - flatten*flatten);
  _radiusRatio2 = pow((_semiMajorAxis/_semiMinorAxis), 2.0);
  _invRadiusRatio2 = 1.0/_radiusRatio2;
  _H = _perspectivePointHeight +  _semiMajorAxis;

}
  
/*********************************************************************
 * _readConstantsVars()
 */

void GoesRnetCDF2Mdv::_readRadianceConstantsVars()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readRadainceConstantsVars()";

  NcxxVar varE = _file.getVar(ESUN);
  if(varE.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << ESUN << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varE.getVal(&_eSun);

  NcxxVar varK = _file.getVar(KAPPA0);
  if(varK.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << KAPPA0 << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varK.getVal(&_kappa0);
  if (_params->debug) {
    cerr <<  "kappa0 is " << _kappa0 << endl;
  }

  NcxxVar varF1 = _file.getVar(PLANCK_FK1);
  if(varF1.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << PLANCK_FK1 << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varF1.getVal(&_planckFk1);

  NcxxVar varF2 = _file.getVar(PLANCK_FK2);
  if(varF2.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << PLANCK_FK2 << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varF2.getVal(&_planckFk2);

  NcxxVar varB1 = _file.getVar(PLANCK_BC1);
  if(varB1.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << PLANCK_BC1 << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varB1.getVal(&_planckBc1);

  NcxxVar varB2 = _file.getVar(PLANCK_BC2);
  if(varB2.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << PLANCK_BC2 << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varB2.getVal(&_planckBc2);

}
  
/*********************************************************************
 * _readQualityControlVars()
 */

void GoesRnetCDF2Mdv::_readQualityControlVars()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readQualityControlVars()";

  NcxxVar varVp = _file.getVar(VALID_PIXEL_COUNT);
  if(varVp.isNull() == true) {
    _validPixelCount = 0;
  } else {
    varVp.getVal(&_validPixelCount);
  }

  NcxxVar varMp = _file.getVar(MISSING_PIXEL_COUNT);
  if(varMp.isNull() == true) {
    _missingPixelCount = 0;
  } else {
    varMp.getVal(&_missingPixelCount);
  }

  NcxxVar varSp = _file.getVar(SATURATED_PIXEL_COUNT);
  if(varSp.isNull() == true) {
    _saturatedPixelCount = 0;
  } else {
    varSp.getVal(&_saturatedPixelCount);
  }

  NcxxVar varUp = _file.getVar(UNDERSATURATED_PIXEL_COUNT);
  if(varUp.isNull() == true) {
    _undersaturatedPixelCount = 0;
  } else {
    varUp.getVal(&_undersaturatedPixelCount);
  }

  NcxxVar varMr = _file.getVar(MIN_RADIANCE_VALUE_OF_VALID_PIXELS);
  if(varMr.isNull() == true) {
    _minRadValueValidPixels = -9999.0;
  } else {
    varMr.getVal(&_minRadValueValidPixels);
  }

  NcxxVar varXr = _file.getVar(MAX_RADIANCE_VALUE_OF_VALID_PIXELS);
  if(varXr.isNull() == true) {
    _maxRadValueValidPixels = -9999.0;
  } else {
    varXr.getVal(&_maxRadValueValidPixels);
  }

  NcxxVar varMm = _file.getVar(MEAN_RADIANCE_VALUE_OF_VALID_PIXELS);
  if(varMm.isNull() == true) {
    _meanRadValueValidPixels = -9999.0;
  } else {
    varMm.getVal(&_meanRadValueValidPixels);
  }

  NcxxVar varSr = _file.getVar(STD_DEV_RADIANCE_VALUE_OF_VALID_PIXELS);
  if(varSr.isNull() == true) {
    _stdDevRadValueValidPixels = -9999.0;
  } else {
    varSr.getVal(&_stdDevRadValueValidPixels);
  }

  NcxxVar varP = _file.getVar(PERCENT_UNCORRECTABLE_L0_ERRORS);
  if(varP.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<
      PERCENT_UNCORRECTABLE_L0_ERRORS << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varP.getVal(&_percentUncorrectableL0Errors);

  NcxxVar varE = _file.getVar(EARTH_SUN_DISTANCE_ANOMALY_IN_AU);
  if(varE.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<
      EARTH_SUN_DISTANCE_ANOMALY_IN_AU << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varE.getVal(&_earthSunDistAnomalyAU);

  NcxxVar varD = _file.getVar(ALGORITHM_DYNAMIC_INPUT_DATA_CONTAINER);
  if(varD.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<
      ALGORITHM_DYNAMIC_INPUT_DATA_CONTAINER << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  // pull out attributes from this variable
  _inputAbiL0Data.clear();
  NcxxVarAtt attD;
  if (_getVarAtt(varD, INPUT_ABI_L0_DATA, attD) == 0) {
    _inputAbiL0Data = _getAsStr(attD);
  }

  NcxxVar varPv = _file.getVar(PROCESSING_PARM_VERSION_CONTAINER);
  if(varPv.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<
      PROCESSING_PARM_VERSION_CONTAINER << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  // pull out attributes from this variable
  std::map< std::string, NcxxVarAtt > attsPv = varPv.getAtts();
  try{
    attsPv[L1B_PROCESSING_PARM_VERSION].getValues(_l1bProcessingParamVersion);
  }
  catch (NcxxException& e) {
    _l1bProcessingParamVersion.clear();
  }

  NcxxVar varV = _file.getVar(ALGORITHM_PRODUCT_VERSION_CONTAINER);
  if(varV.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<
      ALGORITHM_PRODUCT_VERSION_CONTAINER  << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  // pull out attributes from this variable
  std::map< std::string, NcxxVarAtt > attsV = varV.getAtts();
  try{
    attsV[ALGORITHM_VERSION].getValues(_algorithmVersion);
    attsV[PRODUCT_VERSION].getValues(_algoProductVersion);
  }
  catch (NcxxException& e) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "   exception: " << e.what() << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  NcxxDim dim = _file.getDim(NUM_STAR_LOOKS);
  if(!dim.isNull()) {

    size_t numStarLooks = dim.getSize();
    
    NcxxVar slVar = _file.getVar(T_STAR_LOOK);
    if(slVar.isNull() == true) {
      NcxxErrStr err;
      stringstream  info;
      info << ERROR_STR << methodName << endl;
      info << "  cannot find variable, name: " << T_STAR_LOOK << endl;
      err.addErrStr(info.str());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
    
    // TODO -- T_STAR_LOOK contained missing values in test file. Need to put
    // missing value check
    double *dvals = new double[numStarLooks];
    slVar.getVal(dvals);
    time_t theTime;
    for(size_t i = 0; i < numStarLooks; i++) {
      if(dvals[i] > 0.0) {
        theTime = J2000_EPOCH_START + static_cast< time_t >(round(dvals[i]));
      }
      else {
        theTime = 0;
      }    
      _timeStarLook.push_back(theTime);
    }
    delete [] dvals;
    
    NcxxVar bwVar = _file.getVar(BAND_WAVELENGTH_STAR_LOOK);
    if(bwVar.isNull() == true) {
      NcxxErrStr err;
      stringstream  info;
      info << ERROR_STR << methodName << endl;
      info << "  cannot find variable, name: " << BAND_WAVELENGTH_STAR_LOOK << endl;
      err.addErrStr(info.str());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }
    
    _bandWavelengthStarLook.assign(numStarLooks, 0.0);
    bwVar.getVal(_bandWavelengthStarLook.data());
    

    NcxxVar siVar = _file.getVar(STAR_ID);
    if(siVar.isNull() == true) {
      NcxxErrStr err;
      stringstream  info;
      info << ERROR_STR << methodName << endl;
      info << "  cannot find variable, name: " << STAR_ID << endl;
      err.addErrStr(info.str());
      throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
    }

    _starID.assign(numStarLooks, 0);
    siVar.getVal(_starID.data());

  } // NUM_STAR_LOOKS
  
}
  
/*********************************************************************
 * _readFieldVars()
 */

void GoesRnetCDF2Mdv::_readFieldVars()
{

  static const string methodName = "GoesRnetCDF2Mdv::_readFieldVars()";

  // band ID

  NcxxVar varB = _file.getVar(BAND_ID);
  if(varB.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " <<  BAND_ID << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varB.getVal(&_bandID);

  // band wavelength

  NcxxVar varW = _file.getVar(BAND_WAVELENGTH);
  if(varW.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << BAND_WAVELENGTH << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  varW.getVal(&_bandWavelength);

  if (_productType == PRODUCT_LEVEL1B_RADIANCES) {
    _readRadiance();
    _readDataQualFlag();
  } else if (_productType == PRODUCT_CLOUD_AND_MOISTURE_IMAGERY) {
    _readCMI();
    _readDataQualFlag();
  }

  // yaw flip flag

  NcxxVar varY = _file.getVar(YAW_FLIP_FLAG);
  if(!varY.isNull()) {
    uint8_t val;
    varY.getVal(&val);
    _yawFlipFlag = static_cast< bool >(val);
  } else {
    _yawFlipFlag = false;
  }

}

/*********************************************************************
 * _readRadiance()
 */

void GoesRnetCDF2Mdv::_readRadiance()
{
  static const string methodName = "GoesRnetCDF2Mdv::_readRadiance()";

  // get the variable

  NcxxVar varR = _file.getVar(RADIANCE);
  if(varR.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << RADIANCE << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  // retrieve the attributes

  NcxxVarAtt attR;
  float scaleFactor = 1.0;
  if (_getVarAtt(varR, SCALE_FACTOR, attR) == 0) {
    scaleFactor = _getAsFloat(attR);
  }
  float offset = 0.0;
  if (_getVarAtt(varR, ADD_OFFSET, attR) == 0) {
    offset = _getAsFloat(attR);
  }
  _ncRadMissingVal = -9990.0;
  if (_getVarAtt(varR, FILL_VALUE, attR) == 0) {
    _ncRadMissingVal = _getAsFloat(attR);
  }
  _radLongName.clear();
  if (_getVarAtt(varR, LONG_NAME, attR) == 0) {
    _radLongName = _getAsStr(attR);
  }
  _radStandardName.clear();
  if (_getVarAtt(varR, STANDARD_NAME, attR) == 0) {
    _radStandardName = _getAsStr(attR);
  }
  _radUnits.clear();
  if (_getVarAtt(varR, UNITS, attR) == 0) {
    _radUnits = _getAsStr(attR);
  }

  if(_bandID < EMISSIVE_BAND_START) {
    _outName = "Albedo";
    _outLongName = "Albedo";
    _outStandardName = "Albedo";
    _outUnits = "%";
  } else {
    _outName = "Temp";
    _outLongName = "BrightnessTemp";
    _outStandardName = "BrightnessTemp";
    _outUnits = "K";
  }

  // optionally override names / units

  _radName = RADIANCE;
  if (strlen(_params->rad_data_prep.short_name) > 0) {
    _radName = _params->rad_data_prep.short_name;
  }
  if (strlen(_params->rad_data_prep.long_name) > 0) {
    _radLongName = _params->rad_data_prep.long_name;
  }
  if (strlen(_params->rad_data_prep.units) > 0) {
    _radUnits = _params->rad_data_prep.units;
  }

  if (strlen(_params->out_data_prep.short_name) > 0) {
    _outName = _params->out_data_prep.short_name;
  }
  if (strlen(_params->out_data_prep.long_name) > 0) {
    _outLongName = _params->out_data_prep.long_name;
  }
  if (strlen(_params->out_data_prep.units) > 0) {
    _outUnits = _params->out_data_prep.units;
  }

  size_t nPts = _numX*_numY;
  short *svals = new short[nPts];
  varR.getVal(svals);

  if(_params->init_zero == true) {
    _radiance.assign(nPts, 0.0);
  }
  else {
    _radiance.assign(nPts, MISSING_DATA_VALUE);
  }

  for (size_t i = 0; i < nPts; i++) {
    if (svals[i] != _ncRadMissingVal) {
      _radiance[i] = static_cast<float>(svals[i])*scaleFactor + offset;
      // somehow small negative radiances values can be computed
      // for svals that are less than offset/scaleFactor. 
      if(_radiance[i] < 0.0) {
	_radiance[i] = 0.0;
      }
#ifdef DEBUG_PRINT
      if(_radiance[i] < 0.0) {
      	cerr << "radiance < 0" << endl;
      }
      if(i < 100) {
        if(_bandID < EMISSIVE_BAND_START) {
          cerr << "  i: " << i << "  svals: "
               << svals[i] << "  rad data:"
               << _radiance[i] << "   albedo: "
               <<  _radianceToAlbedo(_radiance[i]) << endl;
        } else {
          cerr << "  i: " << i << "  svals: "
               << svals[i] << "  rad data:"
               << _radiance[i] << "   btemp: "
               <<  _radianceToBrightTemp(_radiance[i]) << endl;
        }
      }
#endif
    } else {
#ifdef DEBUG_PRINT
      cerr << "got a missing value here" << endl;
#endif
    }
  }
  delete [] svals;

}

/*********************************************************************
 * _readCMI() - Cloud and Moisture Imagery
 */

void GoesRnetCDF2Mdv::_readCMI()
{

  static const string methodName = "GoesRnetCDF2Mdv::_readCMI()";
  
  // get the variable

  NcxxVar varCmi = _file.getVar(CMI);
  if(varCmi.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << CMI << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  // retrieve the attributes

  NcxxVarAtt attCmi;
  float scaleFactor = 1.0;
  if (_getVarAtt(varCmi, SCALE_FACTOR, attCmi) == 0) {
    scaleFactor = _getAsFloat(attCmi);
  }
  float offset = 0.0;
  if (_getVarAtt(varCmi, ADD_OFFSET, attCmi) == 0) {
    offset = _getAsFloat(attCmi);
  }
  _ncCmiMissingVal = -9990.0;
  if (_getVarAtt(varCmi, FILL_VALUE, attCmi) == 0) {
    _ncCmiMissingVal = _getAsFloat(attCmi);
  }
  _cmiLongName.clear();
  if (_getVarAtt(varCmi, LONG_NAME, attCmi) == 0) {
    _cmiLongName = _getAsStr(attCmi);
  }
  _cmiStandardName.clear();
  if (_getVarAtt(varCmi, STANDARD_NAME, attCmi) == 0) {
    _cmiStandardName = _getAsStr(attCmi);
  }
  _cmiUnits.clear();
  if (_getVarAtt(varCmi, UNITS, attCmi) == 0) {
    _cmiUnits = _getAsStr(attCmi);
  }

  // optionally override names / units

  _cmiName = CMI;
  if (strlen(_params->out_data_prep.short_name) > 0) {
    _cmiName = _params->out_data_prep.short_name;
  }
  if (strlen(_params->out_data_prep.long_name) > 0) {
    _cmiLongName = _params->out_data_prep.long_name;
  }
  if (strlen(_params->out_data_prep.units) > 0) {
    _cmiUnits = _params->out_data_prep.units;
  }

  size_t nPts = _numX*_numY;
  short *svals = new short[nPts];
  varCmi.getVal(svals);

  if(_params->init_zero == true) {
    _cmi.assign(nPts, 0.0);
  }
  else {
    _cmi.assign(nPts, MISSING_DATA_VALUE);
  }
  
  for (size_t i = 0; i < nPts; i++) {
    if (svals[i] != _ncCmiMissingVal) {
      _cmi[i] = static_cast<float>(svals[i])*scaleFactor + offset;
    }
  }
  delete [] svals;

}

/*********************************************************************
 * _readDataQualFlag()
 */

void GoesRnetCDF2Mdv::_readDataQualFlag()
{

  static const string methodName = "GoesRnetCDF2Mdv::_readDataQualFlag()";
 
  // get the variable

  NcxxVar varDqf  = _file.getVar(DATA_QUALITY_FLAG);
  if(varDqf.isNull() == true) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  cannot find variable, name: " << DATA_QUALITY_FLAG << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

  // retrieve the attributes

  NcxxVarAtt attDqf;
  _ncDqfMissingVal = -127;
  if (_getVarAtt(varDqf, FILL_VALUE, attDqf) == 0) {
    _ncDqfMissingVal = _getAsInt(attDqf);
  }
  _dqfLongName.clear();
  if (_getVarAtt(varDqf, LONG_NAME, attDqf) == 0) {
    _dqfLongName = _getAsStr(attDqf);
  }
  _dqfStandardName.clear();
  if (_getVarAtt(varDqf, STANDARD_NAME, attDqf) == 0) {
    _dqfStandardName = _getAsStr(attDqf);
  }
  _dqfUnits.clear();
  if (_getVarAtt(varDqf, UNITS, attDqf) == 0) {
    _dqfUnits = _getAsStr(attDqf);
  }

  // override atts from params file as applicable

  if (strlen(_params->qc_data_prep.short_name) > 0) {
    _dqfName = _params->qc_data_prep.short_name;
  }
  if (strlen(_params->qc_data_prep.long_name) > 0) {
    _dqfLongName = _params->qc_data_prep.long_name;
  }
  if (strlen(_params->qc_data_prep.units) > 0) {
    _dqfUnits = _params->qc_data_prep.units;
  }

  // get flag meanings string

  string dqfFlagMeaningsStr;
  if (_getVarAtt(varDqf, FLAG_MEANINGS, attDqf) == 0) {
    dqfFlagMeaningsStr = _getAsStr(attDqf);
  }

  // split the dqfLabels based on white space
  vector< string > dqfLabels;
  TaStr::tokenize(dqfFlagMeaningsStr, " ", dqfLabels);
  
  // get flag values
  
  vector<int> dqfVals;
  if (_getVarAtt(varDqf, FLAG_VALUES, attDqf) == 0) {
    dqfVals = _getAsInts(attDqf);
  }

  // create quality flags map
  
  if (dqfLabels.size() == dqfVals.size()) {
    _qualityFlags.clear();
    for(size_t ii = 0; ii < dqfVals.size(); ii++) {
      _qualityFlags[dqfVals[ii]] = dqfLabels[ii];
    }
  }

  // set data quality array
  
  size_t nPts = _numX*_numY;
  _dataQuality.assign(nPts, 0);
  varDqf.getVal(_dataQuality.data());

}

/*********************************************************************
 * _setProductType()
 */

void GoesRnetCDF2Mdv::_setProductType(const string &title)
{
  static const string methodName = "GoesRnetCDF2Mdv::_setProductType()";

  if(title == TITLE_LEVEL1B_RADIANCES) {
    _productType = PRODUCT_LEVEL1B_RADIANCES;
  }
  else if (title == TITLE_CLOUD_AND_MOISTURE_IMAGERY) {
    _productType = PRODUCT_CLOUD_AND_MOISTURE_IMAGERY;
  }
  else if (title == TITLE_AEROSOL_DETECTION) {
    _productType = PRODUCT_AEROSOL_DETECTION;
  }
  else if (title == TITLE_AEROSOL_OPTICAL_DEPTH) {
    _productType = PRODUCT_AEROSOL_OPTICAL_DEPTH;
  }
  else if (title == TITLE_CLOUD_TOP_PHASE) {
    _productType = PRODUCT_CLOUD_TOP_PHASE;
  }
  else if (title == TITLE_CLOUD_TOP_HEIGHT) {
    _productType = PRODUCT_CLOUD_TOP_HEIGHT;
  }
  else if (title == TITLE_CLOUD_TOP_TEMPERATURE) {
    _productType = PRODUCT_CLOUD_TOP_TEMPERATURE;
  }
  else if (title == TITLE_CLOUD_TOP_PRESSURE) {
    _productType = PRODUCT_CLOUD_TOP_PRESSURE;
  }
  else if (title == TITLE_DERIVED_STABILITY_INDICES) {
    _productType = PRODUCT_DERIVED_STABILITY_INDICES;
  }
  else if (title == TITLE_TOTAL_PRECIPITABLE_WATER) {
    _productType = PRODUCT_TOTAL_PRECIPITABLE_WATER;
  }
  else if (title == TITLE_CLEAR_SKY_MASK) {
    _productType = PRODUCT_CLEAR_SKY_MASK;
  }
  else if (title == TITLE_FIRE_CHARACTERIZATION) {
    _productType = PRODUCT_FIRE_CHARACTERIZATION;
  }
  else if (title == TITLE_LAND_SURFACE_TEMPERATURE) {
    _productType = PRODUCT_LAND_SURFACE_TEMPERATURE;
  }
  else if (title == TITLE_CLOUD_OPTICAL_DEPTH) {
    _productType = PRODUCT_CLOUD_OPTICAL_DEPTH;
  }
  else if (title == TITLE_CLOUD_PARTICLE_SIZE) {
    _productType = PRODUCT_CLOUD_PARTICLE_SIZE;
  }
  else if (title == TITLE_DERIVED_MOTION_WINDS) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  Processing derived motion winds not implemented." << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  else if (title == TITLE_GLOBAL_LIGHTNING) {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  Processing global lightning not implemented." << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }
  else {
    cerr << "WARNING - product type unknown: " << title << endl;
    _productType = PRODUCT_TYPE_UNKNOWN;
  }
  
}

/*********************************************************************
 * _setScanType()
 */

void GoesRnetCDF2Mdv::_setScanType(const string &scan_id)
{
  static const string methodName = "GoesRnetCDF2Mdv::_setScanType()";

  if(scan_id == FULL_DISK_SCAN_NAME) {
    _scanType = SCAN_FULL_DISK;
  }
  else if (scan_id == CONUS_SCAN_NAME) {
    _scanType = SCAN_CONUS;
  }
  else if (scan_id == MESOSCALE_SCAN_NAME) {
    _scanType = SCAN_MESOSCALE;
  }
  else {
    NcxxErrStr err;
    stringstream  info;
    info << ERROR_STR << methodName << endl;
    info << "  Unknown scan ID." << endl;
    err.addErrStr(info.str());
    throw(NcxxException(err.getErrStr(), __FILE__, __LINE__));
  }

}


/*********************************************************************
 * _latLon2XY()
 */

bool GoesRnetCDF2Mdv::_latLon2XY(double lat, double lon,
				 int& x_idx,  int& y_idx)
{
  static const string methodName = "GoesRnetCDF2Mdv::_latLon2XY()";

  double c_lat = atan(_invRadiusRatio2*tan(lat*DEG_TO_RAD));
  double cos_clat = cos(c_lat);

  double rc = _semiMinorAxis/sqrt(1.0 - pow(_ecc*cos_clat, 2.0));
      
  double del_lon_angle = (lon - _projectionOriginLongitude)*DEG_TO_RAD;

  double sx =  _H - (rc*cos_clat*cos(del_lon_angle));
  double sy = -rc*cos_clat*sin(del_lon_angle);
  double sz = rc*sin(c_lat);
      
  // // check that point is on disk of the earth
  if((_H*(_H - sx)) < (sy*sy + _radiusRatio2*sz*sz)) {
    x_idx = -1;
    y_idx = -1;
    //    cerr << WARN_STR << methodName << endl;
    // cerr << "  lat,lon not on disk." << endl;
    return false;
  }

  double rl = sqrt((sx*sx + sy*sy + sz*sz));
  double xx = asin((-sy/rl));
  double yy = atan((sz/sx));

  
  x_idx = round((xx - _xImageBounds[0])/_dxRad);
  y_idx = round((_yImageBounds[0] - yy)/_dyRad);

  //  cerr << "lat: " << lat << "  lon: " << lon << "  ximage: " << xx << "  yimage: " << yy << endl;
  return true;
}

/*********************************************************************
 * _radianceToBrightTemp()
 */

float GoesRnetCDF2Mdv::_radianceToBrightTemp(float rad)
{
  static const string methodName = "GoesRnetCDF2Mdv::_radianceToBrightTemp()";

  
  float brightTemp = rad;

  // TODO: handle potential divide by 0 if rad is 0
  if((rad > 0.0) && (rad != MISSING_DATA_VALUE)) {

    brightTemp = (_planckFk2/log((_planckFk1/rad) + 1.0) - _planckBc1)/_planckBc2;

  }
  
  return brightTemp;
}


/*********************************************************************
 * _radianceToAlbedo()
 */

float GoesRnetCDF2Mdv::_radianceToAlbedo(float rad,
                                         double lat, double lon)
{
  static const string methodName = "GoesRnetCDF2Mdv::_radianceToAlbedo()";
  
  float albedo = rad;

  if(rad != MISSING_DATA_VALUE) {

    albedo = 100.0 * rad * _kappa0; // the 100 is to convert to percent [0,100]

    // optionally correct for sun angle
    if (_params->correct_albedo_for_sun_angle && lat > -90 && lon > -360) {
      double sinAlt = _sunAngle.computeSinAlt(lat, lon);
      if (sinAlt < 0.1) {
        sinAlt = 0.1;
      }
      albedo = albedo / sinAlt;
      if (albedo > 100.0) {
        albedo = (float) 100.0;
      }
    }

  }
  
  return albedo;
}

/*********************************************************************
 * _updateMasterHeader()
 */

void GoesRnetCDF2Mdv::_updateMasterHeader(DsMdvx &mdvx)
{
  static const string method_name = "GoesRnetCDF2Mdv::_updateMasterHeader()";
  
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(NULL);
  master_hdr.time_begin = _beginTime;
  master_hdr.time_end = _endTime;
  switch (_params->output_timestamp)
  {
  case Params::TIMESTAMP_BEGIN :
    master_hdr.time_centroid = master_hdr.time_begin;
    break;
  case Params::TIMESTAMP_END :
    master_hdr.time_centroid = master_hdr.time_end;
    break;
  case Params::TIMESTAMP_MIDDLE :
    master_hdr.time_centroid = _midpointTime;
    break;
  }
  master_hdr.time_expire = master_hdr.time_end +
    static_cast< time_t >(_params->expire_offset);
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

/////////////////////////////////////////////////////////////////////////////////////
// string constant instantiation
//

const char* GoesRnetCDF2Mdv::ERROR_STR = "ERROR - ";
const char* GoesRnetCDF2Mdv::WARN_STR = "WARNING - ";
const char* GoesRnetCDF2Mdv::INFO_STR = "INFO - ";

// Level-1 and Level-2 Product Dimensions
const char* GoesRnetCDF2Mdv::X_DIM = "x";
const char* GoesRnetCDF2Mdv::Y_DIM = "y";
const char* GoesRnetCDF2Mdv::NUMBER_OF_TIME_BOUNDS = "number_of_time_bounds";
const char* GoesRnetCDF2Mdv::BAND_DIM = "band";
const char* GoesRnetCDF2Mdv::NUMBER_OF_IMAGE_BOUNDS = "number_of_image_bounds";

// Level-1 Specific Dimensions
const char* GoesRnetCDF2Mdv::NUM_STAR_LOOKS = "num_star_looks";

// Level-2 Product Specific Dimensions
const char* GoesRnetCDF2Mdv::NUMBER_OF_LZA_BOUNDS = "number_of_LZA_bounds";
const char* GoesRnetCDF2Mdv::NUMBER_OF_SZA_BOUNDS = "number_of_SZA_bounds";
const char* GoesRnetCDF2Mdv::NUMBER_OF_SUNGLINT_ANGLE_BOUNDS =
  "number_of_sunglint_angle_bounds";
const char* GoesRnetCDF2Mdv::LAND_SENSOR_BANDS = "land_sensor_bands";
const char* GoesRnetCDF2Mdv::SEA_SENSOR_BANDS = "sea_sensor_bands";
const char* GoesRnetCDF2Mdv::LATITUDE_BANDS = "latitude_bands";
const char* GoesRnetCDF2Mdv::NUMBER_OF_LATITUDE_BAND_BOUNDS =
  "number_of_latitude_band_bounds";
const char* GoesRnetCDF2Mdv::NUM_AREA_TYPES = "num_area_types";
const char* GoesRnetCDF2Mdv::MAX_AREA_TYPE_LEN = "max_area_type_len";
const char* GoesRnetCDF2Mdv::NUMBER_OF_LAT_BOUNDS = "number_of_lat_bounds";
const char* GoesRnetCDF2Mdv::SOUNDING_EMISSIVE_BANDS = "sounding_emissive_bands";
 const char* GoesRnetCDF2Mdv::RTM_BT_COMPARISON_BANDS = "RTM_BT_comparison_bands";
 
// Level-1 and Level-2 product Global Attributes
const char* GoesRnetCDF2Mdv::NC_PROPERTIES = "_NCProperties";
const char* GoesRnetCDF2Mdv::NAMING_AUTHORITY = "naming_authority";
const char* GoesRnetCDF2Mdv::CONVENTIONS = "Conventions";
const char* GoesRnetCDF2Mdv::METADATA_CONVENTIONS = "Metadata_Conventions";
const char* GoesRnetCDF2Mdv::STANDARD_NAME_VOCABULARY  = "standard_name_vocabulary";
const char* GoesRnetCDF2Mdv::INSTITUTION = "institution";
const char* GoesRnetCDF2Mdv::PROJECT = "project";
const char* GoesRnetCDF2Mdv::PRODUCTION_SITE = "production_site";
const char* GoesRnetCDF2Mdv::PRODUCTION_ENVIRONMENT = "production_environment";
const char* GoesRnetCDF2Mdv::SPATIAL_RESOLUTION = "spatial_resolution";
const char* GoesRnetCDF2Mdv::ORBITAL_SLOT = "orbital_slot";
const char* GoesRnetCDF2Mdv::PLATFORM_ID = "platform_ID";
const char* GoesRnetCDF2Mdv::INSTRUMENT_TYPE = "instrument_type";
const char* GoesRnetCDF2Mdv::SCENE_ID = "scene_id";
const char* GoesRnetCDF2Mdv::INSTRUMENT_ID = "instrument_ID";
const char* GoesRnetCDF2Mdv::TITLE = "title";
const char* GoesRnetCDF2Mdv::SUMMARY = "summary";
const char* GoesRnetCDF2Mdv::KEYWORDS = "keywords";
const char* GoesRnetCDF2Mdv::KEYWORDS_VOCABULARY = "keywords_vocabulary";
const char* GoesRnetCDF2Mdv::ISO_SERIES_METADATA_ID = "iso_series_metadata_id";
const char* GoesRnetCDF2Mdv::LICENSE = "license";
const char* GoesRnetCDF2Mdv::PROCESSING_LEVEL = "processing_level";
const char* GoesRnetCDF2Mdv::CDM_DATA_TYPE = "cdm_data_type";
const char* GoesRnetCDF2Mdv::DATASET_NAME = "dataset_name";
const char* GoesRnetCDF2Mdv::PRODUCTION_DATA_SOURCE = "production_data_source";
const char* GoesRnetCDF2Mdv::TIMELINE_ID = "timeline_id";
const char* GoesRnetCDF2Mdv::DATE_CREATED = "date_created";
const char* GoesRnetCDF2Mdv::TIME_COVERAGE_START = "time_coverage_start";
const char* GoesRnetCDF2Mdv::TIME_COVERAGE_END = "time_coverage_end";
const char* GoesRnetCDF2Mdv::CREATED_BY = "created_by";

// Level-1 and Level-2  Variables
const char* GoesRnetCDF2Mdv::X_COORD = "x";
const char* GoesRnetCDF2Mdv::Y_COORD = "y";
const char* GoesRnetCDF2Mdv::DATA_QUALITY_FLAG = "DQF";
const char* GoesRnetCDF2Mdv::TIME = "t";
const char* GoesRnetCDF2Mdv::TIME_BOUNDS = "time_bounds";
const char* GoesRnetCDF2Mdv::GOES_IMAGER_PROJECTION =
  "goes_imager_projection";
const char* GoesRnetCDF2Mdv::X_IMAGE = "x_image";
const char* GoesRnetCDF2Mdv::Y_IMAGE = "y_image";
const char* GoesRnetCDF2Mdv::X_IMAGE_BOUNDS = "x_image_bounds";
const char* GoesRnetCDF2Mdv::Y_IMAGE_BOUNDS = "y_image_bounds";
const char* GoesRnetCDF2Mdv::NOMINAL_SATELLITE_SUBPOINT_LAT =
  "nominal_satellite_subpoint_lat";
const char* GoesRnetCDF2Mdv::NOMINAL_SATELLITE_SUBPOINT_LON =
  "nominal_satellite_subpoint_lon";
const char* GoesRnetCDF2Mdv::NOMINAL_SATELLITE_HEIGHT =
  "nominal_satellite_height";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_LAT_LON_EXTENT =
  "geospatial_lat_lon_extent";
const char* GoesRnetCDF2Mdv::ALGORITHM_DYNAMIC_INPUT_DATA_CONTAINER =
  "algorithm_dynamic_input_data_container";
const char* GoesRnetCDF2Mdv::PROCESSING_PARM_VERSION_CONTAINER =
  "processing_parm_version_container";
const char* GoesRnetCDF2Mdv::ALGORITHM_PRODUCT_VERSION_CONTAINER =
  "algorithm_product_version_container";
const char* GoesRnetCDF2Mdv::PERCENT_UNCORRECTABLE_L0_ERRORS =
  "percent_uncorrectable_L0_errors";

// Level-1 Variables
const char* GoesRnetCDF2Mdv::RADIANCE = "Rad";
const char* GoesRnetCDF2Mdv::CMI = "CMI";
const char* GoesRnetCDF2Mdv::YAW_FLIP_FLAG = "yaw_flip_flag";
const char* GoesRnetCDF2Mdv::BAND_ID = "band_id";
const char* GoesRnetCDF2Mdv::BAND_WAVELENGTH = "band_wavelength";
const char* GoesRnetCDF2Mdv::ESUN = "esun";
const char* GoesRnetCDF2Mdv::KAPPA0 = "kappa0";
const char* GoesRnetCDF2Mdv::PLANCK_FK1 = "planck_fk1";
const char* GoesRnetCDF2Mdv::PLANCK_FK2 = "planck_fk2";
const char* GoesRnetCDF2Mdv::PLANCK_BC1 = "planck_bc1";
const char* GoesRnetCDF2Mdv::PLANCK_BC2 = "planck_bc2";
const char* GoesRnetCDF2Mdv::VALID_PIXEL_COUNT = "valid_pixel_count";
const char* GoesRnetCDF2Mdv::MISSING_PIXEL_COUNT = "missing_pixel_count";
const char* GoesRnetCDF2Mdv::SATURATED_PIXEL_COUNT = "saturated_pixel_count";
const char* GoesRnetCDF2Mdv::UNDERSATURATED_PIXEL_COUNT =
  "undersaturated_pixel_count";
const char* GoesRnetCDF2Mdv::MIN_RADIANCE_VALUE_OF_VALID_PIXELS =
  "min_radiance_value_of_valid_pixels";
const char* GoesRnetCDF2Mdv::MAX_RADIANCE_VALUE_OF_VALID_PIXELS =
  "max_radiance_value_of_valid_pixels";
const char* GoesRnetCDF2Mdv::MEAN_RADIANCE_VALUE_OF_VALID_PIXELS =
  "mean_radiance_value_of_valid_pixels";
const char* GoesRnetCDF2Mdv::STD_DEV_RADIANCE_VALUE_OF_VALID_PIXELS =
  "std_dev_radiance_value_of_valid_pixels";
const char* GoesRnetCDF2Mdv::EARTH_SUN_DISTANCE_ANOMALY_IN_AU =
  "earth_sun_distance_anomaly_in_AU";
const char* GoesRnetCDF2Mdv::T_STAR_LOOK = "t_star_look";
const char* GoesRnetCDF2Mdv::BAND_WAVELENGTH_STAR_LOOK =
  "band_wavelength_star_look";
const char* GoesRnetCDF2Mdv::STAR_ID = "star_id";

// Level-2 Product Shared Variables
const char* GoesRnetCDF2Mdv::RETRIEVAL_LOCAL_ZENITH_ANGLE =
  "retrieval_local_zenith_angle";
const char* GoesRnetCDF2Mdv::QUANTITATIVE_LOCAL_ZENITH_ANGLE =
  "quantitative_local_zenith_angle";
const char* GoesRnetCDF2Mdv::RETRIEVAL_LOCAL_ZENITH_ANGLE_BOUNDS =
  "retrieval_local_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::QUANTITATIVE_LOCAL_ZENITH_ANGLE_BOUNDS =
  "quantitative_local_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::RETRIEVAL_SOLAR_ZENITH_ANGLE =
  "retrieval_solar_zenith_angle";
const char* GoesRnetCDF2Mdv::QUANTITATIVE_SOLAR_ZENITH_ANGLE =
  "quantitative_solar_zenith_angle";
const char* GoesRnetCDF2Mdv::RETRIEVAL_SOLAR_ZENITH_ANGLE_BOUNDS =
  "retrieval_solar_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::QUANTITATIVE_SOLAR_ZENITH_ANGLE_BOUNDS =
  "quantitative_solar_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::SOLAR_ZENITH_ANGLE = "local_zenith_angle";
const char* GoesRnetCDF2Mdv::SOLAR_ZENITH_ANGLE_BOUNDS =
  "local_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::LOCAL_ZENITH_ANGLE = "local_zenith_angle";
const char* GoesRnetCDF2Mdv::LOCAL_ZENITH_ANGLE_BOUNDS =
  "local_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::TWILIGHT_SOLAR_ZENITH_ANGLE =
  "twilight_solar_zenith_angle";
const char* GoesRnetCDF2Mdv::TWILIGHT_SOLAR_ZENITH_ANGLE_BOUNDS =
  "twilight_solar_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::SUNGLINT_ANGLE = "sunglint_angle";
const char* GoesRnetCDF2Mdv::SUNGLINT_ANGLE_BOUNDS = "sunglint_angle_bounds";
const char* GoesRnetCDF2Mdv::DAY_RETRIEVAL_LOCAL_ZENITH_ANGLE =
  "day_retrieval_local_zenith_angle";
const char* GoesRnetCDF2Mdv::NIGHT_RETRIEVAL_LOCAL_ZENITH_ANGLE =
  "night_retrieval_local_zenith_angle";
const char* GoesRnetCDF2Mdv::DAY_RETRIEVAL_LOCAL_ZENITH_ANGLE_BOUNDS =
  "day_retrieval_local_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::NIGHT_RETRIEVAL_LOCAL_ZENITH_ANGLE_BOUNDS =
  "night_retrieval_local_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::DAY_SOLAR_ZENITH_ANGLE = "day_solar_zenith_angle";
const char* GoesRnetCDF2Mdv::NIGHT_SOLAR_ZENITH_ANGLE = "night_solar_zenith_angle";
const char* GoesRnetCDF2Mdv::DAY_ALGORITHM_SOLAR_ZENITH_ANGLE =
  "day_algorithm_solar_zenith_angle";
const char* GoesRnetCDF2Mdv::NIGHT_ALGORITHM_SOLAR_ZENITH_ANGLE =
  "night_algorithm_solar_zenith_angle";
const char* GoesRnetCDF2Mdv::DAY_SOLAR_ZENITH_ANGLE_BOUNDS =
  "day_solar_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::NIGHT_SOLAR_ZENITH_ANGLE_BOUNDS =
  "night_solar_zenith_angle_bounds";
const char* GoesRnetCDF2Mdv::DATA_QUALITY_FLAG_OVERALL = "DQF_Overall";
const char* GoesRnetCDF2Mdv::DATA_QUALITY_FLAG_RETRIEVAL = "DQF_Retrieval";
const char* GoesRnetCDF2Mdv::DATA_QUALITY_FLAG_SKINTEMP = "DQF_SkinTemp";
const char* GoesRnetCDF2Mdv::PERCENT_UNCORRECTABLE_GRB_ERRORS =
  "percent_uncorrectable_GRB_errors";
const char* GoesRnetCDF2Mdv::OUTLIER_PIXELS = "outlier_pixels";
const char* GoesRnetCDF2Mdv::CLOUD_PIXELS = "cloud_pixels";
const char* GoesRnetCDF2Mdv::OUTLIER_PIXEL_COUNT = "outlier_pixel_count";
const char* GoesRnetCDF2Mdv::PERCENT_TERMINATOR_PIXELS =
  "percent_terminator_pixels";
const char* GoesRnetCDF2Mdv::DAY_CLOUD_PIXELS = "daytime_cloud_pixels";
const char* GoesRnetCDF2Mdv::NIGHT_CLOUD_PIXELS = "nighttime_cloud_pixels";
const char* GoesRnetCDF2Mdv::PERCENT_DAY_PIXEL = "percent_daytime_pixel";
const char* GoesRnetCDF2Mdv::LATITUDE = "latitude_bounds";
const char* GoesRnetCDF2Mdv::SOUNDING_EMISSIVE_WAVELENGTHS =
  "sounding_emissive_wavelengths";
const char* GoesRnetCDF2Mdv::SOUNDING_EMISSIVE_BAND_IDS = "sounding_emissive_band_ids";
const char* GoesRnetCDF2Mdv::TOTAL_ATTEMPTED_RETRIEVALS = "total_attempted_retrievals";
const char* GoesRnetCDF2Mdv::MEAN_OBS_MODELED_DIFF_SOUNDING_EMISSIVE_BANDS =
  "mean_obs_modeled_diff_sounding_emissive_bands";
const char* GoesRnetCDF2Mdv::STD_DEV_OBS_MODELED_DIFF_SOUNDING_EMISSIVE_BANDS =
  "std_dev_obs_modeled_diff_sounding_emissive_bands";


// Level-2 Aerosol file (A99)
const char* GoesRnetCDF2Mdv::AEROSOL = "Aerosol";
const char* GoesRnetCDF2Mdv::SMOKE = "Smoke";
const char* GoesRnetCDF2Mdv::DUST = "Dust";
const char* GoesRnetCDF2Mdv::NUMBER_GOOD_LZA_PIXELS = "number_good_LZA_pixels";
const char* GoesRnetCDF2Mdv::NUMBER_GOOD_SZA_PIXELS = "number_good_SZA_pixels";
const char* GoesRnetCDF2Mdv::NUMBER_OF_GOOD_SMOKE_RETRIEVALS =
  "number_of_good_smoke_retrievals";
const char* GoesRnetCDF2Mdv::NUMBER_OF_GOOD_DUST_RETRIEVALS = "number_of_good_dust_retrievals";
const char* GoesRnetCDF2Mdv::NUMBER_OF_GOOD_RETRIEVALS_WHERE_SMOKE_DETECTED =
  "number_of_good_retrievals_where_smoke_detected";
const char* GoesRnetCDF2Mdv::NUMBER_OF_GOOD_RETRIEVALS_WHERE_DUST_DETECTED =
  "number_of_good_retrievals_where_dust_detected";


// Level-2 Aerosol Optical Depth File (B99)
const char* GoesRnetCDF2Mdv::AEROSOL_OPTICAL_DEPTH = "AOD";
const char* GoesRnetCDF2Mdv::AOD_PRODUCT_WAVELENGTH = "aod_product_wavelength";
const char* GoesRnetCDF2Mdv::LAND_SENSOR_BAND_WAVELENGTHS = "land_sensor_band_wavelengths";
const char* GoesRnetCDF2Mdv::SEA_SENSOR_BAND_WAVELENGTHS = "sea_sensor_band_wavelengths";
const char* GoesRnetCDF2Mdv::LAND_SENSOR_BAND_IDS = "land_sensor_band_ids";
const char* GoesRnetCDF2Mdv::SEA_SENSOR_BAND_IDS = "sea_sensor_band_ids";
const char* GoesRnetCDF2Mdv::SNOW_FREE_LAND_AND_ICE_FREE_SEA =
"snow_free_land_and_ice_free_sea";
const char* GoesRnetCDF2Mdv::GOES_LAT_LON_PROJECTION = "goes_lat_lon_projection";
const char* GoesRnetCDF2Mdv::AOD550_RETRIEVALS_ATTEMPTED_LAND =
"aod550_retrievals_attempted_land";
const char* GoesRnetCDF2Mdv::AOD550_RETRIEVALS_ATTEMPTED_SEA =
"aod550_retrievals_attempted_sea";
const char* GoesRnetCDF2Mdv::AOD550_GOOD_LZA_RETRIEVALS_ATTEMPTED =
"aod550_good_LZA_retrievals_attempted";
const char* GoesRnetCDF2Mdv::AOD550_OUTLIER_PIXEL_COUNT = "aod550_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::MIN_AOD550_LAND = "min_aod550_land";
const char* GoesRnetCDF2Mdv::MAX_AOD550_LAND = "min_aod550_land";
const char* GoesRnetCDF2Mdv::MEAN_AOD550_LAND = "mean_aod550_land";
const char* GoesRnetCDF2Mdv::STD_DEV_AOD550_LAND = "std_dev_aod550_land";
const char* GoesRnetCDF2Mdv::MIN_AOD550_SEA = "min_aod550_sea";
const char* GoesRnetCDF2Mdv::MAX_AOD550_SEA = "max_aod550_sea";
const char* GoesRnetCDF2Mdv::MEAN_AOD550_SEA = "mean_aod550_sea";
const char* GoesRnetCDF2Mdv::STD_DEV_AOD550_SEA = "std_dev_aod550_sea";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_MIN_AOD_LAND = "sensor_band_min_aod_land";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_MAX_AOD_LAND = "sensor_band_max_aod_land";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_MEAN_AOD_LAND = "sensor_band_mean_aod_land";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_STD_DEV_AOD_LAND =
  "sensor_band_std_dev_aod_land";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_MIN_AOD_SEA = "sensor_band_min_aod_sea";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_MAX_AOD_SEA = "sensor_band_max_aod_sea";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_MEAN_AOD_SEA = "sensor_band_mean_aod_sea";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_STD_DEV_AOD_SEA =
  "sensor_band_std_dev_aod_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_RETRIEVALS_ATTEMPTED_LAND =
  "lat_band_aod550_retrievals_attempted_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_RETRIEVALS_ATTEMPTED_SEA =
  "lat_band_aod550_retrievals_attempted_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_GOOD_LZA_RETRIEVALS_ATTEMPTED_LAND =
  "lat_band_aod550_good_LZA_retrievals_attempted_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_GOOD_LZA_RETRIEVALS_ATTEMPTED_SEA =
  "lat_band_aod550_good_LZA_retrievals_attempted_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_PERCENT_GOOD_RETRIEVALS_LAND =
  "lat_band_aod550_percent_good_retrievals_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_PERCENT_BAD_RETRIEVALS_LAND =
  "lat_band_aod550_percent_bad_retrievals_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_PERCENT_GOOD_RETRIEVALS_SEA =
  "lat_band_aod550_percent_good_retrievals_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_AOD550_PERCENT_BAD_RETRIEVALS_SEA =
  "lat_band_aod550_percent_bad_retrievals_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_MIN_AOD550_LAND = "lat_band_min_aod550_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_MAX_AOD550_LAND = "lat_band_max_aod550_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_MEAN_AOD550_LAND = "lat_band_mean_aod550_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_STD_DEV_AOD550_LAND =
  "lat_band_std_dev_aod550_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_MIN_AOD550_SEA = "lat_band_min_aod550_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_MAX_AOD550_SEA = "lat_band_max_aod550_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_MEAN_AOD550_SEA = "lat_band_mean_aod550_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_STD_DEV_AOD550_SEA =
  "lat_band_std_dev_aod550_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MIN_AOD_LAND =
  "lat_band_sensor_band_min_aod_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MAX_AOD_LAND =
  "lat_band_sensor_band_max_aod_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MEAN_AOD_LAND =
  "lat_band_sensor_band_mean_aod_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_STD_DEV_AOD_LAND =
  "lat_band_sensor_band_std_dev_aod_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MIN_AOD_SEA =
  "lat_band_sensor_band_min_aod_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MAX_AOD_SEA =
  "lat_band_sensor_band_max_aod_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MEAN_AOD_SEA =
  "lat_band_sensor_band_mean_aod_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_STD_DEV_AOD_SEA =
  "lat_band_sensor_band_std_dev_aod_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MIN_SURFACE_REFLECTIVITY_LAND =
  "lat_band_sensor_band_min_surface_reflectivity_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MAX_SURFACE_REFLECTIVITY_LAND =
  "lat_band_sensor_band_max_surface_reflectivity_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MEAN_SURFACE_REFLECTIVITY_LAND =
  "lat_band_sensor_band_mean_surface_reflectivity_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_STD_DEV_SURFACE_REFLECTIVITY_LAND =
  "lat_band_sensor_band_std_dev_surface_reflectivity_land";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MIN_SURFACE_REFLECTIVITY_SEA =
  "lat_band_sensor_band_min_surface_reflectivity_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MAX_SURFACE_REFLECTIVITY_SEA =
  "lat_band_sensor_band_max_surface_reflectivity_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_MEAN_SURFACE_REFLECTIVITY_SEA =
  "lat_band_sensor_band_mean_surface_reflectivity_sea";
const char* GoesRnetCDF2Mdv::LAT_BAND_SENSOR_BAND_STD_DEV_SURFACE_REFLECTIVITY_SEA =
  "lat_band_sensor_band_std_dev_surface_reflectivity_sea";


// Level-2 Cloud Top Phase File (D99)
const char* GoesRnetCDF2Mdv::CLOUD_TOP_PHASE = "Phase";
const char* GoesRnetCDF2Mdv::TOTAL_NUMBER_CLOUDY_PIXELS = "total_number_cloudy_pixels";


// Level-2 Cloud Top Height File (G99)
const char* GoesRnetCDF2Mdv::CLOUD_TOP_HEIGHT = "HT";
const char* GoesRnetCDF2Mdv::MIN_CLOUD_TOP_HEIGHT = "minimum_cloud_top_height";
const char* GoesRnetCDF2Mdv::MAX_CLOUD_TOP_HEIGHT = "maximum_cloud_top_height";
const char* GoesRnetCDF2Mdv::MEAN_CLOUD_TOP_HEIGHT = "mean_cloud_top_height";
const char* GoesRnetCDF2Mdv::STD_DEV_CLOUD_TOP_HEIGHT = "std_dev_cloud_top_height";


// Level-2 Clear Sky Mask File (H99)
const char* GoesRnetCDF2Mdv::CLEAR_SKY_MASK = "BCM";
const char* GoesRnetCDF2Mdv::TOTAL_NUMBER_OF_CLOUD_MASK_POINTS =
  "total_number_of_cloud_mask_points";
const char* GoesRnetCDF2Mdv::NUMBER_OF_CLEAR_PIXELS = "number_of_clear_pixels";
const char* GoesRnetCDF2Mdv::NUMBER_OF_PROBABLY_CLEAR_PIXELS =
  "number_of_probably_clear_pixels";
const char* GoesRnetCDF2Mdv::NUMBER_OF_PROBABLY_CLOUDY_PIXELS =
  "number_of_probably_cloudy_pixels";
const char* GoesRnetCDF2Mdv::NUMBER_OF_CLOUDY_PIXELS = "number_of_cloudy_pixels";
const char* GoesRnetCDF2Mdv::PERCENT_CLEAR_PIXELS = "percent_clear_pixels";
const char* GoesRnetCDF2Mdv::PERCENT_PROBABLY_CLEAR_PIXELS =
  "percent_probably_clear_pixels";
const char* GoesRnetCDF2Mdv::MIN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY =
  "min_obs_modeled_diff_RTM_BT_comparison_bands_all_sky";
const char* GoesRnetCDF2Mdv::MAX_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY =
  "max_obs_modeled_diff_RTM_BT_comparison_bands_all_sky";
const char* GoesRnetCDF2Mdv::MEAN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY =
  "mean_obs_modeled_diff_RTM_BT_comparison_bands_all_sky";
const char* GoesRnetCDF2Mdv::STD_DEV_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_ALL_SKY =
  "std_dev_obs_modeled_diff_RTM_BT_comparison_bands_all_sky";
const char* GoesRnetCDF2Mdv::MIN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY =
  "min_obs_modeled_diff_RTM_BT_comparison_bands_clear_sky";
const char* GoesRnetCDF2Mdv::MAX_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY =
  "max_obs_modeled_diff_RTM_BT_comparison_bands_clear_sky";
const char* GoesRnetCDF2Mdv::MEAN_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY =
  "mean_obs_modeled_diff_RTM_BT_comparison_bands_clear_sky";
const char* GoesRnetCDF2Mdv::STD_DEV_OBS_MODELED_DIFF_RTM_BT_COMPARISON_BANDS_CLEAR_SKY =
  "std_dev_obs_modeled_diff_RTM_BT_comparison_bands_clear_sky";
const char* GoesRnetCDF2Mdv::RTM_BT_COMPARISON_WAVELENGTHS =
  "RTM_BT_comparison_wavelengths";
const char* GoesRnetCDF2Mdv::RTM_BT_COMPARISON_BAND_IDS =
  "RTM_BT_comparison_band_ids";


// Level-2 Cloud Top Temperature File (I99)
const char* GoesRnetCDF2Mdv::CLOUD_TOP_TEMP = "TEMP";
const char* GoesRnetCDF2Mdv::MIN_CLOUD_TOP_TEMPERATURE = "minimum_cloud_top_temperature";
const char* GoesRnetCDF2Mdv::MAX_CLOUD_TOP_TEMPERATURE = "maximum_cloud_top_temperature";
const char* GoesRnetCDF2Mdv::MEAN_CLOUD_TOP_TEMPERATURE = "mean_cloud_top_temperature";
const char* GoesRnetCDF2Mdv::STD_DEV_CLOUD_TOP_TEMPERATURE = "std_dev_cloud_top_temperature";


// Level-2 Fire Weather File (J99)
const char* GoesRnetCDF2Mdv::FIRE_AREA = "Area";
const char* GoesRnetCDF2Mdv::FIRE_TEMP = "Temp";
const char* GoesRnetCDF2Mdv::FIRE_MASK = "Mask";
const char* GoesRnetCDF2Mdv::FIRE_POWER = "Power";
const char* GoesRnetCDF2Mdv::TOTAL_NUMBER_OF_PIXELS_WITH_FIRES_DETECTED =
  "total_number_of_pixels_with_fires_detected";
const char* GoesRnetCDF2Mdv::TOTAL_NUMBER_OF_PIXELS_WITH_FIRE_TEMPERATURE_AND_AREA =
  "total_number_of_pixels_with_fire_temperature_and_area";
const char* GoesRnetCDF2Mdv::TOTAL_NUMBER_OF_PIXELS_WITH_FIRE_RADIATIVE_POWER =
  "total_number_of_pixels_with_fire_radiative_power";
const char* GoesRnetCDF2Mdv::FIRE_TEMPERATURE_OUTLIER_PIXEL_COUNT =
  "fire_temperature_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::FIRE_AREA_OUTLIER_PIXEL_COUNT =
  "fire_area_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::FIRE_RADIATIVE_POWER_OUTLIER_PIXEL_COUNT =
  "fire_radiative_power_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::MIN_FIRE_TEMPERATURE = "minimum_fire_temperature";
const char* GoesRnetCDF2Mdv::MAX_FIRE_TEMPERATURE = "maximum_fire_temperature";
const char* GoesRnetCDF2Mdv::MEAN_FIRE_TEMPERATURE = "mean_fire_temperature";
const char* GoesRnetCDF2Mdv::STD_DEV_FIRE_TEMPERATURE =
  "standard_deviation_fire_temperature";
const char* GoesRnetCDF2Mdv::MIN_FIRE_AREA = "minimum_fire_area";
const char* GoesRnetCDF2Mdv::MAX_FIRE_AREA = "maximum_fire_area";
const char* GoesRnetCDF2Mdv::MEAN_FIRE_AREA = "mean_fire_area";
const char* GoesRnetCDF2Mdv::STD_DEV_FIRE_AREA = "standard_deviation_fire_area";
const char* GoesRnetCDF2Mdv::MIN_FIRE_RADIATIVE_POWER = "minimum_fire_radiative_power";
const char* GoesRnetCDF2Mdv::MAX_FIRE_RADIATIVE_POWER = "maximum_fire_radiative_power";
const char* GoesRnetCDF2Mdv::MEAN_FIRE_RADIATIVE_POWER = "mean_fire_radiative_power";
const char* GoesRnetCDF2Mdv::STD_DEV_FIRE_RADIATIVE_POWER =
  "standard_deviation_fire_radiative_power";


// Level-2 Land surface Temp File (K99)
const char* GoesRnetCDF2Mdv::LAND_SURFACE_TEMP = "LST";
const char* GoesRnetCDF2Mdv::TOTAL_PIXELS_WHERE_LST_IS_RETRIEVED =
  "total_pixels_where_lst_is_retrieved";
const char* GoesRnetCDF2Mdv::NUMBER_GOOD_RETRIEVALS = "number_good_retrievals";
const char* GoesRnetCDF2Mdv::MIN_LAND_SURFACE_TEMP = "min_lst";
const char* GoesRnetCDF2Mdv::MAX_LAND_SURFACE_TEMP = "max_lst";
const char* GoesRnetCDF2Mdv::MEAN_LAND_SURFACE_TEMP = "mean_lst";
const char* GoesRnetCDF2Mdv::STD_DEV_LAND_SURFACE_TEMP = "standard_deviation_lst";


// Level-2 Stability Indicies File (N99)
const char* GoesRnetCDF2Mdv::LIFTED_INDEX = "LI";
const char* GoesRnetCDF2Mdv::CAPE = "CAPE";
const char* GoesRnetCDF2Mdv::TOTAL_TOTALS = "TT";
const char* GoesRnetCDF2Mdv::SHOWALTER_INDEX = "SI";
const char* GoesRnetCDF2Mdv::K_INDEX = "KI";
const char* GoesRnetCDF2Mdv::FINAL_AIR_PRESSURE = "final_air_pressure";
const char* GoesRnetCDF2Mdv::CAPE_OUTLIER_PIXEL_COUNT = "CAPE_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::LIFTED_INDEX_OUTLIER_PIXEL_COUNT =
  "lifted_index_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::K_INDEX_OUTLIER_PIXEL_COUNT = "k_index_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::SHOWALTER_INDEX_OUTLIER_PIXEL_COUNT =
  "showalter_index_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::TOTAL_TOTALS_INDEX_OUTLIER_PIXEL_COUNT =
  "total_totals_index_outlier_pixel_count";
const char* GoesRnetCDF2Mdv::MIN_CAPE = "minimum_CAPE";
const char* GoesRnetCDF2Mdv::MAX_CAPE = "maximum_CAPE";
const char* GoesRnetCDF2Mdv::MEAN_CAPE = "mean_CAPE";
const char* GoesRnetCDF2Mdv::STD_DEV_CAPE = "standard_deviation_CAPE";
const char* GoesRnetCDF2Mdv::MIN_LIFTED_INDEX = "minimum_lifted_index";
const char* GoesRnetCDF2Mdv::MAX_LIFTED_INDEX = "maximum_lifted_index";
const char* GoesRnetCDF2Mdv::MEAN_LIFTED_INDEX = "mean_lifted_index";
const char* GoesRnetCDF2Mdv::STD_DEV_LIFTED_INDEX = "standard_deviation_lifted_index";
const char* GoesRnetCDF2Mdv::MIN_SHOWALTER_INDEX = "minimum_showalter_index";
const char* GoesRnetCDF2Mdv::MAX_SHOWALTER_INDEX = "maximum_showalter_index";
const char* GoesRnetCDF2Mdv::MEAN_SHOWALTER_INDEX = "mean_showalter_index";
const char* GoesRnetCDF2Mdv::STD_DEV_SHOWALTER_INDEX =
  "standard_deviation_showalter_index";
const char* GoesRnetCDF2Mdv::MIN_TOTAL_TOTALS_INDEX = "minimum_total_totals_index";
const char* GoesRnetCDF2Mdv::MAX_TOTAL_TOTALS_INDEX = "maximum_total_totals_index";
const char* GoesRnetCDF2Mdv::MEAN_TOTAL_TOTALS_INDEX = "mean_total_totals_index";
const char* GoesRnetCDF2Mdv::STD_DEV_TOTAL_TOTALS_INDEX =
  "standard_deviation_total_totals_index";
const char* GoesRnetCDF2Mdv::MIN_K_INDEX = "minimum_K_INDEX";
const char* GoesRnetCDF2Mdv::MAX_K_INDEX = "maximum_K_INDEX";
const char* GoesRnetCDF2Mdv::MEAN_K_INDEX = "mean_K_INDEX";
const char* GoesRnetCDF2Mdv::STD_DEV_K_INDEX = "standard_deviation_K_INDEX";


// Level-2 Total Precipitable Water File (O99)
const char* GoesRnetCDF2Mdv::TOTAL_PRECIPITABLE_WATER = "TWP";
const char* GoesRnetCDF2Mdv::MIN_TOTAL_PRECIPITABLE_WATER =
  "minimum_total_precipitable_water";
const char* GoesRnetCDF2Mdv::MAX_TOTAL_PRECIPITABLE_WATER =
  "maximum_total_precipitable_water";
const char* GoesRnetCDF2Mdv::MEAN_TOTAL_PRECIPITABLE_WATER =
  "mean_total_precipitable_water";
const char* GoesRnetCDF2Mdv::STD_DEV_TOTAL_PRECIPITABLE_WATER =
  "standard_deviation_total_precipitable_water";


// Level-2 Derived Motion Winds File (U99)
// Not implementing ingest of this product for now


// Level-2 Cloud Particle Size File (W01)
const char* GoesRnetCDF2Mdv::CLOUD_PARTICLE_SIZE = "PSD";
const char* GoesRnetCDF2Mdv::OUTLIER_PSD_DAY = "outlier_PSD_day";
const char* GoesRnetCDF2Mdv::OUTLIER_PSD_NIGHT = "outlier_PSD_night";
const char* GoesRnetCDF2Mdv::MIN_PSD_DAY = "minimum_PSD_day";
const char* GoesRnetCDF2Mdv::MAX_PSD_DAY = "maximum_PSD_day";
const char* GoesRnetCDF2Mdv::MEAN_PSD_DAY = "mean_PSD_day";
const char* GoesRnetCDF2Mdv::STD_DEV_PSD_DAY = "std_dev_PSD_day";
const char* GoesRnetCDF2Mdv::MIN_PSD_NIGHT = "minimum_PSD_night";
const char* GoesRnetCDF2Mdv::MAX_PSD_NIGHT = "maximum_PSD_night";
const char* GoesRnetCDF2Mdv::MEAN_PSD_NIGHT = "mean_PSD_night";
const char* GoesRnetCDF2Mdv::STD_DEV_PSD_NIGHT = "std_dev_PSD_night";


// Level-2 Cloud Top Pressure File (X01)
const char* GoesRnetCDF2Mdv::CLOUD_TOP_PRESSURE = "PRES";
const char* GoesRnetCDF2Mdv::MIN_CLOUD_TOP_PRESSURE = "minimum_cloud_top_pressure";
const char* GoesRnetCDF2Mdv::MAX_CLOUD_TOP_PRESSURE = "maximum_cloud_top_pressure";
const char* GoesRnetCDF2Mdv::MEAN_CLOUD_TOP_PRESSURE = "mean_cloud_top_pressure";
const char* GoesRnetCDF2Mdv::STD_DEV_CLOUD_TOP_PRESSURE = "std_dev_cloud_top_pressure";

// Level-2 Cloud Optical Depth File (Y01)
const char* GoesRnetCDF2Mdv::CLOUD_OPTIC_DEPTH = "COD";
const char* GoesRnetCDF2Mdv::MIN_COD_DAY = "minimum_COD_day";
const char* GoesRnetCDF2Mdv::MAX_COD_DAY = "maximum_COD_day";
const char* GoesRnetCDF2Mdv::MEAN_COD_DAY = "mean_COD_day";
const char* GoesRnetCDF2Mdv::STD_DEV_COD_DAY = "std_dev_COD_day";
const char* GoesRnetCDF2Mdv::MIN_COD_NIGHT = "minimum_COD_night";
const char* GoesRnetCDF2Mdv::MAX_COD_NIGHT = "maximum_COD_night";
const char* GoesRnetCDF2Mdv::MEAN_COD_NIGHT = "mean_COD_night";
const char* GoesRnetCDF2Mdv::STD_DEV_COD_NIGHT = "std_dev_COD_night";
const char* GoesRnetCDF2Mdv::COD_PRODUCT_WAVELENGTH = "cod_product_wavelength";
const char* GoesRnetCDF2Mdv::OUTLIER_COD_DAY = "outlier_COD_day";
const char* GoesRnetCDF2Mdv::OUTLIER_COD_NIGHT = "outlier_COD_night";

// Level-1 observation, Level-2 product, and data quality flag Variable Attributes
const char* GoesRnetCDF2Mdv::SCALE_FACTOR = "scale_factor";
const char* GoesRnetCDF2Mdv::ADD_OFFSET = "add_offset";
const char* GoesRnetCDF2Mdv::UNITS = "units";
const char* GoesRnetCDF2Mdv::AXIS = "axis";
const char* GoesRnetCDF2Mdv::LONG_NAME = "long_name";
const char* GoesRnetCDF2Mdv::STANDARD_NAME = "standard_name";
const char* GoesRnetCDF2Mdv::FILL_VALUE = "_FillValue";
const char* GoesRnetCDF2Mdv::UNSIGNED = "_Unsigned";
const char* GoesRnetCDF2Mdv::SENSOR_BAND_BIT_DEPTH = "sensor_band_bit_depth";
const char* GoesRnetCDF2Mdv::VALID_RANGE = "valid_range";
const char* GoesRnetCDF2Mdv::RESOLUTION = "resolution";
const char* GoesRnetCDF2Mdv::COORDINATES = "coordinates";
const char* GoesRnetCDF2Mdv::GRID_MAPPING = "grid_mapping";
const char* GoesRnetCDF2Mdv::CELL_METHODS = "cell_methods";
const char* GoesRnetCDF2Mdv::ANCILLARY_VARIABLES = "ancillary_variables";
const char* GoesRnetCDF2Mdv::FLAG_VALUES = "flag_values";
const char* GoesRnetCDF2Mdv::FLAG_MEANINGS = "flag_meanings";
const char* GoesRnetCDF2Mdv::FLAG_MASKS = "flag_masks";
const char* GoesRnetCDF2Mdv::NUMBER_OF_QF_VALUES = "number_of_qf_values";
const char* GoesRnetCDF2Mdv::PERCENT_GOOD_PIXEL_QF = "percent_good_pixel_qf";
const char* GoesRnetCDF2Mdv::PERCENT_CONDITIONALLY_USABLE_PIXEL_QF =
  "percent_conditionally_usable_pixel_qf";
const char* GoesRnetCDF2Mdv::PERCENT_OUT_OF_RANGE_PIXEL_QF =
  "percent_out_of_range_pixel_qf";
const char* GoesRnetCDF2Mdv::PERCENT_NO_VALUE_PIXEL_QF =
  "percent_no_value_pixel_qf";
const char* GoesRnetCDF2Mdv::BOUNDS = "bounds";
const char* GoesRnetCDF2Mdv::GRID_MAPPING_NAME = "grid_mapping_name";
const char* GoesRnetCDF2Mdv::PERSPECTIVE_POINT_HEIGHT = "perspective_point_height";
const char* GoesRnetCDF2Mdv::SEMI_MAJOR_AXIS = "semi_major_axis";
const char* GoesRnetCDF2Mdv::SEMI_MINOR_AXIS = "semi_minor_axis";
const char* GoesRnetCDF2Mdv::INVERSE_FLATTENING = "inverse_flattening";
const char* GoesRnetCDF2Mdv::LATITUDE_OF_PROJECTION_ORIGIN = "latitude_of_projection_origin";
const char* GoesRnetCDF2Mdv::LONGITUDE_OF_PROJECTION_ORIGIN = "longitude_of_projection_origin";
const char* GoesRnetCDF2Mdv::SWEEP_ANGLE_AXIS = "sweep_angle_axis";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_WESTBOUND_LONGITUDE = "geospatial_westbound_longitude";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_NORTHBOUND_LATITUDE = "geospatial_northbound_latitude";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_EASTBOUND_LONGITUDE = "geospatial_eastbound_longitude";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_SOUTHBOUND_LATITUDE = "geospatial_southbound_latitude";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_LAT_CENTER = "geospatial_lat_center";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_LON_CENTER = "geospatial_lon_center";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_LAT_NADIR = "geospatial_lat_nadir";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_LON_NADIR = "geospatial_lon_nadir";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_LAT_UNITS = "geospatial_lat_units";
const char* GoesRnetCDF2Mdv::GEOSPATIAL_LON_UNITS = "geospatial_lon_units";
const char* GoesRnetCDF2Mdv::INPUT_ABI_L0_DATA = "input_ABI_L0_data";
const char* GoesRnetCDF2Mdv::L1B_PROCESSING_PARM_VERSION = "L1b_processing_parm_version";
const char* GoesRnetCDF2Mdv::ALGORITHM_VERSION = "algorithm_version";
const char* GoesRnetCDF2Mdv::PRODUCT_VERSION = "product_version";


// Title strings that are used to set the product type
const char* GoesRnetCDF2Mdv::TITLE_LEVEL1B_RADIANCES = "ABI L1b Radiances";
const char* GoesRnetCDF2Mdv::TITLE_CLOUD_AND_MOISTURE_IMAGERY = "ABI L2 Cloud and Moisture Imagery";
const char* GoesRnetCDF2Mdv::TITLE_AEROSOL_DETECTION = "ABI L2 Aerosol Detection";
const char* GoesRnetCDF2Mdv::TITLE_AEROSOL_OPTICAL_DEPTH = "ABI L2 Aerosol Optical Depth";
const char* GoesRnetCDF2Mdv::TITLE_CLOUD_TOP_PHASE = "ABI L2 Cloud Top Phase";
const char* GoesRnetCDF2Mdv::TITLE_CLOUD_TOP_HEIGHT = "ABI L2 Cloud Top Height";
const char* GoesRnetCDF2Mdv::TITLE_CLOUD_TOP_TEMPERATURE = "ABI L2 Cloud Top Temperature";
const char* GoesRnetCDF2Mdv::TITLE_CLOUD_TOP_PRESSURE = "ABI L2 Cloud Top Pressure";
const char* GoesRnetCDF2Mdv::TITLE_DERIVED_STABILITY_INDICES = "ABI L2 Derived Stability Indices";
const char* GoesRnetCDF2Mdv::TITLE_TOTAL_PRECIPITABLE_WATER = "ABI L2 Total Precipitable Water";
const char* GoesRnetCDF2Mdv::TITLE_CLEAR_SKY_MASK = "ABI L2 Clear Sky Mask";
const char* GoesRnetCDF2Mdv::TITLE_FIRE_CHARACTERIZATION = "ABI L2 Fire - Hot Spot Characterization";
const char* GoesRnetCDF2Mdv::TITLE_LAND_SURFACE_TEMPERATURE = "ABI L2 Land Surface (Skin) Temperature";
const char* GoesRnetCDF2Mdv::TITLE_CLOUD_OPTICAL_DEPTH = "ABI L2 Cloud Optical Depth at 640 nm";
const char* GoesRnetCDF2Mdv::TITLE_CLOUD_PARTICLE_SIZE = "ABI L2 Cloud Particle Size";
const char* GoesRnetCDF2Mdv::TITLE_DERIVED_MOTION_WINDS = "ABI L2 Derived Motion Winds";
const char* GoesRnetCDF2Mdv::TITLE_GLOBAL_LIGHTNING = "GLM L2 Lightning Detections: Events, Groups, and Flashes";

// strings to identify type of scan
const char* GoesRnetCDF2Mdv::FULL_DISK_SCAN_NAME = "Full Disk";
const char* GoesRnetCDF2Mdv::CONUS_SCAN_NAME = "CONUS";
const char* GoesRnetCDF2Mdv::MESOSCALE_SCAN_NAME = "Mesoscale";

