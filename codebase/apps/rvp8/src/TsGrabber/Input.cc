// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:30:34 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// Input.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2007
//
///////////////////////////////////////////////////////////////

#include "Input.hh"
#include <rvp8_rap/uusleep.h>
#include <iostream>
#include <cerrno>

using namespace std;

// Constructor

Input::Input(const Args &args) :
        _args(args)
  
{

  if (_args.verbose) {
    _reader.setDebug(true);
  }
  if (_args.invertHvFlag) {
    _reader.setInvertHvFlag(true);
  }
  if (_args.inputMode == Args::FILE_INPUT_MODE) {
    _fileInput = true;
  } else {
    _fileInput = false;
  }

  return;
  
}

// destructor

Input::~Input()

{
  _reader.cleanUp();
}

//////////////////////////////////////////////////
// initialize for reading
// returns 0 on success, -1 on failure

int Input::initForReading()
{

  if (_fileInput) {

    _fileNum = -1;
    if (_openNextFile()) {
      cerr << "ERROR - Input::initForReading" << endl;
      cerr << "  No files to open" << endl;
      return -1;
    }

  } else {

    // attach to RVP8
    
    if (_reader.connectToTsApi()) {
      cerr << "ERROR - Input::initForReading" << endl;
      return -1;
    }

  }

  return 0;

}

//////////////////////////////////////////////////
// read next pulse
// returns pointer to pulse on success, null on failure
// pulse must be deleted by caller

TsPulse *Input::readNextPulse()

{

  const TsInfo &info = _reader.getInfo();
  TsPulse *pulse = new TsPulse(info.getClockMhz());
  
  // time series input

  if (!_fileInput) {
    int ntries = 0;
    while (_reader.readPulse(*pulse) != 0) {
      umsleep(50);
      ntries++;
      if (ntries > 1000) {
	// timed out
	delete pulse;
	return NULL;
      }
    }
    return pulse;
  }

  // file input

  if (_reader.readPulse(*pulse) == 0) {
    // success on first read
    return pulse;
  }

  // failure on first read
  // open next file, then read

  while (_openNextFile() == 0) {
    if (_reader.readPulse(*pulse) == 0) {
      // success on read, therefore data in file
      return pulse;
    }
  }

  // all files exhausted

  delete pulse;
  return NULL;

}

//////////////////////////////////////////////////
// open next file for reading
// returns 0 on success, -1 on failure

int Input::_openNextFile()
{

  // close file if open

  _reader.closeTsFile();

  // increment file number

  _fileNum++;

  if (_fileNum >= (int) _args.inputFileList.size()) {
    if (_args.debug) {
      cerr << "All files processed" << endl;
      cerr << "  nFiles: " << _args.inputFileList.size() << endl;
    }
    return -1;
  }
  
  int iret = 0;

  const char *filePath = _args.inputFileList[_fileNum].c_str();
  if (_args.debug) {
    cerr << "Opening file: " << filePath << endl;
  }
  
  if (_reader.openTsFile(filePath)) {
    cerr << "ERROR - Input::openNextFile()" << endl;
    cerr << "  Cannot open file: " << filePath << endl;
    return -1;
  }

  return 0;

}

