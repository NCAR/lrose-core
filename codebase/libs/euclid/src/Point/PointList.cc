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

#include <euclid/PointList.hh>
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <euclid/Box.hh>
#include <rapmath/OrderedList.hh>
#include <rapmath/Math.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>
#include <cstring>
#include <cmath>

using std::vector;
using std::pair;  

/*----------------------------------------------------------------*/
static bool _lookaheadBad(int ix, int iy, double x, double y,
			  double r02, double r12, double angle, 
			  double dangle)
{
  double dx, dy, r2, a, d;
  dx = ix - x;
  dy = iy - y;
  r2 = dx*dx + dy*dy;
  if (r2 < r02 || r2 > r12)
    return true;
  a = atan2(dy, dx)*180.0/3.14159;
  d = Math::angleDiff(a, angle);
  return (d > dangle);
}


/*----------------------------------------------------------------*/
PointList::PointList()
{
  _points.clear();
  _nx = 0;
  _ny = 0;
}

/*----------------------------------------------------------------*/
PointList::PointList(const int nx, const int ny)
{
  _points.clear();
  _nx = nx;
  _ny = ny;
}

/*----------------------------------------------------------------*/
PointList::PointList(const int nx, const int ny, clump::Region_t &pts)
{
  for (size_t i=0; i<pts.size(); ++i)
  {
    _points.push_back(Point(pts[i].first, pts[i].second));
  }

  _nx = nx;
  _ny = ny;
}

/*----------------------------------------------------------------*/
// take image and extract all non-flag locations.
PointList::PointList(const Grid2d &img)
{
  fromGrid(img);
}

/*----------------------------------------------------------------*/
PointList::~PointList()
{
}

/*----------------------------------------------------------------*/
PointList::PointList(const PointList &l) : AttributesEuclid(l)
{
  _points = l._points;
  _nx = l._nx;
  _ny = l._ny;
}

/*----------------------------------------------------------------*/
PointList & PointList::operator=(const PointList &l)
{
  if (&l == this)
  {
    return *this;
  }

  _points = l._points;
  _nx = l._nx;
  _ny = l._ny;
  Attributes::operator=(l);
  return *this;
}

/*----------------------------------------------------------------*/
bool PointList::operator==(const PointList &l) const
{
  return (_points == l._points && _nx == l._nx && _ny == l._ny &&
	  Attributes::operator==(l));
}

/*----------------------------------------------------------------*/
std::string PointList::writeXml(const std::string &tag) const
{
  std::string ret = TaXml::writeStartTag(tag, 0);
  ret += Attributes::writeAttXml("PointListAttributes");
  for (size_t i=0; i<_points.size(); ++i)
  {
    ret += _points[i].writeXml("Point");
  }
  ret += TaXml::writeInt("PointsNx", 0, _nx);
  ret += TaXml::writeInt("PointsNy", 0, _ny);

  ret += TaXml::writeEndTag(tag, 0);
  return ret;
}

/*----------------------------------------------------------------*/
bool PointList::readXml(const std::string &xml, const std::string &tag)
{
  *this = PointList();

  string buf;
  if (TaXml::readString(xml, tag, buf))
  {
    LOG(ERROR) << "parsing tag in xml " << tag;
    return false;
  }
  return readXml(buf);
}
/*----------------------------------------------------------------*/
bool PointList::readXml(const std::string &xml)
{
  *this = PointList();

  if (!Attributes::readAttXml(xml, "PointListAttributes"))
  {
    return false;
  }

  vector<string> pts;
  if (TaXml::readStringArray(xml, "Point", pts))
  {
    // assume no points
  }
  else
  {
    for (size_t i=0; i<pts.size(); ++i)
    {
      Point p;
      if (!p.readXml(pts[i]))
      {
	return false;
      }
      else
      {
	_points.push_back(p);
      }
    }
  }

  if (TaXml::readInt(xml, "PointsNx", _nx))
  {
    LOG(ERROR) << "parsing PointsNx";
    return false;
  }
  if (TaXml::readInt(xml, "PointsNy", _ny))
  {
    LOG(ERROR) << "parsing PointsNy";
    return false;
  }
  return true;
}

