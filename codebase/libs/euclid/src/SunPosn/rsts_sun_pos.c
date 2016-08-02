/**
 * copyright
 * @file
 * @author Team RSIS
 * @date 04-07-2004
 * @brief This file interfaces to the NOVAS libraries to calculate the position of the Sun for Suncheck tests.
 *
 * Change Log:
 *
 * Programmer Name--------------------Description-----------------Date
 *
 */

#include "rsts_sun_pos.h"

/* Local Function Declarations */
static void rsts_get_tdt_julian_date( double *current_tdt_julian_date);
static void rsts_get_tdt_julian_date_at_time( double *tdt_julian_date, time_t desired_time );

#ifdef _RSTS_SUN_POS_UNIT_TEST
/**
 * This function is used for stand alone debugging of the code in this file.  Must 
 * Define _RSTS_SUN_POS_UNIT_TEST when the file is compiled, to make a stand alone 
 * program from this file.
 *
 * --------------------------------------------------------
 *  
 * @return (int)  Always 0.
 *
 * --------------------------------------------------------
 */
int main()
{
  /** <h3> The following is a detailed description of the steps for main(): </h3> */

  /** Radar location. */
  site_info here = {35.0 + 13.0/60.0 /*lat*/,
		   -97.0 - 27.0/60.0 /*long*/,
		   300/*height*/,
		   0/*temp Celsius*/,
		   1010/*pressure millibars*/};
  double DELTAT=-0.45;
  double SunAz,SunEl;
  double distanceAU;
  time_t Tnow;
  rsts_SunNovasComputePos(here,DELTAT,&SunAz,&SunEl, &distanceAU );
  Tnow = time(NULL);
  fprintf(stdout,"Sun position on %s  is Az: %g  El : %g\n",ctime(&Tnow),
	  SunAz,SunEl);
}
#else
/* #include "rsts.h" */
#endif

/**
 * This function uses the NOVAS software library to compute 
 * the current position of the Sun on the local horizon. 
 *
 * --------------------------------------------------------
 *  
 * @param here (site_info)  The point on the surface of the earth we are at.
 * @param deltat (double)   The difference between terrestrial dynamical time system 1 time, and universal time.
 * @param *SunAz (double *) The returned sun azimuth, in degrees from true north.
 * @param *SunEl (double *) The returned sun elevation, in degrees above the local horizon.
 * @param *distanceAU (double *) The returned distance to the sun, in astronomical units.
 *
 * --------------------------------------------------------
 */
void rsts_SunNovasComputePos( site_info here, double deltat, double *SunAz, double *SunEl, double *distanceAU)
{
  /** <b> Local Variables </b> */
  /** <ul> */
  /** <li><b> body earth,sun </b> - The earth and sun body structures. See NOVAS 
   * documentation for more information. </li> */
  body earth,sun;
  /** <li><b> double ra,declination </b> - The right ascension, and declination of the sun </li> */
  double ra,declination;
  /** <li><b> double current_tdt_julian_date </b> - the current terrestrial dynamical time (TDT) julian date </li> */
  double  current_tdt_julian_date;
  /** <li><b> double zenith_distance </b> - The Sun's zenith distance, measured from the local vertical. </li> */
  double zenith_distance;
  /** <li><b> double azimuth </b> - The Sun's azimuth is measured from the north </li> */
  double azimuth;
  /** <li><b> double rar,decr </b> - rar -right ascension. decr -declination. In the equatorial coordinate system </li> */
  double rar,decr;
  /** </ul> */

  /**------------------------------------------------------*/
  /** <h3> The following is a detailed description of the steps for rsts_SunNovasComputePos(): </h3> */

  /** Call NOVAS subroutine set_body to setup Sun and Earth body structures. */
  set_body(0,10,"Sun",&sun);
  set_body(0,3,"Earth",&earth);

  /** Get the current TDT date. */
  rsts_get_tdt_julian_date( &current_tdt_julian_date);

  /**  Call NOVAS routine topo_planet to get the right ascension and declination 
   * of the Sun in topographic/equatorial coordinate system. */
  topo_planet( current_tdt_julian_date,
	       &sun, &earth, deltat,
	       &here ,
	       &ra , &declination , distanceAU);
	       
  /** Call NOVAS subroutine equ2hor to convert the Sun's position to local horizon coordinates. */
  equ2hor( current_tdt_julian_date, deltat,
	   0.0 /*x_ephemeris*/, 0.0 /*y_ephemeris*/,
	   &here,ra,declination,1/*normal refraction*/,
	   &zenith_distance,
	   &azimuth,
	   &rar,&decr);

  /** Convert zenith_distance reference point so that the Sun's elevation is referenced to the local horizon. */
  *SunEl = 90.0 - zenith_distance; 
  /** Azimuth needs no conversion, its local horizon reference is already true north. */
  *SunAz = azimuth;
  /** If this is a stand alone unit test, the results of the subroutine are printed for 
   * comparison with the naval observatory's web pages. */
#ifdef _RSTS_SUN_POS_UNIT_TEST
  printf(" El : %f   Az : %f \n", *SunEl, *SunAz);
#endif
}

