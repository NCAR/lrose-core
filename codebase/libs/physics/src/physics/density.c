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
 * density.c
 *
 * Some general density routines.
 *
 * Taken from FORTRAN code found online at:
 *     http://wahiduddin.net/calc/density_algorithms.htm
 *
 * RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000, USA
 *
 * Convert to C by Nancy Rehak
 *
 * June 2009
 *
 *********************************************************************/

#include <stdio.h>
#include <math.h>

#include <physics/density.h>
#include <rapmath/math_macros.h>
#include <toolsa/mem.h>

/********************************************************************
 * PHYdenALCL()
 *
 * This function returns the pressure ALCL (mb) of the lifting 
 * condensation level (LCL) for a parcel initially at temperature
 * T (Celsius), dew point TD (Celsius) and pressure P (mb).  ALCL is
 * computed by an iterative procedure described by eqs. 8-12 in
 * Stipoanuk (1973), pp 13-14.
 *
 * Reference: Stipanuk paper entitled "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenALCL(double p, double t, double td)
{
  double aw;
  double ao;
  double pi;
  
  int i;
  
  /* Determine the mixing ratio line through td and p. */

  aw = PHYdenW(p, td);
  
  /* Determine the dry adiabat through t and p. */

  ao = PHYdenO(t, p);
  
  /* Iterate to locate pressure pi at the intersection of the two
   * curves.  pi has been set to p for the initial guess.
   */

  pi = p;
  
  for (i = 0; i < 10; ++i)
  {
    double x;
    
    x = 0.02 * (PHYdenTMR(aw, pi) - PHYdenTDA(ao, pi));
    
    if (fabs(x) < 0.01)
      break;
    
    pi = pi * pow(2.0, x);
  } /* endfor - i */
  
  return pi;
}


/********************************************************************
 * PHYdenALCLM()
 *
 * This function returns the pressure ALCL (mb) of the lifting 
 * condensation level (LCL) for a parcel initially at mean potential
 * temperature t (Celsius), mean mixing ratio td (g/kg) of the layer
 * containing the parcel and pressure P (mb).  ALCL is
 * computed by an iterative procedure described by eqs. 8-12 in
 * Stipoanuk (1973), pp 13-14.
 */

double PHYdenALCLM(double t, double td, double p)
{
  double pi;
  int i;
  
  /* Iterate to locate pressure pi at the intersection of the two
   * curves.  pi has been set to p for the initial guess.
   */

  pi = p;
  
  for (i = 0; i < 10; ++i)
  {
    double x;
    
    x = 0.02 * (PHYdenTMR(td, pi) - PHYdenTDA(t, pi));
    
    if (fabs(x) < 0.01)
      break;
    
    pi = pi * pow(2.0, x);
  } /* endfor - i */
  
  return pi;
}


/********************************************************************
 * PHYdenCT()
 *
 * This function returns the convective temperature CT (Celsius) given
 * the mean mixing ratio wbar (g/kg) in the surface layer, the pressure
 * pc (mb) at the convective condensation level (ccl) and the surface
 * pressure ps (mb).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenCT(double wbar, double pc, double ps)
{
  double tc;
  double ao;
  
  /* Compute the temperature (Celsius) at the CCL. */

  tc = PHYdenTMR(wbar, pc);
  
  /* Compute the potential temperature (Celsius), i.e., the dry adiabat
   * ao through the CCL.
   */

  ao = PHYdenO(tc, pc);
  
  /* Compute the surface temperature on the same dray adiabat ao. */

  return PHYdenTDA(ao, ps);
}


/********************************************************************
 * PHYdenDEWPT()
 *
 * This function yields the dew point dewpt (Celsius), given the water
 * vapor pressure ew (mb).  The empirical formula appears in Bolton,
 * David, 1980: "The Computation of Equivalent Potential Temperature,"
 * Monthly Weather Review, vol 108, no 7 (July), p 1047, eq (11).
 * The quoted accuracy is 0.03C or less for -35 < dewpt < 35C.
 */

double PHYdenDEWPT(double ew)
{
  double enl = log(ew);
  
  return ((243.5 * enl) - 440.8) / (19.48 - enl);
}


/********************************************************************
 * PHYdenDPT()
 *
 * This function returns the dew point dpt (Celsius), given the water
 * vapor pressure ew (mb).
 *
 * Returns 9999.0 if the dew point can't be calculated.
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88022.
 */

double PHYdenDPT(double ew)
{
/* Define saturation vapor pressure (mb) over water at 0C */

  const double ES0 = 6.1078;

  double x;
  double dnm;
  double t;
  double fac;
  double dt;
  
  /* Return a flag if hte vapor pressure is out of range. */

  if (ew < 0.06 || ew > 1013.0)
  {
    return 9999.0;
  }
  
  /* Approximate dew point by means of Teten's formula.  The formula
   * appears as eq (8) in Bolton, David, 1980: "The Computation of 
   * Equivalent Potential Temperature," Monthly Weather Review, vol 108,
   * no 7 (July), p 1047.  The formula is:
   *       ew(t) = es0*10**(7.5*t/(t+237.3))  or
   *       ew(t) = es0*exp(17.269388*t/(t+237.3))
   * The inverse formula is used below.
   */

  x = log10(ew / ES0);
  dnm = 17.269388 - x;
  t = (237.3 * x ) / dnm;
  fac = 1.0 / (ew * dnm);
  
  /* Loop for iterative improvement of the estimate of dew point */

  do
  {
    double edp;
    double dtdew;
    
    /* Get the precise vapor pressure corresponding to t. */

    edp = PHYdenESW(t);
    
    /* Estimate the change in temperature corresponding to (ew - edp)
     * Assume that the derivative of temperature with respect to vapor
     * pressure (dtdew) is given by the derivative of the inverse
     * Teten formula.
     */

    dtdew = (t + 237.3) * fac;
    dt = dtdew * (ew - edp);
    t += dt;
    
  } while (fabs(dt) > 1.0e-4);
  
  return t;
}


/********************************************************************
 * PHYdenDWPT()
 *
 * This function returns the dew point (Celsius) given the temperature
 * (Celsius) and relative humidity (%).  The formula is used in the
 * processing of U.S. rawinsonde data and is referenced in Parry, H.
 * Dean, 1969: "The Semiautomatic Computation of Rawinsondes,"
 * Technical Memorandum WBTM EDL 10, U.S. Department of Commerce,
 * Environmental Science Services Administration, Weather Bureau,
 * Office of Systems Development, Equipment Development Laboratory,
 * Silver Spring, MD (October), page 9 and page II-4, line 460.
 */

double PHYdenDWPT(double t, double rh)
{
  double x;
  double dpd;
  
  x = 1.0 - (0.01 * rh);
  
  /* Compute dew point depression. */

  dpd = ((14.55 + (0.114 * t)) * x) +
    pow((2.5 + (0.007 * t)) * x, 3.0) +
    (15.9 + (0.117 * t)) * pow(x, 14.0);
  
  return t - dpd;
}


