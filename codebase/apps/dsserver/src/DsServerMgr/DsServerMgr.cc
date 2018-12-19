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
#include <iostream>
#include <sys/wait.h>

#include <DsServerMgr.hh>

#include <toolsa/TaStr.hh>
#include <toolsa/GetHost.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Socket.hh>
#include <toolsa/ServerSocket.hh>

#include <didss/DsMsgPart.hh>
#include <dsserver/DsLocator.hh>
using namespace std;

// constructor

DsServerMgr::DsServerMgr(string executableName,
                         string instanceName,
                         int port,
                         int childMaxQuiescentSecs, // Used for Sub-Servers
                         int maxClients,
                         bool isDebug,
                         bool isVerbose,
			 bool isSecure,
                         bool mdvReadOnly,
                         bool spdbReadOnly)
  : DsThreadedServer(executableName,
		     instanceName,
		     port,
		     -1,   // This server NEVER killed for quiescence.
		     maxClients,
		     isDebug,
		     isVerbose,
		     isSecure),
    _childMaxQuiescentSecs(childMaxQuiescentSecs),
    _mdvReadOnly(mdvReadOnly),
    _spdbReadOnly(spdbReadOnly)

{

  // Set default member values

  _pingTimeoutMsecs = DS_DEFAULT_PING_TIMEOUT_MSECS;
  _commTimeoutMsecs = DS_DEFAULT_COMM_TIMEOUT_MSECS;
   
  // If base class's constructor failed, bail.
  if (!_isOkay) {
    return;
  }

  // initialize mutexes

  pthread_mutex_init(&_pendingMutex, NULL);

  // set ping and comm timeouts
  
  char *DS_PING_TIMEOUT_MSECS = getenv("DS_PING_TIMEOUT_MSECS");
  if (DS_PING_TIMEOUT_MSECS != NULL) {
    int timeout;
    if (sscanf(DS_PING_TIMEOUT_MSECS, "%d", &timeout) == 1) {
      _pingTimeoutMsecs = timeout;
    }
  }

  char *DS_COMM_TIMEOUT_MSECS = getenv("DS_COMM_TIMEOUT_MSECS");
  if (DS_COMM_TIMEOUT_MSECS != NULL) {
    int timeout;
    if (sscanf(DS_COMM_TIMEOUT_MSECS, "%d", &timeout) == 1) {
      _commTimeoutMsecs = timeout;
    }
  }

}

// destructor - virtual

DsServerMgr::~DsServerMgr()
{

}

//////////////////////////////////////////////////////    
// Overload handleDataCommand()

