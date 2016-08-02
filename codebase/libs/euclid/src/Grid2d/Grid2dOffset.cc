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
#include <toolsa/copyright.h>

#include <euclid/Grid2dOffset.hh>
#include <euclid/Line.hh>
#include <rapmath/OrderedList.hh>
#include <cstdio>
#include <cmath>
using std::vector;

/*----------------------------------------------------------------*/
static int _gridIndex(const int tx, const int ty, const int nx)
{
  return ty*nx + tx;
}

/*----------------------------------------------------------------*/
static void _rotatePoint(double *x, double *y, double angle)
{
  double a, cosa, sina, x0, y0;
    
  a = angle*3.14159/180.0;
  cosa = cos(a);
  sina = sin(a);
  x0 = (*x)*cosa + (*y)*sina;
  y0 = -(*x)*sina + (*y)*cosa;
  *x = x0;
  *y = y0;
}


/*----------------------------------------------------------------*/
Grid2dOffset::Grid2dOffset()
{
  _angle = 0.0;
  _max = 0;
  _missing = 0;
}

/*----------------------------------------------------------------*/
Grid2dOffset::Grid2dOffset(const double imissing)
{
  _angle = 0.0;
  _max = 0;
  _missing = imissing;
}

/*----------------------------------------------------------------*/
Grid2dOffset::Grid2dOffset(const double length, const double width, 
		       const double ang, const double step,  
		       const int nx, const double imissing)
{
  _angle = ang;
  _max = 0;
  _missing = imissing;
  _add(length, width, ang, step, nx);
}

/*----------------------------------------------------------------*/
Grid2dOffset::Grid2dOffset(const double radius,
		       const double ang, // rotation angle
		       const int nx, // image width (x npixels)
		       const double imissing /* missing value */
		       )
{
  _angle = ang;
  _max = 0;
  _missing = imissing;
  _initForBox(ang, 2*radius+1.0, radius, nx);
}

/*----------------------------------------------------------------*/
Grid2dOffset::Grid2dOffset(double length, double width, double ang,
			   int nx,  double missing)
{
  _angle = ang;
  _max = 0;
  _missing = missing;
  _initForBox(ang, length, width, nx);
}

/*----------------------------------------------------------------*/
// special constructor for 'octogon' around a point (straight from the
// mind of Jing).
Grid2dOffset::Grid2dOffset(double radius, int nx, double imissing)
{
  int i, np;
  double ang;
    
  _angle = 0.0;
  _max = 0;
  _missing = imissing;

  np = (int)(radius*.38*2/1.3 + .5);	/* sin 22.5 degrees, 
					   pixel distance = 1.3 */
  if (np < 1)
    np = 1;

  /*
   * For each of the 8 octogon directions
   */
  for (i = 0; i < 8; i++)
  {
    /*
     * Get the template for radius, angle ang, 1.3 (mysterious
     * pixel distance) into neighbor region
     */
    ang = 45. * (double)i;
    _add(1.3, radius, ang, np, nx);
  }
}

/*----------------------------------------------------------------*/
Grid2dOffset::~Grid2dOffset()
{
}
    
/*----------------------------------------------------------------*/
int Grid2dOffset::numFlagged(const Grid2d &g, const int x, const int y) const
{
  vector<int>::const_iterator it;
  int cnt;
    
  cnt = 0;
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (!g.getValueAtOffset(x, y, *it, pp))
    {
      (cnt)++;
    }
  }
  return cnt;
}

/*----------------------------------------------------------------*/
int Grid2dOffset::numNonFlagged(const Grid2d &g, const int x,
				const int y) const
{
  vector<int>::const_iterator it;
  int cnt;
    
  cnt = 0;
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      (cnt)++;
    }
  }
  return cnt;
}

/*----------------------------------------------------------------*/
bool Grid2dOffset::average(const Grid2d &g, const int x, const int y,
			   double &v) const
{
  double cnt;
    
  cnt = num();
  if (cnt == 0)
    return false;

  if (!sumValues(g, x, y, v))
    return false;
  else
  {
    v /= cnt;
    return true;
  }
}

/*----------------------------------------------------------------*/
double Grid2dOffset::averageMissingZero(const Grid2d &g, const int x,
					const int y) const
{
  double cnt;
    
  cnt = num();
  if (cnt == 0)
  {
    return 0.0;
  }

  double v;
  if (!sumValues(g, x, y, v))
  {
    return 0.0;
  }
  else
  {
    return v /= cnt;
  }
}

