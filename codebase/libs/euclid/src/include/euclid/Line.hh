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
 * @file Line.hh
 * @brief Line with 2 points
 * @class Line
 * @brief Line with 2 points
 *
 * A Line can have Attributes, as well as local EndPts and Handedness
 */
# ifndef LINE_H
# define LINE_H

#include <euclid/AttributesEuclid.hh>
#include <euclid/EndPts.hh>
#include <euclid/Handedness.hh>
#include <rapmath/Math.hh>
#include <cmath>
#include <vector>
#include <string>

class Box;
class PointList;
class Grid2d;
class MotionVector;

class Line : public AttributesEuclid
{

  friend class LineList;

public:

  /**
   * empty constructor.
   */
  Line(void);

  /**
   * Line from x0,y0 to x1,y1.
   *
   * @param[in] x0
   * @param[in] y0
   * @param[in] x1
   * @param[in] y1
   */
  Line(double x0, double y0, double x1, double y1);

  /**
   * Line centered at 0,0 at angle, length=len.
   *
   * @param[in] angle
   * @param[in] len
   */
  Line(double angle, double len);

  /**
   * Vertical or horizontal line through a point to a bounding box.
   *
   * @param[in] x  If is_vert, the x value for a vertical line, otherwise
   *               the y value for a horizontal line
   *                           
   * @param[in] b  Box to put line into
   * @param[in] is_vert  True for a vertical line, false for horizontal
   */
  Line(double x, const Box &b, bool is_vert);

  /**
   * Sloped line through point x0, y0 to a bounding box
   *
   * @param[in] slope The line slope
   * @param[in] x0  Point on line
   * @param[in] y0  Point on line
   * @param[in] b  Box to put line into
   */
  Line(double slope, double x0, double y0, const Box &b);

  /**
   * Line extending spread on each side of xc,yc, with slope or vertical.
   *
   * @param[in] xc  center point
   * @param[in] yc  center point
   * @param[in] slope  Slope unless is_vert=true
   * @param[in] spread  The distance to spread the line on each side of xc,yc
   * @param[in] is_vert
   * 
   */
  Line(double xc, double yc, double slope, double spread, bool is_vert);

  /**
   * copy constructor
   * @param[in] l
   */
  Line(const Line &l);

  /**
   * Destructor
   */
  virtual ~Line(void);

  /**
   * operator= method
   * @param[in] l
   */
  Line & operator=(const Line &l);

  /**
   * operator== method
   * @param[in] l
   */
  bool operator==(const Line &l) const;

  /**
   * Create an XML string that represents the Line
   * including attributes
   * @param[in] tag  Tag to surround with
   */
  std::string writeXml(const std::string &tag) const;

  /**
   * Parse an XML string that represents the Line to set state
   * including attributes
   * @param[in] xml
   */
  bool readXml(const std::string &xml);

  /**
   * Check for segment equality, without checking attributes
   * @return true if input line segment is the same as local one
   * @param[in] l
   */
  inline bool equalNoAttributes(const Line &l) const
  {
    return (_x0 == l._x0 && _y0 == l._y0 && 
	    _x1 == l._x1 && _y1 == l._y1 && 
	    _is_vertical == l._is_vertical &&
	    _slope == l._slope && _intercept == l._intercept);
  }

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * Debug print
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * Debug print
   */
  std::string sprint(void) const;

  /**
   * Return one of the two input points
   *
   * @param[in] which  0 to return x0,y0  otherwise return x1,y1
   * @param[out] x
   * @param[out] y
   */
  inline void refPoint(int which, double &x, double &y) const
  {
    if (which == 0)
    {
      x = _x0;
      y = _y0;
    }
    else
    {
      x = _x1;
      y = _y1;
    }
  }

  /**
   * @return true if line has no extent
   */
  inline bool isAPoint(void) const {return length() < 1.0e-6;}

  /**
   * @return true if no member values set
   */
  inline bool isBad(void) const
  {
    return (_is_bad);
  }

  /**
   * @return true if line is vertical
   */
  inline bool lineIsVertical(void) const {return _is_vertical;}

  /**
   * @return slope of line
   */
  inline double getSlope(void) const {return _slope;}

  /**
   * @return y intercept of line
   */
  inline double getIntercept(void) const {return _intercept;}

