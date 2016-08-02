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
 * @file Math.hh
 * @brief Assorted math static functions
 * @class Math
 * @brief Assorted math static functions
 */

# ifndef    MATH_H
# define    MATH_H

#include <vector>
#include <cmath>

#define MEDIUM_SMALL   0.05
#define SMALL        1.0e-6
#define VERYSMALL    1.0e-10

class Math
{
public:

  /**
   * @return a small value
   */
  inline static double smallValue(void) {return SMALL;}

  /**
   * @return true if two values are very close
   * @param[in] x0
   * @param[in] x1
   */
  inline static bool veryClose(double x0, double x1)
  {
    return fabs(x0-x1) < VERYSMALL;
  }


  /**
   * @return true if two values are close
   * @param[in] x0
   * @param[in] x1
   */
  inline static bool close(double x0, double x1)
  {
    return fabs(x0-x0) < SMALL;
  }

  /**
   * @return true if input value is 'very small'
   * @param[in] v
   */
  inline static bool verySmall(double v)
  {
    return fabs(v) < VERYSMALL;
  }

  /**
   * @return true if input value is 'medium small'
   * @param[in] v
   */
  inline static bool mediumSmall(double v)
  {
    return fabs(v) < MEDIUM_SMALL;
  }

  /**
   * @return true if input value is 'small'
   * @param[in] v
   */
  inline static bool small(double v)
  {
    return fabs(v) < SMALL;
  }

  /**
   * @return maximum of two integer numbers
   */
  inline static int maxInt(int v0, int v1)
  {
    if (v0 < v1)
      return v1;
    else
      return v0;
  }

  /**
   * @return maximum of three integer numbers
   */
  inline static int maxInt(int v0, int v1, int v2)
  {
    if (v0 < v1)
      return maxInt(v1, v2);
    else
      return maxInt(v0, v2);
  }

  /**
   * @return maximum of two double numbers
   * @param[in] v0
   * @param[in] v1
   */
  inline static double maxDouble(double v0, double v1)
  {
    if (v0 < v1)
      return v1;
    else
      return v0;
  }

  /**
   * @eturn angle gotten by rotating an angle 90 degrees to the 'left',
   * where 0 degrees is north and 90 is east
   *
   * @param[in] angle
   */
  inline static double leftRotation(double angle)
  {
    double aleft = angle - 90.0;
    while (aleft < 0.0) aleft += 360.0;
    return aleft;
  }

  /**
   * @eturn angle gotten by rotating an angle 90 degrees to the 'right',
   * where 0 degrees is north and 90 is east
   *
   * @param[in] angle
   */
  inline static double rightRotation(double angle)
  {
    double aright = angle + 90.0;
    while (aright > 360.0) aright -= 360.0;
    return aright;
  }

  /**
   * @return integer value truncated at a min and max
   *
   * @param[in] v  Original value
   * @Parma[in] min  Minimum returned value
   * @Parma[in] max  Maximum returned value
   */
  inline static int constrainedInt(int v, int min, int max)
  {
    if (v < min)
      return min;
    if (v > max)
      return max;
    return v;
  }

  /**
   * @return pixel (unsigned char) representation of a double value
   *
   * @param[in] value  Double
   * @param[in] scale  
   * @param[in] offset
   */
  inline static unsigned char pixval(double value, double scale, double offset)
  {
    double d = (value - offset)/scale;
    int i = (int)d;
    return (unsigned char)constrainedInt(i, 0, 255);
  }

  /**
   * @return pixel (unsigned char) representation of a double value,
   * represented as a double
   *
   * @param[in] value  Double
   * @param[in] scale  
   * @param[in] offset
   */
  inline static double doublePixval(double value, double scale, double offset)
  {
    double d = (value - offset)/scale;
    return d;
  }

