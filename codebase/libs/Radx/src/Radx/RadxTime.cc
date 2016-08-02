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
///////////////////////////////////////////////////////////////////////////////
//
// Utility class for manipulating date/time
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1999
//
///////////////////////////////////////////////////////////////////////////////

#include <ctime>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ctype.h>
#include <sys/time.h>
#include <vector>
#include <Radx/RadxTime.hh>
using namespace std;

// days in the month
static int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// not set
const time_t RadxTime::NEVER = -1000000000L;

//////////////////////////////////
// default constructor
// Time set to 1970/01/01 00:00:00

RadxTime::RadxTime()
{
  _init();
}

//////////////////////////////////////////////
// constructor with time set according to spec

RadxTime::RadxTime(SpecifyTime_t spec)
{
  set(spec);
}

//////////////////////////////////
// construct from time_t

RadxTime::RadxTime(time_t when)
{
  _init();
  _uTime = when;
}

////////////////////////////////////
// construct from time_t and subsec

RadxTime::RadxTime(time_t when, double subSec)
{
  _init();
  _uTime = when;
  _subSec = subSec;
  _normalize();
}

/////////////////////////////////////////////////////////////////
// construct from year, month, day, hour, min, sec, partial secs
// hour, min, secs and subSec are optional args, that default
// to 0.

RadxTime::RadxTime(int year, int month, int day,
                   int hour, int min, int sec,
                   double subSec)
{
  _init();
  set(year, month, day, hour, min, sec, subSec);
}

//////////////////////////////////
// construct from string

RadxTime::RadxTime(const string &when)
{
  _init();
  if (_scanW3c(when) == 0) {
    return;
  }
  _uTime = parseDateTime(when, _subSec);
}

//////////////////////////////////
// copy constructor

RadxTime::RadxTime(const RadxTime & orig)
{
  copy(orig);
}

/////////////////////////////////////////
// copy function

void
  RadxTime::copy(const RadxTime &source)
{
  _uTime = source._uTime;
  _subSec = source._subSec;
}

//////////////////////////////////
// destructor

RadxTime::~RadxTime()
{
}

///////////////////
// initialize

void RadxTime::_init()
{
  _uTime = 0;
  _subSec = 0;
}

/////////////////////////////
/// Clear - set to Jan 1 1970

void RadxTime::clear()

{
  _init();
}

///////////////////////////////////
// set based on spec

void RadxTime::set(SpecifyTime_t spec)
{
  switch(spec) {
    case NOW:
      struct timeval tv;
      gettimeofday(&tv, NULL);
      _uTime = tv.tv_sec;
      _subSec = tv.tv_usec / 1.0e6;
      break;
    case ZERO:
    default:
      _init();
  }
}

///////////////////
// set from time_t

void
  RadxTime::set(time_t when)
{
  _uTime = when; 
}

///////////////////////////////
// set from time_t and subsec

void
  RadxTime::set(time_t when, double subSec)
{
  _uTime = when; 
  _subSec = subSec;
  _normalize();
}

/////////////////////////////////////////////////////////////////
/// Constructor from a string or char*
///
/// Set time using a string of the format:  "YYYY-MM-DD-HH-MM-SS"
/// where: "-" represents a valid delimeter [/_: ]
///                                    e.g. "1958/12/28 10:57:00"
///                                    e.g. "19581228105700"

time_t RadxTime::set(const string &strWhen)
{
  _init();
  if (_scanW3c(strWhen) == 0) {
    return _uTime;
  }
  _uTime = parseDateTime(strWhen, _subSec);
  return _uTime;
}

////////////////////////////////////////////////////////////////
// set the date/time
// hour, min, sec and subSec default to 0 if not specified.
// returns the unix time

time_t RadxTime::set(int year, int month, int day,
                     int hour, int min, int sec,
                     double subSec)
{
  date_time_t when;
  when.year = year;
  when.month = month;
  when.day = day;
  when.hour = hour;
  when.min = min;
  when.sec = sec;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  _subSec = subSec;
  _normalize();
  return _uTime;
}

////////////////////////////////////////////////////////////////
// modify the time
// hour, min, sec and subSec default to 0 if not specified.
// returns the unix time

time_t RadxTime::setTime(int hour, int min, int sec, double subSec)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  when.hour = hour;
  when.min = min;
  when.sec = sec;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  _subSec = subSec;
  _normalize();
  return _uTime;
}

//////////////////////////////
// modify the year
// returns the unix time

time_t RadxTime::setYear(int year)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  when.year = year;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  return _uTime;
}

