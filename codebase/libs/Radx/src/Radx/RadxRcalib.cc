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
///////////////////////////////////////////////////////////////////////////////
//
// RadxRcalib.cc
//
// Radar calibration support
//
// Mike Dixon, EOL, NCAR, Bouler, CO, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////////////////////
//
// The convention in IWRF is that the radar constant is positive.
// 
// Computing radar constant:
//
// num = (1024.0 * log(2.0) * wavelengthM * wavelengthM);
//
// denom = (peakPowerMilliW * piCubed * pulseMeters * antGainSquared *
//          hBeamWidthRad * vBeamWidthRad * kSquared * 1.0e-24);
//
// factor = num / denom;
//  
// radarConst = (10.0 * log10(factor)
//               + twoWayWaveguideLossDb
//               + twoWayRadomeLossDb
//               + receiverMismatchLossDb);
//
// The power computations are as follows:
//
//    baseDbz1km = noiseDbm - rxGainDb + radarConst
//    dbz = baseDbz1km + SNR + 20*log(r)
//    pwrInWaveguide = rxPower - rxGainDb
//
// Here, baseDbz1km is the dbz at 1 km range with SNR = 0 dB.
//
///////////////////////////////////////////////////////////////////////////////

#include <Radx/RadxRcalib.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxXml.hh>
#include <Radx/RadxStr.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxMsg.hh>
#include <Radx/ByteOrder.hh>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <cerrno>
#include <sys/stat.h>
using namespace std;

///////////////
// constructor

RadxRcalib::RadxRcalib()
{
  init();
}

/////////////
// destructor

RadxRcalib::~RadxRcalib()
{
}

/////////////
// initialize

void RadxRcalib::init()
{
  _radarName.clear();
  _year = 1970;
  _month = 1;
  _day = 1;
  _hour = 0;
  _min = 0;
  _sec = 0;

  _wavelengthCm = Radx::missingMetaDouble;

  _beamWidthDegH = Radx::missingMetaDouble;
  _beamWidthDegV = Radx::missingMetaDouble;

  _antennaGainDbH = Radx::missingMetaDouble;
  _antennaGainDbV = Radx::missingMetaDouble;

  _pulseWidthUsec = Radx::missingMetaDouble;
  _xmitPowerDbmH = Radx::missingMetaDouble;
  _xmitPowerDbmV = Radx::missingMetaDouble;

  _twoWayWaveguideLossDbH = Radx::missingMetaDouble;
  _twoWayWaveguideLossDbV = Radx::missingMetaDouble;

  _twoWayRadomeLossDbH = Radx::missingMetaDouble;
  _twoWayRadomeLossDbV = Radx::missingMetaDouble;

  _receiverMismatchLossDb = Radx::missingMetaDouble;

  _kSquaredWater = Radx::missingMetaDouble;

  _radarConstH = Radx::missingMetaDouble;
  _radarConstV = Radx::missingMetaDouble;

  _noiseDbmHc = Radx::missingMetaDouble;
  _noiseDbmHx = Radx::missingMetaDouble;
  _noiseDbmVc = Radx::missingMetaDouble;
  _noiseDbmVx = Radx::missingMetaDouble;

  _i0DbmHc = Radx::missingMetaDouble;
  _i0DbmHx = Radx::missingMetaDouble;
  _i0DbmVc = Radx::missingMetaDouble;
  _i0DbmVx = Radx::missingMetaDouble;

  _receiverGainDbHc = Radx::missingMetaDouble;
  _receiverGainDbHx = Radx::missingMetaDouble;
  _receiverGainDbVc = Radx::missingMetaDouble;
  _receiverGainDbVx = Radx::missingMetaDouble;

  _receiverSlopeDbHc = Radx::missingMetaDouble;
  _receiverSlopeDbHx = Radx::missingMetaDouble;
  _receiverSlopeDbVc = Radx::missingMetaDouble;
  _receiverSlopeDbVx = Radx::missingMetaDouble;

  _dynamicRangeDbHc = Radx::missingMetaDouble;
  _dynamicRangeDbHx = Radx::missingMetaDouble;
  _dynamicRangeDbVc = Radx::missingMetaDouble;
  _dynamicRangeDbVx = Radx::missingMetaDouble;

  _baseDbz1kmHc = Radx::missingMetaDouble;
  _baseDbz1kmHx = Radx::missingMetaDouble;
  _baseDbz1kmVc = Radx::missingMetaDouble;
  _baseDbz1kmVx = Radx::missingMetaDouble;

  _sunPowerDbmHc = Radx::missingMetaDouble;
  _sunPowerDbmHx = Radx::missingMetaDouble;
  _sunPowerDbmVc = Radx::missingMetaDouble;
  _sunPowerDbmVx = Radx::missingMetaDouble;

  _noiseSourcePowerDbmH = Radx::missingMetaDouble;
  _noiseSourcePowerDbmV = Radx::missingMetaDouble;

  _powerMeasLossDbH = Radx::missingMetaDouble;
  _powerMeasLossDbV = Radx::missingMetaDouble;

  _couplerForwardLossDbH = Radx::missingMetaDouble;
  _couplerForwardLossDbV = Radx::missingMetaDouble;

  _dbzCorrection = 0;
  _zdrCorrectionDb = 0;
  _ldrCorrectionDbH = 0;
  _ldrCorrectionDbV = 0;
  _systemPhidpDeg = 0;

  _testPowerDbmH = Radx::missingMetaDouble;
  _testPowerDbmV = Radx::missingMetaDouble;

}

