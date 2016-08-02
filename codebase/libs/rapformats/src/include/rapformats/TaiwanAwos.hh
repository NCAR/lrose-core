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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: TaiwanAwos.hh,v 1.14 2016/03/03 19:23:53 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	TaiwanAwos
// 
// Author:	G. M. Cunning
// 
// Date:	Sun Oct 19 09:46:24 2008
// 
// Description:	This class manages Taiwan ASOS/AWOS SPDB messages.
// 
//
// the units are:
//   wind speeds     -- knots
//   wind directions -- degrees (from true North?)
//   pressure        -- hPa 
//   RVR             -- meters
//   visibility      -- meters
//   rainfall acc.   -- millimeters
//   temperature     -- celsius
//   dew point       -- celsius
//   humidity        -- %
//   cloudiness      -- enum: NONE (0); FEW (1, 2); SCT (3, 4); BKN (5-7); OVC (8)  
//   cloud height    -- feet
//
//   In awos_obs_t, min_rvr, min_rvr_10_min_avg, min_low_cloud_hgt, 
//   min_med_cloud_hgt and min_high_cloud_hgt are minimum height flags.
//   flag values are 0 or 1 to indicate that rvr, for instance,  is a 
//   minimum height. these flags are mirrored with the private members:
//   _minimumRvr, _avgRvr10Min, _minimumAvgRvr10Min, _minimumLowCloudHgt, 
//   _minimumMedCloudHgt, and _minimumHighCloudHgt.
//

# ifndef    TAIWAN_AWOS_H
# define    TAIWAN_AWOS_H

// C++ include files
#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>

// System/RAP include files

// Local include files

using namespace std;

class TaiwanAwos {
  
public:

  ////////////////////
  // public members //
  ////////////////////

  // missing data value
  static const float missingVal;

  // define the structure of the AWOS message
  static const size_t AIRPORT_ID_STR_LEN = 8;
  static const size_t AWOS_ID_STR_LEN = 8;
  static const size_t INFO_STR_LEN = 64;
  static const size_t WINDS_HISTORY_LEN = 10;
  static const size_t HEADER_32_OFFSET = 80;
  static const size_t HEADER_32_SIZE = 64;

  typedef struct {
    char airport_id[AIRPORT_ID_STR_LEN];
    char awos_id[AWOS_ID_STR_LEN];
    char info[INFO_STR_LEN];
    fl32 latitude;
    fl32 longitude;
    fl32 altitude;
    fl32 spare_1;
    fl32 spare_2;
    fl32 spare_3;
    fl32 spare_4;
    si32 buf_len;
    si32 airport_id_len;
    si32 awos_id_len;
    si32 info_len;
    si32 spare_5;
    si32 spare_6;
    si32 spare_7;
    si32 spare_8;
    si32 spare_9;
  } awos_header_t;

  typedef struct {
    ti32 valid_time;
    fl32 instant_wind_speed;
    fl32 instant_wind_dir;
    fl32 avg_wind_speed_2_min;
    fl32 avg_wind_dir_2_min;
    fl32 max_wind_speed_2_min;
    fl32 min_wind_speed_2_min;
    fl32 max_wind_dir_2_min;
    fl32 min_wind_dir_2_min;
    fl32 avg_wind_speed_10_min;
    fl32 avg_wind_dir_10_min;
    fl32 max_wind_speed_10_min;
    fl32 min_wind_speed_10_min;
    fl32 max_wind_dir_10_min;
    fl32 min_wind_dir_10_min;
    fl32 qnh;
    fl32 qff;
    fl32 qfe;
    fl32 rvr; 
    si32 min_rvr;
    fl32 rvr_10_min_avg;
    si32 min_rvr_10_min_avg; 
    fl32 vis; 
    si32 min_vis;
    fl32 vis_10_min_avg;
    si32 min_vis_10_min_avg; 
    fl32 temperature;
    fl32 dewpoint;
    fl32 humidity;
    fl32 rainfall_acc_1_hr;
    fl32 rainfall_acc_6_hr;
    fl32 rainfall_acc_12_hr;
    fl32 rainfall_acc_24_hr;
    si32 low_cloudiness;
    si32 med_cloudiness;
    si32 high_cloudiness;
    fl32 low_cloud_hgt;
    fl32 med_cloud_hgt;
    fl32 high_cloud_hgt;
    si32 min_low_cloud_hgt;
    si32 min_med_cloud_hgt;
    si32 min_high_cloud_hgt;
    fl32 spare_1;
    fl32 spare_2;
    fl32 spare_3;
    fl32 spare_4;
    si32 spare_5;
    si32 spare_6;
    si32 spare_7;
    si32 spare_8;
  } awos_obs_t;


  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  TaiwanAwos();
  TaiwanAwos(const TaiwanAwos &that);

