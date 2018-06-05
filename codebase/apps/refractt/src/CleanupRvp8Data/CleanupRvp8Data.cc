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
//   $Date: 2018/01/26 20:39:39 $
//   $Id: CleanupRvp8Data.cc,v 1.5 2018/01/26 20:39:39 jcraig Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CleanupRvp8Data: CleanupRvp8Data program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <map>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "CleanupRvp8Data.hh"
#include "NegateVar.hh"
#include "Params.hh"
#include "SweepFile.hh"

using namespace std;

// Global variables

CleanupRvp8Data *CleanupRvp8Data::_instance =
     (CleanupRvp8Data *)NULL;


/*********************************************************************
 * Constructors
 */

CleanupRvp8Data::CleanupRvp8Data(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "CleanupRvp8Data::CleanupRvp8Data()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CleanupRvp8Data *)NULL);
  
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

CleanupRvp8Data::~CleanupRvp8Data()
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

CleanupRvp8Data *CleanupRvp8Data::Inst(int argc, char **argv)
{
  if (_instance == (CleanupRvp8Data *)NULL)
    new CleanupRvp8Data(argc, argv);
  
  return(_instance);
}

CleanupRvp8Data *CleanupRvp8Data::Inst()
{
  assert(_instance != (CleanupRvp8Data *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool CleanupRvp8Data::init()
{
  static const string method_name = "CleanupRvp8Data::init()";
  
  // Initialize the data trigger

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
		      false,
		      PMU_auto_register) != 0)
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
  
  // Create the output directory

  if (ta_makedir_recurse(_params->output_dir) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: " << _params->output_dir << endl;
    return false;
  }
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void CleanupRvp8Data::run()
{
  static const string method_name = "CleanupRvp8Data::run()";
  
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

bool CleanupRvp8Data::_processFile(const string &input_file)
{
  static const string method_name = "CleanupRvp8Data::_processFile()";
  
  PMU_auto_register("Processing file...");

  // Construct the sweep file path.  If the given input file doesn't
  // exist, try prepending the input_dir specified in the parameter
  // file.

  struct stat file_stat;
  
  string sweep_file_path = input_file;
  
  if (stat(sweep_file_path.c_str(), &file_stat) != 0)
    sweep_file_path = string(_params->input_dir) + "/" + input_file;
  
  if (stat(sweep_file_path.c_str(), &file_stat) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Specified sweep file doesn't exist: " << input_file << endl;
    
    return false;
  }

  // Create the list of variables to negate

  map< string, NegateVar > negate_var_list;
  
  for (int i = 0; i < _params->negate_var_list_n; ++i)
    negate_var_list[_params->_negate_var_list[i].variable_name] =
      NegateVar(_params->_negate_var_list[i].variable_name,
		_params->_negate_var_list[i].missing_value_attr_name);
  
  // Process the sweep file

  if (_params->debug)
  {
    cerr << "\n*** Processing sweep file: " << sweep_file_path << endl;
    cerr << "   sweep_file_path = " << sweep_file_path << endl;
    cerr << "   debug = " << _params->debug << endl;
  }

  SweepFile sweep_file(sweep_file_path,
		       negate_var_list,
		       _params->num_begin_delete_gates,
		       _params->gates_dim_name,
		       _params->debug);
  
  if (!sweep_file.initialize())
    return false;

  Path input_path(sweep_file_path);
  
  if (!sweep_file.writeCleanFile(_params->output_dir, input_path.getFile()))
    return false;
  
  return true;
}
