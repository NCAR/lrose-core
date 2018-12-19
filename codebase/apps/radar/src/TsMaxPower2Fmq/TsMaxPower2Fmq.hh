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
// TsMaxPower2Fmq.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// TsMaxPower2Fmq - max power monitoring for HCR.
// TsMaxPower2Fmq reads radar time series data from an FMQ,
// computes the max power at any location, and writes the result
// as XML text to an FMQ.
// The HCR control app reads this data and disables the transmitter
// if the received power is too high.
//
////////////////////////////////////////////////////////////////

#ifndef TsMaxPower2Fmq_HH
#define TsMaxPower2Fmq_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/IwrfCalib.hh>
#include <Fmq/DsFmq.hh>

using namespace std;

////////////////////////
// This class

class TsMaxPower2Fmq {
  
public:

  // constructor

  TsMaxPower2Fmq (int argc, char **argv);

  // destructor
  
  ~TsMaxPower2Fmq();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;
  
  IwrfTsReader *_pulseReader;

  si64 _prevPulseSeqNum;
  ssize_t _pulseCount;
  bool _haveChan1;

  int _nSamples;
  int _startGateRequested;
  int _nGates, _startGate;
  double _startRangeM, _gateSpacingM;

  double _midTime, _midPrt;
  double _midAz, _midEl;
  double _startTime, _endTime;

  vector<double> _maxPowers0, _maxPowers1;
  double _meanMaxPower0, _meanMaxPower1;
  double _peakMaxPower0, _peakMaxPower1;
  vector<int> _gatesForMax0, _gatesForMax1;
  int _gateForMax0, _gateForMax1;
  double _rangeToMaxPower0, _rangeToMaxPower1;
  vector<RadarComplex_t> _iqForVelAtMaxPower0, _iqForVelAtMaxPower1;
  double _velAtMaxPower0, _velAtMaxPower1;

  // calibration

  IwrfCalib _calib;
  double _rxGainHc;
  double _rxGainVc;
  double _rxGainHx;
  double _rxGainVx;

  // Export via FMQ
  
  string _outputFmqPath;
  DsFmq _outputFmq;
  bool _outputFmqOpen;

  // methods
  
  // get next pulse

  IwrfTsPulse *_getNextPulse();
  
  // condition the gate range for ngates in pulse

  void _conditionGateRange(const IwrfTsPulse &pulse);

  // compute stats

  void _initMaxPowerStats();
  void _addToMaxPower(const IwrfTsPulse &pulse);
  void _computeMaxPowerStats();
  double _computeVel(const vector<RadarComplex_t> &iq);

  void _compileXmlStr(string &xmlStr);
  int _writeToFmq(const string &xmlStr);
  int _openOutputFmq();
  
  void _saveCardinalValues(const IwrfTsPulse &pulse);

};

#endif
