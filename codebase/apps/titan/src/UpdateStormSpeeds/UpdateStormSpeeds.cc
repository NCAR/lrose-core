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
//   $Date: 2016/03/04 02:04:38 $
//   $Id: UpdateStormSpeeds.cc,v 1.5 2016/03/04 02:04:38 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * UpdateStormSpeeds: UpdateStormSpeeds program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>
#include <vector>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/Tstorm.hh>
#include <dsdata/TstormGroup.hh>
#include <dsdata/TstormMgr.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "UpdateStormSpeeds.hh"
#include "Params.hh"


// Global variables

UpdateStormSpeeds *UpdateStormSpeeds::_instance = (UpdateStormSpeeds *)NULL;


/*********************************************************************
 * Constructor
 */

UpdateStormSpeeds::UpdateStormSpeeds(int argc, char **argv)
{
  static const string method_name = "UpdateStormSpeeds::UpdateStormSpeeds()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (UpdateStormSpeeds *)NULL);
  
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

UpdateStormSpeeds::~UpdateStormSpeeds()
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

UpdateStormSpeeds *UpdateStormSpeeds::Inst(int argc, char **argv)
{
  if (_instance == (UpdateStormSpeeds *)NULL)
    new UpdateStormSpeeds(argc, argv);
  
  return(_instance);
}

UpdateStormSpeeds *UpdateStormSpeeds::Inst()
{
  assert(_instance != (UpdateStormSpeeds *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool UpdateStormSpeeds::init()
{
  static const string method_name = "UpdateStormSpeeds::init()";
  
  // Initialize the data trigger

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
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  } /* endswitch - _params->trigger_mode */
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void UpdateStormSpeeds::run()
{
  static const string method_name = "UpdateStormSpeeds::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time.utime()))
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
 * _processData() - Process data for the given time.
 *
 * Returns true on success, false on failure.
 */

bool UpdateStormSpeeds::_processData(const DateTime &data_time)
{
  static const string method_name = "UpdateStormSpeeds::_processData()";
  
  if (_params->debug >= Params::DEBUG_WARNINGS)
    cerr << endl << "*** Processing storms for time: " << data_time << endl;
  
  // Read in the storms from the database

  TstormMgr input_tstorm_mgr;
  int num_storm_groups =
    input_tstorm_mgr.readTstorms(_params->input_url,
				 data_time.utime(), 0);

  if (num_storm_groups < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading storms" << endl;

    return false;
  }

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "   Found " << num_storm_groups
	 << " storm groups in the database" << endl;
  
  // If we don't have any storm groups, we don't need to do anything

  if (num_storm_groups == 0)
    return true;
  
  // Process all of the storms in all of the groups

  vector< TstormGroup* > groups = input_tstorm_mgr.getGroups();
  vector< TstormGroup* >::iterator group_iter;
  
  for (group_iter = groups.begin(); group_iter != groups.end();
       ++group_iter)
  {
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "   Found " << (*group_iter)->getNStorms()
	   << " storms in current group" << endl;
    
    TstormGroup *group = *group_iter;
    
    vector< Tstorm* > storms = group->getTstorms();
    vector< Tstorm* >::iterator storm_iter;
    
    for (storm_iter = storms.begin(); storm_iter != storms.end();
	 ++storm_iter)
    {
      // Get a pointer to the current storm object.  This just makes
      // the syntax below a little more readable

      Tstorm *storm = *storm_iter;
      
      // If we are only processing storms with valid forecasts and
      // this storm doesn't have a valid forecast, we can skip it.

      if (_params->valid_forecasts_only &&
	  !storm->isForecastValid())
      {
	if (_params->debug >= Params::DEBUG_NORM)
	  cerr << "    --> Skipping storm with invalid forecast" << endl;
	
	continue;
      }
      
      // Update the storm's speed value.

      storm->setSpeed(storm->getSpeed() * _params->speed_multiplier);
      
    } /* endfor - storm_iter */
    
  } /* endfor - group_iter */
  
  // Write the output databases

  if (!input_tstorm_mgr.writeTstorms(_params->output_url))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing storms to output URL: "
	 << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
