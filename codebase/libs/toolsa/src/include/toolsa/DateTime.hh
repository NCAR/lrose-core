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
// Utility class for manipulating date/time
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1999
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DATE_TIME_HH_
#define _DATE_TIME_HH_

#ifndef DeltaTimeINCLUDED
# include <toolsa/DeltaTime.hh>
#endif

#include <string>
#include <cstring>
#include <ctime>
#include <toolsa/udatetime.h>
using namespace std;


class DateTime {

public:

  static const time_t NEVER;

  DateTime();
  DateTime(time_t when);
  DateTime(time_t when, bool setSubSec, double subSec);
  DateTime(time_t when, size_t leadSec);
  DateTime(const char *when);
  DateTime(const string &when);
  DateTime(int year, int month, int day,
           int hour = 0, int min = 0, int sec = 0,
           double subSec = 0.0);
  DateTime(const DateTime & orig);
  ~DateTime();

  ///////////////////////////////////////////////////////////////////////////
  // Setting the time (see also the class operators for setting time)
  //
  // Support setting time using a string of the format: "YYYY-MM-DD-HH-MM-SS"
  // where: "-" represents a valid delimeter [/_: ]
  // e.g. "1958/12/28 10:57:00"
  // e.g. "19581228105700"
  //
  ///////////////////////////////////////////////////////////////////////////

  void setToNow(); // sets to subsec precision, using getTimeOfDay()

  void set(time_t when);
 
  void set(time_t when, double subSec);

  void set(time_t when, size_t leadSecs);

  time_t set(const char *when);

  time_t set(const string &when)
  { return(set(when.c_str())); }
 
  time_t set(int year, int month, int day,
             int hour = 0, int min = 0, int sec = 0,
             double subSec = 0.0);
 
  time_t setTime(int hour = 0, int min = 0, int sec = 0,
                 double subSec = 0.0);

  time_t setYear(int year);
  time_t setMonth(int month);
  time_t setDay(int day);
  time_t setHour(int hour);
  time_t setMin(int min);
  time_t setSec(int sec);
  void setSubSec(double val);
  time_t setByDayOfYear(int year, int dofy,
                        int hour = 0, int min = 0, 
                        int sec = 0);
 
  void setLeadSecs(size_t leadSecs);
  void setLeadDeltaTime(const DeltaTime * delta);
  void copy(const DateTime &source);

  // scan the input string for conformance to W3C ISO 8601
  // See http://www.w3.org/TR/NOTE-datetime
  // returns 0 on success, -1 on failure
  // If requireSeconds is true, we expect
  // year, month, day, hour, min and sec to all be present.
  
  int setFromW3c(const char *strWhen, bool requireSeconds = false);
 
  // set the time from input string.
  // Accepted are:
  //   *yyyymmdd?hhmmss*
  //   *yyyymmddhhmmss*
  // returns 0 on success, -1 on failure
  
  int setFromYYYYMMDDHHMMSS(const char *strWhen);

  ///////////////////////////////////////////////////////////////////////////
  // Determine if the time is a forecast time (has a lead time).
  ///////////////////////////////////////////////////////////////////////////
  bool isForecastTime() const { return (_leadTime != NULL); }

  // Returns NULL if this is not a forecast time.
  const DeltaTime *getLeadDeltaTime() const { return _leadTime; }

  ///////////////////////////////////////////////////////////////////////////
  // Getting the time
  ///////////////////////////////////////////////////////////////////////////

  bool isValid() const { return _uTime != NEVER; }
  time_t utime() const { return _uTime; }
  time_t forecastUtime() const;

  // return w3c standard time string YYYY-MM-DDTHH:MM:SSZ
  // eg. 1993-0630T21:49:08Z

  string getW3cStr() const;

  // strftime library string
  // Implements the strftime stdC function, but returns a string.
  // Returns an empty string if conversion fails.
  // http://www.cplusplus.com/reference/clibrary/ctime/strftime

  string strfTime(const char* format, bool use_gmt = true) const;
  string strfTime(const string& format, bool use_gmt = true) const;