/********************************************************************
 * PHYdenEPT()
 *
 * This function returns the equivalent potential temperature ept
 * (Celsuis) for a parcel of air initially at temperature t (Celsius),
 * dew point td (Celsius) and pressure p (mb).  The formula used is
 * eq. (43) in Bolton, David, 1980: "The Computation of Equivalent
 * Potential Temperature," Monthly Weather Review, vol 108, no 7 (July),
 * pp 1046-1053.  The maximum error in ept is 0.3C.  In most cases the
 * error is less than 0.1C.
 */

double PHYdenEPT(double t, double td, double p)
{
  double w;
  double tlcl;
  double tk;
  double tl;
  double pt;
  double eptk;
  
  /* Compute the temperature (Celsius) at the lifting condensation level. */

  w = PHYdenWMR(p, td);
  
  /* Compute the temperature (Celsius) at the lifting condensation level. */

  tlcl = PHYdenTCON(t, td);
  tk = TEMP_C_TO_K(t);
  tl = TEMP_C_TO_K(tlcl);
  pt = tk * pow(1000.0 / p, 0.2854 * (1.0 - (0.00028 * w)));
  eptk = pt * exp(((3.376 / tl) - 0.00254) * w * (1.0 + (0.00081 * w)));
  
  return TEMP_K_TO_C(eptk);
}


/********************************************************************
 * PHYdenESAT()
 *
 * This function returns the saturation vapor pressure over water (mb)
 * given the temperature (Celsius).  The algorithm is due to Nordquist,
 * W.S., 1973: "Numerical Approximations of Selected Meteorological
 * Parameters for Cloud Physics Problems," ECOM-5475, Atmospheric Sciences
 * Laboratory, US Army Electronics Command, White Sands Missile Range,
 * New Mexico 88002.
 */

double PHYdenESAT(double t)
{
  double tk;
  double p1;
  double p2;
  double c1;
  
  tk = TEMP_C_TO_K(t);
  p1 = 11.344 - (0.0303998 * tk);
  p2 = 3.49149 - (1302.8844 / tk);
  c1 = 23.832241 - (5.02808 * log10(tk));
  
  return pow(10.0,
	     c1 - (1.3816e-7 * pow(10.0, p1)) +
	     (8.1328e-3 * pow(10.0, p2)) - (2949.076 / tk));
}


/********************************************************************
 * PHYdenESGG()
 *
 * This function returns the saturation vapor ressure over liquid
 * water esgg (mb) given the temperature t (Celsius).  The formula
 * used, due to Goff and Gratch, appears on p 350 of the Smithsonian
 * Meteorological Tables, Sixth Revised Edition, 1963, by Roland List.
 */

double PHYdenESGG(double t)
{
  const double EWS = 1013.246;  /* saturation vapor pressure (mb) over liquid */
                      /*   water at 100C */
  const double TS = 373.15;     /* boiling point of water (K) */

  const double ESGG_C1 = -7.90298;
  const double ESGG_C2 = 5.02808;
  const double ESGG_C3 = 1.3816e-7;
  const double ESGG_C4 = 11.344;
  const double ESGG_C5 = 8.1328e-3;
  const double ESGG_C6 = -3.49149;

  double tk;
  double rhs;
  double esw;
  
  tk = TEMP_C_TO_K(t);
  
  /* Goff-Gratch formula */

  rhs = (ESGG_C1 * ((TS / tk) - 1.0)) +
    (ESGG_C2 * log10(TS / tk)) -
    ESGG_C3 * (pow(10.0, ESGG_C4 * (1.0 - (tk / TS))) - 1.0) +
    ESGG_C5 * (pow(10.0, ESGG_C6 * ((TS / tk) - 1.0)) - 1.0) + log10(EWS);
  esw = pow(10.0, rhs);
  if (esw < 0.0)
    esw = 0.0;
  
  return esw;
}


/********************************************************************
 * PHYdenESICE()
 *
 * This function returns the saturation vapor pressure with respect to
 * ice esice (mb) given the temperature t (Celsius).  The formula used
 * is based upon the integration of the Clausius-Clapeyron equation by
 * Goff and Gratch.  The formula appears on p 350 of the Smithsonian
 * Meteorological Tables, Sixth Revised Edition, 1963.
 *
 * Returns 99999.0 if ESICE cannot be calculated.
 */

double PHYdenESICE(double t)
{
  const double EIS = 6.1071;  /* saturation vapor pressure (mb) over a water-ice */
                    /*   mixture at 0C */

/* Empirical coefficients in the Goff-Gratch formule. */

  const double ESICE_C1 = -9.09718;
  const double ESICE_C2 = 3.56654;
  const double ESICE_C3 = 0.876793;

  double tf;
  double tk;
  double rhs;
  double esi;
  
  if (t > 0.0)
  {
    fprintf(stderr, "Saturation vapor pressure for ice cannot be "
	    "computed for temperature > 0C.\n");
    fprintf(stderr, "Returning 99999.0 for ESICE.\n");
    
    return 99999.0;
  }
      
  /* Freezing point of water (K) */

  tf = 273.15;
  tk = TEMP_C_TO_K(t);
  
  /* Goff-Gratch formula */

  rhs = (ESICE_C1 * ((tf / tk) - 1.0)) - (ESICE_C2 * log10(tf / tk)) +
    (ESICE_C3 * (1.0 - (tk / tf))) + log10(EIS);
  esi = pow(10.0, rhs);
  if (esi < 0.0)
    esi = 0.0;
  
  return esi;
}


/********************************************************************
 * PHYdenESILO()
 *
 * This function returns the saturation vapor pressure over ice esilo (mb)
 * given the temperature t (Celsius).  The formula is due to Lowe, Paul R,
 * 1977: An Approximating Polynomial for the Computation of Saturation Vapor
 * Pressure, Journal of Applied Meteorology, vol 16, no 1 (Jan), pp 100-103.
 *
 * Returns 9999.0 if ESILO cannot be calculated.
 */

double PHYdenESILO(double t)
{
/* Polynomial coefficients */

  const double ESILO_A0 = 6.109177956;
  const double ESILO_A1 = 5.034698970e-1;
  const double ESILO_A2 = 1.886013408e-2;
  const double ESILO_A3 = 4.176223716e-4;
  const double ESILO_A4 = 5.824720280e-6;
  const double ESILO_A5 = 4.838803174e-8;
  const double ESILO_A6 = 1.838826904e-10;

  if (t > 0.0)
  {
    fprintf(stderr, "Saturation vapor pressure over ice is undefined for "
	    "temperature > 0C.\n");
    fprintf(stderr, "Returning 9999.0 for ESILO.\n");
    
    return 9999.0;
  }
  
  return ESILO_A0 + (t * (ESILO_A1 + (t * (ESILO_A2 + (t * (ESILO_A3 +
	 (t * (ESILO_A4 + (t * (ESILO_A5 + (ESILO_A6 * t)))))))))));
}


