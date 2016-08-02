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
/****************************************************************************
 * thermo.c
 *
 * Some general thermodynamics routines.
 *
 * Originally written by Peter Neilley.
 *
 * Converted to C by Mike Dixon.
 *
 * RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000, USA
 *
 * Sept 1998
 *
 *********************************************************************/

#include <stdio.h>
#include <math.h>
#include <physics/thermo.h>

#define TWO_OVER_SEVEN .2857142857142857
#define VL 2.5e6
#define CP 1004.0
#define EPS 0.62197
#define K_MINUS_C 273.15
#define RR 287.05
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/********************************************************************
 * PHYvapr()
 *
 * Returns vapor pressure (mb)
 * given temperature (C).
 *
 * If temp is actual temp, the result is saturation vapor pressure.
 * If temp is dewpoint, result is actual vapor pressure.
 */

double PHYvapr(double t)

{

  double vapr;

  vapr = 6.1121 * exp((17.502*t)/(t+240.97));

  return (vapr);

}

/********************************************************************
 * PHYrelh()
 *
 * Returns relative humidity (%),
 * given temp (C) and dewpoint (C).
 */

double PHYrelh(double t, double td)

{

  double ratio, relh;

  ratio = PHYvapr(td) / PHYvapr(t);
  if (ratio > 1.0) {
    ratio = 1.0;
  }
  relh = 100.0 * ratio;

  return (relh);

}

/********************************************************************
 * PHYthta()
 *
 * Returns potential temperature(C),
 * given temp (C) and pressure (MB)
 */

double PHYthta(double t, double p)

{

  double thta;

  thta = (t + K_MINUS_C) * pow((1000.0 / p), TWO_OVER_SEVEN);

  return (thta);

}


/********************************************************************
 * PHYmixr()
 *
 * Returns water vapor mixing ratio (g/kg)
 * given temp (C) and pressure (MB)
 *
 * If temp is actual temp, the result is saturation mixing ratio.
 * If temp is dewpoint, result is actual mixing ratio.
 */

double PHYmixr(double td, double p)

{

  double corr, e, mixr;

  corr = 1.001 + ((p - 100.0) / 900.0) * 0.0034;
  e = PHYvapr(td) * corr;
  mixr = 0.62197 * (e / (p-e)) * 1000.0;

  return (mixr);

}


/********************************************************************
 * PHYtlcl()
 *
 * Returns Lifted Condensation Level Temperature (K),
 * given temp (C) and dewpoint (C)
 */

double PHYtlcl(double t, double td)

{

  double tk, dk, tlcl;

  tk = t + K_MINUS_C;
  dk = td + K_MINUS_C;
  tlcl = (1.0 / (1.0 / (dk - 56.0) + log(tk / dk) / 800.0)) + 56.0;

  return (tlcl);

}


/********************************************************************
 * PHYthte()
 *
 * Returns equivalent potential temperature (C),
 * given pressure (MB), temp (C) and dewpoint (C)
 */

double PHYthte(double p, double t, double td)

{

  double rmix, e, thtam, thte;

  rmix = PHYmixr(td,p);
  e = TWO_OVER_SEVEN * (1.0 - (0.001 * 0.28 * rmix));
  thtam = (t + K_MINUS_C) * pow((1000.0 / p), e);
  thte = thtam * exp((3.376 / PHYtlcl(t, td) - 0.00254) *
		     (rmix * (1.0 + 0.81 * 0.001 * rmix)));

  return (thte);

}

/********************************************************************
 * PHYtmst()
 *
 * Returns Temperature (K)
 * given p (MB), equivalent potential temp (C) and first guess (K).
 */

double PHYtmst(double thte, double p, double tguess)

{

  int i;
  double tg, tgnu, tgnup, tenu, tenup, cor, tmst;

  tmst = 0.0;
  tg = tguess;
  if (tg == 0.0) {
    tg = ((thte - 0.5 * pow(MAX(thte - 270.0, 0.0), 1.05)) *
	  pow((p / 1000.0), 0.2));
  }
  tgnu = tg - K_MINUS_C;
  for (i = 0; i < 100; i++) {
    tgnup = tgnu + 1.0;
    tenu = PHYthte(p, tgnu, tgnu);
    tenup = PHYthte(p, tgnup, tgnup);
    if ((tenu < 0.0) || (tenup < 0.0)) {
      return (tmst);
    }
    cor = (thte-tenu) / (tenup-tenu);
    tgnu = tgnu + cor;
    tmst = tgnu + K_MINUS_C;
    if (fabs(cor) < 0.01) {
      return (tmst);
    }
  } /* i */

  return (tmst);

}

/********************************************************************
 * PHYrhdp()
 *
 * Returns dewpoint temp (C),
 * given temperature (C) and relhumidty (%)
 */

#define AAA 0.000425
#define AA 17.502
#define BB 240.97

static double _rhdp(double t, double relh)

{

  double td, tk, uu, lnu;

  if (relh > 100.0) {
    relh = 100.0;
  } else if (relh < 0.001) {
    relh = 0.001;
  }

  tk = t + K_MINUS_C;
  uu = relh / 100.0;
  lnu = -1.0 * log10(uu);
  td = (tk / (AAA * tk * lnu + 1)) - K_MINUS_C;
  
  return (td);

}

