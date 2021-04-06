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
// BeamMgr.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
//
// BeamMgr manages the beam data, printing etc
//
///////////////////////////////////////////////////////////////

#include <iomanip>
#include <cerrno>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include "BeamMgr.hh"
#include "Beam.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

BeamMgr::BeamMgr(const string &prog_name,
		 const Params &params) :
  _progName(prog_name),
  _params(params),
  _opsInfo()
  
{

  isOK = true;
  _prt = 0;
  _nSamples = _params.n_samples;
  _nSamplesHalf = _nSamples / 2;
  _xmitRcvMode = IWRF_SINGLE_POL;
  
  // initialize geometry to missing values
  
  _startRange = 0;
  _gateSpacing = 0;

}

//////////////////////////////////////////////////////////////////
// destructor

BeamMgr::~BeamMgr()

{

  _freeGateData();

}

////////////////////////////////////////////////////////////////////////
// read cal file
//
// Returns 0 on success, -1 on failure

int BeamMgr::readCalFromFile(const string &calPath)

{
  
  // Stat the file to get length
  
  struct stat calStat;
  if (ta_stat(calPath.c_str(), &calStat)) {
    int errNum = errno;
    cerr << "ERROR - BeamMgr::_readCalFromFile" << endl;
    cerr << "  Cannot stat file: " << calPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  size_t fileLen = calStat.st_size;

  // open file
  
  FILE *calFile;
  if ((calFile = fopen(calPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - BeamMgr::_readCalFromFile" << endl;
    cerr << "  Cannot open file: " << calPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // create buffer
  
  TaArray<char> bufArray;
  char *xmlBuf = bufArray.alloc(fileLen + 1);
  memset(xmlBuf, 0, fileLen + 1);
  
  // read in buffer, close file
  
  if (ta_fread(xmlBuf, 1, fileLen, calFile) != fileLen) {
    int errNum = errno;
    cerr << "ERROR - BeamMgr::_readCalFromFile" << endl;
    cerr << "  Cannot read file: " << calPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(calFile);
    return -1;
  }
  fclose(calFile);

  // set from XML buffer

  string errStr;
  if (_calib.setFromXml(xmlBuf, errStr)) {
    cerr << "ERROR - cannot decode cal file: " << calPath << endl;
    cerr << errStr;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////////
// Load beam data into manager.

void BeamMgr::loadBeamData(const Beam &beam)

{

  if (beam.getIsStagPrt()) {
    loadBeamDataStaggeredPrt(beam.getInfo(),
                             beam.getTimeSecs(),
                             beam.getTime().getTimeAsDouble(),
                             beam.getEl(),
                             beam.getAz(),
                             beam.getPrtShort(), 
                             beam.getPrtLong(),
                             beam.getNGatesPrtShort(), 
                             beam.getNGatesPrtLong(),
                             beam.getIqChan0(), 
                             beam.getIqChan1());
  } else {
    loadBeamData(beam.getInfo(),
                 beam.getTimeSecs(),
                 beam.getTime().getTimeAsDouble(),
                 beam.getEl(),
                 beam.getAz(),
                 beam.getPrt(),
                 beam.getNGates(),
                 beam.getIqChan0(),
                 beam.getIqChan1());
  }

}

/////////////////////////////////////////////////////////////////
// Load beam data into manager.
// Apply window as appropriate.
// How this is loaded depends on the polarization mode.
//
// Assumptions:
// 1. For alternating mode, first pulse in sequence is H.
// 2. For non-switching dual receivers,
//    H is channel 0 and V channel 1.
// 3. For single pol mode, data is in channel 0.

void BeamMgr::loadBeamData(const IwrfTsInfo &opsInfo,
			   time_t beamTime,
			   double doubleBeamTime,
			   double el, double az,
			   double prt,
			   int nGatesIn,
			   const fl32 **iqChan0,
			   const fl32 **iqChan1)

{

  _opsInfo = opsInfo;
  _beamTime = beamTime;
  _doubleBeamTime = doubleBeamTime;
  _el = el;
  _az = az;
  _prt = prt;
  _nGatesIn = nGatesIn;

  _xmitRcvMode = (iwrf_xmit_rcv_mode) _opsInfo.get_proc_xmit_rcv_mode();
  _isStaggeredPrt = false;
  _prtShort = prt;
  _prtLong = prt;
  _nGatesPrtShort = nGatesIn;
  _nGatesPrtLong = nGatesIn;
  _nGatesStaggeredPrt = MAX(_nGatesPrtShort, _nGatesPrtLong);

  // compute nyquist

  double wavelengthMeters = _opsInfo.get_radar_wavelength_cm() / 100.0;
  _nyquist = ((wavelengthMeters / _prt) / 4.0);

  // load iq data

  _allocGateData(nGatesIn);
  _initFieldData();
  _loadGateIq(nGatesIn, iqChan0, iqChan1);
  
  // range geometry
  
  _startRange = opsInfo.get_proc_start_range_km();
  _gateSpacing = opsInfo.get_proc_gate_spacing_km();

  // override cal data as appropriate

  if (_params.use_cal_from_time_series) {
    _opsInfo.setCalibration(_calib.getStruct());
  }

}
  
/////////////////////////////////////////////////////////////////
// Load beam data into manager for staggered PRT mode
// How this is loaded depends on the polarization mode.
//
// Assumptions:
// 1. First pulse is a short PRT pulse
// 2. For non-switching dual receivers,
//    H is channel 0 and V channel 1.
// 3. For single pol mode, data is in channel 0.

void BeamMgr::loadBeamDataStaggeredPrt(const IwrfTsInfo &opsInfo,
                                       time_t beamTime,
                                       double doubleBeamTime,
                                       double el, double az,
                                       double prtShort,
                                       double prtLong,
                                       int nGatesPrtShort,
                                       int nGatesPrtLong,
                                       const fl32 **iqChan0,
                                       const fl32 **iqChan1)

{

  _opsInfo = opsInfo;
  _beamTime = beamTime;
  _el = el;
  _az = az;

  _xmitRcvMode = (iwrf_xmit_rcv_mode) _opsInfo.get_proc_xmit_rcv_mode();
  _isStaggeredPrt = true;
  _prtShort = prtShort;
  _prtLong = prtLong;
  _nGatesPrtShort = nGatesPrtShort;
  _nGatesPrtLong = nGatesPrtLong;
  _nGatesStaggeredPrt = MAX(_nGatesPrtShort, _nGatesPrtLong);

  _prt = prtShort;
  _nGatesIn = _nGatesStaggeredPrt;

  // compute nyquist

  double wavelengthMeters = _opsInfo.get_radar_wavelength_cm() / 100.0;
  
  double prtRatio = _prtShort / _prtLong;
  int ratio60 = (int) (prtRatio * 60.0 + 0.5);
  if (ratio60 == 40) {
    // 2/3
    _staggeredM = 2;
    _staggeredN = 3;
  } else if (ratio60 == 45) {
    // 3/4
    _staggeredM = 3;
    _staggeredN = 4;
  } else if (ratio60 == 48) {
    // 4/5
    _staggeredM = 4;
    _staggeredN = 5;
  } else {
    // assume 2/3
    cerr << "WARNING - Iq2Dsr::MomentsMgr::loadBeamDataStaggeredPrt" << endl;
    cerr << "  No support for prtRatio: " << prtRatio << endl;
    cerr << "  Assuming 2/3 stagger" << endl;
    _staggeredM = 2;
    _staggeredN = 3;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "===>> staggered PRT, ratio: "
         << _staggeredM << "/" << _staggeredN << " <<===" << endl;
  }
  
  _nyquistPrtShort = ((wavelengthMeters / _prtShort) / 4.0);
  _nyquistPrtLong = ((wavelengthMeters / _prtLong) / 4.0);
  _nyquist = _nyquistPrtShort * _staggeredM;

  // load iq data

  _allocGateData(_nGatesStaggeredPrt);
  _initFieldData();
  _loadGateIqStaggeredPrt(iqChan0, iqChan1);

  // range geometry
  
  _startRange = opsInfo.get_proc_start_range_km();
  _gateSpacing = opsInfo.get_proc_gate_spacing_km();

}
  
/////////////////////////////////////////////////////////////////
// fill out spectra object

void BeamMgr::fillSpectraObj(RadarSpectra &spectra,
                             RadarSpectra::polarization_channel_t channel)
  
{

  spectra.clear();

  int nSamples = _nSamples;
  if (_xmitRcvMode == IWRF_ALT_HV_CO_ONLY ||
      _xmitRcvMode == IWRF_ALT_HV_CO_CROSS ||
      _xmitRcvMode == IWRF_ALT_HV_FIXED_HV) {
    nSamples = _nSamplesHalf;
  }

  spectra.setTimeSecs(_beamTime);
  spectra.setDoubleTime(_doubleBeamTime);
  spectra.setNSamples(nSamples);
  
  int startGate = 0;
  int endGate = _nGatesIn - 1;
  int nGates = endGate - startGate + 1;

  spectra.setNGates(nGates);
  spectra.setStartRange(_startRange + startGate * _gateSpacing);
  spectra.setGateSpacing(_gateSpacing);
  spectra.setElevDeg(_el);
  spectra.setAzDeg(_az);
  spectra.setWavelengthCm(_opsInfo.get_radar_wavelength_cm());
  spectra.setRadarName(_opsInfo.get_radar_name());
  string label = iwrf_xmit_rcv_mode_to_str(_opsInfo.get_proc_xmit_rcv_mode());
  spectra.setNotes(label);
  spectra.setChannel(channel);

  if (_isStaggeredPrt) {

    spectra.setStaggeredPrtMode(true);
    spectra.setPrtShort(_prtShort);
    spectra.setPrtLong(_prtLong);
    spectra.setNGatesPrtShort(_nGatesPrtShort);
    spectra.setNGatesPrtLong(_nGatesPrtLong);
    spectra.setStaggeredM(_staggeredM);
    spectra.setStaggeredN(_staggeredN);
    spectra.setNyquist(_nyquist);

  } else {

    if (nSamples == _nSamplesHalf) {
      spectra.setPrt(_prt * 2.0);
      spectra.setNyquist(_nyquist / 2.0);
    } else {
      spectra.setPrt(_prt);
      spectra.setNyquist(_nyquist);
    }

  }

  switch (channel) {
  case RadarSpectra::CHANNEL_VC:
    spectra.setNoiseDbm(_calib.getNoiseDbmVc());
    spectra.setReceiverGainDb(_calib.getReceiverGainDbVc());
    spectra.setBaseDbz1km(_calib.getBaseDbz1kmVc());
    break;
  case RadarSpectra::CHANNEL_HX:
    spectra.setNoiseDbm(_calib.getNoiseDbmHx());
    spectra.setReceiverGainDb(_calib.getReceiverGainDbHx());
    spectra.setBaseDbz1km(_calib.getBaseDbz1kmHx());
    break;
  case RadarSpectra::CHANNEL_VX:
    spectra.setNoiseDbm(_calib.getNoiseDbmVx());
    spectra.setReceiverGainDb(_calib.getReceiverGainDbVx());
    spectra.setBaseDbz1km(_calib.getBaseDbz1kmVx());
    break;
  case RadarSpectra::CHANNEL_HC:
  default:
    spectra.setNoiseDbm(_calib.getNoiseDbmHc());
    spectra.setReceiverGainDb(_calib.getReceiverGainDbHc());
    spectra.setBaseDbz1km(_calib.getBaseDbz1kmHc());
  } // switch

  for (int igate = startGate; igate <= endGate; igate++) {

    GateData *gate = _gateData[igate];
    const RadarComplex_t *iq = NULL;

    switch (channel) {
    case RadarSpectra::CHANNEL_VC:
      iq = gate->iqvc;
      break;
    case RadarSpectra::CHANNEL_HX:
      iq = gate->iqhx;
      break;
    case RadarSpectra::CHANNEL_VX:
      iq = gate->iqvx;
      break;
    case RadarSpectra::CHANNEL_HC:
    default:
      iq = gate->iqhc;
    } // switch

    // add spectra data for this gate
    
    vector<RadarSpectra::radar_iq_t> gateIq;
    for (int ii = 0; ii < nSamples; ii++) {
      RadarSpectra::radar_iq_t iqVal;
      iqVal.ival = iq[ii].re;
      iqVal.qval = iq[ii].im;
      gateIq.push_back(iqVal);
    }
    spectra.addGateIq(gateIq);

  } // igate

}
    
/////////////////////////////////////////////////////////////////
// Allocate or re-allocate gate data

void BeamMgr::_allocGateData(int nGates)

{
  int nNeeded = nGates - (int) _gateData.size();
  if (nNeeded > 0) {
    for (int ii = 0; ii < nNeeded; ii++) {
      GateData *gate = new GateData;
      gate->allocArrays(_nSamples, false, false, false);
      _gateData.push_back(gate);
    }
  }
}

/////////////////////////////////////////////////////////////////
// Free gate data

void BeamMgr::_freeGateData()

{
  for (int ii = 0; ii < (int) _gateData.size(); ii++) {
    delete _gateData[ii];
  }
  _gateData.clear();
}

/////////////////////////////////////////////////////////////////
// Initialize gate data

void BeamMgr::_initFieldData()

{
  for (int ii = 0; ii < (int) _gateData.size(); ii++) {
    _gateData[ii]->initFields();
  }
}

/////////////////////////////////////////////////////////////////
// Load gate IQ data.
// Apply window as appropriate.
// How this is loaded depends on the polarization mode.
//
// Assumptions:
// 1. For alternating mode, first pulse in sequence is H.
// 2. For non-switching dual receivers,
//    H is channel 0 and V channel 1.
// 3. For single pol mode, data is in channel 0.

void BeamMgr::_loadGateIq(int nGates,
                          const fl32 **iqChan0,
                          const fl32 **iqChan1)
  
{

  // check gate array allocations

  for (int igate = 0; igate < nGates; igate++) {
    _gateData[igate]->allocArrays(_nSamples, false, false, false);
  }
        
  switch (_xmitRcvMode) {
    
    case IWRF_ALT_HV_CO_ONLY: {

      // assumes first pulse is H xmit
      
      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvc = gate->iqvc;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhc++, iqvc++) {
          iqhc->re = iqChan0[jsamp][ipos];
          iqhc->im = iqChan0[jsamp][ipos + 1];
          jsamp++;
          iqvc->re = iqChan0[jsamp][ipos];
          iqvc->im = iqChan0[jsamp][ipos + 1];
          jsamp++;
        }
      }
    
    } break;
        
    case IWRF_ALT_HV_CO_CROSS: {

      // assumes first pulse is H xmit
      
      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvc = gate->iqvc;
        RadarComplex_t *iqhx = gate->iqhx;
        RadarComplex_t *iqvx = gate->iqvx;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhc++, iqvc++, iqhx++, iqvx++) {
          iqhc->re = iqChan0[jsamp][ipos];
          iqhc->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqvx->re = iqChan1[jsamp][ipos];
            iqvx->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
          iqvc->re = iqChan0[jsamp][ipos];
          iqvc->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqhx->re = iqChan1[jsamp][ipos];
            iqhx->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
        }
      }

    } break;

    case IWRF_ALT_HV_FIXED_HV: {

      // not switching
      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvc = gate->iqvc;
        RadarComplex_t *iqhx = gate->iqhx;
        RadarComplex_t *iqvx = gate->iqvx;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhc++, iqvc++, iqhx++, iqvx++) {
          iqhc->re = iqChan0[jsamp][ipos];
          iqhc->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqhx->re = iqChan1[jsamp][ipos];
            iqhx->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
          iqvx->re = iqChan0[jsamp][ipos];
          iqvx->im = iqChan0[jsamp][ipos + 1];
          if (iqChan1) {
            iqvc->re = iqChan1[jsamp][ipos];
            iqvc->im = iqChan1[jsamp][ipos + 1];
          }
          jsamp++;
        }
      }

    } break;

    case IWRF_SIM_HV_FIXED_HV: {

      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvc = gate->iqvc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvc++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
          if (iqChan1) {
            iqvc->re = iqChan1[isamp][ipos];
            iqvc->im = iqChan1[isamp][ipos + 1];
          }
        }
      }
      
    } break;

    case IWRF_SIM_HV_SWITCHED_HV: {

      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvc = gate->iqvc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvc++) {
          if (isamp % 2 == 0) {
            iqhc->re = iqChan0[isamp][ipos];
            iqhc->im = iqChan0[isamp][ipos + 1];
            if (iqChan1) {
              iqvc->re = iqChan1[isamp][ipos];
              iqvc->im = iqChan1[isamp][ipos + 1];
            }
          } else {
            iqvc->re = iqChan0[isamp][ipos];
            iqvc->im = iqChan0[isamp][ipos + 1];
            if (iqChan1) {
              iqhc->re = iqChan1[isamp][ipos];
              iqhc->im = iqChan1[isamp][ipos + 1];
            }
          }
        }
      }
      
    } break;

    case IWRF_H_ONLY_FIXED_HV: {

      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvx = gate->iqvx;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvx++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
          if (iqChan1) {
            iqvx->re = iqChan1[isamp][ipos];
            iqvx->im = iqChan1[isamp][ipos + 1];
          }
        }
      }
    
    } break;
    
    case IWRF_V_ONLY_FIXED_HV: {

      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhx = gate->iqhx;
        RadarComplex_t *iqvc = gate->iqvc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhx++, iqvc++) {
          iqhx->re = iqChan0[isamp][ipos];
          iqhx->im = iqChan0[isamp][ipos + 1];
          if (iqChan1) {
            iqvc->re = iqChan1[isamp][ipos];
            iqvc->im = iqChan1[isamp][ipos + 1];
          }
        }
      }
    
    } break;

    case IWRF_SINGLE_POL:
    default: {
    
      for (int igate = 0, ipos = 0; igate < nGates; igate++, ipos += 2) {
        GateData *gate = _gateData[igate];
        RadarComplex_t *iqhc = gate->iqhc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos + 1];
        }
      }

    } break;
    
  } // switch;

}

