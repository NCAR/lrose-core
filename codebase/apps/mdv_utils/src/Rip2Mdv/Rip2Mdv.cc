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
//   $Date: 2016/03/04 02:22:13 $
//   $Id: Rip2Mdv.cc,v 1.9 2016/03/04 02:22:13 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Rip2Mdv: Rip2Mdv program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Rip2Mdv.hh"
#include "Params.hh"

using namespace std;

// Global variables

Rip2Mdv *Rip2Mdv::_instance =
     (Rip2Mdv *)NULL;


const size_t Rip2Mdv::VAR_DESC_LEN = 64;
const size_t Rip2Mdv::UNITS_LEN = 24;
const size_t Rip2Mdv::CHRIP_LEN = 64;
const size_t Rip2Mdv::CHRIP_NUM_ELEMENTS = 64;


/*********************************************************************
 * Constructor
 */

Rip2Mdv::Rip2Mdv(int argc, char **argv) :
  _field(0)
{
  static const string method_name = "Rip2Mdv::Rip2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Rip2Mdv *)NULL);
  
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

Rip2Mdv::~Rip2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _field;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Rip2Mdv *Rip2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (Rip2Mdv *)NULL)
    new Rip2Mdv(argc, argv);
  
  return(_instance);
}

Rip2Mdv *Rip2Mdv::Inst()
{
  assert(_instance != (Rip2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Rip2Mdv::init()
{
  static const string method_name = "Rip2Mdv::init()";
  
  // Initialize the trigger object

  _initTrigger();
  
  // Initialize process registration.  Don't register with the
  // process mapper if we are running in an archive mode.

  if (_params->trigger_mode == Params::INPUT_DIR)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void Rip2Mdv::run()
{
  static const string method_name = "Rip2Mdv::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error getting next trigger" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info.getFilePath()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing RIP file: "
	   << trigger_info.getFilePath() << endl;
      
      continue;
    }
    
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool Rip2Mdv::_initTrigger(void)
{
  static const string method_name = "Rip2Mdv::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    if (_params->debug)
      cerr << "Initializing FILE_LIST trigger" << endl;
    
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
    if (_params->debug)
    {
      cerr << "Initializing INPUT_DIR trigger: " << endl;
      cerr << "   dir: " << _params->input_dir_trigger.dir << endl;
      cerr << "   file substring: "
	   << _params->input_dir_trigger.file_substring << endl;
      cerr << "   process old files: ";
      if (_params->input_dir_trigger.process_old_files)
	cerr << "true" << endl;
      else
	cerr << "false" << endl;
    }
    
    
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir_trigger.dir,
		      _params->input_dir_trigger.file_substring,
		      _params->input_dir_trigger.process_old_files,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR trigger: " << endl;
      cerr << "   dir: " << _params->input_dir_trigger.dir << endl;
      cerr << "   file substring: "
	   << _params->input_dir_trigger.file_substring << endl;
      cerr << "   process old files: ";
      if (_params->input_dir_trigger.process_old_files)
	cerr << "true" << endl;
      else
	cerr << "false" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData() - Process the data in the given file.
 *
 * Returns true on success, false on failure.
 */

bool Rip2Mdv::_processData(const string &file_path)
{
  static const string method_name = "Rip2Mdv::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data in file: " << file_path << endl;
  
  string pmu_string = "Processing data in file: " + file_path;
  
  PMU_force_register(pmu_string.c_str());
  
  // Open the file

  FILE *rip_file;
  
  if ((rip_file = fopen(file_path.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening RIP file: " << file_path << endl;
    
    return false;
  }
  
  // Read in the header information

  if (!_readRipHeader(rip_file, file_path))
  {
    fclose(rip_file);
    return false;
  }

  // Read in the data

  if (!_readRipData(rip_file, file_path))
  {
    fclose(rip_file);
    return false;
  }
  
  fclose(rip_file);
  
  // Create and write the output MDV file

  if (!_writeMdvFile(file_path))
    return false;
  
  return true;
}


/*********************************************************************
 * _readRipData() - Read the RIP data from the given file.  Update the
   *                  data in the MDV field with the data values read in.
 *
 * Returns true on success, false on failure.
 */

bool Rip2Mdv::_readRipData(FILE *rip_file, const string &rip_file_path)
{
  static const string method_name = "Rip2Mdv::_readRipData()";
  
  // Read the beginning record len

  si32 record_len;
  
  if (fread(&record_len, sizeof(fl32), 1, rip_file) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading FORTRAN record length from RIP file:" <<
      rip_file_path << endl;
    
    return false;
  }
  
  record_len = BE_from_si32(record_len);
  
  if (_params->debug)
    cerr << "   record len = " << record_len << endl;
  
  // Read the data

  Mdvx::field_header_t field_hdr = _field->getFieldHeader();
  size_t volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;

  fl32 *rip_data = new fl32[volume_size];
  
  if (fread(rip_data, sizeof(fl32), volume_size, rip_file) != volume_size)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading data from RIP file: " << rip_file_path << endl;
    
    return false;
  }
  
  BE_swap_array_32(rip_data, volume_size * sizeof(fl32));
  
  fl32 *mdv_data = (fl32 *)_field->getVol();
  
  for (int x = 0; x < field_hdr.nx; ++x)
  {
    for (int y = 0; y < field_hdr.ny; ++y)
    {
      for (int z = 0; z < field_hdr.nz; ++z)
      {
	int mdv_index = (z * field_hdr.nx * field_hdr.ny) +
	  (y * field_hdr.nx) + x;
	int rip_index = (z * field_hdr.nx * field_hdr.ny) +
	  (x * field_hdr.ny) + y;
	
	mdv_data[mdv_index] = rip_data[rip_index];
      } /* endfor - z */
    } /* endfor - y */
  } /* endfor - x */
  
  // Read the ending record len

  if (fread(&record_len, sizeof(fl32), 1, rip_file) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading FORTRAN record length from RIP file:" <<
      rip_file_path << endl;
    
    return false;
  }
  
  record_len = BE_from_si32(record_len);
  
  if (_params->debug)
    cerr << "   record len = " << record_len << endl;
  
  return true;
}


/*********************************************************************
 * _readRipHeader() - Read the RIP header information from the given
 *                    file.  Create the MDV field based on this information.
 *
 * Returns true on success, false on failure.
 */

bool Rip2Mdv::_readRipHeader(FILE *rip_file, const string &rip_file_path)
{
  static const string method_name = "Rip2Mdv::_readRipHeader()";
  
  // Since we will be creating a new field, begin by deleting any old
  // field information

  delete _field;
  
  // Read the beginning FORTRAN record length value

  si32 record_len;
  
  if (fread(&record_len, sizeof(fl32), 1, rip_file) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading FORTRAN record length from RIP file:" <<
      rip_file_path << endl;
    
    return false;
  }
  
  record_len = BE_from_si32(record_len);
  
  if (_params->debug)
    cerr << "   record len = " << record_len << endl;
  
  // Read the variable description

  char *vardesc = new char[VAR_DESC_LEN+1];
  memset(vardesc, 0, VAR_DESC_LEN+1);
  
  if (fread(vardesc, sizeof(char), VAR_DESC_LEN, rip_file) != VAR_DESC_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading description string from RIP file:" <<
      rip_file_path << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "   vardesc = " << vardesc << endl;
  
  // Read the variable units

  char *units = new char[UNITS_LEN+1];
  memset(units, 0, UNITS_LEN+1);
  
  if (fread(units, sizeof(char), UNITS_LEN, rip_file) != UNITS_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading units string from RIP file:" <<
      rip_file_path << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "   units: " << units << endl;
  
  // Read the integer header information

  ihrip_t ihrip;
  
  if (fread(&ihrip, sizeof(ihrip), 1, rip_file) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading ihrip structure from RIP file:"
	 << rip_file_path << endl;
    
    return false;
  }
  
  BE_swap_array_32(&ihrip, sizeof(ihrip));
  
  if (_params->debug)
  {
    cerr << "   map projection: " << ihrip.map_projection << endl;
    cerr << "   coarse ny: " << ihrip.coarse_ny << endl;
    cerr << "   coarse nx: " << ihrip.coarse_nx << endl;
    cerr << "   ny: " << ihrip.ny << endl;
    cerr << "   nx: " << ihrip.nx << endl;
    cerr << "   dimensions: " << ihrip.dimensions << endl;
    cerr << "   grid domain: " << ihrip.grid_domain << endl;
    cerr << "   unknown: " << ihrip.unknown << endl;
    cerr << "   nz: " << ihrip.nz << endl;
    cerr << "   mdateb: " << ihrip.mdateb << endl;
    cerr << "   mdate: " << ihrip.mdate << endl;
    cerr << "   ice physics type: " << ihrip.ice_physics_type << endl;
    cerr << "   vert coord type: " << ihrip.vert_coord_type << endl;
    cerr << "   landuse dataset: " << ihrip.landuse_dataset << endl;
    cerr << "   spare: " << ihrip.spare[0] << endl;
    cerr << "   spare: " << ihrip.spare[1] << endl;
    cerr << "   spare: " << ihrip.spare[2] << endl;
    cerr << "   spare: " << ihrip.spare[3] << endl;
    cerr << "   spare: " << ihrip.spare[4] << endl;
    cerr << "   spare: " << ihrip.spare[5] << endl;
    cerr << "   spare: " << ihrip.spare[6] << endl;
    cerr << "   spare: " << ihrip.spare[7] << endl;
    cerr << "   spare: " << ihrip.spare[8] << endl;
    cerr << "   spare: " << ihrip.spare[9] << endl;
    cerr << "   spare: " << ihrip.spare[10] << endl;
    cerr << "   spare: " << ihrip.spare[11] << endl;
    cerr << "   spare: " << ihrip.spare[12] << endl;
    cerr << "   spare: " << ihrip.spare[13] << endl;
    cerr << "   spare: " << ihrip.spare[14] << endl;
    cerr << "   spare: " << ihrip.spare[15] << endl;
    cerr << "   spare: " << ihrip.spare[16] << endl;
    cerr << "   spare: " << ihrip.spare[17] << endl;
  }
  
  // Read the real header information

  rhrip_t rhrip;
  
  if (fread(&rhrip, sizeof(rhrip), 1, rip_file) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading rhrip from RIP file: "
	 << rip_file_path << endl;
    
    return false;
  }
  
  BE_swap_array_32(&rhrip, sizeof(rhrip));
  
  if (_params->debug)
  {
    cerr << "   first true lat: " << rhrip.first_true_lat << endl;
    cerr << "   second true lat: " << rhrip.second_true_lat << endl;
    cerr << "   coarse central lat: " << rhrip.coarse_central_lat << endl;
    cerr << "   coarse central lon: " << rhrip.coarse_central_lon << endl;
    cerr << "   coarse dxy: " << rhrip.coarse_dxy_km << " km" << endl;
    cerr << "   dxy: " << rhrip.dxy_km << " km" << endl;
    cerr << "   coarse ll y: " << rhrip.coarse_ll_y << endl;
    cerr << "   coarse ll x: " << rhrip.coarse_ll_x << endl;
    for (int j = 0; j < 4; ++j)
      cerr << "   unknown" << j << ": " << rhrip.unknown[j] << endl;
    cerr << "   rhourb: " << rhrip.rhourb << endl;
    cerr << "   rhour: " << rhrip.rhour << endl;
    cerr << "   xtime: " << rhrip.xtime << endl;
    cerr << "   spare: " << rhrip.spare[0] << endl;
    cerr << "   spare: " << rhrip.spare[1] << endl;
    cerr << "   spare: " << rhrip.spare[2] << endl;
    cerr << "   spare: " << rhrip.spare[3] << endl;
    cerr << "   spare: " << rhrip.spare[4] << endl;
    cerr << "   spare: " << rhrip.spare[5] << endl;
    cerr << "   spare: " << rhrip.spare[6] << endl;
    cerr << "   spare: " << rhrip.spare[7] << endl;
    cerr << "   spare: " << rhrip.spare[8] << endl;
    cerr << "   spare: " << rhrip.spare[9] << endl;
    cerr << "   spare: " << rhrip.spare[10] << endl;
    cerr << "   spare: " << rhrip.spare[11] << endl;
    cerr << "   spare: " << rhrip.spare[12] << endl;
    cerr << "   spare: " << rhrip.spare[13] << endl;
    cerr << "   spare: " << rhrip.spare[14] << endl;
    cerr << "   spare: " << rhrip.spare[15] << endl;
    cerr << "   spare: " << rhrip.spare[16] << endl;
  }
  
  char **chrip = new char*[CHRIP_NUM_ELEMENTS];
  
  for (size_t i = 0; i < CHRIP_NUM_ELEMENTS; ++i)
  {
    chrip[i] = new char[CHRIP_LEN+1];
    memset(chrip[i], 0, CHRIP_LEN+1);
    
    if (fread(chrip[i], sizeof(char), CHRIP_LEN, rip_file) != CHRIP_LEN)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading chrip element " << i << " from RIP file: "
	   << rip_file_path << endl;
      
      return false;
    }
    
    if (_params->debug)
      cerr << "   chrip[" << i << "]: " << chrip[i] << endl;
  } /* endfor - i */
  
  // Read the ending FORTRAN record length value

  if (fread(&record_len, sizeof(si32), 1, rip_file) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading FORTRAN record length from RIP file: "
	 << rip_file_path << endl;
    
    return false;
  }
  
  record_len = BE_from_si32(record_len);
  
  if (_params->debug)
    cerr << "   record len: " << record_len << endl;
  
  time_t gen_time;
  if(_params->use_gen_time) 
  {
    int numtokens;
    int year, month, day, hour;
    int min = 0;
    int sec = 0;

    if((numtokens = sscanf(_params->gen_time, "%4d%2d%2d%2d",
			   &year,&month,&day,&hour)) != 4)
    {
	cerr << "ERROR - gen_time not specified correctly, use YYYYMMDDHH\n" << endl;
	return -1;
    }

    if (_params->debug) 
    {
	cerr << "Year = " << year << endl;
	cerr << "Month = " << month << endl;
	cerr << "day = " << day << endl;
	cerr << "hour = " << hour << endl;
    }
    
    DateTime datetime(year, month, day, hour, min, sec);
    gen_time = datetime.utime();
  }
  
  // Create the MDV field header for the field

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  field_hdr.forecast_delta = (int)(rhrip.xtime * 3600.0);
  field_hdr.forecast_time = _getUtime(ihrip.mdateb) + field_hdr.forecast_delta;

  if(_params->use_gen_time) 
  {
      field_hdr.forecast_delta =  field_hdr.forecast_time - gen_time;
  }

  field_hdr.nx = ihrip.nx;
  field_hdr.ny = ihrip.ny;
  if (ihrip.dimensions == 2)
    field_hdr.nz = 1;
  else
    field_hdr.nz = ihrip.nz;
  
  switch(ihrip.map_projection)
  {
  case 1 :
    field_hdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    field_hdr.proj_param[0] = rhrip.first_true_lat;
    field_hdr.proj_param[1] = rhrip.second_true_lat;
    break;
    
  case 2:
    field_hdr.proj_type = Mdvx::PROJ_POLAR_STEREO;
    break;
    
  case 3:
    field_hdr.proj_type = Mdvx::PROJ_MERCATOR;
    break;
    
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Unknown map projection type in RIP file: "
	 << ihrip.map_projection << endl;
    return false;
  }
  
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny *
    field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  if (ihrip.vert_coord_type <= 3)
  {
    field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
    field_hdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  }
  else
  {
    field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  }
  field_hdr.dz_constant = 1;
  field_hdr.data_dimension = ihrip.dimensions;
  field_hdr.proj_origin_lat = rhrip.coarse_central_lat;
  field_hdr.proj_origin_lon = rhrip.coarse_central_lon;
  field_hdr.grid_dx = rhrip.dxy_km;
  field_hdr.grid_dy = rhrip.dxy_km;
  field_hdr.grid_dz = 1.0;
  field_hdr.grid_minx = ((rhrip.coarse_ll_x - ((double)ihrip.coarse_nx / 2.0))
    * rhrip.coarse_dxy_km) - (rhrip.coarse_dxy_km / 2.0);
  field_hdr.grid_miny = ((rhrip.coarse_ll_y - ((double)ihrip.coarse_ny / 2.0))
    * rhrip.coarse_dxy_km) - (rhrip.coarse_dxy_km / 2.0);
  field_hdr.grid_minz = 0.5;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = -9999.0;
  field_hdr.missing_data_value = -9999.0;
  STRcopy(field_hdr.field_name_long, vardesc, MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, vardesc, MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units, MDV_UNITS_LEN);
  
  // Create the MDV vlevel header for the field

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = field_hdr.vlevel_type;
  vlevel_hdr.level[0] = 0.5;
  
  // Create the blank field

  if ((_field = new MdvxField(field_hdr, vlevel_hdr, (void *)0,
			      true, false)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating MDV field" << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _writeMdvFile() - Write the MDV file.
 *
 * Returns true on success, false on failure.
 */

bool Rip2Mdv::_writeMdvFile(const string &rip_file_path)
{
  static const string method_name = "Rip2Mdv::_writeMdvFile()";

  // Create the master header

  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  Mdvx::field_header_t field_hdr = _field->getFieldHeader();

  master_hdr.time_gen = field_hdr.forecast_time - field_hdr.forecast_delta;
  master_hdr.time_begin = field_hdr.forecast_time;
  master_hdr.time_end = master_hdr.time_begin;
  master_hdr.time_centroid = master_hdr.time_begin;
  master_hdr.time_expire = master_hdr.time_begin;
  master_hdr.data_dimension = field_hdr.data_dimension;
  master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  master_hdr.native_vlevel_type = field_hdr.native_vlevel_type;
  master_hdr.vlevel_type = field_hdr.vlevel_type;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info, "Created by Rip2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "Rip2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, rip_file_path.c_str(), MDV_NAME_LEN);
  
  if((field_hdr.forecast_time - master_hdr.time_gen) < 0) 
  {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Negative forecast_delta detected. Not writing file" << endl;

      return false;
  }
  

  // Create the MDV file.  After adding the field to the file, set the
  // global field pointer to 0 since we have just given control of this
  // field pointer to the DsMdvx object.

  DsMdvx mdv_file;
  
  mdv_file.setMasterHeader(master_hdr);
//  mdv_file.setFieldHeader(field_hdr);
  
  mdv_file.addField(_field);
  _field = 0;
  
  mdv_file.setWriteAsForecast();
  mdv_file.setWriteLdataInfo();
  
  if (mdv_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing MDV file to URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
