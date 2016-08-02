/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*************************************************************************
 * udatetime.c: Routines to do time/date convertions
 *
 * Obtained from Frank Hage
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 *************************************************************************/

#include <toolsa/udatetime.h>
#include <time.h>
#include <stdio.h>

extern time_t time(time_t *);

#define JAN_1_1970 2440587

/*************************************************************************
 * uunix_time()
 *
 * return the unix time from date_time_t struct
 *
 * Side-effect: sets the unix_time field in date_time.
 */

time_t uunix_time(date_time_t *date_time)
{

  long day, days;
  time_t u_time;

  day = ujulian_date(date_time->day,
		    date_time->month,
		    date_time->year);

  days = day - JAN_1_1970;

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

date_time_t *udate_time(time_t unix_time)
{

  static date_time_t date_time;

  date_time.unix_time = unix_time;
  uconvert_from_utime(&date_time);

  return (&date_time);

}
 
/*************************************************************************
 * uconvert_to_utime()
 *
 * return the unix time from date_time_t struct.  Also sets the unix_time
 * field in the date_time_t structure.
 */

time_t uconvert_to_utime(date_time_t *date_time)
{
  time_t u_time;

  u_time = uunix_time(date_time);
  
  date_time->unix_time = u_time;
  
  return u_time;

}

/*************************************************************************
 * utime_compute()
 *
 * return the unix time from components.
 */

time_t utime_compute(int year, int month, int day,
		     int hour, int min, int sec)
{

  date_time_t dtime;

  dtime.year = year;
  dtime.month = month;
  dtime.day = day;
  dtime.hour = hour;
  dtime.min = min;
  dtime.sec = sec;

  uconvert_to_utime(&dtime);
  
  return(dtime.unix_time);

}

/*************************************************************************
 * uconvert_from_utime()
 *
 * sets the other fields in the date_time_t structure based on the value
 * of the unix_time field in that structure
 */

void uconvert_from_utime(date_time_t *date_time)
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
  
  ucalendar_date((JAN_1_1970 + day),
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

long ujulian_date(int day, int month, int year)
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
	
  return ((365.25 * year - yr_corr) +
	  (long) (30.6001 * (month +1)) +
	  day + 1720994L + b);

}

/*************************************************************************
 *	CALENDAR_DATE: Calc the calendar Day from the Julian date
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

void ucalendar_date(long jdate, int *day,
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

void ulocaltime(date_time_t *date_time)
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
  uconvert_to_utime(date_time);

}

/**************************************************************************
 * void ugmtime()
 *
 * puts the gm time into a date_time_t struct
 *
 **************************************************************************/

void ugmtime(date_time_t *date_time)
{

  date_time->unix_time = time(NULL);
  uconvert_from_utime(date_time);

}

/**************************************************************************
 * char *utimstr()
 *
 * Calls utimestr() - see notes for this function.
 *
 **************************************************************************/

char *utimstr (time_t time)
{
  return (utimestr(udate_time(time)));
}

/**************************************************************************
 * char *utimestr()
 *
 * returns a string composed from the time struct. This routine has a
 * number of static storage areas, and loops through these areas. Every
 * 10 times you make the call you will overwrite a previous result.
 *
 **************************************************************************/

char *utimestr(date_time_t *date_time)
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

char *utim_str (time_t time)
{
  return (utime_str(udate_time(time)));
}

/**************************************************************************
 * char *utime_str()
 *
 * Same as utimestr(), except puts an underscore between the date and
 * time.
 *
 **************************************************************************/

char *utime_str(date_time_t *date_time)
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