  // strptime library time_t
  // Implements the strftime stdC function, but returns a string.
  // Returns -1 if conversion fails.
  //
  // the add_seconds flag appends '_%S' to the end of format and '_00' to time_str. 
  // Use this option if the time format does not include seconds. strptime requires a
  // fully defined time to work correctly
  //
  // the UTC option indicates that the time locale to use is UTC. Otherwise
  // the default locale is used, and you will get two different returned values
  // for the returned time_t if the code is run on one machine that is set to
  // a default locale of Mountain Daylight Time (MDT) and a machine that is set to use the
  // Universal Time Convention (UTC) locale. Setting UTC to TRUE is highly recommended (it
  // defaults to FALSE for backward compatibility). Niles Oien July 2011.

  static time_t strpTime(const char* format, const char* time_str, bool add_seconds = false, bool UTC=false);
  static time_t strpTime(const string& format, const string& time_str, bool add_seconds = false, bool UTC=false);

  // stdC library string 
  // Returns string of format: "Day Mon DD HH:MM:SS YYYY\n"

  const char* ctime() const;

  // stdC library string - returns time as UTC
  // Returns string of format: "Day Mon DD HH:MM:SS YYYY\n"
  // e.g. "Wed Jun 30 21:49:08 1993\n"

  const char* gtime() const;

  // didss string
  // Returns string of format: "YYYY/MM/DD HH:MM:SS"
  // e.g. "1958/12/28 10:57:00"

  const char* dtime() const;

  // KML string
  // Returns string of format: "yyyy-mm-ddThh:mm:ss"
  // e.g. "1958-12-28T10:57:00"

  const char* kmltime() const;

  // Return string representing date and time
  // of format: "YYYY/MM/DD HH:MM:SS"
  // e.g. "1958/12/28 10:57:00"
  // Optionally UTC is added to the end of string.
  string getStr(bool utc_label = true) const;
  string getStrn() const { return getStr(false); }

  // Returns plain string representing date and time
  // of format: "YYYYMMDDHHMMSS"
  // e.g. "19581228105700"
 
  string getStrPlain() const;

  // Return string representing date only
  // of format: "YYYY/MM/DD"
  // e.g. "1958/12/28"
  string getDateStr() const;

  // Return plain string representing date only
  // of format: "YYYYMMDD"
  // e.g. "19581228"
  string getDateStrPlain() const;

  // Return string representing date only
  // of format: "MM/DD/YYYY"
  // e.g. "12/28/1958"
  string getDateStrMDY() const;

  // Return string representing time only
  // of format: "HH:MM:SS"
  // e.g. "10:57:00"
  // Optionally UTC is added to the end of string.
  string getTimeStr(bool utc_label = true) const;

  // Return plain string representing time only
  // of format: "HHMMSS"
  // e.g. "105700"
 
  string getTimeStrPlain() const;

  // Given a time, returns string.
  // of format: "YYYY/MM/DD HH:MM:SS"
  // e.g. "1958/12/28 10:57:00"
  // If no time given, current time is used.
  // Optionally UTC is added to the end of string.
  // This is static on the class, and is thread-safe.

  static string str(const time_t mytime = 0, const bool utc_label = true);
  static string strn(const time_t mytime = 0) { return str(mytime, false); }

  // Given a time, returns string.
  // of format: "YYYY/MM/DD HH:MM:SS"
  // e.g. "1958/12/28 10:57:00"
  // If mytime == 0, returns "===== NOT SET ====="

  static string strm(const time_t mytime);

  // Given a time, returns string with underscore instead of space.
  // of format: "YYYY/MM/DD_HH:MM:SS"
  // e.g. "1958/12/28 10:57:00"
  // If mytime == 0, returns "=====_NOT_SET_====="

  static string stru(const time_t mytime);

  // Given a time, returns string with one space between fields of format:
  //  "YYYY MM DD HH MM SS"
  // e.g. "1958 12 28 10 57 00"

  static string strs(const time_t mytime = 0);

  ////////////////
  // get as string
  
  string asString(int subsecPrecision = 6) const;

  //////////////////////
  // get as double secs
  
