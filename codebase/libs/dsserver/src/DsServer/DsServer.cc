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

#include <dsserver/DsServer.hh>
#include <dsserver/DsServerMsg.hh>
#include <didss/DsURL.hh>

#include <cassert>
#include <didss/RapDataDir.hh>

#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/procmap.h>
#include <toolsa/file_io.h>
#include <toolsa/Socket.hh>
#include <toolsa/ServerSocket.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>

#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;

// Constructor:
//   o Registers with procmap
//   o Opens socket on specified port
//   o Updates the status of the server.
//       Use isOkay() to determine status.
//
// If maxQuiescentSecs is non-positive, quiescence checking is disabled.
//
DsServer::DsServer(const string & executableName,
                   const string & instanceName,
                   int port,
                   int maxQuiescentSecs /* = -1*/,
                   int maxClients /* = 1024 */,
                   bool forkClientHandlers /* = false*/,
                   bool isDebug /* = false*/,
                   bool isVerbose /* = false*/,
		   bool isSecure /* = false*/ )
  : _isOkay(false),
    _errString(""),
    _isNoThreadDebug(false),
    _serverSocket(NULL),
    _executableName(executableName),
    _instanceName(instanceName),
    _port(port),
    _maxQuiescentSecs(maxQuiescentSecs),
    _numClients(0),
    _maxClients(maxClients),
    _forkClientHandlers(forkClientHandlers),
    _isDebug(isDebug),
    _isVerbose(isVerbose),
    _isSecure(isSecure)
{

    _serverSocket = NULL;

    // Make sure that the messaging flags are consistent.
    //   Can't have verbose without debug.
    if (_isVerbose) {
        if ( !_isDebug ) {
            cerr << "Warning: Server was set to verbose mode without "
                 << "being in debug mode. Making debug..." << endl;
        }

        _isDebug = true;
    }

    pthread_mutex_init(&_numClientsMutex, NULL);
    pthread_mutex_init(&_lastActionMutex, NULL);
    pthread_mutex_init(&_procmapInfoMutex, NULL);
    pthread_mutex_init(&_threadStatusMutex, NULL);
    updateLastActionTime();

    bool isRapDataDirSet = RapDataDir.isEnvSet();
    if (!isRapDataDirSet) {
        _errString  = RapDataDir.getEnvVarName();
        _errString += " environment variable not set.";
	TaStr::AddStr(_errString, "  ", DateTime::str());
        cerr << _errString << endl << endl;

        assert(RapDataDir.isEnvSet());
        return;
    }
    _rapDataDir = RapDataDir.location();

    struct stat statusStruct;
    int status = ta_stat(_rapDataDir.c_str(), &statusStruct);
    if (status < 0) {
        _errString  = "Could not stat ";
        _errString += RapDataDir.getEnvVarName();
        _errString += " file: ";
        _errString += _rapDataDir;
	TaStr::AddStr(_errString, "  ", DateTime::str());
        cerr << _errString << endl;

        assert(status >= 0);
        return;
    }

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
    if (PMU_init_done() != 0) {
        cerr << "ERROR: Procmap Init Was Called Before DsServer Was Constructed."
             << endl << endl;
        assert(PMU_init_done() == 0);
    }
    PMU_auto_init(_executableName.c_str(),
                  _instanceName.c_str(),
                  PROCMAP_REGISTER_INTERVAL);
    // Register with the procmap.
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
        _errString  = "Could not open ServerSocket: ";
	_errString += _serverSocket->getErrString();
	TaStr::AddStr(_errString, "  ", DateTime::str());
        if (_isDebug) {
            cerr << _errString << endl;
        }

        return;
    }

    if (_isVerbose) {
      cerr << "DsServer has opened ServerSocket at port " << _port << endl;
      cerr << "  _maxClients: " << _maxClients << endl;
    }

    // Set status.
    _isOkay = true;
}

