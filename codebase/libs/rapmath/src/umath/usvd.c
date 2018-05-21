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

/************************************************************************

Module:	usvd.c

Author:	C S Morse

Date:	Thu Sep 25 17:56:14 2003

Description: Routines for Singular Value Decomposition

************************************************************************/


/* System include files / Local include files */

#include <stdio.h>
#include <math.h>

#include <rapmath/usvd.h>
#include <rapmath/RMmalloc.h>
#include <toolsa/toolsa_macros.h>

/* Constant definitions / Macro definitions / Type definitions */

#define MAX_ITERATIONS	30

/* External global variables / Non-static global variables / Static globals */

static double DefaultMaxConditionNumber = 1e5;

/* External functions / Internal global functions / Internal static functions */


/************************************************************************

Function Name: 	upolynomial

Description:	polynomial basis functions

Returns:	none

Globals:	none

Notes:

************************************************************************/

void 
upolynomial( double x, 		/* I - specific x to evaluate			*/
	     int n,		/* I - order of polynomial (size of xfunc)	*/
	     double *xfunc	/* O - polynomial functions of x		*/
	     )
{
  int i;
  
  xfunc[0] = 1.0;
  for ( i=1; i<n; ++i )
    xfunc[i] = xfunc[i-1] * x;
}

/************************************************************************

Function Name: 	pythag

Description:	calculates length of hypotenuse of right triangle from sides

Returns:	sqrt(a*a + b*b)

Globals:	none

Notes:

************************************************************************/

static double 
pythag( double a, double b)
{
  return sqrt(a*a+b*b);
}

/************************************************************************

Function Name: 	sign

Description:	determines the sign of the input value

Returns:	1 is input is >= 0 ; -1 otherwise

Globals:	none

Notes:

************************************************************************/

static double 
sign( double x )
{
  return (x >= 0.0) ? 1.0 : -1.0;
}

/************************************************************************

Function Name: 	usvd

Description:	determines singular value decomposition A = U S Vtranspose
                of a real (nrow x ncol) rectangular matrix, A. 

Returns:	0 on success; negative error code otherwise (see notes)

Globals:	none

Notes:  This subroutine is a translation from the FORTRAN version dated 
        august 1983 by burton s. garbow, mathematics and computer science 
	div, argonne national laboratory which in turn was a translation
	of the algol procedure svd, num. math. 14, 403-420(1970) by golub 
	and reinsch.
	handbook for auto. comp., vol ii-linear algebra, 134-151(1971).

	Below are comments from the garbow FORTRAN version, modified as
	per the modifications for this version in c.

     this subroutine determines the singular value decomposition
     a=usv(transpose)  of a real nrow by ncol rectangular matrix.  
     householder bidiagonalization and a variant of the qr algorithm are used.

     on input

        nrow is the number of rows of a (and u).

        ncol is the number of columns of a (and u) and the order of v.

        a contains the rectangular input matrix to be decomposed.


     on output

        a is unaltered (unless overwritten by u or v).

        w contains the n (non-negative) singular values of a (the
          diagonal elements of s).  they are unordered.  if an
          error exit is made, the singular values should be correct
          for indices ierr+1,ierr+2,...,n.

        u contains the matrix u (orthogonal column vectors) of the
          decomposition.   u may coincide with a.
          if an error exit is made, the columns of u corresponding
          to indices of correct singular values should be correct.

        v contains the matrix v (orthogonal) of the decomposition 
           if an error exit is made, the columns of v corresponding 
	   to indices of correct singular values should be correct.

        ierr is set to
          zero       for normal return,
          k          if the k-th singular value has not been
                     determined after 30 iterations.

        rv1 is a temporary storage array.

      calls pythag for  dsqrt(a*a + b*b) .

************************************************************************/

