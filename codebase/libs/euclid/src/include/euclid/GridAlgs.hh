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
 * @file GridAlgs.hh
 * @brief Algorithms applied to Grid2d base class object
 * @class GridAlgs
 * @brief Algorithms applied to Grid2d base class object
 */

# ifndef  GRIDALGS_H
# define  GRIDALGS_H

#include <string>
#include <vector>
#include <euclid/Grid2d.hh>
#include <toolsa/TaThreadDoubleQue.hh>

class Grid2dLoop;
class FuzzyF;

//------------------------------------------------------------------
class GridAlgs : public Grid2d
{

public:

  /**
   * Empty constructor (no data)
   */
  GridAlgs(void);

  /**
   * named grid, no data
   * @param[in] name = name of the grid
   */
  GridAlgs(const std::string &name);

  /**
   * named grid, missing value
   * @param[in] name = name of the grid
   * @param[in] missing = missing data value
   */
  GridAlgs(const std::string &name, double missing);

  /**
   * named grid with dimensions, sets all values to missing
   * @param[in] name
   * @param[in] nx
   * @param[in] ny
   * @param[in] missing
   *
   */
  GridAlgs(const std::string &name, int nx, int ny, double missing);

  /**
   * named grid with all values
   * @param[in] name
   * @param[in] nx
   * @param[in] ny
   * @param[in] data
   * @param[in] missing
   *
   * @note expect size of data to equal nx*ny
   */
  GridAlgs(const std::string &name, int nx, int ny, 
	   const std::vector<double> &data, double missing);

  /**
   * base class passed in
   *
   * @param[in] g  Grid2d
   */
  GridAlgs(const Grid2d &g);

  /**
   * Destructor
   */
  virtual ~GridAlgs(void);

  /**
   * Create from a base class object
   * @param[in] g  Base class object
   */
  static GridAlgs promote(const Grid2d &g);

  /**
   * @return a new missing data value that is not a data value
   *
   * @param[in] debug true to write out debug info.
   *
   * evaluation consists of figuring out min and max and going just bigger
   */
  double evaluateData(bool debug) const;

  /**
   * @return return percentage of data that is >= a value
   * @param[in] v0 a data value
   */
  double percentGreaterOrEqual(double v0) const;

  /**
   * @return vector containing all the unique (nonmissing) data values
   *         in the grid, up to a maximum.
   * @param[in] max  Maximum allowed size of returned vector.
   *
   * The returned vector if max is hit is truncated there with warning
   */
  std::vector<double> listValues(int max) const;

  /**
   * @return vector of x,y locations at which data equals a value
   *
   * @param[in] v  The value
   */
  std::vector<std::pair<int,int> > pointsAtValue(double value) const;

  /**
   * @return true if data at (x0,y0) is an 'edge'
   *
   * @param[in] x0  Index into grid
   * @param[in] y0  Index into grid
   *
   * @note an edge is a point with data that is next to missing data
   */
  bool isEdge(int x0, int y0) const;

  /**
   * @return true if data at a point is not missing, at most one neighboring
   * point is not missing
   * @param[in] ix  Index
   * @param[in] iy  Index
   */
  bool isValidWithAtMostOneNeighbor(const int ix, const int iy) const;

  /**
   * @return true if a point has missing data, and one of the four
   * adjacent sides 'partially surrounds' the point with non-missing data.
   *
   * For example the point is partially surrounded below if the data values
   * at (ix-1,iy-1), (ix,iy-1), (ix,iy+1), (ix-1,iy), (ix+1,iy) are all
   * not missing.
   *
   * The 4 sides are checked in order with the precidence being the first
   * such case where it is partially surrounded: (bottom, top, right, left).
   *
   * If the bottom is found to be partially surrounded, the returned value is
   * the value at the bottom point (x,y-1), and so on.

   * @param[in] ix  Index
   * @param[in] iy  Index
   * @param[out] v  Returned value when true is returned.
   */
  bool isHoleSingle(const int ix, const int iy, double &v) const;

  /**
   * Get minimum and maximum data values in the grid.
   *
   * @param[out] minv  returned minimum data value in grid
   * @param[out] maxv  returned maximum data value in grid
   *
   * @return false if all data is missing 
   */
  inline bool minMax(double &minv, double &maxv) const;

