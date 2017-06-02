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
//   $Date: 2016/03/04 02:22:09 $
//   $Id: MaliSat2Mdv.cc,v 1.3 2016/03/04 02:22:09 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MaliSat2Mdv: MaliSat2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2014
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MaliSat2Mdv.hh"
#include "Params.hh"

using namespace std;

// Global variables

MaliSat2Mdv *MaliSat2Mdv::_instance =
     (MaliSat2Mdv *)NULL;

const string MaliSat2Mdv::LAT_DIM_NAME = "lat";
const string MaliSat2Mdv::LON_DIM_NAME = "lon";

const string MaliSat2Mdv::LATMIN_ATT_NAME = "latmin";
const string MaliSat2Mdv::LONMIN_ATT_NAME = "lonmin";
const string MaliSat2Mdv::LATRES_ATT_NAME = "latres";
const string MaliSat2Mdv::LONRES_ATT_NAME = "lonres";

const string MaliSat2Mdv::TEMP_VAR_NAME = "temperature";
const string MaliSat2Mdv::TEMP_UNITS_ATT_NAME = "units";
const string MaliSat2Mdv::TEMP_MISSING_VALUE_ATT_NAME = "_FillValue";

const double MaliSat2Mdv::MISSING_DATA_VALUE = -9999.0;


/*********************************************************************
 * Constructors
 */

MaliSat2Mdv::MaliSat2Mdv(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MaliSat2Mdv::MaliSat2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MaliSat2Mdv *)NULL);
  
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

MaliSat2Mdv::~MaliSat2Mdv()
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

MaliSat2Mdv *MaliSat2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (MaliSat2Mdv *)NULL)
    new MaliSat2Mdv(argc, argv);
  
  return(_instance);
}