  /**
   * @return double value associated with a pixel (unsigned char) value
   *
   * @param[in] pixel  The pixel value
   * @param[in] scale  
   * @param[in] offset
   */
  inline static double dataValue(unsigned char pixel, double scale,
				 double offset)
  {
    double v = (double)pixel*scale + offset;
    return v;
  }

  /**
   * @return number of pixels for a given kilometer distance
   * @param[in] kms  The number of kilometers
   * @param[in] meters_per_pixel
   */
  inline static double pixperkm(double kms, double meters_per_pixel)
  {
    double d = kms*1000.0/meters_per_pixel;
    return d;
  }

  /**
   * @return number of kilometers for a given number of pixels
   * @param[in] pix The number of pixels
   * @param[in] meters_per_pixel
   */
  inline static double kmperpix(double pix, double meters_per_pixel)
  {
    double d = pix/1000.0*meters_per_pixel;
    return d;

  }

  /**
   * @return number of pixels squared for a given area
   * @param[in] kmsq  The area (kilometers squared)
   * @param[in] meters_per_pixel
   */
  inline static double pixsqperkmsq(double kmsq, double meters_per_pixel)
  {
    double d = kmsq*1000.0*1000.0/(meters_per_pixel*meters_per_pixel);
    return d;
  }

  /**
   * @return number of kilometers squared for a given area
   * @param[in] pixsq  The number of pixels squared
   * @param[in] meters_per_pixel
   */
  inline static double kmsqperpixsq(double pixsq, double meters_per_pixel)
  {
    double d = pixsq/(1000.0*1000.0)*(meters_per_pixel*meters_per_pixel);
    return d;
  }

  /**
   * adjust an angle to be within [0,360) and return that angle
   * @param[in] a  Original angle
   */
  inline static double angle0To360(double a)
  {
    double b = a;
    while (b > 360.0) b -= 360.0;
    while (b < 0.0) b += 360.0;
    return b;
  }

  /**
   * adjust an angle to be within [0,180) and return that angle
   * @param[in] a  Original angle
   */
  inline static double angle0To180(double a)
  {
    double b = a;
    while (b < 0.0) b += 180.0;
    while (b >= 180.0) b -= 180.0;
    return b;
  }

  /**
   * @return the minimum angle difference between two angles
   * @param[in] a0
   * @param[in] a1
   */
  inline static double angleDiff(double a0, double a1)
  {
    double b0 = angle0To360(a0);
    double b1 = angle0To360(a1);
    double d = b0 - b1;
    if (d < 0)  d = -d;
    if (d > 180.0) d = 360.0 - d;
    return d;
  }

  /**
   * @return the rotation (degrees) between two angles 
   * (angles go from 0 to 360)
   * @param[in] a0
   * @param[in] a1
   */
  inline static double angleChange(double a0, double a1)
  {
    double b0 = angle0To360(a0);
    double b1 = angle0To360(a1);
    // rotate so b0 goes to 0, b1 moves with it.
    b1 = b1 - b0;
    // with b0 pointing to the right, does b1 go up or down?
    b1 = angle0To360(b1);
    // 0 to 180 is up, 181 to 359 is down
    if (b1 <= 180.0)
      // goes up, that is the change
      return b1;
    else
      // goes down, difference is 360 - this
      return 360 - b1;
  }

  /**
   * @return the orientation change (degrees) between two orientations
   * (orientations  go from 0 to 180)
   * @param[in] a0
   * @param[in] a1
   */
  inline static double lineOrientationDiff(double a0, double a1)
  {
    double b0 = angle0To180(a0);
    double b1 = angle0To180(a1);
    double d = b0 - b1;
    if (d < 0)  d = -d;
    if (d > 90.0) d = 180.0 - d;
    return d;
  }

  /**
   * @return the angle (in range 0 to 180) between two vector directions
   * @param[in] vx0  Vector0 X
   * @param[in] vy0  Vector0 Y
   * @param[in] vx  Vector1 X
   * @param[in] vy  Vector1 Y
   */
  inline static double angleBetweenVectors(double vx0, double vy0, double vx, 
					   double vy)
  {
    double d = fabs((atan2(vy, vx)-atan2(vy0, vx0))*180.0/3.14159);
    if (d < 180.0) return d;
    return 360.0 - d;
  }

