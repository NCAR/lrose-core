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
//////////////////////////////////////////////////////////
// SunAngle.cc
//
// Computes sun angle and related quantities
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
//////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <iomanip>
#include <euclid/SunAngle.hh>
#include <toolsa/DateTime.hh>

// constants are in MKS

const double SunAngle::_secsPerDay = 86400.0;

// Constructor

SunAngle::SunAngle()

{

  // compute unix time for Mjd2000 - 0 UTC, 31 dec 1999

  DateTime u2000(1999, 12, 31, 0, 0, 0);
  _unixTime2000 = u2000.utime();

}

// Destructor

SunAngle::~SunAngle()

{

}

/////////////////////////////////////////////////////////
// initialize for a given time
//
// This code is based on formulae given on a web site by
// Paul Schlyter, Stockholm, Sweden
// email: pausch@stjarnhimlen.se or WWW: http://stjarnhimlen.se/
// See http://www.stjarnhimlen.se/comp/ppcomp.html#5

void SunAngle::initForTime(time_t unix_time)

{

  _time = unix_time;
  _utHourDeg = (fmod((double) _time, _secsPerDay) / 3600.0) * 15.0;

  // days since 2000

  double day = ((double) _time - (double) _unixTime2000) / _secsPerDay;

  // obliquity of the ecliptic

  double ecl = 23.4393 - 3.563e-7 * day;

  // sun mean anomaly

  double MM = 356.0470 + 0.9856002585 * day;
  double MMRad = MM * DEG_TO_RAD;

  // eccentricity

  double ee = 0.016709 - 1.151E-9 * day;

  // eccentricity anomaly

  double EE = MM +
    (ee * RAD_TO_DEG) * sin(MMRad) * (1.0 + ee * cos(MMRad));
  double EERad = EE * DEG_TO_RAD;

  // compute the Sun's distance r and its true anomaly v

  double xv = cos(EERad) - ee;
  double yv = sqrt(1.0 - ee * ee) * sin(EERad);

  double v = atan2(yv, xv) * RAD_TO_DEG;
  _dist = sqrt(xv*xv + yv*yv);

  // compute w, argument of perihelion

  double w = 282.9404 + 4.70935E-5 * day;

  // compute the Sun's true longitude
  
  double lonsun = v + w;

  // convert lonsun,r to ecliptic rectangular geocentric coordinates xs,ys:

  double xs = _dist * cos(lonsun * DEG_TO_RAD);
  double ys = _dist * sin(lonsun * DEG_TO_RAD);

  // convert equatorial, rectangular, geocentric coordinates

  double xe = xs;
  double ye = ys * cos(ecl * DEG_TO_RAD);
  double ze = ys * sin(ecl * DEG_TO_RAD);

  // compute the Sun's Right Ascension (RA) and Declination (Dec):

  _ra  = atan2(ye, xe) * RAD_TO_DEG;
  _declRad = atan2(ze, sqrt(xe*xe+ye*ye));

  // compute sun's mean longitude

  double LL = MM + w;

  // compute sidereal time at Greenwich
  
  _gmst0 = LL + 180.0;

}

//////////////////////////////////////////////////////////////////////
// compute altitude and azimuth for given time and lat/lon
//
// Must call initForTime first

void SunAngle::computeAltAz(double lat, double lon,
                            double &alt, double &az)

{

  // compute our Local Sidereal Time (LST) in degrees

  double LST = _gmst0 + _utHourDeg + lon;

  // compute the Sun's Local Hour Angle (LHA)
  
  double LHA = LST - _ra;
  double lhaRad = LHA * DEG_TO_RAD;

  // compute the altitude

  double latRad = lat *  DEG_TO_RAD;
  double sinAlt =
    sin(latRad) * sin(_declRad) + cos(latRad) * cos(_declRad) * cos(lhaRad);
  alt = asin(sinAlt) * RAD_TO_DEG;

  // compute azimuth

  double yyy = sin(lhaRad);
  double xxx = cos(lhaRad) * sin(latRad) - tan(_declRad) * cos(latRad);
  az = atan2(yyy, xxx) * RAD_TO_DEG + 180.0;
  if (az < 0) {
    az += 360.0;
  } else if (az > 360.0) {
    az -= 360.0;
  }

}

//////////////////////////////////////////////////////////////////////
// compute altitude for given time and lat/lon
//
// Must call initForTime first

double SunAngle::computeAlt(double lat, double lon)

{

  // compute our Local Sidereal Time (LST) in degrees

  double LST = _gmst0 + _utHourDeg + lon;

  // compute the Sun's Local Hour Angle (LHA)
  
  double LHA = LST - _ra;
  double lhaRad = LHA * DEG_TO_RAD;

  // compute the altitude

  double latRad = lat *  DEG_TO_RAD;
  double sinAlt =
    sin(latRad) * sin(_declRad) + cos(latRad) * cos(_declRad) * cos(lhaRad);
  double alt = asin(sinAlt) * RAD_TO_DEG;

  return alt;

}

//////////////////////////////////////////////////////////////////////
// compute sine of the altitude for given time and lat/lon
//
// Must call initForTime first

double SunAngle::computeSinAlt(double lat, double lon)

{

  // compute our Local Sidereal Time (LST) in degrees
  
  double LST = _gmst0 + _utHourDeg + lon;

  // compute the Sun's Local Hour Angle (LHA)
  
  double LHA = LST - _ra;
  double lhaRad = LHA * DEG_TO_RAD;

  // compute the altitude

  double latRad = lat *  DEG_TO_RAD;
  double sinAlt =
    sin(latRad) * sin(_declRad) + cos(latRad) * cos(_declRad) * cos(lhaRad);

  return sinAlt;

}

//////////////////////////////////////////////////////////////////////
// compute cosine of the altitude for given time and lat/lon
// This is more efficient that calling computeAlt and then taking the cosine.
//
// Must call initForTime first

double SunAngle::computeCosAlt(double lat, double lon)

{

  double sinAlt = computeSinAlt(lat, lon);
  double cosAlt = sqrt(1.0 - sinAlt * sinAlt);
  return cosAlt;

}

//////////////////////////////////////////////////////////////////////
// compute azimuth for given time and lat/lon
//
// Must call initForTime first

double SunAngle::computeAz(double lat, double lon)

{

  // compute our Local Sidereal Time (LST) in degrees

  double LST = _gmst0 + _utHourDeg + lon;

  // compute the Sun's Local Hour Angle (LHA)
  
  double LHA = LST - _ra;
  double lhaRad = LHA * DEG_TO_RAD;

  // compute azimuth

  double latRad = lat *  DEG_TO_RAD;
  double yyy = sin(lhaRad);
  double xxx = cos(lhaRad) * sin(latRad) - tan(_declRad) * cos(latRad);
  double az = atan2(yyy, xxx) * RAD_TO_DEG + 180.0;
  if (az < 0) {
    az += 360.0;
  } else if (az > 360.0) {
    az -= 360.0;
  }

  return az;

}

