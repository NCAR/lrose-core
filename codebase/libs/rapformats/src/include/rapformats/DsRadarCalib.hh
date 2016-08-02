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
// DsRadarCalb.hh
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

#ifndef _DS_RADAR_CALIB_INC_
#define _DS_RADAR_CALIB_INC_

#include <string>
#include <rapformats/ds_radar_calib.h>
using namespace std;

class DsRadarCalib
{
public:
  
  DsRadarCalib();
  ~DsRadarCalib();

  // Is a value missing?

  bool isMissing(double val);

  // adjust based on pulse width and power
  
  void adjustRadarConst(double pulseWidthUs,
                        double xmitPowerDbmH,
                        double xmitPowerDbmV);

  // set methods

  void setRadarName(const string &name);
  void setCalibTime(time_t calTime);
  
  // field-specific set methods
  
  inline void setWavelengthCm(double val) { _calib.wavelengthCm = val; }
  inline void setBeamWidthDegH(double val) { _calib.beamWidthDegH = val; }
  inline void setBeamWidthDegV(double val) { _calib.beamWidthDegV = val; }
  inline void setAntGainDbH(double val) { _calib.antGainDbH = val; }
  inline void setAntGainDbV(double val) { _calib.antGainDbV = val; }

  inline void setPulseWidthUs(double val) { _calib.pulseWidthUs = val; }
  inline void setXmitPowerDbmH(double val) { _calib.xmitPowerDbmH = val; }
  inline void setXmitPowerDbmV(double val) { _calib.xmitPowerDbmV = val; }

  inline void setTwoWayWaveguideLossDbH(double val) { _calib.twoWayWaveguideLossDbH = val; }
  inline void setTwoWayWaveguideLossDbV(double val) { _calib.twoWayWaveguideLossDbV = val; }
  inline void setTwoWayRadomeLossDbH(double val) { _calib.twoWayRadomeLossDbH = val; }
  inline void setTwoWayRadomeLossDbV(double val) { _calib.twoWayRadomeLossDbV = val; }

  inline void setReceiverMismatchLossDb(double val) { _calib.receiverMismatchLossDb = val; }

  inline void setKSquaredWater(double val) { _calib.kSquaredWater = val; }

  inline void setRadarConstH(double val) { _calib.radarConstH = val; }
  inline void setRadarConstV(double val) { _calib.radarConstV = val; }

  inline void setNoiseDbmHc(double val) { _calib.noiseDbmHc = val; }
  inline void setNoiseDbmHx(double val) { _calib.noiseDbmHx = val; }
  inline void setNoiseDbmVc(double val) { _calib.noiseDbmVc = val; }
  inline void setNoiseDbmVx(double val) { _calib.noiseDbmVx = val; }

  inline void setI0DbmHc(double val) { _calib.i0DbmHc = val; }
  inline void setI0DbmHx(double val) { _calib.i0DbmHx = val; }
  inline void setI0DbmVc(double val) { _calib.i0DbmVc = val; }
  inline void setI0DbmVx(double val) { _calib.i0DbmVx = val; }

  inline void setReceiverGainDbHc(double val) { _calib.receiverGainDbHc = val; }
  inline void setReceiverGainDbHx(double val) { _calib.receiverGainDbHx = val; }
  inline void setReceiverGainDbVc(double val) { _calib.receiverGainDbVc = val; }
  inline void setReceiverGainDbVx(double val) { _calib.receiverGainDbVx = val; }

  inline void setReceiverSlopeDbHc(double val) { _calib.receiverSlopeDbHc = val; }
  inline void setReceiverSlopeDbHx(double val) { _calib.receiverSlopeDbHx = val; }
  inline void setReceiverSlopeDbVc(double val) { _calib.receiverSlopeDbVc = val; }
  inline void setReceiverSlopeDbVx(double val) { _calib.receiverSlopeDbVx = val; }

  inline void setBaseDbz1kmHc(double val) { _calib.baseDbz1kmHc = val; }
  inline void setBaseDbz1kmHx(double val) { _calib.baseDbz1kmHx = val; }
  inline void setBaseDbz1kmVc(double val) { _calib.baseDbz1kmVc = val; }
  inline void setBaseDbz1kmVx(double val) { _calib.baseDbz1kmVx = val; }

