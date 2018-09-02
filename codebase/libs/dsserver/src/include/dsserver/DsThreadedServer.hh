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

#ifndef DsThreadedServerINCLUDED
#define DsThreadedServerINCLUDED

#include <toolsa/umisc.h>

#include <string>
#include <pthread.h>
#include <list>

class Socket;
class ServerSocket;

#include <dsserver/DsProcessServer.hh>
#include <dsserver/DsServerMsg.hh>            

using namespace std;

//////////////////////////////////////////////////////
// 
// DsThreadedServer -- Base Server Class for DIDSS
// 
// Paddy McCarthy - Jan 1999 - original DsServer class
// Mike Dixon     - Oct 2000 - subclassing off DsProcessServer
// 
// Design Notes:
// -------------
//
// The class derives from DsProcessServer, and overrides the child spawning
// behavior to use threading instead. This allows the boss and worked
// threads to share memory. It is still an abstract base class, since
// HandleDataCommand() is a pure virtual function.
// 
// This threaded server uses a Boss and Worker thread model. 
//   o A Boss thread is initiated by an external call to waitForClients().
//   o Worker threads are spawned by the Boss to handle individual clients.
//   o The Boss does no management of Worker threads after they are
//       spawned. However, the number of clients at any time may be 
//       obtained through the _numClients data member.
// 
// The DsThreadedServer class is abstract (cannot be instantiated), because it
//   does not contain definitions of all the methods it needs to be a
//   fully-functional server. Specifically, it contains no behavior for
//   handling a client request once it is received. To use this class, 
//   create a subclass which defines the handleDataCommand() method, 
//   which will respond to requests from clients.
// 
//   The DsThreadedServer class contains the functionality that is
//   shared between all threaded DIDSS servers:
//    o Waits on an indicated port for client connections.
//    o Accepts client connections, and reads a client request, which
//        must be a DsServerMsg.
//    o Interprests the client DsServerMsg header to determine if the request
//        is for data or for server information.
//    o Upon interpreting the message header, calls the appropriate virtual
//        method for the request: handleServerCommand() or handleDataCommand().
//    o Inits the procmap when constructed.
//    o Registers with the procmap after every client Worker thread is spawned,
//        and every time waitForClients() times out waiting for clients.
//    
//   The DsThreadedServer class has these additional capabilities,
//   which subclasses may chose to use if they are relevant to a
//   particular server implementaion:
//    o Subclass may perform additional tasks every time waitForClients() times
//        out, by defining a virtual timeoutMethod().
//    o Subclass may perform additional tasks after every time waitForClients() 
//        spawns a Worker thread, by defining a virtual postHandlerMethod().
//    o Subclass may perform pre-exit tasks (when the server becomes
//        quiescent) by defining a virtual exitMethod().
//    o Constructor takes arguments such as port number, and max quiescent
//        secs, to support DIDSS server executables which must take -port
//        and -qmax args.
// 
//   Subclassing notes:
//    In general, when subclasses define special virtual methods such as 
//    timeoutMethod() and exitMethod(), these MUST first call the
//    DsThreadedServer:: version of the method before doing any other tasks.
//    Header comments indicate when this is necessary.
// 
//    The handleDataCommand() and handleServerCommand() methods are run
//    on Worker threads, and so should not modify and data members on the
//    class without first locking a mutex. Initial server implementations
//    use a fine-grained approach for mutex scope (one mutex per data member)
//    with good success. This approach reduces collisions between Worker
//    threads. It's also obvious what *should* be locked when a data member
//    is modified. The tradeoff is more mutexes and more mutex management.
//    Also note that it is good policy to lock mutexes when *READING* data
//    that is potentially being modified by more than one thread. When you 
//    define a mutex for a data member, use it for reads AND writes to the
//    data member.
//
// Maintenance Issues:
// -------------------
// 
// When coding and debugging this class I found it necessary to keep in 
//   mind two things at all times:
//     - Is the method I am working in called by the Boss, Worker, or both?
//     - Is any data member being modified? If so, what are the assumptions
//         about the modification of this data?
//
// Data modification assumptions:
// * Two data members are modified both by the Boss and Worker threads:
// *     _numClients
// *     _lastActionTime
// *   Coordination of the modifications is done through mutexes. It
// *   is absolutely necessary to obtain the appropriate mutex lock 
// *   before modifying these data members. It's recommended to obtain
// *   such locks before accessing the data. The tradeoff for potential
// *   reduction in performance is fewer *very-hard-to-find* bugs!
// *                                      -----------------
// * 
// * Some data members are updated by only the Boss thread:
// *     _isOkay
// *     _errString
// *   These data members are not protected by mutexes because there 
// *   is only one boss, so concurrency is not an issue.
// * 
// *   Worker threads should never modify this data, though there is
// *   no protection from it in the design. Providing this protection
// *   would require putting all the Worker thread functionality in 
// *   a separate class, which would add to the complexity and to the
// *   overhead of creating server subclasses (would require subclassing
// *   two classes instead of one).
// * 
// * Some data members are set in the constructor and should never be modified:
// *     _serverSocket;
// *     _executableName;
// *     _instanceName;
// *     _port;
// *     _maxQuiescentSecs;
// *     _maxClients;
// *     _isDebug;
// *     _isVerbose;
// *     _isSecure