  /**
   * Get minimum and maximum x and y indices where data is not missing
   *
   * @param[out] x0  returned minimum x index where data not missing
   * @param[out] x1  returned maximum x index where data not missing
   * @param[out] y0  returned minimum y index where data not missing
   * @param[out] y1  returned maximum y index where data not missing
   *
   * @return false if all data is missing 
   */
  bool boundingBox(int &x0, int &x1, int &y0, int &y1) const;

  /**
   * compute weighted centroid location
   *
   * @param[in] maskvalue data value to look for in local grid
   * @param[in] weights   weight value at each grid point to use
   * @param[out] x        return centroid x location
   * @param[out] y        return centroid y location
   *
   * at all points where the local grid value is maskvalue compute the weighted
   * average x,y location using weights from input grid.
   * 
   * @return false if there is no data found with maskvalue, weights missing at
   * all points where data = maskvalue.
   */
  bool weightedCentroid(double maskvalue, const Grid2d &weights,
			int &x, int &y) const;

  /**
   * Return some information, and a potential new missing data value
   *
   * @param[in] debug true for debug output 
   * @param[out] missing  current missing data value
   * @param[out] newMissing potential new missing data value computed from data
   * @return name of this grid
   */
  std::string getInfoForAlg(const bool debug, double &missing,
			    double &newMissing) const;

  /**
   * Return some information.
   *
   * @param[out] missing  current missing data value
   * @return name of this grid
   */
  std::string getInfoForAlg(double &missing) const;

  /**
   * @return true if the input and local grids have intersecting non-missing
   *         locations.
   * @param[in] g  grid to compare to local grid.
   * @param[out] quality a measure of how much intersection there was
   */
  bool intersects(const Grid2d &g, double &quality) const;

  /**
   * @return true if the input and local grids are both not missing at a point
   * @param[in] i  index of point
   * @param[in] g  grid to compare to local grid.
   * 
   * @param[in,out] n0 incremented if local grid is not missing at i.   
   * @param[in,out] n1 incremented if input grid is not missing at i.   
   * @param[in,out] ngood0 incremented if BOTH grids are not missing at i
   * @param[in,out] ngood1 incremented if BOTH grids are not missing at i
   */
  inline bool intersects(const int i, const Grid2d &g, int &ngood0, int &n0,
			 int &ngood1, int &n1) const;

  /**
   * @return max of data in a box around a point, missing value if nothing
   *
   * @param[in] ix  center point X
   * @param[in] iy  center point Y
   * @param[in] sx  Radius of box in X
   * @param[in] sy  Radius of box in Y
   */
  double localMax(const int ix, const int iy, const int sx, 
		  const int sy) const;

  /**
   * @return average of data in a box around a point, missing value if nothing
   *
   * @param[in] ix  center point X
   * @param[in] iy  center point Y
   * @param[in] sx  Radius of box in X
   * @param[in] sy  Radius of box in Y
   * @param[in] needHalf  True will require half the data to be non-missing
   *                      to compute an average, otherwise output set to missing
   *                      False will compute an average if any data is
   *                      non-missing
   */
  double localAverage(const int ix, const int iy, const int sx, 
		      const int sy, bool needHalf=false) const;
  /**
   * @return average of data in a box around a point, missing value if 
   *         ANY data in the box is missing.
   *
   * @param[in] ix  center point X
   * @param[in] iy  center point Y
   * @param[in] sx  Radius of box in X
   * @param[in] sy  Radius of box in Y
   */
  double localAverageNoMissing(const int ix, const int iy, const int sx, 
			       const int sy) const;

  /**
   * @return average of data for all y at a fixed x, missing if nothing.
   *
   * @param[in] ix  Index fixed
   */
  double averageAtX(const int ix) const;

  /**
   * @return maximum of data for all y at a fixed x, missing if nothing.
   *
   * @param[in] ix  Index fixed
   */
  double maxAtX(const int ix) const;

  /**
   * Compute average angle value over all points where mask grid is not missing
   *
   * @param[in] mask  Grid to use as mask
   * @param[out] a  The average angle
   *
   * @return false if all local data in mask is missing so no averaging
   *         is possible
   *
   * @note local grid is expected to contains angle values [-180,180].
   */
  bool angleAverageInMask(const Grid2d &mask, double &a) const;

