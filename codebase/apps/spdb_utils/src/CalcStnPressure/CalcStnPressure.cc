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
//   $Id: CalcStnPressure.cc,v 1.2 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * CalcStnPressure: CalcStnPressure program object.
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
#include <physics/stn_pressure.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "CalcStnPressure.hh"
#include "Params.hh"

using namespace std;


// Global variables

CalcStnPressure *CalcStnPressure::_instance =
     (CalcStnPressure *)NULL;


/*********************************************************************
 * Constructor
 */

CalcStnPressure::CalcStnPressure(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "CalcStnPressure::CalcStnPressure()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (CalcStnPressure *)NULL);
  
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

CalcStnPressure::~CalcStnPressure()
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

CalcStnPressure *CalcStnPressure::Inst(int argc, char **argv)
{
  if (_instance == (CalcStnPressure *)NULL)
    new CalcStnPressure(argc, argv);
  
  return(_instance);
}

CalcStnPressure *CalcStnPressure::Inst()
{
  assert(_instance != (CalcStnPressure *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool CalcStnPressure::init()
{
  static const string method_name = "CalcStnPressure::init()";
  
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
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void CalcStnPressure::run()
{
  static const string method_name = "CalcStnPressure::run()";
  
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

bool CalcStnPressure::_processData(const DateTime &trigger_time)
{
  static const string method_name = "CalcStnPressure::_processData()";
  
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
    
    // Update the pressure station fields

    if (stn_report.msg_id != PRESSURE_STATION_REPORT ||
	(stn_report.shared.pressure_station.stn_pres == STATION_NAN &&
	 stn_report.pres != STATION_NAN &&
	 stn_report.alt != STATION_NAN))
      stn_report.shared.pressure_station.stn_pres =
	SL2StnPressure(stn_report.pres, stn_report.alt);
    else
      stn_report.shared.pressure_station.stn_pres = STATION_NAN;
    
    stn_report.shared.pressure_station.Spare1 = STATION_NAN;
    stn_report.shared.pressure_station.Spare2 = STATION_NAN;
    
    stn_report.msg_id = PRESSURE_STATION_REPORT;
    
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
