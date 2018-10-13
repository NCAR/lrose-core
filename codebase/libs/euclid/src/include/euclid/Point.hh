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
/**
 * @file Point.hh
 * @brief 2 dimensional location (point), with attributes.
 * @class Point
 * @brief 2 dimensional location (point), with attributes.
 *
 * Simple pixel x,y location, also can be interpreted as a vector for some uses.
 */
# ifndef    Point_H
# define    Point_H

#include <euclid/AttributesEuclid.hh>
#include <cmath>
#include <string>

class Grid2d;

class Point : public AttributesEuclid
{
 public:

  /**
   * Empty, no point, no attributes
   */
  Point();

  /**
   * A point with no attributes
   * @param[in] x
   * @param[in] y
   */
  Point(double x, double y);

  /**
   *  Create a 'vector' in direction unit, of length/scale, i.e.
   *    -  _x = length*unit._x/scale._x;
   *    -  _y = length*unit._y/scale._y;
   *
   * @param[in] length  Length of created object (as a vector)
   * @param[in] scale  Scale factor to divide created object by
   * @param[in] unit  A unit vector (length=1)  
   */
   Point(double length, const Point &scale, const Point &unit);

  /**
   * Create input point, with a scaled offset (return v + off*scale), no attributes
   *
   * @param[in] v  Initial point
   * @param[in] off  Offset point
   * @param[in] scale  Scale factor
   */
  Point(const Point &v, const Point &off, double scale);


  /**
   * Destructor
   */
  virtual ~Point();

  /**
   * Copy
   * @param[in] p
   */
  Point(const Point &p);

  /**
   * operator=
   * @param[in] p
   */
  Point & operator=(const Point &p);

  /**
   * operator==
   * @param[in] p
   */
  bool operator==(const Point &p) const;

  /**
   * @return local object + input object, x and y
   * @param[in] c
   */
  Point operator+(const Point &c) const;

  /**
   * @return local object - input object, x and y
   * @param[in] c
   */
  Point operator-(const Point &c) const;

  /**
   * @return local object / input object, both x and y
   * @param[in] c
   */
  Point operator/(const Point &c) const;

  /**
   * @return local object * input object, both x and y
   * @param[in] c
   */
  Point operator*(const Point &c) const;

  /**
   * @return the average of the input and local object, both x and y
   * @param[in] c
   */
  Point ave(const Point &c) const;

  /**
   * @return true if no point
   */
  inline bool is_empty(void) const
  {
    return !_ok;
  }

  /**
   * Create an XML string that represents the Point
   * including attributes
   * @param[in] tag  Tag to surround with
   */
  std::string writeXml(const std::string &tag) const;

  /**
   * Parse an XML string to set values for the Point
   * including attributes
   * @param[in] xml  The data
   * @return true for success
   */
  bool readXml(const std::string &xml);

  /**
   * Debug print
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * Debug print
   * @return string describing the point
   */
  std::string sprint(void) const;

  /**
   * @return x value
   */
  inline double getX(void) const {return _x;}

  /**
   * @return y value
   */
  inline double getY(void) const {return _y;}

  /**
   * @return x value cast to int
   */
  inline int getIntX(void) const {return static_cast<int>(_x);}

  /**
   * @return y value cast to int
   */
  inline int getIntY(void) const {return static_cast<int>(_y);}

  /**
   * @return length of _x,_y interpreted as a vector
   */
  double length(void) const;

  /**
   * @return distance between input point and local point
   * @param[in] xy  The other point
   */
  inline double distanceBetween(const Point &xy) const
  {
    return sqrt((_x - xy._x)*(_x - xy._x) + 
		(_y - xy._y)*(_y - xy._y));
  }

  /**
   * modify local point to be the average of the input point and local point
   * @param[in] o2  The other point
   */
  inline void average(const Point &o2) 
  {
    _x = (o2._x + _x)/2.0;
    _y = (o2._y + _y)/2.0;
  }


  /**
   * modify local point to be the sum of the input point and local point
   * @param[in] o2  The other point
   */
  inline void add(const Point &o2)
  {
    _x = (o2._x + _x);
    _y = (o2._y + _y);
  }

  /**
   * modify local point, add to x and y
   * @param[in] dx  Value to add to x
   * @param[in] dy  Value to add to y
   */
  inline void add(double dx, double dy)
  {
    _x += dx;
    _y += dy;
  }

  /*
   * Set x,y values for local point to inputs
   * @param[in] x
   * @param[in] y
   */
  inline void set(double x, double y)
  {
    _x = x;
    _y = y;
  }

  /**
   * @return the orientation difference between the local point
   * and the input point,where each points orientation is relative to 0,0
   *
   * @param[in] xy  Input point
   *
   * Value returned is between [-180, 180]
   */
  double angleBetween(const Point &xy) const;

  /**
   * Rotate the local point by an angle, where the rotation is relative to
   * 0,0
   *
   * @param[in] angle  Degrees
   */
  void rotate(double angle);

  /**
   * Divide local object by input number, x and y
   * @param[in] d  Better not be 0.0
   */
  inline void divide(const double d)
  {
    _x = _x/d;
    _y = _y/d;
  }

  /**
   * Multiply local object by input number, x and y
   * @param[in] d
   */
  inline void mult(const double d)
  {
    _x = _x*d;
    _y = _y*d;
  }


  /**
   * Set _x to 1.0/_x and _y to 1.0/_y
   */
  inline void invert(void)
  {
    _x = 1.0/_x;
    _y = 1.0/_y;
  }

  /**
   * Compute min/max in x,y between local object and pt1 and return.
   * @param[in] pt1 The other point
   * @param[out] dmin  Real valued minima in x,y
   * @param[out] dmax  Real valued maxima in x,y
   * @param[out] min   Integer values for minima, rounded to increase range
   * @param[out] max   Integer values for maxima, rounded to increase range
   *
   * @return false if no range in values at all
   */
  bool minMax(const Point &pt1, Point &dmin, Point &dmax, 
	      Point &min, Point &max) const;

  /**
   * @return true if data is missing at the the point, assuming _x and _y
   *         are grid indices
   *
   * @param[in] grid  Where to look for missing at the point
   */
  bool isMissing(const Grid2d &grid) const;

  /**
   * @return true if point is in range in a grid,
   * assuming _x and _y are grid indices
   *
   * @param[in] nx  Number of x in grid
   * @param[in] ny  Number of y in grid
   */
  bool inGridRange(int nx, int ny) const;

  /**
   * Write value to grid at Point, assuming _x and _y are grid indices
   * @param[in,out] grid
   * @param[in] value
   */
  void toGrid(Grid2d &grid, double value) const;

  /**
   * @return a Point based in input x and y, but scaled so length() = 1
   * @param[in] x
   * @param[in] y
   */
  static Point unit(double x, double y);

  /**
   * @return a Point that represents a unit vector from p0 to p1
   * @param[in] p0
   * @param[in] p1
   */
  static Point unit(const Point &p0, const Point &p1);

  /**
   * @return a Point that is rotated 90 degrees compared to input point
   * @param[in] v  The point
   */
  static Point perpendicular(const Point &v);



 private:

  double _x;  /**< X value */
  double _y;  /**< Y value */
  bool _ok;   /**< True if _x and _y are set */
};

# endif