  /**
   * Change line so values are not set
   */
  inline void makeBad(void) {_is_bad = true;}

  /**
   * Reverse the endpoints
   */
  void reverse(void);

  /**
   * @return length of line
   */
  double length(void) const;

  /**
   * @return slope of line in degrees (-90,90]
   */
  inline double degreesSloped(void) const
  {
    if (_is_vertical)
      return 90.0;
    else
      return atan2(_slope, 1.0)*180.0/3.14159;  
  }

  /**
   * @return slope of line in degrees [0,180)
   */
  inline double degreesSloped0180(void) const
  {
    double d= degreesSloped();
    if (d < 0) d += 180.0;
    return d;
  }

  /**
   * @return endpoint of line segment.
   *
   * @param[in] which   0 to return x0,y0, otherwise return x1,y1
   * @param[out] x  X value
   * @param[out] y  Y value
   */
  inline void point(int which, double &x, double &y) const
  {
    if (which == 0)
    {
      x = _x0;
      y = _y0;
    }
    else
    {
      x = _x1;
      y = _y1;
    }
  }

  /**
   * compute and return  alpha for given x
   *
   * alpha = (x-_x1)/(_x0-_x1)
   *
   * @param[in] x
   * @param[out] alpha

   * @return true if alpha was computed, false if the line is vertical
   */
  inline bool parametricAtX(double x, double &alpha) const
  {
    double denom = (_x0 - _x1);
    if (Math::verySmall(denom))
      return false;
    else
    {
      alpha = (x - _x1)/denom;
      return true;
    }
  }

  /**
   * compute and return  alpha for given y
   *
   * alpha = (y-_y1)/(_y0-_y1)
   *
   * @param[in] y
   * @param[out] alpha

   * @return true if alpha was computed, false if the line is horizontal
   */

  inline bool parametricAtY(double y, double &alpha) const
  {
    double denom = (_y0 - _y1);
    if (Math::verySmall(denom))
      return false;
    else
    {
      alpha = (y - _y1)/denom;
      return true;
    }
  }
  
  /**
   * adjust alpha values that are close to 0 to 0.0 and close to 1 to 1.0.
   *
   * @param[in,out] alpha
   */
  void adjustBorderlineAlpha(double &alpha) const;

  /**
   * @return x value for given alpha, ,where x = alpha*_x0 + (1-alpha)*_x1
   * @param[in] alpha
   */
  inline double xAtParametric(double alpha) const
  {
    return _parametricValue(alpha, _x0, _x1);
  }

  /**
   * @return y value for given alpha, where y=alpha*_y0 + (1-alpha)*_y1
   * @param[in] alpha
   */
  inline double yAtParametric(double alpha) const
  {
    return _parametricValue(alpha, _y0, _y1);
  }

  /**
   * return x,y for given alpha parameter, where x=alpha*_x0+(1-alpha)*_x1 and
   * y = alpha*_y0 + (1-alpha)*_y1
   * @param[in] alpha
   * @param[out] x
   * @param[out] y
   */
  inline void parametricLocation(double alpha, double &x, double &y) const
  {
    x = xAtParametric(alpha);
    y = yAtParametric(alpha);
  }


  /**
   * @return y value on the line for a given x
   * @param[in] x
   * 
   * returns -99.99 when the line is vertical
   */
  inline double yAtX(double x) const
  {
    if (_is_vertical)
      return -99.99;  //weak!
    else
      return _slope*x + _intercept;
  }

  /**
   * @return x value along line segment for a given y
   * @param[in] y
   *
   * returns -99.99 when the line is horizontal
   */
  inline double xAtY(double y) const
  {
    if (_is_vertical)
      return _x0;
    else
    {
      if (fabs(_slope) < 1.0e-10)
	return -99.99; // weak again
      else
	return (y - _intercept)/_slope;
    }
  }

  /**
   * make the line horizontal by rotating
   *
   * @param[out] rotation_degrees   Return how much rotation
   * @param[in] change_endpt_order  If true, change endpoints as needed so that
   *                                _x0 <= _x1 after rotation
   */
  void makeHorizontal(double &rotation_degrees, const bool change_endpt_order);

  /**
   * Rotate the line by an angle
   *
   * @param[in] angle
   * @param[in] change_endpts  If true, change endpoints as needed so that
   *                            _x0 <= _x1 after rotation
   */
  void rotate(double angle, bool change_endpts);

