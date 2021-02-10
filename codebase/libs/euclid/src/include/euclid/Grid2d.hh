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
 * @file Grid2d.hh
 * @brief two dimensional data grid 
 * @class Grid2d
 * @brief two dimensional data grid 
 */

# ifndef  GRID2D_H
# define  GRID2D_H

#include <string>
#include <vector>

//------------------------------------------------------------------
class Grid2d
{
  friend class GridAlgs;

public:

  /**
   * Empty constructor (no data)
   */
  Grid2d(void);

  /**
   * named grid, no data
   * @param[in] name = name of the grid
   */
  Grid2d(const std::string &name);

  /**
   * named grid, missing value
   * @param[in] name = name of the grid
   * @param[in] missing = missing data value
   */
  Grid2d(const std::string &name, double missing);

  /**
   * named grid with dimensions, sets all values to missing
   * @param[in] name
   * @param[in] nx
   * @param[in] ny
   * @param[in] missing
   *
   */
  Grid2d(const std::string &name, int nx, int ny, double missing);

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
  Grid2d(const std::string &name, int nx, int ny, 
	 const std::vector<double> &data, double missing);

  /**
   * Destructor
   */
  virtual ~Grid2d(void);

  /**
   * @return true if grids have same dimensions
   * @param[in] g  Grid to compare
   */
  bool dimensionsEqual(const Grid2d &g) const;

  /**
   * @return true if grids have same data values at all points (name may be
   * different)
   * @param[in] g  Grid to compare
   */
  bool valuesEqual(const Grid2d &g) const;

  /**
   * Print out contents to stdout as a log message with time stamp
   */
  void log(void) const;

  /**
   * Print out contents to stdout
   */
  void print(void) const;

  /**
   * Print out contents with a header and an index to stdout
   * @param[in] n  Header
   * @param[in] i  Index
   */
  void print2(const std::string &n, int i) const;

  /**
   * Print range of values to stdout
   *
   * @param[in] x0  Lower x index value
   * @param[in] x1  Upper x index value
   * @param[in] y0  Lower y index value
   * @param[in] y1  Upper y index value
   */
  void printRange(int x0, int x1, int y0, int y1) const;

  /**
   * Print all non-missing data to stdout including index values
   * Warning this might be a very large print.
   *
   */
  void printNonMissing(void) const;

  /**
   * Print the field name, missing data value, and number of missing gridpts
   */
  void debugShowMissing(void) const;

  /**
  * @param[in] name
  * @return true if input name equals grid name
  */
  inline bool nameEquals(const std::string &name) const {return _name == name;}


  /**
   * @return missing value
   */
  inline double getMissing(void) const {return _missing;}

  /**
   * @return const reference to data
   */
  inline const std::vector<double> &getData(void) const {return _data;}

  /**
   * @return data value an an index
   * @param[in] ipt  The index
   */
  inline double getDataAt(int ipt) const {return _data[ipt];}


  /**
   * @return The data value at an index
   * @param[in] ipt  The index
   */
  inline double getValue(int ipt) const {return _data[ipt];}

  /**
   * @return The data value at an x,y location
   * @param[in] x  Index into first grid dimension
   * @param[in] y  Index into second grid dimension
   */
  inline double getValue(int x, int y) const {return _data[_ipt(x, y)];}

  /**
   * Retrieve value at a point
   *
   * @param[in] i  Index into data
   * @param[out] v  Returned data value at index
   *
   * @return  false if data is missing at index, true otherwise
   */
  bool getValue(int i, double &v) const;

  /**
   * Retrieve value at a point
   *
   * @param[in] x  Index into data
   * @param[in] y  Index into data
   * @param[out] v  Returned data value at x,y
   *
   * @return  false if data is missing at x,y, true otherwise
   */
  bool getValue(int x, int y, double &v) const;

  /**
   * @return number of data points
   */
  inline int getNdata(void) const {return _npt;}

  /**
   * @return x dimension
   */
  inline int getNx(void) const {return _nx;}

  /**
   * @return y dimension
   */
  inline int getNy(void) const {return _ny;}

  /**
   * @return name
   */
  inline const std::string getName(void) const {return _name;}

  /**
   * @return one dimensional index into data from 2 dimensional inputs
   *
   * @param[in] x  Index
   * @param[in] y  Index
   */
  inline int ipt(const int x, const int y) const {return _ipt(x,y);}

