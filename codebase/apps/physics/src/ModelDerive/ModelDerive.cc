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
/* RCS info
 *   $Author: dixon $
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.4 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ModelDerive: ModelDerive program object. Handles finding input data
 * and passing it on to the file handler.
 *
 * ModelDerive is a program designed to read a gridded data file, 
 * derive user requested variables and output the derived grids with 
 * vertical interpolation if requested.
 *
 * Designed to be easily extendable. Derived variable functions are
 * simple to add as are vertical interpolation functions. Additional
 * input/output file handlers also can be created to handle multiple
 * file formats. 
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "ModelDerive.hh"
#include "Params.hh"

using namespace std;


// Global variables
ModelDerive *ModelDerive::_instance = (ModelDerive *)NULL;

/*********************************************************************
 * Constructor
 */
ModelDerive::ModelDerive(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "ModelDerive::ModelDerive()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (ModelDerive *)NULL);

  // Initialize the okay flag.

  okay = true;

  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  if(progname_parts.dir != NULL)
    free(progname_parts.dir);
  if(progname_parts.name != NULL)
    free(progname_parts.name);
  if(progname_parts.base != NULL)
    free(progname_parts.base);
  if(progname_parts.ext != NULL)
    free(progname_parts.ext);

  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  // Get TDRP parameters.

  _params = new Params();
  char *params_path = new char[200];
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

  return;

}


/*********************************************************************
 * Destructor
 */
ModelDerive::~ModelDerive()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;

  //delete _dataTrigger;

  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */
ModelDerive *ModelDerive::Inst(int argc, char **argv)
{
  if (_instance == (ModelDerive *)NULL)
    new ModelDerive(argc, argv);

  return(_instance);
}

ModelDerive *ModelDerive::Inst()
{
  assert(_instance != (ModelDerive *)NULL);

  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */
bool ModelDerive::init()
{
  static const string method_name = "ModelDerive::init()";

  // Initialize the data trigger
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    cout << "Starting up in Realtime Mode." << endl;

    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger with path: " <<
        _params->input_path << endl;

    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_path,
                      -1,
                      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger with path: " <<
        _params->input_path << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::TIME_LIST :
  {
    time_t start_time = DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time = DateTime::parseDateTime(_params->time_list_trigger.end_time);

    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
        _params->time_list_trigger.start_time << endl;

      return false;
    }

    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
        _params->time_list_trigger.end_time << endl;

      return false;
    }

    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_path,
                      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger with path: " <<
        _params->input_path << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
        endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }

  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getInputFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger." << endl;

      return false;
    }

    _dataTrigger = trigger;

    break;
  }
  } /* endswitch - _params->trigger_mode */

  // Make sure that the output directory exists
  if (ta_makedir_recurse(_params->output_path) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output directory: " << _params->output_path << endl;

    return false;
  }

  // initialize process registration
  PMU_auto_init(_progName, _params->instance,
                PROCMAP_REGISTER_INTERVAL);

  // Initialize the input handler object
  if(_params->input_type == Params::INPUT_MDV)
    _fileTypeHandler = new ModelDeriveMdv(_params);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */
int ModelDerive::run()
{
  static const string method_name = "ModelDerive::run()";

  TriggerInfo trigger_info;

  int retVal = 0;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      retVal = -1;

      continue;
    }


    if (_params->debug)
      cerr << endl << "*** Processing data for time: "
	   << DateTime(trigger_info.getIssueTime()) << endl;


    if(!_fileTypeHandler->processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: "
           << DateTime(trigger_info.getIssueTime()) << endl;
      retVal = -1;

      continue;
    }

  } /* endwhile - !_dataTrigger->endOfData() */

  return retVal;

}