/*----------------------------------------------------------------*/
double PointList::cumulativeLength(void) const
{
  double x0, y0, x1, y1, x, y, d;
    
  d = 0.0;
  if (_points.size() < 2)
    return 0.0;
  for (int i=0; i<(int)_points.size()-1; ++i)
  {
    int j = i+1;
    x0 = _points[i].getX();
    y0 = _points[i].getY();
    x1 = _points[j].getX();
    y1 = _points[j].getY();
    x = x1-x0;
    y = y1-y0;
    d += sqrt(x*x + y*y);
  }
  return d;
}

/*----------------------------------------------------------------*/
void PointList::fromGrid(const Grid2d &img)
{
  int x, y;
 
  _points.clear();
  _nx = img.getNx();
  _ny = img.getNy();
  for (x=0; x< _nx; ++x)
  {
    for (y=0; y<_ny; ++y)
    {
      if (!img.isMissing(x, y))
      {
	_points.push_back(Point((double)x, (double)y));
      }
    }
  }
}

/*----------------------------------------------------------------*/
void PointList::toGrid(Grid2d &img, double value) const
{
  int x, y;
  int nx = img.getNx();
  int ny = img.getNy();
  for (size_t i=0; i<_points.size(); ++i)
  {
    x = _points[i].getIntX();
    y = _points[i].getIntY();
    if (x >= 0 && x < nx && y >= 0 && y < ny)
    {
      img.setValue(x, y, value);
    }
  }
}

/*----------------------------------------------------------------*/
int PointList::numPointsAboveThresh(const Grid2d &img,
				    const double thresh) const
{
  int num = 0;
  for (size_t i=0; i<_points.size(); ++i)
  {
    double v;
    if (img.getValue(_points[i].getIntX(), _points[i].getIntY(), v))
    {
      if (v > thresh)
      {
	++num;
      }
    }
  }
  return num;
}

/*----------------------------------------------------------------*/
bool PointList::isCircular(double min_percent) const
{
  int i, j, count, count2;
  double x0, y0, x = 0.0, y = 0.0, r, r0, r_min, percent;

  if (_points.empty())
    return false;
    
  //Find centroid x0, y0.
  centerpoint(x0, y0);

  //Find radius r0 of smallest enclosing circle center at centroid
  r0 = _enclosingCircleRadiusSquared(x0, y0);
  r_min = sqrt(r0);

  // mask where this is.
  Grid2d temp("temp", _nx, _ny, -1);
  double mask_value = 100;
  toGrid(temp, mask_value);

  // Find which % of circle interior is covered  using temp.
  // Use radius to define bounds of search.
  count = count2 = 0;
  for (i=(int)(x0-r_min); i<= (int)(x0+r_min); ++i)
  {
    for (j=(int)(y0-r_min); j<= (int)(y0+r_min); ++j)
    {
      if (!temp.inRange(x, y))
	continue;
      x = (double)i - x0;
      y = (double)j - y0;
      r = (x*x + y*y);
      if (r <= r0)
      {
	++count2;
	if (!temp.isMissing(i, j))
	  ++count;
      }
    }
  }

  // count2 is the total set of points within the circle in image range.
  // count is the total set of points in circle in image range in region.
  if (count2 > 0)
    percent = (double)count/(double)count2;
  else
    percent = 0.0;
  return (percent >= min_percent);
}

/*----------------------------------------------------------------*/
void PointList::centerpoint(double &x, double &y) const
{
  double n;
  
  x = 0.0;
  y = 0.0;
  n = 0.0;

  for (size_t i=0; i<_points.size(); ++i)
  {
    ++n;
    x += _points[i].getX();
    y += _points[i].getY();
  }
  if (n > 0.0)
  {
    x /= n;
    y /= n;
  }
}

/*----------------------------------------------------------------*/
double PointList::xAverage(void) const
{
  if (_points.size()== 0)
    return 0.0;

  double xave = 0;

  for (size_t i=0; i<_points.size(); ++i)
  {
    xave += _points[i].getX();
  }
  xave /= (double)_points.size();
  return xave;
}

