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
 * @file CimmsSeviriNc2Mdv.cc
 *
 * @class CimmsSeviriNc2Mdv
 *
 * CimmsSeviriNc2Mdv program object.
 *  
 * @date 11/13/2008
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
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "CimmsSeviriNc2Mdv.hh"
#include "Params.hh"

#include "LatestGridHandler.hh"
#include "MeanGridHandler.hh"
#include "NearestGridHandler.hh"

using namespace std;

// Global variables

CimmsSeviriNc2Mdv *CimmsSeviriNc2Mdv::_instance =
     (CimmsSeviriNc2Mdv *)NULL;


/*********************************************************************
 * Constructors
 */

CimmsSeviriNc2Mdv::CimmsSeviriNc2Mdv(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "CimmsSeviriNc2Mdv::CimmsSeviriNc2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CimmsSeviriNc2Mdv *)NULL);
  
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

CimmsSeviriNc2Mdv::~CimmsSeviriNc2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _gridHandler;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

CimmsSeviriNc2Mdv *CimmsSeviriNc2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (CimmsSeviriNc2Mdv *)NULL)
    new CimmsSeviriNc2Mdv(argc, argv);
  
  return(_instance);
}

CimmsSeviriNc2Mdv *CimmsSeviriNc2Mdv::Inst()
{
  assert(_instance != (CimmsSeviriNc2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool CimmsSeviriNc2Mdv::init()
{
  static const string method_name = "CimmsSeviriNc2Mdv::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
 
  // Initalize the output projection

  if (!_initOutputProjection())
    return false;
  
  // Initialize the data grid handler.  Note that this must be done after
  // initailizing the output projection.

  if (!_initGridHandler())
    return false;
  
  // Initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void CimmsSeviriNc2Mdv::run()
{
  static const string method_name = "CimmsSeviriNc2Mdv::run()";
  
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
 * _initGridHandler()
 */

bool CimmsSeviriNc2Mdv::_initGridHandler()
{
  static const string method_name = "CimmsSeviriNc2Mdv;:_initGridHandler()";
  
  switch (_params->mapping_type)
  {
  case Params::MAP_LATEST :
    _gridHandler = new LatestGridHandler(_outputProj, _params->debug);
    break;
    
  case Params::MAP_NEAREST_NEIGHBOR :
    _gridHandler = new NearestGridHandler(_params->radius_of_influence,
					  _outputProj, _params->debug);
    break;
    
  case Params::MAP_MEAN :
    _gridHandler = new MeanGridHandler(_params->radius_of_influence,
				       _outputProj, _params->debug);
    break;
  } /* endswitch - _params->mapping_type */

  return true;
}


/*********************************************************************
 * _initOutputProjection()
 */

bool CimmsSeviriNc2Mdv::_initOutputProjection()
{
  switch (_params->output_proj.proj_type)
  {
  case Params::PROJ_FLAT :
    _outputProj.initFlat(_params->output_proj.origin_lat,
			 _params->output_proj.origin_lon,
			 _params->output_proj.rotation,
			 _params->output_proj.nx,
			 _params->output_proj.ny,
			 1,
			 _params->output_proj.dx,
			 _params->output_proj.dy,
			 1.0,
			 _params->output_proj.minx,
			 _params->output_proj.miny,
			 0.0);
    break;
    
  case Params::PROJ_LATLON :
    _outputProj.initLatlon(_params->output_proj.nx,
			   _params->output_proj.ny,
			   1,
			   _params->output_proj.dx,
			   _params->output_proj.dy,
			   1.0,
			   _params->output_proj.minx,
			   _params->output_proj.miny,
			   0.0);
    break;
    
  case Params::PROJ_LC2 :
    _outputProj.initLc2(_params->output_proj.origin_lat,
			_params->output_proj.origin_lon,
			_params->output_proj.lat1,
			_params->output_proj.lat2,
			_params->output_proj.nx,
			_params->output_proj.ny,
			1,
			_params->output_proj.dx,
			_params->output_proj.dy,
			1.0,
			_params->output_proj.minx,
			_params->output_proj.miny,
			0.0);
    break;
  }
  
  return true;
  
}


/*********************************************************************
 * _initTrigger()
 */

bool CimmsSeviriNc2Mdv::_initTrigger()
{
  static const string method_name = "CimmsSeviriNc2Mdv;:_initTrigger()";
  
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
 * _processFile()
 */

bool CimmsSeviriNc2Mdv::_processFile(const string &input_file_path)
{
  static const string method_name = "CimmsSeviriNc2Mdv::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << endl << "*** Processing file: " << input_file_path << endl;
  
  // Create the netCDF file object

  NetcdfFile nc_file(input_file_path, _params->debug, _params->verbose);
  
  if (!nc_file.init())
    return false;
  
  // Create the output file

  DsMdvx mdvx;
  
  // Process the data

  if (!_updateMasterHeader(mdvx, nc_file, input_file_path))
    return false;
  
  vector< MdvxField* > fields;
  
  if (!nc_file.getBandsAsMdv(fields, _outputProj, *_gridHandler,
			     _params->output_grib_code))
    return false;
  
  // Compress the data and add the new field to the MDV file

  vector< MdvxField* >::iterator field;
  
  for (field = fields.begin(); field != fields.end(); ++field)
  {
    (*field)->convertType(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_BZIP,
			  Mdvx::SCALING_DYNAMIC);
  
    mdvx.addField(*field);
  } /* endfor - field */
  
  // Write the output file

  if (_params->debug)
    cerr << "Writing file to " << _params->output_url << endl;
  
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
 * _updateMasterHeader()
 */

bool CimmsSeviriNc2Mdv::_updateMasterHeader(Mdvx &mdvx,
					    const NetcdfFile &nc_file,
					    const string &input_file_path) const
{
  static const string method_name = "CimmsSeviriNc2Mdv::_updateMasterHeader()";
  
  // Get the file time

  DateTime file_time = nc_file.getImageTime();
  if (file_time == DateTime::NEVER)
    return false;
  
  if (_params->debug)
    cerr << "    File time: " << file_time.dtime() << endl;
  
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
  master_hdr.sensor_lat = 0.0;
  master_hdr.sensor_lon = 0.0;
  STRcopy(master_hdr.data_set_info, "CimmsSeviriNc2Mdv", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "CimmsSeviriNc2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, input_file_path.c_str(),
	  MDV_NAME_LEN);
  
  mdvx.setMasterHeader(master_hdr);

  return true;
}