///////////////////////
// is a value missing?
//
// Compare with missing val

bool RadxRcalib::isMissing(double val)
{
  if (fabs(val - Radx::missingMetaDouble) > 0.001) {
    return true;
  }
  return false;
}

////////////////////////////////////////
// adjust based on pulse width and power

void RadxRcalib::adjustRadarConst(double pulseWidthUsec,
                                  double xmitPowerDbmH,
                                  double xmitPowerDbmV)

{

  // do not attempt this if data is missing

  if (isMissing(_pulseWidthUsec)) {
    return;
  }
  if (isMissing(_xmitPowerDbmH)) {
    return;
  }
  if (isMissing(_xmitPowerDbmV)) {
    return;
  }
  if (isMissing(_radarConstH)) {
    return;
  }
  if (isMissing(_radarConstV)) {
    return;
  }
  if (pulseWidthUsec <= 0) {
    return;
  }
  if (xmitPowerDbmH <= 0 || xmitPowerDbmV <= 0) {
    return;
  }

  // compute correction ratios in dB

  double pulseWidthRatioDb
    = 10.0 * log10(pulseWidthUsec / _pulseWidthUsec);

  double xmitPowerRatioDbH = xmitPowerDbmH - _xmitPowerDbmH;
  double xmitPowerRatioDbV = xmitPowerDbmV - _xmitPowerDbmV;

  // adjust radar constant based on pulse width
  // decreases for increasing pulse width

  _radarConstH -= pulseWidthRatioDb;
  _radarConstV -= pulseWidthRatioDb;

  // adjust radar constant based on transmitter power
  // decreases for increasing power

  _radarConstH -= xmitPowerRatioDbH;
  _radarConstV -= xmitPowerRatioDbV;

  // compute base dbz

  if (!isMissing(_noiseDbmHc) && !isMissing(_receiverGainDbHc)) {
    _baseDbz1kmHc = (_noiseDbmHc - _receiverGainDbHc + _radarConstH);
  }

  if (!isMissing(_noiseDbmVc) && !isMissing(_receiverGainDbVc)) {
    _baseDbz1kmVc = (_noiseDbmVc - _receiverGainDbVc + _radarConstV);
  }

  if (!isMissing(_noiseDbmHx) && !isMissing(_receiverGainDbHx)) {
    _baseDbz1kmHx = (_noiseDbmHx - _receiverGainDbHx + _radarConstH);
  }

  if (!isMissing(_noiseDbmVx) && !isMissing(_receiverGainDbVx)) {
    _baseDbz1kmVx = (_noiseDbmVx - _receiverGainDbVx + _radarConstV);
  }

  // save xmit power

  _xmitPowerDbmH = xmitPowerDbmH;
  _xmitPowerDbmV = xmitPowerDbmV;
  
}

///////////////////////////
// set the calibration time

void RadxRcalib::setCalibTime(int year, int month, int day,
                              int hour, int min, int sec)
{
  _year = year;
  _month = month;
  _day = day;
  _hour = hour;
  _min = min;
  _sec = sec;
}

void RadxRcalib::setCalibTime(time_t time)
{
  RadxTime ctime(time);
  _year = ctime.getYear();
  _month = ctime.getMonth();
  _day = ctime.getDay();
  _hour = ctime.getHour();
  _min = ctime.getMin();
  _sec = ctime.getSec();
}

///////////////////////////
// get the calibration time

time_t RadxRcalib::getCalibTime() const
{
  RadxTime ctime(_year, _month, _day, _hour, _min, _sec);
  return ctime.utime();
}

////////
// print

