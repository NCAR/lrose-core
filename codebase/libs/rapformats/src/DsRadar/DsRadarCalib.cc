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
// DsRadarCalb.cc
//
// Radar calibration support
//
// Mike Dixon, RAL, NCAR, Bouler, CO, USA
//
// August 2007
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

#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/file_io.h>
#include <toolsa/ugetenv.hh>
#include <rapformats/DsRadarCalib.hh>
#include <iostream>
#include <cmath>
#include <cerrno>
using namespace std;

///////////////
// constructor

DsRadarCalib::DsRadarCalib()
{
  ds_radar_calib_init(&_calib);
}

/////////////
// destructor

DsRadarCalib::~DsRadarCalib()
{
}

///////////////////////
// is a value missing?
//
// Compare with missing val

bool DsRadarCalib::isMissing(double val)
{
  if (val < (DS_RADAR_CALIB_MISSING + 0.001)) {
    return true;
  }
  return false;
}

////////////////////////////////////////
// adjust based on pulse width and power

void DsRadarCalib::adjustRadarConst(double pulseWidthUs,
                                    double xmitPowerDbmH,
                                    double xmitPowerDbmV)

{

  // do not attempt this if data is missing

  if (isMissing(_calib.pulseWidthUs)) {
    return;
  }
  if (isMissing(_calib.xmitPowerDbmH)) {
    return;
  }
  if (isMissing(_calib.xmitPowerDbmV)) {
    return;
  }
  if (isMissing(_calib.radarConstH)) {
    return;
  }
  if (isMissing(_calib.radarConstV)) {
    return;
  }
  if (pulseWidthUs <= 0) {
    return;
  }
  if (xmitPowerDbmH <= 0 || xmitPowerDbmV <= 0) {
    return;
  }

  // compute correction ratios in dB

  double pulseWidthRatioDb
    = 10.0 * log10(pulseWidthUs / _calib.pulseWidthUs);

  double xmitPowerRatioDbH = xmitPowerDbmH - _calib.xmitPowerDbmH;
  double xmitPowerRatioDbV = xmitPowerDbmV - _calib.xmitPowerDbmV;

  // adjust radar constant based on pulse width
  // decreases for increasing pulse width

  _calib.radarConstH += pulseWidthRatioDb;
  _calib.radarConstV += pulseWidthRatioDb;

  // adjust radar constant based on transmitter power
  // decreases for increasing power

  _calib.radarConstH += xmitPowerRatioDbH;
  _calib.radarConstV += xmitPowerRatioDbV;

  // compute base dbz

  if (!isMissing(_calib.noiseDbmHc) &&
      !isMissing(_calib.receiverGainDbHc)) {
    _calib.baseDbz1kmHc = (_calib.noiseDbmHc - 
                           _calib.receiverGainDbHc + 
                           _calib.radarConstH);
  }

  if (!isMissing(_calib.noiseDbmVc) &&
      !isMissing(_calib.receiverGainDbVc)) {
    _calib.baseDbz1kmVc = (_calib.noiseDbmVc - 
                           _calib.receiverGainDbVc + 
                           _calib.radarConstV);
  }
  
  if (!isMissing(_calib.noiseDbmHx) &&
      !isMissing(_calib.receiverGainDbHx)) {
    _calib.baseDbz1kmHx = (_calib.noiseDbmHx - 
                           _calib.receiverGainDbHx + 
                           _calib.radarConstH);
  }

  if (!isMissing(_calib.noiseDbmVx) &&
      !isMissing(_calib.receiverGainDbVx)) {
    _calib.baseDbz1kmVx = (_calib.noiseDbmVx - 
                           _calib.receiverGainDbVx + 
                           _calib.radarConstV);
  }

  _calib.xmitPowerDbmH = xmitPowerDbmH;
  _calib.xmitPowerDbmV = xmitPowerDbmV;
  
}

/////////////////////
// set the radar name

void DsRadarCalib::setRadarName(const string &name)
{
  STRncopy(_calib.radarName, name.c_str(), DS_RADAR_CALIB_NAME_LEN);
}

///////////////////////////
// set the calibration time

void DsRadarCalib::setCalibTime(time_t calTime)
{
  DateTime ctime(calTime);
  _calib.year = ctime.getYear();
  _calib.month = ctime.getMonth();
  _calib.day = ctime.getDay();
  _calib.hour = ctime.getHour();
  _calib.min = ctime.getMin();
  _calib.sec = ctime.getSec();
}

//////////////////
// set from struct

void DsRadarCalib::set(const ds_radar_calib_t &calib)
{
  _calib = calib;
}

////////////////////////////////////
// set from struct in BE byte order

void DsRadarCalib::setStructFromBE(const ds_radar_calib_t &calib)
{
  _calib = calib;
  BE_to_ds_radar_calib(&_calib);
}

void DsRadarCalib::decode(const ds_radar_calib_t *calib)
{
  setStructFromBE(*calib);
}

///////////////////////////
// get the radar name

string DsRadarCalib::getRadarName() const
{
  return _calib.radarName;
}

///////////////////////////
// get the calibration time

time_t DsRadarCalib::getCalibTime() const
{
  DateTime ctime(_calib.year, _calib.month, _calib.day,
		 _calib.hour, _calib.min, _calib.sec);
  return ctime.utime();
}

////////////////////////////////////////////
// get struct value in Big-Endian byte order

