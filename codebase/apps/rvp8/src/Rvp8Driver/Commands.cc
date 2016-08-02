/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// Commands.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2006
//
///////////////////////////////////////////////////////////////
//
// Current configuration of RVP8.
// Keeps rvp8 configuration state, provides services.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <sigtypes.h>
#include <dsp_lib.h>
#include <rvp8_rap/TaXml.hh>
#include "Commands.hh"
#include "DspDriver.hh"
using namespace std;

////////////////////////////////////////////////////////////
// Constructor for read-write access

Commands::Commands()
  
{

  _debug = false;

  // initialize

  _clearFieldMask();
  DspMaskSet(&_fieldMask, DB_DBZ2);
  DspMaskSet(&_fieldMask, DB_VEL2);
  DspMaskSet(&_fieldMask, DB_WIDTH2);
  
  _procMode =  DSP_PROC_TIME_SERIES;

  _nSamples = 64;
  _majorMode = PMODE_PPP;
  _procFlags = OPF_RNV;
  _prfMode = PRF_FIXED;
  _windowType = WIN_VONHANN;
  _rangeSmooth = 0;
  
  _nGates = 1000;
  _gateSpacing = 0.250;

  _prf = 500;
  
  _phaseCoding = PHSEQ_FIXED;

  _el = 0.0;
  _az = 0.0;
  
  _firstPass = true;

}

// destructor

Commands::~Commands()
  
{

}

////////////////////////////////////////////
// set the configuration from an XML buffer

void Commands::set(const string &xmlBuf)
  
