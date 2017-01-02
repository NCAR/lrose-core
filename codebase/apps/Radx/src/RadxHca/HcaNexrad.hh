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
#include "HcaInterestMap.hh"

using namespace std;

class HcaNexrad {
  
public:
  
  ///////////////////////////////////////////
  // constructor
  
  HcaNexrad();

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
  
  // set the radar wavelength in cm

  void setWavelengthCm(double val) { _wavelengthCm = val; }

  // set the radar ht in km

  void setRadarHtKm(double val) { _radarHtKm = val; }

  // set the SNR threshold

  void setSnrThreshold(double val) { _snrThresholdDb = val; }

  /**
   * Set the temperature profile.
   * This is used to override the temperature profile in the
   * thresholds file, for example from a sounding.
   * @param
   */
  void setTempProfile(const TempProfile &tempProfile);
 
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
  void setSdDbzFilterLen(int nGates) { _sdDbzFilterLen = nGates; }
  void setSdPhidpFilterLen(int nGates) { _sdPhidpFilterLen = nGates; }
  
  /**
   * Initialize the object arrays for later use.
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
   * @param[in] nGates Number of gates
   * @param[in] dbz Reflectivity array
   * @param[in] zdr Differential reflectivity array
   * @param[in] rhohv Correlation coeff array
   * @param[in] phidpUnfolded - unfolded phase difference array
   * @param[in] kdp - pre-computed
   * @param[in] tempC Temperature at each gate, in deg C
   */ 

  void computeHca(int nGates,
                  const double *snr,
                  const double *dbz,
                  const double *zdr,
                  const double *rhohv,
                  const double *phidpUnfolded,
                  const double *kdp,
                  const double *tempC);
  
  /**
   * Get snr field after calling computeHca()
   */
  const double *getSnr() const { return _snr; }

  /**
   * Get smoothed dbz field after calling computeHca()
   */
  const double *getDbz() const { return _dbz; }

  /**
   * Get smoothed zdr field after calling computeHca()
   */
  const double *getZdr() const { return _zdr; }

  /**
   * Get smoothed rhohv field after calling computeHca()
   */
  const double *getRhohv() const { return _rhohv; }

  /**
   * Get log10(kdp) field after calling computeHca()
   */
  const double *getLogKdp() const { return _logKdp; }

  /**
   * Get phidp field after calling computeHca()
   */
  const double *getPhidp() const { return _phidp; }

  /**
   * Get temperature field after calling computeHca()
   */
  const double *getTempC() const { return _tempC; }
  
  /**
   * Get texture of dbz after calling computeHca()
   */
  const double *getSdDbz() const { return _sdDbz; }

  /**
   * Get texture of phidp after calling computeHca()
   */
  const double *getSdPhidp() const { return _sdPhidp; }

  /**
   * Get primary particle id field after calling computeHca()
   */
  const int *getHca() const { return _hca; }
  
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

  /**
   * Fill a temperature array, for a radar beam elevation
   * @param[in] radarHtKm The radar height in Km MSL
   * @param[in] elevDeg The elevation angle of the radar beam
   * @param[in] nGates The number of gates in the radar beam
   * @param[in] startRangeKm The starting gate for the temperature array
   * @param[in] gateSpacingKm The spacing between gates (km)
   * @param[out] temp The filled temperature array
   */
  void fillTempArray(double radarHtKm,
                     bool setPseudoRadiusRatio,
                     double pseudoRadiusRatio,
                     double elevDeg, int nGates,
                     double startRangeKm, double gateSpacingKm,
                     double *tempC);

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

  static double _missingDouble; /**< The value to use for missing data */

  bool _debug;   /**< Flag to indicate whether debug messages should be printed */
  bool _verbose; /**< Flag to indicate whether verbose messages should be printed */
  static pthread_mutex_t _debugPrintMutex; // debug printing
  
  double _wavelengthCm; /**< The wavelength (cm) of the radar beam */

  // geometry

  int _nGates;
  double _startRangeKm;
  double _gateSpacingKm;
  double _radarHtKm;

  // temperature profile

  vector<TempProfile::PointVal> _tmpProfile; /**< Temperature profile */

  TaArray<double> _tmpHtArray_; /**< Array of temperature profile heights */
  double *_tmpHtArray;          /**< Pointer to the array of temperature profile heights */

  int _tmpMinHtMeters;          /**< Mimimum height of the temperature profile (m) */
  int _tmpMaxHtMeters;          /**< Maximum height of the temperature profile (m) */

  double _tmpBottomC;           /**< Temperature at the base of the profile */
  double _tmpTopC;              /**< Temperature at the top of the profile */

  // snr threshold to use

  double _snrThresholdDb;

  // filter lengths in gates

  int _dbzFilterLen;
  int _zdrFilterLen;
  int _rhohvFilterLen;
  int _sdDbzFilterLen;
  int _sdPhidpFilterLen;
  
  // data arrays
  
  RadxArray<double> _snr_;
  RadxArray<double> _dbz_;
  RadxArray<double> _zdr_;
  RadxArray<double> _rhohv_;
  RadxArray<double> _phidp_;
  RadxArray<double> _logKdp_;
  RadxArray<double> _tempC_;

  double *_snr;
  double *_dbz;
  double *_zdr;
  double *_rhohv;
  double *_phidp;
  double *_logKdp;
  double *_tempC;
  
  RadxArray<double> _sdDbz_;
  RadxArray<double> _sdPhidp_;
  RadxArray<double> _sdDbz2_;
  RadxArray<double> _sdPhidp2_;

  double *_sdDbz;
  double *_sdPhidp;
  double *_sdDbz2;
  double *_sdPhidp2;
  
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

  // atmospheric attenuation

  AtmosAtten _atmos;

  // HCA interest maps

  HcaInterestMap* _imaps[HcaInterestMap::nClasses][HcaInterestMap::nFeatures];

  // private methods
  
  void _allocArrays(int nGates);
  void _computeTempHtLookup();

};

#endif
