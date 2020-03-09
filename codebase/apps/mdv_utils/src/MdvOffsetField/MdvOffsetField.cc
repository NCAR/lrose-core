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
//   $Date: 2016/03/04 02:22:12 $
//   $Id: MdvOffsetField.cc,v 1.4 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvOffsetField: MdvOffsetField program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>
#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MdvOffsetField.hh"
#include "Params.hh"

#include "MinXYMover.hh"
#include "RemapMover.hh"

using namespace std;

// Global variables

MdvOffsetField *MdvOffsetField::_instance =
     (MdvOffsetField *)NULL;


/*********************************************************************
 * Constructor
 */

MdvOffsetField::MdvOffsetField(int argc, char **argv) :
  _dataTrigger(0),
  _mover(0)
{
  static const string method_name = "MdvOffsetField::MdvOffsetField()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MdvOffsetField *)NULL);
  
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

MdvOffsetField::~MdvOffsetField()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  delete _mover;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

MdvOffsetField *MdvOffsetField::Inst(int argc, char **argv)
{
  if (_instance == (MdvOffsetField *)NULL)
    new MdvOffsetField(argc, argv);
  
  return(_instance);
}

MdvOffsetField *MdvOffsetField::Inst()
{
  assert(_instance != (MdvOffsetField *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvOffsetField::init()
{
  static const string method_name = "MdvOffsetField::init()";
  
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
  
  // Create the object used to move the grids

  switch (_params->offset_type)
  {
  case Params::HEADER_MINX_MINY_OFFSET :
    _mover = new MinXYMover(_params->x_offset, _params->y_offset,
			    _params->debug);
    break;
    
  case Params::REMAP_OFFSET :
    _mover = new RemapMover(_params->x_offset, _params->y_offset,
			    _params->debug);
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

void MdvOffsetField::run()
{
  static const string method_name = "MdvOffsetField::run()";
  
  while (!_dataTrigger->endOfData())
  {
    DateTime trigger_time;
  
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (trigger_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Invalid trigger time received" << endl;
      
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
 * _processData() - Process the data for the given time.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvOffsetField::_processData(const DateTime &trigger_time)
{
  static const string method_name = "MdvOffsetField::_processData()";
  
  if (_params->debug)
    cerr << "\n*** Processing data for time: " << trigger_time << endl;
  
  // Read in the input file

  DsMdvx input_file;
  
  if (!_readInputFile(trigger_time, input_file))
    return false;
  
  // Offset each of the fields

  vector< MdvxField* > fields = input_file.getFields();
  vector< MdvxField* >::iterator field;
  
  for (field = fields.begin(); field != fields.end(); ++field)
    _mover->moveField(**field);
  
  // Write the output file

  input_file.setWriteLdataInfo();
  
  if (input_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file" << endl;
    cerr << "   URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInputFile() - Read in the input file for the time specified.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool MdvOffsetField::_readInputFile(const DateTime &data_time,
				    Mdvx &input_file) const
{
  static const string method_name = "MdvOffsetField::_readInputFile()";
  
  // Set up the read request

  input_file.clearRead();
  
  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 _params->input_url,
			 0, data_time.utime());
  
  if (_params->process_indicated_fields)
  {
    if (_params->use_field_names)
    {
      for (int i = 0; i < _params->field_names_n; ++i)
	input_file.addReadField(_params->_field_names[i]);
    }
    else
    {
      for (int i = 0; i < _params->field_numbers_n; ++i)
	input_file.addReadField(_params->_field_numbers[i]);
    }
  }
  
  if (_params->debug)
    input_file.printReadRequest(cerr);
  
  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input file" << endl;
    cerr << "   URL: " << _params->input_url << endl;
    cerr << "   data time: " << data_time << endl;
    
    return false;
  }
  
  return true;
}