int 
usvd( double **a, 	/* I - matrix to decompose		*/
      int nrow,		/* I - number of rows in a 		*/
      int ncol, 	/* I - number of columns in a		*/
      double **u, 	/* O - matrix nrow x ncol		*/
      double **v, 	/* O - matrix ncol x ncol		*/
      double *w 	/* O - array (ncol) diagonal elements	*/
      )
{
  double *rv1;
  double c, f,g,h, s, x,y,z;
  double scale, tst1, tst2;
  int converged, its;
  int i,j,k,l, i1,k1,l1, mn;
  

  /* copy the input A matrix into U */
  for ( i=0; i<nrow; ++i )
    for ( j=0; j<ncol; ++j )
      u[i][j] = a[i][j];

  /* allocate temporary array */
  rv1 = (double *) RMmalloc( ncol * sizeof(double) );
  
  /* householder reduction to bidiagonal form */
  g = scale = x = 0.0;
  
  for ( i=0; i<ncol; ++i )
    {
      l = i+1;
      rv1[i] = scale * g;
      g = s = scale = 0.0;
      if ( i < nrow )
	{
	  for ( k=i; k<nrow; ++k )
	    scale += fabs(u[k][i]);
	  
	  if ( scale != 0.0 )
	    {
	      for ( k=i; k<nrow; ++k )
		{
		  u[k][i] /= scale;
		  s += u[k][i]*u[k][i];
		}
	      
	      f = u[i][i];
	      g = -sign(f)*sqrt(s);
	      h = f*g - s;
	      u[i][i] = f - g;
	      
	      if ( i < ncol-1 )
		{
		  for ( j=l; j<ncol; ++j )
		    {
		      s = 0.0;
		      for ( k=i; k<nrow; ++k )
			s += u[k][i] * u[k][j];
		      
		      f = s / h;
		      
		      for ( k=i; k<nrow; ++k ) 
			u[k][j] += f * u[k][i];
		    }		  
		}

	      for ( k=i; k<nrow; ++k )
		u[k][i] *= scale;	      
	    }
	}
      
      w[i] = scale * g;
      g = s = scale = 0.0;
      
      if ( i < nrow && i < ncol-1 )
	{
	  for ( k=l; k<ncol; ++k )
	    scale += fabs(u[i][k]);
	  
	  if ( scale != 0.0 )
	    {
	      for ( k=l; k<ncol; ++k )
		{
		  u[i][k] /= scale;
		  s += u[i][k]*u[i][k];
		}

	      f = u[i][l];
	      g = -sign(f) * sqrt(s);
	      h = f*g - s;
	      u[i][l] = f - g;
	      
	      for ( k=l; k<ncol; ++k )
		rv1[k] = u[i][k] / h;
	      
	      if ( i != nrow-1 )
		{
		  for ( j=l; j<nrow; ++j )
		    {
		      s = 0.0;
		      for ( k=l; k<ncol; ++k )
			s += u[j][k] * u[i][k];
		      for ( k=l; k<ncol; ++k )
			u[j][k] += s * rv1[k];
		    }		  
		}

	      for ( k=l; k<ncol; ++k )
		u[i][k] *= scale;
	    }
	}

      x = MAX(x, fabs(w[i])+fabs(rv1[i]) );
    }

  /* accumulation of right-hand transformations */
  for ( i=ncol-1; i>=0; --i )
    {
      if ( i < ncol-1 )
	{
	  if ( g != 0.0 )
	    {
	      /* double division avoids possible overflow */
	      for ( j=l; j<ncol; ++j )
		v[j][i] = (u[i][j] / u[i][l]) / g;
	      
	      for ( j=l; j<ncol; ++j )
		{
		  s = 0.0;
		  for ( k=l; k<ncol; ++k )
		    s += u[i][k] * v[k][j];

		  for ( k=l; k<ncol; ++k )
		    v[k][j] += s * v[k][i];
		  
		}
	    }

	  for ( j=l; j<ncol; ++j )
	    v[i][j] = v[j][i] = 0.0;
	}
      
      v[i][i] = 1.0;
      g = rv1[i];
      l = i;
    }

  /* accumulation of left-hand transformations */
  mn = MIN(nrow,ncol) - 1;
  for( i=mn; i>=0; --i )
    {
      l = i + 1;
      g = w[i];
      
      if ( i != ncol )
	for ( j=l; j<ncol; ++j )
	  u[i][j] = 0.0;

      if ( g != 0.0 )
	{
	  if ( i != mn )
	    {
	      for ( j=l; j<ncol; ++j )
		{
		  s = 0.0;
		  for ( k=l; k<nrow; ++k )
		    s += u[k][i] * u[k][j];
		  
		  /* double division avoids possible overflow */
		  f = ( s / u[i][i] ) / g;
		
		  for ( k=i; k<nrow; ++k )
		    u[k][j] += f * u[k][i];
		}
	    }
	  
	  for ( j=i; j<nrow; ++j )
	    u[j][i] /= g;
	}
      else
	{	  
	  for( j=i; j<nrow; ++j )
	    u[j][i] = 0.0;
	}
      
      u[i][i] += 1.0;
    }

  /* diagonalization of the bidiagonal form */
  tst1 = x;
  for ( k=ncol-1; k>=0; --k )
    {
      k1 = k - 1;
      converged = 0;
      its = 0;

      while( !converged )
	{
	  for ( l=k; l>=0; --l )
	    {
	      l1 = l - 1;
	      tst2 = tst1 + fabs(rv1[l]);
	      if ( tst2 == tst1 )
		break;
	      tst2 = tst1 + fabs(w[l1]);
	      if ( tst2 == tst1 )
		{
		  /* cancellation of rv1[l] if l > 0 */
		  c = 0.0;
		  s = 1.0;
	      
		  for ( i=l; i<=k; ++i )
		    {
		      f = s * rv1[i];
		      rv1[i] *= c;
		      tst2 = tst1 + fabs(f);
		      if ( tst2 == tst1 )
			break;
		      g = w[i];
		      h = pythag(f,g);
		      w[i] = h;
		      
		      c = g / h;
		      s = -f / h;
		  
		      for ( j=0; j<nrow; ++j )
			{
			  y = u[j][l1];
			  z = u[j][i];
			  u[j][l1] = y*c + z*s;
			  u[j][i] = -y*s + z*c;
			}
		    }
		  break;
		}
	    }
      
	  /* test for convergence */
	  z = w[k];
	  if ( l == k )
	    {
	      /* CONVERGED ! */
	      converged = 1;
	      if ( z < 0.0 )
		{
		  /* w[k] is made non-negative */
		  w[k] = -z;
		  for ( j=0; j<ncol; ++j )
		    v[j][k] *= -1.0;
		}
	    }
	  else if ( its == MAX_ITERATIONS )
	    {
	      /* NOT CONVERGED AFTER MAX NUMBER OF ITERATIONS */
	      RMfree((char *) rv1 );
	      return -k;
	    }
	  else
	    {
	      /* NOT CONVERGED YET - ITERATE */
	      ++its;
	      /* shift from bottom 2 by 2 minor */
	      x = w[l];
	      y = w[k1];
	      g = rv1[k1];
	      h = rv1[k];
	      f = 0.5*(((g + z)/h) * ((g - z)/y) + y/h - h/y); 
	      g = pythag(f,1.0);
	      f = x - (z / x) * z + (h / x) * (y / (f + sign(f)*g) - h);
	      /* next qr transformation */
	      c = s = 1.0;
	      for ( i1=l; i1<=k1; ++i1 )
		{
		  i = i1 + 1;
		  g = rv1[i];
		  y = w[i];
		  h = s * g;
		  g *= c;
		  z = pythag(f,h);
		  rv1[i1] = z;
		  c = f / z;
		  s = h / z;
		  f = x*c + g*s;
		  g = -x*s + g*c;
		  h = y*s;
		  y *= c;

		  /* fill in v array to return */
		  for ( j=0; j<ncol; ++j )
		    {
		      x = v[j][i1];	
		      z = v[j][i];
		      v[j][i1] = x*c + z*s;
		      v[j][i] = -x*s + z*c;
		    }

		  z = pythag(f,h);
		  w[i1] = z;
		  /* rotation can be arbitrary if z is zero */
		  if ( z != 0.0 )
		    {
		      c = f / z;
		      s = h / z;
		    }
		  f = c*g + s*y;
		  x = -s*g + c*y;
	      
		  /* fill in the u array to return */
		  for ( j=0; j<nrow; ++j )
		    {
		      y = u[j][i1];
		      z = u[j][i];
		      u[j][i1] = y*c + z*s;
		      u[j][i] = -y*s + z*c;
		    }	      
		}

	      rv1[l] = 0.0;
	      rv1[k] = f;
	      w[k] = x;
	    }      
	}
    }
  
  
  RMfree((char *) rv1 );
  return 0;
}

