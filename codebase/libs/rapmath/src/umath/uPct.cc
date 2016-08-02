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

#include <math.h>
#include <iostream>
#include <rapmath/umath.h>
#include <rapmath/RMmalloc.h>

using namespace std;

static void _eigSort(double *d, double **v, int nn);
static void _houseReduce(double **aa, double *dd, double *ee, long nn);
static int _triDiagImpl(double *d, double *e, double **z, long n);

/**************************************************************************
 * uPct.cc
 *
 * obtains the principal component transformation for the data
 * 
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * February 1991
 *
 * Args: long ndim - the dimension of the data vector.
 *       long ndata - the number of data vectors.
 *       double **data - array for the data vectors, in the order
 *         data[ndata][ndim].
 *       double *means - address of a (double *) which points to
 *         the array of means. This array is allocated by this routine.
 *       double **eigenvectors - address of a (double **) which points
 *         to the eigenvector matrix array x. The eigenvectors are in the
 *         columns, where the columns are referred to by the second
 *         dereference, i.e. x[irow][icol].
 *       double *eigenvalues - address of a (double *) which points to
 *         the eigenvalue array lambda.
 *
 * Return value: 0 on success, -1 on failure
 *
 **************************************************************************/

int upct(long ndim, long ndata, double **data,
	 double *means, double **eigenvectors, double *eigenvalues)
  
{

  /*
   * set ndata as a double
   */

  double dndata = (double) ndata;

  /*
   * allocate arrays
   */


  double *sum_x = (double *) RMcalloc (ndim, sizeof(double));
  double **sum_xx = (double **) RMcalloc2 (ndim, ndim, sizeof(double));

  /*
   * e is a temporary array which holds the off-diagional elements
   * produced by utridiag_reduce() and passes them to 
   * utridiag_ql_impl()
   */

  double *e = (double *) RMmalloc (ndim * sizeof(double));

  /*
   * sum up for stats computations
   */

  for (int idata = 0; idata < ndata; idata++) {
    for (int irow = 0; irow < ndim; irow++) {
      sum_x[irow] += data[idata][irow];
      for (int icol = 0; icol <= irow; icol++) {
	sum_xx[irow][icol] += data[idata][irow] * data[idata][icol];
      }
    }
  } /* idata */

  /*
   * compute mean vector and covariance matrix
   */

  for (int irow = 0; irow < ndim; irow++) {

    means[irow] = sum_x[irow] / dndata;

    for (int icol = 0; icol <= irow; icol++) {

      if (icol == irow) {
	eigenvectors[irow][icol] =
	  ((dndata * sum_xx[irow][icol] - sum_x[irow] * sum_x[irow]) /
	   (dndata * dndata));
      } else {
	eigenvectors[irow][icol] = 
	  ((dndata * sum_xx[irow][icol] - sum_x[irow] * sum_x[icol]) /
	   (dndata * dndata));
      }

    } /* icol */

  } /* irow */

  /*
   * copy covariance entries to the upper diagonal region of the matrix -
   * note that this is a symmetric real matrix
   */

  for (int irow = 0; irow < ndim - 1; irow++) {
    for (int icol = irow + 1; icol < ndim; icol++) {
      eigenvectors[irow][icol] = eigenvectors[icol][irow];
    } /* icol */
  } /* irow */

  /*
   * reduce the matrix using the householder reduction
   */

  _houseReduce(eigenvectors, eigenvalues, e, ndim);

  /*
   * obtain eigenvectors and eigenvalues
   */

  if (_triDiagImpl(eigenvalues, e, eigenvectors, ndim)) {
    cerr << "ERROR - uPct" << endl;
    return -1;
  }

  /*
   * set any negative eigenvalues to zero
   */

  for (int idim = 0; idim < ndim; idim++) {
    if(eigenvalues[idim] < 0.0) {
      eigenvalues[idim] = 0.0;
    }
  }

  /*
   * sort the eigenvalues and eigenvectors
   */

  _eigSort(eigenvalues, eigenvectors, ndim);
  
  /*
   * free up resources
   */

  RMfree((void *) e);
  RMfree((void *) sum_x);
  RMfree2((void **) sum_xx);

  return 0;

}

/**************************************************************************
 * _houseReduce.c 
 *
 * Householder tridiagonal reduction of real symmetric matrix
 *
 **************************************************************************/

