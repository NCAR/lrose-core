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
//   $Date: 2016/03/04 02:22:12 $
//   $Id: MdvThreat.cc,v 1.9 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvThreat: MdvThreat program object.
 *
 * RAL, NCAR, Boulder CO
 *
 * August 2008
 *
 * Jeff Copeland
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvThreat.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvThreat *MdvThreat::_instance =
     (MdvThreat *)NULL;


const string MdvThreat::DATA_TIME_FIELD_NAME = "threat";
const ui32 MdvThreat::DATA_TIME_MISSING_VALUE = 0;


/*********************************************************************
 * Constructor
 */

MdvThreat::MdvThreat(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvThreat::MdvThreat()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvThreat *)NULL);

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

MdvThreat::~MdvThreat()
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

MdvThreat *MdvThreat::Inst(int argc, char **argv)
{
  if (_instance == (MdvThreat *)NULL)
    new MdvThreat(argc, argv);

  return(_instance);
}

MdvThreat *MdvThreat::Inst()
{
  assert(_instance != (MdvThreat *)NULL);

  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvThreat::init()
{
  static const string method_name = "MdvThreat::init()";

  // Initialize the data trigger

  if (!_initTrigger())
    return false;

  // Initialize the input projection

  if (!_initInputProj())
    return false;

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MdvThreat::run()
{
  static const string method_name = "MdvThreat::run()";

  DateTime trigger_time;
  DsMdvx threat_file;
  bool createThreatFile = true;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;

      continue;
    }

    if (!_processThreat(trigger_time, threat_file, &createThreatFile))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing threat for time: " << trigger_time << endl;

      continue;
    }

  } /* endwhile - !_dataTrigger->endOfData() */

  // Smooth the threat zone
  _smoothField(threat_file, 2);

  // Write the threat file

  threat_file.setWriteLdataInfo();

  if (threat_file.writeToDir(_params->threat_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing accumulation file to URL: "
	 << _params->threat_url << endl;
    cerr << threat_file.getErrStr() << endl;
  }

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createThreatTimeField() - Create the initial threat data time
 *                           field.  This field will have the given time
 *                           where the given field has a data value, and
 *                           will have a missing data value where the
 *                           given field data is missing.
 *
 * Returns a pointer to the created field on success, 0 on failure.
 */

MdvxField *MdvThreat::_createThreatTimeField(const MdvxField &input_field, float forecast_delta) const
{
  static const string method_name = "MdvThreat::_createThreatTimeField()";

  // Create the field header.  It will be a copy of the input field header
  // with a few fields changed.  We know that the input field uses
  // ENCODE_FLOAT32, which is what we also want for the time field since
  // MDV doesn't have an ENCODE_INT32 type.

  Mdvx::field_header_t time_field_hdr;
  memset(&time_field_hdr, 0, sizeof(time_field_hdr));
  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();

  time_field_hdr = input_field_hdr;

  time_field_hdr.bad_data_value = DATA_TIME_MISSING_VALUE;
  time_field_hdr.missing_data_value = DATA_TIME_MISSING_VALUE;
  time_field_hdr.min_value = 0;
  time_field_hdr.max_value = 0;
  time_field_hdr.min_value_orig_vol = 0;
  time_field_hdr.max_value_orig_vol = 0;
  STRcopy(time_field_hdr.field_name_long, DATA_TIME_FIELD_NAME.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(time_field_hdr.field_name, DATA_TIME_FIELD_NAME.c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(time_field_hdr.units, "min", MDV_UNITS_LEN);
  time_field_hdr.transform[0] = '\0';

  // Create the data time field.  It will have the same vlevel header as
  // the input field.  Initialize it with missing data values so we can
  // overwrite the data values where necessary below.

  MdvxField *time_field = new MdvxField(time_field_hdr,
					input_field.getVlevelHeader(),
					(void *)0, true);

  if (time_field == 0)
    return 0;

  // Fill in the time values

  int volume_size = time_field_hdr.nx * time_field_hdr.ny * time_field_hdr.nz;

  fl32 *input_data = (fl32 *)input_field.getVol();

  cerr << "Forecast period create: " << forecast_delta << endl;
  fl32 *time_data = (fl32 *)time_field->getVol();
  for (int i = 0; i < volume_size; ++i)
  {
    if (input_data[i] == input_field_hdr.missing_data_value ||
	input_data[i] == input_field_hdr.bad_data_value)
    {
    	time_data[i] = DATA_TIME_MISSING_VALUE;
    }
    else
    {
      time_data[i] = forecast_delta;
    }
  } /* endfor - i */

  return time_field;
}


/*********************************************************************
 * _createThreat() - Create a new threat file file based on the
 *                         given input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_createThreat(const Mdvx &input_file,
					Mdvx &threat_file) const
{
  static const string method_name = "MdvThreat::_createThreat()";

  // Clear out any existing data in the threat file.
  // There shouldn't be any, but might as well be careful.

  threat_file.clearFields();
  threat_file.clearChunks();

  // Update the master header

  Mdvx::master_header_t input_master_hdr = input_file.getMasterHeader();
  float forecast_delta = (input_master_hdr.time_centroid - input_master_hdr.time_gen)/60.0;
  cerr << "centroid " << input_master_hdr.time_centroid << endl;
  cerr << "start " << input_master_hdr.time_gen << endl;

  _createThreatMasterHeader(threat_file, input_master_hdr,
			   input_file.getPathInUse());

  // Create a new threat time field and add it to the threat file.

  MdvxField *threat_time_field =
    _createThreatTimeField(*(input_file.getField(0)), forecast_delta);

  threat_file.addField(threat_time_field);

  return true;
}


/*********************************************************************
 * __createThreatMasterHeader() - Create the threat file master
 *                               header based on the information in
 *                               the input master header.
 */

void MdvThreat::_createThreatMasterHeader(Mdvx &threat_file,
					     const Mdvx::master_header_t &master_hdr,
					     const string &input_file_path) const
{
  Mdvx::master_header_t threat_master_hdr = threat_file.getMasterHeader();

  threat_master_hdr.time_gen = master_hdr.time_gen;
  threat_master_hdr.time_begin = master_hdr.time_centroid;
  threat_master_hdr.time_end = master_hdr.time_centroid;
  threat_master_hdr.time_centroid = master_hdr.time_gen;
  threat_master_hdr.time_expire = master_hdr.time_expire;
  threat_master_hdr.data_dimension = master_hdr.data_dimension;
  threat_master_hdr.data_collection_type = master_hdr.data_collection_type;
  threat_master_hdr.native_vlevel_type = master_hdr.native_vlevel_type;
  threat_master_hdr.vlevel_type = master_hdr.vlevel_type;
  threat_master_hdr.vlevel_included = master_hdr.vlevel_included;
  threat_master_hdr.grid_orientation = master_hdr.grid_orientation;
  threat_master_hdr.data_ordering = master_hdr.data_ordering;
  threat_master_hdr.sensor_lon = master_hdr.sensor_lon;
  threat_master_hdr.sensor_lat = master_hdr.sensor_lat;
  threat_master_hdr.sensor_alt = master_hdr.sensor_alt;
  STRcopy(threat_master_hdr.data_set_info,
	  "Output from MdvThreat", MDV_INFO_LEN);
  STRcopy(threat_master_hdr.data_set_name, "MdvThreat", MDV_NAME_LEN);
  STRcopy(threat_master_hdr.data_set_source, input_file_path.c_str(),
	  MDV_NAME_LEN);

  threat_file.setMasterHeader(threat_master_hdr);
}

/*********************************************************************
 * _initInputProj() - Initialize the input projection.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_initInputProj(void)
{
  static const string method_name = "MdvThreat::_initInputProj()";

  switch (_params->remap_info.remap_type)
  {
  case Params::REMAP_LATLON :
    _inputProj.initLatlon(_params->remap_info.nx,
			  _params->remap_info.ny,
			  1,
			  _params->remap_info.dx,
			  _params->remap_info.dy,
			  1.0,
			  _params->remap_info.minx,
			  _params->remap_info.miny,
			  0.0);
    break;

  case Params::REMAP_FLAT :
    _inputProj.initFlat(_params->remap_info.origin_lat,
			_params->remap_info.origin_lon,
			_params->remap_info.rotation,
			_params->remap_info.nx,
			_params->remap_info.ny,
			1,
			_params->remap_info.dx,
			_params->remap_info.dy,
			1.0,
			_params->remap_info.minx,
			_params->remap_info.miny,
			0.0);
    break;

  case Params::REMAP_LAMBERT_CONFORMAL2 :
    _inputProj.initLc2(_params->remap_info.origin_lat,
		       _params->remap_info.origin_lon,
		       _params->remap_info.lat1,
		       _params->remap_info.lat2,
		       _params->remap_info.nx,
		       _params->remap_info.ny,
		       1,
		       _params->remap_info.dx,
		       _params->remap_info.dy,
		       1.0,
		       _params->remap_info.minx,
		       _params->remap_info.miny,
		       0.0);
    break;

  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_initTrigger(void)
{
  static const string method_name = "MdvThreat::_initTrigger()";

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
      cerr << "   url: " << _params->input_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
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
 * _processThreat() - Process threat for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_processThreat(const DateTime &trigger_time, DsMdvx &threat_file, bool *createThreatFile)
{
  static const string method_name = "MdvThreat::_processData()";

  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;

  // Read the new input data

  DsMdvx input_file;

  if (!_readInputFile(input_file, trigger_time))
    return false;

  // We only need to create the threat file for the first successful input data read
  // otherwise we just update it

  if (*createThreatFile)
  {
    if (_params->debug)
      cerr << "Creating threat file" << endl;

    if (!_createThreat(input_file, threat_file))
      return false;
  }
  else
  {
    if (_params->debug)
      cerr << "Updating threat" << endl;

    if (!_updateThreat(input_file, threat_file))
      return false;
  }

  *createThreatFile = false;
  return true;
}


/*********************************************************************
 * _readInputFile() - Read the indicated input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_readInputFile(Mdvx &input_file,
				   const DateTime &trigger_time) const
{
  static const string method_name = "MdvThreat::_readInputFile()";

  // Set up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->input_url,
			 0, trigger_time.utime());

  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);

  input_file.setReadFieldFileHeaders();

  if (_params->remap_data)
    input_file.setReadRemap(_inputProj);

  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;

    return false;
  }

  // Set the input projection based on the projection read.  We are assuming
  // that all of the input fields use the same projection.

  if (!_params->remap_data)
    _inputProj.init(input_file);

  return true;
}


/*********************************************************************
 * _updateThreatTimeField() - Update the threat time field with
 *                           the new input forecast time.  The time values will
 *                           only be updated where the input field has
 *                           valid data and the threat field is missing.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_updateThreatTimeField(MdvxField &threat_field, const MdvxField &input_field, float forecast_delta) const
{
  static const string method_name = "MdvThreat::_updateAccumTimeField()";

  // Fill in the time values

  Mdvx::field_header_t threat_field_hdr = threat_field.getFieldHeader();
  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();

  int volume_size = threat_field_hdr.nx * threat_field_hdr.ny * threat_field_hdr.nz;

  fl32 *input_data = (fl32 *)input_field.getVol();
  fl32 *threat_data = (fl32 *)threat_field.getVol();

  cerr << "Forecast period update: " << forecast_delta << endl;

  for (int i = 0; i < volume_size; ++i)
  {
	  if (input_data[i] == input_field_hdr.missing_data_value || input_data[i] == input_field_hdr.bad_data_value)
		  continue;
	  if (threat_data[i] == threat_field_hdr.missing_data_value || threat_data[i] == threat_field_hdr.bad_data_value)
		  threat_data[i] = forecast_delta;
  }

  return true;
}

/*********************************************************************
 * _updateThreat() - Update the given threat file based on the
 *                         given input file.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_updateThreat(const Mdvx &input_file,
					Mdvx &threat_file) const
{
  static const string method_name = "MdvThreat::_updateAccumulation()";

  // Update the previous accumulation field with the background data
  // so it is ready for adding the new AIRS data.

  // Update the master header
  Mdvx::master_header_t input_master_hdr = input_file.getMasterHeader();
  float forecast_delta = (input_master_hdr.time_centroid - input_master_hdr.time_gen)/60.0;
  cerr << "centroid " << utimstr(input_master_hdr.time_centroid) << endl;
  cerr << "begin " << input_master_hdr.time_gen << endl;
  _updateThreatMasterHeader(threat_file, input_master_hdr,
			   input_file.getPathInUse());

  // Time out any old data in the old accumulation fields.  We do this
  // before updating the data since it's easier even though it requires
  // multiple passes through the data.

  MdvxField *time_field;
  if ((time_field = threat_file.getField(DATA_TIME_FIELD_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving time field (" << DATA_TIME_FIELD_NAME
	 << ") from threat file" << endl;
    cerr << "Cannot continue" << endl;

    return false;
  }

  // Now update the accumulation data time file

  if (!_updateThreatTimeField(*time_field,*(input_file.getField(0)), forecast_delta))
    return false;

  return true;
}


/*********************************************************************
 * __updateThreatMasterHeader() - Update the threat file master
 *                               header based on the information in
 *                               the given master header.
 */

void MdvThreat::_updateThreatMasterHeader(Mdvx &threat_file,
					     const Mdvx::master_header_t &master_hdr,
					     const string &input_file_path) const
{
  Mdvx::master_header_t threat_master_hdr = threat_file.getMasterHeader();

  //threat_master_hdr.time_gen = master_hdr.time_gen;
  //threat_master_hdr.time_begin = master_hdr.time_centroid;
  threat_master_hdr.time_end = master_hdr.time_centroid;
  //threat_master_hdr.time_centroid = master_hdr.time_centroid;
  threat_master_hdr.time_expire = master_hdr.time_expire;
  threat_master_hdr.data_dimension = master_hdr.data_dimension;
  threat_master_hdr.data_collection_type = master_hdr.data_collection_type;
  threat_master_hdr.native_vlevel_type = master_hdr.native_vlevel_type;
  threat_master_hdr.vlevel_type = master_hdr.vlevel_type;
  threat_master_hdr.vlevel_included = master_hdr.vlevel_included;
  threat_master_hdr.grid_orientation = master_hdr.grid_orientation;
  threat_master_hdr.data_ordering = master_hdr.data_ordering;
  threat_master_hdr.sensor_lon = master_hdr.sensor_lon;
  threat_master_hdr.sensor_lat = master_hdr.sensor_lat;
  threat_master_hdr.sensor_alt = master_hdr.sensor_alt;
  STRcopy(threat_master_hdr.data_set_info,
	  "Output from MdvThreat", MDV_INFO_LEN);
  STRcopy(threat_master_hdr.data_set_name, "MdvThreat", MDV_NAME_LEN);
  STRcopy(threat_master_hdr.data_set_source, input_file_path.c_str(),
	  MDV_NAME_LEN);

  threat_file.setMasterHeader(threat_master_hdr);
}

/*********************************************************************
 * _smoothField() - Smooth the threat time field.
 *                  Replace data points with simple areal average.
 *                  Radius of filter is a configurable parameter.
 *
 * Returns true on success, false on failure.
 */

bool MdvThreat::_smoothField(Mdvx &threat_file, int filter_radius) const
{
  static const string method_name = "MdvThreat::_smoothField()";

  MdvxField *threat_field;
  if ((threat_field = threat_file.getField(DATA_TIME_FIELD_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving time field (" << DATA_TIME_FIELD_NAME
	 << ") from threat file" << endl;
    cerr << "Cannot continue" << endl;

    return false;
  }



  Mdvx::field_header_t threat_field_hdr = threat_field->getFieldHeader();

  // get data to smooth
  int volume_size = threat_field_hdr.nx * threat_field_hdr.ny * threat_field_hdr.nz;
  fl32 *threat_data = (fl32 *)threat_field->getVol();

  //
  // Allocate a buffer that we will use to hold the values
  // that we are smoothing.
  //
  fl32 *Buffer = (fl32 *) malloc(sizeof(float) * volume_size);
  if (Buffer == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  //kind of nasty, should revisit for a more elegant solution
  for (int ix=0;ix<threat_field_hdr.nx;ix++) {
	  for (int iy=0;iy<threat_field_hdr.ny;iy++) {
	     int num = 0;
	     fl32 sum = 0.0;
	     for (int ixx=-filter_radius;ixx<=filter_radius;ixx++) {
	    	 for (int iyy=-filter_radius;iyy<-filter_radius;iyy++) {
	    		 int ixxx = ix + ixx;
	    		 int iyyy = iy + iyy;
	    		 if (ixxx > -1 && iyyy > -1 && ixxx < threat_field_hdr.nx && iyyy < threat_field_hdr.ny) {
	    			 fl32 data_value = threat_data[ixxx + iyyy*threat_field_hdr.nx];
	    			 if ((data_value != threat_field_hdr.missing_data_value) && (data_value != threat_field_hdr.bad_data_value)) {
	    				 //finally we have good data within the domain!
	    				 sum += data_value;
	    				 num++;
	    			 }
	    		 }
	    	 }
	     }
	     if (num > 0) {
	    	 Buffer[ix + iy*threat_field_hdr.nx] = sum/num;
	     } else {
	    	 Buffer[ix + iy*threat_field_hdr.nx] = threat_data[ix + iy*threat_field_hdr.nx];
	     }
	  }
  }

  // load smoothed data back into field volume
  for (int i = 0; i < volume_size; ++i)
  {
	  threat_data[i] = Buffer[i];
  }

  free(Buffer);
  return true;
}