  // destructor
  virtual ~TaiwanAwos();


  // check the object
  bool check () const;

  ///////////////////////////////////////////////////////////////
  // set methods
  
  // clear the object
  void clear(bool only_obs = false);

  void setAirportId(const string &id) { _airportId = id; }
  void setId(const string &id) { _awosId = id; }
  void setInfo(const string &info) { _info = info; }
  void setLatitude(float val) { _latitude = val; }
  void setLongitude(float val) { _longitude = val; }
  void setAltitude(float val) { _altitude = val; }
  void setValidTime(time_t val) { _validTime = val; }
  void setQnh(float val) { _qnh = val; }
  void setQfe(float val) { _qfe = val; }
  void setQff(float val) { _qff = val; }
  void setVis(float val) { _vis = val; }
  void setMinimumVis(bool val) { _minimumVis = val; }
  void setAvgVis10Min(float val) { _avgVis10Min = val; }
  void setMinimumAvgVis10Min(bool val) { _minimumAvgVis10Min = val; }
  void setRvr(float val) { _rvr = val; }
  void setMinimumRvr(bool val) { _minimumRvr = val; }
  void setAvgRvr10Min(float val) { _avgRvr10Min = val; }
  void setMinimumAvgRvr10Min(bool val) { _minimumAvgRvr10Min = val; }
  void setTemperature(float val) { _temperature = val; }
  void setDewpoint(float val) { _dewpoint = val; }
  void setHumidity(float val) { _humidity = val; }
  void setRainfallAcc1Hr(float val) { _rainfallAcc1Hr = val; }
  void setRainfallAcc6Hr(float val) { _rainfallAcc6Hr = val; }
  void setRainfallAcc12Hr(float val) { _rainfallAcc12Hr = val; }
  void setRainfallAcc24Hr(float val) { _rainfallAcc24Hr = val; }
  void setLowCloudiness(int val) { _lowCloudiness = val; }
  void setMedCloudiness(int val) { _medCloudiness = val; }
  void setHighCloudiness(int val) { _highCloudiness = val; }
  void setLowCloudHeight(float val) { _lowCloudHgt = val; }
  void setMedCloudHeight(float val) { _medCloudHgt = val; }
  void setHighCloudHeight(float val) { _highCloudHgt = val; }
  void setMinimumLowCloudHeight(bool val) { _minimumLowCloudHgt = val; }
  void setMinimumMedCloudHeight(bool val) { _minimumMedCloudHgt = val; }
  void setMinimumHighCloudHeight(bool val) { _minimumHighCloudHgt = val; }
  void setInstantWindSpeed(float val) { _instantWindSpeed = val; }
  void setInstantWindDir(float val) { _instantWindDir = val; }
  void setAvgWindSpeed2Min(float val) { _avgWindSpeed2Min = val; }
  void setAvgWindDir2Min(float val) { _avgWindDir2Min = val; }
  void setMaxWindSpeed2Min(float val) { _maxWindSpeed2Min = val; }
  void setMinWindSpeed2Min(float val) { _minWindSpeed2Min = val; }
  void setMaxWindDir2Min(float val) { _maxWindDir2Min = val; }
  void setMinWindDir2Min(float val) { _minWindDir2Min = val; }
  void setAvgWindSpeed10Min(float val) { _avgWindSpeed10Min = val; }
  void setAvgWindDir10Min(float val) { _avgWindDir10Min = val; }
  void setMaxWindSpeed10Min(float val) { _maxWindSpeed10Min = val; }
  void setMinWindSpeed10Min(float val) { _minWindSpeed10Min = val; }
  void setMaxWindDir10Min(float val) { _maxWindDir10Min = val; }
  void setMinWindDir10Min(float val) { _minWindDir10Min = val; }
  

  ///////////////////////////////////////////////////////////////
  // get methods

