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
///////////////////////////////////////////////////////////////
// RdasSimTest.cc
//
// Mike Dixon
//
// Jan 2003
//
///////////////////////////////////////////////////////////////
//
// RdasSimTest tests out the RdasSim program
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include "RdasSimTest.hh"
#include "RdasBeamSimple.hh"

using namespace std;

// Constructor

RdasSimTest::RdasSimTest(int argc, char **argv)

{
  
  isOK = true;

  // set programe name
  
  _progName = "RdasSimTest";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

RdasSimTest::~RdasSimTest()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RdasSimTest::Run()
{
  
  int iret = 0;
  PMU_auto_register("Run");
  while (true) {
    if (_run()) {
      iret = -1;
    }
    umsleep(1000);
  }

  return iret;

}

int RdasSimTest::_run()
{
  
  // register with procmap
  
  PMU_auto_register("_run");

  // connect to RDAS
  
  PMU_auto_register("Trying for connection");
  if (_params.debug) {
    cerr << "Trying to connect to server .... " << endl;
  }

  if (_rdasSock.open(_params.input_host, _params.input_port, 1000)) {
    cerr << _rdasSock.getErrStr() << endl;
    return -1;
  }
  
  // connection opened, read

  while (true) {
    
    PMU_auto_register("Reading rdas data");
    
    // check for incoming message

    if (_params.debug) {
      cerr << "Checking for incoming message ..." << endl;
    }
    bool messageWaiting = false;
    if (_rdasSock.readSelect(1000)) {
      if (_rdasSock.getErrNum() != Socket::TIMED_OUT) {
	cerr << _rdasSock.getErrStr() << endl;
	_rdasSock.close();
	return -1;
      }
    } else {
      messageWaiting = true;
    }
    
    if (messageWaiting) {
      if (_readBeam()) {
	_rdasSock.close();
	return -1;
      }
    }

    if (_sendCommandsFlag) {
      setSendCommandsFlag(false);
      if (_sendCommands()) {
	_rdasSock.close();
	return -1;
      }
    }
      
  } // while

  _rdasSock.close();
  return 0;
  
}

int RdasSimTest::_sendCommands()

{

  // open the commands file

  FILE *comm;
  if ((comm = fopen(_params.commands_file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RdasSimTest::_sendCommand" << endl;
    cerr << "  Cannot open file: " << _params.commands_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  char line[1024];
  while (!feof(comm)) {

    // read a line
    if (fgets(line, 1024, comm) == NULL) {
      break;
    }

    // ignore comments and blank lines, and commands without an =
    if (line[0] == '#') {
      continue;
    }
    if (strlen(line) == 0) {
      continue;
    }
    if (strchr(line, '=') == NULL) {
      continue;
    }

    // remove line feed

    line[strlen(line)-1] = '\0';
    
    // form the command

    string command("#0#0");
    command += line;

    // send the command

    if (_rdasSock.writeBuffer((void *) command.c_str(), command.size() + 1)) {
      cerr << "ERROR - RdasSimTest::_sendCommands" << endl;
      cerr << "  Failed to write test message" << endl;
      fclose(comm);
      return -1;
    }

  } // while

  fclose(comm);
  return 0;

}

int RdasSimTest::_readBeam()

{

  char buf[2048];
  if (_rdasSock.readBuffer(buf, sizeof(buf))) {
    cerr << "ERROR - RdasSimTest::_readBeam" << endl;
    cerr << "  Failed to read beam" << endl;
    return -1;
  }
  cerr << "Got beam" << endl;
  RdasBeamSimple beam;
  if (beam.disassemble(buf, sizeof(buf))) {
    cerr << "ERROR disassembling beam" << endl;
    return -1;
  }
  cerr << "  Time: " << DateTime::str(beam.getTime()) << endl;
  cerr << "  El: " << beam.getEl() << endl;
  cerr << "  Az: " << beam.getAz() << endl;

  double sumCount = 0.0;
  si16 *counts = beam.getCounts();
  for (int i = 0; i < beam.getNCounts(); i++) {
    sumCount += counts[i];
  }
  double meanCount = sumCount / beam.getNCounts();
  cerr << "  Mean count: " << meanCount << endl;

  return 0;

}

