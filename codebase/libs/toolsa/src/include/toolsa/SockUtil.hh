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
////////////////////////////////////////////////////////////////////
// SockUtil.hh
//
// SockUtil class. This is an abstract base class for the
//                   wrappers of the sockutils functions.
//
// Instantiate one of the derived classes.
//
// Test with $CVSROOT/libs/toolsa/test/SocketTest
//
// Paddy McCarthy, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// Dec 1998
//
////////////////////////////////////////////////////////////////////

// Todo: Check socket state before performing any operations.

#ifndef SOCKUTIL_HH
#define SOCKUTIL_HH


#include <string>
using namespace std;

class SockUtil {

  public:
  
    //////////////
    // constructor  
  
    SockUtil();
  
    //////////////
    // destructor
    //   Closes socket if open.
  
    virtual ~SockUtil();
  
    //////////////
    // Socket types.
    //   One for each derived class.
  
    typedef enum {
      SOCK_UTIL        = 0x0001,
      SOCKET           = 0x0002,
      SERVER_SOCKET    = 0x0004,
      HTTP_SOCKET      = 0x0008
    } SocketType;
  
    //////////////
    // Socket states.
    //   One for each state-changing operation on the socket.
  
    typedef enum {
      STATE_NONE               = 0x0000, // Nothing done yet.
      STATE_OPENED             = 0x0001, // Socket open or ServerSocket open.
      STATE_SERVER_OPENED      = 0x0002, // Socket open or ServerSocket open.
      STATE_CLOSED             = 0x0004, // Socket open or ServerSocket open.
      STATE_WAITING_FOR_CLIENT = 0x0008, // ServerSocket only.

      STATE_READLAST           = 0x0010, // Last operation was a read.
      STATE_WROTELAST          = 0x0020, // Last operation was a write.
  
      STATE_ERROR              = 0x8000
    } SocketState;
    
    //////////////
    // Socket Errors.                                            
    //   A unique set of error codes which are uniform across all the 
    //     operations.
    // 
    //   Note that the only code of real interest is TIMED_OUT. The other
    //     codes are mostly useful for debugging.
    // 

    typedef enum {

      TIMED_OUT            = -9,

      SERVER_OPEN_FAILED   = -10,
      SERVER_BIND_FAILED   = -11,
      SERVER_LISTEN_FAILED = -12,

      SELECT_FAILED        = -13,
      ACCEPT_FAILED        = -14,
      UNKNOWN_HOST         = -15,
      SOCKET_FAILED        = -16,
      CONNECT_FAILED       = -17,

      BAD_BYTE_COUNT       = -18,

      READ_HEADER_FAILED   = -19,
      READ_MESSAGE_FAILED  = -20,
      WRITE_HEADER_FAILED  = -21,
      WRITE_MESSAGE_FAILED = -22,

      BAD_MAGIC_COOKIE     = -23,

      UNEXPECTED           = -100

    } SocketErrors;

    ///////////////
    // close()  
    //                                                                  
    // Close socket                                           
    //   Abstract method. All derived classes must define this.
    //
    virtual void close() = 0;
  
    //////////////////////////////////////
    // Run-time type identification.
  
    bool isA(SocketType type) { return (_objectType & type); }
    int getObjectType() const { return _objectType; }
  
    //////////////////////////////////////
    // code number for error as applicable
  
    int getErrNum() const { return (_errNum); }
  
    //////////////////////////////////////
    // String associated with error.
  
    const string & getErrString() const { return (_errString); }
    const string & getErrStr() const { return (_errString); }
    const string & getErrorStr() const { return (_errString); }
  
    //////////////////////////////////////
    // Socket state.
  
    int getState() const { return (_state); }
    bool hasState(SocketState state) { return (_state & state); }
    void addState(SocketState state) { _state |= state; }
    void removeState(SocketState state) { _state &= ~state; }
    void resetState()                { _state = STATE_NONE; }
  
  protected:
  
    int    _objectType;      // Run-time type identification.
    int    _state;           // Socket state -- mask of all accumulated states.
    int    _errNum;          // Error number which caches last error.
    string _errString;       // Error string which caches last error.
    bool   _timedOut;        // flag set if select call times out
  
  void _setSocketOptions(const int sd);

  int _readSelectPmu(const int sd,
		     const ssize_t wait_msecs = -1);

  int _readSelect(const int sd,
		  const ssize_t wait_msecs = -1);

  int _writeSelect(const int sd,
		   const ssize_t wait_msecs = -1);

  private:

    /////////////////////////////////////////////////
    // Copy constructor private with no body provided.
    //   Do not use.
  
    SockUtil(const SockUtil & orig);
};

#endif
