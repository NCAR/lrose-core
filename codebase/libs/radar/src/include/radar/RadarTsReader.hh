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
// RadarTsReader
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
//
///////////////////////////////////////////////////////////////
//
// Class has been deprecated.
// Use IwrfTsReader instead.
//
///////////////////////////////////////////////////////////////

#ifndef RadarTsReader_hh
#define RadarTsReader_hh

#include <string>
#include <toolsa/pmu.h>
#include <Fmq/DsFmq.hh>
#include <didss/DsInputPath.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <radar/RadarTsInfo.hh>
#include <radar/RadarTsPulse.hh>
using namespace std;

////////////////////////
// Base class

class RadarTsReader {
  
public:

  // constructor
  
  RadarTsReader(RadarTsDebug_t debug = RTS_DEBUG_OFF);
  
  // destructor
  
  virtual ~RadarTsReader();

  // debugging

  void setDebug(RadarTsDebug_t debug) { _debug = debug; }

  // Get next pulse.
  // If pulse arg is non-NULL, it will be filled out and returned.
  // New pulse object is allocated, if pulse arg is NULL.
  // Caller must handle memory management, freeing pulses
  // allocated by this call.
  // Returns pointer to pulse object.
  // Returns NULL at end of data, or error.
  
  virtual const RadarTsPulse *getNextPulse(RadarTsPulse *pulse = NULL) = 0;

  // reset the file queue - used for sim mode

  virtual void reset();

  // get ops info
  
  const RadarTsInfo &getOpsInfo() { return _opsInfo; }
  bool isOpsInfoNew() { return _opsInfoNew; }

protected:
  
  RadarTsDebug_t _debug;

  // pulse info

  RadarTsInfo _opsInfo;
  bool _opsInfoAvail;
  bool _opsInfoNew;

  // private functions
  
private:

};

/////////////////////////////////////
// Read pulses from time-series files
// Derived class.

class RadarTsReaderFile : public RadarTsReader {
  
public:

  // constructor
  
  // REALTIME mode, read files as they arrive
  // Specify input directory to watch.
  //
  // Blocks on read.
  // Calls heartbeat_func when blocked, if non-null.
  
  RadarTsReaderFile(const char *input_dir,
                    int max_realtime_age_secs = 3600,
                    DsInput_heartbeat_t heartbeat_func = PMU_auto_register,
                    bool use_ldata_info = TRUE,
                    RadarTsDebug_t debug = RTS_DEBUG_OFF);

  // ARCHIVE mode - specify list of files to be read
  
  RadarTsReaderFile(const vector<string> &fileList,
                    RadarTsDebug_t debug = RTS_DEBUG_OFF);
 
  // destructor
  
  virtual ~RadarTsReaderFile();

  // Get next pulse.
  // If pulse arg is non-NULL, it will be filled out and returned.
  // New pulse object is allocated, if pulse arg is NULL.
  // Caller must handle memory management, freeing pulses
  // allocated by this call.
  // Returns pointer to pulse object.
  // Returns NULL at end of data, or error.
  
  virtual const RadarTsPulse *getNextPulse(RadarTsPulse *pulse = NULL);

  // reset the file queue - used for sim mode

  virtual void reset();

protected:
  
private:

  // input data
  
  DsInputPath *_input;
  vector<string> _fileList;
  char *_inputPath;
  FILE *_in;

  // private functions
  
  int _openNextFile();

};

/////////////////////////////////////
// Read pulses from time-series FMQ
// Derived class.

class RadarTsReaderFmq : public RadarTsReader {
  
public:

  // constructor
  
  RadarTsReaderFmq(const char *input_fmq,
                   RadarTsDebug_t debug = RTS_DEBUG_OFF,
                   bool position_fmq_at_start = FALSE);
  
  // destructor
  
  virtual ~RadarTsReaderFmq();

  // Get next pulse.
  // If pulse arg is non-NULL, it will be filled out and returned.
  // New pulse object is allocated, if pulse arg is NULL.
  // Caller must handle memory management, freeing pulses
  // allocated by this call.
  // Returns pointer to pulse object.
  // Returns NULL at end of data, or error.
  
  virtual const RadarTsPulse *getNextPulse(RadarTsPulse *pulse = NULL);
  
  // reset the queue - used for sim mode

  virtual void reset();

protected:
  
private:
  
  // input FMQ
  
  string _inputFmq;
  DsFmq _fmq;
  bool _positionFmqAtStart;
  bool _fmqIsOpen;

  // input message

  DsMessage _msg;
  DsMsgPart *_part;
  int _nParts;
  int _pos;
  
  int _getNextPart();

};

#endif

