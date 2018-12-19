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
// IwrfMomReader.hh
//
// IwrfMomReader object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
///////////////////////////////////////////////////////////////
//
// IwrfMomReader reads moments in iwrf format and the older
// DsRadar format.
//
////////////////////////////////////////////////////////////////

#ifndef IwrfMomReader_hh
#define IwrfMomReader_hh

#include <string>
#include <toolsa/pmu.h>
#include <Fmq/DsFmq.hh>
#include <toolsa/Socket.hh>
#include <didss/DsInputPath.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsRadarParams.hh>
#include <radar/IwrfTsInfo.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxEvent.hh>
#include <Radx/RadxPlatform.hh>
#include <Radx/RadxRcalib.hh>
using namespace std;

////////////////////////
// Base class

class IwrfMomReader {
  
public:
  
  // constructor
  
  IwrfMomReader();
  
  // destructor
  
  virtual ~IwrfMomReader();
  
  // debugging
  
  virtual void setDebug(IwrfDebug_t debug) { _debug = debug; }
  
  // set for non-blocking reads
  // specify the number of millisecs to wait for data before returning
  // If this is set, use getTimedOut() if readNext() returns an
  // error to determine if the read timed out.

  void setNonBlocking(int msecsWait) {
    _nonBlocking = true;
    _msecsNonblockingWait = msecsWait;
  }

  // optionally set timeout for blocking reads
  // will return with error after this timeout
  
  void setBlockingTimeout(int msecsTimeout) {
    _nonBlocking = false;
    _msecsBlockingTimeout = msecsTimeout;
  }

  // Read next ray.
  // Returns RadxRay pointer on success, NULL on failure.
  //
  // This method is used by the FMQ and TCP subclasses.
  // It calls the virtual _getNextMsg() method, which will use the
  // appropriate device.
  //
  // Caller is responsible for freeing the ray.
  //
  // The rays are passed back with a lag of 1, so that the
  // event flags can be correctly handled.
  //
  // Call the following for supplementary info on the ray:
  //   getPlatform()
  //   getScan()
  //   getRCalibs()
  //   getEvents()
  //   getStatusXml()
  
  virtual RadxRay *readNextRay();
  
  // check if platform was updated during last call to readNextRay()

  bool getPlatformUpdated() const { return _flags.platformUpdated; }
  
  // get the platform
  
  const RadxPlatform &getPlatform() const { return _platform; }
  
  // check if radar calibration was updated during last call to readNextRay()

  bool getRcalibUpdated() const { return _flags.rcalibUpdated; }
  
  // get the radar calibrations
  // use pulse width to find the relevant calib
  
  const vector<RadxRcalib> &getRcalibs() const { return _rcalibs; }
  
  // check if status xml was updated during last call to readNextRay()

  bool getStatusXmlUpdated() const { return _flags.statusXmlUpdated; }
  
  // get the status string (XML)

  const string &getStatusXml() const { return _statusXml; }
  
  // get the events prior to ray returned by readNextRay()
  
  const vector<RadxEvent> &getEvents() const { return _events; }
  
  // in non-blocking mode, check if read timed out
  
  bool getTimedOut() const { return _timedOut; }

  // reset the queue to the beginning

  virtual void seekToStart();

  // reset the queue at the end
  
  virtual void seekToEnd();

  // get the current file device in use, if applicable

  virtual const string getDeviceInUse() const = 0;
  
  // Check to see if end of file found
  
  bool endOfFile() const { return _endOfFile; }

protected:
  
  IwrfDebug_t _debug;
  IwrfTsInfo _opsInfo;

  // incoming message

  MemBuf _msgBuf;
  int _msgId;

  // reading DsRadar format

  DsRadarMsg _dsRadarMsg;
  int _dsContents;
  
  // Radx objects

  RadxRay *_latestRay; // latest ray read
  RadxRay *_savedRay;  // save 1 ray, so we can dermine the event flags
                       // by comparing with next ray read in
  bool _rayReady;
  
  vector<RadxEvent> _events; // events for ray passed to user
  vector<RadxEvent> _savedEvents; // events for saved ray
  vector<RadxEvent> _latestEvents; // events for latest ray read
  
  RadxPlatform _platform;
  vector<RadxRcalib> _rcalibs; // radar calibrations
  string _statusXml; // general status in XML format

  // flags for keeping track of when items are updated

