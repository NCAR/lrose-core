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
// RadxRcalib.hh
//
// Radar calibration
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

#ifndef NCF_CALIB_RADAR_HH
#define NCF_CALIB_RADAR_HH

#include <Radx/Radx.hh>
class RadxMsg;
using namespace std;

///////////////////////////////////////////////////////////////////
/// RADAR CALIBRATION CLASS
///
/// This class handles the data associated with a radar calibration.
///
/// If a radar operates with only a single pulse width, it is likely
/// that a single calibration will be sufficient. If multiple pulse
/// widths may be used, there will probably be one calibration per
/// pulse width.
///
/// NOTE: the default copy constructor and assignment methods work for
/// objects in this class.

class RadxRcalib
{
public:

  /// Constructor

  RadxRcalib();

  /// re-initialize

  void init();
  
  /// Destructor

  ~RadxRcalib();
  
  /// Check whether a value matches the missing value
  
  bool isMissing(double val);
  
  /// Adjust the radar constant, based on the pulse width and
  /// the measured power.
  ///
  /// This is used to adjust for the fact that the transmitter power
  /// measured during operations varies with respect to the power
  /// measured during the calibration. Therefore the radar constant
  /// must be updated before it is used to compute reflectivity.
  
  void adjustRadarConst(double pulseWidthUsec,
                        double xmitPowerDbmH,
                        double xmitPowerDbmV);

  /// \name Set methods:
  //@{

  /// Set the time at which the calibration was performed.

  inline void setRadarName(const string &val) { _radarName = val; }
  
  void setCalibTime(int year, int month, int day,
                    int hour, int min, int sec);
  void setCalibTime(time_t time);
  
  inline void setWavelengthCm(double val) { _wavelengthCm = val; }

  inline void setBeamWidthDegH(double val) { _beamWidthDegH = val; }
  inline void setBeamWidthDegV(double val) { _beamWidthDegV = val; }

  inline void setAntennaGainDbH(double val) { _antennaGainDbH = val; }
  inline void setAntennaGainDbV(double val) { _antennaGainDbV = val; }

  inline void setPulseWidthUsec(double val) { _pulseWidthUsec = val; }
  inline void setXmitPowerDbmH(double val) { _xmitPowerDbmH = val; }
  inline void setXmitPowerDbmV(double val) { _xmitPowerDbmV = val; }

  inline void setTwoWayWaveguideLossDbH(double val)
  { _twoWayWaveguideLossDbH = val; }
  inline void setTwoWayWaveguideLossDbV(double val)
  { _twoWayWaveguideLossDbV = val; }
  inline void setTwoWayRadomeLossDbH(double val)
  { _twoWayRadomeLossDbH = val; }
  inline void setTwoWayRadomeLossDbV(double val)
  { _twoWayRadomeLossDbV = val; }

  inline void setReceiverMismatchLossDb(double val)
  { _receiverMismatchLossDb = val; }

  inline void setKSquaredWater(double val) { _kSquaredWater = val; }

  inline void setRadarConstantH(double val) { _radarConstH = val; }
  inline void setRadarConstantV(double val) { _radarConstV = val; }

  inline void setNoiseDbmHc(double val) { _noiseDbmHc = val; }
  inline void setNoiseDbmHx(double val) { _noiseDbmHx = val; }
  inline void setNoiseDbmVc(double val) { _noiseDbmVc = val; }
  inline void setNoiseDbmVx(double val) { _noiseDbmVx = val; }

  inline void setI0DbmHc(double val) { _i0DbmHc = val; }
  inline void setI0DbmHx(double val) { _i0DbmHx = val; }
  inline void setI0DbmVc(double val) { _i0DbmVc = val; }
  inline void setI0DbmVx(double val) { _i0DbmVx = val; }

  inline void setReceiverGainDbHc(double val) { _receiverGainDbHc = val; }
  inline void setReceiverGainDbHx(double val) { _receiverGainDbHx = val; }
  inline void setReceiverGainDbVc(double val) { _receiverGainDbVc = val; }
  inline void setReceiverGainDbVx(double val) { _receiverGainDbVx = val; }

  inline void setReceiverSlopeDbHc(double val) { _receiverSlopeDbHc = val; }
  inline void setReceiverSlopeDbHx(double val) { _receiverSlopeDbHx = val; }
  inline void setReceiverSlopeDbVc(double val) { _receiverSlopeDbVc = val; }
  inline void setReceiverSlopeDbVx(double val) { _receiverSlopeDbVx = val; }

