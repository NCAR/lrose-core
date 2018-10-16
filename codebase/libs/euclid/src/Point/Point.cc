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
#include <euclid/Point.hh>
#include <euclid/Grid2d.hh>
#include <rapmath/Math.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaXml.hh>
#include <cstdio>
#include <cmath>

using std::string;

/*----------------------------------------------------------------*/
Point::Point() : AttributesEuclid()
{
  _x = 0.0;
  _y = 0.0;
  _ok = false;
}

/*----------------------------------------------------------------*/
Point::Point(double ix, double iy) : AttributesEuclid()
{
  _x = ix;
  _y = iy;
  _ok = true;
}

//----------------------------------------------------------------
Point::Point(double length, const Point &scale, const Point &unit)
{
  _x = length*unit._x/scale._x;
  _y = length*unit._y/scale._y;
}

/*----------------------------------------------------------------*/
Point::Point(const Point &v, const Point &off, double scale)
{
  _x = v._x + off._x*scale;
  _y = v._y + off._y*scale;
}

/*----------------------------------------------------------------*/
Point::~Point()
{
}

/*----------------------------------------------------------------*/
Point::Point(const Point &p) : AttributesEuclid(p)
{
  _x = p._x;
  _y = p._y;
  _ok = p._ok;
}

/*----------------------------------------------------------------*/
Point & Point::operator=(const Point &p)
{
  if (&p == this)
  {
    return *this;
  }
  _x = p._x;
  _y = p._y;
  _ok = p._ok;
  Attributes::operator=(p);
  return *this;
}

/*----------------------------------------------------------------*/
bool Point::operator==(const Point &p) const
{
  return (_x == p._x && _y == p._y && _ok == p._ok &&
	  Attributes::operator==(p));
}

/*----------------------------------------------------------------*/
Point Point::operator+(const Point &c) const
{
  Point res(_x + c._x, _y + c._y);
  return res;
}

/*----------------------------------------------------------------*/
Point Point::operator-(const Point &c) const
{
  Point res(_x - c._x, _y - c._y);
  return res;
}

/*----------------------------------------------------------------*/
Point Point::operator/(const Point &c) const
{
  Point res(_x/c._x, _y/c._y);
  return res;
}

/*----------------------------------------------------------------*/
Point Point::operator*(const Point &c) const
{
  Point res(_x*c._x, _y*c._y);
  return res;
}

/*----------------------------------------------------------------*/
Point Point::ave(const Point &c) const
{
  Point res((c._x + _x)/2.0, (c._y + _y)/2.0);
  return res;
}

/*----------------------------------------------------------------*/
string Point::writeXml(const std::string &tag) const
{
  string ret = TaXml::writeStartTag(tag, 0);
  ret += Attributes::writeAttXml("PointAttributes");
  ret += TaXml::writeDouble("PointX", 0, _x, "%.6lf");
  ret += TaXml::writeDouble("PointY", 0, _y, "%.6lf");
  ret += TaXml::writeBoolean("PointOk", 0, _ok);
  ret += TaXml::writeEndTag(tag, 0);
  return ret;
}

/*----------------------------------------------------------------*/
bool Point::readXml(const std::string &xml)
{
  if (!Attributes::readAttXml(xml, "PointAttributes"))
  {
    return false;
  }

  if (TaXml::readDouble(xml, "PointX", _x))
  {
    LOG(ERROR) << "Parsing PointX";
    return false;
  }

  if (TaXml::readDouble(xml, "PointY", _y))
  {
    LOG(ERROR) << "Parsing PoinxY";
    return false;
  }

  if (TaXml::readBoolean(xml, "PointOk", _ok))
  {
    LOG(ERROR) << "Parsing PointOx";
    return false;
  }
  return true;
}

/*----------------------------------------------------------------*/
void Point::print(FILE *fp) const
{
  printAtt(fp);
  fprintf(fp, "(%.2lf,%.2lf)\n", _x, _y);
}

/*----------------------------------------------------------------*/
string Point::sprint(void) const
{
  string ret = sprintAtt();
  char buf[100];
  sprintf(buf, "(%.2lf,%.2lf)", _x, _y);
  ret += buf;
  return ret;
}

/*----------------------------------------------------------------*/
double Point::length(void) const
{
  return sqrt(_x*_x + _y*_y);
}

/*----------------------------------------------------------------*/
double Point::angleBetween(const Point &xy) const
{
  double t0, t1, d;
  
  t0 = atan2(_y, _x)*180.0/3.14159;
  t1 = atan2(xy._y, xy._x)*180.0/3.14159;

  if ((d=fabs(t0-t1)) < 180.0)
    return d;
  return 360.0 - d;
}

/*----------------------------------------------------------------*/
void Point::rotate(double angle)
{
  Math::rotatePoint(_x, _y, angle);
}

/*----------------------------------------------------------------*/
bool Point::minMax(const Point &pt1, Point &dmin, Point &dmax, 
		   Point &min, Point &max) const
{
  if (_x < pt1._x)
  {
    min._x = ceil(_x);
    dmin._x = _x;
    max._x = floor(pt1._x);
    dmax._x = pt1._x;
  }
  else
  {
    min._x = ceil(pt1._x);
    dmin._x = pt1._x;
    max._x = floor(_x);
    dmax._x = _x;
  }

  if (_y < pt1._y)
  {
    min._y = ceil(_y);
    dmin._y = _y;
    max._y = floor(pt1._y);
    dmax._y = pt1._y;
  }
  else
  {
    min._y = ceil(pt1._y);
    dmin._y = pt1._y;
    max._y = floor(_y);
    dmax._y = _y;
  }

  // NOTES from original code, which was putting a line seg to a grid:
  //
  // First determine an
  // upper bound to the number of crossings of the line from x1,y1 to x2,y2
  // with the underlying grid. Note that maxx - minx may be -1 if the line
  // segment does not cross a vertical grid line. Similarly maxy -
  // miny may be -1.
  int n_pts = max._x - min._x + max._y - min._y + 2;
  return (n_pts > 0);
}

/*-----------------------------------------------------------------------*/
bool Point::isMissing(const Grid2d &grid) const
{
  return grid.isMissing((int)_x, (int)_y);
}

/*----------------------------------------------------------------*/
bool Point::inGridRange(int nx, int ny) const
{
  int ix = (int)_x;
  int iy = (int)_y;
  return ix >= 0 && ix < nx && iy >=0 && iy < ny;
}    

/*----------------------------------------------------------------*/
void Point::toGrid(Grid2d &grid, double value) const
{
  int ix = (int)_x;
  int iy = (int)_y;
  grid.setValue(ix, iy, value);
}

/*----------------------------------------------------------------*/
Point Point::unit(double x, double y)
{
  double len = sqrt(x*x + y*y);
  if (len > 1.0e-6)
  {
    Point res(x/len, y/len);
    return res;
  }
  else
  {
    Point res(0.0, 0.0);
    return res;
  }
}

/*----------------------------------------------------------------*/
Point Point::unit(const Point &p0, const Point &p1)
{
  return unit(p1._x - p0._x, p1._y - p0._y);
}

/*----------------------------------------------------------------*/
Point Point::perpendicular(const Point &v)
{
  double x = -v._y;
  double y = v._x;

  Point res(x, y);
  return res;
}
