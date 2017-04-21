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
 * structs for holding pulse data
 */

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

/*****************************************************
 * initialize the module for processing
 */

extern void nexradSolarInit(int n_samples);

/*****************************************************
 * free up the memory used in the module
 */

extern void nexradSolarFree();

/*****************************************************
 * initialize the lat/lon/alt for which sun position
 * is computed lat/lon in degrees, alt_m in meters
 */

extern void nexradSolarSetLocation(double lat, double lon, double alt_m);

/*****************************************************
 * clear the pulse queue
 */

extern void nexradSolarClearPulseQueue();

/*****************************************************
 * add a pulse to the queue
 */

extern void nexradSolarAddPulseToQueue(solar_pulse_t *pulse);
  
/**************************************************
 * perform analysis
 *
 * Returns 0 on success, -1 on failure
 */

extern void nexradSolarPerformAnalysis();

/*****************************************************
 * compute receiver gain
 * based on solar flux from Penticton
 *
 * Reference: On Measuring WSR-88D Antenna Gain Using Solar Flux.
 *            Dale Sirmans, Bill Urell, ROC Engineering Branch
 *            2001/01/03.
 */
   
extern void nexradSolarComputeReceiverGain();

#endif

#ifdef __cplusplus
}
#endif

