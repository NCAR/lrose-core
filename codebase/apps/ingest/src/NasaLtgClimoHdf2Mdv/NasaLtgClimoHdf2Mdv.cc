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
 * @file NasaLtgClimoHdf2Mdv.cc
 *
 * @class NasaLtgClimoHdf2Mdv
 *
 * NasaLtgClimoHdf2Mdv program object.
 *  
 * @date 10/30/2008
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

#include "Params.hh"
#include "HdfFile.hh"
#include "NasaLtgClimoHdf2Mdv.hh"

#include "MonthlySdsField.hh"

using namespace std;

// Global variables

NasaLtgClimoHdf2Mdv *NasaLtgClimoHdf2Mdv::_instance =
     (NasaLtgClimoHdf2Mdv *)NULL;


/*********************************************************************
 * Constructors
 */

NasaLtgClimoHdf2Mdv::NasaLtgClimoHdf2Mdv(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "NasaLtgClimoHdf2Mdv::NasaLtgClimoHdf2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (NasaLtgClimoHdf2Mdv *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = progname_parts.base;
  
  // Display ucopyright message.

  ucopyright(_progName.c_str());

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

NasaLtgClimoHdf2Mdv::~NasaLtgClimoHdf2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
}


/*********************************************************************
 * Inst()
 */

NasaLtgClimoHdf2Mdv *NasaLtgClimoHdf2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (NasaLtgClimoHdf2Mdv *)NULL)
    new NasaLtgClimoHdf2Mdv(argc, argv);
  
  return(_instance);
}

NasaLtgClimoHdf2Mdv *NasaLtgClimoHdf2Mdv::Inst()
{
  assert(_instance != (NasaLtgClimoHdf2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool NasaLtgClimoHdf2Mdv::init()
{
  static const string method_name = "NasaLtgClimoHdf2Mdv::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the input handlers

  if (!_initInputHandlers())
    return false;
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName.c_str(), _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void NasaLtgClimoHdf2Mdv::run()
{
  static const string method_name = "NasaLtgClimoHdf2Mdv::run()";
  
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
 * _initInputHandlers()
 */

bool NasaLtgClimoHdf2Mdv::_initInputHandlers()
{
  static const string method_name = "NasaLtgClimoHdf2Mdv;:_initInput Handlers()";
  
  for (int i = 0; i < _params->input_fields_n; ++i)
  {
    switch (_params->_input_fields[i].climo_field_type)
    {
    case Params::MONTHLY_CLIMO_FIELD :
    {
      MonthlySdsField *input_handler =
	new MonthlySdsField(_params->_input_fields[i].sds_field_name,
			    _progName,
			    _params->_input_fields[i].output_url,
			    _params->debug, _params->verbose);
      _inputHandlers.push_back(input_handler);
      break;
    }

    } /* endswitch - _params->_input_fields[i].climo_field_type */
    
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * _initTrigger()
 */

bool NasaLtgClimoHdf2Mdv::_initTrigger()
{
  static const string method_name = "NasaLtgClimoHdf2Mdv;:_initTrigger()";
  
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

bool NasaLtgClimoHdf2Mdv::_processFile(const string &input_file_name)
{
  static const string method_name = "NasaLtgClimoHdf2Mdv::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << "*** Processing file: " << input_file_name << endl;
  
  // Create the HDF file object

  HdfFile hdf_file(input_file_name, _params->debug);
  
  if (!hdf_file.init())
    return false;
  
  // Process the specified input fields

  vector< SdsDataField* >::iterator input_handler_iter;
  
  for (input_handler_iter = _inputHandlers.begin();
       input_handler_iter != _inputHandlers.end(); ++input_handler_iter)
  {
    SdsDataField *input_handler = *input_handler_iter;
    
    input_handler->createMdvFiles(hdf_file);
    
  } /* endfor - input_handler_iter */
  
  if (_params->debug)
    cerr << endl << endl;
  
  return true;
}
