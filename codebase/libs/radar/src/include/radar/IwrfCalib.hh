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
// IwrfCalib.hh
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

#ifndef IwrfCalib_hh
#define IwrfCalib_hh

#include <string>
#include <radar/iwrf_data.h>
class RadxRcalib;
class DsRadarCalib;
using namespace std;

class IwrfCalib
{
public:
  
  IwrfCalib();
  ~IwrfCalib();

  static double LightSpeedMps;

  // Is a value missing?

  bool isMissing(fl32 val);

  // adjust based on pulse width and power
  
  void adjustRadarConst(double pulseWidthUs,
                        double xmitPowerDbmH,
                        double xmitPowerDbmV);

  // set methods

  void setRadarName(const string &name);
  void setCalibTime(time_t calTime);
  
  // field-specific set methods
  
  inline void setWavelengthCm(double val) { _calib.wavelength_cm = val; }
  inline void setBeamWidthDegH(double val) { _calib.beamwidth_deg_h = val; }
  inline void setBeamWidthDegV(double val) { _calib.beamwidth_deg_v = val; }
  inline void setAntGainDbH(double val) { _calib.gain_ant_db_h = val; }
  inline void setAntGainDbV(double val) { _calib.gain_ant_db_v = val; }

  inline void setPulseWidthUs(double val) { _calib.pulse_width_us = val; }
  inline void setXmitPowerDbmH(double val) { _calib.xmit_power_dbm_h = val; }
  inline void setXmitPowerDbmV(double val) { _calib.xmit_power_dbm_v = val; }

  inline void setTwoWayWaveguideLossDbH(double val) { _calib.two_way_waveguide_loss_db_h = val; }
  inline void setTwoWayWaveguideLossDbV(double val) { _calib.two_way_waveguide_loss_db_v = val; }
  inline void setTwoWayRadomeLossDbH(double val) { _calib.two_way_radome_loss_db_h = val; }
  inline void setTwoWayRadomeLossDbV(double val) { _calib.two_way_radome_loss_db_v = val; }

  inline void setReceiverMismatchLossDb(double val) { _calib.receiver_mismatch_loss_db = val; }

  inline void setKSquaredWater(double val) { _calib.k_squared_water = val; }

  inline void setRadarConstH(double val) { _calib.radar_constant_h = val; }
  inline void setRadarConstV(double val) { _calib.radar_constant_v = val; }

  inline void setNoiseDbmHc(double val) { _calib.noise_dbm_hc = val; }
  inline void setNoiseDbmHx(double val) { _calib.noise_dbm_hx = val; }
  inline void setNoiseDbmVc(double val) { _calib.noise_dbm_vc = val; }
  inline void setNoiseDbmVx(double val) { _calib.noise_dbm_vx = val; }

  inline void setI0DbmHc(double val) { _calib.i0_dbm_hc = val; }
  inline void setI0DbmHx(double val) { _calib.i0_dbm_hx = val; }
  inline void setI0DbmVc(double val) { _calib.i0_dbm_vc = val; }
  inline void setI0DbmVx(double val) { _calib.i0_dbm_vx = val; }

  inline void setReceiverGainDbHc(double val) { _calib.receiver_gain_db_hc = val; }
  inline void setReceiverGainDbHx(double val) { _calib.receiver_gain_db_hx = val; }
  inline void setReceiverGainDbVc(double val) { _calib.receiver_gain_db_vc = val; }
  inline void setReceiverGainDbVx(double val) { _calib.receiver_gain_db_vx = val; }

  inline void setReceiverSlopeDbHc(double val) { _calib.receiver_slope_hc = val; }
  inline void setReceiverSlopeDbHx(double val) { _calib.receiver_slope_hx = val; }
  inline void setReceiverSlopeDbVc(double val) { _calib.receiver_slope_vc = val; }
  inline void setReceiverSlopeDbVx(double val) { _calib.receiver_slope_vx = val; }

  inline void setBaseDbz1kmHc(double val) { _calib.base_dbz_1km_hc = val; }
  inline void setBaseDbz1kmHx(double val) { _calib.base_dbz_1km_hx = val; }
  inline void setBaseDbz1kmVc(double val) { _calib.base_dbz_1km_vc = val; }
  inline void setBaseDbz1kmVx(double val) { _calib.base_dbz_1km_vx = val; }