  /**
   * Change the endpoints as needed so that (_x0,_y0) is closer to input point
   * than (_x1, _y1).
   *
   * @param[in] x0
   * @param[in] y0
   */
  void orderEndpts(double x0, double y0);

  /**
   * Return the point at center of line segement (average)
   * @param[out] x
   * @param[out] y
   */
  void centerpoint(double &x, double &y) const;

  /**
   * @return minimum of _x0 and _x1
   */
  double minX(void) const; 

  /**
   * @return maximum of _x0 and _x1
   */
  double maxX(void) const; 

  /**
   * @return minimum of _y0 and _y1
   */
  double minY(void) const; 

  /**
   * @return maximum of _y0 and _y1
   */
  double maxY(void) const; 

  /**
   * @return average distance squared from the intersection of a set of points
   * and a mask image to the centerpoint of a line, weighted by the data values
   * at the intersection points.  
   *
   * Return 0 if there is nothing in the average.
   *
   * @param[in] p  The points that can be used in the average
   * @param[in] mask  The mask image at which points that are used in the
   *                  average cannot be missing
   * @param[in] data  The data grid to use as weights when averaging.
   */
  double averageWeightedDistanceSquared(const PointList &p,
					const Grid2d &mask,
					const Grid2d &data) const;

  /**
   * return average distance between two lines, and input and the local
   * This is effectively the double integral  along both lines of
   * distances
   *
   * @param[in] l  Input line
   * @param[out] dist  The distance
   *
   * @return false if one or both lines have not extent.
   */
  bool averageLineDistance(const Line &l, double &dist) const;

  /**
   * @return the minimum distance from x,y to the line segment.
   * This is a perpendicular distance.
   *
   * @param[in] x
   * @param[in] y
   */
  double minDistance(double x, double y) const;

  /**
   * @return the minimum distance squared from x,y to the line segment.
   * This is a perpendicular distance.
   *
   * @param[in] x
   * @param[in] y
   */
  double minDistanceSquared(double x, double y) const;

  /**
   * @return the minimum distance to the local line from the centerpoint of the
   * input line, squared
   *
   * @param[in] t  The line
   */
  double minDistanceSquared(const Line &t) const; 

  /**
   * @return the minimum distance between the local and input lines, where
   * it is the minimum of 0 (for actual intersection) and the distance 
   * between each of the 4 combinations of endpoint distances
   *
   * @param[in] l1  Line
   */
  double minimumDistanceBetween(const Line &l1) const;

  /**
   * Replace a line with its bisector
   *
   * The bisector is that line segment that is some percentage from one
   * end of the line perpendicular to the line, and extends to the bounding box.
   *
   * @param[in] end_index 0 or 1
   * @param[in] percentage
   * @param[in] b  Bounding box
   */
  void bisector(int end_index, double percentage, const Box &b);

  /**
   * Replace a line with its bisector
   *
   * The bisector is that line segment that is some percentage from one
   * end of the line perpendicular to the line, and of total length len
   *
   * @param[in] end_index 0 or 1
   * @param[in] percentage
   * @param[in] len  
   */
  void bisector(int end_index, double percentage, double len);

  /**
   * Replace a line with the portion starting at end_index that is
   * percentage of the original length
   *
   * @param[in] end_index  0 or 1
   * @param[in] percentage
   */
  void bisect(int end_index, double percentage);


  /**
   * return a unit vector that starts at 'endpt' and points into the line
   * @param[in] endpt  0 or 1
   * @param[out] x  X component of unit vector
   * @param[out] y  Y component of unit vector
   */
  void unitVectorFromEndpt(int endpt, double &x, double &y) const;

  /**
   * Given a line and any unit vector, the direction of the unit vector
   * if it were on the line segment would point to one or the other side
   * of the line segment (unless it was oriented the same as the segment).
   *
   * Filter down a PointList to those points that are on the side of the
   * line indicated by the unit vector, where the line is assumed an extension
   * of the local line that is infinitely long.
   *
   * @param[in,out] l  Pointlist
   * @param[in] ux  Unit vector
   * @param[in] uy  Unit vector
   */
  void oneSideOfLine(PointList &l, const double ux, const double uy) const;