/********************************************************************
 * PHYdenESLO()
 *
 * This function returns the saturation vapor pressure over liquid water
 * eslo (mb) given the temperature t (Celsuis).  The formula is due to
 * Lowe, Paul R, 1977: An Approximating Polynomial for the Computation of
 * Saturation Vapor Pressure, Journal of Applied Meteorology, vol 16,
 * no 1 (January), pp 100-103.
 */

double PHYdenESLO(double t)
{
/* Polynomial coefficients */

  const double ESLO_A0 = 6.107799961;
  const double ESLO_A1 = 4.436518521e-1;
  const double ESLO_A2 = 1.428945805e-2;
  const double ESLO_A3 = 2.650648471e-4;
  const double ESLO_A4 = 3.031240396e-6;
  const double ESLO_A5 = 2.034080948e-8;
  const double ESLO_A6 = 6.136820929e-11;

  double es;
  
  es = ESLO_A0 + (t * (ESLO_A1 + (t * (ESLO_A2 + (t * (ESLO_A3 +
       (t * (ESLO_A4 + (t * (ESLO_A5 + (ESLO_A6 * t)))))))))));
  
  if (es < 0.0)
    es = 0.0;
  
  return es;
}


/********************************************************************
 * PHYdenESRW()
 *
 * This function returns the saturation vapor pressure over liquid water
 * esrw (mb) given the temperature t (Celsius).  The formula used is due
 * to Richards, J. M., 1971: Simple Expression for the Saturation Vapour
 * Pressure of Water in the Range -50 to 140C, British Journal of Applied
 * Physics, vol 4, pp L15-L18.  The formula was quoted more recently by
 * Wigley, T. M. L., 1974: Commens on "A Simple but Accurate Formula for
 * the Saturation Vapor Pressure Over Liquid Water," Journal of Applied
 * Meteorology, vol 13, no 5 (August), p 606.
 */

double PHYdenESRW(double t)
{
  const double TS = 373.15;    /* Temperature of the boiling point of water (K) */
  const double EWS = 1013.25;  /* Saturation vapor pressure over liquid water at 100C */

  const double ESRW_C1 = 13.3185;
  const double ESRW_C2 = -1.9760;
  const double ESRW_C3 = -0.6445;
  const double ESRW_C4 = -0.1299;

  double tk;
  double x;
  double px;
  double vp;
  
  tk = TEMP_C_TO_K(t);
  x = 1.0 - (TS / tk);
  px = x * (ESRW_C1 + (x * (ESRW_C2 + (x * (ESRW_C3 + (ESRW_C4 * x))))));
  vp = EWS * exp(px);
  if (vp < 0.0)
    vp = 0.0;
  
  return vp;
}


/********************************************************************
 * PHYdenESW()
 *
 * This function returns the saturation vapor pressure esw (mb) over
 * liquid water given the temperature t (Celsius).  The polynomial
 * approximation below is due to Herman Wobus, a methmatician who worked
 * at the Navy Weather Reserach Facility, Norfolk, VA, but who is now
 * retired.  The coefficients of the polynomial were chosen to fit the
 * values in table 94 on pp 351-353 of the Smithsonian Meteorological
 * Tables by Roland List (6th Edition).  The approximation is valid for
 * -50 < t < 100C.
 */

double PHYdenESW(double t)
{
  const double ES0 = 6.1078;  /* saturation vapor pressure over liquid water at 0C */

/* Polynomial coefficients */

  const double ESW_A0 = 0.99999683;
  const double ESW_A1 = -0.90826951e-2;
  const double ESW_A2 = 0.78736169e-4;
  const double ESW_A3 = -0.61117958e-6;
  const double ESW_A4 = 0.43884187e-8;
  const double ESW_A5 = -0.29883885e-10;
  const double ESW_A6 = 0.21874435e-12;
  const double ESW_A7 = -0.17892321e-14;
  const double ESW_A8 = 0.11112018e-16;
  const double ESW_A9 = -0.30994571e-19;

  double pol;
  
  pol = ESW_A0 + (t * (ESW_A1 + (t * (ESW_A2 + (t * (ESW_A3 + (t * (ESW_A4 +
       (t * (ESW_A5 + (t * (ESW_A6 + (t * (ESW_A7 + (t * (ESW_A8 +
       (t * ESW_A9)))))))))))))))));
  
  return ES0 / pow(pol, 8.0);
}


/********************************************************************
 * PHYdenES()
 *
 * This function returns the saturation vapor pressure es (mb) over
 * liquid water given the temperature t (Celsius).  The formula appears
 * in Bolton, David, 1980: "The Computation of Equivalent Potential
 * Temperature," Monthly Weather Review, vol 108, no 7 (July), p 1047,
 * eq (10).  The quoted accuracy is 0.3% or better for -35 < t < 35C.
 */

double PHYdenES(double t)
{
  const double ES0 = 6.1121;  /* saturation vapor pressure over liquid water at 0C */

  return ES0 * exp((17.67 * t) / (t + 243.5));
}


/********************************************************************
 * PHYdenHEATL()
 *
 * This function returns the latent heat of:
 *    evaporation/condensation     key = 1
 *    melting/freezing             key = 2
 *    sublimation/deposition       key = 3
 * for water.  The latent heat heatl (J/kg) is a function of temperature
 * t (Celsius).  The formulas are polynomial approximations to the values in
 * table 92, p 343 of the Smithsonian Meteorological Tables, Sixth Revised
 * Edition, 1963 by Roland List.  The approximations were developed by 
 * Eric Smith at Colorado State University.
 *
 * Returns 0.0 if an invalid key is used.
 */

double PHYdenHEATL(int key, double t)
{
/* Polynomial coefficients */

  const double HEATL_A0 = 3337118.5;
  const double HEATL_A1 = -3642.8583;
  const double HEATL_A2 = 2.1263947;

  const double HEATL_B0 = -1161004.0;
  const double HEATL_B1 = 9002.2648;
  const double HEATL_B2 = -12.931292;

  const double HEATL_C0 = 2632536.8;
  const double HEATL_C1 = 1726.9659;
  const double HEATL_C2 = -3.6248111;

  double hltnt = 0.0;
  double tk = TEMP_C_TO_K(t);

  switch (key)
  {
  case 1 :
    hltnt = HEATL_A0 + (HEATL_A1 * tk) + (HEATL_A2 * tk * tk);
    break;
    
  case 2 :
    hltnt = HEATL_B0 + (HEATL_B1 * tk) + (HEATL_B2 * tk * tk);
    break;
    
  case 3 :
    hltnt = HEATL_C0 + (HEATL_C1 * tk) + (HEATL_C2 * tk * tk);
    break;
    
  default :
    fprintf(stderr, "Invalid key value %d.\n", key);
    fprintf(stderr, "Returning 0.0 for HEATL.\n");
    hltnt = 0.0;
    break;
  } /* endswitch - key */
  
  return hltnt;
}