//////////////////////////////
// modify the month
// returns the unix time

time_t RadxTime::setMonth(int month)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  when.month = month;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  return _uTime;
}

//////////////////////////////
// modify the day
// returns the unix time

time_t RadxTime::setDay(int day)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  when.day = day;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  return _uTime;
}

//////////////////////////////
// modify the hour
// returns the unix time

time_t RadxTime::setHour(int hour)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  when.hour = hour;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  return _uTime;
}

//////////////////////////////
// modify the minute
// returns the unix time

time_t RadxTime::setMin(int min)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  when.min = min;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  return _uTime;
}

//////////////////////////////
// modify the second
// returns the unix time

time_t RadxTime::setSec(int sec)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  when.sec = sec;
  uconvert_to_utime(when);
  _uTime = when.unix_time;
  return _uTime;
}

//////////////////////////////
// set the sub secs

void RadxTime::setSubSec(double val)
{
  _subSec = val;
  _normalize();
}

//////////////////////////////
// modify the month
// returns the unix time

time_t RadxTime::setByDayOfYear(int year, int dofy,
                                int hour, int min, 
                                int sec)
{
 
  int month;
  int day;
  // get month and day from day of the year
  getMonthDay(year, dofy, &month, &day);
  return (set(year, month, day, hour, min, sec));
}

//////////////////////////////////////////////////////////////////
/// Interpret a time string of the format: "YYYY-MM-DD-HH-MM-SS"
/// where: "-" represents a valid delimeter [/_: ]
///                  e.g. "1958/12/28 10:57:00"
///                  e.g. "19581228105700"
///
/// Optionally, return the component integral values of the string
///
/// Returns RadxTime::NEVER if the string did not contain a 
///      parseable year, month, and day.
///
/// NOTE: Does not set the time on this class -- only parses!
///    Use the set() method if you want the time to be changed.
///
/// Static

time_t RadxTime::parseDateTime(const string &strWhen,
                               int *year, int *month, int *day,
                               int *hour, int *min, int *sec,
                               double *subSec)

{

  int YYYY, MM, DD, hh, mm, ss;
  double ssec;
  date_time_t when;

  // Tokenize the dateTime string

  tokenizeString(strWhen, YYYY, MM, DD, hh, mm, ss, ssec);

  // Now, do some validity checking

  if (YYYY <= 0) {
    return NEVER;
  }
  if (MM <= 0 || MM > 12) {
    return NEVER;
  }
  if (DD <= 0 || DD > 31) {
    return NEVER;
  }
  if (hh < 0 || hh > 23) {
    return NEVER;
  }
  if (mm < 0 || mm > 59) {
    return NEVER;
  }
  if (ss < 0 || ss > 59) {
    return NEVER;
  }

  //
  // Save the time components, if requested
  //
  if (year)
    *year = YYYY;
  if (month)
    *month = MM;
  if (day)
    *day = DD;
  if (hour)
    *hour = hh;
  if (min)
    *min = mm;
  if (sec)
    *sec = ss;
  if (subSec)
    *subSec = ssec;

  //
  // Return the resulting unix time
  //
  when.year = YYYY;
  when.month = MM;
  when.day  = DD;
  when.hour = hh;
  when.min  = mm;
  when.sec  = ss;

  return(uunix_time(when));

}

time_t RadxTime::parseDateTime(const string &strWhen,
                               double &subSec)
  
{
  int year, month, day, hour, min, sec;
  return parseDateTime(strWhen,
                       &year, &month, &day, &hour, &min, &sec,
                       &subSec);
}

////////////////////////////////////////
// get month and day, given day of year

void RadxTime::getMonthDay(const int year, const int dofy, 
                           int &month, int &day)
{
  getMonthDay(year, dofy, &month, &day);
}

void RadxTime::getMonthDay(const int year, const int dofy, 
                           int *month, int *day)
{

  // create an object, using january and the day of year as the day of month
  // this will be correctly converted into year, month, day

  RadxTime tmp(year, 1, dofy);
  *month = tmp.getMonth();
  *day = tmp.getDay();
  
}

//////////////////////////////////////////////////////////////
/// get as string
/// date is / delimited
/// specify the number of digits of precision in the sub-seconds

