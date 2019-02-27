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
// RadarTsPulse.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
////////////////////////////////////////////////////////////////
//
// Class has been deprecated.
// Use IwrfTsPulse instead.
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/RadarTsPulse.hh>
using namespace std;

bool RadarTsPulse::_floatLutReady = false;
fl32 RadarTsPulse::_floatLut[65536] = { 0 };

// Constructor

RadarTsPulse::RadarTsPulse(const RadarTsInfo &info,
                           RadarTsDebug_t debug) :
        _info(info),
        _debug(debug),
        _nClients(0)
  
{

  _iq = NULL;
  _iq0 = NULL;
  _iq1 = NULL;
  _burstIq0 = NULL;
  _burstIq1 = NULL;
  _packed = NULL;
  _invertHvFlag = false;

  // iitialize header

  ts_pulse_hdr_init(&_hdr);
  _checkScanModeForVert();
  
  // compute float lookup if needed

  if (!_floatLutReady) {
    _computeFloatLut();
  }

  // initialize mutex for protecting _nClients
  
  pthread_mutex_init(&_nClientsMutex, NULL);

}

// destructor

RadarTsPulse::~RadarTsPulse()

{
  if (_iq) {
    delete[] _iq;
  }
  if (_packed) {
    delete[] _packed;
  }
  pthread_mutex_destroy(&_nClientsMutex);
}

///////////////////////////////////////////////////////////
// set from ts_pulse_hdr 
// Returns 0 on success, -1 on failure

int RadarTsPulse::setFromTsBuffer(const void *buf, int len)
  
{

  if (len < (int) sizeof(ts_pulse_hdr_t)) {
    cerr << "ERROR - RadarTsPulse::setFromTsPulse" << endl;
    cerr << "  Buffer passed in too short, len: " << len << endl;
    cerr << "  Must be at least len: " << sizeof(ts_pulse_hdr_t) << endl;
    return -1;
  }
  
  memcpy(&_hdr, buf, sizeof(ts_pulse_hdr_t));

  // derive

  if (_hdr.elevation > 180) {
    _hdr.elevation -= 360.0;
  }
  _checkScanModeForVert();

  int nGatesTotal = _hdr.nGates + _hdr.nGatesBurst;
  int nDataMin = _hdr.nChannels * (nGatesTotal) * 2;
  if ((int) _hdr.nData < nDataMin) {
    if (_debug >= RTS_DEBUG_VERBOSE) {
      cerr << "WARNING: RadarTsPulse::setFromTsBuffer: ndata set to low" << endl;
      cerr << "  nData in pulse header: " << _hdr.nData << endl;
      cerr << "  Setting to min required: " << nDataMin << endl;
    }
    _hdr.nData = nDataMin;
  }
  
  _ftime = (double) _hdr.timeSecsUTC + _hdr.timeNanoSecs / 1.0e9;
  _prf = 1.0 / _hdr.prt;
  
  _phaseDiff[0] = (_hdr.rvp8.iBurstArg[0] / 65536.0) * 360.0;
  _phaseDiff[1] = (_hdr.rvp8.iBurstArg[1] / 65536.0) * 360.0;
  
  // special 9999 code is stored in uiqOnce[0] to indicate
  // nGatesBurst is stored in uiqOnce[1]
  
  _hdr.rvp8.uiqOnce[0] = 9999;
  _hdr.rvp8.uiqOnce[1] = _hdr.nGatesBurst;
  
  // load up IQ data
  
  int requiredLen = (int) sizeof(ts_pulse_hdr_t) + _hdr.nData * sizeof(ui16);
  if (_hdr.iqEncoding == TS_IQ_ENCODING_FLOAT32) {
    requiredLen = (int) sizeof(ts_pulse_hdr_t) + _hdr.nData * sizeof(fl32);
  }
  if (len < requiredLen) {
    cerr << "ERROR - RadarTsPulse::setFromTsPulse" << endl;
    cerr << "  Buffer passed in too short, len: " << len << endl;
    cerr << "  Must be at least len: " << requiredLen << endl;
    cerr << "sizeof(ts_pulse_hdr_t): " << sizeof(ts_pulse_hdr_t) << endl; 
    ts_pulse_hdr_print(stderr, "", &_hdr);
    return -1;
  }
  
  if (_hdr.iqEncoding == TS_IQ_ENCODING_FLOAT32) {
    
    fl32 *iq = (fl32 *) ((char *) buf + sizeof(ts_pulse_hdr_t));
    if (_iq) {
      delete[] _iq;
    }
    _iq = new fl32[_hdr.nData];
    memcpy(_iq, iq, _hdr.nData * sizeof(fl32));

    _burstIq0 = _iq;
    _iq0 = _burstIq0 + _hdr.nGatesBurst * 2;
    
    if (_hdr.nChannels > 1) {
      if (_hdr.iqOffset[1] == 0) {
        _hdr.iqOffset[1] = _hdr.nGates * 2;
      }
      _burstIq1 = _iq + nGatesTotal * 2; // channel 1 stored after channel 0
      _iq1 = _burstIq1 + _hdr.nGatesBurst;
    }

  } else {

    if (_packed) {
      delete[] _packed;
    }
    _packed = new ui16[_hdr.nData];
    ui16 *packed = (ui16 *) ((char *) buf + sizeof(ts_pulse_hdr_t));
    memcpy(_packed, packed, _hdr.nData * sizeof(ui16));
    _loadIqFromPacked();

  }

  return 0;

}