/********************************************************************
 * PHYdenHUM()
 *
 * This function returns relative humidity (%) given the temperature
 * t (Celsius) and dew point td (Celsius).  As calculated here, relative
 * humidity is the ratio of teh actual vapor pressure to the saturation
 * vapor pressure.
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenHUM(double t, double td)
{
  return 100.0 * (PHYdenESAT(td) / PHYdenESAT(t));
}


/********************************************************************
 * PHYdenLCL()
 *
 * This function returns equivalent the LCL height (m) given pressure (mb),
 * height (gpm), temperature (C) and mixing ratio (g/kg).
 *
 * This is based on the LCL calculations found within the CAPE/CIN
 * calculations in the RIP.
 */

double PHYdenLCL(double pressure, double height,
		 double temp, double mixing_ratio)
{
  const double CP = 1004.0;    /* J/K/kg  Note: not using Bolton's value of 1005.7 per RIP code comment */
  const double CPMD = 0.887;
  const double EPS = 0.622;
  const double TLCLC1 = 2840.0;
  const double TLCLC2 = 3.5;
  const double TLCLC3 = 4.805;
  const double TLCLC4 = 55.0;
  const double GRAVITY_CONSTANT = 9.806;      /* m/s^2 */
#define MAX(a, b) ((a) > (b) ? (a) : (b))

  double tk = TEMP_C_TO_K(temp);
  
  double parcel_mixing_ratio = mixing_ratio / 1000.0;  // Convert from g/kg to g/g
  double cpm = CP * (1.0 + CPMD * parcel_mixing_ratio);
  double e = MAX(1.0e-20,
	  parcel_mixing_ratio * pressure /
	  (EPS + parcel_mixing_ratio));
  double lcl_temp = TLCLC1 /
    (log(pow(tk, TLCLC2) / e) - TLCLC3) + TLCLC4;

  return height +
    (tk - lcl_temp) / (GRAVITY_CONSTANT / cpm);
}


/********************************************************************
 * PHYdenOE()
 *
 * This function returns equivalent potential temperature oe (Celsius)
 * of a parcel of air given its temperature t (Celsius), dew point
 * td (Celsius) and pressure p (mb).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenOE(double t, double td, double p)
{
  double atw;
  
  /* Find the wet bulb temperature of the parcel. */

  atw = PHYdenTW(t, td, p);
  
  /* Find the equivalent potential temperature. */

  return PHYdenOS(atw, p);
}


/********************************************************************
 * PHYdenOS()
 *
 * This function returns the equivalent potential temperature os (Celsius)
 * for a parcel of air saturated at temperature t (Celsuis) and pressure
 * p (mb).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenOS(double t, double p)
{
/* B is an empirical constant approximately equal to the latent heat of
 * vaporization for water divided by the specific heat at constant pressure
 * for dry air.
 */

  const double B = 2.6518986;

  double tk = TEMP_C_TO_K(t);
  double osk = tk * pow(1000.0 / p, 0.286) * exp((B * PHYdenW(p, t)) / tk);
  return TEMP_K_TO_C(osk);
}


/********************************************************************
 * PHYdenOW()
 *
 * This function returns the wet-bulb potential temperature ow (Celsius)
 * given the temperature t (Celsius), dew point td (Celsius) and pressure
 * p (mb).  The calculation for ow is very similar to that for wet bulb
 * temperature.  See p 13 Stipanuk (1973).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenOW(double t, double td, double p)
{
  /* Find the wet bulb temperature of the parcel. */

  double atw = PHYdenTW(t, td, p);
  
  /* Find the equivalent potential temperature of the parcel. */

  double aos = PHYdenOS(atw, p);
  
  /* Find the wet-bulb potential temperature. */

  return PHYdenTSA(aos, 1000.0);
}


/********************************************************************
 * PHYdenO()
 *
 * This function returns potential temperature (Celsius) given 
 * temperature t (Celsius) and pressure p (mb) by solving the
 * Poisson equation.
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenO(double t, double p)
{
  double tk;
  double ok;
  
  tk = TEMP_C_TO_K(t);
  ok = tk * pow((1000.0 / p), 0.286);

  return TEMP_K_TO_C(ok);
}


/********************************************************************
 * PHYdenPCCL()
 *
 * This function returns the pressure at the convective condensation level
 * given the appropriate sounding data.
 * On input:
 *    pm = pressure (mb) at upper boundary of the layer for computing the
 *         mean mixing ratio. p[0] is the lower boundary.
 *    p = pressure (mb). Note that p[i] > p[i+1].
 *    t = temperature (Celsius)
 *    td = dew point (Celsius)
 *    n = number of levels in the sounding and the dimension of p, t and td
 * On output:
 *    wbar = mean mixing ratio (g/kg) in the layer bounded by pressure
 *           p[0] at the bottom and pm at the top.
 * Returns the pressure (mb) at the convective condensation level.
 *
 * The algorithm is described on p 17 of Stipanuk, G.S., 1973: "Algorithms
 * for Generating a Skew-T Log P Diagram and Computing Selected Meteorological
 * Quantities," Atmospheric Sciences Laboratory, US Army Electronics Command,
 * White Sands Missile Range, NM 88002.
 */

double PHYdenPCCL(double pm, double *p, double *t, double *td,
		  double *wbar, int n)
{
  double pc;
  int k;
  int j;
  int i;
  int l;
  double tq;
  double x;
  double a;
  
  if (pm == p[0])
  {
    *wbar = PHYdenW(p[0], td[0]);
    pc = pm;
    
    if (fabs(t[0] - td[0]) < 0.05)
    {
      return pc;
    }
  }
  else
  {
    *wbar = 0.0;
    k = 0;
    
    while (p[k] > pm)
    {
      ++k;
    }
    
    --k;
    j = k - 1;

    if (j >= 0)
    {
      /* Compute the average mixing ratio */

      for (i = 0; i <= j; ++i)
      {
	l = i + 1;
	*wbar += (PHYdenW(p[i], td[i]) + PHYdenW(p[l], td[l])) *
	  log(p[i] / p[l]);
      } /* endfor - i */
      
    } /* endif - j >= 0 */
    
    l = k + 1;
    
    /* Estimate the dew point at pressure pm */

    tq = td[k] + (td[l] - td[k]) * log(pm / p[k]) / log(p[l] / p[k]);
    *wbar += (PHYdenW(p[k], td[k]) + PHYdenW(pm, tq)) * log(p[k] / pm);
    *wbar /= 2.0 * log(p[1] / pm);
  }
  
  /* Find level at which the mixing ratio line wbar crosses the
     environmental temperature profile. */

  for (j = 0; j <= n; ++j)
  {
    i = n - j + 1;

    if (p[i] < 300.0)
      continue;
    
    /* TMR = temperature (C) at pressure p (mb) along a mixing ratio
       line given by wbar (g/kg). */

    x = PHYdenTMR(*wbar, p[i]) - t[i];
    
    if (x <= 0.0)
    {
      double del;

      /* Set up bisection routine */

      l = i;
      ++i;
      del = p[l] - p[i];
      pc = p[i] + (0.5 * del);
      a = (t[i] - t[l]) / log(p[l] / p[i]);
      
      for (j = 0; j < 10; ++j)
      {
	double del_sign;
	
	del /= 2;
	x = PHYdenTMR(*wbar, pc) - t[l] - (a * log(p[l] / pc));
	
	del_sign = del;
	
	if ((del < 0.0 && x > 0.0) ||
	    (del > 0.0 && x < 0.0))
	{
	  del_sign *= -1.0;
	}
	
	pc += del_sign;
	
      } /* endfor - j */
      
      return pc;
    }
    
  } /* endfor - j */
  
  return 0.0;
}


