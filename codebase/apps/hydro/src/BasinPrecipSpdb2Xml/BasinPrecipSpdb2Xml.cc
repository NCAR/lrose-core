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
//   $Date: 2016/03/07 18:36:49 $
//   $Id: BasinPrecipSpdb2Xml.cc,v 1.8 2016/03/07 18:36:49 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BasinPrecipSpdb2Xml: BasinPrecipSpdb2Xml program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsIntervalTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "BasinPrecipSpdb2Xml.hh"
#include "Params.hh"
#include "XmlFile.hh"
using namespace std;


// Global variables

BasinPrecipSpdb2Xml *BasinPrecipSpdb2Xml::_instance = (BasinPrecipSpdb2Xml *)NULL;


/*********************************************************************
 * Constructor
 */

BasinPrecipSpdb2Xml::BasinPrecipSpdb2Xml(int argc, char **argv) :
  _trigger(0)
{
  static const string method_name = "BasinPrecipSpdb2Xml::BasinPrecipSpdb2Xml()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (BasinPrecipSpdb2Xml *)NULL);
  
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

BasinPrecipSpdb2Xml::~BasinPrecipSpdb2Xml()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _trigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

BasinPrecipSpdb2Xml *BasinPrecipSpdb2Xml::Inst(int argc, char **argv)
{
  if (_instance == (BasinPrecipSpdb2Xml *)NULL)
    new BasinPrecipSpdb2Xml(argc, argv);
  
  return(_instance);
}

BasinPrecipSpdb2Xml *BasinPrecipSpdb2Xml::Inst()
{
  assert(_instance != (BasinPrecipSpdb2Xml *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool BasinPrecipSpdb2Xml::init()
{
  static const string method_name = "BasinPrecipSpdb2Xml::init()";
  
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

    _trigger = trigger;
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->archive_times.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->archive_times.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->archive_times.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->archive_times.end_time << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->archive_times.start_time <<
	endl;
      cerr << "    End time: " << _params->archive_times.end_time << endl;
      
      return false;
    }
    
    _trigger = trigger;
    
    break;
  }
  
  case Params::REALTIME_INTERVAL :
  {
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_secs,
		      0, 1, PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing REALTIME_INTERVAL trigger" << endl;
      cerr << "     Interval: " << _params->interval_secs <<
	" secs" << endl;
      
      return false;
    }
    
    _trigger = trigger;
    
    break;
  }

  case Params::ARCHIVE_INTERVAL :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->archive_times.start_time);
    time_t end_time =
      DateTime::parseDateTime(_params->archive_times.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for ARCHIVE_INTERVAL trigger: " <<
	_params->archive_times.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for ARCHIVE_INTERVAL trigger: " <<
	_params->archive_times.end_time << endl;
      
      return false;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params->interval_secs,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing ARCHIVE_INTERVAL trigger" << endl;
      cerr << "     Start time: " << _params->archive_times.start_time << endl;
      cerr << "     End time: " << _params->archive_times.end_time << endl;
      cerr << "     Interval: " << _params->interval_secs <<
	" secs" << endl;
      
      return false;
    }
    
    _trigger = trigger;
    
    break;
  }
  } /* endswitch - _params->trigger_mode */
  
  // Create the SPDB handler list

  if (_params->output_times_n <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Must have at least one output time in the output time list." << endl;
    cerr << "Fix the output_times parameter in your parameter file and try again." << endl;
    
    return false;
  }
  
  for (int i = 0; i < _params->output_times_n; ++i)
  {
    string handler_url;
    
    if (_params->_output_times[i] >= 0)
      handler_url = _params->input_url;
    else
      handler_url = _params->input_fcst_url;
    
    SpdbHandler spdb_handler(handler_url,
			     _params->_output_times[i],
			     _params->debug >= Params::DEBUG_NORM);

    _spdbHandlerList.addHandler(spdb_handler);
  } /* endfor - i */
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA ||
      _params->trigger_mode == Params::REALTIME_INTERVAL)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void BasinPrecipSpdb2Xml::run()
{
  static const string method_name = "BasinPrecipSpdb2Xml::run()";
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
    cerr << "Entering " << method_name << endl;
  
  DateTime trigger_time;
  
  while (!_trigger->endOfData())
  {
    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Waiting for trigger..." << endl;
    
    PMU_auto_register("Waiting for trigger...");
    
    if (_trigger->nextIssueTime(trigger_time) != 0)
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
    
  } /* endwhile - !_trigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process data for the given data time.
 *
 * Returns true on success, false on failure.
 */

bool BasinPrecipSpdb2Xml::_processData(const DateTime &trigger_time)
{
  static const string method_name = "BasinPrecipSpdb2Xml::_processData()";
  
  PMU_auto_register("Processing data");
  
  if (_params->debug >= Params::DEBUG_NORM)
  {
    cerr << endl;
    cerr << "*** Processing data for time: " << trigger_time << endl;
  }
  
  // Read in the SPDB data

  if (!_spdbHandlerList.readSpdbDatabase(trigger_time,
					 _params->search_margin))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading in the SPDB database information" << endl;
    
    return false;
  }
  
  // Process each of the fields listed in the parameter file

  for (int i = 0; i < _params->output_field_info_n; ++i)
  {
    // Create and initialize the output file

    XmlFile xml_file(_params->debug >= Params::DEBUG_NORM);
  
    if (!xml_file.init(_params->_output_field_info[i].output_dir,
		       trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing output XML file." << endl;
    
      return false;
    }
  
    // Print out the beginning field tag

    xml_file.writeFieldBeginTag(_params->_output_field_info[i].xml_field_name);
    
    if (!_spdbHandlerList.writeFieldToXmlFile(xml_file,
					      _params->_output_field_info[i].spdb_field_name))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing SPDB field to XML file." << endl;
      cerr << "SPDB field name: <"
	   << _params->_output_field_info[i].spdb_field_name << ">" << endl;
      
      return false;
    }
    
    // Print out the ending field tag

    xml_file.writeFieldEndTag(_params->_output_field_info[i].xml_field_name);
    
  } /* endfor - i */
  
  return true;
}
