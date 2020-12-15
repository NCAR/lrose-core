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
#include <cstdio>
#include <toolsa/toolsa_macros.h>
#include <toolsa/str.h>
#include <ctype.h>
#include <toolsa/DateTime.hh>
#include <vector>
#include <cstdlib>
using namespace std;

const time_t DateTime::NEVER = -LARGE_LONG;

/* days in the month */
static int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

DateTime::DateTime()
{
  _init();
}

DateTime::DateTime(time_t when)
{
  _init();
  _uTime = when;
}

DateTime::DateTime(time_t when, bool setSubSec, double subSec)
{
  _init();
  _uTime = when;
  if (setSubSec) {
    _subSec = subSec;
  }
  _normalize();
}

DateTime::DateTime(time_t when, size_t leadSecs)
{
  _init();
  _uTime = when;
  setLeadSecs(leadSecs);
}

DateTime::DateTime(const char *when)
{
  _init(when);
}

DateTime::DateTime(const string &when)
{
  _init(when);
}

DateTime::DateTime(int year, int month, int day,
                   int hour, int min, int sec,
                   double subSec)
{
  set(year, month, day, hour, min, sec, subSec);
  _leadTime = NULL;
}

DateTime::DateTime(const DateTime & orig)
{
  _leadTime = NULL;
  copy (orig);
}

DateTime::~DateTime()
{
  if (_leadTime != NULL) {
    delete _leadTime;
    _leadTime = NULL;
  }
}

///////////////////
// initialize

void DateTime::_init()
{
  _subSec = 0.0;
  _uTime = 0;
  _leadTime = NULL;
}

void DateTime::_init(const string &when)
{
  _subSec = 0.0;
  int year, month, day, hour, min, sec;
  double subSec;
  _uTime = parseDateTime(when.c_str(), &year, &month, &day, &hour, &min, &sec, &subSec);
  _subSec = subSec;
  _leadTime = NULL;
}

// Interpret a time string of the format: "YYYY-MM-DD-HH-MM-SS"
// where: "-" represents a valid delimeter [/_: ]
// e.g. "1958/12/28 10:57:00"
// e.g. "19581228105700"
//
// Optionally, return the component integral values of the string
//
// Returns DateTime::NEVER if the string did not contain a 
// parseable year, month, and day.
//
// NOTE: Does not set the time on this class -- only parses!
// Use the set() method if you want the time to be changed.
// NOTE: Static

time_t
  DateTime::parseDateTime(const char *strWhen,
                          int *year, int *month, int *day,
                          int *hour, int *min, int *sec,
                          double *subSec)
{
  int YYYY, MM, DD, hh, mm, ss;
  double sss;
  date_time_t when;

  //
  // Tokenize the dateTime string
  //
  tokenizeString(strWhen, YYYY, MM, DD, hh, mm, ss, sss);

  //
  // Now, do some validity checking
  //
  if(YYYY <= 0) {
    return NEVER;
  }
  if(MM <= 0 || MM > 12) {
    return NEVER;
  }
  if(DD <= 0 || DD > 31) {
    return NEVER;
  }
  if(hh < 0 || hh > 23) {
    return NEVER;
  }
  if(mm < 0 || mm > 59) {
    return NEVER;
  }
  if(ss < 0 || ss > 59) {
    return NEVER;
  }

  //
  // Save the time components, if requested
  //
  if(year)
    *year = YYYY;
  if(month)
    *month = MM;
  if(day)
    *day = DD;
  if(hour)
    *hour = hh;
  if(min)
    *min = mm;
  if(sec)
    *sec = ss;
  if(subSec)
    *subSec = sss;

  //
  // Return the resulting unix time
  //
  when.year = YYYY;
  when.month = MM;
  when.day = DD;
  when.hour = hh;
  when.min = mm;
  when.sec = ss;

  return(uunix_time(&when));
}

void DateTime::getMonthDay(const int year, const int dofy, 
                           int &month, int &day)
{
  getMonthDay(year, dofy, &month, &day);
}