  inline void setDynamicRangeDbHc(double val) { _calib.dynamic_range_db_hc = val; }
  inline void setDynamicRangeDbHx(double val) { _calib.dynamic_range_db_hx = val; }
  inline void setDynamicRangeDbVc(double val) { _calib.dynamic_range_db_vc = val; }
  inline void setDynamicRangeDbVx(double val) { _calib.dynamic_range_db_vx = val; }

  inline void setSunPowerDbmHc(double val) { _calib.sun_power_dbm_hc = val; }
  inline void setSunPowerDbmHx(double val) { _calib.sun_power_dbm_hx = val; }
  inline void setSunPowerDbmVc(double val) { _calib.sun_power_dbm_vc = val; }
  inline void setSunPowerDbmVx(double val) { _calib.sun_power_dbm_vx = val; }

  inline void setNoiseSourcePowerDbmH(double val) { _calib.noise_source_power_dbm_h = val; }
  inline void setNoiseSourcePowerDbmV(double val) { _calib.noise_source_power_dbm_v = val; }

  inline void setPowerMeasLossDbH(double val) { _calib.power_meas_loss_db_h = val; }
  inline void setPowerMeasLossDbV(double val) { _calib.power_meas_loss_db_v = val; }

  inline void setCouplerForwardLossDbH(double val) { _calib.coupler_forward_loss_db_h = val; }
  inline void setCouplerForwardLossDbV(double val) { _calib.coupler_forward_loss_db_v = val; }

  inline void setDbzCorrection(double val) { _calib.dbz_correction = val; }
  inline void setZdrCorrectionDb(double val) { _calib.zdr_correction_db = val; }
  inline void setLdrCorrectionDbH(double val) { _calib.ldr_correction_db_h = val; }
  inline void setLdrCorrectionDbV(double val) { _calib.ldr_correction_db_v = val; }
  inline void setSystemPhidpDeg(double val) { _calib.phidp_rot_deg = val; }

  inline void setTestPowerDbmH(double val) { _calib.test_power_dbm_h = val; }
  inline void setTestPowerDbmV(double val) { _calib.test_power_dbm_v = val; }

  // set from struct
  
  void set(const iwrf_calibration_t &calib);

  // get values
  
  string getRadarName() const;
  time_t getCalibTime() const;
  
  double getWavelengthCm() const;
  double getWavelengthM() const;
  double getFrequencyHz() const;
  inline double getBeamWidthDegH() const { return _calib.beamwidth_deg_h; }
  inline double getBeamWidthDegV() const { return _calib.beamwidth_deg_v; }
  inline double getAntGainDbH() const { return _calib.gain_ant_db_h; }
  inline double getAntGainDbV() const { return _calib.gain_ant_db_v; }

  inline double getPulseWidthUs() const { return _calib.pulse_width_us; }
  inline double getXmitPowerDbmH() const { return _calib.xmit_power_dbm_h; }
  inline double getXmitPowerDbmV() const { return _calib.xmit_power_dbm_v; }

  inline double getTwoWayWaveguideLossDbH() const
  { return _calib.two_way_waveguide_loss_db_h; }
  inline double getTwoWayWaveguideLossDbV() const
  { return _calib.two_way_waveguide_loss_db_v; }
  
  inline double getTwoWayRadomeLossDbH() const { return _calib.two_way_radome_loss_db_h; }
  inline double getTwoWayRadomeLossDbV() const { return _calib.two_way_radome_loss_db_v; }
  
  inline double getReceiverMismatchLossDb() const
  { return _calib.receiver_mismatch_loss_db; }
  
  inline double getKSquaredWater() const { return _calib.k_squared_water; }

  inline double getRadarConstH() const { return _calib.radar_constant_h; }
  inline double getRadarConstV() const { return _calib.radar_constant_v; }

  inline double getNoiseDbmHc() const { return _calib.noise_dbm_hc; }
  inline double getNoiseDbmHx() const { return _calib.noise_dbm_hx; }
  inline double getNoiseDbmVc() const { return _calib.noise_dbm_vc; }
  inline double getNoiseDbmVx() const { return _calib.noise_dbm_vx; }

  inline double getI0DbmHc() const { return _calib.i0_dbm_hc; }
  inline double getI0DbmHx() const { return _calib.i0_dbm_hx; }
  inline double getI0DbmVc() const { return _calib.i0_dbm_vc; }
  inline double getI0DbmVx() const { return _calib.i0_dbm_vx; }

