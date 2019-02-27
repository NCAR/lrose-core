/**
 * @file MultiThresholdsBiasMapping.hh
 * @brief The threshold information for gen times, lead times, and tiles
 * @class MultiThresholdsBiasMapping
 * @brief The threshold information for gen times, lead times, and tiles
 *
 * Stored to SPDB using verifying obs time as storage chunk time,
 * contains a vector of MultiThreshFcstBiasMapping objects.  Each of these
 * has information as to the gen time hour/minute/second, and then for each
 * lead time the thresholds information for all tiles.
 */

# ifndef    MultiThresholdsBiasMapping_hh
# define    MultiThresholdsBiasMapping_hh

#include <rapformats/TileInfo.hh>
#include <rapformats/MultiThreshFcstBiasMapping.hh>
#include <string>
#include <vector>

class MultiThreshItem;
class FieldThresh2;
class Grid2d;

//----------------------------------------------------------------
class MultiThresholdsBiasMapping
{
public:
  /**
   * Empty
   */
  MultiThresholdsBiasMapping(void);

  /**
   * Constructor with overall lead times, fields and tiling info passed in,
   * Thresholds not known, filled in later.
   * 
   * @param[in] ltHours The lead times (hours)
   * @param[in] fields  The field names
   * @param[in] tiling  Information about tiles
   */
  MultiThresholdsBiasMapping(const std::vector<double> &ltHours,
			     const std::vector<std::string> &fields,
			     const TileInfo &tiling);

  /**
   * Destructor
   */
  virtual ~MultiThresholdsBiasMapping(void);
  
  /**
   * Clear to completely empty
   */
  void clearMapping(void);

  /**
   * Add lat/lon information to tiling member, if not already there
   * @param[in] info  Information about tiling that contains lat/lon info
   */
  void addLatlonsIfNeeded(const TileInfo &info);

  /**
   * Set state to coldstart values for all the gen times that are spaced
   * apart evenly starting at (h,m,s)=(0,0,0)
   *
   * @param[in]  genFrequencySeconds  Gentimes: (0,0,0) + k*genFrequencySeconds
   *             k=0,1,..
   * @param[in] fieldThresh  The coldstart fieldname/threshold/threshold values
   *                         for each field.  The first threshold is the actual
   *                         the second is the value used in MDV field names
   *
   * @return true if successful
   *
   * @note For tiles, the same thresholds are put into each tile
   */
  bool setColdStart(int genFrequencySeconds,
		    const std::vector<FieldThresh2> &fieldThresh);

  /**
   * Replace values in state with those from an input object, for particular
   * fields.
   * @param[in] filtMap  The input object from which to get replacement values
   * @param[in] filterFields the field names to replace
   *
   * @return true if the input filtMap matched the local state, and the
   * fields were replaced
   */
  bool replaceValues(const MultiThresholdsBiasMapping &filtMap,
		     const std::vector<std::string> &filterFields);
  
  /**
   * @return XML string representing the state
   */
  std::string toXml(void) const;

  /**
   * Parse XML string to set portions of state, comparing to current local
   * values
   *
   * @param[in] xml  The XML to parse
   * @param[in] fieldsLeadsTilesSet  True if local state has fields, tiles,
   *                                 and lead times to compare the parsed XML to
   * @param[in] latlonsOptional True if local state can have lat/lons, but
   *                            input might now.  If False, the local and
   *                            XML must either both have lat/lons, or neither

   * @return true if successful
   */
  bool fromXml(const std::string &xml, bool fieldsLeadsTilesSet=true,
	       bool latlonsOptional = true);

