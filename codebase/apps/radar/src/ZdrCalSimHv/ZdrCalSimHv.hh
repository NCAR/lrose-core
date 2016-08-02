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
// ZdrCalSimHv.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2011
//
///////////////////////////////////////////////////////////////
//
// ZdrCalSimHv computes the cross-polar ratios between H and V
// returns. This is used for the cross-polar method of ZDR
// calibration, in conjunction with SunScan analysis.
// See SunCal for more info on the sun calibration aspects.
//
////////////////////////////////////////////////////////////////

#ifndef ZdrCalSimHv_HH
#define ZdrCalSimHv_HH

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <radar/IwrfTsReader.hh>
#include <radar/GateData.hh>
#include <rapformats/DsRadarCalib.hh>
class DsMdvx;

using namespace std;

////////////////////////
// This class

class ZdrCalSimHv {
  
public:

  // constructor

  ZdrCalSimHv (int argc, char **argv);

  // destructor
  
  ~ZdrCalSimHv();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // class for accumulating power value stats

  class PStats {
  public:
    double powerHx, powerVx;
    double sumPowerHx, sumPowerVx;
    int nSumHx, nSumVx;
    PStats() {
      clear();
    }
    void clear() {
      clearHx();
      clearVx();
      sumPowerHx = 0.0;
      sumPowerVx = 0.0;
      nSumHx = 0;
      nSumVx = 0;
    }
    void clearHx() {
      powerHx = 0.0;
    }
    void clearVx() {
      powerVx = 0.0;
    }
  };

  // members

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // pulse reader and queue
  
  IwrfTsReader *_reader;
  si64 _prevSeqNum;
  si64 _pulseCount;
  deque<const IwrfTsPulse *> _pulseQueue;
  size_t _nSamples;
  size_t _nSamplesInUse;

  // range

  double _startRangeAnalysis, _endRangeAnalysis;
  double _gateSpacingAnalysis;
  int _startGateAnalysis, _endGateAnalysis;
  size_t _nGatesAnalysis;

  // beams are based on azimuth

  time_t _beamTime;
  time_t _prevBeamTime;
  double _azRes;
  size_t _nAz;
  size_t _azIndex;
  double _beamAz, _beamEl;
  double _sumEl, _nEl;
  size_t _nGatesData;
  size_t _nBeams, _nBeamsH, _nBeamsV;
  vector<bool> _processThisAz;

  // flag for H or V transmit

  bool _transmitH;

  // arrays of power values
  
  vector<vector<PStats> > _ppiPStats;
  vector<PStats> _stationaryPStats;
    
  // gate data

  vector<GateData *> _gateData;

  // alternating and switching mode

  bool _alternating;
  bool _switching;

  // calibration

  DsRadarCalib _calib;
  double _noiseHc;
  double _noiseHx;
  double _noiseVc;
  double _noiseVx;
  
  // results

  time_t _startTime, _endTime;
  double _radarLat, _radarLon, _radarAltKm;
  string _radarName;
  string _siteName;
  
  int _nBeamsSinceSwitch;
  bool _endOfScanPair;
  double _minValidRatio, _maxValidRatio;
  
  int _nPairsRunning;
  double _sumNormPowerHxRunning;
  double _sumNormPowerVxRunning;
  double _meanNormPowerHxDbRunning;
  double _meanNormPowerVxDbRunning;
  double _hxVxRatioMeanRunning;
  double _hxVxRatioMeanDbRunning;

  int _nPairs;
  double _sumNormPowerHx;
  double _sumNormPowerVx;
  double _meanNormPowerHxDb;
  double _meanNormPowerVxDb;
  double _hxVxRatioMean;
  double _hxVxRatioMeanDb;

  FILE *_pairsFile;
  FILE *_resultsFile;
  
  int _nPairsByGate;
  double _sumNormPowerHxByGate;
  double _sumNormPowerVxByGate;

  int _nPairsByGateTotal;
  double _sumNormPowerHxByGateTotal;
  double _sumNormPowerVxByGateTotal;

  vector<GateData *> _gateDataByGate;
  int _nHByGate, _nVByGate;

  // methods

  int _runScanningMode();
  int _runStationaryMode();
  int _processPulseScanning(const IwrfTsPulse *pulse);
  int _processPulseStationary(const IwrfTsPulse *pulse);
  void _addPulseToQueue(const IwrfTsPulse *pulse);
  void _clearPulseQueue();
  bool _checkNGates();
  bool _checkDual();
  bool _checkAz();
  int _processBeamScanning();
  int _processBeamStationary();
  void _getNoiseFromTimeSeries();
  void _allocGateData();
  void _freeGateData();
  void _loadGateDataSim();
  void _loadGateDataAlt(bool transmitH);
  void _loadGateDataAltTransmitH();
  void _loadGateDataAltTransmitV();

  double _computeHvPowerRatio();
  void _setPpiPStatsToZero();
  void _setStationaryPStatsToZero();
  void _clearResultsRunning();
  void _clearResults();
  void _computeResults();
  int _writeResults();
  void _writeResults(FILE *out);
  int _writeRunningSummary();
  int _writePairData(double beamEl,
                     double beamAz,
                     double range,
                     double powerHxDb,
                     double powerVxDb);
  int _openResultsFile();
  int _openPairsFile();

  int _processBeamAltByGate();
  void _loadGateDataAltByGate();
  void _allocGateDataByGate();
  void _addToResultsByGate(double range, double snrX,
                           double powerHx, double powerVx);
  void _writeRunningSummaryByGate();

  void _writeMdv();
  void _addMdvField(DsMdvx &outMdvx,
                    const string &field_name,
                    const string &long_field_name,
                    const string &units,
                    const fl32 *data);

};

#endif
