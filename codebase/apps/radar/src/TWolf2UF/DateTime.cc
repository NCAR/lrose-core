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
#include <iostream>
#include <ctype.h>
#include <sys/time.h>
#include "DateTime.hh"
using namespace std;

const time_t DateTime::NEVER = 2000000000L;

/* days in the month */
static int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

DateTime::DateTime()
{
  uTime = 0;
}

DateTime::DateTime( time_t when )
{
  uTime = when;
}

DateTime::DateTime( const char *when )
{
  uTime = parseDateTime( when );
}

DateTime::DateTime( const string &when )
{
  uTime = parseDateTime( when.c_str() );
}

DateTime::DateTime( int year, int month, int day,
                    int hour, int min, int sec )
{
  set( year, month, day, hour, min, sec );
}

DateTime::DateTime( const DateTime & orig )
{
  copy (orig);
}

DateTime::~DateTime( )
{
}

// Static
time_t
  DateTime::parseDateTime( const char *strWhen,
                           int *year, int *month, int *day,
                           int *hour, int *min, int *sec )
{
  int          YYYY, MM, DD, hh, mm, ss;
  date_time_t  when;

  //
  // Tokenize the dateTime string
  //
  tokenizeString( strWhen, YYYY, MM, DD, hh, mm, ss );

  //
  // Now, do some validity checking
  //
  if ( YYYY <= 0 ) {
    return NEVER;
  }
  if ( MM <= 0 || MM > 12 ) {
    return NEVER;
  }
  if ( DD <= 0 || DD > 31 ) {
    return NEVER;
  }
  if ( hh < 0 || hh > 23 ) {
    return NEVER;
  }
  if ( mm < 0 || mm > 59 ) {
    return NEVER;
  }
  if ( ss < 0 || ss > 59 ) {
    return NEVER;
  }

  //
  // Save the time components, if requested
  //
  if ( year )
    *year = YYYY;
  if ( month )
    *month = MM;
  if ( day )
    *day = DD;
  if ( hour )
    *hour = hh;
  if ( min )
    *min = mm;
  if ( sec )
    *sec = ss;

  //
  // Return the resulting unix time
  //
  when.year  = YYYY;
  when.month = MM;
  when.day   = DD;
  when.hour  = hh;
  when.min   = mm;
  when.sec   = ss;

  return( uunix_time( &when ) );
}

void DateTime::getMonthDay( const int year, const int dofy, 
                            int &month, int &day )
{
  getMonthDay(year, dofy, &month, &day);
}

void DateTime::getMonthDay( const int year, const int dofy, 
                            int *month, int *day )
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

double DateTime::getCurrentTimeAsDouble()

{
  struct timeval tval;
  gettimeofday(&tval, NULL);
  return (tval.tv_sec + tval.tv_usec / 1000000.0);
}


void
  DateTime::copy( const DateTime &source )
{
  uTime = source.uTime;
}

void
  DateTime::set( time_t when )
{
  uTime = when; 
}

time_t
  DateTime::set( const char *strWhen )
{
  //
  // Set time using a string of the format:  "YYYY-MM-DD-HH-MM-SS"
  // where: "-" represents a valid delimeter [/_: ]
  //                                    e.g. "1958/12/28 10:57:00"
  //                                    e.g. "19581228105700"
  //
  uTime = parseDateTime( strWhen );
  return( uTime );
}

time_t DateTime::set( int year, int month, int day,
                      int hour, int min, int sec )
{
  date_time_t when;
  when.year = year;
  when.month = month;
  when.day = day;
  when.hour = hour;
  when.min = min;
  when.sec = sec;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setTime( int hour, int min, int sec )
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  when.hour = hour;
  when.min = min;
  when.sec = sec;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setYear( int year )
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  when.year = year;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setMonth( int month )
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  when.month = month;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setDay( int day )
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  when.day = day;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setHour( int hour )
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  when.hour = hour;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setMin( int min )
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  when.min = min;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setSec( int sec )
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  when.sec = sec;
  uconvert_to_utime(&when);
  uTime = when.unix_time;
  return (uTime);
}

