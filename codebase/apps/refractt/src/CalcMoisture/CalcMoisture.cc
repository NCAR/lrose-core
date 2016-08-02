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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 18:17:26 $
//   $Id: CalcMoisture.cc,v 1.9 2016/03/07 18:17:26 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CalcMoisture: CalcMoisture program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <physics/physics.h>
#include <physics/stn_pressure.h>
#include <rapformats/WxObs.hh>
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "CalcMoisture.hh"
#include "Params.hh"

using namespace std;

// Global variables

CalcMoisture *CalcMoisture::_instance =
     (CalcMoisture *)NULL;


/*********************************************************************
 * Constructor
 */

CalcMoisture::CalcMoisture(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "CalcMoisture::CalcMoisture()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CalcMoisture *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
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

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

CalcMoisture *CalcMoisture::Inst(int argc, char **argv)
{
  if (_instance == (CalcMoisture *)NULL)
    new CalcMoisture(argc, argv);
  
  return(_instance);
}

CalcMoisture *CalcMoisture::Inst()
{
  assert(_instance != (CalcMoisture *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool CalcMoisture::init()
{
  static const string method_name = "CalcMoisture::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void CalcMoisture::run()
{
  static const string method_name = "CalcMoisture::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
  if (_params->debug)
    cerr << "_dataTrigger->endOfData() returned true" << endl;
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcDewPointField() - Calculate the dew point field from the given
 *                        data.
 *
 * Returns a pointer to the dew point field on success, 0 on failure.
 */

MdvxField *CalcMoisture::_calcDewPointField(const MdvxField &e_field) const
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
                                                    const double temp_k) const
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
					      const double press_mb) const
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
  
  for (int i = 0; i < vol_size; ++i)
  {
    if (data[i] == field_hdr.missing_data_value ||
	data[i] == field_hdr.bad_data_value)
      continue;
    
    // There's a problem with the N field where the missing data value
    // is being handled incorrectly somehow.  Artificially correct for this
    // for now.

    if (data[i] < -300.0)
    {
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
 * Returns true on success, false on failure.
 */

bool CalcMoisture::_getTempPress(const DateTime &data_time,
				 double &temp_k, double &press_mb)
{
  static const string method_name = "CalcMoisture::_getTempPress()";
  
  // Read in the station data

  double temp_sum = 0.0;
  double press_sum = 0.0;
  int num_temp_stations = 0;
  int num_press_stations = 0;
  
  for (int i = 0; i < _params->station_list_n; ++i)
  {
    int data_type = Spdb::hash4CharsToInt32(_params->_station_list[i].name);
    
    DsSpdb spdb;
    
    if (spdb.getClosest(_params->station_url,
			data_time.utime(),
			_params->max_station_valid_secs,
			data_type) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading station data for station: "
	   << _params->_station_list[i].name << endl;
      cerr << spdb.getErrStr() << endl;
      
      return false;
    }
    
    if (spdb.getNChunks() <= 0)
    {
      if (_params->debug)
	cerr << "No chunks found in database for station "
	     << _params->_station_list[i].name << endl;
      
      continue;
    }
    
    if (_params->debug)
      cerr << "Successfully read " << spdb.getNChunks()
	   << " chunks for station " << _params->_station_list[i].name << endl;
    
    // If we get here, we got some weather obs data.  There should only be one
    // chunk of data, but if there is more for some reason, then just use
    // the first chunk.

    const vector< Spdb::chunk_t > &chunks = spdb.getChunks();
    
    WxObs obs;
    obs.disassemble(chunks[0].data, chunks[0].len);

    if (_params->debug)
    {
      cerr << "Station " << _params->_station_list[i].name << " info:" << endl;
      cerr << "    station_id: " << obs.getStationId() << endl;
    }
    
    double temperature = obs.getTempC();
    double pressure = obs.getPressureMb();

    if (pressure == obs.missing)
    {
      double msl_pressure = obs.getSeaLevelPressureMb();
      double elevation = _params->_station_list[i].elevation;
      if (_params->get_elevation_from_data)
        elevation = obs.getElevationMeters();

      if (_params->debug)
      {
        cerr << "    station pressure value missing -- calculating" << endl;
        if (msl_pressure == obs.missing)
          cerr << "    msl_pressure: MISSING" << endl;
        else
          cerr << "    msl_pressure: " << msl_pressure << " mb" << endl;
        if (elevation == obs.missing)
          cerr << "    elevation: MISSING" << endl;
        else
          cerr << "    elevation: " << elevation << " m" << endl;
      }
    
      if (msl_pressure != obs.missing && elevation != obs.missing)
        pressure = SL2StnPressure(msl_pressure, elevation);
    }

    if (_params->debug)
    {
      if (temperature == obs.missing)
	cerr << "    temperature: MISSING" << endl;
      else
	cerr << "    temperature: " << temperature << " C" << endl;
      if (pressure == obs.missing)
	cerr << "    pressure: MISSING" << endl;
      else
	cerr << "    pressure: " << pressure << " mb" << endl;
    }
    
    if (temperature != obs.missing)
    {
      temp_sum += temperature;
      ++num_temp_stations;
    }
    
    if (pressure != obs.missing)
    {
      press_sum += pressure;
      ++num_press_stations;
    }
    
  } /* endfor - i */
  
  if (num_temp_stations <= 0 || num_press_stations <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    if (num_temp_stations <= 0)
      cerr << "No temperature data found for time period for given stations" << endl;
    if (num_press_stations <= 0)
      cerr << "No pressure data found for time period for given stations" << endl;
    cerr << "Cannot calculate moisture fields" << endl;
    
    return false;
  }
  
  temp_k = TEMP_C_TO_K(temp_sum / (double)num_temp_stations);
  press_mb = press_sum / (double)num_press_stations;
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool CalcMoisture::_initTrigger(void)
{
  static const string method_name = "CalcMoisture::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->n_field.url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->n_field.url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->n_field.url << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->n_field.url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->n_field.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->n_field.url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
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
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool CalcMoisture::_processData(const DateTime &trigger_time)
{
  static const string method_name = "CalcMoisture::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  PMU_auto_register("Processing data");
  
  // Read in the N field

  DsMdvx input_file;
  
  input_file.setReadTime(Mdvx::READ_CLOSEST,
		   _params->n_field.url,
		   0, trigger_time.utime());
  
  if (_params->n_field.use_field_name)
    input_file.addReadField(_params->n_field.field_name);
  else
    input_file.addReadField(_params->n_field.field_num);
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug)
    input_file.printReadRequest(cerr);
  
  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading N field:" << endl;
    cerr << "   Request time: " << trigger_time << endl;
    cerr << "   URL: " << _params->n_field.url;
    if (_params->n_field.use_field_name)
      cerr << "   Field name: " << _params->n_field.field_name << endl;
    else
      cerr << "   Field num: " << _params->n_field.field_num << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  MdvxField *n_field;
  
  if ((n_field = input_file.getField(0)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving N field from MDV file" << endl;
    
    return false;
  }
  
  // Get the mean temperature and pressure from the given stations

  double temp_k, press_mb;
  
  if (!_getTempPress(trigger_time, temp_k, press_mb))
    return false;

  if (_params->debug)
  {
    cerr << "---> Mean temp = " << temp_k << " K" << endl;
    cerr << "     Mean press = " << press_mb << " mb" << endl;
  }
  
  // Calculate the water vapor pressure field

  MdvxField *e_field;
  
  if ((e_field = _calcWaterVaporField(*n_field, temp_k, press_mb)) == 0)
    return false;
  
  // Calculate the dew point field

  MdvxField *td_field;
  
  if ((td_field = _calcDewPointField(*e_field)) == 0)
    return false;
  
  // Calculate the relative humidity field

  MdvxField *rh_field;
  
  if ((rh_field = _calcRelativeHumidityField(*td_field, temp_k)) == 0)
    return false;
  
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
  
  if (_params->compress_output_fields)
  {
    e_field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_BZIP,
			 Mdvx::SCALING_DYNAMIC);

    td_field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_BZIP,
			  Mdvx::SCALING_DYNAMIC);

    rh_field->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_BZIP,
			  Mdvx::SCALING_DYNAMIC);
  }

  output_file.addField(e_field);
  output_file.addField(td_field);
  output_file.addField(rh_field);
  
  // Write the output file

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing to output URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
