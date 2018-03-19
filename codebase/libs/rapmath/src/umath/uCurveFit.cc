// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/***************************************************************************
 * uCurveFit.cc
 *
 * Curve fitting module
 *
 * Public routines:
 *
 *   uCurveFitDebug()
 *   uExponentialFit()
 *   uLinearFit()
 *   uPolynomialFit()
 *   uQuadFit()
 *   uQuadWtFit()
 *
 ***************************************************************************/

#include <iostream>
#include <rapmath/umath.h>
#include <rapmath/RMmalloc.h>
using namespace std;

typedef void (*evalFunc)(double, double *, double *, double *, long);

static int _approxExp(long n, double *x, double *y,
                      double *a, double *ymean_p);

static void _rearrangeCov(double **covar, long nparams,
                          long *qqList, long mOrder);

static void _evalExponential(double x, double *a, double *y,
			     double *dyda, long na);

static void _evalPolynomial(double x, double *a, double *y_p,
			    double *dyda, long na);

static int _gjElim(double **a, long n, double **b, long m);

static void _lmCoeff(double *x, double *y, double *sig,
                     long nn, double *a, long nparams,
                     long *qqList, long mOrder, double **aCoeff,
                     double *bCoeff, double *chisq, evalFunc);

static int _levMarq(double *x, double *y, double *sig,
                    long nn, double *a, long nparams,
                    long *qqList, long mOrder, double **covar,
                    double **aCoeff, double *chisq, evalFunc,
                    double *ldiff,
                    double *da, double *atry, double **oneda,
                    double *bCoeff, double &ochisq);

static int Debug = FALSE;

#define SMALL_CHANGE 0.0001
#define MAX_ITERATIONS 1000
#define MAX_POINTS 3000

/*********************************************************************
 * uCurveFitDebug()
 *
 * Sets debugging status
 */

void uCurveFitDebug(int dflag)
{

  Debug = dflag;
  
}

/*********************************************************************
 * uExponentialFit()
 *
 * exponential fit to array of data
 *
 *  n: number of points in (x, y) data set
 *  x: array of x data
 *  y: array of y data
 *  a[] - exponential coefficients in equation :
 *               y = a[0] + a[1].exp(a[2].x)
 *  std_error - standard error of estimate
 *  r_squared - correlation coefficient squared
 *
 * Returns 0 on success, -1 on error.
 *
 */

int uExponentialFit(long n, double *x, double *y, double *a,
		    double *std_error_est, double *r_squared)

