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
// August 2019
//
///////////////////////////////////////////////////////////////

#ifndef BeamReader_hh
#define BeamReader_hh

#include <pthread.h>
#include <string>
#include <vector>
#include <deque>
#include <radar/AparTsInfo.hh>
#include <radar/AparTsPulse.hh>
#include <radar/AparTsReader.hh>
#include <radar/AtmosAtten.hh>
#include "Params.hh"
#include "Args.hh"
#include "Beam.hh"
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

  // if end of vol is encountered in time series, the _endOfVol flag
  // will be set. Similarly for end of sweep.
  // the flags are cleared when a new beam is created.

  bool getEndOfSweepFlag() const { return _endOfSweepFlag; }
  bool getEndOfVolFlag() const { return _endOfVolFlag; }
    
  // get ops info

  const AparTsInfo &getOpsInfo() const { return _pulseReader->getOpsInfo(); }
  bool isOpsInfoNew() const { return _pulseReader->isOpsInfoNew(); }
  Beam::scan_type_t getScanType() const { return _scanType; }

protected:
  
private:

  string _progName;
  const Params &_params;
  const Args &_args;

  // dwell sampling

  int _beamNumInDwell;
  si64 _dwellSeqNum;

  // Pulse reader
  
  AparTsReader *_pulseReader;

  // active pulse queue
  
  si64 _pulseSeqNum;
  si64 _pulseCount;

  AparTsPulse *_cachedPulse;
  vector<AparTsPulse *> _dwellPulses;
  bool _dwellReady;

  si64 _beamCount;
  vector<const AparTsPulse *> _beamPulses;

  // pulse pool - so that pulses can be recycled

  deque<AparTsPulse *> _pulseRecyclePool;
  
  // start/end times - archive mode

  time_t _startTime;
  time_t _endTime;

  // end of sweep and vol flags
  
  bool _endOfSweepFlag;
  bool _endOfVolFlag;

  // beam pool - so that beam objects can be recycled

  deque<Beam *> &_beamRecyclePool;
  pthread_mutex_t &_beamRecyclePoolMutex;

  // beam properties

  Beam::scan_type_t _scanType;
  int _nGates;

  double _az;
  double _el;

  double _prt;
  double _meanPrf;
  double _pulseWidthUs;

  // pulse-to-pulse HV alternating mode

  bool _isAlternating;
  bool _isStaggeredPrt;

  double _prtShort;
  double _prtLong;
  
  int _nGatesPrtShort;
  int _nGatesPrtLong;
  
  // atmospheric attenuation

  AtmosAtten _atmosAtten;

  // private functions
  
  int _readPulsesForDwell();

  AparTsPulse *_getNextPulse();
  AparTsPulse *_readNextPulse();

  bool _beamOk();
  void _addPulseToDwell(AparTsPulse *pulse);

  void _clearDwellPulses();
  void _addPulseToRecyclePool(AparTsPulse *pulse);
  AparTsPulse *_getPulseFromRecyclePool();
  
  void _setBeamParams();
  int _computeMinNGates();
  void _checkIsAlternating();
  void _checkIsStaggeredPrt();

};

#endif

