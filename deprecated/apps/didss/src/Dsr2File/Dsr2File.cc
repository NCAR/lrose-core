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
//   $Date: 2016/03/06 23:53:39 $
//   $Id: Dsr2File.cc,v 1.5 2016/03/06 23:53:39 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Dsr2File: Dsr2File program object.
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

#include <toolsa/os_config.h>
#include <dataport/bigend.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <cassert>

#include "Dsr2File.hh"
#include "Params.hh"
using namespace std;


// Global variables

Dsr2File *Dsr2File::_instance = (Dsr2File *)NULL;


/*********************************************************************
 * Constructor
 */

Dsr2File::Dsr2File(int argc, char **argv) :
  _currentOutputFile(0),
  _currentFileStartTime(DateTime::NEVER)
{
  static const string method_name = "Dsr2File::Dsr2File()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Dsr2File *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopright message.

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

Dsr2File::~Dsr2File()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Close any open files

  if (_currentOutputFile != 0)
    fclose(_currentOutputFile);
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Dsr2File *Dsr2File::Inst(int argc, char **argv)
{
  if (_instance == (Dsr2File *)NULL)
    new Dsr2File(argc, argv);
  
  return(_instance);
}

Dsr2File *Dsr2File::Inst()
{
  assert(_instance != (Dsr2File *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Dsr2File::init()
{
  static const string method_name = "Dsr2File::init()";
  
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

  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void Dsr2File::run()
{
  static const string method_name = "Dsr2File::run()";
  
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
 * _getCurrentFilePtr() - Make sure _currentFilePtr points to the correct
 *                        file for output.  Opens a new file when
 *                        necessary.
 *
 * Returns true on success, false on failure.
 */

bool Dsr2File::_getCurrentFilePtr(void)
{
  static const string method_name = "Dsr2File::_getCurrentFilePtr()";
  
  // Open a new file if we don't have a currently open file or if
  // the current file is more than an hour old.

  time_t current_time = time(0);
  
  if (_currentFileStartTime == DateTime::NEVER ||
      _currentOutputFile == 0 ||
      current_time - _currentFileStartTime.utime() > 3600)
  {
    // Close the current output file, if there is one

    if (_currentOutputFile != 0)
    {
      fclose(_currentOutputFile);
      _currentOutputFile = 0;
    }
    
    // Get the start time for the new file

    _currentFileStartTime.set(time(0));
    
    _currentFileStartTime.setMin(0);
    _currentFileStartTime.setSec(0);
    
    // Construct the file's directory path and make sure it is created.

    char dirpath[MAX_PATH_LEN];
    
    sprintf(dirpath, "%s/%04d%02d%02d",
	    _params->output_dir,
	    _currentFileStartTime.getYear(),
	    _currentFileStartTime.getMonth(),
	    _currentFileStartTime.getDay());
    
    if (ta_makedir_recurse(dirpath) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error creating output directory: " << dirpath << endl;
      
      return false;
    }

    // Construct the new file's complete path and open the file

    char filepath[MAX_PATH_LEN];
    
    sprintf(filepath, "%s/%02d%02d%02d.dsRadarBin",
	    dirpath,
	    _currentFileStartTime.getHour(),
	    _currentFileStartTime.getMin(),
	    _currentFileStartTime.getSec());
    
    if ((_currentOutputFile = fopen(filepath, "w")) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error opening output file: " << filepath << endl;
      
      return false;
    }
  }
  
  return true;
  
}


/*********************************************************************
 * _processData() - Process data for the given time.
 *
 * Returns true on success, false on failure.
 */

bool Dsr2File::_processData(void)
{
  static const string method_name = "Dsr2File::_processData()";
  
  PMU_auto_register("Processing message");

  // Read the next message from the input queue

  int contents;

  if (_inputQueue.getDsMsg(_radarMsg, &contents) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading next message from input FMQ." << endl;

    return false;
  }

  // Set up the current file pointer

  if (!_getCurrentFilePtr())
    return false;
  
  // Write the message to the file

  ui08 *msg = _radarMsg.assemble(contents);
  ui16 msg_len = _radarMsg.lengthAssembled();
  
  if (msg_len <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error assembling message for output to file" << endl;
    
    return false;
  }
  
  ui16 output_msg_len = BE_from_ui16(msg_len);

  if (fwrite((void *)&output_msg_len, sizeof(output_msg_len), 1,
	     _currentOutputFile) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing message length to output file" << endl;
    
    return false;
  }
  

  if (fwrite((void *)msg, sizeof(ui08), msg_len,
	     _currentOutputFile) != msg_len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing message to output file" << endl;
    
    return false;
  }
  
//  // Write the updated message to the output queue
//
//  if (_outputQueue.putDsMsg(_radarMsg, contents) != 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error writing current message to output FMQ." << endl;
//    
//    return false;
//  }

  return true;
}