void
  RadxRcalib::print(ostream &out)  const
{
   
  out << "RADAR CALIB" << endl;
  out << "------------" << endl;

  if (_radarName.size() > 0) {
    out << "  radarName: " << _radarName << endl;
  }

  out << "  time: " << RadxTime::strm(getCalibTime()) << endl;
  
  out << "  wavelengthCm: " << _wavelengthCm << endl;

  out << "  beamWidthDegH: " << _beamWidthDegH << endl;
  out << "  beamWidthDegV: " << _beamWidthDegV << endl;

  out << "  antennaGainDbH: " << _antennaGainDbH << endl;
  out << "  antennaGainDbV: " << _antennaGainDbV << endl;

  out << "  pulseWidthUsec: " << _pulseWidthUsec << endl;
  out << "  xmitPowerDbmH: " << _xmitPowerDbmH << endl;
  out << "  xmitPowerDbmV: " << _xmitPowerDbmV << endl;

  out << "  twoWayWaveguideLossDbH: " << _twoWayWaveguideLossDbH << endl;
  out << "  twoWayWaveguideLossDbV: " << _twoWayWaveguideLossDbV << endl;

  out << "  twoWayRadomeLossDbH: " << _twoWayRadomeLossDbH << endl;
  out << "  twoWayRadomeLossDbV: " << _twoWayRadomeLossDbV << endl;

  out << "  receiverMismatchLossDb: " << _receiverMismatchLossDb << endl;

  out << "  kSquaredWater: " << _kSquaredWater << endl;

  out << "  radarConstH: " << _radarConstH << endl;
  out << "  radarConstV: " << _radarConstV << endl;

  out << "  antennaGainDbH: " << _antennaGainDbH << endl;
  out << "  antennaGainDbV: " << _antennaGainDbV << endl;

  out << "  noiseDbmHc: " << _noiseDbmHc << endl;
  out << "  noiseDbmHx: " << _noiseDbmHx << endl;
  out << "  noiseDbmVc: " << _noiseDbmVc << endl;
  out << "  noiseDbmVx: " << _noiseDbmVx << endl;

  out << "  i0DbmHc: " << _i0DbmHc << endl;
  out << "  i0DbmHx: " << _i0DbmHx << endl;
  out << "  i0DbmVc: " << _i0DbmVc << endl;
  out << "  i0DbmVx: " << _i0DbmVx << endl;

  out << "  receiverGainDbHc: " << _receiverGainDbHc << endl;
  out << "  receiverGainDbHx: " << _receiverGainDbHx << endl;
  out << "  receiverGainDbVc: " << _receiverGainDbVc << endl;
  out << "  receiverGainDbVx: " << _receiverGainDbVx << endl;

  out << "  receiverSlopeDbHc: " << _receiverSlopeDbHc << endl;
  out << "  receiverSlopeDbHx: " << _receiverSlopeDbHx << endl;
  out << "  receiverSlopeDbVc: " << _receiverSlopeDbVc << endl;
  out << "  receiverSlopeDbVx: " << _receiverSlopeDbVx << endl;

  out << "  dynamicRangeDbHc: " << _dynamicRangeDbHc << endl;
  out << "  dynamicRangeDbHx: " << _dynamicRangeDbHx << endl;
  out << "  dynamicRangeDbVc: " << _dynamicRangeDbVc << endl;
  out << "  dynamicRangeDbVx: " << _dynamicRangeDbVx << endl;

  out << "  baseDbz1kmHc: " << _baseDbz1kmHc << endl;
  out << "  baseDbz1kmHx: " << _baseDbz1kmHx << endl;
  out << "  baseDbz1kmVc: " << _baseDbz1kmVc << endl;
  out << "  baseDbz1kmVx: " << _baseDbz1kmVx << endl;

  out << "  sunPowerDbmHc: " << _sunPowerDbmHc << endl;
  out << "  sunPowerDbmHx: " << _sunPowerDbmHx << endl;
  out << "  sunPowerDbmVc: " << _sunPowerDbmVc << endl;
  out << "  sunPowerDbmVx: " << _sunPowerDbmVx << endl;

  out << "  noiseSourcePowerDbmH: " << _noiseSourcePowerDbmH << endl;
  out << "  noiseSourcePowerDbmV: " << _noiseSourcePowerDbmV << endl;

  out << "  powerMeasLossDbH: " << _powerMeasLossDbH << endl;
  out << "  powerMeasLossDbV: " << _powerMeasLossDbV << endl;

  out << "  couplerForwardLossDbH: " << _couplerForwardLossDbH << endl;
  out << "  couplerForwardLossDbV: " << _couplerForwardLossDbV << endl;

  out << "  dbzCorrection: " << _dbzCorrection << endl;
  out << "  zdrCorrectionDb: " << _zdrCorrectionDb << endl;
  out << "  ldrCorrectionDbH: " << _ldrCorrectionDbH << endl;
  out << "  ldrCorrectionDbV: " << _ldrCorrectionDbV << endl;
  out << "  systemPhidpDeg: " << _systemPhidpDeg << endl;

  out << "  testPowerDbmH: " << _testPowerDbmH << endl;
  out << "  testPowerDbmV: " << _testPowerDbmV << endl;

  out << endl;

  string xml;
  convert2Xml(xml);
  out << xml;

}

////////////////////////////////////////////
// convert to XML - load up xml string

