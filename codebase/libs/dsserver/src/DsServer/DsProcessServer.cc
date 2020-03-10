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

#include <dsserver/DsProcessServer.hh>
#include <dsserver/DsServerMsg.hh>
#include <didss/DsURL.hh>

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/procmap.h>
#include <toolsa/Socket.hh>
#include <toolsa/ServerSocket.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>

#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
using namespace std;

//////////////////////////////////////////////////////////////////////////
// Constructor:
//   o Registers with procmap
//   o Opens socket on specified port
//   o Updates the status of the server.
//       Use isOkay() to determine status.
//
// If maxQuiescentSecs is non-positive, quiescence checking is disabled.
//

DsProcessServer::DsProcessServer(const string & executableName,
				 const string & instanceName,
				 int port,
				 int maxQuiescentSecs /* = -1 */,
				 int maxClients /* = 1024 */,
				 bool isDebug /* = false */,
				 bool isVerbose /* = false */,
				 bool isSecure /* = false */,
				 bool isReadOnly /* = false */,
                 bool allowHttp /* = false */) :
  _isChild(false),
  _isOkay(false),
  _errString(""),
  _isNoThreadDebug(false),
  _serverSocket(NULL),
  _executableName(executableName),
  _instanceName(instanceName),
  _port(port),
  _maxQuiescentSecs(maxQuiescentSecs),
  _doShutdown(false),
  _numClients(0),
  _maxClients(maxClients),
  _isDebug(isDebug),
  _isVerbose(isVerbose),
  _isSecure(isSecure),
  _isReadOnly(isReadOnly),
  _allowHttp(allowHttp),
  _lastPrint(0)

{

  // Make sure that the messaging flags are consistent.
  // Can't have verbose without debug.

  if (_isVerbose) {
    _isDebug = true;
  }
  
  // set last action time to now
 
  _lastActionTime = time(NULL);

  // If the instance name is empty, use the port number.
  if (_instanceName.size() == 0) {
    char buf[100];
    sprintf(buf, "%d", _port);
    _instanceName = buf;
  }

#ifndef DISABLE_PMU
  // Register with the procmap. Skip if another part of the
  //   application already took care of it.
  // 
  if (PMU_init_done()) {
    cerr << "WARNING - " << executableName << endl;
    cerr << "  PMU_init already done." << endl;
    cerr << "  This should be left to DsProcessServer object" << endl;
    cerr << "  " << DateTime::str() << endl;
  } else {
    PMU_auto_init(_executableName.c_str(),
		  _instanceName.c_str(),
		  PROCMAP_REGISTER_INTERVAL);
  }
  string pmuStr;
  TaStr::AddInt(pmuStr, "Listening, port: ", _port, false);
  PMU_force_register((char *) pmuStr.c_str());
#endif
  
  // override max clients from environment?
  
  char *DS_SERVER_MAX_CLIENTS = getenv("DS_SERVER_MAX_CLIENTS");
  if (DS_SERVER_MAX_CLIENTS != NULL) {
    int max_clients;
    if (sscanf(DS_SERVER_MAX_CLIENTS, "%d", &max_clients) == 1) {
      _maxClients = max_clients;
    }
  }
  
  // Open socket on the port.
  _serverSocket = new ServerSocket();
  if (_serverSocket->openServer(_port) < 0) {
    _errString = "";
    TaStr::AddStr(_errString, "ERROR - COMM - ", executableName);
    _errString += "  Could not open ServerSocket: ";
    _errString += _serverSocket->getErrString();
    TaStr::AddStr(_errString, "  ", DateTime::str());
    if (_isDebug) {
      cerr << _errString << endl;
    }
    return;
  }

  if (_isDebug) {
    cerr << "DsProcessServer has opened ServerSocket at port " << _port << endl;
    cerr << "  _maxClients: " << _maxClients << endl;
  }
  
  // Set status.
  _isOkay = true;

}

//////////////////////////////////
// Destructor.
//   Unregisters with the procmap.
//

