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
 * @file MdvRemapNormalize.cc
 *
 * @class MdvRemapNormalize
 *
 * MdvRemapNormalize is the top level application class.
 *  
 * @date 11/20/2002
 *
 */

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

#include "MdvRemapNormalize.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvRemapNormalize *MdvRemapNormalize::_instance =
     (MdvRemapNormalize *)NULL;



/*********************************************************************
 * Constructor
 */

MdvRemapNormalize::MdvRemapNormalize(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvRemapNormalize::MdvRemapNormalize()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvRemapNormalize *)NULL);
  
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

MdvRemapNormalize::~MdvRemapNormalize()
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

MdvRemapNormalize *MdvRemapNormalize::Inst(int argc, char **argv)
{
  if (_instance == (MdvRemapNormalize *)NULL)
    new MdvRemapNormalize(argc, argv);
  
  return(_instance);
}

MdvRemapNormalize *MdvRemapNormalize::Inst()
{
  assert(_instance != (MdvRemapNormalize *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvRemapNormalize::init()
{
  static const string method_name = "MdvRemapNormalize::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialze the output projection

  if (!_initOutputProj())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void MdvRemapNormalize::run()
{
  static const string method_name = "MdvRemapNormalize::run()";
  
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
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initOutputProj()
 */

bool MdvRemapNormalize::_initOutputProj(void)
{
  static const string method_name = "MdvRemapNormalize::_initOutputProj()";
  
  _outputProj.initLatlon(_params->output_proj.nx, _params->output_proj.ny, 1,
			 _params->output_proj.dx, _params->output_proj.dy, 1.0,
			 _params->output_proj.minx, _params->output_proj.miny,
			 1.0);
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool MdvRemapNormalize::_initTrigger(void)
{
  static const string method_name = "MdvRemapNormalize::_initTrigger()";
  
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
 * _processData()
 */

bool MdvRemapNormalize::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvRemapNormalize::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read the input data

  DsMdvx input_mdvx;
  
  if (!_readInputFile(input_mdvx, trigger_time))
    return false;
  
  // Create the output data

  DsMdvx output_mdvx;
  
  if (!_remapData(input_mdvx, output_mdvx))
    return false;
  
  // Write the file

  if (output_mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing file to output URL: " << _params->output_url << endl;
    cerr << output_mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile()
 */

bool MdvRemapNormalize::_readInputFile(Mdvx &input_file,
					    const DateTime &trigger_time) const
{
  static const string method_name = "MdvRemapNormalize::_readInputFile()";
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->input_url,
			 0, trigger_time.utime());
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _remapData()
 */

bool MdvRemapNormalize::_remapData(const DsMdvx &input_file,
				   DsMdvx &output_file) const
{
  static const string method_name = "MdvRemapNormalize::_remapData()";
  
  // Set the master header in the output file

  _setMasterHeader(output_file, input_file.getMasterHeader());
  
  // Loop through each of the fields, remapping the data and adding
  // the remapped field to the output file

  for (int field_num = 0; field_num < input_file.getMasterHeader().n_fields;
       ++field_num)
  {
    MdvxField *input_field = input_file.getField(field_num);
    MdvxField *output_field;
    
    if ((output_field = _remapField(*input_field)) == 0)
      continue;
    
    output_field->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_BZIP,
			      Mdvx::SCALING_DYNAMIC);
    
    output_file.addField(output_field);
    
  } /* endfor - field_num */
  
  return true;
}


/*********************************************************************
 * _remapField()
 */

MdvxField *MdvRemapNormalize::_remapField(const MdvxField &input_field) const
{
  static const string method_name = "MdvRemapNormalize::_remapField()";
  
  // Make sure that the input field is using a lat/lon projection

  Mdvx::field_header_t input_field_hdr = input_field.getFieldHeader();
  
  if (input_field_hdr.proj_type != Mdvx::PROJ_LATLON)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input field " << input_field_hdr.field_name
	 << " doesn't use a lat/lon projection" << endl;
    cerr << "Fields must be lat/lon for this application" << endl;
    
    return 0;
  }
  
  // Create the new field's field header.  Set each value explicitly
  // because sometimes fields are added to the header that shouldn't
  // be copied to a new field.

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  _outputProj.syncToFieldHdr(field_hdr);
  
  field_hdr.field_code = input_field_hdr.field_code;
  field_hdr.user_time1 = input_field_hdr.user_time1;
  field_hdr.forecast_delta = input_field_hdr.forecast_delta;
  field_hdr.user_time2 = input_field_hdr.user_time2;
  field_hdr.user_time3 = input_field_hdr.user_time3;
  field_hdr.forecast_time = input_field_hdr.forecast_time;
  field_hdr.user_time4 = input_field_hdr.user_time4;
  field_hdr.nz = input_field_hdr.nz;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  for (int i = 0; i < 10; ++i)
    field_hdr.user_data_si32[i] = input_field_hdr.user_data_si32[i];
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = input_field_hdr.transform_type;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = input_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = input_field_hdr.vlevel_type;
  field_hdr.dz_constant = input_field_hdr.dz_constant;
  field_hdr.data_dimension = input_field_hdr.data_dimension;
  field_hdr.zoom_clipped = input_field_hdr.zoom_clipped;
  field_hdr.zoom_no_overlap = input_field_hdr.zoom_no_overlap;
  field_hdr.vert_reference = input_field_hdr.vert_reference;
  field_hdr.grid_dz = input_field_hdr.grid_dz;
  field_hdr.grid_minz = input_field_hdr.grid_minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = input_field_hdr.bad_data_value;
  field_hdr.missing_data_value = input_field_hdr.missing_data_value;
  for (int i = 0; i < 4; ++i)
    field_hdr.user_data_fl32[i] = input_field_hdr.user_data_fl32[i];
  STRcopy(field_hdr.field_name_long, input_field_hdr.field_name_long,
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, input_field_hdr.field_name,
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, input_field_hdr.units, MDV_UNITS_LEN);
  STRcopy(field_hdr.transform, input_field_hdr.transform, MDV_TRANSFORM_LEN);
  
  // Create the output field.  The vlevel header for the output field will
  // be the same as the vlevel header for the input field.

  MdvxField *output_field;
  
  if ((output_field = new MdvxField(field_hdr, input_field.getVlevelHeader(),
				    (void *)0, true)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output field for field "
	 << field_hdr.field_name << endl;
    
    return 0;
  }
  
  // Remap the input data

  MdvxPjg input_proj(input_field_hdr);
  fl32 *input_data = (fl32 *)input_field.getVol();
  fl32 *output_data = (fl32 *)output_field->getVol();
  
  double output_lon = field_hdr.grid_minx + (field_hdr.grid_dx / 2.0);
    
  for (int x = 0; x < field_hdr.nx; ++x, output_lon += field_hdr.grid_dx)
  {
    // Normalize the output longitude to the input projection

    double output_lon_norm = output_lon;
      
    while (output_lon_norm < input_field_hdr.grid_minx)
      output_lon_norm += 360.0;
    while (output_lon_norm >= input_field_hdr.grid_minx + 360.0)
      output_lon_norm -= 360.0;
	
    double output_lat = field_hdr.grid_miny + (field_hdr.grid_dy / 2.0);

    for (int y = 0; y < field_hdr.ny; ++y, output_lat += field_hdr.grid_dy)
    {
      // Find this data location in the input grid and copy it to the
      // output grid

      int input_x;
      int input_y;
      
      if (input_proj.latlon2xyIndex(output_lat, output_lon_norm,
				    input_x, input_y) != 0)
	continue;
	
      for (int z = 0; z < field_hdr.nz; ++z)
      {
	int input_index = (z * input_proj.getNx() * input_proj.getNy()) +
	  (input_y * input_proj.getNx()) + input_x;
	int output_index = (z * _outputProj.getNx() * _outputProj.getNy()) +
	  (y * _outputProj.getNx()) + x;
	
	output_data[output_index] = input_data[input_index];
	
      } /* endfor - z */
    } /* endfor - y */
  } /* endfor - x */
  
  return output_field;
}


/*********************************************************************
 * _setMasterHeader()
 */

void MdvRemapNormalize::_setMasterHeader(DsMdvx &output_file,
					 const Mdvx::master_header_t &input_master_hdr) const
{
  static const string method_name = "MdvRemapNormalize::_setMasterHeader()";
  
  // Set the master header field individually because fields might be added
  // in the future that we don't want to copy from the input header.  This
  // has happened in the past....

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = input_master_hdr.time_gen;
  master_hdr.user_time = input_master_hdr.user_time;
  master_hdr.time_begin = input_master_hdr.time_begin;
  master_hdr.time_end = input_master_hdr.time_end;
  master_hdr.time_centroid = input_master_hdr.time_centroid;
  master_hdr.time_expire = input_master_hdr.time_expire;
  master_hdr.num_data_times = input_master_hdr.num_data_times;
  master_hdr.index_number = input_master_hdr.index_number;
  master_hdr.data_collection_type = input_master_hdr.data_collection_type;
  master_hdr.user_data = input_master_hdr.user_data;
  master_hdr.native_vlevel_type = input_master_hdr.native_vlevel_type;
  master_hdr.vlevel_type = input_master_hdr.vlevel_type;
  master_hdr.vlevel_included = input_master_hdr.vlevel_included;
  master_hdr.grid_orientation = input_master_hdr.grid_orientation;
  master_hdr.data_ordering = input_master_hdr.data_ordering;
  for (int i = 0; i < 8; ++i)
    master_hdr.user_data_si32[i] = input_master_hdr.user_data_si32[i];
  master_hdr.epoch = input_master_hdr.epoch;
  for (int i = 0; i < 6; ++i)
    master_hdr.user_data_fl32[i] = input_master_hdr.user_data_fl32[i];
  master_hdr.sensor_lon = input_master_hdr.sensor_lon;
  master_hdr.sensor_lat = input_master_hdr.sensor_lat;
  master_hdr.sensor_alt = input_master_hdr.sensor_alt;
  STRcopy(master_hdr.data_set_info, input_master_hdr.data_set_info,
	  MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, input_master_hdr.data_set_name,
	  MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_master_hdr.data_set_source,
	  MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