/********************************************************************
 * PHYdenCON()
 *
 * This function returns the pressure pcon (mb) at the lifted condensation
 * level, given the initial pressure p (mb) and temperature t (Celsius)
 * of the parcel and the temperature tc (Celsius) at the LCL.  The
 * algorithm is exact.  It makes use of the formula for the potential
 * temperature corresponding to t at p and tc at pcon.  These two potential
 * temperatures are equal.
 */

double PHYdenPCON(double p, double t, double tc)
{
/* AKAPI = (specific heat at constant pressure for dry air) /
 *         (gas constant for dry air)
 */

  const double AKAPI = 3.5037;

  double tk = TEMP_C_TO_K(t);
  double tck = TEMP_C_TO_K(tc);
  
  return p * pow(tck / tk, AKAPI);
}


/********************************************************************
 * PHYdenPOWT()
 *
 * This function yields wet-bulb potential temperature powt (Celsius),
 * given the following input:
 *    t = temperature (Celsius)
 *    p = pressure (mb)
 *    td = dew point (Celsius)
 */

double PHYdenPOWT(double t, double p, double td)
{
/* AKAP = (gas constant for dry air) /
 *        (specific heat at constant pressure for dry air)
 */

  const double AKAP = 0.28541;

  double tk;
  double ptk;
  double pt;
  double tc;
  
  /* Compute the potential temperature (Celsius). */

  tk = TEMP_C_TO_K(t);
  ptk = tk * pow(1000.0 / p, AKAP);
  pt = TEMP_K_TO_C(ptk);
  
  /* Compute the lifting condensation level (LCL). */

  tc = PHYdenTCON(t, td);
  
  /* For the origin of the following approximation, see the documentation
   * for the Wobus function.
   */

  return pt - PHYdenWOBF(pt) + PHYdenWOBF(tc);
}


/********************************************************************
 * PHYdenPRECPW()
 *
 * This function computes total precipitable water precpw (cm) in a
 * vertical column of air based upon sounding data at n levels.
 *    td = dew opint (Celsius)
 *    p = pressure (mb)
 * Calculations are done in CGS units.
 */

double PHYdenPRECPW(double *td, double *p, int n)
{
  const double G = 980.616;  /* Acceleration due to the Earth's gravity (cm/s**2) */

  double pw;
  int nl;
  double wbot;
  int i;
  
  /* Initialize value of precipitable water. */

  pw = 0.0;
  nl = n - 1;
  
  /* Calculate the mixing ratio at the lowest level. */

  wbot = PHYdenWMR(p[0], td[0]);
  
  for (i = 0; i < nl; ++i)
  {
    double wtop;
    double w;
    double wl;
    double ql;
    double dp;
    
    wtop = PHYdenWMR(p[i+1], td[i+1]);
    
    /* Calculate the layer-mean mixing ratio (g/kg). */

    w = (wtop + wbot) / 2.0;
    
    /* Make the mixing ratio dimensionless. */

    wl = 0.001 * w;
    
    /* Calculate the specific humidity. */

    ql = wl / (wl + 1.0);
    
    /* The factor of 1000.0 below converts the bm to dynes/cm**2. */

    dp = 1000.0 * (p[i] - p[i+1]);
    pw += (ql / G) * dp;
    wbot = wtop;
  } /* endfor - i */
  
  return pw;
}


/********************************************************************
 * PHYdenPTLCL()
 *
 * This subroutine estimates the pressure pc (mb) and the temperature
 * tc (Celsius) at the lifted condensation level (LCL), given the initial
 * pressure p (mb), temperature t (Celsius) and dew point (Celsius) of the
 * parcel.  The approximation is that lines of constant potential temperature
 * and constant mixing ratio are straight on the skew t/log p chart.  Teten's
 * formula for saturation vapor pressure as a function of pressure was
 * used in the derivation of the formula below.  For additional details,
 * see math notes by T. Schlatter dated 8 Sept 1981.
 */

void PHYdenPTLCL(double p, double t, double td, double *pc, double *tc)
{
/* AKAP = (gas constant for dry air) /
 *        (specific heat at constant pressure for dry air)
 */

  const double AKAP = 0.28541;

  double tk = TEMP_C_TO_K(t);
  double c1 = 4098.026 / pow(td + 237.2, 2.0);
  double c2 = 1.0 / (AKAP * tk);
  *pc = p * exp(c1 * c2 * (t - td) / (c2 - c1));
  *tc = t + (c1 * (t - td) / (c2 - c1));
}


/********************************************************************
 * PHYdenSATLFT()
 *
 * Input:  thw = wet-bulb potential temperature (Celsius).
 *               thw defines a moist adiabat.
 *         p = pressure (mb)
 * Return: temperature (Celsius) where the moist adiabat crosses p
 *
 * This algorithm can best be understood by referring to a skew t/log p
 * chart.  It was devised by Herman Wobus, a mathematician formerly at the
 * Navy Weather Research Facility but now retired.  The value returned by
 * SATLFT can be checked by referring to table 78, pp 319-322, Smithsonian
 * Meteorological Tables, by Roland List (6th Revised Edition).
 */

