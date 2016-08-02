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
// SunCpCompute.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2007
//
///////////////////////////////////////////////////////////////

#ifndef SunCpCompute_H
#define SunCpCompute_H

#include <string>
#include <vector>
#include <deque>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include "Moments.hh"
#include <radar/IwrfTsReader.hh>
#include <euclid/SunPosn.hh>

using namespace std;

////////////////////////
// This class

class SunCpCompute {
  
public:

  // constructor

  SunCpCompute (int argc, char **argv);

  // destructor
  
  ~SunCpCompute();

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
  IwrfTsReader *_reader;
  SunPosn _sunPosn;

  // pulse queue

  int _nSamples;
  deque<const IwrfTsPulse *> _pulseQueue;
  int _maxPulseQueueSize;
  long _pulseSeqNum;
  int _totalPulseCount;

  // calibration times

  double _startTime;
  double _endTime;
  double _calTime;

  // computing stats
  
  double _midTime, _midPrt;
  double _midAz, _midEl;
  double _offsetAz, _offsetEl;

  // storing moments

  vector<Moments *> _moments;
  vector<Moments *> _momentsNoise;

  // results

  double _nBeamsNoise;
  double _noisePowerHc;
  double _noisePowerHx;
  double _noisePowerVc;
  double _noisePowerVx;

  double _noiseDbmHc;
  double _noiseDbmHx;
  double _noiseDbmVc;
  double _noiseDbmVx;

  double _nBeamsUsed;

  double _meanDbmHc;
  double _meanDbmHx;
  double _meanDbmVc;
  double _meanDbmVx;

  double _meanDiffVcHc;
  double _meanDiffVxHx;
  double _sdevDiffVcHc;
  double _sdevDiffVxHx;
  double _twoSigmaDiffVcHc;
  double _twoSigmaDiffVxHx;

  double _meanRatioVcHc;
  double _meanRatioVxHx;
  double _sdevRatioVcHc;
  double _sdevRatioVxHx;
  double _twoSigmaRatioVcHc;
  double _twoSigmaRatioVxHx;

  // methods

  void _processPulse(const IwrfTsPulse *pulse);
  void _addPulseToQueue(const IwrfTsPulse *pulse);
  void _clearPulseQueue();

  void _printMomentsLabels();

  void _computeMomentsAlt(Moments *moments);
  void _clearMomentsArray();
  void _clearNoiseMomentsArray();

  int _computeNoisePower();

  void _computePowerRatios();
  int _writeResults();

  void _checkForNorthCrossing(double &az0, double &az1);

};

#endif
