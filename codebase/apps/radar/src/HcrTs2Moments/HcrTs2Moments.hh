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
// HcrTs2Moments.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
// Copied from Iq2Dsr, then cleaned up.
//
///////////////////////////////////////////////////////////////
//
// HcrTs2Moments reads IQ time-series data, computes the
// moments and writes the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#ifndef HcrTs2Moments_hh
#define HcrTs2Moments_hh

#include <string>
#include <vector>
#include <rapformats/DsRadarSweep.hh>
#include "Args.hh"
#include "Params.hh"
#include "OutputFmq.hh"
#include "BeamReader.hh"
#include "Threads.hh"
#include "CalibMgr.hh"
using namespace std;

////////////////////////
// This class

class HcrTs2Moments {
  
public:

  // constructor

  HcrTs2Moments (int argc, char **argv);

  // destructor
  
  ~HcrTs2Moments();

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

  CalibMgr *_calMgr;

  // send params to fmq?
  
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

};

#endif

