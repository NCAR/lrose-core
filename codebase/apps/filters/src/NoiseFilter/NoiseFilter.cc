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
//   $Date: 2016/03/07 01:47:03 $
//   $Id: NoiseFilter.cc,v 1.9 2016/03/07 01:47:03 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NoiseFilter: NoiseFilter program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>
#include <cassert>

#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "NoiseFilter.hh"
#include "Params.hh"


// Global variables

NoiseFilter *NoiseFilter::_instance = (NoiseFilter *)NULL;


/*********************************************************************
 * Constructor
 */

NoiseFilter::NoiseFilter(int argc, char **argv)
{
  static const string method_name = "NoiseFilter::NoiseFilter()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (NoiseFilter *)NULL);
  
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

NoiseFilter::~NoiseFilter()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

NoiseFilter *NoiseFilter::Inst(int argc, char **argv)
{
  if (_instance == (NoiseFilter *)NULL)
    new NoiseFilter(argc, argv);
  
  return(_instance);
}

NoiseFilter *NoiseFilter::Inst()
{
  assert(_instance != (NoiseFilter *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool NoiseFilter::init()
{
  static const string method_name = "NoiseFilter::init()";
  
  // Initialize the input FMQ

  if (_inputQueue.initReadOnly(_params->input_fmq_url,
			       _progName,
			       _params->debug,
			       DsFmq::END,
			       -1) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initialize input FMQ at URL: "
	 << _params->input_fmq_url << endl;

    return false;
  }

  // Initialize the output FMQ

  if (_outputQueue.initCreate(_params->output_fmq_url,
			      _progName,
			      _params->debug,
			      false,
			      _params->output_fmq_nslots,
			      _params->output_fmq_size) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing output FMQ at URL: "
	 << _params->output_fmq_url << endl;
    
    return false;
  }

  // Initialize the beam filter

  if (!_beamFilter.init(_params->debug))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing beam filter object" << endl;

    return false;
  }

  // Add the filter fields

  for (int i = 0; i < _params->filter_fields_n; ++i)
    _beamFilter.addFilterField(_params->_filter_fields[i].field_name,
			       _params->_filter_fields[i].filter_value,
			       _params->_filter_fields[i].keep_data_above_filter_value);
  
  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void NoiseFilter::run()
{
  static const string method_name = "NoiseFilter::run()";
  
  // Read and process beams forever

  while (true)
  {
    if (!_processData())
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Trouble reading data" << endl;
    }

//    PMU_auto_register( "Processing data" );
  }

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process data for the given time.
 *
 * Returns true on success, false on failure.
 */

bool NoiseFilter::_processData(void)
{
  static const string method_name = "NoiseFilter::_processData()";
  
  PMU_auto_register("Processing message");

  // Read the next message from the input queue

  int contents;

  if (_inputQueue.getDsMsg(_radarMsg, &contents) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading next message from input FMQ." << endl;

    return false;
  }

  // Update the radar parameter information if there is new information
  // in the message

  if (contents & DsRadarMsg::RADAR_PARAMS &&
      !_beamFilter.updateRadarInfo(_radarMsg.getRadarParams()))
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error updating radar information based on incoming message" << endl;
    cerr << "Message will still be written to the output FMQ..." << endl;
  }

  // Update the field information if there is new information
  // in the message

  if (contents & DsRadarMsg::FIELD_PARAMS &&
      !_beamFilter.updateFieldInfo(_radarMsg.getFieldParams()))
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error updating field information based on incoming message" << endl;
    cerr << "Message will still be written to the output FMQ..." << endl;
  }

  // Filter any beam data included in the message

  if (contents & DsRadarMsg::RADAR_BEAM &&
      !_beamFilter.filterBeam(_radarMsg.getRadarBeam()))
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error filtering beam data" << endl;
    cerr << "Beam will be written to output FMQ even though it may be only partially filtered" << endl;
  }

  // Write the updated message to the output queue

  if (_outputQueue.putDsMsg(_radarMsg, contents) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing current message to output FMQ." << endl;
    
    return false;
  }

  return true;
}
