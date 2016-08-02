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
 * gamma.c
 *
 * Functions related to the gamma function.
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307
 *
 * Feb 1998
 *
 ***********************************************************/

#include <rapmath/stats.h>
#include <rapmath/RMmalloc.h>

/*********************************************
 * STATS_ln_gamma_func()
 *
 * Compute the natural log of the gamma function for given a.
 *
 * Returns value of ln of Gamma function.
 *
 * This algorithm makes use of the Lanczos approximation, see
 * A Precision Approximation of the Gamma Function J. SIAM
 * Numer. Anal. Ser. B, Vol. 1 1964. pp. 86-96
 * and terms calculated by Viktor T. Toth
 * Reference: www.rskey.org/gamma.htm 
 */ 
double STATS_ln_gamma_func(double a)
{
  static double terms[7] = { 75122.6331530, 80916.6278952, 36308.2951477, 
			     8687.24529705, 1168.92649479, 83.8676043424, 
			     2.50662827511 };

  double an = 1.0;
  double as = a;  
  double a5 = a + 5.5;
  double c = (a + 0.5) * log(a5) - a5;

  double num, den;
  double result;
  int i;

  /* initialize the series with the zeroth term */
  num = terms[0];
  den = as;
  for ( i=1; i<=6; ++i ) {
    an *= a;
    num += terms[i]*an;
    den *= ++as;
  }

  result = log(num/den) + c;

  return result;

}
/*********************************************
 * STATS_gamma_func()
 *
 * Compute the gamma function for given a.
 *
 * Returns value of Gamma function.
 *
 */
double STATS_gamma_func(double a)
{
  return exp( STATS_ln_gamma_func( a ) );
}

/*************************************************
 * STATS_incomplete_gamma_func()
 *
 * Compute the incomplete gamma function P(a,x)
 *
 * Uses the series expansion when x < a+1 and
 * uses the continued fraction expansion otherwise.
 * 
 * P(a,x) = g(a,x) / gamma(a) 
 *
 * g(a,x) = exp(-x) * x^a * 
 *          SUM(gamma(a)*x^n/gamma(a+1+n)), n=0,inf
 *
 * Returns the value of P(a,x) or -1.0 if invalid input
 */

double STATS_incomplete_gamma_func( double a, double x )
{
  double epsilon = 1e-10;
  double factor, result;

  if ( a <= 0.0 || x < 0.0 ) 
    {
    fprintf(stderr, "ERROR - STATS_incomplete_gamma_func\n");
    fprintf(stderr, "Both arguments must be greater than zero.\n");
    return -1.0;
    }

  /* using the log form is more efficient than pow(x,a) */
  factor = -x + a*log(x) - STATS_ln_gamma_func(a);

  if ( x < a + 1.0 )
    {
      /* SERIES EXPANSION of P(x,a) */
      double sum, term, z;

      /* set up the zeroth term of the summation */
      z = a + 1;
      sum = term = 1.0/a;
      do 
	{
	  /* make use of gamma(z+1) = z * gamma(z) */
	  term *= x/z++;
	  sum += term;
	} 
      while ( term > epsilon );
      result = sum * exp(factor);
    }
  else
    {
      /* CONTINUED FRACTION EXPANSION of 1-P(x,a) */
      double cf_term, prev;
      int j,i = 0;
      
      result = 0.0;  
      do 
	{
	  prev = result;
	  cf_term = 0.0;
	  for ( j=i; j>0; --j )
	    {
	      cf_term = (j - a) / ( 1 + (j /(x + cf_term)));
	    }

	  result = factor - log(x+cf_term);
	  ++i;
	} while( fabs(result - prev) > epsilon );
  
      result = 1.0 - exp(result);    
    }
  
  return result;  
}

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

double STATS_chisq_prob( double chisq, int deg_of_freedom )
{
  double a = 0.5 * deg_of_freedom;
  double x = 0.5 * chisq;
  
  return (deg_of_freedom < 1) ? 0.0 : 1.0 - STATS_incomplete_gamma_func( a, x );
}

/*************************************************
 * STATS_gamma_pdf()
 *
 * Compute the gamma prob density function for given
 * a, b and xx.
 *
 * Form of density function is:
 *
 *  f(x) = (1.0 / ((b ^ a) * gamma(a))) *
 *         (x ^ (a - 1)) * exp(-x / b)
 *
 * Returns value of Gamma prob density function.
 */

double STATS_gamma_pdf(double a, double b, double xx)

{

  double gamma;
  double pdf;

  gamma = STATS_gamma_func(a);

  pdf = ((1.0 / (pow(b, a) * gamma)) *
	  pow(xx, a - 1.0) *
	  exp(-xx / b));

  return (pdf);

}

