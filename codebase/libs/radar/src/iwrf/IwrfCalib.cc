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
// Dec 2011
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
#include <radar/IwrfCalib.hh>
#include <radar/RadarCalib.hh>
#include <radar/iwrf_functions.hh>
#include <rapformats/DsRadarCalib.hh>
#include <Radx/RadxRcalib.hh>
#include <iostream>
#include <cmath>
#include <cerrno>
using namespace std;

double IwrfCalib::LightSpeedMps = 2.99792458e8;

///////////////
// constructor

IwrfCalib::IwrfCalib()
{
  iwrf_calibration_init(_calib);
}

/////////////
// destructor

IwrfCalib::~IwrfCalib()
{
}

///////////////////////
// is a value missing?
//
// Compare with missing val

bool IwrfCalib::isMissing(fl32 val)
{
  return iwrf_float_is_missing(val);
}

////////////////////////////////////////
// Adjust based on pulse width and power.
// NOTE - this assumes a positive radar constant.

void IwrfCalib::adjustRadarConst(double pulseWidthUs,
                                 double xmitPowerDbmH,
                                 double xmitPowerDbmV)

{

  // do not attempt this if data is missing or bad

  if (isMissing(_calib.pulse_width_us)) {
    return;
  }
  if (isMissing(_calib.xmit_power_dbm_h)) {
    return;
  }
  if (isMissing(_calib.xmit_power_dbm_v)) {
    return;
  }
  if (isMissing(_calib.radar_constant_h)) {
    return;
  }
  if (isMissing(_calib.radar_constant_v)) {
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
    = 10.0 * log10(pulseWidthUs / _calib.pulse_width_us);

  double xmitPowerRatioDbH = xmitPowerDbmH - _calib.xmit_power_dbm_h;
  double xmitPowerRatioDbV = xmitPowerDbmV - _calib.xmit_power_dbm_v;

  // adjust radar constant based on pulse width
  // decreases for increasing pulse width

  _calib.radar_constant_h -= pulseWidthRatioDb;
  _calib.radar_constant_v -= pulseWidthRatioDb;

  // adjust radar constant based on transmitter power
  // decreases for increasing power

  _calib.radar_constant_h -= xmitPowerRatioDbH;
  _calib.radar_constant_v -= xmitPowerRatioDbV;

  // compute base dbz

  if (!isMissing(_calib.noise_dbm_hc) &&
      !isMissing(_calib.receiver_gain_db_hc)) {
    _calib.base_dbz_1km_hc = (_calib.noise_dbm_hc - 
                              _calib.receiver_gain_db_hc + 
                              _calib.radar_constant_h);
  }

  if (!isMissing(_calib.noise_dbm_vc) &&
      !isMissing(_calib.receiver_gain_db_vc)) {
    _calib.base_dbz_1km_vc = (_calib.noise_dbm_vc - 
                              _calib.receiver_gain_db_vc + 
                              _calib.radar_constant_v);
  }

  if (!isMissing(_calib.noise_dbm_hx) &&
      !isMissing(_calib.receiver_gain_db_hx)) {
    _calib.base_dbz_1km_hx = (_calib.noise_dbm_hx - 
                              _calib.receiver_gain_db_hx + 
                              _calib.radar_constant_h);
  }

  if (!isMissing(_calib.noise_dbm_vx) &&
      !isMissing(_calib.receiver_gain_db_vx)) {
    _calib.base_dbz_1km_vx = (_calib.noise_dbm_vx - 
                              _calib.receiver_gain_db_vx + 
                              _calib.radar_constant_v);
  }

  _calib.xmit_power_dbm_h = xmitPowerDbmH;
  _calib.xmit_power_dbm_v = xmitPowerDbmV;
  
}

/////////////////////
// set the radar name

void IwrfCalib::setRadarName(const string &name)
{
  STRncopy(_calib.radar_name, name.c_str(), IWRF_MAX_RADAR_NAME);
}

///////////////////////////
// set the calibration time

void IwrfCalib::setCalibTime(time_t calTime)
{
  _calib.packet.time_secs_utc = calTime;
}

//////////////////
// set from struct

void IwrfCalib::set(const iwrf_calibration_t &calib)
{
  _calib = calib;
  iwrf_calibration_swap(_calib);
}

///////////////////////////
// get the radar name

string IwrfCalib::getRadarName() const
{
  return _calib.radar_name;
}

///////////////////////////
// get the calibration time

time_t IwrfCalib::getCalibTime() const
{
  return _calib.packet.time_secs_utc;
}

////////////////////////////////////////////
// convert to XML - load up xml string

void IwrfCalib::convert2Xml(string &xml)  const
{

  xml.clear();
  xml += TaXml::writeStartTag("IwrfCalib", 0);

  // name

  xml += TaXml::writeString("radarName", 1, _calib.radar_name);

  // time

  DateTime ctime(_calib.packet.time_secs_utc);
  xml += TaXml::writeTime("calibTime", 1, ctime.utime());
  
  // fields

  xml += TaXml::writeDouble("wavelengthCm", 1, _calib.wavelength_cm);
  xml += TaXml::writeDouble("beamWidthDegH", 1, _calib.beamwidth_deg_h);
  xml += TaXml::writeDouble("beamWidthDegV", 1, _calib.beamwidth_deg_v);
  xml += TaXml::writeDouble("antGainDbH", 1, _calib.gain_ant_db_h);
  xml += TaXml::writeDouble("antGainDbV", 1, _calib.gain_ant_db_v);
  xml += TaXml::writeDouble("pulseWidthUs", 1, _calib.pulse_width_us);
  xml += TaXml::writeDouble("xmitPowerDbmH", 1, _calib.xmit_power_dbm_h);
  xml += TaXml::writeDouble("xmitPowerDbmV", 1, _calib.xmit_power_dbm_v);
  xml += TaXml::writeDouble("twoWayWaveguideLossDbH",
                            1, _calib.two_way_waveguide_loss_db_h);
  xml += TaXml::writeDouble("twoWayWaveguideLossDbV",
                            1, _calib.two_way_waveguide_loss_db_v);
  xml += TaXml::writeDouble("twoWayRadomeLossDbH",
                            1, _calib.two_way_radome_loss_db_h);
  xml += TaXml::writeDouble("twoWayRadomeLossDbV",
                            1, _calib.two_way_radome_loss_db_v);
  xml += TaXml::writeDouble("receiverMismatchLossDb",
                            1, _calib.receiver_mismatch_loss_db);
  xml += TaXml::writeDouble("kSquaredWater", 1, _calib.k_squared_water);
  xml += TaXml::writeDouble("radarConstH", 1, _calib.radar_constant_h);
  xml += TaXml::writeDouble("radarConstV", 1, _calib.radar_constant_v);
  xml += TaXml::writeDouble("noiseDbmHc", 1, _calib.noise_dbm_hc);
  xml += TaXml::writeDouble("noiseDbmHx", 1, _calib.noise_dbm_hx);
  xml += TaXml::writeDouble("noiseDbmVc", 1, _calib.noise_dbm_vc);
  xml += TaXml::writeDouble("noiseDbmVx", 1, _calib.noise_dbm_vx);
  xml += TaXml::writeDouble("i0DbmHc", 1, _calib.i0_dbm_hc);
  xml += TaXml::writeDouble("i0DbmHx", 1, _calib.i0_dbm_hx);
  xml += TaXml::writeDouble("i0DbmVc", 1, _calib.i0_dbm_vc);
  xml += TaXml::writeDouble("i0DbmVx", 1, _calib.i0_dbm_vx);
  xml += TaXml::writeDouble("receiverGainDbHc", 1, _calib.receiver_gain_db_hc);
  xml += TaXml::writeDouble("receiverGainDbHx", 1, _calib.receiver_gain_db_hx);
  xml += TaXml::writeDouble("receiverGainDbVc", 1, _calib.receiver_gain_db_vc);
  xml += TaXml::writeDouble("receiverGainDbVx", 1, _calib.receiver_gain_db_vx);
  xml += TaXml::writeDouble("receiverSlopeDbHc", 1, _calib.receiver_slope_hc);
  xml += TaXml::writeDouble("receiverSlopeDbHx", 1, _calib.receiver_slope_hx);
  xml += TaXml::writeDouble("receiverSlopeDbVc", 1, _calib.receiver_slope_vc);
  xml += TaXml::writeDouble("receiverSlopeDbVx", 1, _calib.receiver_slope_vx);
  xml += TaXml::writeDouble("dynamicRangeDbHc", 1, _calib.dynamic_range_db_hc);
  xml += TaXml::writeDouble("dynamicRangeDbHx", 1, _calib.dynamic_range_db_hx);
  xml += TaXml::writeDouble("dynamicRangeDbVc", 1, _calib.dynamic_range_db_vc);
  xml += TaXml::writeDouble("dynamicRangeDbVx", 1, _calib.dynamic_range_db_vx);
  xml += TaXml::writeDouble("baseDbz1kmHc", 1, _calib.base_dbz_1km_hc);
  xml += TaXml::writeDouble("baseDbz1kmHx", 1, _calib.base_dbz_1km_hx);
  xml += TaXml::writeDouble("baseDbz1kmVc", 1, _calib.base_dbz_1km_vc);
  xml += TaXml::writeDouble("baseDbz1kmVx", 1, _calib.base_dbz_1km_vx);
  xml += TaXml::writeDouble("sunPowerDbmHc", 1, _calib.sun_power_dbm_hc);
  xml += TaXml::writeDouble("sunPowerDbmHx", 1, _calib.sun_power_dbm_hx);
  xml += TaXml::writeDouble("sunPowerDbmVc", 1, _calib.sun_power_dbm_vc);
  xml += TaXml::writeDouble("sunPowerDbmVx", 1, _calib.sun_power_dbm_vx);
  xml += TaXml::writeDouble("noiseSourcePowerDbmH",
                            1, _calib.noise_source_power_dbm_h);
  xml += TaXml::writeDouble("noiseSourcePowerDbmV",
                            1, _calib.noise_source_power_dbm_v);
  xml += TaXml::writeDouble("powerMeasLossDbH", 1, _calib.power_meas_loss_db_h);
  xml += TaXml::writeDouble("powerMeasLossDbV", 1, _calib.power_meas_loss_db_v);
  xml += TaXml::writeDouble("couplerForwardLossDbH",
                            1, _calib.coupler_forward_loss_db_h);
  xml += TaXml::writeDouble("couplerForwardLossDbV",
                            1, _calib.coupler_forward_loss_db_v);
  xml += TaXml::writeDouble("dbzCorrection", 1, _calib.dbz_correction);
  xml += TaXml::writeDouble("zdrCorrectionDb", 1, _calib.zdr_correction_db);
  xml += TaXml::writeDouble("ldrCorrectionDbH", 1, _calib.ldr_correction_db_h);
  xml += TaXml::writeDouble("ldrCorrectionDbV", 1, _calib.ldr_correction_db_v);
  xml += TaXml::writeDouble("systemPhidpDeg", 1, _calib.phidp_rot_deg);
  xml += TaXml::writeDouble("testPowerDbmH", 1, _calib.test_power_dbm_h);
  xml += TaXml::writeDouble("testPowerDbmV", 1, _calib.test_power_dbm_v);

  xml += TaXml::writeEndTag("IwrfCalib", 0);
}

////////////////////////////////////////////
// set from XML string
//
// Returns 0 on success, -1 on failure
// Sets errStr on failure

int IwrfCalib::setFromXml(const string &xmlBuf, string &errStr)
{

  iwrf_calibration_init(_calib);

  int iret = 0;
  string xbuf = TaXml::removeComments(xmlBuf);

  // set data

  string name;
  if (TaXml::readString(xbuf, "radarName", name) == 0) {
    STRncopy(_calib.radar_name, name.c_str(), sizeof(_calib.radar_name));
  }

  time_t ctime;
  if (TaXml::readTime(xbuf, "calibTime", ctime) == 0) {
    DateTime cTime(ctime); 
    _calib.packet.time_secs_utc = cTime.utime();
  } else {
    errStr += "ERROR - calibTime missing\n";
    iret = -1;
  }
  
  double val;

  if (TaXml::readDouble(xbuf, "wavelengthCm", val) == 0) {
    _calib.wavelength_cm = val;
  } else {
    errStr += "ERROR - wavelength_cm missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "beamWidthDegH", val) == 0) {
    _calib.beamwidth_deg_h = val;
  }

  if (TaXml::readDouble(xbuf, "beamWidthDegV", val) == 0) {
    _calib.beamwidth_deg_v = val;
  }

  if (TaXml::readDouble(xbuf, "antGainDbH", val) == 0) {
    _calib.gain_ant_db_h = val;
  }

  if (TaXml::readDouble(xbuf, "antGainDbV", val) == 0) {
    _calib.gain_ant_db_v = val;
  }

  if (TaXml::readDouble(xbuf, "pulseWidthUs", val) == 0) {
    _calib.pulse_width_us = val;
  } else {
    errStr += "ERROR - pulseWidthUs missing\n";
    iret = -1;
  }

  if (TaXml::readDouble(xbuf, "xmitPowerDbmH", val) == 0) {
    _calib.xmit_power_dbm_h = val;
  }

  if (TaXml::readDouble(xbuf, "xmitPowerDbmV", val) == 0) {
    _calib.xmit_power_dbm_v = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayWaveguideLossDbH", val) == 0) {
    _calib.two_way_waveguide_loss_db_h = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayWaveguideLossDbV", val) == 0) {
    _calib.two_way_waveguide_loss_db_v = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayRadomeLossDbH", val) == 0) {
    _calib.two_way_radome_loss_db_h = val;
  }

  if (TaXml::readDouble(xbuf, "twoWayRadomeLossDbV", val) == 0) {
    _calib.two_way_radome_loss_db_v = val;
  }

  if (TaXml::readDouble(xbuf, "receiverMismatchLossDb", val) == 0) {
    _calib.receiver_mismatch_loss_db = val;
  }

  if (TaXml::readDouble(xbuf, "kSquaredWater", val) == 0) {
    _calib.k_squared_water = val;
  }

  if (TaXml::readDouble(xbuf, "radarConstH", val) == 0) {
    _calib.radar_constant_h = val;
  }

  if (TaXml::readDouble(xbuf, "radarConstV", val) == 0) {
    _calib.radar_constant_v = val;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmHc", val) == 0) {
    _calib.noise_dbm_hc = val;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmHx", val) == 0) {
    _calib.noise_dbm_hx = val;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmVc", val) == 0) {
    _calib.noise_dbm_vc = val;
  }

  if (TaXml::readDouble(xbuf, "noiseDbmVx", val) == 0) {
    _calib.noise_dbm_vx = val;
  }

  if (TaXml::readDouble(xbuf, "i0DbmHc", val) == 0) {
    _calib.i0_dbm_hc = val;
  } else {
    _calib.i0_dbm_hc = _calib.noise_dbm_hc - _calib.receiver_gain_db_hc;
  }

  if (TaXml::readDouble(xbuf, "i0DbmHx", val) == 0) {
    _calib.i0_dbm_hx = val;
  } else {
    _calib.i0_dbm_hx = _calib.noise_dbm_hx - _calib.receiver_gain_db_hx;
  }

  if (TaXml::readDouble(xbuf, "i0DbmVc", val) == 0) {
    _calib.i0_dbm_vc = val;
  } else {
    _calib.i0_dbm_vc = _calib.noise_dbm_vc - _calib.receiver_gain_db_vc;
  }

  if (TaXml::readDouble(xbuf, "i0DbmVx", val) == 0) {
    _calib.i0_dbm_vx = val;
  } else {
    _calib.i0_dbm_vx = _calib.noise_dbm_vx - _calib.receiver_gain_db_vx;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbHc", val) == 0) {
    _calib.receiver_gain_db_hc = val;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbHx", val) == 0) {
    _calib.receiver_gain_db_hx = val;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbVc", val) == 0) {
    _calib.receiver_gain_db_vc = val;
  }

  if (TaXml::readDouble(xbuf, "receiverGainDbVx", val) == 0) {
    _calib.receiver_gain_db_vx = val;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbHc", val) == 0) {
    _calib.receiver_slope_hc = val;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbHx", val) == 0) {
    _calib.receiver_slope_hx = val;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbVc", val) == 0) {
    _calib.receiver_slope_vc = val;
  }

  if (TaXml::readDouble(xbuf, "receiverSlopeDbVx", val) == 0) {
    _calib.receiver_slope_vx = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbHc", val) == 0) {
    _calib.dynamic_range_db_hc = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbHx", val) == 0) {
    _calib.dynamic_range_db_hx = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbVc", val) == 0) {
    _calib.dynamic_range_db_vc = val;
  }

  if (TaXml::readDouble(xbuf, "dynamicRangeDbVx", val) == 0) {
    _calib.dynamic_range_db_vx = val;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmHc", val) == 0) {
    _calib.base_dbz_1km_hc = val;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmHx", val) == 0) {
    _calib.base_dbz_1km_hx = val;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmVc", val) == 0) {
    _calib.base_dbz_1km_vc = val;
  }

  if (TaXml::readDouble(xbuf, "baseDbz1kmVx", val) == 0) {
    _calib.base_dbz_1km_vx = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmHc", val) == 0) {
    _calib.sun_power_dbm_hc = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmHx", val) == 0) {
    _calib.sun_power_dbm_hx = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmVc", val) == 0) {
    _calib.sun_power_dbm_vc = val;
  }

  if (TaXml::readDouble(xbuf, "sunPowerDbmVx", val) == 0) {
    _calib.sun_power_dbm_vx = val;
  }

  if (TaXml::readDouble(xbuf, "noiseSourcePowerDbmH", val) == 0) {
    _calib.noise_source_power_dbm_h = val;
  }

  if (TaXml::readDouble(xbuf, "noiseSourcePowerDbmV", val) == 0) {
    _calib.noise_source_power_dbm_v = val;
  }

  if (TaXml::readDouble(xbuf, "powerMeasLossDbH", val) == 0) {
    _calib.power_meas_loss_db_h = val;
  }

  if (TaXml::readDouble(xbuf, "powerMeasLossDbV", val) == 0) {
    _calib.power_meas_loss_db_v = val;
  }

  if (TaXml::readDouble(xbuf, "couplerForwardLossDbH", val) == 0) {
    _calib.coupler_forward_loss_db_h = val;
  }

  if (TaXml::readDouble(xbuf, "couplerForwardLossDbV", val) == 0) {
    _calib.coupler_forward_loss_db_v = val;
  }

  if (TaXml::readDouble(xbuf, "dbzCorrection", val) == 0) {
    _calib.dbz_correction = val;
  }

  if (TaXml::readDouble(xbuf, "zdrCorrectionDb", val) == 0) {
    _calib.zdr_correction_db = val;
  }

  if (TaXml::readDouble(xbuf, "ldrCorrectionDbH", val) == 0) {
    _calib.ldr_correction_db_h = val;
  }

  if (TaXml::readDouble(xbuf, "ldrCorrectionDbV", val) == 0) {
    _calib.ldr_correction_db_v = val;
  }

  if (TaXml::readDouble(xbuf, "systemPhidpDeg", val) == 0) {
    _calib.phidp_rot_deg = val;
  }

  if (TaXml::readDouble(xbuf, "testPowerDbmH", val) == 0) {
    _calib.test_power_dbm_h = val;
  }

  if (TaXml::readDouble(xbuf, "testPowerDbmV", val) == 0) {
    _calib.test_power_dbm_v = val;
  }

  return iret;

}

////////////////////////////////////////////////////////////////////////
// read from a given XML cal file
//
// Returns 0 on success, -1 on failure
// Sets errStr on failure

int IwrfCalib::readFromXmlFile(const string &calPath, string &errStr)

{

  errStr = "ERROR - IwrfCalib::_readFromXmlFile\n";

  // Stat the file to get length
  
  struct stat calStat;
  if (ta_stat(calPath.c_str(), &calStat)) {
    int errNum = errno;
    TaStr::AddStr(errStr, "  Cannot stat file: ", calPath);
    TaStr::AddStr(errStr, "  ", strerror(errNum));
    return -1;
  }
  int fileLen = calStat.st_size;

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

void IwrfCalib::overrideFromStruct(const iwrf_calibration_t &calib)
{

  if (!isMissing(calib.wavelength_cm)) {
    _calib.wavelength_cm = calib.wavelength_cm;
  }
  if (!isMissing(calib.beamwidth_deg_h)) {
    _calib.beamwidth_deg_h = calib.beamwidth_deg_h;
  }
  if (!isMissing(calib.beamwidth_deg_v)) {
    _calib.beamwidth_deg_v = calib.beamwidth_deg_v;
  }
  if (!isMissing(calib.gain_ant_db_h)) {
    _calib.gain_ant_db_h = calib.gain_ant_db_h;
  }
  if (!isMissing(calib.gain_ant_db_v)) {
    _calib.gain_ant_db_v = calib.gain_ant_db_v;
  }
  if (!isMissing(calib.pulse_width_us)) {
    _calib.pulse_width_us = calib.pulse_width_us;
  }
  if (!isMissing(calib.xmit_power_dbm_h)) {
    _calib.xmit_power_dbm_h = calib.xmit_power_dbm_h;
  }
  if (!isMissing(calib.xmit_power_dbm_v)) {
    _calib.xmit_power_dbm_v = calib.xmit_power_dbm_v;
  }
  if (!isMissing(calib.two_way_waveguide_loss_db_h)) {
    _calib.two_way_waveguide_loss_db_h = calib.two_way_waveguide_loss_db_h;
  }
  if (!isMissing(calib.two_way_waveguide_loss_db_v)) {
    _calib.two_way_waveguide_loss_db_v = calib.two_way_waveguide_loss_db_v;
  }
  if (!isMissing(calib.two_way_radome_loss_db_h)) {
    _calib.two_way_radome_loss_db_h = calib.two_way_radome_loss_db_h;
  }
  if (!isMissing(calib.two_way_radome_loss_db_v)) {
    _calib.two_way_radome_loss_db_v = calib.two_way_radome_loss_db_v;
  }
  if (!isMissing(calib.receiver_mismatch_loss_db)) {
    _calib.receiver_mismatch_loss_db = calib.receiver_mismatch_loss_db;
  }
  if (!isMissing(calib.k_squared_water)) {
    _calib.k_squared_water = calib.k_squared_water;
  }
  if (!isMissing(calib.radar_constant_h)) {
    _calib.radar_constant_h = calib.radar_constant_h;
  }
  if (!isMissing(calib.radar_constant_v)) {
    _calib.radar_constant_v = calib.radar_constant_v;
  }
  if (!isMissing(calib.noise_dbm_hc)) {
    _calib.noise_dbm_hc = calib.noise_dbm_hc;
  }
  if (!isMissing(calib.noise_dbm_hx)) {
    _calib.noise_dbm_hx = calib.noise_dbm_hx;
  }
  if (!isMissing(calib.noise_dbm_vc)) {
    _calib.noise_dbm_vc = calib.noise_dbm_vc;
  }
  if (!isMissing(calib.noise_dbm_vx)) {
    _calib.noise_dbm_vx = calib.noise_dbm_vx;
  }
  if (!isMissing(calib.receiver_gain_db_hc)) {
    _calib.receiver_gain_db_hc = calib.receiver_gain_db_hc;
  }
  if (!isMissing(calib.receiver_gain_db_hx)) {
    _calib.receiver_gain_db_hx = calib.receiver_gain_db_hx;
  }
  if (!isMissing(calib.receiver_gain_db_vc)) {
    _calib.receiver_gain_db_vc = calib.receiver_gain_db_vc;
  }
  if (!isMissing(calib.receiver_gain_db_vx)) {
    _calib.receiver_gain_db_vx = calib.receiver_gain_db_vx;
  }
  if (!isMissing(calib.i0_dbm_hc)) {
    _calib.i0_dbm_hc = _calib.noise_dbm_hc - _calib.receiver_gain_db_hc;
  }
  if (!isMissing(calib.i0_dbm_hx)) {
    _calib.i0_dbm_hx = _calib.noise_dbm_hx - _calib.receiver_gain_db_hx;
  }
  if (!isMissing(calib.i0_dbm_vc)) {
    _calib.i0_dbm_vc = _calib.noise_dbm_vc - _calib.receiver_gain_db_vc;
  }
  if (!isMissing(calib.i0_dbm_vx)) {
    _calib.i0_dbm_vx = _calib.noise_dbm_vx - _calib.receiver_gain_db_vx;
  }
  if (!isMissing(calib.receiver_slope_hc)) {
    _calib.receiver_slope_hc = calib.receiver_slope_hc;
  }
  if (!isMissing(calib.receiver_slope_hx)) {
    _calib.receiver_slope_hx = calib.receiver_slope_hx;
  }
  if (!isMissing(calib.receiver_slope_vc)) {
    _calib.receiver_slope_vc = calib.receiver_slope_vc;
  }
  if (!isMissing(calib.receiver_slope_vx)) {
    _calib.receiver_slope_vx = calib.receiver_slope_vx;
  }
  if (!isMissing(calib.dynamic_range_db_hc)) {
    _calib.dynamic_range_db_hc = calib.dynamic_range_db_hc;
  }
  if (!isMissing(calib.dynamic_range_db_hx)) {
    _calib.dynamic_range_db_hx = calib.dynamic_range_db_hx;
  }
  if (!isMissing(calib.dynamic_range_db_vc)) {
    _calib.dynamic_range_db_vc = calib.dynamic_range_db_vc;
  }
  if (!isMissing(calib.dynamic_range_db_vx)) {
    _calib.dynamic_range_db_vx = calib.dynamic_range_db_vx;
  }
  if (!isMissing(calib.base_dbz_1km_hc)) {
    _calib.base_dbz_1km_hc = calib.base_dbz_1km_hc;
  }
  if (!isMissing(calib.base_dbz_1km_hx)) {
    _calib.base_dbz_1km_hx = calib.base_dbz_1km_hx;
  }
  if (!isMissing(calib.base_dbz_1km_vc)) {
    _calib.base_dbz_1km_vc = calib.base_dbz_1km_vc;
  }
  if (!isMissing(calib.base_dbz_1km_vx)) {
    _calib.base_dbz_1km_vx = calib.base_dbz_1km_vx;
  }
  if (!isMissing(calib.sun_power_dbm_hc)) {
    _calib.sun_power_dbm_hc = calib.sun_power_dbm_hc;
  }
  if (!isMissing(calib.sun_power_dbm_hx)) {
    _calib.sun_power_dbm_hx = calib.sun_power_dbm_hx;
  }
  if (!isMissing(calib.sun_power_dbm_vc)) {
    _calib.sun_power_dbm_vc = calib.sun_power_dbm_vc;
  }
  if (!isMissing(calib.sun_power_dbm_vx)) {
    _calib.sun_power_dbm_vx = calib.sun_power_dbm_vx;
  }
  if (!isMissing(calib.noise_source_power_dbm_h)) {
    _calib.noise_source_power_dbm_h = calib.noise_source_power_dbm_h;
  }
  if (!isMissing(calib.noise_source_power_dbm_v)) {
    _calib.noise_source_power_dbm_v = calib.noise_source_power_dbm_v;
  }
  if (!isMissing(calib.power_meas_loss_db_h)) {
    _calib.power_meas_loss_db_h = calib.power_meas_loss_db_h;
  }
  if (!isMissing(calib.power_meas_loss_db_v)) {
    _calib.power_meas_loss_db_v = calib.power_meas_loss_db_v;
  }
  if (!isMissing(calib.coupler_forward_loss_db_h)) {
    _calib.coupler_forward_loss_db_h = calib.coupler_forward_loss_db_h;
  }
  if (!isMissing(calib.coupler_forward_loss_db_v)) {
    _calib.coupler_forward_loss_db_v = calib.coupler_forward_loss_db_v;
  }
  if (!isMissing(calib.dbz_correction)) {
    _calib.dbz_correction = calib.dbz_correction;
  }
  if (!isMissing(calib.zdr_correction_db)) {
    _calib.zdr_correction_db = calib.zdr_correction_db;
  }
  if (!isMissing(calib.ldr_correction_db_h)) {
    _calib.ldr_correction_db_h = calib.ldr_correction_db_h;
  }
  if (!isMissing(calib.ldr_correction_db_v)) {
    _calib.ldr_correction_db_v = calib.ldr_correction_db_v;
  }
  if (!isMissing(calib.phidp_rot_deg)) {
    _calib.phidp_rot_deg = calib.phidp_rot_deg;
  }
  if (!isMissing(calib.test_power_dbm_h)) {
    _calib.test_power_dbm_h = calib.test_power_dbm_h;
  }
  if (!isMissing(calib.test_power_dbm_v)) {
    _calib.test_power_dbm_v = calib.test_power_dbm_v;
  }

}

// get methods

double IwrfCalib::getWavelengthCm() const
{
  return _calib.wavelength_cm;
}

double IwrfCalib::getWavelengthM() const
{
  return _calib.wavelength_cm / 100.0;
}

double IwrfCalib::getFrequencyHz() const
{
  double wavelengthM = _calib.wavelength_cm / 100.0;
  double freqHz = LightSpeedMps / wavelengthM;
  return freqHz;
}

void
IwrfCalib::print(ostream &out)  const
{
   
  out << "IWRF CALIB" << endl;
  out << "----------" << endl;
   
  DateTime ctime(_calib.packet.time_secs_utc);
  out << "  radar_name: " << _calib.radar_name << endl;
  out << "  time: " << DateTime::strm(ctime.utime()) << endl;

  out << "  wavelength_cm: " << _calib.wavelength_cm << endl;
  out << "  beamwidth_deg_h: " << _calib.beamwidth_deg_h << endl;
  out << "  beamwidth_deg_v: " << _calib.beamwidth_deg_v << endl;
  out << "  gain_ant_db_h: " << _calib.gain_ant_db_h << endl;
  out << "  gain_ant_db_v: " << _calib.gain_ant_db_v << endl;
  out << "  pulseWidthUs: " << _calib.pulse_width_us << endl;
  out << "  xmitPowerDbmH: " << _calib.xmit_power_dbm_h << endl;
  out << "  xmitPowerDbmV: " << _calib.xmit_power_dbm_v << endl;
  out << "  twoWayWaveguideLossDbH: " << _calib.two_way_waveguide_loss_db_h << endl;
  out << "  twoWayWaveguideLossDbV: " << _calib.two_way_waveguide_loss_db_v << endl;
  out << "  twoWayRadomeLossDbH: " << _calib.two_way_radome_loss_db_h << endl;
  out << "  twoWayRadomeLossDbV: " << _calib.two_way_radome_loss_db_v << endl;
  out << "  receiverMismatchLossDb: " << _calib.receiver_mismatch_loss_db << endl;
  out << "  kSquaredWater: " << _calib.k_squared_water << endl;
  out << "  radarConstH: " << _calib.radar_constant_h << endl;
  out << "  radarConstV: " << _calib.radar_constant_v << endl;
  out << "  noiseDbmHc: " << _calib.noise_dbm_hc << endl;
  out << "  noiseDbmHx: " << _calib.noise_dbm_hx << endl;
  out << "  noiseDbmVc: " << _calib.noise_dbm_vc << endl;
  out << "  noiseDbmVx: " << _calib.noise_dbm_vx << endl;
  out << "  i0DbmHc: " << _calib.i0_dbm_hc << endl;
  out << "  i0DbmHx: " << _calib.i0_dbm_hx << endl;
  out << "  i0DbmVc: " << _calib.i0_dbm_vc << endl;
  out << "  i0DbmVx: " << _calib.i0_dbm_vx << endl;
  out << "  receiverGainDbHc: " << _calib.receiver_gain_db_hc << endl;
  out << "  receiverGainDbHx: " << _calib.receiver_gain_db_hx << endl;
  out << "  receiverGainDbVc: " << _calib.receiver_gain_db_vc << endl;
  out << "  receiverGainDbVx: " << _calib.receiver_gain_db_vx << endl;
  out << "  receiverSlopeDbHc: " << _calib.receiver_slope_hc << endl;
  out << "  receiverSlopeDbHx: " << _calib.receiver_slope_hx << endl;
  out << "  receiverSlopeDbVc: " << _calib.receiver_slope_vc << endl;
  out << "  receiverSlopeDbVx: " << _calib.receiver_slope_vx << endl;
  out << "  dynamicRangeDbHc: " << _calib.dynamic_range_db_hc << endl;
  out << "  dynamicRangeDbHx: " << _calib.dynamic_range_db_hx << endl;
  out << "  dynamicRangeDbVc: " << _calib.dynamic_range_db_vc << endl;
  out << "  dynamicRangeDbVx: " << _calib.dynamic_range_db_vx << endl;
  out << "  baseDbz1kmHc: " << _calib.base_dbz_1km_hc << endl;
  out << "  baseDbz1kmHx: " << _calib.base_dbz_1km_hx << endl;
  out << "  baseDbz1kmVc: " << _calib.base_dbz_1km_vc << endl;
  out << "  baseDbz1kmVx: " << _calib.base_dbz_1km_vx << endl;
  out << "  sunPowerDbmHc: " << _calib.sun_power_dbm_hc << endl;
  out << "  sunPowerDbmHx: " << _calib.sun_power_dbm_hx << endl;
  out << "  sunPowerDbmVc: " << _calib.sun_power_dbm_vc << endl;
  out << "  sunPowerDbmVx: " << _calib.sun_power_dbm_vx << endl;
  out << "  noiseSourcePowerDbmH: " << _calib.noise_source_power_dbm_h << endl;
  out << "  noiseSourcePowerDbmV: " << _calib.noise_source_power_dbm_v << endl;
  out << "  powerMeasLossDbH: " << _calib.power_meas_loss_db_h << endl;
  out << "  powerMeasLossDbV: " << _calib.power_meas_loss_db_v << endl;
  out << "  couplerForwardLossDbH: " << _calib.coupler_forward_loss_db_h << endl;
  out << "  couplerForwardLossDbV: " << _calib.coupler_forward_loss_db_v << endl;
  out << "  dbzCorrection: " << _calib.dbz_correction << endl;
  out << "  zdrCorrectionDb: " << _calib.zdr_correction_db << endl;
  out << "  ldrCorrectionDbH: " << _calib.ldr_correction_db_h << endl;
  out << "  ldrCorrectionDbV: " << _calib.ldr_correction_db_v << endl;
  out << "  systemPhidpDeg: " << _calib.phidp_rot_deg << endl;
  out << "  testPowerDbmH: " << _calib.test_power_dbm_h << endl;
  out << "  testPowerDbmV: " << _calib.test_power_dbm_v << endl;

  out << endl;

}

