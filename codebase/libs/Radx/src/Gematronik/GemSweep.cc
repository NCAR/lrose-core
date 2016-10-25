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
// GemSweep.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////

#include "GemSweep.hh"
#include "GemBlob.hh"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <Radx/RadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/ByteOrder.hh>
#include <Radx/RadxStr.hh>
using namespace std;

// Constructor

GemSweep::GemSweep(int num, bool debug, bool verbose) :
        _debug(debug),
        _verbose(verbose),
        _num(num),
        _fieldData(NULL)

{

  clear();
  _initPulseWidths();
  
}

// Copy constructor

GemSweep::GemSweep(const GemSweep &orig, int num, 
                   bool debug, bool verbose) :
        _debug(debug),
        _verbose(verbose),
        _num(num),
        _fieldData(NULL)

{

  clear();
  _initPulseWidths();
  
  _debug = orig._debug;
  _verbose = orig._verbose;

  _startTime = orig._startTime;
  _fieldName = orig._fieldName;

  _fixedAngle = orig._fixedAngle;
  _nAngles = orig._nAngles;
  _nSamples = orig._nSamples;
  _nGates = orig._nGates;
  _startRange = orig._startRange;
  _gateSpacing = orig._gateSpacing;

  _angleRes = orig._angleRes;
  _isIndexed = orig._isIndexed;

  _prf = orig._prf;
  _highPrf = orig._highPrf;
  _lowPrf = orig._lowPrf;
  _isStaggered = orig._isStaggered;

  _nyquist = orig._nyquist;
  _pulseWidthIndex = orig._pulseWidthIndex;
  _pulseWidthUs = orig._pulseWidthUs;
  _antennaSpeed = orig._antennaSpeed;

  _radarConst = orig._radarConst;
  _radarConstH = orig._radarConstH;
  _radarConstV = orig._radarConstV;

  _xmitPeakPowerKw = orig._xmitPeakPowerKw;
  _ifMhz = orig._ifMhz;

  _noisePowerDbzH = orig._noisePowerDbzH;
  _noisePowerDbzV = orig._noisePowerDbzV;

  _minValue = orig._minValue;
  _maxValue = orig._maxValue;

  _dataBlobId = orig._dataBlobId;
  _anglesBlobId = orig._anglesBlobId;

  _dataByteWidth = orig._dataByteWidth;
  _anglesByteWidth = orig._anglesByteWidth;
  
}

// destructor

GemSweep::~GemSweep()

{
  clear();
}

//////////////////
// clear data

void GemSweep::clear()

{
  
  _startTime = 0;
  _fieldName = "unknown";

  _fixedAngle = 0;
  _nSamples = Radx::missingMetaInt;
  _nAngles = 0;
  _nGates = 0;
  _startRange = 0.0;
  _gateSpacing = 0.0;
  
  _angleRes = 0;
  _isIndexed = false;

  _prf = Radx::missingMetaDouble;
  _highPrf = Radx::missingMetaDouble;
  _lowPrf = Radx::missingMetaDouble;
  _isStaggered = false;

  _nyquist = Radx::missingMetaDouble;
  _pulseWidthIndex = 0;
  _pulseWidthUs = Radx::missingMetaDouble;
  _antennaSpeed = Radx::missingMetaDouble;
  
  _radarConst = Radx::missingMetaDouble;
  _radarConstH = Radx::missingMetaDouble;
  _radarConstV = Radx::missingMetaDouble;

  _xmitPeakPowerKw = Radx::missingMetaDouble;
  _ifMhz = Radx::missingMetaDouble;

  _noisePowerDbzH = Radx::missingMetaDouble;
  _noisePowerDbzV = Radx::missingMetaDouble;

  _minValue = 0;
  _maxValue = 0;
  
  _dataBlobId = 0;
  _anglesBlobId = 0;
  
  _dataByteWidth = 0;
  _anglesByteWidth = 0;

  _angles.clear();

  if (_fieldData != NULL) {
    delete [] _fieldData;
    _fieldData = NULL;
  }

}

//////////////////
// print

void GemSweep::print(ostream &out) const
  
