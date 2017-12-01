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
// IwrfTsReader
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
//
///////////////////////////////////////////////////////////////

#ifndef IwrfTsReader_hh
#define IwrfTsReader_hh

#include <string>
#include <toolsa/pmu.h>
#include <Fmq/DsFmq.hh>
#include <toolsa/Socket.hh>
#include <didss/DsInputPath.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsBurst.hh>
using namespace std;

////////////////////////
// Base class

class IwrfTsReader {
  
public:

  // constructor
  
  IwrfTsReader(IwrfDebug_t debug = IWRF_DEBUG_OFF);
  
  // destructor
  
  virtual ~IwrfTsReader();

  // debugging

  void setDebug(IwrfDebug_t debug) { _debug = debug; }

  // Specify the radar id.
  // There is a radar_id associated with each iwrf packet.
  // Normally this is set to 0.
  // If you specify a non-zero radar id, then only packets with this
  // id, plus any packets with a 0 id, will be read. All non-zero
  // packets that do not match this id will be discarded on read.

  void setRadarId(int id) { _radarId = id; }

  // set for non-blocking reads
  // specify the number of millisecs to wait for data before returning
  // If this is set, use getTimedOut() if getNextPulse() returns an
  // error to determine if the read timed out.
  
  void setNonBlocking(int msecsWait) {
    _nonBlocking = true;
    _msecsWait = msecsWait;
  }

  // set RVP8 legacy unpacking
  // uses 11-bit mantissa instead of the later 12-bit mantissa
  
  void setSigmetLegacyUnpacking(bool state) {
    _sigmetLegacyUnpacking = state;
  }

  // set to cohere the IQ data to the burst phase
  // this is used for magnetron systems that have
  // a random phase and for which correction to the burst
  // phase has not yet been handle

  void setCohereIqToBurst(bool state) {
    _cohereIqToBurst = state;
  }

  // set to copy pulse width from ts_proc in info

  void setCopyPulseWidthFromTsProc(bool val)
  { _copyPulseWidthFromTsProc = val; }

  // set the acceptable time difference between a georef
  // measurement and a pulse for it to be considered
  // suitable for use with a pulse

  void setGeorefTimeMarginSecs(double val) { _georefTimeMarginSecs = val; }
  
  // set/get the georef unit number to use on read
  // defaults to 0

  void setGeorefUseSecondary(bool val) { _georefUseSecondary = val; }
  bool getGeorefUseSecondary() const { return _georefUseSecondary; }

  // Get next pulse.
  // Converts IQ data to floats if requested.
  // If pulse arg is non-NULL, it will be filled out and returned.
  // New pulse object is allocated, if pulse arg is NULL.
  // Caller must handle memory management, freeing pulses
  // allocated by this call.
  // Returns pointer to pulse object.
  // Returns NULL at end of data, or error.
  
  virtual IwrfTsPulse *getNextPulse(bool convertToFloats = false,
                                    IwrfTsPulse *pulse = NULL) = 0;

  // in non-blocking mode, check if read timed out

  bool getTimedOut() const { return _timedOut; }

  // reset the queue to the beginning

  virtual void reset();

  // reset the queue at the end
  
  virtual void seekToEnd() = 0;
  
  // get ops info
  
  const IwrfTsInfo &getOpsInfo() const { return _opsInfo; }
  IwrfTsInfo &getOpsInfo() { return _opsInfo; }

  // get burst
  
  const IwrfTsBurst &getBurst() const { return _burst; }
  IwrfTsBurst &getBurst() { return _burst; }
  
  // check to see if the ops info has changed since the previous pulse
  
  bool isOpsInfoNew() const;

  // Check to see if the burst info has changed since the previous pulse
  
  bool isBurstNew() const;

  // Does the burst match the latest pulse?
  
  bool isBurstForLatestPulse() const;

  // Does the burst match the previous pulse?
  
  bool isBurstForPreviousPulse() const;

  // get the current file path in use

  virtual const string getPathInUse() const = 0;
  
  // get the previous file path in use
  // i.e. the one just before the end of file

  virtual const string getPrevPathInUse() const = 0;
  
  // Check to see if end of file found
  
  bool endOfFile() const { return _endOfFile; }

protected:
  
  IwrfDebug_t _debug;
  IwrfTsInfo _opsInfo;
  IwrfTsBurst _burst;
  
  // read controls

  int _radarId; // set to non-zero to specify radar id
  bool _nonBlocking; // reads are non-blocking (FMQ and TCP only)
  int _msecsWait; // millisecs to wait in non-blocking mode
  bool _cohereIqToBurst; // cohere IQ data to burst
  bool _sigmetLegacyUnpacking;
  bool _copyPulseWidthFromTsProc; /* copy pulse width metadata
                                   * from info to pulse header */

  // status etc

  si64 _pulseSeqNumLatestPulse;
  si64 _pktSeqNumLatestPulse;
  si64 _pktSeqNumPrevPulse;

  // acceptable time difference between a georef measurement
  // and a pulse for it to be considered suitable for use
  // with a pulse

  double _georefTimeMarginSecs; 

  // use georef secondary unit?

  int _georefUseSecondary;

  // end of data conditions

  bool _timedOut; // applies to non-blocking ops only
  bool _endOfFile; /* applies for file-based classes only
                    * other classes will always have this
                    * set to false */
  