///////////////////////////////////////////////////////////
// set RVP8-specific fields

void RadarTsPulse::setRvp8Hdr(const ts_pulse_hdr_t &hdr)
  
{
  
  if (hdr.rvp8.iFlags != 255) {
    return;
  }
  
  _hdr.rvp8.iPolarBits = hdr.hvFlag;
  _hdr.rvp8.iVIQPerBin = hdr.nChannels;
  
  _hdr.rvp8.iAz = (ui16) (hdr.azimuth * 65535.0 / 360.0 + 0.5);
  _hdr.rvp8.iEl = (ui16) (hdr.elevation * 65535.0 / 360.0 + 0.5);
  _hdr.rvp8.iNumVecs = hdr.nGates;
  _hdr.rvp8.iMaxVecs = hdr.nGates;

}
  
///////////////////////////////////////////////////////////
// read pulse from rvp8 time series file
//
// Returns 0 on success, -1 on failure

int RadarTsPulse::readFromRvp8File(FILE *in)

{

  // read in header

  if (_readRvp8Header(in)) {
    return -1;
  }

  _deriveFromRvp8Header();

  if (_readRvp8Data(in)) {
    return -1;
  }

  if (_debug >= RTS_DEBUG_VERBOSE) {
    print(cerr);
  }
  
  return 0;

}
  
/////////////////////////////////////////////////////////////////
// Check to see if horizontally polarized

bool RadarTsPulse::isHoriz() const

{

  if (!_invertHvFlag) {
    if (_hdr.hvFlag) {
      return true;
    } else {
      return false;
    }
  } else {
    if (_hdr.hvFlag) {
      return false;
    } else {
      return true;
    }
  }

}

//////////////////////////////
// get packed data

const ui16 *RadarTsPulse::getPacked() const

{

  // first check if the packed data is already available

  if (_packed) {
    return _packed;
  }
  
  // not avaialable, so we must create is from the floats

  _packed = new ui16[_hdr.nData];
  
  double mult = _hdr.scale;
  
  fl32 *floats = new fl32[_hdr.nData];
  for (int ii = 0; ii < (int) _hdr.nData; ii++) {
    floats[ii] = _iq[ii] / mult;
  }

  vecPackIQFromFloatIQ(_packed, floats, _hdr.nData);

  return _packed;

}

/////////////////////////////
// get elevation and azimuth

double RadarTsPulse::getEl() const {
  if (_hdr.elevation <= 180) {
    return _hdr.elevation;
  } else {
    return (_hdr.elevation - 360.0);
  }
}

double RadarTsPulse::getAz() const {
  if (_hdr.azimuth >= 0) {
    return _hdr.azimuth;
  } else {
    return _hdr.azimuth + 360.0;
  }
}

double RadarTsPulse::getTargetEl() const {
  if (_hdr.targetEl <= 180) {
    return _hdr.targetEl;
  } else {
    return (_hdr.targetEl - 360.0);
  }
}

double RadarTsPulse::getTargetAz() const {
  if (_hdr.targetAz >= 0) {
    return _hdr.targetAz;
  } else {
    return _hdr.targetAz + 360.0;
  }
}

/////////////////////////////////////////////////////////////////
// Compute phase differences between this pulse and previous ones
// to be able to cohere to multiple trips
//
// Before this method is called, this pulse should be added to
// the queue.

int RadarTsPulse::computePhaseDiffs
  (const deque<const RadarTsPulse *> &pulseQueue, int maxTrips) const
  
