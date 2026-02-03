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

/////////////////////////////////////////////////////////////
// CalcMoisture Main
//
// Nancy Rehak, RAL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2006
//
/////////////////////////////////////////////////////////////
//
// CalcMoisture calculates water vapor pressure (e) and dew
// point temperature based on the refractivity N field and
// the mean temperature and pressure values
// from a group of weather stations.
//
/////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include <Mdv/DsMdvx.hh>
#include <physics/physics.h>
#include <physics/stn_pressure.h>
#include <rapformats/WxObs.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/LogStream.hh>
#include <toolsa/LogStreamInit.hh>
#include <didss/DsInputPath.hh>

#include "CalcMoisture.hh"
#include "Params.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

CalcMoisture::CalcMoisture(int argc, char **argv)

{

  _input = NULL;
  okay = true;

  // set programe name

  _progName = "CalcMoisture";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    okay = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    okay = false;
  }

  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {

    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_data_age_secs,
			     PMU_auto_register, true);

  } else if (_params.mode == Params::ARCHIVE) {

    time_t startTime = DateTime::parseDateTime(_params.start_time);
    time_t endTime = DateTime::parseDateTime(_params.end_time);
    if (startTime == DateTime::NEVER) {
      cerr << "ERROR: CalcMoisture" << endl;
      cerr << "  bad start time: " << _params.start_time << endl;
      okay = false;
    }
    if (endTime == DateTime::NEVER) {
      cerr << "ERROR: CalcMoisture" << endl;
      cerr << "  bad end time: " << _params.end_time << endl;
      okay = false;
    }

    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     startTime, endTime);

  } else if (_params.mode == Params::FILELIST) {

    if (_args.inputFileList.size() == 0) {

      cerr << "ERROR: CalcMoisture" << endl;
      cerr << "  Mode is FILELIST"; 
      cerr << "  You must use -f to specify files on the command line."
           << endl;
      _args.usage(_progName, cerr);
      okay = false;
      
    } else {
      
      _input = new DsInputPath(_progName,
                               _params.debug >= Params::DEBUG_VERBOSE,
                               _args.inputFileList);

    }
    
  } // if (_params.mode == ...

  // init process mapper registration
  
  if (_params.mode == Params::REALTIME) {
    PMU_auto_init(_progName.c_str(),
                  _params.instance,
                  _params.procmap_register_interval);
  }
  
  // initialize logging
  LogStreamInit::init(false, false, true, true);
  LOG_STREAM_DISABLE(LogStream::WARNING);
  LOG_STREAM_DISABLE(LogStream::DEBUG);
  LOG_STREAM_DISABLE(LogStream::DEBUG_VERBOSE);
  LOG_STREAM_DISABLE(LogStream::DEBUG_EXTRA);
  if (_params.debug >= Params::DEBUG_EXTRA) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_EXTRA);
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    LOG_STREAM_ENABLE(LogStream::DEBUG_VERBOSE);
    LOG_STREAM_ENABLE(LogStream::DEBUG);
  } else if (_params.debug) {
    LOG_STREAM_ENABLE(LogStream::DEBUG);
    LOG_STREAM_ENABLE(LogStream::WARNING);
  }

}


/*********************************************************************
 * Destructor
 */

CalcMoisture::~CalcMoisture()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  if (_input) {
    delete _input;
  }
  
}

/*********************************************************************
 * run() - run the program.
 */

int CalcMoisture::run()
{

  int nSuccess = 0;

  char *inputPath = nullptr;
  while ((inputPath = _input->next()) != nullptr) {

    time_t inputTime;
    if (DsInputPath::getDataTime(inputPath, inputTime)) {
      cerr << "ERROR: CalcMoisture::run()" << endl;
      cerr << "  Cannot compute time from input file path: " << inputPath << endl;
      cerr << "  Ignoring this file" << endl;
      continue;
    }

    DateTime fileTime(inputTime);
    if (!_processData(inputPath, fileTime)) {
      cerr << "ERROR: CalcMoisture::run()" << endl;
      cerr << "  processing data for time: " << fileTime << endl;
      continue;
    }

    nSuccess++;
    
  } // while ...
  
  if (_params.debug) {
    cerr << "==>>end of data" << endl;
  }

  if (nSuccess > 0) {
    return 0;
  } else {
    return -1;
  }
  
}

