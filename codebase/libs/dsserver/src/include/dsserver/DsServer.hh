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

#ifndef DsServerINCLUDED
#define DsServerINCLUDED

#include <toolsa/umisc.h>

#include <string>
#include <pthread.h>
#include <list>
using namespace std;

class Socket;
class ServerSocket;

#include <dsserver/DsServerMsg.hh>            

//////////////////////////////////////////////////////
// 
// DsServer -- Base Server Class for DIDSS
// 
//             Paddy McCarthy - Jan 1999
// 
// Design Notes:
// -------------
// 
// This threaded server uses a Boss and Worker thread model. 
//   o A Boss thread is initiated by an external call to waitForClients().
//   o Worker threads are spawned by the Boss to handle individual clients.
//   o The Boss does no management of Worker threads after they are
//       spawned. However, the number of clients at any time may be 
//       obtained through the _numClients data member.
//   o A special "timer" thread runs continuously and checks for server
//       quiescence. If the server is quiescent for an excessive period
//       of time (more than _maxQuiescentSecs), the timer thread calls 
//       exit(0). For ease of explanation the timer thread is considered
//       a Worker thread in the following description, though it's pay is
//       higher because it's a specialist.
// 
// The DsServer class is abstract (cannot be instantiated), because it
//   does not contain definitions of all the methods it needs to be a
//   fully-functional server. Specifically, it contains no behavior for
//   handling a client request once it is received. To use this class, 
//   create a subclass which defines the handleDataCommand() method, 
//   which will respond to requests from clients.
// 
//   The DsServer class contains the functionality that is shared between 
//   all DIDSS servers:
//    o Waits on an indicated port for client connections.
//    o Accepts client connections, and reads a client request, which
//        must be a DsServerMsg.
//    o Interprets the client DsServerMsg header to determine if the request
//        is for data or for server information.
//    o Upon interpreting the message header, calls the appropriate virtual
//        method for the request: handleServerCommand() or handleDataCommand().
//    o Inits the procmap when constructed.
//    o Registers with the procmap after every client Worker thread is spawned,
//        and every time waitForClients() times out waiting for clients.
//    
//   The DsServer class has these additional capabilities, which subclasses
//   may chose to use if they are relevant to a particular server implementaion:
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
//    timeoutMethod() and exitMethod(), these MUST first call the DsServer::
//    version of the method before doing any other tasks. Header comments
//    indicate when this is necessary.
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
// *     _forkClientHandlers;
// *     _isDebug;
// *     _isVerbose;
// *     _isSecure

class DsServer {

public:

    // Constructor:
    //   o Registers with procmap
    //   o Opens socket on specified port
    //   o Updates the status of the server.
    //       Use isOkay() to determine status.
    // 
    // If maxQuiescentSecs is non-positive, quiescence checking is disabled.
    //
    DsServer(const string & executableName,
             const string & instanceName,
             int port,
             int maxQuiescentSecs = -1,
             int maxClients = 1024,
             bool forkClientHandlers = false,
             bool isDebug = false,
             bool isVerbose = false,
	     bool isSecure = false);

    // Destructor. 
    //   Unregisters with the procmap.
    // 
    virtual ~DsServer();
    
    // Get the server status. Status is set in the constructor.
    // 
    bool isOkay() const { return _isOkay; };

    // Get the Error String. This has contents when an error is returned.
    // 
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
    //                 the return from timeoutMethod() or postHandlerMethod().
    //          -1 - something terrible happened.
    //                  Do not call waitForClients() again after error!
    // 
    int waitForClients(int timeoutMSecs = 1000);

protected:

    // Status indicating whether the constructor was able to
    //   create a functioning server.
    // 
    // Threads: Updated only by Boss thread.
    // 
    bool _isOkay;

    // The error message string.
    // 
    // Threads: Updated only by Boss thread.
    // 
    string _errString;

    // Flag for turning off threads for debugging.
    // 
    // Threads: Should only be modified before calling waitForClients().
    // 
    bool _isNoThreadDebug;

    // The server socket.
    // 
    // Threads: Should only be set in constructor.
    // 
    ServerSocket * _serverSocket;

    // Name of the executable.
    //
    // Threads: Should only be set in constructor.
    //
    string _executableName;

    // Instance name for procmap:
    //   o "primary" for default (no param file)
    //   o "param"   if started with a param file
    //   o "something_unique" if started manually.
    //
    // Threads: Should only be set in constructor.
    //   
    string _instanceName;

    // Port at which waiting for connections:
    //   o Known port for ServerMgr and manual startups
    //   o Assigned by ServerMgr if started by ServerMgr
    // 
    // Threads: Should only be set in constructor.
    // 
    int _port;
    
private:

    // Last time a client connected or disconnected.
    // 
    // Threads: Modified by Worker threads.
    //          Modified by Boss thread.
    //          Use mutex lock!
    // 
    date_time_t _lastActionTime;

    // Maximum number of seconds The server will wait between
    //   client connections before exiting with success status.
    // 
    // The server will call exit(0) when _maxQuiescentSecs has passed
    //   since the last client was finished being served.
    // 
    // If this is a non-positive value, quiescence checking is disabled.
    // 
    // Threads: Should only be set in constructor.
    // 
    int _maxQuiescentSecs;

    // Number of clients currently being handled.
    // 
    // Threads: Modified by Worker threads.
    //          Modified by Boss thread.
    //          Use mutex lock!
    // 
    int _numClients;

    // Maximum number of clients the server will handle before
    //   ceasing to accept clients.
    // 
    // Threads: Should only be set in constructor.
    // 
    int _maxClients;