  /**
   * Compute average data value over all points where mask grid is not missing
   *
   * @param[in] mask  Grid to use as mask
   * @param[out] a  The average value
   *
   * @return false if all local data in mask is missing so no averaging is
   *         possible
   */
  bool averageInMask(const Grid2d &mask, double &a) const;

  /**
   * @return maximum data value over all points where mask grid not missing,
   *         or the missing value if all mask points are locally missing
   * @param[in] mask  Grid to use as mask
   */
  double maxInMask(const Grid2d &mask) const;

  /**
   * Within a mask find the point at which the local average is largest.
   *
   * At all non-missing mask points, count the number of local data values
   * within a radius of the point that exceed a threshold.  The returned point
   * is the one with the highest count.
   *
   * @param[in] mask Grid to use as mask
   * @param[in] radius  parameter specifying 'local' at each point (grid points)
   * @param[in] threshold  Data threshold parameter
   * @param[out] x  Point where average is largest
   * @param[out] y  Point where average is largest
   * 
   * @return false if no points meet criteria
   */
  bool largestAverageExceedingThreshInMask(const Grid2d &mask,
					   double radius, double threshold,
					   int &x, int &y) const;

  /**
   * Get the data range at points where mask is not missing 
   *
   * @param[in] mask  Mask grid
   * @param[out] v0  Minimum data value at such points
   * @param[out] v1  Maximum data value at such points
   *
   * @return true if successful and there was non-missing data 
   */
  bool rangeInMask(const Grid2d &mask, double &v0, double &v1) const;

  /**
   * at each point where the local value is non-missing and the mask is
   * non-missing, get the absolute difference between the input 'other' and
   * the value, return the average such difference.
   *
   * @param[in] mask  The mask
   * @param[in] other  The other data
   * @param[out] aveDiff  Returned value
   *
   * @return true for success
   *
   * @note The local and other grids are assumed to have 'orientations' which
   * are in the range 0 to 90, like a line.
   */
  bool angleDifferenceInMask(const Grid2d &mask,
			     const Grid2d &other,
			     double &aveDiff) const;
  /**
   * @return percent of non-missing values in the local image that are also
   * non-missing in the mask
   *
   * @param[in] mask
   */
  double percentInMask(const Grid2d &mask) const;
  
  /**
   * returns average diff in motion angle values in the mask.
   *
   * @param[in] mask  A mask grid
   * @param[in] other  A second grid with motion angles in it.
   * @param[out] motionDiff Returned value
   *
   * at each point where the motion angle is non-missing and within the
   * mask, get the difference between the input other motion angle and
   * the angle, return the average such diff.
   *
   * @note assumes local grid and other have 'motion angles', which are
   * in range 0 to 180.
   */
  bool motionDifferenceInMask(const Grid2d &mask,
				 const Grid2d &other,
				 double &motionDiff) const;

  /**
   * set data in x range [0, x0] and from [x1,_nx-1] to missing value
   *
   * @param[in] x0
   * @param[in] x1
   *
   * @note if x1 is -1, do not set [x1,_nx-1] to missing.
   */
  void adjust(int x0, int x1);

  /**
   * increment value at x,y by input value
   * @param[in] x  Index
   * @param[in] y  Index
   * @param[in] value  Value to increment by
   */
  void increment(int x, int y, double value);

  /**
   * Multiply value at x,y by input value
   * @param[in] x  Index
   * @param[in] y  Index
   * @param[in] value  Value to multiply by
   */
  void multiply(int x, int y, double value);

  /**
   * multiply values in two grids together.
   * At each gridpoint, multiply the values from each grid, result to local grid
   * @param[in]  g  Grid to use
   */
  void multiply(const Grid2d &g);

  /**
   * Multiply content of grid by input value
   * At each gridpoint, multiply the value by input
   * @param[in] value  Value to use
   */
  void multiply(const double value);

  /**
   * Add input value to grid.
   * At each gridpoint, Add the input value to local value
   * @param[in] value  Value to use
   */
  void add(const double value);

  /**
   * Set each point to max of local value and input grid value at point.
   * @param[in] g  Grid to compare to 
   */
  void max(const Grid2d &g);

  /**
   * Add values from input grid to local grid, and where do so increment
   * a count grid.
   * @param[in] g  Grid to add into local grid.
   * @param[in,out] count  Incremented at each point where adding happened.
   */
  void add(const Grid2d &g, Grid2d &count);