{

  long i;

  long qqList[3];
  long nparams = 3;
  long mOrder = 3;
  long n_iterations;

  double chisq = 0, prev_chisq = 0;
  double ldiff, prev_ldiff, delta_ldiff;
  double chisq_change;
  double *xp, *yp, xval, yval;
  double ymean;
  double diff;
  double sum_dy_squared;
  double sum_of_residuals;

  /*
   * check number of points
   */
  
  if (n < 4) {
    fprintf(stderr, "WARNING - uExponentialFit: too few data points\n");
    return -1;
  }
  
  if (n > MAX_POINTS) {
    fprintf(stderr,
	    "WARNING - uExponentialFit: too many data points (max %d)\n",
	    MAX_POINTS);
    fprintf(stderr, "Using first %d only\n", MAX_POINTS);
    n = MAX_POINTS;
  }
    
  /*
   * get initial guess at params
   */

  if (_approxExp(n, x, y, a, &ymean)) {
    fprintf(stderr,
	    "WARNING - uExponentialFit : could not find approx params\n");
    return -1;
  }

  /*
   * set qqList to indicate that all 3 params must be set
   */

  qqList[0] = 0;
  qqList[1] = 1;
  qqList[2] = 2;

  // alloc arrays

  double **covar = (double **) RMmalloc2 (n, n, sizeof(double));
  double **aCoeff = (double **) RMmalloc2 (n, n, sizeof(double));
  double **oneda = (double **) RMmalloc2 (mOrder, 1, sizeof(double));
  double *atry = (double *) RMmalloc(nparams * sizeof(double));
  double *da = (double *) RMmalloc(nparams * sizeof(double));
  double *bCoeff = (double *) RMmalloc(nparams * sizeof(double));
  double ochisq = chisq;
  double *sdev = (double *) RMmalloc (n * sizeof(double));

  /*
   * set up array of standard deviations for the data - since they are
   * unknown, set all values to 1.0
   */

  for (i = 0; i < n; i++) {
    sdev[i] = 1.0;
  }

  /*
   * iterate while testing for the change in chisq to drop
   * below a test value
   */

  prev_ldiff = 0.0;
  delta_ldiff = 1.0;
  ldiff = -1.0;
  prev_chisq = LARGE_DOUBLE;
  chisq_change = LARGE_DOUBLE;
  n_iterations = 0;

  int iret = 0;

  while (chisq_change > SMALL_CHANGE ||
	 delta_ldiff < 0.0 ||
	 ldiff > 0.0000001) {

    if (_levMarq(x, y, sdev, n, a, nparams, qqList, mOrder,
                 covar, aCoeff, &chisq, _evalExponential, &ldiff,
                 da, atry, oneda, bCoeff, ochisq)) {
      fprintf(stderr, "WARNING - uFitExponential\n");
      iret = -1;
      break;
    }

    chisq_change = fabs((prev_chisq - chisq) / chisq);
    prev_chisq = chisq;

    delta_ldiff = prev_ldiff - ldiff;
    prev_ldiff = ldiff;

    n_iterations++;

    if (n_iterations > MAX_ITERATIONS) {
      fprintf(stderr,
	      "WARNING - uExponentialFit - did not converge\n");
      iret = -1;
      break;
    }

  } /* while */
  
  if (iret == 0) {
    /*
     * converged - set ldiff to 0.0 for final call
     */
    ldiff = 0.0;
    if (_levMarq(x, y, sdev, n, a, nparams, qqList, mOrder,
                 covar, aCoeff, &chisq, _evalExponential, &ldiff,
                 da, atry, oneda, bCoeff, ochisq)) {
      fprintf(stderr, "WARNING - uFitExponential\n");
      iret = -1;
    }
  }
  
  if (iret == 0) {

    /*
     * compute the sum of the residuals
     */
    
    sum_dy_squared = 0.0;
    sum_of_residuals = 0.0;
    
    xp = x;
    yp = y;
    for (i = 0; i < n; i++) {
      xval = *xp++;
      yval = *yp++;
      diff = (yval - a[0] - a[1] * exp(xval * a[2]));
      sum_of_residuals += diff * diff;
      sum_dy_squared += (yval - ymean) * (yval - ymean);
    }
    
    /*
     * compute standard error of estimate and r-squared
     */
    
    *std_error_est = sqrt(sum_of_residuals / ((double) n - 3.0));
    *r_squared = ((sum_dy_squared - sum_of_residuals) /
                  sum_dy_squared);

  }
    
  /*
   * free up memory
   */

  // free up

  RMfree(bCoeff);
  RMfree(da);
  RMfree(atry);
  RMfree2((void **) oneda);

  RMfree((void *) sdev);
  RMfree2((void **) covar);
  RMfree2((void **) aCoeff);

  return iret;

}

/***************************************************************************
 * uLinearFit.c : fit a line to a data series
 *
 * Mike Dixon  RAP, NCAR, Boulder, Colorado
 *
 * October 1990
 *
 *  n: number of points in (x, y) data set
 *  x: array of x data
 *  y: array of y data
 *  xmean, ymean: means
 *  xsdev, ysdev: standard deviations
 *  corr: correlation coefficient
 *  a[] - linear coefficients (a[0] - bias, a[1] - scale)
 *
 * Returns 0 on success, -1 on error.
 *
 ***************************************************************************/
 
int uLinearFit(long n, double *x, double *y, double *a,
	       double *xmean_p, double *ymean_p, double *xsdev_p,
	       double *ysdev_p, double *corr_p, double *std_error_est_p,
	       double *r_squared_p)

{

  long i;

  double sumx = 0.0, sumx2 = 0.0;
  double sumy = 0.0, sumy2 = 0.0, sumxy = 0.0;
  double dn;
  double term1, term2, term3, term4, term5;
  double xmean, ymean;
  double xsdev, ysdev;
  double corr;
  double xval, yval;
  double *x_p, *y_p;
  double std_error_est, r_squared;
  double diff;
  double sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;

  if (n < 2)
    return -1;

  dn = (double) n;

  /*
   * sum the various terms
   */

  x_p = x;
  y_p = y;
  
  for (i = 0; i < n; i++) {
    xval = *x_p++;
    yval = *y_p++;
    sumx += xval;
    sumx2 += xval * xval;
    sumy += yval;
    sumy2 += yval * yval;
    sumxy += xval * yval;
  }
  
  /*
   * compute the terms
   */

  term1 = dn * sumx2  - sumx * sumx;
  term2 = sumy * sumx2 - sumx * sumxy;
  term3 = dn * sumxy - sumx * sumy;
  term4 = (dn * sumx2 - sumx * sumx);
  term5 = (dn * sumy2 - sumy * sumy);
  
  /*
   * compute mean and standard deviation
   */
  
  xmean = sumx / dn;
  ymean = sumy / dn;

  xsdev = sqrt(fabs(term4)) / (dn - 1.0);
  ysdev = sqrt(fabs(term5)) / (dn - 1.0);

  /*
   * compute correlation coefficient
   */

  corr = term3 / sqrt(fabs(term4 * term5));

  /*
   * compute regression parameters
   */

  a[0] = term2 / term1;
  a[1] = term3 / term1;

  /*
   * compute the sum of the residuals
   */

  x_p = x;
  y_p = y;
  for (i = 0; i < n; i++) {
    xval = *x_p++;
    yval = *y_p++;
    diff = (yval - a[0] - a[1] * xval);
    sum_of_residuals += diff * diff;
    sum_dy_squared += (yval - ymean) * (yval - ymean);
  }

  /*
   * compute standard error of estimate and r-squared
   */
  
  std_error_est = sqrt(sum_of_residuals / (dn - 3.0));
  r_squared = ((sum_dy_squared - sum_of_residuals) /
	       sum_dy_squared);

  /*
   * load up return values
   */

  *xmean_p = xmean;
  *ymean_p = ymean;

  *xsdev_p = xsdev;
  *ysdev_p = ysdev;

  *corr_p = corr;

  *std_error_est_p = std_error_est;
  *r_squared_p = r_squared;

  return 0;
  
}