{
  
  if (_verbose) {
    cerr << "Setting commands from XML: " << endl;
    cerr << xmlBuf << endl;
  }
  
  // initialize change flags to false

  if (_firstPass) {
    setChangeFlags(true);
    _firstPass = false;
  } else {
    setChangeFlags(false);
  }

  // proc settings

  vector<string> fieldNames;
  if (TaXml::readStringArray(xmlBuf, "field", fieldNames) == 0) {
    if (fieldNames.size() > 0) {
      _clearFieldMask();
    }
    for (int ii = 0; ii < (int) fieldNames.size(); ii++) {
      if (fieldNames[ii] == "dbt") {
        DspMaskSet(&_fieldMask, DB_DBT2);
      } else if (fieldNames[ii] == "dbz") {
        DspMaskSet(&_fieldMask, DB_DBZ2);
      } else if (fieldNames[ii] == "dbzc") {
        DspMaskSet(&_fieldMask, DB_DBZC2);
      } else if (fieldNames[ii] == "flags") {
        DspMaskSet(&_fieldMask, DB_FLAGS2);
      } else if (fieldNames[ii] == "kdp") {
        DspMaskSet(&_fieldMask, DB_KDP2);
      } else if (fieldNames[ii] == "ldrh") {
        DspMaskSet(&_fieldMask, DB_LDRH2);
      } else if (fieldNames[ii] == "ldrv") {
        DspMaskSet(&_fieldMask, DB_LDRV2);
      } else if (fieldNames[ii] == "phidp") {
        DspMaskSet(&_fieldMask, DB_PHIDP2);
      } else if (fieldNames[ii] == "phih") {
        DspMaskSet(&_fieldMask, DB_PHIH2);
      } else if (fieldNames[ii] == "phiv") {
        DspMaskSet(&_fieldMask, DB_PHIV2);
      } else if (fieldNames[ii] == "rhoh") {
        DspMaskSet(&_fieldMask, DB_RHOH2);
      } else if (fieldNames[ii] == "rhohv") {
        DspMaskSet(&_fieldMask, DB_RHOHV2);
      } else if (fieldNames[ii] == "rhov") {
        DspMaskSet(&_fieldMask, DB_RHOV2);
      } else if (fieldNames[ii] == "sqi") {
        DspMaskSet(&_fieldMask, DB_SQI2);
      } else if (fieldNames[ii] == "vel") {
        DspMaskSet(&_fieldMask, DB_VEL2);
      } else if (fieldNames[ii] == "velc") {
        DspMaskSet(&_fieldMask, DB_VELC2);
      } else if (fieldNames[ii] == "width") {
        DspMaskSet(&_fieldMask, DB_WIDTH2);
      } else if (fieldNames[ii] == "zdr") {
        DspMaskSet(&_fieldMask, DB_ZDR2);
      } else {
        cerr << "Commands::set()" << endl;
        cerr << "  Unknown field: " << fieldNames[ii]
             << " - ignoring" << endl;
      }
    }
  }

  string procModeStr;
  if (TaXml::readString(xmlBuf, "procMode", procModeStr) == 0) {
    if (procModeStr == "sync") {
      _procMode =  DSP_PROC_SYNCHRONOUS;
    } else if (procModeStr == "free") {
      _procMode =  DSP_PROC_FREE_RUNNING;
    } else if (procModeStr == "time_series") {
      _procMode =  DSP_PROC_TIME_SERIES;
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown proc mode: " << procModeStr << " - ignoring" << endl;
    }
  }

  // options settings

  int nSamples;
  if (TaXml::readInt(xmlBuf, "nSamples", nSamples) == 0) {
    if (nSamples != _nSamples) {
      _nSamples = nSamples;
      _optionsChanged = true;
    }
  }

  string majorModeStr;
  if (TaXml::readString(xmlBuf, "majorMode", majorModeStr) == 0) {
    if (majorModeStr == "pulse-pair") {
      if (_majorMode != PMODE_PPP) {
        _majorMode =  PMODE_PPP;
        _optionsChanged = true;
      }
    } else if (majorModeStr == "fft") {
      if (_majorMode != PMODE_FFT) {
        _majorMode =  PMODE_FFT;
        _optionsChanged = true;
      }
    } else if (majorModeStr == "phase-coded") {
      if (_majorMode != PMODE_RPH) {
        _majorMode =  PMODE_RPH;
        _optionsChanged = true;
      }
    } else if (majorModeStr == "staggered-prt") {
      if (_majorMode != PMODE_DPT2) {
        _majorMode =  PMODE_DPT2;
        _optionsChanged = true;
      }
    } else if (majorModeStr == "user1") {
      if (_majorMode != PMODE_USER1) {
        _majorMode =  PMODE_USER1;
        _optionsChanged = true;
      }
    } else if (majorModeStr == "user2") {
      if (_majorMode != PMODE_USER2) {
        _majorMode =  PMODE_USER2;
        _optionsChanged = true;
      }
    } else if (majorModeStr == "user3") {
      if (_majorMode != PMODE_USER3) {
        _majorMode =  PMODE_USER3;
        _optionsChanged = true;
      }
    } else if (majorModeStr == "user4") {
      if (_majorMode != PMODE_USER4) {
        _majorMode =  PMODE_USER4;
        _optionsChanged = true;
      }
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown major mode: "
           << majorModeStr << " - ignoring" << endl;
    }
  }

  string speckleFiltStr;
  int procFlags = OPF_RNV;
  if (TaXml::readString(xmlBuf, "speckleFilter", speckleFiltStr) == 0) {
    if (speckleFiltStr == "dbz") {
      procFlags |= OPF_LSR;
    } else if (speckleFiltStr == "vel") {
      procFlags |= OPF_DSR;
    } else if (speckleFiltStr == "both") {
      procFlags |= OPF_LSR;
      procFlags |= OPF_DSR;
    } else if (speckleFiltStr == "off") {
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown speckleFilter setting: "
           << speckleFiltStr << " - ignoring" << endl;
    }
  }

  if (procFlags != _procFlags) {
    _procFlags = procFlags;
    _optionsChanged = true;
  }
  
  string prfModeStr;
  if (TaXml::readString(xmlBuf, "prfMode", prfModeStr) == 0) {
    if (prfModeStr == "fixed") {
      if (_prfMode != PRF_FIXED) {
        _prfMode =  PRF_FIXED;
        _optionsChanged = true;
      }
    } else if (prfModeStr == "2_3") {
      if (_prfMode != PRF_2_3) {
        _prfMode =  PRF_2_3;
        _optionsChanged = true;
      }
    } else if (prfModeStr == "3_4") {
      if (_prfMode != PRF_3_4) {
        _prfMode =  PRF_3_4;
        _optionsChanged = true;
      }
    } else if (prfModeStr == "4_5") {
      if (_prfMode != PRF_4_5) {
        _prfMode =  PRF_4_5;
        _optionsChanged = true;
      }
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown prf mode: " << prfModeStr << " - ignoring" << endl;
    }
  }

  string windowTypeStr;
  if (TaXml::readString(xmlBuf, "windowType", windowTypeStr) == 0) {
    if (windowTypeStr == "rect") {
      if (_windowType != WIN_RECT) {
        _windowType =  WIN_RECT;
        _optionsChanged = true;
      }
    } else if (windowTypeStr == "hamming") {
      if (_windowType != WIN_HAMMING) {
        _windowType =  WIN_HAMMING;
        _optionsChanged = true;
      }
    } else if (windowTypeStr == "blackman") {
      if (_windowType != WIN_BLACKMAN) {
        _windowType =  WIN_BLACKMAN;
        _optionsChanged = true;
      }
    } else if (windowTypeStr == "blkmanex") {
      if (_windowType != WIN_BLKMANEX) {
        _windowType =  WIN_BLKMANEX;
        _optionsChanged = true;
      }
    } else if (windowTypeStr == "vonhann") {
      if (_windowType != WIN_VONHANN) {
        _windowType =  WIN_VONHANN;
        _optionsChanged = true;
      }
    } else if (windowTypeStr == "adaptive") {
      if (_windowType != WIN_ADAPTIVE) {
        _windowType =  WIN_ADAPTIVE;
        _optionsChanged = true;
      }
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown prf mode: " << windowTypeStr << " - ignoring" << endl;
    }
  }
  
  int rangeSmooth;
  if (TaXml::readInt(xmlBuf, "rangeSmoothing", rangeSmooth) == 0) {
    if (rangeSmooth != _rangeSmooth) {
      _rangeSmooth = rangeSmooth;
      _optionsChanged = true;
    }
  }

  // range mask

  int nGates;
  if (TaXml::readInt(xmlBuf, "nGates", nGates) == 0) {
    if (nGates != _nGates) {
      _nGates = nGates;
      _rangeMaskChanged = true;
    }
  }

  double gateSpacing;
  if (TaXml::readDouble(xmlBuf, "gateSpacingKm", gateSpacing) == 0) {
    if (gateSpacing != _gateSpacing) {
      _gateSpacing = gateSpacing;
      _rangeMaskChanged = true;
    }
  }

  // prf

  double prf;
  if (TaXml::readDouble(xmlBuf, "prf", prf) == 0) {
    if (prf != _prf) {
      _prf = prf;
      _prfChanged = true;
    }
  }

  // phase sequence
  
  string phaseCodingStr;
  if (TaXml::readString(xmlBuf, "phaseCoding", phaseCodingStr) == 0) {
    if (phaseCodingStr == "fixed") {
      if (_phaseCoding != PHSEQ_FIXED) {
        _phaseCoding = PHSEQ_FIXED;
        _phaseCodingChanged = true;
      }
    } else if (phaseCodingStr == "random") {
      if (_phaseCoding != PHSEQ_RANDOM) {
        _phaseCoding = PHSEQ_RANDOM;
        _phaseCodingChanged = true;
      }
    } else if (phaseCodingStr == "custom") {
      if (_phaseCoding != PHSEQ_CUSTOM) {
        _phaseCoding = PHSEQ_CUSTOM;
        _phaseCodingChanged = true;
      }
    } else if (phaseCodingStr == "sz8_64") {
      if (_phaseCoding != PHSEQ_SZ8_64) {
        _phaseCoding = PHSEQ_SZ8_64;
        _phaseCodingChanged = true;
      }
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown phaseCoding: "
           << phaseCodingStr << " - ignoring" << endl;
    }
  }

  // clutter filter number

  int filtNum;
  if (TaXml::readInt(xmlBuf, "clutterFilterNumber", filtNum) == 0) {
    if (filtNum < 0 || filtNum > 7) {
      cerr << "Commands::set()" << endl;
      cerr << "  Clutter filter number out of range: " << filtNum << endl;
      cerr << "    - ignoring" << endl;
    } else {
      if (filtNum != _clutFiltNum) {
        _clutFiltNum = filtNum;
        _clutFiltNumChanged = true;
      }
    }
  }

  // polarization

  string polarStr;
  if (TaXml::readString(xmlBuf, "polarization", polarStr) == 0) {
    if (polarStr == "horizontal") {
      if (_polarization != POL_HORIZ_FIX) {
        _polarization = POL_HORIZ_FIX;
        _polarizationChanged = true;
      }
    } else if (polarStr == "vertical") {
      if (_polarization != POL_VERT_FIX) {
        _polarization = POL_VERT_FIX;
        _polarizationChanged = true;
      }
    } else if (polarStr == "alternating") {
      if (_polarization != POL_ALTERNATING) {
        _polarization = POL_ALTERNATING;
        _polarizationChanged = true;
      }
    } else if (polarStr == "simultaneous") {
      if (_polarization != POL_SIMULTANEOUS) {
        _polarization = POL_SIMULTANEOUS;
        _polarizationChanged = true;
      }
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown polarization: "
           << polarStr << " - ignoring" << endl;
    }
  }

  // angle synchronization mode

  string angSyncStr;
  if (TaXml::readString(xmlBuf, "angSyncMode", angSyncStr) == 0) {
    if (angSyncStr == "none") {
      if (_angSyncMode != ANGSYN_NONE) {
        _angSyncMode = ANGSYN_NONE;
        _angSyncModeChanged = true;
      }
    } else if (angSyncStr == "dynamic") {
      if (_angSyncMode != ANGSYN_DYNAMIC) {
        _angSyncMode = ANGSYN_DYNAMIC;
        _angSyncModeChanged = true;
      }
    } else {
      cerr << "Commands::set()" << endl;
      cerr << "  Unknown angSyncMode: "
           << angSyncStr << " - ignoring" << endl;
    }
  }

}