ds_radar_calib_t DsRadarCalib::getStructAsBE()  const
{
  ds_radar_calib_t calib = _calib;
  BE_from_ds_radar_calib(&calib);
  return calib;
}

////////////////////////////////////////////
// encode struct value into Big-Endian byte order

void DsRadarCalib::encode(ds_radar_calib_t *calib)  const
{
  *calib = _calib;
  BE_from_ds_radar_calib(calib);
}

////////////////////////////////////////////
// convert to XML - load up xml string

void DsRadarCalib::convert2Xml(string &xml)  const
{

  xml.clear();
  xml += TaXml::writeStartTag("DsRadarCalib", 0);

  // name

  xml += TaXml::writeString("radarName", 1, _calib.radarName);

  // time

  DateTime ctime(_calib.year, _calib.month, _calib.day,
                 _calib.hour, _calib.min, _calib.sec);
  xml += TaXml::writeTime("calibTime", 1, ctime.utime());
  
  // fields

  xml += TaXml::writeDouble("wavelengthCm", 1, _calib.wavelengthCm);
  xml += TaXml::writeDouble("beamWidthDegH", 1, _calib.beamWidthDegH);
  xml += TaXml::writeDouble("beamWidthDegV", 1, _calib.beamWidthDegV);
  xml += TaXml::writeDouble("antGainDbH", 1, _calib.antGainDbH);
  xml += TaXml::writeDouble("antGainDbV", 1, _calib.antGainDbV);
  xml += TaXml::writeDouble("pulseWidthUs", 1, _calib.pulseWidthUs);
  xml += TaXml::writeDouble("xmitPowerDbmH", 1, _calib.xmitPowerDbmH);
  xml += TaXml::writeDouble("xmitPowerDbmV", 1, _calib.xmitPowerDbmV);
  xml += TaXml::writeDouble("twoWayWaveguideLossDbH",
                            1, _calib.twoWayWaveguideLossDbH);
  xml += TaXml::writeDouble("twoWayWaveguideLossDbV",
                            1, _calib.twoWayWaveguideLossDbV);
  xml += TaXml::writeDouble("twoWayRadomeLossDbH",
                            1, _calib.twoWayRadomeLossDbH);
  xml += TaXml::writeDouble("twoWayRadomeLossDbV",
                            1, _calib.twoWayRadomeLossDbV);
  xml += TaXml::writeDouble("receiverMismatchLossDb",
                            1, _calib.receiverMismatchLossDb);
  xml += TaXml::writeDouble("kSquaredWater", 1, _calib.kSquaredWater);
  xml += TaXml::writeDouble("radarConstH", 1, _calib.radarConstH);
  xml += TaXml::writeDouble("radarConstV", 1, _calib.radarConstV);
  xml += TaXml::writeDouble("noiseDbmHc", 1, _calib.noiseDbmHc);
  xml += TaXml::writeDouble("noiseDbmHx", 1, _calib.noiseDbmHx);
  xml += TaXml::writeDouble("noiseDbmVc", 1, _calib.noiseDbmVc);
  xml += TaXml::writeDouble("noiseDbmVx", 1, _calib.noiseDbmVx);
  xml += TaXml::writeDouble("i0DbmHc", 1, _calib.i0DbmHc);
  xml += TaXml::writeDouble("i0DbmHx", 1, _calib.i0DbmHx);
  xml += TaXml::writeDouble("i0DbmVc", 1, _calib.i0DbmVc);
  xml += TaXml::writeDouble("i0DbmVx", 1, _calib.i0DbmVx);
  xml += TaXml::writeDouble("receiverGainDbHc", 1, _calib.receiverGainDbHc);
  xml += TaXml::writeDouble("receiverGainDbHx", 1, _calib.receiverGainDbHx);
  xml += TaXml::writeDouble("receiverGainDbVc", 1, _calib.receiverGainDbVc);
  xml += TaXml::writeDouble("receiverGainDbVx", 1, _calib.receiverGainDbVx);
  xml += TaXml::writeDouble("receiverSlopeDbHc", 1, _calib.receiverSlopeDbHc);
  xml += TaXml::writeDouble("receiverSlopeDbHx", 1, _calib.receiverSlopeDbHx);
  xml += TaXml::writeDouble("receiverSlopeDbVc", 1, _calib.receiverSlopeDbVc);
  xml += TaXml::writeDouble("receiverSlopeDbVx", 1, _calib.receiverSlopeDbVx);
  xml += TaXml::writeDouble("dynamicRangeDbHc", 1, _calib.dynamicRangeDbHc);
  xml += TaXml::writeDouble("dynamicRangeDbHx", 1, _calib.dynamicRangeDbHx);
  xml += TaXml::writeDouble("dynamicRangeDbVc", 1, _calib.dynamicRangeDbVc);
  xml += TaXml::writeDouble("dynamicRangeDbVx", 1, _calib.dynamicRangeDbVx);
  xml += TaXml::writeDouble("baseDbz1kmHc", 1, _calib.baseDbz1kmHc);
  xml += TaXml::writeDouble("baseDbz1kmHx", 1, _calib.baseDbz1kmHx);
  xml += TaXml::writeDouble("baseDbz1kmVc", 1, _calib.baseDbz1kmVc);
  xml += TaXml::writeDouble("baseDbz1kmVx", 1, _calib.baseDbz1kmVx);
  xml += TaXml::writeDouble("sunPowerDbmHc", 1, _calib.sunPowerDbmHc);
  xml += TaXml::writeDouble("sunPowerDbmHx", 1, _calib.sunPowerDbmHx);
  xml += TaXml::writeDouble("sunPowerDbmVc", 1, _calib.sunPowerDbmVc);
  xml += TaXml::writeDouble("sunPowerDbmVx", 1, _calib.sunPowerDbmVx);
  xml += TaXml::writeDouble("noiseSourcePowerDbmH",
                            1, _calib.noiseSourcePowerDbmH);
  xml += TaXml::writeDouble("noiseSourcePowerDbmV",
                            1, _calib.noiseSourcePowerDbmV);
  xml += TaXml::writeDouble("powerMeasLossDbH", 1, _calib.powerMeasLossDbH);
  xml += TaXml::writeDouble("powerMeasLossDbV", 1, _calib.powerMeasLossDbV);
  xml += TaXml::writeDouble("couplerForwardLossDbH",
                            1, _calib.couplerForwardLossDbH);
  xml += TaXml::writeDouble("couplerForwardLossDbV",
                            1, _calib.couplerForwardLossDbV);
  xml += TaXml::writeDouble("dbzCorrection", 1, _calib.dbzCorrection);
  xml += TaXml::writeDouble("zdrCorrectionDb", 1, _calib.zdrCorrectionDb);
  xml += TaXml::writeDouble("ldrCorrectionDbH", 1, _calib.ldrCorrectionDbH);
  xml += TaXml::writeDouble("ldrCorrectionDbV", 1, _calib.ldrCorrectionDbV);
  xml += TaXml::writeDouble("systemPhidpDeg", 1, _calib.systemPhidpDeg);
  xml += TaXml::writeDouble("testPowerDbmH", 1, _calib.testPowerDbmH);
  xml += TaXml::writeDouble("testPowerDbmV", 1, _calib.testPowerDbmV);

  xml += TaXml::writeEndTag("DsRadarCalib", 0);
}