double PHYdenSATLFT(double thw, double p)
{
/* AKAP = (gas constant for dry air) /
 *        (specific heat at constant pressure for dry air)
 */

  const double AKAP = 0.28541;

  double pwrp;
  double tone;
  double eone;
  double rate;
  double ttwo;
  double dlt;
  
  if (p == 1000.0)
    return thw;
  
  /* Compute tone, the temperature where the dry adiabat with value thw
   * (Celsius) crosses p.
   */

  pwrp = pow(p / 1000.0, AKAP);
  tone = TEMP_K_TO_C(TEMP_C_TO_K(thw) * pwrp);
  
  /* Consider the moist adiabat ew1 through tone at p.  Using the definition
   * of the Wobus function (see documenation on PHYdenWOBF), it can be shown
   * that eone = ew1 - thw.
   */

  eone = PHYdenWOBF(tone) - PHYdenWOBF(thw);
  rate = 1.0;
  
  while (1)
  {
    double pt;
    double etwo;
    
    /* ttwo is an improved estimate of satlft. */

    ttwo = tone - (eone * rate);
    
    /* pt is the potential temperature (Celsius) corresponding to ttwo at p */

    pt = TEMP_K_TO_C(TEMP_C_TO_K(ttwo) / pwrp);
    
    /* Consider the moist adiabat ew2 through ttwo at p.  Using the definition
     * of the Wobus function, it can be shown that etwo = ew2 - thw.
     */

    etwo = pt + PHYdenWOBF(ttwo) - PHYdenWOBF(pt) - thw;
    
    /* dlt is the correction to be subtracted from ttwo. */

    dlt = etwo * rate;
    
    if (fabs(dlt) <= 0.1)
      break;
    
    /* rate is the ratio of a change in t to the corresponding change in e.
     * Its initial value was set to 1.0 above.
     */

    rate = (ttwo - tone) / (etwo - eone);
    tone = ttwo;
    eone = etwo;
  }
  
  return ttwo - dlt;
}


/********************************************************************
 * PHYdenSSH()
 *
 * This function returns saturation specific humidity ssh (grams of water
 * vapor per kg of moist air) given the pressure p (mb) and the temperature
 * t (Celsius).  The equation is given in standard meteorological texts.
 * If t is dew point (Celsius), then PHYdenSSH returns the actual specific
 * humidity.
 */

double PHYdenSSH(double p, double t)
{
  /* Compute the dimensionless mixing ratio. */

  double w = 0.001 * PHYdenWMR(p, t);
  
  /* Compute the dimensionless saturation specific humidity. */

  double q = w / (1.0 + w);
  
  return 1000.0 * q;
}


/********************************************************************
 * PHYdenTCON()
 *
 * This function returns the temperature tcon (Celsius) at the lifting
 * condensation level, given the temperature t (Celsius) and the dew point
 * td (Celsius).
 */

double PHYdenTCON(double t, double td)
{
  /* Compute the dew point depression */

  double s = t - td;
  
  /* The approximation below, a third order polynomial in s and t, is due
   * to Herman Wobus.  The source of data for fitting the polynomial is
   * unknown.
   */

  double dlt = s * (1.2185 + (1.278e-3 * t) +
		    (s * (-2.19e-3 + (1.173e-5 * s) - (5.2e-6 * t))));
  
  return t - dlt;
}


/********************************************************************
 * PHYdenTDA()
 *
 * This function returns the temperature TDA (Celsius) on a dry adiabat
 * at pressure p (mb).  The dry adiabat is given by potential temperature
 * o (Celsius).  The computation is based on Poisson's equation.
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenTDA(double o, double p)
{
  double ok;
  double tdak;
  
  ok = TEMP_C_TO_K(o);
  tdak = ok * pow((p * 0.001), 0.286);
  
  return TEMP_K_TO_C(tdak);
}


/********************************************************************
 * PHYdenTE()
 *
 * This function returns the equivalent temperature te (Celsius) of a parcel
 * of air given its temperature t (Celsius), dew point dt (Celsius) and
 * pressure p (mb).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenTE(double t, double td, double p)
{
  /* Calculate equivalent potential temperature. */

  double aoe = PHYdenOE(t, td, p);
  
  /* Use Poisson's equation to calculate equivalent temperature. */

  return PHYdenTDA(aoe, p);
}


/********************************************************************
 * PHYdenTHM()
 *
 * This function returns the wet-bulb potential temperature thm (Celsius)
 * corresponding to a parcel of air saturated at temperature t (Celsius)
 * and pressure p (mb).
 */

double PHYdenTHM(double t, double p)
{
#define F(x) (1.8199427e1 + x * (2.16408e-1 + x * (3.071631e-4 + x * (-3.895366e-6 + x * (1.96182e-8 + x * (5.293557e-11 + x * (7.399595e-14 + x * (-4.19835e-17))))))))

  double thd;
  
  if (p == 1000.0)
    return t;
  
  /* Compute the potential temperature (Celsius). */

  thd = TEMP_K_TO_C(TEMP_C_TO_K(t) * pow(1000.0 / p, 0.286));
  return thd + (6.071 * (exp(t / F(t)) - exp(thd / F(thd))));
}


/********************************************************************
 * PHYdenTLCL1()
 *
 * This function returns the temperature tlcl1 (Celsius) of the lifting
 * condensation level (LCL) given the initial temperature t (Celsius) and
 * dew point td (Celsius) of a parcel of air.  Eric Smith at Colorado
 * State University has used the formula used in this function, but its
 * origin is unknown.
 */

double PHYdenTLCL1(double t, double td)
{
  double tk = TEMP_C_TO_K(t);
  
  /* Compute the parcel vapor pressure (mb). */

  double es = PHYdenESLO(td);
  double tlcl = 2840.0 / ((3.5 * log(tk)) - log(es) - 4.805) + 55.0;

  return TEMP_K_TO_C(tlcl);
}


/********************************************************************
 * PHYdenTLCL()
 *
 * Thsi function yields the temperature tlcl (Celsius) of the lifting
 * condensation level, given the temperature t (Celsius) and the dew point
 * td (Celsius).  The formula used is in Bolton, David, 1980: "The
 * Computation of Equivalent Potential Temperature," Monthly Weather
 * Review, vol 108, no 7 (July), p 1048, eq (15).
 */

double PHYdenTLCL(double t, double td)
{
  double tk = TEMP_C_TO_K(t);
  double tdk = TEMP_C_TO_K(td);
  double a = 1.0 / (tdk - 56.0);
  double b = log(tk / tdk) / 800.0;
  double tc = (1.0 / (a + b)) + 56.0;
  
  return TEMP_K_TO_C(tc);
}


/********************************************************************
 * PHYdenTMLAPS()
 *
 * This function returns the temperature tmlaps (Celsius) at pressure p (mb)
 * along the moist adiabat corresponding to an equivalent potential
 * temperature thetae (Celsius).  The algorithm was written by Eric Smith
 * at Colorado State University.
 */

