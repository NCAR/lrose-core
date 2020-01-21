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
 * @file  Line.hh
 * @brief Information for one line segment
 * @class Line
 * @brief Information for one line segment
 *
 * A line seggment is two points, with a direction perpendicular defined so that
 * there is an 'ahead' direction, which is the direcdtion of the vector that
 * goes from endpoint 0 to endpoint 1, minus 90 degrees. (right rotaton
 * by 90 degrees)
 */

# ifndef    LINE_H
# define    LINE_H

#include "Xyz.hh"

class GustFront;
class Data;

//----------------------------------------------------------------
class Line
{
public:

  /**
   * @enum Orientation_t
   * Possible orientations of line segments
   */
  typedef enum
  {
    UNKNOWN = -1, /**< Unknown orientation */
    X_LINE,       /**< Oriented horizontally */
    Y_LINE,       /**< Oriented vertically */
    SLOPED,       /**< Sloped */
    NONE          /**< Zero length gust front line */
  } Orientation_t;

  /**
   * Constructor
   */
  Line(void);

  /**
   * Destructor
   */
  virtual ~Line(void);

  /**
   * Set location values of endpoints and derive remaining members from that
   * @param[in] loc0  Zero'th endpoint (meters)
   * @param[in] loc1  One'th endpoint  (meters)
   */
  void setLocs(const Xyz &loc0, const Xyz &loc1);

  /**
   * @return true if a point is between two adjacent segments
   *
   * @param[in] previous_segment  Segment just before local segment
   * @param[in] loc  Point to check
   *
   * @param[out] x  Returned distance between common point and input point (m)
   * @param[out] z  Returned height of common point (m)

   * Assume front directions differ by at most 180 degrees and that
   * previous segment endpoint 1 is same as local segment endpoint 0
   */
  bool pointBetweenAdjacentSegments(const Line &previous_segment,
				    const Xyz &loc, double &x,
				    double &z) const;


  /**
   * Create a vector normal to the segment from _loc0 to _loc1
   */
  Xyz normalVector(void) const;

protected:

  Xyz _loc0;     /**< 0th endpoint (meters) */
  Xyz _loc1;     /**< 1th endpoint (meters) */
  double _width; /**< Length of segment (meters) */

  /**
   * Segment orientation 
   */
  Orientation_t _orientation;

  /**
   * The direction vector that is perpendicular to the segment and
   * pointing 'ahead'
   */
  Xyz _front_dir; 


private:

};

# endif 
