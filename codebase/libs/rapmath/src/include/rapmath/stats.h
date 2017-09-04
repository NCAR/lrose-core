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
/************************************************
 * stats.h : header file for math stats routines
 ************************************************/

#ifndef rapmath_stats_h
#define rapmath_stats_h

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdio.h>
#include <toolsa/toolsa_macros.h>

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

extern double STATS_beta_gen(double a, double b);

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

extern double STATS_exponential_pdf(double b, double xx);

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

extern int STATS_exponential_fit(int nx, double *x,
				 double *b_p);

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

extern int STATS_exponential_chisq(int nx, double *x, double b,
				   int n_intv, double *chisq_p);

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
 * Returns variate.
 *
 */

extern double STATS_exponential_gen(double b);

/*********************************************
 * STATS_gamma_func()
 *
 * Compute the gamma function for given alpha.
 *
 * Returns value of Gamma function.
 *
 */

extern double STATS_gamma_func(double alpha);

/*********************************************
 * STATS_ln_gamma_func()
 *
 * Compute the natural log of the gamma function 
 * for given alpha.
 *
 * Returns value of ln(Gamma).
 *
 */

extern double STATS_ln_gamma_func(double alpha);

/*************************************************
 * STATS_incomplete_gamma_func()
 *
 * Compute the incomplete gamma function P(a,x)
 *
 * Uses the series expansion as tests showed this
 * to always be faster than the continued fraction
 * 
 * P(a,x) = g(a,x) / gamma(a) 
 *
 * g(a,x) = exp(-x) * x^a * 
 *          SUM(gamma(a)*x^n/gamma(a+1+n)), n=0,inf
 *
 * Returns the value of P(a,x)
 */
 
extern double STATS_incomplete_gamma_func( double a, double x );

/*************************************************
 * STATS_chisq_prob()
 *
 * Compute the probability that the observed 
 * chisquare value for a correct model will exceed
 * some chisq value. 
 *
 * This probability can be used as a confidence 
 * measure as it is 1 for chisq=0; 0 for chisq=inf.
 *
 * Return the probability
 */

extern double STATS_chisq_prob( double chisq, int deg_of_freedom );

/************************************************************
 * STATS_gamma_chisq()
 *
 * Compute chisq parameter for a gamm-distributed variate
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

extern int STATS_gamma_chisq(int nx, double *x,
			     double a, double b,
			     int n_intv, double *chisq_p);
  
/*************************************************
 * STATS_gamma_pdf()
 *
 * Compute the gamma density function for given
 * a, b and xx.
 *
 * Form of density function is:
 *
 *  f(x) = (1.0 / ((b ^ a) * gamma(a))) *
 *         (x ^ (a - 1)) * exp(-x / b)
 *
 * Returns value of Gamma density function.
 *
 */

extern double STATS_gamma_pdf(double a, double b, double xx);

/*************************************************
 * STATS_trigamma()
 *
 * Copmutes the second derivative of the gamma function
 * with parameter z.
 *
 * Returns 0 on success, -1 on failure
 */

extern int STATS_trigamma(double z, double *trigam_p);

/*************************************************
 * STATS_gamma_fit()
 *
 * Fit a gamma density function to data
 *
 * Form of density function is:
 *
 *  f(x) = (1.0 / ((b ^ a) * gamma(a))) *
 *         (x ^ (a - 1)) * exp(-x / b)
 *
 * Loads up a and b.
 *
 * Returns 0 on success, -1 on failure
 */

extern int STATS_gamma_fit(int nx, double *x,
			   double *a_p, double *b_p);

/*********************************************
 * STATS_gamma_gen()
 *
 * Generate a gamma-distributed variate,
 * with parameters a and b.
 *
 * Form of pdf is:
 *
 *  f(x) = (1.0 / ((b ^ a) * gamma(a))) *
 *         (x ^ (a - 1)) * exp(-x / b)
 *
 * Assumes STATS_uniform_seed() has been called.
 *
 * Reference: Simulation and the Monte-Carlo method.
 *            Reuven Rubinstein. Wiley 1981. pp 71-80.
 * 
 * Returns variate.
 *
 */

extern double STATS_gamma_gen(double a, double b);

/*********************************************
 * STATS_gamma3_gen()
 *
 * Generate a 3-parameter gamma-distributed variate,
 * with parameters a, b and c.
 *
 * Form of pdf is:
 *
 *  f(x) = ((1.0 / ((b ^ a) * gamma(a))) *
 *          (x ^ (a - 1)) * exp(-x / b)) + c
 *
 * Assumes STATS_uniform_seed() has been called.
 *
 * Returns variate.
 *
 */

extern double STATS_gamma3_gen(double a, double b, double c);

/*************************************************
 * STATS_normal_pdf()
 *
 * Compute the normal prob density function for given
 * mean, sdev and xx.
 *
 */

extern double STATS_normal_pdf(double mean, double sdev, double xx);

/*********************************************
 * STATS_normal_fit()
 *
 * Fit a normally-distributed variate
 * with parameters mean and sdev.
 *
 * Loads yo mean and sdev
 * 
 * Returns 0 on success, -1 on failure.
 *
 */

extern int STATS_normal_fit(int nx, double *x,
			    double *mean_p, double *sdev_p);

/*********************************************
 * STATS_normal_skewness()
 *
 * Compute the skewness of a normally-distributed
 * data set, given the mean and standard deviation.
 *
 * Returns the skewness.
 *
 */

extern double STATS_normal_skewness(int nx, double *x,
                                    double mean, double sdev);

/*********************************************
 * STATS_normal_kurtosis()
 *
 * Compute the kurtosis of a normally-distributed
 * data set, given the mean and standard deviation.
 *
 * Returns the kurtosis.
 *
 */

extern double STATS_normal_kurtosis(int nx, double *x,
                                    double mean, double sdev);

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

extern int STATS_normal_chisq(int nx, double *x,
			      double mean, double sdev,
			      int n_intv, double *chisq_p);

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

extern double STATS_normal_gen(double mean, double sdev);

/**********************************************
 * STATS_print_primes()
 *
 * Prints the first n primes to file out.
 *
 * Returns 0 on success, -1 on error.
 */

extern int STATS_print_primes(int n, FILE *out);

/**************************************************
 * STATS_uniform_gen()
 *
 * Generate a random number between 0 and 1.
 *
 * Optionally call uniform_seed() before using this function.
 */

extern double STATS_uniform_gen(void);

/**************************************************
 * STATS_uniform_seed()
 *
 * Seed for uniform random number generation.
 *
 * Given the seed, compute suitable starting values
 * for xx, yy and zz.
 *
 * Optional before using uniform_gen().
 */

extern void STATS_uniform_seed(int seed);

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

extern double STATS_weibull_pdf(double a, double b, double xx);

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
 * Assumes STATS_uniform_seed() has been called.
 * 
 * Returns 0 on success, -1 on failure.
 *
 */

extern int STATS_weibull_fit(int nx, double *x,
			     double *a_p, double *b_p);

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

extern int STATS_weibull_chisq(int nx, double *x,
			       double a, double b,
			       int n_intv, double *chisq_p);

/*********************************************
 * STATS_weibull_gen()
 *
 * Generate an weibull distributed variate,
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

extern double STATS_weibull_gen(double a, double b);

#ifdef __cplusplus
}
#endif

#endif