void DateTime::getMonthDay(const int year, const int dofy, 
                           int *month, int *day)
{

  /* leap year adjustment */
  if ((year % 4) == 0) {
    if ((year % 100) == 0) {
      if ((year % 400) == 0) {
	daysOfMonth[1] = 29;
      }
      else {
	daysOfMonth[1] = 28;
      }
    }
    else {
      daysOfMonth[1] = 29;
    }
  }
	
  int m = 1;
  int d = dofy;
  while (d > daysOfMonth[m-1]) {
    d -= daysOfMonth[m-1];
    m++;
  }
  *day = d;
  *month = m;

  daysOfMonth[1] = 28;

}

//////////////////////////////////////
// get stored time as double secs

double DateTime::getTimeAsDouble() const
  
{
  return (double) _uTime + _subSec;
}

double DateTime::getCurrentTimeAsDouble()

{
  struct timeval tval;
  gettimeofday(&tval, NULL);
  return (tval.tv_sec + tval.tv_usec / 1000000.0);
}


void
  DateTime::copy(const DateTime &source)
{
  _uTime = source._uTime;
  _subSec = source._subSec;
  setLeadDeltaTime(source.getLeadDeltaTime());
}

void
  DateTime::setSubSec(double val)
{
  _subSec = val;
  _normalize();
}

void
  DateTime::setLeadSecs(size_t leadSecs)
{
  DeltaTime tempDelta(leadSecs);
  setLeadDeltaTime(&tempDelta);
}

void
  DateTime::setLeadDeltaTime(const DeltaTime * delta)
{
  if (_leadTime != NULL) {
    delete _leadTime;
    _leadTime = NULL;
  }

  if (delta != NULL) {
    _leadTime = new DeltaTime(*delta);
  }
}

void
  DateTime::setToNow()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  _uTime = tv.tv_sec;
  _subSec = tv.tv_usec / 1.0e6;
}

void
  DateTime::set(time_t when)
{
  _uTime = when; 
  setLeadDeltaTime(NULL);
}

void
  DateTime::set(time_t when, double subSec)
{
  _uTime = when; 
  _subSec = subSec;
  _normalize();
  setLeadDeltaTime(NULL);
}

void
  DateTime::set(time_t when, size_t leadSecs)
{
  _uTime = when;
  setLeadSecs(leadSecs);
}

time_t
  DateTime::set(const char *strWhen)
{
  //
  // Set time using a string of the format: "YYYY-MM-DD-HH-MM-SS"
  // where: "-" represents a valid delimeter [/_: ]
  // e.g. "1958/12/28 10:57:00"
  // e.g. "19581228105700"
  //
  _uTime = parseDateTime(strWhen);
  return(_uTime);
}

time_t DateTime::set(int year, int month, int day,
                     int hour, int min, int sec,
                     double subSec /* = 0.0 */)
{
  date_time_t when;
  when.year = year;
  when.month = month;
  when.day = day;
  when.hour = hour;
  when.min = min;
  when.sec = sec;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  _subSec = subSec;
  _normalize();
  return (_uTime);
}

time_t DateTime::setTime(int hour, int min, int sec, double subSec /* = 0.0 */)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  when.hour = hour;
  when.min = min;
  when.sec = sec;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  _subSec = subSec;
  _normalize();
  return (_uTime);
}

time_t DateTime::setYear(int year)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  when.year = year;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  return (_uTime);
}

time_t DateTime::setMonth(int month)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  when.month = month;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  return (_uTime);
}

time_t DateTime::setDay(int day)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  when.day = day;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  return (_uTime);
}

time_t DateTime::setHour(int hour)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  when.hour = hour;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  return (_uTime);
}

time_t DateTime::setMin(int min)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  when.min = min;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  return (_uTime);
}

time_t DateTime::setSec(int sec)
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  when.sec = sec;
  uconvert_to_utime(&when);
  _uTime = when.unix_time;
  return (_uTime);
}

time_t DateTime::setByDayOfYear(int year, int dofy,
                                int hour, int min, 
                                int sec)
{
 
  int month;
  int day;
  // get month and day from day of the year
  getMonthDay(year, dofy, &month, &day);
  return(set(year, month, day, hour, min, sec));
}


time_t DateTime::forecastUtime() const {
  if (_leadTime == NULL) {
    return 0;
  } else {
    return (_uTime + _leadTime->getDurationInSeconds());
  }
}

