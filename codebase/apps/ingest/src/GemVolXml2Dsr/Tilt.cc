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
// Tilt.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#include "Tilt.hh"
#include "Blob.hh"
#include <cerrno>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
using namespace std;

// Constructor

Tilt::Tilt(const Params &params, int num) :
        _params(params),
        _num(num),
        _fieldData(NULL)

{

  clear();
  
}

// Copy constructor

Tilt::Tilt(const Tilt &orig, int num) :
        _params(orig._params),
        _num(num),
        _fieldData(NULL)

{

  clear();

  _startTime = orig._startTime;
  _fieldName = orig._fieldName;
  _elev = orig._elev;
  _nAz = orig._nAz;
  _nSamples = orig._nSamples;
  _nGates = orig._nGates;
  _startRange = orig._startRange;
  _gateSpacing = orig._gateSpacing;
  _prf = orig._prf;
  _pulseWidth = orig._pulseWidth;
  _antennaSpeed = orig._antennaSpeed;
  _minValue = orig._minValue;
  _maxValue = orig._maxValue;
  _dataBlobId = orig._dataBlobId;
  _anglesBlobId = orig._anglesBlobId;
  _dataByteWidth = orig._dataByteWidth;
  _anglesByteWidth = orig._anglesByteWidth;
  
}

// destructor

Tilt::~Tilt()

{
  clear();
}

//////////////////
// clear data

void Tilt::clear()

{
  
  _startTime = 0;
  _fieldName = "unknown";

  _elev = 0;
  _nSamples = 64;
  _nAz = 0;
  _nGates = 0;
  _startRange = 0;
  _gateSpacing = 0.25;

  _prf = 999;
  _pulseWidth = _params.short_pulse_width_us;
  _antennaSpeed = 0;
  
  _radarConst = -9999;
  _radarConstH = -9999;
  _radarConstV = -9999;
  _xmitPeakPowerKw = -9999;
  _noisePowerDbz = -9999;

  _minValue = 0;
  _maxValue = 0;
  
  _dataBlobId = 0;
  _anglesBlobId = 0;
  
  _dataByteWidth = 0;
  _anglesByteWidth = 0;

  _azAngles.clear();

  if (_fieldData != NULL) {
    delete [] _fieldData;
    _fieldData = NULL;
  }

}

//////////////////
// print

void Tilt::print(ostream &out) const
  
{

  out << "Tilt number: " << _num << endl;
  out << "  Start time: " << DateTime::strm(_startTime) << endl;
  out << "  FieldName: " << _fieldName << endl;
  out << "  nSamples: " << _nSamples << endl;
  out << "  elev: " << _elev << endl;
  out << "  nAz: " << _nAz << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  prf: " << _prf << endl;
  out << "  pulseWidth: " << _pulseWidth << endl;
  out << "  antennaSpeed: " << _antennaSpeed << endl;
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

int Tilt::decodeInfoXml(const string &xmlBuf)
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--->>> Decoding XML for tilt: " << _num << endl;
    cerr << "===XML===XML===XML===XML===XML===XML===" << endl;
    cerr << xmlBuf << endl;
    cerr << "===XML===XML===XML===XML===XML===XML===" << endl;
  }

  // slice

  vector<TaXml::attribute> attributes;
  string sliceBuf;
  if (TaXml::readString(xmlBuf, "slice", sliceBuf, attributes)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Tilt num: " << _num << endl;
    cerr << "  Cannot find <slice> tag" << endl;
    return -1;
  }

  int num;
  if (TaXml::readIntAttr(attributes, "refid", num) == 0) {
    if (num != _num) {
      cerr << "WARNING - Tilt::decodeXml" << endl;
      cerr << "  Tilt num incorrect: " << num << endl;
      cerr << "  Should be: " << _num << endl;
    }
  }

  if (TaXml::readDouble(sliceBuf, "posangle", _elev)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Tilt num: " << _num << endl;
    cerr << "  Cannot set elevation angle <posangle>" << endl;
    return -1;
  }

  TaXml::readDouble(sliceBuf, "rangestep", _gateSpacing);
  _startRange = _gateSpacing / 2.0;

  double startRange;
  if (TaXml::readDouble(sliceBuf, "start_range", startRange) == 0) {
    _startRange = startRange + _gateSpacing / 2.0;
  }

  if (TaXml::readDouble(sliceBuf, "highprf", _prf)) {
    TaXml::readDouble(sliceBuf, "lowprf", _prf);
  }

  string pulseWidthStr;
  TaXml::readString(sliceBuf, "pulsewidth", pulseWidthStr);
  if (pulseWidthStr == "Long") {
    _pulseWidth = _params.long_pulse_width_us;
  } else {
    _pulseWidth = _params.short_pulse_width_us;
  }

  TaXml::readDouble(sliceBuf, "antspeed", _antennaSpeed);
  TaXml::readInt(sliceBuf, "timesamp", _nSamples);

  TaXml::readDouble(sliceBuf, "rspradconst", _radarConst);
  TaXml::readDouble(sliceBuf, "rspdphradconst", _radarConstH);
  TaXml::readDouble(sliceBuf, "rspdpvradconst", _radarConstV);
  TaXml::readDouble(sliceBuf, "rspdpvradconst", _radarConstV);
  TaXml::readDouble(sliceBuf, "gdrxmaxpowkw", _xmitPeakPowerKw);
  TaXml::readDouble(sliceBuf, "noise_power_dbz", _noisePowerDbz);

  // slicedata

  attributes.clear();
  string sliceDataBuf;
  if (TaXml::readString(sliceBuf, "slicedata", sliceDataBuf, attributes)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find slicedata section" << endl;
    return -1;
  }
  if (_decodeDateTime(attributes, _startTime)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find slicedata time" << endl;
    return -1;
  }
  
  // raw data

  attributes.clear();
  string rawDataBuf;
  if (TaXml::readString(sliceDataBuf, "rawdata", rawDataBuf, attributes)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find rawdata attributes" << endl;
    return -1;
  }
  
  if (TaXml::readIntAttr(attributes, "blobid", _dataBlobId)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find blobid attr for data" << endl;
    return -1;
  }
  if (TaXml::readStringAttr(attributes, "type", _fieldName)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find type attr for data" << endl;
    return -1;
  }
  if (TaXml::readIntAttr(attributes, "rays", _nAz)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find rays attr for data" << endl;
    return -1;
  }
  if (TaXml::readIntAttr(attributes, "bins", _nGates)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find bins attr for data" << endl;
    return -1;
  }
  int dataBitWidth;
  if (TaXml::readIntAttr(attributes, "depth", dataBitWidth)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find depth for data" << endl;
    return -1;
  }
  _dataByteWidth = dataBitWidth / 8;
  if (TaXml::readDoubleAttr(attributes, "min", _minValue)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find min attr for data" << endl;
    return -1;
  }
  if (TaXml::readDoubleAttr(attributes, "max", _maxValue)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find max attr for data" << endl;
    return -1;
  }

  // ray info

  attributes.clear();
  string rayinfoBuf;
  if (TaXml::readString(sliceDataBuf, "rayinfo", rayinfoBuf, attributes)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find rayinfo attributes" << endl;
    return -1;
  }
  
  if (TaXml::readIntAttr(attributes, "blobid", _anglesBlobId)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find blobid attr for angles" << endl;
    return -1;
  }
  int anglesBitWidth;
  if (TaXml::readIntAttr(attributes, "depth", anglesBitWidth)) {
    cerr << "ERROR - Tilt::decodeXml" << endl;
    cerr << "  Cannot find depth for angles" << endl;
    return -1;
  }
  _anglesByteWidth = anglesBitWidth / 8;

  return 0;

}