// Destructor.
//   Unregisters with the procmap.
//
DsServer::~DsServer()
{
    // Don't lock mutex for unregister. Seems sketchy to try that here..
#ifndef DISABLE_PMU
    PMU_auto_unregister();
#endif
    if (_serverSocket != NULL) {
      delete (_serverSocket);
    }
}

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
int DsServer::waitForClients(int timeoutMSecs /* = 1000 */)
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

        _errString  = "Error in DsServer::waitForClients(): ";
        _errString += "Server does not have a valid and/or open ServerSocket.";
	TaStr::AddStr(_errString, "  ", DateTime::str());

        if (_isDebug) {
            cerr << _errString << endl;
        }
	umsleep(timeoutMSecs);
	
        return -1;
    }
    
    // Start the timer thread.
    if (!_isNoThreadDebug) {
        if (_isVerbose) {
            cerr << "Server is starting timer thread..." << endl;
        }
        pthread_t timerThread;
        int err = pthread_create(&timerThread, NULL,
                                 __checkQuiescence, this);
        if (err != 0) {
            _errString  = "Error in DsServer::waitForClients(): ";
            _errString += "Could not create timer thread: ";
            _errString += strerror(err);
	    TaStr::AddStr(_errString, "  ", DateTime::str());

            if (_isDebug) { 
                cerr << _errString << endl;
            }
 
            _isOkay = false;
            return -1; // Failure.
        }
	if (_isVerbose) {
	  cerr << "---> created timer thread, id: " << timerThread << endl;
	}
        pthread_detach(timerThread);
    }

    // Wait for connections.
    int numFailures = 0;
    time_t lastPrint = 0;
    while (1) {
        if (_isVerbose) {
	  time_t now = time(NULL);
	  if (now - lastPrint > 60) {
	    cerr << "Server waiting for client connections..." << endl;
	    lastPrint = now;
	  }
        }

        // Wait for connections. Blocks.
        //   The timer thread takes care of exiting if the server has been
        //   quiet for _maxQuiescentSecs.
        //
	// getClient() creates a new socket - it must be deleted by
	// the calling function, i.e. this one
        Socket * socket = _serverSocket->getClient(timeoutMSecs);
        if (socket == NULL) {
            if (_serverSocket->getErrNum() == SockUtil::TIMED_OUT) {
                // Timed out.
	        if (_isVerbose) {
		  time_t now = time(NULL);
		  if (now - lastPrint > 60) {
                    cerr << "Server timed out on getClient():" << endl;
		    cerr << "  " << DateTime::str() << endl;
                    cerr << "  " << _serverSocket->getErrString() << endl;
		    lastPrint = now;
		  }
                }

                // Todo: Wait for all the clients to finish???
                //         Otherwise have threads going while in timeoutMethod.

                // Call the timeout method to give control back to the
                //   user of this DsServer object.
                bool shouldContinue = timeoutMethod();
                if (!shouldContinue) {

                    // Todo: Wait for all the clients to finish???
                    //         Otherwise still have threads going.

                    if (_isDebug) {
                        cerr << "DsServer returning from waitForClients() "
                             << "because timeoutMethod() indicated to do so."
                             << endl;
			cerr << "  " << DateTime::str() << endl;
                    }

                    return 0; // Success.
                }

                numFailures = 0;
                continue;
            }
            else {
                // Something besides timeout happened while waiting.
                _errString  = "Error in DsServer::waitForClients(): ";
                _errString += "ServerSocket experienced error while waiting ";
                _errString += "for client:\n    ";
                _errString += _serverSocket->getErrString();
		TaStr::AddStr(_errString, "  ", DateTime::str());
               
                if (_isDebug) {
                    cerr << _errString << endl;
                }

                numFailures++;

                if (numFailures > 1000) {
                    // Big problem. Make this object invalid and return error.
                    _isOkay = false;
                    return -1; // Failure.
                }
                else {
                    // Temporary failure? Reset the socket and try again.
                    _serverSocket->resetState();
                    continue;
                }
            }
        } // End if (socket == NULL)

        numFailures = 0;
        
        // Spawn a thread if not in no-thread debug mode.

        if (_isVerbose) {
            if (_isNoThreadDebug) {
                cerr << "Server got a client. In No-Thread debug mode." << endl;
            }
            else {
                cerr << "Server got a client. Spawning a thread..." << endl;
            }
        }

        // Got a connection. Check and increment the client count.
        //   Two points of interest here:
        // 
        //     o This will block the server from accepting new clients.
        //         Other threads should minimize obtaining this mutex.
        //         The timer obtains this mutex only when it is about
        //           to exit b/c of extended quiescence.
        // 
        //     o There is a hole here. The timer could obtain a lock
        //         on the mutex while the client connects. The timer
        //         could think there are no clients and exit before the
        //         client count is incremented.
        // 
        // 
        string errMsg;
        bool accepted = true;
        pthread_mutex_lock(&_numClientsMutex);
        if (_maxClients >= 0 && _numClients >= _maxClients) {
            accepted = false;
            TaStr::AddInt(errMsg,
                          "Service Denied. Too many clients being handled: ",
                          _numClients);
        }
        else {
            int status = incrementNumClients();

            // Error handling, if increment failed.
            if (status == -1) {
                accepted = false;
                errMsg  = "Service Denied: ";
                errMsg += "Error in DsServer::waitForClients(): ";
                errMsg += "Could not increment client count:\n    ";
                errMsg += "The _numClientsMutex was not locked for increment.";
            }
        }
        pthread_mutex_unlock(&_numClientsMutex);

        // If the message was not accepted, reply with a service denied message.
        if (!accepted) {
            if (_isDebug) {
                cerr << errMsg << endl;
            }

            // Reply to the client with an error. One second wait.
            string statusString;
            int status = sendReply(socket, DsServerMsg::SERVICE_DENIED,
                                   errMsg, statusString, 1000);

            if (_isDebug && status < 0) {
                cerr << "Error: DsServer could not send SERVICE_DENIED "
                     << "message to client: " << errMsg << endl;
		cerr << "  " << DateTime::str() << endl;
            }

            // Continue -- The status doesn't really matter.
            delete socket;
            continue;
        }

        // Create the ServerSocketStruct -- it is owned by the thread
	// and will be deleted after the thread is done
        ServerSocketStruct * sss = new ServerSocketStruct;
        sss->socket = socket;
        sss->server = this;
        if (_isNoThreadDebug) {
            // Call the thread start func directly.
	    __serveClient(sss);
	    delete socket;
        }
        else {
            // Start a thread.
	    pthread_mutex_lock(&_threadStatusMutex);
            pthread_t thread;
            int err = pthread_create(&thread, NULL,
                                     __serveClient, sss);
            if (err != 0) {
                string errMsg  = "Error in DsServer::waitForClients(): ";
                       errMsg += "Could not create thread to handle client: ";
                       errMsg += strerror(err);

		pthread_mutex_unlock(&_threadStatusMutex);
                if (_isDebug) { 
                    cerr << errMsg << endl;
                }
 
                // Send reply to the client that there was an error.
                string statusString;           
                sendReply(socket, DsServerMsg::SERVER_ERROR,
                          errMsg, statusString, 1000);

                delete sss;
		delete socket;
                continue;
            }

	    if (_isVerbose) {
	      cerr << "---> created client thread, id: " << thread << endl;
	    }
	    // add a status object to the list
	    // this is used later to join the thread when it is done

 	    ThreadStatus status;
	    status.threadId = thread;
	    status.socket = socket;
	    status.done = false;
	    _threadStatus.push_back(status);
	    pthread_mutex_unlock(&_threadStatusMutex);

        }

        // Call the post-handler method.
        bool shouldContinue = postHandlerMethod();
        if (!shouldContinue) {

            // Todo: Wait for all the clients to finish???
            //         Otherwise still have threads going.

            if (_isDebug) {
                cerr << "DsServer returning from waitForClients() "
                     << "because postHandlerMethod() indicated to do so."
                     << endl;
		cerr << "  " << DateTime::str() << endl;
            }

            return 0; // Success.
        }
    }

    return 0; // Success.
}

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
//       NOTE:
//         Subclasses are expected to never return an error
//           condition from this method. All errors should be
//           handled nicely in some way by subclass.
//
//         If a subclass ever returns an error condition,
//           this server exits.
//
// Virtual
int DsServer::handleServerCommand(Socket * socket,
                                   const void * data, ssize_t dataSize)
{
    DsServerMsg msg;
    if (_isVerbose) {
        cerr << "Client handler thread disassembling message..." << endl;
    }
    int status = msg.disassemble(data, dataSize);
    if (status < 0) {
        string errMsg = "Error in DsServer::handleServerCommand(): ";
               errMsg += "Could not disassemble DsServerMsg.";

        if (_isDebug) {
            cerr << errMsg << endl;
        }

        // Reply to the client with a BAD_MESSAGE error. Ten sec wait.
        string statusString;
        sendReply(socket, DsServerMsg::BAD_MESSAGE,
                  errMsg, statusString, 10000);

        // Return success. We did the best we could...
        return 0;
    }

    // Check that this is a server command, as DsServer determined it is.
    if (msg.getMessageCat() != DsServerMsg::ServerStatus) {
        string errMsg  = "Error in DsServerMgr::handleServerCommand(): ";
               errMsg += "Message is not a server command message.";

        if (_isDebug) {
            cerr << errMsg << endl;
        }

        // Send error reply to client.
        string statusString;
        sendReply(socket, DsServerMsg::SERVER_ERROR,
                  errMsg, statusString, 10000);

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
          pthread_mutex_lock(&_numClientsMutex);
          int numClients = _numClients;
          pthread_mutex_unlock(&_numClientsMutex);

          msg.addInt(numClients);
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
    }

    void * msgToSend = msg.assemble();
    ssize_t msgLen = msg.lengthAssembled();

    // Send the message with a ten second wait.
    status = socket->writeMessage(0, msgToSend,
                                  msgLen, 10000); // Ten sec wait.

    if (status < 0) {
        if (_isDebug) {
            cerr << "Error in DsServer::handleServerCommand(): "
                 << "Could not send reply message: "
                 << socket->getErrString() << endl;
	    cerr << "  " << DateTime::str() << endl;
        }

        // Do not return with failure, as there is no way to
        //   recover gracefully.
    }

    if (isShutdown) {
        // Todo: Wait for all the threads to end?
        //       To make this work, need to block new clients.
        cerr << "DsServer::handleServerCommand" << endl;
	cerr << "  " << DateTime::str() << endl;
        cerr << "  Exiting because isShutDown is true" << endl;
        exitMethod();
        exit(0);
    }

    return 0;
}