/////////////////////////////////////////////////////////////////
// Load gate IQ data for staggered PRT data
// How this is loaded depends on the polarization mode.
//
// Assumptions:
// 1. First pulse in sequence is short PRT.
// 2. For non-switching dual receivers,
//    H is channel 0 and V channel 1.
// 3. For single pol mode, data is in channel 0.

void BeamMgr::_loadGateIqStaggeredPrt(const fl32 **iqChan0,
                                      const fl32 **iqChan1)
  
{

  // check gate array allocations

  for (int igate = 0; igate < _nGatesStaggeredPrt; igate++) {
    _gateData[igate]->allocArrays(_nSamples, false, false, false);
  }
        
  switch (_xmitRcvMode) {
    
    case IWRF_SIM_HV_FIXED_HV: {
      
      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort;
           igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvc = gate->iqvc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvc++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos+1];
          iqvc->re = iqChan1[isamp][ipos];
          iqvc->im = iqChan1[isamp][ipos+1];
        }
       
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhcShort = gate->iqhcPrtShort;
        RadarComplex_t *iqvcShort = gate->iqvcPrtShort;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcShort++, iqvcShort++) {
          iqhcShort->re = iqChan0[jsamp][ipos];
          iqhcShort->im = iqChan0[jsamp][ipos+1];
          iqvcShort->re = iqChan1[jsamp][ipos];
          iqvcShort->im = iqChan1[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        
      } // igate < _ngatesPrtShort

      // long prt data

      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhcLong = gate->iqhcPrtLong;
        RadarComplex_t *iqvcLong = gate->iqvcPrtLong;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcLong++, iqvcLong++) {
          jsamp++;
          iqhcLong->re = iqChan0[jsamp][ipos];
          iqhcLong->im = iqChan0[jsamp][ipos+1];
          iqvcLong->re = iqChan1[jsamp][ipos];
          iqvcLong->im = iqChan1[jsamp][ipos+1];
          jsamp++;
        }
        
      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_SIM_HV_SWITCHED_HV: {
      
      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvc = gate->iqvc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvc++) {
          if (isamp % 2 == 0) {
            iqhc->re = iqChan0[isamp][ipos];
            iqhc->im = iqChan0[isamp][ipos+1];
            if (iqChan1) {
              iqvc->re = iqChan1[isamp][ipos];
              iqvc->im = iqChan1[isamp][ipos+1];
            }
          } else {
            iqvc->re = iqChan0[isamp][ipos];
            iqvc->im = iqChan0[isamp][ipos+1];
            if (iqChan1) {
              iqhc->re = iqChan1[isamp][ipos];
              iqhc->im = iqChan1[isamp][ipos+1];
            }
          }
        }
        
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhcShort = gate->iqhcPrtShort;
        RadarComplex_t *iqvcShort = gate->iqvcPrtShort;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcShort++, iqvcShort++) {
          if (isamp % 2 == 0) {
            iqhcShort->re = iqChan0[jsamp][ipos];
            iqhcShort->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqvcShort->re = iqChan1[jsamp][ipos];
              iqvcShort->im = iqChan1[jsamp][ipos+1];
            }
          } else {
            iqvcShort->re = iqChan0[jsamp][ipos];
            iqvcShort->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqhcShort->re = iqChan1[jsamp][ipos];
              iqhcShort->im = iqChan1[jsamp][ipos+1];
            }
          }
          jsamp++;
          jsamp++;
        }
        
      } // igate < _ngatesPrtShort

      // long prt data

      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhcLong = gate->iqhcPrtLong;
        RadarComplex_t *iqvcLong = gate->iqvcPrtLong;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcLong++, iqvcLong++) {
          jsamp++;
          if (isamp % 2 == 0) {
            iqhcLong->re = iqChan0[jsamp][ipos];
            iqhcLong->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqvcLong->re = iqChan1[jsamp][ipos];
              iqvcLong->im = iqChan1[jsamp][ipos+1];
            }
          } else {
            iqvcLong->re = iqChan0[jsamp][ipos];
            iqvcLong->im = iqChan0[jsamp][ipos+1];
            if (iqChan1) {
              iqhcLong->re = iqChan1[jsamp][ipos];
              iqhcLong->im = iqChan1[jsamp][ipos+1];
            }
          }
          jsamp++;
        }
        
      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_H_ONLY_FIXED_HV: {
      
      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhc = gate->iqhc;
        RadarComplex_t *iqvx = gate->iqvx;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++, iqvx++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos+1];
          iqvx->re = iqChan1[isamp][ipos];
          iqvx->im = iqChan1[isamp][ipos+1];
        }
        
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhcShort = gate->iqhcPrtShort;
        RadarComplex_t *iqvxShort = gate->iqvxPrtShort;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcShort++, iqvxShort++) {
          iqhcShort->re = iqChan0[jsamp][ipos];
          iqhcShort->im = iqChan0[jsamp][ipos+1];
          iqvxShort->re = iqChan1[jsamp][ipos];
          iqvxShort->im = iqChan1[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        
      } // igate < _ngatesPrtShort

      // long prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhcLong = gate->iqhcPrtLong;
        RadarComplex_t *iqvxLong = gate->iqvxPrtLong;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhcLong++, iqvxLong++) {
          jsamp++;
          iqhcLong->re = iqChan0[jsamp][ipos];
          iqhcLong->im = iqChan0[jsamp][ipos+1];
          iqvxLong->re = iqChan1[jsamp][ipos];
          iqvxLong->im = iqChan1[jsamp][ipos+1];
          jsamp++;
        }
        
      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_V_ONLY_FIXED_HV: {
      
      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhx = gate->iqhx;
        RadarComplex_t *iqvc = gate->iqvc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhx++, iqvc++) {
          iqhx->re = iqChan0[isamp][ipos];
          iqhx->im = iqChan0[isamp][ipos+1];
          iqvc->re = iqChan1[isamp][ipos];
          iqvc->im = iqChan1[isamp][ipos+1];
        }
        
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhxShort = gate->iqhxPrtShort;
        RadarComplex_t *iqvcShort = gate->iqvcPrtShort;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhxShort++, iqvcShort++) {
          iqhxShort->re = iqChan0[jsamp][ipos];
          iqhxShort->im = iqChan0[jsamp][ipos+1];
          iqvcShort->re = iqChan1[jsamp][ipos];
          iqvcShort->im = iqChan1[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        
      } // igate < _ngatesPrtShort

      // long prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhxLong = gate->iqhxPrtLong;
        RadarComplex_t *iqvcLong = gate->iqvcPrtLong;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf;
             isamp++, iqhxLong++, iqvcLong++) {
          jsamp++;
          iqhxLong->re = iqChan0[jsamp][ipos];
          iqhxLong->im = iqChan0[jsamp][ipos+1];
          iqvcLong->re = iqChan1[jsamp][ipos];
          iqvcLong->im = iqChan1[jsamp][ipos+1];
          jsamp++;
        }
        
      } // igate < _ngatesPrtLong

    } break;
    
    case IWRF_SINGLE_POL:
    default: {

      // short prt data
      
      for (int igate = 0, ipos = 0; igate < _nGatesPrtShort; igate++, ipos += 2) {
        
        GateData *gate = _gateData[igate];
        
        // original data - full series
        
        RadarComplex_t *iqhc = gate->iqhc;
        for (int isamp = 0; isamp < _nSamples; isamp++, iqhc++) {
          iqhc->re = iqChan0[isamp][ipos];
          iqhc->im = iqChan0[isamp][ipos+1];
        }
        
        // short PRT from in sequence, starting with short
        
        RadarComplex_t *iqhcShort = gate->iqhcPrtShort;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf; isamp++, iqhcShort++) {
          iqhcShort->re = iqChan0[jsamp][ipos];
          iqhcShort->im = iqChan0[jsamp][ipos+1];
          jsamp++;
          jsamp++;
        }
        
      } // igate < _ngatesPrtShort

      // long prt data

      for (int igate = 0, ipos = 0; igate < _nGatesPrtLong; igate++, ipos += 2) {

        GateData *gate = _gateData[igate];

        // long PRT from sequence - starts with short

        RadarComplex_t *iqhcLong = gate->iqhcPrtLong;
        for (int isamp = 0, jsamp = 0; isamp < _nSamplesHalf; isamp++, iqhcLong++) {
          jsamp++;
          iqhcLong->re = iqChan0[jsamp][ipos];
          iqhcLong->im = iqChan0[jsamp][ipos+1];
          jsamp++;
        }
        
      } // igate < _ngatesPrtLong

    } break; // SP
    
  } // switch;

  
}
