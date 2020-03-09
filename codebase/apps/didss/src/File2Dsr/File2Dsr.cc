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
//   $Date: 2016/03/06 23:53:40 $
//   $Id: File2Dsr.cc,v 1.6 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * File2Dsr: File2Dsr program object.
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
#include <toolsa/uusleep.h>
#include <cassert>

#include "File2Dsr.hh"
#include "Params.hh"


// Global variables

File2Dsr *File2Dsr::_instance = (File2Dsr *)NULL;


/*********************************************************************
 * Constructor
 */

File2Dsr::File2Dsr(int argc, char **argv)
{
  static const string method_name = "File2Dsr::File2Dsr()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (File2Dsr *)NULL);
  
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

File2Dsr::~File2Dsr()
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

File2Dsr *File2Dsr::Inst(int argc, char **argv)
{
  if (_instance == (File2Dsr *)NULL)
    new File2Dsr(argc, argv);
  
  return(_instance);
}

File2Dsr *File2Dsr::Inst()
{
  assert(_instance != (File2Dsr *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool File2Dsr::init()
{
  static const string method_name = "File2Dsr::init()";
  
  // Initialize the input file list

  for (int i = 0; i < _params->input_files_n; ++i)
    _inputFileList.push_back(_params->_input_files[i]);
  
  // Initialize the output FMQ

  if (_outputQueue.init(_params->output_fmq,
			_progName,
			_params->debug,
			DsFmq::READ_WRITE, DsFmq::END,
			_params->output_fmq_compress,
			_params->output_fmq_nslots,
			_params->output_fmq_size,
			1000))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not initialize radar queue ''"
	 << _params->output_fmq << "'" << endl;
    
    return false;
  }

  // Calculate the sleep time between each beam

  _beamSleepMsecs =
    (int)((1.0 / (float)_params->beams_per_sec * 1000.0) + 0.5);

  // Initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void File2Dsr::run()
{
  static const string method_name = "File2Dsr::run()";
  
  // Process each of the input files

  vector< string >::iterator file_iter;
  
  for (file_iter = _inputFileList.begin();
       file_iter != _inputFileList.end(); ++file_iter)
  {
    if (!_processFile(*file_iter))
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Trouble processing file: " << *file_iter << endl;
    }
  }

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processFile() - Process the data in the given file.
 *
 * Returns true on success, false on failure.
 */

bool File2Dsr::_processFile(const string &file_name)
{
  static const string method_name = " File2Dsr::_processFile()";
  
  if (_params->debug)
    cerr << "*** Processing file: " << file_name << endl;

  // Open the input file

  FILE *input_file;
  
  if ((input_file = ta_fopen_uncompress(file_name.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << file_name << endl;
    
    return false;
  }
  
  // Process the data in the file

  while (!feof(input_file))
  {
    if (!_processData(input_file))
    {
      fclose(input_file);
      
      return false;
    }
  }

  // Close the input file

  fclose(input_file);
  
  return true;
}


/*********************************************************************
 * _processData() - Process the file data. 
 *
 * Returns true on success, false on failure.
 */

bool File2Dsr::_processData(FILE *input_file)
{
  static const string method_name = "File2Dsr::_processData()";
  
  PMU_auto_register("Processing message");

  // Read the next message from the input file

  ui16 input_msg_len;

  if (fread((void *)&input_msg_len, sizeof(input_msg_len), 1,
	    input_file) != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading message length from input file at byte "
	 << ftell(input_file) << endl;
    
    return false;
  }
  
  input_msg_len = BE_to_ui16(input_msg_len);

  if (input_msg_len <= 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Invalid message length in input file: " << input_msg_len << endl;
    
    return false;
  }
  
  ui08 *msg = new ui08[input_msg_len];
  
  if (fread((void *)msg, sizeof(ui08), input_msg_len,
	    input_file) != input_msg_len)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading message from input file" << endl;
    
    return false;
  }
  
  // Disassemble the message

  int contents;

  if (_radarMsg.disassemble((void *)msg, input_msg_len, &contents) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error disassembling message read from input file" << endl;
    
    delete msg;
    
    return false;
  }
  
  delete [] msg;
  
  // Write the message to the output queue after sleeping the
  // specified amount of time.

  umsleep(_beamSleepMsecs);

  if (_params->debug)
  {
    if (contents & DsRadarMsg::RADAR_FLAGS)
    {
      if (_radarMsg.getRadarFlags().startOfTilt)
	cerr << "-----> start of tilt" << endl;
      if (_radarMsg.getRadarFlags().endOfTilt)
	cerr << "-----> end of tilt" << endl;
      if (_radarMsg.getRadarFlags().startOfVolume)
	cerr << "-----> start of volume" << endl;
      if (_radarMsg.getRadarFlags().endOfVolume)
	cerr << "-----> end of volume" << endl;
    }

    if (contents & DsRadarMsg::RADAR_BEAM)
      cerr << "Writing beam: el = " << _radarMsg.getRadarBeam().elevation
	   << ", az = " << _radarMsg.getRadarBeam().azimuth << endl;
  }

  if (_outputQueue.putDsMsg(_radarMsg, contents) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing current message to output FMQ." << endl;
    
    return false;
  }

  return true;
}
