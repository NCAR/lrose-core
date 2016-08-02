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
// ZVis.hh
//
// C++ class for dealing with radar-visibility calibration.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2002
//////////////////////////////////////////////////////////////

#ifndef _ZVis_hh
#define _ZVis_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
using namespace std;

#define ZV_CALIB_HDR_NBYTES_32 200
#define ZV_GAUGE_NAME_LEN 32
#define ZV_MISSING_VAL -9999.0

typedef enum {
  ZV_STATUS_CALIBRATED = 0, // valid calibration
  ZV_STATUS_COASTING =   1, // coasting with prev calibration
  ZV_STATUS_DEFAULT =    2, // using defaults
  ZV_STATUS_PERSIST =    3  // persistence forecast
} zv_calib_status_t;

typedef struct {
  si32 version_num; // version number for this struct
  ti32 valid_time; // time for which the cal is considered valid
  ti32 calib_time; // time of last calibration - used to
                   // determine if coasting is valid, or
                   // whether we should use default values
  si32 calib_secs; // length of calibration period - secs
  si32 status; // see zv_calib_status_t
  si32 n_ze_pairs;
  si32 e_trend_secs; // trend duration in secs. Level after this.
  si32 n_forecasts;  // number of forecasts - includes lead time 0
  ti32 forecast_start_time; // Unix time
  si32 forecast_delta_secs; // time between forecasts
  si32 spare_si32[6];
  fl32 trec_u;     // u-wind component - trec
  fl32 trec_v;     // v-wind component - trec
  fl32 surf_u;     // u-wind component - surface
  fl32 surf_v;     // v-wind component - surface
  fl32 rmse;       // RMSE of fit if status is CALIBRATED
  fl32 fall_secs;  // radar-surface fall time
  fl32 trec_wt;    // TREC wind weight
  fl32 surf_wt;    // surface wind weight
  fl32 coeff;      // ZV coefficent 'a' in E = a * Z ** b
  fl32 expon;      // ZV exponent 'b' in E = a * Z ** b
  fl32 smoothed_rmse;      // RMSE smoothed in matrix space
  fl32 smoothed_fall_secs; // fall time smoothed in time
  fl32 smoothed_trec_wt;   // TREC wind weight smoothed in time
  fl32 smoothed_surf_wt;   // surface wind weight smooted in time
  fl32 smoothed_coeff;     // coefficent smoothed in time
  fl32 smoothed_expon;     // exponent smoothed in time
  fl32 dbz_measured;   // measured DBZ for valid_time
  fl32 e_measured;     // measured e for valid_time
  fl32 e_trend_slope;  // forecast e trend, in 1/sec
  fl32 vis_sensor_lat_deg;
  fl32 vis_sensor_lat_frac_deg;
  fl32 vis_sensor_lon_deg;
  fl32 vis_sensor_lon_frac_deg;
  fl32 spare_fl32[11];
  char vis_sensor_name[ZV_GAUGE_NAME_LEN];
} zv_calib_hdr_t;

class ZVis {

public:

  // constructor

  ZVis();

  // destructor

  ~ZVis();

  //////////////////////// set methods /////////////////////////

  // clear all data members

  void clear();

  // set sensor details
  
  void setVisSensorName(const string &name) { _visSensorName = name; }
  void setVisSensorLat(double lat) { _visSensorLat = lat; }
  void setVisSensorLon(double lon) { _visSensorLon = lon; }

  // set times and status

  void setValidTime(time_t time) { _validTime = time; }
  void setCalibTime(time_t time) { _calibTime = time; }
  void setCalibSecs(int secs) { _calibSecs = secs; }
  void setCalibStatus(zv_calib_status_t status) { _calibStatus = status; }

  // set winds

  void setTrecU(double u) { _trecU = u; }
  void setTrecV(double v) { _trecV = v; }
  void setSurfaceU(double u) { _surfaceU = u; }
  void setSurfaceV(double v) { _surfaceV = v; }

  // set calib results

  void setRmse(double rmse) { _rmse = rmse; }
  void setFallSecs(double secs) { _fallSecs = secs; }
  void setTrecWeight(double wt) { _trecWeight = wt; }
  void setSurfaceWeight(double wt) { _surfaceWeight = wt; }
  void setCalibCoeff(double coeff) { _calibCoeff = coeff; }
  void setCalibExpon(double expon) { _calibExpon = expon; }

  void setSmoothedRmse(double rmse) { _smoothedRmse = rmse; }
  void setSmoothedFallSecs(double secs) { _smoothedFallSecs = secs; }
  void setSmoothedTrecWeight(double wt) { _smoothedTrecWeight = wt; }
  void setSmoothedSurfaceWeight(double wt) { _smoothedSurfaceWeight = wt; }
  void setSmoothedCalibCoeff(double coeff) { _smoothedCalibCoeff = coeff; }
  void setSmoothedCalibExpon(double expon) { _smoothedCalibExpon = expon; }

  void addCalibEz(double e, double dbz) {
    _calibE.push_back(e);
    _calibDbz.push_back(dbz);
  }

