/* ForsytheFit.f -- translated by f2c (version 20160102).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#ifdef JUNK

/* Table of constant values */

static integer c__9 = 9;
static integer c__1 = 1;
static integer c__3 = 3;
static integer c__5 = 5;

/* Main program */ int MAIN__(void)
{
    /* Format strings */
    static char fmt_50[] = "(\002  \002,i2,\002   X  Y = \002)";
    static char fmt_60[] = "(\002  \002,i2,\002   \002,f9.6)";
    static char fmt_70[] = "(\002 Standard deviation: \002,f9.6//)";

    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    integer s_wsle(cilist *), do_lio(integer *, integer *, char *, ftnlen), 
	    e_wsle(void), s_wsfe(cilist *), e_wsfe(void), s_rsle(cilist *), 
	    e_rsle(void), do_fio(integer *, char *, ftnlen);

    /* Local variables */
    static doublereal c__[26];
    static integer i__, l, m, n;
    static doublereal x[25], y[25], e1, dd;
    static real stop;
    extern /* Subroutine */ int ls_poly__(integer *, doublereal *, integer *, 
	    integer *, doublereal *, doublereal *, doublereal *, doublereal *)
	    ;

    /* Fortran I/O blocks */
    static cilist io___1 = { 0, 6, 0, 0, 0 };
    static cilist io___2 = { 0, 6, 0, 0, 0 };
    static cilist io___3 = { 0, 6, 0, "(' What is the order of the fit      "
	    ": ')", 0 };
    static cilist io___4 = { 0, 5, 0, 0, 0 };
    static cilist io___6 = { 0, 6, 0, "(' What is the error reduction factor"
	    ": ')", 0 };
    static cilist io___7 = { 0, 5, 0, 0, 0 };
    static cilist io___9 = { 0, 6, 0, "(' How many data pooints are there   "
	    ": ')", 0 };
    static cilist io___10 = { 0, 5, 0, 0, 0 };
    static cilist io___12 = { 0, 6, 0, 0, 0 };
    static cilist io___13 = { 0, 6, 0, 0, 0 };
    static cilist io___15 = { 0, 6, 0, fmt_50, 0 };
    static cilist io___16 = { 0, 5, 0, 0, 0 };
    static cilist io___19 = { 0, 6, 0, 0, 0 };
    static cilist io___23 = { 0, 6, 0, 0, 0 };
    static cilist io___24 = { 0, 6, 0, 0, 0 };
    static cilist io___25 = { 0, 6, 0, fmt_60, 0 };
    static cilist io___26 = { 0, 6, 0, fmt_70, 0 };


    s_wsle(&io___1);
    do_lio(&c__9, &c__1, " ", (ftnlen)1);
    e_wsle();
    s_wsle(&io___2);
    do_lio(&c__9, &c__1, "LEAST SQUARES POLYNOMIAL FITTING", (ftnlen)32);
    e_wsle();
    s_wsfe(&io___3);
    e_wsfe();
    s_rsle(&io___4);
    do_lio(&c__3, &c__1, (char *)&m, (ftnlen)sizeof(integer));
    e_rsle();
    s_wsfe(&io___6);
    e_wsfe();
    s_rsle(&io___7);
    do_lio(&c__5, &c__1, (char *)&e1, (ftnlen)sizeof(doublereal));
    e_rsle();
    s_wsfe(&io___9);
    e_wsfe();
    s_rsle(&io___10);
    do_lio(&c__3, &c__1, (char *)&n, (ftnlen)sizeof(integer));
    e_rsle();
    s_wsle(&io___12);
    do_lio(&c__9, &c__1, " ", (ftnlen)1);
    e_wsle();
    s_wsle(&io___13);
    do_lio(&c__9, &c__1, "Input the data points as prompted:", (ftnlen)34);
    e_wsle();
    i__1 = n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	s_wsfe(&io___15);
	do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
	e_wsfe();
	s_rsle(&io___16);
	do_lio(&c__5, &c__1, (char *)&x[i__ - 1], (ftnlen)sizeof(doublereal));
	do_lio(&c__5, &c__1, (char *)&y[i__ - 1], (ftnlen)sizeof(doublereal));
	e_rsle();
    }
    s_wsle(&io___19);
    do_lio(&c__9, &c__1, " ", (ftnlen)1);
    e_wsle();
    ls_poly__(&m, &e1, &n, &l, x, y, c__, &dd);
    s_wsle(&io___23);
    do_lio(&c__9, &c__1, "Coefficients are:", (ftnlen)17);
    e_wsle();
    s_wsle(&io___24);
    do_lio(&c__9, &c__1, " ", (ftnlen)1);
    e_wsle();
    i__1 = l;
    for (i__ = 0; i__ <= i__1; ++i__) {
	s_wsfe(&io___25);
	do_fio(&c__1, (char *)&i__, (ftnlen)sizeof(integer));
	do_fio(&c__1, (char *)&c__[i__ - 1], (ftnlen)sizeof(doublereal));
	e_wsfe();
    }
    s_wsfe(&io___26);
    do_fio(&c__1, (char *)&dd, (ftnlen)sizeof(doublereal));
    e_wsfe();
    stop = 4.f;
    return 0;
} /* MAIN__ */