DsProcessServer::~DsProcessServer()
{
#ifndef DISABLE_PMU
  PMU_auto_unregister();
#endif
  if (_serverSocket != NULL) {
    delete (_serverSocket);
  }
}

/////////////////////////////////////////////////////////////////////
// waitForClients()
//
// Block and wait for clients.
//   If a positive timeoutMSecs is provided, the wait times out,
//     PMU registration is performed, and timeoutMethod() is called.
//   If a non-positive timeoutMSecs is provided,
//     the server will only register with PMU when a client request
//     is received.
//
// Threads: This method defines the Boss thread behavior.
//          Worker threads are spawned from this method.
//
// Returns:  0 - the boss thread was instructed to terminate by
//                 the return from timeoutMethod() or postHandlerMethod().
//          -1 - something terrible happened.
//                  Do not call waitForClients() again after error!
//

int DsProcessServer::waitForClients(int timeoutMSecs /* = 1000*/ )

{
  
  // Make the timeout secs -1 if not positive (For the ServerSocket).
  if (timeoutMSecs <= 0) {
    timeoutMSecs = -1;
  }
  
  // Check that we have a valid server socket.
  if (!_isOkay ||
      _serverSocket == NULL ||
      _serverSocket->hasState(SockUtil::STATE_ERROR) ||
      !_serverSocket->hasState(SockUtil::STATE_OPENED)) {

    _errString = "";
    TaStr::AddStr(_errString, "ERROR - ", _executableName);
    _errString += "  Error in DsProcessServer::waitForClients(): ";
    _errString += "  Server does not have a valid and/or open ServerSocket.";
    TaStr::AddStr(_errString, "  ", DateTime::str());

    if (_isDebug) {
      cerr << _errString << endl;
    }
    umsleep(timeoutMSecs);
	
    return -1;
  }
    
  // Wait for connections.
  
  while (true) {
    
    if (_isVerbose) {
      time_t now = time(NULL);
      if (now - _lastPrint > 60) {
	cerr << "Server waiting for client connections..." << endl;
	_lastPrint = now;
      }
    }

    // Wait for connections. Blocks.
    //   The timer thread takes care of exiting if the server has been
    //   quiet for _maxQuiescentSecs.
    //
    // getClient() creates a new socket - it must be deleted by
    // the calling function, i.e. this one

    Socket *socket = NULL;
    if (_allowHttp) 
      socket = _serverSocket->getHttpClient(timeoutMSecs);
    else 
      socket = _serverSocket->getClient(timeoutMSecs);

    if (socket == NULL) {

      if (_serverSocket->getErrNum() == SockUtil::TIMED_OUT) {

	// Timed out - call the timeout method to give control back to the
	// user of this DsProcessServer object.

	bool shouldContinue = timeoutMethod();
	if (!shouldContinue) {
	  if (exitMethod()) {
	    if (_isDebug) {
	      cerr << "DsProcessServer returning from waitForClients() "
		   << "because timeoutMethod() indicated to do so."
		   << endl;
	      cerr << "  " << DateTime::str() << endl;
	    }
	    return 0; // success
	  }
	}
	
	// try again
	continue;

      } else {
	
	// Something besides timeout happened while waiting.
	
	if (_isDebug) {
	  cerr << "Error in DsProcessServer::waitForClients(): " << endl;
	  cerr << "  ServerSocket error while waiting for client." << endl;
	  cerr << _serverSocket->getErrString();
	  cerr << DateTime::str() << endl;
	}
	  
	// Reset the socket and try again.
	_serverSocket->resetState();
	continue;

      }

    } // if (socket == NULL)

    //  Got a connection - set last action time to now
    
    _lastActionTime = time(NULL);

    // Check the client count - can we accept?
    
    if (_maxClients >= 0 && _numClients >= _maxClients) {
      
      string errMsg;
      TaStr::AddInt(errMsg,
                    "Service Denied. Too many clients being handled: ",
                    _numClients);
      
      if (_isDebug) {
	cerr << errMsg << endl;
      }

      // Reply to the client with an error. One second timeout.
      
      string statusString;
      int status = sendReply(socket, DsServerMsg::SERVICE_DENIED,
			     errMsg, statusString, 1000);
      
      if (_isDebug && status < 0) {
	cerr << "Error: DsProcessServer could not send SERVICE_DENIED "
	     << "message to client: " << errMsg << endl;
	cerr << "  " << DateTime::str() << endl;
      }
      
      // Continue -- The status doesn't really matter.
      delete socket;
      socket = NULL;
      continue;

    } // if (_maxClients >= 0 && _numClients >= _maxClients)
    
    // Create the ServerSocketStruct -- it is owned by the child
    // and will be deleted after the child is done

    ServerSocketStruct * sss = new ServerSocketStruct;
    sss->socket = socket;
    sss->server = this;
    
    if (_isNoThreadDebug) {
      
      // No threading - serve the client directly.

      if (_isDebug) {
	cerr << "Server got a client. In No-Thread debug mode." << endl;
      }

      __serveClient(sss);
      delete socket;
      socket = NULL;
      
    } else {
    
      // spawn child / thread to handle client

      spawn(sss, socket);

    } // if (_isNoThreadDebug)

    // Call the post-handler method.

    bool shouldContinue = postHandlerMethod();
    if (!shouldContinue) {
      if (exitMethod()) {
	if (_isDebug) {
	  cerr << "DsProcessServer returning from waitForClients() "
	       << "because postHandlerMethod() indicated to do so."
	       << endl;
	  cerr << "  " << DateTime::str() << endl;
	}
	return 0; // Success.
      }
    }
    
  } // while (true)

  return -1; // should not reach here - no breaks in loop

}

