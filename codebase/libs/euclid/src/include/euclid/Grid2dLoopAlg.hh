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
 * @file Grid2dLoopAlg.hh 
 * @brief Algorithms to apply at each step of Grid2dLoopA.
 * @class Grid2dLoopAlg
 * @brief Algorithms to apply at each step of Grid2dLoopA.
 */

#ifndef GRID2DLOOPALG_H
#define GRID2DLOOPALG_H

// #include <string>
#include <rapmath/FuzzyF.hh>
#include <vector>

class Grid2d;

/*
 * @class Grid2dLoopA 
 * @brief Algorithm to apply at each step of Grid2dLoopA.  virtual base class
 *
 * The algorithms are over a set of points, which gert added and subtracted
 * individually
 */
//------------------------------------------------------------------
class Grid2dLoopAlg
{
public:
    
  /**
   * Constructor
   */
  inline Grid2dLoopAlg(void) {}

  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlg(void) {}

  /**
   * Add data at a point to the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void increment(int x, int y, const Grid2d &G) = 0;

  /**
   * Subtract data at a point from the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void decrement(int x, int y, const Grid2d &G) = 0;

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const = 0;

  protected:
  private:
};

//------------------------------------------------------------------
/**
 * @class Grid2dLoopAlgMean
 * @brief Mean data value algorithm
 */
class Grid2dLoopAlgMean : public Grid2dLoopAlg
{
public:
  /**
   * Constructor
   */
  inline Grid2dLoopAlgMean(void) : Grid2dLoopAlg(), _A(0), _N(0) {}

  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlgMean(void) {}

  /**
   * Add data at a point to the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void increment(int x, int y, const Grid2d &G);

  /**
   * Subtract data at a point from the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void decrement(int x, int y, const Grid2d &G);

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const;

protected:
private:
  double _A;  /**< Sum of all the data values in state */
  double _N;  /**< Number of data points in state */
};

//------------------------------------------------------------------
/**
 * @class Grid2dLoopAlgMeanNoMissing
 * @brief mean at each point, with result set to missing if any values are
 *        missing.
 */
class Grid2dLoopAlgMeanNoMissing : public Grid2dLoopAlg
{
public:
  /**
   * Constructor
   */
  inline Grid2dLoopAlgMeanNoMissing(void) : Grid2dLoopAlg(), _A(0), _N(0),
					    _nmissing(0) {}
  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlgMeanNoMissing(void) {}

  /**
   * Add data at a point to the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void increment(int x, int y, const Grid2d &G);

  /**
   * Subtract data at a point from the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void decrement(int x, int y, const Grid2d &G);

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const;

protected:
private:
  double _A;  /**< Sum of values */
  double _N;  /**< Number in the sum */
  int _nmissing; /**< Number of missing data values */
};

//------------------------------------------------------------------
/**
 * @class Grid2dLoopAlgSdev
 * @brief Standard deviation
 *
 * consider this: sdev = sqrt(sum (xi-xbar)**2/N)
 *                     = sum(xi**2 ) -2*xbar*sum(xi) + sum(xbar**2)
 *                xbar = sum(xi)/N
 *                sdev = sqrt(Z/N)
 *  where
 *           Z = sum(xi**2) -2*sum(xi)*xbar + N*xbar*xbar
 *             = sum(xi**2) -2*sum(xi)*sum(xi)/N + N*sum(xi)*sum(xi)/(N*N)
 *             = sum(xi**2) -sum(xi)*sum(xi)/N
 *             = (N*sum(xi**2) - (sum(xi))**2)/N
 *    
 * we can therefore do this the fast way as follows. Let
 *   A = sum(xi**2)
 *   B = sum(xi)
 * 
 *   Z = (N*A - B*B)/N
 *   sdev = sqrt(Z/N) = sqrt(N*A-B*B)/N
 *
 * When we increment and get old/new points, we can say
 *   A(k+1) = A(k) - sum(xi**2)old + sum(xi**2)new
 *   B(k+1) = B(k) - sum(xi)old + sum(xi)new
 *   N(k+1) = N(k) - number removed + number added
 */
class Grid2dLoopAlgSdev : public Grid2dLoopAlg
{
public:
  /**
   * Constructor
   */
  inline Grid2dLoopAlgSdev(void) : Grid2dLoopAlg(), _A(0), _B(0), _N(0) {}

  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlgSdev(void) {}

  /**
   * Add data at a point to the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void increment(int x, int y, const Grid2d &G);

  /**
   * Subtract data at a point from the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void decrement(int x, int y, const Grid2d &G);

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const;

protected:
private:
  double _A;  /**<  sum of squares */
  double _B;  /**< sum of values */
  double _N;  /**< number of items */
};

//------------------------------------------------------------------
/**
 * @class Grid2dLoopAlgMedian
 * @brief Median using histogram bins
 */
class Grid2dLoopAlgMedian : public Grid2dLoopAlg
{
public:
  /**
   * Constructor
   * @param[in] min  Minimum histogram bin centerpoint
   * @param[in] max  Maximum histogram bin centerpoint
   * @param[in] delta  Spacing between histogram bins
   */
  Grid2dLoopAlgMedian(double min, double max, double delta);

  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlgMedian(void) {}

