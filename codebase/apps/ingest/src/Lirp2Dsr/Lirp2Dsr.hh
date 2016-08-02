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
// Lirp2Dsr.hh
//
// Lirp2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////

#ifndef Lirp2Dsr_hh
#define Lirp2Dsr_hh

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include <didss/DsInputPath.hh>
#include "Args.hh"
#include "Params.hh"
#include "Pulse.hh"
#include "MomentsMgr.hh"
#include "OutputFmq.hh"
#include "Beam.hh"
using namespace std;

////////////////////////
// This class

class Lirp2Dsr {
  
public:

  // constructor

  Lirp2Dsr (int argc, char **argv);

  // destructor
  
  ~Lirp2Dsr();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  // missing data value

  static const double _missingDbl = -9999.0;

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

  vector<MomentsMgr *> _momentsMgrArray;
  MomentsMgr *_momentsMgr;
  double _prevPrfForMoments;
  
  // SZ
  
  static const int _maxTrips = 5;
  static const int _maxGates = MAXBINS * _maxTrips;
  static const int _nSamplesSz = 64;
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
  double _volMinEl;
  double _volMaxEl;
  int _nBeamsThisVol;

  // beam queue

  deque<Beam *> _beamQueue;
  int _maxBeamQueueSize;
  int _midBeamIndex;
  
  // phase drift diagnostics

  int _phaseCount;
  double _phaseDrift;
  double _prevBurstPhase;

  // debug printing 

  int _specPrintCount;
  string _spectraFilePath;
  FILE *_spectraFile;
  
  // REC
  
  bool _applyRec;
  int _recKernelWidth;
  
  // private functions
  
  int _processFile(const char *input_path);

  int _checkForRocHeader(FILE *in, bool &hasRocHdr);

  void _prepareForMoments(Pulse *pulse);

  bool _beamReady();

  int _computeBeamMomentsBasic(Beam *beam);

  void _addPulseToQueue(Pulse *pulse);

  void _addBeamToQueue(Beam *beam);

  void _filterSpikes(double *dbzf,
		     double *velf,
		     double *widthf);
  
  void _filterDregs(double nyquist,
		    double *dbzf,
		    double *velf,
		    double *widthf);
  
  void _computeVelCensoring(const double *vel,
			    double nyquist,
			    double *vtexture,
			    double *vspin,
			    int *vcensor);
  
  void _computeZTexture(const double *dbz,
			double *ztexture);

  void _performInfilling(const double *dbz,
			 const double *vel,
			 const double *width,
			 const double *ztexture,
			 int *zinfill,
			 double *dbzi,
			 double *veli,
			 double *widthi);
  
  double _computeInterest(double xx, double x0, double x1);
  
  void _printNoise();
  
};

#endif

