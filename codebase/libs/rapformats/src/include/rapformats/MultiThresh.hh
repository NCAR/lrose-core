/**
 * @file MultiThresh.hh
 * @brief Storage for thresholds for multiple fields with additional information
 * @class MultiThresh
 * @brief Storage for thresholds for multiple fields with additional information
 *
 * Additional information is a bias value, a time from which the data was
 * generated, and a flag as to whether it is 'coldstart' (i.e. fixed) threshold
 */

# ifndef    MultiThresh_hh
# define    MultiThresh_hh

#include <rapformats/FieldThresh.hh>
#include <vector>

//----------------------------------------------------------------
class MultiThresh
{
public:

  /**
   * Empty
   */
  MultiThresh(void);

  /**
   * constructor in which an XML string is parsed, as would be created by
   * toXml()
   *
   * @param[in] xml  The string to parse
   * @param[in] fields  Corraborating field names to check for in parsed xml
   */
  MultiThresh(const std::string &xml, 
	      const std::vector<std::string> &fields);

  /**
   * 'Coldstart' constructor in which fields/coldstart thresholds are passed in
   *
   * @param[in] fieldThresh The coldstart values
   */
  MultiThresh(const std::vector<FieldThresh> &fieldThresh);

  /**
   * Non coldstrat constructor
   *
   * @param[in] fieldthresh     Fields/thresholds
   * @param[in] bias            bias value
   * @param[in] generatingTime  time of data used to generate thresholds/bias
   * @param[in] obsValue        Observations data value
   * @param[in] fcstValue       Forecast data value
   */
  MultiThresh(const std::vector<FieldThresh> &fieldthresh,
	      double bias, const time_t &generatingTime,
	      double obsValue, double fcstValue);
  /**
   * Destructor
   */
  ~MultiThresh(void);

  /**
   * @return true if values are set
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Update local state with inputs
   * @param[in] item
   * @return true for success
   */
  bool update(const MultiThresh &item);

  /**
   * Convert local state to an XML string
   * @return the XML string
   */
  std::string toXml(void) const;

  /**
   * Set state to coldstart if not yet coldstart, and input time is too
   * new compared to local time
   * @param[in] t  The time to compare to
   * @param[in] maxSecondsBeforeColdstart  Maximum age to keep state
   * @param[in]  coldstartThresh  The coldstart thresholds to use 
   */
  bool checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
		      const std::vector<FieldThresh> &coldstartThresh);

  /**
   * Debug print, including a lead time
   * @param[in] leadTime  
   * @param[in] verbose  True to print more stuff
   */
  void print(int leadTime, bool verbose=false) const;

  /**
   * @return true if input names match local state in the same order
   * @param[in] names  Field names
   * @param[in] printErrors  true to print out when something doesn't match
   */
  bool namesOk(const std::vector<std::string> &names,
	       bool printErrors=true) const;

  /**
   * @return field name associated with this data
   * @param[in] nameChars  Number of leading chars of field name to use
   * @param[in] precision  Number of digits precision in threshold
   */
  std::string fieldName(int nameChars=2, int precision=2) const;

  /**
   * Change local values to inputs
   * @param[in] bias  
   * @param[in] obsTime  The generating time
   * @param[in] obsValue
   * @param[in] fcstValue
   */
  void update(double bias, const time_t &obsTime, double obsValue,
	      double fcstValue);

  /** Get the indexed threshold value
   * @param[in] i  Index into thresholds
   * @param[out] thresh  Returned threshold
   * @return true if index in range and value returned, false otherwise
   */
  bool getIthThreshold(int i, double &thresh) const;

  /**
   * return the FieldThresh for a particular field
   * @param[in] fieldName  
   * @param[out] item
   * @return true if fieldName was found locally and item was set
   */
  bool get(const std::string &fieldName, FieldThresh &item) const;

  /**
   * Value of XML tag
   */ 
  static const std::string _tag;

protected:
private:  

  bool _ok;                           /**< True if object is set */
  std::vector<FieldThresh> _thresh;   /**< fieldname/threshold pairs */
  double _bias;                       /**< Bias value */
  bool _coldstart;                    /**< True if the thresholds are set
				       *   to a coldstart value */
  time_t _generatingTime;             /**< The time of the data that was used
				       *   to generate thresholds */
  double _obsValue;                   /**< Value from observations data */
  double _fcstValue;                  /**< Value from forecast data */
  
};

# endif