  /**
   * Add values from input grid to local grid.
   * At each gridpoint, add the values from the grids, result to local grid
   * @param[in]  g  Grid to use
   */
  void add(const Grid2d &g);

  /**
   * divide value at each gridpoint where div is not missing or 0 by that value.
   * @param[in]  div  Grid to use
   */
  void divide(const Grid2d &div);

  /** 
   * At all locations where grid = oldvalue, set it to newvalue
   * @param[in] oldvalue
   * @param[in] newvalue
   */
  void change(double oldvalue, double newvalue);

  /** 
   * At all locations where grid <= oldvalue, set it to newvalue.
   * Change all places with bad or missing to newvalue.
   *
   * @param[in] oldvalue
   * @param[in] newvalue
   */
  void changeLessOrEqual(double oldvalue, double newvalue);

  /** 
   * At all locations where grid has one of oldvalue, set it to newvalue
   *
   * @param[in] oldvalue  Any number of values
   * @param[in] skipvalue  A particular value to skip in local data
   * @param[in] newvalue
   *
   * @note skipvalue put in for efficiency where most of the grid might have
   * one value that is not to be replaced.
   */
  void change(std::vector<double> oldvalue, double skipvalue, double newvalue);

  /**
   * Merge contents of input grid into local grid, where not missing.
   * @param[in] g  Grid to merge in.
   * 
   * @note at points where g is missing, the original value is preserved.
   */
  bool merge(const Grid2d &g);

  /**
   * Apply a sx by sy smoothing filter to the local grid
   * @param[in] sx
   * @param[in] sy
   */
  void smooth(const int sx, const int sy);
  void smoothThreaded(const int sx, const int sy, int numThread);

  /**
   * Apply a sx by sy smoothing filter to the local grid, 
   * such that if any data is missing in the smooth window, the output
   * is set to missing
   * 
   * @param[in] sx
   * @param[in] sy
   */
  void smoothNoMissing(const int sx, const int sy);

  /**
   * Apply a sx by xy smoothing filter at all points where data is missing
   * so as to fill gaps in data.
   *
   * @param[in] sx
   * @param[in] sy
   */
  void fillGaps(const int sx, const int sy);

  /**
   * Reduce the resolution of data by a factor f
   *
   * @param[in] f   Reduction 1,2,3,...
   */
  void reduce(const int f);

  /**
   * Reduce resolution of data by a factor f using max values in the
   * gridpoints that span the reduction at each low res point
   *
   * @param[in] f  Reduction 1,2,3,...
   */
  void reduceMax(const int f);

  /**
   * Reduce resolution of data by different factors in each dimension 
   * using max values in the spand of points.
   * @param[in] fx   Reduction in x  1,2,..
   * @param[in] fy   Reduction in y  1,2,..
   */
  void reduceMax(const int fx, const int fy);

  /**
   * Interpolate input grid back onto local grid using bilinear method
   *
   * @param[in] lowres  Low res grid
   * @param[in] res  Reduction factor for low res (1,2,..)
   *
   * @note expect (local nx) = (input nx)*res, (local ny)=(input ny)*res
   */
  bool interpolate(const Grid2d &lowres, const int res);

  /** 
   * A second dialation alg - each pixel in the output image is the maximum
   * of the pixels in a neighborhood window in the input image.
   *
   * @param[in] nx  Dilation window x
   * @param[in] ny  Dilation window y
   */
  void dilate2(const int nx, const int ny);

  /** 
   * At all points where a window around it has a value, set output to the
   * value. At all other points, nothing happens.
   *
   * @param[in] value   Value to look for
   * @param[in] nx  Dilation window x
   * @param[in] ny  Dilation window y
   */
  void dilateOneValue(const double value, const int nx, const int ny);

  /**
   * calculate the median bounded in the x and y dimension
   * @return median value
   * @param[in] xLwr  Lower range of x over which to take median
   * @param[in] xUpr  Upper range of x over which to take median
   * @param[in] yLwr  Lower range of y over which to take median
   * @param[in] yUpr  Upper range of y over which to take median
   */
  double medianXy(const int xLwr, const int xUpr, const int yLwr,
		  const int yUpr);
  /**
   * calculate the mean bounded in the x and y dimension
   * @return mean value
   * @param[in] xLwr  Lower range of x over which to take mean
   * @param[in] xUpr  Upper range of x over which to take mean
   * @param[in] yLwr  Lower range of y over which to take mean
   * @param[in] yUpr  Upper range of y over which to take mean
   */
  double meanXy(const int xLwr, const int xUpr, const int yLwr, const int yUpr);