string DateTime::getW3cStr() const
{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
          mtime.year, mtime.month, mtime.day,
          mtime.hour, mtime.min, mtime.sec);
 
  return str;
}


string DateTime::strfTime(const string& format, bool use_gmt) const
{
  return strfTime(format.c_str(), use_gmt);
}

string DateTime::strfTime(const char* format, bool use_gmt) const
{
  struct tm *timeInfo;

  if (use_gmt == true) {
    timeInfo = gmtime(&_uTime);
  }
  else {
    timeInfo = localtime(&_uTime);
  }

  static const size_t bufSize = 256;
  char buffer[bufSize];

  size_t finalSize = std::strftime(buffer, bufSize, format, timeInfo);

  string returnStr = "";

  if (finalSize < bufSize) {
    returnStr = buffer;
  }

  return returnStr;
}

time_t DateTime::strpTime(const string& format, const string& time_str, bool add_seconds, bool UTC)
{
  return strpTime(format.c_str(), time_str.c_str(), add_seconds, UTC);
}

time_t DateTime::strpTime(const char* format, const char* time_str, bool add_seconds, bool UTC)
{
  struct tm timeInfo;

  // initialize to start of epoch
  timeInfo.tm_sec = 0;
  timeInfo.tm_min = 0; 
  timeInfo.tm_hour = 0; 
  timeInfo.tm_mday = 0; 
  timeInfo.tm_mon = 0; 
  timeInfo.tm_year = 0; 
  timeInfo.tm_wday = 0; 
  timeInfo.tm_yday = 0; 
  timeInfo.tm_isdst = 0; 

  char* returnVal = strptime(time_str, format, &timeInfo);

  // add seconds if needed
  if(add_seconds == true) {
    // This has been eliminated due to redundancy.
    // If the seconds are left off, they will be 
    // set to 0 in strptime above.
  }

  time_t returnTime = 0;
  if (returnVal == 0) {
    returnTime = -1;
  }
  else if (UTC){
    //send back as UTC time
    returnTime = timegm(&timeInfo);
  }
  else
  {
    // send back as local time
    returnTime = mktime(&timeInfo);
  }
  return returnTime;
}


//
// String formatting
//
const char *
  DateTime::ctime() const
{
  //
  // stdC library string 
  // returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  // e.g. "Wed Jun 30 21:49:08 1993\n"
  //
  return ::ctime(&_uTime);
}

const char *
  DateTime::gtime() const
{
  //
  // stdC library string - returns time as UTC
  // returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  // e.g. "Wed Jun 30 21:49:08 1993\n"
  //
  return ::asctime(::gmtime(&_uTime));
}

const char *
  DateTime::dtime() const
{
  //
  // didss string
  // Returns string of format: "YYYY/MM/DD HH:MM:SS"
  // e.g. "1958/12/28 10:57:00"
  //
  char whenChar[20];
  date_time_t *when;

  if(_uTime > 0) {
    when = udate_time(_uTime);
    sprintf(whenChar, "%4d/%02d/%02d %02d:%02d:%02d",
            when->year, when->month, when->day,
            when->hour, when->min, when->sec);
  }
  else {
    strcpy(whenChar, "0000/00/00 00:00:00");
  }

  _strTime = whenChar;
  return _strTime.c_str();
}

const char *
  DateTime::kmltime() const
{
  //
  // KML string
  // Returns string of format: "yyyy-mm-ddThh:mm:ss"
  // e.g. "1958-12-28T10:57:00"
  //
  char whenChar[20];
  date_time_t *when;

  if(_uTime > 0) {
    when = udate_time(_uTime);
    sprintf(whenChar, "%4d-%02d-%02dT%02d:%02d:%02d",
            when->year, when->month, when->day,
            when->hour, when->min, when->sec);
  }
  else {
    strcpy(whenChar, "0000-00-00T00:00:00");
  }

  _strTime = whenChar;
  return _strTime.c_str();
}

// Returns string representing date and time
// of format: "YYYY/MM/DD HH:MM:SS"
// e.g. "1958/12/28 10:57:00"
// Optionally UTC is added to the end of string.
// If subsec is non-zero, format will be:
// "YYYY/MM/DD HH:MM:SS.uuuuuu"

