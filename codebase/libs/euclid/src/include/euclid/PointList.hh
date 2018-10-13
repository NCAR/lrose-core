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
 * @file PointList.hh
 * @brief  Vector of x,y locations
 * @class PointList
 * @brief  Vector of x,y locations
 */

# ifndef    POINTLIST_H
# define    POINTLIST_H

#include <euclid/AttributesEuclid.hh>
#include <euclid/Grid2dClump.hh>  // needed for typedef
#include <euclid/Point.hh>
#include <cstdio>
#include <vector>

class Grid2d;
class Box;

class PointList : public AttributesEuclid
{
  
public:

  /**
   * Default constructor..empty list.
   */
  PointList(void);

  /**
   * Default constructor..empty list, with grid dimensions
   * @param[in] nx  Grid dimension
   * @param[in] ny  Grid dimension
   */
  PointList(const int nx, const int ny);

  /**
   * Constructor..points passed in using clump struct
   * @param[in] nx  Grid dimension
   * @param[in] ny  Grid dimension
   * @param[in] pts  The points
   */
  PointList(const int nx, const int ny, clump::Region_t &pts);

  /**
   * Constructor.. all the non-flagged points in the input grid make
   * up the points
   * @param[in] img  Grid
   */
  PointList(const Grid2d &img);

  /**
   * Copy constructor
   * @param[in] p
   */
  PointList(const PointList &p);

  /**
   * Destructor
   */
  virtual ~PointList(void);


  /**
   * Operator=
   * @param[in] p
   */
  PointList & operator=(const PointList &p);

  /**
   * Operator==
   * @param[in] p
   */
  bool operator==(const PointList &p) const;

  /**
   * Create an XML string that represents the entire point list 
   * including attributes
   * @param[in] tag  Tag for entire pointlist
   */
  std::string writeXml(const std::string &tag) const;

  /**
   * Parse an XML string to set values for the entire list 
   * including attributes
   * @param[in] xml  The data
   * @param[in] tag  Tag for entire list
   * @return true for success
   */
  bool readXml(const std::string &xml, const std::string &tag);

  /**
   * Parse an XML string to set values for the entire list , no overall tag
   * including attributes
   * @param[in] xml  The data
   * @return true for success
   */
  bool readXml(const std::string &xml);

  /**
   * @return number of points
   */
  inline int size(void) const {return (int)_points.size();}
  
  /**
   * @return i'th X value
   * @param[in] i
   */
  inline double ithX(const int i) const {return _points[i].getX();}

  /**
   * @return i'th Y value
   * @param[in] i
   */
  inline double ithY(const int i) const {return _points[i].getY();}

  /**
   * @return i'th Point
   * @param[in] i
   */
  inline Point ithPoint(const int i) const {return _points[i];}

  /**
   * Remove all points
   */
  inline void clear(void) {_points.clear();}


  /**
   * @return sum of length between successive list elements.
   */
  double cumulativeLength(void) const;

  /**
   * Set local object x,y to those input grid points that are not missing
   * 
   * @param[in] img  The grid
   *
   * @note clears out state prior to setting
   */
   void fromGrid(const Grid2d &img);

  /**
   * Put points to grid with a value  At all the local points, set the
   * grid to value
   *
   * @param[in,out] img  Grid
   * @param[in] value
   *
   * @note does not clear out Grid prior to setting
   */
  void toGrid(Grid2d &img, double value) const;

  /**
   * @return number of PointList points that are > thresh in img
   *
   * @param[in] img  
   * @param[in] thresh
   */
  int numPointsAboveThresh(const Grid2d &img, const double thresh) const;

  /**
   * @return true if at least min_percent of the area within the smallest
   * enclosing circle of the PointList points is made up of points from the
   * PointList
   *
   * @param[in] min_percent
   */
  bool isCircular(double min_percent) const;

  /**
   * @return the average location of the PointList points (centerpoint)
   * @param[out] x
   * @param[out] y
   */
  void centerpoint(double &x, double &y) const;

  /**
   * @return average x value
   */
  double xAverage(void) const;

  /**
   * @return true if at every point in the PointList, the value in the grid is
   * within tolerance of one of the input values
   *
   * @param[in] num  Number of values
   * @param[in] values
   * @param[in] tolerance
   * @param[in] grid
   */
  bool onIntList(int num, const int *values, double tolerance,
		 const Grid2d &grid) const;

  /**
   * @return minimum X value from the PointList points
   */
  double minX(void) const;

  /**
   * @return maximum X value from the PointList points
   */
  double maxX(void) const;

  /**
   * @return minimum Y value from the PointList points
   */
  double minY(void) const;

  /**
   * @return maximum Y value from the PointList points
   */
  double maxY(void) const;

  /**
   * Return the range of Y values for all PointList points whose X values
   * are in a range
   *
   * @param[in] x0  Lower range of X
   * @param[in] x1  Upper range of X
   * @param[out] min  Minimum Y value
   * @param[out] max  Maximum Y value
   * @return true if there were points in the range x0 to x1 to build min/max
   */
  bool yRangeOverX(double x0, double x1, double &min, double &max) const;