  /**
   * calculate the standard deviation bounded in the x and y dimension
   * @return standard deviation
   * @param[in] xLwr  Lower range of x over which to take stddev
   * @param[in] xUpr  Upper range of x over which to take stddev
   * @param[in] yLwr  Lower range of y over which to take stddev
   * @param[in] yUpr  Upper range of y over which to take stddev
   */
  double sdevXy(const int xLwr, const int xUpr, const int yLwr, const int yUpr);


  /**
   * Median over the entire grid
   * @param[in] nx  Median window size x
   * @param[in] ny  Median window size y
   * @param[in] binMin  Data minimum bin center (histograms)
   * @param[in] binMax  Data maximum bin center (histograms)
   * @param[in] binDelta Data diff between bin centers (histograms)
   */
  void median2(const int nx, const int ny, double binMin, double binMax,
	       double binDelta);

  /**
   * Create a 'speckle' measure, defined as the difference
   * between the 75th percentile data value and the 25th percentile data
   * value. This is done using bins like in the median calculations.
   * 
   * @param[in] nx  Percentile window size x
   * @param[in] ny  Percentile window size y
   * @param[in] binMin  Data minimum bin center (histograms)
   * @param[in] binMax  Data maximum bin center (histograms)
   * @param[in] binDelta Data diff between bin centers (histograms)
   *
   * The local grid is transformed into 'speckle' data.
   */
  void speckle(const int nx, const int ny, double binMin, double binMax,
	       double binDelta);

  /**
   * Create a 'speckle' interest measure from data in a grid, using fuzzy
   * remappings.  This is done using bins like in the median calculations.
   * 
   * @param[in] nx  Percentile window size x
   * @param[in] ny  Percentile window size y
   * @param[in] binMin  Data minimum bin center (histograms)
   * @param[in] binMax  Data maximum bin center (histograms)
   * @param[in] binDelta Data diff between bin centers (histograms)
   *
   * @param[in] fuzzyDataDiff  Mapping from data width to interest
   *                           with large difference getting high interest
   *
   * @param[in] fuzzyCountPctDiff  Mapping from difference in percent counts
   *                               (counts/totalcounts)
   *                               to interest, with small difference getting
   *                               high interest
   *
   *
   * The local grid is transformed into 'speckle interest' data, with high
   * interest where there is speckle, meaning:
   *
   *   data[50] - data[25] is large  AND
   *   data[75] - data[50] is large  AND
   *   |count[75] - count[50]| is small  AND
   *   |count[50] - count[25]| is small
   *
   *
   * where data[i] = percentile data value at percent=i
   * and where count[i] = count data value at percent=i / total count
   */
  void speckleInterest(const int nx, const int ny, double binMin, double binMax,
		       double binDelta, const FuzzyF &fuzzyDataDiff,
		       const FuzzyF &fuzzyCountPctDiff);


  /**
   * Median over the entire grid, with no overlapping boxes (output is
   * replicated within each box, one computation per box, each shift is a
   * full box)
   * @param[in] nx  Median window size x
   * @param[in] ny  Median window size y
   * @param[in] binMin  Data minimum bin center (histograms)
   * @param[in] binMax  Data maximum bin center (histograms)
   * @param[in] binDelta Data diff between bin centers (histograms)
   * @param[in] allowAnyData  True to take medians for any number of
   *                            non-missing data per box, false to take the
   *                            median only when at least half the data
   *                            in the box is non-missing.
   */
  void medianNoOverlap(const int nx, const int ny, const double binMin,
			 const double binMax, const double binDelta,
			 const bool allowAnyData);

  /**
   * For each point, set value to standard deviation in a xw by yw window around
   * the point
   * @param[in] xw  Width (x)
   * @param[in] yw  Width (y)
   */
  void sdev2Old(const int xw, const int yw);

  void sdevThreaded(const int sx, const int sy, int numThread);
  void textureXThreaded(const int sx, const int sy, int numThread);
  void textureYThreaded(const int sx, const int sy, int numThread);