void RadxRcalib::convert2Xml(string &xml)  const
{

  xml.clear();
  xml += RadxXml::writeStartTag("RadxRcalib", 0);

  // name

  xml += RadxXml::writeString("radarName", 1, _radarName);
  
  // time
  
  xml += RadxXml::writeTime("calibTime", 1, getCalibTime());
  
  // fields
  
  xml += RadxXml::writeDouble("wavelengthCm", 1, _wavelengthCm);
  xml += RadxXml::writeDouble("beamWidthDegH", 1, _beamWidthDegH);
  xml += RadxXml::writeDouble("beamWidthDegV", 1, _beamWidthDegV);
  xml += RadxXml::writeDouble("antGainDbH", 1, _antennaGainDbH);
  xml += RadxXml::writeDouble("antGainDbV", 1, _antennaGainDbV);
  xml += RadxXml::writeDouble("pulseWidthUs", 1, _pulseWidthUsec);
  xml += RadxXml::writeDouble("xmitPowerDbmH", 1, _xmitPowerDbmH);
  xml += RadxXml::writeDouble("xmitPowerDbmV", 1, _xmitPowerDbmV);
  xml += RadxXml::writeDouble("twoWayWaveguideLossDbH",
                              1, _twoWayWaveguideLossDbH);
  xml += RadxXml::writeDouble("twoWayWaveguideLossDbV",
                              1, _twoWayWaveguideLossDbV);
  xml += RadxXml::writeDouble("twoWayRadomeLossDbH",
                              1, _twoWayRadomeLossDbH);
  xml += RadxXml::writeDouble("twoWayRadomeLossDbV",
                              1, _twoWayRadomeLossDbV);
  xml += RadxXml::writeDouble("receiverMismatchLossDb",
                              1, _receiverMismatchLossDb);
  xml += RadxXml::writeDouble("kSquaredWater", 1, _kSquaredWater);
  xml += RadxXml::writeDouble("radarConstH", 1, _radarConstH);
  xml += RadxXml::writeDouble("radarConstV", 1, _radarConstV);
  xml += RadxXml::writeDouble("noiseDbmHc", 1, _noiseDbmHc);
  xml += RadxXml::writeDouble("noiseDbmHx", 1, _noiseDbmHx);
  xml += RadxXml::writeDouble("noiseDbmVc", 1, _noiseDbmVc);
  xml += RadxXml::writeDouble("noiseDbmVx", 1, _noiseDbmVx);
  xml += RadxXml::writeDouble("i0DbmHc", 1, _i0DbmHc);
  xml += RadxXml::writeDouble("i0DbmHx", 1, _i0DbmHx);
  xml += RadxXml::writeDouble("i0DbmVc", 1, _i0DbmVc);
  xml += RadxXml::writeDouble("i0DbmVx", 1, _i0DbmVx);
  xml += RadxXml::writeDouble("receiverGainDbHc", 1, _receiverGainDbHc);
  xml += RadxXml::writeDouble("receiverGainDbHx", 1, _receiverGainDbHx);
  xml += RadxXml::writeDouble("receiverGainDbVc", 1, _receiverGainDbVc);
  xml += RadxXml::writeDouble("receiverGainDbVx", 1, _receiverGainDbVx);
  xml += RadxXml::writeDouble("receiverSlopeDbHc", 1, _receiverSlopeDbHc);
  xml += RadxXml::writeDouble("receiverSlopeDbHx", 1, _receiverSlopeDbHx);
  xml += RadxXml::writeDouble("receiverSlopeDbVc", 1, _receiverSlopeDbVc);
  xml += RadxXml::writeDouble("receiverSlopeDbVx", 1, _receiverSlopeDbVx);
  xml += RadxXml::writeDouble("dynamicRangeDbHc", 1, _dynamicRangeDbHc);
  xml += RadxXml::writeDouble("dynamicRangeDbHx", 1, _dynamicRangeDbHx);
  xml += RadxXml::writeDouble("dynamicRangeDbVc", 1, _dynamicRangeDbVc);
  xml += RadxXml::writeDouble("dynamicRangeDbVx", 1, _dynamicRangeDbVx);
  xml += RadxXml::writeDouble("baseDbz1kmHc", 1, _baseDbz1kmHc);
  xml += RadxXml::writeDouble("baseDbz1kmHx", 1, _baseDbz1kmHx);
  xml += RadxXml::writeDouble("baseDbz1kmVc", 1, _baseDbz1kmVc);
  xml += RadxXml::writeDouble("baseDbz1kmVx", 1, _baseDbz1kmVx);
  xml += RadxXml::writeDouble("sunPowerDbmHc", 1, _sunPowerDbmHc);
  xml += RadxXml::writeDouble("sunPowerDbmHx", 1, _sunPowerDbmHx);
  xml += RadxXml::writeDouble("sunPowerDbmVc", 1, _sunPowerDbmVc);
  xml += RadxXml::writeDouble("sunPowerDbmVx", 1, _sunPowerDbmVx);
  xml += RadxXml::writeDouble("noiseSourcePowerDbmH",
                              1, _noiseSourcePowerDbmH);
  xml += RadxXml::writeDouble("noiseSourcePowerDbmV",
                              1, _noiseSourcePowerDbmV);
  xml += RadxXml::writeDouble("powerMeasLossDbH", 1, _powerMeasLossDbH);
  xml += RadxXml::writeDouble("powerMeasLossDbV", 1, _powerMeasLossDbV);
  xml += RadxXml::writeDouble("couplerForwardLossDbH",
                              1, _couplerForwardLossDbH);
  xml += RadxXml::writeDouble("couplerForwardLossDbV",
                              1, _couplerForwardLossDbV);
  xml += RadxXml::writeDouble("dbzCorrection", 1, _dbzCorrection);
  xml += RadxXml::writeDouble("zdrCorrectionDb", 1, _zdrCorrectionDb);
  xml += RadxXml::writeDouble("ldrCorrectionDbH", 1, _ldrCorrectionDbH);
  xml += RadxXml::writeDouble("ldrCorrectionDbV", 1, _ldrCorrectionDbV);
  xml += RadxXml::writeDouble("systemPhidpDeg", 1, _systemPhidpDeg);
  xml += RadxXml::writeDouble("testPowerDbmH", 1, _testPowerDbmH);
  xml += RadxXml::writeDouble("testPowerDbmV", 1, _testPowerDbmV);

  xml += RadxXml::writeEndTag("RadxRcalib", 0);

}

////////////////////////////////////////////
// set from XML string

