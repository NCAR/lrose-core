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
// BeamReader.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2007
//
///////////////////////////////////////////////////////////////

#ifndef BeamReader_hh
#define BeamReader_hh

#include <pthread.h>
#include <string>
#include <vector>
#include <deque>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include "ArrayDeque.hh"
#include "Params.hh"
#include "Args.hh"
#include "Beam.hh"
#include "SharedPointerPool.hh"
using namespace std;

////////////////////////
// This class

class BeamReader {
  
public:

  // constructor
  
  BeamReader(const string &prog_name,
             const Params &params,
             const Args &args,
             deque<Beam *> &beamRecyclePool,
             pthread_mutex_t &beamRecyclePoolMutex);
  
  // destructor
  
  ~BeamReader();

  // constructor OK?

  bool constructorOK;

  // get the next beam
  // returns Beam object pointer on success, NULL on failure
  // caller must free beam
  
  Beam *getNextBeam();

  // get ops info

  const IwrfTsInfo &getOpsInfo() const { return _pulseReader->getOpsInfo(); }
  bool isOpsInfoNew() const { return _pulseReader->isOpsInfoNew(); }
  Beam::scan_type_t getScanType() const { return _scanType; }

protected:
  
private:

  string _progName;
  const Params &_params;
  const Args &_args;

  // beam pool - so that beam objects can be recycled

  deque<Beam *> &_beamRecyclePool;
  pthread_mutex_t &_beamRecyclePoolMutex;
  
  // Pulse reader
  
  IwrfTsReader *_pulseReader;
  
  // start/end times - archive mode
  
  time_t _startTime;
  time_t _endTime;

  // Pulse pool.
  // The pulse pool provides shared pointers to reusable IwrfTsPulse instances.
  // When the reference counts for provided shared pointers go to zero, the
  // associated IwrfTsPulse instances are automatically returned to the pool.
  
  SharedPointerPool<IwrfTsPulse, true> _pulsePool;
  
  // active pulse queue
  
  si64 _pulseSeqNum;
  si64 _prevPulseSeqNum;
  shared_ptr<IwrfTsPulse> _prevPulse;
  shared_ptr<IwrfTsPulse> _latestPulse;
  deque<shared_ptr<IwrfTsPulse>> _pulseQueue;
  deque<shared_ptr<IwrfTsPulse>> _pulseCache;
  int _pulseCount;
  int _pulseCountSinceStatus;
  
  // beam identification

  bool _beamError;
  si64 _beamCount;
  int _midIndex, _startIndex, _endIndex;
  si64 _prevBeamPulseSeqNum; // pulse after center of beam

  // beam properties

  Beam::scan_type_t _scanType;
  int _nGates;
  int _nSamples;

  double _az;
  double _el;
  
  double _prt;
  double _meanPrf;
  double _pulseWidthUs;
  deque<bool> _fixedPulseWidthCheck;

  // beam location/rate in azimuth - PPI mode

  int _azIndex;
  int _prevAzIndex;

  // beam location/rate in elevation - RHI mode
  
  int _elIndex;
  int _prevElIndex;

  // window 90% factors
  // this is the fraction of the window width, centered, which
  // accounts for 90 percent of the power

  double _windowFactorRect;
  double _windowFactorVonhann;
  double _windowFactorBlackman;
  double _windowFactorBlackmanNuttall;

  // private functions
  
  int _initializeQueue();

  int _readNonIndexedBeam();
  int _readPulseWidthChangeBeam();
  int _readIndexedBeam();

  int _finalizeNonIndexedBeam();
  int _checkStartConditions();
  int _checkFixedPulseWidth();
  void _constrainPulsesToWithinDwell();

  shared_ptr<IwrfTsPulse> _getNextPulse();
  shared_ptr<IwrfTsPulse> _readNextPulse();
  shared_ptr<IwrfTsPulse> _doReadNextPulse();
  
  bool _beamOk();
  void _addPulseToQueue(shared_ptr<IwrfTsPulse> pulse);
  void _cacheLatestPulse();
  
  void _clearPulseQueue();
  void _recyclePulses();
  shared_ptr<IwrfTsPulse> _getPulseFromRecyclePool();

  // void _initPpiMode();
  // void _initRhiMode();
  // void _initVertMode();

  void _setPrt();
  
  // int _findBeamCenterPpi();
  // int _findBeamCenterRhi();

  int _computeMinNGates();
  int _computeNSamplesIndexed();
  int _computeNSamplesRect(int nSamples);

  // void _checkIsAlternating();
  // void _checkAlternatingStartsOnH();
  // void _checkIsStaggeredPrt();
  // void _checkStaggeredStartsOnShort();

  double _conditionAz(double az);
  double _conditionEl(double el);
  
  // void _computeProgressiveAzRate(const shared_ptr<IwrfTsPulse> pulse);
  // void _computeProgressiveElRate(const shared_ptr<IwrfTsPulse> pulse);

  // void _computeBeamAzRate(int endIndex, int nSamples);
  // void _computeBeamElRate(int endIndex, int nSamples);

  // void _checkForEndFlags(const vector<shared_ptr<IwrfTsPulse>> &beamPulses);
  void _checkQueueStatus();
  void _computeWindowFactors();

};

#endif

