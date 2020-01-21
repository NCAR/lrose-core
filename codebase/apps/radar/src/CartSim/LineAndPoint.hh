// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  LineAndPoint.hh
 * @brief Information for one line segment and where a single point is
 *        relative to the line segment.
 * @class LineAndPoint
 * @brief Information for one line segment and where a single point is
 *        relative to the line segment.
 */

# ifndef    LINE_AND_POINT_HH
# define    LINE_AND_POINT_HH

#include "Xyz.hh"
#include "Line.hh"

//----------------------------------------------------------------
class LineAndPoint : public Line
{
public:

  /**
   * @enum Location_t
   * Possible location of the point relative to the line segment
   *
   * These are in the direction across the segment
   */
  typedef enum
  {
    OUTSIDE_1,    /**< Outside on the '1th' endpoint side */
    OUTSIDE_0,    /**< Outside on the '0th' endpoint side */
    INSIDE        /**< Inside the segment */
  } Location_t;

  /**
   * Constructor
   */
  LineAndPoint(void);

  /**
   * Destructor
   */
  virtual ~LineAndPoint(void);

  /**
   * Fill in values for a particular point.
   *
   * @param[in] loc  The point
   * @return true if everything is ok.
   *
   * It is assumed the base class has been filled in prior to this call
   */
  bool pointLocation(const Xyz &loc);


  /**
   * Minimization of inputs using local 'distance beyond side' as comparison
   *
   * @param[in] index  Index value that can be used if adjustment is made
   * @param[in,out] a  Current minimum value
   * @param[in,out] imin  Current index value of minimum, -1 for first call
   *
   * IF local distance_beyond_side < a, then a is set to local value and
   * imin is set to input index value
   */
  void minimizeDistanceBeyondSide(int index, double &a, int &imin) const;


  /**
   * @return the distance beyond the side (meters)
   */
  inline double getDistanceBeyondSide(void) const
  {
    return _distance_beyond_side;
  }

  /**
   * @return the distance in front (meters)
   */
  inline double getDistanceInFront(void) const
  {
    return _distance_in_front;
  }

protected:

  /**
   * meters perpendicular to segment, positive=ahead, negative=behind
   */
  double _distance_in_front;

  /**
   * Meters parallel to segment off the end, when outside of the segment,
   * 0 otherwise
   */
  double _distance_beyond_side;

  /**
   * Which side off of the segment the point is, or not outside at all
   */
  Location_t _beyond_side;

  /**
   * Meters parallel to segment from nearest endpoint, when inside the segment,
   * 0 otherwise
   */
  double _distance_inside;

  /*
   * The input "dir" is the wind direction in a coordinate system where
   * +x is the front direction.
   * On exit, "dir" has been rotated into the actual x,y coordinate
   * system in which the front is found.
   *
   * @param[in,out] dir  Vector to rotate
   */
  void _rotateVector(Xyz &dir) const;

private:

  /**
   * Compute the relation between a point and the front for a sloped line.
   */
  void _sloped(const Xyz &p, const double slpe);
};

# endif 