string DateTime::getStr(bool utc_label) const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
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
// e.g. "19581228105700"

string DateTime::getStrPlain() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d%.2d%.2d%.2d%.2d%.2d",
          mtime.year, mtime.month, mtime.day,
          mtime.hour, mtime.min, mtime.sec);
  return str;
}

// Return string representing date only
// of format: "YYYY/MM/DD"
// e.g. "1958/12/28"
string DateTime::getDateStr() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d/%.2d/%.2d",
          mtime.year, mtime.month, mtime.day);
  return str;
}


// Return plain string representing date only
// of format: "YYYYMMDD"
// e.g. "19581228"
string DateTime::getDateStrPlain() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d%.2d%.2d",
          mtime.year, mtime.month, mtime.day);
  return str;
}

// Return string representing date only
// of format: "MM/DD/YYYY"
// e.g. "12/28/1958"
string DateTime::getDateStrMDY() const

{
  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.2d/%.2d/%.4d",
          mtime.month, mtime.day, mtime.year);
  return str;
}

// Return string representing time only
// of format: "HH:MM:SS"
// e.g. "10:57:00"
// Optionally UTC is added to the end of string.

string DateTime::getTimeStr(bool utc_label) const

{

  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
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
// e.g. "105700"

string DateTime::getTimeStrPlain() const

{

  date_time_t mtime;
  mtime.unix_time = _uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.2d%.2d%.2d",
          mtime.hour, mtime.min, mtime.sec);
  return str;
}

// Given a time, returns string
// of format: "YYYY/MM/DD HH:MM:SS"
// e.g. "1958/12/28 10:57:00"
// If no time given, current time is used.
// Optionally UTC is added to the end of string.
// This is static on the class, and is thread-safe.

string DateTime::str(const time_t mytime, const bool utc_label)

{
  date_time_t mtime;
  if (mytime == 0) {
    mtime.unix_time = time(NULL);
  } else {
    mtime.unix_time = mytime;
  }
  uconvert_from_utime(&mtime);

  char str[32];
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
// e.g. "1958/12/28 10:57:00"
// If mytime == 0, returns "===== NOT SET ====="


string DateTime::strm(const time_t mytime)

{

  if (mytime == 0) {
    return "===== NOT SET =====";
  }

  date_time_t mtime;
  mtime.unix_time = mytime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
          mtime.year, mtime.month, mtime.day,
          mtime.hour, mtime.min, mtime.sec);
  return str;

}

// Given a time, returns string with underscores instead of spaces.
// of format: "YYYY/MM/DD_HH:MM:SS"
// e.g. "1958/12/28 10:57:00"
// If mytime == 0, returns "===== NOT SET ====="


string DateTime::stru(const time_t mytime)

{

  if (mytime == 0) {
    return "=====_NOT_SET_=====";
  }

  date_time_t mtime;
  mtime.unix_time = mytime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d/%.2d/%.2d_%.2d:%.2d:%.2d",
          mtime.year, mtime.month, mtime.day,
          mtime.hour, mtime.min, mtime.sec);
  return str;

}

// Given a time, returns string with one space between fields of format:
//  "YYYY MM DD HH MM SS"
// e.g. "1958 12 28 10 57 00"

string DateTime::strs(const time_t mytime)

{
  date_time_t mtime;
  if (mytime == 0) {
    mtime.unix_time = time(NULL);
  } else {
    mtime.unix_time = mytime;
  }
  uconvert_from_utime(&mtime);

  char str[32];
  sprintf(str, "%.4d %.2d %.2d %.2d %.2d %.2d",
	  mtime.year, mtime.month, mtime.day,
	  mtime.hour, mtime.min, mtime.sec);
  return str;

}

//////////////////////////////////////////////////////////////
/// get as string
/// specify the number of digits of precision in the sub-seconds

string DateTime::asString(int subsecPrecision /* = 6*/) const
{

  char text[1024];
  DateTime mtime(_uTime);
  if (_subSec == 0 || subsecPrecision == 0) {
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

////////////////
// get as double

double DateTime::asDouble() const
{
  return (double) _uTime + _subSec;
}

/////////////////////////
// get individual members

int DateTime::getYear() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(&dtime);
  return dtime.year;
}

int DateTime::getMonth() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(&dtime);
  return dtime.month;
}