  inline void setDynamicRangeDbHc(double val) { _dynamicRangeDbHc = val; }
  inline void setDynamicRangeDbHx(double val) { _dynamicRangeDbHx = val; }
  inline void setDynamicRangeDbVc(double val) { _dynamicRangeDbVc = val; }
  inline void setDynamicRangeDbVx(double val) { _dynamicRangeDbVx = val; }

  inline void setBaseDbz1kmHc(double val) { _baseDbz1kmHc = val; }
  inline void setBaseDbz1kmHx(double val) { _baseDbz1kmHx = val; }
  inline void setBaseDbz1kmVc(double val) { _baseDbz1kmVc = val; }
  inline void setBaseDbz1kmVx(double val) { _baseDbz1kmVx = val; }

  inline void setSunPowerDbmHc(double val) { _sunPowerDbmHc = val; }
  inline void setSunPowerDbmHx(double val) { _sunPowerDbmHx = val; }
  inline void setSunPowerDbmVc(double val) { _sunPowerDbmVc = val; }
  inline void setSunPowerDbmVx(double val) { _sunPowerDbmVx = val; }

  inline void setNoiseSourcePowerDbmH(double val)
  { _noiseSourcePowerDbmH = val; }
  inline void setNoiseSourcePowerDbmV(double val)
  { _noiseSourcePowerDbmV = val; }

  inline void setPowerMeasLossDbH(double val) { _powerMeasLossDbH = val; }
  inline void setPowerMeasLossDbV(double val) { _powerMeasLossDbV = val; }

  inline void setCouplerForwardLossDbH(double val)
  { _couplerForwardLossDbH = val; }
  inline void setCouplerForwardLossDbV(double val)
  { _couplerForwardLossDbV = val; }

  inline void setDbzCorrection(double val) { _dbzCorrection = val; }
  inline void setZdrCorrectionDb(double val) { _zdrCorrectionDb = val; }
  inline void setLdrCorrectionDbH(double val) { _ldrCorrectionDbH = val; }
  inline void setLdrCorrectionDbV(double val) { _ldrCorrectionDbV = val; }
  inline void setSystemPhidpDeg(double val) { _systemPhidpDeg = val; }

  inline void setTestPowerDbmH(double val) { _testPowerDbmH = val; }
  inline void setTestPowerDbmV(double val) { _testPowerDbmV = val; }

  //@}
  /// \name Get methods:
  //@{

  const string &getRadarName() const { return _radarName; }

  /// Get the time at which the calibration was performed.

  time_t getCalibTime() const;
  int getYear() const { return _year; }
  int getMonth() const { return _month; }
  int getDay() const { return _day; }
  int getHour() const { return _hour; }
  int getMin() const { return _min; }
  int getSec() const { return _sec; }
  
  inline double getWavelengthCm() const { return _wavelengthCm; }

  inline double getBeamWidthDegH() const { return _beamWidthDegH; }
  inline double getBeamWidthDegV() const { return _beamWidthDegV; }

  inline double getAntennaGainDbH() const { return _antennaGainDbH; }
  inline double getAntennaGainDbV() const { return _antennaGainDbV; }

  inline double getPulseWidthUsec() const { return _pulseWidthUsec; }
  inline double getXmitPowerDbmH() const { return _xmitPowerDbmH; }
  inline double getXmitPowerDbmV() const { return _xmitPowerDbmV; }

  inline double getTwoWayWaveguideLossDbH() const
  { return _twoWayWaveguideLossDbH; }
  inline double getTwoWayWaveguideLossDbV() const
  { return _twoWayWaveguideLossDbV; }
  
  inline double getTwoWayRadomeLossDbH() const { return _twoWayRadomeLossDbH; }
  inline double getTwoWayRadomeLossDbV() const { return _twoWayRadomeLossDbV; }
  
  inline double getReceiverMismatchLossDb() const
  { return _receiverMismatchLossDb; }
  
  inline double getKSquaredWater() const { return _kSquaredWater; }

  inline double getRadarConstantH() const { return _radarConstH; }
  inline double getRadarConstantV() const { return _radarConstV; }

