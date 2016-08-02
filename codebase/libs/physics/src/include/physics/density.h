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
 * density.h : Density routines
 *
 ************************************************************************/

#ifndef DENSITY_WAS_INCLUDED
#define DENSITY_WAS_INCLUDED

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

extern double PHYdenALCL(double p, double t, double td);
   
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

extern double PHYdenALCLM(double t, double td, double p);
   
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

extern double PHYdenCT(double wbar, double pc, double ps);
   
/********************************************************************
 * PHYdenDEWPT()
 *
 * This function yields the dew point dewpt (Celsius), given the water
 * vapor pressure ew (mb).  The empirical formula appears in Bolton,
 * David, 1980: "The Computation of Equivalent Potential Temperature,"
 * Monthly Weather Review, vol 108, no 7 (July), p 1047, eq (11).
 * The quoted accuracy is 0.03C or less for -35 < dewpt < 35C.
 */

extern double PHYdenDEWPT(double ew);
   
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

extern double PHYdenDPT(double ew);
   
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

extern double PHYdenDWPT(double t, double rh);
   
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

extern double PHYdenEPT(double t, double td, double p);
   
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

extern double PHYdenESAT(double t);
   
/********************************************************************
 * PHYdenESGG()
 *
 * This function returns the saturation vapor ressure over liquid
 * water esgg (mb) given the temperature t (Celsius).  The formula
 * used, due to Goff and Gratch, appears on p 350 of the Smithsonian
 * Meteorological Tables, Sixth Revised Edition, 1963, by Roland List.
 */

extern double PHYdenESGG(double t);
   
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

extern double PHYdenESICE(double t);
   
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

extern double PHYdenESILO(double t);
   
/********************************************************************
 * PHYdenESLO()
 *
 * This function returns the saturation vapor pressure over liquid water
 * eslo (mb) given the temperature t (Celsuis).  The formula is due to
 * Lowe, Paul R, 1977: An Approximating Polynomial for the Computation of
 * Saturation Vapor Pressure, Journal of Applied Meteorology, vol 16,
 * no 1 (January), pp 100-103.
 */

extern double PHYdenESLO(double t);
   
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

extern double PHYdenESRW(double t);
   
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

extern double PHYdenESW(double t);
   
/********************************************************************
 * PHYdenES()
 *
 * This function returns the saturation vapor pressure es (mb) over
 * liquid water given the temperature t (Celsius).  The formula appears
 * in Bolton, David, 1980: "The Computation of Equivalent Potential
 * Temperature," Monthly Weather Review, vol 108, no 7 (July), p 1047,
 * eq (10).  The quoted accuracy is 0.3% or better for -35 < t < 35C.
 */

extern double PHYdenES(double t);
   
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

extern double PHYdenHEATL(int key, double t);
   
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

extern double PHYdenHUM(double t, double td);
   
/********************************************************************
 * PHYdenLCL()
 *
 * This function returns equivalent the LCL height (m) given pressure (mb),
 * height (gpm), temperature (C) and mixing ratio (g/kg).
 *
 * This is based on the LCL calculations found within the CAPE/CIN
 * calculations in the RIP.
 */

extern double PHYdenLCL(double pressure, double height,
			double temp, double mixing_ratio);
   

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

extern double PHYdenOE(double t, double td, double p);
   
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

extern double PHYdenOS(double t, double p);
   
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

extern double PHYdenOW(double t, double td, double p);
   
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

extern double PHYdenO(double t, double p);
   
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

extern double PHYdenPCCL(double pm, double *p, double *t, double *td,
			 double *wbar, int n);
   
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

extern double PHYdenPCON(double p, double t, double tc);
   
/********************************************************************
 * PHYdenPOWT()
 *
 * This function yields wet-bulb potential temperature powt (Celsius),
 * given the following input:
 *    t = temperature (Celsius)
 *    p = pressure (mb)
 *    td = dew point (Celsius)
 */

extern double PHYdenPOWT(double t, double p, double td);
   
