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
#include <toolsa/copyright.h>
/**
 * @file Grid2dOffset.hh
 * @brief One dimensional offset indices into Grid2d, used as a lookup table.
 * @class Grid2dOffset
 * @brief One dimensional offset indices into Grid2d, used as a lookup table.
 *
 * Offsets are from a center point to Grid2d locations.
 * This is done for efficiency, giving relative offsets that can be 
 * reused.
 */

# ifndef    GRID2D_OFFSET_HH
# define    GRID2D_OFFSET_HH

#include <vector>
#include <euclid/Grid2d.hh>
class Grid2d;


class Grid2dOffset
{
public:

  /**
   * Default constructor.
   */
  Grid2dOffset(void);

  /**
   * Set the Grid2d missing value for later
   *
   * @param[in] missing
   */
  Grid2dOffset(const double missing);

  /**
   * Offsets to some points along one side of a line
   *
   * The line is assumed rotated counterclockwise from vertical, and is centered
   * at 0,0
   * 
   * @param[in] length  Length of line.
   * @param[in] width   Number of pixels defining the side of the line.
   *                    if line were vertical, +width means to right,
   *                    -width to left.
   * @param[in] ang  Rotation angle counterclockwise degrees 
   * @param[in] step  Number of pixels between each offset point along the line.
   *                  This gives a subset of total number of pixels.
   * @param[in] nx Grid number of points x
   * @param[in] missing  Data missing data value
   */
   Grid2dOffset(const double length, const double width, const double ang,
		const double step,  const int nx, const double missing);

  /**
   * Offsets to all the point inside a rotated square box of radius r.
   *
   * @param[in] r radius
   * @param[in] ang  Rotation angle (mathematical degrees)
   * @param[in] nx  Number of gridpoints X
   * @param[in] missing  Missing data value
   */
  Grid2dOffset(const double r, const double ang, const int nx,
	       const double missing);
    
  /**
   * Offsets to all the point inside a rotated rectangle 
   *
   * @param[in] length  Box length (vertical extent when ang=0).
   * @param[in] width   Box width (horizontal extent when ang=0),
   * @param[in] ang  Rotation angle (mathematical degrees)
   * @param[in] nx  Number of gridpoints x
   * @param[in]  missing missing data value
   */
  Grid2dOffset(double length, double width, double ang, int nx,
	       double missing);

  /**
   * special constructor for 'octogon' around a point
   *
   * @param[in] radius  Radius of octogon
   * @param[in] nx   Grid number of x points
   * @param[in] imissing  Input missing data value
   *
   */
  Grid2dOffset(double radius, int nx, double imissing);

  /**
   * Destructor
   */
  virtual ~Grid2dOffset(void);
    
  /* 
  * @return total # of offset indices.
  */
  inline int num(void) const {return _offsets.size();}

  /**
   * @return ith offset index value, or -1 for out of range
   *
   * @param[in] i
   */
  inline int ith(int i) const
  {
    if (i < 0 || i >= (int)_offsets.size())
      return -1;
    else
      return *(_offsets.begin() + i);
  }

  /**
   * @return the Grid2d index for the ith offset when centered at x,y,
   * or -1 when out of range
   *
   * @param[in] i
   * @param[in] g  Grid used to figure out actual index
   * @param[in] x  Center point
   * @param[in] y  Center point
   */
  inline int ithActual(int i, const Grid2d &g, const int x, const int y) const
  {
    if (i < 0 || i >= (int)_offsets.size())
      return -1;
    else
      return g.ipt(x, y) + _offsets[i];
  }

  /**
   * @return angle of offset
   */
  inline double getAngle(void) const {return _angle;}
    
  /**
   * @return max offset distance (pixels).
   */
  inline int maxOffset(void) const {return _max;}

  /**
   * @return true if x,y value for given ny,nx puts at least one of the
   * offset points out of bounds.
   *
   * @param[in] x  Center point
   * @param[in] y  Center point
   * @param[in] nx  Number of x points in grid
   * @param[in] ny  Number of y points in grid
   */
  inline bool outOfRange(int x, int y, int nx, int ny) const
  {
    return (x < _max || y < _max || 
	    x >= nx - _max || y >= ny - _max);
  }

  /**
   * @return return count of missing values at the offset points
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   */
  int numFlagged(const Grid2d &g, const int x, const int y) const;

  /**
   * @return return count of non-missing values at the offset points
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   */
  int numNonFlagged(const Grid2d &g, const int x, const int y) const;

  /**
   * Return average of non-missing values at the offset points
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[out] v  Value returned
   *
   * @return true for success
   */
  bool average(const Grid2d &g, const int x, const int y, double &v) const;

  /**
   * @return average of values at the offset points, missing data is given
   * the value=0
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   */
  double averageMissingZero(const Grid2d &g, const int x, const int y) const;

  /**
   * Compute average of values exceeding a threshold
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] thresh  threshold value
   * @param[in] background
   * @param[out] v  The value
   *
   * 
   * @return false if all data is bad.
   * if some values are not missing but all are below thresh, return background.
   * if some vaalues are above thresh, return their average.
   *
   */
  bool average(const Grid2d &g, const int x, const int y, const double thresh,
	       const double background, double &v) const;

