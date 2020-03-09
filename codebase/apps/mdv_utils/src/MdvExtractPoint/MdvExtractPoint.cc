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
 * @file MdvExtractPoint.cc
 *
 * @class MdvExtractPoint
 *
 * MdvExtractPoint program object.
 *  
 * @date 8/4/2009
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxProj.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvExtractPoint.hh"
#include "Params.hh"

#include "Ascii1Output.hh"

using namespace std;

// Global variables

MdvExtractPoint *MdvExtractPoint::_instance =
     (MdvExtractPoint *)NULL;


/*********************************************************************
 * Constructors
 */

MdvExtractPoint::MdvExtractPoint(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvExtractPoint::MdvExtractPoint()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvExtractPoint *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // display ucopyright message.

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

MdvExtractPoint::~MdvExtractPoint()
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
 * Inst()
 */

MdvExtractPoint *MdvExtractPoint::Inst(int argc, char **argv)
{
  if (_instance == (MdvExtractPoint *)NULL)
    new MdvExtractPoint(argc, argv);
  
  return(_instance);
}

MdvExtractPoint *MdvExtractPoint::Inst()
{
  assert(_instance != (MdvExtractPoint *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool MdvExtractPoint::init()
{
  static const string method_name = "MdvExtractPoint::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
 
  // Initialize the output handler
  
  if (!_initOutputHandler())
    return false;
  
  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void MdvExtractPoint::run()
{
  static const string method_name = "MdvExtractPoint::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger information" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }
    
    _processData(trigger_info.getIssueTime());

  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initOutputHandler()
 */

bool MdvExtractPoint::_initOutputHandler()
{
  static const string method_name = "MdvExtractPoint;:_initOutputHandler()";
  
  switch (_params->output_type)
  {
  case Params::OUTPUT_ASCII1 :
    _outputHandler = new Ascii1Output(_params->ascii1_missing_string,
				      _params->debug, _params->verbose);
    break;

  } /* endswitch - _params->output_type */
  
  return true;
}

    
/*********************************************************************
 * _initTrigger()
 */

bool MdvExtractPoint::_initTrigger()
{
  static const string method_name = "MdvExtractPoint;:_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_field_info.url,
		      _params->input_field_info.max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      cerr << "   URL: " << _params->input_field_info.url << endl;
      cerr << "   max valid secs: "
	   << _params->input_field_info.max_valid_secs << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    DateTime start_time = _args->getStartTime();
    DateTime end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
	end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "In TIME_LIST mode, start and end times must be included on the command line." << endl;
      cerr << "Update command line and try again." << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_field_info.url,
		      start_time.utime(), end_time.utime()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger" << endl;
      cerr << "   URL: " << _params->input_field_info.url << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
      
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

bool MdvExtractPoint::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvExtractPoint::_processData()";
  
  PMU_auto_register("Processing data...");

  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the MDV data

  MdvxField *input_field;
  
  if ((input_field = _readInputField(trigger_time)) == 0)
    return false;
  
  // Process the points in the field

  _outputHandler->init(trigger_time);
  
  if (!_processPoints(*input_field))
    return false;
  
  _outputHandler->close();
  
  // Reclaim memory

  delete input_field;
  
  return true;
}


/*********************************************************************
 * _processPoints()
 */

bool MdvExtractPoint::_processPoints(const MdvxField &field) const
{
  static const string method_name = "MdvExtractPoint::_processPoints()";
  
  // Retrieve the field information

  Mdvx::field_header_t field_hdr = field.getFieldHeader();
  Mdvx::vlevel_header_t vlevel_hdr = field.getVlevelHeader();
  
  MdvxProj proj(field_hdr);
  fl32 *data = (fl32 *)field.getVol();
  
  double grid_size = proj.xGrid2km(1) * proj.yGrid2km(1);
  
  // Process each of the points in the list

  for (int pt = 0; pt < _params->points_n; ++pt)
  {
    // Get the position in the grid

    double lat = _params->_points[pt].lat;
    double lon = _params->_points[pt].lon;
    
    int x_index, y_index;
    
    proj.latlon2xyIndex(lat, lon, x_index, y_index);
    
    // Loop through the vertical levels.  We will add a point to the output
    // at each level.

    for (int z_index = 0; z_index < field_hdr.nz; ++z_index)
    {
      // Set all of the data values

      double value = _setValue(data, field_hdr.missing_data_value,
			       field_hdr.bad_data_value,
			       x_index, y_index, z_index,
			       field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_nw = _setValue(data, field_hdr.missing_data_value,
				  field_hdr.bad_data_value,
				  x_index - 1, y_index + 1, z_index,
				  field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_n = _setValue(data, field_hdr.missing_data_value,
				 field_hdr.bad_data_value,
				 x_index, y_index + 1, z_index,
				 field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_ne = _setValue(data, field_hdr.missing_data_value,
				  field_hdr.bad_data_value,
				  x_index + 1, y_index + 1, z_index,
				  field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_e = _setValue(data, field_hdr.missing_data_value,
				 field_hdr.bad_data_value,
				 x_index + 1, y_index, z_index,
				 field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_se = _setValue(data, field_hdr.missing_data_value,
				  field_hdr.bad_data_value,
				  x_index + 1, y_index - 1, z_index,
				  field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_s = _setValue(data, field_hdr.missing_data_value,
				 field_hdr.bad_data_value,
				 x_index, y_index - 1, z_index,
				 field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_sw = _setValue(data, field_hdr.missing_data_value,
				  field_hdr.bad_data_value,
				  x_index - 1, y_index - 1, z_index,
				  field_hdr.nx, field_hdr.ny, field_hdr.nz);
      double value_w = _setValue(data, field_hdr.missing_data_value,
				 field_hdr.bad_data_value,
				 x_index - 1, y_index, z_index,
				 field_hdr.nx, field_hdr.ny, field_hdr.nz);
      
      _outputHandler->addPoint(pt, lat, lon, vlevel_hdr.level[z_index],
			       grid_size, value, value_nw, value_n,
			       value_ne, value_e, value_se, value_s,
			       value_sw, value_w);
      
    } /* endfor - z_index */
    
  } /* endfor - pt */
  
  return true;
}


/*********************************************************************
 * _readInputField()
 */

MdvxField *MdvExtractPoint::_readInputField(const DateTime &data_time) const
{
  static const string method_name = "MdvExtractPoint::_readInputField()";
  
  DsMdvx input_file;
  
  // Set up read request

  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 _params->input_field_info.url,
			 600, data_time.utime());
  
  string field_name = _params->input_field_info.field_name;
  
  if (field_name == "")
  {
    input_file.addReadField(_params->input_field_info.field_num);
  }
  else
  {
    input_file.addReadField(field_name);
  }
  
  if (_params->input_field_info.vlevel_num == -1)
  {
    input_file.setReadComposite();
  }
  else if (_params->input_field_info.vlevel_num >= 0)
  {
    input_file.setReadPlaneNumLimits(_params->input_field_info.vlevel_num,
				     _params->input_field_info.vlevel_num);
  }
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->verbose)
    input_file.printReadRequest(cerr);
  
  // Read the data

  int status;
  
  if ((status = input_file.readVolume()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV file" << endl;
    cerr << "status = " << status << endl;
    cerr << input_file.getErrStr() << endl;
    
    return 0;
  }
  
  // Extract the desired field

  MdvxField *return_field = input_file.getField(0);
  
  if (return_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No fields in read file" << endl;
    
    return 0;
  }
  
  return new MdvxField(*return_field);
}

