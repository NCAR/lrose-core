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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: usvd.h,v 1.3 2016/03/03 18:46:09 dixon Exp $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/


/************************************************************************

Header: usvd.h

Author: C S Morse

Date:	Tue Sep 30 10:42:17 2003

Description:	Routines for Single Value Decomposition

*************************************************************************/

# ifndef    USVD_H
# define    USVD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* System include files / Local include files */


/* Constant definitions / Macro definitions / Type definitions */


/* External global variables / Non-static global variables / Static globals */


/* External functions / Internal global functions / Internal static functions */

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

extern int 
usvd( double **a, 	/* I - matrix to decompose		*/
      int nrow,		/* I - number of rows in a 		*/
      int ncol, 	/* I - number of columns in a		*/
      double **u, 	/* O - matrix nrow x ncol		*/
      double **v, 	/* O - matrix ncol x ncol		*/
      double *w 	/* O - array (ncol) diagonal elements	*/
      );


/************************************************************************

Function Name: 	usvd_edit_sv

Description:	edits the singular values produced by usvd
                values below threshold are set to zero

Returns:	condition number or -1. if all values are zero.

Globals:	none

Notes:

************************************************************************/

extern double
usvd_edit_sv( double *w,			/* I/O - W from SVD 	*/ 
	      int n, 				/* I - size of W	*/
	      double max_condition_number 	/* I - max allowed	*/
	      );

/************************************************************************

Function Name: 	usvd_apply

Description:	use the singular value decomposition of a matrix A 
                to solve A X = B for X

Returns:	none

Globals:	none

Notes: 		X = V [1 / w] ( Utranspose B )
   
                usvd decomposes A into U, V, and W.

************************************************************************/

extern void
usvd_apply( double** u,	/* I - U matrix from SVD decomposition		*/ 
	    double *w, 	/* I - W diag array from SVD decomposition	*/ 
	    double **v, /* I - V matrix from SVD decomposition		*/
	    int ndata, 	/* I - number of rows in U			*/
	    int nvar,	/* I - number of cols in U, order of V, W size	*/
	    double* b, 	/* I - known values to solve for		*/
	    double *x 	/* O - values solved for			*/
	    );

/************************************************************************

Function Name: 	usvd_chisquare

Description:	calculates the chisquare score for a fit

Returns:	chisquare value

Globals:	none

Notes:

************************************************************************/

extern double 
usvd_chisquare( double *x,		/* I - data x values			*/
		double *y,		/* I - data y values			*/
		int ndata,		/* I - size of x,y, weights arrays	*/
		int nfit,		/* I - number of fit parameters		*/
		double *a,		/* I - fit parameters			*/
		double *weights,	/* I - data point weights (or NULL)	*/
		void (*bfunc)(double, int, double* )/* I - basis functions	*/
		);

/************************************************************************

Function Name: 	usvd_fit_sigma

Description:	calculate the std dev of fit parameters determined using SVD

Returns:	none

Globals:	none

Notes:

************************************************************************/

extern void 
usvd_fit_sigma( double **vv,	/* I - V matrix from SVD		*/
		double *w,	/* I - W diagonal array from SVD	*/
		int nfit,	/* I - size of w, v dimensions		*/
		double *sigma	/* O - sigma of fit parameters		*/
		);

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

extern int 
usvd_solve( double *x, 		/* I - input x array 			*/
	    double *y, 		/* I - input y array 			*/
	    int ndata, 		/* I - size of x and y arrays		*/
	    int nfit,		/* I - number of fit parameters		*/
	    double *a, 		/* O - fit parameters			*/
	    double *weight,	/* I - weights on y (or NULL)		*/
	    double *chisq,	/* O - chisquare for fit (or NULL)	*/
	    double *a_sigma,	/* O - sigma of fit parameters (or NULL)*/
	    void (*bfunc)(double, int, double* )/* I - basis functions	*/
	    );

/* This version allows the caller to pass in any/all of u,v,w arrays */
extern int 
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
		 );

/************************************************************************

Function Name: 	upolynomial

Description:	polynomial basis functions

Returns:	none

Globals:	none

Notes:

************************************************************************/

extern void 
upolynomial( double x, 		/* I - specific x to evaluate			*/
	     int n,		/* I - order of polynomial (size of xfunc)	*/
	     double *xfunc	/* O - polynomial functions of x		*/
	     );

/************************************************************************

Function Name: 	usvd_print_dbg

Description:	debug print routine for svd matrices prints to stderr

Returns:	none

Globals:	none	

Notes:

************************************************************************/

extern void 
usvd_print_dbg( int ptype,	/* I - 0: print original matrix	**
				**     1: print decomp matrices	** 
				**        and product		*/
		int m,		/* I - dimension of u matrix	*/
		int n,		/* I - dimension of u,v matrix	*/
		double **u,	/* I - a or u matrix		*/
		double *w,	/* I - w array (diagonal matrix)*/
		double **v	/* I - v matrix			*/
		);


#ifdef __cplusplus
}
#endif

# endif     /* USVD_H */







