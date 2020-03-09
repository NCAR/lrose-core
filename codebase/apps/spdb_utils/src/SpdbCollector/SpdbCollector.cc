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
//   $Date: 2016/03/07 01:39:56 $
//   $Id: SpdbCollector.cc,v 1.4 2016/03/07 01:39:56 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SpdbCollector: SpdbCollector program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsIntervalTrigger.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "SpdbCollector.hh"
#include "Params.hh"

using namespace std;


// Global variables

SpdbCollector *SpdbCollector::_instance =
     (SpdbCollector *)NULL;



/*********************************************************************
 * Constructor
 */

SpdbCollector::SpdbCollector(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "SpdbCollector::SpdbCollector()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (SpdbCollector *)NULL);
  
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

SpdbCollector::~SpdbCollector()
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

SpdbCollector *SpdbCollector::Inst(int argc, char **argv)
{
  if (_instance == (SpdbCollector *)NULL)
    new SpdbCollector(argc, argv);
  
  return(_instance);
}

SpdbCollector *SpdbCollector::Inst()
{
  assert(_instance != (SpdbCollector *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool SpdbCollector::init()
{
  static const string method_name = "SpdbCollector::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::INTERVAL_REALTIME :
  {
    if (_params->debug)
    {
      cerr << "Initializing INTERVAL_REALTIME trigger: " << endl;
      cerr << "     trigger interval: " << _params->trigger_interval
	   << " secs" << endl;
      cerr << "     trigger start time: " << _params->trigger_start_time
	   << "secs" << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->trigger_interval,
		      _params->trigger_start_time,
		      1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL_REALTIME trigger" << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    if (_params->debug)
      cerr << "Successfully initialized INTERVAL_REALTIME trigger" << endl;
    
    break;
  }
  
  case Params::INTERVAL_ARCHIVE :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->interval_archive.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->interval_archive.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for INTERVAL_ARCHIVE trigger: "
	   << _params->interval_archive.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for INTERVAL_ARCHIVE trigger: "
	   << _params->interval_archive.end_time << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing INTERVAL_ARCHIVE trigger: " << endl;
      cerr << "     trigger interval: " << _params->trigger_interval
	   << " secs" << endl;
      cerr << "     start time: " << start_time << endl;
      cerr << "     end time: " << end_time << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->trigger_interval,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INTERVAL_ARCHIVE trigger:" << endl;
      cerr << "    Trigger interval: " << _params->trigger_interval
	   << " seconds" << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    if (_params->debug)
      cerr << "Successfully initialized INTERVAL_ARCHIVE trigger:" << endl;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // initialize process registration

  if (_params->trigger_mode == Params::INTERVAL_REALTIME)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void SpdbCollector::run()
{
  static const string method_name = "SpdbCollector::run()";
  
  TriggerInfo trigger_info;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info.getIssueTime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " 
	   << DateTime(trigger_info.getIssueTime()) << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool SpdbCollector::_processData(const DateTime &trigger_time)
{
  static const string method_name = "SpdbCollector::_processData()";
  
  PMU_auto_register("Processing data...");
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " << trigger_time << endl;
  
  // Retrieve the chunks from the input SPDB database

  DsSpdb input_spdb;
  
  time_t end_time = trigger_time.utime();
  time_t start_time = end_time - _params->trigger_interval;
  
  if (input_spdb.getInterval(_params->input_url,
			     start_time, end_time) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving soundings from SPDB database: " << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   start time: " << DateTime::str(start_time) << endl;
    cerr << "   end time: " << DateTime::str(end_time) << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "    Successfully retrieved " << input_spdb.getNChunks()
	 << " chunks from input database" << endl;
  
  // Calculate the times to use for the output chunks

  time_t output_valid_time =
    trigger_time.utime() - _params->chunk_hdr_time_offset;
  time_t output_expire_time = trigger_time.utime();
  
  // Write the chunks to the output database

  DsSpdb output_spdb;
  
  output_spdb.setPutMode(Spdb::putModeAdd);
  
  const vector< Spdb::chunk_t > input_chunks = input_spdb.getChunks();
  vector< Spdb::chunk_t >::const_iterator input_chunk;
  int chunk_num;
  
  for (input_chunk = input_chunks.begin(), chunk_num = 1;
       input_chunk != input_chunks.end();
       ++input_chunk, ++chunk_num)
  {
    int data_type = input_chunk->data_type;
    if (_params->number_chunks)
      data_type = chunk_num;
    
    output_spdb.addPutChunk(data_type,
			    output_valid_time,
			    output_expire_time,
			    input_chunk->len,
			    input_chunk->data,
			    input_chunk->data_type2);
  }
  
  if (output_spdb.nPutChunks() > 0)
  {
    if (output_spdb.put(_params->output_url,
			input_spdb.getProdId(),
			input_spdb.getProdLabel()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing chunks to output URL: "
	   << _params->output_url << endl;
      
      return false;
    }

    if (_params->debug)
      cerr << "    Successfully wrote " << output_spdb.nPutChunks()
	   << " chunks to output database" << endl;
  }
  
  return true;
}