int DateTime::getDay() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(&dtime);
  return dtime.day;
}

int DateTime::getHour() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(&dtime);
  return dtime.hour;
}

int DateTime::getMin() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(&dtime);
  return dtime.min;
}

int DateTime::getSec() const
{
  date_time_t dtime;
  dtime.unix_time = _uTime;
  uconvert_from_utime(&dtime);
  return dtime.sec;
}

int DateTime::getDayOfYear() const
{

  date_time_t start;
  start.unix_time = _uTime;
  uconvert_from_utime(&start);
  //
  // Retain the year but go to the start of Jan 1
  //
  start.day = 1;
  start.month = 1;
  start.hour = 0;
  start.min = 0;
  start.min = 0;
  uconvert_to_utime(&start);

  int delta = _uTime - start.unix_time;
  int doy = 1 + delta/SECS_IN_DAY;

  return doy;
}

int DateTime::getDaysInMonth() const
{
  return daysOfMonth[getMonth()-1];
}

int DateTime::getDayOfWeek() const
{
  // We know that Jan 1, 1970 was a Thursday (= 4), so see how many days have
  // passed since that day and modulo 7 to get the correct day of the week

  int days_since_1970 = _uTime / SECS_IN_DAY;
 
  return (days_since_1970 + 4) % 7;
}

// fill out any args supplied

void DateTime::getAll(int *year,
                      int *month,
                      int *day,
                      int *hour,
                      int *min,
                      int *sec) const
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  if (year) *year = when.year;
  if (month) *month = when.month;
  if (day) *day = when.day;
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

void DateTime::getAll(short *year,
                      short *month,
                      short *day,
                      short *hour,
                      short *min,
                      short *sec) const
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  if (year) *year = when.year;
  if (month) *month = when.month;
  if (day) *day = when.day;
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