{
  
  if (pulseQueue[0] != this) {
    cerr << "ERROR - RadarTsPulse::computePhaseDiffs()" << endl;
    cerr << "  You must add this pulse before calling this function" << endl;
    return -1;
  }

  _phaseDiffs.clear();
  
  // phase diffs for maxTrips previous beams
  
  int qSize = (int) pulseQueue.size();
  
  for (int ii = 0; ii < maxTrips; ii++) { // ii is (trip - 1)
    if (ii == 0) {
      _phaseDiffs.push_back(0.0);
    } else {
      if (ii <= qSize) {
	double sum = _phaseDiffs[ii-1] + pulseQueue[ii-1]->getPhaseDiff0();
	while (sum > 360.0) {
	  sum -= 360.0;
	}
	_phaseDiffs.push_back(sum);
      } else {
	// actual phase diff not available
	// use previous trip's value
	_phaseDiffs.push_back(_phaseDiffs[ii-1]);
      }
    }
  }

  return 0;

}

/////////////////////////////////////
// print

void RadarTsPulse::print(ostream &out) const
{

  out << "==================== Pulse object ==================" << endl;
  out << "-------------------- TS Header ---------------------" << endl;
  out << "radarId: " << _hdr.radarId << endl;
  out << "pulseSeqNum: " << _hdr.pulseSeqNum << endl;
  out << "time: " << DateTime::strm(_hdr.timeSecsUTC) << endl;
  out << "timeNanoSecs: " << _hdr.timeNanoSecs << endl;
  out << "elevation: " << _hdr.elevation << endl;
  out << "azimuth: " << _hdr.azimuth << endl;
  out << "prt: " << _hdr.prt << endl;
  out << "prtNext: " << _hdr.prtNext << endl;
  out << "pulseWidth: " << _hdr.pulseWidth << endl;
  out << "nGates: " << _hdr.nGates << endl;
  out << "tiltNum: " << _hdr.tiltNum << endl;
  out << "volNum: " << _hdr.volNum << endl;
  out << "nChannels: " << (int) _hdr.nChannels << endl;
  out << "iqEncoding: " << (int) _hdr.iqEncoding << endl;
  out << "hvFlag: " << (int) _hdr.hvFlag << endl;
  out << "antennaTransition: " << (int) _hdr.antennaTransition << endl;
  out << "phaseCohered: " << (int) _hdr.phaseCohered << endl;
  out << "status: " << (int) _hdr.status << endl;
  out << "nData: " << (int) _hdr.nData << endl;
  for (int ii = 0; ii < TS_MAX_CHAN; ii++) {
    out << "iqOffset[" << ii << "]: " << (int) _hdr.iqOffset[ii] << endl;
    out << "burstMag[" << ii << "]: " << _hdr.burstMag[ii] << endl;
    out << "burstArg[" << ii << "]: " << _hdr.burstArg[ii] << endl;
    out << "burstArgDiff[" << ii << "]: " << _hdr.burstArgDiff[ii] << endl;
  }
  out << "measXmitPowerDbmH: " << _hdr.measXmitPowerDbmH << endl;
  out << "measXmitPowerDbmV: " << _hdr.measXmitPowerDbmV << endl;
  out << "scale: " << _hdr.scale << endl;
  out << "bias: " << _hdr.bias << endl;
  out << "nGatesBurst: " << (int) _hdr.nGatesBurst << endl;

  out << "scanMode: " << (int) _hdr.scanMode << endl;
  out << "targetEl: " << _hdr.targetEl << endl;
  out << "targetAz: " << _hdr.targetAz << endl;

  out << "phaseDiff[0]: " << _phaseDiff[0] << endl;
  out << "phaseDiff[1]: " << _phaseDiff[1] << endl;

  out << "-------------------- RVP8 section ------------------" << endl;
  out << "iVersion: " << (int) _hdr.version << endl;
  out << "iFlags: " << (int) _hdr.rvp8.iFlags << endl;
  out << "iMSecUTC: " << (int) _hdr.timeNanoSecs / 1000000 << endl;
  out << "iTimeUTC: " << (int) _hdr.timeSecsUTC << endl;
  out << "iBtimeAPI: " << (int) _hdr.rvp8.iBtimeAPI << endl;
  out << "iSysTime: " << (int) _hdr.rvp8.iSysTime << endl;
  out << "iPrevPRT: " << (int) _hdr.rvp8.iPrevPRT << endl;
  out << "iNextPRT: " << (int) _hdr.rvp8.iNextPRT << endl;
  out << "iSeqNum: " << (int) _hdr.pulseSeqNum << endl;
  out << "iAqMode: " << (int) _hdr.rvp8.iAqMode << endl;
  out << "iAz: " << (int) _hdr.rvp8.iAz << endl;
  out << "iEl: " << (int) _hdr.rvp8.iEl << endl;
  out << "iNumVecs: " << _hdr.rvp8.iNumVecs << endl;
  out << "iMaxVecs: " << _hdr.rvp8.iMaxVecs << endl;
  out << "iVIQPerBin: " << (int) _hdr.rvp8.iVIQPerBin << endl;
  out << "iTgBank: " << (int) _hdr.rvp8.iTgBank << endl;
  out << "iTgWave: " << (int) _hdr.rvp8.iTgWave << endl;
  out << "uiqPerm.iLong[0]: " << (int) _hdr.rvp8.uiqPerm[0] << endl;
  out << "uiqPerm.iLong[1]: " << (int) _hdr.rvp8.uiqPerm[1] << endl;
  out << "uiqOnce.iLong[0]: " << (int) _hdr.rvp8.uiqOnce[0] << endl;
  out << "uiqOnce.iLong[1]: " << (int) _hdr.rvp8.uiqOnce[1] << endl;
  out << "RX[0].fBurstMag: " << _hdr.rvp8.fBurstMag[0] << endl;
  out << "RX[0].iBurstArg: " << (int) _hdr.rvp8.iBurstArg[0] << endl;
  out << "RX[1].fBurstMag: " << _hdr.rvp8.fBurstMag[1] << endl;
  out << "RX[1].iBurstArg: " << (int) _hdr.rvp8.iBurstArg[1] << endl;
  out << "----------------------------------------------------" << endl;

}