MaliSat2Mdv *MaliSat2Mdv::Inst()
{
  assert(_instance != (MaliSat2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MaliSat2Mdv::init()
{
  static const string method_name = "MaliSat2Mdv::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MaliSat2Mdv::run()
{
  static const string method_name = "MaliSat2Mdv::run()";
  
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
    
    _processFile(trigger_info.getFilePath());

  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getTempField() - Get the temperature field from the given netCDF file and put
 *                   it into the given MDV file.
 *
 * Returns true on success, false on failure.
 */

bool MaliSat2Mdv::_getTempField(Mdvx &mdvx,
				const Nc3File &nc_file,
				const string &input_file_path) const
{
  static const string method_name = "MaliSat2Mdv::_getTempField()";

  // Get the temperature variable and its attributes from the netCDF file

  Nc3Var *temp_var;
  
  if ((temp_var = nc_file.get_var(TEMP_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << TEMP_VAR_NAME << " variable from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  Nc3Att *units_att;
  
  if ((units_att = temp_var->get_att(TEMP_UNITS_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << TEMP_UNITS_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  string units = units_att->as_string(0);
  delete units_att;
  
  Nc3Att *missing_value_att;
  
  if ((missing_value_att =
       temp_var->get_att(TEMP_MISSING_VALUE_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << TEMP_MISSING_VALUE_ATT_NAME
	 << " attribute from file " << input_file_path << endl;
    
    return false;
  }
  
  double missing_value = missing_value_att->as_double(0);
  delete missing_value_att;
  
  // Create the field and vlevel headers

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  if (!_updateFieldHeader(field_hdr, nc_file,
			  input_file_path, units))
    return false;
  
  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_SATELLITE_IMAGE;
  
  // Create the temperature field

  si16 *temp_nc_data = new si16[field_hdr.nx * field_hdr.ny];
  
  if (!temp_var->get(temp_nc_data, 1, field_hdr.ny, field_hdr.nx))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting temperature data from file " << input_file_path << endl;
    
    delete [] temp_nc_data;
    
    return false;
  }
  
  fl32 *temp_mdv_data = new fl32[field_hdr.nx * field_hdr.ny];
  
  for (int x = 0; x < field_hdr.nx; ++x)
  {
    for (int y = 0; y < field_hdr.ny; ++y)
    {
      int nc_index;
      
      nc_index = ((field_hdr.ny - y - 1) * field_hdr.nx) + x;

      int mdv_index = (y * field_hdr.nx) + x;
      
      if (temp_nc_data[nc_index] == missing_value)
	temp_mdv_data[mdv_index] = MISSING_DATA_VALUE;
      else
	temp_mdv_data[mdv_index] = (fl32)temp_nc_data[nc_index];
    }
  }
  
  delete [] temp_nc_data;
  
  MdvxField *temp_field = new MdvxField(field_hdr, vlevel_hdr, temp_mdv_data);
  
  if (temp_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating temperature field for MDV file" << endl;
    
    delete [] temp_mdv_data;
    
    return false;
  }
  
  delete [] temp_mdv_data;
  
  // Compress the data and add the new field to the MDV file

  temp_field->convertType(Mdvx::ENCODING_FLOAT32,
			  Mdvx::COMPRESSION_BZIP,
			  Mdvx::SCALING_NONE);
  
  mdvx.addField(temp_field);
  
  return true;
}


/*********************************************************************
 * _getDataTime() - Get the data time and accumulation period from the
 *                  input file name.
 *
 * Returns the data time on success, DateTime::NEVER on failure.
 */

DateTime MaliSat2Mdv::_getDataTime(const string &input_file_path) const
{
  static const string method_name = "MaliSat2Mdv::_getDataTime()";
  
  // Extract the date and time information from the file name

  Path path(input_file_path);

  DateTime file_time;
  
  string input_filename = path.getBase();
  
  if (input_filename.length() != 19)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Input file name not correct length" << endl;
    cerr << "Expected 19 characters, file name has "
	 << input_filename.length() << " characaters" << endl;
    
    return DateTime::NEVER;
  }
    
  string year_str = input_filename.substr(7,4);
  string month_str = input_filename.substr(11,2);
  string day_str = input_filename.substr(13,2);
  string hour_str = input_filename.substr(15,2);
  string minute_str = input_filename.substr(17,2);
  
  file_time.set(atoi(year_str.c_str()),
		atoi(month_str.c_str()),
		atoi(day_str.c_str()),
		atoi(hour_str.c_str()),
		atoi(minute_str.c_str()));
  
  if (_params->debug)
    cerr << "    File time: " << file_time.dtime() << endl;
  
  return file_time;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 */

bool MaliSat2Mdv::_initTrigger()
{
  static const string method_name = "MaliSat2Mdv;:_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, false,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR_RECURSE :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, true,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR_RECURSE trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

return true;
}

    
/*********************************************************************
 * _processFile() - Process the given file.
 */

bool MaliSat2Mdv::_processFile(const string &input_file_path)
{
  static const string method_name = "MaliSat2Mdv::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << endl << "*** Processing file: " << input_file_path << endl;
  
  // Create an error object so that the netCDF library doesn't exit when an
  // error is encountered.  This object is not explicitly used in the below
  // code, but is used implicitly by the netCDF library.

  Nc3Error nc_error(Nc3Error::silent_nonfatal);

  // Open the input file
  
  Nc3File nc_file(input_file_path.c_str());
  
  if (!nc_file.is_valid())
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid netCDF file: " << input_file_path << endl;
    
    return false;
  }
  
  // Create the output file

  DsMdvx mdvx;
  
  // Process the data

  DateTime valid_time;
  if ((valid_time = _getDataTime(input_file_path)) == DateTime::NEVER)
    return false;
  
  if (!_updateMasterHeader(mdvx, valid_time, input_file_path))
    return false;

  if (!_getTempField(mdvx, nc_file, input_file_path))
    return false;
  
  // Write the output file

  mdvx.setWriteLdataInfo();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file for time "
	 << DateTime::str(mdvx.getMasterHeader().time_centroid) << endl;
    cerr << mdvx.getErrStr();
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _updateFieldHeader() - Update the field header for the ACCUM field 
 *                        based on information in the netCDF file.
 *
 * Returns true on success, false on failure.
 */

bool MaliSat2Mdv::_updateFieldHeader(Mdvx::field_header_t &field_hdr,
				     const Nc3File &nc_file,
				     const string &input_file_path,
				     const string &units) const
{
  static const string method_name = "MaliSat2Mdv::_updateFieldHeader()";

  // Get the needed information from the netCDF file

  Nc3Dim *lat_dim;

  if ((lat_dim = nc_file.get_dim(LAT_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LAT_DIM_NAME << " dimension from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  Nc3Dim *lon_dim;

  if ((lon_dim = nc_file.get_dim(LON_DIM_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LON_DIM_NAME << " dimension from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "   nx = " << lon_dim->size() << ", ny = " << lat_dim->size() << endl;
  
  Nc3Att *latmin_att;
  
  if ((latmin_att = nc_file.get_att(LATMIN_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LATMIN_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double latmin = latmin_att->as_double(0);
  delete latmin_att;
  
  Nc3Att *lonmin_att;
  
  if ((lonmin_att = nc_file.get_att(LONMIN_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LONMIN_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double lonmin = lonmin_att->as_double(0);
  delete lonmin_att;
  
  Nc3Att *latres_att;
  
  if ((latres_att = nc_file.get_att(LATRES_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LATRES_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double latres = latres_att->as_double(0);
  delete latres_att;
  
  Nc3Att *lonres_att;
  
  if ((lonres_att = nc_file.get_att(LONRES_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LONRES_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double lonres = lonres_att->as_double(0);
  delete lonres_att;
  
  // Update the field header information

  field_hdr.nx = lon_dim->size();
  field_hdr.ny = lat_dim->size();
  field_hdr.nz = 1;
  field_hdr.proj_type = Mdvx::PROJ_LATLON;
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  field_hdr.vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = 2;
  field_hdr.proj_origin_lat = 0.0;
  field_hdr.proj_origin_lon = 0.0;
  field_hdr.grid_dx = lonres;
  field_hdr.grid_dy = latres;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = lonmin;
  field_hdr.grid_miny = latmin;
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(field_hdr.field_name_long, "temperature", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "temp", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  return true;
}


/*********************************************************************
 * _updateMasterHeader() - Update the master header in the given MDV
 *                         file using information from the given netCDF
 *                         file and file path.
 *
 * Returns true on success, false on failure.
 */

bool MaliSat2Mdv::_updateMasterHeader(Mdvx &mdvx,
				      const DateTime &valid_time,
				      const string &input_file_path) const
{
  static const string method_name = "MaliSat2Mdv::_updateMasterHeader()";
  
  // Set the values in the master header

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = valid_time.utime();
  master_hdr.time_end = valid_time.utime();
  master_hdr.time_centroid = valid_time.utime();
  master_hdr.time_expire = valid_time.utime();
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  master_hdr.vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info, "MaliSat2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "MaliSat2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_file_path.c_str(),
	  MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);

  return true;
}
