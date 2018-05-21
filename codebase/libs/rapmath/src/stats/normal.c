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
 * normal.c
 *
 * Functions related to the normal function.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307
 *
 * Feb 1998
 *
 ***********************************************************/

#include <rapmath/stats.h>
#include <rapmath/RMmalloc.h>

/*************************************************
 * STATS_normal_pdf()
 *
 * Compute the normal prob density function for given
 * mean, sdev and xx.
 *
 */

double STATS_normal_pdf(double mean, double sdev, double xx)

{

  double pdf;
  double aa;

  aa  = (xx - mean) / sdev;
  pdf = exp(-0.5 * aa * aa) / (sdev * sqrt(2.0 * M_PI));
  return (pdf);

}

/*********************************************
 * STATS_normal_fit()
 *
 * Fit a normally-distributed variate
 * with parameters mean and sdev.
 *
 * Loads mean and sdev
 * 
 * Returns 0 on success, -1 on failure.
 *
 */

int STATS_normal_fit(int nx, double *x,
		     double *mean_p, double *sdev_p)

{
  int i;
  double xx;
  double sumx = 0.0;
  double sumx2 = 0.0;
  double mean, sdev;
  double var;
  double en = (double) nx;
  
  if (nx < 5) {
    fprintf(stderr, "Too few points (%d) for exponential fit\n", nx);
    return (-1);
  }

  for (i = 0; i < nx; i++, x++) {
    xx = *x;
    sumx += xx;
    sumx2 += xx * xx;
  }

  mean = sumx / en;
  var = (sumx2 - (sumx * sumx) / en) / (en - 1.0);

  if (var >= 0.0) {
    sdev = sqrt(var);
  } else {
    sdev = 0.0;
  }

  *mean_p = mean;
  *sdev_p = sdev;

  return (0);

}

/*********************************************
 * STATS_normal_skewness()
 *
 * Compute the skewness of a normally-distributed
 * data set, given the mean and standard deviation.
 *
 * Returns the skewness.
 *
 */

double STATS_normal_skewness(int nx, double *x,
                             double mean, double sdev)

{
  
  int i;
  double xx;
  double sum = 0.0;
  for (i = 0; i < nx; i++, x++) {
    xx = *x - mean;
    sum += pow(xx, 3.0);
  }

  double skewness = (sum / nx) / pow(sdev, 3.0);

  return skewness;

}

/*********************************************
 * STATS_normal_kurtosis()
 *
 * Compute the kurtosis of a normally-distributed
 * data set, given the mean and standard deviation.
 *
 * Returns the kurtosis.
 *
 */

double STATS_normal_kurtosis(int nx, double *x,
                             double mean, double sdev)

{
  
  int i;
  double xx;
  double sum = 0.0;
  for (i = 0; i < nx; i++, x++) {
    xx = *x - mean;
    sum += pow(xx, 4.0);
  }

  double kurtosis = ((sum / nx) / pow(sdev, 4.0)) - 3.0;

  return kurtosis;

}

/************************************************************
 * STATS_normal_chisq()
 *
 * Compute chisq parameter for a normally-distributed variate
 * with parameters mean and sdev.
 *
 * n_intv in the number of intervals into which the data
 * range is divided to compute chisq.
 *
 * Loads chisq param.
 * 
 * Returns 0 on success, -1 on failure.
 *
 */

int STATS_normal_chisq(int nx, double *x,
		       double mean, double sdev,
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
    pdf = STATS_normal_pdf(mean, sdev, xx);
    pdf_count = pdf * en;
    chisq += (pow((bin_count[ibin] - pdf_count), 2.0)) / pdf_count;
  }
  
  *chisq_p = chisq;

  RMfree(bin_count);

  return (0);

}

/*********************************************
 * STATS_normal_gen()
 *
 * Generate an normally-distributed variate
 * with parameters mean and sdev.
 *
 * Assumes STATS_uniform_seed() has been called.
 * 
 * Reference: Simulation and the Monte-Carlo method.
 *            Reuven Rubinstein. Wiley 1981. p 87.
 * 
 * Returns normal variate.
 *
 */

double STATS_normal_gen(double mean, double sdev)

{

  double u1, u2;
  double z;

  u1 = STATS_uniform_gen();
  u2 = STATS_uniform_gen();

  z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);

  return (mean + z * sdev);

}