  inline double getNoiseDbmHc() const { return _noiseDbmHc; }
  inline double getNoiseDbmHx() const { return _noiseDbmHx; }
  inline double getNoiseDbmVc() const { return _noiseDbmVc; }
  inline double getNoiseDbmVx() const { return _noiseDbmVx; }

  inline double getI0DbmHc() const { return _i0DbmHc; }
  inline double getI0DbmHx() const { return _i0DbmHx; }
  inline double getI0DbmVc() const { return _i0DbmVc; }
  inline double getI0DbmVx() const { return _i0DbmVx; }

  inline double getReceiverGainDbHc() const { return _receiverGainDbHc; }
  inline double getReceiverGainDbHx() const { return _receiverGainDbHx; }
  inline double getReceiverGainDbVc() const { return _receiverGainDbVc; }
  inline double getReceiverGainDbVx() const { return _receiverGainDbVx; }

  inline double getReceiverSlopeDbHc() const { return _receiverSlopeDbHc; }
  inline double getReceiverSlopeDbHx() const { return _receiverSlopeDbHx; }
  inline double getReceiverSlopeDbVc() const { return _receiverSlopeDbVc; }
  inline double getReceiverSlopeDbVx() const { return _receiverSlopeDbVx; }

  inline double getDynamicRangeDbHc() const { return _dynamicRangeDbHc; }
  inline double getDynamicRangeDbHx() const { return _dynamicRangeDbHx; }
  inline double getDynamicRangeDbVc() const { return _dynamicRangeDbVc; }
  inline double getDynamicRangeDbVx() const { return _dynamicRangeDbVx; }

  inline double getBaseDbz1kmHc() const { return _baseDbz1kmHc; }
  inline double getBaseDbz1kmHx() const { return _baseDbz1kmHx; }
  inline double getBaseDbz1kmVc() const { return _baseDbz1kmVc; }
  inline double getBaseDbz1kmVx() const { return _baseDbz1kmVx; }

  inline double getSunPowerDbmHc() const { return _sunPowerDbmHc; }
  inline double getSunPowerDbmHx() const { return _sunPowerDbmHx; }
  inline double getSunPowerDbmVc() const { return _sunPowerDbmVc; }
  inline double getSunPowerDbmVx() const { return _sunPowerDbmVx; }

  inline double getNoiseSourcePowerDbmH() const
  { return _noiseSourcePowerDbmH; }
  inline double getNoiseSourcePowerDbmV() const 
  { return _noiseSourcePowerDbmV; }

  inline double getPowerMeasLossDbH() const { return _powerMeasLossDbH; }
  inline double getPowerMeasLossDbV() const { return _powerMeasLossDbV; }

  inline double getCouplerForwardLossDbH() const 
  { return _couplerForwardLossDbH; }
  inline double getCouplerForwardLossDbV() const 
  { return _couplerForwardLossDbV; }

  inline double getDbzCorrection() const { return _dbzCorrection; }
  inline double getZdrCorrectionDb() const { return _zdrCorrectionDb; }
  inline double getLdrCorrectionDbH() const { return _ldrCorrectionDbH; }
  inline double getLdrCorrectionDbV() const { return _ldrCorrectionDbV; }
  inline double getSystemPhidpDeg() const { return _systemPhidpDeg; }

  inline double getTestPowerDbmH() const { return _testPowerDbmH; }
  inline double getTestPowerDbmV() const { return _testPowerDbmV; }

  //@}

  /// Print the calibration factors.

  void print(ostream &out) const;

  /// convert to XML string

  void convert2Xml(string &xml) const;

  /// set from XML string

  void setFromXml(const string &xmlBuf, bool doInit = false);

  // read from a given XML cal file
  //
  // Returns 0 on success, -1 on failure
  // Sets errStr on failure
  
  int readFromXmlFile(const string &calPath, string &errStr);

  /// \name Serialization:
  //@{

  // serialize into a RadxMsg
  
  void serialize(RadxMsg &msg);
  
  // deserialize from a RadxMsg
  // return 0 on success, -1 on failure

  int deserialize(const RadxMsg &msg);

  //@}

private:

  string _radarName;

  /* time of calibration */

  int _year, _month, _day, _hour, _min, _sec;

  /* wavelength - cm */

  double _wavelengthCm;

  /* Nominal beam width - deg */
  