  /**
   * @return Two dimensional y index for one dimensional index
   * @param[in] ipt  One dimensional index
   */
  inline int yAtIndex(const int ipt) const {return (ipt)/_nx;}

  /**
   * @return Two dimensional x index for one dimensional index
   * @param[in] ipt  One dimensional index
   */
  inline int xAtIndex(const int ipt) const {return ipt -  (yAtIndex(ipt)*_nx);}

  /**
   * Retrieve the data value at an offset from a grid location
   *
   * @param[in] x  index to grid location
   * @param[in] y  index to grid location
   * @param[in] offset  Offset from the x,y location (one dimensional)
   * @param[out] v   returned value
   *
   * @return true if the offset location is in range,
   *         false if offset is out of range, or value is missing there
   */
  bool getValueAtOffset(int x, int y, int offset, double &v) const;

  /**
   * @return true if data is missing at index
   * @param[in] i index into data
   */
  inline bool isMissing(int i) const  {return (_data[i] == _missing); }

  /**
   * @return true if data is missing at (x,y)
   * @param[in] x  Index into grid
   * @param[in] y  Index into grid
   */
  inline bool isMissing(int x, int y) const
  {
    return _data[_ipt(x,y)] == _missing;
  }

  /**
   * @return percent of data that is missing
   */
  double percentMissing(void) const;

  /**
   * @return number of non-missing data values 
   */
  int numGood(void) const;

  /**
   * @return true if indices are in range of Grid specification.
   *
   * @param[in] x  Index into grid
   * @param[in] y  Index into grid
   */
  bool inRange(int x, int y) const;

  /**
   * @return true if data at (x,y) == value
   * @param[in] x  Index into grid
   * @param[in] y  Index into grid
   * @param[in] value  Value to check for
   */
  bool equals(int x, int y, double value) const;

  /**
   * @return true if data at (x,y) is greater than input value
   *
   * @param[in] x  Index into grid
   * @param[in] y  Index into grid
   * @param[in] value  Value to check for
   */
  bool greaterThan(int x, int y, double value) const;

  /**
   * set local values to inputs
   *
   * @param[in] d   Data vector
   * @param[in] missing missing data value
   * @param[in] nx      grid dimension
   * @param[in] ny      grid dimension
   *
   * @note expect size of d to equal nx*ny
   */
  void setGridInfo(const std::vector<double> &d, double missing,
		   int nx, int ny);

  /**
   * set local values to input, with no change of missing value
   *
   * @param[in] d   Data vector
   * @param[in] nx      grid dimension
   * @param[in] ny      grid dimension
   *
   * @note expect size of d to equal nx*ny
   */
  void setGridInfo(const std::vector<double> &d, int nx, int ny);

  /**
   * Set the missing value to input
   *
   * @param[in] missing missing data value
   */
  void setGridInfo(double missing);

  /**
   * Set name to input
   *
   * @param[in] name
   */
  inline void setName(const std::string &name) { _name = name;}

  /**
   * Copy all data related values from input to local object
   * @param[in] a  Grid to copy from
   *
   * @note do not change grid name in output to that of local grid
   */
  void dataCopy(const Grid2d &a);

  /**
   * Set all data values to missing
   */
  void setAllMissing(void);

  /**
   * Set data value to missing at a point
   * @param[in] i  Index
   */
  void setMissing(int i);

  /**
   * Set data value to missing at a point
   * @param[in] x  Index
   * @param[in] y  Index
   */
  void setMissing(int x, int y);

  /**
   * Set data value to missing at a point, warn if out of range
   * @param[in] x  Index
   * @param[in] y  Index
   */
  void setMissingWithWarnings(int x, int y);

  /**
   * Set all existing missing data values to a new value, then change
   * missing value to that value
   * @param[in] newmissing  The new value
   */
  void changeMissingAndData(double newmissing);

  /**
   * Change the missing value without changing data
   * @param[in] newmissing  The new value
   */
  void changeMissing(double newmissing);

  /**
   * Set grid at the vector of input (x,y) locations to a value.
   *
   * @param[in] p  Pairs of x,y at which to change value
   * @param[in] value  The value to change to.
   */
  void setValue(const std::vector<std::pair<int,int> > &p, double value);

  /**
   * Set grid at index i to value
   *
   * @param[in] i  Index to grid
   * @param[in] value  Value to put there
   */
  inline void setValue(int i, double value) { _data[i] = value;}

