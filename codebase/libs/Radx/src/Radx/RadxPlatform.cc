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
// RadxPlatform.cc
//
// RadxPlatform object
//
// Parameters for the platform
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2012
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxPlatform.hh>
#include <Radx/RadxMsg.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
#include <iostream>
#include <cstring>
#include <cmath>
using namespace std;

//////////////
// Constructor

RadxPlatform::RadxPlatform()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxPlatform::RadxPlatform(const RadxPlatform &rhs)
     
{
  _init();
  _copy(rhs);
}

/////////////
// destructor

RadxPlatform::~RadxPlatform()

{

  clear();

}

/////////////////////////////
// Assignment
//

RadxPlatform &RadxPlatform::operator=(const RadxPlatform &rhs)
  

{
  return _copy(rhs);
}

//////////////////////////////////////////
/// Set the instrument name, if available.

void RadxPlatform::setInstrumentName(const string &val)
{
  _instrumentName = val;
  Radx::replaceSpacesWithUnderscores(_instrumentName);
}

/////////////////////////////////////
/// Set the site name, if available.

void RadxPlatform::setSiteName(const string &val)
{ 
  _siteName = val; 
  Radx::replaceSpacesWithUnderscores(_siteName);
}

// set radar and lidar parameters

void RadxPlatform::setRadarBeamWidthDegH(double val) { 
  _radarBeamWidthDegH = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
}
void RadxPlatform::setRadarBeamWidthDegV(double val) {
  _radarBeamWidthDegV = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
}
void RadxPlatform::setRadarAntennaGainDbH(double val) {
  _radarAntGainDbH = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
}
void RadxPlatform::setRadarAntennaGainDbV(double val) {
  _radarAntGainDbV = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
}
void RadxPlatform::setRadarReceiverBandwidthMhz(double val) {
  _radarReceiverBandwidthMhz = val;
}

void RadxPlatform::setIsLidar(bool val) {
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}

void RadxPlatform::setLidarConstant(double val) {
  _lidarConstant = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}
void RadxPlatform::setLidarPulseEnergyJ(double val) {
  _lidarPulseEnergyJ = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}
void RadxPlatform::setLidarPeakPowerW(double val) {
  _lidarPeakPowerW = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}
void RadxPlatform::setLidarApertureDiamCm(double val) {
  _lidarApertureDiamCm = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}
void RadxPlatform::setLidarApertureEfficiency(double val) {
  _lidarApertureEfficiency = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}
void RadxPlatform::setLidarFieldOfViewMrad(double val) {
  _lidarFieldOfViewMrad = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}
void RadxPlatform::setLidarBeamDivergenceMrad(double val) {
  _lidarBeamDivergenceMrad = val;
  _instrumentType = Radx::INSTRUMENT_TYPE_LIDAR;
}

/////////////////////////////////////////////////////////
// initialize data members

void RadxPlatform::_init()
  
{

  clearFrequency();

  _instrumentName = "unknown";
  _siteName = "unknown";
  _instrumentType = Radx::INSTRUMENT_TYPE_RADAR;
  _platformType = Radx::missingPlatformType;
  _primaryAxis = Radx::PRIMARY_AXIS_Z;

  _latitudeDeg = Radx::missingMetaDouble;
  _longitudeDeg = Radx::missingMetaDouble;
  _altitudeKm = Radx::missingMetaDouble;
  // _sensorHtAglM = Radx::missingMetaDouble;
  _sensorHtAglM = 0.0;

  _radarAntGainDbH = Radx::missingMetaDouble;
  _radarAntGainDbV = Radx::missingMetaDouble;
  _radarBeamWidthDegH = Radx::missingMetaDouble;
  _radarBeamWidthDegV = Radx::missingMetaDouble;
  _radarReceiverBandwidthMhz = Radx::missingMetaDouble;

  _lidarConstant = Radx::missingMetaDouble;
  _lidarPulseEnergyJ = Radx::missingMetaDouble;
  _lidarPeakPowerW = Radx::missingMetaDouble;
  _lidarApertureDiamCm = Radx::missingMetaDouble;
  _lidarApertureEfficiency = Radx::missingMetaDouble;
  _lidarFieldOfViewMrad = Radx::missingMetaDouble;
  _lidarBeamDivergenceMrad = Radx::missingMetaDouble;

}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxPlatform &RadxPlatform::_copy(const RadxPlatform &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }

  _instrumentName = rhs._instrumentName;
  _siteName = rhs._siteName;
  _instrumentType = rhs._instrumentType;
  _platformType = rhs._platformType;
  _primaryAxis = rhs._primaryAxis;

  _latitudeDeg = rhs._latitudeDeg;
  _longitudeDeg = rhs._longitudeDeg;
  _altitudeKm = rhs._altitudeKm;
  _sensorHtAglM = rhs._sensorHtAglM;

  _frequencyHz = rhs._frequencyHz;

  _radarAntGainDbH = rhs._radarAntGainDbH;
  _radarAntGainDbV = rhs._radarAntGainDbV;
  _radarBeamWidthDegH = rhs._radarBeamWidthDegH;
  _radarBeamWidthDegV = rhs._radarBeamWidthDegV;
  _radarReceiverBandwidthMhz = rhs._radarReceiverBandwidthMhz;

  _lidarConstant = rhs._lidarConstant;
  _lidarPulseEnergyJ = rhs._lidarPulseEnergyJ;
  _lidarPeakPowerW = rhs._lidarPeakPowerW;
  _lidarApertureDiamCm = rhs._lidarApertureDiamCm;
  _lidarApertureEfficiency = rhs._lidarApertureEfficiency;
  _lidarFieldOfViewMrad = rhs._lidarFieldOfViewMrad;
  _lidarBeamDivergenceMrad = rhs._lidarBeamDivergenceMrad;

  return *this;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxPlatform::clear()
  
