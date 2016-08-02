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
//   $Date: 2016/03/07 01:23:02 $
//   $Id: MitLtg2Spdb.cc,v 1.9 2016/03/07 01:23:02 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MitLtg2Spdb: MitLtg2Spdb program object.
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
#include <string>

#include <toolsa/os_config.h>
#include <rapformats/MitLtg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "MitLtg2Spdb.hh"
#include "Params.hh"
using namespace std;


// Global variables

MitLtg2Spdb *MitLtg2Spdb::_instance =
     (MitLtg2Spdb *)NULL;


/*********************************************************************
 * Constructor
 */

MitLtg2Spdb::MitLtg2Spdb(int argc, char **argv)
{
  static const string method_name = "MitLtg2Spdb::MitLtg2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (MitLtg2Spdb *)NULL);
  
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

MitLtg2Spdb::~MitLtg2Spdb()
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

MitLtg2Spdb *MitLtg2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (MitLtg2Spdb *)NULL)
    new MitLtg2Spdb(argc, argv);
  
  return(_instance);
}

MitLtg2Spdb *MitLtg2Spdb::Inst()
{
  assert(_instance != (MitLtg2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MitLtg2Spdb::init()
{
  static const string method_name = "MitLtg2Spdb::init()";

  // Set the port parameters

  _mitClient.setDebugFlag(_params->debug);
  
  _mitClient.setHostname(_params->mit_hostname);
  _mitClient.setPort(_params->mit_port);
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void MitLtg2Spdb::run()
{
  static const string method_name = "MitLtg2Spdb::run()";
  
//  MemBuf test_buff;
//  ui08 test_byte;
//  
//  test_byte = 0x00;
//  test_buff.load(&test_byte, 1);
//  
//  test_byte = 0x03;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x00;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x01;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x07;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xd2;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x00;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x12;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x00;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x14;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x00;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x35;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x02;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xbc;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x00;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x04;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x2d;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x46;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xff;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xf1;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x9e;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xe5;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xff;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xff;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0xa2;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x36;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x00;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x01;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x04;
//  test_buff.add(&test_byte, 1);
//  
//  test_byte = 0x00;
//  test_buff.add(&test_byte, 1);
//  
//
//  cerr << "*** Test buffer contains " << test_buff.getBufLen() <<
//    " bytes" << endl;
//  
//  MitLtg test_strike;
//  
//  if (!test_strike.disassembleMitMsg(test_buff.getBufPtr(),
//				     test_buff.getBufLen()))
//  {
//    cerr << "ERROR disassembling test buffer" << endl;
//    exit(0);
//  }
//  
//  cerr << "Test strike:" << endl;
//  test_strike.print(cerr, "   ");
//  
//  exit(0);
  
  LtgSpdbBuffer ltg_buffer;
    
  // Process messages forever

  time_t first_strike_time = -1;
  time_t buffer_started_time = time(0);
  
  while (true)
  {
    PMU_auto_register("Waiting for data");
    
    // Get the ltg message from the server.  If there isn't a message,
    // see if we need to write the buffer and continue on.

    if (!_mitClient.getNextMsg(MitClient::LRTC_LGHT_LLP,
			       _params->sleep_seconds))
    {
      if (_params->debug)
	cerr << "---> No messages of type " <<
	  MitClient::LRTC_LGHT_LLP <<
	  " currently available from server" << endl;
      
      // See if we should write the ltg buffer to the database
      
      time_t current_time = time(0);
    
      if (current_time > buffer_started_time + _params->buffer_seconds)
      {
	_writeBufferToDatabase(ltg_buffer);

	buffer_started_time = current_time;
	first_strike_time = -1;
      }
	  
      continue;
    }
    
    // If we get here, we received a ltg strike message

    MitLtg mit_strike;
    
    if (!mit_strike.disassembleMitLlpMsg(_mitClient.getMsgBuffer(),
					 _mitClient.getMsgLen()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error converting MIT ltg message into ltg object" << endl;
      
      continue;
    }
    
    // Convert the strike information to the local format

    LTG_strike_t rap_strike;
    
    rap_strike.time = mit_strike.getUnixTime();
    mit_strike.getLocation(rap_strike.latitude, rap_strike.longitude);
    rap_strike.amplitude = (int)(mit_strike.getStrength() * 1000.0);
    rap_strike.type = LTG_TYPE_UNKNOWN;
    
    if (first_strike_time == -1)
      first_strike_time = rap_strike.time;
    
    // See if we should write the ltg buffer to the database before
    // adding the new strike to the buffer

    time_t current_time = time(0);
    
    if (rap_strike.time > first_strike_time + _params->buffer_seconds)
    {
      _writeBufferToDatabase(ltg_buffer);

      first_strike_time = rap_strike.time;
      buffer_started_time = current_time;
    }
    
    // Print out the new strike information

    if (_params->debug)
    {
      cerr << "Received strike:" << endl;
      mit_strike.print(cerr, "   ");
      cerr << endl;
    }
    
    // Add the new strike to the buffer

    ltg_buffer.addStrike(&rap_strike);
    
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _writeBufferToDatabase() - Write the given lightning buffer to the
 *                            database.  Also cleans out the buffer
 *                            so strikes can be added to it for the
 *                            next write.
 */

void MitLtg2Spdb::_writeBufferToDatabase(LtgSpdbBuffer &ltg_buffer) const
{
  if (ltg_buffer.getNumStrikes() > 0)
  {
    if (_params->debug)
      cerr << "---> Writing ltg buffer to database" << endl;
      
    ltg_buffer.writeToDatabase(_params->output_url,
			       _params->strike_expire_secs,
			       false);
    
    ltg_buffer.clear();
  }
  else
  {
    if (_params->debug)
      cerr << "---> Ltg buffer empty, not writing" << endl;
  }
      
}