  inline double getReceiverGainDbHc() const { return _calib.receiver_gain_db_hc; }
  inline double getReceiverGainDbHx() const { return _calib.receiver_gain_db_hx; }
  inline double getReceiverGainDbVc() const { return _calib.receiver_gain_db_vc; }
  inline double getReceiverGainDbVx() const { return _calib.receiver_gain_db_vx; }

  inline double getReceiverSlopeDbHc() const { return _calib.receiver_slope_hc; }
  inline double getReceiverSlopeDbHx() const { return _calib.receiver_slope_hx; }
  inline double getReceiverSlopeDbVc() const { return _calib.receiver_slope_vc; }
  inline double getReceiverSlopeDbVx() const { return _calib.receiver_slope_vx; }

  inline double getDynamicRangeDbHc() const { return _calib.dynamic_range_db_hc; }
  inline double getDynamicRangeDbHx() const { return _calib.dynamic_range_db_hx; }
  inline double getDynamicRangeDbVc() const { return _calib.dynamic_range_db_vc; }
  inline double getDynamicRangeDbVx() const { return _calib.dynamic_range_db_vx; }

  inline double getBaseDbz1kmHc() const { return _calib.base_dbz_1km_hc; }
  inline double getBaseDbz1kmHx() const { return _calib.base_dbz_1km_hx; }
  inline double getBaseDbz1kmVc() const { return _calib.base_dbz_1km_vc; }
  inline double getBaseDbz1kmVx() const { return _calib.base_dbz_1km_vx; }

  inline double getSunPowerDbmHc() const { return _calib.sun_power_dbm_hc; }
  inline double getSunPowerDbmHx() const { return _calib.sun_power_dbm_hx; }
  inline double getSunPowerDbmVc() const { return _calib.sun_power_dbm_vc; }
  inline double getSunPowerDbmVx() const { return _calib.sun_power_dbm_vx; }

  inline double getNoiseSourcePowerDbmH() const
  { return _calib.noise_source_power_dbm_h; }
  inline double getNoiseSourcePowerDbmV() const 
  { return _calib.noise_source_power_dbm_v; }

  inline double getPowerMeasLossDbH() const { return _calib.power_meas_loss_db_h; }
  inline double getPowerMeasLossDbV() const { return _calib.power_meas_loss_db_v; }

  inline double getCouplerForwardLossDbH() const 
  { return _calib.coupler_forward_loss_db_h; }
  inline double getCouplerForwardLossDbV() const 
  { return _calib.coupler_forward_loss_db_v; }

  inline double getDbzCorrection() const { return _calib.dbz_correction; }
  inline double getZdrCorrectionDb() const { return _calib.zdr_correction_db; }
  inline double getLdrCorrectionDbH() const { return _calib.ldr_correction_db_h; }
  inline double getLdrCorrectionDbV() const { return _calib.ldr_correction_db_v; }
  inline double getSystemPhidpDeg() const { return _calib.phidp_rot_deg; }

  inline double getTestPowerDbmH() const { return _calib.test_power_dbm_h; }
  inline double getTestPowerDbmV() const { return _calib.test_power_dbm_v; }

  // get struct value, in place
  
  inline const iwrf_calibration_t &getStruct() const { return _calib; }

  // get struct value in Big-Endian byte order
  
  iwrf_calibration_t getStructAsBE() const;
 
  // encode struct into Big-Endian byte order

  void encode(iwrf_calibration_t *calib) const;

  // convert to XML - load up xml string
  
  void convert2Xml(string &xml) const;
  
  // set from XML string
  // returns 0 on success, -1 on error
  // Sets errStr on failure
  
  int setFromXml(const string &xml, string &errStr);
  
  // read from a given XML cal file
  // Returns 0 on success, -1 on failure
  // Sets errStr on failure
  
  int readFromXmlFile(const string &calPath, string &errStr);

  // override from struct, if struct member data is not missing
  
  void overrideFromStruct(const iwrf_calibration_t &calib);

  // print

  void print(ostream &out) const;

private:

  /* values are stored in iwrf_calibration_t struct */

  iwrf_calibration_t _calib;

  void _addToXml(const string &tag, double val, string &xml);

};

#endif