  /**
   * @return the percentage that is missing.
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   */
  double percentBad(const Grid2d &g, const int x, const int y) const;

  /**
   * @return true if at least p percent of the data is missing
   *
   * @param[in] p  Percent
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   */
  inline bool percentIsBad(double p, const Grid2d &g, const int x,
			     const int y) const
  {
    return (percentBad(g, x, y) >= p);
  }

  /**
   * @return all the values from the grid at the offset locations.
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   */
  std::vector<double> fillPixels(const Grid2d &g, const int x,
				 const int y) const;

  /**
   * return the sum of non-missing values at offsets
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[out] v  The sum value
   *
   * @return false if all values are flagged
   */
  bool sumValues(const Grid2d &g, const int x, const int y, double &v) const;

  /**
   * return the max value at offsets
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[out] v  The max
   *
   * @return false if all values are flagged
   */
  bool maxValue(const Grid2d &g, const int x, const int y, double &v) const;

  /**
   * @return max value in offsets, or 0 if all values are missing
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   *
   */
  double maxValueOrZero(const Grid2d &g, const int x, const int y) const;

  /**
   * return variance of values at offsets
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[out] v  The variance
   *
   * @return false if all values are flagged
   */
  bool variance(const Grid2d &g, const int x, const int y, double &v) const;

  /**
   * Compute variance of those values exceeding a threshold
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] thresh  threshold value
   * @param[in] background
   * @param[out] v  The value
   *
   * 
   * @return false if all data is bad
   *
   *
   * if some values are not missing but all are below thresh, return background
   *
   */
  bool variance(const Grid2d &g, const int x, const int y,
		const double thresh, const double  background,
		double &v) const;

  /**
   * return sum of squares of difference of value from a particular value,
   * with sum of square of (a - value[offset]) where value[offset]=missing
   * added in as square of (a - 0.0)
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] a  Particular value
   * @param[out] m  The sum of squares
   *
   * @return false if all values are missing
   */
  bool sumSqOffsets(const Grid2d &g, const int x, const int y,
		    const double a, double &m) const;

  /**
   * return median of non-missing values at offsets
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[out] v  The median
   *
   * @return false if all values are flagged
   */
  bool median(const Grid2d &g, const int x, const int y, double &v) const;

  /**
   * return median of non-missing values at offsets that are >=thresh.
   *
   * If some values are non-missing but all are below thresh, return background.
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] thresh
   * @param[in] background
   * @param[out] v  The median
   * 
   * @return false if all values are flagged
   */
  bool median(const Grid2d &g, const int x, const int y, const double thresh,
	      const double background, double &v) const;

  /**
   * return average of one half the offset values
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] bottom_half  If true, take the lower half of the offset index
   *                         values, if false, talke the upper half
   * @param[out] v  The average
   *
   * @return false if all values are missing
   */
  bool halfAverage(const Grid2d &g, const int x, const int y,
		    const bool bottom_half, double &v) const;

  /**
   * return average of half the offset values, replacing missing values
   * in the average with 0.0.  This means there will always
   * be an average returned.
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] bottom_half  If true, take the lower half of the offset index
   *                         values, if false, talke the upper half
   *
   * @return The average
   */
  double halfAverageMissingZero(const Grid2d &g, const int x, const int y,
				const bool bottom_half) const;

  /**
   * @return percent of points above (greater than) a threshold
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] thresh
   */
  double percentAbove(const Grid2d &g, const int x, const int y,
		       const double thresh) const;

  /**
   * @return percent of points below (less or equal) a threshold
   *
   * @param[in] g  Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] thresh
   */
  double percentBelow(const Grid2d &g, const int x, const int y,
		       const double thresh) const;


  /**
   * Return the number of offset points for which a mask is non-missing and
   * the data is greater or equal a threshold
   *
   * @param[in] data  Data Grid
   * @param[in] x  Centerpoint
   * @param[in] y  Centerpoint
   * @param[in] minv  Threshold
   * @param[in] mask  Mask Grid
   * 
   * @return the Number 
   */
  int numMaskedExceeding(const Grid2d &data, const int x, const int y,
			   const double  minv,  const Grid2d &mask) const;

private:

  std::vector<int> _offsets; /**< set of offsets from center pixel. (1d) */
  double _angle;             /** angle of the template. */
  int _max;                  /**< max pixel offset from center in the set. */
  double _missing;           /**< missing data value */
    
  void _initForBox(const double ang, const double length, const double width,
		   const int image_width);
  void _add(const double length, const double width, const double ang,
	    const double step, const int image_width);

  int _computeOffset(const int i, const double cs, const double sn,
		      const double width, const double step, 
		      const int npixels, const int image_width);
  void _updateOffsets(const int ix, const int iy, const double ang, 
		       const double x0r, const double x1r, const double y0r,
		       const double y1r, const int width);
  void _rotatePointBack(const double x, const double y, const double ang,
			  double &xmin,	double &ymin, double &xmax, 
			  double &ymax, const bool first);

  inline bool _getPtr(const Grid2d &g, const int x, const int y, int &ipt) const
  {
    if (g.inRange(x, y))
    {
      ipt = g.ipt(x, y);
      return true;
    }
    else
    {
      return false;
    }
  }
};

# endif