////////////////////////
// set all change flags

void Commands::setChangeFlags(bool state)

{

  _optionsChanged = state;
  _rangeMaskChanged = state;
  _prfChanged = state;
  _phaseCodingChanged = state;
  _clutFiltNumChanged = state;
  _polarizationChanged = state;
  _angSyncModeChanged = state;

}

///////////////////////////////////////////////
// configure the DSP based on the command state

int Commands::configureDsp(DspDriver *dspDriver)
  
{

  if (dspDriver == NULL) {
    return 0;
  }

  // reset the FIFO

  if (dspDriver->resetFifo()) {
    cerr << "ERROR - Commands::configureDsp" << endl;
    return -1;
  }

  if (_rangeMaskChanged) {
    if (_debug) {
      cerr << "Range mask has changed:" << endl;
      cerr << "  Setting nGates: " << _nGates << endl;
      cerr << "  Setting gateSpacing: " << _gateSpacing << endl;
    }
    if (dspDriver->setRangeMask(_nGates, _gateSpacing)) {
      cerr << "ERROR - Commands::configureDsp" << endl;
      return -1;
    }
    _rangeMaskChanged = false;
  }

  if (_optionsChanged) {
    if (_debug) {
      cerr << "Options have changed:" << endl;
      cerr << "  Setting nSamples: " << _nSamples << endl;
      cerr << "  Setting majorMode: " << majorMode2String(_majorMode) << endl;
      cerr << "  Setting procFlags: " << procFlags2SpeckleFiltString(_procFlags) << endl;
      cerr << "  Setting prfMode: " << prfMode2String(_prfMode) << endl;
      cerr << "  Setting windowType: " << windowType2String(_windowType) << endl;
      cerr << "  Setting rangeSmooth: " << _rangeSmooth << endl;
    }
    if (dspDriver->setOptions(_nSamples, _majorMode, _procFlags,
                              _prfMode, _windowType, _rangeSmooth)) {
      cerr << "ERROR - Commands::configureDsp" << endl;
      return -1;
    }
    _optionsChanged = false;
  }

  if (_prfChanged) {
    if (_debug) {
      cerr << "PRF has changed:" << endl;
      cerr << "  Setting prf: " << _prf << endl;
    }
    if (dspDriver->setPrf(_prf)) {
      cerr << "ERROR - Commands::configureDsp" << endl;
      return -1;
    }
    _prfChanged = false;
  }

  if (_phaseCodingChanged) {
    if (_debug) {
      cerr << "Phase sequence has changed:" << endl;
      cerr << "  Setting phase sequence: " << phaseCoding2String(_phaseCoding) << endl;
    }
    if (dspDriver->setPhaseCoding(_phaseCoding)) {
      cerr << "ERROR - Commands::configureDsp" << endl;
      return -1;
    }
    _phaseCodingChanged = false;
  }

  if (_polarizationChanged) {
    if (_debug) {
      cerr << "Polarization has changed:" << endl;
      cerr << "  Setting polarization: " << polarization2String(_polarization) << endl;
    }
    if (dspDriver->setPolarization(_polarization)) {
      cerr << "ERROR - Commands::configureDsp" << endl;
      return -1;
    }
    _polarizationChanged = false;
  }

  if (_clutFiltNumChanged) {
    if (_debug) {
      cerr << "Clutter filter has changed:" << endl;
      cerr << "  Setting clutFiltNum: " << _clutFiltNum << endl;
    }
    if (dspDriver->setClutFiltNum(_clutFiltNum)) {
      cerr << "ERROR - Commands::configureDsp" << endl;
      return -1;
    }
    _clutFiltNumChanged = false;
  }

  if (_angSyncModeChanged) {
    if (_debug) {
      cerr << "Angle sync mode has changed:" << endl;
      cerr << "  Setting angle sync mode: "
           << angSyncMode2String(_angSyncMode) << endl;
    }
    if (dspDriver->setAngSyncMode(_angSyncMode)) {
      cerr << "ERROR - Commands::configureDsp" << endl;
      return -1;
    }
    _angSyncModeChanged = false;
  }

  if (_debug) {
    cerr << "Issuing proc command:" << endl;
    cerr << "  fields: " << fieldMask2String(_fieldMask) << endl;
    cerr << "  procMode: " << procMode2String(_procMode) << endl;
  }
  if (dspDriver->setProc(_fieldMask, _procMode)) {
    cerr << "ERROR - Commands::configureDsp" << endl;
    return -1;
  }

  return 0;
  
}

