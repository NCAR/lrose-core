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
// Iq2Dsr.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////

#ifndef Iq2Dsr_hh
#define Iq2Dsr_hh

#include <string>
#include <vector>
#include <rapformats/DsRadarSweep.hh>
#include "Args.hh"
#include "Params.hh"
#include "MomentsMgr.hh"
#include "OutputFmq.hh"
#include "BeamReader.hh"
#include "Threads.hh"
#include "Calibration.hh"
using namespace std;

////////////////////////
// This class

class Iq2Dsr {
  
public:

  // constructor

  Iq2Dsr (int argc, char **argv);

  // destructor
  
  ~Iq2Dsr();

  // run 

  int Run();

  // data members

  bool constructorOK;

  // get parameters

  const Params &getParams() const { return _params; }
  double getWavelengthM() const {
    return _beamReader->getOpsInfo().get_radar_wavelength_cm() / 100.0;
  }

  // get output fmq
  
  OutputFmq *getOutputFmq() const { return _fmq; }

  // write beams - called from thread method
  
  int writeBeams();

  // SZ constants
  
  static const int maxTrips = 3;
  static const int maxBins = 4096;
  static const int maxGates = maxBins * maxTrips;

  pthread_mutex_t *getDebugPrintMutex() { return &_debugPrintMutex; }

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // beam reader

  BeamReader *_beamReader;

  // output fmq
  
  OutputFmq *_fmq;
  WriteThread *_writeThread;

  // Compute thread pool, and active compute threads.
  // When a new beam is read, we take a thread from the pool and
  // add it to the active threads. When done, we move it back
  // to the pool.

  vector<ComputeThread *> _threadPool;
  int _threadPoolSize;
  int _threadPoolInsertPos;
  int _threadPoolRetrievePos;

  // beam pool

  deque<Beam *> _beamRecyclePool;
  pthread_mutex_t _beamRecyclePoolMutex;

  // debug print mutex

  pthread_mutex_t _debugPrintMutex;

  // calibration

  Calibration *_calib;

  // Moments computation management.
  // We keep two sets of managers, so that we can toggle between them
  // for each alternate beam. This keeps the memory separate when we
  // are using a separate thread for the BeamReader.
  
  vector<const MomentsMgr *> _momentsMgrArray;
  double _prevPrtForMoments;
  
  // sweep and volume identification
  
  int _beamScanMode;

  int _beamVolNum;
  int _prevVolNum;

  int _beamSweepNum;
  int _prevSweepNum;

  int _currentScanMode;
  int _currentVolNum;
  int _currentSweepNum;

  bool _endOfVolFlag;
  bool _endOfSweepFlag;

  bool _startOfSweepPending;
  bool _startOfVolPending;
  bool _endOfVolPending;

  bool _antennaTransition;

  double _prevEl;
  double _volMinEl;
  double _volMaxEl;
  int _nBeamsThisVol;

  // delaying sweep change until antenna changes direction

  bool _prevAntennaTransition;
  bool _inTransition;
  double _prevAngle;
  double _motionDirn;
  int _nRaysInSweep;
  
  // end of vol decision

  double _prevPrtForEndOfVol;
  double _prevPulseWidthForEndOfVol;

  // send params to fmq?
  
  double _prevPrtForParams;
  int _prevNGatesForParams;
  int _prevScanModeForParams;
  int _nBeamsSinceParams;

  // run time

  double _startTimeSecs;
  double _endTimeSecs;
  double _nGatesComputed;

  // private functions

  void _cleanUp();
  int _runSingleThreaded();
  int _runMultiThreaded();
  
  int _processBeamSingleThreaded(Beam *beam);
  int _processBeamMultiThreaded(Beam *beam);
  
  // thread functions

  static void* _handleWrites(void *thread_data);
  static void* _computeMomentsInThread(void *thread_data);
  
  // radar and field params, and calibration

  int _writeParamsAndCalib(const Beam *beam);

  // write any remaining beams on exit
  
  int _writeRemainingBeamsOnExit();

// sweep and vol info

  void _handleSweepAndVolChange(const Beam *beam);
  void _putEndOfVol(const Beam *beam);
  void _deduceEndOfVol(const Beam *beam);
  void _changeSweepOnDirectionChange(const Beam *beam);

};

#endif

