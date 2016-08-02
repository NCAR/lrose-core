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
////////////////////////////////////////////////////////////////////////////////
//
// RadxTime
//
// Utility class for manipulating date/time
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _NCF_RAD_TIME_HH_
#define _NCF_RAD_TIME_HH_

#include <string>
#include <cstring>
#include <ctime>
using namespace std;

///////////////////////////////////////////////////////////////
/// UTILITY CLASS FOR TIME MANIPULATION IN RADX
///
/// This class is based on the NCAR/RAL toolsa DateTime class.

class RadxTime {

public:

  /// missing time

  static const time_t NEVER;

  /// enum to indicate specified time in constructor

  typedef enum { ZERO, NOW } SpecifyTime_t;
  
  /// Default constructor

  RadxTime();

  /// Construct with current time

  RadxTime(SpecifyTime_t);

  /// Constructor from a time_t

  RadxTime(time_t when);
  
  /// construct from time_t and subsec

  RadxTime(time_t when, double subSec);

  /// construct from year, month, day, hour, min, sec, sub secs
  /// hour, min, secs and subSec are optional args, that default
  /// to 0.

  RadxTime(int year, int month, int day,
           int hour = 0, int min = 0, int sec = 0,
           double subSec = 0.0);
  
  /// Constructor from a string
  ///
  /// Set time using a string of the format:  "YYYY-MM-DD-HH-MM-SS"
  /// where: "-" represents a valid delimeter [/_: ]
  ///                                    e.g. "1958/12/28 10:57:00"
  ///                                    e.g. "19581228105700"
  
  RadxTime(const string &when);

  /// Copy constructor

  RadxTime(const RadxTime &orig);

  /// Copying from another object

  void copy(const RadxTime &source);
  
  /// Destructor

  ~RadxTime();

  ///////////////////////////////////////////////////////////////////////////
  // Setting the time (see also the class operators for setting time)

  /// \name Set methods:
  //@{

  /// Clear - set to Jan 1 1970

  void clear();

  /// Set from time_t

  void set(time_t when);

  // set based on spec
  
  void set(SpecifyTime_t spec);

  /// Set from time_t and subSecs

  void set(time_t when, double subSec);

  /// Set date and time, with time defaulting to 00:00:00
  /// Returns time in secs from 1 Jan 1970 UTC
  
  time_t set(int year, int month, int day,
             int hour = 0, int min = 0, int sec = 0,
             double subSec = 0.0);
  
  /// Set just the time, do not change date
  /// Returns time in secs from 1 Jan 1970 UTC
  
  time_t setTime(int hour = 0, int min = 0, int sec = 0,
                 double subSec = 0.0);

  /// Set time from string
  ///
  /// Use string of the format:  "YYYY-MM-DD-HH-MM-SS"
  /// where: "-" represents a valid delimeter [/_: ]
  ///                                    e.g. "1958/12/28 10:57:00"
  ///                                    e.g. "19581228105700"
  ///
  /// Returns time in secs from 1 Jan 1970 UTC

  time_t set(const string &when);

  /// Set year
  ///
  /// Returns time in secs from 1 Jan 1970 UTC

  time_t setYear(int year);

  /// Set month
  ///
  /// Returns time in secs from 1 Jan 1970 UTC

  time_t setMonth(int month);

  /// Set day
  ///
  /// Returns time in secs from 1 Jan 1970 UTC

  time_t setDay(int day);

  /// Set hour
  ///
  /// Returns time in secs from 1 Jan 1970 UTC

  time_t setHour(int hour);

  /// Set minute
  ///
  /// Returns time in secs from 1 Jan 1970 UTC

  time_t setMin(int min);

  /// Set second
  ///
  /// Returns time in secs from 1 Jan 1970 UTC

  time_t setSec(int sec);
  
  /// Set sub seconds

  void setSubSec(double val);
  
  /// Set by day-of-year
  ///
  /// Returns time in secs from 1 Jan 1970 UTC
  
  time_t setByDayOfYear(int year, int dayOfYear,
                        int hour = 0, int min = 0, 
                        int sec = 0);

  /// Interpret a time string of the format:  "YYYY-MM-DD-HH-MM-SS"
  /// where: "-" represents a valid delimeter [/_: ]
  ///                                    e.g. "1958/12/28 10:57:00"
  ///                                    e.g. "19581228105700"
  ///
  /// Optionally, return the component integral values of the string
  ///
  /// Returns RadxTime::NEVER if the string did not contain a 
  ///           parseable year, month, and day.
  ///
  /// NOTE: Does not set the time on this class -- only parses!
  ///       Use the set() method if you want the time to be changed.

  static time_t parseDateTime(const string &when,
                              int *year = NULL, 
                              int *month = NULL,
                              int *day = NULL,
                              int *hour = NULL, 
                              int *min = NULL, 
                              int *sec = NULL,
                              double *subSec = NULL);

  static time_t parseDateTime(const string &when,
                              double &subSec);

  //@}
  
  ///////////////////////////////////////////////////////////////////////////
  // Getting the time

  /// \name Get methods:
  //@{

