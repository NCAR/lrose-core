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
#include <euclid/Line.hh>
#include <euclid/DataAtt.hh>
#include <euclid/Box.hh>
#include <euclid/PointList.hh>
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <euclid/MotionVector.hh>
#include <rapmath/Math.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>

#include <cmath>
#include <cstdio>

using std::string;
using std::vector;
using std::pair;

/*----------------------------------------------------------------*/
Line::Line(void) : AttributesEuclid()
{
  _init();
}

/*----------------------------------------------------------------*/
Line::Line(double ix0, double iy0, double ix1, double iy1) : AttributesEuclid()
{
  _init();
  _is_bad = false;
  _x0 = ix0;
  _y0 = iy0;
  _x1 = ix1;
  _y1 = iy1;
  _setOtherValues();
}

/*----------------------------------------------------------------*/
// line centered at 0,0 oriented at angle, of total length len
Line::Line(double angle, double len)  : AttributesEuclid()
{
  _init();
  _is_bad = false;

  double a = angle*3.14159/180.0;
    
  _x0 = -cos(a)*len/2.0;
  _x1 = cos(a)*len/2.0;
  _y0 = -sin(a)*len/2.0;
  _y1 = sin(a)*len/2.0;
  _setOtherValues();
}

/*----------------------------------------------------------------*/
Line::Line(double common_value, const Box &b, bool is_vert)  : AttributesEuclid()
{
  _init();
  _is_bad = false;
  if (is_vert)
  {
    _x0 = _x1 = common_value;
    b.getRange(_y0, _y1, false);
    _is_vertical = true;
    _slope = 0.0;
    _intercept = 0.0;
  }
  else
  {
    b.getRange(_x0, _x1, true);
    _y0 = _y1 = common_value;
    _is_vertical = false;
    _slope = 0.0;
    _intercept = common_value;
  }
}

/*----------------------------------------------------------------*/
Line::Line(double slope, double x0, double y0, const Box &b)  : AttributesEuclid()
{
  _init();
  _is_bad = false;
  _slope = slope;
  _intercept = y0 - slope*x0;
  _is_vertical = false;

  if (fabs(slope) < 1.0e-10)
  {
    /*
     * Horizontal..minx is point 0
     */
    _x0 = b._minx;
    _x1 = b._maxx;
    _y0 = _y1 = y0;
  }
  else
    _fillBounds(b);
}

/*----------------------------------------------------------------*/
// line spead each side of xc, yc, with slope unless is_vert.
Line::Line(double xc, double yc, double islope, double spread,
	   bool line_is_vert) : AttributesEuclid()
{
  _init();
  _is_bad = false;
  if (line_is_vert)
  {
    /*
     * Create it so that "min" is point 0, "max" is point 1
     */
    _x0 = _x1 = xc;
    _y0 = yc - spread;
    _y1 = yc + spread;
    _is_vertical = true;
    _slope = 0.0;
    _intercept = 0.0;
  }
  else
  {
    _slope = islope;
    _intercept = yc - _slope*xc;
    _is_vertical = false;
    if (fabs(islope) < 1.0e-10)
    {
      /*
       * Horizontal..
       */
      _x0 = xc - spread;
      _x1 = xc + spread;
      _y0 = _y1 = yc;
    }
    else
    {
      double a, b, c, d, m, i;
      m = _slope;
      i = _intercept;

      a = 1.0 + m*m;
      b = 2.0*(-xc + m*i - m*yc);
      c = xc*xc + i*i - 2.0*i*yc + yc*yc -  spread*spread;
      d = b*b - 4.0*a*c;
      if (d < 0.0)
      {
	LOG(ERROR) << "args are bad";
	makeBad();
      }
      _x0 = (-b + sqrt(d))/(2.0*a);
      _x1 = (-b - sqrt(d))/(2.0*a);
      _y0 = _slope*_x0 + _intercept;
      _y1 = _slope*_x1 + _intercept;
    }
  }
}

/*----------------------------------------------------------------*/
Line::~Line()
{
}

/*----------------------------------------------------------------*/
Line & Line::operator=(const Line &l)
{
  if (&l == this)
  {
    return *this;
  }
  _x0 = l._x0;
  _y0 = l._y0;
  _x1 = l._x1;
  _y1 = l._y1;
  _is_vertical = l._is_vertical;
  _slope = l._slope;
  _intercept = l._intercept;
  _is_bad = l._is_bad;
  _has_endpts = l._has_endpts;
  _endpts = l._endpts;
  _has_hand = l._has_hand;
  _hand = l._hand;
  Attributes::operator=(l);
  return *this;
}

/*----------------------------------------------------------------*/
Line::Line(const Line &l) : AttributesEuclid(l)
{
  _x0 = l._x0;
  _y0 = l._y0;
  _x1 = l._x1;
  _y1 = l._y1;
  _is_vertical = l._is_vertical;
  _slope = l._slope;
  _intercept = l._intercept;
  _is_bad = l._is_bad;
  _has_endpts = l._has_endpts;
  _endpts = l._endpts;
  _has_hand = l._has_hand;
  _hand = l._hand;
}

/*----------------------------------------------------------------*/
bool Line::operator==(const Line &l) const
{
  return (_is_bad == l._is_bad &&
	  Attributes::operator==(l) &&
	  _x0 == l._x0 &&
	  _y0 == l._y0 && 
	  _x1 == l._x1 &&
	  _y1 == l._y1 && 
	  _is_vertical == l._is_vertical &&
	  _slope == l._slope &&
	  _intercept == l._intercept &&
	  _has_endpts == l._has_endpts &&
	  _endpts == l._endpts &&
	  _has_hand == l._has_hand &&
	  _hand == l._hand);
}

/*----------------------------------------------------------------*/
string Line::writeXml(const std::string &tag) const
{
  string ret = TaXml::writeStartTag(tag, 0);
  ret += Attributes::writeAttXml("LineAttributes");
  ret += TaXml::writeDouble("X0", 0, _x0, "%.6lf");
  ret += TaXml::writeDouble("Y0", 0, _y0, "%.6lf");
  ret += TaXml::writeDouble("X1", 0, _x1, "%.6lf");
  ret += TaXml::writeDouble("Y1", 0, _y1, "%.6lf");
  ret += TaXml::writeBoolean("isVert", 0, _is_vertical);
  ret += TaXml::writeDouble("Slope", 0, _slope, "%.6lf");
  ret += TaXml::writeDouble("Intercept", 0, _intercept, "%.6lf");
  ret += TaXml::writeBoolean("isBad", 0, _is_bad);
  ret += TaXml::writeBoolean("HasEndpts", 0, _has_endpts);
  ret += _endpts.writeXml();
  ret += TaXml::writeBoolean("HasHand", 0, _has_hand);
  ret += _hand.writeXml();
  ret += TaXml::writeEndTag(tag, 0);
  return ret;
}

/*----------------------------------------------------------------*/
bool Line::readXml(const std::string &xml)
{
  Attributes::operator=(Attributes());
  _init();

  if (!Attributes::readAttXml(xml, "LineAttributes"))
  {
    return false;
  }

  if (TaXml::readDouble(xml, "X0", _x0))
  {
    LOG(ERROR) << "Parsing for tag X0";
    return false;
  }

  if (TaXml::readDouble(xml, "X1", _x1))
  {
    LOG(ERROR) << "Parsing for tag X1";
    return false;
  }

  if (TaXml::readDouble(xml, "Y0", _y0))
  {
    LOG(ERROR) << "Parsing for tag Y0";
    return false;
  }

  if (TaXml::readDouble(xml, "Y1", _y1))
  {
    LOG(ERROR) << "Parsing for tag Y1";
    return false;
  }


  if (TaXml::readDouble(xml, "Slope", _slope))
  {
    LOG(ERROR) << "Parsing for tag Slope";
    return false;
  }

  if (TaXml::readBoolean(xml, "isVert", _is_vertical))
  {
    LOG(ERROR) << "Parsing for tag isVert";
    return false;
  }

  if (TaXml::readBoolean(xml, "isBad", _is_bad))
  {
    LOG(ERROR) << "Parsing for tag isBad";
    return false;
  }

  if (TaXml::readDouble(xml, "Intercept", _intercept))
  {
    LOG(ERROR) << "Parsing for tag Intercept";
    return false;
  }

  if (TaXml::readBoolean(xml, "HasEndpts", _has_endpts))
  {
    LOG(ERROR) << "Parsing for tag HasEndpts";
    return false;
  }

  if (TaXml::readBoolean(xml, "HasHand", _has_hand))
  {
    LOG(ERROR) << "Parsing for tag HasHand";
    return false;
  }
  if (!_endpts.readXml(xml))
  {
    return false;
  }
  if (!_hand.readXml(xml))
  {
    return false;
  }
  return true;
}