  /**
   * Return the range of X values for all PointList points whose Y values
   * are in a range
   *
   * @param[in] y0  Lower range of Y
   * @param[in] y1  Upper range of Y
   * @param[out] min  Minimum X value
   * @param[out] max  Maximum X value
   * @return true if there were points in the range y0 to y1 to build min/max
   */
  bool xRangeOverY(double y0, double y1, double &min, double &max) const;

  /**
   * @return the smallest Box that contains the PointList points
   */
  Box extrema(void) const;

  /**
   * Debug print
   * @param[in] fp
   */
  void print(FILE *fp) const;

  /**
   * Debug print to stdout
   */
  void print(void) const;

  /**
   * Debug 'graphical' representation of the pointlist points
   * @param[out] landscape  True if X has a bigger range than Y 
   */
  void printAsciiPicture(bool &landscape) const;
  
  /**
   * Rotate the x,y points by an angle
   * @param[in] angle degrees
   */
  void rotate(double angle);

  /**
   * Remove all points whos x value is not equal to input
   * @param[in] x
   */
  void keepX(double x);

  /**
   * Create object of all points with a certain x value
   * @param[in] x
   * @return new pointlist
   */
  PointList commonX(double x) const;

  /**
   * Remove all points whos y value is not equal to input
   * @param[in] y
   */
  void keepY(double y);

  /**
   * Create object of all points with a certain y value
   * @param[in] y
   * @return new pointlist
   */
  PointList commonY(double y) const;

  /**
   * Remove all points that are missing in a mask grid
   * @param[in] mask  Grid
   */
  void clearNonMasked(const Grid2d &mask);

  /**
   * Modify local object to add in points from input PointList
   * @param[in] l  Points to add
   * 
   * @note efficient implementation in Grid space
   */
  void formXyUnion(const PointList &l);

  /**
   * Modify local object to be only points in both input PointList and
   * local PointList
   * @param[in] l  Points to intersect with
   * 
   * @note efficient implementation in Grid space
   */
  void formXyIntersection(const PointList &l);

  /**
   * Append a point
   * @param[in] x  The point 
   * @param[in] y  The point 
   *
   * @note does not check if point already in place
   */ 
  inline void append(double x, double y)
  {
    _points.push_back(Point(x,y));
  }

  /**
   * Append a point
   * @param[in] p  The point 
   *
   * @note does not check if point already in place
   */ 
  inline void append(const Point  &p)
  {
    _points.push_back(p);
  }

  /**
   * Return index to the input list (input is a list of lists) that is
   * partially contained inside of local list, or for which the local
   * list is partially contained within the input list, where 'partial'
   * is defined by a min percent of overlap.
   *
   * @param[in] list_of_lists  Some PointLists
   * @param[in] min_percent
   * 
   * @return The 1st index that satisfies the criteria, or -1 for none
   */
  int partiallyContainedIndex(const std::vector<PointList> &list_of_lists,
			      double min_percent) const;


  /**
   * @return percentile value of data values at PointList points
   *
   * @param[in] data  The grid
   * @param[in] percentile
   */
  double percentileDataValue(const Grid2d &data, double percentile) const;

  /**
   * @return correlation at Pointlist points from two grids
   * @param[in] x  One grid
   * @param[in] y  The other grid
   */
  double correlation(const Grid2d &x, const Grid2d &y) const;
  
  /**
   * return the maximum grid value at pointlist points
   *
   * @param[in] g  Grid
   * @param[out] maxV
   * @return true if at least one pointlist point had non-missing value
   */
  bool max(const Grid2d &g, double &maxV) const;

  /**
   * return the minimum grid value at pointlist points
   *
   * @param[in] g  Grid
   * @param[out] minV
   * @return true if at least one pointlist point had non-missing value
   */
  bool min(const Grid2d &g, double &minV) const;

  /**
   * Keep PointList points that are >= a threshold in a grid, remove all
   * others
   *
   * @param[in] img  The Grid
   * @param[in] threshold
   */
  void geThreshold(const Grid2d &img, double threshold);

  /**
   * @return Grid x dimension
   */
  inline int getNx(void) const {return _nx;}
  /**
   * @return Grid y dimension
   */
  inline int getNy(void) const {return _ny;}

  /**
   * Remove PointList points that are in a mask (not missing in mask).
   * For each point that is removed, set the value to missing in a grid
   *
   * @param[in] mask  The mask data
   * @param[in,out] img  The grid in which to set points missing
   */
  void clearMasked(const Grid2d &mask, Grid2d &img);

  /**
   * Remove PointList points that are in a mask (not missing in mask).
   * For each point that is removed, set the mask to missing also.
   *
   * @param[in,out] mask  The mask data
   */
  void clearMasked(const Grid2d &mask);


