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
///////////////////////////////////////////////////////////////
// RadarTsReader.cc
//
// RadarTsReader object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
//
///////////////////////////////////////////////////////////////
//
// RadarTsReader reads radar time series data
//
////////////////////////////////////////////////////////////////
//
// Class has been deprecated.
// Use IwrfTsReader instead.
//
///////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <radar/RadarTsReader.hh>
using namespace std;

////////////////////////////////////////////////////
// Base class

RadarTsReader::RadarTsReader(RadarTsDebug_t debug) :
        _debug(debug)
  
{
  
  _opsInfoAvail = false;
  _opsInfoNew = false;

}

//////////////////////////////////////////////////////////////////
// destructor

RadarTsReader::~RadarTsReader()

{

}

//////////////////////////////////////////////////////////////////
// reset - used for sim mode

void RadarTsReader::reset()

{
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from file
// Derived class

// REALTIME mode, read files as they arrive
// Specify input directory to watch.
//
// Blocks on read.
// Calls heartbeat_func when blocked, if non-null.

RadarTsReaderFile::RadarTsReaderFile(const char *input_dir,
                                     int max_realtime_age_secs,
                                     DsInput_heartbeat_t heartbeat_func,
                                     bool use_ldata_info,
                                     RadarTsDebug_t debug) :
        RadarTsReader(debug)

{
  
  _input = new DsInputPath("",
                           _debug >= RTS_DEBUG_VERBOSE,
                           input_dir,
                           max_realtime_age_secs,
                           heartbeat_func,
                           use_ldata_info);

  _inputPath = NULL;
  _in = NULL;
  
}

// ARCHIVE mode - specify list of files to be read

RadarTsReaderFile::RadarTsReaderFile(const vector<string> &fileList,
                                     RadarTsDebug_t debug) :
        RadarTsReader(debug),
        _fileList(fileList)
  
{
  
  _input = new DsInputPath("", debug, _fileList);
  _inputPath = NULL;
  _in = NULL;
  
}

//////////////////////////////////////////////////////////////////
// destructor

RadarTsReaderFile::~RadarTsReaderFile()

{

  if (_input) {
    delete _input;
  }

  if (_in) {
    fclose(_in);
    _in = NULL;
  }

}

///////////////////////////////////////////
// Get next pulse.
// If pulse arg is non-NULL, it will be filled out and returned.
// New pulse object is allocated, if pulse arg is NULL.
// Caller must handle memory management, freeing pulses
// allocated by this call.
// Returns pointer to pulse object.
// Returns NULL at end of data, or error.

const RadarTsPulse*
  RadarTsReaderFile::getNextPulse(RadarTsPulse *pulse /* = NULL*/)
  
{
  
  if (_in == NULL || feof(_in)) {
    if (_openNextFile()) {
      return NULL;
    }
  } else {
    _opsInfoNew = false; // file already open
  }
  
  // Create a new pulse object
  
  if (pulse == NULL) {
    pulse = new RadarTsPulse(_opsInfo, _debug);
  }
  
  // read in pulse headers and data, opening new files as needed
  
  while (_in != NULL) {
    
    if (pulse->readFromRvp8File(_in) == 0) {
      // success
      return pulse;
    }
    
    // failure with this file

    if (_debug && !feof(_in)) {
      cerr << "ERROR - RadarTsReader::_processFile" << endl;
      cerr << "  Cannot read in pulse headers and data" << endl;
      cerr << "  File: " << _inputPath << endl;
    }

    // try new file
    if (_openNextFile()) {
      // no good
      delete pulse;
      return NULL;
    }

  } // while

  // should not get here

  delete pulse;
  return NULL;

}

////////////////////////////
// open next available file
//
// Returns 0 on success, -1 on failure

int RadarTsReaderFile::_openNextFile()

{

  PMU_auto_register("Opening next file");

  if (_in) {
    fclose(_in);
    _in = NULL;
  }
  
  _inputPath = _input->next();
  if (_inputPath == NULL) {
    // no more files
    return -1;
  }

  if (_debug) {
    cerr << "Opening file: " << _inputPath << endl;
  }

  // open file
  
  if ((_in = fopen(_inputPath, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RadarTsReaderFile::_openNextFile" << endl;
    cerr << "  Cannot open file: " << _inputPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // read in ops info
  
  if (_opsInfo.readFromRvp8File(_in)) {
    cerr << "ERROR - RadarTsReaderFile::_openNextFile" << endl;
    cerr << "  Cannot read pulse info" << endl;
    cerr << "  File: " << _inputPath << endl;
    fclose(_in);
    _in = NULL;
    return -1;
  }
  _opsInfoAvail = true;
  _opsInfoNew = true;
  
  if (_debug >= RTS_DEBUG_VERBOSE) {
    _opsInfo.print(cerr);
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////////
// reset - used for sim mode

void RadarTsReaderFile::reset()

{
  if (_input) {
    _input->reset();
  }
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
// Read pulses from file
// Derived class

RadarTsReaderFmq::RadarTsReaderFmq(const char *input_fmq,
                                   RadarTsDebug_t debug,
                                   bool position_fmq_at_start) :
        RadarTsReader(debug),
        _inputFmq(input_fmq),
        _positionFmqAtStart(position_fmq_at_start)
  
{
  _nParts = 0;
  _pos = 0;
  _fmqIsOpen = false;
}

//////////////////////////////////////////////////////////////////
// destructor

RadarTsReaderFmq::~RadarTsReaderFmq()

{

}

///////////////////////////////////////////
// Get next pulse.
// If pulse arg is non-NULL, it will be filled out and returned.
// New pulse object is allocated, if pulse arg is NULL.
// Caller must handle memory management, freeing pulses
// allocated by this call.
// Returns pointer to pulse object.
// Returns NULL at end of data, or error.

const RadarTsPulse*
  RadarTsReaderFmq::getNextPulse(RadarTsPulse *pulse /* = NULL*/)
  
{

  while (!_opsInfoAvail) {
    
    // get next message part
    
    if (_getNextPart()) {
      return NULL;
    }

    // if this is an info part, load up info
    
    if (_part->getType() == TS_INFO_ID) {
      if (_opsInfo.setFromTsBuffer(_part->getBuf(), _part->getLength()) == 0) {
        _opsInfoAvail = true;
        _opsInfoNew = true;
      }
    }
    
  } // while
  
  // Create a new pulse object

  if (pulse == NULL) {
    pulse = new RadarTsPulse(_opsInfo, _debug);
  }
  
  // read in pulse
  
  while (true) {
    
    // get next message part
    
    if (_getNextPart()) {
      delete pulse;
      return NULL;
    }
    
    // if this is an info part, load up info
    
    if (_part->getType() == TS_INFO_ID) {
      
      if (_opsInfo.setFromTsBuffer(_part->getBuf(), _part->getLength()) == 0) {
        _opsInfoAvail = true;
        _opsInfoNew = true;
        // _opsInfo.print(cerr);
      }

    } else if (_part->getType() == TS_PULSE_ID) {
      
      if (pulse->setFromTsBuffer(_part->getBuf(), _part->getLength()) == 0) {
        _opsInfoNew = false;
        // pulse->print(cerr);
	_opsInfo.setRvp8Info(_opsInfo.getInfo(), pulse->getHdr());
	pulse->setRvp8Hdr(pulse->getHdr());
	return pulse;
      }
      
    }
    
  } // while

  delete pulse;
  return NULL;

}

////////////////////////////
// get next message part
//
// Returns 0 on success, -1 on failure

int RadarTsReaderFmq::_getNextPart()
  
{

  PMU_auto_register("Get next part");
  
  // check we have an open FMQ
  
  if (!_fmqIsOpen) {

    // initialize FMQ

    Fmq::openPosition initPos = Fmq::END;
    if (_positionFmqAtStart) {
      initPos = Fmq::START;
    }
    _fmq.setHeartbeat(PMU_auto_register);
    
    if (_fmq.initReadBlocking(_inputFmq.c_str(),
			      "RadarTsReader",
			      _debug >= RTS_DEBUG_VERBOSE,
			      initPos)) {
      cerr << "ERROR - RadarTsReaderFmq::_getNextPart" << endl;
      cerr << "  Cannot init FMQ for reading" << endl;
      cerr << "  Fmq: " << _inputFmq << endl;
      cerr << _fmq.getErrStr() << endl;
      return -1;
    }

    _fmqIsOpen = true;

  } // if

  while (_pos >= _nParts) {
    
    // we need a new message
    // blocking read registers with Procmap while waiting
    
    if (_fmq.readMsgBlocking()) {
      cerr << "ERROR - RadarTsReaderFmq::_getNextPart" << endl;
      cerr << "  Cannot read message from FMQ" << endl;
      cerr << "  Fmq: " << _inputFmq << endl;
      cerr << _fmq.getErrStr() << endl;
      _fmq.closeMsgQueue();
      _fmqIsOpen = false;
      return -1;
    }

    // disassemble the message

    const void *msg = _fmq.getMsg();
    int len = _fmq.getMsgLen();
  if (_msg.disassemble(msg, len) == 0) {
      _pos = 0;
      _nParts = _msg.getNParts();
    }
    
  } // while

  _part = _msg.getPart(_pos);
  _pos++;

  return 0;

}

//////////////////////////////////////////////////////////////////
// reset - used for sim mode

void RadarTsReaderFmq::reset()

{
  _fmq.seek(Fmq::FMQ_SEEK_START);
}

