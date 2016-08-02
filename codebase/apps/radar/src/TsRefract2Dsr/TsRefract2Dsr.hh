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
// TsRefract2Dsr.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////
//
// TsRefract2Dsr reads time-series data in Sigmet TsArchive
// format, computes the basic moments and refractivity quantities,
// and writes the results into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#ifndef TsRefract2Dsr_hh
#define TsRefract2Dsr_hh

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <didss/DsInputPath.hh>
#include <rapformats/DsRadarSweep.hh>
#include "Args.hh"
#include "Params.hh"
#include "OpsInfo.hh"
#include "Pulse.hh"
#include "MomentsMgr.hh"
#include "OutputFmq.hh"
#include "Beam.hh"
using namespace std;

////////////////////////
// This class

class TsRefract2Dsr {
  
public:

  // constructor

  TsRefract2Dsr (int argc, char **argv);

  // destructor
  
  ~TsRefract2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // missing data value

  static const double _missingDbl;

  // basic

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // input data

  DsInputPath *_input;

  // output fmq

  OutputFmq *_fmq;

  // pulse queue

  deque<Pulse *> _pulseQueue;
  int _maxPulseQueueSize;
  long _pulseSeqNum;
  
  // moments computation management

  MomentsMgr *_momentsMgr;
  int _nSamples;

  // beam identification

  int _midIndex1;
  int _midIndex2;
  int _countSinceBeam;

  // beam time and location
  
  time_t _time;
  double _az;
  double _el;
  double _prevAz;
  double _prevEl;
  
  int _nGatesPulse;
  int _nGatesOut;

  // send params to fmq
  
  double _prevPrfForParams;
  int _prevNGatesForParams;
  bool _paramsSentThisFile;

  // volume identification
  
  int _volNum;
  int _tiltNum;
  double _volMinEl;
  double _volMaxEl;
  int _nBeamsThisVol;

  bool _guessingSweepInfo;
  bool _currentSweepValid;
  DsRadarSweep _currentSweep;
  double _prevSweepEndTime;

  // phase drift diagnostics

  int _phaseCount;
  double _phaseDrift;
  double _prevBurstPhase;

  // pulse info

  OpsInfo *_opsInfo;

  // private functions
  
  int _processFile(const char *input_path);
  
  int _readPulseInfo(FILE *in);

  void _printPulseInfo(ostream &out) const;

  void _prepareForMoments(Pulse *pulse);
  
  bool _beamReady();
  
  int _computeBeamMoments(Beam *beam);
  
  void _addPulseToQueue(Pulse *pulse);

  void _setSweepInfo(Beam *beam, bool &endOfVol);

  int _readSweepInfo(Beam *beam, int &volNum, int &tiltNum);
  
  bool _beamWithinSweep(const Beam *beam,
                        const DsRadarSweep &sweep);

};

#endif

