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
//   $Id: VerticalAverage.cc,v 1.7 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * VerticalAverage: VerticalAverage program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "VerticalAverage.hh"
#include "Params.hh"


// Global variables

VerticalAverage *VerticalAverage::_instance =
     (VerticalAverage *)NULL;


/*********************************************************************
 * Constructor
 */

VerticalAverage::VerticalAverage(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "VerticalAverage::VerticalAverage()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (VerticalAverage *)NULL);
  
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

VerticalAverage::~VerticalAverage()
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

VerticalAverage *VerticalAverage::Inst(int argc, char **argv)
{
  if (_instance == (VerticalAverage *)NULL)
    new VerticalAverage(argc, argv);
  
  return(_instance);
}

VerticalAverage *VerticalAverage::Inst()
{
  assert(_instance != (VerticalAverage *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool VerticalAverage::init()
{
  static const string method_name = "VerticalAverage::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
      
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
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
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

void VerticalAverage::run()
{
  static const string method_name = "VerticalAverage::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {

    TriggerInfo triggerInfo;

    
    if (_dataTrigger->next(triggerInfo) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(triggerInfo.getIssueTime(),
		      triggerInfo.getForecastTime() - triggerInfo.getIssueTime()))
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
 * _calcVerticalAverage() - Calculate the level average field for the
 *                       given 3D field.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *VerticalAverage::_calcVerticalAverage(const MdvxField &input_field) const
{
  static const string method_name = "VerticalAverage::_calcVerticalAverage()";
  
  MdvxField *vert_avg_field;
  
  // Create the level average field

  if ((vert_avg_field = _createLevelAvgField(input_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating level average field" << endl;
    
    return 0;
  }
  
  // Calculate the level averages

  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  Mdvx::field_header_t vert_avg_field_hdr = vert_avg_field->getFieldHeader();
  
  MdvxPjg input_proj(input_field_hdr);
  MdvxPjg vert_avg_proj(vert_avg_field_hdr);
  
  fl32 *input_data = (fl32 *)input_field.getVol();
  fl32 *vert_avg_data = (fl32 *)vert_avg_field->getVol();
  
  for (int x = 0; x < input_proj.getNx(); ++x)
  {
    for (int y = 0; y < input_proj.getNy(); ++y)
    {
      double vert_sum = 0.0;
      int num_levels = 0;
      
      for (int z = 0; z < input_proj.getNz(); ++z)
      {
	int input_index = input_proj.xyIndex2arrayIndex(x, y, z);
	
	if (input_index < 0)
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Internal error calculating data index" << endl;
	  cerr << "x = " << x << ", y = " << y << ", z = " << z << endl;
	  
	  return 0;
	}
	
	if (input_data[input_index] == input_field_hdr.bad_data_value ||
	    input_data[input_index] == input_field_hdr.missing_data_value)
	{
	  if (_params->include_missing_in_avg)
	  {
	    vert_sum += _params->missing_avg_value;
	    ++num_levels;
	  }
	}
	else
	{
	  vert_sum += input_data[input_index];
	  ++num_levels;
	}
	
      } /* endfor - z */
      
      int vert_avg_index = vert_avg_proj.xyIndex2arrayIndex(x, y, 0);
	
      if (vert_avg_index < 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Internal error calculating level avg data index" << endl;
	cerr << "x = " << x << ", y = " << y << endl;
	  
	return 0;
      }
	
      if (num_levels > 0)
	vert_avg_data[vert_avg_index] = vert_sum / (double)num_levels;
      else
	vert_avg_data[vert_avg_index] = vert_avg_field_hdr.missing_data_value;
      
    } /* endfor - y */
  } /* endfor - x */
  
  return vert_avg_field;
}


/*********************************************************************
 * _createLevelAvgField() - Create the blank level average field.  Upon
 *                          return, the field values will be set to the
 *                          input field's missing data value.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *VerticalAverage::_createLevelAvgField(const MdvxField &input_field) const
{
  // Create the new field header

  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = input_field_hdr.field_code;
  field_hdr.forecast_delta = input_field_hdr.forecast_delta;
  field_hdr.forecast_time = input_field_hdr.forecast_time;
  field_hdr.nx = input_field_hdr.nx;
  field_hdr.ny = input_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = input_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = input_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  
  field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = input_field_hdr.proj_param[i];
  field_hdr.vert_reference = 0;
  field_hdr.grid_dx = input_field_hdr.grid_dx;
  field_hdr.grid_dy = input_field_hdr.grid_dy;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = input_field_hdr.grid_minx;
  field_hdr.grid_miny = input_field_hdr.grid_miny;
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = input_field_hdr.bad_data_value;
  field_hdr.missing_data_value = input_field_hdr.missing_data_value;
  field_hdr.proj_rotation = input_field_hdr.proj_rotation;
  
  STRcopy(field_hdr.field_name_long, "level avg of ", MDV_LONG_FIELD_LEN);
  STRconcat(field_hdr.field_name_long, input_field_hdr.field_name_long,
	  MDV_LONG_FIELD_LEN);

  STRcopy(field_hdr.field_name, input_field_hdr.field_name,
	  MDV_SHORT_FIELD_LEN);
  STRconcat(field_hdr.field_name, " avg", MDV_SHORT_FIELD_LEN);
  
  STRcopy(field_hdr.units, input_field_hdr.units, MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "vertical avg", MDV_TRANSFORM_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.5;
  
  // Create the blank field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool VerticalAverage::_processData(const time_t trigger_time, int lead_time)
{
  static const string method_name = "VerticalAverage::_processData()";
  
  // Read in the input file

  DsMdvx input_file;
  
  if (_params->is_forecast_data)
    {
       input_file.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
			      _params->input_url,
			      0, trigger_time, lead_time);
    }
  else
    input_file.setReadTime(Mdvx::READ_CLOSEST,
			   _params->input_url,
			   0, trigger_time);
  
  input_file.clearReadFields();
  
  for (int i = 0; i < _params->input_fields_n; ++i)
  {
    if (_params->use_field_name)
      input_file.addReadField(_params->_input_fields[i].field_name);
    else
      input_file.addReadField(_params->_input_fields[i].field_num);
  } /* endfor - i */
  
  input_file.setReadVlevelLimits(_params->pressure_limits.lower_level,
				 _params->pressure_limits.upper_level);
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug)
  {
    cerr << endl;
    input_file.printReadRequest(cerr);
  }
  
  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV volume for time: " <<
      DateTime::str(trigger_time) << endl;
    
    return false;
  }
  
  // Create the output file

  DsMdvx output_file;
  
  _updateOutputMasterHeader(output_file,
			    input_file.getMasterHeader(),
			    input_file.getPathInUse(),
			    _params->write_as_forecast );
  
  // Calculate the level averages, adding each to the output file

  for (int field_num = 0; field_num < input_file.getMasterHeader().n_fields;
       ++field_num)
  {
    // Retrieve the field information from the input file

    MdvxField *input_field;
    
    if ((input_field = input_file.getField(field_num)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting field number " << field_num << 
	" from input file" << endl;

      if (_params->use_field_name)
      {
	cerr << "This should be field " <<
	  _params->_input_fields[field_num].field_name <<
	  " from the parameter file" << endl;
      }
      else
      {
	cerr << "This should be field " <<
	  _params->_input_fields[field_num].field_num <<
	  " from the parameter file" << endl;
      }
      
      cerr << "*** Skipping field ***" << endl;
      
      continue;
    }
    
    // Calculate the level average field

    MdvxField *vert_avg_field;
    
    if ((vert_avg_field = _calcVerticalAverage(*input_field)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error calculating level average for field <" <<
	input_field->getFieldHeader().field_name << ">" << endl;
      cerr << "*** Skipping field ***" << endl;
      
      continue;
    }
    
    // Add the level average field to the output file

    vert_avg_field->convertType(Mdvx::ENCODING_INT8,
				Mdvx::COMPRESSION_RLE,
				Mdvx::SCALING_DYNAMIC);
    
    output_file.addField(vert_avg_field);
    
  } /* endfor - field_num */
  
  // Write the output file
  if (_params->write_as_forecast)
    output_file.setWriteAsForecast();

  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing to output URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _updateOutputMasterHeader() - Update the master header values for
 *                               the output file.
 */

void VerticalAverage::_updateOutputMasterHeader(DsMdvx &output_file,
						const Mdvx::master_header_t &input_master_hdr,
						const string &data_set_source,
						const bool write_as_forecast)
{
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  if( write_as_forecast)
  {
    master_hdr.time_gen = input_master_hdr.time_gen;
  }
  else
  {
    master_hdr.time_gen = time(0);
  }

  master_hdr.time_begin = input_master_hdr.time_begin;
  master_hdr.time_end = input_master_hdr.time_end;
  master_hdr.time_centroid = input_master_hdr.time_centroid;
  master_hdr.time_expire = input_master_hdr.time_expire;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = input_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = input_master_hdr.grid_orientation;
  master_hdr.data_ordering = input_master_hdr.data_ordering;
  
  master_hdr.sensor_lon = input_master_hdr.sensor_lon;
  master_hdr.sensor_lat = input_master_hdr.sensor_lat;
  master_hdr.sensor_alt = input_master_hdr.sensor_alt;
  
  STRcopy(master_hdr.data_set_info, "VerticalAverage output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "VerticalAverage", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, data_set_source.c_str(), MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