// Called from waitForClients() on timeout.
// Perform any special operations that need to happen
//   when the server times out waiting for clients.
//   Note that threads serving clients may still be running
//   when this method is called, but no more clients will be accepted.
//
// Subclasses should define this if they need control occasionally
//   while waiting for clients. If a subclass defines this method,
//   it *MUST* call the base class version before executing its own
//   code.
//
// Design Note: If a derived class needs to give control to the
//   main application, it will be tempting to introduce knowledge
//   of the main app into the derived class. That is not the intention
//   here -- it would be better to accomplish this through using a
//   friend function.
//
// Threads: Called by Boss thread.
//
// Returns: true if the server should continue to wait for clients,
//          false if the server should return from waitForClients().
//
// Virtual
bool DsServer::timeoutMethod()
{
#ifndef DISABLE_PMU
    // Register with the procmap.
    pthread_mutex_lock(&_procmapInfoMutex);
    string pmuStr;
    TaStr::AddInt(pmuStr, "Listening, port: ", _port, false);
    PMU_auto_register((char *) pmuStr.c_str());
    pthread_mutex_unlock(&_procmapInfoMutex);
#endif

    // purge completed threads
    _purgeCompletedThreads();

    return true;
}

// Called from waitForClients() after Worker thread is spawned.
// Perform any special operations that need to happen
//   when the server is finished handling a client.
//   Note that threads serving clients may still be running
//   when this method is called, but no more clients will be accepted.
//
// Subclasses should define this if they need control occasionally
//   while waiting for clients. If a subclass defines this method,
//   it *MUST* call the base class version before executing its own
//   code.
//
// Design Note: If a derived class needs to give control to the
//   main application, it will be tempting to introduce knowledge
//   of the main app into the derived class. That is not the intention
//   here -- it would be better to accomplish this through using a
//   friend function.
//
// Threads: Called by Boss thread.
//
// Returns: true if the server should continue to wait for clients,
//          false if the server should return from waitForClients().
//
// Virtual
bool DsServer::postHandlerMethod()
{
#ifndef DISABLE_PMU
    // Register with the procmap.
    pthread_mutex_lock(&_procmapInfoMutex);
    string pmuStr;
    TaStr::AddInt(pmuStr, "Received a client, port: ", _port, false);
    PMU_auto_register((char *) pmuStr.c_str());
    pthread_mutex_unlock(&_procmapInfoMutex);
#endif

    return true;
}