/***************************************************************************
 * compute linear fit of line through x/y data, using principal components
 *
 * Compute a (slope) and b (intercept) so that: y = ax + b
 *
 * Returns 0 on success, -1 on error.
 *
 ***************************************************************************/
 
int uLinearFitPC(int n, double *x, double *y,
                 double *a, double *b)
  
{

  int ii;
  double means[3];
  double eigenvalues[3];
  double **eigenvectors = (double **) RMmalloc2(3, 3, sizeof(double));
  double **coords = (double **) RMmalloc2(n, 2, sizeof(double));
  double sumx = 0.0;
  double sumy = 0.0;
  double xmean, ymean;
  double slope, intercept;
  
  /* load up coords */
  
  for (ii = 0; ii < n; ii++) {
    double xx = x[ii];
    double yy = y[ii];
    coords[ii][0] = xx;
    coords[ii][1] = yy;
    sumx += xx;
    sumy += yy;
  }

  /* compute means */

  xmean = sumx / (double) n;
  ymean = sumy / (double) n;

  /*
   * obtain the principal component transformation for the coord data
   * The technique is applicable here because the first principal
   * component will lie along the axis of maximum variance, which
   * is equivalent to fitting a line through the data points,
   * minimizing the sum of the sguared perpendicular distances
   * from the data to the line.
   */
  
  if (upct(2, n, coords,
           means, eigenvectors, eigenvalues) == 0) {
    double uu = eigenvectors[0][0];
    double vv = eigenvectors[1][0];
    slope = vv / uu; /* slope */
  } else {
    RMfree2((void **) coords);
    RMfree2((void **) eigenvectors);
    return -1;
  }

  RMfree2((void **) coords);
  RMfree2((void **) eigenvectors);

  /* compute intercept */

  intercept = ymean - xmean * slope;

  *a = slope;
  *b = intercept;
  
  return 0;

}

/*********************************************************************
 * uPolyFit()
 *
 * polynomial fit to array of data
 *
 *  order: order of polynomial (linear = 1, quadratic = 2 etc. )
 *  a[] - polynomial coefficients
 *     y = a[0] + a[1] * x + a[2] * x**2 + ...
 *  n: number of points in (x, y) data set
 *  x: array of x data
 *  y: array of y data
 *  std_error - standard error of estimate
 *  r_squared - correlation coefficient squared
 *
 * Returns 0 on success, -1 on error.
 *
 */

int uPolyFit(long n, double *x, double *y, double *a,
	     long order,
	     double *std_error_est, double *r_squared)
     
