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
//   $Date: 2016/03/07 18:17:27 $
//   $Id: RefrMerge.cc,v 1.6 2016/03/07 18:17:27 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RefrMerge: RefrMerge program object.
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
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsMultipleTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "RefrMerge.hh"
#include "Params.hh"

using namespace std;

// Global variables

RefrMerge *RefrMerge::_instance =
     (RefrMerge *)NULL;


/*********************************************************************
 * Constructor
 */

RefrMerge::RefrMerge(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "RefrMerge::RefrMerge()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (RefrMerge *)NULL);
  
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

RefrMerge::~RefrMerge()
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

RefrMerge *RefrMerge::Inst(int argc, char **argv)
{
  if (_instance == (RefrMerge *)NULL)
    new RefrMerge(argc, argv);
  
  return(_instance);
}

RefrMerge *RefrMerge::Inst()
{
  assert(_instance != (RefrMerge *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool RefrMerge::init()
{
  static const string method_name = "RefrMerge::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the projection

  if (!_initProjection())
    return false;
  
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

void RefrMerge::run()
{
  static const string method_name = "RefrMerge::run()";
  
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
 * _addMergeField() - Add the given field information to the merge field.
 */

void RefrMerge::_addMergeField(fl32 *mean_n, fl32 *weight,
			       const MdvxField &n_field,
			       const MdvxField &sigma_n_field) const
{
  Mdvx::field_header_t n_field_hdr = n_field.getFieldHeader();
  Mdvx::field_header_t sigma_n_field_hdr = sigma_n_field.getFieldHeader();
  
  int plane_size = n_field_hdr.nx * n_field_hdr.ny;
  
  fl32 *n_data = (fl32 *)n_field.getVol();
  fl32 *sigma_n_data = (fl32 *)sigma_n_field.getVol();
  
  for (int i = 0; i < plane_size; ++i)
  {
    // Must have both N and Sigma N data in order to process.
    // There is a problem with the missing data value in the files,
    // so also don't process large negative values.

    if (n_data[i] == n_field_hdr.bad_data_value ||
	n_data[i] == n_field_hdr.missing_data_value ||
	n_data[i] < -1000.0 ||
	sigma_n_data[i] == sigma_n_field_hdr.bad_data_value ||
	sigma_n_data[i] == sigma_n_field_hdr.missing_data_value ||
	sigma_n_data[i] < -1000.0)
      continue;
    
    weight[i] += 1.0 / sigma_n_data[i];
    mean_n[i] += n_data[i] / sigma_n_data[i];
    
  } /* endfor - i */
  
}


/*********************************************************************
 * _createMergeField() - Create the MdvxField object with the given merge
 *                       data.
 *
 * Returns a pointer to the new field on success, 0 on failure.
 */

MdvxField *RefrMerge::_createMergeField(const fl32 *mean_n_data,
					const DateTime &data_time) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.forecast_time = data_time.utime();
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = -32768.0;
  field_hdr.missing_data_value = -32768.0;
  
  STRcopy(field_hdr.field_name_long, "Index of Refraction",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "N", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "none", MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, "", MDV_TRANSFORM_LEN);
  
  _projection.syncToFieldHdr(field_hdr);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.0;
  
  // Create the field object

  return new MdvxField(field_hdr, vlevel_hdr, (void *)mean_n_data);
}


/*********************************************************************
 * _initProjection() - Initialize the projection.
 *
 * Returns true on success, false on failure.
 */

bool RefrMerge::_initProjection(void)
{
  switch (_params->remap_info.remap_type)
  {
  case Params::REMAP_LATLON :
    _projection.initLatlon(_params->remap_info.nx, _params->remap_info.ny, 1,
			   _params->remap_info.dx, _params->remap_info.dy, 1.0,
			   _params->remap_info.minx, _params->remap_info.miny, 0.0);
    return true;
    
  case Params::REMAP_FLAT :
    _projection.initFlat(_params->remap_info.origin_lat,
			 _params->remap_info.origin_lon,
			 _params->remap_info.rotation,
			 _params->remap_info.nx, _params->remap_info.ny, 1,
			 _params->remap_info.dx, _params->remap_info.dy, 1.0,
			 _params->remap_info.minx, _params->remap_info.miny,0.0);
    return true;
    
  case Params::REMAP_LAMBERT_CONFORMAL2 :
    _projection.initLc2(_params->remap_info.origin_lat,
			_params->remap_info.origin_lon,
			_params->remap_info.lat1, _params->remap_info.lat2,
			_params->remap_info.nx,	_params->remap_info.ny, 1,
			_params->remap_info.dx,	_params->remap_info.dy, 1.0,
			_params->remap_info.minx, _params->remap_info.miny, 0.0);
    return true;
  } /* endswitch - _params->remap_info.remap_type */

  // Should never get here

  return false;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool RefrMerge::_initTrigger(void)
{
  static const string method_name = "RefrMerge::_initTrigger()";
  
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
  
  case Params::INTERVAL_REALTIME :
  {
    if (_params->debug)
    {
      cerr << "Initializing INTERVAL_REALTIME trigger: " << endl;
      cerr << "   interval secs: " << _params->interval_realtime << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_realtime,
		      0, 1, PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL_REALTIME trigger" << endl;
      cerr << "    Interval secs: " << _params->interval_realtime << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INTERVAL_ARCHIVE :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->interval_archive.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->interval_archive.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for INTERVAL_ARCHIVE trigger: " <<
	_params->interval_archive.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for INTERVAL_ARCHIVE trigger: " <<
	_params->interval_archive.end_time << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing INTERVAL_ARCHIVE trigger: " << endl;
      cerr << "   interval secs: " << _params->interval_archive.interval_secs << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_archive.interval_secs,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL_ARCHIVE trigger: " << endl;
      cerr << "    Interval secs: " << _params->interval_archive.interval_secs << endl;
      cerr << "    Start time: " << _params->interval_archive.start_time <<
	endl;
      cerr << "    End time: " << _params->interval_archive.end_time << endl;
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

bool RefrMerge::_processData(const DateTime &trigger_time)
{
  static const string method_name = "RefrMerge::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the fields and merge them

  int volume_size = _params->remap_info.nx * _params->remap_info.ny;
  fl32 *mean_n = new fl32[volume_size];
  memset(mean_n, 0, volume_size * sizeof(fl32));
  fl32 *weight = new fl32[volume_size];
  memset(weight, 0, volume_size * sizeof(fl32));
  
  bool field_read = false;
  
  for (int i = 0; i < _params->merge_fields_n; ++i)
  {
    Mdvx *merge_field;
    
    // Read in the input field

    if ((merge_field =
	 _readFields(trigger_time,
		     _params->_merge_fields[i].url,
		     _params->_merge_fields[i].n_field_name,
		     _params->_merge_fields[i].sigma_n_field_name)) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading merge fields:" << endl;
      cerr << "   Url: " << _params->_merge_fields[i].url << endl;
      cerr << "   N field: " << _params->_merge_fields[i].n_field_name << endl;
      cerr << "   Sigma N field: " << _params->_merge_fields[i].sigma_n_field_name << endl;

      continue;
    }
    
    if (_params->debug)
    {
      cerr << "Read merge field:" << endl;
      cerr << "        url: " << _params->_merge_fields[i].url << endl;
      cerr << "        field time: "
	   << DateTime::str(merge_field->getMasterHeader().time_centroid) << endl;
    }
    
    // Add the new information to the merged field

    _addMergeField(mean_n, weight,
		   *(merge_field->getField(0)),
		   *(merge_field->getField(1)));
    
    // Reclaim memory

    delete merge_field;
    
    field_read = true;
  } /* endfor - i */
  
  if (!field_read)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "No input fields were successfully read -- waiting for next trigger" << endl;
    
    return false;
  }
  
  // Calculate the final merged values

  for (int i = 0; i < volume_size; ++i)
  {
    if (weight[i] != 0.0)
      mean_n[i] /= weight[i];
  } /* endfor - i */
  
  MdvxField *output_field = _createMergeField(mean_n, trigger_time);
  if (output_field == 0)
    return false;
  
  // Create the output file

  DsMdvx output_file;
  
  _updateOutputMasterHeader(output_file, trigger_time);
  
  output_field->convertType(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_RLE,
			    Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(output_field);
  
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
 * _readFields() - Read in the indicated merge field
 *
 * Returns a pointer to the merge field information if successful, 0 otherwise.
 * The N field will be field 0; the Sigma N field will be field 1.
 */

Mdvx *RefrMerge::_readFields(const DateTime &data_time,
			     const string &url,
			     const string &n_field_name,
			     const string &sigma_n_field_name) const
{
  static const string method_name = "RefrMerge::_readFields()";

  DsMdvx *input_file = new DsMdvx;
  
  // Set up the read request

  input_file->setReadTime(Mdvx::READ_CLOSEST,
			  url, _params->max_input_valid_secs,
			  data_time.utime());
  
  input_file->clearReadFields();
  
  input_file->addReadField(n_field_name);
  input_file->addReadField(sigma_n_field_name);
  
  input_file->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file->setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file->setReadScalingType(Mdvx::SCALING_NONE);
  
  input_file->setReadRemap(_projection);
  
  if (_params->debug)
  {
    cerr << endl;
    input_file->printReadRequest(cerr);
  }
  
  // Read the data

  if (input_file->readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV volume for time: " << data_time << endl;
    
    delete input_file;
    return 0;
  }
  
  // Make sure we were able to read both fields.  I don't know if the
  // readVolume() call will fail if only one field is found.

  if (input_file->getNFields() != 2)
  {
    delete input_file;
    return 0;
  }
  
  return input_file;
}


/*********************************************************************
 * _updateOutputMasterHeader() - Update the master header values for
 *                               the output file.
 */

void RefrMerge::_updateOutputMasterHeader(DsMdvx &output_file,
					  const DateTime &data_time) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = data_time.utime();
  master_hdr.time_end = data_time.utime();
  master_hdr.time_centroid = data_time.utime();
  master_hdr.time_expire = data_time.utime();
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  
  STRcopy(master_hdr.data_set_info, "RefrMerge output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "RefrMerge", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, "RefrMerge", MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
