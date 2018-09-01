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
//
// main for PrintServerMgr
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
///////////////////////////////////////////////////////////////

#include "Args.hh"
#include "Params.hh"
#include <toolsa/str.h>
#include <toolsa/port.h>
#include <toolsa/Socket.hh>

#include <signal.h>
#include <new>
#include <iostream>
#include <cstdlib>

#ifndef _DS_SERVER_MESSAGE_INC_                  
# include <dsserver/DsServerMsg.hh>
#endif

#include <dsserver/DsSvrMgrSocket.hh>
using namespace std;

// file scope

int sendServerCommand(int port, string host, int command, int & replyInt, string & replyString);
static void tidy_and_exit (int sig);
static void out_of_store();
static int _argc;
static char **_argv;

// main

int main(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;

  char   * _progName = NULL;
  char   * _paramsPath = const_cast<char*>(string("unknown").c_str());
  Args   * _args = NULL;
  Params * _params = NULL;
  
  // set programe name
  
  _progName = STRdup("print_server_mgr");

  // get command line args
  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    return 1;
  }

  // get TDRP params
  _params = new Params();
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    return 1;
  }


  // set signal handling
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set new() memory failure handler function
  set_new_handler(out_of_store);

  // Contact the DsServerMgr on the named host and port.
  int status;
  int command = 0;
  int replyInt;
  string replyString;

  cout << "Sending DsServerMsg::IS_ALIVE:" << endl;
  command = DsServerMsg::IS_ALIVE;
  status = sendServerCommand(_params->port, _params->host, command,
                               replyInt, replyString);
  if (status < 0) {
    cerr << "Could not contact server. Exiting." << endl;
    return 1;
  }
  cout << "Yes. " << replyInt << " " << replyString << endl << endl;

  cout << "Sending DsServerMsg::GET_NUM_CLIENTS:" << endl;
  command = DsServerMsg::GET_NUM_CLIENTS;    
  status = sendServerCommand(_params->port, _params->host, command,
                               replyInt, replyString);
  if (status < 0) {
    cerr << "Could not contact server. Exiting." << endl;
    return 1;
  }
  cout << replyInt << " Clients Are Being Handled By The Mgr." << endl << endl;

  // cout << "Sending DsServerMsg::SHUTDOWN:" << endl;
  // command = DsServerMsg::SHUTDOWN;               
  // status = sendServerCommand(_params->port, _params->host, command,
  //                              replyInt, replyString);
  // if (status < 0) {
  //   cerr << "Could not contact server. Exiting." << endl;
  //   return 1;
  // }
  // cout << replyString << endl << endl;

  cout << "Sending DsServerMsg::GET_NUM_SERVERS:" << endl;
  command = DsServerMsg::GET_NUM_SERVERS;        
  status = sendServerCommand(_params->port, _params->host, command,
                               replyInt, replyString);
  if (status < 0) {
    cerr << "Could not contact server. Exiting." << endl;
    return 1;
  }
  cout << replyInt << " Servers Currently Running Under The Mgr." << endl << endl;

  cout << "Sending DsServerMsg::GET_SERVER_INFO:" << endl;
  command = DsServerMsg::GET_SERVER_INFO;  
  status = sendServerCommand(_params->port, _params->host, command,
                               replyInt, replyString);
  if (status < 0) {
    cerr << "Could not contact server. Exiting." << endl;
    return 1;
  }
  cout << replyInt << " Servers Running: " << endl;
  cout << replyString << endl << endl;

  cout << "Sending DsServerMsg::GET_FAILURE_INFO:" << endl;
  command = DsServerMsg::GET_FAILURE_INFO;
  status = sendServerCommand(_params->port, _params->host, command,
                               replyInt, replyString);
  if (status < 0) {
    cerr << "Could not contact server. Exiting." << endl;
    return 1;
  }
  cout << replyInt << " Server Failures So Far: " << endl;
  cout << replyString << endl << endl;

  cout << "Sending DsServerMsg::GET_DENIED_SERVICES:" << endl;
  command = DsServerMsg::GET_DENIED_SERVICES;
  status = sendServerCommand(_params->port, _params->host, command,
                               replyInt, replyString);
  if (status < 0) {
    cerr << "Could not contact server. Exiting." << endl;
    return 1;
  }
  cout << replyInt << " Denied Services: " << endl;
  cout << replyString << endl << endl;

  // clean up
  tidy_and_exit(0);
  return (0);
  
}

///////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{

  exit(sig);

}

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  cerr << "FATAL ERROR - program DsMdvServer" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}


int sendServerCommand(int port, string host, int command, int & replyInt, string & replyString)
{
    Socket * socket = new Socket();
      
    DsServerMsg msg;
    msg.setCategory(DsServerMsg::ServerStatus);
    msg.setType(command);
    void * msgToSend = msg.assemble();
    int msgLen = msg.lengthAssembled();
    
    int status = socket->open(host.c_str(), port);
    if (status != 0) {
      cerr << "   Could not connect to " << host << ":" << port << endl;
      return -1;
    }
    
    status = socket->writeMessage(0, msgToSend, msgLen);
    if (status == 0) {                                              
      // cout << "Socket wrote " << msgLen << " Bytes." << endl;
    }
    else if (status == -1) {
      cout << "Socket write failed. Error Num: "
           << socket->getErrNum() << ". Error String: " << endl;
      cout << socket->getErrString().c_str() << endl;
    }
    else {                   
      cout << "Error: Unknown status returned from writeMessage(): "
           << status << ". Error Num: " << socket->getErrNum()
           << ". Error String: " << endl;
      cout << socket->getErrString().c_str() << endl;
      return -1;
    }
    
    // Read in the reply.                                    
    status = socket->readMessage(); // Block.
    if (status != 0) {
      cout << "Socket read failed. Error Num: "
           << socket->getErrNum() << ". Error String: " << endl;
      cout << socket->getErrString().c_str() << endl;
      return -1;
    }

    const void * data = socket->getData();
    ssize_t dataSize = socket->getNumBytes();

    // cerr << "Client Read " << dataSize << " Bytes." << endl;
    // cerr << "Client decoding server reply..." << endl;

    // Disassemble the incoming message.
    DsServerMsg reply;
    status = reply.disassemble(data, dataSize);
    if (status < 0) {
       cerr << "Error: Could not disassemble message. Either too small or bad "
           << "category." << endl;
      return -1;
    }

    // Determine if this is a server command or a task request.
    bool isServerCommand = false;
    DsServerMsg::category_t category = reply.getMessageCat();
    if (category == DsServerMsg::ServerStatus) {
      isServerCommand = true;
    }
    else {
      cerr << "Error: Server replied to server status command with a generic message." << endl;
      return -1;
    }
    cout << "isServerCommand: " << isServerCommand << endl;

    replyInt = reply.getFirstInt();
    replyString = reply.getFirstString();

    delete socket;

    return 0;
}