  /**
   * @return the angle 180 degrees opposite of an angle, in range 0 to 360
   * @param[in] a0  Angle
   */
  inline static double oppositeAngle(double a0)
  {
    return angle0To360(a0 + 180.0);
  }

  /**
   * @return the parametric value  for inputs
   *
   * @param[in] alpha parameter
   * @param[in] x0  value to use if alpha=1
   * @param[in] x1  value to use if alpha=0
   */
  inline static double parametricValue(double alpha, double x0, double x1)
  {
    return alpha*x0 + (1.0-alpha)*x1;
  }

  /**
   * @return true if a parametric value indicagtes it is between the two points
   * @param[in] alpha
   */
  inline static bool parametricLineAlphaOk(double alpha)
  {
    return (alpha >= 0.0 && alpha <= 1.0);
  }

  /**
   * adjust orientation a1 to be as close to a2 as possible.
   * a1 is constrained to be either a1 or a1 + 180.
   *
   * @param[in] a1  
   * @param[in] a2
   */ 
  static double adjustOrientation(double a1, double a2);

  /**
   * @return mathematical angle [0,360] for the orientation of the line
   * from x0,y0 to x1,y1.
   *
   * @param[in] x0
   * @param[in] y0
   * @param[in] x1
   * @param[in] y1
   * @param[in] is_vert true for a vertical line
   * @param[in] slope Slope of line if not vertical
   */
  static double vectorLineAngle(double x0, double x1, double y0,
				double y1, bool is_vert, double slope);

  /**
   * @return mathematical angle [0,360] of the vector from x0,y0 to x1,y1
   *
   * @param[in] x0
   * @param[in] y0
   * @param[in] x1
   * @param[in] y1
   */
  static double vectorAngle(double x0, double y0, double x1, double y1);

  /**
   * Sort an array of data, leaving array unchanged, seting indx values
   * @param[in] array data
   * @param[out] indx  Index values indx[0] refers to smallest element in array
   */
  static void sort( const std::vector<double> &array, std::vector<int> &indx);

  /**
   * Sort an array of data, leaving first unchanged, seting second (indx) values
   * @param[in,out] doubles where first = data (entry) and second=index (exit)
   */
  static void sort( std::vector<std::pair<double,int> > &array);

  /**
   * Sort an array of data, leaving array unchanged, seting indx values
   * @param[in] array data
   * @param[out] indx  Index values indx[0] refers to smallest element in array
   */
  static void sort( int n, double *array, int *indx);

  /**
   * @return index into a vector at which maximum occurs
   * @param[in] v data
   */
  static int maxIndex(std::vector<double> &v);

  /**
   * Return smallest angle (degrees) between two vectors.
   *
   * The first vector is from (vx0,vy0) to (vx1,vy1).
   * The second vector is from (vx1,vy1) to (vx2,vy2)
   *
   * @param[in] vx0
   * @param[in] vy0
   * @param[in] vx1
   * @param[in] vy1
   * @param[in] vx2
   * @param[in] vy2
   */
  static double absoluteVectorAngle(double vx0, double vy0,
				    double vx1, double vy1,
				    double vx2, double vy2);

  /**
   * rotate input point x,y by angle. (assuming a vector from 0,0 to x,y)
   * @param[in,out] x  Point to rotate
   * @param[in,out] y  Point to rotate
   * @param[in] angle  Rotation angle
   */
  static void rotatePoint(double &x, double &y, double angle);

  /**
   * @return true if two angles are not close enough
   * @param[in] a0 angle deg
   * @param[in] a1 other angle deg
   * @param[in] max_change maximum allowed difference degrees
   */
  static bool anglesTooFarApart(double a0, double a1, double max_change);

protected:
private:

};

# endif 