/************************************************************************

Function Name: 	usvd_edit_sv

Description:	edits the singular values produced by usvd
                values below threshold are set to zero

Returns:	condition number or -1. if all values are zero.

Globals:	none

Notes:

************************************************************************/

double
usvd_edit_sv( double *w,			/* I/O - W from SVD 	*/ 
	      int n, 				/* I - size of W	*/
	      double max_condition_number 	/* I - max allowed	*/
	      )
{
  double wmax = 0.0;		/* maximum value in w matrix */
  double wmin = 99999.;		/* minimum value in w matrix */
  double thr;			/* singular value threshold  */	
  double condition_number;
  
  int i;
  
  /* find the maximum value */
  for ( i=0; i<n; ++i )
    wmax = MAX( wmax, w[i] );
  
  /* set the threshold based on the max condition number allowed */
  thr = wmax / max_condition_number;

  /* edit the values and find the minimum value */
  for ( i=0; i<n; ++i )
    {
      if ( w[i] < thr )
	w[i] = 0.0;
      else 
	wmin = (wmin == 99999.) ? w[i] : MIN( wmin, w[i] );
    }

  /* evaluate the condition number */
  condition_number = ( wmin == 99999. ) ? -1 : wmax / wmin;
  
  return( condition_number ); 
} 