{

  long i, j;

  long *qqList;
  long nparams;
  long mOrder;
  long n_iterations;

  double chisq = 0, prev_chisq = 0;
  double ldiff, prev_ldiff, delta_ldiff;
  double chisq_change;
  double *sdev;
  double **covar, **aCoeff;
  double xfac, y_est;
  double xval, yval;
  double *xp, *yp;
  double sumy, xmean, ymean;
  double corr, xsdev, ysdev;
  double diff;
  double sum_dy_squared;
  double sum_of_residuals;

  if (order < 1) {
    fprintf(stderr,
	    "WARNING - uPolyFit: polynomial order must be 1 or greater\n");
    return -1;
  }
  
  if (n < order + 1) {
    fprintf(stderr, "WARNING - uPolyFit: too few data points\n");
    return -1;
  }
  
  if (order == 1) {
    
    /*
     * linear fit
     */
    
    if (uLinearFit(n, x, y, a,
		   &xmean, &ymean,
		   &xsdev, &ysdev,
		   &corr,
		   std_error_est, r_squared)) {
      return -1;
    } else {
      return 0;
    }
    
  } 

  if (order == 2) {
    
    /*
     * quadratic fit
     */
    
    if (uQuadFit(n, x, y, a,
		 std_error_est, r_squared)) {
      return -1;
    } else {
      return 0;
    }
    
  }
  
  // higher order fit
    
  if (n > MAX_POINTS) {
    fprintf(stderr,
            "WARNING - uPolyFit: too many data points (max %d)\n",
            MAX_POINTS);
    fprintf(stderr, "Using first %d only\n", MAX_POINTS);
    n = MAX_POINTS;
  }
  
  nparams = order + 1;
  mOrder = nparams;
  
  /*
   * set up array of standard deviations for the data - since they are
   * unknown, set all values to 1.0
   */
  
  sdev = (double *) RMmalloc (n * sizeof(double));
  
  for (i = 0; i < n; i++)
    sdev[i] = 1.0;
  
  /*
   * malloc space for arrays
   */
  
  covar = (double **) RMmalloc2 (n, n, sizeof(double));
  aCoeff = (double **) RMmalloc2 (n, n, sizeof(double));
  qqList = (long *) RMmalloc ((order + 1) * sizeof(long));
  
  double **oneda = (double **) RMmalloc2 (mOrder, 1, sizeof(double));
  double *atry = (double *) RMmalloc(nparams * sizeof(double));
  double *da = (double *) RMmalloc(nparams * sizeof(double));
  double *bCoeff = (double *) RMmalloc(nparams * sizeof(double));
  double ochisq = chisq;
  
  /*
   * initialize
   */
  
  for (i = 0; i <= order; i++) {
    qqList[i] = i;
    a[i] = 1.0;
  }
  
  /*
   * iterate while testing for the change in chisq to drop
   * below a test value
   */
  
  prev_ldiff = 0.0;
  delta_ldiff = 1.0;
  ldiff = -1.0;
  prev_chisq = LARGE_DOUBLE;
  chisq_change = LARGE_DOUBLE;
  n_iterations = 0;
  
  int iret = 0;
  
  while ((chisq_change > SMALL_CHANGE) ||
         (delta_ldiff < 0.0) || (ldiff > 0.001)) {
    
    if (_levMarq(x, y, sdev, n, a, nparams, qqList, mOrder,
                 covar, aCoeff, &chisq, _evalPolynomial, &ldiff,
                 da, atry, oneda, bCoeff, ochisq)) {
      fprintf(stderr, "WARNING - uPolyFit\n");
      iret = -1;
      break;
    }
    
    chisq_change = fabs((prev_chisq - chisq) / chisq);
    prev_chisq = chisq;
    
    delta_ldiff = prev_ldiff - ldiff;
    prev_ldiff = ldiff;
    
    n_iterations++;
    
    if (n_iterations > MAX_ITERATIONS) {
      fprintf(stderr, "WARNING - uPolyFit - did not converge\n");
      iret = -1;
      break;
    }
    
  } /* while */
  
  if (iret == 0) {
    /*
     * converged - set ldiff to 0.0 for final call
     */
    ldiff = 0.0;
    if (_levMarq(x, y, sdev, n, a, nparams, qqList, mOrder,
                 covar, aCoeff, &chisq, _evalPolynomial, &ldiff,
                 da, atry, oneda, bCoeff, ochisq)) {
      fprintf(stderr, "WARNING - uFitExponential\n");
      iret = -1;
    }
  }

  if (iret == 0) {
    
    /*
     * compute the sum of the residuals
     */
    
    sumy = 0.0;
    yp = y;
    
    for (i = 0; i < n; i++) {
      sumy += *yp++;
    }
    
    ymean = sumy / (double) n;
    sum_dy_squared = 0.0;
    sum_of_residuals = 0.0;
    xp = x;
    yp = y;
    
    for (i = 0; i < n; i++) {
      xval = *xp++;
      yval = *yp++;
      y_est = a[0];
      xfac = 1.0;
      for (j = 1; j < nparams; j++) {
        xfac *= xval;
        y_est += a[j] * xfac;
      }
      diff = yval - y_est;
      sum_of_residuals += diff * diff;
      sum_dy_squared += (yval - ymean) * (yval - ymean);
    }
    
    /*
     * compute standard error of estimate and r-squared
     */
    
    *std_error_est = sqrt(sum_of_residuals / ((double) n - 3.0));
    *r_squared = ((sum_dy_squared - sum_of_residuals) /
                  sum_dy_squared);
    
  } // if (iret == 0)
  
  /*
   * free up memory
   */
  
  RMfree((void *) sdev);
  RMfree((void *) qqList);
  RMfree2((void **) covar);
  RMfree2((void **) aCoeff);
  
  RMfree(bCoeff);
  RMfree(da);
  RMfree(atry);
  RMfree2((void **) oneda);
  
  return 0;
  
}