{

  out << "Sweep number: " << _num << endl;

  out << "  Start time: " << RadxTime::strm(_startTime) << endl;
  out << "  FieldName: " << _fieldName << endl;

  out << "  fixedAngle: " << _fixedAngle << endl;
  out << "  nAngles: " << _nAngles << endl;
  out << "  nSamples: " << _nSamples << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  startRange: " << _startRange << endl;
  out << "  gateSpacing: " << _gateSpacing << endl;

  out << "  angleRes: " << _angleRes << endl;
  out << "  isIndexed: " << (_isIndexed?"Y":"N") << endl;

  out << "  prf: " << _prf << endl;
  out << "  highPrf: " << _highPrf << endl;
  out << "  lowPrf: " << _lowPrf << endl;
  out << "  isStaggered: " << (_isStaggered?"Y":"N") << endl;

  out << "  nyquist: " << _nyquist << endl;
  out << "  pulseWidthIndex: " << _pulseWidthIndex << endl;
  out << "  pulseWidthUs: " << _pulseWidthUs << endl;
  out << "  antennaSpeed: " << _antennaSpeed << endl;

  out << "  radarConst: " << _radarConst << endl;
  out << "  radarConstH: " << _radarConstH << endl;
  out << "  radarConstV: " << _radarConstV << endl;

  out << "  xmitPeakPowerKw: " << _xmitPeakPowerKw << endl;
  out << "  ifMhz: " << _ifMhz << endl;

  out << "  noisePowerDbzH: " << _noisePowerDbzH << endl;
  out << "  noisePowerDbzV: " << _noisePowerDbzV << endl;

  out << "  minValue: " << _minValue << endl;
  out << "  maxValue: " << _maxValue << endl;

  out << "  dataBlobId: " << _dataBlobId << endl;
  out << "  anglesBlobId: " << _anglesBlobId << endl;

  out << "  dataByteWidth: " << _dataByteWidth << endl;
  out << "  anglesByteWidth: " << _anglesByteWidth << endl;

}

/////////////////////////////////////
// decode info from XML
// returns 0 on success, -1 on failure

int GemSweep::decodeInfoXml(const string &xmlBuf)
  
{

  if (_verbose) {
    cerr << "--->>> Decoding XML for sweep: " << _num << endl;
    cerr << "===XML===XML===XML===XML===XML===XML===" << endl;
    cerr << xmlBuf << endl;
    cerr << "===XML===XML===XML===XML===XML===XML===" << endl;
  }

  // slice

  vector<RadxXml::attribute> attributes;
  string sliceBuf;
  if (RadxXml::readString(xmlBuf, "slice", sliceBuf, attributes)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Sweep num: " << _num << endl;
    cerr << "  Cannot find <slice> tag" << endl;
    return -1;
  }

  int num;
  if (RadxXml::readIntAttr(attributes, "refid", num) == 0) {
    if (num != _num) {
      cerr << "WARNING - GemSweep::decodeXml" << endl;
      cerr << "  Sweep num incorrect: " << num << endl;
      cerr << "  Should be: " << _num << endl;
    }
  }

  // fixed angle

  if (RadxXml::readDouble(sliceBuf, "posangle", _fixedAngle)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Sweep num: " << _num << endl;
    cerr << "  Cannot set fixed angle <posangle>" << endl;
    return -1;
  }

  // gate geometry

  RadxXml::readDouble(sliceBuf, "rangestep", _gateSpacing);
  _startRange = _gateSpacing / 2.0;

  double startRange;
  if (RadxXml::readDouble(sliceBuf, "start_range", startRange) == 0) {
    _startRange = startRange + _gateSpacing / 2.0;
  }
  if (RadxXml::readDouble(sliceBuf, "anglestep", _angleRes) == 0) {
    _isIndexed = true;
  }
  
  // nsamples

  RadxXml::readInt(sliceBuf, "timesamp", _nSamples);

  // prf

  if (RadxXml::readDouble(sliceBuf, "highprf", _highPrf) == 0) {
    if (RadxXml::readDouble(sliceBuf, "lowprf", _lowPrf) == 0) {
      if (_highPrf != _lowPrf) {
        _isStaggered = true;
      }
    }
  }
  _prf = _highPrf;

  // pulse width index and value
  
  if (RadxXml::readInt(sliceBuf, "pw_index", _pulseWidthIndex) == 0) {
    _pulseWidthUs = _getPulseWidth(_pulseWidthIndex);
  }

  // antenna speed
  
  RadxXml::readDouble(sliceBuf, "antspeed", _antennaSpeed);
  
  // radar constant - based on pulse width index

  string listStr;
  if (RadxXml::readString(sliceBuf, "rspradconst", listStr) == 0) {
    _radarConst = _getValFromList(listStr, _pulseWidthIndex);
  }
  if (RadxXml::readString(sliceBuf, "rspdphradconst", listStr) == 0) {
    _radarConstH = _getValFromList(listStr, _pulseWidthIndex);
  }
  if (RadxXml::readString(sliceBuf, "rspdpvradconst", listStr) == 0) {
    _radarConstV = _getValFromList(listStr, _pulseWidthIndex);
  }

  // transmit power

  RadxXml::readDouble(sliceBuf, "gdrxmaxpowkw", _xmitPeakPowerKw);
  RadxXml::readDouble(sliceBuf, "noise_power_dbz", _noisePowerDbzH);
  RadxXml::readDouble(sliceBuf, "noise_power_dbz_dpv", _noisePowerDbzV);

  // IF frequency

  RadxXml::readDouble(sliceBuf, "gdrxanctxfreq", _ifMhz);

  // get nyquist from max velocity value

  attributes.clear();
  string dynvBuf;
  if (RadxXml::readString(sliceBuf, "dynv", dynvBuf, attributes) == 0) {
    double maxVel;
    if (RadxXml::readDoubleAttr(attributes, "max", maxVel) == 0) {
      _nyquist = maxVel;
    }
  }

  // slicedata

  attributes.clear();
  string sliceDataBuf;
  if (RadxXml::readString(sliceBuf, "slicedata", sliceDataBuf, attributes)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find slicedata section" << endl;
    return -1;
  }
  if (_decodeDateTime(attributes, _startTime)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find slicedata time" << endl;
    return -1;
  }
  
  // raw data

  attributes.clear();
  string rawDataBuf;
  if (RadxXml::readString(sliceDataBuf, "rawdata", rawDataBuf, attributes)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find rawdata attributes" << endl;
    return -1;
  }
  
  if (RadxXml::readIntAttr(attributes, "blobid", _dataBlobId)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find blobid attr for data" << endl;
    return -1;
  }
  if (RadxXml::readStringAttr(attributes, "type", _fieldName)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find type attr for data" << endl;
    return -1;
  }
  if (RadxXml::readIntAttr(attributes, "rays", _nAngles)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find rays attr for data" << endl;
    return -1;
  }
  if (RadxXml::readIntAttr(attributes, "bins", _nGates)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find bins attr for data" << endl;
    return -1;
  }
  int dataBitWidth;
  if (RadxXml::readIntAttr(attributes, "depth", dataBitWidth)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find depth for data" << endl;
    return -1;
  }
  _dataByteWidth = dataBitWidth / 8;
  if (RadxXml::readDoubleAttr(attributes, "min", _minValue)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find min attr for data" << endl;
    return -1;
  }
  if (RadxXml::readDoubleAttr(attributes, "max", _maxValue)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find max attr for data" << endl;
    return -1;
  }

  // ray info

  attributes.clear();
  string rayinfoBuf;
  if (RadxXml::readString(sliceDataBuf, "rayinfo", rayinfoBuf, attributes)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find rayinfo attributes" << endl;
    return -1;
  }
  
  if (RadxXml::readIntAttr(attributes, "blobid", _anglesBlobId)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find blobid attr for angles" << endl;
    return -1;
  }
  int anglesBitWidth;
  if (RadxXml::readIntAttr(attributes, "depth", anglesBitWidth)) {
    cerr << "ERROR - GemSweep::decodeXml" << endl;
    cerr << "  Cannot find depth for angles" << endl;
    return -1;
  }
  _anglesByteWidth = anglesBitWidth / 8;

  return 0;

}