  /**
   * Remove PointList points (x,y) where mask = mask_value, and where the
   * angle from (x0,y0) to (x,y) differs from a0 by more than max_change.
   *
   * Set img values to missing at places where points were removed
   *
   * @param[in] x0  Point to get angle from
   * @param[in] y0  Point to get angle from
   * @param[in] a0  Angle to compare to
   * @param[in] max_change  max allowed angle change degrees
   * @param[in] mask  The mask data
   * @param[in] mask_value
   * @param[in,out] img  The grid in which to set points missing
   */

  void filterAngleDiffTooLargeAndMask(double x0, double y0,
				      double a0,
				      double max_change,
				      const Grid2d &mask,
				      double mask_value,
				      Grid2d &img);

  /**
   * Remove PointList points where mask = mask_value, and set the mask
   * to missing at those points.
   *
   * @param[in] mask  The mask data
   * @param[in] mask_value
   */
   void clearMaskAtValue(Grid2d &mask, double mask_value);

  /**
   * Remove PointList points where mask = value0 or mask = value1
   * 
   * If mask = value0, remove all points in the lookahead 'wedge'
   * radius in range r0 to r1 and angle in range plus or minus dangle0
   *
   * If mask = value1, remove all points in the lookahead 'wedge'
   * radius in range r0 to r1 and angle in range plus or minus dangle1
   *
   * The radius = sqrt((xi-x)*(xi-x) + (yi-y)*(yi-y))
   * The angle = atan2((yi-y)/(xi-x))
   *
   * Set mask grid values to missing at removed points.
   *
   * @param[in] x  starting point
   * @param[in] y  starting point
   * @param[in] angle  
   * @param[in] r0
   * @param[in] r1
   * @param[in] dangle0
   * @param[in] value0
   * @param[in] dangle1
   * @param[in] value1
   * @param[in,out] mask
   */
  void filterLookahead(double x, double y, double angle,
			double r0, double r1,
			double dangle0, double value0,
			double dangle1, double value1,
			Grid2d &mask);

  /**
   * Filter the local data so that all values are in the range 0 to nx-1, 0 to ny-1
   * Set the grid dimensions to these inputs as well
   * @param[in] nx
   * @param[in] ny
   */
  void filter(int nx, int ny);

  /**
   * Erase the index'th point from the point list
   */
  void erase(int index);

  /**
   * Remove points with biggest data outlier values from a PointList one by one
   * until all data values along pointlist are within a tolerance of each other
   * or until a minimum number of points is reached.
   * @param[in] data  Data grid
   * @param[in] maxDataRange  threshold for the max-min data value spread 
   * @param[in] minPts  Minimum number of points
   */
  void removeOutlierValuedPoints(const Grid2d &data, double maxDataRange,
				 int minPts);

 protected:
 private:

  std::vector<Point> _points;   /**< The points */

  int _nx;  /**< Assumed  grid dimensions */
  int _ny;  /**< Assumed  grid dimensions */

  double _enclosingCircleRadiusSquared(double x0, double y0) const;

  /**
   * @return true if input pointlist is partially contained in mask grid
   * or vice versa
   */
  bool _partiallyContained(const Grid2d &mask, int nmasked,
			   double percent) const;


  /**
   * Debug 'graphics', portrait mode (Y has bigger range than X)
   * y should increase from top to bottom, x should increase from left to right.
   */
  void _printAsciiPortrait(void) const;

  /**
   * Debug 'graphics', landscape mode (X has bigger range than Y)
   * y should increase from left to right, x should increase from bottom to top.
   */
  void _printAsciiLandscape(void) const;

  /**
   * @class PointListDataDiff
   * @brief A private class to evaluate data differences in a grid
   *        for points in a PointList
   */
  class PointListDataDiff
  {
  public:
    /**
     * Constructor
     */
    PointListDataDiff(void);

    /**
     * Destructor
     */
    ~PointListDataDiff(void);

    /**
     * Add a value
     * @param[in] v  Value
     * @param[in] index  Index into pointlist where value occured
     */
    void inc(double v, int index);

    /**
     * Finish up setting state after calling 'inc' for all pointlist points
     * @return true if pointlist data does not have outlier values, i.e.
     *         all values are within a tolerance
     *
     * @param[in] maxDiff Maximum allowed different between max an min values
     */
    bool finish(double maxDiff);

    /**
     * @return pointlist index that is the biggest outlier
     *
     * @note a little weak because it compares to the mean, which can be
     * overly influenced by one big value. Should switch to median and do
     * it a different way
     */
    int biggestOutlierIndex(void) const;

  protected:
  private:
    int _iMin;    /**< index to minimum data value. */
    int _iMax;    /**< index to maximum data value. */
    double _min;  /**< The min data value */
    double _max;  /**< The max data value */
    double _mean; /**< mean data value */
    double _num;  /**< Number of values */
    bool _first;  /**< True for nothing yet added */
    bool _debug;  /**< Flag */
  };    
};

# endif   
