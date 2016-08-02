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
// SimCpCompute.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#ifndef SimCpCompute_H
#define SimCpCompute_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "Beam.hh"
#include "Ppi.hh"
#include "Complex.hh"
#include <radar/IwrfTsReader.hh>

using namespace std;

////////////////////////
// This class

class SimCpCompute {
  
public:

  // constructor

  SimCpCompute (int argc, char **argv);

  // destructor
  
  ~SimCpCompute();

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

  // pulse queue
  
  IwrfTsReader *_reader;
  deque<const IwrfTsPulse *> _pulseQueue;
  int _maxPulseQueueSize;
  long _pulseSeqNum;
  int _totalPulseCount;
  int _nSamples;
  int _nSamplesHalf;
  int _nGates;

  // H and V queue

  IwrfTsReader *_horizReader;
  IwrfTsReader *_vertReader;
  deque<const IwrfTsPulse *> _horizQueue;
  deque<const IwrfTsPulse *> _vertQueue;

  // beam

  double _beamAz;
  double _beamEl;
  double _beamTime;
  int _midIndex1;
  int _midIndex2;
  bool _clockwise;

  // analysis in range
  
  double _startRangeAnalysis;
  double _endRangeAnalysis;
  double _gateSpacingAnalysis;
  int _startGateAnalysis;
  int _endGateAnalysis;
  int _nGatesAnalysis;
  bool _gateSpacingWarningPrinted;

  // range correction

  double *_rangeCorr;
  double _startRange;
  double _gateSpacing;

  // calibration
  
  double _atmosAtten;  // db/km
  double _noiseHc;
  double _noiseHx;
  double _noiseVc;
  double _noiseVx;
  double _dbz0Hc;
  double _dbz0Hx;
  double _dbz0Vc;
  double _dbz0Vx;

  // moments computations

  vector<MomentData> _momentData;
  double _wavelengthMeters;

  // ppis

  Ppi *_currentPpi;
  Ppi *_prevPpi;

  // xdr power difference

  double _countVcMinusHc;
  double _sumVcMinusHc;
  double _meanVcMinusHc;

  // cross-polar power difference

  double _countVxMinusHx;
  double _sumVxMinusHx;
  double _sum2VxMinusHx;
  double _meanVxMinusHx;
  double _sdevVxMinusHx;

  // methods

  int _runMovingMode();
  int _runPointingMode();

  void _processPulse(const IwrfTsPulse *pulse);
  void _processHVPulses(const IwrfTsPulse *horizPulse,
                        const IwrfTsPulse *vertPulse);

  void _processBeam();
  void _computePointing();

  void _addPulseToQueue(const IwrfTsPulse *pulse);
  int _loadHorizQueue();
  int _loadVertQueue();

  void _clearPulseQueue();
  void _clearHorizQueue();
  void _clearVertQueue();

  bool _beamReady();

  void _initForMoments(const IwrfTsInfo &opsInfo);

  void _computeMomentsDualAlt(const deque<const IwrfTsPulse *> &pulseQueue,
                              const IwrfTsInfo &opsInfo,
                              vector<MomentData> &momentData);

  void _computeRangeCorrection();

  void _computePpiStats();
  void _computeGlobalStats();
  int _writeResults();
  int _writeGlobalResults();

  double _computePower(const Complex_t *IQ,
                       int nSamples) const;

  void _velWidthFromTd(const Complex_t *IQ,
                       int nSamples,
                       double prtSecs,
                       double &vel, double &width) const;
  
  void _markValidMoments(vector<MomentData> &momentData);

};

#endif