string RadxTime::asString(int subsecPrecision /* = 0*/) const
{

  char text[1024];
  RadxTime mtime(_uTime);
  if (subsecPrecision == 0) {
    sprintf(text, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
            mtime.getYear(), mtime.getMonth(), mtime.getDay(),
            mtime.getHour(), mtime.getMin(), mtime.getSec());
  } else { 
    if (subsecPrecision > 12) {
      subsecPrecision = 12;
    } else if (subsecPrecision < 0) {
      subsecPrecision = 3;
    }
    double power = pow(10.0, subsecPrecision);
    int usecs = (int) (_subSec * power + 0.5);
    char format[128];
    sprintf(format, "%%.4d/%%.2d/%%.2d %%.2d:%%.2d:%%.2d.%%.%dd",
            subsecPrecision);
    sprintf(text, format,
            mtime.getYear(), mtime.getMonth(), mtime.getDay(),
            mtime.getHour(), mtime.getMin(), mtime.getSec(), usecs);
  }
  return text;
}

//////////////////////////////////////////////////////////////
/// get as string
/// date is - delimited
/// specify the number of digits of precision in the sub-seconds

string RadxTime::asStringDashed(int subsecPrecision /* = 0*/) const
{

  char text[1024];
  RadxTime mtime(_uTime);
  if (subsecPrecision == 0) {
    sprintf(text, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
            mtime.getYear(), mtime.getMonth(), mtime.getDay(),
            mtime.getHour(), mtime.getMin(), mtime.getSec());
  } else { 
    if (subsecPrecision > 12) {
      subsecPrecision = 12;
    } else if (subsecPrecision < 0) {
      subsecPrecision = 3;
    }
    double power = pow(10.0, subsecPrecision);
    int usecs = (int) (_subSec * power + 0.5);
    char format[128];
    sprintf(format, "%%.4d-%%.2d-%%.2d %%.2d:%%.2d:%%.2d.%%.%dd",
            subsecPrecision);
    sprintf(text, format,
            mtime.getYear(), mtime.getMonth(), mtime.getDay(),
            mtime.getHour(), mtime.getMin(), mtime.getSec(), usecs);
  }
  return text;
}

//////////////////////////////////////
// get current time as double secs

double RadxTime::getCurrentTimeAsDouble()
  
{
  struct timeval tval;
  gettimeofday(&tval, NULL);
  return (tval.tv_sec + tval.tv_usec / 1000000.0);
}


//////////////////////////////////////
// get current time in W3C format

string RadxTime::getW3cStr() const
{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[32];
  sprintf(str, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          mtime.year, mtime.month, mtime.day,
          mtime.hour, mtime.min, mtime.sec);
 
  return str;
}

//
// String formatting
//
const string RadxTime::ctime() const
{
  //
  // stdC library string 
  // returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  //           e.g. "Wed Jun 30 21:49:08 1993\n"
  //
  return ::ctime(&_uTime);
}

const string RadxTime::gtime() const
{
  //
  // stdC library string - returns time as UTC
  // returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  //           e.g. "Wed Jun 30 21:49:08 1993\n"
  //
  return ::asctime(::gmtime(&_uTime));
}

const string RadxTime::dtime() const
{
  //
  // didss string
  // Returns string of format: "YYYY/MM/DD HH:MM:SS"
  //           e.g. "1958/12/28 10:57:00"
  //
  char whenChar[20];
  
  if (_uTime > 0) {
    date_time_t when = udate_time(_uTime);
    sprintf(whenChar, "%4d/%02d/%02d %02d:%02d:%02d",
            when.year, when.month, when.day,
            when.hour, when.min, when.sec);
  } else {
    strcpy(whenChar, "0000/00/00 00:00:00");
  }

  return whenChar;
}

const string RadxTime::kmltime() const
{
  //
  // KML string
  // Returns string of format: "yyyy-mm-ddThh:mm:ss"
  //           e.g. "1958-12-28T10:57:00"
  //
  char     whenChar[20];

  if (_uTime > 0) {
    date_time_t when = udate_time(_uTime);
    sprintf(whenChar, "%4d-%02d-%02dT%02d:%02d:%02d",
            when.year, when.month, when.day,
            when.hour, when.min, when.sec);
  } else {
    strcpy(whenChar, "0000-00-00T00:00:00");
  }

  return whenChar;
}

// Returns string representing date and time
// of format: "YYYY/MM/DD HH:MM:SS"
//    e.g. "1958/12/28 10:57:00"
// Optionally UTC is added to the end of string.
// If subsecs is non-zero, format will be:
//            "YYYY/MM/DD HH:MM:SS.uuuuuu"

