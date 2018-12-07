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
// HcaNexrad.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////
//
// HcaNexrad - hydrometeor classification algorithm
//
// References:
// Park HyangSuk, A. V. Ryzhhov, D. S. Zrnic, Kyung0Eak Kim.
// June 2009.
// The Hydrometeor Classification Algorithm for the Polarimetric
// WSR-88D: Description and Application to an MCS.
// AMS Weather and Forecasting, Vol 24, 730-748, June 2009.
//
//
///////////////////////////////////////////////////////////////

#ifndef HcaNexrad_HH
#define HcaNexrad_HH

#include <radar/KdpFilt.hh>
#include <radar/TempProfile.hh>
#include <radar/AtmosAtten.hh>
#include <Radx/RadxArray.hh>
#include <radar/FilterUtils.hh>
#include <radar/PhidpFilt.hh>
#include "HcaInterestMap.hh"

using namespace std;

class HcaNexrad {
  
public:
  
  ///////////////////////////////////////////
  // constructor
  
  HcaNexrad(const TempProfile &tempProfile);

  // destructor
  
  ~HcaNexrad();

  /**
   * Free up memory
   */
  void clear();

  // set debugging state

  void setDebug(bool state) { _debug = state; }

  void setVerbose(bool state) {
    _verbose = state;
    if (state) {
      _debug = true;
    }
  }
  
  // set the radar and ray properties

  void setWavelengthM(double val) { _wavelengthM = val; }
  void setVertBeamWidthDeg(double val) { _vertBeamWidthDeg = val; }
  void setRadarHtKm(double val) { _radarHtKm = val; }
  void setElevation(double val) { _elevation = val; }
  void setAzimuth(double val) { _azimuth = val; }
  void setStartRangeKm(double val) { _startRangeKm = val; }
  void setGateSpacingKm(double val) { _gateSpacingKm = val; }

  // details for computing beam height

  void setPseudoRadiusRatio(double pseudoRadiusRatio) {
    _setPseudoRadiusRatio = true;
    _pseudoRadiusRatio = pseudoRadiusRatio;
  }

  // set the SNR threshold

  void setSnrThresholdDb(double val) { _snrThresholdDb = val; }

  /**
   * Set the temperature profile.
   * This is used to override the temperature profile in the
   * thresholds file, for example from a sounding.
   * @param
   */

  void setTempAtTopOfMeltingLayerC(double val) { _tempAtTopOfMeltingLayerC = val; }
  void setTempAtBottomOfMeltingLayerC(double val) { _tempAtBottomOfMeltingLayerC = val; }
 
  /** 
   * Get temperature at a given height
   * @param[in] htKm The height (in km)
   * @return The temperature (C)
   */
  double getTmpC(double htKm);

  /**
   * Set smoothing filtering length (in gates) for various fields
   */

  void setDbzFilterLen(int nGates) { _dbzFilterLen = nGates; }
  void setZdrFilterLen(int nGates) { _zdrFilterLen = nGates; }
  void setRhohvFilterLen(int nGates) { _rhohvFilterLen = nGates; }
  void setPhidpFilterLen(int nGates) { _phidpFilterLen = nGates; }
  void setPhidpHvyFilterLen(int nGates) { _phidpHvyFilterLen = nGates; }

  /**
   * Set individual class thresholds
   */

  void setHcaMaxAbsVelForGC(double val) { _hcaMaxAbsVelForGC = val; }
  void setHcaMaxRhohvForBS(double val) { _hcaMaxRhohvForBS = val; }
  void setHcaMaxZdrForDS(double val) { _hcaMaxZdrForDS = val; }
  void setHcaMinZdrForBD(double val) { _hcaMinZdrForBD = val; }
  void setHcaMinZdrForWS(double val) { _hcaMinZdrForWS = val; }
  void setHcaMinDbzForWS(double val) { _hcaMinDbzForWS = val; }
  void setHcaMaxDbzForCR(double val) { _hcaMaxDbzForCR = val; }
  void setHcaMinDbzForGR(double val) { _hcaMinDbzForGR = val; }
  void setHcaMaxDbzForGR(double val) { _hcaMaxDbzForGR = val; }
  void setHcaMaxDbzForRA(double val) { _hcaMaxDbzForRA = val; }
  void setHcaMinDbzForHR(double val) { _hcaMinDbzForHR = val; }
  void setHcaMinDbzForRH(double val) { _hcaMinDbzForRH = val; }