////////////////////////////////////////////
// set from XML string
//
// Returns 0 on success, -1 on failure
// Sets errStr on failure

int DsRadarCalib::setFromXml(const string &xmlBuf, string &errStr)
{

  ds_radar_calib_init(&_calib);

  int iret = 0;
  string xbuf = TaXml::removeComments(xmlBuf);

  // set data

  string name;
  if (TaXml::readString(xbuf, "radarName", name) == 0) {
    STRncopy(_calib.radarName, name.c_str(), sizeof(_calib.radarName));
  } else {
    errStr += "ERROR - radarName missing\n";
    iret = -1;
  }

  time_t ctime;
  if (TaXml::readTime(xbuf, "calibTime", ctime) == 0) {
    DateTime cTime(ctime); 
    _calib.year = cTime.getYear();
    _calib.month = cTime.getMonth();
    _calib.day = cTime.getDay();
    _calib.hour = cTime.getHour();
    _calib.min = cTime.getMin();
    _calib.sec = cTime.getSec();
  } else {
    errStr += "ERROR - calibTime missing\n";
    iret = -1;
  }
  
  double val;

  if (TaXml::readDouble(xbuf, "wavelengthCm", val) == 0) {
    _calib.wavelengthCm = val;
  } else {
    errStr += "ERROR - wavelengthCm missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "beamWidthDegH", val) == 0) {
    _calib.beamWidthDegH = val;
  }

  if (TaXml::readDouble(xbuf, "beamWidthDegV", val) == 0) {
    _calib.beamWidthDegV = val;
  }

  if (TaXml::readDouble(xbuf, "antGainDbH", val) == 0) {
    _calib.antGainDbH = val;
  }

  if (TaXml::readDouble(xbuf, "antGainDbV", val) == 0) {
    _calib.antGainDbV = val;
  }

  if (TaXml::readDouble(xbuf, "pulseWidthUs", val) == 0) {
    _calib.pulseWidthUs = val;
  } else {
    errStr += "ERROR - pulseWidthUs missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "xmitPowerDbmH", val) == 0) {
    _calib.xmitPowerDbmH = val;
  }

  if (TaXml::readDouble(xbuf, "xmitPowerDbmV", val) == 0) {
    _calib.xmitPowerDbmV = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayWaveguideLossDbH", val) == 0) {
    _calib.twoWayWaveguideLossDbH = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayWaveguideLossDbV", val) == 0) {
    _calib.twoWayWaveguideLossDbV = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayRadomeLossDbH", val) == 0) {
    _calib.twoWayRadomeLossDbH = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayRadomeLossDbV", val) == 0) {
    _calib.twoWayRadomeLossDbV = val;
  }

  if (TaXml::readDouble(xbuf, "receiverMismatchLossDb", val) == 0) {
    _calib.receiverMismatchLossDb = val;
  }

  if (TaXml::readDouble(xbuf, "kSquaredWater", val) == 0) {
    _calib.kSquaredWater = val;
  }

  if (TaXml::readDouble(xbuf, "radarConstH", val) == 0) {
    _calib.radarConstH = val;
  } else {
    errStr += "ERROR - radarConstH missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "radarConstV", val) == 0) {
    _calib.radarConstV = val;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmHc", val) == 0) {
    _calib.noiseDbmHc = val;
  } else {
    errStr += "ERROR - noiseDbmHc missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmHx", val) == 0) {
    _calib.noiseDbmHx = val;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmVc", val) == 0) {
    _calib.noiseDbmVc = val;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmVx", val) == 0) {
    _calib.noiseDbmVx = val;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbHc", val) == 0) {
    _calib.receiverGainDbHc = val;
  } else {
    errStr += "ERROR - receiverGainDbHc missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbHx", val) == 0) {
    _calib.receiverGainDbHx = val;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbVc", val) == 0) {
    _calib.receiverGainDbVc = val;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbVx", val) == 0) {
    _calib.receiverGainDbVx = val;
  }

  if (TaXml::readDouble(xbuf, "i0DbmHc", val) == 0) {
    _calib.i0DbmHc = val;
  } else {
    _calib.i0DbmHc = _calib.noiseDbmHc - _calib.receiverGainDbHc;
  }

  if (TaXml::readDouble(xbuf, "i0DbmHx", val) == 0) {
    _calib.i0DbmHx = val;
  } else {
    _calib.i0DbmHx = _calib.noiseDbmHx - _calib.receiverGainDbHx;
  }

  if (TaXml::readDouble(xbuf, "i0DbmVc", val) == 0) {
    _calib.i0DbmVc = val;
  } else {
    _calib.i0DbmVc = _calib.noiseDbmVc - _calib.receiverGainDbVc;
  }

  if (TaXml::readDouble(xbuf, "i0DbmVx", val) == 0) {
    _calib.i0DbmVx = val;
  } else {
    _calib.i0DbmVx = _calib.noiseDbmVx - _calib.receiverGainDbVx;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbHc", val) == 0) {
    _calib.receiverSlopeDbHc = val;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbHx", val) == 0) {
    _calib.receiverSlopeDbHx = val;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbVc", val) == 0) {
    _calib.receiverSlopeDbVc = val;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbVx", val) == 0) {
    _calib.receiverSlopeDbVx = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbHc", val) == 0) {
    _calib.dynamicRangeDbHc = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbHx", val) == 0) {
    _calib.dynamicRangeDbHx = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbVc", val) == 0) {
    _calib.dynamicRangeDbVc = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbVx", val) == 0) {
    _calib.dynamicRangeDbVx = val;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmHc", val) == 0) {
    _calib.baseDbz1kmHc = val;
  } else {
    errStr += "ERROR - baseDbz1kmHc missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmHx", val) == 0) {
    _calib.baseDbz1kmHx = val;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmVc", val) == 0) {
    _calib.baseDbz1kmVc = val;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmVx", val) == 0) {
    _calib.baseDbz1kmVx = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmHc", val) == 0) {
    _calib.sunPowerDbmHc = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmHx", val) == 0) {
    _calib.sunPowerDbmHx = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmVc", val) == 0) {
    _calib.sunPowerDbmVc = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmVx", val) == 0) {
    _calib.sunPowerDbmVx = val;
  }

  if (TaXml::readDouble(xbuf, "noiseSourcePowerDbmH", val) == 0) {
    _calib.noiseSourcePowerDbmH = val;
  }

  if (TaXml::readDouble(xbuf, "noiseSourcePowerDbmV", val) == 0) {
    _calib.noiseSourcePowerDbmV = val;
  }

  if (TaXml::readDouble(xbuf, "powerMeasLossDbH", val) == 0) {
    _calib.powerMeasLossDbH = val;
  }

  if (TaXml::readDouble(xbuf, "powerMeasLossDbV", val) == 0) {
    _calib.powerMeasLossDbV = val;
  }

  if (TaXml::readDouble(xbuf, "couplerForwardLossDbH", val) == 0) {
    _calib.couplerForwardLossDbH = val;
  }

  if (TaXml::readDouble(xbuf, "couplerForwardLossDbV", val) == 0) {
    _calib.couplerForwardLossDbV = val;
  }

  if (TaXml::readDouble(xbuf, "dbzCorrection", val) == 0) {
    _calib.dbzCorrection = val;
  }

  if (TaXml::readDouble(xbuf, "zdrCorrectionDb", val) == 0) {
    _calib.zdrCorrectionDb = val;
  }

  if (TaXml::readDouble(xbuf, "ldrCorrectionDbH", val) == 0) {
    _calib.ldrCorrectionDbH = val;
  }

  if (TaXml::readDouble(xbuf, "ldrCorrectionDbV", val) == 0) {
    _calib.ldrCorrectionDbV = val;
  }

  if (TaXml::readDouble(xbuf, "systemPhidpDeg", val) == 0) {
    _calib.systemPhidpDeg = val;
  }

  if (TaXml::readDouble(xbuf, "testPowerDbmH", val) == 0) {
    _calib.testPowerDbmH = val;
  }

  if (TaXml::readDouble(xbuf, "testPowerDbmV", val) == 0) {
    _calib.testPowerDbmV = val;
  }

  return iret;

}

