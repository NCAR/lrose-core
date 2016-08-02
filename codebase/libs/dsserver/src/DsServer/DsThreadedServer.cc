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

#include <dsserver/DsThreadedServer.hh>
#include <dsserver/DsServerMsg.hh>
#include <didss/DsURL.hh>
#include <didss/RapDataDir.hh>

#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/procmap.h>
#include <toolsa/Socket.hh>
#include <toolsa/ServerSocket.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>

#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;

//////////////////////////////////////////////////////////////////////////
/// Constructor:
//   o Registers with procmap
//   o Opens socket on specified port
//   o Updates the status of the server.
//       Use isOkay() to determine status.
//
// If maxQuiescentSecs is non-positive, quiescence checking is disabled.
//
DsThreadedServer::DsThreadedServer(const string & executableName,
				   const string & instanceName,
				   int port,
				   int maxQuiescentSecs /* = -1 */,
				   int maxClients /* = 128 */,
				   bool isDebug /* = false */,
				   bool isVerbose /* = false */,
				   bool isSecure /* = false */,
				   bool isRdOnly /* = false */) :
  DsProcessServer(executableName, instanceName, port,
		  maxQuiescentSecs, maxClients,
		  isDebug, isVerbose, isSecure, isRdOnly)

{

  if (!_isNoThreadDebug) {
    pthread_mutex_init(&_threadStatusMutex, NULL);
    pthread_mutex_init(&_procmapInfoMutex, NULL);
  }

}

// Destructor.
//   Uses the base class
//
DsThreadedServer::~DsThreadedServer()
{

}

///////////////////////////////////////////////////
// spawn()
//
// Spawn a thread for handling the client
// 
// virtual

void DsThreadedServer::spawn(ServerSocketStruct * sss, Socket * socket)

{
  
  // Start a thread.
  
  pthread_mutex_lock(&_threadStatusMutex);
  pthread_t thread;
  int err = pthread_create(&thread, NULL, __serveClient, sss);
  
  if (err != 0) {
  
    // thread creation error
    
    pthread_mutex_unlock(&_threadStatusMutex);
    
    string errMsg  = "Error in DsThreadedServer::spawn(): ";
    errMsg += "Could not create thread to handle client: ";
    errMsg += strerror(err);
    if (_isDebug) { 
      cerr << errMsg << endl;
    }
    
    // Send reply to the client that there was an error - 1 sec timeout

    string statusString;           
    sendReply(socket, DsServerMsg::SERVER_ERROR,
	      errMsg, statusString, 1000);
    
    // clean up and return
    delete sss;
    delete socket;
    return;
    
  }
  
  // thread created successfully

  if (_isVerbose) {
    cerr << "---> created client thread, id: " << thread << endl;
  }
  
  // add a status object to the list
  // this is used by the worker thread to indicate that it is done, and by the
  // boss thread to join the thread when it is done
  
  ThreadStatus status;
  status.threadId = thread;
  status.socket = socket;
  status.done = false;
  _threadStatus.push_back(status);
  pthread_mutex_unlock(&_threadStatusMutex);
  
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

bool DsThreadedServer::timeoutMethod()
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

  // string pmuStr;
  // TaStr::AddInt(pmuStr, "Listening, port: ", _port, false);

  pthread_mutex_lock(&_procmapInfoMutex);
  char label[128];
  sprintf(label, "Listening, port: %d", _port);
  PMU_auto_register(label);
  pthread_mutex_unlock(&_procmapInfoMutex);
#endif

  if (_doShutdown) {
    if (_isDebug) {
      cerr << "DsThreadedServer::timeoutMethod" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Exiting because _doShutDown has been set" << endl;
    }
#ifndef DISABLE_PMU
    pthread_mutex_lock(&_procmapInfoMutex);
    PMU_auto_unregister();
    pthread_mutex_unlock(&_procmapInfoMutex);
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
bool DsThreadedServer::postHandlerMethod()
{

#ifndef DISABLE_PMU
  // Register with the procmap.
  
  // string pmuStr;
  // TaStr::AddInt(pmuStr, "Received a client, port: ", _port, false);

  pthread_mutex_lock(&_procmapInfoMutex);
  char label[128];
  sprintf(label, "Received client, port: %d", _port);
  PMU_auto_register(label);
  pthread_mutex_unlock(&_procmapInfoMutex);
#endif

  if (_doShutdown) {
    if (_isDebug) {
      cerr << "DsThreadedServer::postHandlerMethod" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  Exiting because _doShutDown has been set" << endl;
    }
#ifndef DISABLE_PMU
    pthread_mutex_lock(&_procmapInfoMutex);
    PMU_auto_unregister();
    pthread_mutex_unlock(&_procmapInfoMutex);
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

bool DsThreadedServer::exitMethod()
{

  // check for quiescence

  if (checkQuiescence()) {
    return false;
  }
  
#ifndef DISABLE_PMU
  // unregister with procmap

  pthread_mutex_lock(&_procmapInfoMutex);
  PMU_auto_unregister();
  pthread_mutex_unlock(&_procmapInfoMutex);
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
// Overrides empty method in DsProcessServer class.
//
// Virtual

void DsThreadedServer::clientDone()
{
  
  if (_isNoThreadDebug) {
    return;
  }
  pthread_mutex_lock(&_threadStatusMutex);
  bool found = false;
  list<ThreadStatus>::iterator ii;
  for (ii = _threadStatus.begin(); ii != _threadStatus.end(); ii++) {
    ThreadStatus &status = *ii;
    if (pthread_equal(status.threadId, pthread_self())) {
      status.done = true;
      found = true;
      break;
    }
  }
  if (!found) {
    cerr << "ERROR - DsThreadedServer::clientDone" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Cannot find thread id in list: " << pthread_self() << endl;
  }
  pthread_mutex_unlock(&_threadStatusMutex);

}

////////////////////////////////////////////////////////////////
// Purge completed threads
//
// virtual

void DsThreadedServer::purgeCompletedThreads()

{

  if (_isNoThreadDebug) {
    return;
  }
  pthread_mutex_lock(&_threadStatusMutex);
  
  bool done = false;
  
  while (!done) {
    
    done = true;
    
    list<ThreadStatus>::iterator ii;
    for (ii = _threadStatus.begin(); ii != _threadStatus.end(); ii++) {
      ThreadStatus &status = *ii;
      if (status.done) {
	pthread_join(status.threadId, NULL);
	delete status.socket;
	if (_isVerbose) {
	  cerr << "******** Joined thread: " << status.threadId << endl;
	}
	_threadStatus.erase(ii);
	done = false;
	_numClients--;
	_lastActionTime = time(NULL);
	break;
      }

    } // ii

  } // while

  pthread_mutex_unlock(&_threadStatusMutex);

}