  /**
   * @return true if local and input Line intersect, and set x, y to
   * the intersection point if so
   *
   * @param[in] l1  input Line
   * @param[out] x  Intersection point
   * @param[out] y  Intersection point
   */
  bool intersect(const Line &l1, double &x, double &y) const;

  /**
   * If the line is close enough to (x,y), append the orientation of the  local
   * line and its weight attribute to a pair vector.
   * 
   * If the distance from (x,y) to the line is > sqrt(radiusSquared),
   * do nothing.
   *
   * @note assumes the line has an attribute associated with input name
   * which is the weight to use.
   *
   * @note The line orientation is degrees_sloped_0_180()
   *
   * @param[in] x  point to check how close to
   * @param[in] y  point to check how close to
   * @param[in] radius_squared  Maximum allowed square radius
   * @param[in] attribute_name  Expected 'weight' attribute name
   * @param[in,out] o Pairs to append to where first=orientation, second=weight
   */
  void
  appendOrientationsToVector(int x, int y, 
			     int radius_squared, 
			     const std::string &attribute_name,
			     std::vector<std::pair<double,double> > &o) const;

  /**
   * @return angle [0,360) of the vector which starts at one endpoint
   * of the Line is oriented along the line.
   *
   * @param[in] which  0 or 1
   */
  double vectorAngleFromEnd(int which) const;

  /**
   * Change the line endpoints to inputs
   *
   * @param[in] x0  New _x0 value
   * @param[in] y0  New _y0 value
   * @param[in] x1  New _x1 value
   * @param[in] y1  New _y1 value
   */
  inline void adjustEndpoints(double x0, double y0, double x1, double y1)
  {
    _x0 = x0;
    _y0 = y0;
    _x1 = x1;
    _y1 = y1;
    _setOtherValues();
  }


  /**
   * Change one of the line endpoints to input values
   * @param[in] index  0 or 1
   * @param[in] x  new value
   * @param[in] y  new value
   */
  inline void adjustEndpoint(int index, double x, double y)
  {
    if (index == 0)
    {
      _x0 = x;
      _y0 = y;
    }
    else
    {
      _x1 = x;
      _y1 = y;
    }
    _setOtherValues();
  }

  /**
   * Clear grid values in the space between two line segments, which are
   * assumed separated by some empty space.
   *
   * Clear all points ahead of 0th endpt of 'other' and behind the 1th endpt of
   * local line, i.e. assume the local line is 'behind' the input line
   *
   * @param[in] other  Input line
   * @param[in,out] g  Grid to filter
   */
  void clearBetween(const Line &other, Grid2d &g) const;

  /**
   * Move a line backwards
   *
   * change line so the 0'th endpoint is moved 'length_0' in the direction
   * pointed to by the vector from the 1 to 0 endpoints, and so that the
   * 1'th endpoint is moved 'length_1' in this same direction
   *
   * @param[in] length1
   * @param[in] length0
   */
  void extendedBehind(int length1, int length0);

  /**
   * Move a line forwards
   *
   * change line so the 0'th endpoint is moved 'length0' in the direction
   * pointed to by the vector from the 0 to 1 endpoints, and so that the
   * 1'th endpoint is moved 'length1' in this same direction
   *
   * @param[in] length1
   * @param[in] length0
   */
  void extendedAhead(int length1, int length0);
  
  /**
   * add in motion to line using inputs.
   * @param[in,out] mask Grid to use locally
   * @param[in] motion_angle  Grid with motion angles
   * @param[in] motion_magnitude Grid with motion magnitudes, pixels per second
   * @param[in] debug True to see more
   *
   * Take average of the motion grid values near the line to come up
   * with a motion, and add a MotionVector the the Attributes base class
   */
  void addMotion(Grid2d &mask, const Grid2d &motion_angle, 
		 const Grid2d &motion_magnitude, bool debug=false);

  /**
   * Along the line set Grid image values to the input value
   *
   * @param[in,out] image
   * @param[in] value
   *
   * Does not clear out grid prior to filling in values
   */
  void toGrid(Grid2d &image, double value) const;

  /**
   * Create a Box object that just encloses the line segment
   *
   * @return the box
   */
  Box extrema(void) const;

