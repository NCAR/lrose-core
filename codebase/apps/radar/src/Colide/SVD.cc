#include <cmath>
#include "SVD.hh"
#include <euclid/Box.hh>
#include <euclid/Line.hh>
#include <rapmath/usvd.h>
#include <rapmath/RMmalloc.h>

// using namespace std;

/*----------------------------------------------------------------*/
SVD::SVD(const PointList &l) : PointList(l)
{
}

/*----------------------------------------------------------------*/
SVD::~SVD()
{
}

/*----------------------------------------------------------------*/
bool SVD::svd(double &slope, double &intercept) const
{
  double **a, **v, **u, *data;
  double w[2], c[2], max, min, condition, x, y, missing;
  int n, i;
  bool status;
  // cldMem *data;

  status = false;
  if ((n=size()) <= 0)
    return true;

  //a = NRmatrix(n, 2);
  //v = NRmatrix(2, 2);
  a = (double **)RMcalloc2(n, 2, sizeof(double));
  u = (double **)RMcalloc2(n, 2, sizeof(double));
  v = (double **)RMcalloc2(2, 2, sizeof(double));
    
  missing = -99.99;
  data = new double[n];

  for (i=0; i<n; ++i)
  {
    data[i] = missing;
    x = ithX(i);
    y = ithY(i);
    a[i][0] = x;
    a[i][1] = 1.0;
    data[i] = y;
  }

  if (usvd(a, n, 2, u, v, w) < 0)
  {
    printf("ERROR in NRsvdcmp\n");
    goto error;
  }

  if (w[0] > w[1])
  {
    max = w[0];
    min = w[1];
  }
  else
  {
    max = w[1];
    min = w[0];
  }
  if ( min < 1e-9 && min > -1e-9 )
  {
    printf("Bad condition # from SVD terms:%f %f\n",  w[0], w[1]);
    goto error;
  }
  else
  {
    condition = max/min;
    if (condition > 1e4)
    {
      printf("Bad condition number %f   from %f, %f\n",
	     condition, w[0], w[1]);
      goto error;
    }
  }

  usvd_apply(u, w, v, n, 2, data, c);
  slope = c[0];
  intercept = c[1];
  status = true;
 error:
  RMfree2((void **)a);
  RMfree2((void **)v);
  delete [] data;
  return status;
}

/*----------------------------------------------------------------*/
bool SVD::lsq_min(const Grid2d &g, Line &l) const
{
  double mux, muy, sxx, sxy, syy;
  Box b;
  int len;
  bool is_data;

  len = size();
  if (len <= 0)
  {
    return false;
  }

  /*
   * Find min and max values, and build up mu
   */
  if (!_lsq_min_init(len, g, mux, muy, b))
  {
    is_data = false;
    if (!_lsq_min_init(len, mux, muy, b))
      return false;
  }
  else
    is_data = true;

  /*
   * Build up 2nd moments
   */
  if (is_data)
  {
    if (!_lsq_min_second_mom(len, g, mux, muy, sxx, sxy, syy))
      return false;
  }
  else
  {
    if (!_lsq_min_second_mom(len, mux, muy, sxx, sxy, syy))
      return false;
  }
  return _lsq_compute(sxx, sxy, syy, mux, muy, b, l);
}

/*----------------------------------------------------------------*/
bool SVD::lsq_min(const Grid2d &mask, const Grid2d &seed, const Grid2d &data,
		  Line &l) const
{
  double mux, muy, sxx, sxy, syy;
  Box b;
  int len;
  bool is_data;

  len = size();
  if (len <= 0)
    return false;

  /*
   * Find min and max values, and build up mu
   */
  if (!_lsq_min_init(len, mask, seed, data, mux, muy, b))
  {
    is_data = false;
    if (!_lsq_min_init(len, mux, muy, b))
      return false;
  }
  else
    is_data = true;

  /*
   * Build up 2nd moments
   */
  if (is_data)
  {
    if (!_lsq_min_second_mom(len, mask, seed, data, mux, muy, sxx, sxy, syy))
      return false;
  }
  else
  {
    if (!_lsq_min_second_mom(len, mux, muy, sxx, sxy, syy))
      return false;
  }
  return _lsq_compute(sxx, sxy, syy, mux, muy, b, l);
}

