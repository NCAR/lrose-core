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
// Input.cc
//
// Base and derived classes for dealing with different input streams.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2003
//
///////////////////////////////////////////////////////////////

#include "Input.hh"
#include <toolsa/pmu.h>
using namespace std;

//////////////////////
// Input - base class

// constructor
  
Input::Input(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

}

// destructor
  
Input::~Input() 
{

}

////////////////////////////////////////////////////////////////  
// TtyInput constructor
  
TtyInput::TtyInput(const string &prog_name, const Params &params) :
  Input(prog_name, params),
  _tty(prog_name, params)

{
  
}

// destructor
  
TtyInput::~TtyInput() 
{
  close();
}

////////////////////////////////////////////////////////
// TtyInput::open()
//
// returns 0 on success, -1 on failure

int TtyInput::open()

{
  return _tty.openPort();
}

////////////////////////////////////////////////////////
// TtyInput::close()

void TtyInput::close()

{
  return _tty.closePort();
}

////////////////////////////////////////////////////////
// TtyInput::readSelect()
// wait for input
// returns 0 on success, -1 on failure.
// Sets timedOut to true if timedOut, returns 0.
// Blocks if wait_msecs == -1

int TtyInput::readSelect(int wait_msecs, bool &timedOut)
  
{
  timedOut = false;
  int iret = _tty.readSelect(wait_msecs, timedOut);
  if (iret) {
    return -1;
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////
// TtyInput::readChar(char &cc)
// returns 0 on success, -1 on failure

int TtyInput::readChar(char &cc)
  
{
  return _tty.readChar(cc);
}


////////////////////////////////////////////////////////////////  
// TcpInput constructor
  
TcpInput::TcpInput(const string &prog_name, const Params &params) :
  Input(prog_name, params)

{
  
}

// destructor
  
TcpInput::~TcpInput() 
{
  close();
}

////////////////////////////////////////////////////////
// TcpInput::open()
//
// returns 0 on success, -1 on failure

int TcpInput::open()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Trying to open for TCP, host, port: "
         << _params.tcp_server_host_name << ", "
         << _params.tcp_server_port << endl;
  }

  _sock.close();
  if (_sock.open(_params.tcp_server_host_name,
                 _params.tcp_server_port,
                 10000)) {
    
    cerr << "ERROR - TcpInput::open()" << endl;
    cerr << "  Cannot connect" << endl;
    cerr << "  Host: " << _params.tcp_server_host_name << endl;
    cerr << "  Port: " << _params.tcp_server_port << endl;
    cerr << "  " << _sock.getErrStr() << endl;
    
    return -1;
    
  } // if (_sock.open ...
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Opened for TCP, host, port: "
         << _params.tcp_server_host_name << ", "
         << _params.tcp_server_port << endl;
  }
  
  if (_params.send_tcp_handshake) {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Sending handshake bytes to server" << endl;
      for (int ii = 0; ii < _params.tcp_handshake_bytes_n; ii++) {
        int decimal = _params._tcp_handshake_bytes[ii];
        cerr << "Byte: " << decimal << endl;
      }
    } // verbose
    
    for (int ii = 0; ii < _params.tcp_handshake_bytes_n; ii++) {
      int decimal = _params._tcp_handshake_bytes[ii];
      if (decimal >= 0 && decimal <= 255) {
        unsigned char bb = (unsigned char) decimal;
        if (_sock.writeBuffer(&bb, 1, 5000)) {
          cerr << "ERROR writing handshake byte: " << hex << bb << endl;
          cerr << _sock.getErrStr() << endl;
          _sock.close();
          return -1;
        }
      }
    } // ii

  } // if (_params.send_tcp_handshake)
  
  return 0;

}

////////////////////////////////////////////////////////
// TcpInput::close()

void TcpInput::close()

{
  return _sock.close();
}

////////////////////////////////////////////////////////
// TcpInput::readSelect()
// wait for input
// returns 0 on success, -1 on failure.
// Sets timedOut to true if timedOut, returns 0.
// Blocks if wait_msecs == -1

int TcpInput::readSelect(int wait_msecs, bool &timedOut)
  
{
  timedOut = false;
  int iret = _sock.readSelect(wait_msecs);
  if (iret == 0) {
    return 0;
  } else if (_sock.getErrNum() == Socket::TIMED_OUT) {
    timedOut = true;
    return 0;
  } else {
    return -1;
  }
}

////////////////////////////////////////////////////////
// TcpInput::readChar(char &cc)
// returns 0 on success, -1 on failure

int TcpInput::readChar(char &cc)
  
{
  return _sock.readBuffer(&cc, 1, 10);
}