// Called by the timer thread from __checkQuiescence()
// Perform any special operations that need to happen
//   when the server is about to exit due to quiescence.
//   Note that this method will never get called if a non-positive
//   value was supplied for maxQuiescentSecs.
//
// Subclasses should define this if they need to do anything special
//   before the application exits. If a subclass defines this method,
//   it *MUST* call the base class version before executing its own
//   code.
//
// Design Note: If a derived class needs to give control to the
//   main application, it will be tempting to introduce knowledge
//   of the main app into the derived class. That is not the intention
//   here -- it would be better to accomplish this through using a
//   friend function.
//
// Threads: Called by the timer thread, a specialized Worker thread.
//
// Returns: true if the server should continue with the exit.
//          false if the server should not exit afterall.
//
// Virtual
bool DsServer::exitMethod()
{

    // purge completed threads
    _purgeCompletedThreads();

#ifndef DISABLE_PMU
    // unregister with procmap
    pthread_mutex_lock(&_procmapInfoMutex);
    PMU_auto_unregister();
    pthread_mutex_unlock(&_procmapInfoMutex);
#endif
    return true;
}

// Send a simple reply to the client, containing the passed-in
//   error information.
//
// Threads: Called by Worker threads.
//          Called by Boss thread.
//
// Returns:  0 on success
//          -1 on failure, in which case _errString contains a message.
//
int DsServer::sendReply(Socket * socket,
                        DsServerMsg::msgErr errCode, const string & errMsg,
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
        errString  = "Error in DsServer::sendReply(): ";
        errString += "Could not send error reply message: ";
        errString += socket->getErrString();
        return -1;
    }

    return 0;
}