/************************************************************************

Function Name: 	usvd_apply

Description:	use the singular value decomposition of a matrix A 
                to solve A X = B for X

Returns:	none

Globals:	none

Notes: 		X = V [1 / w] ( Utranspose B )
   
                usvd decomposes A into U, V, and W.

************************************************************************/

void
usvd_apply( double** u,	/* I - U matrix from SVD decomposition		*/ 
	    double *w, 	/* I - W diag array from SVD decomposition	*/ 
	    double **v, /* I - V matrix from SVD decomposition		*/
	    int ndata, 	/* I - number of rows in U			*/
	    int nvar,	/* I - number of cols in U, order of V, W size	*/
	    double* b, 	/* I - known values to solve for		*/
	    double *x 	/* O - values solved for			*/
	 )
{
  double *utb;
  int idat, ivar, jvar;

  /* first calculate Utranspose B */
  utb = (double *) RMcalloc( nvar, sizeof(double) );  
  for ( ivar=0; ivar<nvar; ++ivar )
    {
      utb[ivar] = 0.0;
      if ( w[ivar] != 0.0 )
	{
	  for ( idat=0; idat<ndata; ++idat )
	    utb[ivar] += u[idat][ivar] * b[idat];

	  /* multiply by diagonal 1/w */
	  utb[ivar] /= w[ivar];
	}
    }

  /* matrix multiply by V */
  for ( ivar=0; ivar<nvar; ++ivar )
    {
      x[ivar] = 0.0;
      for ( jvar=0; jvar<nvar; ++jvar )
	x[ivar] += v[ivar][jvar] * utb[jvar];
    }
  
  RMfree( utb );
}

/************************************************************************

Function Name: 	usvd_chisquare

Description:	calculates the chisquare score for a fit

Returns:	chisquare value

Globals:	none

Notes:

************************************************************************/

double 
usvd_chisquare( double *x,		/* I - data x values			*/
		double *y,		/* I - data y values			*/
		int ndata,		/* I - size of x,y, weights arrays	*/
		int nfit,		/* I - number of fit parameters		*/
		double *a,		/* I - fit parameters			*/
		double *weight,		/* I - data point weights (or NULL)	*/
		void (*bfunc)(double, int, double* )/* I - basis functions	*/
		)
{
  double chisq;
  double y_calc, wt, term, *basis;
  int idat, ifit;

  
  chisq = 0.0;
  basis = (double *) RMcalloc( nfit, sizeof(double) );

  for ( idat=0; idat<ndata; ++idat )
    {
      /* evaluate the basis functions for this x */
      bfunc( x[idat], nfit, basis );

      /* calculate the predicted value from the fit */
      y_calc = 0.0;
      for ( ifit=0; ifit<nfit; ++ifit )
	y_calc += a[ifit]*basis[ifit];

      /* calculate the weighted difference between observed and predicted y */
      wt = (weight == NULL) ? 1.0 : weight[idat];
      term = wt * (y[idat] - y_calc);

      /* sum the squares of the difference */
      chisq += term*term;
    }

  RMfree( basis );
  
  return chisq;
}

/************************************************************************

Function Name: 	usvd_fit_sigma

Description:	calculate the std dev of fit parameters determined using SVD

Returns:	none

Globals:	none

Notes:

************************************************************************/

