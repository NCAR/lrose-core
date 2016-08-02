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
///////////////////////////////////////////////////////////////////////
//
// SunPosn
//
// Computes sun postion
//
// Mike Dixon
//
// June 2006
//
////////////////////////////////////////////////////////////////////////

#include <euclid/SunPosn.hh>
#include <euclid/sincos.h>
#include <cmath>
#include "rsts_sun_pos.h"
#include "novas.h"

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308092
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994372
#endif

#define JAN_1_1970 2440587

// constructor

SunPosn::SunPosn()

{
  _prevTime = 0.0;
  _lat = 0.0;
  _lon = 0.0;
  _el = -99.0;
  _az = -99.0;
}

// destructor

SunPosn::~SunPosn()

{
}

// set the lat/lon/alt for which sun position is computed
// lat/lon in degrees, alt_m in meters

void SunPosn::setLocation(double lat, double lon, double alt_m)

{
  _prevTime = 0.0;
  _lat = lat;
  _lon = lon;
  _alt_m = alt_m;
}

    
// compute sun position

void SunPosn::computePosn(double now, double &el, double &az)

{

  // if less than 1 second has elapsed, used previously-computed values

  _el = 0.0;
  _az = 0.0;
  
  // get the time at the start of the year

  time_t tnow = (time_t) now;
  int nowYear, nowMonth, nowDay, nowHour, nowMin, nowSec;
  _compute_date_time(tnow,
                     nowYear, nowMonth, nowDay,
                     nowHour, nowMin, nowSec);
  double yearStartUtime = _compute_unix_time(nowYear, 1, 1, 0, 0, 0);

  double jday = (now - yearStartUtime) / 86400.0;
  double jdeg = jday * (360.0 / 365.25);
  
  double eqt = 0.123 * cos((jdeg + 87.0) * DEG_TO_RAD) -
    0.16666667 * sin((jdeg + 10.0) * DEG_TO_RAD * 2.0);
  
  double decl =
    -23.5 * cos(((jday + 10.3) * (360.0 / 365.25)) * DEG_TO_RAD);
  double decl_rad = decl * DEG_TO_RAD;
  double sin_decl, cos_decl;
  EG_sincos(decl_rad, &sin_decl, &cos_decl);
  
  double hr = now / 3600.0;
  double gmt_hr = fmod((hr - 12.0), 24.0);
  
  double solar_time = (gmt_hr + (_lon / 15.0) + eqt);
  
  double hour_deg = solar_time * 15.0;
  double hour_rad = hour_deg * DEG_TO_RAD;
  double sin_hour, cos_hour;
  EG_sincos(hour_rad, &sin_hour, &cos_hour);
  
  double lat_rad = _lat * DEG_TO_RAD;
  double sin_lat, cos_lat;
  EG_sincos(lat_rad, &sin_lat, &cos_lat);

  double sin_el = sin_lat * sin_decl + cos_lat * cos_decl * cos_hour;
  double el_rad = asin(sin_el);
  double el_deg = el_rad * RAD_TO_DEG;
  
  double tan_az = (sin_hour / ((cos_hour * sin_lat) - (tan(decl_rad) * cos_lat)));
  
  double az_rad = atan(tan_az);
  double az_deg = az_rad * RAD_TO_DEG;
  
  double az_adjusted = az_deg;
  
  if (hour_deg >= 0.0 && hour_deg < 180.0) {
    if (az_adjusted > 0.0) {
      az_adjusted += 180.0;
    } else {
      az_adjusted += 360.0;
    }
  } else {
    if (az_adjusted < 0.0) {
      az_adjusted += 180.0;
    }
  }
  
  if (false) {
    cerr << "    jday: " << jday << endl;
    cerr << "    jdeg: " << jdeg << endl;
    cerr << "    gmt_hr: " << gmt_hr << endl;
    cerr << "    solar_time: " << solar_time << endl;
    cerr << "    hour_deg: " << hour_deg << endl;
    cerr << "    lat_rad: " << lat_rad << endl;
    cerr << "    el: " << el_deg << endl;
    cerr << "    az: " << az_adjusted << endl;
  }
  
  _el = el_deg;
  _az = az_adjusted;
  _prevTime = now;

  el = _el;
  az = _az;
  
}

