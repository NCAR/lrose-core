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
// Beam.hh
//
// Beam object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////

#ifndef Beam_hh
#define Beam_hh

#include <string>
#include <vector>
#include <cstdio>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/GateData.hh>
#include <toolsa/TaArray.hh>
#include "Params.hh"
#include "BeamMgr.hh"
using namespace std;

////////////////////////
// This class

class Beam {
  
public:

  // constructor
  
  Beam (const string &prog_name,
        const Params &params,
        bool isPpi,
        double meanPointingAngle,
        bool isAlternating,
        bool isStaggeredPrt,
        int nGates,
        int nGatesPrtLong,
        double prt,
        double prtLong,
        BeamMgr &beamMgr,
        const IwrfTsInfo &opsInfo,
        const deque<const IwrfTsPulse *> pulses);
  
  // destructor
  
  ~Beam();

  // load data into the manager
  
  void loadManager(BeamMgr &beamMgr);

  // set methods

  void setTargetEl(double el) { _targetEl = el; }
  
  // get methods

  int getNSamples() const { return _nSamples; }

  double getEl() const { return _el; }
  double getAz() const { return _az; }
  double getTargetEl() const { return _targetEl; }
  double getTargetAz() const { return _targetAz; }
  double getPrt() const { return _prt; }
  double getPulseWidth() const { return _pulseWidth; }

  double getMeasXmitPowerDbmH() const { return _measXmitPowerDbmH; }
  double getMeasXmitPowerDbmV() const { return _measXmitPowerDbmV; }

  time_t getTime() const { return _time; }
  double getDoubleTime() const { return _dtime; }

  int getScanMode() const;
  int getTiltNum() const { return _tiltNum; }
  int getVolNum() const { return _volNum; }

  bool getBeamIsIndexed() const { return _beamIsIndexed; }
  double getAngularResolution() const { return _angularResolution; }

  int getNGates() const { return _nGates; }

  const IwrfTsInfo &getOpsInfo() const { return _opsInfo; }

  const fl32* *getIqChan0() { return _iqChan0; }
  const fl32* *getIqChan1() { return _iqChan1; }

protected:
  
private:

  string _progName;
  const Params &_params;
  
  int _nSamples;
  int _nSamplesHalf;
  
  time_t _time;
  double _dtime;

  double _el;
  double _az;
  double _targetEl;
  double _targetAz;

  int _scanMode;
  int _tiltNum;
  int _volNum;
  bool _antennaTransition;

  bool _beamIsIndexed;
  double _angularResolution;

  // pulse mode

  bool _isAlternating;
  bool _isStaggeredPrt;

  // number of gates
  
  int _nGates;

  // prt and pulse width

  double _prt;
  double _pulseWidth;

  // staggered PRT mode

  double _prtShort;
  double _prtLong;
  int _nGatesPrtShort;
  int _nGatesPrtLong;

  // measured xmit power

  double _measXmitPowerDbmH;
  double _measXmitPowerDbmV;
  
  // operations information

  IwrfTsInfo _opsInfo;
  
  // channel data

  TaArray<const fl32 *> _iqChan0_;
  const fl32* *_iqChan0;

  bool _haveChan1;
  TaArray<const fl32 *> _iqChan1_;
  const fl32* *_iqChan1;

};

#endif