  void _setEventFlags(IwrfTsPulse &pulse);
  void _setPlatformGeoref(IwrfTsPulse &pulse);
  void _updatePulse(IwrfTsPulse &pulse);

private:
  
};

/////////////////////////////////////
// Read pulses from time-series files
// Derived class.

class IwrfTsReaderFile : public IwrfTsReader {
  
public:

  // constructor
  
  // REALTIME mode, read files as they arrive
  // Specify input directory to watch.
  //
  // Blocks on read.
  // Calls heartbeat_func when blocked, if non-null.
  
  IwrfTsReaderFile(const char *input_dir,
                    int max_realtime_age_secs = 3600,
                    DsInput_heartbeat_t heartbeat_func = PMU_auto_register,
                    bool use_ldata_info = TRUE,
                    IwrfDebug_t debug = IWRF_DEBUG_OFF);

  // ARCHIVE mode - specify list of files to be read
  
  IwrfTsReaderFile(const vector<string> &fileList,
                    IwrfDebug_t debug = IWRF_DEBUG_OFF);
 
  // destructor
  
  virtual ~IwrfTsReaderFile();

  // Get next pulse.
  // Converts IQ data to floats if requested.
  // If pulse arg is non-NULL, it will be filled out and returned.
  // New pulse object is allocated, if pulse arg is NULL.
  // Caller must handle memory management, freeing pulses
  // allocated by this call.
  // Returns pointer to pulse object.
  // Returns NULL at end of data, or error.
  
  virtual IwrfTsPulse *getNextPulse(bool convertToFloats = false,
                                    IwrfTsPulse *pulse = NULL);

  // reset the file queue - used for sim mode

  virtual void reset();

  // reset the queue at the end
  
  virtual void seekToEnd();

  // get the current file path in use

  virtual const string getPathInUse() const { return _inputPath; }
  
  // get the previous file path in use
  // i.e. the one that has just closed on end-of-file

  virtual const string getPrevPathInUse() const { return _prevInputPath; }
  
protected:
  
private:

  // input data
  
  DsInputPath *_input;
  vector<string> _fileList;
  string _inputPath;
  string _prevInputPath;
  FILE *_in;
  bool _fileIsRvp8Type;
  MemBuf _pktBuf; // buffer for reading packets

  // private functions
  
  int _openNextFile();
  bool _isRvp8File();
  int _readPulseIwrf(IwrfTsPulse &pulse);
  int _readPulseRvp8(IwrfTsPulse &pulse);
  int _resync();

};

/////////////////////////////////////
// Read pulses from time-series FMQ
// Derived class.

class IwrfTsReaderFmq : public IwrfTsReader {
  
public:

  // constructor
  
  IwrfTsReaderFmq(const char *input_fmq,
                   IwrfDebug_t debug = IWRF_DEBUG_OFF,
                   bool position_fmq_at_start = FALSE);
  
  // destructor
  
  virtual ~IwrfTsReaderFmq();

  // Get next pulse.
  // Converts IQ data to floats if requested.
  // If pulse arg is non-NULL, it will be filled out and returned.
  // New pulse object is allocated, if pulse arg is NULL.
  // Caller must handle memory management, freeing pulses
  // allocated by this call.
  // Returns pointer to pulse object.
  // Returns NULL at end of data, or error.
  
  virtual IwrfTsPulse *getNextPulse(bool convertToFloats = false,
                                    IwrfTsPulse *pulse = NULL);

  // reset the queue to the beginning

  virtual void reset();

  // reset the queue at the end
  
  virtual void seekToEnd();

  // get the FMQ path in use

  virtual const string getPathInUse() const { return _inputFmq; }
  virtual const string getPrevPathInUse() const { return _inputFmq; }
  
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
  void _handleReadError();

};

///////////////////////////////////////////
// Read pulses from time-series TCP server
// Derived class.

class IwrfTsReaderTcp : public IwrfTsReader {
  
public:

  // constructor
  
  IwrfTsReaderTcp(const char *server_host,
                  int server_port,
                  IwrfDebug_t debug = IWRF_DEBUG_OFF);
  
  // destructor
  
  virtual ~IwrfTsReaderTcp();

  // Get next pulse.
  // Converts IQ data to floats if requested.
  // If pulse arg is non-NULL, it will be filled out and returned.
  // New pulse object is allocated, if pulse arg is NULL.
  // Caller must handle memory management, freeing pulses
  // allocated by this call.
  // Returns pointer to pulse object.
  // Returns NULL at end of data, or error.
  
  virtual IwrfTsPulse *getNextPulse(bool convertToFloats = false,
                                    IwrfTsPulse *pulse = NULL);

  // reset the queue to the beginning
  // this is a no-op for TCP, cannot reset

  virtual void reset();

  // reset the queue at the end
  // this is a no-op for TCP
  
  virtual void seekToEnd();
  
  // get the TCP path in use
  
  virtual const string getPathInUse() const { return _serverDetails; }
  virtual const string getPrevPathInUse() const { return _serverDetails; }
  
protected:
  
private:
  
  // input TCP

  string _serverHost;
  int _serverPort;
  string _serverDetails;
  Socket _sock;

  // methods

  int _openSocket();
  int _readTcpPacket(int &id, int &len, MemBuf &buf);
  int _reSync();
  int _peekAtBuffer(void *buf, int nbytes);

};

#endif