/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns 0 on success, -1 on failure.
 */

int CalcMoisture::_processData(const string filePath,
                               const DateTime &fileTime)
  
{

  static const string method_name = "CalcMoisture::_processData()";
  
  if (_params.debug) {
    cerr << endl << "*** Processing data for time: " << fileTime << endl;
  }
  
  PMU_auto_register("Processing data");
  
  // Read in the N field

  DsMdvx input_file;
  input_file.setReadTime(Mdvx::READ_CLOSEST,
                         _params.input_dir,
                         0, fileTime.utime());
  
  input_file.addReadField(_params.n_field_name);
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    input_file.printReadRequest(cerr);
  }
  
  if (input_file.readVolume() != 0) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading N field:" << endl;
    cerr << "   time: " << fileTime.asString(0) << endl;
    cerr << "   dir: " << _params.input_dir << endl;
    cerr << "   field name: " << _params.n_field_name << endl;
    cerr << input_file.getErrStr() << endl;
    return -1;
  }
  
  MdvxField *n_field;
  if ((n_field = input_file.getField(0)) == 0) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving N field from MDV file" << endl;
    return -1;
  }
  
  // Get the mean temperature and pressure from the given stations

  double temp_k, press_mb;
  if (!_getTempPress(fileTime, temp_k, press_mb)) {
    return -1;
  }

  if (_params.debug) {
    cerr << "---> Mean temp = " << temp_k << " K" << endl;
    cerr << "     Mean press = " << press_mb << " mb" << endl;
  }
  
  // Calculate the water vapor pressure field

  MdvxField *e_field;
  if ((e_field = _calcWaterVaporField(*n_field, temp_k, press_mb)) == nullptr) {
    return -1;
  }
  
  // Calculate the dew point field

  MdvxField *td_field;
  if ((td_field = _calcDewPointField(*e_field)) == nullptr) {
    return -1;
  }
  
  // Calculate the relative humidity field

  MdvxField *rh_field;
  if ((rh_field = _calcRelativeHumidityField(*td_field, temp_k)) == nullptr) {
    return -1;
  }
  
  // Create the output file

  DsMdvx output_file;
  
  Mdvx::master_header_t master_hdr = input_file.getMasterHeader();
  
  master_hdr.num_data_times = 0;
  master_hdr.index_number = 0;
  master_hdr.n_fields = 0;
  master_hdr.max_nx = 0;
  master_hdr.max_ny = 0;
  master_hdr.max_nz = 0;
  master_hdr.n_chunks = 0;
  STRcopy(master_hdr.data_set_info, "CalcMoisture", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "CalcMoisture", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_file.getPathInUse().c_str(),
	  MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
  
  // compress

  e_field->convertType(Mdvx::ENCODING_FLOAT32,
                       Mdvx::COMPRESSION_GZIP);
  
  td_field->convertType(Mdvx::ENCODING_FLOAT32,
                        Mdvx::COMPRESSION_GZIP);
  
  rh_field->convertType(Mdvx::ENCODING_FLOAT32,
                        Mdvx::COMPRESSION_GZIP);
  
  output_file.addField(e_field);
  output_file.addField(td_field);
  output_file.addField(rh_field);
  
  // Write the output file

  output_file.setWriteLdataInfo();
  if (output_file.writeToDir(_params.output_dir)) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing to output URL: " << _params.output_dir << endl;
    cerr << output_file.getErrStr() << endl;
    return -1;
  }
  
  return 0;
  
}

/*********************************************************************
 * _calcDewPointField() - Calculate the dew point field from the given
 *                        data.
 *
 * Returns a pointer to the dew point field on success, 0 on failure.
 */

