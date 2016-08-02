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

/************************************************************************
 * physics.h : Header file for the physics library
 *
 ************************************************************************/

#ifndef PHYSICS_WAS_INCLUDED
#define PHYSICS_WAS_INCLUDED

#include <physics/physics_macros.h>

/**********************************************************************
 * PHYe_sub_s() - Computes the saturation vapor pressure (Pa) over 
 *                liquid with polynomial fit of goff-gratch formulation.
 */

extern double PHYe_sub_s(double tempK);


/**********************************************************************
 * PHYhumidity() - Calculate the relative humidity given the current
 *                 temperature and the dew point in degrees Kelvin.
 */

extern double PHYhumidity(double tempK, double dew_tempK);


/************************************************************************
 ************************************************************************
 * Temperature conversions
 *
 ************************************************************************/

/**********************************************************************
 * PHYtemp_c_to_f() - Convert from Celsius temperature to Fahrenheit.
 */

#define PHYtemp_c_to_f(tempC) (1.8 * (double)tempC + 32.0)


/**********************************************************************
 * PHYtemp_c_to_k() - Convert from Celsius temperature to Kelvin.
 */

#define PHYtemp_c_to_k(tempC) ((double)tempC + 273.16)


/**********************************************************************
 * PHYtemp_f_to_c() - Convert from Fahrenheit temperature to Celsius.
 */

#define PHYtemp_f_to_c(tempF) (0.5555556 * ((double)tempF - 32.0))


/**********************************************************************
 * PHYtemp_f_to_k() - Convert from Fahrenheit temperature to Kelvin.
 */

#define PHYtemp_f_to_k(tempF) ((0.5555556 * ((double)tempF - 32.0)) + 273.16)


/**********************************************************************
 * PHYtemp_k_to_c() - Convert from Kelvin temperature to Celsius.
 */

#define PHYtemp_k_to_c(tempK) ((double)tempK - 273.16)


/**********************************************************************
 * PHYtemp_k_to_f() - Convert from Kelvin temperature to Fahrenheit.
 */

#define PHYtemp_k_to_f(tempK) ((1.8 * (double)tempK) - 459.688)


/************************************************************************
 ************************************************************************
 * Thermodynamics routines
 *
 ************************************************************************/

/********************************************************************
 * PHYvapr()
 *
 * Returns vapor pressure (mb)
 * given temperature (C).
 *
 * If temp is actual temp, the result is saturation vapor pressure.
 * If temp is dewpoint, result is actual vapor pressure.
 */

extern double PHYvapr(double t);

/********************************************************************
 * PHYrelh()
 *
 * Returns relative humidity (%),
 * given temp (C) and dewpoint (C).
 */

extern double PHYrelh(double t, double td);

/********************************************************************
 * PHYthta()
 *
 * Returns potential temperature(C),
 * given temp (C) and pressure (MB)
 */

extern double PHYthta(double t, double p);

/********************************************************************
 * PHYmixr()
 *
 * Returns water vapor mixing ratio (g/kg)
 * given temp (C) and pressure (MB)
 *
 * If temp is actual temp, the result is saturation mixing ratio.
 * If temp is dewpoint, result is actual mixing ratio.
 */

extern double PHYmixr(double td, double p);

/********************************************************************
 * PHYtlcl()
 *
 * Returns Lifted Condensation Level Temperature (K),
 * given temp (C) and dewpoint (C)
 */

extern double PHYtlcl(double t, double td);

/********************************************************************
 * PHYthte()
 *
 * Returns equivalent potential temperature (C),
 * given pressure (MB), temp (C) and dewpoint (C)
 */

extern double PHYthte(double p, double t, double td);

/********************************************************************
 * PHYtmst()
 *
 * Returns Temperature (K)
 * given p (MB), equivalent potential temp (C) and first guess (K).
 */

extern double PHYtmst(double thte, double p, double tguess);

/********************************************************************
 * PHYrhdp()
 *
 * Returns dewpoint temp (C),
 * given temperature (C) and relhumidty (%)
 */

extern double PHYrhdp(double t, double relh);

/********************************************************************
 * PHYtwet()
 *
 * Returns wet bulb temperature (C),
 * given pressure (MB), temp (C) and dew point (C).
 */

extern double PHYtwet(double p, double t, double td);

/********************************************************************
 * PHYtvrt()
 *
 * Returns virtual temperature (C),
 * given pressure (MB), temperature (C) and dew point (C).
 */

extern double PHYtvrt(double p, double t, double td);

/********************************************************************
 * PHYrhmr()
 *
 * Returns relative humidity (%),
 * given mixing ratio (g/kg), pressre (MB) and temp (C)
 */

extern double PHYrhmr(double mixr, double p, double t);



/************************************************************************
 ************************************************************************
 * Wind conversion routines
 *
 ************************************************************************/

/****************************************************************************
 * PHYwind_speed - Calculate the wind speed given the U and V values.
 *
 * Expects the u and v values to be given in m/sec, returns the wind
 * speed value in m/sec.
 *
 * Returns -9999.0 if the wind speed cannot be calculated from the
 * given vectors.
 *********************************************************************/

double PHYwind_speed(const double u, const double v);
   

/****************************************************************************
 * PHYwind_dir - Calculate the wind direction given the U and V components.
 *
 * Expects the U and V components in m/sec, returns the wind direction in
 * degrees.  The returned wind direction is normalized to be between
 * 0.0 and 360.0.
 *
 * Returns -9999.0 if the V component cannot be calculated from the
 * given data.
 *********************************************************************/

double PHYwind_dir(const double u, const double v);


/****************************************************************************
 * PHYwind_u - Calculate the U component of the wind given the wind
 *             speed and direction.
 *
 * Expects the wind_speed value in m/sec and the wind_dir value in
 * degrees, returns the U component in m/sec.
 *
 * Returns -9999.0 if the U component cannot be calculated from the
 * given data.
 *********************************************************************/

double PHYwind_u(const double wind_speed, const double wind_dir);
   

/****************************************************************************
 * PHYwind_v - Calculate the V component of the wind given the wind
 *             speed and direction.
 *
 * Expects the wind_speed value in m/sec and the wind_dir value in
 * degrees, returns the V component in m/sec.
 *
 * Returns -9999.0 if the V component cannot be calculated from the
 * given data.
 *********************************************************************/

double PHYwind_v(const double wind_speed, const double wind_dir);


/************************************************************************
 ************************************************************************
 * Lifted index
 *
 ************************************************************************/

/****************************************************************************
 * PHYli - Calculate the lifted index given pressure (mb), temperature (C),
 *         dewpoint temperature (C), lifted index temperature (K) and
 *         lifted index pressure (mb).
 *
 * Returns the bad data value if the lifted index cannot be calculated
 * from the given information.
 *********************************************************************/

double PHYli(double p,
	     double tc, double tdc, double tli_k,
	     double pres_li, double bad);
   

#endif
#ifdef __cplusplus
}
#endif
