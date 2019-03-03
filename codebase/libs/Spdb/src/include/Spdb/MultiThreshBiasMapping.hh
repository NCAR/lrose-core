/**
 * @file MultiThreshBiasMapping.hh
 * @brief Mapping from lead seconds to bias and to threshold for all gen times
 * @class MultiThreshBiasMapping
 * @brief Mapping from lead seconds to bias and to threshold for all gen times
 *
 * Stored to SPDB by gen time, contains XML mappings from lead seconds to
 * bias, and from lead seconds to threshold
 */

# ifndef    MultiThreshBiasMapping_hh
# define    MultiThreshBiasMapping_hh

#include <rapformats/MultiThresholdsBiasMapping.hh>
class DsSpdb;
class FieldThresh2;

//----------------------------------------------------------------
class MultiThreshBiasMapping : public MultiThresholdsBiasMapping
{
public:

  /**
   * Empty
   */
  MultiThreshBiasMapping(void);

  /**
   * URL, but no other info (for reading only)
   *
   * @param[in] spdb  The url
   */
  MultiThreshBiasMapping(const std::string &spdb);

  /**
   * @param[in] spdb  The URL
   * @param[in] ltHours  The lead times
   * @param[in] fields  The fields
   * @param[in] doTile  True to break thresholds into tiles
   * @param[in] tiling  Information about tiles
   * @param[in] latLonsOptional  True if lat/lon information in the
   *                             data that is read is optional or not.
   *                             If not true, the latlon information is
   *                             expected.  This is put in for backwards
   *                             compatibility in reading older data.
   *
   * Thresholds not known
   */
  MultiThreshBiasMapping(const std::string &spdb,
			 const std::vector<double> &ltHours,
			 const std::vector<std::string> &fields,
			 const TileInfo &tiling,
			 bool latLonsOptional=true);

  /**
   * Destructor
   */
  virtual ~MultiThreshBiasMapping(void);
  
  /**
   * Read the newest data whose time is less than input, back to a maximum
   *
   * @param[in] t  Upper time limit
   * @param[in] maxSecondsBack  Look back [t-maxSecondsBack,t)
   * @param[in] maxSecondsBeforeColdstart  If data is read in, change that
   *                                       data to coldstart values if the
   *                                       generating time is older than
   *                                       this value compared to input t
   * @param[in]  coldstartThresh  The coldstart field/threshold pairs
   *
   *
   * @return true if successful and state is filled in
   */
  bool readFirstBefore(const time_t &t, int maxSecondsBack,
		       int maxSecondsBeforeColdstart,
		       const std::vector<FieldThresh2> &coldstartThresh);

  /**
   * Read the newest data whose time is in the range specified
   *
   * @param[in] t0  Lower time limit
   * @param[in] t1  Upper time limit
   *
   * @return true if successful and state is filled in
   */
  bool readNewestInRange(const time_t &t0, const time_t &t1);


  /**
   * Read the data within a range closest in time to a target time
   *
   * @param[in] target  The target time
   * @param[in] t0  Lower time limit
   * @param[in] t1  Upper time limit
   *
   * @return true if successful and state is filled in
   */
  bool readClosestToTargetInRange(const time_t &target, const time_t &t0,
				  const time_t &t1);

  /**
   * Read the newes data that has the same hour/min/second as the input 
   * generation time either an exact match, or back some number of days
   *
   * @param[in] gt  The generation time
   * @param[in] maxDaysBack
   * @return true if successful and state is filled in
   */
  bool readExactPreviousOrCurrent(const time_t &gt, int maxDaysBack);

  /**
   * Read the newest data that has the same hour/min/second as the input 
   * generation time back a minimum of 1 day and a maximum of maxDaysBack
   *
   * @param[in] gt  The generation time
   * @param[in] maxDaysBack
   * @return true if successful and state is filled in
   */
  bool readExactPrevious(const time_t &gt, int maxDaysBack);

  /**
   * Write state to SPDB at time t
   *
   * @param[in] t
   * @return true if successful
   */
  bool write(const time_t &t);

  /**
   * @return the times that are in the range of inputs 
   *
   * @param[in] t0 Earliest time
   * @param[in] t1 Latest time
   */
  std::vector<time_t> timesInRange(const time_t &t0, const time_t &t1) const;
  
  /**
   * Read from SPDB into state, at a time
   *
   * @param[in] time
   * @return true if successful
   */
  bool read(const time_t &time);

  /**
   * @return the chunk valid time, which will be set with any successful
   * read
   */
  inline time_t getChunkValidTime(void) const {return _chunkValidTime;}

  /**
   * @return the chunk time written, which will be set with any successful
   * read
   */
  inline time_t getChunkTimeWritten(void) const {return _chunkTimeWritten;}

protected:
private:  

  /**
   * SPDB location
   */
  std::string _url;

  /**
   * Valid time of the chunk that was loaded, when  you read
   */
  time_t _chunkValidTime;

  /**
   * Time written of the chunk that was loaded, when  you read
   */
  time_t _chunkTimeWritten;

  /**
   * True for optional lat/lon information on read
   */
  bool _latlonsOptional;

  bool _load(DsSpdb &s, bool fieldLeadsTilesSet=true);
};

# endif