/*----------------------------------------------------------------*/
double Grid2dOffset::percentBad(const Grid2d &g, const int x,
				const int y) const
{
  int n, ng;
  double d;
    
  n = num();
  if (n <= 0)
  {
    // there is no data
    return 0.0;
  }
    
  ng = numFlagged(g, x, y);
  d = (double)ng / (double)n;
  return d;
}

/*----------------------------------------------------------------*/
vector<double> Grid2dOffset::fillPixels(const Grid2d &G,
					const int x,
					const int y) const
{
  vector<double> ret;
  vector<int>::const_iterator it;
    
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    int ipt = G.ipt(x,y) + *it;
    double pp = G.getValue(ipt);
    // if (G.getValueAtOffset(x, y, *it, pp))
    // {
    ret.push_back(pp);
    // }
  }
  return ret;
}

/*----------------------------------------------------------------*/
bool Grid2dOffset::sumValues(const Grid2d &g, const int x,
			     const int y, double &v) const
{
  vector<int>::const_iterator it;
  bool first = true;
    
  v = 0;
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      first = false;
      v += pp;
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
bool Grid2dOffset::maxValue(const Grid2d &g, const int x,
			    const int y, double &v) const
{
  vector<int>::const_iterator it;

  v = 0;
  bool first = true;
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      if (first)
      {
	first = false;
	v = pp;
      }
      else
      {
	if (pp > v)
	  v = pp;
      }
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
double Grid2dOffset::maxValueOrZero(const Grid2d &g, const int x,
				    const int y) const
{
  vector<int>::const_iterator it;

  double v = 0;
  bool first = true;
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      if (first)
      {
	first = false;
	v = pp;
      }
      else
      {
	if (pp > v)
	  v = pp;
      }
    }
  }
  return v;
}

/*----------------------------------------------------------------*/
// variance, bad values set to mv
bool Grid2dOffset::variance(const Grid2d &g, const int x,
			    const int y, double &v) const
{
  double a;
    
  if (!average(g, x, y, a))
    return false;
    
  // at each point compute square diff.
  return sumSqOffsets(g, x, y, a, v);
}


/*----------------------------------------------------------------*/
bool Grid2dOffset::sumSqOffsets(const Grid2d &g, const int x,
				const int y, const double a, double &m) const
{
  vector<int>::const_iterator it;
  double d;
    
  m = 0.0;
  bool first = true;
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      first = false;
      d = pp - a;
      m += d*d;
    }
  }
  return !first;
}
  
/*----------------------------------------------------------------*/
// return median of non-flag pixel values at offsets
bool Grid2dOffset::median(const Grid2d &g, const int x, const int y,
			double &m) const
{
  vector<int>::const_iterator it;
  OrderedList l;

  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      l.addToListUnordered(pp);
    }
  }
  if (l.num() == 0)
  {
    return false;
  }
  else
  {
    l.order();
    m = l.percentile(0.5);
    return true;
  }
}

/*----------------------------------------------------------------*/
// return median of non-flag pixel values at offsets
bool Grid2dOffset::median(const Grid2d &g, const int x, const int y,
			const double thresh, const double background,
			double &v) const
{
  OrderedList l;
  vector<int>::const_iterator it;
  bool non_flag = false;

  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      non_flag = true;
      if (pp >= thresh)
      {
	l.addToListUnordered(pp);
      }
    }
  }
  if (l.num() == 0)
  {
    if (non_flag)
    {
      v = background;
      return true;
    }
    else
      return false;
  }
  else
  {
    l.order();
    v = l.percentile(0.5);
    return true;
  }
}

/*----------------------------------------------------------------*/
// variance, bad values set to mv
bool Grid2dOffset::variance(const Grid2d &g, const int x, const int y,
			  const double thresh, const double  background,
			  double &v) const
{
  vector<int>::const_iterator it;
  double m, n, vsum;

  if (!average(g, x, y, thresh, background, m))
    return false;

  if (m == background)
    v = background;
  else if (m == 0)
    v = 0;
  else
  {
    n = vsum = 0;
    for (it=_offsets.begin(); it!=_offsets.end(); it++)
    {
      double pp;
      if (g.getValueAtOffset(x, y, *it, pp))
      {
	if (pp >= thresh)
	{
	  ++n;
	  vsum += (pp - m)*(pp - m);
	}
      }
    }
    if (n != 0.0)
      v = vsum/n;
    else
      v = 0.0;
  }
  return true;
}

