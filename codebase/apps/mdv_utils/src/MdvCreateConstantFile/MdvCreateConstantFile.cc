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
 * @file MdvCreateConstantFile.cc
 *
 * @class MdvCreateConstantFile
 *
 * MdvCreateConstantFile is the top level application class.
 *  
 * @date 12/12/2011
 *
 */

#include <assert.h>
#include <iostream>
#include <string>

#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvCreateConstantFile.hh"
#include "Params.hh"

using namespace std;


// Global variables

MdvCreateConstantFile *MdvCreateConstantFile::_instance = (MdvCreateConstantFile *)NULL;

/*********************************************************************
 * Constructor
 */

MdvCreateConstantFile::MdvCreateConstantFile(int argc, char **argv)
{
  static const string method_name = "MdvCreateConstantFile::MdvCreateConstantFile()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvCreateConstantFile *)NULL);
  
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
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
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

MdvCreateConstantFile::~MdvCreateConstantFile()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

MdvCreateConstantFile *MdvCreateConstantFile::Inst(int argc, char **argv)
{
  if (_instance == (MdvCreateConstantFile *)NULL)
    new MdvCreateConstantFile(argc, argv);
  
  return(_instance);
}

MdvCreateConstantFile *MdvCreateConstantFile::Inst()
{
  assert(_instance != (MdvCreateConstantFile *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool MdvCreateConstantFile::init()
{
  static const string method_name = "MdvCreateConstantFile::init()";
  
  return true;
}
  

/*********************************************************************
 * run()
 */

void MdvCreateConstantFile::run()
{
  static const string method_name = "MdvCreateConstantFile::run()";
  
  // Create the MDV field

  MdvxField *field;
  
  if ((field = _createField()) == 0)
    return;
  
  // Create the MDV file

  DsMdvx mdvx;
  
  _updateMasterHeader(mdvx);
  
  // Write the file to the specified output URL

  if (_params->is_forecast)
    mdvx.setWriteAsForecast();
  else
    mdvx.clearWriteAsForecast();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing file to output URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;

    return;
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createField()
 */

MdvxField *MdvCreateConstantFile::_createField()
{
  static const string method_name = "MdvCreateConstantFile::_createField()";
  
  // Extract the data times from the parameter file

  DateTime valid_time(_params->valid_time);
  if (valid_time == DateTime::NEVER)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing valid_time from parameter file" << endl;
    cerr << "Please fix and try again" << endl;
    
    return 0;
  }
  
  DateTime gen_time(_params->gen_time);
  if (_params->is_forecast)
  {
    if (gen_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing gen_time from parameter file" << endl;
      cerr << "Please fix and try again" << endl;
      
      return 0;
    }
  }
  
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  if (_params->is_forecast)
  {
    field_hdr.forecast_delta = valid_time.utime() - gen_time.utime();
    field_hdr.forecast_time = valid_time.utime();
  }
  
  field_hdr.nx = _params->proj_info.nx;
  field_hdr.ny = _params->proj_info.ny;
  field_hdr.nz = _params->vlevel_info_n;
  
  switch (_params->proj_info.proj_type)
  {
  case Params::PROJ_LATLON:
    field_hdr.proj_type = Mdvx::PROJ_LATLON;
    break;

  case Params::PROJ_FLAT:
    field_hdr.proj_type = Mdvx::PROJ_FLAT;
    field_hdr.proj_param[0] = _params->proj_info.rotation;
    break;

  case Params::PROJ_LC:
    field_hdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    field_hdr.proj_param[0] = _params->proj_info.lat1;
    field_hdr.proj_param[1] = _params->proj_info.lat2;
    break;
  }
  
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = _vertType2Mdv(_params->native_vlevel_type);
  field_hdr.vlevel_type = _vertType2Mdv(_params->vlevel_type);
  if (_params->dz_constant)
    field_hdr.dz_constant = 1;
  else
    field_hdr.dz_constant = 0;
  
  if (field_hdr.nz > 1)
    field_hdr.data_dimension = 3;
  else
    field_hdr.data_dimension = 2;

  field_hdr.proj_origin_lat = _params->proj_info.origin_lat;
  field_hdr.proj_origin_lon = _params->proj_info.origin_lon;
  field_hdr.grid_dx = _params->proj_info.dx;
  field_hdr.grid_dy = _params->proj_info.dy;
  field_hdr.grid_dz = _params->proj_info.dz;
  field_hdr.grid_minx = _params->proj_info.minx;
  field_hdr.grid_miny = _params->proj_info.miny;
  field_hdr.grid_minz = _params->proj_info.minz;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  
  if (_params->field_value == 0.0)
  {
    field_hdr.bad_data_value = -1.0;
    field_hdr.missing_data_value = -1.0;
  }
  else
  {
    field_hdr.bad_data_value = 0.0;
    field_hdr.missing_data_value = 0.0;
  }
  
  field_hdr.min_value = _params->field_value;
  field_hdr.max_value = _params->field_value;
  
  STRcopy(field_hdr.field_name_long, _params->field_name_long,
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, _params->field_name, MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, _params->field_units, MDV_UNITS_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  for (int i = 0; i < _params->vlevel_info_n; ++i)
  {
    vlevel_hdr.type[i] = _vertType2Mdv(_params->_vlevel_info[i].vert_type);
    vlevel_hdr.level[i] = _params->_vlevel_info[i].vert_level;
  }
  
  // Create the new MDV field

  MdvxField *field;
  
  if ((field = new MdvxField(field_hdr, vlevel_hdr, (void *)0)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating new data field" << endl;
    
    return 0;
  }
  
  // Set the data value

  fl32 *data = (fl32 *)field->getVol();

  for (int i = 0; i < field_hdr.nx * field_hdr.ny * field_hdr.nz; ++i)
    data[i] = _params->field_value;
  
  // Compress the data as requested

  field->convertType(_encodingType2Mdv(_params->encoding_type),
		     _compressionType2Mdv(_params->compression_type),
		     _scalingType2Mdv(_params->scaling_type),
		     _params->scale, _params->bias);
  
  return field;
}


/*********************************************************************
 * _updateMasterHeader()
 */

void MdvCreateConstantFile::_updateMasterHeader(DsMdvx &mdvx)
{
  static const string method_name = "MdvCreateConstantFile::_updateMasterHeader()";
  
  // Extract the data times from the parameter file

  DateTime valid_time(_params->valid_time);
  if (valid_time == DateTime::NEVER)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error parsing valid_time from parameter file";
    cerr << "Please fix and try again" << endl;
    
    return;
  }
  
  DateTime gen_time(_params->gen_time);
  if (_params->is_forecast)
  {
    if (gen_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing gen_time from parameter file";
      cerr << "Please fix and try again" << endl;
      
      return;
    }
  }
  
  // Update the master header value

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  if (_params->is_forecast)
  {
    master_hdr.time_gen = gen_time.utime();
    master_hdr.time_begin = gen_time.utime();
    master_hdr.time_end = gen_time.utime();
    master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  }
  else
  {
    master_hdr.time_gen = time(0);
    master_hdr.time_begin = valid_time.utime();
    master_hdr.time_end = valid_time.utime();
    master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  }
  
  master_hdr.time_centroid = valid_time.utime();
  master_hdr.time_expire = valid_time.utime();

  if (_params->vlevel_info_n > 1)
    master_hdr.data_dimension = 3;
  else
    master_hdr.data_dimension = 2;
  
  master_hdr.native_vlevel_type = _vertType2Mdv(_params->native_vlevel_type);
  master_hdr.vlevel_type = _vertType2Mdv(_params->vlevel_type);
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.sensor_lon = _params->sensor_position.lon;
  master_hdr.sensor_lat = _params->sensor_position.lat;
  master_hdr.sensor_alt = _params->sensor_position.alt;
  
  STRcopy(master_hdr.data_set_info, _params->data_set_info, MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, _params->data_set_name, MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->data_set_source, MDV_NAME_LEN);
  
  // Set the master header in the file

  mdvx.setMasterHeader(master_hdr);
}