/***************************************************************************
 * uQuadFit.c : fit a quadratic to a data series
 *
 * Mike Dixon  RAP, NCAR, Boulder, Colorado
 *
 * October 1990
 *
 *  n: number of points in (x, y) data set
 *  x: array of x data
 *  y: array of y data
 *  a[] - quadratic coefficients (a[0] - bias, a[1] - linear, a[2] - squared)
 *  std_error - standard error of estimate
 *  r_squared - correlation coefficient squared
 *
 * Returns 0 on success, -1 on error.
 *
 ***************************************************************************/

int uQuadFit(long n, double *x, double *y, double *a,
	     double *std_error_est_p, double *r_squared_p)

{

  long i;

  double sumx = 0.0, sumx2 = 0.0, sumx3 = 0.0, sumx4 = 0.0;
  double sumy = 0.0, sumxy = 0.0, sumx2y = 0.0;
  double dn;
  double term1, term2, term3, term4, term5;
  double std_error_est, r_squared;
  double diff;
  double ymean, sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;
  double xval, yval;
  double *x_p, *y_p;

  if (n < 4)
    return -1;

  dn = (double) n;

  /*
   * sum the various terms
   */

  x_p = x;
  y_p = y;
  
  for (i = 0; i < n; i++) {

    xval = *x_p++;
    yval = *y_p++;

    sumx = sumx + xval;
    sumx2 += xval * xval;
    sumx3 += xval * xval * xval;
    sumx4 += xval * xval * xval * xval;
    sumy += yval;
    sumxy += xval  * yval;
    sumx2y += xval * xval * yval;

  }

  ymean = sumy / dn;

  /*
   * compute the coefficients
   */

  term1 = sumx2 * sumy / dn - sumx2y;
  term2 = sumx * sumx / dn - sumx2;
  term3 = sumx2 * sumx / dn - sumx3;
  term4 = sumx * sumy / dn - sumxy;
  term5 = sumx2 * sumx2 / dn - sumx4;

  a[2] = (term1 * term2 / term3 - term4) / (term5 * term2 / term3 - term3);
  a[1] = (term4 - term3 * a[2]) / term2;
  a[0] = (sumy - sumx * a[1]  - sumx2 * a[2]) / dn;

  /*
   * compute the sum of the residuals
   */

  x_p = x;
  y_p = y;
  for (i = 0; i < n; i++) {
    xval = *x_p++;
    yval = *y_p++;
    diff = (yval - a[0] - a[1] * xval - a[2] * xval * xval);
    sum_of_residuals += diff * diff;
    sum_dy_squared += (yval - ymean) * (yval - ymean);
  }

  /*
   * compute standard error of estimate and r-squared
   */
  
  std_error_est = sqrt(sum_of_residuals / (dn - 3.0));
  r_squared = ((sum_dy_squared - sum_of_residuals) /
	       sum_dy_squared);

  /*
   * set return values
   */

  *std_error_est_p = std_error_est;
  *r_squared_p = r_squared;

  return 0;

}

/****************************************************************************
 * uQuadWtFit.c : fit a quadratic to a data series, using weigths
 *
 *  n: number of points in (x, y) data set
 *  x: array of x data
 *  y: array of y data
 *  a[] - quadratic coefficients (a[0] - bias, a[1] - linear, a[2] - squared)
 *  std_error - standard error of estimate
 *  r_squared - correlation coefficient squared
 *
 ****************************************************************************/

int uQuadWtFit(long n, double *x, double *y, double *weight,
	       double *a, double *std_error_est, double *r_squared)