time_t DateTime::setByDayOfYear( int year, int dofy,
                                 int hour, int min, 
                                 int sec )
{
  
  int month;
  int day;
  // get month and day from day of the year
  getMonthDay( year, dofy, &month, &day );
  return ( set( year, month, day, hour, min, sec ) );
}


string DateTime::getW3cStr() const
{
  date_time_t mtime;
  mtime.unix_time = uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d-%.2d-%.2dT%.2d:%.2d:%.2dZ",
	  mtime.year, mtime.month, mtime.day,
	  mtime.hour, mtime.min, mtime.sec);
  
  return str;
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
  //                      e.g. "Wed Jun 30 21:49:08 1993\n"
  //
  return ::ctime( &uTime );
}

const char *
  DateTime::gtime() const
{
  //
  // stdC library string - returns time as UTC
  // returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  //                      e.g. "Wed Jun 30 21:49:08 1993\n"
  //
  return ::asctime(::gmtime( &uTime ));
}

const char *
  DateTime::dtime() const
{
  //
  // didss string
  // Returns string of format:  "YYYY/MM/DD HH:MM:SS"
  //                      e.g.  "1958/12/28 10:57:00"
  //
  char         whenChar[20];
  date_time_t *when;

  if ( uTime > 0 ) {
    when = udate_time( uTime );
    sprintf( whenChar, "%4d/%02d/%02d %02d:%02d:%02d",
             when->year, when->month, when->day,
             when->hour, when->min, when->sec );
  }
  else {
    strcpy( whenChar, "0000/00/00 00:00:00" );
  }

  strTime = whenChar;
  return strTime.c_str();
}

const char *
  DateTime::kmltime() const
{
  //
  // KML string
  // Returns string of format:  "yyyy-mm-ddThh:mm:ss"
  //                      e.g.  "1958-12-28T10:57:00"
  //
  char         whenChar[20];
  date_time_t *when;

  if ( uTime > 0 ) {
    when = udate_time( uTime );
    sprintf( whenChar, "%4d-%02d-%02dT%02d:%02d:%02d",
             when->year, when->month, when->day,
             when->hour, when->min, when->sec );
  }
  else {
    strcpy( whenChar, "0000-00-00T00:00:00" );
  }

  strTime = whenChar;
  return strTime.c_str();
}

// Returns string representing date and time
// of format:  "YYYY/MM/DD HH:MM:SS"
//       e.g.  "1958/12/28 10:57:00"
// Optionally UTC is added to the end of string.

string DateTime::getStr(bool utc_label) const

{
  date_time_t mtime;
  mtime.unix_time = uTime;
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

// Returns plain string representing date and time
// of format:  "YYYYMMDDHHMMSS"
//       e.g.  "19581228105700"

string DateTime::getStrPlain() const

{
  date_time_t mtime;
  mtime.unix_time = uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d%.2d%.2d%.2d%.2d%.2d",
	  mtime.year, mtime.month, mtime.day,
	  mtime.hour, mtime.min, mtime.sec);
  return str;
}

// Return string representing date only
// of format:  "YYYY/MM/DD"
//       e.g.  "1958/12/28"
string DateTime::getDateStr() const

{
  date_time_t mtime;
  mtime.unix_time = uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d/%.2d/%.2d",
	  mtime.year, mtime.month, mtime.day);
  return str;
}


// Return plain string representing date only
// of format:  "YYYYMMDD"
//       e.g.  "19581228"
string DateTime::getDateStrPlain() const

{
  date_time_t mtime;
  mtime.unix_time = uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.4d%.2d%.2d",
	  mtime.year, mtime.month, mtime.day);
  return str;
}

// Return string representing date only
// of format:  "MM/DD/YYYY"
//       e.g.  "12/28/1958"
string DateTime::getDateStrMDY() const