/*----------------------------------------------------------------*/
void Line::print(FILE *fp) const
{
  if (!_is_vertical)
    fprintf(fp, "(%5.2f,%5.2f) to (%5.2f,%5.2f) m=%5.2f b=%5.2f ",
	    _x0, _y0, _x1, _y1, _slope, _intercept);
  else
    fprintf(fp, "(%5.2f,%5.2f) to (%5.2f,%5.2f) vertical ",
	    _x0, _y0, _x1, _y1);
  printAtt(fp);
  if (_has_endpts)
  {
    _endpts.print(fp);
    printf(" ");
  }
  if (_has_hand)
    _hand.print(fp);
  printf("\n");
}

/*----------------------------------------------------------------*/
void Line::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
string Line::sprint(void) const
{
  char buf[1000];
  if (!_is_vertical)
    sprintf(buf, "(%5.2f,%5.2f) to (%5.2f,%5.2f) m=%5.2f b=%5.2f ",
	    _x0, _y0, _x1, _y1, _slope, _intercept);
  else
    sprintf(buf, "(%5.2f,%5.2f) to (%5.2f,%5.2f) vertical ",
	    _x0, _y0, _x1, _y1);
  string ret = buf;

  ret += sprintAtt();
  if (_has_endpts)
  {
    ret += _endpts.sprint();
    ret += " ";
  }
  if (_has_hand)
    ret += _hand.sprint();
  return ret;
}

/*----------------------------------------------------------------*/
double Line::length(void) const
{
  double x, y;

  x = (_x0 - _x1);
  y = (_y0 - _y1);
  return sqrt(x*x + y*y);
}

/*----------------------------------------------------------------*/
// should worry about handedness and such?
void Line::reverse(void)
{
  double x, y;
  
  x = _x0;
  y = _y0;
  _x0 = _x1;
  _y0 = _y1;
  _x1 = x;
  _y1 = y;
}

/*----------------------------------------------------------------*/
void Line::adjustBorderlineAlpha(double &alpha) const
{
  double d;

  d = length();
  if (Math::small(d*alpha))
    alpha = 0.0;
  if (Math::small(d*(1.0 - alpha)))
    alpha = 1.0;
}

/*----------------------------------------------------------------*/
void Line::makeHorizontal(double &rotation_degrees,
			  const bool change_endpt_order)
{
  double a;
    
  if (_is_vertical)
    a = 90.0;
  else
    a = atan2(_slope, 1.0)*180.0/3.14159;
  rotation_degrees = a;

  rotate(a, change_endpt_order);

  /*
   * Adjust so its exactly horizontal
   */
  _slope = 0.0;
  _y0 = _y1;
  _intercept = _y0;
}

/*----------------------------------------------------------------*/
void Line::rotate(double angle, bool change_endpts)
{
  _rotatePoint(_x0, _y0, angle);
  _rotatePoint(_x1, _y1, angle);
  if (_x0 > _x1 && change_endpts)
  {
    double x, y;

    x = _x0;
    y = _y0;
    _x0 = _x1;
    _y0 = _y1;
    _x1 = x;
    _y1 = y;
  }
  _setOtherValues();
}

/*----------------------------------------------------------------*/
void Line::orderEndpts(double x, double y)
{
  double d0, d1;
  
  /*
   * Get 0'th and 1'th distances from endpoint to x,y
   */
  d0 = (_x0 - x)*(_x0 - x) + (_y0 - y)*(_y0 - y);
  d1 = (_x1 - x)*(_x1 - x) + (_y1 - y)*(_y1 - y);

  /*
   * Reverse endpoints if needed so 0th endpoint closest
   * to x0,y0.
   */
  if (d1 < d0)
    reverse();
}

/*----------------------------------------------------------------*/
void Line::centerpoint(double &x, double &y) const
{
  x = (_x0 + _x1)/2.0;
  y = (_y0 + _y1)/2.0;
}

/*----------------------------------------------------------------*/
double Line::minX(void) const
{
  if (_x0 <= _x1)
    return _x0;
  else
    return _x1;
}

/*----------------------------------------------------------------*/
double Line::maxX(void) const
{
  if (_x0 >= _x1)
    return _x0;
  else
    return _x1;
}
/*----------------------------------------------------------------*/
double Line::minY(void) const
{
  if (_y0 <= _y1)
    return _y0;
  else
    return _y1;
}

/*----------------------------------------------------------------*/
double Line::maxY(void) const
{
  if (_y0 >= _y1)
    return _y0;
  else
    return _y1;
}

/*----------------------------------------------------------------*/
double Line::averageWeightedDistanceSquared(const PointList &p,
					       const Grid2d &mask,
					       const Grid2d &data) const
{
  double v = 0.0, w=0.0, v0, v1, centerx, centery, x0, y0;
 
  centerpoint(centerx, centery);
  for (int i=0; i<p.size(); ++i)
  {
    x0 = p.ithX(i);
    y0 = p.ithY(i);

    // distance from centerpoint to x,y
    v0 = (x0-centerx)*(x0-centerx) + (y0-centery)*(y0-centery);

    // closest distance from objects point of view.
    v1 = minDistanceSquared(centerx, centery);
    if (v1 < v0)
      v0 = v1;

    if (mask.isMissing((int)centerx, (int)centery))
      continue;
    double w0;
    if (data.getValue((int)centerx, (int)centery, w0))
    {
      v += v0*w0;
      w += w0;
    }
  }
  if (w != 0.0)
    return v/w;
  else
    return 0.0;
}

/*----------------------------------------------------------------*/
bool Line::averageLineDistance(const Line &l, double &dist) const
{
  // this is analytical computation..the double integral along both
  // lines of distances
  double ax, ay, bx, by, cx, cy, len, llen;

  len = length();
  llen = l.length();
  if (Math::verySmall(len) || Math::verySmall(llen))
  {
    return false;
  }
  ax = l._x0 - l._x1;
  bx = _x1 - _x0;
  cx = l._x1 - _x1;
  ay = l._y0 - l._y1;
  by = _y1 - _y0;
  cy = l._y1 - _y1;
    
  dist = ((ax*ax + ay*ay + bx*bx + by*by)/3.0 + (ax*bx + ay*by)/2.0 +
	  ax*cx + ay*cy + bx*cx + by*cy + cx*cx + cy*cy);

  // normalize by distance squared:
  dist = dist/(len*llen);
  return true;
}

/*----------------------------------------------------------------*/
double Line::minDistance(double x, double y) const
{
  return sqrt(minDistanceSquared(x, y));
}

/*----------------------------------------------------------------*/
double Line::minDistanceSquared(double x, double y) const
{
  // horizontal, vertical, or in between
  if (fabs(_slope) < 1.0e-10)
    return _minDistanceHorizontalSquared(x, y);
  else if (_is_vertical)
    return _minDistanceVerticalSquared(x, y);
  else
    return _minDistanceSlopedSquared(x, y);
}

/*----------------------------------------------------------------*/
double Line::minDistanceSquared(const Line &t) const
{
  // this is wrong!
  double x0, y0;
  t.centerpoint(x0, y0);
  return minDistanceSquared(x0, y0);
}

/*----------------------------------------------------------------*/
double Line::minimumDistanceBetween(const Line &l1) const

