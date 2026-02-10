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
// TsPrint.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////
//
// TsPrint reads time-series data from an FMQ
// and processes it in various ways
//
////////////////////////////////////////////////////////////////

#ifndef TsPrint_H
#define TsPrint_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "Stats.hh"
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/IwrfCalib.hh>
#include <toolsa/Socket.hh>

using namespace std;

////////////////////////
// This class

class TsPrint {
  
public:

  // constructor

  TsPrint (int argc, char **argv);

  // destructor
  
  ~TsPrint();

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

  ssize_t _nPulsesRead;
  IwrfTsPulse *_firstPulse;
  IwrfTsPulse *_secondPulse;

  iwrf_pulse_header _prevPulseHdr;
  si64 _prevPulseSeqNum;
  ssize_t _totalPulseCount;
  ssize_t _pulseCount;
  ssize_t _printCount;

  bool _midTransition;
  double _midTime, _midPrt;
  double _midAz, _midEl;
  double _startTime, _endTime;

  Stats _stats;
  vector<Stats> _ascopeStats;

  vector<double> _maxPowers0, _maxPowers1;
  double _meanMaxPower0, _meanMaxPower1;
  double _peakMaxPower0, _peakMaxPower1;
  vector<int> _gatesForMax0, _gatesForMax1;
  int _gateForMax0, _gateForMax1;
  double _rangeToMaxPower0, _rangeToMaxPower1;
  vector<RadarComplex_t> _iqForVelAtMaxPower0, _iqForVelAtMaxPower1;
  double _velAtMaxPower0, _velAtMaxPower1;

  int _nSamples;
  int _nGatesRequested, _startGateRequested;
  int _nGates, _startGate, _endGate;
  double _startRangeM, _gateSpacingM;

  bool _haveChan1;
  bool _fastAlternating;
  bool _latestIsH;
  bool _labviewRequest;

  // previous values - to check for changes

  iwrf_scan_segment_t _scanPrev;
  iwrf_ts_processing_t _procPrev;
  iwrf_calibration_t _calibPrev;
  bool _infoChanged;

  // calibration

  IwrfCalib _calib;
  double _rxGainHc;
  double _rxGainVc;
  double _rxGainHx;
  double _rxGainVx;

  // angle change

  double _prevAzimuth;
  double _prevElevation;

  // extra columns from XML blocks

  vector<string> _extraColLabels;
  vector<double> _extraColMeans;
  vector<double> _extraColSums;
  vector<double> _extraColCounts;
  si64 _statusXmlPktSeqNum;

  // methods

  int _runPrintMode();
  int _printPrtDetails();
  int _runAscopeMode();
  int _runMaxPowerMode();
  int _runServerMode();
  int _runMaxPowerServerMode();
  
  // get the next pulse

  IwrfTsPulse *_getNextPulse();

  // get the next pulse, checking for timeout etc.
  
  IwrfTsPulse *_getNextPulseCheckTimeout();

  // condition the gate range for ngates in pulse

  void _conditionGateRange(const IwrfTsPulse &pulse);

  // save the metadata
  
  void _saveMetaData(const IwrfTsPulse &pulse);

  // accumulate stats

  void _addToSummary(const IwrfTsPulse &pulse);
  void _addToAlternating(const IwrfTsPulse &pulse);
  void _addToAscope(const IwrfTsPulse &pulse);
  void _addToMaxPower(const IwrfTsPulse &pulse);
  
  // compute stats

  void _computeStats();

  // clear stats

  void _clearStats();

  // check info changed
  
  bool _checkInfoChanged(const IwrfTsPulse &pulse);

  // server mode
  
  int _handleClient(Socket *sock);
  int _handleMaxPowerClient(Socket *sock);
  void _reapChildren();
  int _readCommands(Socket *sock, string &xmlBuf);
  int _decodeCommands(string &xmlBuf);
  void _prepareXmlResponse(int iret, string &xmlResults);
  void _prepareMaxPowerResponse(string &xmlResults);
  void _prepareLabviewResponse(int iret, string &xmlResults);
  int _writeResponse(Socket *sock, string &xmlBuf);

  // printing

  void _printRunDetails(ostream &out);
  void _printOpsInfo(ostream &out, const IwrfTsPulse *pulse);
  
  void _printPulseLabels(ostream &out);
  string _pulseString(const IwrfTsPulse &pulse);

  void _printSummaryHeading(ostream &out);
  void _printSummaryLabels(ostream &out);
  void _printSummaryData(FILE *out);

  void _initMaxPowerData();
  void _computeMaxPowerData();
  void _printMaxPowerHeading(ostream &out);
  void _printMaxPowerLabels(ostream &out);
  void _printMaxPowerData(FILE *out);
  
  double _computeVel(const vector<RadarComplex_t> &iq);

  // check angle change

  void _checkAngleChange(const IwrfTsPulse &pulse);

  // read XML data for extra columns

  void _initExtraCols();
  void _decodeXmlForExtraCols();
  void _computeExtraColMeans();

};

#endif