//////////////////////////////////////
// print

void Commands::print(ostream &out)

{

  out << "  nSamples: " << _nSamples << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  gateSpacingKm: " << _gateSpacing << endl;
  out << "  majorMode: " << majorMode2String(_majorMode) << endl;
  out << "  speckleFilter: "
      << procFlags2SpeckleFiltString(_procFlags) << endl;
  out << "  prfMode: " << prfMode2String(_prfMode) << endl;
  out << "  prf: " << _prf << endl;
  out << "  windowType: " << windowType2String(_windowType) << endl;
  out << "  rangeSmoothing: " << _rangeSmooth << endl;
  out << "  procMode: " << procMode2String(_procMode) << endl;
  out << "  phaseCoding: " << phaseCoding2String(_phaseCoding) << endl;
  out << "  clutFiltNum: " << _clutFiltNum << endl;
  out << "  polarization: " << polarization2String(_polarization) << endl;

  out << "  fields:" << endl;
  if (lDspMaskTest(&_fieldMask, DB_DBT2)) {
    out << "    dbt" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_DBZ2)) {
    out << "    dbz" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_DBZC2)) {
    out << "    dbzc" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_FLAGS2)) {
    out << "    flags" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_KDP2)) {
    out << "    kdp" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_LDRH2)) {
    out << "    ldrh" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_LDRV2)) {
    out << "    ldrv" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_PHIDP2)) {
    out << "    phidp" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_PHIH2)) {
    out << "    phih" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_PHIV2)) {
    out << "    phiv" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_RHOH2)) {
    out << "    rhoh" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_RHOHV2)) {
    out << "    rhohv" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_RHOV2)) {
    out << "    rhov" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_SQI2)) {
    out << "    sqi" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_VEL2)) {
    out << "    vel" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_VELC2)) {
    out << "    velc" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_WIDTH2)) {
    out << "    width" << endl;
  }
  if (lDspMaskTest(&_fieldMask, DB_ZDR2)) {
    out << "    zdr" << endl;
  }

}

