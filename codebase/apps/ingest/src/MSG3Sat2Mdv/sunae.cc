/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * RCS info
 *   $Author: dettling $
 *   $Locker:  $
 *   $Date: 2016/04/26 23:55:15 $
 *   $Revision: 1.1 $
 *   $State: Exp $
 *   $Log: sunae.cc,v $
 *   Revision 1.1  2016/04/26 23:55:15  dettling
 *   *** empty log message ***
 *
 *   Revision 1.1  2014/03/11 20:10:50  dettling
 *   *** empty log message ***
 *
 *   Revision 1.3  2007-02-27 20:46:56  jcraig
 *   Addition of rcs tags.
 *
 *   Revision 1.1  2007/02/26 22:58:41  sgc
 *   Initial commit of NTDA v1.10
 *
 *   $Id: sunae.cc,v 1.1 2016/04/26 23:55:15 dettling Exp $
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*   Calculates the local solar azimuth and elevation angles, and
     the distance to and angle subtended by the Sun, at a specific
     location and time using approximate formulas in The Astronomical
     Almanac.  Accuracy of angles is 0.01 deg or better (the angular
     width of the Sun is about 0.5 deg, so 0.01 deg is more than
     sufficient for most applications).

     Unlike many GCM (and other) sun angle routines, this
     one gives slightly different sun angles depending on
     the year.  The difference is usually down in the 4th
     significant digit but can slowly creep up to the 3rd
     significant digit after several decades to a century.

     A refraction correction appropriate for the "US Standard
     Atmosphere" is added, so that the returned sun position is
     the APPARENT one.  The correction is below 0.1 deg for solar
     elevations above 9 deg.  To remove it, comment out the code
     block where variable REFRAC is set, and replace it with
     REFRAC = 0.0.

   References:

     Michalsky, J., 1988: The Astronomical Almanac's algorithm for
        approximate solar position (1950-2050), Solar Energy 40,
        227-235 (but the version of this program in the Appendix
        contains errors and should not be used)

     The Astronomical Almanac, U.S. Gov't Printing Office, Washington,
        D.C. (published every year): the formulas used from the 1995
        version are as follows:
        p. A12: approximation to sunrise/set times
        p. B61: solar elevation ("altitude") and azimuth
        p. B62: refraction correction
        p. C24: mean longitude, mean anomaly, ecliptic longitude,
                obliquity of ecliptic, right ascension, declination,
                Earth-Sun distance, angular diameter of Sun
        p. L2:  Greenwich mean sidereal time (ignoring T^2, T^3 terms)


   Authors:  Dr. Joe Michalsky (joe@asrc.albany.edu)
             Dr. Lee Harrison (lee@asrc.albany.edu)
             Atmospheric Sciences Research Center
             State University of New York
             Albany, New York

   WARNING:  Do not use this routine outside the year range
             1950-2050.  The approximations become increasingly
             worse, and the calculation of Julian date becomes
             more involved.

   Input:

      YEAR     year (INTEGER; range 1950 to 2050)

      DAY      day of year at LAT-LONG location (INTEGER; range 1-366)

      HOUR     hour of DAY [GMT or UT] (REAL; range -13.0 to 36.0)
               = (local hour) + (time zone number)
                      + (Daylight Savings Time correction; -1 or 0)
               where (local hour) range is 0 to 24,
               (time zone number) range is -12 to +12, and
               (Daylight Time correction) is -1 if on Daylight Time,
               0 otherwise;  Example: 8:30 am Eastern Daylight Time
               would be
                           HOUR = 8.5 + 5 - 1 = 12.5

      LAT      latitude [degrees]
               (REAL; range -90.0 to 90.0; north is positive)

      LONG     longitude [degrees]
               (REAL; range -180.0 to 180.0; east is positive)


   Output:

      AZ       solar azimuth angle (measured east from north,
               0 to 360 degs)

      EL       solar elevation angle [-90 to 90 degs];
               solar zenith angle = 90 - EL

      SOLDIA   solar diameter [degs]

      SOLDST   distance to sun [Astronomical Units, AU]
               (1 AU = mean Earth-sun distance = 1.49597871E+11 m
                in IAU 1976 System of Astronomical Constants)


   Local Variables:

     DEC       Declination (radians)

     ECLONG    Ecliptic longitude (radians)

     GMST      Greenwich mean sidereal time (hours)

     HA        Hour angle (radians, -pi to pi)

     JD        Modified Julian date (number of days, including
               fractions thereof, from Julian year J2000.0);
               actual Julian date is JD + 2451545.0

     LMST      Local mean sidereal time (radians)

     MNANOM    Mean anomaly (radians, normalized to 0 to 2*pi)

     MNLONG    Mean longitude of Sun, corrected for aberration

     OBLQEC    Obliquity of the ecliptic (radians)

     RA        Right ascension  (radians)

     REFRAC    Refraction correction for US Standard Atmosphere (degs)


   Uses double precision for safety and because Julian dates can
   have a large number of digits in their full form (but in practice
   this version seems to work fine in single precision if you only
   need about 3 significant digits and aren't doing precise climate
   change or solar tracking work).
 --------------------------------------------------------------------

   Why does this routine require time input as Greenwich Mean Time
   (GMT; also called Universal Time, UT) rather than "local time"?
   Because "local time" (e.g. Eastern Standard Time) can be off by
   up to half an hour from the actual local time (called "local mean
   solar time").  For society's convenience, "local time" is held
   constant across each of 24 time zones (each technically 15 longitude
   degrees wide although some are distorted, again for societal
   convenience).  Local mean solar time varies continuously around a
   longitude belt;  it is not a step function with 24 steps.
   Thus it is far simpler to calculate local mean solar time from GMT,
   by adding 4 min for each degree of longitude the location is
   east of the Greenwich meridian or subtracting 4 min for each degree
   west of it.

 --------------------------------------------------------------------

   TIME

   The measurement of time has become a complicated topic.  A few
   basic facts are:

   (1) The Gregorian calendar was introduced in 1582 to replace
   Julian calendar; in it, every year divisible by four is a leap
   year just as in the Julian calendar; but centurial years must be
   exactly divisible by 400 to be leap years.  Thus 2000 is a leap
   year, but not 1900 or 2100.

   (2) The Julian day begins at Greenwich noon whereas the calendar
   day begins at the preceding midnight;  and Julian years begin on
   "Jan 0" which is really Greenwich noon on Dec 31.  True Julian
   dates are a continous count of day numbers beginning with JD 0 on
   1 Jan 4713 B.C.  The term "Julian date" is widely misused and few
   people understand it; it is best avoided unless you want to study
   the Astronomical Almanac and learn to use it correctly.

   (3) Universal Time (UT), the basis of civil timekeeping, is
   defined by a formula relating UT to GMST (Greenwich mean sidereal
   time).  UTC (Coordinated Universal Time) is the time scale
   distributed by most broadcast time services.  UT, UTC, and other
   related time measures are within a few sec of each other and are
   frequently used interchangeably.

   (4) Beginning in 1984, the "standard epoch" of the astronomical
   coordinate system is Jan 1, 2000, 12 hr TDB (Julian date
   2,451,545.0, denoted J2000.0).  The fact that this routine uses
   1949 as a point of reference is merely for numerical convenience.
 --------------------------------------------------------------------*/

#include "sunae.h"

#ifdef TEST
#include <stdio.h>
#endif

double sunae(ae_pack *aep)
{
    double	delta, leap, jd, tm, mnlong, mnanom, eclong, oblqec,
		num, den, ra, lat, sd, cd, sl, cl, gmst, lmst, refrac,
		el_deg, tst,
		airmass(double el);
    int		year;

    year = aep->year < 100 ? aep->year+1900 : aep->year;

#ifdef TEST
fprintf(stderr, "yr=%d doy=%d lat=%.3f lon=%.3f hour=%.3f\n", year, aep->doy,
	aep->lat, aep->lon, aep->hour);
#endif

    delta = year - 1949.0;
    leap = (int) (0.25 * delta);
    jd = 32916.5 + delta*365.0 + leap + aep->doy + aep->hour/24.0;

    /*
    ** "the last year of a century is not a leap year therefore"
    */
    if (year % 100 == 0)
	jd -= 1.0;

    tm = jd - 51545.0;
#ifdef TEST
fprintf(stderr, "delta=%f leap=%f jd=%f tm=%f\n", delta, leap, jd, tm);
#endif
    mnlong = fmod(280.460 + 0.9856474*tm, 360.0);
    if (mnlong < 0.0)
	mnlong += 360.0;

    mnanom = fmod(357.528 + 0.9856003*tm, 360.0);
    mnanom *= M_DTOR;

    eclong = fmod(mnlong + 1.915*sin(mnanom) + 0.020*sin(2.0*mnanom), 360.0);
    eclong *= M_DTOR;

    oblqec = 23.439 - 0.0000004*tm;
    oblqec *= M_DTOR;

    num = cos(oblqec)*sin(eclong);
    den = cos(eclong);
    ra = atan(num/den);

    /* force ra between 0 and 2*pi */
    if (den < 0.0)
	ra += M_PI;
    else if (num < 0.0)
	ra += M_2PI;
#ifdef TEST
fprintf(stderr, "mnlong=%f mnanom=%f eclong=%f oblqec=%f ra=%f\n",
		mnlong, mnanom, eclong, oblqec, ra);
#endif
    /* dec in radians */
    aep->dec = asin(sin(oblqec)*sin(eclong));

    /* calculate Greenwich mean sidereal time in hours */
    gmst = fmod(6.697375 + 0.0657098242*tm + aep->hour, 24.0);

    /* calculate local mean sidereal time in radians  */
    lmst = fmod(gmst + aep->lon*M_DTOH, 24.0);
    lmst = lmst*M_HTOR;

    /* calculate hour angle in radians between -pi and pi */
    aep->ha = lmst - ra;
    if (aep->ha < -M_PI)
	aep->ha += M_2PI;
    if (aep->ha > M_PI)
	aep->ha -= M_2PI;
#ifdef TEST
fprintf(stderr, "dec=%f gmst=%f lmst=%f ha=%f\n", aep->dec, gmst, lmst, aep->ha);
#endif
    /* change latitude to radians */
    lat = aep->lat*M_DTOR;
    sd = sin(aep->dec);
    cd = cos(aep->dec);
    sl = sin(lat);
    cl = cos(lat);

    /* calculate azimuth and elevation */
    aep->el = asin(sd*sl + cd*cl*cos(aep->ha));
    aep->az = asin(-cd*sin(aep->ha)/cos(aep->el));

    /* this puts azimuth between 0 and 2*pi radians */
    if (sd - sin(aep->el)*sl >= 0.0) {
	if(sin(aep->az) < 0.0)
	    aep->az += M_2PI;
    } else
	aep->az = M_PI - aep->az;


    /* calculate refraction correction for US stan. atmosphere */

    /* Refraction correction and airmass updated on 11/19/93 */
 
    /* need to have el in degs before calculating correction */
    el_deg = aep->el * M_RTOD;
    
    if (el_deg >= 19.225)
        refrac = 0.00452 * 3.51823/tan(aep->el);

    else if (el_deg > -0.766 && el_deg < 19.225)
        refrac = 3.51823*(0.1594 + 0.0196*el_deg + 0.00002 * el_deg*el_deg) /
                         (1.0 + 0.505 * el_deg + 0.0845 * el_deg*el_deg);
    else
        refrac = 0.0;

    el_deg += refrac;
    aep->el = el_deg * M_DTOR;

    /* 
     * old refraction equations
     *
     * if (aep->el > -0.762)
     *   refrac = 3.51561*(0.1594 + 0.0196*aep->el + 0.00002*aep->el*aep->el) /
     *     (1.0 + 0.505*aep->el + 0.0845*aep->el*aep->el);
     * else
     *	 refrac = 0.762;
     */

    /* note that 3.51561=1013.2 mb/288.2 C */

    /* calculate distance to sun in A.U. & diameter in degs */
    aep->soldst = 1.00014 - 0.01671*cos(mnanom) - 0.00014*cos(mnanom + mnanom);

    aep->am = airmass(el_deg);


    /* convert back to radians - return EVERYTHING in radians */

    /* do zenith angle */

    aep->zen = M_PI/2.0 - aep->el;

    tst = 12.0 + aep->ha*M_RTOH;
    if (tst < 0.0)
	tst += 24.0;
    if (tst >= 24.0)
	tst -= 24.0;

#ifdef TEST
fprintf(stderr, "az=%.2f el=%.2f ha=%.2f dec=%.2f soldst=%.2f am=%.3f\n",
	M_RTOD*aep->az, M_RTOD*aep->el, M_RTOD*aep->ha,
	M_RTOD*aep->dec, aep->soldst, aep->am);
#endif

    return tst;
}



double airmass(double el)    /* elevation given in degrees */
{
    double	zr;

    if(el < 0.0)  /* sun below the horizon */
	return(-1.0);

    zr = M_DTOR * (90.0 - el);
    return 1.0/(cos(zr) + 0.50572*pow(6.07995 + el, -1.6364));
}
