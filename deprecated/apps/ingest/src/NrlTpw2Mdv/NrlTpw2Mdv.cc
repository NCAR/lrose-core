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
//   $Author: jcraig $
//   $Locker:  $
//   $Date: 2018/01/22 19:54:16 $
//   $Id: NrlTpw2Mdv.cc,v 1.8 2018/01/22 19:54:16 jcraig Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NrlTpw2Mdv: NrlTpw2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2007
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

#include "NrlTpw2Mdv.hh"
#include "Params.hh"

using namespace std;

// Global variables

NrlTpw2Mdv *NrlTpw2Mdv::_instance =
     (NrlTpw2Mdv *)NULL;

const string NrlTpw2Mdv::LAT_ATT_NAME = "center_lat";
const string NrlTpw2Mdv::LON_ATT_NAME = "center_lon";

const string NrlTpw2Mdv::TPW_VAR_NAME = "water_vapor";
const string NrlTpw2Mdv::TPW_UNITS_ATT_NAME = "units";
const string NrlTpw2Mdv::TPW_MISSING_VALUE_ATT_NAME = "missing_value";

const double NrlTpw2Mdv::MISSING_DATA_VALUE = -9999.0;


/*********************************************************************
 * Constructors
 */

NrlTpw2Mdv::NrlTpw2Mdv(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "NrlTpw2Mdv::NrlTpw2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (NrlTpw2Mdv *)NULL);
  
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
  char *params_path = (char *)"unknown";
  
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

NrlTpw2Mdv::~NrlTpw2Mdv()
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

NrlTpw2Mdv *NrlTpw2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (NrlTpw2Mdv *)NULL)
    new NrlTpw2Mdv(argc, argv);
  
  return(_instance);
}

