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
// SunAngle.hh
//
// Computes sun angle and related quantities
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
/////////////////////////////////////////////////////////////

#ifndef SunAngle_HH
#define SunAngle_HH

#include <ctime>
#include <string>
#include <euclid/euclid_macros.h>

using namespace std;

class SunAngle {
  
public:
  
  // constructor
  
  SunAngle();
  
  // Destructor
  
  virtual ~SunAngle();

  // initialize for a given time
  
  void initForTime(time_t unix_time);

  // compute altitude and azimuth for given time and lat/lon
  // Must call initForTime first

  void computeAltAz(double lat, double lon,
                    double &alt, double &az);

  // compute altitude for given time and lat/lon
  // Must call initForTime first
  
  double computeAlt(double lat, double lon);
  
  // compute azimuth for given time and lat/lon
  // Must call initForTime first

  double computeAz(double lat, double lon);
  
  // compute sine of the altitude for given time and lat/lon.
  // Must call initForTime first
  
  double computeSinAlt(double lat, double lon);

  // compute cosine of the altitude for given time and lat/lon.
  // This is more efficient that calling computeAlt and then taking the cosine.
  // Must call initForTime first
  
  double computeCosAlt(double lat, double lon);

  // get methods

  time_t getTime() const { return _time; }
  double getUtHourDeg() const { return _utHourDeg; }
  double getRightAscension() const { return _ra; }
  double getDeclination() const { return _declRad * RAD_TO_DEG; }
  double getDeclinationRad() const { return _declRad; }
  double getGmst0() const { return _gmst0; }
  double getDist() const { return _dist; }

private:

  static const double _secsPerDay;

  time_t _unixTime2000; // time at 1999/12/31 00:00:00

  time_t _time; // time for which the object is initialized
  double _utHourDeg; // decimal hour of the UT, in degrees

  double _ra; // right ascension
  double _declRad; // declination in radians
  double _gmst0; // sidereal time at Greenwich

  double _dist; // distance to sun in astronomical units
  
};

#endif