  inline void setDynamicRangeDbHc(double val) { _calib.dynamicRangeDbHc = val; }
  inline void setDynamicRangeDbHx(double val) { _calib.dynamicRangeDbHx = val; }
  inline void setDynamicRangeDbVc(double val) { _calib.dynamicRangeDbVc = val; }
  inline void setDynamicRangeDbVx(double val) { _calib.dynamicRangeDbVx = val; }

  inline void setSunPowerDbmHc(double val) { _calib.sunPowerDbmHc = val; }
  inline void setSunPowerDbmHx(double val) { _calib.sunPowerDbmHx = val; }
  inline void setSunPowerDbmVc(double val) { _calib.sunPowerDbmVc = val; }
  inline void setSunPowerDbmVx(double val) { _calib.sunPowerDbmVx = val; }

  inline void setNoiseSourcePowerDbmH(double val) { _calib.noiseSourcePowerDbmH = val; }
  inline void setNoiseSourcePowerDbmV(double val) { _calib.noiseSourcePowerDbmV = val; }

  inline void setPowerMeasLossDbH(double val) { _calib.powerMeasLossDbH = val; }
  inline void setPowerMeasLossDbV(double val) { _calib.powerMeasLossDbV = val; }

  inline void setCouplerForwardLossDbH(double val) { _calib.couplerForwardLossDbH = val; }
  inline void setCouplerForwardLossDbV(double val) { _calib.couplerForwardLossDbV = val; }

  inline void setDbzCorrection(double val) { _calib.dbzCorrection = val; }
  inline void setZdrCorrectionDb(double val) { _calib.zdrCorrectionDb = val; }
  inline void setLdrCorrectionDbH(double val) { _calib.ldrCorrectionDbH = val; }
  inline void setLdrCorrectionDbV(double val) { _calib.ldrCorrectionDbV = val; }
  inline void setSystemPhidpDeg(double val) { _calib.systemPhidpDeg = val; }

  inline void setTestPowerDbmH(double val) { _calib.testPowerDbmH = val; }
  inline void setTestPowerDbmV(double val) { _calib.testPowerDbmV = val; }

  // set from struct
  
  void set(const ds_radar_calib_t &calib);

  // set from struct in Big-Endian byte order
  
  void setStructFromBE(const ds_radar_calib_t &calib);
  void decode(const ds_radar_calib_t *calib);

  // get values

  string getRadarName() const;
  time_t getCalibTime() const;
  
  inline double getWavelengthCm() const { return _calib.wavelengthCm; }
  inline double getBeamWidthDegH() const { return _calib.beamWidthDegH; }
  inline double getBeamWidthDegV() const { return _calib.beamWidthDegV; }
  inline double getAntGainDbH() const { return _calib.antGainDbH; }
  inline double getAntGainDbV() const { return _calib.antGainDbV; }

  inline double getPulseWidthUs() const { return _calib.pulseWidthUs; }
  inline double getXmitPowerDbmH() const { return _calib.xmitPowerDbmH; }
  inline double getXmitPowerDbmV() const { return _calib.xmitPowerDbmV; }

  inline double getTwoWayWaveguideLossDbH() const
  { return _calib.twoWayWaveguideLossDbH; }
  inline double getTwoWayWaveguideLossDbV() const
  { return _calib.twoWayWaveguideLossDbV; }
  
  inline double getTwoWayRadomeLossDbH() const { return _calib.twoWayRadomeLossDbH; }
  inline double getTwoWayRadomeLossDbV() const { return _calib.twoWayRadomeLossDbV; }
  
  inline double getReceiverMismatchLossDb() const
  { return _calib.receiverMismatchLossDb; }
  
  inline double getKSquaredWater() const { return _calib.kSquaredWater; }

  inline double getRadarConstH() const { return _calib.radarConstH; }
  inline double getRadarConstV() const { return _calib.radarConstV; }

  inline double getNoiseDbmHc() const { return _calib.noiseDbmHc; }
  inline double getNoiseDbmHx() const { return _calib.noiseDbmHx; }
  inline double getNoiseDbmVc() const { return _calib.noiseDbmVc; }
  inline double getNoiseDbmVx() const { return _calib.noiseDbmVx; }

