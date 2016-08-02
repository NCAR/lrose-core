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
// DsServerQuery.cc
//
// DsServerQuery object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////
//
// DsServerQuery queries a DsServer for status etc.
//
///////////////////////////////////////////////////////////////

#include "DsServerQuery.hh"
#include "Args.hh"
#include <dsserver/DsLocator.hh>
#include <dsserver/DsServerMsg.hh>
#include <didss/DsURL.hh>
#include <toolsa/ThreadSocket.hh>
using namespace std;

// Constructor

DsServerQuery::DsServerQuery(int argc, char **argv)

{

  // initialize

  OK = true;
  
  // set programe name
  
  _progName = "DsServerQuery";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Problem with command line args" << endl;
    OK = false;
    return;
  }

  return;

}

// destructor

DsServerQuery::~DsServerQuery()

{

}

//////////////////////////////////////////////////
// Run

int DsServerQuery::Run()
{

  int iret = 0;
  DsURL url(_args.urlStr);

  bool contactServer;
  if (DsLocator.resolve(url, &contactServer, _args.useMgr)) {
    cerr << "ERROR - COMM -DsServerQuery::Run" << endl;
    cerr << "  Cannot resolve url: " << _args.urlStr << endl;
    return -1;
  }

  cout << endl << "Querying server at url: " << _args.urlStr << endl;

  // isAlive

  if (_args.isAlive) {

    ThreadSocket sock;
    if (sock.open(url.getHost().c_str(), url.getPort())) {

      cerr << "ERROR - COMM -DsServerQuery::Run" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      iret = -1;

    } else {
  
      cout << endl;
      cout << "  Checking for isAlive" << endl;
      DsServerMsg msg;
      msg.setCategory(DsServerMsg::ServerStatus);
      msg.setType(DsServerMsg::IS_ALIVE);
      void * msgToSend = msg.assemble();
      int msgLen = msg.lengthAssembled();
      int wait_msecs = 2000;
      
      if (sock.writeMessage(0, msgToSend, msgLen, wait_msecs) == 0) {
	// Read in the reply.
	if (sock.readMessage(wait_msecs) == 0) {
	  cout << "    Server is alive" << endl;
	} else {
	  cerr << "  ERROR - COMM - on sock.readMessage, isAlive" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
      } else {
	cerr << "  ERROR on sock.writeMessage, isAlive" << endl;
	cerr << "  " << sock.getErrStr() << endl;
      }

    }

    sock.close();
  
  }

  // num clients
  
  if (_args.getNumClients) {
   
    cout << endl;
    cout << "  Checking for getNumClients" << endl;

    ThreadSocket sock;
    if (sock.open(url.getHost().c_str(), url.getPort())) {
      
      cerr << "ERROR - COMM -DsServerQuery::Run" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      iret = -1;

    } else {
      
      DsServerMsg msg;
      msg.setCategory(DsServerMsg::ServerStatus);
      msg.setType(DsServerMsg::GET_NUM_CLIENTS);
      void * msgToSend = msg.assemble();
      int msgLen = msg.lengthAssembled();
      int wait_msecs = 2000;
      
      if (sock.writeMessage(0, msgToSend, msgLen, wait_msecs) == 0) {
	// Read in the reply.
	if (sock.readMessage(wait_msecs) == 0) {
	  int nClients = msg.getFirstInt();
	  cout << "    N clients: " << nClients << endl;
	} else {
	  cerr << "  ERROR - COMM - on sock.readMessage" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
      } else {
	cerr << "  ERROR on sock.writeMessage" << endl;
	cerr << "  " << sock.getErrStr() << endl;
      }
    }

    sock.close();

  }
  
  // num servers
  
  if (_args.getNumServers) {
   
    cout << endl;
    cout << "  Checking for getNumServers" << endl;

    ThreadSocket sock;
    if (sock.open(url.getHost().c_str(), url.getPort())) {
      
      cerr << "ERROR - COMM - DsServerQuery::Run" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      iret = -1;

    } else {
      
      DsServerMsg msg;
      msg.setCategory(DsServerMsg::ServerStatus);
      msg.setType(DsServerMsg::GET_NUM_SERVERS);
      void * msgToSend = msg.assemble();
      int msgLen = msg.lengthAssembled();
      int wait_msecs = 2000;
      
      if (sock.writeMessage(0, msgToSend, msgLen, wait_msecs) == 0) {
	// Read in the reply.
	if (sock.readMessage(wait_msecs) == 0) {
	  int nServers = msg.getFirstInt();
	  cout << "    N servers: " << nServers << endl;
	} else {
	  cerr << "  ERROR - COMM - on sock.readMessage" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
      } else {
	cerr << "  ERROR on sock.writeMessage" << endl;
	cerr << "  " << sock.getErrStr() << endl;
      }
    }

    sock.close();

  }
  
  // server info
  
  if (_args.getServerInfo) {
   
    cout << endl;
    cout << "  Checking for ServerInfo" << endl;

    ThreadSocket sock;
    if (sock.open(url.getHost().c_str(), url.getPort())) {

      cerr << "ERROR - COMM - DsServerQuery::Run" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      iret = -1;

    } else {
  
      DsServerMsg msg;
      msg.setCategory(DsServerMsg::ServerStatus);
      msg.setType(DsServerMsg::GET_SERVER_INFO);
      void * msgToSend = msg.assemble();
      int msgLen = msg.lengthAssembled();
      int wait_msecs = 2000;
      
      if (sock.writeMessage(0, msgToSend, msgLen, wait_msecs) == 0) {
	// Read in the reply.
	if (sock.readMessage(wait_msecs) == 0) {
	  int nServers = msg.getFirstInt();
	  string info = msg.getFirstString();
	  cout << "    N servers: " << nServers << endl;
	  cout << "    Server info: " << endl;
	  cout << info << endl;
	} else {
	  cerr << "  ERROR - COMM - on sock.readMessage" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
      } else {
	cerr << "  ERROR on sock.writeMessage" << endl;
	cerr << "  " << sock.getErrStr() << endl;
      }
    }

    sock.close();

  }
  
  // failure info
  
  if (_args.getFailureInfo) {
   
    cout << endl;
    cout << "  Checking for getFailureInfo" << endl;
    
    ThreadSocket sock;
    if (sock.open(url.getHost().c_str(), url.getPort())) {

      cerr << "ERROR - COMM - DsServerQuery::Run" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      iret = -1;

    } else {
  
      DsServerMsg msg;
      msg.setCategory(DsServerMsg::ServerStatus);
      msg.setType(DsServerMsg::GET_FAILURE_INFO);
      void * msgToSend = msg.assemble();
      int msgLen = msg.lengthAssembled();
      int wait_msecs = 2000;
      
      if (sock.writeMessage(0, msgToSend, msgLen, wait_msecs) == 0) {
	// Read in the reply.
	if (sock.readMessage(wait_msecs) == 0) {
	  int nFailure = msg.getFirstInt();
	  string info = msg.getFirstString();
	  cout << "    N failures: " << nFailure << endl;
	  cout << "    Failure info: " << endl;
	  cout << info << endl;
	} else {
	  cerr << "  ERROR - COMM - on sock.readMessage" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
      } else {
	cerr << "  ERROR on sock.writeMessage" << endl;
	cerr << "  " << sock.getErrStr() << endl;
      }
    }

    sock.close();

  }
  
  // denied services
  
  if (_args.getDeniedServices) {
   
    cout << endl;
    cout << "  Checking for getDeniedServices" << endl;

    ThreadSocket sock;
    if (sock.open(url.getHost().c_str(), url.getPort())) {

      cerr << "ERROR - COMM - DsServerQuery::Run" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      iret = -1;

    } else {
  
      DsServerMsg msg;
      msg.setCategory(DsServerMsg::ServerStatus);
      msg.setType(DsServerMsg::GET_DENIED_SERVICES);
      void * msgToSend = msg.assemble();
      int msgLen = msg.lengthAssembled();
      int wait_msecs = 2000;
      
      if (sock.writeMessage(0, msgToSend, msgLen, wait_msecs) == 0) {
	// Read in the reply.
	if (sock.readMessage(wait_msecs) == 0) {
	  int nDenied = msg.getFirstInt();
	  string denied = msg.getFirstString();
	  cout << "    N denied services: " << nDenied << endl;
	  cout << "    Denied services: " << endl;
	  cout << denied << endl;
	} else {
	  cerr << "  ERROR - COMM - on sock.readMessage" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
      } else {
	cerr << "  ERROR on sock.writeMessage" << endl;
	cerr << "  " << sock.getErrStr() << endl;
      }
    }

    sock.close();

  }
  
  if (_args.shutDown) {

    ThreadSocket sock;
    if (sock.open(url.getHost().c_str(), url.getPort())) {

      cerr << "ERROR - COMM - DsServerQuery::Run" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      iret = -1;

    } else {
  
      cout << endl;
      cout << "  Signalling shutdown" << endl;
      DsServerMsg msg;
      msg.setCategory(DsServerMsg::ServerStatus);
      msg.setType(DsServerMsg::SHUTDOWN);
      void * msgToSend = msg.assemble();
      int msgLen = msg.lengthAssembled();
      int wait_msecs = 2000;
      
      if (sock.writeMessage(0, msgToSend, msgLen, wait_msecs) == 0) {
	// Read in the reply.
	if (sock.readMessage(wait_msecs) == 0) {
	  cout << "    Server will shut down" << endl;
	} else {
	  cerr << "  ERROR - COMM - on sock.readMessage" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
      } else {
	cerr << "  ERROR on sock.writeMessage" << endl;
	cerr << "  " << sock.getErrStr() << endl;
      }

    }

    sock.close();
  
  }

  return iret;

}