/********************************************************************
 * PHYdenPRECPW()
 *
 * This function computes total precipitable water precpw (cm) in a
 * vertical column of air based upon sounding data at n levels.
 *    td = dew opint (Celsius)
 *    p = pressure (mb)
 * Calculations are done in CGS units.
 */

extern double PHYdenPRECPW(double *td, double *p, int n);
   
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

extern void PHYdenPTLCL(double p, double t, double td, double *pc, double *tc);
   
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

extern double PHYdenSATLFT(double thw, double p);
   
/********************************************************************
 * PHYdenSSH()
 *
 * This function returns saturation specific humidity ssh (grams of water
 * vapor per kg of moist air) given the pressure p (mb) and the temperature
 * t (Celsius).  The equation is given in standard meteorological texts.
 * If t is dew point (Celsius), then PHYdenSSH returns the actual specific
 * humidity.
 */

extern double PHYdenSSH(double p, double t);
   
/********************************************************************
 * PHYdenTCON()
 *
 * This function returns the temperature tcon (Celsius) at the lifting
 * condensation level, given the temperature t (Celsius) and the dew point
 * td (Celsius).
 */

extern double PHYdenTCON(double t, double td);
   
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

extern double PHYdenTDA(double o, double p);
   
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

extern double PHYdenTE(double t, double td, double p);
   
/********************************************************************
 * PHYdenTHM()
 *
 * This function returns the wet-bulb potential temperature thm (Celsius)
 * corresponding to a parcel of air saturated at temperature t (Celsius)
 * and pressure p (mb).
 */

extern double PHYdenTHM(double t, double p);
   
/********************************************************************
 * PHYdenTLCL1()
 *
 * This function returns the temperature tlcl1 (Celsius) of the lifting
 * condensation level (LCL) given the initial temperature t (Celsius) and
 * dew point td (Celsius) of a parcel of air.  Eric Smith at Colorado
 * State University has used the formula used in this function, but its
 * origin is unknown.
 */

extern double PHYdenTLCL1(double t, double td);
   
/********************************************************************
 * PHYdenTLCL()
 *
 * Thsi function yields the temperature tlcl (Celsius) of the lifting
 * condensation level, given the temperature t (Celsius) and the dew point
 * td (Celsius).  The formula used is in Bolton, David, 1980: "The
 * Computation of Equivalent Potential Temperature," Monthly Weather
 * Review, vol 108, no 7 (July), p 1048, eq (15).
 */

extern double PHYdenTLCL(double t, double td);
   
/********************************************************************
 * PHYdenTMLAPS()
 *
 * This function returns the temperature tmlaps (Celsius) at pressure p (mb)
 * along the moist adiabat corresponding to an equivalent potential
 * temperature thetae (Celsius).  The algorithm was written by Eric Smith
 * at Colorado State University.
 */

extern double PHYdenTMLAPS(double thetae, double p);
   
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

extern double PHYdenTMR(double w, double p);
   
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

extern double PHYdenTSA(double os, double p);
   
/********************************************************************
 * PHYdenTV()
 *
 * This function returns the virtual temperature tv (Celsius) of a parcel
 * of air at temperature t (Celsius), dew point td (Celsius) and pressure
 * p (mb).  The equation appears in most standard meteorological texts.
 */

extern double PHYdenTV(double t, double td, double p);
   
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

extern double PHYdenTW(double t, double td, double p);
   
/********************************************************************
 * PHYdenWMR()
 *
 * This function approximates the mixing ratio wmr (grams of water vapor
 * per kg of dry air) given the pressure p (mb) and the temperature t
 * (Celsius). The formula used is given on p 102 of the Smithsonian
 * Meteorological Tables by Roland List (6th Edition).
 */

extern double PHYdenWMR(double p, double t);
   
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

extern double PHYdenWOBF(double t);
   
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

extern double PHYdenW(double p, double t);
   
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

extern double PHYdenZ(double pt, double *p, double *t, double *td, int n);
   

#endif
#ifdef __cplusplus
}
#endif