  /**
   * For each point, set value to standard deviation in a xw by yw window around
   * the point, fast method
   * @param[in] nx  Width (x)
   * @param[in] ny  Width (y)
   */
  void sdev2(const int nx, const int ny);

  /**
   * For each point, set value to standard deviation in a xw by yw window around
   * the point, fast method, with no overlapping boxes (output is replicated
   * within each box, one computation per box, each shift is a full box)
   * @param[in] nx  Width (x)
   * @param[in] ny  Width (y)
   */
  void sdevNoOverlap(const int nx, const int ny);

  /**
   * Compute x texture at a scale
   * @param[in] nx  X scale
   * @param[in] ny  Y scale
   */
  void texture2X(const int nx, const int ny);

  /**
   * Compute y texture at a scale
   * @param[in] nx  X scale
   * @param[in] ny  Y scale
   */
  void texture2Y(const int nx, const int ny);

  //   void texture2XOld(const int nx, const int ny);
  //   void texture2YOld(const int nx, const int ny);

  /**
   * Smooth data at a scale
   * @param[in] xw  X scale
   * @param[in] yw  Y scale
   */
  void smooth2(const int xw, const int yw);

  /**
   * Smooth data at a scale, if data missing in the averaage, output is missing
   * @param[in] xw  X scale
   * @param[in] yw  Y scale
   */
  void smooth2NoMissing(const int xw, const int yw);

  /**
   * convert this grid from db to linear
   */
  void db2linear(void);

  /**
   * convert this grid from linear to db 
   */
  void linear2db(void);

  /**
   * Take max over a range of x for all y, apply to all y values at each x
   * @param[in] nx  Range of x over which to get max
   */
  void xMaxForAllY(const int nx);

  /**
   * Take max over a range of x for all y, apply to all y values at each x.
   * If data below threshold, don't contribute to max
   * 
   * @param[in] nx  Range of x over which to get max
   * @param[in] minV  Threshold
   */
  void xMaxForAllY(const int nx, const double minV);

  /**
   * Take average over a range of x for all y, apply to all y values at each x
   * @param[in] nx  Range of x over which to get max
   */
  void xAverageForAllY(const int nx);

  /**
   * Take average over a range of x for all y, apply to all y values at each x
   * If data below threshold, don't contribute to ave.
   * 
   * @param[in] nx  Range of x over which to get max
   * @param[in] minV  Threshold
   */
  void xAverageForAllY(const int nx, const double minV);

  /**
   * Compute percent of values exceeding a threshold over a range of x for all
   * y, apply to all y values at each x.  Return range of x at which the
   * percentage was greater than a min.
   * 
   * @param[in] nx  Range of x over which to get max
   * @param[in] minV  Threshold
   * @param[in] minPct   Minimum percent used to set x0, x1
   * @param[out] x0  Minimum x with at least minPct
   * @param[out] x1  Maximum x with at least minPct
   */
  void xPcntGeForAllY(const int nx, const double minV, const double minPct,
			   int &x0, int &x1);

  /**
   * Remove all points that are not part of a clump (set to missing)
   */
  void clumpFilter(void);

  /**
   * Set all points within n of x,y to missing
   * @param[in] x
   * @param[in] y
   * @param[in] n
   */
  void clearNear(const int x, const int y, const int n);

  /**
   * Shift grid in x by this many points, fill shifted 'from' points with
   * missing.
   * @param[in] npt  Number of shift points
   */
  void shiftX(const int npt);

  /**
   * Set local grid to missing whereever mask input grid is within input range,
   * inclusive (mask if the data in range [low,high])
   * @param[in] mask  Grid to use as mask
   * @param[in] low  Low value to mask
   * @param[in] high  High value to mask
   */
  void maskRange(const Grid2d &mask, const double low, const double high);

  /**
   * Expand at each point by setting all neighboring points to the same value
   */
  void boxExpand(void);

  /**
   * At each point where mask is not missing and local data is not missing,
   * multiply data by a scale factor
   *
   * @param[in] mask  Grid to use as mask
   * @param[in] scale
   */
  void rescaleInMask(const Grid2d &mask, double scale);

  /**
   * At each point where mask is not missing, set data to value
   * @param[in] mask  Grid to use as mask
   * @param[in] value
   */
  void fillInMask(const Grid2d &mask, double value);

