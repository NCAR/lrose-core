/**
 * @file GridDistToNonMissing.hh
 * @brief Keep track of the distance from every point where data is missing
 *        to the nearest point where data is not missing
 * @class GridDistToNonMissing
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

# ifndef    GRID_DIST_TO_NONMISSING_HH
# define    GRID_DIST_TO_NONMISSING_HH

#include <euclid/Grid2d.hh>
#include <Mdv/MdvxProj.hh>
#include <vector>

class GridDistToNonMissing 
{

public:

  /**
   * @param[in] maxSearch  Maximum number of gridpoints away from a
   *                       point where data is missing to search for
   *                       data that is not missing
   *
   * @param[in] searchScale  The resolution of the search in number of
   *                         gridpoints (1, 2, ...),
   */
  GridDistToNonMissing(int maxSearch, int searchScale);

  /**
   * Destructor
   */
  virtual ~GridDistToNonMissing (void);

  /**
   * @return the searchScale value
   */
  inline int getSearchScale(void) const { return _searchScale; }

  /**
   * update using input grid
   *
   * @param[in] g   Grid with data to use
   */
  void update(const Grid2d &g, const MdvxProj &proj);
	      
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
   * For a given input data grid and projection,
   * fill in two output
   * grids, one with distances to nearest non-missing data and one with
   * data values at the nearest non-missing data point
   * 
   * @param[in] proj  Projection to use to compute actual distances
   * @param[in] data  Data grid to evaluate
   * @param[out] distOut  At each point the value is set to the distance
   *                      (taxicab metric) to the nearest point at which
   *                      data is not missing.  The value is set to missing
   *                      at points where there is no data near enough to use
   * @param[out] valOut  At each point the value is set to the data value at
   *                     the nearest point where data is not missing, or
   *                     missing if no such point is found.
   *
   * @return true for success, false for error
   *
   * This method updates the internal state by calling the update() method
   * as needed.
   */
  bool distanceToNonMissing(const MdvxProj &proj, const Grid2d &data,
			    Grid2d &distOut, Grid2d &valOut);

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
   * At points where data is missing, the X grid index location of the nearest
   * non-missing data, or MISSING if no non missing data up to max search
   * distance.
   * At points where the input data is not missing (distance=0), _xIndex is
   * set to HAS_DATA
   */
  Grid2d _xIndex;  

  /**
   * At points where data is missing, the Y grid index location of the nearest
   * non-missing data, or MISSING if no non missing data up to max search
   * distance.
   * At points where the input data is not missing (distance=0), _xIndex is
   * set to HAS_DATA
   */
  Grid2d _yIndex;  

  bool _debugBad, _debugData;
  int _xDebugBad, _yDebugBad;
  int _xDebugData, _yDebugData;

  /**
   * @return true if the set of points with missing data value in the local
   * state is different than that of the input grid
   * @param[in] g  Grid to compare to
   */
  bool _missingChanged(const Grid2d &g) const;


  /**
   * Completely rebuild the local state using input data
   * @param[in] g  Data to use
   */
  void _rebuild(const Grid2d &g, const MdvxProj &proj);

  /**
   * Rebuild local state at a point
   *
   * @param[in] r  Radius in number of pixels
   * @param[in] x  Center point
   * @param[in] y  Center point
   * @param[in] g  Grid to use
   *
   * @return number of changes made
   */
  int _rebuild1(int r, int x, int y, const Grid2d &g,
		const MdvxProj &proj);
};

#endif