void RadxRcalib::setFromXml(const string &xmlBuf,
                            bool doInit /* = false */)
{
  
  if (doInit) {
    init();
  }
  
  string xbuf = RadxXml::removeComments(xmlBuf);

  // set data
  
  string name;
  if (RadxXml::readString(xbuf, "radarName", name) == 0) {
    _radarName = name;
  }

  time_t ctime;
  if (RadxXml::readTime(xbuf, "calibTime", ctime) == 0) {
    RadxTime cTime(ctime); 
    _year = cTime.getYear();
    _month = cTime.getMonth();
    _day = cTime.getDay();
    _hour = cTime.getHour();
    _min = cTime.getMin();
    _sec = cTime.getSec();
  }
  
  double val;

  if (RadxXml::readDouble(xbuf, "wavelengthCm", val) == 0) {
    _wavelengthCm = val;
  }

  if (RadxXml::readDouble(xbuf, "beamWidthDegH", val) == 0) {
    _beamWidthDegH = val;
  }

  if (RadxXml::readDouble(xbuf, "beamWidthDegV", val) == 0) {
    _beamWidthDegV = val;
  }

  if (RadxXml::readDouble(xbuf, "antGainDbH", val) == 0) {
    _antennaGainDbH = val;
  }

  if (RadxXml::readDouble(xbuf, "antGainDbV", val) == 0) {
    _antennaGainDbV = val;
  }

  if (RadxXml::readDouble(xbuf, "pulseWidthUs", val) == 0) {
    _pulseWidthUsec = val;
  }

  if (RadxXml::readDouble(xbuf, "xmitPowerDbmH", val) == 0) {
    _xmitPowerDbmH = val;
  }

  if (RadxXml::readDouble(xbuf, "xmitPowerDbmV", val) == 0) {
    _xmitPowerDbmV = val;
  }

  if (RadxXml::readDouble(xbuf, "twoWayWaveguideLossDbH", val) == 0) {
    _twoWayWaveguideLossDbH = val;
  }

  if (RadxXml::readDouble(xbuf, "twoWayWaveguideLossDbV", val) == 0) {
    _twoWayWaveguideLossDbV = val;
  }

  if (RadxXml::readDouble(xbuf, "twoWayRadomeLossDbH", val) == 0) {
    _twoWayRadomeLossDbH = val;
  }

  if (RadxXml::readDouble(xbuf, "twoWayRadomeLossDbV", val) == 0) {
    _twoWayRadomeLossDbV = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverMismatchLossDb", val) == 0) {
    _receiverMismatchLossDb = val;
  }

  if (RadxXml::readDouble(xbuf, "kSquaredWater", val) == 0) {
    _kSquaredWater = val;
  }

  if (RadxXml::readDouble(xbuf, "radarConstH", val) == 0) {
    _radarConstH = val;
  }

  if (RadxXml::readDouble(xbuf, "radarConstV", val) == 0) {
    _radarConstV = val;
  }

  if (RadxXml::readDouble(xbuf, "noiseDbmHc", val) == 0) {
    _noiseDbmHc = val;
  }

  if (RadxXml::readDouble(xbuf, "noiseDbmHx", val) == 0) {
    _noiseDbmHx = val;
  }

  if (RadxXml::readDouble(xbuf, "noiseDbmVc", val) == 0) {
    _noiseDbmVc = val;
  }

  if (RadxXml::readDouble(xbuf, "noiseDbmVx", val) == 0) {
    _noiseDbmVx = val;
  }

  if (RadxXml::readDouble(xbuf, "i0DbmHc", val) == 0) {
    _i0DbmHc = val;
  } else {
    _i0DbmHc = _noiseDbmHc - _receiverGainDbHc;
  }

  if (RadxXml::readDouble(xbuf, "i0DbmHx", val) == 0) {
    _i0DbmHx = val;
  } else {
    _i0DbmHx = _noiseDbmHx - _receiverGainDbHx;
  }

  if (RadxXml::readDouble(xbuf, "i0DbmVc", val) == 0) {
    _i0DbmVc = val;
  } else {
    _i0DbmVc = _noiseDbmVc - _receiverGainDbVc;
  }

  if (RadxXml::readDouble(xbuf, "i0DbmVx", val) == 0) {
    _i0DbmVx = val;
  } else {
    _i0DbmVx = _noiseDbmVx - _receiverGainDbVx;
  }

  if (RadxXml::readDouble(xbuf, "receiverGainDbHc", val) == 0) {
    _receiverGainDbHc = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverGainDbHx", val) == 0) {
    _receiverGainDbHx = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverGainDbVc", val) == 0) {
    _receiverGainDbVc = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverGainDbVx", val) == 0) {
    _receiverGainDbVx = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverSlopeDbHc", val) == 0) {
    _receiverSlopeDbHc = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverSlopeDbHx", val) == 0) {
    _receiverSlopeDbHx = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverSlopeDbVc", val) == 0) {
    _receiverSlopeDbVc = val;
  }

  if (RadxXml::readDouble(xbuf, "receiverSlopeDbVx", val) == 0) {
    _receiverSlopeDbVx = val;
  }

  if (RadxXml::readDouble(xbuf, "dynamicRangeDbHc", val) == 0) {
    _dynamicRangeDbHc = val;
  }

  if (RadxXml::readDouble(xbuf, "dynamicRangeDbHx", val) == 0) {
    _dynamicRangeDbHx = val;
  }

  if (RadxXml::readDouble(xbuf, "dynamicRangeDbVc", val) == 0) {
    _dynamicRangeDbVc = val;
  }

  if (RadxXml::readDouble(xbuf, "dynamicRangeDbVx", val) == 0) {
    _dynamicRangeDbVx = val;
  }

  if (RadxXml::readDouble(xbuf, "baseDbz1kmHc", val) == 0) {
    _baseDbz1kmHc = val;
  }

  if (RadxXml::readDouble(xbuf, "baseDbz1kmHx", val) == 0) {
    _baseDbz1kmHx = val;
  }

  if (RadxXml::readDouble(xbuf, "baseDbz1kmVc", val) == 0) {
    _baseDbz1kmVc = val;
  }

  if (RadxXml::readDouble(xbuf, "baseDbz1kmVx", val) == 0) {
    _baseDbz1kmVx = val;
  }

  if (RadxXml::readDouble(xbuf, "sunPowerDbmHc", val) == 0) {
    _sunPowerDbmHc = val;
  }

  if (RadxXml::readDouble(xbuf, "sunPowerDbmHx", val) == 0) {
    _sunPowerDbmHx = val;
  }

  if (RadxXml::readDouble(xbuf, "sunPowerDbmVc", val) == 0) {
    _sunPowerDbmVc = val;
  }

  if (RadxXml::readDouble(xbuf, "sunPowerDbmVx", val) == 0) {
    _sunPowerDbmVx = val;
  }

  if (RadxXml::readDouble(xbuf, "noiseSourcePowerDbmH", val) == 0) {
    _noiseSourcePowerDbmH = val;
  }

  if (RadxXml::readDouble(xbuf, "noiseSourcePowerDbmV", val) == 0) {
    _noiseSourcePowerDbmV = val;
  }

  if (RadxXml::readDouble(xbuf, "powerMeasLossDbH", val) == 0) {
    _powerMeasLossDbH = val;
  }

  if (RadxXml::readDouble(xbuf, "powerMeasLossDbV", val) == 0) {
    _powerMeasLossDbV = val;
  }

  if (RadxXml::readDouble(xbuf, "couplerForwardLossDbH", val) == 0) {
    _couplerForwardLossDbH = val;
  }

  if (RadxXml::readDouble(xbuf, "couplerForwardLossDbV", val) == 0) {
    _couplerForwardLossDbV = val;
  }

  if (RadxXml::readDouble(xbuf, "dbzCorrection", val) == 0) {
    _dbzCorrection = val;
  }

  if (RadxXml::readDouble(xbuf, "zdrCorrectionDb", val) == 0) {
    _zdrCorrectionDb = val;
  }

  if (RadxXml::readDouble(xbuf, "ldrCorrectionDbH", val) == 0) {
    _ldrCorrectionDbH = val;
  }

  if (RadxXml::readDouble(xbuf, "ldrCorrectionDbV", val) == 0) {
    _ldrCorrectionDbV = val;
  }

  if (RadxXml::readDouble(xbuf, "systemPhidpDeg", val) == 0) {
    _systemPhidpDeg = val;
  }

  if (RadxXml::readDouble(xbuf, "testPowerDbmH", val) == 0) {
    _testPowerDbmH = val;
  }

  if (RadxXml::readDouble(xbuf, "testPowerDbmV", val) == 0) {
    _testPowerDbmV = val;
  }

}