#endif

/* ***************************************************************** */
/* *         LEAST SQUARES POLYNOMIAL FITTING PROCEDURE            * */
/* * ------------------------------------------------------------- * */
/* * This program least squares fits a polynomial to input data.   * */
/* * forsythe orthogonal polynomials are used in the fitting.      * */
/* * The number of data points is n.                               * */
/* * The data is input to the subroutine in x[i], y[i] pairs.      * */
/* * The coefficients are returned in c[i],                        * */
/* * the smoothed data is returned in v[i],                        * */
/* * the order of the fit is specified by m.                       * */
/* * The standard deviation of the fit is returned in d.           * */
/* * There are two options available by use of the parameter e:    * */
/* *  1. if e = 0, the fit is to order m,                          * */
/* *  2. if e > 0, the order of fit increases towards m, but will  * */
/* *     stop if the relative standard deviation does not decrease * */
/* *     by more than e between successive fits.                   * */
/* * The order of the fit then obtained is l.                      * */
/* ***************************************************************** */
int ForsytheFit::_doFit(integer *m, doublereal *e1, integer *n, 
                        integer *l, doublereal *x, doublereal *y, doublereal *c__, doublereal 
                        *dd)
{
    /* System generated locals */
    integer i__1;
    doublereal d__1;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static doublereal a[25], b[25], d__[25], e[25], f[25];
    static integer i__;
    static doublereal v[25], w, a1, a2, b1, c2[25], b2, c1, d1, f1, f2;
    static integer l2, n1;
    static doublereal v1, v2;
    static real vv;

/*     Labels: 10,15,20,30,50 */
    /* Parameter adjustments */
    --c__;
    --y;
    --x;

    /* Function Body */
    n1 = *m + 1;
    *l = 0;
    v1 = 1e7;
/*     Initialize the arrays */
    i__1 = n1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	a[i__ - 1] = 0.;
	b[i__ - 1] = 0.;
	f[i__ - 1] = 0.;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	v[i__ - 1] = 0.;
	d__[i__ - 1] = 0.;
    }
    d1 = sqrt((doublereal) (*n));
    w = d1;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	e[i__ - 1] = 1. / w;
    }
    f1 = d1;
    a1 = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	a1 += x[i__] * e[i__ - 1] * e[i__ - 1];
    }
    c1 = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c1 += y[i__] * e[i__ - 1];
    }
    b[0] = 1. / f1;
    f[0] = b[0] * c1;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	v[i__ - 1] += e[i__ - 1] * c1;
    }
    *m = 1;
/*     Save latest results */
L10:
    i__1 = *l;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c2[i__ - 1] = c__[i__];
    }
    l2 = *l;
    v2 = v1;
    f2 = f1;
    a2 = a1;
    f1 = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	b1 = e[i__ - 1];
	e[i__ - 1] = (x[i__] - a2) * e[i__ - 1] - f2 * d__[i__ - 1];
	d__[i__ - 1] = b1;
	f1 += e[i__ - 1] * e[i__ - 1];
    }
    f1 = sqrt(f1);
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	e[i__ - 1] /= f1;
    }
    a1 = 0.;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c1 += e[i__ - 1] * y[i__];
    }
    ++(*m);
    i__ = 0;
L15:
    *l = *m - i__;
    b2 = b[*l - 1];
    d1 = 0.;
    if (*l > 1) {
	d1 = b[*l - 2];
    }
    d1 = d1 - a2 * b[*l - 1] - f2 * a[*l - 1];
    b[*l - 1] = d1 / f1;
    a[*l - 1] = b2;
    ++i__;
    if (i__ != *m) {
	goto L15;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	v[i__ - 1] += e[i__ - 1] * c1;
    }
    i__1 = n1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	f[i__ - 1] += b[i__ - 1] * c1;
	c__[i__] = f[i__ - 1];
    }
    vv = 0.f;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	vv += (v[i__ - 1] - y[i__]) * (v[i__ - 1] - y[i__]);
    }
