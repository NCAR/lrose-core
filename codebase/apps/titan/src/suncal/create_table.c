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
/*********************************************************************
 * create_table.c
 *
 * create the table - write to stdout
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1996
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "suncal.h"
#include <toolsa/toolsa_macros.h>

void create_table(void)

{

  int i;

  date_time_t dtime, ystart;

  time_t ttime;

  double gmt_hr;
  double decl, decl_rad;
  double lat;
  double solar_time;
  double hr, hour_rad, hour_deg;
  double alt, sin_alt, alt_deg;
  double az, az_deg, az_adjusted, tan_az;
  double jday, jdeg, eqt;

  lat = Glob->params.station_pos.lat * DEG_TO_RAD;

  ttime = Glob->start_time;

  while (ttime <= Glob->end_time) {

    dtime.unix_time = ttime;
    uconvert_from_utime(&dtime);

    /*
     * compute start of year time
     */
    
    ystart = dtime;
    ystart.day = 1;
    ystart.month = 1;
    ystart.hour = 0;
    ystart.min = 0;
    ystart.sec = 0;
    uconvert_to_utime(&ystart);

    jday = (double) (ttime - ystart.unix_time) / 86400.0;
    jdeg = jday * (360.0 / 365.25);

    eqt = 0.123 * cos((jdeg + 87.0) * DEG_TO_RAD) -
      0.16666667 * sin(((jdeg + 10.0) * DEG_TO_RAD) * 2.0);

    decl = -23.5 * cos((jday + 10.3) * (360.0 / 365.25) * DEG_TO_RAD);
    decl_rad = decl * DEG_TO_RAD;

    hr = (double) ttime / 3600.0;
    gmt_hr = fmod((hr - 12.0), 24.0);

    solar_time =
      (gmt_hr + (Glob->params.station_pos.lon / 15.0) + eqt);
    
    hour_deg = solar_time * 15.0;
    hour_rad = hour_deg * DEG_TO_RAD;

    sin_alt = sin(lat) * sin(decl_rad) +
      cos(lat) * cos(decl_rad) * cos(hour_rad);
    alt = asin(sin_alt);
    alt_deg = alt * RAD_TO_DEG;

    tan_az = (sin(hour_rad) /
	      ((cos(hour_rad) * sin(lat)) - (tan(decl_rad) * cos(lat))));

    az = atan(tan_az);
    az_deg = az * RAD_TO_DEG;
    az_adjusted = az_deg;

    if (hour_deg >= 0.0 && hour_deg < 180.0) {
      if (az_adjusted > 0.0) {
	az_adjusted += 180.0;
      } else {
	az_adjusted += 360.0;
      }
    } else {
      if (az_adjusted < 0.0) {
	az_adjusted += 180.0;
      }
    }

    fprintf(stdout, "Time az el: %s %10.3f %10.3f\n",
	    utimstr(ttime), az_adjusted, alt_deg);

    if (Glob->params.debug) {

      fprintf(stdout,
	      "      hour_deg, az_deg, az_adjusted: "
	      "%10.3f %10.3f %10.3f\n",
	      hour_deg, az_deg, az_adjusted);
      
      fprintf(stdout,
 	      "      jday, jdeg, decl, eqt(hr/sec): "
 	      "%g, %g, %g, %g/%d\n",
 	      jday, jdeg, decl, eqt, (int) (eqt * 3600.0 + 0.5));
      
      fprintf(stdout,
 	      "      hr, gmt_hr, solar_time, lat: "
 	      "%g, %g, %g, %g\n",
 	      hr, gmt_hr, solar_time, lat);

      fprintf(stdout, "------------------------\n");

    }

    for (i = 0; i < Glob->params.n_blank_lines; i++) {
      fprintf(stdout, "\n");
    }
    
    ttime += Glob->params.delta_t;

  }

}

void create_table_2(void)