////////////////////////
// write to tsarchive file

int RadarTsPulse::write2TsarchiveFile(FILE *out)

{
  
  if (out == NULL) {
    return -1;
  }

  fprintf(out, "rvp8PulseHdr start\n");

  fprintf(out, "iVersion=%d\n", _hdr.version);
  fprintf(out, "iFlags=%d\n", _hdr.rvp8.iFlags);
  fprintf(out, "iMSecUTC=%d\n", _hdr.timeNanoSecs);
  fprintf(out, "iTimeUTC=%d\n", _hdr.timeSecsUTC);
  fprintf(out, "iBtimeAPI=%d\n", _hdr.rvp8.iBtimeAPI);
  fprintf(out, "iSysTime=%d\n", _hdr.rvp8.iSysTime);
  fprintf(out, "iPrevPRT=%d\n", _hdr.rvp8.iPrevPRT);
  fprintf(out, "iNextPRT=%d\n", _hdr.rvp8.iNextPRT);
  fprintf(out, "iSeqNum=%d\n", _hdr.pulseSeqNum);
  fprintf(out, "iAqMode=%d\n", _hdr.rvp8.iAqMode);
  fprintf(out, "iPolarBits=%d\n", _hdr.rvp8.iPolarBits);
  fprintf(out, "iTxPhase=%d\n", _hdr.rvp8.iTxPhase);
  fprintf(out, "iAz=%d\n", _hdr.rvp8.iAz);
  fprintf(out, "iEl=%d\n", _hdr.rvp8.iEl);
  fprintf(out, "iNumVecs=%d\n", _hdr.rvp8.iNumVecs);
  fprintf(out, "iMaxVecs=%d\n", _hdr.rvp8.iMaxVecs);
  fprintf(out, "iVIQPerBin=%d\n", _hdr.rvp8.iVIQPerBin);
  fprintf(out, "iTgBank=%d\n", _hdr.rvp8.iTgBank);
  fprintf(out, "iTgWave=%d\n", _hdr.rvp8.iTgWave);

  fprintf(out, "uiqPerm.iLong=%d %d\n",
          (unsigned) _hdr.rvp8.uiqPerm[0], (unsigned) _hdr.rvp8.uiqPerm[1]);

  fprintf(out, "uiqOnce.iLong=%d %d\n",
          (unsigned) _hdr.rvp8.uiqOnce[0], (unsigned) _hdr.rvp8.uiqOnce[1]);

  fprintf(out, "RX[0].fBurstMag=%g\n", _hdr.rvp8.fBurstMag[0]);
  fprintf(out, "RX[0].iBurstArg=%d\n", _hdr.rvp8.iBurstArg[0]);
  fprintf(out, "RX[1].fBurstMag=%g\n", _hdr.rvp8.fBurstMag[1]);
  fprintf(out, "RX[1].iBurstArg=%d\n", _hdr.rvp8.iBurstArg[1]);

  fprintf(out, "rvp8PulseHdr end\n");

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Memory management.
// This class uses the notion of clients to decide when it should be deleted.
// If removeClient() returns 0, the object should be deleted.
// These functions are protected by a mutex for multi-threaded ops

int RadarTsPulse::addClient(const string &clientName) const
  
{

  pthread_mutex_lock(&_nClientsMutex);
  _nClients++;
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

int RadarTsPulse::removeClient(const string &clientName) const

{
  pthread_mutex_lock(&_nClientsMutex);
  _nClients--;
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

///////////////////////////////////////////////////////////////
// check the scan mode for vert pointing
//
// If surveillance and angle is high, set to vert point

void RadarTsPulse::_checkScanModeForVert()
  
{

  if (_hdr.scanMode == TS_SCAN_MODE_SURVEILLANCE &&
      _hdr.elevation > 85 && _hdr.elevation < 95) {
    _hdr.scanMode = TS_SCAN_MODE_VERTICAL_POINTING;
  }
  
}
  
///////////////////////////////////////////////////////////////
// read in pulse header
//
// Returns 0 on success, -1 on failure

int RadarTsPulse::_readRvp8Header(FILE *in)

{
  
  if (_debug >= RTS_DEBUG_VERBOSE) {
    cerr << "--->> Reading RadarTsPulse Header <<-----" << endl;
  }

  char line[BUFSIZ];

  // initialize to missing values

  memset(&_hdr, 0, sizeof(_hdr));

  bool versionFound = false;
  bool iFlagsFound = false;
  bool timeNanoSecsFound = false;
  bool timeSecsUTCFound = false;
  bool iBtimeAPIFound = false;
  bool iSysTimeFound = false;
  bool iPrevPRTFound = false;
  bool iNextPRTFound = false;
  bool pulseSeqNumFound = false;
  bool iAqModeFound = false;
  bool iPolarBitsFound = false;
  bool iTxPhaseFound = false;
  bool iAzFound = false;
  bool iElFound = false;
  bool iNumVecsFound = false;
  bool iMaxVecsFound = false;
  bool iVIQPerBinFound = false;
  bool iTgBankFound = false;
  bool iTgWaveFound = false;
  bool uiqPermFound = false;
  bool uiqOnceFound = false;
  bool fBurstMag0Found = false;
  bool iBurstArg0Found = false;
  bool fBurstMag1Found = false;
  bool iBurstArg1Found = false;

  // read to the pulse header
  
  if (RadarTsInfo::findNextStr(in, "PulseHdr start")) {
    return -1;
  }
    
  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    
    if (_debug >= RTS_DEBUG_VERBOSE) {
      char shortLine[80];
      memcpy(shortLine, line, 79);
      shortLine[79] = '\0';
      cerr << shortLine;
    }
    
    if (strstr(line, "rvp8PulseHdr end")) {
      return 0;
    }

    // look for integers

    int ival, jval;

    if (!versionFound && sscanf(line, "iVersion=%d", &ival) == 1) {
      _hdr.version = ival;
      versionFound = true;
      continue;
    }

    if (!iFlagsFound && sscanf(line, "iFlags=%d", &ival) == 1) {
      _hdr.rvp8.iFlags = ival;
      iFlagsFound = true;
      continue;
    }

    if (!timeNanoSecsFound && sscanf(line, "iMSecUTC=%d", &ival) == 1) {
      _hdr.timeNanoSecs = ival * 1000000;
      timeNanoSecsFound = true;
      continue;
    }

    if (!timeSecsUTCFound && sscanf(line, "iTimeUTC=%d", &ival) == 1) {
      _hdr.timeSecsUTC = ival;
      timeSecsUTCFound = true;
      continue;
    }

    if (!iBtimeAPIFound && sscanf(line, "iBtimeAPI=%d", &ival) == 1) {
      _hdr.rvp8.iBtimeAPI = ival;
      iBtimeAPIFound = true;
      continue;
    }

    if (!iSysTimeFound && sscanf(line, "iSysTime=%d", &ival) == 1) {
      _hdr.rvp8.iSysTime = ival;
      iSysTimeFound = true;
      continue;
    }

    if (!iPrevPRTFound && sscanf(line, "iPrevPRT=%d", &ival) == 1) {
      _hdr.rvp8.iPrevPRT = ival;
      iPrevPRTFound = true;
      continue;
    }

    if (!iNextPRTFound && sscanf(line, "iNextPRT=%d", &ival) == 1) {
      _hdr.rvp8.iNextPRT = ival;
      iNextPRTFound = true;
      continue;
    }

    if (!pulseSeqNumFound && sscanf(line, "iSeqNum=%d", &ival) == 1) {
      _hdr.pulseSeqNum = ival;
      pulseSeqNumFound = true;
      continue;
    }

    if (!iAqModeFound && sscanf(line, "iAqMode=%d", &ival) == 1) {
      _hdr.rvp8.iAqMode = ival;
      iAqModeFound = true;
      continue;
    }

    if (!iPolarBitsFound && sscanf(line, "iPolarBits=%d", &ival) == 1) {
      _hdr.rvp8.iPolarBits = ival;
      iPolarBitsFound = true;
      continue;
    }

    if (!iTxPhaseFound && sscanf(line, "iTxPhase=%d", &ival) == 1) {
      _hdr.rvp8.iTxPhase = ival;
      iTxPhaseFound = true;
      continue;
    }

    if (!iAzFound && sscanf(line, "iAz=%d", &ival) == 1) {
      _hdr.rvp8.iAz = ival;
      iAzFound = true;
      continue;
    }

    if (!iElFound && sscanf(line, "iEl=%d", &ival) == 1) {
      _hdr.rvp8.iEl = ival;
      iElFound = true;
      continue;
    }

    if (!iNumVecsFound && sscanf(line, "iNumVecs=%d", &ival) == 1) {
      _hdr.rvp8.iNumVecs = ival;
      iNumVecsFound = true;
      continue;
    }

    if (!iMaxVecsFound && sscanf(line, "iMaxVecs=%d", &ival) == 1) {
      _hdr.rvp8.iMaxVecs = ival;
      iMaxVecsFound = true;
      continue;
    }

    if (!iVIQPerBinFound && sscanf(line, "iVIQPerBin=%d", &ival) == 1) {
      _hdr.rvp8.iVIQPerBin = ival;
      iVIQPerBinFound = true;
      continue;
    }

    if (!iTgBankFound && sscanf(line, "iTgBank=%d", &ival) == 1) {
      _hdr.rvp8.iTgBank = ival;
      iTgBankFound = true;
      continue;
    }

    if (!iTgWaveFound && sscanf(line, "iTgWave=%d", &ival) == 1) {
      _hdr.rvp8.iTgWave = ival;
      iTgWaveFound = true;
      continue;
    }

    if (!uiqPermFound && sscanf(line, "uiqPerm.iLong=%d %d", &ival, &jval) == 2) {
      _hdr.rvp8.uiqPerm[0] = ival;
      _hdr.rvp8.uiqPerm[1] = jval;
      uiqPermFound = true;
      continue;
    }
    
    if (!uiqOnceFound && sscanf(line, "uiqOnce.iLong=%d %d", &ival, &jval) == 2) {
      _hdr.rvp8.uiqOnce[0] = ival;
      _hdr.rvp8.uiqOnce[1] = jval;
      uiqOnceFound = true;
      continue;
    }
    
    // look for floats
    
    double dval;

    if (!fBurstMag0Found && sscanf(line, "RX[0].fBurstMag=%lg", &dval) == 1) {
      _hdr.rvp8.fBurstMag[0] = dval;
      fBurstMag0Found = true;
      continue;
    }

    if (!iBurstArg0Found && sscanf(line, "RX[0].iBurstArg=%d", &ival) == 1) {
      _hdr.rvp8.iBurstArg[0] = ival;
      iBurstArg0Found = true;
      continue;
    }

    if (!fBurstMag1Found && sscanf(line, "RX[1].fBurstMag=%lg", &dval) == 1) {
      _hdr.rvp8.fBurstMag[1] = dval;
      fBurstMag1Found = true;
      continue;
    }

    if (!iBurstArg1Found && sscanf(line, "RX[1].iBurstArg=%d", &ival) == 1) {
      _hdr.rvp8.iBurstArg[1] = ival;
      iBurstArg1Found = true;
      continue;
    }

  } // while
  
  return 0;

}

///////////////////////////////////////////////////////////////
// derive quantities from rvp8 pulse header

void RadarTsPulse::_deriveFromRvp8Header()

{
  
  _ftime = (double) _hdr.timeSecsUTC + _hdr.timeNanoSecs / 1.0e9;

  // if special 9999 code is stored in uiqOnce[0], then the
  // nGatesBurst is stored in uiqOnce[1]
  // otherwise use the default
  
  if (_hdr.rvp8.uiqOnce[0] == 9999) {
    _hdr.nGatesBurst = _hdr.rvp8.uiqOnce[1];
  } else {
    _hdr.nGatesBurst = RVP8_NGATES_BURST;
  }
  
  // first gates hold the burst
  _hdr.nGates = _hdr.rvp8.iNumVecs - _hdr.nGatesBurst;
  _hdr.nChannels = _hdr.rvp8.iVIQPerBin;
  _hdr.nData = _hdr.rvp8.iNumVecs * _hdr.rvp8.iVIQPerBin * 2;
  _hdr.azimuth = (_hdr.rvp8.iAz / 65535.0) * 360.0;
  _hdr.elevation = (_hdr.rvp8.iEl / 65535.0) * 360.0;
  if (_hdr.elevation > 180) {
    _hdr.elevation -= 360.0;
  }
  _checkScanModeForVert();

  if (_hdr.rvp8.iPrevPRT == 0) {
    _hdr.prt = 0.001;
  } else {
    double fSyClkMhz = _info.getRvp8ClockMhz();
    _hdr.prt = ((double) _hdr.rvp8.iPrevPRT / fSyClkMhz) / 1.0e6;
  }
  if (_hdr.rvp8.iNextPRT == 0) {
    _hdr.prtNext = 0.001;
  } else {
    double fSyClkMhz = _info.getRvp8ClockMhz();
    _hdr.prtNext = ((double) _hdr.rvp8.iNextPRT / fSyClkMhz) / 1.0e6;
  }
  _prf = 1.0 / _hdr.prt;

  _hdr.pulseWidth = _info.getRvp8PulseWidthUs() * 1000;
  _hdr.tiltNum = -1;
  _hdr.volNum = -1;
  _hdr.hvFlag = _hdr.rvp8.iPolarBits;
  _hdr.phaseCohered = 1;

  double fSaturationDBM = _info.getRvp8SaturationDbm();
  double mult = pow(10.0, fSaturationDBM / 20.0);
  _hdr.scale = mult;

  _phaseDiff[0] = (_hdr.rvp8.iBurstArg[0] / 65536.0) * 360.0;
  _phaseDiff[1] = (_hdr.rvp8.iBurstArg[1] / 65536.0) * 360.0;

}

///////////////////////////////////////////////////////////////
// read in pulse data
//
// Should follow _readRvp8Header and _derivedFromPulseHeader()
// immediately
//
// Returns 0 on success, -1 on failure

int RadarTsPulse::_readRvp8Data(FILE *in)
  
{
  
  if (_debug >= RTS_DEBUG_VERBOSE) {
    cerr << "--->> Reading Pulse Data <<-----" << endl;
  }

  // read in packed data

  _hdr.nData = _hdr.rvp8.iNumVecs * _hdr.rvp8.iVIQPerBin * 2;
  if (_packed) {
    delete[] _packed;
  }
  _packed = new ui16[_hdr.nData];

  size_t nRead = fread(_packed, sizeof(ui16), _hdr.nData, in);
  if (nRead != _hdr.nData) {
    cerr << "ERROR - Pulse::_readRvp8Data" << endl;
    cerr << "  Cannot fread on pulse data" << endl;
    cerr << "  Expecting nData: " << _hdr.nData << endl;
    cerr << "  Got nRead: " << nRead << endl;
    if (feof(in)) {
      cerr << "  At end of file" << endl;
    }
    return -1;
  }

  // load the IQ float data

  _loadIqFromPacked();
  
  return 0;

}

///////////////////////////////////////////////////////////
// load IQ data from packed array

void RadarTsPulse::_loadIqFromPacked()
  
{

  // unpack the shorts into floats
  // adjust the IQ values for the saturation characteristics
  // apply the square root of the multiplier, since power is
  // I squared plus Q squared
  
  if (_iq) {
    delete[] _iq;
  }

  _iq = new fl32[_hdr.nData];
  double fSaturationDBM = _info.getRvp8SaturationDbm();
  double mult = pow(10.0, fSaturationDBM / 20.0);
  for (int ii = 0; ii < (int) _hdr.nData; ii++) {
    _iq[ii] = _floatLut[_packed[ii]] * mult;
  }

  _burstIq0 = _iq;
  _iq0 = _burstIq0 + _hdr.nGatesBurst * 2; // burst is in first 2 gates

  int nGatesTotal = _hdr.nGates + _hdr.nGatesBurst;
  
  if (_hdr.nChannels > 1) {
    _burstIq1 = _iq + nGatesTotal * 2; // channel 1 after channel 0
    _iq1 = _burstIq1 + _hdr.nGatesBurst * 2; // burst is in first 2 gates
  }

}

// compute lookup table for converting packed shorts to floats

void RadarTsPulse::_computeFloatLut()

{

  ui16 packed[65536];
  for (int ii = 0; ii < 65536; ii++) {
    packed[ii] = ii;
  }
  vecFloatIQFromPackIQ(_floatLut, packed, 65536);
  _floatLutReady = true;

}

/* ======================================================================
 * Convert a normalized floating "I" or "Q" value from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 *
 * From Sigmet lib.
 */

void RadarTsPulse::vecFloatIQFromPackIQ
( volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
  si32 iCount_a)
{

  si32 iCount ; volatile const ui16 *iCodes = iCodes_a ;
  volatile fl32 *fIQVals = fIQVals_a ;

  /* High SNR packed format with 12-bit mantissa
   */
  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode = *iCodes++ ; fl32 fVal = 0.0 ;
    
    if( iCode & 0xF000 ) {
      si32 iMan =  iCode        & 0x7FF ;
      si32 iExp = (iCode >> 12) & 0x00F ;
      
      if( iCode & 0x0800 ) iMan |= 0xFFFFF000 ;
      else                 iMan |= 0x00000800 ;
      
      fVal = ((fl32)iMan) * ((fl32)(ui32)(1 << iExp)) / 3.355443E7 ;
    }
    else {
      fVal = ( (fl32)(((si32)iCode) << 20) ) / 1.759218E13 ;
    }
    *fIQVals++ = fVal ;
  }
}

/* Convert a normalized floating "I" or "Q" value to/from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 */

/* ------------------------------
 * Convert an array of FLT4's to packed floating.
 * assumes HIGH SNR packing and no swapping
 */

#define NINT(fvalue) ((si32)floor(0.5+(double)(fvalue)))

void RadarTsPulse::vecPackIQFromFloatIQ
  ( volatile ui16 iCodes_a[], volatile const fl32 fIQVals_a[],
    si32 iCount_a )
{

  si32 iCount ; volatile const fl32 *fIQVals = fIQVals_a ;
  volatile ui16 *iCodes = iCodes_a ;

  /* High SNR packed format with 12-bit mantissa
   */
  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode ; fl32 fIQVal = *fIQVals++ ;
    
    if     (  fIQVal >=  4.0 ) iCode = 0xF7FF ;
    else if(  fIQVal <= -4.0 ) iCode = 0xF800 ;
    else if( (fIQVal >  -1.221299E-4) &&
             (fIQVal <   1.220703E-4) ) {
      si32 iUnderFlow = NINT( 1.677721E7 * fIQVal ) ;
      iCode = 0xFFF & MAX( -2048, MIN( 2047, iUnderFlow ) ) ;
    } else {
      int iSign, iExp, iMan ;
#ifdef IEEE754_FLOAT_BIAS
      /* Much faster (x4.5) version on LINUX uses hardware definition
       * of fl32.  Native mantissa has 23 bits, we want 12, rounded.
       * Courtesy of Nathan Parker, MIT/LL, Aug 2002.
       */
      union ieee754_float u ; int uMan ;
      u.f = fIQVal ; uMan = u.ieee.mantissa ; iSign = u.ieee.negative ;
      iMan = (uMan >> 12) | (1<<11) ; if( iSign ) iMan = ~iMan ;
      iMan += ((uMan >> 11) & 1 ) ^ iSign ;
      
      iExp = (int)u.ieee.exponent - (IEEE754_FLOAT_BIAS - 1) + 13 ;
#else
      iMan = NINT( 4096.0 * frexp( fIQVal, &iExp ) ) ;
      iExp += 13 ; iSign = (fIQVal < 0.0) ;
#endif
      
      if( iMan ==  4096 ) iExp++ ; /* Fix up mantissa overflows */
      if( iMan == -2048 ) iExp-- ; /*   by adjusting the exponent. */
      
      iCode = (iExp << 12) | (iSign << 11) | (0x7FF & iMan) ;
    }
    *iCodes++ = iCode ;
  }

}