  /**
   * Set state to coldstart values if not already set to coldstart,
   * but only when the input time is too new compared to the local time
   *
   * @param[in] t  The time to compare to
   * @param[in] maxSecondsBeforeColdstart  Maximum age to keep current state
   * @param[in]  coldstartThresh  The coldstart thresholds to use for each field
   *                              in all tiles
   * @return true if successful
   */
  bool checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
		      const std::vector<FieldThresh2> &coldstartThresh);

  /**
   * Update local state using input
   * @param[in] item  The values to be inserted into state
   * 
   * @return true if successful
   */
  bool update(const MultiThreshItem &item);

  /**
   * Filter the state down to a subset of the total fields
   *
   * @param[in] fieldNames The full field names to filter down to
   * @return true if successful, false if input fieldname not found locally,
   */
  bool filterFields(const std::vector<std::string> &fieldNames);

  /**
   * Set to coldstart values for a particular gen time/lead time, all tiles
   *
   * @param[in] genTime
   * @param[in] leadTime
   * @param[in] numTiles  Number of tiles, all get same coldstart values
   * @param[in] fieldThresh  The coldstart fieldname/threshold/threshold values
   *                         for each field.  The first threshold is the actual
   *                         the second is the value used in MDV field names
   *
   * @return true if successful
   */
  bool setColdStart(const time_t &genTime, int leadTime,  int numTiles,
		    const std::vector<FieldThresh2> &fieldThresh);

  /**
   * Debug print
   *
   * @param[in] t  The time of this data
   * @param[in] verbose  True for more printing
   */
  void printState(const time_t &t, bool verbose) const;

  /**
   * Debug print, with filtering of what is printed
   *
   * @param[in] t  The time of this data
   * @param[in] verbose  True for more printing
   * @param[in] genHours  Print only gen time hours equal to input, unless 
   *                      empty in which case print all gen times
   * @param[in] leadSeconds  Print only lead time seconds equal to input,
   *                         unless empty in which case print all lead times
   * @param[in] tiles  Print only tile indices equal to input,
   *                   unless empty in which case print all tiles
   */
  void printState(const time_t &t, bool verbose,
		  const std::vector<int> &genHours,
		  const std::vector<int> &leadSeconds,
		  const std::vector<int> &tiles) const;
  
  /**
   * Retrieve debug tiled grids for a particular gen/lead time/field
   *
   * @param[in] genTime  Gen time
   * @param[in] leadTime Lead seconds
   * @param[in] field  Field name
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out] item   Returned information, one grid per debug field
   *
   * @return true if successful
   */
  bool getDebugTiledGrids(const time_t &genTime, int leadTime,
			  const std::string &field,
			  double centerWeight, double edgeWeight,
			  int nptSmooth, std::vector<Grid2d> &item) const;


  /**
   * Retrieve information for a particular gen/lead/tile
   *
   * @param[in] genTime  Gen time
   * @param[in] leadTime Lead seconds
   * @param[in] tileInd  Tile index
   * @param[out] item   Returned information
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
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out] item   Returned information, one grid per field
   *
   * @return true if successful
   */
  bool getTiledGrids(const time_t &genTime, int leadTime,
		     double centerWeight, double edgeWeight,
		     int nptSmooth, std::vector<Grid2d> &item) const;

  /**
   * Retrieve information for a particular gen/lead/field for all tiles
   *
   * @param[in] genTime  Gen time
   * @param[in] leadTime  Lead seconds
   * @param[in] numTiles  Used to check for consistency
   * @param[in] fieldName Field name 
   * @param[out] item   Name and actual threshold for each tile
   *
   * @return true if successful
   */
  bool get(const time_t &genTime, int leadTime, int numTiles,
	   const std::string &fieldName, std::vector<FieldThresh> &item) const;

  /**
   * Construct and return a Grid2d that contains tiled thresholds 
   * with averaging in the overlap.  If there is no tiling, return
   * a grid with a single threshold at all points
   *
   * @param[in] fieldName  name of field for which to return thresholds
   * @param[in] genTime  Gen time to match to
   * @param[in] leadTime  Lead time to match to
   * @param[in] centerWeight  Weight at center of each tile
   * @param[in] edgeWeight  Weight at edge of each tile
   * @param[in] nptSmooth Smoothing to do in the tiled grid, number of points
   * @param[out]  grid   The constructed grid
   *
   * @return true for success, false for inputs were wrong
   */
  bool  constructTiledGrid(const std::string &fieldName, const time_t &genTime,
			   int leadTime, double centerWeight,
			   double edgeWeight, int nptSmooth,
			   Grid2d &grid) const;
			     

  /**
   * @return the lead seconds
   */
  inline std::vector<int> getLeadSeconds(void) const {return _leadSeconds;}

 protected:
 private:  

  std::vector<std::string> _fields;        /**< Field names */
  std::vector<int> _leadSeconds;           /**< Lead seconds */
  TileInfo _tiling;                        /**< Information about tiles */

  /**
   * The mappings for each gen time
   */
  std::vector<MultiThreshFcstBiasMapping> _fcst;

  std::string _fieldsToXml(void) const;
  std::string _leadsToXml(void) const;
  std::string _mappingsToXml(void) const;
  bool _fieldsFromXml(const std::string &xml, bool fieldsLeadTilesSet);
  bool _leadsFromXml(const std::string &xml, bool fieldsLeadTilesSet);
  bool _tilingFromXml(const std::string &xml, bool fieldsLeadTilesSet,
		      bool latlonsOptional);
  bool _mappingsFromXml(const std::string &xml);
  bool _fieldThreshNamesOk(const std::vector<FieldThresh2> &fieldThresh) const;
  bool _leadTimeExists(int leadTime, const std::string &debugName) const;
};


# endif
