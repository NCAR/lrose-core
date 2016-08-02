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
//   $Id: TrendField.cc,v 1.5 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TrendField: TrendField program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2002
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

#include "TrendField.hh"
#include "Params.hh"

#include "FirstOrderTrender.hh"



// Global variables

TrendField *TrendField::_instance =
     (TrendField *)NULL;


/*********************************************************************
 * Constructor
 */

TrendField::TrendField(int argc, char **argv) :
  _dataTrigger(0),
  _fieldTrender(0)
{
  static const string method_name = "TrendField::TrendField()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TrendField *)NULL);
  
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

TrendField::~TrendField()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _fieldTrender;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TrendField *TrendField::Inst(int argc, char **argv)
{
  if (_instance == (TrendField *)NULL)
    new TrendField(argc, argv);
  
  return(_instance);
}

TrendField *TrendField::Inst()
{
  assert(_instance != (TrendField *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TrendField::init()
{
  static const string method_name = "TrendField::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;

  // Initialize the field trender

  if (!_initTrender())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void TrendField::run()
{
  static const string method_name = "TrendField::run()";
  
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
 * _initTrender() - Initialize the field trender.
 *
 * Returns true on success, false on failure.
 */

bool TrendField::_initTrender(void)
{
  static const string method_name = "TrendField::_initTrender()";
  
  switch (_params->trending_type)
  {
  case Params::FIRST_ORDER_TRENDING :
  {
    if (_params->debug)
      cerr << "Initializing FIRST_ORDER_TRENDING trender." << endl;
    
    _fieldTrender = new FirstOrderTrender();
    
    break;
  }
  
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing field trender." << endl;
    cerr << "Invalid trending type in parameter file" << endl;
    
    return false;
    
  } /* endswitch - _params->trending_type */

  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool TrendField::_initTrigger(void)
{
  static const string method_name = "TrendField::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->input_field.url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_field.url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->input_field.url << endl;
      
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
    if (trigger->init(_params->input_field.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_field.url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing data trigger." << endl;
    cerr << "Invalid trigger type in parameter file" << endl;
    
    return false;
    
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool TrendField::_processData(const time_t trigger_time)
{
  static const string method_name = "TrendField::_processData()";
  
  // Read in the latest input field

  MdvxField *field;
  Mdvx::master_header_t curr_master_hdr;
    
  // Read in the input field

  if ((field = _readField(trigger_time,
			  _params->input_field.url,
			  _params->input_field.field_name,
			  _params->input_field.field_num,
			  curr_master_hdr)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input field " <<
      _params->input_field.field_name << " (" <<
      _params->input_field.field_num << ")" << endl;
    cerr << "Url: " << _params->input_field.url << endl;
    
    return false;
  }
    
  // Give the field to the trender

  _fieldTrender->updateField(*field);
  delete field;
  
  // Create the trended field

  MdvxField *trended_field;
  
  if ((trended_field = _fieldTrender->createTrendedField()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating trended field" << endl;
    cerr << "*** Waiting for next data file ***" << endl;
    
    return false;
  }
  
  // Create the output file

  DsMdvx output_file;
  
  _updateOutputMasterHeader(output_file,
			    curr_master_hdr);
  
  // Add the trended field to the output file

  trended_field->convertType(Mdvx::ENCODING_INT8,
			     Mdvx::COMPRESSION_GZIP,
			     Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(trended_field);
  
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

MdvxField *TrendField::_readField(const time_t data_time,
				     const string &url,
				     const string &field_name,
				     const int field_num,
				     Mdvx::master_header_t &master_hdr) const
{
  static const string method_name = "TrendField::_readField()";

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

void TrendField::_updateOutputMasterHeader(DsMdvx &output_file,
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
  
  STRcopy(master_hdr.data_set_info, "TrendField output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "TrendField", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "TrendField", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