{

  double sumx = 0.0, sumx2 = 0.0, sumx3 = 0.0, sumx4 = 0.0;
  double sumy = 0.0, sumxy = 0.0, sumx2y = 0.0;
  double num;
  double part1, part2, part3, part4, part5;
  double diff;
  double ymean, sum_dy_squared = 0.0;
  double sum_of_residuals = 0.0;
  int i;

  if (n < 4)
    return -1;

  num = (double) n;

  /*
   * sum the various terms
   */

  for (i = 0; i < n; i++) {

    sumx += x[i] * weight[i];
    sumx2 += x[i] * x[i] * weight[i];
    sumx3 += x[i] * x[i] * x[i] * weight[i];
    sumx4 += x[i] * x[i] * x[i] * x[i] * weight[i];
    sumy += y[i] * weight[i];
    sumxy += x[i]  * y[i] * weight[i];
    sumx2y += x[i] * x[i] * y[i] * weight[i];

  }

  ymean = sumy / num;

  /*
   * compute the coefficients
   */

  part1 = sumx2 * sumy / num - sumx2y;
  part2 = sumx * sumx / num - sumx2;
  part3 = sumx2 * sumx / num - sumx3;
  part4 = sumx * sumy / num - sumxy;
  part5 = sumx2 * sumx2 / num - sumx4;

  a[2] = (part1 * part2 / part3 - part4) / (part5 * part2 / part3 - part3);
  a[1] = (part4 - part3 * a[2]) / part2;
  a[0] = (sumy - sumx * a[1]  - sumx2 * a[2]) / num;

  /*
   * compute the sum of the residuals
   */

  for (i = 0; i < n; i++) {
    diff = (y[i] - a[0] - a[1] * x[i] - a[2] * x[i] * x[i]);
    sum_of_residuals += diff * diff;
    sum_dy_squared += (y[i] - ymean) * (y[i] - ymean);
  }

  /*
   * compute standard error of estimate and r-squared
   */

  *std_error_est = sqrt(sum_of_residuals / (double) (n - 3));
  *r_squared = ((sum_dy_squared - sum_of_residuals) /
		sum_dy_squared);

  return 0;

}

/**********************************************************************
 * _rearrangeCov()
 *
 * Rearrange covariance matrix
 *
 */

static void _rearrangeCov(double **covar, long nparams,
                          long *qqList, long mOrder)

{

  for (int j = 0; j < nparams - 1; j++) {
    for (int i = j + 1 ; i < nparams; i++) {
      covar[i][j] = 0.0;
    }
  }

  for (int i = 0; i < mOrder - 1; i++) {
    for (int j = i + 1; j < mOrder; j++) {
      if (qqList[j] > qqList[i]) {
	covar[qqList[j]][qqList[i]] = covar[i][j];
      } else {
	covar[qqList[i]][qqList[j]] = covar[i][j];
      }
    }
  }

  double swap = covar[0][0];

  for (int j = 0; j < nparams; j++) {
    covar[0][j] = covar[j][j];
    covar[j][j] = 0.0;
  }

  covar[qqList[0]][qqList[0]] = swap;

  for (int j = 1; j < mOrder; j++) {
    covar[qqList[j]][qqList[j]] = covar[0][j];
  }

  for (int j = 1; j < nparams; j++) {
    for (int i = 0; i <= j - 1; i++) {
      covar[i][j] = covar[j][i];
    }
  }

}

/**********************************************************************
 * _evalExponential()
 *
 * evaluates the exponential function and the derivatives of it's
 * parameters
 */

static void _evalExponential(double x, double *a, double *y,
			     double *dyda, long na)

{


  double exp_term;

  exp_term = exp(a[na - 1] * x);

  *y = a[0] + a[1] * exp_term;

  dyda[0] = 1.0;

  dyda[1] = exp_term;

  dyda[2] = a[1] * x * exp_term;

}

/**********************************************************************
 * _evalPolynomial()
 *
 * evaluates the polynomial function and the derivatives of it's
 * parameters
 */

static void _evalPolynomial(double x, double *a, double *y_p,
			    double *dyda, long na)

{

  long i;
  double xfac;
  double y; 

  xfac = 1.0;
  y = a[0];
  dyda[0] = 1.0;

  for (i = 1; i < na; i++) {
    xfac *= x;
    y += a[i] * xfac;
    dyda[i] = xfac;
  }

  *y_p = y;

}

/**********************************************************************
 * _gjElim()
 *
 * Gauss-Jordan elimination
 *
 */

static int _gjElim(double **a, long n, double **b, long m)