  /**
   * @return a PointList representation of the line segement
   * which is effectively a mapping of the point in a Grid that
   * are along the line segment
   *
   * @return The PointList
   */
  PointList xyValues(void) const;

  /**
   * @return true if an infinitely long vector intersects the Line
   *
   * @param[in] x  Start point of vector
   * @param[in] y  Start point of vector
   * @param[in] deg  Degrees orientation of vector
   * @param[out] xp  Intersection point when return is true
   * @param[out] yp  Intersection point when return is true
   */
  bool vectorIntersects(double x, double y, double deg, double &xp,
			double &yp) const;

  /**
   * If the Line contains a motion vector attribute, move the Line based on the
   * time input.  The MotionVector units is assumed pixels/second
   *
   * @param[in] seconds  Number of seconds the Line is assumed moved
   */
  void extrapolate(double seconds);

  /**
   * Move the line by the distances given in x and y
   * @param[in] dx  Amount to move the line, x
   * @param[in] dy  Amount to move the line, y
   */
  void move(const double dx, const double dy);

  /**
   * Return the Endpts object associated with the line if it has one,
   *
   * @param[out] e  Endpts
   * @return true if Endpts was available to return
   */
  bool getEndpts(Endpts &e) const;

  /**
   * Give the line an Endpts object
   *
   * @param[in] e  Endpts
   */
  void setEndpts(const Endpts &e);

  /**
   * Return the Handedness object associated with the line if it has one,
   *
   * @param[out] e  Handedness
   * @return true if Handedness was available to return
   */
  bool getHandedness(Handedness &e) const;

  /**
   * Give the Line Handedness by computing it internally using MotionVector
   * attribute.
   */
  void setHandedness(void);

  /**
   * Give the Line the Handedness indicated by the input type
   *
   * @param[in] e  Handedness type
   */
  void setHandedness(Handedness::e_hand_t &e);

  /**
   * Give the Line Handedness equal to input
   * @param[in] h  Handedness object to use
   */
  void setHandedness(const Handedness &h);

  /**
   * Remove the Handedness from the Line if it is present
   */
  void removeHandedness(void);

  /**
   * get the 'average' handedness between input and local Line
   * @param[in] l  Input line
   * @return Handedness object that is the average
   */
  Handedness averageHandedness(const Line &l) const;

  /**
   * get the 'average' endpoints between input and local Line
   * @param[in] l  Input line
   * @return Endpts object that is the average
   */
  Endpts averageEndpts(const Line &l) const;

  /**
   * When Line has MotionVector attribute, adjust the MotionVector to
   * be normal to the Line
   */
  void adjustMotionDirection(void);

  /**
   * When Line has MotionVector attribute, adjust the MotionVector to
   * be normal to the Line, first using input Line as a reference
   * @param[in] s  Line
   */
  void adjustMotionDirection(const Line &s);

  /**
   * @return distance between the centerpoints of input and local line
   * @param[in] l1  Line
   */
  double centerpointDistance(const Line &l1) const;

  /**
   * Adjust lines MotionVector
   *
   * Create new MotionVector perpendicular to the input vector
   * with input speed and handedness equal to handedness attribute.
   *
   * @param[in] speed 
   */
  void setVelToMatchHandedness(double speed);

  /**
   * Adjust lines MotionVector
   *
   * If the existing speed is significant, just change speed to input,
   * otherwise use the handedness attribute to set motion vector to be
   * perpendicular to the line, at the input speed.
   *
   * @param[in] speed 
   */
  void adjustVelWithHandedness(double speed);

  /**
   * Create a DataAtt object using input data, and add it as Attributes
   * to the Line
   * @param[in] data
   */
  void adjustForData(const Grid2d &data);

  /**
   * @return angle between input and local line segments [0,90].
   * @param[in] l1
   */
  double minimumAngleBetween(const Line &l1) const;

  /**
   * @return true if the both endpoints of the input line are 'behind'
   * the 1'th endpoint of the local line.
   * @param[in] l1
   * @param[out] gap  The minimum distance behind when return is true
   */
  bool isBehind(const Line &l1, double &gap) const;

  /**
   * @return true if the input line is ahead of the local line, with local
   * line assumed horizontal.
   * 
   * @param[in] l1  Line to check
   * @param[out] gap  The gap, when return status is true
   */
  bool isAheadOfHorizontal(const Line &l1, double &gap) const;

