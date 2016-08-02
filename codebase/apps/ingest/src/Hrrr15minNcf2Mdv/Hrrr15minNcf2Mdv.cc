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
 * @file Hrrr15minNcf2Mdv.cc
 *
 * @class Hrrr15minNcf2Mdv
 *
 * Hrrr15minNcf2Mdv program object.
 *  
 * @date 3/30/2009
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
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Hrrr15minNcf2Mdv.hh"
#include "Params.hh"

using namespace std;

// Global variables

Hrrr15minNcf2Mdv *Hrrr15minNcf2Mdv::_instance =
     (Hrrr15minNcf2Mdv *)NULL;


/*********************************************************************
 * Constructors
 */

Hrrr15minNcf2Mdv::Hrrr15minNcf2Mdv(int argc, char **argv) :
  _dataTrigger(0),
  _outputCompressionType(Mdvx::COMPRESSION_BZIP),
  _outputScalingType(Mdvx::SCALING_DYNAMIC)
{
  static const string method_name = "Hrrr15minNcf2Mdv::Hrrr15minNcf2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Hrrr15minNcf2Mdv *)NULL);
  
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

Hrrr15minNcf2Mdv::~Hrrr15minNcf2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  vector< NetcdfField* >::iterator field_iter;
  for (field_iter = _fieldList.begin(); field_iter != _fieldList.end();
       ++field_iter)
    delete *field_iter;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

Hrrr15minNcf2Mdv *Hrrr15minNcf2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (Hrrr15minNcf2Mdv *)NULL)
    new Hrrr15minNcf2Mdv(argc, argv);
  
  return(_instance);
}