/////////////////////////////////////////////
// compute sun position using NOVA routines

void SunPosn::computePosnNova(double now, double &el, double &az)

{

  // set up site info
  
  double tempC = 20;
  double pressureMb = 1013;

  site_info site = { _lat, _lon, _alt_m, tempC, pressureMb };
  
  // set time
  
  time_t tnow = (time_t) now;
  double deltat = -0.45;
  
  // compute sun posn
  
  rsts_SunNovasComputePosAtTime(site, deltat, &az, &el, tnow);

}

/////////////////////////////////////////////
// original sun position - Reinhart

void SunPosn::computePosnOld(double now, double &el, double &az)

{

  // get the time at the start of the year

  time_t tnow = (time_t) now;
  int nowYear, nowMonth, nowDay, nowHour, nowMin, nowSec;
  _compute_date_time(tnow,
                     nowYear, nowMonth, nowDay,
                     nowHour, nowMin, nowSec);
  double yearStartUtime = _compute_unix_time(nowYear, 1, 1, 0, 0, 0);
  
  double jday = (double) (now - yearStartUtime) / 86400.0;
  double jdeg = jday * (360.0 / 365.25);

  double eqt = 0.123 * cos((jdeg + 87.0) * DEG_TO_RAD) -
    0.16666667 * sin(((jdeg + 10.0) * DEG_TO_RAD) * 2.0);

  double decl = -23.5 * cos((jday + 10.3) * (360.0 / 365.25) * DEG_TO_RAD);
  double decl_rad = decl * DEG_TO_RAD;
  double sin_decl, cos_decl;
  EG_sincos(decl_rad, &sin_decl, &cos_decl);

  double gmt_hr = fmod(((now / 3600.0) - 12.0), 24.0);
  double solar_time = (gmt_hr + (_lon / 15.0) + eqt);
  
  double hour_deg = solar_time * 15.0;
  double hour_rad = hour_deg * DEG_TO_RAD;
  double sin_hour, cos_hour;
  EG_sincos(hour_rad, &sin_hour, &cos_hour);

  double lat_rad = _lat * DEG_TO_RAD;
  double sin_lat, cos_lat;
  EG_sincos(lat_rad, &sin_lat, &cos_lat);
  
  double sin_alt = sin_lat * sin_decl + cos_lat * cos_decl * cos_hour;
  double alt = asin(sin_alt);
  double alt_deg = alt * RAD_TO_DEG;

  double tan_az = (sin_hour / ((cos_hour * sin_lat) - (tan(decl_rad) * cos_lat)));

  double az_rad = atan(tan_az);
  double az_deg = az_rad * RAD_TO_DEG;
  
  if (hour_deg >= 0.0 && hour_deg < 180.0) {
    if (az_deg > 0.0) {
      az_deg += 180.0;
    } else {
      az_deg += 360.0;
    }
  } else {
    if (az_deg < 0.0) {
      az_deg += 180.0;
    }
  }

  el = alt_deg;
  az = az_deg;

}

/////////////////////////////////////////////
// sun position - Meeus
//
// This is lifted from the original ERL program.
// Converted to VB J. Lutz 2/16/98.
// Converted to C++ Mike Dixon Sept 2007

void SunPosn::computePosnMeeus(double now, double &el, double &az)