void DateTime::getTime(int *hour,
                       int *min,
                       int *sec) const
{
  date_time_t when;
  when.unix_time = _uTime;
  uconvert_from_utime(&when);
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

DateTime DateTime::operator+ (const DeltaTime& delta) const
{
  DateTime answer(*this);
  answer += delta;
  return answer;
}
 
DateTime DateTime::operator+ (const double secs) const
{
  DateTime answer(*this);
  answer += secs;
  return answer;
}
 
DateTime DateTime::operator- (const DeltaTime& delta) const
{
  DateTime answer(*this);
  answer -= delta;
  return answer;
}
 
DateTime DateTime::operator- (const double secs) const
{
  DateTime answer(*this);
  answer -= secs;
  return answer;
}
 
DateTime& DateTime::operator+= (double secs)
{
  _subSec += secs;
  _normalize();
  return *this;
}
 
DateTime& DateTime::operator-= (double secs)
{
  _subSec -= secs;
  _normalize();
  return *this;
}
 
DateTime& DateTime::operator+= (const DeltaTime& delta)
{
  _uTime += delta.getDurationInSeconds();
  return *this;
}

DateTime& DateTime::operator-= (const DeltaTime& delta)
{
  _uTime -= delta.getDurationInSeconds();
  return *this;
}

// difference in seconds

double DateTime::operator- (const DateTime& other) const
{
  // Can't subtract forecast times.
  if (isForecastTime() || other.isForecastTime()) {
    return NEVER;
  }
  double idiff = _uTime - other._uTime;
  double fdiff = _subSec - other._subSec;
  return idiff + fdiff;
}
 
////////////////////////////////////////////////////////
// scan the input string for conformance to W3C ISO 8601
// See http://www.w3.org/TR/NOTE-datetime
// returns 0 on success, -1 on failure
// If requireSeconds is true, we expect
// year, month, day, hour, min and sec to all be present.

int DateTime::setFromW3c(const char *strWhen,
                         bool requireSeconds /* = false */)
 
{
 
  // Degenerate case

  if (strWhen == NULL) {
    return -1;
  }
  if (strlen(strWhen) < 4) {
    // must at least have yyyy
    return -1;
  }

  // tokenize the string into numerals and non-numerals

  bool inNumeral = false;
  if (isdigit(strWhen[0])) {
    inNumeral = true;
  }
  
  vector<string> numerals;     // tokens which are numerals
  vector<string> nonNumerals;  // tokens which are not numerals
  vector<string> preNumerals;  // tokens which precede numerals

  string numeral;
  string nonNumeral;
  int nNonNumerals = 0;
  
  for (size_t ii = 0; ii < strlen(strWhen); ii++) {
    char cc = strWhen[ii];
    if (isdigit(cc)) {
      if (!inNumeral) {
        nonNumerals.push_back(nonNumeral);
        nonNumeral.clear();
        inNumeral = true;
      }
      numeral += cc;
    } else {
      if (inNumeral) {
        numerals.push_back(numeral);
        numeral.clear();
        inNumeral = false;
        if (nonNumerals.size() == 0) {
          preNumerals.push_back("");
        } else {
          preNumerals.push_back(nonNumerals[nonNumerals.size()-1]);
        }
      }
      nonNumeral += cc;
      nNonNumerals++;
    }
  }

  if (nonNumeral.size() > 0) {
    nonNumerals.push_back(nonNumeral);
  }
  if (numeral.size() > 0) {
    numerals.push_back(numeral);
    if (nonNumerals.size() == 0) {
      preNumerals.push_back("");
    } else {
      preNumerals.push_back(nonNumerals[nonNumerals.size()-1]);
    }
  }

  // must have at least year
  
  if (numerals.size() == 0) {
    return -1;
  }

  // check we have some non-numerals if numerals size > 4
  
  if (nNonNumerals == 0 && strlen(strWhen) > 4) {
    // no non-numerals, cannot be w3c
    return -1;
  }

  // do we require seconds?

  if (requireSeconds) {
    if (numerals.size() < 6) {
      return -1;
    }
  }

  int year = 0;
  int month = 1;
  int day = 1;
  int hour = 0;
  int min = 0;
  int sec = 0;
  double subSec = 0.0;
  int tzoneIndex = -1;

  // year, month, day, hour, min

  if (numerals.size() > 0) {
    year = atoi(numerals[0].c_str());
  }
  if (numerals.size() > 1) {
    month = atoi(numerals[1].c_str());
  }
  if (numerals.size() > 2) {
    day = atoi(numerals[2].c_str());
  }
  if (numerals.size() > 3) {
    hour = atoi(numerals[3].c_str());
  }
  if (numerals.size() > 4) {
    min = atoi(numerals[4].c_str());
  }
  
  // second? or time zone?

  if (numerals.size() > 5) {
    if (preNumerals[5].find("-") != string::npos ||
        preNumerals[5].find("+") != string::npos) {
      tzoneIndex = 5;
    } else {
      sec = atoi(numerals[5].c_str());
    }
  }

  // decimal seconds, or time zone?
  
  if (numerals.size() > 6) {
    if (preNumerals[6] == ".") {
      string decStr = "0.";
      decStr += numerals[6];
      subSec = atof(decStr.c_str());
    } else if (preNumerals[6].find("-") != string::npos ||
               preNumerals[6].find("+") != string::npos) {
      tzoneIndex = 6;
    }
  }
  
  // time zone?
  
  if (numerals.size() > 7) {
    if (preNumerals[7].find("-") != string::npos ||
        preNumerals[7].find("+") != string::npos) {
      tzoneIndex = 7;
    }
  }

  bool zuluTime = false;
  if (nonNumerals[nonNumerals.size() - 1].find("Z") != string::npos) {
    zuluTime = true;
  }

  int tdiff = 0;
  if (zuluTime) {
    tdiff = 0;
  } else if (tzoneIndex > 0) {
    int thour = 0;
    if ((int) numerals.size() > tzoneIndex) {
      thour = atoi(numerals[tzoneIndex].c_str());
    }
    int tmin = 0;
    if ((int) numerals.size() > tzoneIndex + 1) {
      tmin = atoi(numerals[tzoneIndex + 1].c_str());
    }
    tdiff = thour * 3600 + tmin * 60;
    if ((int) preNumerals.size() > tzoneIndex &&
        preNumerals[tzoneIndex].find("-") != string::npos) {
      tdiff *= -1;
    }
  }

  set(year, month, day, hour, min, sec, subSec);
  _uTime -= tdiff;

  return 0;

}

////////////////////////////////////////////////////////
// parse the time from input string.
// Accepted strings are:
//   *yyyymmdd?hhmmss*
//   *yyyymmddhhmmss*
// returns 0 on success, -1 on failure

int DateTime::setFromYYYYMMDDHHMMSS(const char *strWhen)
{

  if (strWhen == NULL) {
    return -1;
  }
  
  const char *start = strWhen;
  const char *end = start + strlen(start);

  while (start < end - 14) {
    
    if (!isdigit(start[0])) {
      start++;
      continue;
    }
    
    int year, month, day, hour, min, sec;
    char cc;

    if (sscanf(start, "%4d%2d%2d%1c%2d%2d%2d",
               &year, &month, &day, &cc, &hour, &min, &sec) == 7) {

      // format - yyyymmdd_hhmmss

      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        start++;
        continue;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        start++;
        continue;
      }
      set(year, month, day, hour, min, sec);
      return 0;

    }

    if (sscanf(start, "%4d%2d%2d%2d%2d%2d",
               &year, &month, &day, &hour, &min, &sec) == 6) {

      // format - yyyymmddhhmmss

      if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        start++;
        continue;
      }
      if (hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59) {
        start++;
        continue;
      }
      set(year, month, day, hour, min, sec);
      return 0;

    }

    start++;
    
  } // while (start ...

  // no luck

  return -1;

}
 
