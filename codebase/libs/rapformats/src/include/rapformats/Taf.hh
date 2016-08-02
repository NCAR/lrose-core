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
// Taf.hh
//
// C++ class for dealing with TAF - Terminal Aerodrome Forecast
// For TAF details, see ICAO Annex 3
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Oct 2009
//////////////////////////////////////////////////////////////

#ifndef _Taf_hh
#define _Taf_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>

using namespace std;

class Taf {

public:

  // internal types

  typedef enum {
    PERIOD_MAIN,
    PERIOD_BECMG,
    PERIOD_TEMPO,
    PERIOD_FROM,
    PERIOD_PROB
  } period_type_t;

  class CloudLayer {
  public:
    double heightKm; // ht in Km MSL
    double cloudCover; // 0 to 1
    bool cbPresent; // is CB present
    CloudLayer() {
      clear();
    }
    void clear() {
      heightKm = missing;
      cloudCover = missing;
      cbPresent = false;
    }
  };

  class ForecastPeriod {
  public:
    time_t startTime;   
    time_t endTime;
    period_type_t pType;
    double probPercent;  // probability if pType == PERIOD_PROB
    double windDirnDegT; // wind dirn in deg T
    bool windDirnVrb; // is wind variable?
    double windSpeedKmh; // wind speed in kmh
    double windGustKmh; // gust speed in kmh
    double visKm; // horizontal visibility in km
    bool isCavok; // is CAVOK?
    vector<CloudLayer> layers;
    double ceilingKm; // ceiling in km
    vector<string> wx;
    double maxTempC;
    time_t maxTempTime;
    double minTempC;
    time_t minTempTime;
    string text;
    ForecastPeriod() {
      clear();
    }
    void clear() {
      startTime = 0;   
      endTime = 0;
      pType = PERIOD_MAIN;
      probPercent = 100.0;
      windDirnDegT = missing;
      windDirnVrb = false;
      windSpeedKmh = missing;
      windGustKmh = missing;
      visKm = missing;
      isCavok = false;
      ceilingKm = missing;
      layers.clear();
      wx.clear();
      maxTempC = missing;
      maxTempTime = 0;
      minTempC = missing;
      minTempTime = 0;
      text.clear();
    }
  };

  // constructor

  Taf();

  // destructor

  ~Taf();

  // missing value
  
  static const double missingVal;

  //////////////////////// set methods /////////////////////////

  // clear to default state
  
  void clear();
  
  // get location and time

  void setStationId(const string &id) { _stationId = id; }
  void setIssueTime(time_t time) { _issueTime = time; }
  void setValidTime(time_t time) { _validTime = time; }
  void setExpireTime(time_t time) { _expireTime = time; }
  void setLatitude(double lat) { _latitude = lat; }
  void setLongitude(double lon) { _longitude = lon; }
  void setElevationM(double lon) { _elevationM = lon; }

  void setNil(bool state = true) { _isNil = state; }
  void setAmended(bool state = true) { _isAmended = state; }
  void setCorrected(bool state = true) { _isCorrected = state; }
  void setCancelled(bool state = true) { _isCancelled = state; }
  void setCancelTime(time_t time) { _cancelTime = time; }

  // set other members

  void setText(const string &text) { _text = text; }

  // add a forecast period

  void addForecastPeriod(const ForecastPeriod &period) {
    _periods.push_back(period);
  }

  //////////////////////// get methods /////////////////////////

  // get location and time
  
  inline const string &getStationId() const { return _stationId; }
  inline time_t getIssueTime() const { return _issueTime; }
  inline time_t getValidTime() const { return _validTime; }
  inline time_t getExpireTime() const { return _expireTime; }
  inline double getLatitude() const { return _latitude; }
  inline double getLongitude() const { return _longitude; }
  inline double getElevationMeters() const { return _elevationM; }
  inline double getElevationKm() const { return _elevationM / 1000.0; }

  inline bool isNil() const { return _isNil; } 
  inline bool isAmended() const { return _isAmended; } 
  inline bool isCorrected() const { return _isCorrected; }
  inline bool isCancelled() const { return _isCancelled; }
  inline time_t getCancelTime() const { return _cancelTime; }
  
  // get other members

  inline const string &getText() const { return _text; }
  inline const vector<ForecastPeriod> &getPeriods() { return _periods; }

  ///////////////////////////////////////////
  // assemble as XML
  // Load up an XML buffer from the object.

  void assemble();

  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ///////////////////////////////////////////
  // load XML from object
  
  void loadXml(string &xml, int startIndentLevel = 0) const;
  
  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  /////////////////////////
  // print
  
  void print(ostream &out, string spacer = "") const;

  static string periodType2Str(period_type_t ptype);
  static period_type_t periodTypeStr2Enum(const string ptypeStr);

  // Print an XML representation of the object.
  
  void printAsXml(ostream &out, int startIndentLevel = 0) const;

  // missing data value
  
  static const double missing;

  // weather strings

  static const int nWxTypes = 40;
  static const char *wxTypes[nWxTypes];
  
protected:
private:

  // time

  time_t _issueTime;
  time_t _validTime;
  time_t _expireTime;

  // location

  string _stationId;
  double _latitude;
  double _longitude;
  double _elevationM;

  // is NIL taf

  bool _isNil;

  // amended / corrected

  bool _isAmended;
  bool _isCorrected;

  // is cancelled

  bool _isCancelled;
  time_t _cancelTime;
  
  // message text

  string _text;

  // forecasts

  vector<ForecastPeriod> _periods;

  // xml

  mutable string _xml;
  
  // buffer for assemble / disassemble

  MemBuf _memBuf;
  
  // functions
  
  void _tokenize(const string &str,
                 const string &spacer,
                 vector<string> &toks);
  
};

// set static const values

#ifdef _in_taf_cc
const double Taf::missing = -9999;
#endif

#endif