/*     Note the division is by the number of degrees of freedom */
    vv = sqrt(vv / (doublereal) (*n - *l - 1));
    *l = *m;
    if (*e1 == 0.) {
	goto L20;
    }
/*     Test for minimal improvement */
    if ((d__1 = v1 - vv, abs(d__1)) / vv < *e1) {
	goto L50;
    }
/*     if error is larger, quit */
    if (*e1 * vv > *e1 * v1) {
	goto L50;
    }
    v1 = vv;
L20:
    if (*m == n1) {
	goto L30;
    }
    goto L10;
/*     Shift the c[i] down, so c(0) is the constant term */
L30:
    i__1 = *l;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c__[i__ - 1] = c__[i__];
    }
    c__[*l] = 0.;
/*     l is the order of the polynomial fitted */
    --(*l);
    *dd = vv;
    return 0;
/*     Aborted sequence, recover last values */
L50:
    *l = l2;
    vv = v2;
    i__1 = *l;
    for (i__ = 1; i__ <= i__1; ++i__) {
	c__[i__] = c2[i__ - 1];
    }
    goto L30;
} /* ls_poly__ */

/* Main program alias */ int demo_ls_poly__ () { MAIN__ (); return 0; }
#ifdef JUNK

////////////////////////////////////////////////////
// perform a fit

int ForsytheFit::performFit()
  
{

  // allocate arrays
  
  _allocDataArrays();

  // check conditions

  if (_nObs < 2 * (_order + 2)) {
    cerr << "ERROR - ForsytheFit::performFit()" << endl;
    cerr << "  Not enough observations to  fit order: " << _order << endl;
    cerr << "  Min n obs: " << 2 * (_order + 2) << endl;
    return -1;
  }
  
  // do the fit

  int m = _order;
  int m1 = _order + 1;
  int m2 = _order + 2;

  for (int k = 1; k <= m2; k++) {
    _Xc[k] = 0.0;
    for (size_t i = 0; i < _nObs; i++) {
      _Xc[k] += _intPower(_xObs[i], k);
    }
  }

  double yc = 0.0;
  for (size_t i = 0; i < _nObs; i++) {
    yc += _yObs[i];
  }
  
  for (int k = 1; k <= m; k++) {
    _Yx[k]=0.0;
    for (size_t i = 0; i < _nObs; i++) {
      _Yx[k] += _yObs[i] * _intPower(_xObs[i], k);
    } // i
  } // k
    
  for (int i = 1; i <= m1; i++) {
    for (int j = 1; j <= m1; j++) {
      int ij = i + j - 2;
      if (i == 1 && j == 1) {
        _CC[1][1]= _nObs;
      } else {
        _CC[i][j] = _Xc[ij];
      }
    } // j
  } // i

  _BB[1] = yc;
  for (int i = 2; i <= m1; i++) {
    _BB[i] = _Yx[i-1];
  } // i
    
  for (int k = 1; k <= m; k++) {
    for (int i = k+1; i <= m1; i++) {
      _BB[i] -= _CC[i][k]/_CC[k][k]*_BB[k];
      for (int j = k+1; j <= m1; j++) {
        _CC[i][j] -= _CC[i][k] / _CC[k][k] * _CC[k][j];
      }
    }
  } // k

  _AA[m1] = _BB[m1] / _CC[m1][m1];
  for (int i = m; i > 0; i--)  {
    double s = 0.0;
    for (int k= i+1; k <= m1; k++) {
      s = s + _CC[i][k] * _AA[k];
    }
    _AA[i] = (_BB[i] - s) / _CC[i][i];
  } // i

  // printf("\n Polynomial approximation of degree %d (%ld points)\n", m, _nObs);
  // printf(" Coefficients of polynomial:\n");
  // for (int i = 1; i <= m1; i++) {
  //   printf("  A(%d) = %15.9f\n", i-1, _AA[i]);
  // }
  // printf("\n Approximated function:\n");
  // printf("        X           Y\n");
  // for (size_t i = 0; i < _nObs; i++) {
  //   double xx = _xObs[i];
  //   double p = 0;
  //   for (int k = 1; k <= m1; k++) {
  //     p = p * xx + _AA[m1+1-k];
  //   }
  //   printf(" %11.6f %11.6f\n", xx, p);
  // }
  // printf("\n\n");

  // save the coefficients
  
  _coeffs.clear();
  for (size_t ii = 0; ii < _order + 1; ii++) {
    _coeffs.push_back(_AA[ii+1]);
  }
  
  return 0;

}

#endif
