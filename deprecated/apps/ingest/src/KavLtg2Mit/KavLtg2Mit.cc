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
//   $Date: 2016/03/07 01:23:01 $
//   $Id: KavLtg2Mit.cc,v 1.6 2016/03/07 01:23:01 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * KavLtg2Mit: KavLtg2Mit program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <iostream>
#include <signal.h>
#include <cstdlib>
#include <string>

#include <toolsa/os_config.h>
#include <rapformats/KavLtgFile.hh>
#include <rapformats/MitLtg.hh>
#include <toolsa/InputDir.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "KavLtg2Mit.hh"
#include "Params.hh"
using namespace std;


// Global variables

KavLtg2Mit *KavLtg2Mit::_instance =
     (KavLtg2Mit *)NULL;


/*********************************************************************
 * Constructor
 */

KavLtg2Mit::KavLtg2Mit(int argc, char **argv)
{
  static const string method_name = "KavLtg2Mit::KavLtg2Mit()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (KavLtg2Mit *)NULL);
  
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

  PMU_auto_init(_progName, _params->instance, PROCMAP_REGISTER_INTERVAL);

}


/*********************************************************************
 * Destructor
 */

KavLtg2Mit::~KavLtg2Mit()
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

KavLtg2Mit *KavLtg2Mit::Inst(int argc, char **argv)
{
  if (_instance == (KavLtg2Mit *)NULL)
    new KavLtg2Mit(argc, argv);
  
  return(_instance);
}

KavLtg2Mit *KavLtg2Mit::Inst()
{
  assert(_instance != (KavLtg2Mit *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool KavLtg2Mit::init()
{
  static const string method_name = "KavLtg2Mit::init()";
  
  if (_mitServer.openServer(_params->output_port) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error openning server socket" << endl;
    cerr << _mitServer.getErrStr() << endl;
    
    return false;
  }
  
  // See if there are any clients waiting to connect

  _mitServer.getClients();
  
  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void KavLtg2Mit::run()
{
  static const string method_name = "KavLtg2Mit::run()";

  // Create the objects controlling the Kavouras ltg files

  KavLtgFile ltg_file(_params->debug);
  InputDir input_dir(_params->input_dir,
		     _params->input_substring,
		     false);
  
  // Process any new files

  while (true)
  {
    // Register with the process mapper

    PMU_auto_register("Checking for new files");
    
    // See if there are any clients waiting to connect

    _mitServer.getClients();
    
    // Process any new files

    char *input_filename;
    
    while ((input_filename = input_dir.getNextFilename(TRUE)) != NULL)
    {
      char procmap_string[BUFSIZ];
      
      // Register with the process mapper

      sprintf(procmap_string, "Processing file <%s>", input_filename);
      PMU_force_register(procmap_string);
      
      if (!ltg_file.loadFile(input_filename, _params->processing_delay))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error loading Kavouras data from file <" << input_filename <<
	  ">" << endl;
	cerr << "   *** SKIPPING FILE ***" << endl;
	
	continue;
      }
      
      // Process the data.  Note that KAVLTG_read_strike() returns
      // a pointer to static memory that should NOT be freed.

      KAVLTG_strike_t *strike_kav;
      int strike_count = 0;
      
      while ((strike_kav = ltg_file.getNextStrike()) != NULL)
      {
	// Create the MIT format strike

	MitLtg strike_mit(*strike_kav);
	
	if (_params->debug)
	{
	  cerr << "New strike:" << endl;
	  strike_mit.print(cerr, "   ");
	  cerr << endl;
	}
	
	// Create a message with the MIT strike

	strike_mit.assembleMitMsg();
	
	// Send the message to all clients

	_mitServer.sendMsgToAllClients(MitServer::LRTC_LGHT_ARSI,
				       strike_mit.getMsgBuffer(),
				       strike_mit.getMsgBufferLen());
	
	// Increment the strike count

	++strike_count;

      } /* endwhile - strike_kav != NULL */
      
      if (_params->debug)
	cerr << "Found " << strike_count << " strikes in input file <" <<
	  input_filename << ">" << endl;
      
      // Close the input file

      ltg_file.close();
	
    } /* endwhile - input_filename != NULL */
    
    _sendTestStatusMsg();
    
    sleep(_params->sleep_seconds);
    
  } /* endwhile - true */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _sendTestStatusMsg() - Send a test status msg to the client.  The
 *                        client should ignore this message.
 */

void KavLtg2Mit::_sendTestStatusMsg()
{
  char *status_msg;
  
  int status_msg_len = 10 + (int)(50.0 * rand() / (RAND_MAX + 1.0));
  
  status_msg = new char[status_msg_len];
  
  for (int i = 0; i < status_msg_len; ++i)
    status_msg[i] = i;
  
  _mitServer.sendMsgToAllClients(MitServer::LRTC_LGHT_STATUS,
				 status_msg,
				 status_msg_len);
	
  delete status_msg;
}