  inline double getI0DbmHc() const { return _calib.i0DbmHc; }
  inline double getI0DbmHx() const { return _calib.i0DbmHx; }
  inline double getI0DbmVc() const { return _calib.i0DbmVc; }
  inline double getI0DbmVx() const { return _calib.i0DbmVx; }

  inline double getReceiverGainDbHc() const { return _calib.receiverGainDbHc; }
  inline double getReceiverGainDbHx() const { return _calib.receiverGainDbHx; }
  inline double getReceiverGainDbVc() const { return _calib.receiverGainDbVc; }
  inline double getReceiverGainDbVx() const { return _calib.receiverGainDbVx; }

  inline double getReceiverSlopeDbHc() const { return _calib.receiverSlopeDbHc; }
  inline double getReceiverSlopeDbHx() const { return _calib.receiverSlopeDbHx; }
  inline double getReceiverSlopeDbVc() const { return _calib.receiverSlopeDbVc; }
  inline double getReceiverSlopeDbVx() const { return _calib.receiverSlopeDbVx; }

  inline double getDynamicRangeDbHc() const { return _calib.dynamicRangeDbHc; }
  inline double getDynamicRangeDbHx() const { return _calib.dynamicRangeDbHx; }
  inline double getDynamicRangeDbVc() const { return _calib.dynamicRangeDbVc; }
  inline double getDynamicRangeDbVx() const { return _calib.dynamicRangeDbVx; }

  inline double getBaseDbz1kmHc() const { return _calib.baseDbz1kmHc; }
  inline double getBaseDbz1kmHx() const { return _calib.baseDbz1kmHx; }
  inline double getBaseDbz1kmVc() const { return _calib.baseDbz1kmVc; }
  inline double getBaseDbz1kmVx() const { return _calib.baseDbz1kmVx; }

  inline double getSunPowerDbmHc() const { return _calib.sunPowerDbmHc; }
  inline double getSunPowerDbmHx() const { return _calib.sunPowerDbmHx; }
  inline double getSunPowerDbmVc() const { return _calib.sunPowerDbmVc; }
  inline double getSunPowerDbmVx() const { return _calib.sunPowerDbmVx; }

  inline double getNoiseSourcePowerDbmH() const
  { return _calib.noiseSourcePowerDbmH; }
  inline double getNoiseSourcePowerDbmV() const 
  { return _calib.noiseSourcePowerDbmV; }

  inline double getPowerMeasLossDbH() const { return _calib.powerMeasLossDbH; }
  inline double getPowerMeasLossDbV() const { return _calib.powerMeasLossDbV; }

  inline double getCouplerForwardLossDbH() const 
  { return _calib.couplerForwardLossDbH; }
  inline double getCouplerForwardLossDbV() const 
  { return _calib.couplerForwardLossDbV; }

  inline double getDbzCorrection() const { return _calib.dbzCorrection; }
  inline double getZdrCorrectionDb() const { return _calib.zdrCorrectionDb; }
  inline double getLdrCorrectionDbH() const { return _calib.ldrCorrectionDbH; }
  inline double getLdrCorrectionDbV() const { return _calib.ldrCorrectionDbV; }
  inline double getSystemPhidpDeg() const { return _calib.systemPhidpDeg; }

  inline double getTestPowerDbmH() const { return _calib.testPowerDbmH; }
  inline double getTestPowerDbmV() const { return _calib.testPowerDbmV; }

  // get struct value, in place
  
  inline const ds_radar_calib_t &getStruct() const { return _calib; }

  // get struct value in Big-Endian byte order
  
  ds_radar_calib_t getStructAsBE() const;
 
  // encode struct into Big-Endian byte order

  void encode(ds_radar_calib_t *calib) const;

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
  
  void overrideFromStruct(const ds_radar_calib_t &calib);

  // print

  void print(FILE *out=stdout) const;
  void print(ostream &out) const;

private:

  /* values are stored in ds_radar_calib_t struct */
  
  ds_radar_calib_t _calib;

  void _addToXml(const string &tag, double val, string &xml);

};

#endif