// 
// Private Methods and Friend Functions.
// 

// Update the last action time with the current time.
//   Note that this potentially blocks b/c it locks
//   the _lastActionMutex.
//
// Threads: Called by Worker threads.
//
void DsServer::updateLastActionTime()
{
    pthread_mutex_lock(&_lastActionMutex);
    ulocaltime(&_lastActionTime);
    pthread_mutex_unlock(&_lastActionMutex);
}

// Update the server to reflect that a client is finished.
//   Updates last action time.
//   Decrements the client count.
//   (Uses nested mutex locks to make this atomic with respect to the timer)
//
// Threads: Called by Worker threads.
//
void DsServer::clientDone()
{
    // Update the last action time.
    pthread_mutex_lock(&_lastActionMutex);
    ulocaltime(&_lastActionTime); // Equiv to updateLastActionTime() w/o lock.

    // Remove this thread from the client count.
    //   This is done while the last action mutex is still locked:
    //     1) To avoid giving the timer thread an incorrect count.
    //     2) To avoid blocking the server thread unneccesarily.
    // 
    pthread_mutex_lock(&_numClientsMutex);
    int status = decrementNumClients();
    pthread_mutex_unlock(&_numClientsMutex);

    // Error handling, if decrement failed.
    if (status == -1) {

        // Don't use _errString in this method -- called by threads.
        string newError  = "Error in DsServer::clientDone(): ";
               newError += "Could not decrement client count:\n    ";
               newError += "The _numClientsMutex was not locked for increment.";
	TaStr::AddStr(newError, "  ", DateTime::str());

        if (_isDebug) {
            cerr << newError << endl;
        }
    }

    pthread_mutex_unlock(&_lastActionMutex);
    
    // set the thread status to done

    _setThreadStatusDone(pthread_self());

}