  // set measured values

  void setDbzMeasured(double dbz) { _dbzMeasured = dbz; }
  void setEMeasured(double e) { _eMeasured = e; }

  // set trend forecast for e

  void setETrendSecs(int secs) { _eTrendSecs = secs; }
  void setETrendSlope(double slope) { _eTrendSlope = slope; }

  // set E forecast

  void setForecastDeltaSecs(int secs) { _forecastDeltaSecs = secs; }
  void setForecastStartTime(time_t time) { _forecastStartTime = time; }
  void addForecastEz(double e, double dbz) {
    _forecastE.push_back(e);
    _forecastDbz.push_back(dbz);
  }

  //////////////////////// get methods /////////////////////////

  // version number for object and struct

  int getVersionNum() const { return _versionNum; }
  
  // get sensor details

  const string &getVisSensorName() const { return _visSensorName; }
  double getVisSensorLat() const { return _visSensorLat; }
  double getVisSensorLon() const { return _visSensorLon; }

  // get calib time and status

  time_t getValidTime() const { return _validTime; }
  time_t getCalibTime() const { return _calibTime; }
  int getCalibSecs() const { return _calibSecs; }
  zv_calib_status_t getCalibStatus() const { return _calibStatus; }

  // get winds

  double getTrecU() const { return _trecU; }
  double getTrecV() const { return _trecV; }
  double getSurfaceU() const { return _surfaceU; }
  double getSurfaceV() const { return _surfaceV; }

  // get calib results

  double getRmse() const { return _rmse; }
  double getFallSecs() const { return _fallSecs; }
  double getTrecWeight() const { return _trecWeight; }
  double getSurfaceWeight() const { return _surfaceWeight; }
  double getCalibCoeff() const { return _calibCoeff; }
  double getCalibExpon() const { return _calibExpon; }

  double getSmoothedRmse() const { return _smoothedRmse; }
  double getSmoothedFallSecs() const { return _smoothedFallSecs; }
  double getSmoothedTrecWeight() const { return _smoothedTrecWeight; }
  double getSmoothedSurfaceWeight() const { return _smoothedSurfaceWeight; }
  double getSmoothedCalibCoeff() const { return _smoothedCalibCoeff; }
  double getSmoothedCalibExpon() const { return _smoothedCalibExpon; }

  const vector<double> &getCalibE() const { return _calibE; }
  const vector<double> &getCalibDbz() const { return _calibDbz; }

  // get measured values
  
  double getDbzMeasured() { return _dbzMeasured; }
  double getEMeasured() { return _eMeasured; }

  // get trend forecast

  int getETrendSecs() const { return _eTrendSecs; }
  double getETrendSlope() const { return _eTrendSlope; }

  // get E forecast

  int getForecastDeltaSecs() const { return _forecastDeltaSecs; }
  time_t getForecastStartTime() const { return _forecastStartTime; }
  const vector<double> &getForecastE() const { return _forecastE; }
  const vector<double> &getForecastDbz() const { return _forecastDbz; }

  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  
  void assemble();

  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  /////////////////////////
  // print
  
  void print(ostream &out, string spacer = "") const;
  
  // byte swapping

  static void calib_hdr_to_BE(zv_calib_hdr_t &hdr);
  static void calib_hdr_from_BE(zv_calib_hdr_t &hdr);
  
protected:

  int _versionNum;
  
  // vis gauge details

  string _visSensorName;
  double _visSensorLat;
  double _visSensorLon;

  // calib time and status

  time_t _validTime;
  time_t _calibTime;
  int _calibSecs;
  zv_calib_status_t _calibStatus;

  // winds

  double _trecU, _trecV;
  double _surfaceU, _surfaceV;

  // calib results

  double _rmse;
  double _fallSecs; // secs to fall from radar to surface
  double _trecWeight;
  double _surfaceWeight;
  double _calibCoeff;
  double _calibExpon;

  // rmse is smoothed in matrix space

  double _smoothedRmse;

  // calib results smoothed in time

  double _smoothedFallSecs;
  double _smoothedTrecWeight;
  double _smoothedSurfaceWeight;
  double _smoothedCalibCoeff;
  double _smoothedCalibExpon;

  // ZE data from cal

  vector<double> _calibE;
  vector<double> _calibDbz;

  // measured dbz and E

  double _dbzMeasured;
  double _eMeasured;
  
  // trend forecast
  // forecast trend slope, out to _trendSecs, then level.
  // Units are 1/sec. Trend is normalized with respect to the
  // E value at a lead time of 0.
  
  int _eTrendSecs; // time period for computing trend
  double _eTrendSlope; 

  // E forecast

  int _forecastDeltaSecs; // number of secs between forecasts
  time_t _forecastStartTime; // unix time
  vector<double> _forecastE;
  vector<double> _forecastDbz;

  // buffer for assemble / disassemble

  MemBuf _memBuf;

private:

  void _loadHeader(zv_calib_hdr_t &hdr) const;
  void _unloadHeader(const zv_calib_hdr_t &hdr);

};


#endif