////////////////////////////////////////
// decode date and time from attributes

int GemSweep::_decodeDateTime(const vector<RadxXml::attribute> &attributes,
                              time_t &time)
  
  
{
  
  string dateStr, timeStr;
  if (RadxXml::readStringAttr(attributes, "date", dateStr)) {
    cerr << "  Cannot find date attribute" << endl;
    return -1;
  }
  if (RadxXml::readStringAttr(attributes, "time", timeStr)) {
    cerr << "  Cannot find time attribute" << endl;
    return -1;
  }
  int year, month, day, hour, min, sec;
  if (sscanf(dateStr.c_str(), "%4d-%2d-%2d", &year, &month, &day) != 3) {
    cerr << "  Cannot decode date attribute" << endl;
    return -1;
  }
  if (sscanf(timeStr.c_str(), "%2d:%2d:%2d", &hour, &min, &sec) != 3) {
    cerr << "  Cannot decode time attribute" << endl;
    return -1;
  }
  RadxTime volTime(year, month, day, hour, min, sec);
  time = volTime.utime();
  return 0;

}

////////////////////////////////////////
// set the angle array from a GemBlob

int GemSweep::setAngles(const GemBlob &blob)

{

  _angles.clear();

  int nBytesAvail = blob.getSize();
  int nBytesNeeded = _nAngles * _anglesByteWidth;
  if (nBytesNeeded != nBytesAvail) {
    cerr << "ERROR - GemSweep::setAngles" << endl;
    cerr << "  Cannot set angles, nbytes do not match" << endl;
    cerr << "  nBytesNeeded: " << nBytesNeeded << endl;
    cerr << "  nBytesAvail: " << nBytesAvail << endl;
    cerr << "  fixed angle: " << _fixedAngle << endl;
    return -1;
  }

  if (_anglesByteWidth == 2) {
    const Radx::ui16 *shorts = (Radx::ui16 *) blob.getData();
    Radx::ui16 *copy = new Radx::ui16[_nAngles];
    memcpy(copy, shorts, nBytesNeeded);
    ByteOrder::swap16(copy, nBytesNeeded);
    for (int ii = 0; ii < _nAngles; ii++) {
      double angle = (double) copy[ii] * (360.0 / 65536.0);
      _angles.push_back(angle);
    }
    delete[] copy;
  } else {
    const Radx::ui08 *bytes = (Radx::ui08 *) blob.getData();
    for (int ii = 0; ii < _nAngles; ii++) {
      double angle = (double) bytes[ii] * (360.0 / 256);
      _angles.push_back(angle);
    }
  }

  if (_verbose) {
    cerr << "Sweep fixed angle: " << _fixedAngle << ", angles: ";
    for (int ii = 0; ii < (int) _angles.size(); ii++) {
      cerr << " " << _angles[ii];
    }
    cerr << endl;
  }

  return 0;

}

