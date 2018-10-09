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
//   $Date: 2018/01/26 20:06:07 $
//   $Id: TamdarNcSndg2Spdb.cc,v 1.5 2018/01/26 20:06:07 jcraig Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TamdarNcSndg2Spdb: TamdarNcSndg2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2005
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
#include <dsdata/DsLdataTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "TamdarNcSndg2Spdb.hh"
#include "Params.hh"

using namespace std;

// Global variables

TamdarNcSndg2Spdb *TamdarNcSndg2Spdb::_instance =
     (TamdarNcSndg2Spdb *)NULL;


/*********************************************************************
 * Constructors
 */

TamdarNcSndg2Spdb::TamdarNcSndg2Spdb(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "TamdarNcSndg2Spdb::TamdarNcSndg2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TamdarNcSndg2Spdb *)NULL);
  
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
  char *params_path = "unknown";
  
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

TamdarNcSndg2Spdb::~TamdarNcSndg2Spdb()
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

TamdarNcSndg2Spdb *TamdarNcSndg2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (TamdarNcSndg2Spdb *)NULL)
    new TamdarNcSndg2Spdb(argc, argv);
  
  return(_instance);
}

TamdarNcSndg2Spdb *TamdarNcSndg2Spdb::Inst()
{
  assert(_instance != (TamdarNcSndg2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TamdarNcSndg2Spdb::init()
{
  static const string method_name = "TamdarNcSndg2Spdb::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      600, PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
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
  
  } /* endswitch - _params->trigger_mode */
  
  // Initialize the TAMDAR file object

  if (!_tamdarFile.initialize(_params->num_recs_dim_name,
			      _params->tail_len_dim_name,
			      _params->missing_data_value_att_name,
			      _params->latitude_var_name,
			      _params->longitude_var_name,
			      _params->altitude_var_name,
			      _params->temperature_var_name,
			      _params->wind_dir_var_name,
			      _params->wind_speed_var_name,
			      _params->rel_hum_var_name,
			      _params->dew_point_var_name,
			      _params->tail_number_var_name,
			      _params->data_source_var_name,
			      _params->sounding_flag_var_name,
			      _params->launch_times_var_name,
			      _params->debug))
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

void TamdarNcSndg2Spdb::run()
{
  static const string method_name = "TamdarNcSndg2Spdb::run()";
  
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
 * _processFile() - Process the given file.
 */

bool TamdarNcSndg2Spdb::_processFile(const string &input_file)
{
  static const string method_name = "TamdarNcSndg2Spdb::_processFile()";
  
  // Construct the sweep file path.  If the given input file doesn't
  // exist, try prepending the input_dir specified in the parameter
  // file.

  struct stat file_stat;
  
  string tamdar_file_path = input_file;
  
  if (stat(tamdar_file_path.c_str(), &file_stat) != 0)
    tamdar_file_path = string(_params->input_dir) + "/" + input_file;
  
  if (stat(tamdar_file_path.c_str(), &file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Specified tamdar file doesn't exist: " << input_file << endl;
    
    return false;
  }

  // Make sure the file is uncompressed

  char uncompressed_path[BUFSIZ];
  STRcopy(uncompressed_path, tamdar_file_path.c_str(), BUFSIZ);
  
  if (ta_file_uncompress(uncompressed_path) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error uncompressing file: " << tamdar_file_path << endl;
    
    return false;
  }
  
  // Process the sweep file

  if (_params->debug)
    cerr << "\n*** Processing TAMDAR file: " << uncompressed_path << endl;
  
  if (!_tamdarFile.initializeFile(uncompressed_path))
    return false;
  
  if (!_tamdarFile.writeAsSpdb(_params->output_url,
			       _params->sort_points_on_output))
    return false;
  
  return true;
}
