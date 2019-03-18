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
///////////////////////////////////////////////////////////////
// Beam.cc
//
// Beam object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
//
// Beam object holds time series and beam data
//
////////////////////////////////////////////////////////////////

#include "Beam.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

Beam::Beam(const string &prog_name,
	   const Params &params,
           bool isPpi,
           double meanPointingAngle,
           bool isAlternating,
           bool isStaggeredPrt,
           int nGates,
           int nGatesPrtLong,
           double prt,
           double prtLong,
           bool beamIsIndexed,
           double indexedResolution,
           const IwrfTsInfo &opsInfo,
           const deque<const IwrfTsPulse *> pulses) :
        _progName(prog_name),
        _params(params),
        _isAlternating(isAlternating),
        _isStaggeredPrt(isStaggeredPrt),
        _nGates(nGates),
        _prt(prt),
        _prtShort(prt),
        _prtLong(prtLong),
        _nGatesPrtShort(nGates),
        _nGatesPrtLong(nGatesPrtLong),
        _opsInfo(opsInfo)

{

  _scanMode = IWRF_SCAN_MODE_NOT_SET;
  _iqChan0 = NULL;
  _iqChan1 = NULL;
  _haveChan1 = false;
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "---->> Pulse info BEFORE overrides <<----" << endl;
    _opsInfo.print(stderr);
    cerr << "---->> End of Pulse info BEFORE overrides <<----" << endl;
  }

  // override time-series values as needed
  
  if (_params.override_radar_name) {
    _opsInfo.overrideRadarName(_params.radar_name);
  }
  if (_params.override_radar_location) {
    _opsInfo.overrideRadarLocation(_params.radar_altitude_meters,
				   _params.radar_latitude_deg,
				   _params.radar_longitude_deg);
  }
  if (_params.override_gate_geometry) {
    _opsInfo.overrideGateGeometry(_params.start_range_meters,
                                  _params.gate_spacing_meters);
  }
  if (_params.override_radar_wavelength) {
    _opsInfo.overrideWavelength(_params.radar_wavelength_cm);
  }
    
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "---->> Pulse info AFTER overrides <<----" << endl;
    _opsInfo.print(stderr);
    cerr << "---->> End of Pulse info AFTER overrides <<----" << endl;
  }

  // initialize
  
  _nSamples = _params.n_samples;
  _nSamplesHalf = _nSamples / 2;
  const IwrfTsPulse *midPulse = pulses[_nSamplesHalf];

  // range geom

  _startRangeKm = midPulse->get_start_range_km();
  _gateSpacingKm = midPulse->get_gate_spacing_km();

  // pulse width

  _pulseWidth = midPulse->getPulseWidthUs();

  // transmitter power

  _measXmitPowerDbmH = midPulse->getMeasXmitPowerDbmH();
  _measXmitPowerDbmV = midPulse->getMeasXmitPowerDbmV();

  // set time
  
  _time = (time_t) midPulse->getTime();
  _timeDouble = midPulse->getFTime();
  _nanoSecs = midPulse->getNanoSecs();

  // set elevation
  
  _beamIsIndexed = beamIsIndexed;
  _indexedResolution = indexedResolution;

  if (isPpi) {
    _az = meanPointingAngle;
    _el = midPulse->getEl();
    if (_el > 180.0) {
      _el -= 360.0;
    }
  } else {
    _el = meanPointingAngle;
    _az = midPulse->getAz();
    if (_az < 0.0) {
      _az += 360.0;
    }
  }
  _targetEl = midPulse->getFixedEl();
  _targetAz = midPulse->getFixedAz();

  // scan details

  _scanMode = midPulse->get_scan_mode();
  _tiltNum = midPulse->get_sweep_num();
  _volNum = midPulse->get_volume_num();

  // set transition for beam if mid pulse is in transition

  _antennaTransition = false;
  if (midPulse->get_antenna_transition()) {
    _antennaTransition = true;
  }
  
  // set up data pointer arrays
  // channel 0

  _iqChan0 = _iqChan0_.alloc(_nSamples);
  for (int ii = 0; ii < _nSamples; ii++) {
    _iqChan0[ii] = pulses[ii]->getIq0();
  }
  
  // channel 1 - will be NULLs for single pol
  
  _haveChan1 = true;
  for (int ii = 0; ii < _nSamples; ii++) {
    if (pulses[ii]->getIq1() == NULL) {
      _haveChan1 = false;
      break;
    }
  }

  if (_haveChan1) {
    _iqChan1 = _iqChan1_.alloc(_nSamples);
    for (int ii = 0; ii < _nSamples; ii++) {
      _iqChan1[ii] = pulses[ii]->getIq1();
    }
  }

  // compute nyquist

  double wavelengthMeters = _opsInfo.get_radar_wavelength_cm() / 100.0;

  if (_isStaggeredPrt) {
    
    double prtRatio = _prtShort / _prtLong;
    int ratio60 = (int) (prtRatio * 60.0 + 0.5);
    if (ratio60 == 40) {
      // 2/3
      _stagM = 2;
      _stagN = 3;
    } else if (ratio60 == 45) {
      // 3/4
      _stagM = 3;
      _stagN = 4;
    } else if (ratio60 == 48) {
      // 4/5
      _stagM = 4;
      _stagN = 5;
    } else {
      // assume 2/3
      cerr << "WARNING - Iq2Dsr::Beam::_initStagPrt" << endl;
      cerr << "  No support for prtRatio: " << prtRatio << endl;
      cerr << "  Assuming 2/3 stagger" << endl;
      _stagM = 2;
      _stagN = 3;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "===>> staggered PRT, ratio: "
           << _stagM << "/" << _stagN << " <<===" << endl;
    }
    
    _nyquistPrtShort = ((wavelengthMeters / _prtShort) / 4.0);
    _nyquistPrtLong = ((wavelengthMeters / _prtLong) / 4.0);
    _nyquist = _nyquistPrtShort * _stagM;

  } else {

    _nyquist = ((wavelengthMeters / _prt) / 4.0);
    _nyquistPrtShort = _nyquist;
    _nyquistPrtLong = _nyquist;

    _stagM = 1;
    _stagN = 1;

    _unambigRange = IwrfCalib::LightSpeedMps * _prt / 2000.0;

  }

}

//////////////////////////////////////////////////////////////////
// destructor

Beam::~Beam()

{

}

////////////////////////////////////////////////
// get scan mode

int Beam::getScanMode() const

{

  if (_scanMode != IWRF_SCAN_MODE_NOT_SET) {
    return _scanMode;
  }
  int scanMode = _opsInfo.get_scan_mode();
  if (scanMode < 0) {
    scanMode = IWRF_SCAN_MODE_AZ_SUR_360;
  }
  return scanMode;

}

