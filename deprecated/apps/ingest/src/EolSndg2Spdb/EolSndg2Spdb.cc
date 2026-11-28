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
 * @file EolSndg2Spdb.cc
 *
 * @class EolSndg2Spdb
 *
 * EolSndg2Spdb program object.
 *  
 * @date 9/28/2009
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "EolSndg2Spdb.hh"
#include "FileHandler.hh"

using namespace std;

// Global variables

EolSndg2Spdb *EolSndg2Spdb::_instance =
     (EolSndg2Spdb *)NULL;


/*********************************************************************
 * Constructors
 */

EolSndg2Spdb::EolSndg2Spdb(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "EolSndg2Spdb::EolSndg2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (EolSndg2Spdb *)NULL);
  
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

EolSndg2Spdb::~EolSndg2Spdb()
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
 * Inst()
 */

EolSndg2Spdb *EolSndg2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (EolSndg2Spdb *)NULL)
    new EolSndg2Spdb(argc, argv);
  
  return(_instance);
}

EolSndg2Spdb *EolSndg2Spdb::Inst()
{
  assert(_instance != (EolSndg2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool EolSndg2Spdb::init()
{
  static const string method_name = "EolSndg2Spdb::init()";
  
  // If we are in verbose mode, also set the debug flag

  if (_params->verbose)
    _params->debug = pTRUE;
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void EolSndg2Spdb::run()
{
  static const string method_name = "EolSndg2Spdb::run()";
  
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
 * _initTrigger()
 */

bool EolSndg2Spdb::_initTrigger()
{
  static const string method_name = "EolSndg2Spdb;:_initTrigger()";
  
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

bool EolSndg2Spdb::_processFile(const string &file_path)
{
  static const string method_name = "EolSndg2Spdb::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << "*** Processing file: " << file_path << endl;
  
  // Process the file

  FileHandler file_handler(file_path, _params->debug, _params->verbose);
  
  if (!file_handler.init())
    return false;
  
  if (!file_handler.processFile())
    return false;
  
  // Get the soundings and write them to the output database

  DsSpdb spdb;
  
  spdb.setPutMode(Spdb::putModeAdd);
  
  vector< Sndg > &soundings = file_handler.getSoundings();
  vector< Sndg >::const_iterator sounding;
  
  for (sounding = soundings.begin(); sounding != soundings.end(); ++sounding)
  {
    Sndg output_sounding = *sounding;
    
    if (_params->verbose)
      output_sounding.print(cerr);
    else if (_params->debug)
      output_sounding.print_header(cerr);
    
    output_sounding.assemble();
    
    spdb.addPutChunk(0,
		     output_sounding.getHeader().launchTime,
		     output_sounding.getHeader().launchTime +
		       _params->expire_secs,
		     output_sounding.getBufLen(),
		     output_sounding.getBufPtr());
    
  } /* endfor - sounding */
  
  if (spdb.put(_params->output_url,
	       SPDB_SNDG_PLUS_ID, SPDB_SNDG_PLUS_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing soundings to database" << endl;
    cerr << spdb.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << endl << endl;
  
  return true;
}
