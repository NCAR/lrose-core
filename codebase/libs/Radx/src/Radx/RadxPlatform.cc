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

