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
//   $Id: Sndg2Ascii.cc,v 1.6 2016/03/07 01:39:56 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Sndg2Ascii: Sndg2Ascii program object.
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
#include <dsdata/DsFileListTrigger.hh>
#include <rapformats/Sndg.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "Sndg2Ascii.hh"
#include "Params.hh"

#include "HuaqingAsciiWriter.hh"
#include "ClassAsciiWriter.hh"

using namespace std;


// Global variables

Sndg2Ascii *Sndg2Ascii::_instance =
     (Sndg2Ascii *)NULL;



/*********************************************************************
 * Constructor
 */

Sndg2Ascii::Sndg2Ascii(int argc, char **argv) :
  _dataTrigger(0),
  _writer(0)
{
  static const string method_name = "Sndg2Ascii::Sndg2Ascii()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Sndg2Ascii *)NULL);
  
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
  char *params_path = (char *)"unknown";
  
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

Sndg2Ascii::~Sndg2Ascii()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete _writer;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Sndg2Ascii *Sndg2Ascii::Inst(int argc, char **argv)
{
  if (_instance == (Sndg2Ascii *)NULL)
    new Sndg2Ascii(argc, argv);
  
  return(_instance);
}

Sndg2Ascii *Sndg2Ascii::Inst()
{
  assert(_instance != (Sndg2Ascii *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Sndg2Ascii::init()
{
  static const string method_name = "Sndg2Ascii::init()";
  
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
  
  // Initialize the ASCII writer object.  If we are writing everything
  // to the same output file, then open the file now.  It will be closed
  // when this process exits and the _writer object is deleted.

  switch (_params->output_format)
  {
  case Params::HUAQING_FORMAT :
    _writer = new HuaqingAsciiWriter(_params->debug);
    break;
  case Params::CLASS_FORMAT :
    _writer = new ClassAsciiWriter(_params->debug);
    break;
  } /* endswitch - _params->output_format */
  
  if (!_params->create_separate_output_files &&
      !_params->separate_sounding_output_files)
    _writer->openFile(_params->output_path);
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);


  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void Sndg2Ascii::run()
{
  static const string method_name = "Sndg2Ascii::run()";
  
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

bool Sndg2Ascii::_processData(const DateTime &trigger_time)
{
  static const string method_name = "Sndg2Ascii::_processData()";

  string extension;

  if(_params->output_format == Params::CLASS_FORMAT)
  {
    extension = "class";
  }
  else
  {
    extension = "txt";
  }
  
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
  
  // Open the output file, if necessary

  if (_params->create_separate_output_files &&
      !_params->separate_sounding_output_files)
  {
    char ascii_file_path[BUFSIZ];
  
    sprintf(ascii_file_path, "%s/%04d%02d%02d/%02d%02d%02d.%s",
	    _params->output_path,
	    trigger_time.getYear(), trigger_time.getMonth(),
	    trigger_time.getDay(),
	    trigger_time.getHour(), trigger_time.getMin(),
	    trigger_time.getSec(), extension.c_str());
  
    if (!_writer->openFile(ascii_file_path))
      return false;
  }

  // Process the soundings

  const vector< Spdb::chunk_t > sndg_chunks = spdb.getChunks();
  
  vector< Spdb::chunk_t >::const_iterator sndg_chunk;
  
  for (sndg_chunk = sndg_chunks.begin(); sndg_chunk != sndg_chunks.end();
       ++sndg_chunk)
  {

    // Convert the chunk to a sounding

    Sndg sounding;

    if (sounding.disassemble(sndg_chunk->data, sndg_chunk->len) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error converting SPDB chunk to Sndg format" << endl;
      cerr << "Skipping chunk" << endl;
      
      continue;
    }
    
    // Write the sounding to the output file
    
    Sndg::header_t sndg_hdr = sounding.getHeader();

    if(_params->separate_sounding_output_files)
    {
      char ascii_file_path[BUFSIZ];
  
      sprintf(ascii_file_path, "%s/%s/%04d%02d%02d/%02d%02d%02d.%s",
	      _params->output_path,
	      sndg_hdr.siteName,
	      trigger_time.getYear(), trigger_time.getMonth(),
	      trigger_time.getDay(),
	      trigger_time.getHour(), trigger_time.getMin(),
	      trigger_time.getSec(), extension.c_str());
  
      if (!_writer->openFile(ascii_file_path))
	return false;
    }  

    _writer->writeSndg(sounding);

    if (_params->separate_sounding_output_files)
      _writer->closeFile();
    
  } /* endfor - sndg_chunk */
  
  // Close the output file, if necessary

  if (_params->create_separate_output_files)
    _writer->closeFile();
  
  return true;
}
