#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include "rdr.h"
#include "rdrutils.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

/* 
 * juldate.c
 * 
 * Timezone conversion notes:
 * 
 * UTC-timezone=Local
 * Local+timezone=UTC
 * 
 * System variables:
 * timezone - main timezone correction,  not daylight
 * altzone  - daylight timezone correction
 * daylight - true if daylight savings period
 * 
 * NOTE - IN SOUTHERN HEMISPHERE DAYLIGHT SAVINGS NEEDS TO BE SET
 * AS THE MAIN TIMEZONE, AND STANDARD TIME AS THE ALTERNATE TO ALLOW
 * AUTO-HANDLING OF ALTZONE,  
 * BECAUSE ALTZONE START DATE MUST BE < ALTZONE END DATE
 * E.G. FOR MELBOURNE TZ=EDT-11:00EST-10:00:00,84/2:00:00,303/2:00:00
 */

int julian_day(int year,int month,int day) { 	// returns day since 
	float	y1;				// 1 Jan 4713 BC

	y1 = year + (month - 2.85) / 12;
	return(int(int(int(367.0 * y1) - int(y1) - 0.75 * int(y1) + day) - 0.75 * 2.0) + 1721115);
}

void julian_date(int jul_day, int &year, int &month, int &day) {
	float	N1,N2,Y1,M1;
	bool debug = FALSE;


	if (debug) printf("%d ",jul_day);
	N1 = (jul_day - 1721119) + 2;
	Y1 = int((N1 - 0.2) / 365.25);
	N2 = N1 - int(365.25 * Y1);
	M1 = int((N2 - 0.5) / 30.6);
	day = int(N2 - 30.6 * M1 + 0.5);
	year = int(Y1);
	if (M1 > 9) year++;
	N1 = 0;
	if (M1 > 9) N1 = 12;
	month = int(M1 + 3 - N1);
	if (debug) printf("%d/%d/%d\n",day,month,year);
	}

/*
 * The date integer used in the Rapic headers is
 * jjjyy
 * where jjj is the day of year
 * yy is 2digit year,  if yy < 80 use 19yy else 20yy
 */
void rpdate2ymd(int rpdate, int &year, int &month, int &day) {
    int	juldate, doy,yr;
 
    yr = rpdate % 100;
    if (yr > 80) yr +=1900;		// assume 80-99 = 1980-1999
    else yr += 2000;			// 00-80 = 2000-2080
    doy = rpdate / 100;			// get day of year
    juldate = julian_day(yr,1,1) + doy - 1;  // calc julian day from yr&day
    julian_date(juldate,year,month,day); // convert to year, month, day
    }

int DoY(int year, int month, int day) {
    return julian_day(year, month, day) - julian_day(year, 1, 1) + 1;
    }

// Convert I/P time to seconds since 1/1/70 UTC
time_t DateTime2UnixTime_mktime(int year,int month,int day,int hour,
			 int min,int sec, int InputLocalTime) {		
	
//	int	jday = 0;

tm	TmStruct;
time_t	time;

    TmStruct.tm_year = year - 1900;
    TmStruct.tm_mon = month - 1;
    TmStruct.tm_mday = day;
    TmStruct.tm_hour = hour;
    TmStruct.tm_min = min;
    TmStruct.tm_sec = sec;
    TmStruct.tm_isdst = daylight;
    time = mktime(&TmStruct); // ASSUMES I/P TIME IS LOCAL
    if (!InputLocalTime)    // mktime converts local to UTC by adding timezone
	time -= TimeZone(); // if I/P UTC, correct by subtracting timezone
    return(time);
    }

int jul_day_1_1_1970 = -1;
// Convert I/P time to seconds since 1/1/70 UTC
time_t DateTime2UnixTime(int year,int month,int day,int hour,
			   int min,int sec, int InputLocalTime) {		
	
  int	jday = 0;

  time_t	time;

  if (jul_day_1_1_1970 < 0)
    jul_day_1_1_1970 = julian_day(1970, 1, 1);
  jday = julian_day(year, month, day);
  time = (jday - jul_day_1_1_1970) * SecsPerDay;
  time += (((hour * 60) + min) * 60) + sec;

  if (InputLocalTime)    // mktime converts local to UTC by adding timezone
    time += TimeZone();  // if I/P UTC, correct by subtracting timezone
  return(time);
}

