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
//   $Id: TstormTemporalSmooth.cc,v 1.9 2016/03/04 02:04:38 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TstormTemporalSmooth: TstormTemporalSmooth program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
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

#include "Params.hh"
#include "TstormTemporalSmooth.hh"

#include "SimpleIdVecCalc.hh"
#include "WeightedAvgVecCalc.hh"

using namespace std;


// Global variables

TstormTemporalSmooth *TstormTemporalSmooth::_instance =
     (TstormTemporalSmooth *)NULL;


/*********************************************************************
 * Constructor
 */

TstormTemporalSmooth::TstormTemporalSmooth(int argc, char **argv) :
  _constraintVecCalc(0),
  _prevStormTime(DateTime::NEVER)
{
  static const string method_name = "TstormTemporalSmooth::TstormTemporalSmooth()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TstormTemporalSmooth *)NULL);
  
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

TstormTemporalSmooth::~TstormTemporalSmooth()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  delete _constraintVecCalc;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TstormTemporalSmooth *TstormTemporalSmooth::Inst(int argc, char **argv)
{
  if (_instance == (TstormTemporalSmooth *)NULL)
    new TstormTemporalSmooth(argc, argv);
  
  return(_instance);
}

TstormTemporalSmooth *TstormTemporalSmooth::Inst()
{
  assert(_instance != (TstormTemporalSmooth *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TstormTemporalSmooth::init()
{
  static const string method_name = "TstormTemporalSmooth::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug >= Params::DEBUG_VERBOSE)
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
  
  // Initialize the constraint vector calculator

  switch (_params->constraint_vector_calc_type)
  {
  case Params::CONSTRAINT_VECTOR_FROM_SAME_SIMPLE_ID :
    _constraintVecCalc = new SimpleIdVecCalc();
    break;
    
  case Params::CONSTRAINT_VECTOR_WEIGHTED_AVG_AT_CURR_LOCATION :
    _constraintVecCalc = new WeightedAvgVecCalc(_params->weighted_average_radius,
						true);
    break;
    
  case Params::CONSTRAINT_VECTOR_WEIGHTED_AVG_AT_PREV_LOCATION :
    _constraintVecCalc = new WeightedAvgVecCalc(_params->weighted_average_radius,
						false);
    break;
  } /* endswitch - _params->constraint_vector_calc_type */
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void TstormTemporalSmooth::run()
{
  static const string method_name = "TstormTemporalSmooth::run()";
  
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

bool TstormTemporalSmooth::_processData(const DateTime &data_time)
{
  static const string method_name = "TstormTemporalSmooth::_processData()";
  
  if (_params->debug >= Params::DEBUG_WARNINGS)
    cerr << endl << "*** Processing storms for time: " << data_time << endl;

  // Make sure we have a previous storm time to use since we need this 
  // in some cases.

  if (_prevStormTime == DateTime::NEVER)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Process just started up so we don't have the needed previous storm time" << endl;
    cerr << "Skipping storms...." << endl;
    
    _prevStormTime = data_time;

    return false;
  }
  
  // Read in the storms from the input database

  TstormMgr curr_tstorm_mgr;
  
  int num_curr_storm_groups =
    curr_tstorm_mgr.readTstorms(_params->input_url,
				data_time.utime(), 0);

  if (num_curr_storm_groups < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input storms for time: " << data_time << endl;

    _prevStormTime = data_time;

    return false;
  }

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "   Found " << num_curr_storm_groups
	 << " storm groups in the database" << endl;
  
  // If we don't have any storm groups, we don't need to do anything

  if (num_curr_storm_groups == 0)
  {
    _prevStormTime = data_time;

    return true;
  }
  
  // Read in the storms from the constraint database

  DateTime constraint_time;
  
  switch (_params->constraint_time_type)
  {
  case Params::USE_CURRENT_STORM_TIME_FOR_CONSTRAINTS :
    constraint_time = data_time;
    break;
    
  case Params::USE_PREVIOUS_STORM_TIME_FOR_CONSTRAINTS :
    constraint_time = _prevStormTime;
    break;
  }
  
  TstormMgr constraint_tstorm_mgr;
  string constraint_url = _params->constraint_url;
  if (constraint_url.length() == 0)
    constraint_url = _params->input_url;
  
  int num_constraint_storm_groups =
    constraint_tstorm_mgr.readTstorms(constraint_url,
				      constraint_time.utime(),
				      _params->constraint_offset_max);
  
  if (num_constraint_storm_groups <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading constraint storms for time: " << data_time << endl;

    _prevStormTime = data_time;

    // Write the input storms to the output database because we can't
    // constrain any of them.

    if (!curr_tstorm_mgr.writeTstorms(_params->output_url))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing storms to output URL: " 
	   << _params->output_url << endl;
    }
    
    return false;
  }
  
  // Process all of the storms in the group

  TstormGroup *curr_group = *(curr_tstorm_mgr.getGroups().begin());
        
  vector< Tstorm* > curr_storms = curr_group->getTstorms();
  vector< Tstorm* >::iterator curr_storm_iter;
    
  for (curr_storm_iter = curr_storms.begin();
       curr_storm_iter != curr_storms.end(); ++curr_storm_iter)
  {
    // Get a pointer to the current storm object.  This just makes
    // the syntax below a little more readable

    Tstorm *curr_storm = *curr_storm_iter;
      
    // Get the matching storm from the previous time period

    double constraint_speed;
    double constraint_dir;
    
    if (!_constraintVecCalc->getConstraintVector(*curr_storm,
						 data_time - _prevStormTime,
						 constraint_tstorm_mgr,
						 constraint_speed,
						 constraint_dir,
						 _params->valid_constraint_storms_only))
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error getting constraint vector for input storm with simple track id "
	   << curr_storm->getSimpleTrack() << endl;

      continue;
    }
      
    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "===> Constraint vector for simple track "
	   << curr_storm->getSimpleTrack()
	   << ": speed = " << constraint_speed
	   << ", direction = " << constraint_dir << endl;
    
    // Check the speed change between the time periods

    double speed_diff = curr_storm->getSpeed() - constraint_speed;
    if (fabs(speed_diff) > _params->speed_delta_max)
    {
      if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "--> Resetting storm speed value:" << endl;
	cerr << "    curr storm speed: " << curr_storm->getSpeed() << endl;
	cerr << "    constraint speed: " << constraint_speed << endl;
	cerr << "    speed difference: " << speed_diff << endl;
      }
      
      if (speed_diff > 0.0)
	curr_storm->setSpeed(constraint_speed + _params->speed_delta_max);
      else
	curr_storm->setSpeed(constraint_speed - _params->speed_delta_max);

      if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "    new speed value: " << curr_storm->getSpeed() << endl;
      }
    }

    // Check the direction change between time periods.  Note that we
    // have to normalize the difference to between -180 and 180 to account
    // for changes across the 0 degree mark.

    double dir_diff = curr_storm->getDirection() - constraint_dir;
    
    while (dir_diff < -180.0)
      dir_diff += 360.0;
    while (dir_diff >= 180.0)
      dir_diff -= 360.0;
    
    if (fabs(dir_diff) > _params->direction_delta_max)
    {
      if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "--> Resetting storm direction value:" << endl;
	cerr << "    curr storm dir: " << curr_storm->getDirection() << endl;
	cerr << "    constraint dir: " << constraint_dir << endl;
	cerr << "    dir difference: " << dir_diff << endl;
      }
      
      // Calculate the output direction and normalize it to between
      // 0 and 360.

      double new_curr_dir;
      
      if (dir_diff > 0.0)
	new_curr_dir = constraint_dir + _params->direction_delta_max;
      else
	new_curr_dir = constraint_dir - _params->direction_delta_max;

      while (new_curr_dir < 0.0)
	new_curr_dir += 360.0;
      while (new_curr_dir >= 360.0)
	new_curr_dir -= 360.0;
      
      curr_storm->setDirection(new_curr_dir);
      
      if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "    new dir value: " << curr_storm->getDirection() << endl;
      }
    }
    
  } /* endfor - curr_storm_iter */

  // Write the output database

  if (!curr_tstorm_mgr.writeTstorms(_params->output_url))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing storms to output URL: "
	 << _params->output_url << endl;
    
    _prevStormTime = data_time;

    return false;
  }
  
  // Save the current data for the next time period

  _prevStormTime = data_time;
  
  return true;
}
