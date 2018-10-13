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
/* UTIM.c **/

#include <stdio.h>
#include <sys/time.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>

/* days in the month */
static int Dom[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*************************************************************************
 *	JULIAN_DATE: Calc the Julian calandar Day Number
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

long UTIMjulian_date(int day, int month, int year)
{
    long 	a,b;
    double	yr_corr;

    /* correct for negative year */
    yr_corr = (year > 0? 0.0: 0.75);
    if(month <=2) {
	year--;
	month += 12;
    }
    b=0;

    /* Cope with Gregorian Calandar reform */
    if(year * 10000.0 + month * 100.0 + day >= 15821015.0) {
	a = year / 100;
	b = 2 - a + a / 4;
    }
    return((365.25 * year - (double)yr_corr) + 
	   (long) (30.6001 * ((long)month +1)) + (long)day + 1720994L + b);
}

/*************************************************************************
 *	CALENDAR_DATE: Calc the calandar Day from the Julian date
 *	As Taken from Computer Language- Dec 1990, pg 58
 */

void UTIMcalendar_date(long jdate,long *day, long *month, long *year)
{
	long	a,b,c,d,e,z,alpha;

	z = jdate +1;

	/* Gregorian reform correction */
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
	*day = (int) b - d - (long) (30.6001 * e);
	*month = (int) (e < 13.5)? e - 1 : e - 13;
	*year = (int) (*month > 2.5)? (c - 4716) : c - 4715;
}


time_t UTIMdate_to_unix( UTIMstruct *date)
{
	long	u_day,day,days;
	long	u_time;

	u_day = UTIMjulian_date(1,1,1970);
	day = UTIMjulian_date(date->day,date->month,date->year);

	days = day - u_day;

	u_time = (days * 86400) + (date->hour * 3600) + (date->min * 60) + date->sec;

	date->unix_time = u_time;

	return u_time;
}

/*************************************************************************
 *
 */

void UTIMunix_to_date(time_t utime, UTIMstruct *date)
{
	long	u_day, day;

	u_day = UTIMjulian_date(1,1,1970);

	day = (utime / 86400);
	if ( day < 0 ) day--;
	
	UTIMcalendar_date((u_day + day), &date->day, &date->month, &date->year);

	day = (utime % 86400);
	if ( day < 0 ) day += 86400;
	
	date->hour = day / 3600;
	date->min = (day / 60) - (date->hour * 60);
	date->sec = day % 60;
	date->unix_time = utime;

}
 
time_t UTIMtime6_to_unix(UTIMtime6 * time6)
    /* convert old "time6" structure to unix time 
	return the unix time */
    {
	UTIMstruct date;

	date.year = time6->year + 1900;
	date.month = time6->month;
	date.day = time6->day;
	date.hour = time6->hour;
	date.min = time6->minute;
	date.sec = time6->second;

	return UTIMdate_to_unix( &date);

    }

void UTIMunix_to_time6( time_t utime, UTIMtime6 *date)
    /* convert unix time to old "time6" structure */
    {
	UTIMstruct us;

	UTIMunix_to_date( utime, &us);

	date->month = us.month;
	date->day = us.day;
	date->year = us.year - 1900;
	date->hour = us.hour;
	date->minute = us.min;
	date->second = us.sec;
    }


int UTIMdayofyear( UTIMstruct *date)
    /* return the day of the year for this date */
    {
	int i, doy;

	doy = date->day;
	for (i=0; i<date->month-1; i++)
	    doy += Dom[i];
	
	/* leap year */
	if (((date->year % 4) == 0) && (date->month > 2))
	   doy++;

	return doy;
    }

void UTIMmonthday( int year, int doy, int *month, int *day)
    /* return the month, day knowing the year and the day of year */
    {
	int m = 1;

	/* leap year adjustment */
	if ((year % 4) == 0)
	{
	  if ((year % 100) == 0)
	  {
	    if ((year % 400) == 0)
	    {
	      Dom[1] = 29;
	    }
	    else
	    {
	      Dom[1] = 28;
	    }
	  }
	  else
	  {
	    Dom[1] = 29;
	  }
	}
	
	while (doy > Dom[m-1])
	    {
	    doy -= Dom[m-1];
	    m++;
	    }
	*day = doy;
	*month = m;

	Dom[1] = 28;
    }


/********************************************************************/

unsigned long UTIMtime_diff( int start)
   /* returns time differences in millisecs.
      call with start = 1 to begin, then subsequent calls
      return the (wall clock) time difference since that starting
      point in millisecs.
    */
    {
	static struct timeval start_tv;
	struct timeval  tv;
	long diff;

	if (start)
	    {
	    gettimeofday( &start_tv, (struct timezone *) NULL);
	    return 0;
	    }

	if (0 != gettimeofday( &tv, (struct timezone *) NULL))
	    return 0;

	diff = (tv.tv_sec - start_tv.tv_sec) * 1000;
	diff += (tv.tv_usec - start_tv.tv_usec) / 1000;
	return (unsigned long) diff;
    }

/**************************************************************************
 * char *UTIMstr()
 *
 * returns a string composed from the time struct. This routine has a
 * number of static storage areas, and loops through these areas. Every
 * 10 times you make the call you will overwrite a previous result.
 *
 **************************************************************************/

char *UTIMstr(time_t utime)
{

  static char time_buf[10][32];
  static int count = 0;

  char *timestr;
  UTIMstruct udate;

  UTIMunix_to_date(utime, &udate);

  count++;
  
  if (count > 9)
    count = 0;

  timestr = time_buf[count];

  sprintf(timestr, "%.4ld/%.2ld/%.2ld %.2ld:%.2ld:%.2ld",
	  udate.year,
	  udate.month,
	  udate.day,
	  udate.hour,
	  udate.min,
	  udate.sec);

  return (timestr);

}

/*************************************************************************
 * UTIMstring_US_to_time:   Parse US format time string to time structure.
 * Returns time_t representation or 0 on error .
 * Using US time string conventions:  hour:min:sec month/day/year
 *                                    hour:min month/day/year
 *                                    hour:min month/day
 *                                    hour month/day
 *                                    hour:min 
 *                                    hour
 */                                                 
            
time_t UTIMstring_US_to_time(const char *string)
{
    int    num_fields;
    double  field[16];
    UTIMstruct tim_s;
    struct tm *tm;
    time_t now;
        
        
    num_fields = STRparse_double(string, field, 40, 16);

    switch(num_fields) {                                   
	default: 
	    return 0;
	break;

        case 6:     /* hour:min:sec month/day/year */                  
        
            tim_s.hour = (int)(field[0]) % 24;
            tim_s.min = (int)(field[1]) % 60;
            tim_s.sec = (int)(field[2]) % 60;
        
            tim_s.month = (int)(field[3]) % 13;
            tim_s.day = (int)(field[4]) % 32;
            
            if(field[5] < 70 ) field[5] += 2000;
            if(field[5] < 1900) field[5] += 1900;                                                         
            tim_s.year = (int)(field[5]);
        break;
        
        case 5:     /* hour:min month/day/year */
                
            tim_s.hour = (int)(field[0]) % 24;
            tim_s.min = (int)(field[1]) % 60;
            tim_s.sec = 0;             

            tim_s.month = (int)(field[2]) % 13;
            tim_s.day = (int)(field[3]) % 32;
            if(field[4] < 90 ) field[4] += 2000;
            if(field[4] < 1900) field[4] += 1900;
            tim_s.year = (int)(field[4]);
        break;

        case 4:     /* hour:min month/day */
   
            tim_s.hour = (int)(field[0]) % 24;
            tim_s.min = (int)(field[1]) % 60;
            tim_s.sec = 0;             
	     
            tim_s.month = (int)(field[2]) % 13;
            tim_s.day = (int)(field[3]) % 32;
            now = time(0);    /* Now is used to fill in missing values */
            tm = gmtime(&now);
	    tim_s.year = tm->tm_year + 1900;
    
        break;

        case 3:     /* hr month/day */
            tim_s.hour = (int)(field[0]) % 60;
            tim_s.min = 0;
            tim_s.sec = 0;             
            
            tim_s.day = (int) field[2];
	    tim_s.month = (int) field[1];
            now = time(0);    /* Now is used to fill in missing values */
            tm = gmtime(&now);
	    tim_s.year = tm->tm_year + 1900;

        break;
	 
        case 2:     /* hr:min */
            tim_s.hour = (int)(field[0]) % 60;
            tim_s.min = (int)(field[1]) % 60;
            tim_s.sec = 0;             
	     
            now = time(0);    /* Now is used to fill in missing values */
            tm = gmtime(&now);
            tim_s.day = tm->tm_mday;
	    tim_s.month = tm->tm_mon + 1;
	    tim_s.year = tm->tm_year + 1900;

        break;

        case 1:     /* hr */
            tim_s.hour = (int)(field[0]) % 60;
            tim_s.min = 0;
            tim_s.sec = 0;             
            tm = gmtime(&now);
	    tim_s.day = tm->tm_mday;
	    tim_s.month = tm->tm_mon + 1;
	    tim_s.year = tm->tm_year + 1900;
        break;
    }

    return(UTIMdate_to_unix(&tim_s));
}