////////////////////////////////////////////////////////////////////////
// read from a given XML cal file
//
// Returns 0 on success, -1 on failure
// Sets errStr on failure

int RadxRcalib::readFromXmlFile(const string &calPath, string &errStr)

{

  errStr = "ERROR - RadxRcalib::readFromXmlFile\n";

  // Stat the file to get length
  
  struct stat calStat;
  if (stat(calPath.c_str(), &calStat)) {
    int errNum = errno;
    RadxStr::addStr(errStr, "  Cannot stat file: ", calPath);
    RadxStr::addStr(errStr, "  ", strerror(errNum));
    return -1;
  }
  size_t fileLen = calStat.st_size;

  // open file
  
  FILE *calFile;
  if ((calFile = fopen(calPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    RadxStr::addStr(errStr, "  Cannot open file for reading: ", calPath);
    RadxStr::addStr(errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // create buffer
  
  RadxArray<char> bufArray;
  char *xmlBuf = bufArray.alloc(fileLen + 1);
  memset(xmlBuf, 0, fileLen + 1);
  
  // read in buffer, close file
  
  if (fread(xmlBuf, 1, fileLen, calFile) != fileLen) {
    int errNum = errno;
    RadxStr::addStr(errStr, "  Cannot read from file: ", calPath);
    RadxStr::addStr(errStr, "  ", strerror(errNum));
    fclose(calFile);
    return -1;
  }
  fclose(calFile);

  // set from buffer

  setFromXml(xmlBuf);

  return 0;

}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxRcalib::serialize(RadxMsg &msg)
  
{

  // init
  
  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxRcalibMsg);

  // add metadata strings as xml part
  // include null at string end
  
  string xml;
  _loadMetaStringsToXml(xml);
  msg.addPart(_metaStringsPartId, xml.c_str(), xml.size() + 1);

  // add metadata numbers
  
  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));
  
}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxRcalib::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxRcalibMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata strings

  const RadxMsg::Part *metaStringPart = msg.getPartByType(_metaStringsPartId);
  if (metaStringPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::deserialize" << endl;
    cerr << "  No metadata string part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaStringsFromXml((char *) metaStringPart->getBuf(),
                             metaStringPart->getLength())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::deserialize" << endl;
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
    cerr << "ERROR - RadxRcalib::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// convert string metadata to XML

void RadxRcalib::_loadMetaStringsToXml(string &xml, int level /* = 0 */)  const
  
{
  xml.clear();
  xml += RadxXml::writeStartTag("RadxRcalib", level);
  xml += RadxXml::writeString("radarName", level + 1, _radarName);
  xml += RadxXml::writeEndTag("RadxRcalib", level);
}

/////////////////////////////////////////////////////////
// set metadata strings from XML
// returns 0 on success, -1 on failure

int RadxRcalib::_setMetaStringsFromXml(const char *xml,
                                    size_t bufLen)
  
{

  // check for NULL
  
  if (xml[bufLen - 1] != '\0') {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::_setMetaStringsFromXml" << endl;
    cerr << "  XML string not null terminated" << endl;
    string xmlStr(xml, bufLen);
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;    
  }

  string xmlStr(xml);
  string contents;

  if (RadxXml::readString(xmlStr, "RadxRcalib", contents)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::_setMetaStringsFromXml" << endl;
    cerr << "  XML not delimited by 'RadxRcalib' tags" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  if (RadxXml::readString(contents, "radarName", _radarName)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::_setMetaStringsFromXml" << endl;
    cerr << "  Cannot find 'radarName' tag" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxRcalib::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set time

  _metaNumbers.timeSecs = getCalibTime();

  // set 64 bit vals

  _metaNumbers.wavelengthCm = _wavelengthCm;
  _metaNumbers.beamWidthDegH = _beamWidthDegH;
  _metaNumbers.beamWidthDegV = _beamWidthDegV;
  _metaNumbers.antennaGainDbH = _antennaGainDbH;
  _metaNumbers.antennaGainDbV = _antennaGainDbV;
  _metaNumbers.pulseWidthUsec = _pulseWidthUsec;
  _metaNumbers.xmitPowerDbmH = _xmitPowerDbmH;
  _metaNumbers.xmitPowerDbmV = _xmitPowerDbmV;
  _metaNumbers.twoWayWaveguideLossDbH = _twoWayWaveguideLossDbH;
  _metaNumbers.twoWayWaveguideLossDbV = _twoWayWaveguideLossDbV;
  _metaNumbers.twoWayRadomeLossDbH = _twoWayRadomeLossDbH;
  _metaNumbers.twoWayRadomeLossDbV = _twoWayRadomeLossDbV;
  _metaNumbers.receiverMismatchLossDb = _receiverMismatchLossDb;
  _metaNumbers.kSquaredWater = _kSquaredWater;
  _metaNumbers.radarConstH = _radarConstH;
  _metaNumbers.radarConstV = _radarConstV;
  _metaNumbers.noiseDbmHc = _noiseDbmHc;
  _metaNumbers.noiseDbmHx = _noiseDbmHx;
  _metaNumbers.noiseDbmVc = _noiseDbmVc;
  _metaNumbers.noiseDbmVx = _noiseDbmVx;
  _metaNumbers.i0DbmHc = _i0DbmHc;
  _metaNumbers.i0DbmHx = _i0DbmHx;
  _metaNumbers.i0DbmVc = _i0DbmVc;
  _metaNumbers.i0DbmVx = _i0DbmVx;
  _metaNumbers.receiverGainDbHc = _receiverGainDbHc;
  _metaNumbers.receiverGainDbHx = _receiverGainDbHx;
  _metaNumbers.receiverGainDbVc = _receiverGainDbVc;
  _metaNumbers.receiverGainDbVx = _receiverGainDbVx;
  _metaNumbers.receiverSlopeDbHc = _receiverSlopeDbHc;
  _metaNumbers.receiverSlopeDbHx = _receiverSlopeDbHx;
  _metaNumbers.receiverSlopeDbVc = _receiverSlopeDbVc;
  _metaNumbers.receiverSlopeDbVx = _receiverSlopeDbVx;
  _metaNumbers.dynamicRangeDbHc = _dynamicRangeDbHc;
  _metaNumbers.dynamicRangeDbHx = _dynamicRangeDbHx;
  _metaNumbers.dynamicRangeDbVc = _dynamicRangeDbVc;
  _metaNumbers.dynamicRangeDbVx = _dynamicRangeDbVx;
  _metaNumbers.baseDbz1kmHc = _baseDbz1kmHc;
  _metaNumbers.baseDbz1kmHx = _baseDbz1kmHx;
  _metaNumbers.baseDbz1kmVc = _baseDbz1kmVc;
  _metaNumbers.baseDbz1kmVx = _baseDbz1kmVx;
  _metaNumbers.sunPowerDbmHc = _sunPowerDbmHc;
  _metaNumbers.sunPowerDbmHx = _sunPowerDbmHx;
  _metaNumbers.sunPowerDbmVc = _sunPowerDbmVc;
  _metaNumbers.sunPowerDbmVx = _sunPowerDbmVx;
  _metaNumbers.noiseSourcePowerDbmH = _noiseSourcePowerDbmH;
  _metaNumbers.noiseSourcePowerDbmV = _noiseSourcePowerDbmV;
  _metaNumbers.powerMeasLossDbH = _powerMeasLossDbH;
  _metaNumbers.powerMeasLossDbV = _powerMeasLossDbV;
  _metaNumbers.couplerForwardLossDbH = _couplerForwardLossDbH;
  _metaNumbers.couplerForwardLossDbV = _couplerForwardLossDbV;
  _metaNumbers.dbzCorrection = _dbzCorrection;
  _metaNumbers.zdrCorrectionDb = _zdrCorrectionDb;
  _metaNumbers.ldrCorrectionDbH = _ldrCorrectionDbH;
  _metaNumbers.ldrCorrectionDbV = _ldrCorrectionDbV;
  _metaNumbers.systemPhidpDeg = _systemPhidpDeg;
  _metaNumbers.testPowerDbmH = _testPowerDbmH;
  _metaNumbers.testPowerDbmV = _testPowerDbmV;
    
}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxRcalib::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                       size_t bufLen,
                                       bool swap)
  
{

  // check size

  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRcalib::_setMetaNumbersFromMsg" << endl;
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
  
  // set time

  setCalibTime(_metaNumbers.timeSecs);
  
  // set 64 bit values

  _wavelengthCm = _metaNumbers.wavelengthCm;
  _beamWidthDegH = _metaNumbers.beamWidthDegH;
  _beamWidthDegV = _metaNumbers.beamWidthDegV;
  _antennaGainDbH = _metaNumbers.antennaGainDbH;
  _antennaGainDbV = _metaNumbers.antennaGainDbV;
  _pulseWidthUsec = _metaNumbers.pulseWidthUsec;
  _xmitPowerDbmH = _metaNumbers.xmitPowerDbmH;
  _xmitPowerDbmV = _metaNumbers.xmitPowerDbmV;
  _twoWayWaveguideLossDbH = _metaNumbers.twoWayWaveguideLossDbH;
  _twoWayWaveguideLossDbV = _metaNumbers.twoWayWaveguideLossDbV;
  _twoWayRadomeLossDbH = _metaNumbers.twoWayRadomeLossDbH;
  _twoWayRadomeLossDbV = _metaNumbers.twoWayRadomeLossDbV;
  _receiverMismatchLossDb = _metaNumbers.receiverMismatchLossDb;
  _kSquaredWater = _metaNumbers.kSquaredWater;
  _radarConstH = _metaNumbers.radarConstH;
  _radarConstV = _metaNumbers.radarConstV;
  _noiseDbmHc = _metaNumbers.noiseDbmHc;
  _noiseDbmHx = _metaNumbers.noiseDbmHx;
  _noiseDbmVc = _metaNumbers.noiseDbmVc;
  _noiseDbmVx = _metaNumbers.noiseDbmVx;
  _i0DbmHc = _metaNumbers.i0DbmHc;
  _i0DbmHx = _metaNumbers.i0DbmHx;
  _i0DbmVc = _metaNumbers.i0DbmVc;
  _i0DbmVx = _metaNumbers.i0DbmVx;
  _receiverGainDbHc = _metaNumbers.receiverGainDbHc;
  _receiverGainDbHx = _metaNumbers.receiverGainDbHx;
  _receiverGainDbVc = _metaNumbers.receiverGainDbVc;
  _receiverGainDbVx = _metaNumbers.receiverGainDbVx;
  _receiverSlopeDbHc = _metaNumbers.receiverSlopeDbHc;
  _receiverSlopeDbHx = _metaNumbers.receiverSlopeDbHx;
  _receiverSlopeDbVc = _metaNumbers.receiverSlopeDbVc;
  _receiverSlopeDbVx = _metaNumbers.receiverSlopeDbVx;
  _dynamicRangeDbHc = _metaNumbers.dynamicRangeDbHc;
  _dynamicRangeDbHx = _metaNumbers.dynamicRangeDbHx;
  _dynamicRangeDbVc = _metaNumbers.dynamicRangeDbVc;
  _dynamicRangeDbVx = _metaNumbers.dynamicRangeDbVx;
  _baseDbz1kmHc = _metaNumbers.baseDbz1kmHc;
  _baseDbz1kmHx = _metaNumbers.baseDbz1kmHx;
  _baseDbz1kmVc = _metaNumbers.baseDbz1kmVc;
  _baseDbz1kmVx = _metaNumbers.baseDbz1kmVx;
  _sunPowerDbmHc = _metaNumbers.sunPowerDbmHc;
  _sunPowerDbmHx = _metaNumbers.sunPowerDbmHx;
  _sunPowerDbmVc = _metaNumbers.sunPowerDbmVc;
  _sunPowerDbmVx = _metaNumbers.sunPowerDbmVx;
  _noiseSourcePowerDbmH = _metaNumbers.noiseSourcePowerDbmH;
  _noiseSourcePowerDbmV = _metaNumbers.noiseSourcePowerDbmV;
  _powerMeasLossDbH = _metaNumbers.powerMeasLossDbH;
  _powerMeasLossDbV = _metaNumbers.powerMeasLossDbV;
  _couplerForwardLossDbH = _metaNumbers.couplerForwardLossDbH;
  _couplerForwardLossDbV = _metaNumbers.couplerForwardLossDbV;
  _dbzCorrection = _metaNumbers.dbzCorrection;
  _zdrCorrectionDb = _metaNumbers.zdrCorrectionDb;
  _ldrCorrectionDbH = _metaNumbers.ldrCorrectionDbH;
  _ldrCorrectionDbV = _metaNumbers.ldrCorrectionDbV;
  _systemPhidpDeg = _metaNumbers.systemPhidpDeg;
  _testPowerDbmH = _metaNumbers.testPowerDbmH;
  _testPowerDbmV = _metaNumbers.testPowerDbmV;

  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxRcalib::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta.timeSecs, 72 * sizeof(Radx::si64));
}
