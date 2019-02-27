/**
 * @file MultiThreshFcstBiasMapping.hh
 * @brief Information for one gen time hour/minute/second at all lead times.
 *        Maps from lead time to threshold information for all tiles.
 * @class MultiThreshFcstBiasMapping
 * @brief Information for one gen time hour/minute/second at all lead times.
 *        Maps from lead time to threshold information for all tiles.
 */

# ifndef    MultiThreshFcstBiasMapping_hh
# define    MultiThreshFcstBiasMapping_hh

#include <string>
#include <vector>
#include <map>
#include <rapformats/TiledMultiThresh.hh>

class MultiThreshItem;
class FieldThresh2;
class TileInfo;
class Grid2d;
class TileInfo;

//----------------------------------------------------------------
class MultiThreshFcstBiasMapping
{
public:

  /**
   * Constructor from XML string, as from toXml()
   *
   * @param[in] xml   String to parse
   * @param[in] fields  Corraborating field names expected in xml
   * @param[in] leadSeconds  Corraborating lead times expected in xml
   * @param[in] tiling Corraborating tiling information expected in xml
   */
  MultiThreshFcstBiasMapping(const std::string &xml,
			     const std::vector<std::string> &fields,
			     const std::vector<int> &leadSeconds,
			     const TileInfo &tiling);

  /**
   * Constructor set directly from inputs.  Each tile is given the same
   * thresholds.  This is the coldstart constructor
   * 
   * @param[in] hour    Gen time
   * @param[in] minute  Gen time
   * @param[in] second  Gen time
   * @param[in] leadSeconds  All the lead times
   * @param[in] tiling  The tiling information
   * @param[in] fieldThresh  The fields, each with coldstart thresholds
   */
  MultiThreshFcstBiasMapping(int hour, int minute, int second,
			     const std::vector<int> &leadSeconds,
			     const TileInfo &tiling,
			     const std::vector<FieldThresh2> &fieldThresh);

  /**
   * Destructor
   */
  virtual ~MultiThreshFcstBiasMapping(void);
  
  /**
   * @return true if input genTime has hour/min/second matching local state
   *
   * @param[in] genTime
   */
  bool hmsMatch(const time_t &genTime) const;

  /**
   * @return true if input h,m,s matches local state
   *
   * @param[in] h  Hour
   * @param[in] m  Minute
   * @param[in] s  Second
   */
  bool hmsMatch(int h, int m, int s) const;

  /**
   * @return XML repesentation of local state
   *
   * @param[in] indent  Number of tabs to indent the returned lines
  */
  std::string toXml(int indent=0) const;

  /**
   * Check data for each lead time and change it to coldstart if it is too
   * old compared to input time.
   *
   * @param[in] t  The time to compare to
   * @param[in] maxSecondsBeforeColdstart  Maximum age to keep state
   * @param[in]  coldstartThresh  Coldstart thresholds to use for each field 
   *                              in all tiles.
   *
   * @return true was able to check and modify when needed
   */
  bool checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
		      const std::vector<FieldThresh2> &coldstartThresh);
  /**
   * Update local state using input
   *
   * @param[in] item  Object with values to put into state
   * 
   * @return true if item was inserted into state
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
  bool replaceValues(const MultiThreshFcstBiasMapping &filtMap,
		     const std::vector<std::string> &filterFields);

  /**
   * Set local state to coldstart for a lead time
   *
   * @param[in] leadTime  Particular lead time
   * @param[in] numTiles  Number of tiles.  Each tile gets same coldstart thresh
   *                      values
   * @param[in] thresh   name, coldstart thresholds. First threshold is actual,
   *                     second is what is used in MDV field names
   */
  void setColdstart(int leadTime, int numTiles,
		    const std::vector<FieldThresh2> &thresh);

  /**
   * Debug print
   *
   * @param[in] info  Information about tiling
   * @param[in] verbose  True to print more
   */
  void print(const TileInfo &info, bool verbose=false) const;

  /**
   * Debug print with limitation on what is printed
   *
   * @param[in] genHours  Print only gen time hours equal to input, unless 
   *                      empty in which case print all gen times
   * @param[in] leadSeconds  Print only lead time seconds equal to input,
   *                         unless empty in which case print all lead times
   * @param[in] tiles     Tile indices to print, empty to print all tiles
   * @param[in] info  Information about tiling
   * @param[in] verbose  True to print more
   */
  void print(const std::vector<int> &genHours,
	     const std::vector<int> &leadSeconds, 
	     const std::vector<int> &tiles,
	     const TileInfo &info,
	     bool verbose=false) const;

  /**
   * Retrieve information for a particular gen/lead time/ tile
   *
   * @param[in] genTime  Gen time
   * @param[in] leadTime Lead seconds
   * @param[in] tileInd  Tile index
   * @param[out] item    Returned information
   *
   * @return true if successful
   */
  bool get(const time_t &genTime, int leadTime, int tileInd,
	   MultiThreshItem &item) const;

  /**
   * Retrieve tiled grids for a particular gen/lead time
   *
   * @param[in] genTime  Gen time
   * @param[in] leadTime Lead seconds
   * @param[in] tiling   Information about the tiling
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out] item   Returned information, one grid per field
   *
   * @return true if successful
   */
  bool getTiledGrids(const time_t &genTime, int leadTime,
		     const TileInfo &tiling,
		     double centerWeight, double edgeWeight,
		     int nptSmooth,  std::vector<Grid2d> &item) const;

  /**
   * Retrieve debug tiled grids for a particular gen/lead time field
   *
   * @param[in] genTime  Gen time
   * @param[in] leadTime Lead seconds
   * @param[in] field  Field name
   * @param[in] tiling   Information about the tiling
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out] item   Returned information, one grid per field
   *
   * @return true if successful
   */
  bool getDebugTiledGrids(const time_t &genTime, int leadTime,
			  const std::string &field, const TileInfo &tiling,
			  double centerWeight, double edgeWeight,
			  int nptSmooth,  std::vector<Grid2d> &item) const;


  /**
   * Construct and return a Grid2d that contains tiled thresholds 
   * with averaging in the overlap.  If there is no tiling, return
   * a grid with a single threshold at all points
   *
   * @param[in] fieldName  name of field for which to return thresholds
   * @param[in] leadTime  Lead time to match to
   * @param[in] tiling   Information about the tiling
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out]  grid   The constructed grid
   *
   * @return true for success, false for inputs were wrong
   */
  bool  constructTiledGrid(const std::string &fieldName, int leadTime,
			   const TileInfo &tiling, double centerWeight,
			   double edgeWeight, int nptSmooth,
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
  int _genHour;      /**< forecast gen time */
  int _genMinute;    /**< forecast gen time */
  int _genSecond;    /**< forecast gen time */

  /**
   * Mapping from lead second to Tiled thresholds
   */
  std::map<int, TiledMultiThresh> _map;

  /**
   * @return pointer to the TiledMultiThresh that goes with the leadtime if
   * there is one, or NULL if no such leadtime in the map
   * @param[in] leadTime
   */
  const TiledMultiThresh *_mapFromLeadTime(int leadTime) const;

};

# endif

