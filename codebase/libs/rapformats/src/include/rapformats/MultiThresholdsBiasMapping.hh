/**
 * @file MultiThresholdsBiasMapping.hh
 * @brief Mapping from lead seconds to bias and to threshold for all gen times
 * @class MultiThresholdsBiasMapping
 * @brief Mapping from lead seconds to bias and to threshold for all gen times
 *
 * Stored to SPDB by gen time, contains XML mappings from lead seconds to
 * bias, and from lead seconds to threshold
 */

# ifndef    MultiThresholdsBiasMapping_hh
# define    MultiThresholdsBiasMapping_hh

#include <rapformats/MultiThreshFcstBiasMapping.hh>
#include <string>
#include <vector>

class MultiThreshItem;
class FieldThresh;

//----------------------------------------------------------------
class MultiThresholdsBiasMapping
{
public:
  /**
   * Empty
   */
  MultiThresholdsBiasMapping(void);

  /**
   * Constructor with overall lead times and fields passed in ,
   * Thresholds not known (filled in later)
   * 
   * @param[in] ltHours The lead times (hours)
   * @param[in] fields  The field names
   */
  MultiThresholdsBiasMapping(const std::vector<double> &ltHours,
			     const std::vector<std::string> &fields);

  /**
   * Destructor
   */
  virtual ~MultiThresholdsBiasMapping(void);
  
  /**
   * Clear to completely empty
   */
  void clearMapping(void);


  /**
   * Set to coldstart values for gen times spaced apart evenely starting at
   * (h,m,s)=(0,0,0)
   * @param[in]  genFrequencySeconds  Gentimes: (0,0,0) + k*genFrequencySeconds
   *             k=0,1,..
   * @param[in] fieldThresh  The coldstart fieldname/threshold pairs
   *
   * @return true if successful
   */
  bool setColdStart(int genFrequencySeconds,
		    const std::vector<FieldThresh> &fieldThresh);

  /**
   * Set to coldstart values for a particular gen time/lead time
   * @param[in] genTime
   * @param[in] leadTime
   * @param[in] fieldThresh  The coldstart fieldname/threshold pairs
   *
   * @return true if successful
   */
  bool setColdStart(const time_t &genTime, int leadTime,
		    const std::vector<FieldThresh> &fieldThresh);

  /**
   * Update local state using input
   * @param[in] item   The values to be inserted into state
   * 
   * @return true if successful
   */
  bool update(const MultiThreshItem &item);

  /**
   * @return XML string of state
   */
  std::string toXml(void) const;

  /**
   * Parse XML string to set portions of state, comparing to local values
   *
   * @param[in] xml  The XML to parse
   * @param[in] fieldsAndLeadSet  True if local state has fields and
   *                              lead times to compare parsed XML to
   * @return true if successful
   */
  bool fromXml(const std::string &xml, bool fieldsAndLeadSet=true);

  /**
   * Set state to coldstart if not yet coldstart, and input time is too
   * new compared to local time
   * @param[in] t  The time to compare to
   * @param[in] maxSecondsBeforeColdstart  Maximum age to keep state
   * @param[in]  coldstartThresh  The coldstart thresholds to use 
   * @return true if successful
   */
  bool checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
		      const std::vector<FieldThresh> &coldstartThresh);

  /**
   * Debug print
   * @param[in] t  The time of this data
   * @param[in] verbose  True for more printing
   */
  void printState(const time_t &t, bool verbose) const;

  
  /**
   * Retrieve information for a particular gen/lead time
   * @param[in] genTime
   * @param[in] leadTime
   * @param[out] item
   * @return true if successful
   */
  bool get(const time_t &genTime, int leadTime, MultiThreshItem &item) const;

  /**
   * Retrieve information for a particular gen/lead/field
   * @param[in] genTime
   * @param[in] leadTime
   * @param[in] fieldName  
   * @param[out] item
   * @return true if successful
   */
  bool get(const time_t &genTime, int leadTime, const std::string &fieldName,
	   FieldThresh &item) const;

 protected:
 private:  

  std::vector<std::string> _fields;        /**< Field names */
  std::vector<int> _leadSeconds;           /**< Lead seconds */

  /**
   * The mappings for each gen time
   */
  std::vector<MultiThreshFcstBiasMapping> _fcst;

  std::vector<std::string> _fieldsFromXml(const std::string &xml);
  std::vector<int> _leadSecondsFromXml(const std::string &xml);
  std::string _fieldsToXml(void) const;
  std::string _leadsToXml(void) const;
  std::string _mappingsToXml(void) const;
  bool _fieldThreshNamesOk(const std::vector<FieldThresh> &fieldThresh) const;
};


# endif
