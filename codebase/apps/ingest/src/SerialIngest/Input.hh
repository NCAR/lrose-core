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
/////////////////////////////////////////////////////////////
// Input.hh
//
// Base and derived classes for dealing with different input streams.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2003
//
///////////////////////////////////////////////////////////////

#ifndef Input_HH
#define Input_HH

#include <cstdio>
#include <string>
#include <toolsa/Socket.hh>
#include "Params.hh"
#include "Tty.hh"
using namespace std;

///////////////////////////////
// Input - abstract base class

class Input {
  
public:

  // constructor
  
  Input(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~Input();

  // open
  // returns 0 on success, -1 on failure
  
  virtual int open() = 0;

  // close

  virtual void close() = 0;
  
  // wait for input
  // returns 0 on success, -1 on failure.
  // Sets timedOut to true if timedOut, returns 0.
  // Blocks if wait_msecs == -1
  
  virtual int readSelect(int wait_msecs, bool &timedOut) = 0;

  // get the next character
  // returns 0 on success, -1 on failure

  virtual int readChar(char &cc) = 0;
  
protected:
  
  const string &_progName;
  const Params &_params;

private:

};

////////////////////////////////////////////////////////
// TtyInput
//
// Derived class for TTY input
//

class TtyInput : public Input {
  
public:

  // constructor
  
  TtyInput(const string &prog_name, const Params &params);

  // destructor

  virtual ~TtyInput();

  // open
  // returns 0 on success, -1 on failure
  
  virtual int open();

  // close
  
  virtual void close();
  
  // wait for input
  // returns 0 on success, -1 on failure.
  // Sets timedOut to true if timedOut, returns 0.
  // Blocks if wait_msecs == -1
  
  virtual int readSelect(int wait_msecs, bool &timedOut);

  // get the next character
  // returns 0 on success, -1 on failure
  // Blocks if wait_msecs == -1

  virtual int readChar(char &cc);
  
private:

  Tty _tty;

};

////////////////////////////////////////////////////////
// TcpInput
//
// Derived class for TCP input
//

class TcpInput : public Input {
  
public:

  // constructor
  
  TcpInput(const string &prog_name, const Params &params);

  // destructor

  virtual ~TcpInput();

  // open
  // returns 0 on success, -1 on failure
  
  virtual int open();

  // close
  
  virtual void close();
  
  // wait for input
  // returns 0 on success, -1 on failure.
  // Sets timedOut to true if timedOut, returns 0.
  // Blocks if wait_msecs == -1
  
  virtual int readSelect(int wait_msecs, bool &timedOut);

  // get the next character
  // returns 0 on success, -1 on failure
  // Blocks if wait_msecs == -1

  virtual int readChar(char &cc);
  
private:

  Socket _sock;

};

#endif

