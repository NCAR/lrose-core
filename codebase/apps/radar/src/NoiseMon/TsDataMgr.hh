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
// TsDataMgr.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////
//
// Data manager for time series data
//
///////////////////////////////////////////////////////////////

#ifndef TsDataMgr_H
#define TsDataMgr_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "StatsMgr.hh"
#include <radar/IwrfTsReader.hh>
#include <radar/RadarComplex.hh>
#include <radar/RadarMoments.hh>
#include <radar/GateData.hh>

using namespace std;

////////////////////////
// This class

class TsDataMgr : public StatsMgr {
  
public:

  // constructor

  TsDataMgr(const string &prog_name,
            const Args &args,
	    const Params &params);

  // destructor
  
  virtual ~TsDataMgr();

  // run 

  virtual int run();

protected:
  
private:

  IwrfTsReader *_reader;

  // pulse queue
  
  deque<const IwrfTsPulse *> _pulseQueue;
  RadxTime _pulseTime;
  int _maxPulseQueueSize;
  long _pulseSeqNum;
  int _totalPulseCount;
  int _nSamples;
  int _nSamplesHalf;
  int _nGates;
  int _nGatesAlloc;

  // moments

  vector<GateData *> _gateData;
  iwrf_xmit_rcv_mode_t _xmitRcvMode;
  RadarMoments *_mom;

  // geometry

  double _maxHt;

  // geometry
  
  double _startRange;
  double _gateSpacing;
  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeKm;
  
  // calibration

  DsRadarCalib _calib;

  // transmitter power

  double _measXmitPowerDbmH;
  double _measXmitPowerDbmV;

  // methods
  
  void _processPulse(const IwrfTsPulse *pulse);
  void _addPulseToQueue(const IwrfTsPulse *pulse);
  void _clearPulseQueue();
  
  void _initForMoments();
  void _computeMoments(const IwrfTsPulse *midPulse);
  void _computeDpAltHvCoCross();
  void _computeDpAltHvCoOnly();
  void _computeDpSimHv();
  
  void _allocGateData(int nGates);
  void _freeGateData();
  void _initFieldData();
  void _loadGateIq(const fl32 **iqChan0, const fl32 **iqChan1);

};

#endif