///////////////////////////////////////////////////
// spawn()
//
// Spawn a child process for handling the client
// 
// virtual

void DsProcessServer::spawn(ServerSocketStruct * sss, Socket * socket)

{

  if (_isDebug) {
    cerr << "Server got a client. Spawning a child ..." << endl;
  }
  
  // Fork a child.
  
  pid_t childPid = fork();
  
  if (childPid == 0) {
	
    // child  - first close server socket
    
    _isChild = true;
    _serverSocket->close();

    // then serve the client and exit
	
    __serveClient(sss);
    delete socket;
	
    _exit(0);

  } else {

    // parent
    
    _numClients++;
	
    if (_isDebug) {
      cerr << "  Started child: " << childPid << endl;
    }

    // free up memory associated with this connection

    delete sss;
    delete socket;
    
  }

}

////////////////////////////////////////////////////////////////////////
// handleServerCommand()
//
// Handle a command from a client that requests information
//   directly from and about the server. The passed
//   data is a DsServerMsg.
//
// Subclasses should define this only if they have special commands
//   to handle. Always call the superclass's method for
//   commands not recognized by the subclass!
//
// Threads: Called by Worker threads.
//
// Returns 0 if command handled successfully, -1 otherwise.
//   Does not set the _errString member, as this is called by threads.
//
// NOTE:
//   Subclasses are expected to never return an error
//   condition from this method. All errors should be
//   handled nicely in some way by subclass.
//
//   If a subclass ever returns an error condition,
//   this server exits.
//
// Virtual