{
  double x, y;
  if (intersect(l1, x, y))
  {
    return 0.0;
  }

  /*
   * 4 straight distances between endpoints and other line.
   */
  double dmin = minDistance(l1._x0, l1._y0);
  double d = minDistance(l1._x1, l1._y1);
  if (d < dmin)
    dmin = d;
  d = l1.minDistance(_x0, _y0);
  if (d < dmin)
    dmin = d;
  d = l1.minDistance(_x1, _y1);
  if (d < dmin)
    dmin = d;
  return dmin;
}

/*----------------------------------------------------------------*/
void Line::bisector(int end_index, double percentage, const Box &b)
{
  double x, y;
    
  // find bisect point, percentage from end_index
  if (end_index == 0)
    parametricLocation(1.0-percentage, x, y);
  else
    parametricLocation(percentage, x, y);


  // build line through this point perpendicular to original line
  if (_is_vertical)
  {
    _x0 = b._minx;
    _x1 = b._maxx;
    _y0 = _y1 = y;
  }
  else if (fabs(_slope) < 1.0e-10)
  {
    _x0 = _x1 = x;
    _y0 = b._miny;
    _y1 = b._maxy;
  }
  else
  {
    Line l(-1.0/_slope, x, y, b);
    *this = l;
  }
  _setOtherValues();
}

/*----------------------------------------------------------------*/
void Line::bisector(int end_index, double percentage, double len)
{
  double x, y;
    
  // find bisect point, percentage from end_index
  if (end_index == 0)
  {
    parametricLocation(1.0 - percentage, x, y);
  }
  else
  {
    parametricLocation(percentage, x, y);
  }

  // build line through this point perpendicular to original line
  if (_is_vertical)
  {
    _x0 = x - len/2.0;
    _x1 = x + len/2.0;
    _y0 = _y1 = y;
  }
  else if (fabs(_slope) < 1.0e-10)
  {
    _x0 = _x1 = x;
    _y0 = y - len/2.0;
    _y1 = y + len/2.0;
  }
  else
  {
    // line spead each side of xc, yc, with slope unless is_vert.
    *this = Line(x, y, -1.0/_slope, len/2.0, false);
  }
  _setOtherValues();
}

/*----------------------------------------------------------------*/
void Line::bisect(int end_index, double percentage)
{
  double xb, yb, alpha;

  if (percentage >= 1.0 || percentage <= 0.0)
    return;
  if (end_index == 0)
  {
    alpha = 1.0 - percentage;
    parametricLocation(alpha, xb, yb);
    _x1 = xb;
    _y1 = yb;
  }
  else
  {
    alpha = percentage;
    parametricLocation(alpha, xb, yb);
    _x0 = xb;
    _y0 = yb;
  }
}

/*----------------------------------------------------------------*/
void Line::unitVectorFromEndpt(int endpt, double &x, double &y) const
{
  if (endpt == 0)
  {
    x = _x1 - _x0;
    y = _y1 - _y0;
  }
  else
  {
    x = _x0 - _x1;
    y = _y0 - _y1;
  }

  // normalize?  doesn't seem to need it.
}
/*----------------------------------------------------------------*/
void Line::oneSideOfLine(PointList &l, const double ux, const double uy) const
{
  double len;
  int i;
  bool which_side;
    
  if (fabs(ux) < 1.0e-10)
    which_side = (uy > 0.0);
  else
  {
    if (fabs(uy) < 1.0e-10)
      which_side = (ux > 0.0);
    else
      which_side = (uy > 0.0);
  }
	    
  len = l.size();
  PointList lnew(l);
  lnew.clear();
  for (i=0; i<len; ++i)
  {
    double x = l.ithX(i);
    double y = l.ithY(i);
    if (_pointIsGreater(x, y) == which_side)
      lnew.append(x, y);
  }
  l = lnew;
}

/*----------------------------------------------------------------*/
bool Line::intersect(const Line &l1, double &x, double &y) const
{
  bool horiz, horiz1, vert, vert1;

  // are the lines the same?
  if (fabs(l1._x0 - _x0) < 1.0e-10 &&
      fabs(l1._x1 - _x1) < 1.0e-10 &&
      fabs(l1._y0 - _y0) < 1.0e-10 &&
      fabs(l1._y1 - _y1) < 1.0e-10)
  {
    centerpoint(x, y);
    return true;
  }

  // is current line horizontal?
  horiz = fabs(_slope) < 1.0e-10;
  horiz1 = fabs(l1._slope) < 1.0e-10;
  vert = _is_vertical;
  vert1 = l1._is_vertical;
  
  if (horiz)
  {
    if (horiz1)
    {
      if (fabs(_y0 - l1._y0) < 1.0e-10)
      {
	y = _y0;
	if (_x1 >= l1._x0 && l1._x1 >= _x0)
	{
	  // just pick  a point
	  x = (_x1 + l1._x0)/2.0;
	  // make sure
	  if (x >= _x0 && x <= _x1 &&
	      x >= l1._x0 && x <= l1._x1)
	  {
	    return true;
	  }
	  else
	  {
	    LOG(ERROR) << "Logic error horiz/horiz case";
	    return false;
	  }
	}
      }
    }
    else if (vert1)
    {
      if (l1._x0 >= _x0 && l1._x0 <= _x1 && l1._y0 <= _y0 && l1._y1 >= _y0)
      {
	x = l1._x0;
	y = _y0;
	return true;
      }
    }
    else
    {
      x = l1.xAtY(_x0);
      if (x >= _x0 && x <= _x1)
      {
	y = _y0;
	return true;
      }
    }
  }
  else if (vert)
  {
    if (horiz1)
    {
      if (_x0 >= l1._x0 && _x0 <= l1._x1 && _y0 <= l1._y0 && _y1 >= l1._y0)
      {
	x = _x0;
	y = l1._y0;
	return true;
      }
    }
    else if (vert1)
    {
      if (fabs(_x0 - l1._x0) < 1.0e-10)
      {
	x = _x0;
	if (_y1 >= l1._y0 && l1._y1 >= _y0)
	{
	  // just pick  a point
	  y = (_y1 + l1._y0)/2.0;
	  // make sure
	  if (y >= _y0 && x <= _y1 &&
	      y >= l1._y0 && x <= l1._y1)
	  {
	    return true;
	  }
	  else
	  {
	    LOG(ERROR) << "logic error, vert/vert case";
	    return false;
	  }
	}
      }
    }
    else
    {
      y = l1.yAtX(_x0);
      if (y >= _y0 && y <= _y1)
      {
	x = _x0;
	return true;
      }
    }
  }
  else
  {
    if (horiz1)
    {
      x = xAtY(l1._x0);
      if (x >= l1._x0 && x <= l1._x1)
      {
	y = l1._y0;
	return true;
      }
    }
    else if (vert1)
    {
      y = yAtX(l1._x0);
      if (y >= l1._y0 && y <= l1._y1)
      {
	x = l1._x0;
	return true;
      }
    }
    else
    {
      if (fabs(_slope - l1._slope) < 1.0e-10)
      {
	if (fabs(l1._intercept - _intercept) < 1.0e-10)
	{
	  // same slope and intercept, overlap in x will prove overlap in y
	  if (_x1 >= l1._x0 && l1._x1 >= _x0)
	  {
	    // just pick  a point
	    x = (_x1 + l1._x0)/2.0;
	    // make sure
	    if (x >= _x0 && x <= _x1 &&
		x >= l1._x0 && x <= l1._x1)
	    {
	      y = yAtX(x);
	      return true;
	    }
	  }
	  else
	  {
	    LOG(ERROR) << "Logic error parallel sloped case";
	    return false;
	  }
	}
      }
      else
      {
	x = (l1._intercept - _intercept)/(_slope - l1._slope);
	y = _slope*x + _intercept;
	return true;
      }
    }
  }
  return false;
}