  /**
   * Initialize the object arrays for later use.
   * Sets the number of gates.
   * Do this if you need access to the arrays, but have not yet called
   * computePidBeam(), and do not plan to do so.
   * For example, you may want to output missing fields that you have
   * not computed, but the memory needs to be there.
   */ 

  void initializeArrays(int nGates);

  /**
   * Compute HCA for ray.
   * Input fields at a gate should be set to _missingDouble
   * if they are not valid for that gate.
   * Results are stored in local arrays on this class.
   * Use get() methods to retieve them.
   * @param[in] snr Signal-to-noise ratio
   * @param[in] dbz Reflectivity array
   * @param[in] zdr Differential reflectivity array
   * @param[in] rhohv Correlation coeff array
   * @param[in] phidpUnfolded - unfolded phase difference array
   * @param[in] kdp - pre-computed
   */ 

  void computeHca(const double *snr,
                  const double *dbz,
                  const double *vel,
                  const double *zdr,
                  const double *rhohv,
                  const double *phidpUnfolded,
                  const double *kdp);
  
  /**
   * Get fields used as input
   */
  const double *getSnr() const { return _snr; }
  const double *getDbz() const { return _dbz; }
  const double *getZdr() const { return _zdr; }
  const double *getRhohv() const { return _rhohv; }
  const double *getPhidp() const { return _phidp; }
  const double *getLogKdp() const { return _logKdp; }
  const double *getTempLow() const { return _tempLow; }
  const double *getTempMid() const { return _tempMid; }
  const double *getTempHigh() const { return _tempHigh; }
  
  /**
   * Get smoothed fields
   */
  const double *getSmoothDbz() const { return _smoothDbz; }
  const double *getSmoothZdr() const { return _smoothZdr; }
  const double *getSmoothRhohv() const { return _smoothRhohv; }
  const double *getSmoothPhidp() const { return _smoothPhidp; }
  const double *getHvySmoothPhidp() const { return _hvySmoothPhidp; }

  /**
   * Get textures
   */
  const double *getTextureDbz() const { return _textureDbz; }
  const double *getTextureZdr() const { return _textureZdr; }
  const double *getTextureRhohv() const { return _textureRhohv; } // sdev
  const double *getTexturePhidp() const { return _texturePhidp; }

  const double *getSdDbz() const { return _sdDbz; } // sdev
  const double *getSdPhidp() const { return _sdPhidp; } // sdev

  /**
   * Get primary particle id field after calling computeHca()
   */
  const int *getHca() const { return _hca; }
  const int *getTempCat() const { return _tempCat; }
  
  /**
   * Get interest for classes after calling computeHca()
   */
  const double *getGcInterest() const { return _gcInterest; }
  const double *getBsInterest() const { return _bsInterest; }
  const double *getDsInterest() const { return _dsInterest; }
  const double *getWsInterest() const { return _wsInterest; }
  const double *getCrInterest() const { return _crInterest; }
  const double *getGrInterest() const { return _grInterest; }
  const double *getBdInterest() const { return _bdInterest; }
  const double *getRaInterest() const { return _raInterest; }
  const double *getHrInterest() const { return _hrInterest; }
  const double *getRhInterest() const { return _rhInterest; }
  
  /**
   * Print status 
   * @param[out] out The stream to print to
   */
  void print(ostream &out);

  /**
   * Set the missing data value to use
   * @param[in] missing The missing data value to use
   */ 
  void setMissingDouble(double missing) { _missingDouble = missing; }

  const static double pseudoEarthDiamKm; /**< pseudo earth diameter
                                          * for computing radar beam heights */

  // Create and add an interest map,

  int addInterestMap(HcaInterestMap::imap_class_t hcaClass,
                     HcaInterestMap::imap_feature_t feature,
                     double x1, double x2,
                     double x3, double x4,
                     double weight);

  // check that all interest maps are non-NULL

  int checkInterestMaps();
  
  // print all interest maps

  void printInterestMaps(ostream &out);

  // delete all interest maps, set to NULL

  void deleteInterestMaps();
  
protected:
private:

  double _missingDouble; /**< The value to use for missing data */

  bool _debug;   /**< Flag to indicate whether debug messages should be printed */
  bool _verbose; /**< Flag to indicate whether verbose messages should be printed */
  static pthread_mutex_t _debugPrintMutex; // debug printing
  
  // temperature profile

  const TempProfile &_tempProfile;

  // radar parameters

  double _wavelengthM;
  double _vertBeamWidthDeg;

  int _nGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _radarHtKm;
  double _elevation;
  double _azimuth;
  