{

  double Dtr = 0.017453292519943;
  double Rtd = 57.295779513082;
  double Pi = 3.1415926535898;
  double Pi2 = 2.0 * Pi;
  double Pio2 = Pi / 2.0;
  double Pi32 = Pi * (3.0/ 2.0);
  double Htr = 0.261799387799;
      
  //  LONGITUDE IN HOURS

  double S3 = (90.0 - _lat) * Dtr;
  double Ourlon = _lon / -15.0;

  // LOCAL TRUE TIME

  time_t tnow = (time_t) now;
  int nowYear, nowMonth, nowDay, nowHour, nowMin, nowSec;
  _compute_date_time(tnow,
                     nowYear, nowMonth, nowDay,
                     nowHour, nowMin, nowSec);
  double yearStartUtime = _compute_unix_time(nowYear, 1, 1, 0, 0, 0);
  
  double jday = (now - yearStartUtime) / 86400.0;
  int Md = (int) jday + 1; // day in year
  int Myr = nowYear - 1900; // year since 1900
  double Tgmt = fmod(tnow, 86400.0) / 3600; // hours

  //   cerr << "DateTime: " << ntime.strn() << endl;
  //   cerr << "Myr, Md, Tgmt: " << Myr << ", " << Md << ", " << Tgmt << endl;
 
  double Tloc = Tgmt - Ourlon;
  if (Tloc >= 24) Tloc = Tloc - 24.0;
  if (Tloc < 0) Tloc = Tloc + 24.0;

  // TIME IN CENTURIES FROM 1900 JANUARY 0.5 OF GREENWITCH
  // MIDNIGHT OF THE DAY OF THE OBSERVATION

  int It = (int) (365.25 * (Myr - 1) + 365.0 + Md);
  double T = (It - 0.5) / 36525.0;

  // TIME OF OBSERVATION IN CENTURIES
  
  double Tp = T + Tgmt / 876600.0; // 876600 = hours per century
  
  // THE TIME DEPENDENT TERMS HAVE 2 FORMS GIVEN
  // THE LONG FORM IS THAT GIVEN BY MEEUS (USUALLY TAKEN FROM
  // THE AENA/AA)
  // THE SHORT FORM IS A LINEAR FIT AT 1985.0 (OR WAS IT 1982.0?)
  // OBLIQUITY OF THE ECLIPTIC
  
  double Obl = 23.452294 - (0.0130125 + (0.00000164 - 0.000000503 * Tp) * Tp) * Tp;
  // double Obl = 23.452294 - 0.0130135 * Tp;

  // ECCENTRICITY OF EARTH'S ORBIT - not used
  // CC ECS = 0.01675104 - 0.0000419 * TP
  // double Ecs = 0.01675104 - (0.0000418 + 0.000000126 * Tp) * Tp;

  // MEAN GECOENTRIC LONGITUDE OF THE SUN

  double Amb = (0.76892 + 0.0003025 * Tp) * Tp;
  double Ambd1 = 279.69668 + Amb + 36000.0 * Tp;
  double Ambda = fmod(Ambd1, 360.0);

  // MEAN ANNOMALLY OF THE SUN
  // CC ANOM = -0.95037 * TP
  double Anom = -(0.95025 + (0.00015 + 0.0000033 * Tp) * Tp) * Tp;
  double Ann = -1.52417 + Anom + 36000.0 * Tp;
  double Annom = fmod(Ann, 360.0);
  double Amr = Ambda * Dtr;
  double Anr = Annom * Dtr;

  // LONGITUDE OF MOON'S ASCENDING NODE
  // MAKE THE SOLUTION POSITIVE FOR THE NEXT FEW CENTURIES
  double Omeg = 2419.1833 - (1934.142 - 0.002078 * Tp) * Tp;
  Omeg = fmod(Omeg, 360.0);
  double Omega = Omeg * Dtr;

  // NUTATION TERMS
  double Sinom = 0.00479 * sin(Omega);
  double Sinm = sin(Anr);
  double S2l = sin(2.0 * Amr);
  // double C2l = cos(2.0 * Amr);
  // double Snm = sin(Anr);

  // MORE NUTATION
  Sinom = Sinom + 0.000354 * S2l;

  // EQUATION OF THE CENTER -- A SPECIALIZED FORM OF
  // KEPPLER'S EQUATION FOR THE SUN
  // CC   CENT = (-.001172 * SINM ** 2 + (1.920339 - .0048 * TP)) * SINM
  // CC 1 + 0.020012 * Sin(2.0 * ANR)
  // * CENT = (-.001172 * SINM ** 2 + (1.920339 -
  // * (4.789E-3 + 1.4E-5 * TP) * TP)) * SINM + (.020094 - 1.E-4 * TP) * SIN(2. * ANR)
  
  double Cent = ((-0.001172 * Sinm * Sinm +
                  (1.920339 - (0.004789 + 0.000014 * Tp) * Tp)) * 
                 Sinm + (0.020094 - 0.0001 * Tp) * sin(2.0 * Anr));

  // CENTER = CENT * DTR
  double Solong = Ambda + Cent;

  // SIDEREAL TIME
  double Sid0 = (0.00002581 * T + 0.051262) * T + 6.6460656 + 2400.0 * T;
  double Sid = fmod(Sid0, 24.0);
  double Sidloc = Sid + 0.002737909 * Tgmt + Tloc;
  if (Sidloc >= 24) Sidloc = Sidloc - 24;

  // THE MAJOR PLANETARY PERTURBATIONS ON THE EARTH
  double Av = 153.23 + 22518.7541 * Tp;
  double Bv = 216.57 + 45037.5082 * Tp;
  double Cj = 312.69 + 32964.3577 * Tp;

  // CC DL = 267.113 * TP
  double Dl = (267.1142 - 0.00144 * Tp) * Tp;
  Dl = (Dl + 350.74) + 445000.0 * Tp;
  double Ec = 231.19 + 20.2 * Tp;
  double Aav = 0.00134 * cos(Av * Dtr);
  double Bbv = 0.00154 * cos(Bv * Dtr);
  double Ccj = 0.002 * cos(Cj * Dtr);
  double Ddl = 0.00179 * sin(Dl * Dtr);
  double Eec = 0.00178 * sin(Ec * Dtr);
  double Corlo = Aav + Bbv + Ccj + Ddl + Eec;

  // APPARENT SOLAR LONGITUDE AND LOCAL SIDEREAL TIME
  double Solap = Solong - 0.00569 - Sinom + Corlo;
  double Sidap = Sidloc - Sinom * 0.061165;

  // IF(SIDAP .GE. 24.) SIDAP = SIDAP - 24.
  // IF(SIDAP .LT. 0.) SIDAP = SIDAP + 24.
  if (Sidap >= 24) Sidap = Sidap - 24;
  if (Sidap < 0) Sidap = Sidap + 24;

  // OBLIQUITY CORRECTED FOR NUTATION
  double Oblap = Obl + 0.00256 * cos(Omega);
  double Solr = Solap * Dtr;

  // SIDR = sidap * Dtr
  double Oblr = Oblap * Dtr;

  // DECLINATION AND RIGHT ASCENSION
  double Rdec = asin(sin(Oblr) * sin(Solr));
  double Ra = atan(cos(Oblr) * tan(Solr));
  Ra = Ra * Rtd / 15.0;
  // double Dec = Rdec * Rtd;

  if (Solr > Pio2 && Solr < Pi32) Ra = Ra + 12;
  if (Ra < 0) Ra = Ra + 24;
  if (Ra >= 24) Ra = Ra - 24;

  // HOUR ANGLE FROM SIDEREAL TIME AND RA
  double Sha = Sidap - Ra;
  if (Sha > 12.0) Sha = Sha - 24;
  if (Sha <= -12.0) Sha = Sha + 24;

  double Rsha = fabs(Sha) * Htr;
  double S1 = Pio2 - Rdec;
  double A2 = Rsha;
      
  double Tnha2 = 1.0 / tan(0.5 * A2);
  double Hds13 = 0.5 * (S1 - S3);
  double Hss13 = 0.5 * (S1 + S3);
  double Tnhda13 = Tnha2 * sin(Hds13) / sin(Hss13);
  double Tnhsa13 = Tnha2 * cos(Hds13) / cos(Hss13);
  double Hda13 = atan(Tnhda13);
  double Hsa13 = atan(Tnhsa13);

  if (Hsa13 < 0) Hsa13 = Hsa13 + Pi;
  double A1 = Hsa13 + Hda13;
  double A3 = Hsa13 - Hda13;
  
  double Sns2 = 0;
  if (fabs(Pio2 - A3) < fabs(Pio2 - A1)) {
    Sns2 = sin(A2) * sin(S3) / sin(A3);
  } else {
    Sns2 = sin(A2) * sin(S1) / sin(A1);
  }
  
  double Css2 = cos(S1) * cos(S3) + sin(S1) * sin(S3) * cos(A2);

  double S2 = 0;
  if (Sns2 > 0.71) {
    S2 = acos(Css2);
  } else {
    S2 = asin(Sns2);
    if (Css2 < 0) S2 = Pi - S2;
  }
     
  double ZEN = S2 * Rtd;
  double azi = A1;

  if (Sha > 0) azi = Pi2 - A1;
  azi = azi * Rtd;

  el = 90.0 - ZEN;
  az = azi;

  return;

}