void UnixTime2DateTime(time_t UTime,int &year, int &month, int &day,
		       int &hour, int &min, int &sec,  int OutputLocalTime) {
  tm	TmStruct;
	
  if (OutputLocalTime) localtime_r(&UTime, &TmStruct);	
  else gmtime_r(&UTime, &TmStruct);
  year = TmStruct.tm_year + 1900;
  month = TmStruct.tm_mon + 1;
  day = TmStruct.tm_mday;
  hour = TmStruct.tm_hour;
  min = TmStruct.tm_min;
  sec = TmStruct.tm_sec;
}

void UnixTime2DateTimeStr(time_t UTime, int OutputLocalTime,  char *DateStr, char *TimeStr) {
int	yr,mn,dy,hr,min,sec;

	UnixTime2DateTime(UTime,yr,mn,dy,hr,min,sec, OutputLocalTime);
	if (DateStr)
		sprintf(DateStr,"%02d/%02d/%02d",dy,mn,yr%100);
	if (TimeStr)
		sprintf(TimeStr,"%02d:%02d:%02d",hr,min,sec);
	}

void UnixTime2DateTimeString(time_t UTime, int OutputLocalTime,  char *DateTimeStr) {
int	yr,mn,dy,hr,min,sec;

	UnixTime2DateTime(UTime,yr,mn,dy,hr,min,sec, OutputLocalTime);
	sprintf(DateTimeStr,"%02d/%02d/%02d %02d:%02d:%02d",dy,mn,yr%100,hr,min,sec);
	}

time_t DateTimeStr2UnixTime(int InputLocalTime, char *DateStr = 0, char *TimeStr = 0) {
int	yr,mn,dy,hr,min,sec;
	yr=mn=dy=hr=min=sec=0;
	if (DateStr) {
		sscanf(DateStr,"%02d/%02d/%02d",&dy,&mn,&yr);
		if (yr > 80)
			yr += 1900;
		else
			yr += 2000;
		}
	if (TimeStr)
		sscanf(TimeStr,"%02d:%02d:%02d",&hr,&min,&sec);
	if (DateStr) 
	    return DateTime2UnixTime(yr,mn,dy,hr,min,sec, InputLocalTime); 
	else return((hr * 3600) + (min * 60) + sec); 
	}

// Decode ISO 8601 time string
// e.g. 2005-02-01T00:00:00Z
time_t DateTimeStr8601_2UnixTime(char *str8601) {
  int	yr,mn,dy,hr,min,sec;
  bool InputLocalTime = false;
  yr=mn=dy=hr=min=sec=0;
  char tmzone = ' ';
  if (str8601) 
    {
      if (sscanf(str8601,"%04d-%02d-%02dT%02d:%02d:%02d%c",
		 &yr,&mn,&dy, &hr, &min, &sec, &tmzone) >= 6)
	{
	  if (tmzone == 'Z') 
	    InputLocalTime = false;
	  return DateTime2UnixTime(yr,mn,dy,hr,min,sec, InputLocalTime);
	}
    }
  return 0;
}

time_t DateTimeStr2UnixTime(char *DateTimeStr, bool InputLocalTime)  // decode yyyymmddhhmm[ss] string to time_t
{
  int	yr,mn,dy,hr,min,sec;
  yr=mn=dy=hr=min=sec=0;

  if (DateTimeStr) {
    sscanf(DateTimeStr,"%04d%02d%02d%02d%02d%02d",&yr,&mn,&dy,&hr,&min,&sec);
  }
  if (DateTimeStr) 
    return DateTime2UnixTime(yr,mn,dy,hr,min,sec, InputLocalTime); 
  else 
    return 0; 
}

time_t TimeZone() {
#ifdef sgi
	if (daylight) return altzone;
	else return timezone;
#else
	return timezone;
#endif
}

time_t CorrectLocalTime(int localtime) {
	if (localtime < 0) localtime = LocalTime;
	if (!localtime) return 0;
	else return TimeZone();
	}

time_t timet_day(time_t tm, int OutputLocalTime) {     /* truncate Unix Time to start of day */
	tm -= CorrectLocalTime(OutputLocalTime);
	return ((tm/SecsPerDay) * SecsPerDay);
	}