  /**
   * Add data at a point to the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void increment(int x, int y, const Grid2d &G);

  /**
   * Subtract data at a point from the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void decrement(int x, int y, const Grid2d &G);

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const;

  /**
   * Get the percentile value from the state
   * @param[in] pct  Percentile 0 to 1
   * @param[in] minGood  Minimum number of non-missing values required
   * @param[out] result  Returned value
   * @return true if percentile computed and returned
   */
  bool getPercentile(double pct, int minGood, double &result) const;

  /**
   * @return total count of values within all bins
   */
  inline double getNum(void) const {return _nc;}

protected:

  int _nbin;                   /**< Number of bins */
  double _binMin;              /**< Smallest bin value */
  double _binMax;              /**< largest bin value */
  double _binDelta;            /**< Bin increment */
  std::vector<double> _bin;    /**< All the bin values */
  std::vector<double> _counts; /**< counts within each bin */
  int _nc;                     /**< Sum of all counts */

  bool _pcntile(double pct, double &result) const;
  bool _count(double pct, double &result) const;

private:
};

//------------------------------------------------------------------
/**
 * @class Grid2dLoopAlgTexture
 * @brief Texture algorithm, X or Y
 *
 * Texture X is the average of sum of squares of (data(x,y)-data(x,y-1))
 * Texture Y is the average of sum of squares of (data(x,y)-data(x-1,y))
 */
class Grid2dLoopAlgTexture : public Grid2dLoopAlg
{
public:
  /**
   * Constructor
   * @param[in] isX  True for X texture, false for Y
   */
  inline Grid2dLoopAlgTexture(bool isX) : Grid2dLoopAlg(), _isX(isX), _A(0),
					  _N(0) {}
  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlgTexture(void) {}

  /**
   * Add data at a point to the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void increment(int x, int y, const Grid2d &G);

  /**
   * Subtract data at a point from the state 
   * @param[in] x  index
   * @param[in] y  index
   * @param[in] G  Data
   */
  virtual void decrement(int x, int y, const Grid2d &G);

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const;

protected:
private:
  bool _isX;  /**< True for X texture, false for Y */
  double _A;  /**< sum of squares of adjacent differences */
  double _N;  /**< number of items in sum */
};

//------------------------------------------------------------------
/**
 * @class Grid2dLoopAlgSpeckle
 * @brief Speckle algorithm
 *
 * Speckle is a particular value that can be computed from the Median
 * state, namely:  the 75'th percentile value - 25'th percentile value.
 * Hence the Grid2dLoopAlgSpeckle is a derived class of Grid2dLoopAlgMedian
 */
class Grid2dLoopAlgSpeckle : public Grid2dLoopAlgMedian
{
public:
  /**
   * Constructor
   * @param[in] min  Minimum histogram bin centerpoint
   * @param[in] max  Maximum histogram bin centerpoint
   * @param[in] delta  Spacing between histogram bins
   */
  inline Grid2dLoopAlgSpeckle(double min, double max, double delta) :
    Grid2dLoopAlgMedian(min, max, delta) {}

  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlgSpeckle(void) {}

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const;

protected:
private:

};

//------------------------------------------------------------------
/**
 * @class Grid2dLoopAlgSpeckleInterest
 * @brief Another speckle measure, this one is defined in terms of
 * percentiles and counts, where count[pct] = number of values with 
 * value <= pct value
 *
 * speckle = 
 * fuzzyDataDiff(75th percentile - 50th percentile)*
 * fuzzyDataDiff(50th percentile - 25th percentile)*
 * fuzzyCountDiff(|75th count - 50th count|)*
 * fuzzyCountDiff(|50th count - 25th count|)
 *
 * counts and percentiles are in Grid2dLoopAlgMedian, Hence the
 * Grid2dLoopAlgSpeckleInterest is a derived class of Grid2dLoopAlgMedian
 */
class Grid2dLoopAlgSpeckleInterest : public Grid2dLoopAlgMedian
{
public:
  /**
   * Constructor
   * @param[in] min  Minimum histogram bin centerpoint
   * @param[in] max  Maximum histogram bin centerpoint
   * @param[in] delta  Spacing between histogram bins
   * @param[in] fuzzyDataDiff  Mapping to use on percentile diffs
   * @param[in] fuzzyCountPctDiff  Mapping to use on count diffs
   */
  inline Grid2dLoopAlgSpeckleInterest(double min, double max, double delta,
				      const FuzzyF &fuzzyDataDiff, 
				      const FuzzyF &fuzzyCountPctDiff) :
    Grid2dLoopAlgMedian(min, max, delta), _fuzzyDataDiff(fuzzyDataDiff),
    _fuzzyCountPctDiff(fuzzyCountPctDiff) {}

  /**
   * Destructor
   */
  inline virtual ~Grid2dLoopAlgSpeckleInterest(void) {}

  /**
   * Compute the algorithm result and return it
   * @param[in] minGood  Minimum number of non-missing data values in the
   *                     state.
   * @param[out] result  The algorithm result
   * @return true if result is returned
   */
  virtual bool getResult(int minGood, double &result) const;

protected:
private:

  FuzzyF _fuzzyDataDiff;        /**< Fuzzy mapping */
  FuzzyF _fuzzyCountPctDiff;    /**< Fuzzy mapping */
};

#endif
