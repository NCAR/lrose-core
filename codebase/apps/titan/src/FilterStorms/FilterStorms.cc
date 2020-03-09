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
//   $Id: FilterStorms.cc,v 1.15 2016/03/04 02:04:38 dixon Exp $
//   $Revision: 1.15 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FilterStorms: FilterStorms program object.
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

#include "FilterStorms.hh"
#include "Params.hh"

#include "AreaFilter.hh"
#include "InterestFilter.hh"
#include "LongitudeFilter.hh"
#include "PartitionFilter.hh"
using namespace std;


// Global variables

FilterStorms *FilterStorms::_instance = (FilterStorms *)NULL;


/*********************************************************************
 * Constructor
 */

FilterStorms::FilterStorms(int argc, char **argv)
{
  static const string method_name = "FilterStorms::FilterStorms()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (FilterStorms *)NULL);
  
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

FilterStorms::~FilterStorms()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  delete _filter;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

FilterStorms *FilterStorms::Inst(int argc, char **argv)
{
  if (_instance == (FilterStorms *)NULL)
    new FilterStorms(argc, argv);
  
  return(_instance);
}

FilterStorms *FilterStorms::Inst()
{
  assert(_instance != (FilterStorms *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool FilterStorms::init()
{
  static const string method_name = "FilterStorms::init()";
  
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
  
  // Initialize the filter object

  MdvFilter::missing_input_action_t missing_input_action = 
    MdvFilter::MISSING_NO_OUTPUT;
  
  switch (_params->missing_input_action)
  {
  case Params::MISSING_NO_OUTPUT :
    missing_input_action = MdvFilter::MISSING_NO_OUTPUT;
    break;
    
  case Params::MISSING_PASSING :
    missing_input_action = MdvFilter::MISSING_PASSING;
    break;
    
  case Params::MISSING_FAILING :
    missing_input_action = MdvFilter::MISSING_FAILING;
    break;
  } /* endswitch - _params->missing_input_action */
  
  switch (_params->filter_type)
  {
  case Params::AREA_FILTER :
    _filter = new AreaFilter(_params->area_min_storm_size,
			     _params->debug >= Params::DEBUG_VERBOSE);
    break;
    
  case Params::LONGITUDE_FILTER :
    _filter = new LongitudeFilter(_params->longitude_min,
				  _params->debug >= Params::DEBUG_VERBOSE);
    break;
    
  case Params::INTEREST_FILTER :
    _filter = new InterestFilter(_params->interest_url,
				 _params->interest_field_name,
				 _params->interest_level_num,
				 _params->interest_max_valid_secs,
				 _params->interest_value_type == Params::MAX,
				 _params->interest_min_value,
				 _params->interest_storm_growth,
				 missing_input_action,
				 _params->debug >= Params::DEBUG_VERBOSE);
    break;
    
  case Params::PARTITION_FILTER :
    _filter = new PartitionFilter(_params->partition_url,
				  _params->partition_field_name,
				  _params->partition_level_num,
				  _params->partition_max_valid_secs,
				  _params->partition_value,
				  _params->partition_percent,
				  _params->partition_storm_growth,
				  missing_input_action,
				  _params->debug >= Params::DEBUG_VERBOSE);
    break;
    
  } /* endswitch - _params->filter_type */
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void FilterStorms::run()
{
  static const string method_name = "FilterStorms::run()";
  
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

bool FilterStorms::_processData(const DateTime &data_time)
{
  static const string method_name = "FilterStorms::_processData()";
  
  if (_params->debug >= Params::DEBUG_WARNINGS)
    cerr << endl << "*** Processing storms for time: " << data_time << endl;
  
  PMU_auto_register("Processing data");

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
  
  // Initialize the filter for the current data time

  PMU_auto_register("Initializing current filter");

  if (!_filter->initFilter(data_time))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error intializing filter for data time: "
	 << data_time << endl;
    
    return false;
  }
  
  // Process all of the storms in all of the groups

  TstormMgr passing_mgr;
  TstormMgr failing_mgr;
  
  vector< TstormGroup* > groups = input_tstorm_mgr.getGroups();
  vector< TstormGroup* >::iterator group_iter;
  
  for (group_iter = groups.begin(); group_iter != groups.end();
       ++group_iter)
  {
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "   Found " << (*group_iter)->getNStorms()
	   << " storms in current group" << endl;
    
    TstormGroup *group = *group_iter;
    
    TstormGroup *passing_group =
      new TstormGroup(group->getNSides(),
		      group->getDataTime(),
		      group->getDbzThreshold(),
		      group->getStartAzimuth(),
		      group->getDeltaAzimuth(),
		      group->getGrid());

    TstormGroup *failing_group =
      new TstormGroup(group->getNSides(),
		      group->getDataTime(),
		      group->getDbzThreshold(),
		      group->getStartAzimuth(),
		      group->getDeltaAzimuth(),
		      group->getGrid());
  
    vector< Tstorm* > storms = group->getTstorms();
    vector< Tstorm* >::iterator storm_iter;
    
    for (storm_iter = storms.begin(); storm_iter != storms.end();
	 ++storm_iter)
    {
      PMU_auto_register("Processing storm");

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
      
      // If the storm passes the filter criteria add it to the
      // passing group.  Otherwise, add it to the failing group.
      // While doing the filtering, calculate the filter algorithm
      // value and add it to the output storm object, if requested.

      double algorithm_value;
      Tstorm *new_storm = new Tstorm(*storm);
      
      bool is_passing = _filter->isPassing(*storm, algorithm_value);

      if (_params->output_algorithm_value)
      {
	if (_params->debug >= Params::DEBUG_VERBOSE)
	  cerr << "     Setting algorithm value to "
	       << algorithm_value << endl;
	
	new_storm->setAlgorithmValue(algorithm_value);
      }
      
      if (is_passing)
      {
	if (_params->debug >= Params::DEBUG_VERBOSE)
	  cerr << "     Adding storm to passing group" << endl;
	
	passing_group->addTstorm(new_storm);
      }
      else
      {
	if (_params->debug >= Params::DEBUG_VERBOSE)
	  cerr << "     Adding storm to failing group" << endl;
	
	failing_group->addTstorm(new_storm);
      }
      
    } /* endfor - storm_iter */
    
    passing_mgr.addGroup(passing_group);
    failing_mgr.addGroup(failing_group);
    
  } /* endfor - group_iter */
  
  // Write the output databases

  if (_params->write_passing_storms &&
      !passing_mgr.writeTstorms(_params->passing_storms_url))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing passing storms to passing storms URL: "
	 << _params->passing_storms_url << endl;
    
    return false;
  }
  
  if (_params->write_failing_storms &&
      !failing_mgr.writeTstorms(_params->failing_storms_url))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing failing storms to failing storms URL: "
	 << _params->failing_storms_url << endl;
    
    return false;
  }
  
  return true;
}
