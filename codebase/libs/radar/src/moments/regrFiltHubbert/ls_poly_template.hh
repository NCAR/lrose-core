/****************************************************************
*         LEAST SQUARES POLYNOMIAL FITTING SUBROUTINE           *
* ------------------------------------------------------------- *
* This program least squares fits a polynomial to input data.   *
* forsythe orthogonal polynomials are used in the fitting.      *
* The number of data points is n.                               *
* The data is input to the subroutine in x[i], y[i] pairs.      *
* The coefficients are returned in c[i],                        *
* the smoothed data is returned in v[i],                        *
* the order of the fit is specified by m.                       *
* The standard deviation of the fit is returned in d.           *
* There are two options available by use of the parameter e:    *
*  1. if e = 0, the fit is to order m,                          *
*  2. if e > 0, the order of fit increases towards m, but will  *
*     stop if the relative standard deviation does not decrease *
*     by more than e between successive fits.                   *
* The order of the fit then obtained is l.                      *
****************************************************************/

/***********************************************************
 * 'templatized' C++ version with loops instead of goto's
 * by M.Toussaint GAMIC GmbH Germany
 *
 * Index starts at 0 instead of 1 as in the original
 * fortran code
 **********************************************************/
template<typename T>
std::vector<T>
LS_POLY( int m, const std::vector<T> &x,
         const std::vector<T> &y,
         T &dd, std::vector<T> &v )
{
  std::vector<T> a, b, c, d, e, f, c2;

  int n = x.size();

  //Labels: e10,e15,e20,e30,e50,fin;
  int i,l2,n1;
  T a1,a2,b1,b2,c1,d1,f1,f2,v1,v2,w;
  n1 = m + 1;
  v1 = 1e7;

  a.resize(n1);
  b.resize(n1);
  c.resize(n1);
  f.resize(n1);
  c2.resize(n1);

  //v.resize(n);
  d.resize(n);
  e.resize(n);

  // Initialize the arrays
  for (i = 0; i < n1; i++) a[i] = b[i] = f[i] = 0;
  for (i = 0; i < n; i++)  v[i] = d[i] = 0;
  d1 = sqrt(n);
  w = d1;
  for (i = 0; i < n; i++) e[i] = 1.0 / w;
  f1 = d1;
  a1 = 0;
  for (i = 0; i < n; i++) a1 = a1 + x[i] * e[i] * e[i];
  c1 = 0;
  for (i = 0; i < n; i++) c1 = c1 + y[i] * e[i];

  b[0] = 1 / f1;
  f[0] = b[0] * c1;
  for (i = 0; i < n; i++) v[i] = v[i] + e[i] * c1;
  m = 1;

  bool aborted = false;

  do {
    for (i = 0; i < l; i++)  c2[i] = c[i];

    l2 = l;
    v2 = v1;
    f2 = f1;
    a2 = a1;
    f1 = 0;

    for (i = 0; i < n; i++) {
      b1 = e[i];
      e[i] = (x[i] - a2) * e[i] - f2 * d[i];
      d[i] = b1;
      f1 = f1 + e[i] * e[i];
    }

    f1 = sqrt(f1);
    for (i = 0; i < n; i++)  e[i] = e[i] / f1;
    a1 = 0;
    for (i = 0; i < n; i++)  a1 = a1 + x[i] * e[i] * e[i];
    c1 = 0;
    for (i = 0; i < n; i++)  c1 = c1 + e[i] * y[i];
    m = m + 1;
    i = 0;

    do {
      l = m - i;
      b2 = b[l-1];
      d1 = 0;
      if (l > 1) d1 = b[l - 2];
      d1 = d1 - a2 * b[l-1] - f2 * a[l-1];
      b[l-1] = d1 / f1;
      a[l-1] = b2;
      i = i + 1;
    } while (i!=m);

    for (i = 0; i < n; i++)  v[i] = v[i] + e[i] * c1;
    for (i = 0; i < n1; i++) {
      f[i] = f[i] + b[i] * c1;
      c[i] = f[i];
    }
    vv = 0;
    for (i = 0; i < n; i++) {
      vv = vv + (v[i] - y[i]) * (v[i] - y[i]);
    }

    //Note the division is by the number of degrees of freedom
    vv = sqrt(vv / (n - l));
    l = m;
    if (e1 != 0) {
      //Test for minimal improvement
      if (fabs(v1 - vv) / vv < e1) {
        aborted = true;
        break;
      }
      //if error is larger, quit
      if (e1 * vv > e1 * v1) {
        aborted = true;
        break;
      }
      v1 = vv;
    }

  } while (m != n1);

  if (aborted) {
    l = l2;
    vv = v2;
    for (i = 0; i < l+1; i++)  c[i] = c2[i];
  }
  l = l - 1;
  dd = vv;

  c.resize(l+1);

  return c;
}