/*----------------------------------------------------------------*/
bool PointList::onIntList(int num, const int *values, double tolerance,
			  const Grid2d &grid) const
{
  for (size_t i=0; i<_points.size(); ++i)
  {
    double v;
    int ix = (int)_points[i].getX();
    int iy = (int)_points[i].getY();
    if (grid.getValue(ix, iy, v))
    {
      bool ok = false;
      for (int j=0; j<num; ++j)
      {
	if (fabs(v -(double)values[j]) < tolerance)
	{
	  ok = true;
	  break;
	}
      }
      if (!ok)
      {
	return false;
      }
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
double PointList::minX(void) const
{
  double x0 = 0;
  double x;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    x = _points[i].getX();
    if (first)
    {
      first = false;
      x0 = x;
    }
    else
    {
      if (x < x0)
	x0 = x;
    }
  }
  return x0;
}

/*----------------------------------------------------------------*/
double PointList::maxX(void) const
{
  double x0 = 0;
  double x;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    x = _points[i].getX();
    if (first)
    {
      first = false;
      x0 = x;
    }
    else
    {
      if (x > x0)
	x0 = x;
    }
  }
  return x0;
}

/*----------------------------------------------------------------*/
double PointList::minY(void) const
{
  double y0 = 0;
  double y;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    y = _points[i].getY();
    if (first)
    {
      first = false;
      y0 = y;
    }
    else
    {
      if (y < y0)
	y0 = y;
    }
  }
  return y0;
}

/*----------------------------------------------------------------*/
double PointList::maxY(void) const
{
  double y0 = 0;
  double y;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    y = _points[i].getY();
    if (first)
    {
      first = false;
      y0 = y;
    }
    else
    {
      if (y > y0)
	y0 = y;
    }
  }
  return y0;
}

