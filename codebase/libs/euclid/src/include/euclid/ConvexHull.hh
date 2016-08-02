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
 * @file ConvexHull.hh
 * @brief Algorithm that takes a set of points and reduces it to the convex 
 *        hull points..
 * @class ConvexHull
 * @brief Algorithm that takes a set of points and reduces it to the convex 
 *        hull points.

 *
 * Order the points in a counterclockwise direction relative to 0th point
 * If two points have same angle, keep only furthest one from 0th point
 *
 * Create convex hull from ordered points
 */

# ifndef    CONVEX_HULL_H
# define    CONVEX_HULL_H

#include <cstdio>
#include <vector>
#include <string>

//----------------------------------------------------------------
class ConvexHull
{
 public:

  /**
   * Constructor
   */
  ConvexHull(const std::vector<int> x, const std::vector<int> y);

  /**
   * Destructor
   */
  virtual ~ConvexHull(void);

  void print(void) const;
  void printBuild(void) const;
  void printFinal(void) const;
  std::vector<int> getX(void) const;
  std::vector<int> getY(void) const;

protected:
private:

  /**
   * @class ConvexHullPoint
   * @brief One point in convex hull, relative to a fixed 0th point
   */
  class ConvexHullPoint
  {
  public:
    /**
     * Constructor, really a line segment from 0th point to another point
     * @param[in] x  Point
     * @param[in] y  Point
     * @param[in] x0 0th Point
     * @param[in] y0 0th Point
     */
    inline ConvexHullPoint(int x, int y, int x0, int y0) :
      _x0(x0), _y0(y0), _x(x), _y(y) {}

    /**
     * Destructor
     */
    inline virtual ~ConvexHullPoint(void) {}

    void print(void) const;
  
    /**
     * @return length of the line segement represented by local object
     */
    double lengthSq(void) const;

    /**
     * test if one point is Left|On|Right another point
     *
     * @param[in] p1  One point
     * @param[in] p2  Other point
     * @return true if p2 is left/on p1, false if p2 is right of p1
     */
    static bool isLeft(const ConvexHullPoint &p1, const ConvexHullPoint &p2);

    /**
     * Test if one point is on the same line as another one
     * @param[in] p1
     * @param[in] p2
     */
    static bool isEqual(const ConvexHullPoint &p1, const ConvexHullPoint &p2);

    /**
     * @return true if p1 is on the same line as p2, and is further away
     * than p2, false o.w.
     *
     * @param[in] p1
     * @param[in] p2
     */
    static bool isFurther(const ConvexHullPoint &p1, const ConvexHullPoint &p2);

    int _x0; /**< Initial point */
    int _y0; /**< Initial point */
    int _x; /**< Final point */
    int _y; /**< Final point */

  protected:
  private:

  };

  /**
   * @class ConvexHullPoint
   * @brief One point in convex hull
   */
  class ConvexHullPoint1
  {
  public:
    /**
     * Constructor
     * @param[in] x
     * @param[in] y
     */
    inline ConvexHullPoint1(int x, int y) : _x(x), _y(y) {}

    /**
     * Destructor
     */
    inline virtual ~ConvexHullPoint1(void) {}

    void print(void) const;
  
    int _x;  /**< point */
    int _y;  /**< point */

  protected:
  private:

  };


  /**
   * Points used to build convex hull
   */
  std::vector<ConvexHullPoint> _xyBuild;

  /**
   * Points in the convex hull
   */
  std::vector<ConvexHullPoint1> _xyFinal;

  bool _removeFirstEqual(void);
  bool _removeEqual(size_t i);
};

# endif