/*----------------------------------------------------------------*/
// This assumes the line has a generic double attribute which is
// the percentile weight to use.
void Line::
appendOrientationsToVector(int x, int y, 
			   int radius_squared, 
			   const string &attribute_name,
			   vector<pair<double,double> > &o) const
{
  double di, weight;
    
  di = minDistanceSquared((double)x, (double)y);
  if (di > radius_squared)
    return;
  di = degreesSloped0180();
 
  if (!getDouble(attribute_name, weight))
    //     weight = get_generic_double();
    //     type = get_generic_double_type();
    // if (!att_get_double(attribute_name, weight))
  {
    //     if (type != (int)cldGenericDouble::PERCENTILE_LINE_BUILD)
    LOG(WARNING) << "missing attribute " << attribute_name;
    weight = 0.0;
  }
  o.push_back(pair<double,double>(di,weight));
}

/*----------------------------------------------------------------*/
double Line::vectorAngleFromEnd(int which) const
{
  if (which == 0)
    return Math::vectorLineAngle(_x0, _x1, _y0, _y1, _is_vertical, _slope);
  else
    return Math::vectorLineAngle(_x1, _x0, _y1, _y0, _is_vertical, _slope);
}

/*----------------------------------------------------------------*/
void Line::clearBetween(const Line &other, Grid2d &tmp) const
{
  int ix, iy;
  bool is_greater0, is_greater1;
  Line l0(other);
  Line l1(*this);

  // build bisector at _x0,_y0 of other.
  l0.bisector(0, 0.0, 100.0);

  // see if _x1,_y1 of other is greater than or less than
  // equation of l.
  is_greater0 = l0._pointIsGreater(other._x1, other._y1);

  // build bisector at _x1,_y1 of this.
  l1.bisector(1, 0.0, 100.0);

  // see if _x0,_y0 of this is greater than or less than equation of line.
  is_greater1 = l1._pointIsGreater(_x0, _y0);

  // For each point in the image
  int nx = tmp.getNx();
  int ny = tmp.getNy();
  for (iy=0; iy<ny; ++iy)
  {
    for (ix=0; ix<nx; ++ix)
    {
      if ((l0._pointIsGreater(ix, iy) == is_greater0) &&
	  (l1._pointIsGreater(ix, iy) == is_greater1))
      {
	// its between
	tmp.setMissing(ix, iy);
      }
    }
  }
}

/*----------------------------------------------------------------*/
void Line::extendedBehind(int width, int len)
{
  double x2, y2, x3, y3, alpha, llen, w;

  llen = length();

  // make a new line that extends behind current line length
  // same line equation, but new 0'th endpoint x2,y2
  // using parametric line equation, come out with alpha as follows:
  alpha = 1.0 + (double)len/llen;
  x2 = alpha*_x0 + (1.0-alpha)*_x1;
  y2 = alpha*_y0 + (1.0-alpha)*_y1;
    
  // set so back up width 
  w = (double)width;

  // want it so that a bandaid constructed on the new line will
  // just touch the line at _x1,_y1 - w

  // now back up by w from _x1,_y1
  alpha = w/llen;
  x3 = alpha*_x0 + (1.0-alpha)*_x1;
  y3 = alpha*_y0 + (1.0-alpha)*_y1;

  _x0 = x2;
  _y0 = y2;
  _x1 = x3;
  _y1 = y3;
  _setOtherValues();
}

/*----------------------------------------------------------------*/
void Line::extendedAhead(int width, int len)
{
  double x2, y2, x3, y3, alpha, llen, w;

  llen = length();

  // make a new line that extends ahead of current line length
  // same line equation, but new 1'th endpoint x2,y2
  // using parametric line equation, come out with alpha as follows:
  alpha = -(double)len/llen;
  x2 = alpha*_x0 + (1.0-alpha)*_x1;
  y2 = alpha*_y0 + (1.0-alpha)*_y1;
    
  // set so back up width
  w = (double)width;

  // want it so that a bandaid constructed on the new line will
  // just touch the line at _x0,_y0 + w

  // now move ahead by w from _x0,_y0
  alpha = 1.0 - w/llen;
  x3 = alpha*_x0 + (1.0-alpha)*_x1;
  y3 = alpha*_y0 + (1.0-alpha)*_y1;

  _x0 = x3;
  _y0 = y3;
  _x1 = x2;
  _y1 = y2;
  _setOtherValues();
}

/*----------------------------------------------------------------*/
void Line::addMotion(Grid2d &mask, const Grid2d &motion_angle, 
		      const Grid2d &motion_magnitude, bool debug)
{
  double a, m;

  // put average values to a mask.
  mask.setAllMissing();
  toGrid(mask, 100);

  GridAlgs ma = GridAlgs::promote(motion_angle);
  GridAlgs mm = GridAlgs::promote(motion_magnitude);
  if (ma.angleAverageInMask(mask, a) && mm.averageInMask(mask, m))
    _replaceMotionWithSimilarMotion(a, m, debug);
  else
    removeMotionVector();
}

/*----------------------------------------------------------------*/
void Line::toGrid(Grid2d &image, double value) const
{
  LineFollow l(*this);
  double x, y;
  while (l.next(x, y))
  {
    image.setValue((int)x, (int)y, value);
  }
}

/*----------------------------------------------------------------*/
PointList Line::xyValues(void) const
{
  PointList xyl;
    
  LineFollow l(*this);

  double x, y;
  while (l.next(x,y))
  {
    xyl.append(x, y);
  }

  return xyl;
}

/*----------------------------------------------------------------*/
Box Line::extrema(void) const
{
  double bx0, bx1, by0, by1;
  
  if (_x0 <= _x1)
  {
    bx0 = _x0;
    bx1 = _x1;
  }
  else
  {
    bx0 = _x1;
    bx1 = _x0;
  }
  if (_y0 <= _y1)
  {
    by0 = _y0;
    by1 = _y1;
  }
  else
  {
    by0 = _y1;
    by1 = _y0;
  }
  return Box(bx0, by0, bx1, by1);
}

/*----------------------------------------------------------------*/
bool Line::vectorIntersects(double x, double y, double deg,
			    double &xp, double &yp) const
{
  double rad = deg*3.14159/180.0;
  double costheta, sintheta, z, r;
  bool stat;

  costheta = cos(rad);
  sintheta = sin(rad);

  // let xp,yp = common point of line and vector from
  // x,y at deg degrees.  

  if (_is_vertical)
  {
    // there is no intersection if the angle is also
    // vertical and x is not equal to x0.  A vertical line
    // is one for which the angle has no cosine component.
    if (Math::verySmall(costheta))
    {
      if (!Math::veryClose(x, _x0))
	return false;
      // the result is true if the point and angle
      // are pointing the right way.
      if (y < _y0 && y < _y1)
      {
	if ((stat = Math::verySmall(Math::angleDiff(deg, 90.0))))
	{
	  xp = x;
	  yp = _y0;
	  if (_y0 > _y1)
	    yp = _y1;
	}
      }
      else if ( y > _y1 && y > _y0)
      {
	if ((stat = Math::verySmall(Math::angleDiff(deg, 270.0))))
	{
	  xp = x;
	  yp = _y0;
	  if (_y0 < _y1)
	    yp = _y1;
	}
      }
      else
      {
	if (_y0 < _y1)
	{
	  if ((stat=(y >= _y0 && y <= _y1)))
	  {
	    xp = x;
	    yp = y;
	  }
	}
	else
	{
	  if ((stat=(y >= _y1 && y <= _y0)))
	  {
	    xp = x;
	    yp = y;
	  }
	}
      }
      return stat;
    }
    else
    {
      // the line is vertical but input vector isn't.
      // this means costheta not zero.
      // solve for r where _x0 = r*costheta + x
      r = (_x0 - x)/costheta;
      xp = x + r*costheta;
      yp = y + r*sintheta;
      if (r < 0.0)
	return false;
      if (_y0 < _y1)
	stat= (yp >= _y0 && yp <= _y1);
      else
	stat = (yp >= _y1 && yp <= _y0);
      return stat;
    }
  }        
  else
  {
    // not vertical...find intersection point..
    // line = y = _slope*x + _interecept.
    // points are x + r*costheta, y + r*sintheta
    // do some math and get:
    z = sintheta - _slope*costheta;
    if (Math::verySmall(z))
      return false;
    r = (_slope*x + _intercept - y)/z;
    if (r < 0.0)
      return false;
    xp = x + r*costheta;
    yp = y + r*sintheta;
    if (_x0 < _x1)
    {
      if (xp < _x0 || xp > _x1)
	return false;
    }
    else
    {
      if (xp < _x1 || xp > _x0)
	return false;
    }
    if (_y0 < _y1)
    {
      if (yp < _y0 || yp > _y1)
	return false;
    }
    else
    {
      if (yp < _y1 || yp > _y0)
	return false;
    }
    return true;
  }
}