//////////////////////////////////////
// convert enums to strings

string Commands::fieldMask2String(const struct dsp_data_mask &fieldMask)

{

  string str;

  if (lDspMaskTest(&fieldMask, DB_DBT2)) {
    str += " dbt";
  }
  if (lDspMaskTest(&fieldMask, DB_DBZ2)) {
    str += " dbz";
  }
  if (lDspMaskTest(&fieldMask, DB_DBZC2)) {
    str += " dbzc";
  }
  if (lDspMaskTest(&fieldMask, DB_FLAGS2)) {
    str += " flags";
  }
  if (lDspMaskTest(&fieldMask, DB_KDP2)) {
    str += " kdp";
  }
  if (lDspMaskTest(&fieldMask, DB_LDRH2)) {
    str += " ldrh";
  }
  if (lDspMaskTest(&fieldMask, DB_LDRV2)) {
    str += " ldrv";
  }
  if (lDspMaskTest(&fieldMask, DB_PHIDP2)) {
    str += " phidp";
  }
  if (lDspMaskTest(&fieldMask, DB_PHIH2)) {
    str += " phih";
  }
  if (lDspMaskTest(&fieldMask, DB_PHIV2)) {
    str += " phiv";
  }
  if (lDspMaskTest(&fieldMask, DB_RHOH2)) {
    str += " rhoh";
  }
  if (lDspMaskTest(&fieldMask, DB_RHOHV2)) {
    str += " rhohv";
  }
  if (lDspMaskTest(&fieldMask, DB_RHOV2)) {
    str += " rhov";
  }
  if (lDspMaskTest(&fieldMask, DB_SQI2)) {
    str += " sqi";
  }
  if (lDspMaskTest(&fieldMask, DB_VEL2)) {
    str += " vel";
  }
  if (lDspMaskTest(&fieldMask, DB_VELC2)) {
    str += " velc";
  }
  if (lDspMaskTest(&fieldMask, DB_WIDTH2)) {
    str += " width";
  }
  if (lDspMaskTest(&fieldMask, DB_ZDR2)) {
    str += " zdr";
  }

  return str;

}

