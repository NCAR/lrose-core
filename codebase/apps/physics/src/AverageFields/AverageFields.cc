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
//   $Id: AverageFields.cc,v 1.9 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AverageFields: AverageFields program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2002
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
#include <dsdata/DsMultipleTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "AverageFields.hh"
#include "GridAverager.hh"
#include "GridStandardDev.hh"
#include "GridSummer.hh"
#include "Params.hh"

using namespace std;

// Global variables

AverageFields *AverageFields::_instance =
     (AverageFields *)NULL;


/*********************************************************************
 * Constructor
 */

AverageFields::AverageFields(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "AverageFields::AverageFields()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (AverageFields *)NULL);
  
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

AverageFields::~AverageFields()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _gridCalc;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

AverageFields *AverageFields::Inst(int argc, char **argv)
{
  if (_instance == (AverageFields *)NULL)
    new AverageFields(argc, argv);
  
  return(_instance);
}

AverageFields *AverageFields::Inst()
{
  assert(_instance != (AverageFields *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool AverageFields::init()
{
  static const string method_name = "AverageFields::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the grid calculator

  switch (_params->calculation)
  {
  case Params::MEAN :
    _gridCalc = new GridAverager(_params->include_missing_in_avg,
				 _params->missing_avg_value,
				 _params->debug);
    break;

  case Params::STDDEV :
    _gridCalc = new GridStandardDev(_params->include_missing_in_avg,
				    _params->missing_avg_value,
				    _params->debug);
    break;
    
  case Params::SUM :
    _gridCalc = new GridSummer(_params->debug);
    break;
  }
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  if (_params->debug)
    cerr << "Finished call to PMU_auto_init" << endl;
  
  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void AverageFields::run()
{
  static const string method_name = "AverageFields::run()";
  
  DateTime trigger_time;
  
  if (_params->debug)
    cerr << "About to call _dataTrigger->endOfData()" << endl;
  
  while (!_dataTrigger->endOfData())
  {
    if (_params->debug)
      cerr << "_dataTrigger->endOfData() returned true" << endl;
  
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
  
  if (_params->debug)
    cerr << "_dataTrigger->endOfData() returned false" << endl;
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createAverageField() - Create the average field using the given
 *                         information.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *AverageFields::_createAverageField(const Mdvx::field_header_t input_field_hdr,
					      const Mdvx::vlevel_header_t input_vlevel_hdr) const
{
  // Create the new field header

  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = input_field_hdr.field_code;
  field_hdr.forecast_delta = input_field_hdr.forecast_delta;
  field_hdr.forecast_time = input_field_hdr.forecast_time;
  field_hdr.nx = input_field_hdr.nx;
  field_hdr.ny = input_field_hdr.ny;
  field_hdr.nz = input_field_hdr.nz;
  field_hdr.proj_type = input_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = input_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = input_field_hdr.vlevel_type;
  field_hdr.dz_constant = input_field_hdr.dz_constant;
  
  field_hdr.proj_origin_lat = input_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = input_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = input_field_hdr.proj_param[i];
  field_hdr.vert_reference = 0;
  field_hdr.grid_dx = input_field_hdr.grid_dx;
  field_hdr.grid_dy = input_field_hdr.grid_dy;
  field_hdr.grid_dz = input_field_hdr.grid_dz;
  field_hdr.grid_minx = input_field_hdr.grid_minx;
  field_hdr.grid_miny = input_field_hdr.grid_miny;
  field_hdr.grid_minz = input_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = _params->output_field_info.missing_data_value;
  field_hdr.missing_data_value = _params->output_field_info.missing_data_value;
  field_hdr.proj_rotation = input_field_hdr.proj_rotation;
  
  STRcopy(field_hdr.field_name_long,
	  _params->output_field_info.field_name_long, MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _params->output_field_info.field_name,
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, _params->output_field_info.units, MDV_UNITS_LEN);
  field_hdr.transform[0] = '\0';
  
  // Create the average field

  return new MdvxField(field_hdr, input_vlevel_hdr, (void *)0, true);
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool AverageFields::_initTrigger(void)
{
  static const string method_name = "AverageFields::_initTrigger()";
  
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
      cerr << "   url: " << _params->time_list_trigger.url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
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
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::MULTIPLE_URL :
  {
    if (_params->debug)
    {
      cerr << "Initializing MULTIPLE_URL trigger using urls: " << endl;
      for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
	cerr << "    " << _params->_multiple_url_trigger[i] << endl;
    }
    
    DsMultipleTrigger *trigger = new DsMultipleTrigger();

    if (!trigger->initRealtime(-1,
			       PMU_auto_register))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing MULTIPLE_URL trigger using urls: " << endl;
      for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
	cerr << "    " << _params->_multiple_url_trigger[i] << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    for (int i = 0; i < _params->multiple_url_trigger_n; ++i)
      trigger->add(_params->_multiple_url_trigger[i]);
    trigger->set_debug(_params->debug);
    
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

bool AverageFields::_processData(const time_t trigger_time)
{
  static const string method_name = "AverageFields::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: "
	 << DateTime::str(trigger_time) << endl;
  
  // Read in the input fields and give the information to the averager

  bool field_read = false;
  Mdvx::master_header_t master_hdr;
  Mdvx::field_header_t field_hdr;
  Mdvx::vlevel_header_t vlevel_hdr;
  
  vector< MdvxField* > input_field_list;
  
  for (int i = 0; i < _params->input_fields_n; ++i)
  {
    MdvxField *field;
    Mdvx::master_header_t curr_master_hdr;
    
    // Read in the input field

    if ((field = _readField(trigger_time,
			    _params->_input_fields[i].url,
			    _params->_input_fields[i].field_name,
			    _params->_input_fields[i].field_num,
			    curr_master_hdr)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading input field " <<
	_params->_input_fields[i].field_name << " (" <<
	_params->_input_fields[i].field_num << ")" << endl;
      cerr << "Url: " << _params->_input_fields[i].url << endl;
      
      if (_params->calc_with_fields_missing)
	continue;
      else
	return false;
    }
    
    if (_params->debug)
    {
      cerr << "Read input field:" << endl;
      cerr << "        url: " << _params->_input_fields[i].url << endl;
      cerr << "        field time: "
	   << DateTime::str(curr_master_hdr.time_centroid) << endl;
    }
    
    // Check for inconsistencies between the fields

    Mdvx::field_header_t curr_field_hdr = field->getFieldHeader();
    
    if (field_read)
    {
      if (curr_field_hdr.nz != field_hdr.nz)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Number of vertical levels in input fields don't match" << endl;
	cerr << curr_field_hdr.field_name << " has " <<
	  curr_field_hdr.nz << " vertical levels" << endl;
	cerr << field_hdr.field_name << " has " <<
	  field_hdr.nz << " vertical levels" << endl;

	return false;
      }
    }
    else
    {
      master_hdr = curr_master_hdr;
    }
    
    // Add the field to the computation

    input_field_list.push_back(field);
    
    // Set values for next iteration

    field_hdr = curr_field_hdr;
    vlevel_hdr = field->getVlevelHeader();

    field_read = true;
  } /* endfor - i */
  
  if (!field_read)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No input fields were successfully read -- waiting for next trigger" << endl;
    
    return false;
  }
  
  // Create the average field and add it to the output file

  MdvxField *average_field;
  
  if ((average_field =
       _createAverageField(field_hdr, vlevel_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output field" << endl;
    
    return false;
  }
  
  if (!_gridCalc->doCalculation(input_field_list, average_field))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating new grid" << endl;
    
    return false;
  }
  
  // Reclaim memory

  vector< MdvxField* >::iterator field_iter;
  for (field_iter = input_field_list.begin();
       field_iter != input_field_list.end(); ++field_iter)
    delete *field_iter;
  
  // Create the output file

  DsMdvx output_file;
  
  _updateOutputMasterHeader(output_file,
			    master_hdr);
  
  average_field->convertType(Mdvx::ENCODING_INT8,
			     Mdvx::COMPRESSION_GZIP,
			     Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(average_field);
  
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


/*********************************************************************
 * _readField() - Read in the indicated input field
 *
 * Returns a pointer to the input field if successful, 0 otherwise.
 * Also returns the master header for the input file in the master_hdr
 * field.
 */

MdvxField *AverageFields::_readField(const time_t data_time,
				     const string &url,
				     const string &field_name,
				     const int field_num,
				     Mdvx::master_header_t &master_hdr) const
{
  static const string method_name = "AverageFields::_readField()";

  DsMdvx input_file;
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 url,
			 _params->max_input_valid_secs, data_time);
  
  input_file.clearReadFields();
  
  if (_params->use_field_name)
    input_file.addReadField(field_name);
  else
    input_file.addReadField(field_num);
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  switch (_params->remap_info.remap_type)
  {
  case Params::REMAP_LATLON :
    input_file.setReadRemapLatlon(_params->remap_info.nx,
				  _params->remap_info.ny,
				  _params->remap_info.minx,
				  _params->remap_info.miny,
				  _params->remap_info.dx,
				  _params->remap_info.dy);
    break;
    
  case Params::REMAP_FLAT :
    input_file.setReadRemapFlat(_params->remap_info.nx,
				_params->remap_info.ny,
				_params->remap_info.minx,
				_params->remap_info.miny,
				_params->remap_info.dx,
				_params->remap_info.dy,
				_params->remap_info.origin_lat,
				_params->remap_info.origin_lon,
				_params->remap_info.rotation);
    break;
    
  case Params::REMAP_LAMBERT_CONFORMAL2 :
    input_file.setReadRemapLc2(_params->remap_info.nx,
			       _params->remap_info.ny,
			       _params->remap_info.minx,
			       _params->remap_info.miny,
			       _params->remap_info.dx,
			       _params->remap_info.dy,
			       _params->remap_info.origin_lat,
			       _params->remap_info.origin_lon,
			       _params->remap_info.lat1,
			       _params->remap_info.lat2);
    break;
  } /* endswitch - _params->remap_info.remap_type */
  

  if (_params->debug)
  {
    cerr << endl;
    input_file.printReadRequest(cerr);
  }
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV volume for time: " <<
      DateTime::str(data_time) << endl;
    
    return 0;
  }
  
  // Create the returned information

  master_hdr = input_file.getMasterHeader();
  return new MdvxField(*input_file.getField(0));
}


/*********************************************************************
 * _updateOutputMasterHeader() - Update the master header values for
 *                               the output file.
 */

void AverageFields::_updateOutputMasterHeader(DsMdvx &output_file,
					     const Mdvx::master_header_t &input_master_hdr)
{
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = input_master_hdr.time_begin;
  master_hdr.time_end = input_master_hdr.time_end;
  master_hdr.time_centroid = input_master_hdr.time_centroid;
  master_hdr.time_expire = input_master_hdr.time_expire;
  master_hdr.data_dimension = input_master_hdr.data_dimension;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = input_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = input_master_hdr.vlevel_type;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = input_master_hdr.grid_orientation;
  master_hdr.data_ordering = input_master_hdr.data_ordering;
  
  master_hdr.sensor_lon = input_master_hdr.sensor_lon;
  master_hdr.sensor_lat = input_master_hdr.sensor_lat;
  master_hdr.sensor_alt = input_master_hdr.sensor_alt;
  
  STRcopy(master_hdr.data_set_info, "AverageFields output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "AverageFields", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "AverageFields", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
