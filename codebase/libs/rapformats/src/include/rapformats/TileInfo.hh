/**
 * @file TileInfo.hh
 * @brief Information about tiling of grid, for thresholds
 * @class TileInfo
 * @brief Information about tiling of grid, for thresholds
 */

# ifndef    TileInfo_hh
# define    TileInfo_hh

#include <rapformats/TileLatLon.hh>
#include <string>
#include <vector>
class TileRange;
class Grid2d;

//----------------------------------------------------------------
class TileInfo
{
public:

  /**
   * Empty
   */
  inline TileInfo(void) : _tileNptX(0), _tileNptY(0),
			  _tileNptOverlapX(0), _tileNptOverlapY(0),
			  _gridNptX(0), _gridNptY(0), _nTiles(0),
			  _nTilesX(0), _nTilesY(0), _motherSubset(false),
			  _motherMinX(0), _motherMaxX(0), _motherMinY(0),
			  _motherMaxY(0), _hasLatlon(false), _ok(false){}


  /**
   * Mother Tile implying it is one tile total, covering entire grid
   *
   * @param[in] gridNptX  Grid dimension
   * @param[in] gridNptY  Grid dimension
   */
  inline TileInfo(int gridNptX, int gridNptY) :
    _tileNptX(gridNptX), _tileNptY(gridNptY), _tileNptOverlapX(0),
    _tileNptOverlapY(0), _gridNptX(gridNptX), _gridNptY(gridNptY),
    _nTiles(1), _nTilesX(1), _nTilesY(1), _motherSubset(false),
    _motherMinX(0), _motherMaxX(0), _motherMinY(0), _motherMaxY(0),
    _hasLatlon(false), _ok(true)
  {
  }

  /**
   * Constructor for more than one tile
   *
   * @param[in] tileNptX  Number of points per tile X
   * @param[in] tileNptY  Number of points per tile Y
   * @param[in] tileNptOverlapX  Number of overlap points per tile
   * @param[in] tileNptOverlapY  Number of overlap points per tile
   * @param[in] motherIsSubset  True if mother tile is subset of domain
   * @param[in] motherLowerLeftX Lower left grid index of mother tile, when
   *                             motherIsSubset
   * @param[in] motherLowerLeftY Lower left grid index of mother tile, when
   *                             motherIsSubset
   * @param[in] motherUpperRightX  Upper right grid index of mother tile,
   *                               when motherIsSubset
   * @param[in] motherUpperRightY  Upper right grid index of mother tile,
   *                               when motherIsSubset
   * @param[in] gridNptX  Number of points in grid
   * @param[in] gridNptY  Number of points in grid
   */
  inline TileInfo(int tileNptX, int tileNptY, int tileNptOverlapX,
		  int tileNptOverlapY, bool motherIsSubset,
		  int motherLowerLeftX, int motherLowerLeftY,
		  int motherUpperRightX, int motherUpperRightY,
		  int gridNptX, int gridNptY) :
    _tileNptX(tileNptX), _tileNptY(tileNptY),
    _tileNptOverlapX(tileNptOverlapX), _tileNptOverlapY(tileNptOverlapY),
    _gridNptX(gridNptX), _gridNptY(gridNptY), _motherSubset(motherIsSubset),
    _motherMinX(motherLowerLeftX), _motherMaxX(motherUpperRightX),
    _motherMinY(motherLowerLeftY), _motherMaxY(motherUpperRightY),
    _hasLatlon(false), _ok(true)
  {
    _deriveNumTiles();
  }
							      
  /**
   * Constructor from XML
   * 
   * @param[in] xml  The xml data
   */
  TileInfo(const std::string &xml);

  /**
   * Destructor
   */
  inline virtual ~TileInfo(void) {}

  /**
   * @return true if the input object is the same as local object, except no
   * comparisions of lat/lon info
   * @param[in] t
   */
  bool equalExceptLatlons(const TileInfo &t) const;

  /**
   * @return true if the input object is the same as local object
   * @param[in] t
   */
  bool operator==(const TileInfo &t) const;

  void printDiffs(const TileInfo &t) const;

  /**
   * Add lat/lon from input object to local state
   * @param[in] t
   */
  void addLatlons(const TileInfo &t);

  /**
   * Set lat/lon using input object into local state
   * @param[in] latlon
   */
  void setLatLons(const TileLatLon &latlon);

  /**
   * @return XML representation of state
   */
  std::string toXml(void) const;

  /**
   * @return true if index is the special 'mother tile' index
   * @param[in] index
   */
  inline static bool isMotherTile(int index) {return index == 0;}

  /**
   * @return the mother tile index
   */
  inline static int motherTileIndex(void) {return 0;}

  /**
   * @return the range of x and y grid index values for a tile
   * @param[in] tileIndex  Which tile
   */
  TileRange range(int tileIndex) const;

  /**
   * @return the range of x and y grid index values for a tile, with no overlap
   * @param[in] tileIndex  Which tile
   */
  TileRange rangeNoOverlap(int tileIndex) const;

  /**
   * @return number of grid points, X
   */
  inline int gridNptX(void) const {return _gridNptX;}

  /**
   * @return number of grid points, Y
   */
  inline int gridNptY(void) const {return _gridNptY;}
  