  /**
   * @return true if the input line is behind of the local line, with local
   * line assumed horizontal.
   * 
   * @param[in] l1  Line to check
   * @param[out] gap  The gap, when return status is true
   */
  bool isBehindHorizontal(const Line &l1, double &gap) const;

  /**
   * The input line intersects the local line at parametric location
   * alpha1 of l1.  Return the smallest angle between the two lines at this
   * point, assuming that l1 is intended to 'extend' or continue a line build
   * from the local line as a vector... i.e. it should move 'away' from
   * the local line as much as possible
   *
   * @param[in] l1  The extending line
   * @param[in] alpha1 The point on l1 where the local line intersects it
   *
   * @return the angle
   */
  double bestVectorAngle(const Line &l1,  double alpha1) const;

  /**
   * @return the smallest angle between input and local line, with the angle of
   * the 0th line restricted to be that from 0th endpoint.
   *
   * @param[in] l1  Input line
   */
  double bestVectorAngle(const Line &l1) const;

  /**
   * Take average of input line and local line.
   *
   * @param[in] v1 the other line
   *
   * assumes 0th end of v1 is the same as the 0th endpoint of the local line.
   */
  void averageCommon0(const Line &v1);

  /**
   * Set motion vector in local Line to the average of that and the motion
   * vector of input line, with weighted averaging based on line lengths
   * @param[in] other
   */
  void lengthAveragedVel(const Line &other);

  /**
   * set local object motion vector
   *
   * take local and input motion vectors/length, 
   * replace velocity in local object with the length weighted average speed
   * in the original direction.
   *
   * @param[in] mv1  Motion vector to get a speed from to average in
   * @param[in] l1 Weight to give mv1s speed
   */
  void lengthAveragedVel(const MotionVector &mv1, double l1);

  /**
   * Check endpoints of input lines and return status about that based on
   * which end is closest to which.  If the local lines 1th endpoint is close
   * to l1 0th endpoint, no reversing is wanted.
   *
   * @param[in] l1  Line to check
   * @param[in] thresh  Ratio of endpoint distance differences threshold
   * @param[out] reverse0   True to reverse local lines endpoints
   * @param[out] reverse1   True to reverse l1's endpoints
   */
  void checkForEndpointReverse(const Line &l1, double thresh,
			       bool &reverse0, bool &reverse1) const;

  /**
   * Use input speed and dir, to set local Line motion vector perpendicular
   * to the line
   *
   * @param[in] bestDir  Motion vector whose direction is used to decide
   *                      which of two directions perpendicular to the line
   *                      to use
   * @param[in] speed  Speed to use
   */
  void fillVelWithBestSpeed(const MotionVector &bestDir, double speed);

  /**
   * Use input line for directiona and local line for speed to set local
   * motion vector, then move the local line using this motion vector
   *
   * @param[in] lcopy  Line from which to get a motion vector that specifies
   *                   direction.  This direction is used to decide which
   *                   of two dirs perpendicular ot the line to use.
   * @param[in] dt  The seconds used to extrapolate the line.
   */
  void fillBestSpeedAndMove(const Line &lcopy, double dt);     // seconds

protected:

  double _x0;        /**< 0'th endpoint X */
  double _y0;        /**< 0'th endpoint y */
  double _x1;        /**< 1'th endpoint X */
  double _y1;        /**< 1'th endpoint Y */
  bool _is_vertical; /*< true for a vertical segment */
  double _slope;     /*< slope (non-vertical case */
  double _intercept; /*< y intercept (non-vertical case */
  bool _is_bad;      /**< True if values not set */

private:

  bool _has_endpts;  /**< Used in algorithms where line is in a linelist*/
  Endpts _endpts;    /**< Used in algorithms where line is in a linelist*/

  bool _has_hand;    /**< Used in some algorithms */
  Handedness _hand;  /**< Used in some algorithms */