// Change the client count by the indicated amount.
//   Note that a mutex lock should be obtained before calling these methods.
//   An error will be issued if pthread_mutex_trylock succeeds.
//
// Threads: Called by Worker threads.
//          Called by Boss thread.
//
// Returns: -1 if method is able to obtain lock. This is an error.
//           0 otherwise. Success.
//
int DsServer::incrementNumClients()
{
    return changeNumClients(1);
}

// Change the client count by the indicated amount.
//   Note that a mutex lock should be obtained before calling these methods.
//   An error will be issued if pthread_mutex_trylock succeeds.
//
// Threads: Called by Worker threads.
//          Called by Boss thread.
//
// Returns: -1 if method is able to obtain lock. This is an error.
//           0 otherwise. Success.
//
int DsServer::decrementNumClients()
{
    return changeNumClients(-1);
}

// Protected method used to implement increment and decrementNumClients.
//
// Threads: Called indirectly by Worker threads.
//          Called indirectly by Boss thread.
//
//   Returns: -1 if method was able to obtain lock. This is an error.
//             0 otherwise. Success.
//
int DsServer::changeNumClients(int delta)
{
    int failure = pthread_mutex_trylock(&_numClientsMutex);

    if (failure != EBUSY) {
        // Free the mutex lock if it was obtained.
        // 
        if (failure == 0) {
            pthread_mutex_unlock(&_numClientsMutex);
        }

        if (_isDebug) {
            cerr << "BAD ERROR in DsServer::changeNumClients(): "
                 << "_numClientsMutex was not locked for the change." << endl;
        }

        return -1;
    }

    _numClients += delta;
    return 0;
}

