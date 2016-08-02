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
//   $Date: 2016/03/04 02:29:42 $
//   $Id: Sndg2Sounding.cc,v 1.3 2016/03/04 02:29:42 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Sndg2Sounding: Sndg2Sounding program object.
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
#include <Spdb/sounding.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "Sndg2Sounding.hh"
#include "Params.hh"

using namespace std;


// Global variables

Sndg2Sounding *Sndg2Sounding::_instance =
     (Sndg2Sounding *)NULL;



/*********************************************************************
 * Constructor
 */

Sndg2Sounding::Sndg2Sounding(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "Sndg2Sounding::Sndg2Sounding()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Sndg2Sounding *)NULL);
  
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

Sndg2Sounding::~Sndg2Sounding()
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

Sndg2Sounding *Sndg2Sounding::Inst(int argc, char **argv)
{
  if (_instance == (Sndg2Sounding *)NULL)
    new Sndg2Sounding(argc, argv);
  
  return(_instance);
}

Sndg2Sounding *Sndg2Sounding::Inst()
{
  assert(_instance != (Sndg2Sounding *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Sndg2Sounding::init()
{
  static const string method_name = "Sndg2Sounding::init()";
  
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
    
    if (_params->debug)
    {
      cerr << "Successfully initialized LATEST_DATA trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
    }
    
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
    
    if (_params->debug)
    {
      cerr << "Successfully initialized TIME_LIST trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    start time: " << DateTime::str(start_time) << endl;
      cerr << "    end time: " << DateTime::str(end_time) << endl;
    }
    
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

void Sndg2Sounding::run()
{
  static const string method_name = "Sndg2Sounding::run()";
  
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

bool Sndg2Sounding::_processData(const DateTime &trigger_time)
{
  static const string method_name = "Sndg2Sounding::_processData()";
  
  PMU_auto_register("Processing data...");
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " << trigger_time << endl;
  
  // Retrieve the chunks from the SPDB database

  DsSpdb spdb;
  
  if (spdb.getExact(_params->input_url,
		    trigger_time.utime()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving soundings from SPDB database for time: "
	 << trigger_time << endl;
    
    return false;
  }
  
  // Process the soundings

  const vector< Spdb::chunk_t > sndg_chunks = spdb.getChunks();
  
  vector< Spdb::chunk_t >::const_iterator sndg_chunk;
  
  for (sndg_chunk = sndg_chunks.begin(); sndg_chunk != sndg_chunks.end();
       ++sndg_chunk)
  {
    // Convert the chunk to a Sndg object

    Sndg sndg;

    if (sndg.disassemble(sndg_chunk->data, sndg_chunk->len) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error converting SPDB chunk to Sndg format" << endl;
      cerr << "Skipping chunk" << endl;
      
      continue;
    }
    
    // Now convert the Sndg object to an SPDB sounding object and write
    // it to the output database

    _writeSounding(sndg,
		   sndg_chunk->valid_time, sndg_chunk->expire_time);
    
  } /* endfor - sndg_chunk */
  
  return true;
}


/*********************************************************************
 * _writeSounding() - Convert the given Sndg object to an SPDB sounding
 *                    object and write it to the output database.
 */

void Sndg2Sounding::_writeSounding(const Sndg &sndg,
				   const time_t valid_time,
				   const time_t expire_time) const
{
  static const string method_name = "Sndg2Sounding::_writeSounding()";
  
  // Retrieve the information from the input Sndg.

  Sndg::header_t sndg_hdr = sndg.getHeader();
  const vector< Sndg::point_t > sndg_pts = sndg.getPoints();
  
  // Allocate space for the new sounding object

  int buffer_size = sizeof(SNDG_spdb_product_t) +
    (sizeof(SNDG_spdb_point_t) * sndg_hdr.nPoints - 1);
  
  char *sounding_buffer = new char[buffer_size];
  memset(sounding_buffer, 0, buffer_size);
  
  SNDG_spdb_product_t *sounding = (SNDG_spdb_product_t *)sounding_buffer;
  
  // Convert the sounding values

  sounding->launchTime = sndg_hdr.launchTime;
  sounding->nPoints = sndg_hdr.nPoints;
  sounding->sourceId = sndg_hdr.sourceId;
  sounding->leadSecs = sndg_hdr.leadSecs;
  sounding->lat = sndg_hdr.lat;
  sounding->lon = sndg_hdr.lon;
  sounding->alt = sndg_hdr.alt;
  STRcopy(sounding->sourceName, sndg_hdr.sourceName, SOURCE_NAME_LENGTH);
  STRcopy(sounding->sourceFmt, sndg_hdr.sourceFmt, SOURCE_FMT_LENGTH);
  STRcopy(sounding->siteName, sndg_hdr.siteName, SOURCE_FMT_LENGTH);
  
  for (int i = 0; i < sndg_hdr.nPoints; ++i)
  {
    Sndg::point_t sndg_pt = sndg_pts[i];
    
    sounding->points[i].pressure = sndg_pt.pressure;
    sounding->points[i].altitude = sndg_pt.altitude;
    sounding->points[i].u = sndg_pt.u;
    sounding->points[i].v = sndg_pt.v;
    sounding->points[i].w = sndg_pt.w;
    sounding->points[i].rh = sndg_pt.rh;
    sounding->points[i].temp = sndg_pt.temp;
  } /* endfor - i */
  
  // Write the new sounding to the output database

  SNDG_spdb_product_to_BE(sounding);
  
  DsSpdb spdb;
  
  spdb.setPutMode(Spdb::putModeAdd);
  
  if (spdb.put(_params->output_url,
	       SPDB_SNDG_ID,
	       SPDB_SNDG_LABEL,
	       0,
	       valid_time,
	       expire_time,
	       buffer_size,
	       sounding_buffer) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing sounding to output URL: "
	 << _params->output_url << endl;
    
    delete [] sounding_buffer;
    
    return;
  }
  
  // Clean up memory

  delete [] sounding_buffer;
}