// Static
void
  DateTime::tokenizeString(const char *strWhen,
                           int &YYYY, int &MM, int &DD,
                           int &hh, int &mm, int &ss,
                           double &subSec)
{
  char *sPtr;
  char strWhenCopy[1024];

  //
  // Initialize the resulting values
  //
  YYYY = MM = DD = hh = mm = ss = 0;
  subSec = 0.0;

  //
  // Degenerate case
  //
  if(strWhen == NULL)
    return;
  strcpy(strWhenCopy, strWhen);
  sPtr = strWhenCopy;

  //
  // Year
  //
  if(sscanf(sPtr, "%4d", &YYYY) != 1)
    return;

  //
  // Month
  //
  sPtr += 4;
  if(!isdigit(sPtr[0]))
    sPtr++;
  if(sscanf(sPtr, "%2d", &MM) != 1)
    return;

  //
  // Day
  //
  sPtr += 2;
  if(!isdigit(sPtr[0]))
    sPtr++;
  if(sscanf(sPtr, "%2d", &DD) != 1)
    return;

  //
  // Hour
  //
  sPtr += 2;
  if(!isdigit(sPtr[0]))
    sPtr++;
  if(sscanf(sPtr, "%2d", &hh) != 1)
    return;

  //
  // Minute
  //
  sPtr += 2;
  if(!isdigit(sPtr[0]))
    sPtr++;
  if(sscanf(sPtr, "%2d", &mm) != 1)
    return;

  //
  // Second
  //
  sPtr += 2;
  if(!isdigit(sPtr[0]))
    sPtr++;
  if(sscanf(sPtr, "%2d", &ss) != 1)
    return;

  //
  // Sub-second
  //
  sPtr += 2;
  if(sPtr[0] != '.') {
    return;
  }
  sPtr++;
  double frac;
  if(sscanf(sPtr, "%lg", &frac) != 1)
    return;
  double divisor = 1.0;
  for (size_t ii = 0; ii < strlen(sPtr); ii++) {
    if (isdigit(sPtr[ii])) {
      divisor *= 10.0;
    } else {
      break;
    }
  }
  subSec = frac / divisor;

}

///////////////////////
// printing to ostream

ostream& operator<< (ostream &os, const DateTime &d)
{
  char buf[128];
  date_time_t dtime;
  dtime.unix_time = d.utime();
  uconvert_from_utime(&dtime);
  if (d.isForecastTime()) {
    sprintf(buf, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d %ld",
            dtime.year, dtime.month, dtime.day,
            dtime.hour, dtime.min, dtime.sec,
            d.getLeadDeltaTime()->getDurationInSeconds());
  } else {
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
  }
  string str(buf);
  return os << str;
}

ostream& operator<< (ostream &os, const DateTime *d)
{
  return operator<<(os, *d);
}

//////////////////////
// normalize subSec

void DateTime::_normalize()
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