  double _beamWidthDegH;
  double _beamWidthDegV;

  /* Measured antenna gain */
  
  double _antennaGainDbH;
  double _antennaGainDbV;

  /* pulse width and transmit power */

  double _pulseWidthUsec; /* pulse width in Usec */
  double _xmitPowerDbmH;  /* peak transmit H power in dBm */
  double _xmitPowerDbmV;  /* peak transmit V power in dBm */
  
  /* 2-way waveguide loss from feedhorn to measurement plane.
   * Set to 0 is the loss is incorporated into the antenna gain */
  
  double _twoWayWaveguideLossDbH;
  double _twoWayWaveguideLossDbV;
  
  /* 2-way Radome loss (dB)
   * Set to 0 is the loss is incorporated into the antenna gain */

  double _twoWayRadomeLossDbH;
  double _twoWayRadomeLossDbV;

  /* receiver mistmatch loss (dB) */
    
  double _receiverMismatchLossDb;

  /* dielectric constant in water */

  double _kSquaredWater;
  
  /* Radar constant for each waveguide */
  
  double _radarConstH;
  double _radarConstV;

  /* noise Dbm - noise level for each channel, from calibration
   * SNR is computed relative to these noise values */

  double _noiseDbmHc; /* calibrated noise value, dBm - Hc H co-polar */
  double _noiseDbmHx; /* calibrated noise value, dBm - Hx H cross-polar */
  double _noiseDbmVc; /* calibrated noise value, dBm - Vc V co-polar */
  double _noiseDbmVx; /* calibrated noise value, dBm - Vx V cross-polar */

  /* calibrated noise Dbm - noise level for each channel */

  double _i0DbmHc; /* calibrated noise value, dBm - Hc H co-polar */
  double _i0DbmHx; /* calibrated noise value, dBm - Hx H cross-polar */
  double _i0DbmVc; /* calibrated noise value, dBm - Vc V co-polar */
  double _i0DbmVx; /* calibrated noise value, dBm - Vx V cross-polar */

  /* Receiver gain for each channel - dB 
   * Gain from waveguide power to digitized power */
  
  double _receiverGainDbHc;
  double _receiverGainDbHx;
  double _receiverGainDbVc;
  double _receiverGainDbVx;
  
  /* slope of linear part of receiver response curve 
   * in dB units */

  double _receiverSlopeDbHc;
  double _receiverSlopeDbHx;
  double _receiverSlopeDbVc;
  double _receiverSlopeDbVx;

  /* dynamic range in dB */

  double _dynamicRangeDbHc;
  double _dynamicRangeDbHx;
  double _dynamicRangeDbVc;
  double _dynamicRangeDbVx;

  /* Base reflectivity at 1 km, for SNR of 0.
   * dBZ = baseDbz1km + SNR + 20log10(rangeKm) + (atmosAtten * rangeKm)
   * BaseDbz1km can be computed as follows:
   *   baseDbz1km = noiseDbm - receiverGainDb - radarConst
   * However, sometimes these values are not available, and the
   * baseDbz1km value must be used as is. This is the case for RVP8 time
   * series data */
  
  double _baseDbz1kmHc;
  double _baseDbz1kmHx;
  double _baseDbz1kmVc;
  double _baseDbz1kmVx;

  /* Sun power for each channel - dBm */
  
  double _sunPowerDbmHc;
  double _sunPowerDbmHx;
  double _sunPowerDbmVc;
  double _sunPowerDbmVx;

  /* noise source power */
  
  double _noiseSourcePowerDbmH; /* H power in dBm */
  double _noiseSourcePowerDbmV; /* V power in dBm */

  /* Power measurement loss from ref. point
   * to power meter sensor, each channel.
   * This will generally be positive, to indicate a loss.
   * If there is an amplifier in the calibration circuit, use a negative
   * number to indicate a gain */

  double _powerMeasLossDbH;
  double _powerMeasLossDbV;

  /* Directional coupler forward loss for H and V
   * This will be negative */
  
  double _couplerForwardLossDbH;
  double _couplerForwardLossDbV;

  /* ZDR / LDR / PHIDP */

  double _dbzCorrection;    /* DBZ correction, dB */
  double _zdrCorrectionDb;  /* ZDR correction, dB */
  double _ldrCorrectionDbH; /* LDR correction, dB, H */
  double _ldrCorrectionDbV; /* LDR correction, dB, V */
  double _systemPhidpDeg;   /* system phipd - degrees */