// 
// __checkQuiescence(void * svr)
// 
// Polls the last action time, to determine if it was more than 
//   maxQuiescentSecs before the current time.
// 
// If the above condition is met, checks that the server has no clients,
//   to eliminate the possibility that a client has been busy with a
//   request that whole time. (last action time is updated when client
//   starts and finishes.
// 
// If both conditions are met, this thread calls exit(0) b/c this
//   server is stale.
// 
// Design note:
//   This approach is used in order to minimize the amount of time the
//     numClientsMutex is locked, since locking it blocks the server
//     from spawning clients.
//   Another, more efficient, approach would be to check the time only
//     when the client count goes to zero. But this would require polling
//     the client count, potentially blocking out the server. If timed
//     waits on condition variables (POSIX.1g) were more widely available,
//     that would be ideal way to handle this -- do a timed wait on
//     client count condition variable reaching zero, and check for
//     stale server on timeout (since client count may stay at zero).
// 
void *DsServer::__checkQuiescence(void * svr)
{
    DsServer * server = (DsServer *) svr;

    int maxQuiescentSecs = server->getMaxQuiescentSecs();

    // If there is no specified wait time, let this thread die.
    if (maxQuiescentSecs <= 0) {
        return NULL;
    }

    while (1) {
        int waitSecs = maxQuiescentSecs;

        // Get the last action time.
        pthread_mutex_t lastActionMutex = server->getLastActionMutex();
        pthread_mutex_lock(&lastActionMutex);
        date_time_t lastActionStruct = server->getLastActionTime();
        time_t lastActionTime = lastActionStruct.unix_time;
        pthread_mutex_unlock(&lastActionMutex);

        // Get the current time.
        date_time_t currentTimeStruct;
        ulocaltime(&currentTimeStruct);            

        // Decrease the wait time by the amount of time already waited.
        // 
        //   Note that this step is skipped if the time already quiescent
        //   is greater than the wait time. This indicates that a slow
        //   client is being served. Not sure whan that client will be
        //   done, so just wait maxQuiescentSecs.
        // 
        time_t quiescentTime = currentTimeStruct.unix_time - lastActionTime;
        if (quiescentTime > 0 && waitSecs > quiescentTime) {
            waitSecs -= quiescentTime;
        }

        // Wait for the determined number of seconds.
        sleep(waitSecs);

        // Check the last action time again. If more that 
        //   maxQuiescentSecs ago, this server is potentially stale.
        //   Need to check if there are any clients being served.
        // 
        // Could implement this by checking old against current
        //   last action time, but it wasn't very readable.
        // 
        pthread_mutex_lock(&lastActionMutex);
        lastActionStruct = server->getLastActionTime();
        lastActionTime = lastActionStruct.unix_time;
        ulocaltime(&currentTimeStruct);
        quiescentTime = currentTimeStruct.unix_time - lastActionTime;
        if (quiescentTime >= maxQuiescentSecs) {
            // Long time since last action, make sure that 
            //   server has no clients.
            pthread_mutex_t numClientsMutex = server->getNumClientsMutex();
            pthread_mutex_lock(&numClientsMutex);
            if (server->getNumClients() == 0) {
                if (server->isDebug()) {
                    cerr << "Server has been quiescent for " 
                         << quiescentTime
                         << " seconds. Exiting this server." << endl;
		    cerr << "  " << DateTime::str() << endl;
                }

                bool shouldExit = server->exitMethod();
                if (shouldExit) {
	    	    cerr << "DsServer::__checkQuiescence" << endl;
		    cerr << "  " << DateTime::str() << endl;
 		    cerr << "  Exiting - maxQuiescentSecs has passed" << endl;
		    cerr << "  maxQuiescentSecs: " << maxQuiescentSecs << endl;
                    exit(0);
                }

                if (server->isDebug()) {
                    cerr << "    Exit was cancelled by exitMethod()." << endl;
                }
            }

            pthread_mutex_unlock(&numClientsMutex);
        }

        pthread_mutex_unlock(&lastActionMutex);
    }

    return NULL;
}