////////////////////////////////////////////////////////////////////////
// read from a given XML cal file
//
// Returns 0 on success, -1 on failure
// Sets errStr on failure

int DsRadarCalib::readFromXmlFile(const string &calPath, string &errStr)

{

  errStr = "ERROR - DsRadarCalib::_readFromXmlFile\n";

  // Stat the file to get length
  
  struct stat calStat;
  if (ta_stat(calPath.c_str(), &calStat)) {
    int errNum = errno;
    TaStr::AddStr(errStr, "  Cannot stat file: ", calPath);
    TaStr::AddStr(errStr, "  ", strerror(errNum));
    return -1;
  }
  size_t fileLen = calStat.st_size;

  // open file
  
  FILE *calFile;
  if ((calFile = fopen(calPath.c_str(), "r")) == NULL) {
    int errNum = errno;
    TaStr::AddStr(errStr, "  Cannot open file for reading: ", calPath);
    TaStr::AddStr(errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // create buffer
  
  TaArray<char> bufArray;
  char *xmlBuf = bufArray.alloc(fileLen + 1);
  memset(xmlBuf, 0, fileLen + 1);
  
  // read in buffer, close file
  
  if (ta_fread(xmlBuf, 1, fileLen, calFile) != fileLen) {
    int errNum = errno;
    TaStr::AddStr(errStr, "  Cannot read from file: ", calPath);
    TaStr::AddStr(errStr, "  ", strerror(errNum));
    fclose(calFile);
    return -1;
  }
  fclose(calFile);

  // null terminate xmlBuf

  xmlBuf[fileLen] = '\0';

  string xmlErrStr;
  if (setFromXml(xmlBuf, xmlErrStr)) {
    TaStr::AddStr(errStr, "ERROR decoding XML file: ", calPath);
    errStr += xmlErrStr;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
// override from struct, if struct member data is not missing

void DsRadarCalib::overrideFromStruct(const ds_radar_calib_t &calib)
{

  if (!isMissing(calib.wavelengthCm)) {
    _calib.wavelengthCm = calib.wavelengthCm;
  }
  if (!isMissing(calib.beamWidthDegH)) {
    _calib.beamWidthDegH = calib.beamWidthDegH;
  }
  if (!isMissing(calib.beamWidthDegV)) {
    _calib.beamWidthDegV = calib.beamWidthDegV;
  }
  if (!isMissing(calib.antGainDbH)) {
    _calib.antGainDbH = calib.antGainDbH;
  }
  if (!isMissing(calib.antGainDbV)) {
    _calib.antGainDbV = calib.antGainDbV;
  }
  if (!isMissing(calib.pulseWidthUs)) {
    _calib.pulseWidthUs = calib.pulseWidthUs;
  }
  if (!isMissing(calib.xmitPowerDbmH)) {
    _calib.xmitPowerDbmH = calib.xmitPowerDbmH;
  }
  if (!isMissing(calib.xmitPowerDbmV)) {
    _calib.xmitPowerDbmV = calib.xmitPowerDbmV;
  }
  if (!isMissing(calib.twoWayWaveguideLossDbH)) {
    _calib.twoWayWaveguideLossDbH = calib.twoWayWaveguideLossDbH;
  }
  if (!isMissing(calib.twoWayWaveguideLossDbV)) {
    _calib.twoWayWaveguideLossDbV = calib.twoWayWaveguideLossDbV;
  }
  if (!isMissing(calib.twoWayRadomeLossDbH)) {
    _calib.twoWayRadomeLossDbH = calib.twoWayRadomeLossDbH;
  }
  if (!isMissing(calib.twoWayRadomeLossDbV)) {
    _calib.twoWayRadomeLossDbV = calib.twoWayRadomeLossDbV;
  }
  if (!isMissing(calib.receiverMismatchLossDb)) {
    _calib.receiverMismatchLossDb = calib.receiverMismatchLossDb;
  }
  if (!isMissing(calib.kSquaredWater)) {
    _calib.kSquaredWater = calib.kSquaredWater;
  }
  if (!isMissing(calib.radarConstH)) {
    _calib.radarConstH = calib.radarConstH;
  }
  if (!isMissing(calib.radarConstV)) {
    _calib.radarConstV = calib.radarConstV;
  }
  if (!isMissing(calib.noiseDbmHc)) {
    _calib.noiseDbmHc = calib.noiseDbmHc;
  }
  if (!isMissing(calib.noiseDbmHx)) {
    _calib.noiseDbmHx = calib.noiseDbmHx;
  }
  if (!isMissing(calib.noiseDbmVc)) {
    _calib.noiseDbmVc = calib.noiseDbmVc;
  }
  if (!isMissing(calib.noiseDbmVx)) {
    _calib.noiseDbmVx = calib.noiseDbmVx;
  }
  if (!isMissing(calib.receiverGainDbHc)) {
    _calib.receiverGainDbHc = calib.receiverGainDbHc;
  }
  if (!isMissing(calib.receiverGainDbHx)) {
    _calib.receiverGainDbHx = calib.receiverGainDbHx;
  }
  if (!isMissing(calib.receiverGainDbVc)) {
    _calib.receiverGainDbVc = calib.receiverGainDbVc;
  }
  if (!isMissing(calib.receiverGainDbVx)) {
    _calib.receiverGainDbVx = calib.receiverGainDbVx;
  }
  if (!isMissing(calib.i0DbmHc)) {
    _calib.i0DbmHc = _calib.noiseDbmHc - _calib.receiverGainDbHc;
  }
  if (!isMissing(calib.i0DbmHx)) {
    _calib.i0DbmHx = _calib.noiseDbmHx - _calib.receiverGainDbHx;
  }
  if (!isMissing(calib.i0DbmVc)) {
    _calib.i0DbmVc = _calib.noiseDbmVc - _calib.receiverGainDbVc;
  }
  if (!isMissing(calib.i0DbmVx)) {
    _calib.i0DbmVx = _calib.noiseDbmVx - _calib.receiverGainDbVx;
  }
  if (!isMissing(calib.receiverSlopeDbHc)) {
    _calib.receiverSlopeDbHc = calib.receiverSlopeDbHc;
  }
  if (!isMissing(calib.receiverSlopeDbHx)) {
    _calib.receiverSlopeDbHx = calib.receiverSlopeDbHx;
  }
  if (!isMissing(calib.receiverSlopeDbVc)) {
    _calib.receiverSlopeDbVc = calib.receiverSlopeDbVc;
  }
  if (!isMissing(calib.receiverSlopeDbVx)) {
    _calib.receiverSlopeDbVx = calib.receiverSlopeDbVx;
  }
  if (!isMissing(calib.dynamicRangeDbHc)) {
    _calib.dynamicRangeDbHc = calib.dynamicRangeDbHc;
  }
  if (!isMissing(calib.dynamicRangeDbHx)) {
    _calib.dynamicRangeDbHx = calib.dynamicRangeDbHx;
  }
  if (!isMissing(calib.dynamicRangeDbVc)) {
    _calib.dynamicRangeDbVc = calib.dynamicRangeDbVc;
  }
  if (!isMissing(calib.dynamicRangeDbVx)) {
    _calib.dynamicRangeDbVx = calib.dynamicRangeDbVx;
  }
  if (!isMissing(calib.baseDbz1kmHc)) {
    _calib.baseDbz1kmHc = calib.baseDbz1kmHc;
  }
  if (!isMissing(calib.baseDbz1kmHx)) {
    _calib.baseDbz1kmHx = calib.baseDbz1kmHx;
  }
  if (!isMissing(calib.baseDbz1kmVc)) {
    _calib.baseDbz1kmVc = calib.baseDbz1kmVc;
  }
  if (!isMissing(calib.baseDbz1kmVx)) {
    _calib.baseDbz1kmVx = calib.baseDbz1kmVx;
  }
  if (!isMissing(calib.sunPowerDbmHc)) {
    _calib.sunPowerDbmHc = calib.sunPowerDbmHc;
  }
  if (!isMissing(calib.sunPowerDbmHx)) {
    _calib.sunPowerDbmHx = calib.sunPowerDbmHx;
  }
  if (!isMissing(calib.sunPowerDbmVc)) {
    _calib.sunPowerDbmVc = calib.sunPowerDbmVc;
  }
  if (!isMissing(calib.sunPowerDbmVx)) {
    _calib.sunPowerDbmVx = calib.sunPowerDbmVx;
  }
  if (!isMissing(calib.noiseSourcePowerDbmH)) {
    _calib.noiseSourcePowerDbmH = calib.noiseSourcePowerDbmH;
  }
  if (!isMissing(calib.noiseSourcePowerDbmV)) {
    _calib.noiseSourcePowerDbmV = calib.noiseSourcePowerDbmV;
  }
  if (!isMissing(calib.powerMeasLossDbH)) {
    _calib.powerMeasLossDbH = calib.powerMeasLossDbH;
  }
  if (!isMissing(calib.powerMeasLossDbV)) {
    _calib.powerMeasLossDbV = calib.powerMeasLossDbV;
  }
  if (!isMissing(calib.couplerForwardLossDbH)) {
    _calib.couplerForwardLossDbH = calib.couplerForwardLossDbH;
  }
  if (!isMissing(calib.couplerForwardLossDbV)) {
    _calib.couplerForwardLossDbV = calib.couplerForwardLossDbV;
  }
  if (!isMissing(calib.dbzCorrection)) {
    _calib.dbzCorrection = calib.dbzCorrection;
  }
  if (!isMissing(calib.zdrCorrectionDb)) {
    _calib.zdrCorrectionDb = calib.zdrCorrectionDb;
  }
  if (!isMissing(calib.ldrCorrectionDbH)) {
    _calib.ldrCorrectionDbH = calib.ldrCorrectionDbH;
  }
  if (!isMissing(calib.ldrCorrectionDbV)) {
    _calib.ldrCorrectionDbV = calib.ldrCorrectionDbV;
  }
  if (!isMissing(calib.systemPhidpDeg)) {
    _calib.systemPhidpDeg = calib.systemPhidpDeg;
  }
  if (!isMissing(calib.testPowerDbmH)) {
    _calib.testPowerDbmH = calib.testPowerDbmH;
  }
  if (!isMissing(calib.testPowerDbmV)) {
    _calib.testPowerDbmV = calib.testPowerDbmV;
  }

}

////////
// print

void
DsRadarCalib::print(FILE *out) const
{
   
  fprintf(out, "RADAR CALIB\n");

  DateTime ctime(_calib.year, _calib.month, _calib.day,
                 _calib.hour, _calib.min, _calib.sec);
  fprintf(out, "  time: %s\n", DateTime::strm(ctime.utime()).c_str());

  fprintf(out, "  wavelengthCm: %g\n", _calib.wavelengthCm);
  fprintf(out, "  beamWidthDegH: %g\n", _calib.beamWidthDegH);
  fprintf(out, "  beamWidthDegV: %g\n", _calib.beamWidthDegV);
  fprintf(out, "  antGainDbH: %g\n", _calib.antGainDbH);
  fprintf(out, "  antGainDbV: %g\n", _calib.antGainDbV);
  fprintf(out, "  pulseWidthUs: %g\n", _calib.pulseWidthUs);
  fprintf(out, "  xmitPowerDbmH: %g\n", _calib.xmitPowerDbmH);
  fprintf(out, "  xmitPowerDbmV: %g\n", _calib.xmitPowerDbmV);
  fprintf(out, "  twoWayWaveguideLossDbH: %g\n",
          _calib.twoWayWaveguideLossDbH);
  fprintf(out, "  twoWayWaveguideLossDbV: %g\n",
          _calib.twoWayWaveguideLossDbV);
  fprintf(out, "  twoWayRadomeLossDbH: %g\n", _calib.twoWayRadomeLossDbH);
  fprintf(out, "  twoWayRadomeLossDbV: %g\n", _calib.twoWayRadomeLossDbV);
  fprintf(out, "  receiverMismatchLossDb: %g\n",
          _calib.receiverMismatchLossDb);
  fprintf(out, "  kSquaredWater: %g\n", _calib.kSquaredWater);
  fprintf(out, "  radarConstH: %g\n", _calib.radarConstH);
  fprintf(out, "  radarConstV: %g\n", _calib.radarConstV);
  fprintf(out, "  noiseDbmHc: %g\n", _calib.noiseDbmHc);
  fprintf(out, "  noiseDbmHx: %g\n", _calib.noiseDbmHx);
  fprintf(out, "  noiseDbmVc: %g\n", _calib.noiseDbmVc);
  fprintf(out, "  noiseDbmVx: %g\n", _calib.noiseDbmVx);
  fprintf(out, "  i0DbmHc: %g\n", _calib.i0DbmHc);
  fprintf(out, "  i0DbmHx: %g\n", _calib.i0DbmHx);
  fprintf(out, "  i0DbmVc: %g\n", _calib.i0DbmVc);
  fprintf(out, "  i0DbmVx: %g\n", _calib.i0DbmVx);
  fprintf(out, "  receiverGainDbHc: %g\n", _calib.receiverGainDbHc);
  fprintf(out, "  receiverGainDbHx: %g\n", _calib.receiverGainDbHx);
  fprintf(out, "  receiverGainDbVc: %g\n", _calib.receiverGainDbVc);
  fprintf(out, "  receiverGainDbVx: %g\n", _calib.receiverGainDbVx);
  fprintf(out, "  receiverSlopeDbHc: %g\n", _calib.receiverSlopeDbHc);
  fprintf(out, "  receiverSlopeDbHx: %g\n", _calib.receiverSlopeDbHx);
  fprintf(out, "  receiverSlopeDbVc: %g\n", _calib.receiverSlopeDbVc);
  fprintf(out, "  receiverSlopeDbVx: %g\n", _calib.receiverSlopeDbVx);
  fprintf(out, "  dynamicRangeDbHc: %g\n", _calib.dynamicRangeDbHc);
  fprintf(out, "  dynamicRangeDbHx: %g\n", _calib.dynamicRangeDbHx);
  fprintf(out, "  dynamicRangeDbVc: %g\n", _calib.dynamicRangeDbVc);
  fprintf(out, "  dynamicRangeDbVx: %g\n", _calib.dynamicRangeDbVx);
  fprintf(out, "  baseDbz1kmHc: %g\n", _calib.baseDbz1kmHc);
  fprintf(out, "  baseDbz1kmHx: %g\n", _calib.baseDbz1kmHx);
  fprintf(out, "  baseDbz1kmVc: %g\n", _calib.baseDbz1kmVc);
  fprintf(out, "  baseDbz1kmVx: %g\n", _calib.baseDbz1kmVx);
  fprintf(out, "  sunPowerDbmHc: %g\n", _calib.sunPowerDbmHc);
  fprintf(out, "  sunPowerDbmHx: %g\n", _calib.sunPowerDbmHx);
  fprintf(out, "  sunPowerDbmVc: %g\n", _calib.sunPowerDbmVc);
  fprintf(out, "  sunPowerDbmVx: %g\n", _calib.sunPowerDbmVx);
  fprintf(out, "  noiseSourcePowerDbmH: %g\n", _calib.noiseSourcePowerDbmH);
  fprintf(out, "  noiseSourcePowerDbmV: %g\n", _calib.noiseSourcePowerDbmV);
  fprintf(out, "  powerMeasLossDbH: %g\n", _calib.powerMeasLossDbH);
  fprintf(out, "  powerMeasLossDbV: %g\n", _calib.powerMeasLossDbV);
  fprintf(out, "  couplerForwardLossDbH: %g\n", _calib.couplerForwardLossDbH);
  fprintf(out, "  couplerForwardLossDbV: %g\n", _calib.couplerForwardLossDbV);
  fprintf(out, "  dbzCorrection: %g\n", _calib.dbzCorrection);
  fprintf(out, "  zdrCorrectionDb: %g\n", _calib.zdrCorrectionDb);
  fprintf(out, "  ldrCorrectionDbH: %g\n", _calib.ldrCorrectionDbH);
  fprintf(out, "  ldrCorrectionDbV: %g\n", _calib.ldrCorrectionDbV);
  fprintf(out, "  systemPhidpDeg: %g\n", _calib.systemPhidpDeg);
  fprintf(out, "  testPowerDbmH: %g\n", _calib.testPowerDbmH);
  fprintf(out, "  testPowerDbmV: %g\n", _calib.testPowerDbmV);
  fprintf(out, "\n");

}

void
DsRadarCalib::print(ostream &out)  const
{
   
  out << "RADAR CALIB" << endl;
  out << "-----------" << endl;
   
  DateTime ctime(_calib.year, _calib.month, _calib.day,
                 _calib.hour, _calib.min, _calib.sec);
  out << "  time: " << DateTime::strm(ctime.utime()) << endl;

  out << "  wavelengthCm: " << _calib.wavelengthCm << endl;
  out << "  beamWidthDegH: " << _calib.beamWidthDegH << endl;
  out << "  beamWidthDegV: " << _calib.beamWidthDegV << endl;
  out << "  antGainDbH: " << _calib.antGainDbH << endl;
  out << "  antGainDbV: " << _calib.antGainDbV << endl;
  out << "  pulseWidthUs: " << _calib.pulseWidthUs << endl;
  out << "  xmitPowerDbmH: " << _calib.xmitPowerDbmH << endl;
  out << "  xmitPowerDbmV: " << _calib.xmitPowerDbmV << endl;
  out << "  twoWayWaveguideLossDbH: " << _calib.twoWayWaveguideLossDbH << endl;
  out << "  twoWayWaveguideLossDbV: " << _calib.twoWayWaveguideLossDbV << endl;
  out << "  twoWayRadomeLossDbH: " << _calib.twoWayRadomeLossDbH << endl;
  out << "  twoWayRadomeLossDbV: " << _calib.twoWayRadomeLossDbV << endl;
  out << "  receiverMismatchLossDb: " << _calib.receiverMismatchLossDb << endl;
  out << "  kSquaredWater: " << _calib.kSquaredWater << endl;
  out << "  radarConstH: " << _calib.radarConstH << endl;
  out << "  radarConstV: " << _calib.radarConstV << endl;
  out << "  noiseDbmHc: " << _calib.noiseDbmHc << endl;
  out << "  noiseDbmHx: " << _calib.noiseDbmHx << endl;
  out << "  noiseDbmVc: " << _calib.noiseDbmVc << endl;
  out << "  noiseDbmVx: " << _calib.noiseDbmVx << endl;
  out << "  i0DbmHc: " << _calib.i0DbmHc << endl;
  out << "  i0DbmHx: " << _calib.i0DbmHx << endl;
  out << "  i0DbmVc: " << _calib.i0DbmVc << endl;
  out << "  i0DbmVx: " << _calib.i0DbmVx << endl;
  out << "  receiverGainDbHc: " << _calib.receiverGainDbHc << endl;
  out << "  receiverGainDbHx: " << _calib.receiverGainDbHx << endl;
  out << "  receiverGainDbVc: " << _calib.receiverGainDbVc << endl;
  out << "  receiverGainDbVx: " << _calib.receiverGainDbVx << endl;
  out << "  receiverSlopeDbHc: " << _calib.receiverSlopeDbHc << endl;
  out << "  receiverSlopeDbHx: " << _calib.receiverSlopeDbHx << endl;
  out << "  receiverSlopeDbVc: " << _calib.receiverSlopeDbVc << endl;
  out << "  receiverSlopeDbVx: " << _calib.receiverSlopeDbVx << endl;
  out << "  dynamicRangeDbHc: " << _calib.dynamicRangeDbHc << endl;
  out << "  dynamicRangeDbHx: " << _calib.dynamicRangeDbHx << endl;
  out << "  dynamicRangeDbVc: " << _calib.dynamicRangeDbVc << endl;
  out << "  dynamicRangeDbVx: " << _calib.dynamicRangeDbVx << endl;
  out << "  baseDbz1kmHc: " << _calib.baseDbz1kmHc << endl;
  out << "  baseDbz1kmHx: " << _calib.baseDbz1kmHx << endl;
  out << "  baseDbz1kmVc: " << _calib.baseDbz1kmVc << endl;
  out << "  baseDbz1kmVx: " << _calib.baseDbz1kmVx << endl;
  out << "  sunPowerDbmHc: " << _calib.sunPowerDbmHc << endl;
  out << "  sunPowerDbmHx: " << _calib.sunPowerDbmHx << endl;
  out << "  sunPowerDbmVc: " << _calib.sunPowerDbmVc << endl;
  out << "  sunPowerDbmVx: " << _calib.sunPowerDbmVx << endl;
  out << "  noiseSourcePowerDbmH: " << _calib.noiseSourcePowerDbmH << endl;
  out << "  noiseSourcePowerDbmV: " << _calib.noiseSourcePowerDbmV << endl;
  out << "  powerMeasLossDbH: " << _calib.powerMeasLossDbH << endl;
  out << "  powerMeasLossDbV: " << _calib.powerMeasLossDbV << endl;
  out << "  couplerForwardLossDbH: " << _calib.couplerForwardLossDbH << endl;
  out << "  couplerForwardLossDbV: " << _calib.couplerForwardLossDbV << endl;
  out << "  dbzCorrection: " << _calib.dbzCorrection << endl;
  out << "  zdrCorrectionDb: " << _calib.zdrCorrectionDb << endl;
  out << "  ldrCorrectionDbH: " << _calib.ldrCorrectionDbH << endl;
  out << "  ldrCorrectionDbV: " << _calib.ldrCorrectionDbV << endl;
  out << "  systemPhidpDeg: " << _calib.systemPhidpDeg << endl;
  out << "  testPowerDbmH: " << _calib.testPowerDbmH << endl;
  out << "  testPowerDbmV: " << _calib.testPowerDbmV << endl;

  out << endl;

}