string Commands::majorMode2String(int majorMode)

{

  switch (majorMode) {
    case PMODE_PPP: return "pulse-pair";
    case PMODE_FFT: return "fft";
    case PMODE_RPH: return "phase-coded";
    case PMODE_DPT2: return "staggered-prt";
    case PMODE_USER1: return "user1";
    case PMODE_USER2: return "user2";
    case PMODE_USER3: return "user3";
    case PMODE_USER4: return "user4";
    default: return "unknown";
  }

}

string Commands::procFlags2SpeckleFiltString(int procFlags)

{

  if ((procFlags & OPF_LSR) && 
      (procFlags & OPF_DSR)) {
    return "both";
  } else if (procFlags & OPF_LSR) {
    return "dbz";
  } else if (procFlags & OPF_DSR) {
    return "vel";
  } else {
    return "off";
  }

}

string Commands::prfMode2String(int prfMode)

{

  switch (prfMode) {
    case PRF_FIXED: return "fixed";
    case PRF_2_3: return "2_3";
    case PRF_3_4: return "3_4";
    case PRF_4_5: return "4_5";
    default: return "unknown";
  }

}

string Commands::procMode2String(int procMode)

{

  switch (procMode) {
    case DSP_PROC_SYNCHRONOUS: return "sync";
    case DSP_PROC_FREE_RUNNING: return "free";
    case DSP_PROC_TIME_SERIES: return "time_series";
    default: return "unknown";
  }

}

string Commands::windowType2String(int windowType)

{

  switch (windowType) {
    case WIN_RECT: return "rect";
    case WIN_HAMMING: return "hamming";
    case WIN_BLACKMAN: return "blackman";
    case WIN_BLKMANEX: return "blkmanex";
    case WIN_VONHANN: return "vonhann";
    case WIN_ADAPTIVE: return "adaptive";
    default: return "unknown";
  }

}

string Commands::phaseCoding2String(int phaseCoding)

{
  
  switch (phaseCoding) {
    case PHSEQ_FIXED: return "fixed";
    case PHSEQ_RANDOM: return "random";
    case PHSEQ_CUSTOM: return "custom";
    case PHSEQ_SZ8_64: return "sz8_64";
    default: return "unknown";
  }

}

string Commands::polarization2String(int polarization)

{
  
  switch (polarization) {
    case POL_HORIZ_FIX: return "horizontal";
    case POL_VERT_FIX: return "vertical";
    case POL_ALTERNATING: return "alternating";
    case POL_SIMULTANEOUS: return "simultaneous";
    default: return "unknown";
  }

}

