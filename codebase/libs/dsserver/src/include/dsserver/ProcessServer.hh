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

#ifndef ProcessServerINCLUDED
#define ProcessServerINCLUDED

#include <string>
using namespace std;

class Socket;
class ServerSocket;

//////////////////////////////////////////////////////
// 
// ProcessServer -- Simple Base Server Class
//
// This class uses child processes for the main activities.
// 
// Mike Dixon - Feb 2009
// 
// Design Notes:
// -------------
// 
// This server spawns a child for each client connection received:
//   o A Boss process is initiated by an external call to waitForClients().
//   o Worker threads are spawned by the Boss to handle individual clients.
//   o The Boss does no management of Worker threads after they are
//     spawned. The the number of clients is determined by checking for
//     children which have exited.
// 
// The ProcessServer class is abstract (cannot be instantiated), because it
//   does not contain definitions of the handleClient() methods.
//   Specifically, it contains no behavior for
//   handling a client request once it is received. To use this class, 
//   create a subclass which defines the handleClient() method for
//   writing to and reading from the client.
// 
//   The ProcessServer class contains the following functionality:
//    o Waits on an indicated port for client connections.
//    o Accepts client connections
//    o Registers with the procmap after every client Worker thread is spawned,
//        and every time waitForClients() times out waiting for clients.
//      Note: PMU initialization must be done by the parent which instantiates 
//            this object.
//    
//   The ProcessServer class has additional capabilities, which subclasses
//   may chose to use if they are relevant to a particular server implementaion:
//    o Subclass may perform additional tasks every time waitForClients() times
//        out, by defining a virtual timeoutMethod().
//    o Subclass may perform additional tasks after every time waitForClients() 
//        spawns a Worker thread, by defining a virtual postHandlerMethod().
//    o Subclass may perform pre-exit tasks by defining a virtual exitMethod().
// 
//   Subclassing notes:
//    In general, when subclasses define special virtual methods such as 
//    timeoutMethod() and exitMethod(), these MUST first call the
//    ProcessServer::version of the method before doing any other tasks.
//    Header comments indicate when this is necessary.
// 
//    The handleClient() method is run in the child process.

class ProcessServer {
  
public:
  
  // Constructor:
  //   o Registers with procmap
  //   o Opens socket on specified port
  //   o Updates the status of the server.
  //       Use isOkay() to determine status.
  ProcessServer(const string & executableName,
		const string & instanceName,
		int port,
		int maxClients = 1024,
		bool isDebug = false,
		bool isVerbose = false);

  // Destructor. 
  //   Unregisters with the procmap.

  virtual ~ProcessServer();
    
  // Get the server status. Status is set in the constructor.

  bool isOkay() const { return _isOkay; };

  // Get the Error String. This has contents when an error is returned.

  string getErrString() const { return _errString; }

  // Set the server to use no threads (for debugging). The server will 
  //   block and not accept clients while a single client is handled.
  //   After the client is handled, another client will be accepted.
  // 
  void setNoThreadDebug(bool isNoThread) { _isNoThreadDebug = isNoThread; }
  bool isNoThreadDebug() const { return _isNoThreadDebug; }

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
  //               the return from timeoutMethod() or postHandlerMethod().
  //          -1 - something terrible happened.
  //               Do not call waitForClients() again after error!
  // 
  int waitForClients(int timeoutMSecs = 1000);

protected:

  // struct for passing args to thread

  typedef struct {
    Socket * socket;
    ProcessServer * server;
  } ServerSocketStruct;
  
  // data members

  // Is this a child process?

  bool _isChild;

  // Status indicating whether the constructor was able to
  //   create a functioning server.
  // 
  // Threads: Updated only by Boss thread.

  bool _isOkay;

  // The error message string.
  // 
  // Threads: Updated only by Boss thread.

  string _errString;

  // Flag for turning off threads for debugging.
  // 
  // Threads: Should only be modified before calling waitForClients().

  bool _isNoThreadDebug;

  // The server socket.
  // 
  // Threads: Should only be set in constructor.

  ServerSocket * _serverSocket;

  // Name of the executable.
  //
  // Threads: Should only be set in constructor.

  string _executableName;

  // Instance name for procmap:
  //   o "primary" for default (no param file)
  //   o "param"   if started with a param file
  //   o "something_unique" if started manually.
  //
  // Threads: Should only be set in constructor.

  string _instanceName;

  // Port at which waiting for connections:
  //   o Known port for ServerMgr and manual startups
  //   o Assigned by ServerMgr if started by ServerMgr
  // 
  // Threads: Should only be set in constructor.

  int _port;
    
  // Number of clients currently being handled.
  // 
  // Threads: Modified by Worker threads.
  //          Modified by Boss thread.
  //          Use mutex lock!

  int _numClients;

  // Maximum number of clients the server will handle before
  //   ceasing to accept clients.
  // 
  // Threads: Should only be set in constructor.

  int _maxClients;

  // Is this a debug version -- some messages provided.
  // Threads: Should only be set in constructor.

  int _isDebug;

  // Is this a verbose version -- lots of messages.
  // Threads: Should only be set in constructor.

  int _isVerbose;

  // keep track of debug printing
  // Threads: should only be set in main thread
  time_t _lastPrint;

  // communications timeout in msecs

  ssize_t _commTimeoutMsecs;

  ////////////////////////////////////////////////////////////
  // ACCESS FUNCTIONS FOR DATA MEMBERS

  // Get the number of clients.
  int getNumClients() const { return _numClients; }

  // Other accessors.
  bool isDebug() const { return _isDebug; }
  bool isVerbose() const { return _isVerbose; }
    
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

  // Handle a client connection.
  // 
  // Threads: Called by Worker threads.
  // 
  // Returns 0 if command handled successfully, -1 otherwise.
  //   Does not set the _errString member, as this is called by threads.
  //   If the subclass ever returns error condition, this server exits.
  // 
  // Abstract -- Must be defined by subclasses.
  //               NOTE:
  //                 Subclasses are expected to never return an error
  //                   condition from this method. All errors should be
  //                   handled nicely in some way by subclass.
  // 
  //                 If a subclass ever returns an error condition, 
  //                   this server exits.
  // 
  virtual int handleClient(Socket* socket) = 0;
    
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
  // This does nothing in the ProcessServer() class, since we cannot
  // communicate with the main thread. It will be overridden in the
  // DsThreadServer class.

  virtual void clientDone();

  // Purge completed put threads
  
  virtual void purgeCompletedThreads();

  // 
  // END OF VIRTUAL METHODS.
  /////////////////////////////////////////////////////////

  // Static function for servicing request.
  // This is called by the child or thread created for servicing the request.

  static void *__serveClient(void * svrsockstruct);

private:

  // Private methods with no bodies. DO NOT USE!
  // 
  ProcessServer();
  ProcessServer(const ProcessServer & orig);
  ProcessServer & operator = (const ProcessServer & other);

};

#endif