NrlTpw2Mdv *NrlTpw2Mdv::Inst()
{
  assert(_instance != (NrlTpw2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool NrlTpw2Mdv::init()
{
  static const string method_name = "NrlTpw2Mdv::init()";
  
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

void NrlTpw2Mdv::run()
{
  static const string method_name = "NrlTpw2Mdv::run()";
  
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
 * _getTpwField() - Get the TPW field from the given netCDF file and put
 *                  it into the given MDV file.
 *
 * Returns true on success, false on failure.
 */

bool NrlTpw2Mdv::_getTpwField(Mdvx &mdvx,
			      const Nc3File &nc_file,
			      const string &input_file_path) const
{
  static const string method_name = "NrlTpw2Mdv::_getTpwField()";

  // Get the TPW variable and its attributes from the netCDF file

  Nc3Var *tpw_var;
  
  if ((tpw_var = nc_file.get_var(TPW_VAR_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << TPW_VAR_NAME << " variable from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  Nc3Att *units_att;
  
  if ((units_att = tpw_var->get_att(TPW_UNITS_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << TPW_UNITS_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  string units = units_att->as_string(0);
  delete units_att;
  
  Nc3Att *missing_value_att;
  
  if ((missing_value_att =
       tpw_var->get_att(TPW_MISSING_VALUE_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << TPW_MISSING_VALUE_ATT_NAME
	 << " attribute from file " << input_file_path << endl;
    
    return false;
  }
  
  double missing_value = missing_value_att->as_double(0);
  delete missing_value_att;
  
  // Create the field and vlevel headers

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  if (!_updateFieldHeader(field_hdr, nc_file, input_file_path, units))
    return false;
  
  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_SATELLITE_IMAGE;
  
  // Create the TPW field

  float *tpw_nc_data = new float[field_hdr.nx * field_hdr.ny];
  
  if (!tpw_var->get(tpw_nc_data, field_hdr.ny, field_hdr.nx))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting TPW data from file " << input_file_path << endl;
    
    delete [] tpw_nc_data;
    
    return false;
  }
  
  fl32 *tpw_mdv_data = new fl32[field_hdr.nx * field_hdr.ny];
  
  for (int x = 0; x < field_hdr.nx; ++x)
  {
    for (int y = 0; y < field_hdr.ny; ++y)
    {
      int nc_index;
      
      if (_params->input_x_left_to_right)
	nc_index = ((field_hdr.ny - y - 1) * field_hdr.nx) + x;
      else
	nc_index = ((field_hdr.ny - y - 1) * field_hdr.nx) + 
	  (field_hdr.nx - x - 1);

      int mdv_index = (y * field_hdr.nx) + x;
      
      if (tpw_nc_data[nc_index] == missing_value)
	tpw_mdv_data[mdv_index] = MISSING_DATA_VALUE;
      else
	tpw_mdv_data[mdv_index] = (fl32)tpw_nc_data[nc_index];
    }
  }
  
  delete [] tpw_nc_data;
  
  MdvxField *tpw_field = new MdvxField(field_hdr, vlevel_hdr, tpw_mdv_data);
  
  if (tpw_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating TPW field for MDV file" << endl;
    
    delete [] tpw_mdv_data;
    
    return false;
  }
  
  delete [] tpw_mdv_data;
  
  // Compress the data and add the new field to the MDV file

  tpw_field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_BZIP,
			 Mdvx::SCALING_DYNAMIC);
  
  mdvx.addField(tpw_field);
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 */

bool NrlTpw2Mdv::_initTrigger()
{
  static const string method_name = "NrlTpw2Mdv;:_initTrigger()";
  
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

bool NrlTpw2Mdv::_processFile(const string &input_file_path)
{
  static const string method_name = "NrlTpw2Mdv::_processFile()";
  
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

  if (!_updateMasterHeader(mdvx, nc_file, input_file_path))
    return false;
  
  if (!_getTpwField(mdvx, nc_file, input_file_path))
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
 * _updateFieldHeader() - Update the field header for the TPW field 
 *                        based on information in the netCDF file.
 *
 * Returns true on success, false on failure.
 */

bool NrlTpw2Mdv::_updateFieldHeader(Mdvx::field_header_t &field_hdr,
				    const Nc3File &nc_file,
				    const string &input_file_path,
				    const string &tpw_units) const
{
  static const string method_name = "NrlTpw2Mdv::_updateFieldHeader()";

  // Get the needed information from the netCDF file

  Nc3Dim *x_dim;

  if ((x_dim = nc_file.get_dim(_params->x_dim_name)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << _params->x_dim_name << " dimension from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  Nc3Dim *y_dim;

  if ((y_dim = nc_file.get_dim(_params->y_dim_name)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << _params->y_dim_name << " dimension from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "   nx = " << x_dim->size() << ", ny = " << y_dim->size() << endl;
  
  Nc3Att *lat_att;
  
  if ((lat_att = nc_file.get_att(LAT_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LAT_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double lat = lat_att->as_double(0);
  delete lat_att;
  
  Nc3Att *lon_att;
  
  if ((lon_att = nc_file.get_att(LON_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LON_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double lon = lon_att->as_double(0);
  delete lon_att;
  
  // Update the field header information

  field_hdr.nx = x_dim->size();
  field_hdr.ny = y_dim->size();
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
  field_hdr.proj_origin_lat = lat;
  field_hdr.proj_origin_lon = lon;
  field_hdr.grid_dx = _params->input_grid.grid_dx;
  field_hdr.grid_dy = _params->input_grid.grid_dy;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = lon -
    (((double)x_dim->size() / 2.0) * _params->input_grid.grid_dx);
  field_hdr.grid_miny = lat -
    (((double)y_dim->size() / 2.0) * _params->input_grid.grid_dy);
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(field_hdr.field_name_long, "water_vapor", MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, "TPW", MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, tpw_units.c_str(), MDV_UNITS_LEN);
  
  return true;
}


/*********************************************************************
 * _updateMasterHeader() - Update the master header in the given MDV
 *                         file using information from the given netCDF
 *                         file and file path.
 *
 * Returns true on success, false on failure.
 */

bool NrlTpw2Mdv::_updateMasterHeader(Mdvx &mdvx,
				     const Nc3File &nc_file,
				     const string &input_file_path) const
{
  static const string method_name = "NrlTpw2Mdv::_updateMasterHeader()";
  
  // Extract the date and time information from the file name

  Path path(input_file_path);

  DateTime file_time;
  
  switch (_params->filename_type)
  {
  case Params::NRL_FILENAME :
  {
    string input_filename = path.getFile();
  
    if (input_filename.length() < 13)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Input file name not long enough" << endl;
      cerr << "Need at least 13 characters, file name has "
	   << input_filename.length() << " characater" << endl;
      
      return false;
    }
    
    string year_str = input_filename.substr(0,4);
    string month_str = input_filename.substr(4,2);
    string day_str = input_filename.substr(6,2);
    string hour_str = input_filename.substr(9,2);
    string minute_str = input_filename.substr(11,2);

    file_time.set(atoi(year_str.c_str()),
		  atoi(month_str.c_str()),
		  atoi(day_str.c_str()),
		  atoi(hour_str.c_str()),
		  atoi(minute_str.c_str()));
  
    break;
  }
  
  case Params::RAP_FILENAME :
  {
    string input_filename = path.getFile();
    
    if (input_filename.length() < 6)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Input file name not long enough" << endl;
      cerr << "Need at least 6 characters, file name has "
	   << input_filename.length() << " character" << endl;
      
      return false;
    }
    
    string input_dir = path.getDirectory();
    string input_subdir;
    
    size_t delimiter_loc = input_dir.rfind(path.getDelimiter(),
					   input_dir.length());
    if (delimiter_loc == string::npos)
      input_subdir = input_dir;
    else
      input_subdir = input_dir.substr(delimiter_loc + 1);
    
    if (input_subdir.length() != 8)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Input subdirectory not the right length: "
	   << input_subdir << endl;
      cerr << "Expected 6 characters, subdir has "
	   << input_subdir.length() << " characters" << endl;
    
      return false;
    }
    
    string year_str = input_subdir.substr(0,4);
    string month_str = input_subdir.substr(4,2);
    string day_str = input_subdir.substr(6,2);
    string hour_str = input_filename.substr(0,2);
    string min_str = input_filename.substr(2,2);
    string sec_str = input_filename.substr(4,2);
    
    file_time.set(atoi(year_str.c_str()),
		  atoi(month_str.c_str()),
		  atoi(day_str.c_str()),
		  atoi(hour_str.c_str()),
		  atoi(min_str.c_str()),
		  atoi(sec_str.c_str()));
    
    break;
  }
  
  }
  
  if (_params->debug)
    cerr << "    File time: " << file_time.dtime() << endl;
  
  // Get the attributes needed for the master header

  Nc3Att *lat_att;
  
  if ((lat_att = nc_file.get_att(LAT_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LAT_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double lat = lat_att->as_double(0);
  delete lat_att;
  
  Nc3Att *lon_att;
  
  if ((lon_att = nc_file.get_att(LON_ATT_NAME.c_str())) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting " << LON_ATT_NAME << " attribute from file "
	 << input_file_path << endl;
    
    return false;
  }
  
  double lon = lon_att->as_double(0);
  delete lon_att;
  
  // Set the values in the master header

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = file_time.utime();
  master_hdr.time_end = file_time.utime();
  master_hdr.time_centroid = file_time.utime();
  master_hdr.time_expire = file_time.utime();
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  master_hdr.vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.sensor_lat = lat;
  master_hdr.sensor_lon = lon;
  STRcopy(master_hdr.data_set_info, "NrlTpw2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "NrlTpw2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_file_path.c_str(),
	  MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);

  return true;
}
