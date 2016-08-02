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
// TsScanInfoMerge.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////
//
// TsScanInfoMerge reads time-series data from 2 file message
// queues, a master and a slave. The master queue has all of
// the meta data set correctly. The slave queue is missing time,
// angle and scan meta data. The program synchronizes the two
// queues based on the pulse sequence number in the pulse headers.
// It then copies the time, angle and scan information from the
// master to the slave. It writes out the updated slave queue,
// and optionally also echoes the master queue to an output queue.
//
////////////////////////////////////////////////////////////////

#ifndef TsScanInfoMerge_H
#define TsScanInfoMerge_H

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include "MasterReader.hh"
#include "SlaveReader.hh"
#include <Fmq/Fmq.hh>

using namespace std;

////////////////////////
// This class

class TsScanInfoMerge {
  
public:

  // constructor

  TsScanInfoMerge(int argc, char **argv);

  // destructor
  
  ~TsScanInfoMerge();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // input queues

  MasterReader *_masterReader;
  SlaveReader *_slaveReader;
  si64 _masterSeqNum, _slaveSeqNum;
  double _slaveTime;
  double _masterTime, _prevMasterTime;
  iwrf_pulse_header_t _latestPulseHdr;
  iwrf_pulse_header_t _prevPulseHdr;

  // output queues
  // we conbine a number of packets into a message before
  // writing to the FMQ

  DsFmq _writer;
  DsMessage _writeMsg;

  // functions
  
  int _syncUsingNumbers();
  int _syncUsingTime();
  int _init();
  int _mergeAndWriteUsingNumbers();
  int _mergeAndWriteUsingTime();
  int _readMaster();
  int _readSlave();
  int _openOutputFmq();
  int _writeToFmq();

  void _interpAngles(double prevEl, double latestEl,
                     double prevAz, double latestAz,
                     double pulseTime,
                     double prevTime, double latestTime,
                     double &interpEl, double &interpAz);

  void _printExtraVerbose(const string &label);

};

#endif