  /**
   * @return total number of tiles
   */
  inline int numTiles(void) const {return _nTiles;}

  /**
   * @return true if the tile index is a valid one
   * @param[in] tileInd
   */
  inline bool inRange(int tileInd) const
  {
    return (tileInd >= 0 && tileInd < _nTiles);
  }

  /**
   * @return the tile index for a particular tile x,y index
   *
   * @param[in] tileIndexX
   * @param[in] tileIndexY  can be 0
   *
   * Does not check bounds so results can be out of bounds
   */
  int tileFromTileIndex(int tileIndexX, int tileIndexY) const;

  /**
   * Create a stitched thresholds grid from the tiles
   * @param[in] fieldName  Name to give the grid
   * @param[in] tileThresh  The threshold value for each tile
   * @param[out] grid  The returned grid
   * @return true if was able to build a grid to return
   *
   * Currently takes average in areas of overlap
   */
  bool constructTiledGrid(const std::string &fieldName,
			  const std::vector<double> &tileThresh,
			  Grid2d &grid) const;

  /**
   * Create a stitched thresholds grid from the tiles using weights
   * that decrease linearly from center to edge of each tile
   *
   * @param[in] fieldName  Name to give the grid
   * @param[in] tileThresh  The threshold value for each tile
   * @param[in] centerWeight  Weight at center of tile
   * @param[in] nptSmooth  Smoothing radius
   * @param[in] edgeWeight  Weight at edge of tile
   * @param[out] grid  The returned grid
   * @return true if was able to build a grid to return
   *
   * Currently takes average in areas of overlap
   */
  bool constructWeightedTiledGrid(const std::string &fieldName,
				  const std::vector<double> &tileThresh,
				  double centerWeight, double edgeWeight,
				  int nptSmooth, Grid2d &grid) const;

  /**
   * Create a non-stitched grid from the tile data inputs
   * @param[in] fieldName  Name to give the grid
   * @param[in] values The value for each tile
   * @param[out] grid  The returned grid
   * @return true if was able to build a grid to return
   *
   */
  bool constructTiledGridNoOverlap(const std::string &fieldName,
				   const std::vector<double> &values,
				   Grid2d &grid) const;

  /**
   * Check if tile index goes out of bounds in y, and if so, figure out
   * the tile directly below
   *
   * @param[in] tileIndex  Index of the tile to check
   * @param[out] belowTile Index of the tile below the out of bounds one, when
   *                       true is returned, set to -1 for errors
   * @return true if out of bounds in y, false if not
   */
  bool outOfBoundsY(int tileIndex, int &belowTile) const;

  /**
   * @return index to the tile that is 'below' (smaller Y) than the tile at
   * the input index, or -1 for error
   * @param[in] tileIndex
   */
  int tileBelow(int tileIndex) const;

  /**
   * Debug print
   * @param[in] verbose
   */
  void print(bool verbose=false) const;

  
  /**
   * For debug print, return a string that has lat/lon value for 
   * a tile.  If lat/lons not set, return the index number instead
   *
   * @param[in] tileIndex
   * @return lat/lon string
   */
  std::string latlonDebugString(int tileIndex) const;

  /**
   * @return true if object is valid
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * Value of XML tag
   */ 
  static const std::string _tag;

protected:
private:  

  int _tileNptX;                        /**< # of points per tile, x */
  int _tileNptY;                        /**< # of points per tile, y */
  int _tileNptOverlapX;                 /**< # of overlap points per tile, x */
  int _tileNptOverlapY;                 /**< # of overlap points per tile, y */
  
  int _gridNptX;                        /**< total gridpoints, X */ 
  int _gridNptY;                        /**< total gridpoints, Y */
  int _nTiles;                          /**< Total # of tiles, derived */
  int _nTilesX;                         /**< Derived # of tiles in X */
  int _nTilesY;                         /**< Derived # of tiles in Y */

  bool _motherSubset;                   /**< True if mother tile is a subset */
  int _motherMinX;                      /**< Minimum X index, mother tile,
					 * used only when _motherSubset */
  int _motherMaxX;                      /**< Maximum X index, mother tile,
					 * used only when _motherSubset */
  int _motherMinY;                      /**< Minimum Y index, mother tile,
					 * used only when _motherSubset */
  int _motherMaxY;                      /**< Maximum Y index, mother tile,
					 * used only when _motherSubset */

  bool _hasLatlon;        /**< True if lat/lon for each tile centerpt is set */
  TileLatLon _latlon;     /**< The lat/lon pairs if _hasLatlon is true */

  bool _ok;                             /**< status */

  void _deriveNumTiles(void);

  /**
   * @return Y tile index of a tile
   * @param[in] tileIndex Overall tile index, assumed > 0 (not mothertile)
   */
  inline int _tileYpt(int tileIndex) const
  {
    // subtract one for the mother tile to get the indexing right
    return ((tileIndex-1)/_nTilesX);
  }

  /**
   * @return X tile index of a tile
   * @param[in] tileIndex Overall tile index, assumed > 0 (not mothertile)
   */
  inline int _tileXpt(int tileIndex) const
  {
    return ((tileIndex-1) - _tileYpt(tileIndex)*_nTilesX);
  }
};


# endif
