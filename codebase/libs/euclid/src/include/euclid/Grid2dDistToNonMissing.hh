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
 * @file Grid2dDistToNonMissing.hh
 * @brief Keep track of the distance from every point where data is missing
 *        to the nearest point where data is not missing
 * @class Grid2dDistToNonMissing
 * @brief Keep track of the distance from every point where data is missing
 *        to the nearest point where data is not missing
 *
 * Search the neighborhood of grid point with coordinates (x,y) for non 
 * missing data. The search is conducted along edges of a square with center 
 * (x,y). If a valid data value is found, the distance assigned is the
 * shortest distance to an edge of the search region. If a valid data value 
 * is not found, then the shortest distance to the edge of the search 
 * region is increased by '_searchScale' grid points. A widening
 * search may continue until a user defined maximum distance is reached and 
 * the maximum distance is assigned to the grid point.
 */

# ifndef    GRID2D_DIST_TO_NONMISSING_HH
# define    GRID2D_DIST_TO_NONMISSING_HH

#include <euclid/Grid2d.hh>
#include <vector>

class Grid2dDistToNonMissing 
{

public:

  /**
   * @param[in] maxSearch  Maximum number of gridpoints away from a
   *                       point where data is missing to search for
   *                       data that is not missing
   *
   * @param[in] searchScale  The resolution of the search in number of
   *                         gridpoints (1, 2, ...)
   */
  Grid2dDistToNonMissing(int maxSearch, int searchScale);

  /**
   * Destructor
   */
  virtual ~Grid2dDistToNonMissing (void);

  /**
   * @return the searchScale value
   */
  inline int getSearchScale(void) const { return _searchScale; }

  /**
   * update using input grid
   *
   * @param[in] g   Grid with data to use
   */
  void update(const Grid2d &g);
	      
  /**
   * Return the index to the nearest point that was not missing,
   * @param[in] x  Index to a  point
   * @param[in] y  Index to a  point
   * @param[out] nearX  index to nearest point where data is not missing,
   *                    if there is such a point
   * @param[out] nearY  index to nearest point where data is not missing,
   *                    if there is such a point
   * @return true if index was set to nearby point, false if no point was set
   *         because there is no nearby point.
   *
   * @note that nearX, nearY = x, y when data is not missing at x,y
   */
  bool nearestPoint(int x, int y, int &nearX, int &nearY) const;

  /**
   * For a given input data grid, fill in two output
   * grids, one with distances to nearest non-missing data and one with
   * data values at the nearest non-missing data point
   * 
   * @param[in] data  Data grid to evaluate
   * @param[out] distOut  At each point the value is set to the distance
   *                      (taxicab metric) to the nearest point at which
   *                      data is not missing.  The value is set to missing
   *                      at points where there is no data near enough to use.
   *                      The units is number of gridpoints
   * @param[out] valOut  At each point the value is set to the data value at
   *                     the nearest point where data is not missing, or
   *                     missing if no such point is found.
   *
   * @return true for success, false for error
   *
   * This method updates the internal state by calling the update() method
   * as needed.
   */
  bool distanceToNonMissing(const Grid2d &data, Grid2d &distOut,
			    Grid2d &valOut);

  /**
   * For a given input data grid, return a modified grid that has data values
   * at the nearest non-missing data point replacing those points at which
   * data is missing.
   * 
   * @param[in] data  Data grid to evaluate
   * @param[out] dataOut  At each point the value is set to the data value at
   *                      the nearest point where data is not missing, or
   *                      missing if no such point is found.
   *
   * @return true for success, false for error
   *
   * This method updates the internal state by calling the update() method
   * as needed.
   */
  bool replaceMissing(const Grid2d &data, Grid2d &dataOut);

protected:
private:

  /**
   * @enum SpecialValue_t
   * @brief Special values to put in to index grids
   */
  typedef enum
  {
    HAS_DATA = -1,    /**< Good data at a point */
    MISSING = -2      /**< Missing data value */
  } SpecialValue_t;
    
  int _maxSearch;   /**< Search radius (# of gridpoints) */
  int _searchScale; /**< resolution of search (# of gridpoints) */
  int _nx;          /**< Grid dimension x */
  int _ny;          /**< Grid dimension y */

  /**
   * At points where data is missing, the X grid index location of nearest
   * non-missing data, or NONE if no non missing data up to max search distance.
   * At points where the input data is not missing (distance=0), _xIndex is
   * set to HAS_DATA
   */
  Grid2d _xIndex;  

  /**
   * At points where data is missing, the Y grid index location of nearest
   * non-missing data, or NONE if no non missing data up to max search distance.
   * At points where the input data is not missing (distance=0), _xIndex is
   * set to HAS_DATA
   */
  Grid2d _yIndex;  

  bool _missingChanged(const Grid2d &g) const;
  void _rebuild(const Grid2d &g);
  int _rebuild1(int r, int x, int y, const Grid2d &g);
};

#endif
