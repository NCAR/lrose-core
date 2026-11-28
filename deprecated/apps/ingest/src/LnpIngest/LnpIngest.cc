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
//   $Id: LnpIngest.cc,v 1.5 2016/03/07 01:23:02 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LnpIngest: LnpIngest program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "LnpIngest.hh"
#include "LtgStrike.hh"
#include "Params.hh"

using namespace std;

// Global variables

LnpIngest *LnpIngest::_instance =
     (LnpIngest *)NULL;

const unsigned char LnpIngest::MSG_BOP = 0x7d;
const unsigned char LnpIngest::MSG_EOP = 0x7e;
const unsigned char LnpIngest::MSG_DLE = 0x10;

const unsigned char LnpIngest::MSG_STROKE_FLAG = 0x97;

const int LnpIngest::SECS_PER_DAY = 24 * 60 * 60;
const int LnpIngest::SECS_PER_HOUR = 60 * 60;
const int LnpIngest::SECS_PER_MINUTE = 60;


/*********************************************************************
 * Constructor
 */

LnpIngest::LnpIngest(int argc, char **argv) :
  _lnpSockFd(-1)
{
  static const string method_name = "LnpIngest::LnpIngest()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (LnpIngest *)NULL);
  
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

LnpIngest::~LnpIngest()
{
  // Unregister process

  PMU_auto_unregister();

  // Close any open sockets

  _closeSocket();
  
  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

LnpIngest *LnpIngest::Inst(int argc, char **argv)
{
  if (_instance == (LnpIngest *)NULL)
    new LnpIngest(argc, argv);
  
  return(_instance);
}

LnpIngest *LnpIngest::Inst()
{
  assert(_instance != (LnpIngest *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool LnpIngest::init()
{
  static const string method_name = "LnpIngest::init()";
  
  // Calculate the unix time for the LNP reference date.

  DateTime reference_date(1980, 2, 29);
  _refTime = reference_date.utime();
  
  // Set the output directory

  if (!_writer.init(_params->output_dir,
		    _params->seconds_per_file))
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void LnpIngest::run()
{
  static const string method_name = "LnpIngest::run()";
  
  // Just process data from the socket forever

  while (true)
    _processMsg();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _closeSocket() - Close the LNP socket
 */

void LnpIngest::_closeSocket()
{
  if (_lnpSockFd >= 0)
  {
    SKU_close(_lnpSockFd);
    _lnpSockFd = -1;
  }
}


/*********************************************************************
 * _fillMsg() - Fill the given message buffer with the next message
 *              available.
 *
 * Returns true on success, false on failure.
 */

bool LnpIngest::_fillMsg(MemBuf &msg)
{
  static const string method_name = "LnpIngest::_fillMsg()";

  PMU_auto_register("Filling message buffer");
  
  bool bop_found = false;
  bool eop_found = false;

  // Keep reading in data and checking the bytes until we have a full message

  while (true)
  {
    // Check the previously read buffer for the beginning of a new message

    PMU_auto_register("Checking for new message");
    int avail_len = _availBytes.getLen();
    unsigned char *avail_ptr = (unsigned char *)_availBytes.getPtr();
    if (avail_len > 0)
    {
      int bop_index;
    
      for (bop_index = 0; bop_index < avail_len; ++bop_index)
      {
    	if (avail_ptr[bop_index] == MSG_BOP &&
    	    (bop_index == 0 || avail_ptr[bop_index-1] != MSG_DLE))
	{
	  bop_found = true;
	  break;
	}
      } /* endfor - bop_index */
      int eop_index;
    
      if (bop_found)
      {
	// See if we already have the eop
	for (eop_index = bop_index+1; eop_index < avail_len; ++eop_index)
	{
	  if (avail_ptr[eop_index] == MSG_EOP)
	  {
	      int DLE_count = 0;
	      for(int step_back = eop_index - 1;step_back > bop_index+1;--step_back)
	      {
		  if(avail_ptr[step_back] == MSG_DLE)
		      DLE_count++;
		  else
		      break;
	      }
	      if(DLE_count % 2 == 0 )
	      {
		  eop_found = true;
		  break;
	      }
	  }
	} /* endfor - eop_index */
      
	if (eop_found)
	{
	  msg.load(avail_ptr, eop_index+1);
	  _availBytes.load(&(avail_ptr[eop_index+1]),
			   avail_len - eop_index - 1);
	  _removeDLE(msg);
	  return true;
	}
	else
	{
	  msg.load(avail_ptr, avail_len);
	} /* endif - eop_found */
      
      }
      else
      {
	_availBytes.reset();
      } /* endif - bop_found */
    
    } /* endif - avail_len > 0 */
  
    // Make sure the socket is open

    if (! _openSocket() )
      return false;

    // Read the available data from the socket and add it to the memory buffer

    unsigned char buffer[BUFSIZ];
    
    int bytes_read = read(_lnpSockFd, buffer, BUFSIZ);
    _availBytes.add(buffer, bytes_read);
    
  } /* endwhile - true */
  
  return false;
}


/*********************************************************************
 * _openSocket() - Open the LNP socket
 *
 * Returns true on success, false on failure.
 */

bool LnpIngest::_openSocket()
{
  static const string method_name = "LnpIngest::_openSocket()";
  
  PMU_auto_register("_openSocket");

  // Close the socket if it's already open

  _closeSocket();
  
  // Now reopen the socket

  _lnpSockFd = SKU_open_client(_params->lnp_hostname, _params->lnp_port);
  switch (_lnpSockFd)
  {
  case -1 :
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not find LNP host name: " << _params->lnp_hostname << endl;
    return false;

  case -2 :
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not setup socket (max file descriptors?)" << endl;
    return false;

  case -3 :
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not connect to specified host and port:" << endl;
    cerr << "   Host: " << _params->lnp_hostname << endl;
    cerr << "   Port: " << _params->lnp_port << endl;
    return false;

  default:
    // Everything is okay
    cerr << "Connected to socket" << endl;
    break;
  }

  return true;
}


/*********************************************************************
 * _processMsg() - Process the next message available on the socket
 *
 * Returns true on success, false on failure.
 */

bool LnpIngest::_processMsg()
{
  static const string method_name = "LnpIngest::_processMsg()";
  
  PMU_auto_register("Processing data");
  
  if (_params->debug)
    cerr << endl;
  
  MemBuf msg;
  

  if (!_fillMsg(msg))
    return false;

  if (_params->debug)
  {
    cerr << "Full message found: ";
    _printMsg(msg);
  }
  
  // We only process stroke messages so can throw any others away.
  // Return true in this case because we successfully processed the
  // message.

  unsigned char *msg_ptr = (unsigned char *)msg.getPtr();

  if (msg_ptr[1] != MSG_STROKE_FLAG)
  {
    if (_params->debug)
	cerr << "   Skipping non-stroke message...." << endl;

    return true;
  }
  
  if (!_processStrokeMsg(msg))
    return false;
  
  return true;
}


/*********************************************************************
 * _processStrokeMsg() - Process the given stroke message
 *
 * Returns true on success, false on failure.
 */

bool LnpIngest::_processStrokeMsg(const MemBuf &msg)
{
  static const string method_name = "LnpIngest::_processStrokeMsg()";
  
  if (_params->debug)
    cerr << "   Processing stroke message..." << endl;
  
  unsigned char *msg_ptr = (unsigned char *)msg.getPtr();
  int msg_len = msg.getLen();
  
  // Skip the BOP and the msg flag

  msg_ptr += 2;
  msg_len -= 2;
  
  // Extract the tdate elements

  DateTime strike_time(_extractTdate(*(ui32 *)msg_ptr));
  msg_ptr += 4;
  msg_len -= 4;
  
  if (_params->debug)
    cerr << "      strike time = " << strike_time << endl;
  
  // Extract each of the strikes.  We are accounting for the trailing
  // check sum and EOP bytes at the end of the message when calculating the
  // number of strikes.

  int num_strikes = (msg_len - 2) / 8;
  
  if (_params->debug)
    cerr << "      Processing " << num_strikes << " from "
	 << msg_len << " bytes" << endl;
  
  PMU_auto_register("Processing each stroke");

  for (int i = 0; i < num_strikes; ++i)
  {
    PMU_auto_register("In stroke processing loop");

    double latitude, longitude, amplitude;
    bool cloud_to_ground_flag, qa_flag;
    int multiplicity;
    double seconds_fraction;
    
    _extractStrokeData(*(ui32 *)msg_ptr, *(ui32 *)(msg_ptr + 4),
		       latitude, longitude, amplitude,
		       cloud_to_ground_flag, multiplicity,
		       seconds_fraction, qa_flag);
    msg_ptr += 8;
    msg_len -= 8;

    LtgStrike strike;
    
    strike.setTime(strike_time, (int)(seconds_fraction * 1000000000.0));
    strike.setLocation(latitude, longitude);
    strike.setAmplitude(amplitude);
    strike.setMultiplicity(multiplicity);
    if (cloud_to_ground_flag)
      strike.setDischargeType(LtgStrike::CLOUD_TO_GROUND_DISCHARGE);
    else
      strike.setDischargeType(LtgStrike::CLOUD_DISCHARGE);
    
    if (_params->debug)
	strike.print(cerr);

    _writer.writeStrike(strike);
  }
  
  return false;
}