/*---------------------------------------------------------*/
/**
 * This function uses the NOVAS software library to compute the position 
 * of the Sun on the local horizon, but for an input time, instead of the current time.
 *
 * --------------------------------------------------------
 *  
 * @param here (site_info)  The point on the surface of the earth we are at.
 * @param deltat (double)   The difference between terrestrial dynamical time system 1 time, and universal time.
 * @param *SunAz (double *) The returned sun azimuth, in degrees from true north.
 * @param *SunEl (double *) The returned sun elevation, in degrees above the local horizon.
 * @param time_in (time_t)  The utc time in seconds that we need to compute the Sun's position.
 *
 * --------------------------------------------------------
 */
void rsts_SunNovasComputePosAtTime( site_info here, double deltat, double *SunAz, double *SunEl, time_t time_in )
{
  /** <b> Local Variables </b> */
  /** <ul> */
  /** <li><b> body earth,sun </b> - The earth and sun body structures. See NOVAS documentation for more information. </li> */
  body earth,sun;
  /** <li><b> double ra,declination </b> - The right ascension, and declination of the sun </li> */
  double ra,declination;
  /** <li><b> double tdt_julian_date </b> - the current terrestrial dynamical time (TDT) julian date </li> */
  double  tdt_julian_date;
  /** <li><b> double zenith_distance </b> - Zenith distance, measured from the local vertical. </li> */
  double zenith_distance;
  /** <li><b> double azimuth </b> - The Sun's azimuth is measured from the north </li> */
  double azimuth;
  /** <li><b> double rar,decr </b> - rar -right ascension. decr -declination. In equatorial coordinate system </li> */
  double rar,decr;
  /** <li><b> double distanceAU </b> - The distance to the Sun, units of AU, not returned by this function. </li> */
  double distanceAU;
  /** </ul> */

  /**------------------------------------------------------*/
  /** <h3> The following is a detailed description of the steps for rsts_SunNovasComputePosAtTime(): </h3> */
  
  /** Call NOVAS subroutine set_body to setup Sun and Earth body structures. */
  set_body(0,10,"Sun",&sun);
  set_body(0,3,"Earth",&earth);

  /** Get the TDT date for the input time. */
  rsts_get_tdt_julian_date_at_time( &tdt_julian_date, time_in);

  /** Call NOVOS routine topo_planet to get the right ascension and declination 
   * of the Sun in topographic/equatorial coordinate system. */
  topo_planet( tdt_julian_date,
	       &sun, &earth, deltat,
	       &here ,
	       &ra , &declination , &distanceAU);
	       
  /** Call NOVAS subroutine equ2hor to convert the Sun's position to local horizon coordinates. */
  equ2hor( tdt_julian_date, deltat,
	   0.0 /*x_ephemeris*/, 0.0 /*y_ephemeris*/,
	   &here,ra,declination,1/*normal refraction*/,
	   &zenith_distance,
	   &azimuth,
	   &rar,&decr);

 /** Convert zenith_distance reference point so that the Sun's elevation is referenced to the local horizon. */
  *SunEl = 90.0 - zenith_distance; 
  /** Azimuth needs no conversion, its local horizon reference is already true north. */
  *SunAz = azimuth;
  /** Log a message to the test log with the input time, and position of the sun given. */
  /** RSTS_LOG_MSG(LOG_RDA_OTEST,"Sun Pos at TDT JD %f - Az %f  El %f",tdt_julian_date,*SunAz,*SunEl); */
  /** If this is a stand alone unit test, the results of the subroutine are printed 
   * for comparison with the naval observatory's web pages. */
#ifdef _RSTS_SUN_POS_UNIT_TEST
  printf(" El : %f   Az : %f \n", *SunEl, *SunAz);
#endif
}