  /// Is the object valid - i.e. has the time been set?

  bool isValid() const { return _uTime != NEVER; }
  
  /// Get time in seconds since 1 Jan 1970 UTC

  time_t utime() const { return _uTime; }

  /// Return w3c standard time string YYYY-MM-DDTHH:MM:SSZ
  /// eg. 1993-0630T21:49:08Z

  string getW3cStr() const;

  /// stdC library string 
  /// Returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  ///                      e.g. "Wed Jun 30 21:49:08 1993\n"

  const string ctime() const;
  
  /// stdC library string - returns time as UTC
  /// Returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  ///                      e.g. "Wed Jun 30 21:49:08 1993\n"

  const string gtime() const;

  /// Returns string of format:  "YYYY/MM/DD HH:MM:SS"
  ///                      e.g.  "1958/12/28 10:57:00"

  const string dtime() const;

  /// KML string
  /// Returns string of format:  "yyyy-mm-ddThh:mm:ss"
  ///                      e.g.  "1958-12-28T10:57:00"

  const string kmltime() const;

  /// Return string representing date and time
  /// of format:  "YYYY/MM/DD HH:MM:SS"
  ///       e.g.  "1958/12/28 10:57:00"
  /// Optionally UTC is added to the end of string.

  string getStr(bool utc_label = true) const;

  /// Return string representing date and time
  /// of format:  "YYYY/MM/DD HH:MM:SS"
  ///       e.g.  "1958/12/28 10:57:00"

  string getStrn() const { return getStr(false); }
  
  /// Returns plain string representing date and time
  /// of format:  "YYYYMMDDHHMMSS"
  ///       e.g.  "19581228105700"
  
  string getStrPlain() const;

  /// Return string representing date only
  /// of format:  "YYYY/MM/DD"
  ///       e.g.  "1958/12/28"

  string getDateStr() const;

  /// Return plain string representing date only
  /// of format:  "YYYYMMDD"
  ///       e.g.  "19581228"

  string getDateStrPlain() const;

  /// Return string representing date only
  /// of format:  "MM/DD/YYYY"
  ///       e.g.  "12/28/1958"

  string getDateStrMDY() const;

  /// Return string representing time only
  /// of format:  "HH:MM:SS"
  ///       e.g.  "10:57:00"
  /// Optionally UTC is added to the end of string.

  string getTimeStr(bool utc_label = true) const;

  /// Return plain string representing time only
  /// of format:  "HHMMSS"
  ///       e.g.  "105700"
  
  string getTimeStrPlain() const;

  /// Given a time, returns string.
  /// of format:  "YYYY/MM/DD HH:MM:SS"
  ///       e.g.  "1958/12/28 10:57:00"
  /// If no time given, current time is used.
  /// Optionally UTC is added to the end of string.
  /// This is static on the class, and is thread-safe.

  static string str(const time_t mytime = 0, const bool utc_label = true);

  /// Given a time, returns string.
  /// of format:  "YYYY/MM/DD HH:MM:SS"
  ///       e.g.  "1958/12/28 10:57:00"
  /// If no time given, current time is used.
  /// This is static on the class, and is thread-safe.

  static string strn(const time_t mytime = 0) { return str(mytime, false); }

  /// Given a time, returns string.
  /// of format:  "YYYY/MM/DD HH:MM:SS"
  ///       e.g.  "1958/12/28 10:57:00"
  /// If mytime == 0, returns "===== NOT SET ====="

  static string strm(const time_t mytime);

  /// as string
  /// date is / delimited
  /// specify the number of digits of precision in the sub-seconds

  string asString(int subsecPrecision = -1) const;

  /// as string
  /// date is - delimited
  /// specify the number of digits of precision in the sub-seconds

  string asStringDashed(int subsecPrecision = -1) const;

  /// Get year

  int getYear() const;

  /// Get month, 1-12

  int getMonth() const;

  /// Get day, 1-31

  int getDay() const;

  /// Get hour, 0-23

  int getHour() const;

  /// Get minute, 0-59

  int getMin() const;

  /// Get second, 0-59

  int getSec() const;

  /// Get sub seconds

  double getSubSec() const;

  /// Get day of year, 1-366

  int getDayOfYear() const;

  /// Get days in month, 28-31

  int getDaysInMonth() const;
  
  /// Get date/time elements which are non-null
  
  void getAll(int *year = NULL,
              int *month = NULL,
              int *day = NULL,
              int *hour = NULL,
              int *min = NULL,
              int *sec = NULL) const;
  
  /// Get date/time elements which are non-null
  
  void getAll(short *year = NULL,
              short *month = NULL,
              short *day = NULL,
              short *hour = NULL,
              short *min = NULL,
              short *sec = NULL) const;
  
  /// Get time elements which are non-null
  
  void getTime(int *hour = NULL,
               int *min = NULL,
               int *sec = NULL) const;

  /// Get the month and day of the month, given the year and day of year.

  static void getMonthDay(const int year, 
                          const int dofy, 
                          int *month, 
                          int *day);

  /// Get the month and day of the month, given the year and day of year.

