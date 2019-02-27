/**
 * @file TiledMultiThresh.hh
 * @brief Mapping from tile index to threshold information for that tile.
 * @class TiledMultiThresh
 * @brief Mapping from tile index to threshold information for that tile.
 */

# ifndef    TiledMultiThresh_hh
# define    TiledMultiThresh_hh

#include <string>
#include <vector>
#include <map>
#include <rapformats/MultiThresh.hh>

class MultiThreshItem;
class TileInfo;
class Grid2d;

//----------------------------------------------------------------
class TiledMultiThresh
{
public:

  /**
   * Empty Constructor
   */
  TiledMultiThresh(void);


  /**
   * Constructor from XML string, as from toXml()
   *
   * @param[in] xml   String to parse
   * @param[in] fields  Corraborating field names expected in xml
   * @param[in] tiling  Corraborating tiling information expected in xml
   */
  TiledMultiThresh(const std::string &xml,
		   const std::vector<std::string> &fields,
		   const TileInfo &tiling);

  /**
   * Coldstart constructor
   *
   * @param[in] numTiles  Tile specification (number of tiles)
   * @param[in] fieldThresh  The coldstart thresholds for each field
   */
  TiledMultiThresh(int numTiles,
		   const std::vector<FieldThresh2> &fieldThresh);

  /**
   * Destructor
   */
  virtual ~TiledMultiThresh(void);

  /**
   * For a given tile index, return pointer to the thresholds information
   * for that tile.
   *
   * @param[in] tileInd  Tile index
   *
   * @return the pointer, or NULL if index out of range
   */
  const MultiThresh *mapFromTileIndex(int tileInd) const;

  /**
   * @return XML repesentation of local state
   * 
   * @param[in] indent  Number of tabs to indent the xml
   */
  std::string toXml(int indent=0) const;

  /**
   * Change to coldstart values if data is too old compared
   * to input time.
   *
   * @param[in] t  The time to compare to
   * @param[in] maxSecondsBeforeColdstart  Maximum age to keep state
   * @param[in]  coldstartThresh  The coldstart thresholds to use 
   *
   * @return true if checking/changing happened without error, false otherwise
   */
  bool checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
		      const std::vector<FieldThresh2> &coldstartThresh);
  /**
   * Update local state using input
   *
   * @param[in] item Object with values to put into state
   *
   * @return true if update happened without error, false otherwise
   */
  bool update(const MultiThreshItem &item);

  /**
   * Filter the state down to a subset of the total fields
   *
   * @param[in] fieldNames The full field names to filter down to
   * @return true if successful
   */
  bool filterFields(const std::vector<std::string> &fieldNames);

  /**
   * Replace values in state with those from an input object, for particular
   * fields.
   * @param[in] filtMap  The input object from which to get replacement values
   * @param[in] filterFields the field names to replace
   *
   * @return true if the input filtMap matched the local state, and the
   * fields were replaced
   */
  bool replaceValues(const TiledMultiThresh &filtMap,
		     const std::vector<std::string> &filterFields);

  /**
   * Debug print to stdout
   *
   * @param[in] leadTime  Leadtime seconds represented here
   * @param[in] info  Information about tiling
   * @param[in] verbose  True for more output
   */
  void print(int leadTime, const TileInfo &info, bool verbose) const;

  /**
   * Debug print to stdout, specific tiles only
   *
   * @param[in] leadTime  Leadtime seconds represented here
   * @param[in] tiles     Tile indices to print, empty to print all tiles
   * @param[in] tileInfo  Information about tiling
   * @param[in] verbose  True for more output
   */
  void print(int leadTime, const std::vector<int> &tiles,
	     const TileInfo &tileInfo, bool verbose) const;

  /**
   * Construct grids with thresholds at each grid point for each field
   * @param[in] tiling  Tiling information
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out] item  The grids, named with field names
   * @return true for success
   */
  bool getTiledGrids(const TileInfo &tiling, double centerWeight,
		     double edgeWeight, int nptSmooth,  
		     std::vector<Grid2d> &item) const;

  /**
   * Construct debug grids for a field
   * @param[in] tiling  Tiling information
   * @param[in] field  Field name
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out] item  The grids
   *
   * @return true for success
   */
  bool getDebugTiledGrids(const TileInfo &tiling, const std::string &field,
			  double centerWeight, double edgeWeight,
			  int nptSmooth, std::vector<Grid2d> &item) const;

  /**
   * Construct and return a Grid2d that contains tiled thresholds 
   * with averaging in the overlap, using tile thresholds found locally.
   * If there is no tiling, return
   * a grid with a single threshold at all points
   *
   * @param[in] fieldName  name of field for which to return thresholds
   * @param[in] tiling  Tiling information
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out]  grid   The constructed grid
   *
   * @return true for success, false for inputs were wrong
   */
  bool constructTiledGrid(const std::string &fieldName, const TileInfo &tiling,
			  double centerWeight, double edgeWeight, int nptSmooth,
			  Grid2d &grid) const;


  /**
   * @return true if set
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Value of XML tag
   */ 
  static const std::string _tag;

protected:
private:  

  bool _ok;          /**< True if object is set */

  /**
   * tile index to multiple thresholds mapping
   */
  std::map<int, MultiThresh> _map;

  /**
   * @return pointer to the MultiThresh that goes with the tile if there
   * is one, or NULL if no such tile in the map
   * @param[in] tileIndex
   */
  MultiThresh *_mapFromTile(int tileIndex);

  /**
   * @return pointer to the MultiThresh that goes with the tile if there
   * is one, or NULL if no such tile in the map
   * @param[in] tileIndex
   */
  const MultiThresh *_constMapFromTile(int tileIndex) const;

  /**
   * @return tile thresholds for all tiles, i'th field
   * @param[in] fieldIndex  Index into fields
   * @param[in] numTiles  Number of tiles
   */
  std::vector<double> _tileThresh(int fieldIndex, int numTiles) const;

  /**
   * @return tile bias for all tiles
   * @param[in] numTiles  Number of tiles
   */
  std::vector<double> _tileBias(int numTiles) const;
  /**
   * @return tile obs value for all tiles
   * @param[in] numTiles  Number of tiles
   */
  std::vector<double> _tileObs(int numTiles) const;

  /**
   * @return tile forecast value for all tiles
   * @param[in] numTiles  Number of tiles
   */
  std::vector<double> _tileFcst(int numTiles) const;

  /**
   * @return tile 'yes=1/no=0' does this tile use the mothertile for all tiles
   * @param[in] numTiles  Number of tiles
   */
  std::vector<double> _tileIsMother(int numTiles) const;
};

# endif