  /*
   * At each point where mask is missing, set data to missing
   * @param[in] mask  Grid to use as mask
   */
  void maskMissingToMissing(const Grid2d &mask);

  /**
   * At each point where mask is not missing, set data to the mask value
   * @param[in] mask  Grid to use as mask
   */
  void fillInMask(const Grid2d &mask);

  /** 
   * At all points where data is missing and mask data is not missing, set 
   * local data to a value, and set mask at that point to missing
   *
   * @param[in] mask  Mask grid
   * @param[in]  v  The value to use
   */
  void clearMaskWhereMissingWhileSetting(Grid2d &mask, double v);

  /**
   * Fill in gaps in the mask.
   *  
   * @param[in] mask  Mask grid
   *
   * At all points where data is missing and mask data is not missing, set local
   * value to the nearest local non-missing value.
   *
   * This fills in gaps in a mask..
   */
  void expandInMask(const Grid2d &mask);

  /**
   * At all points where the input mask grid is missing, set the local data to
   * missing.  This leaves non-missing values locally only where both local
   * and input grids are non-missing, i.e. the intersection
   *
   * @param[in] mask  Mask grid
   */
  void intersection(const Grid2d &mask);


  /**
   * At the point closest to x,y for which mask data = a maskValue and
   * both input data and conf are not  missing, set the local data to the
   * input data value at that point, and outConf to the input conf value
   * at that point
   * @param[in] data  Data grid
   * @param[in] conf  Confidence grid
   * @param[in] x  Grid point index
   * @param[in] y  Grid point index
   * @param[in] mask  Mask grid
   * @param[in] maskValue  Value to look for in mask
   * @param[in,out] outConf   Output confidence grid
   */
  void nearestSameWithMaskSame(const Grid2d &data,
				   const Grid2d &conf,
				   int x, int y,
				   const Grid2d &mask,
				   const double  maskValue,
				   Grid2d &outConf);

  /**
   * At each grid point, set the output value to the value r0 + i*res that
   * is closest to its original value, choosing i such that the result is
   * in the range r0 to r1
   * @param[in] res
   * @param[in] r0
   * @param[in] r1
   */
  void roundToNearest(double res, double r0, double r1);

  /**
   * Set local value to maskValue at each point for which
   * the average of input data values within a radius of the point exceeds
   * a threshold.

   * @param[in] data Input data values Grid
   * @param[in] minData  the threshold
   * @param[in] minArea  the radius
   * @param[in] maskValue  the value to set the local values to where
   *                        condition is met.
   */
  void thinlineMask(const Grid2d &data, double minData, double minArea,
		     double maskValue); 

  /**
   * Set input data values to missing whereever local image is missing.
   * @param[in,out] data  The grid to modify
   */
  void copyMissingToInput(Grid2d &data) const;

  /**
   * At each point where mask is not missing, set local value to missing
   * @param[in] mask Mask grid
   */
  void setMaskToMissing(const Grid2d &mask);

  /**
   * Return average angle value for data points where mask is not missing
   * @param[in] mask  Mask grid
   * @param[out] ave  The average
   *
   * @return true if successful
   * 
   * Assumes the local data valuse are angles 0 to 360 with wraparound
   * meaning this is non deterministic
   */
  bool orientationAngleAverageInMask(const Grid2d &mask, double &ave) const;

  /**
   * Set all data values that are below a threshold to missing
   *
   * @param[in] thresh
   */
  void belowThresholdToMissing(const double thresh);

  /**
   * Set all values that are below a threshold grid to missing,
   * i.e. if data[i] < thresh[i], data[i]  is set missing
   * @param[in] thresh  Thresholds grid
   */
  void thresholdMask(const Grid2d &thresh);


  /**
   * At all points where data is missing, set it to a value,
   * at all pointer where data is not missing, set it to missing
   *
   * @param[in] value  Value to use when inverting. If value happens
   *                   to equal missing, use value+1.
   */
  void invert(double value);

  /**
   * Set data to missing everywhere it is not equal to a particular value
   * @param[in] v  The particular value
   */
  void maskExcept(double v);


  /**
   * Rearrange grid in y by a wraparound shiting
   * @param[in] n
   *
   *     for all y>=n:   data[x,y-n] = data[x,y]  left shift
   *     for all y< n:   data[x,y-n+ny] = data[x,y]  left shift with wraparound
   */
  void shiftY(int n);

