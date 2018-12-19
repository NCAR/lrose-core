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
// TempProfile.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2011
//
///////////////////////////////////////////////////////////////
//
// Get temperature profile from sounding
//
////////////////////////////////////////////////////////////////

#ifndef TempProfile_H
#define TempProfile_H

#include <string>
#include <vector>

using namespace std;

class TempProfile {
  
public:

  ////////////////////////////
  // Interior class: PointVal
  
  /**
   * @class TmpPoint
   *   An object to hold temperature, pressure, and height values
   *   at a point
   */
  class PointVal {
  
  public:
    
    // constructor

    // default

    PointVal();
    
    /**
     * Constructor
     * @param[in] press The pressure at this point (Hpa)
     * @param[in] ht The height of this point (km)
     * @param[in] temp The temperature at this point (C)
     */
    PointVal(double press, double ht, double tmp);

    /**
     * Constructor
     * @param[in] ht The height of this point (km)
     * @param[in] temp The temperature at this point (C)
     */
    PointVal(double ht, double tmp);
    
    /**
     * Destructor
     */
    ~PointVal();

    // set methods

    void setPressHpa(double val) { pressHpa = val; }
    void setHtKm(double val) { htKm = val; }
    void setTmpC(double val) { tmpC = val; }
    void setRhPercent(double val) { rhPercent = val; }

    // sget methods

    double getPressHpa() const { return pressHpa; }
    double getHtKm() const { return htKm; }
    double getTmpC() const { return tmpC; }
    double getRhPercent() const { return rhPercent; }

    /**
     * Print the contents of this TmpPoint
     * @param[out] out The stream to print to
     */
    void print(ostream &out) const;
    
    // data

    double pressHpa;   /**< The pressure at this point (Hpa),
                        ** or -9999 if unavailable */
    double htKm;       /**< The height of this point (km) */
    double tmpC;       /**< The temperature at this point (C) */
    double rhPercent;  /**< relative humidity (%) */
    
  };

  // constructor
  
  TempProfile();
  
  // destructor
  
  ~TempProfile();

  // clear the profile

  void clear() { _profile.clear(); }

  // add a profile point

  void addPoint(PointVal &val) { _profile.push_back(val); }

  // if you use addPoint, you need to call prepareForUse()
  // after all points have been added

  void prepareForUse();

  // set methods for private members

  void setSoundingLocationName(const string &val) {
    _soundingLocationName = val; 
  }
  void setSoundingSearchTimeMarginSecs(int val) {
    _soundingSearchTimeMarginSecs = val; 
  }

  void setCheckPressureRange(bool val) {
    _checkPressureRange = val;
  }
  void setSoundingRequiredMinPressureHpa(double val) {
    _soundingRequiredMinPressureHpa = val; 
  }
  void setSoundingRequiredMaxPressureHpa(double val) {
    _soundingRequiredMaxPressureHpa = val; 
  }

  void setCheckHeightRange(bool val) {
    _checkHeightRange = val;
  }
  void setSoundingRequiredMinHeightM(double val) {
    _soundingRequiredMinHeightM = val; 
  }
  void setSoundingRequiredMaxHeightM(double val) {
    _soundingRequiredMaxHeightM = val; 
  }

  void setCheckTempRange(bool val) {
    _checkTempRange = val;
  }
  void setSoundingRequiredMinTempC(double val) {
    _soundingRequiredMinTempC = val; 
  }
  void setSoundingRequiredMaxTempC(double val) {
    _soundingRequiredMaxTempC = val; 
  }

  void setCheckPressureMonotonicallyDecreasing(bool val) {
    _checkPressureMonotonicallyDecreasing = val;
  }

  // set the full profile

  void setProfile(const vector<PointVal> &profile);
  
  // set the height correction in km
  // this corection is added to the height read in

  void setHeightCorrectionKm(double val) { _heightCorrectionKm = val; }

  // set to use wet-bulb temp instead of dry bulb

  void setUseWetBulbTemp(bool val) {
    _useWetBulbTemp = val;
  }