{

  // cerr << "11111111111111 _gjElim 111111111111111111" << endl;

  long *indexc = (long *) RMmalloc(n * sizeof(long));
  long *indexr = (long *) RMmalloc(n * sizeof(long));
  long *indexPv = (long *) RMmalloc(n * sizeof(long));

  for (int j = 0; j < n; j++) {
    indexPv[j] = 0;
  }

  for (int i = 0; i < n; i++) {

    double large = 0.0;

    int irow = 0;
    int icol = 0;

    for (int j = 0; j < n; j++) {
      if (indexPv[j] !=  1) {
	for (int k = 0;k<n;k++) {
	  if (indexPv[k]  ==  0) {
	    if (fabs(a[j][k]) >=  large) {
	      large = fabs(a[j][k]);
	      irow = j;
	      icol = k;
	    }
	  } else if (indexPv[k] > 1) {
	    fprintf(stderr, "WARNING - _gjElim(1): singular matrix\n");
	    return -1;
	  }
	}
      }
    }

    indexPv[icol]++;

    if (irow !=  icol) {
      for (int l = 0; l < n; l++) {
        double tmp = a[irow][l];
        a[irow][l] = a[icol][l];
        a[icol][l] = tmp;
      }
      for (int l = 0;l < m; l++) {
        double tmp = b[irow][l];
        b[irow][l] = b[icol][l];
        b[icol][l] = tmp;
      }
    }
    indexr[i] = irow;
    indexc[i] = icol;
    if (a[icol][icol] == 0.0) {
      fprintf(stderr, "WARNING - _gjElim(2): singular matrix\n");
      return -1;
    }
    double inverse = 1.0 / a[icol][icol];
    a[icol][icol] = 1.0;
    for (int l = 0; l < n; l++) {
      a[icol][l] *=  inverse;
    }
    for (int l = 0; l < m;l++) {
      b[icol][l] *=  inverse;
    }
    for (int ll = 0; ll < n; ll++) {
      if (ll !=  icol) {
	double dummy = a[ll][icol];
	a[ll][icol] = 0.0;
	for (int l = 0; l < n; l++) {
          a[ll][l] -=  a[icol][l] * dummy;
        }
	for (int l = 0; l < m; l++) {
          b[ll][l] -=  b[icol][l] * dummy;
        }
      }
    }
  }

  for (int l = n-1; l >= 0; l--) {
    if (indexr[l] !=  indexc[l]) {
      for (int k = 0;k<n;k++) {
        double tmp = a[k][indexr[l]];
	a[k][indexr[l]] = a[k][indexc[l]];
	a[k][indexc[l]] = tmp;
      }
    }
  }
  
  RMfree((char *) indexPv);
  RMfree((char *) indexr);
  RMfree((char *) indexc);

  return 0;

}

/**************************************************************************
 * _lmCoeff()
 */

static void _lmCoeff(double *x, double *y, double *sig,
                     long nn, double *a, long nparams,
                     long *qqList, long mOrder, double **aCoeff,
                     double *bCoeff, double *chisq, evalFunc myfunc)
  
{
  
  double *dyda = (double *) RMmalloc (nparams * sizeof(double));
  
  for (int j = 0; j < mOrder; j++) {
    for (int k = 0; k <= j; k++) {
      aCoeff[j][k] = 0.0;
    }
    bCoeff[j] = 0.0;
  }

  *chisq = 0.0;
  for (int i = 0;i < nn; i++) {
    double ymod;
    myfunc(x[i], a, &ymod, dyda, nparams);
    double sig2i = 1.0 / (sig[i] *sig[i]);
    double dy = y[i] - ymod;
    for (int j = 0; j < mOrder; j++) {
      double wt = dyda[qqList[j]] * sig2i;
      for (int k = 0;k <= j; k++) {
	aCoeff[j][k] +=  wt*dyda[qqList[k]];
      }
      bCoeff[j] +=  dy*wt;
    }
    (*chisq) +=  dy * dy * sig2i;
  }

  for (int j = 1;j<mOrder;j++) {
    for (int k = 0; k <= j; k++) {
      aCoeff[k][j] = aCoeff[j][k];
    }
  }

  RMfree((char *) dyda);
  
}

/************************************************************************
 * _levMarq()
 *
 * Levenberg-Marquardt method for non-linear fit
 *
 */

