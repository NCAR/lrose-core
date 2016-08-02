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
#ifdef __cplusplus
 extern "C" {
#endif
#ifndef UTIM_WAS_INCLUDED
#define UTIM_WAS_INCLUDED


#include <time.h>

   typedef struct UTIMstruct_ {
     long	year;		/* YYYY e.g. 1992,2001 etc */
     long	month;		/* 1-12 */
     long	day;		/* 1-31 */
     long	hour;		/* 0-24 */
     long	min;		/* 0-60 */
     long	sec;		/* 0-60 */
     time_t	unix_time; 	/* secs since Jan 1, 1970 */
   } UTIMstruct;
   
   /* this is the Time6 structure found in prims.h */
   typedef struct UTIMtime6_  {
     short		month;
     short		day;
     short		year;
     short		hour;
     short		minute;
     short		second;
   } UTIMtime6;
   
   extern time_t UTIMdate_to_unix( UTIMstruct *date );
   /* convert UTIM date structure to unix time 
      return the unix time */
   
   extern time_t UTIMtime6_to_unix( UTIMtime6 *date );
   /* convert old "time6" structure to unix time 
      return the unix time */
   
   extern void UTIMunix_to_time6( time_t unix_time, UTIMtime6 *date );
   /* convert unix time to old "time6" structure */
   
   extern void UTIMunix_to_date( time_t unix_time, UTIMstruct *date );
   /* convert unix time to UTIM date structure */
   
   extern long UTIMjulian_date( int day, int month, int year );
   /* 	convert day, month, year to julian date
	return the julian date (= days from 0 BC ?)
	year = e.g. 1992
	month = 1-12
	day = 1-31
	*/
   
   extern void UTIMcalendar_date(long jdate,
				 long *day, long *month, long *year);
   /* convert julian date to day, month, year
      input : jdate = julian date
      output: year, month, day
      */
   
   
   extern int UTIMdayofyear( UTIMstruct *date);
   /* return the day of the year for this date */
   
   extern void UTIMmonthday( int year, int doy, int *month, int *day);
   /* return the month, day knowing the year and the day of year */
   
   extern unsigned long UTIMtime_diff( int start);
   /* returns time differences in millisecs.
      call with start = 1 to begin, then subsequent calls
      return the (wall clock) time difference since that starting
      point in millisecs.
      */
   
   extern char *UTIMstr(time_t utime);
   /*
    * returns a string composed from the time struct. This routine has a
    * number of static storage areas, and loops through these areas. Every
    * 10 times you make the call you will overwrite a previous result.
    */
   
   extern void tu_sleep(long usecs);
   /*
    * tu_sleep() - sleep in micro-secs
    */

   extern time_t UTIMstring_US_to_time(const char *string);
     /* UTIMstring_US_to_time:   Parse US format time string to time structure.
      * Returns time_t representation or 0 on error .
      * Using US time string conventions:  hour:min:sec month/day/year
      *                                    hour:min month/day/year
      *                                    hour:min month/day
      *	                                   hour month/day
      *	                                   hour:min
      *	                                   hour
      * Missing minutes or seconds - set to 0.
      * Missing day,year,month - set to present date
      */



   
#endif
#ifdef __cplusplus
}
#endif