string RadxTime::getStr(bool utc_label) const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[128];
  int usecs = (int) (_subSec * 1.0e6 + 0.5);
  if (utc_label) {
    if (_subSec != 0) {
      sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.6d UTC",
              mtime.year, mtime.month, mtime.day,
              mtime.hour, mtime.min, mtime.sec, usecs);
    } else {
      sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d UTC",
              mtime.year, mtime.month, mtime.day,
              mtime.hour, mtime.min, mtime.sec);
    }
  } else {
    if (_subSec != 0) {
      sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.6d",
              mtime.year, mtime.month, mtime.day,
              mtime.hour, mtime.min, mtime.sec, usecs);
    } else {
      sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
              mtime.year, mtime.month, mtime.day,
              mtime.hour, mtime.min, mtime.sec);
    }
  }
  return str;
}

// Returns plain string representing date and time
// of format: "YYYYMMDDHHMMSS"
//    e.g. "19581228105700"

string RadxTime::getStrPlain() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[32];
  sprintf(str, "%.4d%.2d%.2d%.2d%.2d%.2d",
          mtime.year, mtime.month, mtime.day,
          mtime.hour, mtime.min, mtime.sec);
  return str;
}

// Return string representing date only
// of format: "YYYY/MM/DD"
//    e.g. "1958/12/28"
string RadxTime::getDateStr() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[32];
  sprintf(str, "%.4d/%.2d/%.2d",
          mtime.year, mtime.month, mtime.day);
  return str;
}


// Return plain string representing date only
// of format: "YYYYMMDD"
//    e.g. "19581228"
string RadxTime::getDateStrPlain() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[32];
  sprintf(str, "%.4d%.2d%.2d",
          mtime.year, mtime.month, mtime.day);
  return str;
}

// Return string representing date only
// of format: "MM/DD/YYYY"
//    e.g. "12/28/1958"
string RadxTime::getDateStrMDY() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[32];
  sprintf(str, "%.2d/%.2d/%.4d",
          mtime.month, mtime.day, mtime.year);
  return str;
}

// Return string representing time only
// of format: "HH:MM:SS"
//    e.g. "10:57:00"
// Optionally UTC is added to the end of string.

string RadxTime::getTimeStr(bool utc_label) const

{

  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[32];
  if (utc_label) {
    sprintf(str, "%.2d:%.2d:%.2d UTC",
            mtime.hour, mtime.min, mtime.sec);
  } else {
    sprintf(str, "%.2d:%.2d:%.2d",
            mtime.hour, mtime.min, mtime.sec);
  }
  return str;
}

// Return plain string representing time only
// of format: "HHMMSS"
//    e.g. "105700"

string RadxTime::getTimeStrPlain() const

{

  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(mtime);
  char str[32];
  sprintf(str, "%.2d%.2d%.2d",
          mtime.hour, mtime.min, mtime.sec);
  return str;
}

// Given a time, returns string
// of format: "YYYY/MM/DD HH:MM:SS"
//    e.g. "1958/12/28 10:57:00"
// If no time given, current time is used.
// Optionally UTC is added to the end of string.
// This is static on the class, and is thread-safe.

string RadxTime::str(const time_t mytime, const bool utc_label)

{
  date_time_t mtime;
  if (mytime == 0) {
    mtime.unix_time = time(NULL);
  } else {
    mtime.unix_time = mytime;
  }
  uconvert_from_utime(mtime);
  char str[128];
  if (utc_label) {
    sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d UTC",
            mtime.year, mtime.month, mtime.day,
            mtime.hour, mtime.min, mtime.sec);
  } else {
    sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
            mtime.year, mtime.month, mtime.day,
            mtime.hour, mtime.min, mtime.sec);
  }
  return str;
}

// Given a time, returns string.
// of format: "YYYY/MM/DD HH:MM:SS"
//    e.g. "1958/12/28 10:57:00"
// If mytime == 0, returns "===== NOT SET ====="


string RadxTime::strm(const time_t mytime)

{

  if (mytime == 0) {
    return "===== NOT SET =====";
  }

  date_time_t mtime;
  mtime.unix_time = mytime;
  uconvert_from_utime(mtime);
  char str[32];
  sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
          mtime.year, mtime.month, mtime.day,
          mtime.hour, mtime.min, mtime.sec);
  return str;

}

int RadxTime::getYear() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(dtime);
  return dtime.year;
}

int RadxTime::getMonth() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(dtime);
  return dtime.month;
}

int RadxTime::getDay() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(dtime);
  return dtime.day;
}

int RadxTime::getHour() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(dtime);
  return dtime.hour;
}

