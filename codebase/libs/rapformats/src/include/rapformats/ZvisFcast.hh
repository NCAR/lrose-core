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
// ZvisFcast.hh
//
// C++ class for dealing with z-v probability calibration
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// August 2006
//////////////////////////////////////////////////////////////

#ifndef _ZvisFcast_hh
#define _ZvisFcast_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <toolsa/MemBuf.hh>

using namespace std;

class ZvisFcast {

public:

  // constructor

  ZvisFcast();

  // destructor

  ~ZvisFcast();
  
  // enum for visibility units

  typedef enum {
    VIS_UNITS_KM,  // kilometers
    VIS_UNITS_MI,  // statute miles
    VIS_UNITS_NM   // nautical miles
  } vis_units_t;

  // class for vis categories
  
  class VisCat {
  public:
    string name;   // VFR, MVFR, IFR, LIFR
    double minVis; // min vis for category
  };

  // class for forecast entry
  
  class FcastEntry {
  public:
    int leadSecs;
    vector<double> prob; // probability for each category
  };

  // enum for calibration status

  typedef enum {
    CALIBRATED,
    CLIMATOLOGY,
    CAL_DEFAULTS
  } cal_status_t;

  // clear all data members
  
  void clear();

  //////////////////////// set methods /////////////////////////

  void setVisUnits(vis_units_t units) { _visUnits = units; }

  // calibration

  void setCalTime(time_t ctime) { _calTime = ctime; }
  void setCalStatus(cal_status_t status) { _calStatus = status; }
  void setCalIntercept(double intercept) { _calIntercept = intercept; }
  void setCalSlope(double slope) { _calSlope = slope; }

  // forecast location

  void setLocationName(const string &name) { _locationName = name; }
  void setLocationLat(double lat) { _locationLat = lat; }
  void setLocationLon(double lon) { _locationLon = lon; }

  // add a visibility category
  
  void addCategory(const string &name, double minVis);

  // forecasts

  void setGenTime(time_t gtime) { _genTime = gtime; }
  void setDeltaSecs(int secs) { _deltaSecs = secs; }

  // add a forecast entry
  // Number of forecast probabilities must match the number of vis categories.
  // returns 0 on success, -1 on failure.

  int addEntry(const FcastEntry &entry);

  //////////////////////// get methods /////////////////////////

  int getVersionNum() const { return _versionNum; }
  
  // visibility units

  vis_units_t getVisUnits() const { return _visUnits; }
  string getVisUnitsStr() const;

  // calibration
  
  time_t getCalTime() const { return  _calTime; }
  cal_status_t getCalStatus() const { return _calStatus; }
  string getCalStatusStr() const;
  double getCalIntercept() const { return _calIntercept; }
  double getCalSlope() const { return _calSlope; }

  // forecast location

  const string &getLocationName() const { return  _locationName; }
  double getLocationLat() const { return  _locationLat; }
  double getLocationLon() const { return  _locationLon; }

  // forecast categories

  int getNCategories() const { return (int) _cats.size(); }
  const vector<VisCat> &getCategories() const { return _cats; }
  
  // forecasts
  
  time_t getGenTime() const { return  _genTime; }
  int getDeltaSecs() const { return _deltaSecs; }
  int getNForecasts() const { return (int) _entries.size(); }
  const vector<FcastEntry> &getFcastEntries() const { return _entries; }

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
  
protected:

  int _versionNum;

  // visibility units
  
  vis_units_t _visUnits;

  // calibration
  // log(viskm) = intercept + slope * dbz

  time_t _calTime;
  cal_status_t _calStatus;
  double _calIntercept;
  double _calSlope;

  // forecast location
  
  string _locationName;
  double _locationLat;
  double _locationLon;
  
  // visibility categories

  vector<VisCat> _cats;

  // forecasts

  time_t _genTime;      // start time of forecasts
  int _deltaSecs;  // time between forecasts
  vector<FcastEntry> _entries; // forecast entries

  // buffer for assemble / disassemble

  MemBuf _memBuf;

private:

};


#endif

