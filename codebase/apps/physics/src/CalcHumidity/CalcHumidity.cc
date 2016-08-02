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
//   $Date: 2016/03/06 23:15:37 $
//   $Id: CalcHumidity.cc,v 1.5 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CalcHumidity: CalcHumidity program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <physics/physics.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "CalcHumidity.hh"
#include "Params.hh"


// Global variables

CalcHumidity *CalcHumidity::_instance =
     (CalcHumidity *)NULL;


const fl32 CalcHumidity::HUMIDITY_MISSING_DATA_VALUE = -999.0;


/*********************************************************************
 * Constructor
 */

CalcHumidity::CalcHumidity(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "CalcHumidity::CalcHumidity()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CalcHumidity *)NULL);
  
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
  char *params_path = "unknown";
  
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

CalcHumidity::~CalcHumidity()
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

CalcHumidity *CalcHumidity::Inst(int argc, char **argv)
{
  if (_instance == (CalcHumidity *)NULL)
    new CalcHumidity(argc, argv);
  
  return(_instance);
}

CalcHumidity *CalcHumidity::Inst()
{
  assert(_instance != (CalcHumidity *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool CalcHumidity::init()
{
  static const string method_name = "CalcHumidity::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->latest_data_trigger << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->latest_data_trigger,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->latest_data_trigger << endl;
      
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
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->time_list_trigger.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->time_list_trigger.url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INTERVAL :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->interval_trigger.start_time);
    time_t end_time =
      DateTime::parseDateTime(_params->interval_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for INTERVAL trigger: " <<
	_params->interval_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for INTERVAL trigger: " <<
	_params->interval_trigger.end_time << endl;
      
      return false;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_trigger.interval,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL trigger" << endl;
      cerr << "     Start time: " << _params->interval_trigger.start_time << endl;
      cerr << "     End time: " << _params->interval_trigger.end_time << endl;
      cerr << "     Interval: " << _params->interval_trigger.interval <<
	" secs" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  } /* endswitch - _params->trigger_mode */
  
    
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void CalcHumidity::run()
{
  static const string method_name = "CalcHumidity::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time.utime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createHumidityField() - Create the empty humidity field.  The data
 *                          volume will be filled with missing data
 *                          values when returned.
 *
 * Returns a pointer to the newly created field on success, 0 on failure.
 */

MdvxField *CalcHumidity::_createHumidityField(const Mdvx::field_header_t base_field_hdr,
					      const Mdvx::vlevel_header_t base_vlevel_hdr)
{
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = 0;
  field_hdr.forecast_delta = base_field_hdr.forecast_delta;
  field_hdr.forecast_time = base_field_hdr.forecast_time;
  field_hdr.nx = base_field_hdr.nx;
  field_hdr.ny = base_field_hdr.ny;
  field_hdr.nz = base_field_hdr.nz;
  field_hdr.proj_type = base_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = base_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = base_field_hdr.vlevel_type;
  field_hdr.dz_constant = base_field_hdr.dz_constant;
  field_hdr.proj_origin_lat = base_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = base_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = base_field_hdr.proj_param[i];
  field_hdr.vert_reference = base_field_hdr.vert_reference;
  field_hdr.grid_dx = base_field_hdr.grid_dx;
  field_hdr.grid_dy = base_field_hdr.grid_dy;
  field_hdr.grid_dz = base_field_hdr.grid_dz;
  field_hdr.grid_minx = base_field_hdr.grid_minx;
  field_hdr.grid_miny = base_field_hdr.grid_miny;
  field_hdr.grid_minz = base_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = HUMIDITY_MISSING_DATA_VALUE;
  field_hdr.missing_data_value = HUMIDITY_MISSING_DATA_VALUE;
  field_hdr.proj_rotation = base_field_hdr.proj_rotation;

  // Set the min/max range explicitly because we know the possible
  // range of values and this keeps the Mdv library from having to
  // calculate these values.

  if (_params->output_range_0_to_100)
  {
    field_hdr.min_value = 0.0;
    field_hdr.max_value = 100.0;
  }
  else
  {
    field_hdr.min_value = 0.0;
    field_hdr.max_value = 1.0;
  }
  
  STRcopy(field_hdr.field_name_long, "Humidity", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "RH", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "%", MDV_UNITS_LEN);
  
  // We can just use the base vlevel header directly since ours
  // will be exactly the same.

  return new MdvxField(field_hdr,
		       base_vlevel_hdr,
		       (void *)NULL,
		       true);
}


/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool CalcHumidity::_processData(const time_t trigger_time)
{
  static const string method_name = "CalcHumidity::_processData()";
  
  if (_params->debug)
    cerr << "*** Processing data for time: " << DateTime::str(trigger_time) <<
      endl;
  
  // Read in all of the fields

  MdvxField *temperature_field;
  
  if ((temperature_field =
       _readField(trigger_time,
		  _params->temperature_field_info.url,
		  _params->temperature_field_info.field_name,
		  _params->temperature_field_info.field_num,
		  _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading temperature field from url: " <<
      _params->temperature_field_info.url << endl;
    cerr << "Field name = \"" << _params->temperature_field_info.field_name <<
      "\", field num = " << _params->temperature_field_info.field_num << endl;
    
    return false;
  }
  
  MdvxField *dew_point_field;
  
  if ((dew_point_field =
       _readField(trigger_time,
		  _params->dew_point_field_info.url,
		  _params->dew_point_field_info.field_name,
		  _params->dew_point_field_info.field_num,
		  _params->max_input_valid_secs)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading dew_point field from url: " <<
      _params->dew_point_field_info.url << endl;
    cerr << "Field name = \"" << _params->dew_point_field_info.field_name <<
      "\", field num = " << _params->dew_point_field_info.field_num << endl;
    
    return false;
  }
  
  // Make sure all of the fields are on the same projection

  Mdvx::field_header_t temperature_field_hdr =
    temperature_field->getFieldHeader();
  Mdvx::field_header_t dew_point_field_hdr =
    dew_point_field->getFieldHeader();
  
  Mdvx::vlevel_header_t temperature_vlevel_hdr =
    temperature_field->getVlevelHeader();
  
  MdvxPjg temperature_proj(temperature_field_hdr);
  MdvxPjg dew_point_proj(dew_point_field_hdr);
  
  if (temperature_proj != dew_point_proj)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot process fields -- projections don't match" <<endl;
    
    delete temperature_field;
    delete dew_point_field;
    
    return false;
  }

  // Create the output humidity field.  Put it on the same grid
  // as the input temperature field, which should be the same as
  // the grid for the input dew point field based on the above
  // check.

  MdvxField *humidity_field;

  if ((humidity_field = _createHumidityField(temperature_field_hdr,
					     temperature_vlevel_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating humidity field object" << endl;
    
    delete temperature_field;
    delete dew_point_field;
    
    return false;
  }
  
  // Calculate the humidity values

  Mdvx::field_header_t temp_field_hdr = temperature_field->getFieldHeader();
  Mdvx::field_header_t dewpt_field_hdr = dew_point_field->getFieldHeader();

  fl32 *temp_data = (fl32 *)temperature_field->getVol();
  fl32 *dewpt_data = (fl32 *)dew_point_field->getVol();
  fl32 *hum_data = (fl32 *)humidity_field->getVol();
  
  int num_data_values = temperature_field_hdr.nx *
    temperature_field_hdr.ny * temperature_field_hdr.nz;
  
  for (int i = 0; i < num_data_values; ++i)
  {
    if (temp_data[i] == temp_field_hdr.missing_data_value ||
	temp_data[i] == temp_field_hdr.bad_data_value ||
	dewpt_data[i] == dewpt_field_hdr.missing_data_value ||
	dewpt_data[i] == dewpt_field_hdr.bad_data_value)
      continue;

    if (_params->output_range_0_to_100)
      hum_data[i] = PHYhumidity(temp_data[i], dewpt_data[i]) * 100.0;
    else
      hum_data[i] = PHYhumidity(temp_data[i], dewpt_data[i]);
  } /* endfor - i */
  
  // Create and write the output file

  DsMdvx output_file;
  
  _updateMasterHeader(output_file, trigger_time, temperature_field_hdr);
  
  if (_params->include_input_fields)
  {
    temperature_field->convertType(Mdvx::ENCODING_INT8,
				   Mdvx::COMPRESSION_RLE,
				   Mdvx::SCALING_DYNAMIC);
    dew_point_field->convertType(Mdvx::ENCODING_INT8,
				 Mdvx::COMPRESSION_RLE,
				 Mdvx::SCALING_DYNAMIC);
    
    output_file.addField(temperature_field);
    output_file.addField(dew_point_field);
  }
  else
  {
    delete temperature_field;
    delete dew_point_field;
  }
  
  // Add the humidity field to the output file

  humidity_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_RLE,
			      Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(humidity_field);
  
  // Finally, write the output file

  output_file.setWriteLdataInfo();

  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output MDV file to URL: " <<
      _params->output_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readField() - Read the indicated field data.
 */

MdvxField *CalcHumidity::_readField(const time_t data_time,
				     const string &url,
				     const string &field_name,
				     const int field_num,
				     const int max_input_valid_secs)
{
  static const string method_name = "CalcHumidity::_readField()";
  
  // Set up the read request

  DsMdvx input_file;
  
  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 url,
			 max_input_valid_secs,
			 data_time);
  
  if (field_name.length() > 0)
    input_file.addReadField(field_name);
  else
    input_file.addReadField(field_num);
  
  input_file.setReadNoChunks();
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Now read the volume

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading volume:" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   field: \"" << field_name << "\" (" << field_num << ")" << endl;
    
    return 0;
  }
  
  // Make sure the data in the volume is ordered in the way we expect.

  Mdvx::master_header_t master_hdr = input_file.getMasterHeader();
  
  if (master_hdr.grid_orientation != Mdvx::ORIENT_SN_WE)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input grid has incorrect orientation" << endl;
    cerr << "Expecting " << Mdvx::orientType2Str(Mdvx::ORIENT_SN_WE) <<
      " orientation, got " <<
      Mdvx::orientType2Str(master_hdr.grid_orientation) <<
      " orientation" << endl;
    cerr << "Url: " << url << endl;
    
    return 0;
  }
  
  if (master_hdr.data_ordering != Mdvx::ORDER_XYZ)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input grid has incorrect data ordering" << endl;
    cerr << "Expectin " << Mdvx::orderType2Str(Mdvx::ORDER_XYZ) <<
      " ordering, got " << Mdvx::orderType2Str(master_hdr.data_ordering) <<
      " ordering" << endl;
    cerr << "Url: " << url << endl;
    
    return 0;
  }
  
  // Pull out the appropriate field and make a copy to be returned.
  // We must make a copy here because getField() returns a pointer
  // into the DsMdvx object and the object is automatically deleted
  // when we exit this method.

  MdvxField *field = input_file.getField(0);
  
  if (field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving field from volume:" << endl;
    cerr << "   url: " << url << endl;
    cerr << "   field: \"" << field_name << "\" (" << field_num << ")" << endl;
    
    return 0;
  }
  
  return new MdvxField(*field);
}


/*********************************************************************
 * _updateMasterheader() - Update the master header for the output file.
 */

void CalcHumidity::_updateMasterHeader(DsMdvx &output_file,
					const time_t data_time,
					const Mdvx::field_header_t &field_hdr)
{
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = data_time;
  master_hdr.time_end = data_time;
  master_hdr.time_centroid = data_time;
  master_hdr.time_expire = data_time;
  master_hdr.data_dimension = 3;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = field_hdr.vlevel_type;
  master_hdr.vlevel_type = field_hdr.vlevel_type;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info,
	  "Generated by CalcHumidity", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name,
	  "CalcHumidity", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source,
	  "CalcHumidity", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
