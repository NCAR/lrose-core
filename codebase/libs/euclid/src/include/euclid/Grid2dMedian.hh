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
 * @file Grid2dMedian.hh
 * @brief fast median of 2d grid over a local scale using histograms
 * @class Grid2dMedian
 * @brief fast median of 2d grid over a local scale using histograms
 */

# ifndef    GRID2D_MEDIAN_H
# define    GRID2D_MEDIAN_H

#include <euclid/Grid2d.hh>
#include <vector>

class Grid2dMedian : public Grid2d
{

public:

  /**
   * @param[in] g  Grid to use as base class values used in computing medians
   * @param[in] xw  width of median box (x)
   * @param[in] yw  width of median box (x)
   * @param[in] res  Resolution of histogram bins
   * @param[in] min  Mimimum histogram bin value
   * @param[in] max  Maximum histogram bin value
   */
  Grid2dMedian(const Grid2d &g, const int xw, const int yw, const double res,
	       const double min, const double max);

  /**
   * Destructor
   */
  virtual ~Grid2dMedian(void);

  /**
   * clear so no data stored
   */
  void clear(void);

  /**
   * update to new set of inputs (a window of data)
   *
   * @param[in] newv  The indicies (x=first, y=second) to data to add
   * @param[in] oldv  The indicies (x=first, y=second) to data to remove
   */
  void update(const std::vector<std::pair<int,int> > &newv,
	      const std::vector<std::pair<int,int> > &oldv);

  /**
   * add one value to state
   *
   * @param[in] d  Value to add
   */
  void addValue(double d);

  /**
   * @return median, requires 50% of data to be non-missing
   *
   * if more than 50% is missing, return the missing data value from the
   * base class
   */
  double getMedian(void) const;

  /**
   *  @return median, allowing any number of non-missing data values
   */
  double getMedianAllData(void) const;

  /**
   * @return percentile, requiring 50% of data to be non-missing
   *
   * @param[in] pct  Percentile
   */
  double getPercentile(double pct) const;

  /**
   * @return percentile, allow any # of data values to be non-missing
   *
   * @param[in] pct Percentile
   */
  double getPercentileAllData(double pct) const;

  /**
   * @return bin count at a percentile, requiring 50% of data to be non-missing
   *
   * @param[in] pct  Percentile
   */
  double getCount(double pct) const;

  /**
   * @return bin count at a percentile, allow any # of data values to be
   * non-missing
   *
   * @param[in] pct Percentile
   */
  double getCountAllData(double pct) const;

  /**
   * @return total count
   */
  inline double getNum(void) const {return _nc;}

  /**
   * @return smallest bin value with count > 0, missing if none
   */
  double smallestBinWithCount(void) const;

  /**
   * @return largest bin value with count > 0, missing if none
   */
  double largestBinWithCount(void) const;


protected:
private:

  int _xw;  /**< window size, number of points, x, over which to take median */
  int _yw;  /**< window size, number of points, y, over which to take median */
  int _nbin;         /**< Number of bins */
  double _binMin;    /**< Smallest bin value */
  double _binMax;    /**< largest bin value */
  double _binDelta;  /**< Bin increment */
  std::vector<double> _bin;      /**< All the bin values */
  std::vector<double> _counts;   /**< counts within each bin */
  int _nc;                       /**< Sum of all counts */

  void _subtract(const int x, const int y);
  void _add(const int x, const int y);
  void _removeValue(const double d);
  void _addValue(double d);
  double _pcntile(double pct) const;
  double _count(double pct) const;
};

#endif