int RadxTime::getMin() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(dtime);
  return dtime.min;
}

int RadxTime::getSec() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(dtime);
  return dtime.sec;
}

double RadxTime::getSubSec() const
{
  return _subSec;
}

int RadxTime::getDayOfYear() const
{

  date_time_t start;
  start.unix_time = _uTime;
  uconvert_from_utime(start);
  //
  // Retain the year but go to the start of Jan 1
  //
  start.day = 1;
  start.month = 1;
  start.hour = 0;
  start.min = 0;
  start.min = 0;
  uconvert_to_utime(start);

  int delta = _uTime - start.unix_time;
  int doy = 1 + delta/RADX_SECS_IN_DAY;

  return doy;
}

int RadxTime::getDaysInMonth() const
{
  return daysOfMonth[getMonth()-1];
}

// fill out any args supplied

void RadxTime::getAll(int *year,
                      int *month,
                      int *day,
                      int *hour,
                      int *min,
                      int *sec) const
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  if (year) *year = when.year;
  if (month) *month = when.month;
  if (day) *day = when.day;
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

void RadxTime::getAll(short *year,
                      short *month,
                      short *day,
                      short *hour,
                      short *min,
                      short *sec) const
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  if (year) *year = when.year;
  if (month) *month = when.month;
  if (day) *day = when.day;
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

void RadxTime::getTime(int *hour,
                       int *min,
                       int *sec) const
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(when);
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

RadxTime RadxTime::operator+ (double secs) const
{
  RadxTime answer(*this);
  answer += secs;
  return answer;
}
 
RadxTime RadxTime::operator- (double secs) const
{
  RadxTime answer(*this);
  answer -= secs;
  return answer;
}
 
RadxTime& RadxTime::operator+= (double secs)
{
  _subSec += secs;
  _normalize();
  return *this;
}
 
RadxTime& RadxTime::operator-= (double secs)
{
  _subSec -= secs;
  _normalize();
  return *this;
}
 
double RadxTime::operator- (const RadxTime& other) const
{
  long int idiff = _uTime - other._uTime;
  double fdiff = _subSec - other._subSec;
  double diff = fdiff + (double) idiff;
  return diff;
}

/////////////////////////////////////
// comparison operators using time_t

bool RadxTime::operator< (time_t when) const
{
  return(_uTime < when ? true : false); 
}

bool RadxTime::operator<= (time_t when) const
{
  return(_uTime <= when ? true : false); 
}

bool RadxTime::operator== (time_t when) const
{
  return(_uTime == when ? true : false); 
}

bool RadxTime::operator!= (time_t when) const
{
  return(_uTime != when ? true : false); 
}

bool RadxTime::operator> (time_t when) const
{
  return(_uTime > when ? true : false); 
}

bool RadxTime::operator>= (time_t when) const
{
  return(_uTime >= when ? true : false); 
}

/////////////////////////////////////
// comparison operators using double

bool RadxTime::operator< (double when) const
{
  return(asDouble() < when ? true : false); 
}

bool RadxTime::operator<= (double when) const
{
  return(asDouble() <= when ? true : false); 
}

bool RadxTime::operator== (double when) const
{
  return(asDouble() == when ? true : false); 
}

bool RadxTime::operator!= (double when) const
{
  return(asDouble() != when ? true : false); 
}

bool RadxTime::operator> (double when) const
{
  return(asDouble() > when ? true : false); 
}

bool RadxTime::operator>= (double when) const
{
  return(asDouble() >= when ? true : false); 
}

/////////////////////////////////////
// comparison operators using RadxTime

bool RadxTime::operator< (const RadxTime &other) const
{
  return(asDouble() < other.asDouble() ? true : false); 
}

bool RadxTime::operator<= (const RadxTime &other) const
{
  return(asDouble() <= other.asDouble() ? true : false); 
}

bool RadxTime::operator== (const RadxTime &other) const
{
  return(asDouble() == other.asDouble() ? true : false); 
}

bool RadxTime::operator!= (const RadxTime &other) const
{
  return(asDouble() != other.asDouble() ? true : false); 
}

bool RadxTime::operator> (const RadxTime &other) const
{
  return(asDouble() > other.asDouble() ? true : false); 
}

bool RadxTime::operator>= (const RadxTime &other) const
{
  return(asDouble() >=  other.asDouble() ? true : false); 
}

////////////////////////////////////////////////////////
// scan the input string for conformance to W3C ISO 8601
// See http://www.w3.org/TR/NOTE-datetime
// returns 0 on success, -1 on failure