int DsServerMgr::handleDataCommand(Socket * socket,
                                   const void * data,
				   ssize_t dataSize)
{

  // always return success otherwise the program will exit

  string errStr = "ERROR - DsServerMgr::handleDataCommand\n";

  if (_isVerbose) {
    cerr << "================= start handleDataCommand ==============" << endl;
  }

  // disassemble message

  DsServerMsg msg;
  if (_isVerbose) {
    cerr << "Disassembling message..." << endl;
  }
  if (msg.disassemble(data, dataSize)) {
    errStr += "  Could not disassemble DsServerMessage.";
    if (_isDebug) {                                       
      cerr << errStr << endl;
    }
    if (sendReply(socket, DsServerMsg::BAD_MESSAGE, errStr)) {
      cerr << "Error in DsServerMgr::handleDataCommand()" << endl;
    }
    return 0;
  }

  // get URL
  
  string urlStr = msg.getFirstURLStr();
  if (urlStr.size() == 0) {
    errStr += "  Data command has no URL.";
    if (_isDebug) {
      cerr << errStr << endl;
    }
    if (sendReply(socket, DsServerMsg::BAD_MESSAGE, errStr)) {
      cerr << "Error in DsServerMgr::handleDataCommand()" << endl;
    }
    return 0;
  }
  if (_isVerbose) {
    cerr << "URL: " << urlStr << endl;
  }

  // handle URL, starting server as appropriate
  
  DsURL url(urlStr);
  DsServerMsg::msgErr errCode = DsServerMsg::DSS_MSG_SUCCESS;

  if (handleURL(socket, url, errCode, errStr) == 0) {
    
    // success
    
    if (sendReply(socket, url, DsServerMsg::DSS_MSG_SUCCESS, "")) {
      cerr << "ERROR - COMM -DsServerMgr::handleDataCommand()" << endl;
    }
    
  } else {
    
    // failure
    
    if (_isDebug) {
      cerr << errStr << endl;
    }
    
    if (sendReply(socket, url, errCode, errStr)) {
      cerr << "ERROR - COMM -DsServerMgr::handleDataCommand()" << endl;
    }
    
  } // if (handleURL ...
  
  if (_isVerbose) {
    cerr << "============== end handleDataCommand ===================" << endl;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Overload handleServerCommand()

int DsServerMgr::handleServerCommand(Socket * socket,
                                     const void * data,
				     ssize_t dataSize)

{

  // always return success otherwise the program will exit

  string errStr = "ERROR - DsServerMgr::handleServerCommand\n";

  if (_isVerbose) {
    cerr << "DsServerMgr::handleServerCommand()" << endl;
  }

  // get message and disassemble
  
  DsServerMsg msg;
  if (_isVerbose) {
    cerr << "Disassembling message" << endl;
  }
  if (msg.disassemble(data, dataSize)) {
    errStr += "Could not disassemble DsServerMessage.";
    if (_isDebug) {
      cerr << errStr << endl;
    }
    if (sendReply(socket, DsServerMsg::BAD_MESSAGE, errStr)) {
      cerr << "Error in DsServerMgr::handleServerCommand()" << endl;
    }
    return 0;
  }

  // Check that this is a server command, as DsServer determined it is.

  if (msg.getMessageCat() != DsServerMsg::ServerStatus) {
    errStr += "Message is not a server command message.";
    if (_isDebug) {
      cerr << errStr << endl;
    }
    if (sendReply(socket, DsServerMsg::SERVER_ERROR, errStr)) {
      cerr << "ERROR - COMM - error in DsServerMgr::handleServerCommand()" << endl;
    }
    return 0;
  }

  // Handle special server commands

  bool handledHere = true;
  msg.clearParts();
  int command = msg.getType();

  switch (command) {

  case DsServerMsg::GET_NUM_SERVERS:
    {
      msg.addInt(-1);
      break;
    }
  
  case DsServerMsg::GET_SERVER_INFO:
    {
      msg.addInt(-1);
      msg.addString("Server info not available");
      break;
    }

  case DsServerMsg::GET_FAILURE_INFO:
    {
      msg.addInt(0);
      msg.addString("Server Failures now logged to stderr.");
      break;
    }

  case DsServerMsg::GET_DENIED_SERVICES:
    {
      msg.addInt(0);
      msg.addString("Denied Services No Longer Supported.");
      break;
    }

  case DsServerMsg::SHUTDOWN:
    {
      msg.addString("Cannot SHUTDOWN the DsServerMgr remotely.");
      break;
    }

  default:
    handledHere = false;
    break;

  }

  if (handledHere) {
    
    if (_isVerbose) {
      cerr <<  "Replying to message ..." << endl;
    }
    
    // Send the message reply.
    void *msgToSend = msg.assemble();
    int msgLen = msg.lengthAssembled();
    if (socket->writeMessage(0, msgToSend,
			     msgLen, _commTimeoutMsecs)) {
      cerr << "ERROR - handleServerCommand" << endl;
      cerr << socket->getErrStr() << endl;
    }
    
    return 0;

  }

  // If the command was not one recognized by this subclass, ask the
  // base class to handle it.
  
  if (_isVerbose) {
    cerr << "Forwarding message to DsServer class ..." << endl;
  }
  
  return DsThreadedServer::handleServerCommand(socket, data, dataSize);
  
}

///////////////////////////////////
// overload timeoutMethod()

bool DsServerMgr::timeoutMethod()
{

  reapChildren();
  
  // Call the base class method for PMU registration.

  bool cont = DsThreadedServer::timeoutMethod();
  if (!cont) {
    return false;
  }

  return true; // Continue to wait for clients.

}

///////////////////////////////////
// overload postHandlerMethod()

bool DsServerMgr::postHandlerMethod()
{

  if (_isVerbose) {
    cerr << "In DsServerMgr::postHandlerMethod()." << endl;
  }
  
  reapChildren();
  
  // Call the base class method for PMU registration.

  bool cont = DsThreadedServer::postHandlerMethod();
  if (!cont) {
    return false;
  }
  
  return true; // Continue to wait for clients.

}

///////////////////////////////////
// overload exitMethod()

bool DsServerMgr::exitMethod()
{

  reapChildren();
  
  // Call the base class method for PMU de-registration.

  bool cont = DsThreadedServer::exitMethod();
  if (!cont) {
    return false;
  }
  
  cerr << endl << "In DsServerMgr::exitMethod(). " << endl
       << "DsServerMgr should NEVER exit because of quiescence." << endl;
  cerr << endl;
  
  return false; // Do not continue with the exit!

}

/////////////////////////////////////////////////////////////////////////
// handle URL, starting server as appropriate
//
// Returns 0 on success, -1 on failure
//
// On failure, fills out errCode and errStr

int DsServerMgr::handleURL(Socket * socket,
			   DsURL & url,
			   DsServerMsg::msgErr &errCode,
			   string & errStr)

{

  errStr += "  ERROR - DsServerMgr::handleURL\n";
  
  // Find the server executable for the url.
  
  string executableName;
  if (DsLocator.getServerName(url, executableName)) {
    errStr += "  DsLocator cannot identify a server for URL\n";
    errCode = DsServerMsg::NO_SERVICE_AVAIL;
    return -1;
  }

  if (_isVerbose) {
    cerr << "DsLocator found executable " << executableName
	 << " for protocol " << url.getProtocol() << endl;
  }
  
  // Determine the port that the executable should be running on.

  int port = DsLocator.getDefaultPort(executableName);

  // If the client provided a port, verify it's correct.

  if (url.getPort() > 0 && url.getPort() != port) {
    if (_isDebug) {
      cerr << "Port sent with URL: " << url.getPort()
	   << " not correct." << endl;
      cerr << "Port reset to: " << port << endl;
    }
  }
  url.setPort(port);

  // Test if server is already running
  
  if (pingServer(executableName, port) == 0) {
    if (_isVerbose) {
      cerr << "Server: " << executableName
	   << ", port: " << port
	   << " already running" << endl;
    }
    return 0;
  }

  // check if a request for this server is pending.
  // if so, try  to connect to the server, assuming it will be started by
  // another thread

  if (isPending(executableName)) {
    if (waitForServer(executableName, port, 10) == 0) {
      if (_isVerbose) {
	cerr << "Server: " << executableName
	     << ", port: " << port
	     << " started by another thread" << endl;
      }
      return 0;
    }
  }

  // Need to start a new server.
  
  if (_isVerbose) {
    cerr << "No server found running for URL request. " << endl;
    cerr << "Must start a new server" << endl;
  }
  
  // add to pending list

  addPending(executableName);
  
  // spawn server
  
  if (spawnServer(executableName, port, socket, errStr)) {
    errCode = DsServerMsg::SERVICE_DENIED;
    removePending(executableName);
    return -1;
  }

  // The server has been started. Verify it is running.

  if (_isVerbose) {
    cerr << "Server has been started. Ping it as a check." << endl;
  }
  
  if (waitForServer(executableName, port, 5)) {
    TaStr::AddStr(errStr," Could not start new server to handle URL: ",
		  url.getURLStr());
    TaStr::AddStr(errStr, "Server name: ", executableName);
    errCode = DsServerMsg::SERVER_ERROR;
    removePending(executableName);
    return -1;
  } else {
    if (_isVerbose) {
      cerr << "Server: " << executableName
	   << ", port: " << port
	   << " started" << endl;
    }
  }

  // remove from pending list

  removePending(executableName);

  // Success

  return 0;
  
}

////////////////////////////////////////////////////////////
// Reap the children as they die - called by boss thread
// in the timeout and post-handler method

void DsServerMgr::reapChildren()
{

  int pid = 0;
  int pidstatus;
  while ( ( pid = waitpid((pid_t) -1,
			  &pidstatus,
			  (int)(WNOHANG | WUNTRACED)) ) > 0) {
    
    if (_isVerbose) {
      cerr << "DsServerMgr::reapChildren - Reaped child with pid: "
	   << pid << endl;
    }

    int status = WIFEXITED(pidstatus);
    if (status != 0 && WEXITSTATUS(pidstatus) != 0) {
      // The child exited abnormally.
      cerr << "WARNING: DsServerMgr reaped child with pid: "
	   << pid << " which had a BAD EXIT. " << endl;
    }
    
    status = WIFSIGNALED(pidstatus);
    if (status != 0) {
      // The child exited abnormally.
      cerr << "WARNING: DsServerMgr reaped child with pid: "
	   << pid << " which had a BAD SIGNAL. " << endl;
    }
    
    status = WIFSTOPPED(pidstatus);
    if (status != 0) {
      // The child exited abnormally.
      cerr << "WARNING: DsServerMgr reaped child with pid: "
	   << pid << " which had a BAD STOP. " << endl;
    }
    
  } // while
  
}

////////////////////////////////////////////////////////////////////////////
// spawn a child
//
// Executed by Boss process.

int DsServerMgr::spawnServer(const string &executable_name,
			     int port,
			     Socket *socket, string & errStr)

{

  errStr += "ERROR - DsServerMgr::spawnServer\n";

  // Fill in the port arg.
  char port_str[32];
  sprintf(port_str, "%d", port);
  
  // qmax value
  char qmax_str[32];
  sprintf(qmax_str, "%d", _childMaxQuiescentSecs);
  
  // Set up the args.
  char *args[32];
  int nargs = 0;
  args[nargs++] = (char *) executable_name.c_str();
  args[nargs++] = (char *) "-port";
  args[nargs++] = port_str;
  args[nargs++] = (char *) "-instance";
  args[nargs++] = (char *) "manager";
  args[nargs++] = (char *) "-qmax";
  args[nargs++] = qmax_str;
  if (_isVerbose) {
    args[nargs++] = (char *) "-debug";
  }
  if (_isSecure) {
    args[nargs++] = (char *) "-secure";
  }
  if (_mdvReadOnly && executable_name == "DsMdvServer") {
    args[nargs++] = (char *) "-readOnly";
  }
  if (_spdbReadOnly && executable_name == "DsSpdbServer") {
    args[nargs++] = (char *) "-readOnly";
  }
  args[nargs] = NULL;

  string commandLine = executable_name;
  for (int i = 1; i < nargs; i++) {
    commandLine += " ";
    commandLine += args[i];
  }
  
  // Construct snuff string.
  string snuffString = "snuff \"";
  snuffString += args[0];
  snuffString += " ";
  snuffString += args[1];
  snuffString += " ";
  snuffString += args[2];
  snuffString += "\"";
  
  // Snuff the server here.
  if (_isVerbose) {
    cerr << "Executing: " << endl << "    " << snuffString << endl;
  }
  system(snuffString.c_str());

  int pid = fork();
  if (pid < 0) {

    // error in fork

    errStr += "  Could not fork for sub-server.";
    return -1;

  } else if (pid == 0) {

    // This is now the child. Exec the new server.

    if (_isDebug) {
      cerr << "Forked child, PID: " << getpid() << endl;
      cerr << "  Now exec: " << endl;
      cerr << "  " << commandLine << endl;
    }

    // close both the listening and communicating socket

    socket->close();
    _serverSocket->close();

    // exec to mutate to server
    
    execvp(args[0], args); // Cast away const.

    // should not reach here
    
    cerr << "Error: Couldn't execute: " << commandLine << endl;
    cerr << "    errno: " << errno << endl;
    cerr << "        EACCES = " << EACCES << endl;
    cerr << "        ENOEXEC = " << ENOEXEC << endl;
    cerr << "        EPERM = " << EPERM << endl;
    cerr << "        E2BIG = " << E2BIG << endl;
    cerr << "        EFAULT = " << EFAULT << endl;
    cerr << "        ENAMETOOLONG = " << ENAMETOOLONG << endl;
    cerr << "        ENOENT = " << ENOENT << endl;
    cerr << "        ENOMEM = " << ENOMEM << endl;
    cerr << "        ENOTDIR = " << ENOTDIR << endl;
    cerr << "        ELOOP = " << ELOOP << endl;
    _exit(1);
    
  }
  
  // This is the parent - increment number of servers
  
  return 0;

}

///////////////////////////////////////////////////////////////
// wait for server to come up, waiting 200 msecs between pings
//
// Returns 0 on success, -1 on failure

int DsServerMgr::waitForServer(const string & executableName, int port,
			       int wait_secs)
  
{

  time_t start = time(NULL);
  time_t now = start;
  while (now - start < wait_secs) {
    if (pingServer(executableName, port) == 0) {
      return 0;
    }
    umsleep(200);
    now = time(NULL);
  }

  return -1;

}

///////////////////////////////////////
// ping a server to see if it is alive
//
// Returns 0 on success (there is a responsive server at that port)
//         -1 on failure

int DsServerMgr::pingServer(const string & executableName, int port)
  
{
  
  if (_isVerbose) {
    cerr << "Trying to ping server: " << executableName
	 << ", port:" << port << endl;
  }

  // try to connect

  GetHost gh;
  string host = "localhost";
  Socket socket;
  
  if (socket.open(host.c_str(), port)) {
    if (_isVerbose) {
      cerr << "  Could not connect to server for ping" << endl;
    }
    return -1;
  }
  
  // send ping  message

  DsServerMsg msg;
  msg.setCategory(DsServerMsg::ServerStatus);
  msg.setType(DsServerMsg::IS_ALIVE);
  void * msgToSend = msg.assemble();
  int msgLen = msg.lengthAssembled();
  
  if (socket.writeMessage(0, msgToSend, msgLen,
			  _pingTimeoutMsecs)) {
    if (_isVerbose) {
      cerr << "  Could not write ping message." << endl;
    }
    return -1;
  }
  
  // Read in the reply.
  if (socket.readMessage(_pingTimeoutMsecs)) {
    if (_isVerbose) {
      cerr << "  Could not read ping reply." << endl;
    }
    return -1;
  }
  
  const void * data = socket.getData();
  ssize_t dataSize = socket.getNumBytes();
  
  // Disassemble the incoming message.
  DsServerMsg reply;
  if (reply.disassemble(data, dataSize)) {
    if (_isDebug) {
      cerr << "Error: Could not disassemble message. Either too small "
	   << "or bad category." << endl;
    }
    return -1;
  }
  
  int pid = reply.getFirstInt();
  if (_isVerbose) {
    cerr << "  Found server running, pid: " << pid << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////////////
// send reply - overloaded
//
// Returns 0 on success, -1 on failure

int DsServerMgr::sendReply(Socket * socket,
                           DsServerMsg::msgErr errCode,
			   const string & errStr)

{
  string statusString;
  if (DsThreadedServer::sendReply(socket, errCode,
				  errStr, statusString,
				  _commTimeoutMsecs)) {
    cerr << statusString << endl;
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////
// send reply - overloaded
//
// Includes the filled-out URL in the reply
//
// Returns 0 on success, -1 on failure

int DsServerMgr::sendReply(Socket * socket,
			   const DsURL & url,
                           DsServerMsg::msgErr errCode,
			   const string & errStr)

{
  
  DsServerMsg msg;
  msg.setCategory(DsServerMsg::Generic);
  msg.addURL(url);
  if (errCode != DsServerMsg::DSS_MSG_SUCCESS) {
    msg.setErr(errCode);
    if (errStr.size() > 0) {
      msg.addErrString(errStr);
    }
  }
  void * msgToSend = msg.assemble();
  int msgLen = msg.lengthAssembled();
  
  if (socket->writeMessage(0, msgToSend,
			   msgLen, _commTimeoutMsecs)) {
    cerr << "ERROR - COMM -DsServerMgr::sendReply" << endl;
    cerr << socket->getErrStr() << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// add a pending request to the set

void DsServerMgr::addPending(const string &executable_name)

{
  
  pthread_mutex_lock(&_pendingMutex);
  
  _pending.insert(executable_name);

  if (_isVerbose) {
    cerr << "Adding " << executable_name << " to pending set" << endl;
    cerr << "Pending requests: " << endl;
    set<string>::iterator ii;
    for (ii = _pending.begin(); ii != _pending.end(); ii++) {
      cerr << "  " << *ii << endl;
    }
  }
  
  pthread_mutex_unlock(&_pendingMutex);

}

///////////////////////////////////////////////////////////////////
// remove a pending request from the set

void DsServerMgr::removePending(const string &executable_name)

{
  
  pthread_mutex_lock(&_pendingMutex);
  
  _pending.erase(executable_name);

  if (_isVerbose) {
    cerr << "Removing " << executable_name << " from pending set" << endl;
    if (_pending.size() == 0) {
      cerr << "No requests pending" << endl;
    } else {
      cerr << "Pending requests: " << endl;
      set<string>::iterator ii;
      for (ii = _pending.begin(); ii != _pending.end(); ii++) {
	cerr << "  " << *ii << endl;
      }
    }
  }
  
  pthread_mutex_unlock(&_pendingMutex);

}

///////////////////////////////////////////////////////////////////
// check whether there is already a pending request for this server

bool DsServerMgr::isPending(const string &executable_name)

{
  
  pthread_mutex_lock(&_pendingMutex);
  
  if (_pending.find(executable_name) != _pending.end()) {
    pthread_mutex_unlock(&_pendingMutex);
    return true;
  }
  
  pthread_mutex_unlock(&_pendingMutex);
  return false;

}