/*************************************************
 * STATS_gamma_fit()
 *
 * Fit a gamma density function to data.
 * Method is rational approximation as presented
 * in Johnson and Kotz;
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

int STATS_gamma_fit(int nx, double *x,
		    double *a_p, double *b_p)

{

  int i;
  double sumx = 0.0;
  double sumlnx = 0.0;
  double arith_mean;
  double geom_mean;
  double shape, scale;
  double shape1, scale1;
  double shape2, scale2;
  double test1, test2;
  double trigam1, trigam2;
  double y, yy;
  double delta, dd;
  double en = (double) nx;

  /*
   * compute arithmetic and geometric means
   */

  for (i = 0; i < nx; i++, x++) {
    sumx += *x;
    sumlnx += log(*x);
  }

  arith_mean = sumx / en;
  geom_mean = exp(sumlnx / en);

  y = log(arith_mean / geom_mean);

  if (y <= 0) {

    fprintf(stderr, "ERROR - STATS_gamma_fit\n");
    fprintf(stderr, "y < 0\n");
    return (-1);

  } else if (y <= 0.5772) {

    shape = (0.5000876 + 0.1648852 * y - 0.0544274 * y * y) / y;
    scale = shape / arith_mean;
    
  } else if (y <= 17.0) {
    
    yy = 8.898919 + 9.059950 * y + 0.9775373 * y * y;
    shape = yy / (y * (17.79728 + 11.968477 * y + y * y));
    scale = shape / arith_mean;
    
  } else { /* y > 17 */

    delta = exp(y) / (1.0 - exp(y));
    dd = sqrt(delta * delta / 4.0 + delta / 6.0) / 2.0;

    shape1 = (-delta / 4.0) + dd;
    shape2 = (-delta / 4.0) - dd;

    scale1 = shape1 / arith_mean;
    scale2 = shape2 / arith_mean;

    if (STATS_trigamma(shape1, &trigam1)) {
      return (-1);
    }
    test1 = (pow((en / scale1), 2.0) +
	     (en * trigam1) * (en * shape1) / (scale1 * scale1));

    if (test1 <= 0.0) {

      shape = shape1;
      scale = scale1;

    } else {

      if (STATS_trigamma(shape2, &trigam2)) {
	return (-1);
      }
      test2 = (pow((en / scale2), 2.0) +
	       (en * trigam2) * (en * shape2) / (scale2 * scale2));

      if (test2 <= 0.0) {
	shape = shape2;
	scale = scale2;
      } else {
	fprintf(stderr, "ERROR - STATS_gamma_fit\n");
	fprintf(stderr, "Neither estimate is a maximum\n");
	return (-1);
      } /* if (test2 <= 0.0) */

    } /* if (test1 <= 0.0) */      

  }

  *a_p = shape;
  *b_p = 1.0 / scale;

  return (0);

}

/************************************************************
 * STATS_gamma_chisq()
 *
 * Compute chisq parameter for a gamma-distributed variate
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

int STATS_gamma_chisq(int nx, double *x,
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
    pdf = STATS_gamma_pdf(a, b, xx);
    pdf_count = pdf * en;
    chisq += (pow((bin_count[ibin] - pdf_count), 2.0)) / pdf_count;
  }
  
  *chisq_p = chisq;

  RMfree(bin_count);

  return (0);

}

/*************************************************
 * STATS_trigamma()
 *
 * Copmutes the second derivative of the gamma function
 * with parameter z.
 *
 * Returns 0 on success, -1 on failure
 */

int STATS_trigamma(double z, double *trigam_p)