Hrrr15minNcf2Mdv *Hrrr15minNcf2Mdv::Inst()
{
  assert(_instance != (Hrrr15minNcf2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool Hrrr15minNcf2Mdv::init()
{
  static const string method_name = "Hrrr15minNcf2Mdv::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
 
  // Initialize the field list

  if (!_initFieldList())
    return false;
  
  // Initialize the output compression and scaling types

  if (!_initOutputCompressionType())
    return false;
  
  if (!_initOutputScalingType())
    return false;
  
  // Initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  1500);

  return true;
}


/*********************************************************************
 * run()
 */

void Hrrr15minNcf2Mdv::run()
{
  static const string method_name = "Hrrr15minNcf2Mdv::run()";
  
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
 * _initFieldList()
 */

bool Hrrr15minNcf2Mdv::_initFieldList()
{
  static const string method_name = "Hrrr15minNcf2Mdv;:_initFieldList()";
  
  // Create the remap projection object

  MdvxProj remap_proj;
  
  switch (_params->remap_proj.proj_type)
  {
  case Params::PROJ_LATLON :
    remap_proj.initLatlon();
    break;
    
  case Params::PROJ_FLAT :
    remap_proj.initFlat(_params->remap_proj.origin_lat,
			_params->remap_proj.origin_lon,
			_params->remap_proj.rotation);
    break;
    
  case Params::PROJ_LC2 :
    remap_proj.initLc2(_params->remap_proj.origin_lat,
		       _params->remap_proj.origin_lon,
		       _params->remap_proj.lat1,
		       _params->remap_proj.lat2);
    break;
  } /* endswitch - _params->remap_proj.proj_type */
  
  remap_proj.setGrid(_params->remap_proj.nx, _params->remap_proj.ny,
		     _params->remap_proj.dx, _params->remap_proj.dy,
		     _params->remap_proj.minx, _params->remap_proj.miny);
  
  // Create the fields and put them in the list

  for (int i = 0; i < _params->output_fields_n; ++i)
  {
    NetcdfField *field =
      new NetcdfField(_params->_output_fields[i].nc_field_name,
		      _params->_output_fields[i].nc_missing_data_value,
		      _params->_output_fields[i].transform_data,
		      _params->_output_fields[i].transform_multiplier,
		      _params->_output_fields[i].transform_constant,
		      _params->_output_fields[i].transform_units,
		      _params->_output_fields[i].replace_data,
		      _params->_output_fields[i].replace_nc_value,
		      _params->_output_fields[i].replace_mdv_value,
		      _params->debug,
		      _params->verbose);

    if (_params->remap_output)
      field->setRemapOutput(remap_proj);
    
    _fieldList.push_back(field);
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _initOutputCompressionType()
 */

bool Hrrr15minNcf2Mdv::_initOutputCompressionType()
{
  static const string method_name = "Hrrr15minNcf2Mdv;:_initOutputCompressionType()";
  
  switch (_params->output_compression_type)
  {
  case Params::COMPRESSION_NONE :
    _outputCompressionType = Mdvx::COMPRESSION_NONE;
    break;
    
  case Params::COMPRESSION_RLE :
    _outputCompressionType = Mdvx::COMPRESSION_RLE;
    break;
    
  case Params::COMPRESSION_LZO :
    _outputCompressionType = Mdvx::COMPRESSION_LZO;
    break;
    
  case Params::COMPRESSION_ZLIB :
    _outputCompressionType = Mdvx::COMPRESSION_ZLIB;
    break;
    
  case Params::COMPRESSION_BZIP :
    _outputCompressionType = Mdvx::COMPRESSION_BZIP;
    break;
    
  case Params::COMPRESSION_GZIP :
    _outputCompressionType = Mdvx::COMPRESSION_GZIP;
    break;
    
  case Params::COMPRESSION_GZIP_VOL :
    _outputCompressionType = Mdvx::COMPRESSION_GZIP_VOL;
    break;
    
  } /* endswitch - _params->output_compression_type */

  return true;
}

    
/*********************************************************************
 * _initOutputScalingType()
 */

bool Hrrr15minNcf2Mdv::_initOutputScalingType()
{
  static const string method_name = "Hrrr15minNcf2Mdv;:_initOutputScalingType()";
  
  switch (_params->output_scaling_info.type)
  {
  case Params::SCALING_NONE :
    _outputScalingType = Mdvx::SCALING_NONE;
    break;
    
  case Params::SCALING_ROUNDED :
    _outputScalingType = Mdvx::SCALING_ROUNDED;
    break;
    
  case Params::SCALING_INTEGRAL :
    _outputScalingType = Mdvx::SCALING_INTEGRAL;
    break;
    
  case Params::SCALING_DYNAMIC :
    _outputScalingType = Mdvx::SCALING_DYNAMIC;
    break;
    
  case Params::SCALING_SPECIFIED :
    _outputScalingType = Mdvx::SCALING_SPECIFIED;
    break;
    
  } /* endswitch - _params->output_scaling_info.type */

  return true;
}

    
/*********************************************************************
 * _initTrigger()
 */

bool Hrrr15minNcf2Mdv::_initTrigger()
{
  static const string method_name = "Hrrr15minNcf2Mdv;:_initTrigger()";
  
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

bool Hrrr15minNcf2Mdv::_processFile(const string &input_file_path)
{
  static const string method_name = "Hrrr15minNcf2Mdv::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << endl << "*** Processing file: " << input_file_path << endl;
  

  //
  // Get the start and end lead times from the filename
  // The filenaming convention for a file with all forecasts 
  // lead times in one file is:YYDDDHH00FFFF (YY=model init year, 
  //                                          DDD=model init julian day, 
  //                                          HH00=model init hour,
  //                                          FFFF=model forecast hours) 
  // 
  // The filenaming convention for a file with a subset of 
  // lead times in one file is: yydddgggglljj (yy - year
  //                                           ddd - day of year
  //                                           gggg - gentime (hhmm)
  //                                           ll - start leadtime
  //                                           jj - end leadtime
  //

  
  int strLen = (int) input_file_path.size();
  
  char hoursStr[3];
  
  sprintf(hoursStr, "%c%c", input_file_path[strLen -4],input_file_path[strLen - 3]); 
  
  int hours = atoi(hoursStr);

  int start_lead_time;


  if (_params->all_forecasts_in_file )
    start_lead_time = 0;
  else
    start_lead_time = hours * 3600;
  

  // Create and initialize the netCDF file object

  NetcdfFile nc_file(input_file_path, _params->forecast_interval,
                     start_lead_time,
		     _outputCompressionType,
		     _outputScalingType,
		     _params->output_scaling_info.scale,
		     _params->output_scaling_info.bias,
		     _params->debug, _params->verbose);
  
  if (!nc_file.init())
    return false;
  
  vector< NetcdfField* >::iterator field_iter;
  for (field_iter = _fieldList.begin(); field_iter != _fieldList.end();
       ++field_iter)
  {
    nc_file.addField(*field_iter);
  } /* endfor - field_iter */
  
  // Create the output files

  if (!nc_file.createMdvFiles(_params->output_url))
    return false;
  
  return true;
}