double PHYrhdp(double t, double relh)

{

  double td;
  double check_rh, rh_diff;

  /* use successive approximation to get close */

  td = _rhdp(t, relh);

  check_rh = PHYrelh(t, td);
  rh_diff = check_rh - relh;
  relh -= rh_diff;
  td = _rhdp(t, relh);

  return (td);

#ifdef NOTNOW
  double vapr, top, bottom, rhdp;
  vapr = relh * PHYvapr(t) / 100.0;
  top = 240.97 * (log(EPS) - log(vapr));
  bottom = log(vapr) - log(EPS) - 17.502;
  rhdp = top / bottom;
  return (rhdp);
#endif

}

/********************************************************************
 * PHYtwet()
 *
 * Returns wet bulb temperature (C),
 * given pressure (MB), temp (C) and dew point (C).
 */

double PHYtwet(double p, double t, double td)

{

  int iters = 0;
  double dt = 9.9e9;
  double twet, top, bottom, rmixr, smixr, twet_new;

  top = t;
  bottom = td;
  while ((iters < 100) && (dt > 0.1)) {
    iters++;
    twet = (top + bottom) / 2.0;
    rmixr = PHYmixr(td, p) / 1000.0;
    smixr = PHYmixr(twet, p) / 1000.0;
    twet_new = t - (VL / CP) * (smixr - rmixr);
    dt = fabs(twet - twet_new);
    if (twet_new < twet ) {
      top = twet;
    } else {
      bottom = twet;
    }
  } /* while */

  return (twet);

}

/********************************************************************
 * PHYtvrt()
 *
 * Returns virtual temperature (C),
 * given pressure (MB), temperature (C) and dew point (C).
 */

double PHYtvrt(double p, double t, double td)

{
  double tvrt, rmix, tk;

  if (p < 0.0) {
    return (-9999.0);
  }
  
  rmix = PHYmixr(td, p) / 1000.0;
  tk = t + K_MINUS_C;
  tvrt = tk * (1.0 + rmix / EPS) / (1.0 + rmix) - K_MINUS_C;

  return (tvrt);

}

/********************************************************************
 * PHYrhmr()
 *
 * Returns relative humidity (%),
 * given mixing ratio (g/kg), pressre (MB) and temp (C)
 */

double PHYrhmr(double mixr, double p, double t)

{

  double kg_per_kg, e, es, rmhr;

  kg_per_kg = mixr / 1000.0;

  e = (p * kg_per_kg) / (EPS + kg_per_kg);
  es = PHYvapr(t);
  rmhr = (e / es) * 100.0;
  if (rmhr > 100.0) {
    rmhr = 100.0;
  }

  return (rmhr);

}

/********************************************************************
 * PHYdeltaZ()
 *
 * Returns the thickness of a layer between p1 and p2.
 * Integrate over a sounding to get all heights.
 *
 * pressures are in Mb, temperatures in C, dewpt in C.
 * Returned thickness is in m.
 */

double PHYdeltaZ(double p1, double p2,
		 double t1, double t2,
		 double td1, double td2)

{

  double tvk1, tvk2;
  double deltaz;

  tvk1 = PHYtvrt(p1, t1, td1) + K_MINUS_C;
  tvk2 = PHYtvrt(p2, t2, td2) + K_MINUS_C;

  deltaz = 14.6355 * (tvk1 + tvk2) * log(p1 / p2);

  return (deltaz);

}

/*************************************************************************
 * PHYmb2meters()
 *
 * Return the altitude in m 
 * given an atmospheric pressure level in mb 
 *
 *    G. Wiener 1991
 */

double PHYmb2meters( double mb )
{
  double meters;

  meters = 44307.692 * (1.0 - pow(mb/1013.2, 0.190));
  return(meters);
}

/*************************************************************************
 * PHYmeters2mb()
 *
 * Return the pressure level in mb
 * given a height in m
 *
 *    G. Wiener 1991
 */

double PHYmeters2mb( double meters )
{
  double mb;

  mb = exp(log(1.0 - meters/44307.692)/0.19)*1013.2;
  return(mb);
}


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

double PHYmslTempCorrectedP(double h, double t, double p)
{
  /*
   * define some constants.
   */

  double g = 9.81;
  double R = 287.0;
  double K = 0.003255;

  /*
   * We are correcting to sea level (H=0.0). I will
   * define this as a constant so that if this routine
   * is ever modified to correct to a level other than
   * sea level, the user will just pass in H rather
   * than defining it to be 0.0 here.
   */

  double H = 0.0;

  double exponent, factor;

  /*
   * put T into kelvin and do the calculation.
   */

  t = t + K_MINUS_C;

  exponent = -g*(H-h)/(R*(t-K*(H-h)));
  factor = exp(exponent);

  return factor*p;

}

/*************************************************************************
 * PHYprestemp2density()
 *
 * Set pressure (mb) and temperature (K)
 * Return density in kg/m3
 */

double PHYprestemp2density(double mb, double tk)
{

  double pascals = mb * 100.0;
  double rho = pascals / (RR * tk);
  return rho;

}