  class Flags {
  public:
    bool platformUpdated;
    bool statusXmlUpdated;
    bool rcalibUpdated;
    Flags() {
      clear();
    }
    void clear() {
      platformUpdated = false;
      statusXmlUpdated = false;
      rcalibUpdated = false;
    }
  };

  Flags _flags;
  Flags _latestFlags;
  Flags _savedFlags;
  
  // device info

  bool _nonBlocking;         // reads are non-blocking (FMQ and TCP only)
  int _msecsNonblockingWait; // millisecs to wait in non-blocking reads
  int _msecsBlockingTimeout; // timeout millisecs for blocking reads
  
  bool _timedOut;    // applies to non-blocking ops only
  bool _endOfFile;   /* applies for file-based classes only
                      * other classes will always have this
                      * set to false */

  // methods

  int _handleDsRadarMessage();
  void _decodeDsRadarParams();
  void _decodeDsStatusXml();
  void _decodeDsRadarCalib();
  void _decodeDsRadarFlags();
  void _decodeDsRadarBeam();
  void _setEventFlags();
  void _setRcalibIndex();

  int _handleIwrfMessage();

  virtual int _getNextMsg();

private:
  
};

/////////////////////////////////////
// Read pulses from time-series files
// Derived class.

class IwrfMomReaderFile : public IwrfMomReader {
  
public:

  // constructor
  
  // REALTIME mode, read files as they arrive
  // Specify input directory to watch.
  //
  // Blocks on read.
  // Calls heartbeat_func when blocked, if non-null.
  
  IwrfMomReaderFile(const char *input_dir,
                    int max_realtime_age_secs = 3600,
                    DsInput_heartbeat_t heartbeat_func = PMU_auto_register,
                    bool use_ldata_info = TRUE);
  
  // ARCHIVE mode - specify list of files to be read
  
  IwrfMomReaderFile(const vector<string> &fileList);
 
  // destructor
  
  virtual ~IwrfMomReaderFile();

  // debugging
  
  virtual void setDebug(IwrfDebug_t debug);

  // Read next ray
  // Returns RadxRay pointer on success, NULL on failure.
  //
  // Caller is responsible for freeing the ray.
  //
  // Call the following for supplementary info on the ray:
  //   getPlatform()
  //   getScan()
  //   getRCalibs()
  //   getEvents()
  //   getStatusXml()
  
  virtual RadxRay *readNextRay();
  
  // reset the file queue - used for sim mode

  virtual void seekToStart();

  // get the current file path in use

  virtual const string getDeviceInUse() const { return _inputPath; }
  
protected:
  
private:

  // input data
  
  DsInputPath *_input;
  vector<string> _fileList;
  string _inputPath;
  RadxVol _vol;
  size_t _rayIndex;

  // private functions
  
  int _readNextFile();

};

/////////////////////////////////////
// Read pulses from time-series FMQ
// Derived class.

class IwrfMomReaderFmq : public IwrfMomReader {
  
public:

  // constructor
  
  IwrfMomReaderFmq(const char *input_fmq,
                   bool position_at_start = false);
  
  // destructor
  
  virtual ~IwrfMomReaderFmq();
  
  // reset the queue to the beginning
  // FMQ must be open
  
  virtual void seekToStart();

  // reset the queue at the end
  // FMQ must be open
  
  virtual void seekToEnd();

  // get the FMQ path in use

  virtual const string getDeviceInUse() const { return _inputFmq; }
  
protected:
  
private:
  
  // input FMQ
  
  string _inputFmq;
  DsFmq _fmq;
  bool _positionFmqAtStart;
  bool _fmqIsOpen;

  // input message

  DsMessage _msg;
  int _nParts;
  int _pos;

  RadxRay _rayLocal; // _ray points to this
  
  virtual int _getNextMsg();

};

///////////////////////////////////////////
// Read pulses from time-series TCP server
// Derived class.

class IwrfMomReaderTcp : public IwrfMomReader {
  
public:

  // constructor
  
  IwrfMomReaderTcp(const char *server_host,
                   int server_port);
  
  // destructor
  
  virtual ~IwrfMomReaderTcp();
  
  // get the TCP details
  
  virtual const string getDeviceInUse() const { return _serverDetails; }
  
protected:
  
private:
  
  // input TCP

  string _serverHost;
  int _serverPort;
  string _serverDetails;
  Socket _sock;

  RadxRay _rayLocal; // _ray points to this
  
  // methods

  int _openSocket();
  virtual int _getNextMsg();
  int _reSync();
  int _peekAtBuffer(void *buf, int nbytes);

};

#endif