/*----------------------------------------------------------------*/
bool Grid2dOffset::halfAverage(const Grid2d &g, const int x, const int y,
			       const bool bottom_half, double &v) const
{
  int h, i0, i1, n;
  double c, cn;
  int it, p;
  n = (int)_offsets.size();
  h = n/2;
  if (bottom_half)
  {
    i0 = 0;
    i1 = h;
  }
  else
  {
    i0 = h;
    i1 = n;
  }
    
  c = cn = 0.0;
  for (p=i0; p<i1; p++)
  {
    if (p >= n)
      break;
    it = _offsets[p];
    double pp;
    if (g.getValueAtOffset(x, y, it, pp))
    { 
      c += pp;
      cn++;
    }
  }
  if (cn > 0)
  {
    c = c/cn;
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
double Grid2dOffset::halfAverageMissingZero(const Grid2d &g, const int x,
					    const int y,
					    const bool bottomHalf) const
{
  int h, i0, i1, n;
  double c, cn;
  int it, p;
  n = (int)_offsets.size();
  h = n/2;
  if (bottomHalf)
  {
    i0 = 0;
    i1 = h;
  }
  else
  {
    i0 = h;
    i1 = n;
  }
    
  c = cn = 0.0;
  for (p=i0; p<i1; p++)
  {
    if (p >= n)
      break;
    it = _offsets[p];
    double pp;
    if (g.getValueAtOffset(x, y, it, pp))
    {
      c += pp;
    }
    cn++;
  }
  if (cn > 0)
  {
    c = c/cn;
    return c;
  }
  else
    return 0.0;
}

/*----------------------------------------------------------------*/
double Grid2dOffset::percentAbove(const Grid2d &g, const int x, const int y,
				  const double thresh) const
{
  vector<int>::const_iterator it;
  double total=0.0, above=0.0;

  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    ++total;
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      if (pp > thresh)
	++above;
    }
  }
  if (total > 0)
    return (above/total);
  else
    return 0.0;
}

/*----------------------------------------------------------------*/
double Grid2dOffset::percentBelow(const Grid2d &g, const int x, const int y,
				  const double thresh) const
{
  return 1.0 - percentAbove(g, x, y, thresh);
}

/*----------------------------------------------------------------*/
bool Grid2dOffset::average(const Grid2d &g, const int x, const int y, 
			   const double thresh, const double background,
			   double &v) const
{
  vector<int>::const_iterator it;
  double m, n;
  bool all_missing = true;
  m = 0;
  n = 0;
  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    double pp;
    if (g.getValueAtOffset(x, y, *it, pp))
    {
      all_missing = false;
      if (pp >= thresh)
      {
	++n;
	m += pp;
      }
    }
  }
  if (all_missing)
    return false;
  if (m == 0)
    v = background;
  if (n > 0)
    v = m/n;
  else
    v = 0.0;
  return true;
}

/*----------------------------------------------------------------*/
int Grid2dOffset::numMaskedExceeding(const Grid2d &data, const int x,
				     const int y, const double  minv, 
				     const Grid2d &mask) const
{
  vector<int>::const_iterator it;
  double count=0.0;

  for (it=_offsets.begin(); it!=_offsets.end(); it++)
  {
    int ind = data.ipt(x, y) + *it;
    if (ind >= 0 && ind < data.getNdata())
    {
      double pp;
      if (!data.getValue(ind, pp))
	continue;
      if (pp < minv)
	continue;
      if (mask.isMissing(ind))
	continue;
      ++count;
    }
  }
  return count;
}

