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
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////

#ifndef BeamReader_hh
#define BeamReader_hh

#include <string>
#include <vector>
#include <deque>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include "Params.hh"
#include "Args.hh"
#include "Beam.hh"
#include "BeamMgr.hh"
using namespace std;

////////////////////////
// This class

class BeamReader {
  
public:

  // scan mode for determining PPI vs RHI operations
  
  typedef enum {
    SCAN_TYPE_UNKNOWN,
    SCAN_TYPE_PPI,
    SCAN_TYPE_RHI
  } scan_type_t;
  
  // constructor
  
  BeamReader (const string &prog_name,
              const Params &params,
              const Args &args,
	      BeamMgr &mgr);
  
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
  scan_type_t getScanType() const { return _scanType; }

protected:
  
private:

  string _progName;
  const Params &_params;
  const Args &_args;
  BeamMgr &_beamMgr;

  // Pulse reader
  
  IwrfTsReader *_pulseReader;

  // pulse queue
  
  deque<const IwrfTsPulse *> _pulseQueue;
  int _maxPulseQueueSize;
  long _pulseSeqNum;
  
  // beam identification

  long _beamCount;
  int _midIndex1;
  int _midIndex2;
  int _pulseCountSinceBeam;
  int _offsetToBeamStart; // from front of queue

  // number of gates

  int _nGates;

  // number of samples

  int _nSamples;

  // beam time and location

  scan_type_t _scanType;
  time_t _time;
  double _az;
  double _el;
  double _prt;

  // pulse-to-pulse HV alternating mode

  bool _isAlternating;
  bool _startsOnHoriz;

  // staggered PRT

  bool _isStaggeredPrt;
  bool _startsOnPrtShort;

  double _prtShort;
  double _prtLong;
  
  int _nGatesPrtShort;
  int _nGatesPrtLong;

  // beam location/rate in azimuth - PPI mode
  
  int _azIndex;
  int _prevAzIndex;
  
  // beam location/rate in elevation - RHI mode

  int _elIndex;
  int _prevElIndex;

  // private functions
  
  void _initPpiMode();
  void _initRhiMode();
  bool _beamReady();
  bool _beamReadyPpi();
  bool _beamReadyRhi();
  void _checkIsAlternating();
  void _checkIsStaggeredPrt();
  double _conditionAz(double az);
  double _conditionEl(double el);
  void _addPulseToQueue(const IwrfTsPulse *pulse);

};

#endif

