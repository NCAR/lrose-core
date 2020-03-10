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

#include <toolsa/pmu.h>
#include <toolsa/uusleep.h>
#include <toolsa/Socket.hh>
#include <toolsa/ServerSocket.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>

#include <sys/wait.h>
#include <csignal>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

#include <dsserver/ProcessServer.hh>

using namespace std;

//////////////////////////////////////////////////////////////////////////
// Constructor:
//   o Opens socket on specified port
//   o Updates the status of the server.
//       Use isOkay() to determine status.

ProcessServer::ProcessServer(const string & executableName,
			     const string & instanceName,
			     int port,
			     int maxClients /* = 1024 */,
			     bool isDebug /* = false */,
			     bool isVerbose /* = false */) :
	_isChild(false),
	_isOkay(false),
	_errString(""),
	_isNoThreadDebug(false),
	_serverSocket(NULL),
	_executableName(executableName),
	_instanceName(instanceName),
	_port(port),
	_numClients(0),
	_maxClients(maxClients),
	_isDebug(isDebug),
	_isVerbose(isVerbose),
	_lastPrint(0),
	_commTimeoutMsecs(60)

{

  // Make sure that the messaging flags are consistent.
  // Can't have verbose without debug.

  if (_isVerbose) {
    _isDebug = true;
  }
  
  // If the instance name is empty, use the port number.
  if (_instanceName.size() == 0) {
    char buf[100];
    sprintf(buf, "%d", _port);
    _instanceName = buf;
  }

  // Register with the procmap. Skip if another part of the
  //   application already took care of it.
  // 
  if (PMU_init_done()) {
    cerr << "WARNING - " << executableName << endl;
    cerr << "  PMU_init already done." << endl;
    cerr << "  This should be left to ProcessServer object" << endl;
    cerr << "  " << DateTime::str() << endl;
  } else {
    PMU_auto_init(_executableName.c_str(),
		  _instanceName.c_str(),
		  PROCMAP_REGISTER_INTERVAL);
  }
  
  // get comm timeout
  
  char *DS_COMM_TIMEOUT_MSECS = getenv("DS_COMM_TIMEOUT_MSECS");
  if (DS_COMM_TIMEOUT_MSECS != NULL) {
    ssize_t timeout;
    if (sscanf(DS_COMM_TIMEOUT_MSECS, "%ld", &timeout) == 1) {
      _commTimeoutMsecs = timeout;
    }
  }
  
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
    cerr << "ProcessServer has opened ServerSocket at port " << _port << endl;
    cerr << "  _maxClients: " << _maxClients << endl;
  }
  
  // Set status.
  _isOkay = true;

}

//////////////////////////////////
// Destructor.
//   Unregisters with the procmap.
//

ProcessServer::~ProcessServer()
{
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

int ProcessServer::waitForClients(int timeoutMSecs /* = 1000*/ )

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
    _errString += "  Error in ProcessServer::waitForClients(): ";
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

    // Wait for connections. Blocks or times out.
    // getClient() creates a new socket - it must be deleted by
    // the calling function, i.e. this one

    Socket *socket = _serverSocket->getClient(timeoutMSecs);

    if (socket == NULL) {

      if (_serverSocket->getErrNum() == SockUtil::TIMED_OUT) {

	// Timed out - call the timeout method to give control back to the
	// user of this ProcessServer object.

	bool shouldContinue = timeoutMethod();
	if (!shouldContinue) {
	  if (exitMethod()) {
	    if (_isDebug) {
	      cerr << "ProcessServer returning from waitForClients() "
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
	  cerr << "Error in ProcessServer::waitForClients(): " << endl;
	  cerr << "  ServerSocket error while waiting for client." << endl;
	  cerr << _serverSocket->getErrString();
	  cerr << DateTime::str() << endl;
	}
	  
	// Reset the socket and try again.
	_serverSocket->resetState();
	continue;

      }

    } // if (socket == NULL)

    // purge completed threads - children which have exited
    
    purgeCompletedThreads();
    
    // Check the client count - can we accept?
    
    if (_maxClients >= 0 && _numClients >= _maxClients) {
      
      string errMsg;
      TaStr::AddInt(errMsg,
                    "Service Denied. Too many clients being handled: ",
                    _numClients);
      
      if (_isDebug) {
	cerr << errMsg << endl;
      }

      // delete the socket, which closes the connection

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
	  cerr << "ProcessServer returning from waitForClients() "
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

void ProcessServer::spawn(ServerSocketStruct * sss, Socket * socket)

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

bool ProcessServer::timeoutMethod()
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

  // Register with the procmap.
  string pmuStr;
  TaStr::AddInt(pmuStr, "Listening, port: ", _port, false);
  PMU_auto_register((char *) pmuStr.c_str());

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
bool ProcessServer::postHandlerMethod()
{

  // Register with the procmap.
  string pmuStr;
  TaStr::AddInt(pmuStr, "Received a client, port: ", _port, false);
  PMU_auto_register((char *) pmuStr.c_str());

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

bool ProcessServer::exitMethod()
{

  return true;

}

/////////////////////////////////////////////////////////////////////
// clientDone()
//
// Update the server to reflect that a client is finished.
//
// Threads: called by worked thread.
//
// This does nothing in the ProcessServer() class, since we cannot
// communicate with the main thread. It will be overridden in the
// DsThreadServer class.
//
// Virtual

void ProcessServer::clientDone()
{
  
}

////////////////////////////////////////////////////////////////
// Purge completed threads
//
// Virtual

void ProcessServer::purgeCompletedThreads()

{

  if (_isNoThreadDebug) {
    return;
  }

  pid_t dead_pid;
  int status;

  while ((dead_pid = waitpid((pid_t) -1,
			     &status,
			     (int) (WNOHANG | WUNTRACED))) > 0) {
    
    if (_isDebug) {
      cerr << "Child died, pid: " << dead_pid << endl;
      if (WIFEXITED(status) && (WEXITSTATUS(status) == SIGQUIT)) {
	cerr << "  Child exited with SIGQUIT signal" << endl;
      }
    }
    
    _numClients--;
    
  } // while
  
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
void * ProcessServer::__serveClient(void * svrsockstruct)

{

  // sanity check
  if (!svrsockstruct) {
    return NULL;
  }

  ServerSocketStruct * sss = (ServerSocketStruct *) svrsockstruct;
  Socket * socket = sss->socket;
  ProcessServer * server = sss->server;

  // sanity check
  if (server == NULL) {
    return NULL;
  }

  // Delete the ServerSocketStruct -- it is owned by this thread
  delete sss;

  // Should have a valid open socket now.
  if (socket == NULL) {
    if (server->_isDebug) {
      cerr << "ERROR - ProcessServer::__serveClient." << endl;
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
      cerr << "ERROR - COMM - ProcessServer::__serveClient." << endl;
      cerr << "  Socket received has error state:" << endl;
      cerr << "  " << socket->getErrString() << endl;
      cerr << "  " << DateTime::str() << endl;
    }
    // No way to tell client. This is a bad error.
    server->clientDone();
    return NULL;
  }
  
  if (server->_isVerbose) {
    cerr << "Client handler thread ready for socket read/writes..." << endl;
  }

  // call the client handler - which must be provided by derived classes

  int success = server->handleClient(socket);
 
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
    string newError  = "Error in ProcessServer::__serveClient: ";
    newError += "Could not handle client.\n";
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
      cerr << " ProcessServer::__serveClient" << endl;
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