class DsThreadedServer : public DsProcessServer {

public:

  // class for keeping status of threads

  class ThreadStatus {
  public:
    pthread_t threadId;
    Socket * socket;
    bool done;
  };

  // Constructor:
  //   o Registers with procmap
  //   o Opens socket on specified port
  //   o Updates the status of the server.
  //       Use isOkay() to determine status.
  // 
  // If maxQuiescentSecs is non-positive, quiescence checking is disabled.
  //
  DsThreadedServer(const string & executableName,
		   const string & instanceName,
		   int port,
		   int maxQuiescentSecs = -1,
		   int maxClients = 1024,
		   bool isDebug = false,
		   bool isVerbose = false,
		   bool isSecure = false,
                   bool isReadOnly = false);
  
  // Destructor. 
  //   Unregisters with the procmap.
  // 
  virtual ~DsThreadedServer();
    
protected:

  /////////////////////////////////////////////////////////
  // VIRTUAL METHODS FOR SUBCLASSES
  // 
  //   Subclasses must define some of these
  //              can define the rest if special functionality needed.
  // 

  // spawn()
  //
  // Spawn a child process for handling the client
  
  virtual void spawn(ServerSocketStruct * sss, Socket * socket);

  // timeoutMethod()
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
  // Threads: Called by Boss thread.
  // 
  // Returns: true if the server should continue to wait for clients,
  //          false if the server should return from waitForClients().
  // 
  virtual bool timeoutMethod();

  // postHandlerMethod()
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
  // Threads: Called by Boss thread.
  // 
  // Returns: true if the server should continue to wait for clients,
  //          false if the server should return from waitForClients().
  // 
  virtual bool postHandlerMethod();

  // exitMethod()
  // Called by the timer thread from _DsServer__checkQuiescence()
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
  // Threads: Called by the timer thread, a specialized Worker thread.
  // 
  // Returns: true if the server should continue with the exit.
  //          false if the server should not exit afterall.
  // 
  virtual bool exitMethod();

  // clientDone()
  //
  // Update the server to reflect that a client is finished.
  //
  // Threads: called by worked thread.
  //
  // This does nothing in the DsProcessServer() class, since we cannot
  // communicate with the main thread. It will be overridden in the
  // DsThreadServer class.

  virtual void clientDone();

  // Purge completed put threads
  
  virtual void purgeCompletedThreads();

  // 
  // END OF VIRTUAL METHODS.
  /////////////////////////////////////////////////////////

protected:

  string _rapDataDir;
  
  // Thread mutex variables and access methods.
  pthread_mutex_t _threadStatusMutex;
  pthread_mutex_t _procmapInfoMutex;

  // list of active threads - used for deciding when to join
  list<ThreadStatus> _threadStatus;
  
private:
  // Private methods with no bodies. DO NOT USE!
  // 
  DsThreadedServer();
  DsThreadedServer(const DsThreadedServer & orig);
  DsThreadedServer & operator = (const DsThreadedServer & other);

};

#endif
