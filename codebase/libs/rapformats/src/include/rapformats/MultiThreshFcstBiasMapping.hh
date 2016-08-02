/**
 * @file MultiThreshFcstBiasMapping.hh
 * @brief Mapping from lead seconds to bias and to threshold, for all lead times
 *        associated with a gen hour/minute/second
 * @class MultiThreshFcstBiasMapping
 * @brief Mapping from lead seconds to bias and to threshold, for all lead times
 *        associated with a gen hour/minute/second
 */

# ifndef    MultiThreshFcstBiasMapping_hh
# define    MultiThreshFcstBiasMapping_hh

#include <string>
#include <vector>
#include <map>
#include <rapformats/MultiThresh.hh>

class MultiThreshItem;
class FieldThresh;

//----------------------------------------------------------------
class MultiThreshFcstBiasMapping
{
public:

  /**
   * Constructor from XML string, as from toXml()
   * @param[in] xml   String to parse
   * @param[in] fields  Corraborating field names expected in xml
   * @param[in] leadSeconds  Corraborating lead times expected in xml
   */
  MultiThreshFcstBiasMapping(const std::string &xml,
			     const std::vector<std::string> &fields,
			     const std::vector<int> &leadSeconds);

  /**
   * Constructor set directly from inputs, all have same thresholds, i.e.
   * it is a coldstart
   * 
   * @param[in] hour    Gen time
   * @param[in] minute  Gen time
   * @param[in] second  Gen time
   * @param[in] leadSeconds
   * @param[in] fieldThresh
   */
  MultiThreshFcstBiasMapping(int hour, int minute, int second,
			     const std::vector<int> &leadSeconds,
			     const std::vector<FieldThresh> &fieldThresh);

  /**
   * Destructor
   */
  virtual ~MultiThreshFcstBiasMapping(void);
  
  /**
   * @return true if input genTime has hour/min/second matching local state
   * @param[in] genTime
   */
  bool hmsMatch(const time_t &genTime) const;

  /**
   * @return true if input h,m,s matches local state
   * @param[in] h
   * @param[in] m
   * @param[in] s
   */
  bool hmsMatch(int h, int m, int s) const;

  /**
   * Update local state using input
   * @param[in] item  Object with values to put into state
   */
  bool update(const MultiThreshItem &item);

  /**
   * Set local state to coldstart for a lead time
   * @param[in] leadTime  
   * @param[in] thresh
   */
  void setColdstart(int leadTime, const std::vector<FieldThresh> &thresh);

  /**
   * @return XML repesenttation of local state
   */
  std::string toXml(void) const;


  /**
   * Check each lead time and change to coldstart if it is too old compared
   * to input time.
   *
   * @param[in] t  The time to compare to
   * @param[in] maxSecondsBeforeColdstart  Maximum age to keep state
   * @param[in]  coldstartThresh  The coldstart thresholds to use 
   */
  bool checkColdstart(const time_t &t,
		      int maxSecondsBeforeColdstart,
		      const std::vector<FieldThresh> &coldstartThresh);

  /**
   * Debug print
   * @param[in] verbose  True to print more
   */
  void print(bool verbose=false) const;

  /**
   * Retrieve information for a particular gen/lead time
   * @param[in] genTime
   * @param[in] leadTime
   * @param[out] item
   * @return true if successful
   */
  bool get(const time_t &genTime, int leadTime, MultiThreshItem &item) const;

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
   * Lead to multiple thresholds mapping
   */
  std::map<int, MultiThresh> _map;

  /**
   * @return pointer to the MultiThresh that goes with the leadtime if there
   * is one, or NULL if no such leadtime in the map
   * @param[in] leadTime
   */
  const MultiThresh *_mapFromLeadTime(int leadTime) const;
};

# endif