{

  const double secsPerDay = 86400.0;

  int i;
  
  date_time_t dtime;
  time_t ttime;

  date_time_t u2000;
  double day, ecl, MM, MMRad, ee, EE, EERad;
  double xv, yv, v, r, w, lonsun, xs, ys;
  double xe, ye, ze;
  double RA, DeclRad, Decl, LL, GMST0, utHour, LST;
  double LHA, latRad, sinAlt, alt, lhaRad, yyy, xxx, az;
  double lat, lon;

  u2000.year = 1999;
  u2000.month = 12;
  u2000.day = 31;
  u2000.hour = 0;
  u2000.min = 0;
  u2000.sec = 0;
  uconvert_to_utime(&u2000);

  lat = Glob->params.station_pos.lat;
  lon = Glob->params.station_pos.lon;

  ttime = Glob->start_time;

  while (ttime <= Glob->end_time) {

    dtime.unix_time = ttime;
    uconvert_from_utime(&dtime);

    /* reference:  */
    /* http://www.stjarnhimlen.se/comp/ppcomp.html#5 */

    /* days since 2000 */

    day = ((double) dtime.unix_time - (double) u2000.unix_time) / secsPerDay;
    
    /* obliquity of the ecliptic */
    
    ecl = 23.4393 - 3.563e-7 * day;
    
    /* sun mean anomaly */

    MM = 356.0470 + 0.9856002585 * day;
    MMRad = MM * DEG_TO_RAD;
    
    /* eccentricity */

    ee = 0.016709 - 1.151E-9 * day;

    /* eccentricity anomaly */
    
    EE = MM +
      (ee * RAD_TO_DEG) * sin(MMRad) * (1.0 + ee * cos(MMRad));
    EERad = EE * DEG_TO_RAD;
    
    /* compute the Sun's distance r and its true anomaly v */
    
    xv = cos(EERad) - ee;
    yv = sqrt(1.0 - ee * ee) * sin(EERad);
    
    v = atan2(yv, xv) * RAD_TO_DEG;
    r = sqrt(xv*xv + yv*yv);
    
    /* compute w, argument of perihelion */
    
    w = 282.9404 + 4.70935E-5 * day;
    
    /* compute the Sun's true longitude */
    
    lonsun = v + w;
    
    /* convert lonsun,r to ecliptic rectangular geocentric coordinates xs,ys: */
    
    xs = r * cos(lonsun * DEG_TO_RAD);
    ys = r * sin(lonsun * DEG_TO_RAD);
    
    /* convert equatorial, rectangular, geocentric coordinates */
    
    xe = xs;
    ye = ys * cos(ecl * DEG_TO_RAD);
    ze = ys * sin(ecl * DEG_TO_RAD);
    
    /* compute the Sun's Right Ascension (RA) and Declination (Dec): */
    
    RA  = atan2(ye, xe) * RAD_TO_DEG;
    DeclRad = atan2(ze, sqrt(xe*xe+ye*ye));
    Decl = DeclRad * RAD_TO_DEG;
    
    /* compute sun's mean longitude */
    
    LL = MM + w;
    
    /* compute sidereal time at Greenwich */
    
    GMST0 = LL + 180.0;
    
    /* compute our Local Sidereal Time (LST) in degrees */
    
    utHour = fmod((double) dtime.unix_time, secsPerDay) / 3600.0;
    LST = GMST0 + utHour*15.0 + lon;
    
    /* compute the Sun's Local Hour Angle (LHA) */
    
    LHA = LST - RA;
    
    /* compute the altitude */
    
    latRad = lat *  DEG_TO_RAD;
    sinAlt =
      sin(latRad) * sin(DeclRad) + cos(latRad) * cos(DeclRad) * cos(LHA * DEG_TO_RAD);
    
    alt = asin(sinAlt) * RAD_TO_DEG;

    /* compute azimuth */
    
    lhaRad = LHA * DEG_TO_RAD;
    yyy = sin(lhaRad);
    xxx = cos(lhaRad) * sin(latRad) - tan( DeclRad) * cos(latRad);
    az = atan2(yyy, xxx) * RAD_TO_DEG + 180.0;
    if (az < 0) {
      az += 360.0;
    } else if (az > 360.0) {
      az -= 360.0;
    }
    
    fprintf(stdout, "Time az el: %s %10.3f %10.3f\n",
	    utimstr(ttime), az, alt);
    
    for (i = 0; i < Glob->params.n_blank_lines; i++) {
      fprintf(stdout, "\n");
    }
    
    ttime += Glob->params.delta_t;

  }

}

