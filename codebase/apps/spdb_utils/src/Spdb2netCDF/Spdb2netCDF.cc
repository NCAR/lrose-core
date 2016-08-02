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
 * @file Spdb2netCDF.cc
 *
 * @class Spdb2netCDF
 *
 * Spdb2netCDF is the top level application class.
 *  
 * @date 8/7/2014
 *
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <os_config.h>
#include <dsdata/DsIntervalTrigger.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Spdb2netCDF.hh"
#include "Params.hh"
#include "TaiwanAwos.hh"
#include "WxObs.hh"

using namespace std;

// Global variables

Spdb2netCDF *Spdb2netCDF::_instance =
     (Spdb2netCDF *)NULL;


/*********************************************************************
 * Constructor
 */

Spdb2netCDF::Spdb2netCDF(int argc, char **argv) :
  _dataTrigger(0),
  _ldataInfo(0)
{
  static const string method_name = "Spdb2netCDF::Spdb2netCDF()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Spdb2netCDF *)NULL);
  
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
  char *params_path = new char[strlen("unknown")+1];
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


  // create and initialize the Ldata object
  bool ldataDebug = false;
  if ((_params->debug == true) || (_params->verbose == true)) 
  {
    ldataDebug = true;
  }
  _ldataInfo = new DsLdataInfo(_params->output_dir, ldataDebug);
  _ldataInfo->setWriter(_progName);
  _ldataInfo->setDataType("netCDF");
  _ldataInfo->setDataFileExt("nc");
  

  // If the user has specified verbose output, make sure they also get
  // the regular debug output

  if (_params->verbose)
    _params->debug = pTRUE;
  
}


/*********************************************************************
 * Destructor
 */

Spdb2netCDF::~Spdb2netCDF()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;

  delete _ldataInfo;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

Spdb2netCDF *Spdb2netCDF::Inst(int argc, char **argv)
{
  if (_instance == (Spdb2netCDF *)NULL)
    new Spdb2netCDF(argc, argv);
  
  return(_instance);
}

Spdb2netCDF *Spdb2netCDF::Inst()
{
  assert(_instance != (Spdb2netCDF *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool Spdb2netCDF::init()
{
  static const string method_name = "Spdb2netCDF::init()";
  
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

void Spdb2netCDF::run()
{
  static const string method_name = "Spdb2netCDF::run()";
  
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
 * _initTrigger()
 */

bool Spdb2netCDF::_initTrigger(void)
{
  static const string method_name = "Spdb2netCDF::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::REALTIME_INTERVAL :
  {
    if (_params->debug)
    {
      cerr << "Initializing REALTIME_INTERVAL trigger: " << endl;
      cerr << "    Interval secs = " << _params->interval_secs << endl;
      cerr << "    Start secs = " << _params->start_secs << endl;
      cerr << "    Delay secs = " << _params->delay_secs << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_secs,
		      _params->start_secs + _params->delay_secs,
		      1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing REALTIME_INTERVAL trigger: " << endl;
      cerr << "    Interval secs = " << _params->interval_secs << endl;
      cerr << "    Start secs = " << _params->start_secs << endl;
      cerr << "    Delay secs = " << _params->delay_secs << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::ARCHIVE_INTERVAL :
  {
    DateTime start_time = _args->getStartTime();
    DateTime end_time = _args->getEndTime();
    
    if (start_time == DateTime::NEVER ||
	end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Must specify start and end times on command line" << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing ARCHIVE_INTERVAL trigger: " << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      cerr << "    Interval secs = " << _params->interval_secs << endl;
    }
    
    // Initialize the trigger.  Start at the end of the first interval since we are
    // looking back from the trigger time.  This will make the output files start at
    // the given start time, as the user would expect.

    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_secs,
		      start_time.utime() + _params->interval_secs,
		      end_time.utime() + _params->interval_secs) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing ARCHIVE_INTERVAL trigger:" << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      cerr << "    Interval secs = " << _params->interval_secs << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;

    // If we are in archive mode, we need to make sure that the delay
    // seconds are set to 0 for the trigger to work properly.

    _params->delay_secs = 0;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool Spdb2netCDF::_processData(const DateTime &trigger_time)
{
  static const string method_name = "Spdb2netCDF::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Triggering at time: " << trigger_time << endl;

  DateTime interval_start =
    trigger_time - (_params->interval_secs + _params->delay_secs);
  DateTime interval_end =
    trigger_time - _params->delay_secs - 1;
  
  if (_params->verbose)
  {
    cerr << "    Interval start time: " << interval_start << endl;
    cerr << "    Interval end time: " << interval_end << endl;
  }
  
  // Read the input data

  if (_params->verbose)
    cerr << "    Reading input data..." << endl;
  
  DsSpdb spdb;
  if (spdb.getInterval(_params->input_url,
		       interval_start.utime(),
		       interval_end.utime()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input SPDB data" << endl;
    cerr << spdb.getErrorStr() << endl;
    
    return false;
  }
  
  const vector< Spdb::chunk_t > chunks = spdb.getChunks();
  
  if (_params->verbose)
    cerr << "      Found " << chunks.size() << " chunks" << endl;
  
  // Create and initialize the feature file

  FeatureNcFile nc_file(_params->feature_name, _params->verbose);
  if (!nc_file.openWrite(_params->output_dir, interval_start, interval_end))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening feature file for writing" << endl;
    
    return false;
  }
  
  // Process each of the chunks.
  // NOTE: I'd like to create a FeatureFactory class that will create the correct
  // feature object so we can get rid of this switch statement. That would clean
  // up the code considerably.

  vector< Spdb::chunk_t >::const_iterator chunk;
  for (chunk = chunks.begin(); chunk != chunks.end(); ++chunk)
  {
    switch (spdb.getProdId())
    {
    case SPDB_STATION_REPORT_ID :
    {
      WxObs wx_obs;
    
      wx_obs.fromSpdbChunk(*chunk);
      wx_obs.toNetcdf(nc_file);

      break;
    }
    
    case SPDB_TAIWAN_AWOS_REPORT_ID :
    {
      TaiwanAwos awos;
    
      awos.fromSpdbChunk(*chunk);
      awos.toNetcdf(nc_file);

      break;
    }
    
    default:
    {
      cerr << "---> Encountered product id: " << spdb.getProdId() << endl;
      cerr << "     We do not yet process this type of database" << endl;
      exit(0);
    }
    
    } /* endswitch - spdb.getProdId() */
    
  }
  
  // Write the output file

  nc_file.close();

  // write ldata_info file, if requested
  if (_params->write_ldata) 
  {
    _ldataInfo->setRelDataPath(nc_file.getFileName());
    _ldataInfo->setIsFcast(false);
    _ldataInfo->write(interval_end.utime());
  }

  return true;
}