string Commands::angSyncMode2String(int angSyncMode)

{
  
  switch (angSyncMode) {
    case ANGSYN_NONE: return "none";
    case ANGSYN_DYNAMIC: return "dynamic";
    default: return "unknown";
  }

}

///////////////////////////////////////////
// clear the mask which holds the fields

void Commands::_clearFieldMask()

{
  memset(&_fieldMask, 0, sizeof(_fieldMask));
#ifdef NOTNOW
  DspMaskClear(&_fieldMask, DB_DBT2);
  DspMaskClear(&_fieldMask, DB_DBZ2);
  DspMaskClear(&_fieldMask, DB_DBZC2);
  DspMaskClear(&_fieldMask, DB_FLAGS2);
  DspMaskClear(&_fieldMask, DB_KDP2);
  DspMaskClear(&_fieldMask, DB_LDRH2);
  DspMaskClear(&_fieldMask, DB_LDRV2);
  DspMaskClear(&_fieldMask, DB_PHIDP2);
  DspMaskClear(&_fieldMask, DB_PHIH2);
  DspMaskClear(&_fieldMask, DB_PHIV2);
  DspMaskClear(&_fieldMask, DB_RHOH2);
  DspMaskClear(&_fieldMask, DB_RHOHV2);
  DspMaskClear(&_fieldMask, DB_RHOV2);
  DspMaskClear(&_fieldMask, DB_SQI2);
  DspMaskClear(&_fieldMask, DB_VEL2);
  DspMaskClear(&_fieldMask, DB_VELC2);
  DspMaskClear(&_fieldMask, DB_WIDTH2);
  DspMaskClear(&_fieldMask, DB_ZDR2);
#endif
}

//////////////////////////////////////////////////
// Get current command state in XML form

void Commands::getCommandXml(string &xml)
{

  // form XML string
  
  xml = "";

  xml += TaXml::writeStartTag("rvp8Commands", 0);

  if (lDspMaskTest(&_fieldMask, DB_DBZ2)) {
    xml += TaXml::writeString("field", 1, "dbz");
  }
  if (lDspMaskTest(&_fieldMask, DB_VEL2)) {
    xml += TaXml::writeString("field", 1, "vel");
  }
  if (lDspMaskTest(&_fieldMask, DB_WIDTH2)) {
    xml += TaXml::writeString("field", 1, "width");
  }
  if (lDspMaskTest(&_fieldMask, DB_DBT2)) {
    xml += TaXml::writeString("field", 1, "dbt");
  }
  if (lDspMaskTest(&_fieldMask, DB_DBZC2)) {
    xml += TaXml::writeString("field", 1, "dbzc");
  }
  if (lDspMaskTest(&_fieldMask, DB_VELC2)) {
    xml += TaXml::writeString("field", 1, "velc");
  }
  if (lDspMaskTest(&_fieldMask, DB_FLAGS2)) {
    xml += TaXml::writeString("field", 1, "flags");
  }
  if (lDspMaskTest(&_fieldMask, DB_SQI2)) {
    xml += TaXml::writeString("field", 1, "sqi");
  }

  if (lDspMaskTest(&_fieldMask, DB_KDP2)) {
    xml += TaXml::writeString("field", 1, "kdp");
  }
  if (lDspMaskTest(&_fieldMask, DB_LDRH2)) {
    xml += TaXml::writeString("field", 1, "ldrh");
  }
  if (lDspMaskTest(&_fieldMask, DB_LDRV2)) {
    xml += TaXml::writeString("field", 1, "ldrv");
  }
  if (lDspMaskTest(&_fieldMask, DB_PHIDP2)) {
    xml += TaXml::writeString("field", 1, "phidp");
  }
  if (lDspMaskTest(&_fieldMask, DB_PHIH2)) {
    xml += TaXml::writeString("field", 1, "phih");
  }
  if (lDspMaskTest(&_fieldMask, DB_PHIV2)) {
    xml += TaXml::writeString("field", 1, "phiv");
  }
  if (lDspMaskTest(&_fieldMask, DB_RHOH2)) {
    xml += TaXml::writeString("field", 1, "rhoh");
  }
  if (lDspMaskTest(&_fieldMask, DB_RHOHV2)) {
    xml += TaXml::writeString("field", 1, "rhohv");
  }
  if (lDspMaskTest(&_fieldMask, DB_RHOV2)) {
    xml += TaXml::writeString("field", 1, "rhov");
  }
  if (lDspMaskTest(&_fieldMask, DB_ZDR2)) {
    xml += TaXml::writeString("field", 1, "zdr");
  }

  xml += TaXml::writeInt("nGates", 1, _nGates);
  xml += TaXml::writeDouble("gateSpacingKm", 1, _gateSpacing);

  xml += TaXml::writeString("prfMode", 1, prfMode2String(_prfMode));
  xml += TaXml::writeDouble("prf", 1, _prf);

  xml += TaXml::writeString("phaseCoding", 1, phaseCoding2String(_phaseCoding));
  xml += TaXml::writeString("polarization", 1,
                            polarization2String(_polarization));

  xml += TaXml::writeString("angSyncMode", 1,
                            angSyncMode2String(_angSyncMode));

  xml += TaXml::writeInt("nSamples", 1, _nSamples);
  xml += TaXml::writeString("majorMode", 1, majorMode2String(_majorMode));
  xml += TaXml::writeString("windowType", 1, windowType2String(_windowType));
  xml += TaXml::writeInt("clutterFilterNumber", 1, _clutFiltNum);
  xml += TaXml::writeInt("rangeSmoothing", 1, _rangeSmooth);
  xml += TaXml::writeString("speckleFilter", 1,
                            procFlags2SpeckleFiltString(_procFlags));

  xml += TaXml::writeEndTag("rvp8Commands", 0);

}

