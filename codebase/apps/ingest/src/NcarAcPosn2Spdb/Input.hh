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
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2000
//
///////////////////////////////////////////////////////////////

#ifndef Input_HH
#define Input_HH

#include <cstdio>
#include <string>
#include <vector>
#include <toolsa/Tty.hh>
#include <toolsa/Socket.hh>
#include "Params.hh"
class DsInputPath;
using namespace std;

class AcSim;

////////////////////////////////
// Input - abstract base class

class Input {
  
public:

  // constructor
  
  Input(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~Input();

  // read in input line
  // returns 0 on success, -1 on error (input exhausted)
  
  virtual int readLine(char *line, int len) = 0;

  // get the reference time, if available
  // this is a time which should be close to the data time,
  // to allow the date to be deduced if not available
  // from the line-by-line data
  // For most cases, this is 'now'

  virtual time_t getRefTime();
  
protected:
  
  const string _progName;
  const Params &_params;

private:

};

////////////////////////////////////////////////////////
// SerialInput
//
// Derived class for Tty input
//

class SerialInput : public Input {
  
public:
  
  // constructor
  
  SerialInput(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~SerialInput();

  // read in input line
  // returns 0 on success, -1 on error (input exhausted)
  
  virtual int readLine(char *line, int len);
  
protected:

  Tty _tty;

};

////////////////////////////////////////////////////////
// SimInput
//
// Derived class for Test input
//

class SimInput : public Input {
  
public:

  // constructor
  
  SimInput(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~SimInput();

  // read in input line
  // returns 0 on success, -1 on error (input exhausted)
  
  virtual int readLine(char *line, int len);
  
protected:

  int _acNum;
  vector<AcSim *> _acs;

};

////////////////////////////////////////////////////////
// TcpInput
//
// Derived class for reading serial data from tcp
//

class TcpInput : public Input {
  
public:

  // constructor
  
  TcpInput(const string &prog_name, const Params &params,
           const char *host, int port);
  
  // destructor
  
  virtual ~TcpInput();

  // read in input line
  // returns 0 on success, -1 on error (input exhausted)
  
  virtual int readLine(char *line, int len);
  
protected:

  string _host;
  int _port;
  Socket _sock;
  bool _sockIsOpen;

  int _openSocket();
  void _closeSocket();
  int _readSelect(int wait_msecs, bool &timedOut);
  int _readChar(char &cc);
  
};

////////////////////////////////////////////////////////
// FileInput
//
// Derived class for File input
//

class FileInput : public Input {
  
public:

  // constructor - realtime file mode
  
  FileInput(const string &prog_name, const Params &params);

  // constructor - archive file mode
  
  FileInput(const string &prog_name, const Params &params,
            time_t startTime, time_t endTime);
  
  // constructor - file list mode
  
  FileInput(const string &prog_name, const Params &params,
            const vector<string> &input_file_paths);
  
  // destructor
  
  virtual ~FileInput();
  
  // read in input line
  // returns 0 on success, -1 on error (input exhausted)
  
  virtual int readLine(char *line, int len);
  
  // get the reference time, if possible, from the
  // name of the current file

  virtual time_t getRefTime();
  
protected:

  DsInputPath *_input;
  string _currentPath;
  FILE *_in;

  int _openNext();

};

#endif