void 
usvd_fit_sigma( double **vv,	/* I - V matrix from SVD		*/
		double *w,	/* I - W diagonal array from SVD	*/
		int nfit,	/* I - size of w, v dimensions		*/
		double *sigma	/* O - sigma of fit parameters		*/
	     )
{
  double term;
  int ifit, jfit;

  for ( ifit=0; ifit<nfit; ++ifit )
    {
      sigma[ifit] = 0.0;
      for ( jfit=0; jfit<nfit; ++jfit )
	if ( w[ifit] != 0.0 )
	  {
	    term = vv[ifit][jfit] / w[jfit];
	    sigma[ifit] += term * term;
	  }
      sigma[ifit] = sqrt(sigma[ifit]);
    }
}

/************************************************************************

Function Name: 	usvd_solve

Description:	finds the best solution A for Y = A X using SVD
                where X uses a specified set of basis functions of x

Returns:	0 on success; -1 othewise

Globals:	DefaultMaxConditionNumber

Notes:          optional outputs are calculated only if receiving arrays
                are supplied; caller responsible for correct sizing.

		One suggestion for weights is 1/sigma for the y measurement.

************************************************************************/

int 
usvd_solve( double *x, 		/* I - input x array 			*/
	    double *y, 		/* I - input y array 			*/
	    int ndata, 		/* I - size of x and y arrays		*/
	    int nfit,		/* I - number of fit parameters		*/
	    double *a, 		/* O - fit parameters			*/
	    double *weight,	/* I - weights on y (or NULL)		*/
	    double *chisq,	/* O - chisquare for fit (or NULL)	*/
	    double *a_sigma,	/* O - sigma of fit parameters (or NULL)*/
	    void (*bfunc)(double, int, double* )/* I - basis functions	*/
	    )
{
  double **uu, **vv, *w;
  double *basis, *b, wt;
  int rc;
  int idat, ifit;
  
  /* Create the svd arrays */
  uu = (double **) RMcalloc2( ndata, nfit, sizeof(double) );
  vv = (double **) RMcalloc2( nfit, nfit, sizeof(double) );
  w = (double *) RMcalloc( nfit, sizeof(double) );
  
  /* Create the input A array for SVD as basis functions of x */
  basis = (double *) RMcalloc( nfit, sizeof(double) );
  b = (double *) RMcalloc( ndata, sizeof(double) );
  for ( idat=0; idat<ndata; ++idat )
    {
      /* calculate the basis functions for x[idat] */
      wt = 1.0;
      bfunc( x[idat], nfit, basis );
      for ( ifit=0; ifit<nfit; ++ifit )
	{
	  wt = (weight == NULL) ? 1.0 : weight[idat];
	  uu[idat][ifit] = basis[ifit] * wt;
	}
      b[idat] = y[idat] * wt;
    }

  /* Perform the singular value decomposition (reuse the uu array) */
  if ( 0 == (rc=usvd( uu, ndata, nfit, uu, vv, w )) )
    {
      /* edit the singular values */
      usvd_edit_sv( w, nfit, DefaultMaxConditionNumber );
      
      /* solve the equation */
      usvd_apply( uu, w, vv, ndata, nfit, b, a );
      
      /* calculate the chisquare values, if desired */
      if ( chisq != NULL )
	{
	  *chisq = usvd_chisquare( x, y, ndata, nfit, a, weight, bfunc );
	}

      /* calculate the variances of the fit parameters, if desired */
      if ( a_sigma != NULL )
	{
	  usvd_fit_sigma( vv, w, nfit, a_sigma );
	}
    }
  
  RMfree2( (void **) uu );
  RMfree2( (void **) vv );
  RMfree( w );
  RMfree( b );
  RMfree( basis );

  return rc;
}