/*----------------------------------------------------------------*/
void Grid2dOffset::_initForBox(const double ang, const double length,
			       const double width, const int image_width)
{
  int i, j;
  double x0r, x1r, y0r, y1r, xmin, ymin, xmax, ymax, langle;

  // start with a line through 0,0 with input angle.
  Line l(ang, length);

  // make this line horizontal to get a new coordinate system.
  l.makeHorizontal(langle, true);

  // in rotated coordinate system, get box endpoints
  l.point(0, x0r, y0r);
  l.point(1, x1r, y1r);
  y0r -= width/2.0;
  y1r += width/2.0;

  // rotate the 4 points back to non-horizontal and get extrema in x and y
  _rotatePointBack(x0r, y0r, -langle, xmin, ymin, xmax, ymax, true);
  _rotatePointBack(x1r, y0r, -langle, xmin, ymin, xmax, ymax, false);
  _rotatePointBack(x1r, y1r, -langle, xmin, ymin, xmax, ymax, false);
  _rotatePointBack(x0r, y1r, -langle, xmin, ymin, xmax, ymax, false);

  // loop through x and y extrema
  for (j=(int)ymin; j<=(int)ymax+1; ++j)
    for (i=(int)xmin; i<=(int)xmax+1; ++i)
      // see if i,j point is in the box when i,j is rotated.
      _updateOffsets(i, j, langle, x0r, x1r, y0r, y1r, image_width);
}

/*----------------------------------------------------------------*/
void Grid2dOffset::_add(const double length, const double width,
		      const double ang, const double step, 
		      const int image_width)
{
  double cs, sn, a;
  int i, v, npixels;

  // figure out number of pixels along the line of length.
  npixels = (int)((length/step) + 1.0);
  if (npixels < 2)
    npixels = 2;

  a = ang * 3.141592653589 / 180.;
  cs = cos(a);
  sn = sin(a);
  for (i = 0; i < npixels; i++)
  {
    v = _computeOffset(i, cs, sn, width, step, npixels, image_width);
    _offsets.push_back(v);
  }
}

/*----------------------------------------------------------------*/
int Grid2dOffset::_computeOffset(const int i, const double cs, const double sn,
				 const double width, const double step, 
				 const int npixels, const int image_width)
{
  double x, y, xt;
  int tx, ty, ret;
    
  // get offset x and y (notice that y coordinates is graphics (tp to bottom)
  x = width;
  y = (.5 * (double)(npixels - 1) - (double)i) * step;
    
  /* rotate ****COUNTERCLOCKWISE***  */
  // note a zero degree rotation gives a vertical (north/south) template
  xt = x;
  x = xt*cs - y*sn;
  y = xt*sn + y*cs;

  /* change to graphics (image) coordinates and round */
  // NOTE THIS MIGHT BE WRONG 
  if (x >= 0.)
    tx = (int)(x + .5);
  else
    tx = -(int)(-x + .5);
  if (y < 0.)
    ty = (int)(-y + .5);
  else
    ty = -(int)(y + .5);

  // return value is the image pointer offset thing to this rotated location.
  ret = _gridIndex(tx, ty, image_width);
  
  // adjust max value to biggest offset in x or y.
  if (tx < 0)
    tx = -tx;
  if (ty < 0)
    ty = -ty;
  if (ty > tx)
    tx = ty;
  if (tx > _max)
    _max = tx;
  return ret;
}

/*----------------------------------------------------------------*/
void Grid2dOffset::_rotatePointBack(const double x, const double y, 
				    const double ang,
				    double &xmin, double &ymin, 
				    double &xmax, double &ymax,
				    const bool first)
{
  double x0, y0;
    
  x0 = x;
  y0 = y;
  _rotatePoint(&x0, &y0, ang);
  if (first)
  {
    xmax = xmin = x0;
    ymax = ymin = y0;
  }
  else
  {
    if (x0 < xmin)
      xmin = x0;
    if (x0 > xmax)
      xmax = x0;
    if (y0 < ymin)
      ymin = y0;
    if (y0 > ymax)
      ymax = y0;
  }
}

/*----------------------------------------------------------------*/
void Grid2dOffset::_updateOffsets(const int ix, const int iy, const double ang, 
				  const double x0r, const double x1r, 
				  const double y0r, const double y1r,
				  const int width)
{
  double x, y;
  int o, xx, yy;
    
  // rotate point to rotated coordinates and see if rotated value is
  // in range or not
  x = ix;
  y = iy;
  _rotatePoint(&x, &y, ang);
  if (x >= x0r && x <= x1r && y >= y0r && y <= y1r)
  {
    o = _gridIndex(ix, iy, width);
    _offsets.push_back(o);

    // adjust max value to biggest offset in x or y.
    xx = ix;
    if (xx < 0)
      xx = -xx;
    yy = iy;
    if (yy < 0)
      yy = -yy;
    if (xx < yy)
      xx = yy;
    if (xx > _max)
      _max = xx;
  }
}

