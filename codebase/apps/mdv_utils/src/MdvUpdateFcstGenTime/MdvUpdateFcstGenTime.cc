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
 * @file MdvUpdateFcstGenTime.cc
 *
 * @class MdvUpdateFcstGenTime
 *
 * MdvUpdateFcstGenTime is the top level application class.
 *  
 * @date 2/20/2012
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsFcstTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvUpdateFcstGenTime.hh"
#include "Params.hh"

using namespace std;

// Global variables

MdvUpdateFcstGenTime *MdvUpdateFcstGenTime::_instance =
     (MdvUpdateFcstGenTime *)NULL;



/*********************************************************************
 * Constructor
 */

MdvUpdateFcstGenTime::MdvUpdateFcstGenTime(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "MdvUpdateFcstGenTime::MdvUpdateFcstGenTime()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvUpdateFcstGenTime *)NULL);
  
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

MdvUpdateFcstGenTime::~MdvUpdateFcstGenTime()
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
 * Inst()
 */

MdvUpdateFcstGenTime *MdvUpdateFcstGenTime::Inst(int argc, char **argv)
{
  if (_instance == (MdvUpdateFcstGenTime *)NULL)
    new MdvUpdateFcstGenTime(argc, argv);
  
  return(_instance);
}

MdvUpdateFcstGenTime *MdvUpdateFcstGenTime::Inst()
{
  assert(_instance != (MdvUpdateFcstGenTime *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvUpdateFcstGenTime::init()
{
  static const string method_name = "MdvUpdateFcstGenTime::init()";
  
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

void MdvUpdateFcstGenTime::run()
{
  static const string method_name = "MdvUpdateFcstGenTime::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: "
	   << trigger_info.getIssueTime() << endl;
      
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

bool MdvUpdateFcstGenTime::_initTrigger(void)
{
  static const string method_name = "MdvUpdateFcstGenTime::_initTrigger()";
  
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
      cerr << trigger->getErrStr() << endl;
      
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
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->input_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    
    DsFcstTimeListTrigger *trigger = new DsFcstTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool MdvUpdateFcstGenTime::_processData(const TriggerInfo &trigger_info)
{
  static const string method_name = "MdvUpdateFcstGenTime::_processData()";
  
  DateTime gen_time(trigger_info.getIssueTime());
  DateTime fcst_time(trigger_info.getForecastTime());
  
  if (_params->debug)
  {
    cerr << endl << "*** Processing data for gen time: " << gen_time << endl;
    cerr << endl << "                        valid time: " << fcst_time << endl;
  }
  
  if (fcst_time < gen_time)
  {
    if (_params->debug)
      cerr << "    Forecast valid time before gen time, skipping" << endl;
    
    return true;
  }
  
  // Read the new input data

  DsMdvx mdvx;
  
  if (!_readInputFile(mdvx, gen_time, fcst_time))
    return false;
  
  // Update the master header

  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();
  master_hdr.time_gen += _params->gen_time_offset;
  master_hdr.forecast_delta -= _params->gen_time_offset;
  mdvx.setMasterHeader(master_hdr);
  
  // Update the field headers

  for (size_t i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    
    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    field_hdr.forecast_delta -= _params->gen_time_offset;
    field->setFieldHeader(field_hdr);
  }
  
  // Write the file

  mdvx.setWriteAsForecast();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing file to output URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile()
 */

bool MdvUpdateFcstGenTime::_readInputFile(Mdvx &input_file,
					  const DateTime &gen_time,
					  const DateTime &fcst_time) const
{
  static const string method_name = "MdvUpdateFcstGenTime::_readInputFile()";
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
			 _params->input_url,
			 0,
			 gen_time.utime(),
			 fcst_time.utime() - gen_time.utime());
  
  // Read the data

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _params->input_url << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}
