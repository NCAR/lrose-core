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
 * weibull.c
 *
 * Functions related to the weibull function.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307
 *
 * Feb 1998
 *
 ***********************************************************/

#include <rapmath/stats.h>
#include <rapmath/RMmalloc.h>

/*************************************************
 * STATS_weibull_pdf()
 *
 * Compute the weibull prob density function for given
 * a, b and xx.
 *
 * Form of density function is:
 *
 *  f(x) = (a / (b^a)) * x^(a-1) * exp(-(x/b)^a)
 *
 * Returns value of Weibull prob density function.
 */

double STATS_weibull_pdf(double a, double b, double xx)

{

  double pdf;

  double term1 = (a / pow(b, a));
  double term2 = pow(xx, a - 1);
  double term3 = exp(-pow(xx / b, a));

  pdf = term1 * term2 * term3;

  return (pdf);

}

/*********************************************
 * STATS_weibull_fit()
 *
 * Fit a weibull distributed variate,
 * with parameters a and b.
 *
 * Form of pdf is:
 *
 *   f(x) = (a / (b^a)) * x^(a-1) * exp(-(x/b)^a)
 *
 * Load up a and b.
 *
 * Returns 0 on success, -1 on failure.
 *
 */

int STATS_weibull_fit(int nx, double *x,
		      double *a_p, double *b_p)

{

  int i;
  int count;

  double sumlnx = 0.0;
  double en = (double) nx;
  double shape, prev_shape;
  double scale;
  double diff;
  double *xp;

  xp = x;
  for (i = 0; i < nx; i++, xp++) {
    sumlnx += log(*xp);
  }

  /*
   * iteratively solve for shape
   */

  prev_shape = 1.0;
  count = 0;
  
  while (count < 1000) {

    double sum1 = 0.0;
    double sum2 = 0.0;

    xp = x;
    for (i = 0; i < nx; i++, xp++) {
      sum1 += pow(*xp, prev_shape) * log(*xp);
      sum2 += pow(*xp, prev_shape);
    }

    shape = 1.0 / (sum1 / sum2 - sumlnx / en);
    diff = shape - prev_shape;

    if (diff < 0.0001) {
      scale = pow((sum2 / en), 1.0 / shape);
      *a_p = shape;
      *b_p = scale;
      return (0);
    }

    prev_shape = shape;
    count++;

  } /* while */

  fprintf(stderr, "ERROR - STATS_weibull_fit\n");
  fprintf(stderr, "No convergence on iterative fit\n");
    
  return (-1);

}

/************************************************************
 * STATS_weibull_chisq()
 *
 * Compute chisq parameter for a weibull-distributed variate
 * with parameters a and b.
 *
 * n_intv in the number of intervals into which the data
 * range is divided to compute chisq.
 *
 * Loads chisq param.
 * 
 * Returns 0 on success, -1 on failure.
 *
 */

int STATS_weibull_chisq(int nx, double *x,
		       double a, double b,
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
    pdf = STATS_weibull_pdf(a, b, xx);
    pdf_count = pdf * en;
    chisq += (pow((bin_count[ibin] - pdf_count), 2.0)) / pdf_count;
  }
  
  *chisq_p = chisq;

  RMfree(bin_count);

  return (0);

}

/*********************************************
 * STATS_weibull_gen()
 *
 * Generate a weibull distributed variate,
 * with parameters a and b.
 *
 * Form of pdf is:
 *
 *   f(x) = (a / (b^a)) * x^(a-1) * exp(-(x/b)^a)
 *
 * Assumes STATS_uniform_seed() has been called.
 * 
 * Reference: Simulation and the Monte-Carlo method.
 *            Reuven Rubinstein. Wiley 1981. p 93.
 * 
 * Returns Weibull variate.
 *
 */

double STATS_weibull_gen(double a, double b)

{

  double v, w;

  v = STATS_exponential_gen(1.0);
  w = b * pow(v, 1.0/a);

  return (w);

}

