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
// AparTsPrint.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
///////////////////////////////////////////////////////////////
//
// AparTsPrint reads apar time-series data, processes it
// in various ways, and prints out the results
//
////////////////////////////////////////////////////////////////

#ifndef AparTsPrint_H
#define AparTsPrint_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "Stats.hh"
#include <apardata/AparTsInfo.hh>
#include <apardata/AparTsPulse.hh>
#include <apardata/AparTsReader.hh>
#include <apardata/AparTsCalib.hh>
#include <toolsa/Socket.hh>

using namespace std;

////////////////////////
// This class

class AparTsPrint {
  
public:

  // constructor

  AparTsPrint (int argc, char **argv);

  // destructor
  
  ~AparTsPrint();

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
  
  AparTsReader *_pulseReader;

  si64 _prevPulseSeqNum;
  ssize_t _totalPulseCount;
  ssize_t _pulseCount;
  ssize_t _printCount;
  bool _haveChan1;

  double _midTime, _midPrt;
  double _midAz, _midEl;
  double _startTime, _endTime;

  Stats _stats;
  vector<Stats> _aStats;

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

  bool _dualChannel;
  bool _fastAlternating;
  bool _labviewRequest;

  // previous values - to check for changes

  apar_ts_scan_segment_t _scanPrev;
  apar_ts_processing_t _procPrev;
  apar_ts_calibration_t _calibPrev;
  bool _infoChanged;

  // calibration

  AparTsCalib _calib;
  double _rxGainHc;
  double _rxGainVc;
  double _rxGainHx;
  double _rxGainVx;

  // angle change

  double _prevAzimuth;
  double _prevElevation;

  // methods

  int _runPrintMode();
  int _printPrtDetails();
  int _runAscopeMode();
  int _runMaxPowerMode();
  int _runCalMode();
  int _runServerMode();
  int _runMaxPowerServerMode();
  
  // get next pulse

  AparTsPulse *_getNextPulse();

  // condition the gate range for ngates in pulse

  void _conditionGateRange(const AparTsPulse &pulse);

  // compute stats

  void _saveCardinalValues(const AparTsPulse &pulse);
  void _addToSummary(const AparTsPulse &pulse);
  void _computeSummary();

  void _addToAlternating(const AparTsPulse &pulse);
  void _computeAlternating();

  void _addToDual(const AparTsPulse &pulse);
  void _computeDual();

  void _addToAscope(const AparTsPulse &pulse);
  void _addToMaxPower(const AparTsPulse &pulse);

  void _clearStats();

  // check info changed
  
  bool _checkInfoChanged(const AparTsPulse &pulse);

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
  void _printOpsInfo(ostream &out, const AparTsPulse *pulse);
  
  void _printPulseLabels(ostream &out);
  string _pulseString(const AparTsPulse &pulse);

  void _printSummaryHeading(ostream &out);
  void _printAlternatingHeading(ostream &out);
  void _printDualHeading(ostream &out);
  
  void _printSummaryLabels(ostream &out);
  void _printSummaryData(FILE *out);
  void _printAlternatingLabels(ostream &out);
  void _printDualLabels(ostream &out);
  void _printAlternatingData(FILE *out);
  void _printDualData(FILE *out);

  void _printMaxPowerHeading(ostream &out);
  void _printMaxPowerLabels(ostream &out);
  void _initMaxPowerData();
  void _computeMaxPowerData();
  double _computeVel(const vector<RadarComplex_t> &iq);
  void _printMaxPowerData(FILE *out);

  // check angle change

  void _checkAngleChange(const AparTsPulse &pulse);

  // print sizes of structures

  void _printStructSizes(ostream &out);
  
};

#endif