  /**
   * Set grid at x,y to value
   *
   * @param[in] x  Index
   * @param[in] y  Index
   * @param[in] value  Value to put there
   */
  inline void setValue(int x, int y, double value) {_data[_ipt(x, y)] = value;}

  /**
   * Set grid values at all points to value
   * @param[in] value  Value
   */
  void setAllToValue(double value);

  /**
   * Set data at a point to a value, give warning if indices out of range.
   *
   * @param[in] x  Index
   * @param[in] y  Index
   * @param[in] value  Value
   *
   * @return true if x,y in range and value was set into grid, false otherwise
   */
  bool setWithWarnings(int x, int y, double value);

  /**
   * Reduce resolution of the local grid by a factor 
   *
   * @param[in] res  Reduction, 1 = no resolution reduction, 2 = 
   *                 resolution reduction by a factor of 2, and so on.
   *
   * @return  True if successful, false if some problem was encountered,
   *          and in that case no change is made to the grid.
   *          Problems include res < 2 or res so high that nothing is left.
   *
   * @note this method subsamples the original grid
   */
  bool reduce(const int f);

  /**
   * Interpolate a grid from low to higher resolution, with results put into
   * the local grid. Uses a bilinear interpolation method.
   *
   * @param[in] lowres  The low resolution grid to interpolate
   * @param[in] res  Resolution factor, 1 = no change, 2 = increase resolution
   *                 by a factor of two, etc.
   *
   * @return  True if the interpolation was successful,
   *          false if inputs were not as expected.
   *
   * @note  Expect (local nx) = (input nx)*res, (local ny)=(input ny)*res.
   *        An error occurs it this is not true.
   */
  bool interpolate(const Grid2d &lowres, const int res);

  /**
   * Shorthand for iterator
   */
  typedef std::vector<double>::iterator iterator;
  /**
   * Shorthand for iterator
   */
  typedef std::vector<double>::const_iterator const_iterator;
  /**
   * Shorthand for iterator
   */
  typedef std::vector<double>::reverse_iterator reverse_iterator;
  /**
   * Shorthand for iterator
   */
  typedef std::vector<double>::const_reverse_iterator const_reverse_iterator;

  /**
   * @return iterator to beginning of data
   */
  iterator begin()                       { return _data.begin(); }
  /**
   * @return const iterator to beginning of data
   */
  const_iterator begin() const           { return _data.begin(); }

  /**
   * @return iterator to end of data
   */
  iterator end()                         { return _data.end(); }

  /**
   * @return const iterator to end of data
   */
  const_iterator end() const             { return _data.end(); }

  /**
   * @return reference to i'th data element
   * @param[in] i
   */
  double& operator[](size_t i)                { return _data[i]; }

  /**
   * @return const reference to i'th data element
   * @param[in] i
   */
  const double& operator[](size_t i) const    { return _data[i]; }

  /**
   * @return reference to x,y'th data element
   * @param[in] x
   * @param[in] y
   */
  double& operator()(size_t x, size_t y)    { return _data[_ipt(x,y)]; }
  /**
   * @return const reference to x,y'th data element
   * @param[in] x
   * @param[in] y
   */
  const double& operator()(size_t x, size_t y) const { return _data[_ipt(x,y)]; }

protected:

  
  std::string _name;          /**< description of the data. */
  std::vector<double> _data;   /**< The data values */
  double _missing;             /**< data missing value */
  int _npt;                   /**< number of data grid points */
  int _nx;                    /**< number of x dimension data grid points */
  int _ny;                    /**< number of y dimension data grid points */

  /**
   * @return index into grid based on 2d indices
   * @param[in] ix
   * @param[in] iy
   */
  inline int _ipt(int ix, int iy) const { return iy*_nx + ix;}

  int _firstValidIndex(int y) const;
  int _lastValidIndex(int y) const;

private:

  /**
   * Do bilinear interpolation from low to high resolution at a point
   *
   * @param[in] ry0  Low resolution point in the input grid
   * @param[in] rx0  Low resolution point in the input grid
   * @param[in] res  Factor to convert from low to high resolution 1, 2, ..
   * @param[in] y   High resolution output point (in the local grid)
   * @param[in] x   High resolution output point (in the local grid)
   * @param[in] lowres  The grid with low resolution data
   *
   * @return interpolated value for position x, y in the local grid
   */
  double _bilinear(int ry0, int rx0, int res, int y, int x,
		   const Grid2d &lowres) const;
};

#endif
