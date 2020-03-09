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
//   $Id: VerticalShear.cc,v 1.6 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * VerticalShear: VerticalShear program object.
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

#include "VerticalShear.hh"
#include "Params.hh"


// Global variables

VerticalShear *VerticalShear::_instance =
     (VerticalShear *)NULL;


/*********************************************************************
 * Constructor
 */

VerticalShear::VerticalShear(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "VerticalShear::VerticalShear()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (VerticalShear *)NULL);
  
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

  _inputPath = NULL;

  _dataTrigger = NULL;
}


/*********************************************************************
 * Destructor
 */

VerticalShear::~VerticalShear()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  if (_dataTrigger)
    delete _dataTrigger;
  
  if (_inputPath)
    delete _inputPath;

  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

VerticalShear *VerticalShear::Inst(int argc, char **argv)
{
  if (_instance == (VerticalShear *)NULL)
    new VerticalShear(argc, argv);
  
  return(_instance);
}

VerticalShear *VerticalShear::Inst()
{
  assert(_instance != (VerticalShear *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool VerticalShear::init()
{
  static const string method_name = "VerticalShear::init()";
  
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

  case Params::LOCAL_FILEPATH_REALTIME:
    {
       _inputPath = new DsInputPath(_progName,
				    (bool)_params->debug,
				    _params->input_url,
				    7200,
				    PMU_auto_register);
       if (!_inputPath)
	 {
	   cerr << "Failed to instantiate DsInputPath object for LOCAL_FILEPATH_REALTIME" << endl;
	   
	   return false;
	 }
       
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

void VerticalShear::run()
{
  static const string method_name = "VerticalShear::run()";
  
  DateTime trigger_time;
  
  if (_params->trigger_mode != Params::LOCAL_FILEPATH_REALTIME)
    {

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
  else
    {
      //
      // process data: Mode is LOCAL_FILEPATH_REALTIME (DsInput path used)
      //
      time_t inputTime;
      
      char *file;
      
      while ( (file = _inputPath->next()) != NULL)
	{
	  inputTime = _inputPath->getDataTime(file);
	  
	  if (!_processData(inputTime))
	    {	       
	      cerr << "ERROR: " << method_name << endl;

	      cerr << "Error processing data for time: " << trigger_time << endl;
	      continue; 
	    }
	} // while
    }  
} 
  


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcVerticalShear() - Calculate the vertical shear field.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *VerticalShear::_calcVerticalShear(const MdvxField &u_field,
					     const MdvxField &v_field) const
{
  static const string method_name = "VerticalShear::_calcVerticalShear()";
  
  MdvxField *vert_shear_field;
  
  // Create the vertical shear field

  if ((vert_shear_field = _createVertShearField(u_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating vertical shear field" << endl;
    
    return 0;
  }
  
  // Get all of the header information and make sure the projections
  // are compatible

  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field.getFieldHeader();
  Mdvx::vlevel_header_t u_vlevel_hdr = u_field.getVlevelHeader();
  
  Mdvx::field_header_t vert_shear_field_hdr =
    vert_shear_field->getFieldHeader();
  
  MdvxPjg u_proj(u_field_hdr);
  MdvxPjg v_proj(v_field_hdr);
  MdvxPjg vert_shear_proj(vert_shear_field_hdr);
  
  if (u_proj != v_proj)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot calculate vertical shear" << endl;
    cerr << "U and V fields have different projections" << endl;
    
    delete vert_shear_field;
    
    return 0;
  }
  
  fl32 *u = (fl32 *)u_field.getVol();
  fl32 *v = (fl32 *)v_field.getVol();
  fl32 *vert_shear_data = (fl32 *)vert_shear_field->getVol();
  
  int upper_z_index = u_proj.getNz() - 1;
  double lower_z_level = u_vlevel_hdr.level[0];
  double upper_z_level = u_vlevel_hdr.level[upper_z_index];
  
  for (int x = 0; x < u_proj.getNx(); ++x)
  {
    for (int y = 0; y < u_proj.getNy(); ++y)
    {
      int lower_index = u_proj.xyIndex2arrayIndex(x, y, 0);
      int upper_index = u_proj.xyIndex2arrayIndex(x, y, upper_z_index);
      int vert_shear_index = vert_shear_proj.xyIndex2arrayIndex(x, y, 0);
      
      if (u[upper_index] == u_field_hdr.missing_data_value ||
	  u[upper_index] == u_field_hdr.bad_data_value ||
	  u[lower_index] == u_field_hdr.missing_data_value ||
	  u[lower_index] == u_field_hdr.bad_data_value ||
	  v[upper_index] == v_field_hdr.missing_data_value ||
	  v[upper_index] == v_field_hdr.bad_data_value ||
	  v[lower_index] == v_field_hdr.missing_data_value ||
	  v[lower_index] == v_field_hdr.bad_data_value)
      {
	vert_shear_data[vert_shear_index] = 
	  vert_shear_field_hdr.missing_data_value;
	
	continue;
      }
      
      double u_diff = u[upper_index] - u[lower_index];
      double v_diff = v[upper_index] - v[lower_index];
      double level_diff = lower_z_level - upper_z_level;
      
      double vert_shear =
	sqrt((u_diff * u_diff) + (v_diff * v_diff)) / level_diff;
      
      vert_shear_data[vert_shear_index] = (fl32)vert_shear;
      
    } /* endfor - y */
  } /* endfor - x */
  
  return vert_shear_field;
}


/*********************************************************************
 * _createVertShearField() - Create the blank vertical shear field.  Upon
 *                           return, the field values will be set to the
 *                           U field's missing data value.
 *
 * Returns a pointer to the created field, or 0 if the field could
 * not be created for some reason.
 */

MdvxField *VerticalShear::_createVertShearField(const MdvxField &u_field) const
{
  // Create the new field header

  Mdvx::field_header_t u_field_hdr = u_field.getFieldHeader();
  
  Mdvx::field_header_t field_hdr;
  
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.field_code = u_field_hdr.field_code;
  field_hdr.forecast_delta = u_field_hdr.forecast_delta;
  field_hdr.forecast_time = u_field_hdr.forecast_time;
  field_hdr.nx = u_field_hdr.nx;
  field_hdr.ny = u_field_hdr.ny;
  field_hdr.nz = 1;
  field_hdr.proj_type = u_field_hdr.proj_type;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = u_field_hdr.native_vlevel_type;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  
  field_hdr.proj_origin_lat = u_field_hdr.proj_origin_lat;
  field_hdr.proj_origin_lon = u_field_hdr.proj_origin_lon;
  for (int i = 0; i < MDV_MAX_PROJ_PARAMS; ++i)
    field_hdr.proj_param[i] = u_field_hdr.proj_param[i];
  field_hdr.vert_reference = 0;
  field_hdr.grid_dx = u_field_hdr.grid_dx;
  field_hdr.grid_dy = u_field_hdr.grid_dy;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = u_field_hdr.grid_minx;
  field_hdr.grid_miny = u_field_hdr.grid_miny;
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = u_field_hdr.bad_data_value;
  field_hdr.missing_data_value = u_field_hdr.missing_data_value;
  field_hdr.proj_rotation = u_field_hdr.proj_rotation;
  
  STRcopy(field_hdr.field_name_long, "magnitude of vertical shear",
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "vert_shear", MDV_SHORT_FIELD_LEN);
  
  switch (u_field_hdr.vlevel_type)
  {
  case Mdvx::VERT_TYPE_PRESSURE :
    STRcopy(field_hdr.units, "m/s/mb", MDV_UNITS_LEN);
    break;
    
  case Mdvx::VERT_TYPE_Z :
    STRcopy(field_hdr.units, "m/s/km", MDV_UNITS_LEN);
    break;
    
  default :
    STRcopy(field_hdr.units, "unknown", MDV_UNITS_LEN);
    break;
    
  }
  
  field_hdr.transform[0] = '\0';
  
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

bool VerticalShear::_processData(const time_t trigger_time)
{
  static const string method_name = "VerticalShear::_processData()";
  
  // Read in the input file

  DsMdvx input_file;
  
  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 _params->input_url,
			 0, trigger_time);
  
  input_file.clearReadFields();
  
  if (_params->use_field_name)
  {
    input_file.addReadField(_params->u_field.field_name);
    input_file.addReadField(_params->v_field.field_name);
  }
  else
  {
    input_file.addReadField(_params->u_field.field_num);
    input_file.addReadField(_params->v_field.field_num);
  }

  input_file.setReadVlevelLimits(_params->pressure_limits.lower_level,
				 _params->pressure_limits.upper_level);
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  
  //
  // Remap data 
  //
  if (_params->remap_grid)
    {
      switch (_params->grid_projection)
	{
	  case Params::PROJ_FLAT:
	    input_file.setReadRemapFlat(_params->grid_params.nx,
					_params->grid_params.ny,
					_params->grid_params.minx,
					_params->grid_params.miny,
					_params->grid_params.dx,
					_params->grid_params.dy,
					_params->origin_lat,
					_params->origin_lon,
					_params->flat_rotation);
	    break;
	    
	case Params::PROJ_LATLON :
	  
	  input_file.setReadRemapLatlon(_params->grid_params.nx,
					_params->grid_params.ny,
					_params->grid_params.minx,
					_params->grid_params.miny,
					_params->grid_params.dx,
					_params->grid_params.dy);
	  break;
	  
	case Params::PROJ_LAMBERT :
	  
	  input_file.setReadRemapLc2(_params->grid_params.nx,
				     _params->grid_params.ny,
				     _params->grid_params.minx,
				     _params->grid_params.miny,
				     _params->grid_params.dx,
				     _params->grid_params.dy,
				     _params->origin_lat,
				     _params->origin_lon,
				     _params->lambert_lat1,
				     _params->lambert_lat2);
	  break;
	  
	default:
	  
	  break;
	} // end switch
    } // end remap

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

  if (_params->output_as_forecast)
    output_file.setWriteAsForecast();

  _updateOutputMasterHeader(output_file,
			    input_file.getMasterHeader(),
			    input_file.getPathInUse());
  
  // Retrieve the field information from the input file

  MdvxField *u_field;
    
  if ((u_field = input_file.getField(0)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting U field from input file" << endl;

    return false;
  }
    
  MdvxField *v_field;
    
  if ((v_field = input_file.getField(1)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting V field from input file" << endl;

    return false;
  }
    
  // Calculate the vertical shear field

  MdvxField *vert_shear_field;
    
  if ((vert_shear_field = _calcVerticalShear(*u_field, *v_field)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating vertical shear field" << endl;
    
    return false;
  }
    
  // Add the vertical shear field to the output file

  vert_shear_field->convertType(Mdvx::ENCODING_INT8,
				Mdvx::COMPRESSION_RLE,
				Mdvx::SCALING_DYNAMIC);
    
  output_file.addField(vert_shear_field);
    
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
 * _updateOutputMasterHeader() - Update the master header values for
 *                               the output file.
 */

void VerticalShear::_updateOutputMasterHeader(DsMdvx &output_file,
					     const Mdvx::master_header_t &input_master_hdr,
					     const string &data_set_source)
{
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = input_master_hdr.time_gen;
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
  
  STRcopy(master_hdr.data_set_info, "VerticalShear output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "VerticalShear", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, data_set_source.c_str(), MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}