int 
usvd_solve_peek( double *x, 		/* I - input x array 			*/
		 double *y, 		/* I - input y array 			*/
		 int ndata, 		/* I - size of x and y arrays		*/
		 int nfit,		/* I - number of fit parameters		*/
		 double *a, 		/* O - fit parameters			*/
		 double *weight,	/* I - weights on y (or NULL)		*/
		 double *chisq,		/* O - chisquare for fit (or NULL)	*/
		 double *a_sigma,	/* O - sigma of fit parameters (or NULL)*/
		 double **u,		/* O - u matrix (or NULL)		*/
		 double **v,		/* O - v matrix (or NULL)		*/
		 double *w,		/* O - w array (or NULL)		*/
		 void (*bfunc)(double, int, double* )/* I - basis functions	*/
	    )
{
  double **uu, **vv, *ww;
  double *basis, *b, wt;
  int rc;
  int idat, ifit;
  
  /* Create the svd arrays */
  uu = (u == NULL) ? (double **) RMcalloc2( ndata, nfit, sizeof(double) ) : u;
  vv = (v == NULL) ? (double **) RMcalloc2( nfit, nfit, sizeof(double) ) : v;
  ww = (w == NULL) ? (double *) RMcalloc( nfit, sizeof(double) ) : w;
  
  /* Create the input A array for SVD as basis functions of x */
  basis = (double *) RMcalloc( nfit, sizeof(double) );
  b = (double *) RMcalloc( ndata, sizeof(double) );
  for ( idat=0; idat<ndata; ++idat )
    {
      /* calculate the basis functions for x[idat] */
      wt = 1.0;
      bfunc( x[idat], nfit, basis );
      for ( ifit=0; ifit<nfit; ++ifit )
	{
	  wt = (weight == NULL) ? 1.0 : weight[idat];
	  uu[idat][ifit] = basis[ifit] * wt;
	}
      b[idat] = y[idat] * wt;
    }

  /* Perform the singular value decomposition (reuse the uu array) */
  if ( 0 == (rc=usvd( uu, ndata, nfit, uu, vv, ww )) )
    {
      /* edit the singular values */
      usvd_edit_sv( ww, nfit, DefaultMaxConditionNumber );
      
      /* solve the equation */
      usvd_apply( uu, ww, vv, ndata, nfit, b, a );
      
      /* calculate the chisquare values, if desired */
      if ( chisq != NULL )
	{
	  *chisq = usvd_chisquare( x, y, ndata, nfit, a, weight, bfunc );
	}

      /* calculate the variances of the fit parameters, if desired */
      if ( a_sigma != NULL )
	{
	  usvd_fit_sigma( vv, ww, nfit, a_sigma );
	}
    }
  
  if ( u == NULL )
    RMfree2( (void **) uu );
  if ( v == NULL )
    RMfree2( (void **) vv );
  if ( w == NULL )
    RMfree( ww );
  RMfree( b );
  RMfree( basis );

  return rc;
}

/************************************************************************

Function Name: 	usvd_print_dbg

Description:	debug print routine for svd matrices prints to stderr

Returns:	none

Globals:	none	

Notes:

************************************************************************/

void usvd_print_dbg( int ptype,		/* I - 0: print original matrix	**
					**     1: print decomp matrices	** 
					**        and product		*/
		     int m,		/* I - dimension of u matrix	*/
		     int n,		/* I - dimension of u,v matrix	*/
		     double **u,	/* I - a or u matrix		*/
		     double *w,		/* I - w array (diagonal matrix)*/
		     double **v		/* I - v matrix			*/
		     )
{
  int j,k,l;
  double *a;

  switch (ptype) 
    {
    case 0:
      fprintf(stderr,"\nCheck product against original matrix:\n");
      fprintf(stderr,"Original matrix: ( Precision 6 )\n");
      for ( k=0; k<m; k++ ) 
	{
	  for ( l=0; l<n; l++ )
	    fprintf(stderr,"%12.6f",u[k][l]);
	  fprintf(stderr,"\n");
	}
      break;
    case 1:
      a = (double *) RMcalloc( n, sizeof(double) );
       
      fprintf(stderr,"Product u*w*(v-transpose):\n");
      for ( k=0; k<m; k++ ) 
	{
	  for ( l=0; l<n; l++ ) 
	    {
	      a[l]=0.0;
	      for ( j=0; j<n; j++ )
		a[l] += u[k][j]*w[j]*v[l][j];
	    }
	  for (l=0;l<n;l++ ) fprintf(stderr,"%12.6f",a[l]);
	  fprintf(stderr,"\n");
	}
      fprintf(stderr,"\nDecomposition matrices:\n");    
      fprintf(stderr,"Matrix u\n");    
      for ( k=0; k<m; k++ ) 
	{
	  for ( l=0; l<n; l++ )
	    fprintf(stderr,"%12.6f",u[k][l]);
	  fprintf(stderr,"\n");
	}
      fprintf(stderr,"Diagonal of matrix w\n");
      for ( k=0; k<n; k++ )
	fprintf(stderr,"%12.6f",w[k]);
      fprintf(stderr,"\nMatrix v-transpose\n");
      for ( k=0;k<n; k++ ) 
	{
	  for ( l=0; l<n; l++ )
	    fprintf(stderr,"%12.6f",v[l][k]);
	  fprintf(stderr,"\n");
	}
      fprintf(stderr,"**********************************\n");
      RMfree(a);
      break;
    }
}





