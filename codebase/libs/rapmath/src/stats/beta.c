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
/***********************************************************
 * beta.c
 *
 * Functions related to the beta function.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307
 *
 * Feb 1998
 *
 ***********************************************************/

#include <rapmath/stats.h>

/*********************************************
 * STATS_beta_gen()
 *
 * Generate a beta-distributed variate,
 * with parameters a and b.
 *
 * Form of pdf is:
 *
 *   f(x) = (gamma(a + b) / (gamma(a) * gamma(b))) *
 *          (x ^ (a - 1)) * ((1 - x) ^ (b - 1))
 *
 * Assumes STATS_uniform_seed() has been called.
 *
 * Reference: Simulation and the Monte-Carlo method.
 *            Reuven Rubinstein. Wiley 1981. p 84.
 *            Alg developed by Johnk.
 * 
 * Returns variate.
 *
 */

double STATS_beta_gen(double a, double b)

{

  int count = 0;
  double u1, u2;
  double y1, y2;
  double x;

  while (count < 1000) {

    u1 = STATS_uniform_gen();
    u2 = STATS_uniform_gen();

    y1 = pow(u1, 1.0 / a);
    y2 = pow(u2, 1.0 / b);

    if (y1 + y2 >= 1.0) {
      x = y1 / (y1 + y2);
      return (x);
    }

    count++;

  }

  fprintf(stderr, "ERROR - STATS_beta_gen\n");
  fprintf(stderr, "Failure, returning 0.0\n");
  return(0.0);

}

