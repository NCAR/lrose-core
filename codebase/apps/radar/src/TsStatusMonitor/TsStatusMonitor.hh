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
// TsStatusMonitor.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2011
//
///////////////////////////////////////////////////////////////
//
// TsStatusMonitor reads IWRF time-series data from a file message
// queue (FMQ).
// It locates monitoring information in the time series, and
// writes that information out to SPDB, and in a form suitable
// for Nagios.
//
////////////////////////////////////////////////////////////////

#ifndef TsStatusMonitor_HH
#define TsStatusMonitor_HH

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "StatsField.hh"
#include <toolsa/MemBuf.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/RadarComplex.hh>
#include <Radx/RadxTime.hh>
#include <Fmq/Fmq.hh>
#include <Spdb/DsSpdb.hh>

using namespace std;

////////////////////////
// This class

class TsStatusMonitor {
  
public:

  // constructor

  TsStatusMonitor(int argc, char **argv);

  // destructor
  
  ~TsStatusMonitor();

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

  // reading pulses

  IwrfTsReader *_pulseReader;
  time_t _pulseLatestTime;

  time_t _prevSpdbTime;
  time_t _prevNagiosTime;
  
  // test pulse

  int _nSamplesTestPulse;
  time_t _testPulseLatestTime;
  string _testPulseXml;

  RadarComplex_t *_testIqHc;
  RadarComplex_t *_testIqHx;
  RadarComplex_t *_testIqVc;
  RadarComplex_t *_testIqVx;

  double _testPowerDbHc;
  double _testPowerDbHx;
  double _testPowerDbVc;
  double _testPowerDbVx;

  double _testVelHc;
  double _testVelHx;
  double _testVelVc;
  double _testVelVx;

  // g0 velocity

  int _nSamplesG0;
  time_t _g0LatestTime;
  string _g0Xml;

  RadarComplex_t *_g0IqHc;
  double _g0PowerDbHc;
  double _g0VelHc;
  
  // movement check
  
  double _moveCheckAz;
  double _moveCheckEl;
  string _movementXml;
  time_t _movementMonitorTime;
  time_t _antennaStopTime;
  int _antennaStopSecs;
  bool _isMoving;

  // IWRF xml string if available

  si64 _iwrfStatusXmlPktSeqNum;
  string _iwrfStatusXml;
  time_t _iwrfStatusLatestTime;
  
  // SPDB

  DsSpdb _spdb;
  
  // catalog stats

  vector<StatsField *> _catFields;
  RadxTime _statsScheduledTime;
  time_t _statsStartTime;
  time_t _statsEndTime;

  // functions

  void _clearStatus();

  int _handlePulse(IwrfTsPulse &pulse);

  void _monitorAntennaMovement(IwrfTsPulse &pulse);
  
  void _monitorTestPulse(IwrfTsPulse &pulse,
                         const IwrfTsInfo &info);

  void _loadTestPulseIq(IwrfTsPulse &pulse,
                        int channelNum,
                        int gateNum,
                        RadarComplex_t *iq);

  void _monitorG0(IwrfTsPulse &pulse,
                  const IwrfTsInfo &info);
  
  void _loadG0Iq(IwrfTsPulse &pulse,
                 int channelNum,
                 int gateNum,
                 RadarComplex_t *iq);

  string _getCombinedXml(time_t now);

  int _updateSpdb(time_t now);

  int _updateNagios(time_t now);

  int _handleBooleanNagios(const string &xml,
                           const Params::xml_entry_t &entry,
                           FILE *nagiosFile);

  int _handleIntNagios(const string &xml,
                       const Params::xml_entry_t &entry,
                       FILE *nagiosFile);

  int _handleDoubleNagios(const string &xml,
                          const Params::xml_entry_t &entry,
                          FILE *nagiosFile);

  int _handleStringNagios(const string &xml,
                          const Params::xml_entry_t &entry,
                          FILE *nagiosFile);

  int _handleMissingEntry(const string &xml,
                          const Params::xml_entry_t &entry,
                          FILE *nagiosFile);
  
  int _addMovementToNagios(FILE *nagiosFile);

  void _removeNagiosStatusFile();

  void _initStatsFields();

  int _updateCatalogStats(time_t now);

  void _printStats(FILE *out);

  void _writeStatsFile();

};

#endif
