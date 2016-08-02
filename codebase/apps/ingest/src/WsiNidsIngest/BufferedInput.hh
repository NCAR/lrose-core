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
// BufferedInput.hh
//
// BufferedInput mechanism 
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#ifndef BufferedInput_HH
#define BufferedInput_HH

#include <string>
#include <toolsa/Socket.hh>
#include "Params.hh"
using namespace std;

//////////////////////////////////////
// BufferedInput - abstract base class

class BufferedInput {
  
public:

  // constructor

  BufferedInput(const int buf_len = 128);

  // destructor
  
  virtual ~BufferedInput();

  // read - returns 0 on success, -1 on failure

  int read (void *obuf, const size_t olen);

  size_t getTotalRead() { return _totalRead; }

protected:

  size_t _totalRead;
  size_t _ilen;
  size_t _ileft;
  unsigned char *_ibuf;
  unsigned char *_iptr;

  // pure virtual fuction - must be overridden in derived classes

  virtual int _fillBuf() = 0;

private:

};

///////////////////////////////
// StdInput - input from stdin

class StdInput : public BufferedInput {
  
public:

  // constructor

  StdInput(const int buf_len = 128);

  // destructor
  
  virtual ~StdInput();

  int _fillBuf();

protected:
  
private:
  
};

////////////////////////////////////////////////////////
// SockInput
//
// Derived class for realtime mode BufferedInput based on files
// in input directory
//

class SockInput : public BufferedInput {
  
public:
  
  // constructor

  SockInput(const char *hostname,
	    const int port,
	    const int buf_len = 128,
	    const int sleep_secs_on_failed_open = 5);

  // destructor
  
  virtual ~SockInput();

  int _fillBuf();

protected:
  
private:
  
  Socket _socket;
  string _hostname;
  int _port;
  int _sleepSecsOnFailedOpen;

  void _open();

};

#endif