  /**
   * Compute method used in threaded algorithms
   */
  static void compute(void *ti);


protected:

private:

  /**
   * @class AlgThreads
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   * the clone() method.
   */
  class GridAlgThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Empty constructor
     */
    inline GridAlgThreads() : TaThreadDoubleQue() {}
    /**
     * Empty destructor
     */
    inline virtual ~GridAlgThreads() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index 
     */
    TaThread *clone(const int index);
  };

  class GridAlgsInfo
  {
  public:
    typedef enum {SMOOTH, SDEV, TEXTURE_X, TEXTURE_Y, NONE} Info_t;
    
    inline GridAlgsInfo(Info_t t, int sx, int sy, int iy, const GridAlgs *alg,
			Grid2d &out) :
      _type(t), _sx(sx), _sy(sy), _y(iy), _gridAlgs(alg), _out(out) {}
    inline virtual ~GridAlgsInfo(void) {}

    Info_t _type;
    int _sx;
    int _sy;
    int _y;
    const GridAlgs *_gridAlgs;
    Grid2d &_out;
  protected:
  private:
  };


  /**
   * @return ordered (based on taxicab metric distance, small to large)
   * indices into the box at radius r from x,y.
   */
  std::vector<int> _orderedIndices(int x, int y, int r) const;

  /**
   * @return the mean of the data values inside a box, _missing for none
   * @param[in] x0  Lower left corner index
   * @param[in] y0  Lower left corner index
   * @param[in] nx  Box number of points x
   * @param[in] ny  Box number of points y
   */
  double _meanInBox(const int x0, const int y0, const int nx,
		    const int ny) const;

  /**
   * @return the standard deviation of the data values inside a box,
   *         _missing for none
   * @param[in] x0  Lower left corner index
   * @param[in] y0  Lower left corner index
   * @param[in] nx  Box number of points x
   * @param[in] ny  Box number of points y
   * @param[in] xbar  Mean value in the box
   */
  double _sdevInBox(const int x0, const int y0, const int nx,
		    const int ny, const double xbar) const;

  void _appendIfOk(int x, int y, std::vector<int> &o) const;
  double _bilinear(int ry0, int rx0, int res, int y, int x, 
		   const Grid2d &lowres);



  double _dilate(const int x, const int y, const int xw, const int yw) const;
  double _dilateOneValue(const double value, const int x, const int y,
			 const int xw, const int yw) const;

  double _median(const int x, const int y, const int xw, const int yw) const;
  double _sdev(const int x, const int y, const int xw, const int yw,
	       bool needHalf=false) const;
  double _sdev2(const int x, const int y, const int xw, const int yw) const;
  double _textureX(const int x, const int y, const int xw, const int yw,
		   bool needHalf=false) const;
  double _textureY(const int x, const int y, const int xw, const int yw,
		   bool needHalf=false) const;

  void _fillEdge(const int xw, const int yw, const double value);
  bool _fillHole(const int n, const int ix, const int iy);
  bool _fillHhole(const int n, const int ix, const int iy);
  bool _fillVhole(const int n, const int ix, const int iy);

  void _sdev2Final(const int xw, const int yw, const Grid2d &tmp,
		    Grid2dLoop &G, double &A, double &B, double &N);
  void _textureXNew(const int xw, const int yw, const Grid2d &tmp,
		      Grid2dLoop &G, double &A, double &N);
  void _textureYNew(const int xw, const int yw, const Grid2d &tmp,
		      Grid2dLoop &G, double &A, double &N);
  void _smooth2(const int xw, const int yw, const Grid2d &tmp,
		Grid2dLoop &G, double &A, double &N);
  void _smooth2NoMissing(const int xw, const int yw, const Grid2d &tmp,
			   Grid2dLoop &G, double &A, double &N, int &nmissing);
  /**
   * Set data within a box to a constant value
   *
   * @param[in] x0  Lower left corner index
   * @param[in] y0  Lower left corner index
   * @param[in] nx  Box number of points x
   * @param[in] ny  Box number of points y
   * @param[in] v  Value to fill box with
   */
  void _fillBox(const int x0, const int y0, const int nx,
		const int ny, const double v);

};

#endif
