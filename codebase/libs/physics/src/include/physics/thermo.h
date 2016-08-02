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
 * thermo.h : Thermodynamic routines
 *
 ************************************************************************/

#ifndef THERMO_WAS_INCLUDED
#define THERMO_WAS_INCLUDED

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

/********************************************************************
 * PHYdeltaZ()
 *
 * Returns the thickness of a layer between p1 and p2.
 * Integrate over a sounding to get all heights.
 *
 * pressures are in Mb, temperatures in C, dewpt in C.
 * Returned thickness is in m.
 */

extern double PHYdeltaZ(double p1, double p2,
			double t1, double t2,
			double td1, double td2);

/*************************************************************************
 * PHYmb2meters()
 *
 * Return the altitude in m 
 * given an atmospheric pressure level in mb
 */   
extern double PHYmb2meters( double mb );

/*************************************************************************
 * PHYmeters2mb()
 *
 * Return the pressure level in mb
 * given a height in m 
 *
 */
extern double PHYmeters2mb( double meters );

/*************************************************************************
 * PHYmslTempCorrectedP()
 *
 * Routine to correct a pressure measured at altitude to
 * sea level.
 *
 * You pass in :
 * 
 *  h, the height in meters that the pressure was measured at,
 *  t, the temperature at the altitude that pressure 
 *     was measured at, degrees C, and
 *  p, the measured pressure at that altitude, arbitrary units.
 *
 * The return value is the effective pressure at sea level.
 *
 * Niles Oien, October 2002.
 *  
 */

extern double PHYmslTempCorrectedP(double h, double t, double p);

/*************************************************************************
 * PHYprestemp2density()
 *
 * Set pressure (mb) and temperature (K)
 * Return density in kg/m3
 */

extern double PHYprestemp2density(double mb, double tk);

#endif
#ifdef __cplusplus
}
#endif