MdvxField *CalcMoisture::_calcDewPointField(const MdvxField &e_field)
{
  
  static const string method_name = "CalcMoisture::_calcDewPointField()";
  
  // Start with a copy of the water vapor pressure field

  MdvxField *td_field = new MdvxField(e_field);
  
  // Update the field header values

  Mdvx::field_header_t field_hdr = td_field->getFieldHeader();
  
  field_hdr.field_code = 0;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  field_hdr.min_value_orig_vol = 0.0;
  field_hdr.max_value_orig_vol = 0.0;
  STRcopy(field_hdr.field_name_long, "Dew Point", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "Td", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "C", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  td_field->setFieldHeader(field_hdr);
  
  // Update the data values

  fl32 *data = (fl32 *)td_field->getVol();
  int vol_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  
  for (int i = 0; i < vol_size; ++i)
  {
    if (data[i] == field_hdr.missing_data_value ||
	data[i] == field_hdr.bad_data_value)
      continue;
    
    // Cannot take the log of a negative number

    if (data[i] <= 0.0)
    {
      data[i] = field_hdr.bad_data_value;
      continue;
    }
    
    double log_e = log(data[i] / 10.0);
    
    data[i] = (116.9 + (237.3 * log_e)) / (16.78 - log_e);
  }
  
  return td_field;
}


/*********************************************************************
 * _calcRelativeHumidityField() - Calculate the relative humidity field
 *                                from the given dewpoint data.
 *
 * Returns a pointer to the RH field on success, 0 on failure.
 */

MdvxField *CalcMoisture::_calcRelativeHumidityField(const MdvxField &dp_field,
                                                    const double temp_k)

{

  static const string method_name = "CalcMoisture::_calcRelativeHumidityField()";
  
  // Start with a copy of the dewpoint field

  MdvxField *rh_field = new MdvxField(dp_field);
  
  // Update the field header values

  Mdvx::field_header_t field_hdr = rh_field->getFieldHeader();
  
  field_hdr.field_code = 0;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  field_hdr.min_value_orig_vol = 0.0;
  field_hdr.max_value_orig_vol = 0.0;
  STRcopy(field_hdr.field_name_long, "Relative Humidity", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "RH", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "%", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  rh_field->setFieldHeader(field_hdr);
  
  // Update the data values

  fl32 *data = (fl32 *)rh_field->getVol();
  int vol_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  
  for (int i = 0; i < vol_size; ++i) {

    if (data[i] == field_hdr.missing_data_value ||
	data[i] == field_hdr.bad_data_value) {
      continue;
    }
    
    // Cannot take the log of a negative number
    
    if (data[i] <= 0.0) {
      data[i] = field_hdr.bad_data_value;
      continue;
    }
    
    data[i] = PHYhumidity(temp_k, TEMP_C_TO_K(data[i]));
    
  }
  
  return rh_field;
  
}


/*********************************************************************
 * _calcWaterVaporField() - Calculate the water vapor field from the
 *                          given data.
 *
 * Returns a pointer to the water vapor field on success, 0 on failure.
 */

MdvxField *CalcMoisture::_calcWaterVaporField(const MdvxField &n_field,
					      const double temp_k,
					      const double press_mb)
{

  static const string method_name = "CalcMoisture::_calcWaterVaporField()";
  
  // Start with a copy of the N field
  
  MdvxField *e_field = new MdvxField(n_field);
  
  // Update the field header values

  Mdvx::field_header_t field_hdr = e_field->getFieldHeader();
  
  field_hdr.field_code = 0;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  field_hdr.min_value_orig_vol = 0.0;
  field_hdr.max_value_orig_vol = 0.0;
  STRcopy(field_hdr.field_name_long, "Water Vapor Pressure",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "e", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "mb", MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  e_field->setFieldHeader(field_hdr);
  
  // Update the data values

  fl32 *data = (fl32 *)e_field->getVol();
  int vol_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  
  for (int i = 0; i < vol_size; ++i) {

    if (data[i] == field_hdr.missing_data_value ||
	data[i] == field_hdr.bad_data_value) {
      continue;
    }
    
    // There's a problem with the N field where the missing data value
    // is being handled incorrectly somehow.  Artificially correct for this
    // for now.

    if (data[i] < -300.0) {
      data[i] = field_hdr.bad_data_value;
      continue;
    }
    
    data[i] = (temp_k * temp_k) *
              (data[i] - 77.6 * (press_mb / temp_k)) / 373000.0;
    
  }
  
  return e_field;
  
}


/*********************************************************************
 * _getTempPress() - Get the mean temperature (in K) and pressure (in mb)
 *                   values using the stations specifiec in the parameter
 *                   file.
 *
 * Returns 0 on success, -1 on failure.
 */

int CalcMoisture::_getTempPress(const DateTime &data_time,
                                double &temp_k, double &press_mb)
  
{

  static const string method_name = "CalcMoisture::_getTempPress()";
  
  // Read in the station data

  double temp_sum = 0.0;
  double press_sum = 0.0;
  int num_temp_stations = 0;
  int num_press_stations = 0;
  
  for (int i = 0; i < _params.station_list_n; ++i) {

    int data_type = Spdb::hash4CharsToInt32(_params._station_list[i].name);
    
    DsSpdb spdb;
    if (spdb.getClosest(_params.station_url,
			data_time.utime(),
			_params.max_station_valid_secs,
			data_type) != 0) {

      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading station data for station: "
	   << _params._station_list[i].name << endl;
      cerr << spdb.getErrStr() << endl;
      
      return -1;

    }
    
    if (spdb.getNChunks() <= 0) {
      if (_params.debug)
	cerr << "No chunks found in database for station "
	     << _params._station_list[i].name << endl;
      
      continue;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Successfully read " << spdb.getNChunks()
	   << " chunks for station " << _params._station_list[i].name << endl;
    }
    
    // If we get here, we got some weather obs data.  There should only be one
    // chunk of data, but if there is more for some reason, then just use
    // the first chunk.

    const vector< Spdb::chunk_t > &chunks = spdb.getChunks();
    WxObs obs;
    obs.disassemble(chunks[0].data, chunks[0].len);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Station " << _params._station_list[i].name << " info:" << endl;
      cerr << "    station_id: " << obs.getStationId() << endl;
    }
    
    double temperature = obs.getTempC();
    double pressure = obs.getPressureMb();

    if (pressure == obs.missing) {

      double msl_pressure = obs.getSeaLevelPressureMb();
      double elevation = _params._station_list[i].elevation;
      if (_params.get_elevation_from_data) {
        elevation = obs.getElevationMeters();
      }

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "    station pressure value missing -- calculating" << endl;
        if (msl_pressure == obs.missing) {
          cerr << "    msl_pressure: MISSING" << endl;
        } else {
          cerr << "    msl_pressure: " << msl_pressure << " mb" << endl;
        }
        if (elevation == obs.missing) {
          cerr << "    elevation: MISSING" << endl;
        } else {
          cerr << "    elevation: " << elevation << " m" << endl;
        }
      }
    
      if (msl_pressure != obs.missing && elevation != obs.missing) {
        pressure = SL2StnPressure(msl_pressure, elevation);
      }
      
    } // if (pressure == obs.missing)

    if (_params.debug >= Params::DEBUG_VERBOSE) {

      if (temperature == obs.missing) {
	cerr << "    temperature: MISSING" << endl;
      } else {
	cerr << "    temperature: " << temperature << " C" << endl;
      }
      if (pressure == obs.missing) {
	cerr << "    pressure: MISSING" << endl;
      } else {
	cerr << "    pressure: " << pressure << " mb" << endl;
      }
    }
    
    if (temperature != obs.missing) {
      temp_sum += temperature;
      ++num_temp_stations;
    }
    
    if (pressure != obs.missing) {
      press_sum += pressure;
      ++num_press_stations;
    }
    
  } /* endfor - i */
  
  if (num_temp_stations <= 0 || num_press_stations <= 0) {
    cerr << "ERROR: " << method_name << endl;
    if (num_temp_stations <= 0) {
      cerr << "No temperature data found for time period for given stations" << endl;
    }
    if (num_press_stations <= 0) {
      cerr << "No pressure data found for time period for given stations" << endl;
    }
    cerr << "Cannot calculate moisture fields" << endl;
    return -1;
  }
  
  temp_k = TEMP_C_TO_K(temp_sum / (double)num_temp_stations);
  press_mb = press_sum / (double)num_press_stations;
  
  return 0;
  
}