int RadxTime::_scanW3c(const string &strWhen)
  
{
  
  // Degenerate case

  if (strWhen.size() < 4) {
    return -1;
  }

  // tokenize the string into digits and non-digits

  bool inDigit = false;
  if (isdigit(strWhen[0])) {
    inDigit = true;
  }

  vector<string> digits;
  vector<string> nonDigits;

  string digit;
  string nonDigit;
  int nNonDigits = 0;
  bool gotDecimal = false;
  char timeZoneSign = '*';
  // bool zuluTime = false;
  size_t timeZoneIndex = 0;

  for (size_t ii = 0; ii < strWhen.size(); ii++) {
    char cc = strWhen[ii];
    if (isdigit(cc)) {
      if (!inDigit) {
        nonDigits.push_back(nonDigit);
        nonDigit.clear();
        inDigit = true;
      }
      digit += cc;
    } else {
      if (inDigit) {
        digits.push_back(digit);
        digit.clear();
        inDigit = false;
      }
      nonDigit += cc;
      nNonDigits++;
      if (cc == '.' && digits.size() == 6) {
        gotDecimal = true;
      }
      if (digits.size() >= 6) {
        if (cc == '+') {
          timeZoneSign = cc;
          timeZoneIndex = digits.size();
        } else if (cc == '-') {
          timeZoneSign = cc;
          timeZoneIndex = digits.size();
        } else if (cc == 'Z') {
          // zuluTime = true;
        }
      }
    }
  }
  if (nonDigits.size() > 0) {
    nonDigits.push_back(nonDigit);
  }
  if (digits.size() > 0) {
    digits.push_back(digit);
  }

  // check we have some non-digits if digits size > 4
  if (nNonDigits == 0 && strWhen.size() > 4) {
    // no non-digits, cannot be w3c
    return -1;
  }

  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;
  double subSec = 0.0;
  int thour = 0;
  int tmin = 0;

  if (digits.size() > 0) {
    year = atoi(digits[0].c_str());
  }
  if (digits.size() > 1) {
    month = atoi(digits[1].c_str());
  }
  if (digits.size() > 2) {
    day = atoi(digits[2].c_str());
  }
  if (digits.size() > 3) {
    hour = atoi(digits[3].c_str());
  }
  if (digits.size() > 4) {
    min = atoi(digits[4].c_str());
  }
  if (digits.size() > 5) {
    sec = atoi(digits[5].c_str());
  }
  if (digits.size() > 6 && gotDecimal) {
    string decStr = "0.";
    decStr += digits[6];
    subSec = atof(decStr.c_str());
  }
  if (timeZoneSign != '*') {
    if (digits.size() > timeZoneIndex) {
      thour = atoi(digits[timeZoneIndex].c_str());
    }
    if (digits.size() > timeZoneIndex + 1) {
      tmin = atoi(digits[timeZoneIndex + 1].c_str());
    }
  }

  set(year, month, day, hour, min, sec, subSec);
  int tdiff = thour * 3600 + tmin * 60;
  if (timeZoneSign == '-') {
    tdiff *= -1;
  }
  _uTime -= tdiff;

  return 0;

}

////////////////////////////////////////////////// 
// tokenize string for setting time from string

void RadxTime::tokenizeString(const string strWhen,
                              int &YYYY, int &MM, int &DD,
                              int &hh, int &mm, int &ss,
                              double &subSec)
 
{

  // Initialize the resulting values

  YYYY = MM = DD = hh = mm = ss = 0;
  subSec = 0.0;

  // Degenerate case

  if (strWhen.size() == 0)
    return;

  const char *sPtr = strWhen.c_str();
  
  // skip over any leading non-digits

  while (*sPtr != '\0' && !isdigit(*sPtr)) {
    sPtr++;
  }
  if (sPtr == NULL) {
    return;
  }

  // Year
  
  if (sscanf(sPtr, "%4d", &YYYY) != 1) {
    return;
  }

  // Month
  
  sPtr += 4;
  if (!isdigit(sPtr[0])) {
    sPtr++;
  }
  if (sscanf(sPtr, "%2d", &MM) != 1) {
    if (sscanf(sPtr, "%1d", &MM) != 1) {
      return;
    }
  }

  // Day
  
  sPtr += 2;
  if (!isdigit(sPtr[0])) {
    sPtr++;
  }
  if (sscanf(sPtr, "%2d", &DD) != 1) {
    if (sscanf(sPtr, "%1d", &DD) != 1) {
      return;
    }
  }

  // Hour
  
  sPtr += 2;
  if (!isdigit(sPtr[0])) {
    sPtr++;
  }
  if (sscanf(sPtr, "%2d", &hh) != 1) {
    if (sscanf(sPtr, "%1d", &hh) != 1) {
      return;
    }
  }

  // Minute

  sPtr += 2;
  if (!isdigit(sPtr[0])) {
    sPtr++;
  }
  if (sscanf(sPtr, "%2d", &mm) != 1) {
    if (sscanf(sPtr, "%1d", &mm) != 1) {
      return;
    }
  }

  // Second
  
  sPtr += 2;
  if (!isdigit(sPtr[0])) {
    sPtr++;
  }

  if (strchr(sPtr, '.') != NULL) {
    double secs;
    if (sscanf(sPtr, "%lg", &secs) == 1) {
      ss = (int) secs;
      subSec = secs - ss;
      return;
    }
  }

  if (sscanf(sPtr, "%2d", &ss) != 1) {
    if (sscanf(sPtr, "%1d", &ss) != 1) {
      return;
    }
  }

}

