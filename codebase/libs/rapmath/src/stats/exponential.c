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
 * exponential.c
 *
 * Functions related to the exponential function.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307
 *
 * Feb 1998
 *
 ***********************************************************/

#include <rapmath/stats.h>
#include <rapmath/RMmalloc.h>

/*************************************************
 * STATS_exponential_pdf()
 *
 * Compute the exponential prob density function for
 * given scale parameter b.
 *
 * Form of pdf is:
 *
 *   f(x) = (1.0 / b) * exp(-x /b)
 */

double STATS_exponential_pdf(double b, double xx)

{

  double pdf;
  pdf = (1.0 / b) * exp(-xx /b);
  return (pdf);

}

/*********************************************
 * STATS_exponential_fit()
 *
 * Fit an exponentially distributed variate,
 * with parameter a.
 *
 * Form of pdf is:
 *
 *   f(x) = (1.0 / b) * exp(-x /b)
 *
 * Loads up scale b.
 *
 * Returns 0 on success, -1 on failure.
 *
 */

int STATS_exponential_fit(int nx, double *x,
			  double *b_p)

{

  int i;
  double sumx = 0.0;
  double arith_mean;
  double en = (double) nx;

  if (nx < 5) {
    fprintf(stderr, "Too few points (%d) for exponential fit\n", nx);
    return (-1);
  }

  /*
   * compute arithmetic mean
   */
  
  for (i = 0; i < nx; i++, x++) {
    sumx += *x;
  }

  arith_mean = sumx / en;

  *b_p = arith_mean;

  return (0);
  
}


/*********************************************************
 * STATS_exponential_chisq()
 *
 * Compute chisq parameter for a exponentially-distributed
 * variate with parameter b.
 *
 * n_intv in the number of intervals into which the data
 * range is divided to compute chisq.
 *
 * Loads chisq param.
 * 
 * Returns 0 on success, -1 on failure.
 *
 */

int STATS_exponential_chisq(int nx, double *x, double b,
			    int n_intv, double *chisq_p)

{

  int i, ibin;
  double en = (double) nx;
  double xx;
  double minx = 1.0e99; 
  double maxx = -1.0e99;
  double range, bin_width;
  double *bin_count;
  double pdf, pdf_count;
  double chisq;

  /*
   * compute min and max
   */

  for (i = 0; i < nx; i++) {
    xx = x[i];
    minx = MIN(minx, xx);
    maxx = MAX(maxx, xx);
  }

  /*
   * compute range, adding a small fraction at each end
   * to avoid rounding into a non-existent bin
   */
  
  range = (maxx - minx);
  minx -= range / 1000.0;
  maxx += range / 1000.0;
  range = (maxx - minx);
  bin_width = range / (double) n_intv;

  /*
   * accumulate count per bin
   */
  
  bin_count = (double *) RMcalloc(n_intv, sizeof(double));
  
  for (i = 0; i < nx; i++) {
    xx = x[i];
    ibin = (int) ((xx - minx) / bin_width);
    bin_count[ibin]++;
  }
  
  /*
   * Compute the pdf count per bin - this is the 
   * theoretical count for a perfect distribution.
   * Then compute the chisq parameter.
   */

  chisq = 0.0;

  for (ibin = 0; ibin < n_intv; ibin++) {
    xx = minx + (ibin + 0.5) * bin_width;
    pdf = STATS_exponential_pdf(b, xx);
    pdf_count = pdf * en;
    chisq += (pow((bin_count[ibin] - pdf_count), 2.0)) / pdf_count;
  }
  
  *chisq_p = chisq;

  RMfree(bin_count);

  return (0);

}

/*********************************************
 * STATS_exponential_gen()
 *
 * Generate an exponentially-distributed variate,
 * with parameter b.
 *
 * Form of pdf is:
 *
 *   f(x) = (1.0 / b) * exp(-x /b)
 *
 * Assumes STATS_uniform_seed() has been called.
 * 
 * Reference: Simulation and the Monte-Carlo method.
 *            Reuven Rubinstein. Wiley 1981. p 68.
 * 
 * Returns exp variate.
 *
 */

double STATS_exponential_gen(double b)

{

  double u;

  u = STATS_uniform_gen();
  return (-b * log(u));

}
