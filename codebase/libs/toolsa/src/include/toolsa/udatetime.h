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

/************
 * datetime.h
 ************/

#ifndef udatetime_h
#define udatetime_h

#ifdef __cplusplus
extern "C" {
#endif

/*
 * includes
 */

#include <toolsa/os_config.h>
#include <time.h>

/*
 * Define some commonly needed constants.
 */

#define SECS_IN_HOUR     3600
#define SECS_IN_DAY      86400
#define SECS_IN_12HRS    43200

/*
 * date and time struct with unix time
 */

typedef struct {
  int year, month, day, hour, min, sec;
  time_t unix_time;
} date_time_t;

/* uunix_time()
 * return the unix time from date_time_t struct
 * Side-effect: sets the unix_time field in date_time.
 */

extern time_t uunix_time(date_time_t *date_time);

/* udate_time()
 * return pointer to date_time struct corresponding to unix time.
 * The pointer refers to a static held by this routine.
 */

extern date_time_t *udate_time(time_t unix_time);

/* uconvert_to_utime()
 * return the unix time from date_time_t struct.  Also sets the unix_time
 * field in the date_time_t structure.
 */

extern time_t uconvert_to_utime(date_time_t *date_time);

/* uconvert_from_utime()
 * sets the other fields in the date_time_t structure based on the value
 * of the unix_time field in that structure
 */

extern void uconvert_from_utime(date_time_t *date_time);

/* utime_compute()
 *
 * return the unix time from components.
 */

time_t utime_compute(int year, int month, int day,
		     int hour, int min, int sec);

/* ujulian_date()
 * calculate the Julian calendar day number
 */

extern long ujulian_date(int day, int month, int year);

/* ucalendar_date()
 * calculate the calendar day from the Julian date
 */

extern void ucalendar_date(long jdate, int *day,
			   int *month, int *year);

/* ulocaltime()
 * puts the local time into a date_time_t struct
 */

extern void ulocaltime(date_time_t *date_time);

/* ugmtime()
 * puts the gm time into a date_time_t struct
 */

extern void ugmtime(date_time_t *date_time);

/* utimestr()
 * returns a string composed from the time struct. This routine has a
 * number of static storage areas, and loops through these areas. Every
 * 10 times you make the call you will overwrite a previous result.
 */

extern char *utimestr(date_time_t *date_time);

/* utimstr()
 * returns a string composed from the time value. This routine calls
 * utimestr and, thus, uses the same static storage areas.
 */

extern char *utimstr(time_t time);

/* utime_str()
 * Same as utimestr(), except puts an underscore between the date and
 * time.
 */

extern char *utime_str(date_time_t *date_time);

/* utim_str()
 * same as utimstr(), except calls utime_str().
 */

extern char *utim_str(time_t time);

/***************************************************************
 * uvalid_datetime.c
 */

/* uvalid_datetime()
 * returns 1 if a valid date and time is passed, 0 otherwise.
 */

extern int uvalid_datetime(date_time_t *datetime);


#ifdef __cplusplus
}
#endif

#endif