{

  double qrzeta[29] = {1.6449340668482,
		       1.2020569031595,
		       1.0823232337111,
		       1.0369277551433,
		       1.0173430619844,
		       1.0083492773819,
		       1.0040773561979,
		       1.0020083928260,
		       1.0009945751278,
		       1.0004941886041,
		       1.0002460865533,
		       1.0001227133475,
		       1.0000612481350,
		       1.0000305882363,
		       1.0000152822594,
		       1.0000076317976,
		       1.0000038172932,
		       1.0000019082127,
		       1.0000009539620,
		       1.0000004769329,
		       1.0000002384505,
		       1.0000001192199,
		       1.0000000596081,
		       1.0000000298035,
		       1.0000000149015,
		       1.0000000074507,
		       1.0000000037253,
		       1.0000000018626,
		       1.0000000009313};

  int k, l, n, n1;
  double x, u;
  double trigam;
  double s1;

  *trigam_p = 0.0;

  if (z < 1.0e-8) {
    fprintf(stderr, "ERROR - STATS_trigamma: z < 1.0e-8\n");
    return (-1);
  }

  if (z >= 10.0) {
    trigam = ((1.0 / z) +
	      (1.0 / (2.0 * z * z)) +
	      (1.0 / (6.0 * pow(z, 3.0))) +
	      (-1.0 / (30.0 * pow(z, 5.0))) +
	      (1.0 / (42.0 * pow(z, 7.0))) +
	      (-1.0 / (30.0 * pow(z, 9.0))));
    *trigam_p = trigam;
    return(0);
  }

  n = (int) z;
  x = z - n;
  u = x;

  if (u > 0.5) {
    u = 1.0 - x;
  }

  if (fabs(u) <= 1.0e-8) {

    n = (int) (z + 0.1);
    trigam = qrzeta[0];
    if (n == 1) {
      *trigam_p = trigam;
      return (0);
    }

    n1 = n - 1;
    for (k = 1; k <= n1; k++) {
      trigam -= 1.0 / (k * k);
      *trigam_p = trigam;
      return (0);
    }
    
  } /* if (fabs(u) <= 1.0e-8) */  

  trigam = 1.0 / (u * u);
  s1 = -1.0;
  for (k = 1; k <= 29; k++) {
    l = k - 1;
    s1 = -s1;
    trigam += k * qrzeta[k-1] * pow(u, l) * s1;
  }

  if (x > 0.5) {
    trigam = (M_PI * M_PI) / pow((sin(M_PI * x)), 2.0) - trigam;
  }

  if (n == 0) {
    *trigam_p = trigam;
    return (0);
  }

  for (k = 1; k <= n; k++) {
    trigam -= 1.0 / (x * x);
    x++;
  }
  *trigam_p = trigam;
  return (0);

}

static double integer_gamma_gen(double a, double b);
static double a_low_gamma_gen(double a, double b);
static double a_high_gamma_gen(double a, double b);
static double a_high_b_one_gamma_gen(double a);

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

double STATS_gamma_gen(double a, double b)

{

  if (fmod(a, 1.0) == 0.0) {

    /*
     * a is integer
     */

    return (integer_gamma_gen(a, b));

  } else if (a < 1.0) {
    
    return (a_low_gamma_gen(a, b));

  } else if (b == 1.0) {
    
    return (a_high_b_one_gamma_gen(a));

  } else {

    return (a_high_gamma_gen(a, b));

  }

}

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

double STATS_gamma3_gen(double a, double b, double c)

{

  return STATS_gamma_gen(a, b) + c;

}

/*********************
 * integer_gamma_gen()
 *
 * For cases in which a is an integer.
 *
 * Rubinstein alg G1, p 71.
 */

static double integer_gamma_gen(double a, double b)

{

  double xx = 0.0;
  double v;

  while (a >= 1.0) {
    v = STATS_exponential_gen(1.0);
    xx += v;
    a--;
  }

  return (b * xx);

}

/*********************
 * a_low_gamma_gen()
 *
 * For cases in which a < 1
 *
 * Rubinstein alg G2, p 73.
 */

static double a_low_gamma_gen(double a, double b)

{

  double w;
  double v;

  w = STATS_beta_gen(a, 1.0 - a);
  v = STATS_exponential_gen(1.0);

  return (b * w * v);

}

/*********************
 * a_high_gamma_gen()
 *
 * For cases in which a > 1
 *
 * Rubinstein alg G6, p 77.
 */

static double a_high_gamma_gen(double a, double b)

{

  int count = 0;
  double lambda = a - 1.0;
  double u, v, w, x, y;

  while (count < 1000) {

    u = STATS_uniform_gen();

    y = lambda + b * tan(M_PI * (u - 0.5));

    v = STATS_exponential_gen(1.0);
    
    x = y - lambda;

    w = -1.0 * (log(1.0 + (x * x) / (b * b)) +
		(lambda * log(y / lambda)) - y + lambda);

    if (v >= w) {
      return (y);
    }

    count++;

  }

  fprintf(stderr, "ERROR - STATS_gamma_gen\n");
  fprintf(stderr, "Failure, returning 0.0\n");
  return(0.0);

}

/**************************
 * a_high_b_one_gamma_gen()
 *
 * For cases in which a > 1, b == 1.
 *
 * Rubinstein alg G4, p 75.
 */

static double a_high_b_one_gamma_gen(double a)

{

  int count = 0;
  double lambda = a - 1.0;
  double v1, v2;
  double w;

  while (count < 1000) {

    v1 = STATS_exponential_gen(1.0);
    v2 = STATS_exponential_gen(1.0);

    w = lambda * (v1 - log(v1) - 1.0);

    if (v2 >= w) {
      return (v1);
    }
    
    count++;

  }

  fprintf(stderr, "ERROR - STATS_gamma_gen\n");
  fprintf(stderr, "Failure, returning 0.0\n");
  return(0.0);

}