/*----------------------------------------------------------------*/
void Line::extrapolate(double seconds)
{
  MotionVector mv;
  if (getMotionVector(mv))
  {
    double vx = mv.getVx();
    double vy = mv.getVy();
    // assum pixels per second is units
    move(vx*seconds, vy*seconds);
  }
}

/*----------------------------------------------------------------*/
void Line::move(double dx, double dy)
{
  _x0 += dx;
  _x1 += dx;
  _y0 += dy;
  _y1 += dy;
  _setOtherValues();
}

/*----------------------------------------------------------------*/
bool Line::getEndpts(Endpts &e) const
{
  if (_has_endpts)
  {
    e = _endpts;
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
void Line::setEndpts(const Endpts &h)
{
  _endpts = h;
  _has_endpts = true;
}

/*----------------------------------------------------------------*/
Endpts Line::averageEndpts(const Line &l) const
{
  Endpts h;
  if (_has_endpts)
  {
    h = _endpts;
    if (l._has_endpts)
      h = h.average(l._endpts);
  }
  else
  {
    if (l._has_endpts)
      h = l._endpts;
    else
      h = Endpts();
  }
  return h;
}

/*----------------------------------------------------------------*/
bool Line::getHandedness(Handedness &e) const
{
  if (_has_hand)
  {
    e = _hand;
    return true;
  }
  else
    return false;
}

/*----------------------------------------------------------------*/
void Line::setHandedness(void)
{
  double angle0, aleft, aright, angle1, dl, dr, vx, vy;
    
  MotionVector mv;
  if (!getMotionVector(mv))
  {
    _has_hand = true;
    _hand= Handedness(Handedness::UNKNOWN);
    return;
  }
  vx = mv.getVx();
  vy = mv.getVy();
    
  _has_hand = true;
  if (Math::verySmall(vx) && Math::verySmall(vy))
  {
    // velocity is zero.
    _hand = Handedness(Handedness::NONE);
    return;
  }

  /*
   * Vector goes from 0'th to 1th endpoint..has some angle
   */
  angle0 = vectorAngleFromEnd(0);

  /*
   * compute left and a right handedness angle based on that.
   * Remember, in our coordinate system, positive y is "down"
   */
  aleft = Math::leftRotation(angle0);
  aright = Math::rightRotation(angle0);

  /*
   * Get the vector angle of the velocity
   */
  Line line(0, 0, vx, vy);
  angle1 = line.vectorAngleFromEnd(0);

  /*
   * expect either left or right handed direction to be
   * approximately equal to the velocity angle
   */
  dl = Math::angleDiff(angle1, aleft);
  dr = Math::angleDiff(angle1, aright);
  if (dl < dr)
    _hand = Handedness(Handedness::LEFT);
  else
    _hand = Handedness(Handedness::RIGHT);
}
  
/*----------------------------------------------------------------*/
void Line::setHandedness(Handedness::e_hand_t &e)
{
  _hand = Handedness(e);
  _has_hand = true;
}

/*----------------------------------------------------------------*/
void Line::setHandedness(const Handedness &h)
{
  _hand = h;
  _has_hand = true;
}

/*----------------------------------------------------------------*/
void Line::removeHandedness(void)
{
  _has_hand = false;
  _hand = Handedness();
}

/*----------------------------------------------------------------*/
// get the 'average' handedness (a new object is created).
Handedness Line::averageHandedness(const Line &l) const
{
  Handedness h;
  if (_has_hand)
  {
    h = _hand;
    if (l._has_hand)
      h.average(l._hand);
  }
  else
  {
    if (l._has_hand)
      h = l._hand;
    else
      h = Handedness(Handedness::UNKNOWN);
  }
  return h;
}

/*----------------------------------------------------------------*/
void Line::adjustMotionDirection(void)
{
  double angle;
  MotionVector mv;
  if (!getMotionVector(mv))
    return;

  /*
   * make the line horizontal to get the rotation angle
   */
  Line vr(*this);
  vr.makeHorizontal(angle, false);
 
  // adjust the vel.
  mv.adjustDirection(angle);
  setMotionVector(mv);
}

/*----------------------------------------------------------------*/
void Line::adjustMotionDirection(const Line &s)
{
  MotionVector mv;
  double angle;
  if (!getMotionVector(mv))
    return;

  // rotate so s is horizontal.
  Line vr(s);
  vr.makeHorizontal(angle, false);
 
  // adjust the vel so it's perpendicular to angle.
  mv.adjustDirection(angle);
  setMotionVector(mv);

  // hopefully that helped, now just do the adjustment relative
  // to *this
  adjustMotionDirection();
}

/*----------------------------------------------------------------*/
double Line::centerpointDistance(const Line &l1) const
{
  double lx0, ly0, lx1, ly1, dx, dy, d;

  lx0 = (double)(_x0 + _x1)/2.0;
  ly0 = (double)(_y0 + _y1)/2.0;

  lx1 = (double)(l1._x0 + l1._x1)/2.0;
  ly1 = (double)(l1._y0 + l1._y1)/2.0;
    
  dx = lx0 - lx1;
  dy = ly0 - ly1;
  d = dx*dx + dy*dy;
  return sqrt(d);
}

/*----------------------------------------------------------------*/
void Line::setVelToMatchHandedness(double speed)
{
  double angle, vx, vy;
  Handedness::e_hand_t e;
  MotionVector mv;

  if (Math::verySmall(speed))
  {
    mv = MotionVector(0, 0);
    setMotionVector(mv);
    return;
  }

  if (!_has_hand)
    e = Handedness::UNKNOWN;
  else
    e = _hand.getType();

  /*
   * Get the angle of the vector.
   */
  angle = vectorAngleFromEnd(0);

  /*
   * Adjust that angle based on handedness
   */
  if (e == Handedness::RIGHT)
    angle = Math::rightRotation(angle);
  else if (e == Handedness::LEFT)
    angle = Math::leftRotation(angle);
  else
  {
    mv = MotionVector(0, 0);
    setMotionVector(mv);
    return;
  }
  /*
   * Build the vector velocity
   */
  angle = angle*3.14159/180.0;
  vx = cos(angle)*speed;
  vy = sin(angle)*speed;
  mv = MotionVector(vx, vy);
  setMotionVector(mv);
}

/*----------------------------------------------------------------*/
void Line::adjustVelWithHandedness(double speed)
{
  double s;
  if (Math::verySmall(speed))
    speed = 0.0;
    
  if (!getMotionSpeed(s))
    return;
  if (Math::verySmall(s))
    // adjust to speed based on handedness
    setVelToMatchHandedness(speed);
  else
  {
    MotionVector mv;
    if ( getMotionVector(mv))
    {
      mv.scale(speed/s);
      setMotionVector(mv);
    }
  }
}

/*----------------------------------------------------------------*/
void Line::adjustForData(const Grid2d &data)
{
  PointList xy = xyValues();
  DataAtt d(xy, data);
  setDataAtt(d);
}

/*----------------------------------------------------------------*/
double Line::minimumAngleBetween(const Line &l1) const
{
  Line lr0, lr1;
  double angle;
    
  _rotatePair(l1, lr0, lr1, false);
  if (lr1._is_vertical)
    angle = 90.0;
  else
  {
    angle = atan2(lr1._slope, 1.0)*180.0/3.14159;
    if (angle < 0.0)
      angle = -angle;
    if (angle > 90.0)
      angle = 180.0 - angle;
  }
  return angle;
}

/*----------------------------------------------------------------*/
bool Line::isAheadOfHorizontal(const Line &line, double &gap) const
{
  bool stat;
    
  if (_x0 <= _x1)
  {
    if (line._x0 >= _x1 && line._x1 >= _x1)
    {
      if (line._x0 <= line._x1)
	gap = line._x0 - _x1;
      else
	gap = line._x1 - _x1;
      stat = true;
    }
    else
    {
      gap = 0.0;
      stat = false;
    }
  }
  else
  {
    if (line._x0 <= _x1 && line._x1 <= _x1)
    {
      if (line._x0 >= line._x1)
	gap = _x1 - line._x0;
      else
	gap = _x1 - line._x1;
      stat = true;
    }
    else
    {
      gap = 0.0;
      stat = false;
    }
  }
  return stat;
}

/*----------------------------------------------------------------*/
bool Line::isBehind(const Line &line, double &gap) const
{
  Line lr0, lr1;
  
  _rotatePair(line, lr0, lr1, false);
  return lr0.isBehindHorizontal(lr1, gap);
}

/*----------------------------------------------------------------*/
bool Line::isBehindHorizontal(const Line &line, double &gap) const
{
  bool stat;
    
  if (_x0 <= _x1)
  {
    if (line._x0 <= _x1 && line._x1 <= _x1)
    {
      if (line._x0 >= line._x1)
	gap = _x1 - line._x0;
      else
	gap = _x1 - line._x1;
      stat = true;
    }
    else
    {
      gap = 0.0;
      stat = false;
    }
  }
  else
  {
    if (line._x0 >= _x1 && line._x1 >= _x1)
    {
      if (line._x0 <= line._x1)
	gap = line._x0 - _x1;
      else
	gap = line._x1 - _x1;
      stat = true;
    }
    else
    {
      gap = 0.0;
      stat = false;
    }
  }
  return stat;
}

/*----------------------------------------------------------------*/
double Line::bestVectorAngle(const Line &l1, double alpha1) const
{
  double angle0, angle1, angle;

  angle0 = vectorAngleFromEnd(0);
    
  /*
   * Determine two possible directions out of line,
   * and see which one is best, as well as which one is possible
   */
  if (Math::small(alpha1))
  {
    /*
     * intersection is at 1th endpoint of l1, only can go
     * as a vector from 1 to 0 out of l1.
     */
    angle1 = l1.vectorAngleFromEnd(1);
    angle = fabs(angle1 - angle0);
    if (angle > 180.0)
      angle = 360.0 - angle;
  }
  else if (Math::close(1.0, alpha1))
  {
    /*
     * intersection at 0th endpoint of line, only can go
     * from 0 to 1 out of l1.
     */
    angle1 = l1.vectorAngleFromEnd(0);
    angle = fabs(angle1 - angle0);
    if (angle > 180.0)
      angle = 360.0 - angle;
  }
  else
    angle = bestVectorAngle(l1);
  return angle;
}

    
/*----------------------------------------------------------------*/
double Line::bestVectorAngle(const Line &l1) const
{
  double a0, a10, a11;
  
  a0 = vectorAngleFromEnd(0);

  a10 = l1.vectorAngleFromEnd(0);
  a10 = fabs(a10 - a0);
  if (a10 > 180.0)
    a10 = 360.0 - a10;

  a11 = l1.vectorAngleFromEnd(1);
  a11 = fabs(a11 - a0);
  if (a11 > 180.0)
    a11 = 360.0 - a11;

  if (a11 < a10)
    return a11;
  else
    return a10;
}

/*----------------------------------------------------------------*/
void Line::averageCommon0(const Line &v1)
{
  double x00, y00, len0;
  double x10, y10, len1, l, vx, vy;
  MotionVector v;

    
  x00 = _x0;
  y00 = _y0;

  x10 = v1._x0;
  y10 = v1._y0;
    
  if (!Math::close(x00, x10) || !Math::close(y00, y10))
  {
    LOG(ERROR) << "";
    return;
  }
    
  /*
   * Find average direction
   */
  v = _averageDirection(v1, 0);
  vx = v.getVx();
  vy = v.getVy();
    
  /*
   * Find longest length
   */
  len0 = length();
  len1 = v1.length();
  if (len0 > len1)
    l = len0;
  else 
    l = len1;
    

  /*
   * Build line from point 0 of length l in direction dx, dy.
   * First save attributes of line before overwriting
   */
  AttributesEuclid this_att = *this;
  *this = Line(x00, y00, x00 + vx*l, y00 + vy*l);

  /*
   * Take average of motion and quality, everything else stays the same.
   */
  MotionVector mv;
  double q;
  if (this_att.averageMotionVector(v1, mv))
  {
    this->setMotionVector(mv);
  }
  if (this_att.averageQuality(v1, q))
  {
    this->setQuality(q);
  }
}

/*----------------------------------------------------------------*/
void Line::lengthAveragedVel(const Line &other)
{
  MotionVector mv;
  if (other.getMotionVector(mv))
  {
    double l1 = other.length();
    lengthAveragedVel(mv, l1);
  }
}

/*----------------------------------------------------------------*/
void Line::lengthAveragedVel(const MotionVector &mv1, double l1)
{
  MotionVector mv;
  if (getMotionVector(mv))
  {
    double vx0 = mv.getVx();
    double vy0 = mv.getVy();
    double l0 = length();

    if (Math::verySmall(l0 + l1))
    {
      return;
    }

    double vx1 = mv1.getVx();
    double vy1 = mv1.getVy();
    double s0 = sqrt(vx0*vx0 + vy0*vy0);
    double s1 = sqrt(vx1*vx1 + vy1*vy1);
    if (Math::verySmall(s0))
    {
      LOG(WARNING) << "Not yet implemented..average vel when seed is 0";
      return;
    }
    double s = (s0*l0 + s1*l1)/(l0 + l1);
    vx0 *= s/s0;
    vy0 *= s/s0;
    setMotionVector(MotionVector(vx0, vy0));
  }
}

/*----------------------------------------------------------------*/
void Line::checkForEndpointReverse(const Line &l1, double thresh,
				      bool &reverse0, bool &reverse1) const
{
  double d00, d01, d10, d11;

  reverse0 = false;
  reverse1 = false;

  /*
   * Get endpoint distances (0 to 0, 0 to 1, 1 to 0, 1 to 1)
   * (l0 is this)
   */
  d00 = ((_x0 - l1._x0)*(_x0 - l1._x0) +
	 (_y0 - l1._y0)*(_y0 - l1._y0));
  d01 = ((_x0 - l1._x1)*(_x0 - l1._x1) +
	 (_y0 - l1._y1)*(_y0 - l1._y1));
  d11 = ((_x1 - l1._x1)*(_x1 - l1._x1) +
	 (_y1 - l1._y1)*(_y1 - l1._y1));
  d10 = ((_x1 - l1._x0)*(_x1 - l1._x0) +
	 (_y1 - l1._y0)*(_y1 - l1._y0));

  // first check if 1th endpoint of l0 is about the same as 0th
  // endpoint of l1
  if (Math::verySmall(d10))
    // yes..no reversing needed.
    return;
    
  // next see if one is way less than the others.
  // if so, set the reverse flags appropriately.
  if (d00 < d01 && d00 < d10 && d00 < d11)
  {
    if (d00/d01 < thresh && d00/d10 < thresh && d00/d11 < thresh)
      /*
       * 0 is closest to 0...make 0'th endpoint of 0th line
       * the 1th.
       */
      reverse0 = true;
  }
  else if (d01 < d00 && d01 < d10 && d01 < d11)
  {
    if (d01/d00 < thresh && d01/d10 < thresh && d01/d11 < thresh)
    {
      /*
       * 0th endpoint from 0th line is closest to
       * 1th endpoint of 1th line...reverse endpoints of
       * both lines
       */
      reverse0 = true;
      reverse1 = true;
    }
  }
  else if (d11 < d01 && d11 < d10 && d11 < d00)
  {
    if (d11/d01 < thresh && d11/d10 < thresh && d11/d00 < thresh)
      /*
       * 1th endpoint of 1th line is closest to 0th line.
       * want to reverse 1th lines endpoints
       */
      reverse1 = true;
  }
}

/*----------------------------------------------------------------*/
void Line::fillVelWithBestSpeed(const MotionVector &best_dir, double speed)
{
  // get the best speed to use. store that as the vel attribute.
  MotionVector v = _bestSpeed(best_dir, speed);
  setMotionVector(v);
}

/*----------------------------------------------------------------*/
void Line::fillBestSpeedAndMove(const Line &lcopy, double dt)
{
  double speed;
  if (!getMotionSpeed(speed))
    setMotionVector(MotionVector(0, 0));
  else
  {
    if (Math::verySmall(speed))
      // give it zero speed, low quality, and don't move it.
      setMotionVector(MotionVector(0, 0));
    else
    {
      // get best direction from lcopy.
      MotionVector v;
      if (lcopy.getMotionVector(v))
      {
	// motion length is now computed from speed.
	// put same units as input into our new speed.
	MotionVector vnew = _bestSpeed(v, speed);
	setMotionVector(vnew);
      }

      // move if no motion vector in lcopy, extrapolate using existing
      // motion vector
      extrapolate(dt);
    }
  }
}

/*----------------------------------------------------------------*/
void Line::_init(void)
{
  _x0 = _x1 = 0;
  _y0 = _y1 = 0;
  _is_vertical = false;
  _slope = 0.0;
  _intercept = 0.0;
  _is_bad = true;
  _has_endpts = false;
  _has_hand = false;
}

/*----------------------------------------------------------------*/
void Line::_rotatePoint(double &x, double &y, double angle)
{
  double a, cosa, sina, x0, y0;
    
  a = angle*3.14159/180.0;
  cosa = cos(a);
  sina = sin(a);
  x0 = (x)*cosa + (y)*sina;
  y0 = -(x)*sina + (y)*cosa;
  x = x0;
  y = y0;
}

/*----------------------------------------------------------------*/
/*
 * We want minimum y to be point 0
 * Set line so that it has endpoints on the input box boundary.
 * at the two crossing points.
 */
void Line::_fillBounds(const Box &b)
{
  double xx0, yy0, xx1, yy1;
  double minx, miny, maxx, maxy;
  int numok;
  bool yy0ok, yy1ok, xx0ok, xx1ok;

  minx = b._minx;
  miny = b._miny;
  maxx = b._maxx;
  maxy = b._maxy;
    
  /*
   * Find intersection of bounding box with line
   */
  yy0 = minx*_slope + _intercept; // y value at minx
  yy1 = maxx*_slope + _intercept; // y value at maxx
  xx0 = (miny-_intercept)/_slope; // x value at miny
  xx1 = (maxy-_intercept)/_slope; // x value at maxy

  /*
   * Hopefully, two of these points are outside the box and two are
   * crossing the box..check for that.
   */
  numok = 0;
  if ((yy0ok = (yy0 >= miny && yy0 <= maxy)))
    ++numok;
  if ((yy1ok = (yy1 >= miny && yy1 <= maxy)))
    ++numok;
  if ((xx0ok = (xx0 >= minx && xx0 <= maxx)))
    ++numok;
  if ((xx1ok = (xx1 >= minx && xx1 <= maxx)))
    ++numok;
  if (numok < 2)
  {
    LOG(ERROR) << "unexpected siutation";
    makeBad();
    return;
  }
  else if (numok == 2)
    _fillBoundsNonCorner(minx, miny, maxx, maxy, yy0, yy1, xx0, xx1);
  else if (numok == 3)
    _fillBoundsOneCorner(minx, miny, maxx, maxy, yy0, yy1, xx0, xx1,
			 yy0ok, yy1ok, xx0ok, xx1ok);
  else // numok = 4, take any two different points.
  {
    _x0 = minx;
    _y0 = yy0;
    if (yy1 != yy0)
    {
      _y1 = yy1;
      _x1 = maxx;
    }
    else if (xx0 != minx)
    {
      _x1 = xx0;
      _y1 = miny;
    }
    else if (xx1 != minx)
    {
      _x1 = xx1;
      _y1 = maxy;
    }
    else
    {
      LOG(ERROR) << "in setting up 4 intersection bounds";
      makeBad();
      return;
    }
    _setOtherValues();
  }
    
  if (_y1 < _y0)
    /*
     * REverse so _y0 < _y1
     */
    reverse();
}



/*----------------------------------------------------------------*/
void Line::_fillBoundsNonCorner(double minx, double miny,
				   double maxx, double maxy,
				   double yy0, double yy1,
				   double xx0, double xx1)
{
  /*
   * Check if minimum y line is one crossing.
   */
  if (yy0 >= miny && yy0 <= maxy)
  {
    /*
     * Yes...store minx,yy0 as one of the two points
     */
    _x0 = minx;
    _y0 = yy0;

    /*
     * Other crossing is either minx, maxx, or maxy
     */
    if (yy1 >= miny && yy1 <= maxy)
    {
      /*
       * Yes..store maxx,yy1 as next point
       */
      _x1 = maxx;
      _y1 = yy1;
    }
    else
    {
      if (xx0 >= minx && xx0 <= maxx)
      {
	_x1 = xx0;
	_y1 = miny;
      }
      else if (xx1 >= minx && xx1 <= maxx)
      {
	_x1 = xx1;
	_y1 = maxy;
      }
      else
      {
	LOG(ERROR) << "setting min/max in";
	makeBad();
	return;
      }
    }
  }
  else
  {
    /*
     * How about minimum x line?
     */
    if (xx0 >= minx && xx0 <= maxx)
    {
      /*
       * Yes..store xx0,miny as one of the two points
       */
      _x0 = xx0;
      _y0 = miny;

      /*
       * Other crossing is either maxx, or maxy
       */
      if (yy1 >= miny && yy1 <= maxy)
      {
	_x1 = maxx;
	_y1 = yy1;
      }
      else if (xx1 >= minx && xx1 <= maxx)
      {
	_x1 = xx1;
	_y1 = maxy;
      }
      else
      {
	LOG(ERROR) << "logic setting min/max in INE_create";
	makeBad();
	return;
      }
    }
    else
    {
      /*
       * Expect crossing along maximum x and maximum y lines
       */
      _x0 = xx1;
      _y0 = maxy;
      _x1 = maxx;
      _y1 = yy1;
      if (_x0 > maxx || _x0 < minx || _x1 > maxx || _x1 < minx ||
	  _y0 > maxy || _y0 < miny ||	_y1 > maxy || _y1 < miny)
      {
	LOG(ERROR) << "logic setting min/max in LINE_create";
	makeBad();
	return;
      }
    }
  }
  _setOtherValues();
}

/*----------------------------------------------------------------*/
void Line::_fillBoundsOneCorner(double minx, double miny, double maxx,
				   double maxy, double yy0, double yy1,
				   double xx0, double xx1, bool yy0ok,
				   bool yy1ok, bool xx0ok, bool xx1ok)
{
  // hitting one corner..get the one not ok.
  if (!xx0ok)
  {
    // one crossing must be at xx1
    _x0 = xx1;
    _y0 = maxy;
	    
    // the other must be different.
    if (yy0 == maxy)
    {
      _y1 = yy1;
      _x1 = maxx;
    }
    else
    {
      _y1 = yy0;
      _x1 = minx;
    }
  }
  else if (!xx1ok)
  {
    // one crossing must be at xx0
    _x0 = xx0;
    _y0 = miny;
	    
    // the other must be different.
    if (yy0 == miny)
    {
      _y1 = yy1;
      _x1 = maxx;
    }
    else
    {
      _y1 = yy0;
      _x1 = minx;
    }
  }
  else if (!yy0ok)
  {
    // one crossing must be at yy1
    _x0 = maxx;
    _y0 = yy1;
	    
    // the other must be different.
    if (xx0 == maxx)
    {
      _x1 = xx1;
      _y1 = maxy;
    }
    else
    {
      _x1 = xx0;
      _y1 = miny;
    }
  }
  else
  {
    // one crossing must be at yy0
    _x0 = minx;
    _y0 = yy0;
	    
    // the other must be different.
    if (xx0 == minx)
    {
      _x1 = xx1;
      _y1 = maxy;
    }
    else
    {
      _x1 = xx0;
      _y1 = miny;
    }
  }
  _setOtherValues();
}

/*----------------------------------------------------------------*/
double Line::_minDistanceSlopedSquared(double x, double y) const
{
  double b0, b1, yy0, yy1, xbottom, ybottom, xtop, ytop;
  double bb, temp, xx, yy, dist;
  bool reverse, between;
    
  // the equation of the two perpendicular lines to y = mx + b
  // through the two endpoints:  y = -1/m*x + b0,  y = -1/m*x + b1
  // where b0 = _y0 + 1/m*_x0,   b1 = _y1 + 1/m*_x1

  // at a given x,y  the point is between the these two lines if:
  // y is inbetween the two lines above:

  b0 = _y0 + _x0/_slope;
  b1 = _y1 + _x1/_slope;
  reverse = (b1 < b0);
  if (!reverse)
  {
    yy0 = -x/_slope + b0;
    yy1 = -x/_slope + b1;
    xbottom = _x0;
    ybottom = _y0;
    xtop = _x1;
    ytop = _y1;
  }
  else
  {
    yy0 = -x/_slope + b1;
    yy1 = -x/_slope + b0;
    xbottom = _x1;
    ybottom = _y1;
    xtop = _x0;
    ytop = _y0;
    temp = b0;
    b0 = b1;
    b1 = temp;
  }
  between = (yy0 <= y && y <= yy1);
  if (between)
  {
    // distance = perpendiclar (to line) between x,y and the line
    // intersection point = xx, yy where:
    //    yy = -1/m*xx + bb
    //    yy = m*xx + b
    // bb = y + x/m
    bb = y + x/_slope;
    xx = (bb - _intercept)/(_slope + 1/_slope);
    yy = (bb - _intercept)/(_slope + 1/_slope)*_slope + _intercept;
    dist = (x-xx)*(x-xx) + (y-yy)*(y-yy);
  }
  else
  {
    if (y < yy0)
      // below bottom line.
      dist = (x-xbottom)*(x-xbottom) + (y-ybottom)*(y-ybottom);
    else
      // above top line.
      dist = (x-xtop)*(x-xtop) + (y-ytop)*(y-ytop);
  }
  return dist;
}

/*----------------------------------------------------------------*/
// return minimum distance to *horizontal* line
double Line::_minDistanceHorizontalSquared(double x, double y) const 
{
  double dx, dy, d, xmin, xmax, ymin, ymax;
  
  if (_x0 <= _x1)
  {
    xmin = _x0;
    ymin = _y0;
    xmax = _x1;
    ymax = _y1;
  }
  else
  {
    xmin = _x1;
    ymin = _y1;
    xmax = _x0;
    ymax = _y0;
  }

  /*
   * 3 cases
   */
  if (x < xmin)
  {
    /*
     * Point left of line..shorteest dist. is to
     * 0'th endpoint of line.
     */
    dx = x - xmin;
    dy = y - ymin;
    d = dx*dx + dy*dy;
  }
  else if (x > xmax)
  {
    /*
     * Point right of line..shorteest dist. is to
     * 1'th endpoint of line.
     */
    dx = x - xmax;
    dy = y - ymax;
    d = dx*dx + dy*dy;
  }
  else
  {
    /*
     * the point lies directly above or below line.
     * the minimum is the perpendicular distance.
     * (difference in y).
     */
    d = fabs(y - _y0);
    d = d*d;
  }
  return d;
}

/*----------------------------------------------------------------*/
// return minimum distance to *vertical* line
double Line::_minDistanceVerticalSquared(double x, double y) const 
{
  double dx, dy, d, xmin, xmax, ymin, ymax;
  
  if (_y0 <= _y1)
  {
    xmin = _x0;
    ymin = _y0;
    xmax = _x1;
    ymax = _y1;
  }
  else
  {
    xmin = _x1;
    ymin = _y1;
    xmax = _x0;
    ymax = _y0;
  }

  /*
   * 3 cases
   */
  if (y < ymin)
  {
    /*
     * Point below line..shorteest dist. is to
     * 0'th endpoint of line.
     */
    dx = x - xmin;
    dy = y - ymin;
    d = dx*dx + dy*dy;
  }
  else if (y > ymax)
  {
    /*
     * Point above line..shorteest dist. is to
     * 1'th endpoint of line.
     */
    dx = x - xmax;
    dy = y - ymax;
    d = dx*dx + dy*dy;
  }
  else
  {
    /*
     * the point lies directly left or right .
     * the minimum is the horiz. dist. (difference in x).
     */
    d = fabs(x - _x0);
    d = d*d;
  }
  return d;
}

/*----------------------------------------------------------------*/
bool Line::_pointIsGreater(double x, double y) const
{
  double yline;
  
  if (_is_vertical)
    return (x >= _x0);
  else
  {
    yline = x*_slope + _intercept;
    return (y >= yline);
  }
}

/*----------------------------------------------------------------*/
// replace motion vector with one that is perp to line
// most similar to input motion, with magnitude set to input.
void Line::_replaceMotionWithSimilarMotion(double angle, double mag,
					   bool debug)
{
  double aline, d, vx, vy;
  if (fabs(mag) < 1.0e-6)
  {
    vx = vy = 0.0;
  }
  else
  {
    aline = degreesSloped0180();
    // try adding 90
    aline += 90.0;
    while (aline > 360.0)
      aline -= 360.0;
    d = angle - aline;
    while (d > 360.0)
      d -= 360.0;
    while (d < 0.0)
      d += 360.0;
    if (d >= 90 && d <= 270)
      aline -= 180.0;
    aline = aline*3.14159/180.0;
    vx = mag*cos(aline);
    vy = mag*sin(aline);
  }
  if (debug) printf("Similar motion = vx,vy=%f,%f\n", vx, vy);

  MotionVector v(vx, vy);
  setMotionVector(v);
}


/*----------------------------------------------------------------*/
void Line::_rotatePair(const Line &l1, Line &l0r, 
			Line &l1r, bool change_endpts) const
{
  double angle;
  
  l0r = Line(*this);
  l0r.makeHorizontal(angle, change_endpts);
  l1r = Line(l1);
  l1r.rotate(angle, change_endpts);
}

/*----------------------------------------------------------------*/
MotionVector Line::_averageDirection(const Line &l1, int which) const
{
  MotionVector v0, v1;

  double x,  y;
  unitVectorFromEndpt(which, x, y);
  v0 = MotionVector(x, y);
  
  l1.unitVectorFromEndpt(which, x, y);
  v1 = MotionVector(x, y);

  v0.average(v1);
  return v0;
}

/*----------------------------------------------------------------*/
// determine best speed that is perpendicular to the line, given inputs.
MotionVector Line::_bestSpeed(const MotionVector &best_unit_dir,
			       double speed) const
{
  double angle, y;
  
  /*
   * Rotate line to horiz to get an angle.
   */
  Line line(*this);;
  line.makeHorizontal(angle, true);

  /*
   * Rotate best_unit_dir by same angle. y component indidates up or down.
   */
  MotionVector v(best_unit_dir);
  v.rotate(angle, false);
  y = v.getVy();
 
  /*
   * If y component is positive, best dir is positive in rotated
   * coordinates, else vice versa.
   */
  if (y >= 0.0)
    v = MotionVector(0.0, speed);
  else
    v = MotionVector(0.0, -speed);

  /*
   * Rotate vel back to original orientation
   */
  v.rotate(-angle, false);
  return v;
}