int DsProcessServer::handleServerCommand(Socket * socket,
					 const void * data, ssize_t dataSize)
{

  DsServerMsg msg;
  if (_isVerbose) {
    cerr << "Client handler thread disassembling server command..." << endl;
  }

  // get comm timeout
  
  ssize_t commTimeoutMsecs = DS_DEFAULT_COMM_TIMEOUT_MSECS;
  char *DS_COMM_TIMEOUT_MSECS = getenv("DS_COMM_TIMEOUT_MSECS");
  if (DS_COMM_TIMEOUT_MSECS != NULL) {
    ssize_t timeout;
    if (sscanf(DS_COMM_TIMEOUT_MSECS, "%ld", &timeout) == 1) {
      commTimeoutMsecs = timeout;
    }
  }
  
  int status = msg.disassemble(data, dataSize);
  if (status < 0) {
    string errMsg = "Error in DsProcessServer::handleServerCommand(): ";
    errMsg += "Could not disassemble DsServerMsg.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Reply to the client with a BAD_MESSAGE error. Ten sec timeout.
    string statusString;
    sendReply(socket, DsServerMsg::BAD_MESSAGE,
	      errMsg, statusString, commTimeoutMsecs);
    // Return success. We did the best we could...
    return 0;
  }

  // Check that this is a server command, as DsProcessServer determined it is.
  if (msg.getMessageCat() != DsServerMsg::ServerStatus) {
    string errMsg  = "Error in DsServerMgr::handleServerCommand(): ";
    errMsg += "Message is not a server command message.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    sendReply(socket, DsServerMsg::SERVER_ERROR,
	      errMsg, statusString, commTimeoutMsecs);
    return 0;
  }

  int command = msg.getType();
  msg.clearParts();

  bool isShutdown = false;
  switch (command) {

  case DsServerMsg::IS_ALIVE:
    // Send back the pid and the name of the process.
    msg.addInt(getppid());
    msg.addString("Executable Name should go here.");
    break;

  case DsServerMsg::GET_NUM_CLIENTS:
    {
      // Send back the client count.
      msg.addInt(_numClients);
      break;
    }

  case DsServerMsg::SHUTDOWN:
    // Send back an empty message and exit.
    isShutdown = true;
    break;

  default:
    // Not handled -- this is an error.
    msg.setErr(DsServerMsg::UNKNOWN_COMMAND);
    break;

  } // switch

  void * msgToSend = msg.assemble();
  ssize_t msgLen = msg.lengthAssembled();
  
  // Send the message with a ten second timeout
  status = socket->writeMessage(0, msgToSend,
				msgLen, commTimeoutMsecs);
  
  if (status < 0) {
    if (_isDebug) {
      cerr << "Error in DsProcessServer::handleServerCommand(): "
	   << "Could not send reply message: "
	   << socket->getErrString() << endl;
      cerr << "  " << DateTime::str() << endl;
    }
    
    // Do not return with failure, as there is no way to
    // recover gracefully.

  }

  if (isShutdown) {
    if (_isChild) {
      // exit with SIGQUIT signal to let the parent know that it should
      // shutdown
      if (_isDebug) {
	cerr << "DsProcessServer::handleServerCommand - in child" << endl;
	cerr << "  " << DateTime::str() << endl;
	cerr << "  Received shutdown command, so exiting with SIGQUIT." << endl;
      }
      _exit(SIGQUIT);
    } else {
      // This must be a threaded server, set the doShutdown flag
      _doShutdown = true;
    }
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////
// checkQuiescence()
// 
// Checks that the server has no clients,
// to eliminate the possibility that a client has been busy with a
// request that whole time (last action time is updated only when
// client starts and finishes).
// 
// Checks the last action time, to determine if it was more than 
//   maxQuiescentSecs before the current time.
//
// Returns true if quiescent, false otherwise.

bool DsProcessServer::checkQuiescence()

{

  // purge completed threads

  purgeCompletedThreads();
  
  // if _maxQuiescentSecs is -1, we are never quiescent

  if (_maxQuiescentSecs < 0) {
    return false;
  }

  // if we have clients, we should continue

  if (_numClients > 0) {
    return false;
  }
  
  // check for quiesence

  time_t now = time(NULL);
  int quiteTime = now - _lastActionTime;
  if (quiteTime > _maxQuiescentSecs) {
    // been quiet too long, we should quit
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////////////////
// timeoutMethod()
// 
// Called from waitForClients() on timeout.
// Perform any special operations that need to happen
//   when the server times out waiting for clients.
//   Note that threads serving clients may still be running
//   when this method is called, but no more clients will be accepted.
//
// Subclasses should define this if they need control occasionally
//   while waiting for clients. If a subclass defines this method,
//   it *MUST* call the base class version before executing its own
//   code. The subclass should also be sure to pass a correct return
//   value back, so that we can decide whether to exit or not.
//   Normally, the subclass should pass back the return value from
//   the base class routine.
//
// Returns: true if the server should continue to wait for clients,
//          false if the server should return from waitForClients().
//
// Virtual

bool DsProcessServer::timeoutMethod()
{

  if (_isVerbose) {
    time_t now = time(NULL);
    if (now - _lastPrint > 60) {
      cerr << "Server timed out on getClient():" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  " << _serverSocket->getErrString() << endl;
      _lastPrint = now;
    }
  }

#ifndef DISABLE_PMU
  // Register with the procmap.

  string pmuStr;
  TaStr::AddInt(pmuStr, "Listening, port: ", _port, false);
  PMU_auto_register((char *) pmuStr.c_str());
#endif

  if (_doShutdown) {
    if (_isDebug) {
      cerr << "DsProcessServer::timeoutMethod" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Exiting because _doShutDown has been set" << endl;
    }
#ifndef DISABLE_PMU
    PMU_auto_unregister();
#endif

    _exit(0);
  }

  // check for quiescence

  if (checkQuiescence()) {
    return false;
  }
  
  return true;

}
///////////////////////////////////////////////////////////////////////
// postHandlerMethod()
//
// Called from waitForClients() after Worker thread is spawned.
// Perform any special operations that need to happen
//   when the server is finished handling a client.
//   Note that threads serving clients may still be running
//   when this method is called, but no more clients will be accepted.
//
// Subclasses should define this if they need control occasionally
//   while waiting for clients. If a subclass defines this method,
//   it *MUST* call the base class version before executing its own
//   code. The subclass should also be sure to pass a correct return
//   value back, so that we can decide whether to exit or not.
//   Normally, the subclass should pass back the return value from
//   the base class routine.
//
// Returns: true if the server should continue to wait for clients,
//          false if the server should return from waitForClients().
//
// Virtual
bool DsProcessServer::postHandlerMethod()
{

#ifndef DISABLE_PMU
  // Register with the procmap.
  
  string pmuStr;
  TaStr::AddInt(pmuStr, "Received a client, port: ", _port, false);
  PMU_auto_register((char *) pmuStr.c_str());
#endif

  if (_doShutdown) {
    if (_isDebug) {
      cerr << "DsProcessServer::postHandlerMethod" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Exiting because _doShutDown has been set" << endl;
    }
#ifndef DISABLE_PMU
    PMU_auto_unregister();
#endif
    _exit(0);
  }

  // check for quiescence

  if (checkQuiescence()) {
    return false;
  }
  
  return true;

}

////////////////////////////////////////////////////////////////////////
// exitMethod()
//
// Called from the main loop when (a) there has not been activity in the
// required period and (b) there are no clients.
//
// Perform any special operations that need to happen
//   when the server is about to exit due to quiescence.
//   Note that this method will never get called if a non-positive
//   value was supplied for maxQuiescentSecs.
//
// Subclasses should define this if they need to do anything special
//   before the application exits. If a subclass defines this method,
//   it *MUST* call the base class version before executing its own
//   code. The subclass should also pass a correct return value back
//   to the calling routine. Normally it should pass back the return
//   value from the base class routine.
//
// Returns: true if the server should continue with the exit.
//          false if the server should not exit after all.
//
// Virtual

bool DsProcessServer::exitMethod()
{

  // check for quiescence

  if (checkQuiescence()) {
    return false;
  }
  
  // unregister with procmap

#ifndef DISABLE_PMU
  PMU_auto_unregister();
#endif

  return true;

}

/////////////////////////////////////////////////////////////////////
// clientDone()
//
// Update the server to reflect that a client is finished.
//
// Threads: called by worked thread.
//
// This does nothing in the DsProcessServer() class, since we cannot
// communicate with the main thread. It will be overridden in the
// DsThreadServer class.
//
// Virtual

void DsProcessServer::clientDone()
{
  
}

////////////////////////////////////////////////////////////////
// Purge completed threads
//
// Virtual

void DsProcessServer::purgeCompletedThreads()

{

  if (_isNoThreadDebug) {
    return;
  }

  pid_t dead_pid;
  int status;

  while ((dead_pid = waitpid((pid_t) -1,
			     &status,
			     (int) (WNOHANG | WUNTRACED))) > 0) {
    
    // if exited with SIGQUIT, this tells us that the child received a 
    // shutdown message, so we should set the _doShutdown flag
    
    if (WIFEXITED(status)) {
      if (WEXITSTATUS(status) == SIGQUIT) {
	_doShutdown = true;
      }
    }
    
    if (_isDebug) {
      cerr << "Child died, pid: " << dead_pid << endl;
    }
    _numClients--;
    _lastActionTime = time(NULL);
    
  } // while
  
}

///////////////////////////////////////////////////////////////////////
// sendReply()
//
// Send a simple reply to the client, containing the passed-in
//   error information.
//
// Threads: Called by Worker threads.
//          Called by Boss thread.
//
// Returns:  0 on success
//          -1 on failure, in which case _errString contains a message.
//
int DsProcessServer::sendReply(Socket * socket,
			       DsServerMsg::msgErr errCode,
			       const string & errMsg,
			       string & errString, int wait_msecs /* = 10000*/ )
{

  // Reply to the client with the appropriate error.
  DsServerMsg msg;
  msg.setCategory(DsServerMsg::Generic);
  msg.setErr(errCode);
  if (errMsg.size() > 0) {
    msg.addErrString(errMsg);
  }
  void * msgToSend = msg.assemble();
  ssize_t msgLen = msg.lengthAssembled();
  
  // Send the message with a ten second wait.
  int status = socket->writeMessage(0, msgToSend,
				    msgLen, wait_msecs);
  
  if (status < 0) {
    errString  = "Error in DsProcessServer::sendReply(): ";
    errString += "Could not send error reply message: ";
    errString += socket->getErrString();
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////
// __serveClient()
// 
// Start method for all threaded clients. Called from waitForClients()
//   when a client request is received.
// 
// Looks at the incoming data, verifies that it is a valid DsServerMsg,
//   and determines whether it is a server command or a data command.
//   Calls the appropriate handler method based on the determined type.
// 
// If there is a problem reading or interpreting the message, replies
//   to the client with an error message having one of the following types:
//     o DsServerMsg::SERVER_ERROR -- Couldn't read from the socket.
//     o DsServerMsg::BAD_MESSAGE  -- Message is corrupt or unreadable.
//  
void * DsProcessServer::__serveClient(void * svrsockstruct)

{

  // sanity check
  if (!svrsockstruct) {
    return NULL;
  }

  ServerSocketStruct * sss = (ServerSocketStruct *) svrsockstruct;
  Socket * socket = sss->socket;
  DsProcessServer * server = sss->server;

  // sanity check
  if (server == NULL) {
    return NULL;
  }

  // Delete the ServerSocketStruct -- it is owned by this thread
  delete sss;

  // Should have a valid open socket now.
  if (socket == NULL) {
    if (server->_isDebug) {
      cerr << "ERROR - DsProcessServer::__serveClient." << endl;
      cerr << "  Got NULL socket." << endl;
    }
    server->clientDone();
    return NULL;
  }

  if (server->_isVerbose) {
    cerr << "Client handler started..." << endl;
  }

  if (socket->hasState(SockUtil::STATE_ERROR)) {
    if (server->_isDebug) {
      cerr << "ERROR - COMM - DsProcessServer::__serveClient." << endl;
      cerr << "  Socket received has error state:" << endl;
      cerr << "  " << socket->getErrString() << endl;
      cerr << "  " << DateTime::str() << endl;
    }
    // No way to tell client. This is a bad error.
    server->clientDone();
    return NULL;
  }

  if (server->_isVerbose) {
    cerr << "Client handler thread reading from socket..." << endl;
  }

  // get comm timeout
  
  ssize_t commTimeoutMsecs = DS_DEFAULT_COMM_TIMEOUT_MSECS;
  char *DS_COMM_TIMEOUT_MSECS = getenv("DS_COMM_TIMEOUT_MSECS");
  if (DS_COMM_TIMEOUT_MSECS != NULL) {
    ssize_t timeout;
    if (sscanf(DS_COMM_TIMEOUT_MSECS, "%ld", &timeout) == 1) {
      commTimeoutMsecs = timeout;
    }
  }
  
  // Read from the socket.
  int status = socket->readMessage(commTimeoutMsecs);

  if (status != 0) {
    char buf[10];
    sprintf(buf, "%d", status);
    string errMsg  = "Error: Server could not read. Got status: ";
    errMsg += buf;
    sprintf(buf, "%d", socket->getErrNum());
    errMsg += " Error Num: ";
    errMsg += buf;
    errMsg += ". Error String: ";
    errMsg += socket->getErrString();
    if (server->_isDebug) {
      cerr << errMsg << endl;
    }

    // Send error reply to client
    string statusString;
    server->sendReply(socket, DsServerMsg::SERVER_ERROR,
		      errMsg, statusString, commTimeoutMsecs);

    // wait up to 10 secs for client to close socket
    // Disabled because it breaks the operation of the tunnel - Mike
    // socket->readSelect(commTimeoutMsecs);

    // Indicate that the client is done
    server->clientDone();
    return NULL;

  }
  
  if (server->_isVerbose) {
    cerr << "Client handler thread performed successful read." << endl;
  }

  DsServerMsg msg;
  const void * data = socket->getData();
  size_t dataSize = socket->getNumBytes();
  
  if (server->_isVerbose) {
    cerr << "  Client handler thread Read " << dataSize << " Bytes." << endl;
    cerr << "  Client handler thread decoding message..." << endl;
  }

  // Decode the incoming message header only.
  status = msg.decodeHeader(data, dataSize);
  if (status < 0) {
    // Could not decode message header -- bail.
    string errMsg  = "Error: Message from client could not be decoded. ";
    errMsg += "Either the message is too small, or it has an ";
    errMsg += "invalid category.";
    if (server->_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    server->sendReply(socket, DsServerMsg::BAD_MESSAGE,
		      errMsg, statusString, commTimeoutMsecs);

    // wait up to 10 secs for client to close socket
    // Disabled because it breaks the operation of the tunnel - Mike
    // socket->readSelect(10000);

    // Indicate that the client is done
    server->clientDone();
    return NULL;
  }

  // Determine if this is a server command or a task request.
  int success = 0;
  DsServerMsg::category_t category = msg.getMessageCat();
  if (category == DsServerMsg::ServerStatus) {

    success = server->handleServerCommand(socket, data, dataSize);

  } else {
    
    success = server->handleDataCommand(socket, data, dataSize);

  }
 
  // wait up to 10 secs for client to close socket
  // Disabled because it breaks the operation of the tunnel - Mike
  // socket->readSelect(10000);

  // Deal with failures to handle the message.
  //   Note that if the subclass returns an error code, it is
  //     considered failure to handle an error, which is fatal.
  // 
  if (success == -1) {

    // Server (subclass) takes care of all error handling and reporting.
    //   Just output generic message here.
    // 
    // Note that the subclass is not intended to ever return an error.
    // 
    string newError  = "Error in DsProcessServer::__serveClient: ";
    newError += "Could not handle message.\n";
    newError += DateTime::str();
    cerr << newError << endl;
 
    // Remove this thread from the client count.
    server->clientDone();

    // Exit if this is a debug server.
    // 
    if (server->_isDebug) {
      // Todo: Wait for all the threads to end?
      //       To make this work, need to block new clients.

      server->exitMethod();
      cerr << " DsProcessServer::__serveClient" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Exiting because debug server" << endl;
      exit(1);
    }

    return NULL;
  }
    
  // Notify the server this thread is finished.
  server->clientDone();

  // Done with the thread. Exit cleanly.
  return NULL;

}