double PHYdenTMLAPS(double thetae, double p)
{
  const double CRIT = 0.1;   /* Convergence criterion (Kelvin) */

  double eq0;
  double eq1;
  double tlev;
  double dif;
  double dt;    /* Initial stepping increment */
  int i;
  int j;
  
  eq0 = thetae;
  
  /* Initial guess for solution. */

  tlev = 25.0;
  
  /* Compute the saturation equivalent potential temperature corresponding
   * to temperature tlev and pressure p.
   */

  eq1 = PHYdenEPT(tlev, tlev, p);
  dif = fabs(eq1 - eq0);
  
  if (dif < CRIT)
    return tlev;
  
  if (eq1 > eq0)
  {
    dt = -10.0;
    i = 1;
  }
  else
  {
    dt = 10.0;
    i = -1;
  }
  
  while (1)
  {
    tlev += dt;
    eq1 = PHYdenEPT(tlev, tlev, p);
    dif = fabs(eq1 - eq0);
  
    if (dif < CRIT)
      return tlev;
  
    j = -1;
    if (eq1 > eq0)
      j = 1;
    
    if (i == j)
      continue;
    
    /* The solution has been passed.  Reverse the direction of search and
     * decrease the stepping increment.
     */

    tlev -= dt;
    dt /= 10.0;
  }
}


/********************************************************************
 * PHYdenTMR()
 *
 * This function returns the temperature (Celsius) on a mixing ratio
 * line w (g/kg) at pressure p (mb).  The formula is given in table 1
 * on page 7 of Stipanuk (1973).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenTMR(double w, double p)
{
  const double TMR_C1 = 0.0498646455;
  const double TMR_C2 = 2.4082965;
  const double TMR_C3 = 7.07475;
  const double TMR_C4 = 38.9114;
  const double TMR_C5 = 0.0915;
  const double TMR_C6 = 1.2035;

  double x;
  double tmrk;
  
  x = log10((w * p) / (622.0 + w));
  tmrk = pow(10.0, (TMR_C1 * x) + TMR_C2) - TMR_C3 +
    (TMR_C4 * pow(pow(10.0, TMR_C5 * x) - TMR_C6, 2.0));
  
  return TEMP_K_TO_C(tmrk);
}


/********************************************************************
 * PHYdenTSA()
 *
 * This function returns the temperature tsa (Celsius) on a saturation
 * adiabat at pressure p (mb).  os is the equivalent potential temperature
 * of the parcel (Celsius).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenTSA(double os, double p)
{
/* B is an empirical constant approximately equal to 0.001 of the latent
 * heat of vaporization for water divided by the specific heat at constant
 * pressure of dry air.
 */

  const double B = 2.6518986;

  double a = TEMP_C_TO_K(os);
  double tq = 253.15;      /* First guess for tsa */
  double d = 120.0;        /* Initial value used in iteration below */
  int i;
  double tqk;
  double x;
  
  /* Iterate to obtain sufficient accuracy... See table 1, p8 of
   * Stipanuk (1973) for equation used in iteration.
   */

  for (i = 0; i < 12; ++i)
  {
    double d_sign;
    
    tqk = TEMP_K_TO_C(tq);
    d /= 2.0;
    
    x = (a * exp(-B * PHYdenW(p, tqk) / tq)) - (tq * pow(1000.0 / p, 0.286));
    
    if (fabs(x) < 0.1e-7)
      break;
    
    d_sign = d;
    if ((d < 0 && x > 0) ||
	(d > 0 && x < 0))
      d_sign *= -1.0;
    
    tq += d_sign;
  } /* endfor - i */
  
  return TEMP_K_TO_C(tq);
}


/********************************************************************
 * PHYdenTV()
 *
 * This function returns the virtual temperature tv (Celsius) of a parcel
 * of air at temperature t (Celsius), dew point td (Celsius) and pressure
 * p (mb).  The equation appears in most standard meteorological texts.
 */

double PHYdenTV(double t, double td, double p)
{
/* EPS = ratio of the mean molecular weight of water (18.016 g/mole)
 * to that of dry air (28.966 g/mole)
 */

  const double EPS = 0.62197;

  double tk = TEMP_C_TO_K(t);
  
  /* Calculate the dimensionless mixing ratio. */

  double w = 0.001 * PHYdenWMR(p, td);
  
  return TEMP_K_TO_C(tk * (1.0 + w / EPS) / (1.0 + w));
}


/********************************************************************
 * PHYdenTW()
 *
 * This function returns the wet-bulb temperature tw (Celsius) given the
 * temperature t (Celsius), dew point td (Celsius) and pressure p (mb).
 * See p 13 in Stipanuk (1973), referenced below, for a description of the
 * technique.
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenTW(double t, double td, double p)
{
  double aw;
  double ao;
  double pi;
  int i;
  double x;
  double ti;
  double aos;
  
  /* Determine the mixing ratio line through td and p. */

  aw = PHYdenW(p, td);
  
  /* Determine the dry adiabat through t and p. */

  ao = PHYdenO(t, p);
  
  /* Iterate to locate pressure pi at the intersection of the two curves.
   */

  pi = p;

  for (i = 0; i < 10; ++i)
  {
    x = 0.02 * (PHYdenTMR(aw, pi) - PHYdenTDA(ao, pi));

    if (fabs(x) < 0.01)
      break;
    
    pi *= pow(2.0, x);
  } /* endfor - i */
  
  /* Find the temperature on the dry adiabat ao at pressure pi. */

  ti = PHYdenTDA(ao, pi);
  
  /* The intersection has been located.  Now find a saturation adiabat through
   * this point.  Function PHYdenOS() returns the equivalent potential
   * temperature (C) of a parcel saturated at temperature ti and pressure pi.
   */

  aos = PHYdenOS(ti, pi);
  
  /* Function PHYdenTSA() returns the wet-bulb temperature (C) of a parcel
   * at pressure p whose equivalent potential temperature is aos.
   */

  return PHYdenTSA(aos, p);
}


/********************************************************************
 * PHYdenWMR()
 *
 * This function approximates the mixing ratio wmr (grams of water vapor
 * per kg of dry air) given the pressure p (mb) and the temperature t
 * (Celsius). The formula used is given on p 102 of the Smithsonian
 * Meteorological Tables by Roland List (6th Edition).
 */

double PHYdenWMR(double p, double t)
{
/* EPS = ratio of the mean molecular weight of water (18.016 g/mole)
 * to that of dry air (28.966 g/mole).
 */

  const double EPS = 0.62197;

  /* The next two lines contain a formula by Herman Wobus for the correction
   * factor wfw for the departure of the mixture of air and water vapor
   * from the ideal gas law.  The formula fits values in table 89, p 340
   * of the Smithsonian Meteorological Tables, but only for temperatures
   * and pressures normally encountered in the atmosphere.
   */

  double x = 0.02 * (t - 12.5 + (7500.0 / p));
  double wfw = 1.0 + (4.5e-6 * p) + (1.4e-3 * x * x);
  double fwesw = wfw * PHYdenESW(t);
  double r = EPS * fwesw / (p - fwesw);
  
  /* Convert r from a dimensionless ratio to g/kg. */

  return 1000.0 * r;
}


