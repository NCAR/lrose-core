/* ----------------------------------------------------------------------------
	Sample source code for Himawari Satandard Data

	Copyright (C) 2015 MSC (Meteorological Satellite Center) of JMA

	Disclaimer:
		MSC does not guarantee regarding the correctness, accuracy, reliability,
		or any other aspect regarding use of these sample codes.

	Detail of Himawari Standard Format:
		For data structure of Himawari Standard Format, prelese refer to MSC
		Website and Himawari Standard Data User's Guide.

		MSC Website
		http://www.jma-net.go.jp/msc/en/

		Himawari Standard Data User's Guide
		http://www.data.jma.go.jp/mscweb/en/himawari89/space_segment/hsd_sample/HS_D_users_guide_en.pdf

	History
		March,   2015  First release

---------------------------------------------------------------------------- */
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <time.h>
# include <math.h>
# include "date_utl.h"

/* ---------------------------------------------------------------------------
 *	DateIntsToMjd()
 * -------------------------------------------------------------------------*/
double DateIntsToMjd(const int date[7] ){
	double a, b, mjd, tmp;
    double year = date[0];
    double mon  = date[1];
    double day  = date[2];
    double hour = date[3];
    double min  = date[4];
    double sec  = date[5];
    double msec = date[6];
    double p = year + ( mon -1 ) / 12. + day / 365.25;

    if( mon <= 2 ) {
        mon += 12;
        year --;
    }
    if( p < 1582.78 )  b = 0;
    else {
        a = floor( year / 100. );
        b = 2 - a + floor( a / 4 );
    }
    tmp = floor( 365.25 * year ) + floor( 30.6001 * (mon+1) ) - 679006 + b;
    mjd = tmp + day + hour/24. + min/24./60 + sec/24./60/60
        + msec/24./60/60/1000;
    return mjd;
}
/* ---------------------------------------------------------------------------
 *	mjd_to_date()
 * -------------------------------------------------------------------------*/
void mjd_to_date(double mjd, int date[7]){
	double jd = mjd + 2400000.5;
	double m, aa, a, b, c, d, e;
	int    year, mon, day, hour, min, sec, msec,  tt;
	jd += 0.5;
	
    m = floor( jd );
    aa = m;
    if( m >= 2299161 ) {
        a = floor( (m - 1867216.25) / 36524.25 );
        aa = m + 1 + a - floor( a / 4 );
    }
    b = aa + 1524;
    c = floor( (b-122.1) / 365.25 );
    d = floor( 365.25 * c );
    e = floor( (b - d) / 30.6001 );
    year  = c - 4716;
    mon   = e - 1;
    day   = b - d - floor( 30.6001 * e );
    if( mon > 12 ) { mon -= 12; year++; }
    tt = (int)(( jd - floor( jd ) ) * 24 * 3600 * 1000 + 0.5 );
    hour =   tt / 3600 / 1000;
    min  = ( tt /   60 / 1000 ) % 60;
    sec  = ( tt / 1000 ) % 60;
    msec =   tt % 1000;
    date[0] = year;
    date[1] = mon;
    date[2] = day;
    date[3] = hour;
    date[4] = min;
    date[5] = sec;
    date[6] = msec;
} 
/* ---------------------------------------------------------------------------
 *	DateGetNowInts()
 * -------------------------------------------------------------------------*/
void DateGetNowInts(int  date[7] ){
    time_t     t;
    struct tm *tm_ptr;

    t = time( NULL ) ;
    tm_ptr = gmtime( &t );

    date[0] = tm_ptr->tm_year + 1900;
    date[1] = tm_ptr->tm_mon  + 1;
    date[2] = tm_ptr->tm_mday;
    date[3] = tm_ptr->tm_hour;
    date[4] = tm_ptr->tm_min;
    date[5] = tm_ptr->tm_sec;
    date[6] = 0;
}