  static void getMonthDay(const int year, 
			   const int dofy, 
			   int &month, 
			   int &day);

  /// get stored time as secs in double precision - includes sub secs.

  inline double asDouble() const
  {
    return (double) _uTime + _subSec;
  }
  
  /// get current time in double precision - includes sub secs.

  static double getCurrentTimeAsDouble();

  //@}

  /// \name Operators:
  //@{

  RadxTime& operator=  (const time_t when)
  { set(when); return(*this); }
  RadxTime& operator=  (const string& when)
  { set(when); return(*this); }
  RadxTime& operator=  (const RadxTime& when)
  { copy(when); return(*this); }

  RadxTime& operator+= (double secs);
  RadxTime& operator-= (double secs);

  RadxTime  operator+ (double secs) const;
  RadxTime  operator- (double secs) const;

  double    operator- (const RadxTime& other) const;

  bool      operator<  (time_t when) const;
  bool      operator<= (time_t when) const;
  bool      operator== (time_t when) const;
  bool      operator!= (time_t when) const;
  bool      operator>  (time_t when) const;
  bool      operator>= (time_t when) const;

  bool      operator<  (double when) const;
  bool      operator<= (double when) const;
  bool      operator== (double when) const;
  bool      operator!= (double when) const;
  bool      operator>  (double when) const;
  bool      operator>= (double when) const;

  bool      operator<  (const RadxTime &other) const;
  bool      operator<= (const RadxTime &other) const;
  bool      operator== (const RadxTime &other) const;
  bool      operator!= (const RadxTime &other) const;
  bool      operator>  (const RadxTime &other) const;
  bool      operator>= (const RadxTime &other) const;

  friend ostream& operator<< (ostream&, const RadxTime&);

  //@}

  const static int RADX_SECS_IN_HOUR = 3600;
  const static int RADX_SECS_IN_DAY = 86400;
  const static int RADX_JAN_1_1970 = 2440587;
  
protected:
private:

  // time state

  time_t _uTime;
  double _subSec;

  // string from which to return time representation
  
  mutable string strTime;

  // initialize

  void _init();

  // scan from w3c string

  int _scanW3c(const string &strWhen);
  
  // tokenize string for setting time from string
  
  static void tokenizeString(const string when,
                             int &YYYY, int &MM, int &DD,
                             int &hh,  int &mm, int &ss,
                             double &subSec);

  // normalize subSec
  
  void _normalize();

  /* uunix_time()
   * return the unix time from date_time_t struct
   * Side-effect: sets the unix_time field in date_time.
   */
  
  typedef struct {
    int year, month, day, hour, min, sec;
    time_t unix_time;
  } date_time_t;
  
  static time_t uunix_time(date_time_t &date_time);

  /* udate_time()
   * return pointer to date_time struct corresponding to unix time.
   * The pointer refers to a static held by this routine.
   */
  
  static date_time_t udate_time(time_t unix_time);

  /* uconvert_to_utime()
   * return the unix time from date_time_t struct.  Also sets the unix_time
   * field in the date_time_t structure.
   */

  static time_t uconvert_to_utime(date_time_t &date_time);

  /* uconvert_from_utime()
   * sets the other fields in the date_time_t structure based on the value
   * of the unix_time field in that structure
   */

  static void uconvert_from_utime(date_time_t &date_time);

  /* ujulian_date()
   * calculate the Julian calendar day number
   */
  
  static long ujulian_date(int day, int month, int year);
  
  /* ucalendar_date()
   * calculate the calendar day from the Julian date
   */
  
  static void ucalendar_date(long jdate, int *day,
                             int *month, int *year);
  
  /* ulocaltime()
   * puts the local time into a date_time_t struct
   */
  
  static void ulocaltime(date_time_t &date_time);
  
  /* ugmtime()
   * puts the gm time into a date_time_t struct
   */
  
  static void ugmtime(date_time_t &date_time);

#ifdef JUNK
  
  /* utime_compute()
   *
   * return the unix time from components.
   */
  
  static time_t utime_compute(int year, int month, int day,
                              int hour, int min, int sec);

  /* utimestr()
   * returns a string composed from the time struct. This routine has a
   * number of static storage areas, and loops through these areas. Every
   * 10 times you make the call you will overwrite a previous result.
   */
  
  static string utimestr(const date_time_t &date_time);
  
  /* utimstr()
   * returns a string composed from the time value. This routine calls
   * utimestr and, thus, uses the same static storage areas.
   */
  
  static string utimstr(time_t time);
  
  /* utime_str()
   * Same as utimestr(), except puts an underscore between the date and
   * time.
   */
  
  static string utime_str(const date_time_t &date_time);
  
  /* utim_str()
   * same as utimstr(), except calls utime_str().
   */
  
  static string utim_str(time_t time);

  /***************************************************************
   * uvalid_datetime.c
   */
  
  /* uvalid_datetime()
   * returns 1 if a valid date and time is passed, 0 otherwise.
   */
  
  static int uvalid_datetime(date_time_t *datetime);

#endif

};
  
#endif