///////////////////////
// printing to ostream

ostream& operator<< (ostream &os, const RadxTime &d)
{
  char buf[128];
  RadxTime::date_time_t dtime;
  dtime.unix_time = d.utime();
  RadxTime::uconvert_from_utime(dtime);
  if (d._subSec != 0) {
    int usecs = (int) (d._subSec * 1.0e6 + 0.5);
    sprintf(buf, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.6d",
            dtime.year, dtime.month, dtime.day,
            dtime.hour, dtime.min, dtime.sec, usecs);
  } else {
    sprintf(buf, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
            dtime.year, dtime.month, dtime.day,
            dtime.hour, dtime.min, dtime.sec);
  }
  string str(buf);
  return os << str;
}

//////////////////////
// normalize subSec

void RadxTime::_normalize()
{
  double intPart;
  double fracPart = modf(_subSec, &intPart);
  if (fracPart < 0) {
    fracPart += 1.0;
    intPart -= 1.0;
  }
  _subSec = fracPart;
  _uTime += (int) intPart;
}

/*************************************************************************
 * uunix_time()
 *
 * return the unix time from date_time_t struct
 *
 * Side-effect: sets the unix_time field in date_time.
 */

time_t RadxTime::uunix_time(RadxTime::date_time_t &date_time)
{

  long day, days;
  time_t u_time;

  day = RadxTime::ujulian_date(date_time.day,
                               date_time.month,
                               date_time.year);
 
  days = day - RadxTime::RADX_JAN_1_1970;

  u_time = (days * 86400) + (date_time.hour * 3600) +
    (date_time.min * 60) + date_time.sec;

  date_time.unix_time = u_time;

  return u_time;

}

/*************************************************************************
 * udate_time()
 *
 * return date_time struct corresponding to unix time.
 *
 */

RadxTime::date_time_t RadxTime::udate_time(time_t unix_time)
{

  RadxTime::date_time_t date_time;
  
  date_time.unix_time = unix_time;
  RadxTime::uconvert_from_utime(date_time);

  return date_time;

}
 
/*************************************************************************
 * uconvert_to_utime()
 *
 * return the unix time from date_time_t struct. Also sets the unix_time
 * field in the date_time_t structure.
 */

time_t RadxTime::uconvert_to_utime(RadxTime::date_time_t &date_time)
{

  time_t u_time;
  u_time = RadxTime::uunix_time(date_time);
  date_time.unix_time = u_time;
  return u_time;

}

/*************************************************************************
 * uconvert_from_utime()
 *
 * sets the other fields in the date_time_t structure based on the value
 * of the unix_time field in that structure
 */

void RadxTime::uconvert_from_utime(RadxTime::date_time_t &date_time)
{

  time_t unix_time;
  long day;
  int time_of_day;
 
  unix_time = date_time.unix_time;

  time_of_day = (unix_time % 86400);
  day = (unix_time / 86400);
  if (unix_time < 0 && time_of_day != 0) {
    day--;
    time_of_day += 86400;
  }
 
  RadxTime::ucalendar_date((RADX_JAN_1_1970 + day),
                           &date_time.day,
                           &date_time.month,
                           &date_time.year);
 
  date_time.hour = time_of_day / 3600;
  date_time.min = (time_of_day / 60) - (date_time.hour * 60);
  date_time.sec = time_of_day % 60;

}
 