  void _init(void);
  void _fillBounds(const Box &b);
  void _fillBoundsNonCorner(double minx, double miny,
			       double maxx, double maxy,
			       double yy0, double yy1,
			       double xx0, double xx1);
  void _fillBoundsOneCorner(double minx, double miny, double maxx,
			       double maxy, double yy0, double yy1,
			       double xx0, double xx1, bool yy0ok,
			       bool yy1ok, bool xx0ok, bool xx1ok);
  double _minDistanceSlopedSquared(double x, double y) const; 
  double _minDistanceHorizontalSquared(double x, double y) const; 
  double _minDistanceVerticalSquared(double x, double y) const; 
  bool _pointIsGreater(double x, double y) const;
  static void _rotatePoint(double &x, double &y, double angle);
  inline static double _parametricValue(double alpha, double x0, double x1)
  {
    return alpha*x0 + (1.0-alpha)*x1;
  }

  inline void _setOtherValues(void)
  {
    if (fabs(_x0-_x1) < 1.0e-10)
    {
      _is_vertical = true;
      _slope = 0.0;
      _intercept = 0.0;
    }
    else
    {
      _is_vertical = false;
      _slope = (_y1 - _y0)/(_x1 - _x0);
      _intercept = _y1 - _slope*_x1;
    }
  }
  void _setupXyComp(void) const;

  // replace motion vector with one that is perp to line
  // most similar to input motion, with magnitude set to input.
  void _replaceMotionWithSimilarMotion(double angle, double mag, bool debug);
  void _rotatePair(const Line &l1, Line &l0r, Line &l1r,
		   bool change_endpts) const;
  MotionVector _averageDirection(const Line &l1, int which) const;

  // determine best speed that is perpendicular to the line, given inputs.
  MotionVector _bestSpeed(const MotionVector &best_unit_dir,
			  double speed) const;
};

/**
 * @class LineFollow
 * @brief  Move one gridpoint at a time along a line
 *
 * The line is traversed from an initial gridpoint to a final one through
 * calls to next()
 *
 * If the line is 'more vertical' (has slope > 1),
 * then the y values increment by 1 and the
 * x values increment by less than 1 per call to next().
 *
 * If the slope <= 1 ('more horizontal') the x values increment by 1
 * and the y values  increment by less than 1.
 */
class LineFollow : public Line
{
public:
  /*
   * Constructor 
   * @param[in] l  The line to follow
   */
  inline LineFollow(const Line &l) : Line(l)
  {
    if (_is_vertical || (!_is_vertical && fabs(_slope) > 1.0))
    {
      if (_y0 < _y1)
      {
	_ly0 = (double)((int)_y0);
	_ly1 = (double)((int)_y1);
      }
      else
      {
	_ly0 = (double)((int)_y1);
	_ly1 = (double)((int)_y0);
      }
      _more_vertical = true;
      _yi = _ly0;
      _xi = (double)(_is_vertical?((int)_x0):(int)((_yi-_intercept)/_slope));
    }
    else
    {
      if (_x0 < _x1)
      {
	_lx0 = (double)((int)_x0);
	_lx1 = (double)((int)_x1);
      }
      else
      {
	_lx0 = (double)((int)_x1);
	_lx1 = (double)((int)_x0);
      }
      _more_vertical = false;
      _xi=_lx0;
      _yi = (double)((int)(_xi*_slope+_intercept));
    }
  }

  /**
   * Destructor
   */
  inline virtual ~LineFollow(void) { }
  

  /**
   * Get the next gridpoint along the line
   * @param[out] x  
   * @param[out] y  
   * @return true if there is a next point
   */
  inline bool next(double &x, double &y)
  {
    if (_more_vertical)
    {
      if (_yi <= _ly1)
      {
	y = _yi++;
	x = _xi;
	_xi = 
	  (double)(_is_vertical?((int)_x0):(int)((_yi-_intercept)/_slope));
	return true;
      }
      else
      {
	return false;
      }
    }
    else
    {
      if (_xi <= _lx1)
      {
	x = _xi++;
	y = _yi;
	_yi = (double)((int)(_xi*_slope+_intercept));
	return true;
      }
      else
      {
	return false;
      }
    }
  }
    
private:

  double _lx0; /**< Minimum X for the 'horizontal' case */
  double _lx1; /**< Maxmum X for the 'horizontal' case */
  double _ly0; /**< Minimum Y for the 'vertical' case */
  double _ly1; /**< Maximum Y for the 'vertical' case */
  bool   _more_vertical;  /**< True for 'more vertical */
  double _xi;  /**< Current x value */
  double _yi;  /**< Current y value */

};

# endif 