////////////////////////////////////////
// decode date and time from attributes

int Tilt::_decodeDateTime(const vector<TaXml::attribute> &attributes,
                          time_t &time)
  
  
{
  
  string dateStr, timeStr;
  if (TaXml::readStringAttr(attributes, "date", dateStr)) {
    cerr << "  Cannot find date attribute" << endl;
    return -1;
  }
  if (TaXml::readStringAttr(attributes, "time", timeStr)) {
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
  DateTime volTime(year, month, day, hour, min, sec);
  time = volTime.utime()  + _params.input_file_time_offset_secs;
  return 0;

}

////////////////////////////////////////
// set the angle array from a Blob

int Tilt::setAzAngles(const Blob &blob)

{

  _azAngles.clear();

  int nBytesAvail = blob.getSize();
  int nBytesNeeded = _nAz * _anglesByteWidth;
  if (nBytesNeeded != nBytesAvail) {
    cerr << "ERROR - Tilt::setAzAngle" << endl;
    cerr << "  Cannot set angles, nbytes do not match" << endl;
    cerr << "  nBytesNeeded: " << nBytesNeeded << endl;
    cerr << "  nBytesAvail: " << nBytesAvail << endl;
    cerr << "  elev angle: " << _elev << endl;
    return -1;
  }

  if (_anglesByteWidth == 2) {
    const ui16 *shorts = (ui16 *) blob.getData();
    ui16 *copy = new ui16[_nAz];
    memcpy(copy, shorts, nBytesNeeded);
    BE_from_array_16(copy, nBytesNeeded);
    for (int ii = 0; ii < _nAz; ii++) {
      double az = (double) copy[ii] * (360.0 / 65536.0);
      _azAngles.push_back(az);
    }
    delete[] copy;
  } else {
    const ui08 *bytes = (ui08 *) blob.getData();
    for (int ii = 0; ii < _nAz; ii++) {
      double az = (double) bytes[ii] * (360.0 / 256);
      _azAngles.push_back(az);
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Tilt elev: " << _elev << ", azimuths: ";
    for (int ii = 0; ii < (int) _azAngles.size(); ii++) {
      cerr << " " << _azAngles[ii];
    }
    cerr << endl;
  }

  return 0;

}

////////////////////////////////////////
// set the field data from a Blob

int Tilt::setFieldData(const Blob &blob)

{

  if (_fieldData != NULL) {
    delete[] _fieldData;
    _fieldData = NULL;
  }

  int nPts = _nAz * _nGates;
  int nBytesAvail = blob.getSize();
  int nBytesNeeded = nPts * _dataByteWidth;
  if (nBytesNeeded != nBytesAvail) {
    cerr << "ERROR - Tilt::setFieldData" << endl;
    cerr << "  Cannot set field data, nbytes do not match" << endl;
    cerr << "  nBytesNeeded: " << nBytesNeeded << endl;
    cerr << "  nBytesAvail: " << nBytesAvail << endl;
    cerr << "  elev angle: " << _elev << endl;
    return -1;
  }

  if (_dataByteWidth == 2) {
    const ui16 *shorts = (ui16 *) blob.getData();
    _fieldData = new ui08[nPts * 2];
    memcpy(_fieldData, shorts, nBytesNeeded);
    BE_from_array_16(_fieldData, nBytesNeeded);
  } else {
    const ui08 *bytes = (ui08 *) blob.getData();
    _fieldData = new ui08[nPts];
    memcpy(_fieldData, bytes, nBytesNeeded);
  }

  return 0;
  
}