/*----------------------------------------------------------------*/
bool PointList::yRangeOverX(double x0, double x1,
			    double &min, double &max) const
{
  double x, y;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    x = _points[i].getX();
    if (x < x0 || x > x1)
      continue;
    y = _points[i].getY();
    if (first)
    {
      first = false;
      min = max = y;
    }
    else
    {
      if (y < min)
	min = y;
      if (y > max)
	max = y;
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
bool PointList::xRangeOverY(double y0, double y1,
			    double &min, double &max) const
{
  double x, y;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    y = _points[i].getY();
    if (y < y0 || y > y1)
      continue;
    x = _points[i].getX();
    if (first)
    {
      first = false;
      min = max = x;
    }
    else
    {
      if (x < min)
	min = x;
      if (x > max)
	max = x;
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
Box PointList::extrema(void) const
{
  double x0=0.0, y0=0.0, x1=0.0, y1=0.0, x, y;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    x = _points[i].getX();
    y = _points[i].getY();
    if (first)
    {
      first = false;
      x0 = x1 = x;
      y0 = y1 = y;
    }
    else
    {
      if (x < x0)
	x0 = x;
      if (x > x1)
	x1 = x;
      if (y < y0)
	y0 = y;
      if (y > y1)
	y1 = y;
    }
  }
  Box ret;
  if (!first)
    ret = Box(x0, y0, x1, y1);
  return ret;
}

/*----------------------------------------------------------------*/
void PointList::print(FILE *fp) const
{
  fprintf(fp, "List:Points=(%d)", (int)_points.size());
  printAtt(fp);
  fprintf(fp, "\n");
  for (size_t i=0; i<_points.size(); ++i)
  {
    fprintf(fp, "XY=%6.2f,%6.2f ", _points[i].getX(),
	    _points[i].getY());
    _points[i].printAtt(fp);
    fprintf(fp, "\n");
  }
}

/*----------------------------------------------------------------*/
void PointList::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
void PointList::printAsciiPicture(bool &landscape) const
{
  double x0, y0, x1, y1;
  x0 = minX();
  y0 = minY();
  x1 = maxX();
  y1 = maxY();
  landscape = (y1-y0 < x1-x0);
  if (landscape)
    _printAsciiLandscape();
  else
    _printAsciiPortrait();
}

/*----------------------------------------------------------------*/
void PointList::rotate(double angle)
{
  for (size_t i=0; i<_points.size(); ++i)
  {
    double x = _points[i].getX();
    double y = _points[i].getY();
    Math::rotatePoint(x, y, angle);
    _points[i].set(x, y);
  }
}

/*----------------------------------------------------------------*/
void PointList::keepX(double x)
{
  std::vector<Point>::iterator i;

  for (i=_points.begin(); i != _points.end(); )
  {
    if (i->getX() == x)
      ++i;
    else
      i = _points.erase(i);
  }
}

/*----------------------------------------------------------------*/
void PointList::keepY(double y)
{
  std::vector<Point>::iterator i;

  for (i=_points.begin(); i != _points.end(); )
  {
    if (i->getY() == y)
      ++i;
    else
      i = _points.erase(i);
  }
}

/*----------------------------------------------------------------*/
PointList PointList::commonX(double x) const
{
  PointList ret;
  ret._nx = _nx;
  ret._ny = _ny;
  ret._points.clear();
  for (size_t i=0; i<_points.size(); ++i)
  {
    if (_points[i].getX() == x)
    {
      ret._points.push_back(_points[i]);
    }
  }
  return ret;
}

/*----------------------------------------------------------------*/
PointList PointList::commonY(double y) const
{
  PointList ret;
  ret._nx = _nx;
  ret._ny = _ny;
  ret._points.clear();
  for (size_t i=0; i<_points.size(); ++i)
  {
    if (_points[i].getY() == y)
    {
      ret._points.push_back(_points[i]);
    }
  }
  return ret;
}

/*----------------------------------------------------------------*/
void PointList::clearNonMasked(const Grid2d &img)
{
  int x, y;
  std::vector<Point>::iterator i;

  for (i=_points.begin(); i != _points.end(); )
  {
    x = i->getIntX();
    y = i->getIntY();
    if (img.isMissing(x, y))
      // not mask point..delete
      i = _points.erase(i);
    else
      // in mask, keep.
      ++i;
  }
}

/*----------------------------------------------------------------*/
void PointList::formXyUnion(const PointList &l)  
{
  Grid2d i("temp", _nx, _ny, -1);

  // put this to an image.
  toGrid(i, 100.0);

  // add in l.
  l.toGrid(i, 100.0);
    
  // clear out the local list.
  _points.clear();
    
  // pull back into a list.
  for (int y=0; y<_ny; ++y)
  {
    for (int x=0; x<_nx; ++x)
    {
      if (!i.isMissing(x, y))
      {
	_points.push_back(Point((double)x, (double)y));
      }
    }
  }
}
  
/*----------------------------------------------------------------*/
void PointList::formXyIntersection(const PointList &l)  
{
  Grid2d i("temp", _nx, _ny, -1);

  // put list to an image.
  l.toGrid(i, 100.0);

  // reduce points to points from l
  std::vector<Point>::iterator j;

  for (j=_points.begin(); j!= _points.end(); )
  {
    int x = j->getIntX();
    int y = j->getIntY();
    if (i.isMissing(x, y))
      j = _points.erase(j);
    else
      ++j;
  }
}
  
/*----------------------------------------------------------------*/
int PointList::
partiallyContainedIndex(const vector<PointList> &list_of_lists,
			double min_percent) const
{
  // make sure 'contained' makes sense at all before doing
  // anything major.
  if (size() <= 0 || list_of_lists.size() <= 0)
    return -1;

  // put local list to image as mask, count pixels
  Grid2d img("mask", _nx, _ny, -1);
  toGrid(img, 100.0);
  int nthis = img.numGood();

  // for each list in the list of lists.
  for (size_t i=0; i<list_of_lists.size(); ++i)
  {
    //if (li->_partially_contained(*limg, *img, mv, nthis, percent))
    if (list_of_lists[i]._partiallyContained(img, nthis, min_percent))
    {
      return i;
    }
  }
  return -1;
}

/*----------------------------------------------------------------*/
double PointList::percentileDataValue(const Grid2d &data,
				      double percentile) const
{
  // make an ordered list of all values (nonflag)
  OrderedList o;
  for (size_t i=0; i<_points.size(); ++i)
  {
    int x = _points[i].getIntX();
    int y = _points[i].getIntY();
    double v;
    if (data.getValue(x, y, v))
    {
      o.addToListUnordered(v);
    }
  }
 
  double d;
  if (o.num() == 0)
    d = data.getMissing();
  else
  {
    o.order();
    d = o.percentile(percentile);
  }
  return d;
}

/*----------------------------------------------------------------*/
double PointList::correlation(const Grid2d &x, const Grid2d &y) const
{
  double xbar=0, ybar=0, n=0;
  for (size_t i=0; i<_points.size(); ++i)
  {
    int ix = _points[i].getIntX();
    int iy = _points[i].getIntY();
    double xi, yi;
    if (x.getValue(ix, iy, xi) && y.getValue(ix, iy, yi))
    {
      xbar += xi;
      ybar += yi;
      ++n;
    }
  }
  if (n == 0)
    return 0.0;
  
  xbar /= n;
  ybar /= n;

  double num=0, denomx=0, denomy=0;
  for (size_t i=0; i<_points.size(); ++i)
  {
    int ix = _points[i].getIntX();
    int iy = _points[i].getIntY();
    double xi, yi;
    if (x.getValue(ix, iy, xi) && y.getValue(ix, iy, yi))
    {
      num += (xi-xbar)*(yi-ybar);
      denomx += (xi-xbar)*(xi-xbar);
      denomy += (yi-ybar)*(yi-ybar);
    }
  }
  if (denomx == 0 || denomy == 0)
    return 0.0;
  else
    return (num/sqrt(denomx*denomy));
}

/*----------------------------------------------------------------*/
bool PointList::max(const Grid2d &g, double &maxV) const
{
  maxV=0;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    double v;
    int ix = _points[i].getIntX();
    int iy = _points[i].getIntY();
    if (g.getValue(ix, iy, v))
    {
      if (first)
      {
	first = false;
	maxV = v;
      }
      else
      {
	if (v > maxV)
	  maxV = v;
      }
    }
  }
  return !first;
}

/*----------------------------------------------------------------*/
bool PointList::min(const Grid2d &g, double &minV) const
{
  minV=0;
  bool first = true;

  for (size_t i=0; i<_points.size(); ++i)
  {
    double v;
    int ix = _points[i].getIntX();
    int iy = _points[i].getIntY();
    if (g.getValue(ix, iy, v))
    {
      if (first)
      {
	first = false;
	minV = v;
      }
      else
      {
	if (v < minV)
	  minV = v;
      }
    }
  }
  return !first;
}


/*----------------------------------------------------------------*/
void PointList::geThreshold(const Grid2d &img, double threshold)
{
  vector<Point>::iterator it;
  for (it=_points.begin(); it!=_points.end(); )
  {
    int x = it->getIntX();
    int y = it->getIntY();
    double v;
    if (img.getValue(x, y, v))
    {
      if (v >= threshold)
	++it;
      else
	it = _points.erase(it);
    }
    else
    {
      it = _points.erase(it);
    }
  }
}

/*----------------------------------------------------------------*/
void PointList::clearMasked(const Grid2d &mask, Grid2d &img)
{
  std::vector<Point>::iterator i;

  for (i=_points.begin(); i!=_points.end(); )
  {
    int x = i->getIntX();
    int y = i->getIntY();
    if (mask.isMissing(x, y))
      //not mask point..keep
      ++i;
    else
    {
      // in mask, delete.
      i = _points.erase(i);
      img.setMissing(x, y);
    }
  }
}

/*----------------------------------------------------------------*/
void PointList::clearMasked(const Grid2d &img)
{
  std::vector<Point>::iterator i;

  for (i=_points.begin(); i != _points.end(); )
  {
    int x = i->getIntX();
    int y = i->getIntY();
    if (img.isMissing(x, y))
      // not mask point..keep
      ++i;
    else
      // in mask, delete.
      i = _points.erase(i);
  }
}

/*----------------------------------------------------------------*/
void
PointList::filterAngleDiffTooLargeAndMask(double x, double y,
					  double a0,
					  double max_change,
					  const Grid2d &mask,
					  double mask_value,
					  Grid2d &img)
{
  std::vector<Point>::iterator i;
  double x0, y0, a;
 
  for (i=_points.begin(); i!=_points.end(); )
  {
    x0 = i->getX();
    y0 = i->getY();
    if (mask.getValue((int)x0, (int)y0) == mask_value)
      i = _points.erase(i);
    else
    {
      // get the angle from x,y to x0,y0
      a = atan2(y0 - y, x0 - x)*180.0/3.14159;
      if (a < 0.0)
	a += 360.0;
      if (Math::anglesTooFarApart(a0, a, max_change))
	i = _points.erase(i);
      else
	++i;
    }
  }
}

/*----------------------------------------------------------------*/
void PointList::clearMaskAtValue(Grid2d &mask, double mask_value)
{
  std::vector<Point>::iterator i;
  for (i=_points.begin(); i != _points.end(); )
  {
    int x = i->getIntX();
    int y = i->getIntY();
    if (mask.getValue(x, y) == mask_value)
    {
      // in mask, delete.
      i = _points.erase(i);
      mask.setMissing(x, y);
    }
    else
      // not mask point..keep
      ++i;
  }
}

/*----------------------------------------------------------------*/
void PointList::filterLookahead(double x, double y, double angle,
				double r0, double r1, double dangle0,
				double value0, double dangle1, 
				double value1, Grid2d &mask)
{
  if (size() == 0)
    return;

  double r02 = r0*r0;
  double r12 = r1*r1;

  std::vector<Point>::iterator i;
  for (i=_points.begin(); i!= _points.end(); )
  {
    int ix = i->getIntX();
    int iy = i->getIntY();
    double d = mask.getValue(ix, iy);
    bool bad;
    if (d == value0)
      bad = _lookaheadBad(ix, iy, x, y, r02, r12, angle, dangle0);
    else if (d == value1)
      bad = _lookaheadBad(ix, iy, x, y, r02, r12, angle, dangle1);
    else
    {
      LOG(ERROR) <<  "bad inputs to filter_lookahead";
      bad = false;
    }
    if (bad)
    {
      mask.setMissing(ix, iy);
      i = _points.erase(i);
    }
    else
      ++i;
  }
}

void PointList::filter(int nx, int ny)
{
  _nx = nx;
  _ny = ny;
  std::vector<Point>::iterator i;
  for (i=_points.begin(); i!= _points.end(); )
  {
    int ix = i->getIntX();
    int iy = i->getIntY();
    if (ix < 0 || ix >= nx || iy < 0 || iy >= ny)
    {
      i = _points.erase(i);
    }
    else
    {
      i++;
    }
  }
}  

/*----------------------------------------------------------------*/
void PointList::removeOutlierValuedPoints(const Grid2d &data,
					  double maxDataRange,
					  int minPts)
{
  int n = 0;
  int bigNumber = data.getNdata();
  
  while (true)
  {
    PointListDataDiff diffs;
    for (size_t ii=0; ii<_points.size(); ++ii)
    {
      double v;
      int ix = _points[ii].getIntX();
      int iy = _points[ii].getIntY();
      if (data.getValue(ix, iy, v))
      {
	diffs.inc(v, ii);
      }
    }

    if (diffs.finish(maxDataRange))
    {
      break;
    }

    // get the index to remove and remove it.
    int index = diffs.biggestOutlierIndex();
    erase(index);
      
    if ((int)(_points.size()) < minPts)
    {
      break;
    }
    if (++n > bigNumber)
    {
      LOG(ERROR) << "Logic error could be infinite loop";
      break;
    }
  }
}

/*----------------------------------------------------------------*/
bool PointList::_partiallyContained(const Grid2d &inmask, int ninmask,
				     double percent) const
{
  double ni = static_cast<double>(ninmask);
  if (ni <= 0)
  {
    // contained not possible
    return false;
  }

  // put list to local temporary mask.
  GridAlgs mask("mask", _nx, _ny, -1);
  toGrid(mask, 100.0);
  double nm = static_cast<double>(mask.numGood());
  if (nm <= 0)
  {
    // contained not possible
    return false;
  }
	
  // now take intersection of masks and get # of intersection points
  // (=area of intersection).
  mask.intersection(inmask);
  double nintersect = static_cast<double>(mask.numGood());
	
  // see how this is compared to percent.
  return (nintersect/ni > percent || nintersect/nm > percent);
}

/*----------------------------------------------------------------*/
void PointList::erase(int index)
{
  if (index < 0 || index >= (int)_points.size())
  {
    LOG(ERROR) << "Erasing out of range index " << index << " ignore";
    return;
  }
  _points.erase(_points.begin() + index);
}

/*----------------------------------------------------------------*/
/*
 * Return radius of smallest circle centered at x0,y0 that encloses
 * the list
 */
double PointList::_enclosingCircleRadiusSquared(double x0, double y0) const
{
  double r0, r, x, y;
  
  r0 = 0.0;
  for (size_t i=0; i<_points.size(); ++i)
  {
    x = _points[i].getX();
    y = _points[i].getY();
    r = (x - x0)*(x - x0) + (y - y0)*(y - y0);
    if (r > r0)
      r0 = r;
  }
  return r0;
}



/*----------------------------------------------------------------*/
// more y than x range...y should increase from top to bottom,
// x should increase from left to right.
void PointList::_printAsciiPortrait(void) const
{
  int delta, x, y, ix, iy, ny, nx, maxx, maxy;
  char grid[200][79]; // [y][x]
  char xlabel[100];
  char ylabel[300];
  char tmp[100];
  double ax, ay;
  double x0, y0, x1, y1;

  x0 = minX();
  y0 = minY();
  x1 = maxX();
  y1 = maxY();

  // figure out resolution based on x range (left-right)
  delta = (int)((x1-x0-1)/78.0) + 1;

  // get nx and ny of data.
  ny = (int)((double)(y1-y0+1)/(double)delta);
  if (ny >= 200)
    ny = 200;
  nx = (int)((double)(x1-x0+1)/(double)delta);
  if (nx >= 78)
    nx = 78;

  // build x label directly over where data grid will be.
  for (x=0; x<100; ++x)
    xlabel[x] = ' ';
  maxx = nx;
  for (ax=x0; ax<=x1; ++ax)
    if (fmod(ax, 10.0) == 0)
    {
      ix = (int)((ax-x0)/(double)delta);
      sprintf(tmp, "%d", (int)ax);
      memcpy(&xlabel[ix], tmp, strlen(tmp));
      if (ix + (int)strlen(tmp) > maxx)
	maxx = ix + strlen(tmp);
    }
  xlabel[maxx] = 0;

  // build y label to go directly over data..top to bottom.
  for (y=0; y<300; ++y)
    ylabel[y] = ' ';
  maxy = ny+1;
  for (ay=y0; ay<=y1; ++ay)
    if (fmod(ay, 10.0) == 0.0)
    {
      iy = (int)((ay-y0)/(double)delta);
      sprintf(tmp, "%d", (int)ay);
      memcpy(&ylabel[iy], tmp, strlen(tmp));
      if (iy + (int)strlen(tmp) > maxy)
	maxy = iy + strlen(tmp);
    }
  ylabel[maxy] = 0;

  // clear the grid
  for (y=0; y<200; ++y)
  {
    for (x=0; x<maxx; ++x)
      grid[y][x] = ' ';
    grid[y][maxx] = 0;
  }

  // fill in the grid.
  for (size_t i=0; i<_points.size(); ++i)
  {
    ax = _points[i].getX() - x0;
    ay = _points[i].getY()- y0;
    ax = ax/(double)delta;
    ay = ay/(double)delta;
    ix = (int)ax;
    iy = (int)ay;
    grid[iy][ix] = 'x';
  }

  printf("     X\n");
  printf(" %s\n", xlabel);
    
  // each row preceded and followed by the ylabel for that row.
  for (iy=0; iy<ny; ++iy)
  {
    if (iy == ny/2)
      printf("%1c%s%1c Y\n", ylabel[iy], grid[iy], ylabel[iy]);
    else
      printf("%1c%s%1c\n", ylabel[iy], grid[iy], ylabel[iy]);
  }

  // x label, preceded and postceded by ylabel
  printf("%1c%s%1c\n", ylabel[ny], xlabel, ylabel[ny]);

  // remaininy rows where ylabel hangs down
  for (iy=ny+1; iy<maxy; ++iy)
    printf("%1c%s%1c\n", ylabel[iy], grid[iy], ylabel[iy]);
}

/*----------------------------------------------------------------*/
// more x than y..y should increase from left to right, x should
// increase from bottom to top.
void PointList::_printAsciiLandscape(void) const
{
  int delta, x, y, ix, iy, ny, nx, maxx, maxy;
  char grid[200][79]; // [x][y]
  char ylabel[100];
  char xlabel[300];
  char tmp[100];
  double ax, ay;
  double x0, y0, x1, y1;

  x0 = minX();
  y0 = minY();
  x1 = maxX();
  y1 = maxY();

  // figure out resolution based on y range (left-right)
  delta = (int)((y1-y0-1)/78.0) + 1;

  // get nx and ny of data.
  ny = (int)((double)(y1-y0+1)/(double)delta);
  if (ny >= 78)
    ny = 78;
  nx = (int)((double)(x1-x0+1)/(double)delta);
  if (nx >= 200)
    nx = 200;

  // build y label (left to right) directly over data.
  for (y=0; y<100; ++y)
    ylabel[y] = ' ';
  maxy = ny;
  for (ay=y0; ay<=y1; ++ay)
    //       if (fmod((float)ay, 10.0) == 0)
    if (fmod(ay, 10.0) == 0)
    {
      iy = (int)((ay-y0)/(double)delta);
      sprintf(tmp, "%d", (int)ay);
      memcpy(&ylabel[iy], tmp, strlen(tmp));
      if (iy + (int)strlen(tmp) > maxy)
	maxy = iy + strlen(tmp);
    }
  ylabel[maxy] = 0;

  // build x label to go directly over data..bottom to top.
  for (x=0; x<300; ++x)
    xlabel[x] = ' ';
  maxx = nx + 1;
  for (ax=x0; ax<=x1; ++ax)
    if (fmod(ax, 10.0) == 0)
      //       if (fmod((float)ax, 10.0) == 0)
    {
      ix = (int)((ax-x0)/(double)delta);
      ix = nx-1-ix;
      sprintf(tmp, "%d", (int)ax);
      memcpy(&xlabel[ix], tmp, strlen(tmp));
      if (ix + (int)strlen(tmp) > maxx)
	maxx = ix + strlen(tmp);
    }
  xlabel[maxx] = 0;
    
  // clear the grid
  for (x=0; x<200; ++x)
  {
    for (y=0; y<maxy; ++y)
      grid[x][y] = ' ';
    grid[x][maxy] = 0;
  }

  // fill in the grid...x goes bottom to top.
  for (size_t i=0; i<_points.size(); ++i)
  {
    ax = _points[i].getX() - x0;
    ay = _points[i].getY() - y0;
    ax = ax/(double)delta;
    ay = ay/(double)delta;
    ix = (int)ax;
    iy = (int)ay;
    ix = nx-1-ix;
    grid[ix][iy] = 'x';
  }

  // now put all this together.
  printf("     Y\n");

  // y label, offset by 1
  printf(" %s\n", ylabel);
    
  // each row preceded and followed by the xlabel for that row.
  for (ix=0; ix<nx; ++ix)
  {
    if (ix == nx/2)
      printf("%1c%s%1c X\n", xlabel[ix], grid[ix], xlabel[ix]);
    else
      printf("%1c%s%1c\n", xlabel[ix], grid[ix], xlabel[ix]);
  }
    
  // y label, preceded and postceded by xlabel for that row.
  printf("%1c%s%1c\n", xlabel[nx], ylabel, xlabel[nx]);

  // remaininy rows where xlabel hangs down
  for (ix=nx+1; ix<maxx; ++ix)
    printf("%1c%s%1c\n", xlabel[ix], grid[ix], xlabel[ix]);
}


/*----------------------------------------------------------------*/
PointList::PointListDataDiff::
PointListDataDiff(void): _iMin(0), _iMax(0), _min(0),
			 _max(0), _mean(0), _num(0),
			 _first(true), _debug(false)
{
}

/*----------------------------------------------------------------*/
PointList::PointListDataDiff::~PointListDataDiff(void)
{
}

/*----------------------------------------------------------------*/
void PointList::PointListDataDiff::inc(double v, int index)
{
  _mean += v;
  _num ++;
  if (_first)
  {
    _first = false;
    _iMin = _iMax = index;
    _min = _max = v;
  }
  else
  {
    if (v < _min)
    {
      _iMin = index;
      _min = v;
    }
    if (v > _max)
    {
      _iMax = index;
      _max = v;
    }
  }
}

/*----------------------------------------------------------------*/
bool PointList::PointListDataDiff::finish(double maxDiff)
{
  if (_first)
  {
    // no data
    if (_debug)
    {
      LOG(WARNING) << "No Points at all to filter";
      return true;
    }
  }
  if (_max - _min <= maxDiff)
  {
    // data is all within the tolerated differences range
    if (_debug)
    {
      LOG(DEBUG) << "difference within tolerence max=" << _max
		 << " min=" << _min;
    }
    return true;
  }

  // compute the mean from what was accumulated for later
  _mean /= _num;
  return false;
}

/*----------------------------------------------------------------*/
int PointList::PointListDataDiff::biggestOutlierIndex(void) const
{
  if (_max - _mean > _mean - _min)
  {
    // remove max
    if (_debug)
    {
      LOG(DEBUG) <<  "Removing MAX range:" << _max-_min
		 << " min:" << _min << " max" << _max << " mean:" << _mean;
    }
    return _iMax;
  }
  else
  {
    // remove max
    if (_debug)
    {
      LOG(DEBUG) <<  "Removing MIN range:" << _max-_min
		 << " min:" << _min << " max" << _max << " mean:" << _mean;
    }
    return _iMin;
  }
}