/*---------------------------------------------------------*/
/**
 * This function calculates the current tdt (Terrestrial Dynamic Time) Julian Date.
 *
 * --------------------------------------------------------
 *  
 * @param current_tdt_julian_date (double) The current tdt date is returned here.
 *
 * --------------------------------------------------------
 */
void rsts_get_tdt_julian_date( double *current_tdt_julian_date)
{
  /** <b> Local Variables </b> */
  /** <ul> */
  /** <li><b> time_t tnow </b> - the current time in UNIX Epoch (UTC on rcp8.) </li> */
  time_t tnow;
  /** </ul> */

  /**------------------------------------------------------*/
  /** <h3> The following is a detailed description of the steps for rsts_get_tdt_julian_date(): </h3> */
  /** Get the current time. */
  tnow = time (NULL);
  /** Calculate the tdt_julian_date for the current time, using rsts_get_tdt_julian_date_at_time(). */
  rsts_get_tdt_julian_date_at_time( current_tdt_julian_date, tnow);
}

/*---------------------------------------------------------*/
/**
 * This function gets the tdt (Terrestrial Dynamic Time) Julian Date at the time desired.
 *
 * --------------------------------------------------------
 *  
 * @param tdt_julian_date (double) The tdt date is returned here, corresponding to the desired time.
 * @param desired_time (time_t) The time in seconds since the epoch that the Sun position is desired for.
 *
 * --------------------------------------------------------    
 */
void rsts_get_tdt_julian_date_at_time( double *tdt_julian_date, time_t desired_time )
{
  /** <b> Local Variables </b> */
  /** <ul> */
  /** <li><b> double epic2k=2451545.0 </b> - The year 2000 corresponds to a 
   * modified julian tdt of 2451545.0 (days) </li> */
  double epic2k=2451545.0;
  /** <li><b> double secs_per_geoid = 86400.0 </b> - basically the number of seconds in a tdt day. </li> */
  double secs_per_geoid = 86400.0;
  /** <li><b> double seconds_since_epic2k </b> - The number of seconds since the year 2000 </li> */
  double seconds_since_epic2k;
  /** <li><b> time_t tnow </b> - The time sinch the (unix) epoch </li> */
  time_t tnow;
  /** <li><b> time_t ty2k = 946684800 </b> - Seconds from the epoch to y2k-GMT </li> */
  time_t ty2k = 946684800;
  /** </ul> */

  /**------------------------------------------------------*/
  /** <h3> The following is a detailed description of the steps for rsts_get_tdt_julian_date_at_time(): </h3> */
  
  /** Use the input time as the current time. */
  tnow = desired_time ;
   
  /** Get seconds since y2k TT. */
  seconds_since_epic2k = tnow - ty2k;
  /** Calculate tdt julian date. Subtract half a day ( the -0.5 term ) to get back from 
   * modified julian date.(Noon referenced). */
  *tdt_julian_date = epic2k + (seconds_since_epic2k/secs_per_geoid) - 0.5;
  /** If called during a stand alone unit test, print the results of the 
   * intermediate calculation for desk checking. */
#ifdef _RSTS_SUN_POS_UNIT_TEST
  printf("The current seconds since epoch %d\n",tnow);
  printf("The seconds y2k was from epoch %d\n",ty2k);
  printf("TDT Julian Date for given time %f\n",*tdt_julian_date);
#else
  /** Not compiled for unit test, then log the tdt date to the OTLOG. */
  /* RSTS_DEBUG_MSG(VERBOSE, "TDT Julian Date for given time %f\n",*tdt_julian_date); */
#endif
}
