// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// Input.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2007
//
///////////////////////////////////////////////////////////////

#ifndef Input_HH
#define Input_HH

#include <string>
#include <vector>
#include <cstdio>
#include "Args.hh"

#include <rvp8_rap/TsReader.hh>

using namespace std;

////////////////////////
// This class

class Input {
  
public:

  // constructor

  Input (const Args &args);

  // destructor
  
  ~Input();

  // initialize for reading
  // returns 0 on success, -1 on failure
  
  int initForReading();

  // get time series info

  const TsInfo &getTsInfo() { return _reader.getInfo(); }

  // read next pulse
  // returns pointer to pulse on success, null on failure
  // pulse must be deleted by caller

  TsPulse *readNextPulse();

protected:
  
private:

  const Args &_args;
  TsReader _reader;
  bool _fileInput;
  int _fileNum;

  // open next file for reading
  // returns 0 on success, -1 on failure
  
  int _openNextFile();

};

#endif