  // load a valid temperature profile from SPDB
  // returns 0 on success, -1 on failure
  // on failure, tmpProfile will be empty

  int loadFromSpdb(const string &url,
                   time_t dataTime,
                   time_t &soundingTime);

  // Load from a PID thresholds file
  // returns 0 on success, -1 on failure
  
  int loadFromPidThresholdsFile(const string &pidThresholdsPath);
  
  // optionally get access to the profile
  // If getTempProfile() returned failure, tmpProfile will be empty

  const vector<PointVal> &getProfile() const { return _profile; }
  
  // get temperature at a given height
  // returns -9999 if no temp profile available
  // on first call will create lut
  
  double getTempForHtKm(double htKm) const;

  // get the freezing level height

  double getFreezingLevel() const;

  // get height for specified temp
  
  double getHtKmForTempC(double tempC) const;

  // get methods for private members

  const string &getSoundingSpdbUrl() const {
    return _soundingSpdbUrl; 
  }
  int getSoundingSearchTimeMarginSecs() const {
    return _soundingSearchTimeMarginSecs; 
  }

  bool getCheckPressureRange() const {
    return _checkPressureRange;
  }
  double getSoundingRequiredMinPressureHpa() const {
    return _soundingRequiredMinPressureHpa; 
  }
  double getSoundingRequiredMaxPressureHpa() const {
    return _soundingRequiredMaxPressureHpa; 
  }

  bool getCheckHeightRange() const {
    return _checkHeightRange;
  }
  double getSoundingRequiredMinHeightM() const {
    return _soundingRequiredMinHeightM; 
  }
  double getSoundingRequiredMaxHeightM() const {
    return _soundingRequiredMaxHeightM; 
  }

  bool getCheckTempRange() const {
    return _checkTempRange;
  }
  double getSoundingRequiredMinTempC() const {
    return _soundingRequiredMinTempC; 
  }
  double getSoundingRequiredMaxTempC() const {
    return _soundingRequiredMaxTempC; 
  }

  bool getCheckPressureMonotonicallyDecreasing() const {
    return _checkPressureMonotonicallyDecreasing;
  }

  bool getUseWetBulbTmp() const {
    return _useWetBulbTemp;
  }

  /// print

  void print(ostream &out) const;

  // debugging

  void setDebug() { _debug = true; }
  void setVerbose() { _verbose = true; }

  // missing value

  static const double missingValue;

protected:
private:

  bool _debug;
  bool _verbose;

  time_t _soundingTime;
  vector<PointVal> _profile;
  double _freezingLevel;
  
  string _soundingSpdbUrl;
  string _soundingLocationName;
  int _soundingSearchTimeMarginSecs;
  
  bool _checkPressureRange;
  double _soundingRequiredMinPressureHpa;
  double _soundingRequiredMaxPressureHpa;

  bool _checkHeightRange;
  double _soundingRequiredMinHeightM;
  double _soundingRequiredMaxHeightM;

  bool _checkTempRange;
  double _soundingRequiredMinTempC;
  double _soundingRequiredMaxTempC;

  bool _checkPressureMonotonicallyDecreasing;
  bool _useWetBulbTemp;

  double _heightCorrectionKm; /* correction made to sounding heights
                               * as they are read in */

  mutable vector<double> _lutByMeterHt; /* array of temperature with height
                                         * one entry per meter */
  
  mutable int _tmpMinHtMeters;  /**< Mimimum height of the temperature profile (m) */
  mutable int _tmpMaxHtMeters;  /**< Maximum height of the temperature profile (m) */

  mutable double _tmpBottomC;    /**< Temperature at the base of the profile */
  mutable double _tmpTopC;       /**< Temperature at the top of the profile */

  // methods

  int _setTempProfileFromPidLine(const char *line);
  int _getTempProfile(time_t searchTime);
  int _checkTempProfile();
  void _computeFreezingLevel();
  void _createLutByMeterHt() const;

};


#endif