/*************************************************************************
 *	JULIAN_DATE: Calc the Julian calendar Day Number
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

long SunPosn::_julian_date(int year, int month, int day)
{

  /*
   * correct for negative year
   */

  double yr_corr = (year > 0? 0.0: 0.75);

  if(month <=2) {
    year--;
    month += 12;
  }

  /*
   * Cope with Gregorian Calendar reform
   */

  long b = 0;
  if(year * 10000.0 + month * 100.0 + day >= 15821015.0) {
    long a = year / 100;
    b = 2 - a + a / 4;
  }
	
  return (long) ((365.25 * year - yr_corr) +
                 (long) (30.6001 * (month +1)) +
                 day + 1720994L + b);

}

/*************************************************************************
 * CALENDAR_DATE: Calc the calendar Day from the Julian date
 */

void SunPosn::_calendar_date(long jdate, int &year, int &month, int &day)
{

  long z = jdate +1;

  /*
   * Gregorian reform correction
   */

  long a = 0;
  if (z < 2299161) { 
    a = z; 
  } else {
    long alpha = (long) ((z - 1867216.25) / 36524.25);
    a = z + 1 + alpha - alpha / 4;
  }

  long b = a + 1524;
  long c = (long) ((b - 122.1) / 365.25);
  long d = (long) (365.25 * c);
  long e = (long) ((b - d) / 30.6001);
  day = (long) b - d - (long) (30.6001 * e);
  month = (long) (e < 13.5)? e - 1 : e - 13;
  year = (long) (month > 2.5)? (c - 4716) : c - 4715;
  
}

/*************************************************************************
 * compute unix time from datetime
 *
 */

time_t SunPosn::_compute_unix_time(int year, int month, int day,
                                   int hour, int min, int sec)
{
  
  long thisDay = _julian_date(year, month, day);
  long daysSince1970 = thisDay - JAN_1_1970;
  time_t u_time = (daysSince1970 * 86400) + (hour * 3600) + (min * 60) + sec;
  return u_time;

}

/*************************************************************************
 * compute datetime from unix time
 *
 */

void SunPosn::_compute_date_time(time_t unix_time,
                                 int &year, int &month, int &day,
                                 int &hour, int &min, int &sec)
{

  int time_of_day = (unix_time % 86400);
  long this_day = (unix_time / 86400);
  if (unix_time < 0 && time_of_day != 0) {
    day--;
    time_of_day += 86400;
  }
  
  _calendar_date((JAN_1_1970 + this_day), year, month, day);
  
  hour = time_of_day / 3600;
  min = (time_of_day / 60) - (hour * 60);
  sec = time_of_day % 60;

}
 