  double asDouble() const;

  // Get time components.
  // No weird zero-based where not expected (year, hour, min, sec only!)

  int getYear() const;
  int getMonth() const;
  int getDay() const;
  int getHour() const;
  int getMin() const;
  int getSec() const;
  double getSubSec() const { return _subSec; }
  int getDayOfYear() const;
  int getDaysInMonth() const;
 
  // Get the day of the week. 0 = Sunday ... 6 = Saturday

  int getDayOfWeek() const;
 
  // fill out any args supplied

  void getAll(int *year = NULL,
              int *month = NULL,
              int *day = NULL,
              int *hour = NULL,
              int *min = NULL,
              int *sec = NULL) const;

  void getAll(short *year = NULL,
              short *month = NULL,
              short *day = NULL,
              short *hour = NULL,
              short *min = NULL,
              short *sec = NULL) const;

  void getTime(int *hour = NULL,
               int *min = NULL,
               int *sec = NULL) const;

  // get stored time as double secs
  
  double getTimeAsDouble() const;
    
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

  static time_t parseDateTime(const char *when,
                              int *year = NULL, 
                              int *month = NULL, 
                              int *day = NULL,
                              int *hour = NULL, 
                              int *min = NULL, 
                              int *sec = NULL,
                              double *subSec = NULL);

  // Determinr the month and day of the month, given the year and day of year
  //
  // NOTE: This is a copy of UTIMmonthday.

  static void getMonthDay(const int year, 
                          const int dofy, 
                          int *month, 
                          int *day);

  static void getMonthDay(const int year, 
                          const int dofy, 
                          int &month, 
                          int &day);

  // get current time in double precision - includes partial secs

  static double getCurrentTimeAsDouble();

  // operators

  DateTime& operator= (const time_t when)
  { set(when); return(*this); }
  DateTime& operator= (const char* when)
  { set(when); return(*this); }
  DateTime& operator= (const string& when)
  { set(when.c_str()); return(*this); }
  DateTime& operator= (const DateTime& when)
  { copy(when); return(*this); }

  DateTime& operator+= (double secs);
  DateTime& operator+= (const DeltaTime& delta);

  DateTime& operator-= (double secs);
  DateTime& operator-= (const DeltaTime& delta);

  DateTime operator+ (const DeltaTime& delta) const;
  DateTime operator+ (const double secs) const;
  DateTime operator- (const DeltaTime& delta) const;
  DateTime operator- (const double secs) const;
  double operator- (const DateTime& other) const;

  bool operator< (time_t when) const
  { return(_uTime < when ? true : false); }
  bool operator<= (time_t when) const
  { return(_uTime <= when ? true : false); }
  bool operator== (time_t when) const
  { return(_uTime == when ? true : false); }
  bool operator!= (time_t when) const
  { return(_uTime != when ? true : false); }
  bool operator> (time_t when) const
  { return(_uTime > when ? true : false); }
  bool operator>= (time_t when) const
  { return(_uTime >= when ? true : false); }

  bool operator< (const DateTime &other) const
  { return(_uTime < other._uTime ? true : false); }
  bool operator<= (const DateTime &other) const
  { return(_uTime <= other._uTime ? true : false); }
  bool operator== (const DateTime &other) const
  { return(_uTime == other._uTime ? true : false); }
  bool operator!= (const DateTime &other) const
  { return(_uTime != other._uTime ? true : false); }
  bool operator> (const DateTime &other) const
  { return(_uTime > other._uTime ? true : false); }
  bool operator>= (const DateTime &other) const
  { return(_uTime >= other._uTime ? true : false); }

  friend ostream& operator<< (ostream&, const DateTime&);
  friend ostream& operator<< (ostream&, const DateTime*);

private:

  time_t _uTime;
  double _subSec;
  DeltaTime *_leadTime;
  mutable string _strTime;

  void _init();
  void _init(const string &when);

  static void tokenizeString(const char *when,
                             int &YYYY, int &MM, int &DD,
                             int &hh, int &mm, int &ss,
                             double &subSec);

  void _normalize();

};

#endif