static int _levMarq(double *x, double *y, double *sig,
                    long nn, double *a, long nparams,
                    long *qqList, long mOrder,
                    double **covar, double **aCoeff, double *chisq,
                    evalFunc myfunc, double *ldiff,
                    double *da, double *atry, double **oneda,
                    double *bCoeff, double &ochisq)

     
{

  // cerr << "11111111111111 _levMarq 111111111111111111" << endl;

  if (*ldiff < 0.0) {

    int kk=mOrder;
    for (int j=0;j<nparams;j++) {
      int ihit=0;
      for (int k=0;k<mOrder;k++)
	if (qqList[k] == j) ihit++;
      if (ihit == 0) {
	qqList[kk++]=j;
      } else if (ihit > 1) {
	fprintf(stderr,
		"WARNING - _levMarq: Bad QQLIST permutation in LEVMARQ-1\n");
	return -1;
      }
    }
    if (kk != nparams) {
      fprintf(stderr,
	      "WARNING - _levMarq: Bad QQLIST permutation in LEVMARQ-2\n");
      return -1;
    }
    *ldiff=0.001;
    _lmCoeff(x,y,sig,nn,a,nparams,qqList,mOrder,aCoeff,bCoeff,chisq,myfunc);
    ochisq=(*chisq);
  }

  for (int j=0;j<mOrder;j++) {
    for (int k=0;k<mOrder;k++) {
      covar[j][k]=aCoeff[j][k];
    }
    covar[j][j] = aCoeff[j][j]*(1.0+(*ldiff));
    oneda[j][0] = bCoeff[j];
  }

  if (_gjElim(covar,mOrder,oneda,1L)) {
    fprintf(stderr,
	    "WARNING - _levMarq: error in _gjElim\n");
    return -1;
  }

  for (int j=0;j<mOrder;j++) {
    da[j]=oneda[j][0];
  }
  if (*ldiff == 0.0) {
    _rearrangeCov(covar,nparams,qqList,mOrder);
    return 0;
  }

  for (int j=0;j<nparams;j++) {
    atry[j]=a[j];
  }

  for (int j=0;j<mOrder;j++) {
    atry[qqList[j]] = a[qqList[j]]+da[j];
  }

  _lmCoeff(x,y,sig,nn,atry,nparams,qqList,mOrder,covar,da,chisq,myfunc);

  if (*chisq < ochisq) {
    *ldiff *= 0.1;
    ochisq=(*chisq);
    for (int j=0;j<mOrder;j++) {
      for (int k=0;k<mOrder;k++) {
        aCoeff[j][k]=covar[j][k];
      }
      bCoeff[j]=da[j];
      a[qqList[j]]=atry[qqList[j]];
    }
  } else {
    *ldiff *= 10.0;
    *chisq=ochisq;
  }

  if (Debug) {
    fprintf(stderr, "a[%ld], chisq, ldiff = ", (long) nparams);
    for (int j = 0; j < nparams; j++)
      fprintf(stderr, " %g", a[j]);
    fprintf(stderr, " %g, %g\n", *chisq, *ldiff);
  } /* if (Debug) */
  
  return 0;

}

/***************************************************************************
 * approx_expon()
 *
 * approx exponential fit, to yield first-guess parameters
 * for uExponentialFit()
 *
 *  n: number of points in (x, y) data set
 *  x: array of x data
 *  y: array of y data
 *  a[] - first-guess params
 *
 * Returns 0 on success, -1 on error.
 *
 ***************************************************************************/
 
static int _approxExp(long n, double *x, double *y,
                      double *a, double *ymean_p)

{

  long i;

  double sign;
  double ymin, ymax;
  double ymean;
  double q[3];

  double sumx, sumx2;
  double sumy, sumxy;
  double dn;
  double term1, term2, term3;
  double xval, yval;
  double *x_p, *y_p;
  double std_error_est, r_squared;

  if (n < 3)
    return -1;
  
  dn = (double) n;

  /*
   * get min, max and mean values for y
   */

  ymin = LARGE_DOUBLE;
  ymax = -LARGE_DOUBLE;
  sumy = 0.0;

  y_p = y;
  for (i = 0; i < n; i++) {
    yval = *y_p++;
    sumy += yval;
    ymin = MIN(ymin, yval);
    ymax = MAX(ymax, yval);
  } /* i */
  
  ymean = sumy / dn;
  
  /*
   * perform a quadratic fit to test the concavity of the
   * data - concave up or concave down
   */

  uQuadFit(n, x, y, q,
	   &std_error_est, &r_squared);

  /*
   * set the a[0] parameter based on concavity (up or down)
   * If concave down, change the sign of the y values so that 
   * the fit is done concave upwards, and later multiply the
   * a[0] and a[1] values by -1. This technique allows us to use the
   * max value as a reasonable estimator for a[0].
   */
  
  if (q[2] > 0.0) {

    /*
     * concave upwards
     */

    a[0] = ymin - 1.0;
    sign = 1.0;

  } else {

    /*
     * concave downwards
     */
    
    a[0] = -ymax - 1.0;
    sign = -1.0;

  }
  
  /*
   * sum the various terms
   */
  
  x_p = x;
  y_p = y;
  sumx = 0.0;
  sumx2 = 0.0;
  sumy = 0.0;
  sumxy = 0.0;
  
  for (i = 0; i < n; i++) {
    xval = *x_p++;
    yval = log ((*y_p++ * sign) - a[0]);
    sumx += xval;
    sumx2 += xval * xval;
    sumy += yval;
    sumxy += xval * yval;
  }
  
  /*
   * compute the terms
   */

  term1 = dn * sumx2  - sumx * sumx;
  term2 = sumy * sumx2 - sumx * sumxy;
  term3 = dn * sumxy - sumx * sumy;
  
  /*
   * compute regression parameters
   */

  a[1] = exp(term2 / term1);
  a[2] = term3 / term1;

  /*
   * load up return values
   */

  *ymean_p = ymean;

  return 0;
  
}