/********************************************************************
 * PHYdenWOBF()
 *
 * This function calculates the difference of the wet-bulb potential
 * temperature for saturated and dry air given the temperature.
 *
 * Let wbpts = wet-bulb potential temperature for saturated air at temperature
 * t (Celsius). Let wbptd = wet-bulb potential temperature for completely dry
 * air at the same temperature t.  The Wobus function WOBF (in degrees
 * Celsius) is defined by:
 *        WOBF(t) = wbpts - wbptd
 * Although wbpts and wbptd are functions of both pressure and temperature,
 * their difference is a function of temperature only.
 *
 * To understand why, consider a parcel of dry air at temperature t and
 * pressure p.  The thermodynamic state of the parcel is represented by a
 * point on a pseudoadiabatic chart.  The wet-bulb potential temperature
 * curve (moist adiabat) passing through this point is wbpts.  Now t is the
 * equivalent temperature for another parcel saturated at some lower
 * temperature tw, but at the same pressure p.  To find tw, ascend along the
 * dry adiabat through (t, p).  At a great height, the dry adiabat and some
 * moist adiabat will nearly coincide.  Descend along this moist adiabat back
 * to p.  The parcel temperature is now tw.  The wet-bulb potential
 * temperature curve (moist adiabat) through (tw, p) is wbptd.  The difference
 * (wbpts - wbptd) is proportional to the heat imparted to a parcel saturated
 * at temperature tw if all its water vapor were condensed, since the amount
 * of water vapor a parcel can hold depends upon temperature alone,
 * (wbptd - wbpts) must depend on temperature alone.
 *
 * The Wobus function is useful for evaluating several thermodynamic
 * quantities.  By definition:
 *       WOBF(t) = wbpts - wbptd.                  (1)
 * If t is at 1000 mb, then t is a potential temperature pt and wbpts = pt.
 * Thus
 *       WOBF(pt) = pt - wbptd.                    (2)
 * If t is at the condensation level, then t is the condensation temperature
 * tc and wbpts is the wet-bulb potential temperature wbpt.  Thus
 *       WOBF(tc) = wbpt - wbptd.                  (3)
 * If wbptd is eliminated from (2) and (3), there results
 *      wbpt = pt - WOBF(pt) + WOBF(tc).
 * If wbptd is eliminated from (1) and (2), there results
 *      wbpts = pt - WOBF(pt) + WOBF(t).
 *
 * If t is an equivalent potential temperature ept (implying that the air at
 * 1000 mb is completely dry), then wbpts = ept and wbptd = wbpt.  Thus
 *      WOBF(ept) = ept - wbpt.
 * This form is the basis for a polynomial approximation to WOBF.  In table 78
 * on pp 319-322 of the Smithsonian Meteorological Tables by Roland List
 * (6th Revised Edition), one finds wet-bulb potential temperatures and the
 * corresponding equivalent potential temperatures listed together.
 * Herman Wobus, a methmatician formerly at the Navy Weather Research
 * Facility, Norfolk, VA, and now retired, computed the coefficients for the
 * polynomial approximation from numbers in this table.
 *
 *                              Notes by T.W. Schlatter
 *                              NOAA/ERL/PROFS Program Office
 *                              August 1981
 */

double PHYdenWOBF(double t)
{
  double x;
  double pol;
  
  x = t - 20.0;
  
  if (x > 0.0)
  {
    pol = 1.0 + x * (3.6182989e-3 + x * (-1.3603273e-5 +
        x * (4.9618922e-7 + x * (-6.1059365e-9 + x * (3.9401551e-11 +
        x * (-1.2588129e-13 + x * (1.668828e-16)))))));

    return 29.930 / pow(pol, 4.0) + 0.96 * x - 14.8;
  }
  else
  {
    pol = 1.0 + x * (-8.8416605e-3 + x * (1.4714143e-4 +
      x * ( -9.671989e-7 + x * (-3.2607217e-8 + x * (-3.8598073e-10)))));
    
    return 15.13 / pow(pol, 4.0);
  }
  
}


/********************************************************************
 * PHYdenW()
 *
 * This function returns the mixing ratio (grams of water vapor per
 * kg of dry air) given the dew point (Celsius) and pressure (mb).
 * If the temperature is input instead of the dew point, then
 * saturation mixing ratio (same units) is returned.  The formula is
 * found in most meteorological texts.
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenW(double p, double t)
{
  double x;
  
  x = PHYdenESAT(t);

  return 622.0 * x / (p - x);
}


/********************************************************************
 * PHYdenZ()
 *
 * This function returns the thickness of a layer bounded by pressure p[0]
 * at the bottom and pressure pt at the top.
 * On input:
 *    p = pressure (mb).  Note that p[i] > p[i+1]
 *    t = temperature (Celsuis)
 *    td = dew point (Celsius)
 *    n = number of levels in the sounding and the dimension of p, t and td.
 * Returns:
 *    the geometric thickness of the layer (m) or -1.0 if the thickness
 *      cannot be determined
 * The algorithm involves numerical integration of the hydrostatic equation
 * from p[0] to pt.  It is described on p 15 of Stipanuk (1973).
 *
 * Reference: Stipanuk paper entitled: "Algorithms for Generating a Skew-T,
 * Log P Diagram and Computing Selected Meteorological Quantities,"
 * Atmospheric Sciences Laboratory, US Army Electronics Command, White Sands
 * Missile Range, NM 88002.
 */

double PHYdenZ(double pt, double *p, double *t, double *td, int n)
{
/* C1 = 0.001 * (1.0 / EPS - 1.0) where EPS = 0.62197 is the ratio of the
 * molecular weight of water to that of dry air.  The factor 1000.0 converts
 * the mixing ratio w from g/kg to a dimensionless ratio.
 */

  const double C1 = 0.0006078;

/* C2 = R / (2.0 * G) where R is the gas constant for dry air (287 kg/J/K)
 * and G is the acceleration due to the Earth's gravity (9.8 m/s**2).  The
 * factor of 2 is used in averaging the two virtual temperatures.
 */

  const double C2 = 14.64285;

  double *tk;
  int i;
  int j;
  double a1;
  double a2;
  double z;
  
  tk = umalloc(n * sizeof(double));
  
  for (i = 0; i < n; ++i)
  {
    tk[i] = TEMP_C_TO_K(t[i]);
  }
  
  z = 0.0;
  
  if (pt < p[n-1])
    return -1.0;
  
  i = -1;
  
  while (1)
  {
    ++i;
    j = i + 1;
    
    if (pt >= p[j])
      break;

    a1 = tk[j] * (1.0 + (C1 * PHYdenW(p[j], td[j])));
    a2 = tk[i] * (1.0 + (C1 * PHYdenW(p[i], td[i])));
    z += C2 * (a1 + a2) * log(p[i] / p[j]);
  }
  
  a1 = tk[j] * (1.0 + (C1 * PHYdenW(p[j], td[j])));
  a2 = tk[i] * (1.0 + (C1 * PHYdenW(p[i], td[i])));
  z += C2 * (a1 + a2) * log(p[i] / pt);
  
  return z;
}
