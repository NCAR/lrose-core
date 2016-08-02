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
// Tty.hh
//
// Tty class - reading serial ports
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#ifndef Tty_HH
#define Tty_HH

#include <termios.h>
#include <string>
#include "Params.hh"
using namespace std;

class Tty {
  
public:

  // constructor
  Tty(const string &prog_name, const Params &params);

  // destructor
  virtual ~Tty();

  // check for valid object
  bool isOK() { return (_isOK); }

  // open serial port
  // returns 0 on success, -1 on failure
  int openPort();

  // close serial port
  void closePort();

  // reads a character
  // returns 0 on success, -1 on failure
  int readChar(char &cc);

  // readSelect - waits for read access on fd
  // Blocks if wait_msecs == -1
  // Returns 0 on success, -1 on failure.
  // Sets timedOut to true if timed out.
  
  int readSelect(long wait_msecs, bool &timedOut);

protected:

private:

  bool _isOK;
  const string &_progName;
  const Params &_params;

  int _fd;
  struct termios _termOld;
  struct termios _termNew;

  char *_speedStr(speed_t speed);

};

#endif

