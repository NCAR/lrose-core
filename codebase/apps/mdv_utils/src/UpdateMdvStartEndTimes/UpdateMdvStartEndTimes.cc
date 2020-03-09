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
 * @file UpdateMdvStartEndTimes.cc
 *
 * @class UpdateMdvStartEndTimes
 *
 * UpdateMdvStartEndTimes is the top level application class.
 *  
 * @date 10/7/2002
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "UpdateMdvStartEndTimes.hh"
#include "Params.hh"

using namespace std;

// Global variables

UpdateMdvStartEndTimes *UpdateMdvStartEndTimes::_instance =
     (UpdateMdvStartEndTimes *)NULL;



/*********************************************************************
 * Constructor
 */

UpdateMdvStartEndTimes::UpdateMdvStartEndTimes(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "UpdateMdvStartEndTimes::UpdateMdvStartEndTimes()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (UpdateMdvStartEndTimes *)NULL);
  
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

UpdateMdvStartEndTimes::~UpdateMdvStartEndTimes()
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

UpdateMdvStartEndTimes *UpdateMdvStartEndTimes::Inst(int argc, char **argv)
{
  if (_instance == (UpdateMdvStartEndTimes *)NULL)
    new UpdateMdvStartEndTimes(argc, argv);
  
  return(_instance);
}

UpdateMdvStartEndTimes *UpdateMdvStartEndTimes::Inst()
{
  assert(_instance != (UpdateMdvStartEndTimes *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool UpdateMdvStartEndTimes::init()
{
  static const string method_name = "UpdateMdvStartEndTimes::init()";
  
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

void UpdateMdvStartEndTimes::run()
{
  static const string method_name = "UpdateMdvStartEndTimes::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getNextVolTime()
 */

DateTime UpdateMdvStartEndTimes::_getNextVolTime(const DateTime &curr_vol_time) const
{
  static const string method_name = "UpdateMdvStartEndTimes::_getNextVolTime()";
  
  DsMdvx mdvx;
  
  mdvx.clearTimeListMode();
  mdvx.setTimeListModeFirstAfter(_params->input_url,
				 curr_vol_time.utime() + 1,
				 _params->max_valid_vol_time);

  if (mdvx.compileTimeList() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error compiling time list" << endl;
    
    return DateTime::NEVER;
  }
  
  vector< time_t > time_list = mdvx.getTimeList();
  
  if (time_list.size() != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Time list contains " << time_list.size() << " entries" << endl;
    cerr << "List should contain just 1 entry" << endl;
    
    return DateTime::NEVER;
  }
  
  return DateTime(time_list[0]);
}


/*********************************************************************
 * _getPrevVolTime()
 */

DateTime UpdateMdvStartEndTimes::_getPrevVolTime(const DateTime &curr_vol_time) const
{
  static const string method_name = "UpdateMdvStartEndTimes::_getPrevVolTime()";
  
  DsMdvx mdvx;
  
  mdvx.clearTimeListMode();
  mdvx.setTimeListModeFirstBefore(_params->input_url,
				  curr_vol_time.utime() - 1,
				  _params->max_valid_vol_time);

  if (mdvx.compileTimeList() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error compiling time list" << endl;
    
    return DateTime::NEVER;
  }
  
  vector< time_t > time_list = mdvx.getTimeList();
  
  if (time_list.size() != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Time list contains " << time_list.size() << " entries" << endl;
    cerr << "List should contain just 1 entry" << endl;
    
    return DateTime::NEVER;
  }
  
  return DateTime(time_list[0]);
}


/*********************************************************************
 * _initTrigger()
 */

bool UpdateMdvStartEndTimes::_initTrigger(void)
{
  static const string method_name = "UpdateMdvStartEndTimes::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);
    
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
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool UpdateMdvStartEndTimes::_processData(const DateTime &trigger_time)
{
  static const string method_name = "UpdateMdvStartEndTimes::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read the new input data

  DsMdvx mdvx;
  
  if (!_readInputFile(mdvx, trigger_time))
    return false;
  
  // Get the volume times

  DateTime start_time;
  DateTime end_time;
  
  switch (_params->centroid_interp)
  {
  case Params::CENTROID_START_TIME :
    start_time = trigger_time;
    end_time = _getNextVolTime(trigger_time);
    break;
    
  case Params::CENTROID_END_TIME :
    start_time = _getPrevVolTime(trigger_time);
    end_time = trigger_time;
    break;
    
  } /* endswitch - _params->centroid_interp */
  
  if (_params->debug)
  {
    cerr << "start_time = " << start_time << endl;
    cerr << "end_time = " << end_time << endl;
  }
  
  // Make sure we got the data times we need

  if (start_time == DateTime::NEVER ||
      end_time == DateTime::NEVER)
    return false;
  
  // Update and write the file

  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
  master_hdr.time_begin = start_time.utime();
  master_hdr.time_end = end_time.utime();
  mdvx.setMasterHeader(master_hdr);
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing file to output URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile()
 */

bool UpdateMdvStartEndTimes::_readInputFile(Mdvx &input_file,
					    const DateTime &trigger_time) const
{
  static const string method_name = "UpdateMdvStartEndTimes::_readInputFile()";
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _params->input_url,
			 0, trigger_time.utime());
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}