////////////////////////////////////////
// set the field data from a GemBlob

int GemSweep::setFieldData(const GemBlob &blob)

{

  if (_fieldData != NULL) {
    delete[] _fieldData;
    _fieldData = NULL;
  }

  int nPts = _nAngles * _nGates;
  int nBytesAvail = blob.getSize();
  int nBytesNeeded = nPts * _dataByteWidth;
  if (nBytesNeeded != nBytesAvail) {
    cerr << "ERROR - GemSweep::setFieldData" << endl;
    cerr << "  Cannot set field data, nbytes do not match" << endl;
    cerr << "  nBytesNeeded: " << nBytesNeeded << endl;
    cerr << "  nBytesAvail: " << nBytesAvail << endl;
    cerr << "  fixed angle: " << _fixedAngle << endl;
    return -1;
  }

  if (_dataByteWidth == 2) {
    const Radx::ui16 *shorts = (Radx::ui16 *) blob.getData();
    _fieldData = new Radx::ui08[nPts * 2];
    memcpy(_fieldData, shorts, nBytesNeeded);
    ByteOrder::swap16(_fieldData, nBytesNeeded);
  } else {
    const Radx::ui08 *bytes = (Radx::ui08 *) blob.getData();
    _fieldData = new Radx::ui08[nPts];
    memcpy(_fieldData, bytes, nBytesNeeded);
  }

  return 0;
  
}

////////////////////////////////////////
// initialize pulse width vector

void GemSweep::_initPulseWidths()

{

  _pulseWidths.clear();

  const char *widthStr = getenv("GEMATRONIK_PULSE_WIDTHS");
  if (widthStr == NULL) {
    widthStr = "0.5 1.0 2.0";
  }

  if (_parseList(widthStr, _pulseWidths)) {
    cerr << "ERROR - GemSweep::_initPulseWidths()" << endl;
    cerr << "  Cannot properly parse pulse widths" << endl;
  }

}

////////////////////////////////////////
// get the pulse width from the index

double GemSweep::_getPulseWidth(int index)

{

  if (_pulseWidths.size() == 0) {
    return Radx::missingMetaDouble;
  }

  if (index < 0) {
    return _pulseWidths[0];
  } else if (index > ((int) _pulseWidths.size() - 1)) {
    return _pulseWidths[_pulseWidths.size() - 1];
  } else {
    return _pulseWidths[index];
  }
    
}

////////////////////////////////////////
// get value from list

double GemSweep::_getValFromList(const string &listStr, int index)

{

  vector<double> vals;
  if (_parseList(listStr.c_str(), vals)) {
    return Radx::missingMetaDouble;
  }
  
  if (index < 0) {
    return vals[0];
  } else if (index > ((int) vals.size() - 1)) {
    return vals[vals.size() - 1];
  } else {
    return vals[index];
  }
    
}

////////////////////////////////////////////////////////
// parse a space-delimited or comma-delimited list of 
// entries into a vector

int GemSweep::_parseList(const char *listStr,
                         vector<double> &list)

{

  int iret = 0;

  vector<string> toks;
  RadxStr::tokenize(listStr, " ,", toks);

  list.clear();
  
  for (size_t ii = 0; ii < toks.size(); ii++) {
    double val;
    if (sscanf(toks[ii].c_str(), "%lg", &val) == 1) {
      list.push_back(val);
    } else {
      cerr << "ERROR - GemSweep::_parseList" << endl;
      cerr << "  Cannot parse list: " << listStr << endl;
      cerr << "  Bad token: " << toks[ii] << endl;
      iret = -1;
    }
  }

  return iret;

}