{

  _init();

}

/////////////////////////////////////////////////////////
// clear the frequency list

void RadxPlatform::clearFrequency()
  
{
  _frequencyHz.clear();
}

/////////////////////////////////////////////////////////
// print

void RadxPlatform::print(ostream &out) const
  
{
  
  out << "--------------- RadxPlatform ---------------" << endl;
  out << "  instrumentName: " << _instrumentName << endl;
  out << "  siteName: " << _siteName << endl;
  out << "  instrumentType: "
      << Radx::instrumentTypeToStr(_instrumentType) << endl;
  out << "  platformType: " << Radx::platformTypeToStr(_platformType) << endl;
  out << "  primaryAxis: " << Radx::primaryAxisToStr(_primaryAxis) << endl;
  out << "  latitudeDeg: " << _latitudeDeg << endl;
  out << "  longitudeDeg: " << _longitudeDeg << endl;
  out << "  altitudeKm: " << _altitudeKm << endl;
  out << "  sensorHtAglM: " << _sensorHtAglM << endl;
  if (_frequencyHz.size() > 0) {
    out << "  frequencyHz:";
    for (size_t ii = 0; ii < _frequencyHz.size(); ii++) {
      out << " " << _frequencyHz[ii];
    }
    out << endl;
  }
  
  if (_instrumentType == Radx::INSTRUMENT_TYPE_RADAR) {

    out << "  radarAntGainDbH: " << _radarAntGainDbH << endl;
    out << "  radarAntGainDbV: " << _radarAntGainDbV << endl;
    out << "  radarBeamWidthDegH: " << _radarBeamWidthDegH << endl;
    out << "  radarBeamWidthDegV: " << _radarBeamWidthDegV << endl;
    out << "  radarReceiverBandwidthMhz: "
        << _radarReceiverBandwidthMhz << endl;

  } else {

    out << "  lidarConstant: " << _lidarConstant << endl;
    out << "  lidarPulseEnergyJ: " << _lidarPulseEnergyJ << endl;
    out << "  lidarPeakPowerW: " << _lidarPeakPowerW << endl;
    out << "  lidarApertureDiamCm: " << _lidarApertureDiamCm << endl;
    out << "  lidarApertureEfficiency: " << _lidarApertureEfficiency << endl;
    out << "  lidarFieldOfViewMrad: " << _lidarFieldOfViewMrad << endl;
    out << "  lidarBeamDivergenceMrad: " << _lidarBeamDivergenceMrad << endl;

  }

  out << "--------------------------------------------" << endl;

}

////////////////////////////////////////////////////////////
// set frequency or wavelength

void RadxPlatform::setFrequencyHz(double val)
{
  _frequencyHz.clear();
  addFrequencyHz(val);
}

void RadxPlatform::setWavelengthM(double val)

{
  _frequencyHz.clear();
  addWavelengthM(val);
}

void RadxPlatform::setWavelengthCm(double val)

{
  _frequencyHz.clear();
  addWavelengthCm(val);
}

////////////////////////////////////////////////////////////
// add frequency or wavelength

