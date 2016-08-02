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
//   $Date: 2016/03/07 01:39:55 $
//   $Id: ConvertStnPressure.cc,v 1.3 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ConvertStnPressure: ConvertStnPressure program object.
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

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "ConvertStnPressure.hh"
#include "Params.hh"

#include "SL2StnConverter.hh"
#include "Stn2SLConverter.hh"

using namespace std;


// Global variables

ConvertStnPressure *ConvertStnPressure::_instance =
     (ConvertStnPressure *)NULL;


/*********************************************************************
 * Constructor
 */

ConvertStnPressure::ConvertStnPressure(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "ConvertStnPressure::ConvertStnPressure()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ConvertStnPressure *)NULL);
  
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

ConvertStnPressure::~ConvertStnPressure()
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

ConvertStnPressure *ConvertStnPressure::Inst(int argc, char **argv)
{
  if (_instance == (ConvertStnPressure *)NULL)
    new ConvertStnPressure(argc, argv);
  
  return(_instance);
}

ConvertStnPressure *ConvertStnPressure::Inst()
{
  assert(_instance != (ConvertStnPressure *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool ConvertStnPressure::init()
{
  static const string method_name = "ConvertStnPressure::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
    {
      cerr << "Initializing LATEST_DATA trigger: " << endl;
      cerr << "     URL: " << _params->input_url << endl;
    }
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      300,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    if (_params->debug)
      cerr << "Successfully initialized LATEST_DATA trigger" << endl;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    time_t start_time = DateTime::parseDateTime(_params->start_time);
    time_t end_time = DateTime::parseDateTime(_params->end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: "
	   << _params->start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: "
	   << _params->end_time << endl;
      
      return false;
    }
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "     URL: " << _params->input_url << endl;
      cerr << "     start time: " << start_time << endl;
      cerr << "     end time: " << end_time << endl;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    Start time: " << start_time << endl;
      cerr << "    End time: " << end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    if (_params->debug)
      cerr << "Successfully initialized TIME_LIST trigger:" << endl;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // Initialize the pressure converter object

  switch(_params->pressure_convert_type)
  {
  case Params::STATION_TO_SEA_LEVEL :
    _pressureConverter = new Stn2SLConverter(_params->debug);
    break;

  case Params::SEA_LEVEL_TO_STATION :
    _pressureConverter = new SL2StnConverter(_params->debug);
    break;
  }
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void ConvertStnPressure::run()
{
  static const string method_name = "ConvertStnPressure::run()";
  
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

bool ConvertStnPressure::_processData(const DateTime &trigger_time)
{
  static const string method_name = "ConvertStnPressure::_processData()";
  
  PMU_auto_register("Processing data...");
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " << trigger_time << endl;
  
  // Retrieve the chunks from the input SPDB database

  DsSpdb input_spdb;
  
  if (input_spdb.getExact(_params->input_url,
			  trigger_time.utime()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving soundings from SPDB database: " << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   request time: " << trigger_time << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << "    Successfully retrieved " << input_spdb.getNChunks()
	 << " chunks from input database" << endl;
  
  // Write the chunks to the output database

  DsSpdb output_spdb;
  
  output_spdb.setPutMode(Spdb::putModeAdd);
  
  const vector< Spdb::chunk_t > input_chunks = input_spdb.getChunks();
  vector< Spdb::chunk_t >::const_iterator input_chunk;
  
  for (input_chunk = input_chunks.begin(); input_chunk != input_chunks.end();
       ++input_chunk)
  {
    // Convert the chunk into a station report object

    if (input_chunk->len != sizeof(station_report_t))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error in chunk size" << endl;
      cerr << "Current chunk contains " << input_chunk->len << " bytes" << endl;
      cerr << "Station_report chunks should contain "
	   << sizeof(station_report_t) << " bytes" << endl;
      cerr << "Skipping chunk...." << endl;
      
      continue;
    }
		     
    station_report_t stn_report;
    memcpy(&stn_report, input_chunk->data, sizeof(stn_report));
    
    station_report_from_be(&stn_report);
    
    if (_params->debug)
    {
      cerr << "   Read station report:" << endl;
      print_station_report(stderr, "      ", &stn_report);
    }
    
    // Update the pressure field

    _pressureConverter->updatePressure(stn_report);
    
    if (_params->debug)
    {
      cerr << "   Updated station report:" << endl;
      print_station_report(stderr, "      ", &stn_report);
    }
    
    // Write out the updated station report

    station_report_to_be(&stn_report);
    
    output_spdb.addPutChunk(input_chunk->data_type,
			    input_chunk->valid_time,
			    input_chunk->expire_time,
			    sizeof(stn_report),
			    &stn_report,
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