{
  date_time_t mtime;
  mtime.unix_time = uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.2d/%.2d/%.4d",
	  mtime.month, mtime.day, mtime.year );
  return str;
}

// Return string representing time only
// of format:  "HH:MM:SS"
//       e.g.  "10:57:00"
// Optionally UTC is added to the end of string.

string DateTime::getTimeStr(bool utc_label) const

{

  date_time_t mtime;
  mtime.unix_time = uTime;
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
// of format:  "HHMMSS"
//       e.g.  "105700"

string DateTime::getTimeStrPlain() const

{

  date_time_t mtime;
  mtime.unix_time = uTime;
  uconvert_from_utime(&mtime);
  char str[32];
  sprintf(str, "%.2d%.2d%.2d",
	  mtime.hour, mtime.min, mtime.sec);
  return str;
}

// Given a time, returns string
// of format:  "YYYY/MM/DD HH:MM:SS"
//       e.g.  "1958/12/28 10:57:00"
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
// of format:  "YYYY/MM/DD HH:MM:SS"
//       e.g.  "1958/12/28 10:57:00"
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

int DateTime::getYear() const
{
  date_time_t dtime;
  dtime.unix_time = uTime;
  uconvert_from_utime(&dtime);
  return dtime.year;
}

int DateTime::getMonth() const
{
  date_time_t dtime;
  dtime.unix_time = uTime;
  uconvert_from_utime(&dtime);
  return dtime.month;
}

int DateTime::getDay() const
{
  date_time_t dtime;
  dtime.unix_time = uTime;
  uconvert_from_utime(&dtime);
  return dtime.day;
}

int DateTime::getHour() const
{
  date_time_t dtime;
  dtime.unix_time = uTime;
  uconvert_from_utime(&dtime);
  return dtime.hour;
}

int DateTime::getMin() const
{
  date_time_t dtime;
  dtime.unix_time = uTime;
  uconvert_from_utime(&dtime);
  return dtime.min;
}

int DateTime::getSec() const
{
  date_time_t dtime;
  dtime.unix_time = uTime;
  uconvert_from_utime(&dtime);
  return dtime.sec;
}

int DateTime::getDayOfYear() const
{

  date_time_t start;
  start.unix_time = uTime;
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

  int delta = uTime - start.unix_time;
  int doy = 1 + delta/SECS_IN_DAY;

  return doy;
}

int DateTime::getDaysInMonth() const
{
  return daysOfMonth[getMonth()-1];
}

// fill out any args supplied

void DateTime::getAll( int *year,
                       int *month,
                       int *day,
                       int *hour,
                       int *min,
                       int *sec ) const
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  if (year) *year = when.year;
  if (month) *month = when.month;
  if (day) *day = when.day;
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

void DateTime::getAll( short *year,
                       short *month,
                       short *day,
                       short *hour,
                       short *min,
                       short *sec ) const
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  if (year) *year = when.year;
  if (month) *month = when.month;
  if (day) *day = when.day;
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

void DateTime::getTime( int *hour,
                        int *min,
                        int *sec ) const
{
  date_time_t when;
  when.unix_time = uTime;
  uconvert_from_utime(&when);
  if (hour) *hour = when.hour;
  if (min) *min = when.min;
  if (sec) *sec = when.sec;
}

DateTime DateTime::operator+ (const long secs) const
{
  // Can't operate on an invalid time.
  if (uTime < 0)
    return *this;

  DateTime answer(*this);
  answer += secs;
  return answer;
}
 
DateTime DateTime::operator- (const long secs) const
{
  // Can't operate on an invalid time.
  if (uTime < 0)
    return *this;

  DateTime answer(*this);
  answer -= secs;
  return answer;
}
 
DateTime& DateTime::operator+= (long secs)
{
  // Can't operate on an invalid time.
  if (uTime < 0) {
    return *this;
  }
  else {
    uTime += secs;
    return *this;
  }
}
 
DateTime& DateTime::operator-= (long secs)
{
  // Can't operate on an invalid time.
  if (uTime < 0) {
    return *this;
  }
  else {
    uTime -= secs;
    return *this;
  }
}
 
long DateTime::operator- (const DateTime& other) const
{
  // Can't operate on an invalid time.
  if (uTime < 0) {
    return NEVER;
  }

  return ( uTime - other.uTime );
}
 
// Static
void
  DateTime::tokenizeString( const char *strWhen,
                            int &YYYY, int &MM, int &DD,
                            int &hh,  int &mm, int &ss )
{
  char      *sPtr;
  char       strWhenCopy[1024];

  //
  // Initialize the resulting values
  //
  YYYY = MM = DD = hh = mm = ss = 0;

  //
  // Degenerate case
  //
  if ( strWhen == NULL )
    return;
  strcpy( strWhenCopy, strWhen );
  sPtr = strWhenCopy;

  //
  // Year
  //
  if ( sscanf( sPtr, "%4d", &YYYY ) != 1 )
    return;

  //
  // Month
  //
  sPtr += 4;
  if ( !isdigit( sPtr[0] ))
    sPtr++;
  if ( sscanf( sPtr, "%2d", &MM ) != 1 )
    return;

  //
  // Day
  //
  sPtr += 2;
  if ( !isdigit( sPtr[0] ))
    sPtr++;
  if ( sscanf( sPtr, "%2d", &DD ) != 1 )
    return;

  //
  // Hour
  //
  sPtr += 2;
  if ( !isdigit( sPtr[0] ))
    sPtr++;
  if ( sscanf( sPtr, "%2d", &hh ) != 1 )
    return;

  //
  // Minute
  //
  sPtr += 2;
  if ( !isdigit( sPtr[0] ))
    sPtr++;
  if ( sscanf( sPtr, "%2d", &mm ) != 1 )
    return;

  //
  // Second
  //
  sPtr += 2;
  if ( !isdigit( sPtr[0] ))
    sPtr++;
  if ( sscanf( sPtr, "%2d", &ss ) != 1 )
    return;
}

/*************************************************************************
 * uunix_time()
 *
 * return the unix time from date_time_t struct
 *
 * Side-effect: sets the unix_time field in date_time.
 */

time_t DateTime::uunix_time(DateTime::date_time_t *date_time)
{

  long day, days;
  time_t u_time;

  day = DateTime::ujulian_date(date_time->day,
                               date_time->month,
                               date_time->year);
  
  days = day - DateTime::JAN_1_1970;

  u_time = (days * 86400) + (date_time->hour * 3600) +
    (date_time->min * 60) + date_time->sec;

  date_time->unix_time = u_time;

  return u_time;

}

/*************************************************************************
 * udate_time()
 *
 * return pointer to date_time struct corresponding to unix time. The pointer
 * refers to a static held by this routine
 *
 */

DateTime::date_time_t *DateTime::udate_time(time_t unix_time)
{

  static DateTime::date_time_t date_time;

  date_time.unix_time = unix_time;
  DateTime::uconvert_from_utime(&date_time);

  return (&date_time);

}
 
/*************************************************************************
 * uconvert_to_utime()
 *
 * return the unix time from date_time_t struct.  Also sets the unix_time
 * field in the date_time_t structure.
 */

time_t DateTime::uconvert_to_utime(DateTime::date_time_t *date_time)
{
  time_t u_time;

  u_time = DateTime::uunix_time(date_time);
  
  date_time->unix_time = u_time;
  
  return u_time;

}

/*************************************************************************
 * utime_compute()
 *
 * return the unix time from components.
 */

time_t DateTime::utime_compute(int year, int month, int day,
                               int hour, int min, int sec)
{

  DateTime::date_time_t dtime;

  dtime.year = year;
  dtime.month = month;
  dtime.day = day;
  dtime.hour = hour;
  dtime.min = min;
  dtime.sec = sec;

  DateTime::uconvert_to_utime(&dtime);
  
  return(dtime.unix_time);

}

/*************************************************************************
 * uconvert_from_utime()
 *
 * sets the other fields in the date_time_t structure based on the value
 * of the unix_time field in that structure
 */

void DateTime::uconvert_from_utime(DateTime::date_time_t *date_time)
{

  time_t unix_time;
  long day;
  int time_of_day;
  
  unix_time = date_time->unix_time;

  time_of_day = (unix_time % 86400);
  day = (unix_time / 86400);
  if (unix_time < 0 && time_of_day != 0) {
    day--;
    time_of_day += 86400;
  }
  
  DateTime::ucalendar_date((JAN_1_1970 + day),
                           &date_time->day,
                           &date_time->month,
                           &date_time->year);
  
  date_time->hour = time_of_day / 3600;
  date_time->min = (time_of_day / 60) - (date_time->hour * 60);
  date_time->sec = time_of_day % 60;

}
 
/*************************************************************************
 *	JULIAN_DATE: Calc the Julian calendar Day Number
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

long DateTime::ujulian_date(int day, int month, int year)
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

void DateTime::ucalendar_date(long jdate, int *day,
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

void DateTime::ulocaltime(DateTime::date_time_t *date_time)
{

  struct tm *local_time;
  time_t now;

  time(&now);
  local_time = localtime(&now);

  date_time->year = local_time->tm_year + 1900;
  date_time->month = local_time->tm_mon + 1;
  date_time->day = local_time->tm_mday;
  date_time->hour = local_time->tm_hour;
  date_time->min = local_time->tm_min;
  date_time->sec = local_time->tm_sec;
  DateTime::uconvert_to_utime(date_time);

}

/**************************************************************************
 * void ugmtime()
 *
 * puts the gm time into a date_time_t struct
 *
 **************************************************************************/

void DateTime::ugmtime(DateTime::date_time_t *date_time)
{

  date_time->unix_time = time(NULL);
  DateTime::uconvert_from_utime(date_time);

}

/**************************************************************************
 * char *utimstr()
 *
 * Calls utimestr() - see notes for this function.
 *
 **************************************************************************/

char *DateTime::utimstr (time_t time)
{
  return (DateTime::utimestr(DateTime::udate_time(time)));
}

/**************************************************************************
 * char *utimestr()
 *
 * returns a string composed from the time struct. This routine has a
 * number of static storage areas, and loops through these areas. Every
 * 10 times you make the call you will overwrite a previous result.
 *
 **************************************************************************/

char *DateTime::utimestr(DateTime::date_time_t *date_time)
{

  static char time_buf[10][32];
  static int count = 0;

  char *timestr;

  count++;
  
  if (count > 9)
    count = 0;

  timestr = time_buf[count];

  sprintf(timestr, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
	  date_time->year,
	  date_time->month,
	  date_time->day,
	  date_time->hour,
	  date_time->min,
	  date_time->sec);

  return (timestr);

}

/**************************************************************************
 * char *utim_str()
 *
 * Calls utime_str() - see notes for this function.
 *
 **************************************************************************/

char *DateTime::utim_str (time_t time)
{
  return (DateTime::utime_str(DateTime::udate_time(time)));
}

/**************************************************************************
 * char *utime_str()
 *
 * Same as utimestr(), except puts an underscore between the date and
 * time.
 *
 **************************************************************************/

char *DateTime::utime_str(DateTime::date_time_t *date_time)
{

  static char time_buf[10][32];
  static int count = 0;

  char *time_str;

  count++;
  
  if (count > 9)
    count = 0;

  time_str = time_buf[count];

  sprintf(time_str, "%.4d/%.2d/%.2d_%.2d:%.2d:%.2d",
	  date_time->year,
	  date_time->month,
	  date_time->day,
	  date_time->hour,
	  date_time->min,
	  date_time->sec);

  return (time_str);

}
