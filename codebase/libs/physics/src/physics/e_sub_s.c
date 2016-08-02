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
 * PHYe_sub_s : This routine computes the saturation vapor pressure (Pa)
 *              over liquid with polynomial fit of goff-gratch (1946)
 *              formulation. (walko, 1991)
 *
 * double PHYe_sub_s(tempK)
 *      double tempK;      * temperature in degrees Kelvin
 *
 * Taken from FORTRAN code written by Greg Thompson
 *********************************************************************/

#include <stdio.h>

#include <physics/physics.h>


#define C0 610.5851
#define C1 44.40316
#define C2 1.430341
#define C3 0.2641412e-1
#define C4 0.2995057e-3
#define C5 0.2031998e-5
#define C6 0.6936113e-8
#define C7 0.2564861e-11
#define C8 -0.3704404e-13


double PHYe_sub_s(double tempK)
{
  double tempC = PHYtemp_k_to_c(tempK);
  double e_sub_s;
  
  tempC = MAX(-80.0, tempC);

  e_sub_s = C0 + (tempC * (C1 + (tempC * (C2 + (tempC * (C3 + (tempC *
	(C4 + (tempC * (C5 + (tempC * (C6 + (tempC *
        (C7 + (tempC * C8)))))))))))))));
  
  return(e_sub_s);
}