static void _houseReduce(double **aa, double *dd, double *ee, long nn)
{

  for (int ii = nn - 1; ii > 0; ii--) {

    int ll = ii - 1;
    double qq = 0.0;
    double scale = 0.0;

    if (ll > 0) {

      for (int kk = 0; kk <= ll; kk++) {
	scale += fabs(aa[ii][kk]);
      }

      if (scale == 0.0) {

	ee[ii] = aa[ii][ll];

      } else {

	for (int kk = 0; kk <= ll; kk++) {
	  aa[ii][kk] /= scale;
	  qq += aa[ii][kk] * aa[ii][kk];
	}

	double ff = aa[ii][ll];
        double gg = sqrt(qq);
        if (ff > 0) {
          gg *= -1.0;
        }
	ee[ii] = scale * gg;
	qq -= ff * gg;
	aa[ii][ll] = ff - gg;
	ff = 0.0;

	for (int jj = 0; jj <= ll; jj++) {

	  aa[jj][ii] = aa[ii][jj] / qq;

	  gg = 0.0;

	  for (int kk = 0; kk <= jj; kk++) {
	    gg += aa[jj][kk] * aa[ii][kk];
          }
          
	  for (int kk = jj + 1; kk <= ll; kk++) {
	    gg += aa[kk][jj] * aa[ii][kk];
          }

	  ee[jj] = gg/qq;
	  ff += ee[jj] * aa[ii][jj];

	} /* jj */

	double hh = ff / (2.0 * qq);

	for (int jj = 0; jj <= ll; jj++) {
          
	  ff = aa[ii][jj];
	  gg = ee[jj] - hh * ff;
	  ee[jj] = gg;

	  for (int kk = 0; kk <= jj; kk++) {
	    aa[jj][kk] -= (ff * ee[kk] + gg * aa[ii][kk]);
          }

	} /* jj */

      } /* if (scale == 0.0) */

    } else {

      ee[ii] = aa[ii][ll];

    } /* if (ll > 0) */

    dd[ii] = qq;

  } /* i */

  /* prepare eigenvectors */

  dd[0] = 0.0;
  ee[0] = 0.0;
  
  for (int ii = 0; ii < nn; ii++) {

    int ll = ii - 1;

    if (dd[ii]) {
 
     for (int jj = 0; jj <= ll; jj++) {

	double gg = 0.0;

	for (int kk = 0; kk <= ll; kk++) {
	  gg += aa[ii][kk] * aa[kk][jj];
        }

	for (int kk = 0; kk <= ll; kk++) {
	  aa[kk][jj] -= gg * aa[kk][ii];
        }

      } /* jj */

    } /* ii */

    dd[ii] = aa[ii][ii];
    aa[ii][ii] = 1.0;

    for (int jj = 0; jj <= ll; jj++) {
      aa[jj][ii] = aa[ii][jj] = 0.0;
    }

  } /* i */

}

/**************************************************************************
 * _triDiagImpl.c 
 *
 * Householder tridiagonal reduction of real symmetric matrix
 *
 * Returns 0 on success, -1 on failure.
 *
 **************************************************************************/

static int _triDiagImpl(double *dd, double *ee, double **zz, long nn)
{

  for (int ii = 1; ii < nn; ii++) {
    ee[ii-1] = ee[ii];
  }

  ee[nn - 1] = 0.0;

  for (int ll = 0; ll < nn; ll++) {

    int iter = 0;
    int mm = 0;

    do {

      for (mm = ll; mm < nn-1; mm++) {
	double xx = fabs(dd[mm]) + fabs(dd[mm+1]);
	if (fabs(ee[mm]) + xx == xx) break;
      }
 
     if (mm != ll) {

	if (iter++ == 30) {
	  fprintf(stderr, "ERROR - routine '_triDiagImpl'\n");
	  fprintf(stderr, "  Too many iterations.\n");
	  return -1;
	}

	double gg = (dd[ll+1] - dd[ll]) / (2.0 * ee[ll]);
	double rr = sqrt((gg * gg) + 1.0);
        double xx = fabs(rr);
        if (gg < 0) {
          xx *= -1.0;
        }
	gg = dd[mm] - dd[ll] + ee[ll] / (gg + xx);
	double ss = 1.0;
	double cc = 1.0;
	double pp = 0.0;

	for (int ii = mm - 1; ii >= ll; ii--) {

	  double ff = ss * ee[ii];
	  double bb = cc * ee[ii];

	  if (fabs(ff) >= fabs(gg)) {

	    cc = gg / ff;
	    rr = sqrt((cc * cc) + 1.0);
	    ee[ii+1] = ff * rr;
	    ss = 1.0 / rr;
	    cc *= ss;

	  } else {
            
	    ss = ff / gg;
	    rr = sqrt((ss * ss) + 1.0);
	    ee[ii+1] = gg * rr;
	    cc = 1.0 / rr;
	    ss *= cc;

	  } /* if (fabs(ff) ... */

	  gg = dd[ii+1] - pp;
	  rr = (dd[ii] - gg) * ss + 2.0 * cc * bb;
	  pp = ss * rr;
	  dd[ii+1] = gg + pp;
	  gg = cc * rr - bb;

	  /* Next loop can be omitted if eigenvectors not wanted */

	  for (int kk = 0; kk < nn; kk++) {
            
	    ff = zz[kk][ii+1];
	    zz[kk][ii+1] = ss * zz[kk][ii] + cc * ff;
	    zz[kk][ii] = cc * zz[kk][ii] - ss * ff;

	  } /* kk */

	} /* ii */

	dd[ll] = dd[ll] - pp;
	ee[ll] = gg;
	ee[mm] = 0.0;
 
      } /* if (mm != ll) */

    } while (mm != ll); /* do */

  } /* l */

  return 0;

}

/**************************************************************************
 * _eigSort
 *
 * sort eigenvalue vector and eigenvector matrix
 *
 **************************************************************************/

static void _eigSort(double *dd, double **vv, int nn)
{

  for (int ii = 0; ii < nn - 1; ii++) {

    int kk = ii;
    double pp = dd[ii];
    
    for (int jj = ii + 1; jj < nn; jj++) {
      if (dd[jj] >= pp) {
        kk = jj;
	pp = dd[jj];
      }
    }

    if (kk != ii) {

      dd[kk] = dd[ii];
      dd[ii] = pp;

      for (int mm = 0; mm < nn; mm++) {

	pp = vv[mm][ii];
	vv[mm][ii] = vv[mm][kk];
	vv[mm][kk] = pp;

      } /* mm */

    } /* if (kk != ii) */

  } /* ii */

}
