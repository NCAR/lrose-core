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
// Tty.cc
//
// Tty class - reading serial ports
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
/////////////////////////////////////////////////////////////

#include "Tty.hh"

#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <sys/time.h>
#include <toolsa/pmu.h>
using namespace std;

static const int SPEED_STR_LEN = 16;
static const int NUM_SPEEDS = 8;
static char speedStrings[NUM_SPEEDS][SPEED_STR_LEN] =
  {"300\0", "1200\0", "2400\0", "4800\0", "9600\0", 
   "19200\0", "38400\0", "unknown\0"};

//////////////
// constructor

Tty::Tty (const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  _isOK = true;
  _fd = -1;

}

/////////////
// destructor

Tty::~Tty ()

{

  closePort();

}

/////////////////////////////////////////////////////////////////
//
// openPort()
//
// open serial port
//
// set the charateristic for the particular tty to be:
//   
// Input Modes:	IGNBRK (ignore break)
// ICRNL (terminate by <cr>)
// 
// Local Modes:	ICANON (cannonical input processing)
// ~ECHO (no echo)
// ~ISIG (signal characters off)
// ~IEXTEN (extended input processing off)
// 
// Output Modes: ~OPOST (output processing off),
// 
// Control Modes: 7-or-8 bit, stop bits, parity and baud rate set
// 
// returns 0 on success, -1 on failure
//

int Tty::openPort()

{

  _fd = open (_params.input_device, O_RDONLY);
  
  if (_fd < 0) {
    perror (_params.input_device);
    return (-1);
  }
	
  // get the termio characterstics for the tty
  
  if (tcgetattr(_fd, &_termOld) < 0) {
    fprintf(stderr, "Error on tcgetattr()\n");
    perror (_params.input_device);
    return(-1);
  }
  
  // copy terminal characteristics

  _termNew = _termOld;
  
  // set desired characteristics

  _termNew.c_lflag |= ICANON;
  _termNew.c_lflag &= ~(ISIG | ECHO | IEXTEN);
  _termNew.c_iflag |= (IGNBRK | IGNCR);

  _termNew.c_cflag &= ~CSIZE;
  if (_params.dataIs7Bit) {
    _termNew.c_cflag |= CS7;
  } else {
    _termNew.c_cflag |= CS8;
  }

  _termNew.c_cflag &= ~CSTOPB;
  if (_params.twoStopBits) {
    _termNew.c_cflag |= CSTOPB;
  }

  _termNew.c_cflag &= ~PARENB;
  if (_params.enableParity) {
    _termNew.c_cflag |= PARENB;
  }

  _termNew.c_cflag &= ~PARODD;
  if (_params.enableParity && _params.oddParity) {
    _termNew.c_cflag |= PARODD;
  }

  if (tcsetattr(_fd, TCSADRAIN, &_termNew) < 0) {
    fprintf(stderr, "Error on tcsetattr()\n");
    perror (_params.input_device);
    return(-1);
  }
  
  speed_t speed;

  switch (_params.baud_rate) {

  case Params::BAUD_300:
    speed = B300;
    break;

  case Params::BAUD_1200:
    speed = B1200;
    break;

  case Params::BAUD_2400:
    speed = B2400;
    break;

  case Params::BAUD_4800:
    speed = B4800;
    break;

  case Params::BAUD_9600:
    speed = B9600;
    break;

  case Params::BAUD_19200:
    speed = B19200;
    break;

  case Params::BAUD_38400:
    speed = B38400;
    break;

  default:
    speed = B9600;

  }

  cfsetispeed(&_termNew, speed);
  cfsetospeed(&_termNew, speed);
  
  speed_t ispeed = cfgetospeed(&_termNew);
  speed_t ospeed = cfgetispeed(&_termNew);

  if (_params.debug) {
    fprintf(stderr, "output speed %s\n", _speedStr(ospeed));
    fprintf(stderr, "input speed %s\n", _speedStr(ispeed));
  }
  
  return (0);

}

/////////////////////////////////////////////////////////////////
//
// closePort()
//
// close the serial port
//

void Tty::closePort()

{

  if (_fd >= 0) {
    // reset port to earlier state
    if (tcsetattr(_fd, TCSADRAIN, &_termOld) < 0) {
      fprintf(stderr, "Error on tcsetattr()\n");
      perror (_params.input_device);
    }
    // close the serial port
    close(_fd);
  }

}

/////////////////////////////////////////////////////////////////
//
// readChar()
//
// reads a character
//
// returns 0 on success, -1 on failure
//

int Tty::readChar(char &cc)

{

  if (read(_fd, &cc, 1) != 1) {
    return (-1);
  } else {
    return (0);
  }

}

///////////////////////////////////////////////////////
// readSelect - waits for read access on fd
//
// Blocks if wait_msecs == -1
//
// Returns 0 on success, -1 on failure.
// Sets timedOut to true if timed out.

int Tty::readSelect(long wait_msecs, bool &timedOut)

{

  timedOut = false;

  int ret, maxfdp1;
  fd_set read_fd;
  
  struct timeval wait;
  struct timeval * waitp;
  
  waitp = &wait;
  
  // check only on _fd file descriptor

  FD_ZERO(&read_fd);
  FD_SET(_fd, &read_fd);
  maxfdp1 = _fd + 1;
  
 again:

  /*
   * set timeval structure
   */
  
  if (-1 == wait_msecs) {
    waitp = NULL;
  } else {
    wait.tv_sec = wait_msecs / 1000;
    wait_msecs -= wait.tv_sec * 1000;
    wait.tv_usec = wait_msecs * 1000;
  }
  
  errno = 0;
  if (0 > (ret = select(maxfdp1, &read_fd, NULL, NULL, waitp))) {
      if (errno == EINTR) /* system call was interrupted */
	goto again;
      fprintf(stderr,"Read select failed on _fd %d; error = %d\n",
	      _fd, errno);
      return -1; /* select failed */
    } 
  
  if (ret == 0) {
    timedOut = true;
  }
  
  return 0;

}

////////////////////////////
// get string for speed enum

char *Tty::_speedStr(speed_t speed)

{

  switch (speed) {

  case B300:
    return (speedStrings[0]);

  case B1200:
    return (speedStrings[1]);

  case B2400:
    return (speedStrings[2]);

  case B4800:
    return (speedStrings[3]);

  case B9600:
    return (speedStrings[4]);

  case B19200:
    return (speedStrings[5]);

  case B38400:
    return (speedStrings[6]);

  default:
    return (speedStrings[7]);

  }
  
}
