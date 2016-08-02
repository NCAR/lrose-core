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
// BufferedInput.cc
//
// Buffered input for WSI data stream
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
/////////////////////////////////////////////////////////////

#include "BufferedInput.hh"
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <cstdio>
using namespace std;

//////////////////////////////////////
// BufferedInput - abstract base class

BufferedInput::BufferedInput (const int buf_len /* = 128*/ )

{
  _ilen = buf_len;
  _ibuf = new unsigned char [_ilen];
  _ileft = 0;
  _totalRead = 0;
}

BufferedInput::~BufferedInput ()

{
  delete[] _ibuf;
}

int BufferedInput::read (void *obuf, const size_t olen)

{

  size_t oleft = olen;
  unsigned char *optr = (unsigned char *) obuf;

  while (oleft > 0) {

    size_t ncopy;

    if (_ileft > oleft) {
      ncopy = oleft;
    } else {
      ncopy = _ileft;
    }
      
    memcpy(optr, _iptr, ncopy);
    optr += ncopy;
    _iptr += ncopy;
    oleft -= ncopy;
    _ileft -= ncopy;

    if (_ileft <= 0) {
      if (_fillBuf()) {
	return (-1);
      }
      _iptr = _ibuf;
      _ileft = _ilen;
    }

  } // while

  _totalRead += olen;

  return (0);

#ifdef TEST
  if (fread(obuf, 1, olen, stdin) != olen) {
    return (-1);
  }
  _totalRead += olen;
  return (0);
#endif

}

///////////////////////////////
// StdInput - input from stdin

StdInput::StdInput (const int buf_len /* = 128*/ )
  : BufferedInput(buf_len)

{
  
}

/////////////
// destructor

StdInput::~StdInput()

{
}

int StdInput::_fillBuf ()

{
  
  PMU_auto_register("Filling input buffer from stdin");

  if (fread(_ibuf, 1, _ilen, stdin) != _ilen) {
    return (-1);
  }
  return (0);

}


/////////////////////////////////
// SockInput - input from Socket

SockInput::SockInput (const char *hostname,
		      const int port,
		      const int buf_len /* = 128*/,
		      const int sleep_secs_on_failed_open /* = 5*/) :
  BufferedInput(buf_len)

{

  _hostname = hostname;
  _port = port;
  _sleepSecsOnFailedOpen = sleep_secs_on_failed_open;
  
}

/////////////
// destructor

SockInput::~SockInput()

{
}

int SockInput::_fillBuf ()

{
  
  PMU_auto_register("Filling input buffer from socket");

  while (_socket.readBuffer(_ibuf, _ilen)) {
    _socket.close();
    _open();
  }

  return (0);

}

void SockInput::_open()

{

  while (_socket.open(_hostname.c_str(), _port)) {
    cerr << _socket.getErrString() << endl;
    PMU_auto_register("Trying to open socket");
    uusleep(_sleepSecsOnFailedOpen * 1000000);
  }

}