/*----------------------------------------------------------------*/
bool SVD::lsq_min(const Grid2d &data, const Grid2d &lineData, Line &l) const
{
  double mux, muy, sxx, sxy, syy;
  Box b;
  int len;
  bool is_data;

  len = size();
  if (len <= 0)
  {
    // 	printf("No data in lsq_min\n");
    return false;
  }

  /*
   * Find min and max values, and build up mu
   */
  if (!_lsq_min_init(len, data, lineData, mux, muy, b))
  {
    is_data = false;
    if (!_lsq_min_init(len, mux, muy, b))
      return false;
  }
  else
    is_data = true;

  /*
   * Build up 2nd moments
   */
  if (is_data)
  {
    if (!_lsq_min_second_mom(len, data, lineData, mux, muy, sxx, sxy, syy))
      return false;
  }
  else
  {
    if (!_lsq_min_second_mom(len, mux, muy, sxx, sxy, syy))
      return false;
  }
  return _lsq_compute(sxx, sxy, syy, mux, muy, b, l);
}

/*----------------------------------------------------------------*/
bool SVD::lsq_min(Line &l) const
{
  double mux, muy, sxx, sxy, syy;
  Box b;
  int len;
    
  len = size();
  if (len <= 0)
    return false;

  /*
   * Find min and max values, and build up mu
   */
  if (!_lsq_min_init(len, mux, muy, b))
    return false;

  /*
   * Build up 2nd moments
   */
  if (!_lsq_min_second_mom(len, mux, muy, sxx, sxy, syy))
    return false;
    
  return _lsq_compute(sxx, sxy, syy, mux, muy, b, l);
}

/*----------------------------------------------------------------*/
bool SVD::_lsq_min_init(int len, const Grid2d &data,
			double &mux, double &muy, Box &b) const
{
  int i;
  double w, x, y;
    
  mux = muy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    double wi;
    if (data.getValue((int)x, (int)y, wi))
    {
      (mux) += x*wi;
      (muy) += y*wi;
      w += wi;
    }
  }
  if (fabs(w) > 1.0e-10)
  {
    mux = mux/w;
    muy = muy/w;
    b = extrema();
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
bool SVD::_lsq_min_init(int len, const Grid2d &mask, const Grid2d &seed,
			const Grid2d &data,
			double &mux, double &muy, Box &b) const
{
  int i;
  double w, x, y;
    
  mux = muy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    if (mask.isMissing((int)x, (int)y))
      continue;
    double wi=0.0;
    if (data.getValue((int)x, (int)y), wi)
    {
      mux += x*wi;
      muy += y*wi;
      w += wi;
    }
  }
  if (fabs(w) > 1.0e-10)
  {
    mux = mux/w;
    muy = muy/w;
    b =  extrema();
    return true;
  }
  else
    return false;
}