void RadxPlatform::addFrequencyHz(double val)
{
  for (size_t ii = 0; ii < _frequencyHz.size(); ii++) {
    if (fabs(val - _frequencyHz[ii]) < 0.001) {
      // already has this frequency, so don't add again
      return;
    }
  }
  _frequencyHz.push_back(val);
}

void RadxPlatform::addWavelengthM(double val)

{
  double freqHz = Radx::LIGHT_SPEED / val;
  addFrequencyHz(freqHz);
}

void RadxPlatform::addWavelengthCm(double val)

{
  addWavelengthM(val / 100.0);
}

////////////////////////////////////////////////////////////
// get wavelength

double RadxPlatform::getWavelengthM() const

{
  
  if (_frequencyHz.size() < 1) {
    return Radx::missingMetaDouble;
  }
  double wavelengthM = Radx::LIGHT_SPEED / _frequencyHz[0];
  return wavelengthM;

}

double RadxPlatform::getWavelengthCm() const

{
  
  if (_frequencyHz.size() < 1) {
    return Radx::missingMetaDouble;
  }
  double wavelengthM = Radx::LIGHT_SPEED / _frequencyHz[0];
  return wavelengthM * 100.0;

}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxPlatform::serialize(RadxMsg &msg)
  
{

  // init

  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxPlatformMsg);

  // add metadata strings as xml part
  // include null at string end
  
  string xml;
  _loadMetaStringsToXml(xml);
  msg.addPart(_metaStringsPartId, xml.c_str(), xml.size() + 1);

  // add metadata numbers

  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));
  
  // add frequency part

  if (_frequencyHz.size() > 0) {

    RadxArray<Radx::fl64> freq_;
    double *freq = freq_.alloc(_frequencyHz.size());
    for (size_t ii = 0; ii < _frequencyHz.size(); ii++) {
      freq[ii] = _frequencyHz[ii];
    }
    ByteOrder::swap64(freq, _frequencyHz.size() * sizeof(Radx::si64));
    msg.addPart(_frequencyPartId, freq, _frequencyHz.size() * sizeof(Radx::si64));
    
  }

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxPlatform::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  _init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxPlatformMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata strings

  const RadxMsg::Part *metaStringPart = msg.getPartByType(_metaStringsPartId);
  if (metaStringPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::deserialize" << endl;
    cerr << "  No metadata string part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaStringsFromXml((char *) metaStringPart->getBuf(),
                             metaStringPart->getLength())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "  Bad string XML for metadata: " << endl;
    string bufStr((char *) metaStringPart->getBuf(),
                  metaStringPart->getLength());
    cerr << "  " << bufStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata numbers
  
  const RadxMsg::Part *metaNumsPart = msg.getPartByType(_metaNumbersPartId);
  if (metaNumsPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the frequencies if available 

  const RadxMsg::Part *dataPart = msg.getPartByType(_frequencyPartId);
  if (dataPart != NULL) {
    size_t nFreq = dataPart->getLength() / sizeof(Radx::fl64);
    if (nFreq > 0) {
      RadxArray<Radx::fl64> freq_;
      double *freq = freq_.alloc(nFreq);
      memcpy(freq, dataPart->getBuf(), nFreq * sizeof(Radx::fl64));
      ByteOrder::swap64(freq, nFreq * sizeof(Radx::si64));
      for (size_t ii = 0; ii < nFreq; ii++) {
        _frequencyHz.push_back(freq[ii]);
      }
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////
// convert string metadata to XML

void RadxPlatform::_loadMetaStringsToXml(string &xml, int level /* = 0 */)  const
  
{
  xml.clear();
  xml += RadxXml::writeStartTag("RadxPlatform", level);
  xml += RadxXml::writeString("instrumentName", level + 1, _instrumentName);
  xml += RadxXml::writeString("siteName", level + 1, _siteName);
  xml += RadxXml::writeEndTag("RadxPlatform", level);
}

/////////////////////////////////////////////////////////
// set metadata strings from XML
// returns 0 on success, -1 on failure

int RadxPlatform::_setMetaStringsFromXml(const char *xml,
                                    size_t bufLen)

{

  // check for NULL
  
  if (xml[bufLen - 1] != '\0') {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::_setMetaStringsFromXml" << endl;
    cerr << "  XML string not null terminated" << endl;
    string xmlStr(xml, bufLen);
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;    
  }

  string xmlStr(xml);
  string contents;

  if (RadxXml::readString(xmlStr, "RadxPlatform", contents)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::_setMetaStringsFromXml" << endl;
    cerr << "  XML not delimited by 'RadxPlatform' tags" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  if (RadxXml::readString(contents, "instrumentName", _instrumentName)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::_setMetaStringsFromXml" << endl;
    cerr << "  Cannot find 'instrumentName' tag" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  if (RadxXml::readString(contents, "siteName", _siteName)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::_setMetaStringsFromXml" << endl;
    cerr << "  Cannot find 'siteName' tag" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxPlatform::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set 64 bit values

  _metaNumbers.latitudeDeg = _latitudeDeg;
  _metaNumbers.longitudeDeg = _longitudeDeg;
  _metaNumbers.altitudeKm = _altitudeKm;
  _metaNumbers.sensorHtAglM = _sensorHtAglM;
    
  _metaNumbers.radarBeamWidthDegH = _radarBeamWidthDegH;
  _metaNumbers.radarBeamWidthDegV = _radarBeamWidthDegV;
  _metaNumbers.radarAntGainDbH = _radarAntGainDbH;
  _metaNumbers.radarAntGainDbV = _radarAntGainDbV;
  _metaNumbers.radarReceiverBandwidthMhz = _radarReceiverBandwidthMhz;

  _metaNumbers.lidarConstant = _lidarConstant;
  _metaNumbers.lidarPulseEnergyJ = _lidarPulseEnergyJ;
  _metaNumbers.lidarPeakPowerW = _lidarPeakPowerW;
  _metaNumbers.lidarApertureDiamCm = _lidarApertureDiamCm;
  _metaNumbers.lidarApertureEfficiency = _lidarApertureEfficiency;
  _metaNumbers.lidarFieldOfViewMrad = _lidarFieldOfViewMrad;
  _metaNumbers.lidarBeamDivergenceMrad = _lidarBeamDivergenceMrad;

  // set 32-bit values

  _metaNumbers.instrumentType = _instrumentType;
  _metaNumbers.platformType = _platformType;
  _metaNumbers.primaryAxis = _primaryAxis;

}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxPlatform::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                    size_t bufLen,
                                    bool swap)
  
{

  // check size

  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxPlatform::_setMetaNumbersFromMsg" << endl;
    cerr << "  Incorrect message size: " << bufLen << endl;
    cerr << "  Should be: " << sizeof(msgMetaNumbers_t) << endl;
    return -1;
  }

  // copy into local struct
  
  _metaNumbers = *metaNumbers;

  // swap as needed

  if (swap) {
    _swapMetaNumbers(_metaNumbers); 
  }

  // set 64 bit values

  _latitudeDeg = _metaNumbers.latitudeDeg;
  _longitudeDeg = _metaNumbers.longitudeDeg;
  _altitudeKm = _metaNumbers.altitudeKm;
  _sensorHtAglM = _metaNumbers.sensorHtAglM;
    
  _radarBeamWidthDegH = _metaNumbers.radarBeamWidthDegH;
  _radarBeamWidthDegV = _metaNumbers.radarBeamWidthDegV;
  _radarAntGainDbH = _metaNumbers.radarAntGainDbH;
  _radarAntGainDbV = _metaNumbers.radarAntGainDbV;
  _radarReceiverBandwidthMhz = _metaNumbers.radarReceiverBandwidthMhz;

  _lidarConstant = _metaNumbers.lidarConstant;
  _lidarPulseEnergyJ = _metaNumbers.lidarPulseEnergyJ;
  _lidarPeakPowerW = _metaNumbers.lidarPeakPowerW;
  _lidarApertureDiamCm = _metaNumbers.lidarApertureDiamCm;
  _lidarApertureEfficiency = _metaNumbers.lidarApertureEfficiency;
  _lidarFieldOfViewMrad = _metaNumbers.lidarFieldOfViewMrad;
  _lidarBeamDivergenceMrad = _metaNumbers.lidarBeamDivergenceMrad;

  // set 32-bit values

  _instrumentType = (Radx::InstrumentType_t) _metaNumbers.instrumentType;
  _platformType = (Radx::PlatformType_t) _metaNumbers.platformType;
  _primaryAxis = (Radx::PrimaryAxis_t) _metaNumbers.primaryAxis;

  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxPlatform::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta.latitudeDeg, 24 * sizeof(Radx::si64));
  ByteOrder::swap32(&meta.instrumentType, 16 * sizeof(Radx::si32));
}