/*************************************************************************
 *	JULIAN_DATE: Calc the Julian calendar Day Number
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

long RadxTime::ujulian_date(int day, int month, int year)
{

  long a, b;
  double yr_corr;

  /*
   * correct for negative year
   */

  yr_corr = (year > 0? 0.0: 0.75);

  if(month <=2) {
    year--;
    month += 12;
  }

  b=0;

  /*
   * Cope with Gregorian Calendar reform
   */

  if(year * 10000.0 + month * 100.0 + day >= 15821015.0) {
    a = year / 100;
    b = 2 - a + a / 4;
  }
	
  return (long) ((365.25 * year - yr_corr) +
                 (long) (30.6001 * (month +1)) +
                 day + 1720994L + b);

}

/*************************************************************************
 *	CALENDAR_DATE: Calc the calendar Day from the Julian date
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

void RadxTime::ucalendar_date(long jdate, int *day,
                              int *month, int *year)
{

  long a, b, c, d, e, z, alpha;

  z = jdate +1;

  /*
   * Gregorian reform correction
   */

  if (z < 2299161) { 
    a = z; 
  } else {
    alpha = (long) ((z - 1867216.25) / 36524.25);
    a = z + 1 + alpha - alpha / 4;
  }

  b = a + 1524;
  c = (long) ((b - 122.1) / 365.25);
  d = (long) (365.25 * c);
  e = (long) ((b - d) / 30.6001);
  *day = (long) b - d - (long) (30.6001 * e);
  *month = (long) (e < 13.5)? e - 1 : e - 13;
  *year = (long) (*month > 2.5)? (c - 4716) : c - 4715;

}

/**************************************************************************
 * void ulocaltime()
 *
 * puts the local time into a date_time_t struct
 *
 **************************************************************************/

void RadxTime::ulocaltime(RadxTime::date_time_t &date_time)
{

  struct tm *local_time;
  time_t now;

  time(&now);
  local_time = localtime(&now);

  date_time.year = local_time->tm_year + 1900;
  date_time.month = local_time->tm_mon + 1;
  date_time.day = local_time->tm_mday;
  date_time.hour = local_time->tm_hour;
  date_time.min = local_time->tm_min;
  date_time.sec = local_time->tm_sec;
  RadxTime::uconvert_to_utime(date_time);

}

/**************************************************************************
 * void ugmtime()
 *
 * puts the gm time into a date_time_t struct
 *
 **************************************************************************/

void RadxTime::ugmtime(RadxTime::date_time_t &date_time)
{

  date_time.unix_time = time(NULL);
  RadxTime::uconvert_from_utime(date_time);

}

#ifdef JUNK

/*************************************************************************
 * utime_compute()
 *
 * return the unix time from components.
 */

time_t RadxTime::utime_compute(int year, int month, int day,
                               int hour, int min, int sec)
{

  RadxTime::date_time_t dtime;

  dtime.year = year;
  dtime.month = month;
  dtime.day = day;
  dtime.hour = hour;
  dtime.min = min;
  dtime.sec = sec;

  RadxTime::uconvert_to_utime(dtime);
 
  return(dtime.unix_time);

}

/**************************************************************************
 * utimstr()
 *
 * Calls utimestr() - see notes for this function.
 *
 **************************************************************************/

string RadxTime::utimstr(time_t time)
{
  return (RadxTime::utimestr(RadxTime::udate_time(time)));
}

/**************************************************************************
 * utimestr()
 *
 * returns a string composed from the time struct.
 *
 **************************************************************************/

string RadxTime::utimestr(const RadxTime::date_time_t &date_time)
{

  char timestr[128];

  sprintf(timestr, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
          date_time.year,
          date_time.month,
          date_time.day,
          date_time.hour,
          date_time.min,
          date_time.sec);
  
  return timestr;

}

/**************************************************************************
 * string utim_str()
 *
 * Calls utime_str() - see notes for this function.
 *
 **************************************************************************/

string RadxTime::utim_str (time_t time)
{
  return RadxTime::utime_str(RadxTime::udate_time(time));
}

/**************************************************************************
 * utime_str()
 *
 * Same as utimestr(), except puts an underscore between the date and
 * time.
 *
 **************************************************************************/

string RadxTime::utime_str(const RadxTime::date_time_t &date_time)
{

  char time_str[128];
  sprintf(time_str, "%.4d/%.2d/%.2d_%.2d:%.2d:%.2d",
          date_time.year,
          date_time.month,
          date_time.day,
          date_time.hour,
          date_time.min,
          date_time.sec);

  return time_str;

}

#endif