/*----------------------------------------------------------------*/
bool SVD::_lsq_min_init(int len, const Grid2d &data, const Grid2d &lineData,
			double &mux, double &muy, Box &b) const
{
  int i;
  double w, x, y;
    
  mux = muy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    if (lineData.isMissing((int)x, (int)y))
      continue;
    double wi;
    if (data.getValue((int)x, (int)y, wi))
    {
      // supposed to be max leaf
      mux += x*wi;
      muy += y*wi;
      w += wi;
    }
  }
  if (fabs(w) > 1.0e-10)
  {
    mux = mux/w;
    muy = muy/w;
    b = extrema();
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
bool SVD::_lsq_min_second_mom(int len, const Grid2d &data,
			      double mux, double muy, double &sxx,
			      double &sxy, double &syy) const
{
  int i;
  double w, x, y;
    
  sxx = sxy = syy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    double wi;
    if (data.getValue((int)x, (int)y, wi))
    {
      sxx += (x-mux)*(x-mux)*wi;
      sxy += (x-mux)*(y-muy)*wi;
      syy += (y-muy)*(y-muy)*wi;
      w += wi;
    }
  }
  if (w >= 1.0e-6)
  {
    sxx = sxx/w;
    sxy = sxy/w;
    syy = syy/w;
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
bool SVD::_lsq_min_second_mom(int len, const Grid2d &mask, const Grid2d &seed,
			      const Grid2d &data, double mux, double muy,
			      double &sxx, double &sxy, double &syy) const
{
  int i;
  double w, x, y;
    
  sxx = sxy = syy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    if (mask.isMissing((int)x, (int)y))
      continue;
    double wi;
    if (data.getValue((int)x, (int)y, wi))
    {
      sxx += (x-mux)*(x-mux)*wi;
      sxy += (x-mux)*(y-muy)*wi;
      syy += (y-muy)*(y-muy)*wi;
      w += wi;
    }
  }
  if (w >= 1.0e-6)
  {
    sxx = sxx/w;
    sxy = sxy/w;
    syy = syy/w;
    return true;
  }
  else
    return false;
}


/*----------------------------------------------------------------*/
bool SVD::_lsq_min_second_mom(int len, const Grid2d &data, 
			      const Grid2d &lineData,
			      double mux, double muy, double &sxx,
			      double &sxy, double &syy) const
{
  int i;
  double w, x, y;
    
  sxx = sxy = syy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    if (lineData.isMissing((int)x, (int)y))
      continue;
    // if (rules.max_leaf_true_data_value((int)x, (int)y, wi))
    double wi;
    if (data.getValue((int)x, (int)y, wi))
    {
      sxx += (x-mux)*(x-mux)*wi;
      sxy += (x-mux)*(y-muy)*wi;
      syy += (y-muy)*(y-muy)*wi;
      w += wi;
    }
  }
  if (w >= 1.0e-6)
  {
    sxx = sxx/w;
    sxy = sxy/w;
    syy = syy/w;
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
bool SVD::_lsq_min_init(int len, double &mux, double &muy, 
			Box &b) const
{
  int i;
  double w, x, y;
    
  mux = muy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    mux += x;
    muy += y;
    w += 1.0;
  }
  if (fabs(w) > 1.0e-10)
  {
    mux = mux/w;
    muy = muy/w;
    b = extrema();
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
bool SVD::_lsq_min_second_mom(int len, double mux, double muy,
			      double &sxx, double &sxy, double &syy) const
{
  int i;
  double w, x, y;

  sxx = sxy = syy = 0.0;
  w = 0.0;
  for (i=0; i<len; ++i)
  {
    x = ithX(i);
    y = ithY(i);
    sxx += (x - mux)*(x - mux);
    sxy += (x - mux)*(y - muy);
    syy += (y - muy)*(y - muy);
    w += 1.0;
  }
  if (w >= 1.0e-6)
  {
    sxx = sxx/w;
    sxy = sxy/w;
    syy = syy/w;
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
bool SVD::_lsq_compute(double sxx, double sxy, double syy,
		       double mux, double muy, Box &b,
		       Line &l) const
{
  double rho, x, y;
    
  if (size() <= 0)
    return false;

  x = ithX(0);
  y = ithY(0);
  if (fabs(sxx) < 1.0e-10)
  {
    /*
     * Horizontal line
     */
    l = Line(y, b, false);
  }
  else if (fabs(syy) < 1.0e-10)
  {
    /*
     * Vertical line
     */
    l = Line(x, b, true);
  }
  else
  {
    if (fabs(sxy) < 1.0e-10)
    {
      // cannot do this computation
      return false;
    }

    rho = sxy/sqrt(sxx*syy);
    if (fabs(rho) < 0.05)  /* correlation tolerance */
    {
      double B, s, t1, t2, v1, v2, t;

      B = (syy - sxx)/sxy;
      s = sqrt(B*B + 4.0);
      t1 = (B + s)/2.0;
      t2 = (B - s)/2.0;
      v1 = (1.0/(1.0+t1*t1))*(sxx*t1*t1 - 2.0*sxy*t1 + syy);
      v2 = (1.0/(1.0+t2*t2))*(sxx*t2*t2 - 2.0*sxy*t2 + syy);
      if (v1 < v2)
	t = t1;
      else
	t = t2;
      /*
       * We know it has slope t and passes through mux, muy
       */
      l = Line(t, mux, muy, b);
    }
    else
    {
      if (sxx >= syy)
	/*
	 * Horizontal
	 */
	l = Line(muy, b, false);
      else
	/*
	 * Vertical
	 */
	l = Line(mux, b, true);
    }
  }
    
  return !(l.isBad() || l.isAPoint());
}