//////////////////////////////////////////////////
// Get status in XML form

void Commands::getSimulatedStatusXml(string &xml)
{

  // form XML string

  xml = "";
  xml += TaXml::writeStartTag("rvp8Status", 0);
  xml += TaXml::writeBoolean("readFromTsApi", 1, false);
  xml += TaXml::writeString("siteName", 1, "unknown");
  xml += TaXml::writeString
    ("majorMode", 1, majorMode2String(_majorMode));
  xml += TaXml::writeString
    ("polarization", 1, polarization2String(_polarization));
  xml += TaXml::writeString
    ("phaseCoding", 1, phaseCoding2String(_phaseCoding));
  xml += TaXml::writeString
    ("prfMode", 1, prfMode2String(_prfMode));
  xml += TaXml::writeDouble("pulseWidthUs", 1, 1.5);
  xml += TaXml::writeDouble("dbzCal1km", 1, -42.0);
  xml += TaXml::writeDouble("ifdClockMhz", 1, 36.0);
  xml += TaXml::writeDouble("wavelengthCm", 1, 10.00);
  xml += TaXml::writeDouble("satPowerDbm", 1, 10.0);
  xml += TaXml::writeDouble("rangeMaskResKm", 1, 0.025);
  xml += TaXml::writeDouble("startRangeKm", 1, 0.125);
  xml += TaXml::writeDouble("maxRangeKm", 1, _nGates * _gateSpacing);
  xml += TaXml::writeDouble("gateSpacingKm", 1, _gateSpacing);
  xml += TaXml::writeDouble("noiseChan0", 1, -113,0);
  xml += TaXml::writeDouble("noiseChan1", 1, -112.5);
  xml += TaXml::writeDouble("noiseSdevChan0", 1, 1,0);
  xml += TaXml::writeDouble("noiseSdevChan1", 1, 1.1);
  xml += TaXml::writeDouble("noiseRangeKm", 1, 160.0);
  xml += TaXml::writeDouble("noisePrfHz", 1, 600);
  xml += TaXml::writeString("rdaVersion", 1, "9.99");
  xml += TaXml::writeTime("time", 1, time(NULL));
  xml += TaXml::writeDouble("prf", 1, _prf);
  xml += TaXml::writeInt("nGates", 1, _nGates);
  xml += TaXml::writeDouble("el", 1, _el);
  xml += TaXml::writeDouble("az", 1, _az);
  xml += TaXml::writeEndTag("rvp8Status", 0);

  _az += 10.0;
  if (_az >= 360) {
    _az -= 360.0;
  }

  _el += 0.01;
  if (_el > 45) {
    _el = 0.0;
  }

}