// __serveClient(void * svrsockstruct)
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
void * DsServer::__serveClient(void * svrsockstruct)
{
    ServerSocketStruct * sss = (ServerSocketStruct *) svrsockstruct;
    Socket * socket = sss->socket;
    DsServer * server = sss->server;

    // Delete the ServerSocketStruct -- it is owned by this thread.
    delete sss;

    // Must have a valid server.
    if (server == NULL) {
      // No way to tell client. This is a bad error.
      cerr << "Error: Got NULL server in __serveClient." << endl;
      return NULL;
    }

    if (server->isVerbose()) {
        cerr << "Client handler thread started..." << endl;
    }

    // Get the server mutexes, so can lock them later.
    // pthread_mutex_t numClientsMutex = server->getNumClientsMutex();

    // Register this as the last action time on the server.
    server->updateLastActionTime();

    // Should have a valid open socket now.
    if (socket == NULL) {
        if (server->isDebug()) {
            cerr << "Error: Got NULL socket in DsServer::__serveClient." << endl;
        }
        // No way to tell client. This is a bad error.
        // Remove this thread from the client count.
        server->clientDone();
        return NULL;
    }
    if (socket->hasState(SockUtil::STATE_ERROR)) {
        if (server->isDebug()) {
            cerr << "Error: Socket received by DsServer::__serveClient has "
                 << "error state:" << endl
                 << socket->getErrString() << endl;
	    cerr << "  " << DateTime::str() << endl;
        }
        // No way to tell client. This is a bad error.
        // Remove this thread from the client count.
        server->clientDone();
        return NULL;
    }

    if (server->isVerbose()) {
        cerr << "Client handler thread reading from socket..." << endl;
    }

    // Read from the socket.
    int status = socket->readMessage();
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
        if (server->isDebug()) {
            cerr << errMsg << endl;
        }

        // Send error reply to client.
        string statusString;
        server->sendReply(socket, DsServerMsg::SERVER_ERROR,
                          errMsg, statusString, 10000);

        // Remove this thread from the client count.
        server->clientDone();
        return NULL;
    }

    if (server->isVerbose()) {
        cerr << "Client handler thread performed successful read." << endl;
    }

    DsServerMsg msg;
    const void * data = socket->getData();
    size_t dataSize = socket->getNumBytes();

    if (server->isVerbose()) {
        cerr << "Client handler thread Read " << dataSize << " Bytes." << endl;
        cerr << "Client handler thread decoding message..." << endl;
    }

    // Decode the incoming message header only.
    status = msg.decodeHeader(data, dataSize);
    if (status < 0) {
        // Could not decode message header -- bail.
        string errMsg  = "Error: Message from client could not be decoded. ";
               errMsg += "Either the message is too small, or it has an ";
               errMsg += "invalid category.";
        if (server->isDebug()) {
            cerr << errMsg << endl;
        }
            
        // Send error reply to client.
        string statusString;
        server->sendReply(socket, DsServerMsg::BAD_MESSAGE,
                          errMsg, statusString, 10000);

        // Remove this thread from the client count.
        server->clientDone();
        return NULL;
    }

    // Determine if this is a server command or a task request.
    int success = 0;
    DsServerMsg::category_t category = msg.getMessageCat();
    if (category == DsServerMsg::ServerStatus) {
        success = server->handleServerCommand(socket, data, dataSize);
    }
    else {
        success = server->handleDataCommand(socket, data, dataSize);
    }
 
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
      string newError  = "Error in DsServer::__serveClient: ";
      newError += "Could not handle message.\n";
      newError += DateTime::str();
      cerr << newError << endl;
 
        // Remove this thread from the client count.
        server->clientDone();

        // Exit if this is a debug server.
        // 
        if (server->isDebug()) {
            // Todo: Wait for all the threads to end?
            //       To make this work, need to block new clients.

            server->exitMethod();
	    cerr << " DsServer::__serveClient" << endl;
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

void DsServer::_setThreadStatusDone(const pthread_t thread_id)
{
  if (_isNoThreadDebug) {
    return;
  }
  pthread_mutex_lock(&_threadStatusMutex);
  bool found = false;
  list<ThreadStatus>::iterator ii;
  for (ii = _threadStatus.begin(); ii != _threadStatus.end(); ii++) {
    ThreadStatus &status = *ii;
    if (pthread_equal(status.threadId, thread_id)) {
      status.done = true;
      found = true;
      break;
    }
  }
  if (!found) {
    cerr << "ERROR - DsServer::_setThreadStatusDone" << endl;
    cerr << "  " << DateTime::str() << endl;
    cerr << "  Cannot find thread id in list: " << thread_id << endl;
  }
  pthread_mutex_unlock(&_threadStatusMutex);
}

////////////////////////////////////////////////////////////////
// Purge completed put threads

void DsServer::_purgeCompletedThreads()

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
	break;
      }

    } // ii

  } // while

  pthread_mutex_unlock(&_threadStatusMutex);

}