    // Fork the handling of client requests, rather than
    //   using threads. Threads are the default if 
    //   available.
    // 
    // Threads: Should only be set in constructor.
    // 
    bool _forkClientHandlers;

protected:
    // Is this a debug version -- some messages provided.
    // 
    // Threads: Should only be set in constructor.
    // 
    int _isDebug;

    // Is this a verbose version -- lots of messages.
    // 
    // Threads: Should only be set in constructor.
    // 
    int _isVerbose;

    // Is this running in secure mode?
    // 
    // Threads: Should only be set in constructor.
    // 
    int _isSecure;

    /////////////////////////////////////////////////////////
    // VIRTUAL METHODS FOR SUBCLASSES
    // 
    //   Subclasses must define some of these
    //              can define the rest if special functionality needed.
    // 

    // Handle a client's (request for)/(submission of) data. The passed
    //   data is a DsServerMsg.
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
    virtual int handleDataCommand(Socket* socket,
                                   const void * data, ssize_t dataSize) = 0;
    
    // Handle a command from a client that requests information 
    //   directly from and about the server. The passed
    //   data is a DsServerMsg.
    // 
    // Subclasses should define this only if they have special commands
    //   to handle. Always call the superclass's method for
    //   commands not recognized by the subclass!
    // 
    // Handles the following commands:
    //       IS_ALIVE,              Returns empty message.      
    //       GET_NUM_CLIENTS,       Returns integer.
    //       SHUTDOWN,              Returns empty message, then calls exit(0).
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
    virtual int handleServerCommand(Socket* socket,
                                     const void * data, ssize_t dataSize);

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
    virtual bool timeoutMethod();

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
    virtual bool postHandlerMethod();

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
    virtual bool exitMethod();

    // 
    // END OF VIRTUAL METHODS.
    /////////////////////////////////////////////////////////


    // Send a simple reply to the client, containing the passed-in
    //   error information.
    // 
    // Threads: Called by Worker threads.
    //          Called by Boss thread.
    // 
    // Returns:  0 on success
    //          -1 on failure, in which case _errString contains a message.
    // 
    int sendReply(Socket * socket,
                  DsServerMsg::msgErr errCode, const string & errMsg,
                  string & errString, int wait_msecs = 10000);

    // Get the $RAP_DATA_DIR environment variable for the
    //   application. This is guaranteed to be a valid directory
    //   if _isOkay is true.
    // 
    const string & getRapDataDir() { return _rapDataDir; }

private:
    // Get the last action time. 
    //   Use mutex lock if you care about data integrity.
    // 
    //   Last action time is the most recent of:
    //     o The last time a Worker thread was started, or
    //     o The last time a Worker thread finished.
    // 
    //   This is used to determine server quiescence. It is only
    //     useful for that purpose if there are no clients, as
    //     a slow client might make the server appear quiescent
    //     when it is actually doing work to serve the client.
    // 
    // Threads: Called by Worker threads.
    //          Called by Boss thread.
    // 
    // Returns: The last action time, as described above.
    // 
    const date_time_t & getLastActionTime() const  { return _lastActionTime;   }

protected:
    // Get the number of clients.
    //   Use mutex lock if you care about data integrity.
    // 
    int getNumClients() const { return _numClients;       }

    // Other accessors.
    int getMaxQuiescentSecs() const { return _maxQuiescentSecs; }
    bool isDebug() const { return _isDebug; }
    bool isVerbose() const { return _isVerbose; }
    bool isSecure() const { return _isSecure; }
    
private:
    // Update the last action time with the current time.
    //   Note that this potentially blocks b/c it locks
    //   the _lastActionMutex.
    // 
    // Threads: Called by Worker threads.
    // 
    void updateLastActionTime();

    // Update the server to reflect that a client is finished.
    //   Updates last action time.
    //   Decrements the client count.
    //   (Uses nested mutex locks to make this atomic with respect to the timer)
    // 
    // Threads: Called by Worker threads.
    // 
    void clientDone();

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
    int incrementNumClients();
    int decrementNumClients();

    // Private method used to implement increment and decrementNumClients.
    // 
    // Threads: Called indirectly by Worker threads.
    //          Called indirectly by Boss thread.
    // 
    //   Returns: -1 if method was able to obtain lock. This is an error.
    //             0 otherwise. Success.
    // 
    int changeNumClients(int delta);

    // Static functions for thread creation.
    //   See implementation for comments regarding behavior.
    // 
    // Threads: These are the start functions for Worker threads.
    // 
    static void *__checkQuiescence(void * svr);
    static void *__serveClient(void * svrsockstruct);

// Public mutex accessors for the threaded version of the server.
// 
public:
    pthread_mutex_t & getNumClientsMutex()       { return _numClientsMutex; }
    pthread_mutex_t & getLastActionMutex()       { return _lastActionMutex; }
    pthread_mutex_t & getProcmapInfoMutex()      { return _procmapInfoMutex; }
    pthread_mutex_t & getthreadStatusMutex()     { return _threadStatusMutex; }

private:
    string _rapDataDir;

    // Thread mutex variables and access methods.
    pthread_mutex_t _numClientsMutex;
    pthread_mutex_t _lastActionMutex;
    pthread_mutex_t _procmapInfoMutex;
    pthread_mutex_t _threadStatusMutex;

    // struct for passing args to thread
    typedef struct {
      Socket * socket;
      DsServer * server;
    } ServerSocketStruct;
  
    // list of active threads - used for deciding when to join
    class ThreadStatus {
    public:
      pthread_t threadId;
      Socket * socket;
      bool done;
    };
    list<ThreadStatus> _threadStatus;
  
    void _setThreadStatusDone(const pthread_t thread_id);
    void _purgeCompletedThreads();

    // Private methods with no bodies. DO NOT USE!
    // 
    DsServer();
    DsServer(const DsServer & orig);
    DsServer & operator = (const DsServer & other);
};

#endif
