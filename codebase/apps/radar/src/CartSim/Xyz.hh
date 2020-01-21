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
 * @file  Xyz.hh
 * @brief  3d vector
 * @class  Xyz
 * @brief  3d vector
 *
 */

# ifndef    XYZ_H
# define    XYZ_H

//----------------------------------------------------------------
class Xyz
{
public:

  /**
   * Default constructor x=y=z=0
   */
  inline Xyz(void) {_x = _y = _z = 0.0;}

  /**
   * Default constructor with values
   */
  inline Xyz(double x, double y, double z) {_x = x; _y = y; _z = z;}
  
  /**
   * Linear transform, created object = v + c*u
   *
   * @param[in] v  
   * @param[in] c
   * @param[in] u
   * @param[in] zpositive  True to truncate z values at >= 0
   */
  Xyz(const Xyz &v, double c, const Xyz &u, const bool zpositive);

  /**
   * Scale input vector, created object = v*scale
   *
   * @param[in] v
   * @param[in] scale
   */
  Xyz(const Xyz &v, const double scale);


  /**
   * Destructor
   */
  inline virtual ~Xyz(void) {}

  bool operator==(const Xyz &x) const;

  /**
   * Set (x,y,z) = s*(x,y,z)
   * @param[in] s
   */
  void scale(const double s);

  /**
   * Set (x,y,z) = (sx*x,sy*y,sz*z)
   * @param[in] sx
   * @param[in] sy
   * @param[in] sz
   */
  void scale(const double sx, const double sy, const double sz);

  /**
   * @return true if (x,yz) is in the box centered at center, with
   * offsets up to radius in all 3 dimensions
   * @parma[in] center
   * @parma[in] radius
   */
  bool inBox(const Xyz &center, const Xyz &radius) const;

  /**
   * @return s._x*_x + s._y*_y + s._z*_z
   * @param[in] s
   */
  double dotProduct(const Xyz &s) const;

  /**
   * @return angle of the vector (x,y) (mathematical)
   */
  double xyAngleDegrees0to360(void) const;

  /**
   * @return angle of the vector above horizontal
   */
  double zAngleDegrees0to360(void) const;

  /**
   * @return length of the vector
   */
  double magnitude(void) const;

  /**
   * @return true if the input vector r is between the vectors
   * a and b (in x,y)
   *
   * @param[in] a
   * @param[in] r
   * @param[in] b
   *
   * Assumes angle between a and b directions is less than  180 degrees.
   */
  static bool vectorBetween(const Xyz &a, const Xyz &r, const Xyz &b);

  /*
   * @return true if x is between a and b, when all are angles
   * and a is >= 0 and b < 0 and a,b,x in range [-180, 180].
   * This is a scalar computation.
   *
   * @param[in] a
   * @param[in] x
   * @param[in] b
   */
  static bool vectorBetweenMixed(double a, double x, double b);

  double _x;  /**< Component */
  double _y;  /**< Component */
  double _z;  /**< Component */

protected:
private:  

};

# endif 