  /* test power */
  
  double _testPowerDbmH;
  double _testPowerDbmV;

  /////////////////////////////////////////////////
  // serialization
  /////////////////////////////////////////////////

  static const int _metaStringsPartId = 1;
  static const int _metaNumbersPartId = 2;
  
  // struct for metadata numbers in messages
  // strings not included - they are passed as XML
  
  typedef struct {
    
    Radx::si64 timeSecs;
    Radx::fl64 wavelengthCm;
    Radx::fl64 beamWidthDegH;
    Radx::fl64 beamWidthDegV;
    Radx::fl64 antennaGainDbH;
    Radx::fl64 antennaGainDbV;
    Radx::fl64 pulseWidthUsec;
    Radx::fl64 xmitPowerDbmH;
    Radx::fl64 xmitPowerDbmV;
    Radx::fl64 twoWayWaveguideLossDbH;
    Radx::fl64 twoWayWaveguideLossDbV;
    Radx::fl64 twoWayRadomeLossDbH;
    Radx::fl64 twoWayRadomeLossDbV;
    Radx::fl64 receiverMismatchLossDb;
    Radx::fl64 kSquaredWater;
    Radx::fl64 radarConstH;
    Radx::fl64 radarConstV;
    Radx::fl64 noiseDbmHc;
    Radx::fl64 noiseDbmHx;
    Radx::fl64 noiseDbmVc;
    Radx::fl64 noiseDbmVx;
    Radx::fl64 i0DbmHc;
    Radx::fl64 i0DbmHx;
    Radx::fl64 i0DbmVc;
    Radx::fl64 i0DbmVx;
    Radx::fl64 receiverGainDbHc;
    Radx::fl64 receiverGainDbHx;
    Radx::fl64 receiverGainDbVc;
    Radx::fl64 receiverGainDbVx;
    Radx::fl64 receiverSlopeDbHc;
    Radx::fl64 receiverSlopeDbHx;
    Radx::fl64 receiverSlopeDbVc;
    Radx::fl64 receiverSlopeDbVx;
    Radx::fl64 dynamicRangeDbHc;
    Radx::fl64 dynamicRangeDbHx;
    Radx::fl64 dynamicRangeDbVc;
    Radx::fl64 dynamicRangeDbVx;
    Radx::fl64 baseDbz1kmHc;
    Radx::fl64 baseDbz1kmHx;
    Radx::fl64 baseDbz1kmVc;
    Radx::fl64 baseDbz1kmVx;
    Radx::fl64 sunPowerDbmHc;
    Radx::fl64 sunPowerDbmHx;
    Radx::fl64 sunPowerDbmVc;
    Radx::fl64 sunPowerDbmVx;
    Radx::fl64 noiseSourcePowerDbmH;
    Radx::fl64 noiseSourcePowerDbmV;
    Radx::fl64 powerMeasLossDbH;
    Radx::fl64 powerMeasLossDbV;
    Radx::fl64 couplerForwardLossDbH;
    Radx::fl64 couplerForwardLossDbV;
    Radx::fl64 dbzCorrection;
    Radx::fl64 zdrCorrectionDb;
    Radx::fl64 ldrCorrectionDbH;
    Radx::fl64 ldrCorrectionDbV;
    Radx::fl64 systemPhidpDeg;
    Radx::fl64 testPowerDbmH;
    Radx::fl64 testPowerDbmV;
    
    Radx::fl64 spareFl64[13];
    
  } msgMetaNumbers_t;

  msgMetaNumbers_t _metaNumbers;
  
  /// convert metadata to XML
  
  void _loadMetaStringsToXml(string &xml, int level = 0) const;
  
  /// set metadata from XML
  /// returns 0 on success, -1 on failure
  
  int _setMetaStringsFromXml(const char *xml, 
                             size_t bufLen);
  
  /// load meta numbers to message struct
  
  void _loadMetaNumbersToMsg();
  
  /// set the meta number data from the message struct
  /// returns 0 on success, -1 on failure
  
  int _setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                             size_t bufLen,
                             bool swap);
  
  /// swap meta numbers
  
  static void _swapMetaNumbers(msgMetaNumbers_t &msgMetaNumbers);
          
};

#endif
