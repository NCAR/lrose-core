/****************************************************************
 * sunscan_nexrad.h
 *
 * C implementation of sunscan code, for use by NEXRAD
 *
 * Mike Dixon, EOL, NCAR
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * April 2017
 *
 *****************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef sunscan_nexrad_h
#define sunscan_nexrad_h

#include <sys/time.h>

/************************************************
 * structs for holding pulse and beam data
 */

/***************************************************************************
 * struct site_info: data for the observer's location.  The atmospheric 
 *                   parameters are used only by the refraction 
 *                   function called from function 'equ_to_hor'.
 *                   Additional parameters can be added to this 
 *                   structure if a more sophisticated refraction model 
 *                   is employed.
 *                    
 *  latitude         = geodetic latitude in degrees; north positive.
 *  longitude        = geodetic longitude in degrees; east positive.
 *  height           = height of the observer in meters.
 *  temperature      = temperature (degrees Celsius).
 *  pressure         = atmospheric pressure (millibars)
 */

typedef struct
{
  double latitude;
  double longitude;
  double height;
  double temperature;
  double pressure;
} solar_site_info;

/*****************************************************
 * Complex math object 
 */

typedef struct {
  double re;
  double im;
} solar_complex_t;

/******************************
 * Pulse implementation example
 */

typedef struct {
  
  /* meta data */
  
  double time; /* time in secs and fractions from 1 Jan 1970 */
  double el; /* elevation angle (deg) */
  double az; /* azimuth angle (deg) */
  
  /* moments along the beam, between specified gates */
  
  int nGatesUsed; /* number of gates from which moments are computed */
  double powerH; /* power for H channel I*I+Q*Q */
  double powerV; /* power for V channel I*I+Q*Q */
  solar_complex_t rvvhh0; // rvvhh0
  
} solar_pulse_t;

/*****************************
 * Beam implementation example
 *
 * Note that the meta-data  time, el and az are not actually
 * used in this code, they are just included for context.
 */

typedef struct {
  
  /* meta data */
  
  /* int solarNSamples; number of pulse samples in beam */
  int nGates; /* number of gates */
  double time; /* time for the center pulse of beam */
  double el; /* elevation angle for center of beam (deg) */
  double az; /* azimuth angle for center of beam(deg) */
  double elOffset; /* elevation offset to theoretical sun center (deg) */
  double azOffset; /* azimuth offset to theoretical sun center (deg) */
  
  /* moments */
  
  double powerH; /* power for H channel I*I+Q*Q */
  double powerV; /* power for V channel I*I+Q*Q */
  double dbmH;   /* power for H channel in dBm */
  double dbmV;   /* power for V channel in dBm */
  double dbm;    /* mean of dbmH and dbmV */
  double corrHV;  /* correlation between H and V */
  double phaseHV; /* mean phase between H and V */
  double dbBelowPeak; /* peak sun power minus mean dbm */
  double zdr; /* dbmH minus dbmV */
  double ratioDbmVH; /* ratio of V / H power */
  double SS; /* 1.0 / (zdr^2) */

  solar_complex_t rvvhh0;

} solar_beam_t;

/*****************************************************
 * initialize the lat/lon/alt for which sun position
 * is computed lat/lon in degrees, alt_m in meters
 */

extern void solarSetLocation(double lat, double lon, double alt_m);

/*****************************************************
 * initialize the pulse queue
 */

extern void solarInitPulseQueue(int n_samples);
  
/*****************************************************
 * delete the pulse queue
 */

extern void solarFreePulseQueue();
  
/*****************************************************
 * add a pulse to the queue
 */

extern void solarAddPulseToQueue(solar_pulse_t *pulse);
  
/*****************************************************
 * compute sun moments in dual-pol simultaneous mode
 * load up Beam with moments
 */

extern int computeMoments(int startGate,
                          int endGate,
                          solar_beam_t *beam);

/*****************************************************
 * Add a beam to the raw beam array 
 */

extern int addBeam(solar_beam_t *beam);

/*****************************************************
 * interp ppi moments onto regular 2-D grid
 *
 * global 2D array of Beam objects to store the interpolated data:
 * Beam _interpBeamArray[gridNAz][gridNEl];
 * double _interpDbmH[gridNAz][gridNEl];
 * double _interpDbmV[gridNAz][gridNEl];
 * double _interpDbm[gridNAz][gridNEl];
 */

extern void interpMomentsToRegularGrid();

/*****************************************************
 * compute receiver gain
 * based on solar flux from Penticton
 *
 * Reference: On Measuring WSR-88D Antenna Gain Using Solar Flux.
 *            Dale Sirmans, Bill Urell, ROC Engineering Branch
 *            2001/01/03.
 */
   
extern void computeReceiverGain();

#endif

#ifdef __cplusplus
}
#endif