  bool _setPseudoRadiusRatio;
  double _pseudoRadiusRatio;

  // filtering

  FilterUtils _filt;
  PhidpFilt _phidpFilt;

  // temperature profile

  vector<TempProfile::PointVal> _tmpProfile; /**< Temperature profile */

  TaArray<double> _tmpHtArray_; /**< Array of temperature profile heights */
  double *_tmpHtArray;          /**< Pointer to the array of temperature profile heights */

  int _tmpMinHtMeters;          /**< Mimimum height of the temperature profile (m) */
  int _tmpMaxHtMeters;          /**< Maximum height of the temperature profile (m) */

  double _tmpBottomC;           /**< Temperature at the base of the profile */
  double _tmpTopC;              /**< Temperature at the top of the profile */

  double _tempAtTopOfMeltingLayerC;
  double _tempAtBottomOfMeltingLayerC;

  // snr threshold to use

  double _snrThresholdDb;

  // class thresholds

  double _hcaMaxAbsVelForGC;
  double _hcaMaxRhohvForBS;
  double _hcaMaxZdrForDS;
  double _hcaMinZdrForBD;
  double _hcaMinZdrForWS;
  double _hcaMinDbzForWS;
  double _hcaMaxDbzForCR;
  double _hcaMinDbzForGR;
  double _hcaMaxDbzForGR;
  double _hcaMaxDbzForRA;
  double _hcaMinDbzForHR;
  double _hcaMinDbzForRH;

  // filter lengths in gates

  int _dbzFilterLen;
  int _zdrFilterLen;
  int _rhohvFilterLen;
  int _phidpFilterLen;
  int _phidpHvyFilterLen;
  
  // data arrays
  
  RadxArray<double> _snr_;
  RadxArray<double> _dbz_;
  RadxArray<double> _vel_;
  RadxArray<double> _zdr_;
  RadxArray<double> _rhohv_;
  RadxArray<double> _phidp_;
  RadxArray<double> _logKdp_;
  RadxArray<double> _tempLow_;
  RadxArray<double> _tempMid_;
  RadxArray<double> _tempHigh_;

  double *_snr;
  double *_dbz;
  double *_vel;
  double *_zdr;
  double *_rhohv;
  double *_phidp;
  double *_logKdp;
  double *_tempLow;
  double *_tempMid;
  double *_tempHigh;
  
  RadxArray<double> _smoothDbz_;
  RadxArray<double> _smoothZdr_;
  RadxArray<double> _smoothRhohv_;
  RadxArray<double> _smoothPhidp_;
  RadxArray<double> _hvySmoothPhidp_;

  double *_smoothDbz;
  double *_smoothZdr;
  double *_smoothRhohv;
  double *_smoothPhidp;
  double *_hvySmoothPhidp;
  
  RadxArray<double> _textureDbz_;
  RadxArray<double> _textureZdr_;
  RadxArray<double> _textureRhohv_;
  RadxArray<double> _texturePhidp_;
  RadxArray<double> _sdDbz_;
  RadxArray<double> _sdPhidp_;

  double *_textureDbz;
  double *_textureZdr;
  double *_textureRhohv;
  double *_texturePhidp;
  double *_sdDbz;
  double *_sdPhidp;
  
  RadxArray<double> _gcInterest_;
  RadxArray<double> _bsInterest_;
  RadxArray<double> _dsInterest_;
  RadxArray<double> _wsInterest_;
  RadxArray<double> _crInterest_;
  RadxArray<double> _grInterest_;
  RadxArray<double> _bdInterest_;
  RadxArray<double> _raInterest_;
  RadxArray<double> _hrInterest_;
  RadxArray<double> _rhInterest_;

  double *_gcInterest;
  double *_bsInterest;
  double *_dsInterest;
  double *_wsInterest;
  double *_crInterest;
  double *_grInterest;
  double *_bdInterest;
  double *_raInterest;
  double *_hrInterest;
  double *_rhInterest;

  RadxArray<int> _hca_;
  int *_hca;

  RadxArray<int> _tempCat_;
  int *_tempCat;

  // atmospheric attenuation

  AtmosAtten _atmos;

  // HCA interest maps

  HcaInterestMap* _imaps[HcaInterestMap::nClasses][HcaInterestMap::nFeatures];

  // private methods
  
  void _allocArrays();
  void _fillTempArrays();
  void _suppressClass(vector<HcaInterestMap::imap_class_t> &validClasses,
                      HcaInterestMap::imap_class_t suppressedClass);

};

#endif