  string getAirportId() { return _airportId; }
  string getId() { return _awosId; }
  string getInfo() { return _info; }
  float getLatitude() { return _latitude; }
  float getLongitude() { return _longitude; }
  float getAltitude() { return _altitude; }
  time_t getValidTime() { return _validTime; }
  float getQnh() { return _qnh; }
  float getQff() { return _qff; }
  float getQfe() { return _qfe; }
  float getVis() { return _vis; }
  bool getMinimumVis() { return _minimumVis; }
  float getAvgVis10Min() { return _avgVis10Min; }
  bool getMinimumAvgVis10Min() { return _minimumAvgVis10Min; }
  float getRvr() { return _rvr; }
  bool getMinimumRvr() { return _minimumRvr; }
  float getAvgRvr10Min() { return _avgRvr10Min; }
  bool getMinimumAvgRvr10Min() { return _minimumAvgRvr10Min; }
  float getTemperature() { return _temperature; }
  float getDewpoint() { return _dewpoint; }
  float getHumidity() { return _humidity; }
  float getRainfallAcc1Hr() { return _rainfallAcc1Hr; }
  float getRainfallAcc6Hr() { return _rainfallAcc6Hr; }
  float getRainfallAcc12Hr() { return _rainfallAcc12Hr; }
  float getRainfallAcc24Hr() { return _rainfallAcc24Hr; }
  int getLowCloudiness() { return _lowCloudiness; }
  int getMedCloudiness() { return _medCloudiness; }
  int getHighCloudiness() { return _highCloudiness; }
  float getLowCloudHeight() { return _lowCloudHgt; }
  float getMedCloudHeight() { return _medCloudHgt; }
  float getHighCloudHeight() { return _highCloudHgt; }
  bool getMinimumLowCloudHeight() { return _minimumLowCloudHgt; }
  bool getMinimumMedCloudHeight() { return _minimumMedCloudHgt; }
  bool getMinimumHighCloudHeight() { return _minimumHighCloudHgt; }
  float getInstantWindSpeed() { return _instantWindSpeed; }
  float getInstantWindDir() { return _instantWindDir; }
  float getAvgWindSpeed2Min() { return _avgWindSpeed2Min; }
  float getAvgWindDir2Min() { return _avgWindDir2Min; }
  float getMaxWindSpeed2Min() { return _maxWindSpeed2Min; }
  float getMinWindSpeed2Min() { return _minWindSpeed2Min; }
  float getMaxWindDir2Min() { return _maxWindDir2Min; }
  float getMinWindDir2Min() { return _minWindDir2Min; }
  float getAvgWindSpeed10Min() { return _avgWindSpeed10Min; }
  float getAvgWindDir10Min() { return _avgWindDir10Min; }
  float getMaxWindSpeed10Min() { return _maxWindSpeed10Min; }
  float getMinWindSpeed10Min() { return _minWindSpeed10Min; }
  float getMaxWindDir10Min() { return _maxWindDir10Min; }
  float getMinWindDir10Min() { return _minWindDir10Min; }


  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the object values.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure

  int disassemble(const void *buf, int len);

  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  //
  // returns 0 on success, -1 on failure
  // Use getErrStr() on failure.
  
  int assemble();


  // get the assembled buffer pointer

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }


  // getErrStr -- returns error message
  string getErrStr() const { return _errStr; }

  /////////////////////////
  // print

  void print(FILE *out) const;
  void print(ostream &out) const;

  TaiwanAwos &operator=(const TaiwanAwos &);

  friend bool operator==(const TaiwanAwos &, const TaiwanAwos &);


protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  mutable string _errStr;
  static const string _className;

  string _airportId;
  string _awosId;
  string _info;
  float _latitude;  // degrees
  float _longitude; // degrees
  float _altitude;  // meters
  time_t _validTime;
  float _qnh;
  float _qff;
  float _qfe;
  float _rvr; 
  bool _minimumRvr; 
  float _avgRvr10Min; 
  bool _minimumAvgRvr10Min; 
  float _vis; 
  bool _minimumVis; 
  float _avgVis10Min; 
  bool _minimumAvgVis10Min; 
  float _temperature;
  float _dewpoint;
  float _humidity;
  float _rainfallAcc1Hr;
  float _rainfallAcc6Hr;
  float _rainfallAcc12Hr;
  float _rainfallAcc24Hr;
  int _lowCloudiness;
  int _medCloudiness;
  int _highCloudiness;
  float _lowCloudHgt;
  float _medCloudHgt;
  float _highCloudHgt;
  bool _minimumLowCloudHgt;
  bool _minimumMedCloudHgt;
  bool _minimumHighCloudHgt;
  float _instantWindSpeed;
  float _instantWindDir;
  float _avgWindSpeed2Min;
  float _avgWindDir2Min;
  float _maxWindSpeed2Min;
  float _minWindSpeed2Min;
  float _maxWindDir2Min;
  float _minWindDir2Min;
  float _avgWindSpeed10Min;
  float _avgWindDir10Min;
  float _maxWindSpeed10Min;
  float _minWindSpeed10Min;
  float _maxWindDir10Min;
  float _minWindDir10Min;
 
  MemBuf _memBuf;

  /////////////////////
  // private methods //
  /////////////////////

  void _BE_from_header(awos_header_t &hdr);
  void _BE_to_header(awos_header_t &hdr);

  void _BE_from_obs(awos_obs_t &obs);
  void _BE_to_obs(awos_obs_t &obs);

  void _copy(const TaiwanAwos &from);

};

bool operator==(const TaiwanAwos &left, 
		const TaiwanAwos &right);

inline bool operator!=(const TaiwanAwos &left, 
		       const TaiwanAwos &right)
{
  return !(left == right);
}

# endif     /* TAIWAN_AWOS_H */
