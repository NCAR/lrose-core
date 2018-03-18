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
// April 2000
//
/////////////////////////////////////////////////////////////


#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <sys/time.h>
#include <toolsa/Tty.hh>
using namespace std;

static const int SPEED_STR_LEN = 16;
static const int NUM_SPEEDS = 8;
static char speedStrings[NUM_SPEEDS][SPEED_STR_LEN] =
  {"300\0", "1200\0", "2400\0", "4800\0", "9600\0", 
   "19200\0", "38400\0", "unknown\0"};

//////////////
// Constructor

Tty::Tty (const string &prog_name) : _progName(prog_name)

{

  _fd = -1;
  _fp = NULL;
  _debug = false;

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

int Tty::openPort(const char *input_device,
		  speed_t baud_rate /* = B9600*/,
		  bool twoStopBits /* = false*/,
		  bool enableParity /* = false*/,
		  bool oddParity /* = false*/,
		  bool dataIs7Bit /* = false*/ )

{

  _inputDevice = input_device;

  _fd = open (input_device, O_RDONLY);
  
  if (_fd < 0) {
    perror (input_device);
    return (-1);
  }

  if (_debug) {
    cerr << "Tty::openPort" << endl;
    cerr << "  prog_name: " << _progName << endl;
    cerr << "  device name: " << input_device << endl;
    cerr << "  baud rate: " << _speedStr(baud_rate) << endl;
    if (twoStopBits) {
      cerr << "  two stop bits" << endl;
    } else {
      cerr << "  one stop bit" << endl;
    }
    if (enableParity) {
      cerr << "  parity enabled" << endl;
    } else {
      cerr << "  parity disabled" << endl;
    }
    if (enableParity) {
      if (oddParity) {
	cerr << "  odd parity" << endl;
      } else {
	cerr << "  even parity" << endl;
      }
    }
    if (dataIs7Bit) {
      cerr << "  data is 7 bit" << endl;
    } else {
      cerr << "  data is 8 bit" << endl;
    }
  }
	
  // get the termio characterstics for the tty
  
  if (tcgetattr(_fd, &_termOld) < 0) {
    fprintf(stderr, "Error on tcgetattr()\n");
    perror (input_device);
    return(-1);
  }
  
  // copy terminal characteristics

  _termNew = _termOld;
  
  // set desired characteristics

  _termNew.c_lflag |= ICANON;
  _termNew.c_lflag &= ~(ISIG | ECHO | IEXTEN);
  _termNew.c_iflag |= (IGNBRK | IGNCR);

  _termNew.c_cflag &= ~CSIZE;
  if (dataIs7Bit) {
    _termNew.c_cflag |= CS7;
  } else {
    _termNew.c_cflag |= CS8;
  }

  _termNew.c_cflag &= ~CSTOPB;
  if (twoStopBits) {
    _termNew.c_cflag |= CSTOPB;
  }

  _termNew.c_cflag &= ~PARENB;
  if (enableParity) {
    _termNew.c_cflag |= PARENB;
  }

  _termNew.c_cflag &= ~PARODD;
  if (enableParity && oddParity) {
    _termNew.c_cflag |= PARODD;
  }

  if (tcsetattr(_fd, TCSADRAIN, &_termNew) < 0) {
    fprintf(stderr, "Error on tcsetattr()\n");
    perror (input_device);
    return(-1);
  }
  
  cfsetispeed(&_termNew, baud_rate);
  cfsetospeed(&_termNew, baud_rate);
  
  speed_t ispeed = cfgetospeed(&_termNew);
  speed_t ospeed = cfgetispeed(&_termNew);

  if (_debug) {
    fprintf(stderr, "output speed %s\n", _speedStr(ospeed));
    fprintf(stderr, "input speed %s\n", _speedStr(ispeed));
  }

  // open file destriptor

  _fp = (FILE *) fdopen(_fd, "r");
  
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
      perror (_inputDevice.c_str());
    }
    // close the serial port
    close(_fd);
    _fd = -1;
  }

  if (_fp) {
    fclose(_fp);
    _fp = NULL;
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

/////////////////////////////////////////////////////////////////
//
// readLine()
//
// reads a line
//
// returns 0 on success, -1 on failure
//

int Tty::readLine(char *line, int max_len)

{

  if (fgets(line, max_len, _fp) == NULL) {
    return (-1);
  } else {
    return (0);
  }

}

///////////////////////////////////////////////////////
// readSelect - waits for read access on fd
//
// returns 1 on success, -1 on timeout, -2 on failure
//
// Blocks if wait_msecs == -1

int Tty::readSelect(long wait_msecs)

{
  
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
      return -2; /* select failed */
    } 
  
  if (ret == 0) {
    return (-1); /* timeout */
  }
  
  return (1);

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
