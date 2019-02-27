/**
 * @file MultiThresh.hh
 * @brief Storage for thresholds for multiple fields with additional information
 * @class MultiThresh
 * @brief Storage for thresholds for multiple fields with additional information
 *
 *
 * Two thresholds are stored for each field, one is the actual threshold,
 * the second is the threshold that would go with a fixed field name (MDV)
 *
 * Additional information is a bias value, a time from which the data was
 * generated, and a flag as to whether it is 'coldstart' (i.e. fixed) threshold,
 * and a flag as to whether the values are those of the 'mother tile' or not.
 *
 * Within the XML that is read/written for this object, a tile index
 * is also read/written, and is passed out/into XML related methods.
 * No tile information is stored in local members.
 */

# ifndef    MultiThresh_hh
# define    MultiThresh_hh

#include <rapformats/FieldThresh2.hh>
#include <vector>
#include <map>

class TileInfo;

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
   * @param[out] tileIndex  tiling index to be parsed and returned
   */
  MultiThresh(const std::string &xml, const std::vector<std::string> &fields,
	      int &tileIndex);

  /**
   * 'Coldstart' constructor in which fields/thresholds are passed in
   *
   * @param[in] fieldThresh The thresholds
   * @param[in] fromMother  True if coldstart thresholds came from a mother tile
   *
   * No tiling information is stored locally, so none is passed in.
   */
  MultiThresh(const std::vector<FieldThresh2> &fieldThresh, bool fromMother);

  /**
   * 'Non coldstart' constructor
   *
   * @param[in] fieldthresh     Fields/thresholds
   * @param[in] bias            bias value
   * @param[in] generatingTime  time of data used to generate thresholds/bias
   * @param[in] obsValue        Observations data value
   * @param[in] fcstValue       Forecast data value
   * @param[in] fromMother      True if input values came from the mother tile
   *
   * No tiling information is stored locally, so none is passed in
   */
  MultiThresh(const std::vector<FieldThresh2> &fieldthresh,
	      double bias, const time_t &generatingTime,
	      double obsValue, double fcstValue, bool fromMother);
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
   *
   * @param[in] item  Object to merge into local state
   *
   * @return true for success
   */
  bool update(const MultiThresh &item);

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
  bool replaceValues(const MultiThresh &filtMap,
		     const std::vector<std::string> &filterFields);

  /**
   * Convert local state to an XML string, including the tiling indices
   *
   * @param[in] tileIndex  The tile index value
   * @param[in] indent   Number of tabs to indent the XML
   *
   * @return the XML string
   */
  std::string toXml(int tileIndex, int indent=0) const;

  /**
   * Set state to coldstart if it is not yet coldstart, only if the input time
   * is too new compared to local generation time
   *
   * @param[in] t  The time to compare to
   * @param[in] maxSecondsBeforeColdstart  Maximum age to keep state
   * @param[in]  coldstartThresh  The coldstart thresholds to use 
   *
   * @return true if everything was checked/modified as needed, false for error
   */
  bool checkColdstart(const time_t &t, int maxSecondsBeforeColdstart,
		      const std::vector<FieldThresh2> &coldstartThresh);

  /**
   * Debug print, including a lead time and tile index
   *
   * @param[in] leadTime  Lead seconds
   * @param[in] tileIndex Tile index
   * @param[in] info  Information about tiling
   * @param[in] verbose  True to print more stuff
   */
  void print(int leadTime, int tileIndex, const TileInfo &info,
	     bool verbose=false) const;

  /**
   * Debug logging, including a lead time and tile index
   *
   * @param[in] leadTime  Lead seconds
   * @param[in] tileIndex  Tile index
   * @param[in] verbose  True to print more stuff
   */
  void logDebug(int leadTime, int tileIndex, bool verbose=false) const;

  /**
   * Debug print to a string, including a lead time and tile index
   *
   * @param[in] leadTime  Lead seconds
   * @param[in] tileIndex Tile index
   * @param[in] info  Information about tiling
   * @param[in] verbose  True to print more stuff
   *
   * @return string with output print
   */
  std::string sprint(int leadTime, int tileIndex, const TileInfo &info,
		     bool verbose=false) const;

  /**
   * @return true if input names match local state in the same order
   *
   * @param[in] names  Field names
   * @param[in] printErrors  true to log an error when something doesn't match
   */
  bool namesOk(const std::vector<std::string> &names,
	       bool printErrors=true) const;

  /**
   * @return true if named field is one of the fields in the local state
   *
   * @param[in] name
   */
  bool hasField(const std::string &name) const;

  /**
   * Get the index associated with a field name 
   *
   * @param[in] fieldName
   * @return 0,1,..  or -1 for no match
   */
  int getThresholdIndex(const std::string &fieldName) const;

  /**
   * @return  A field name associated with this data.

   * This is the name that gets written to files as a fixed field name.
   * It is a single string with names/thresholds, where the thresholds are the
   * 2nd (fixed) ones in the FieldThresh2 objects
   *
   * @param[in] nameChars  Number of leading chars of field name to use
   * @param[in] precision  Number of digits precision in threshold
   */
  std::string fieldName2(int nameChars=2, int precision=2) const;

  /**
   * @return  A field name associated with this data.

   * This is the name that gets written to files as a fixed field name.
   * It is a single string with names/thresholds, where the thresholds are the
   * 2nd (fixed) ones in the FieldThresh2 objects
   *
   * This is retricted to a wanted subset of fields, so not all fields are
   * in the string.
   *
   * @param[in] ignore  Full names of the fields to ignore, can be empty
   * @param[in] nameChars  Number of leading chars of field name to use
   * @param[in] precision  Number of digits precision in threshold
   */
  std::string fieldName2Limited(const std::vector<std::string> &ignore,
				int nameChars=2, int precision=2) const;

  /**
   * Change local values to inputs
   *
   * @param[in] bias  Bias
   * @param[in] obsTime  The generating time
   * @param[in] obsValue  Observations value
   * @param[in] fcstValue  Forecast value
   * @param[in] nameThresh  a set of names and (actual) thresholds
   */
  void
  update(double bias, const time_t &obsTime, double obsValue, double fcstValue,
	 const std::vector<std::pair<std::string,double> > &nameThresh);

  /**
   * Add new fields to the state
   *
   * @param[in] nameThresh  a set of names and thresholds
   * 
   * @return true if added, false if already in state
   *
   * Uses each threshold for both of the state threshold values
   */
  bool add(const std::vector<std::pair<std::string,double> > &nameThresh);

  /**
   * Get the indexed actual threshold value, which is the first threshold
   * in the FieldThresh2 data
   *
   * @param[in] i  Index into thresholds
   * @param[out] thresh  Returned threshold
   *
   * @return true if index in range and value returned, false otherwise
   */
  bool getIthThreshold(int i, double &thresh) const;

  /**
   * Get the indexed field name
   *
   * @param[in] i  Index into thresholds
   * @param[out] name  Returned name
   *
   * @return true if index in range and value returned, false otherwise
   */
  bool getIthName(int i, std::string &name) const;

  /**
   * @return number of fields
   */
  inline int num(void) const {return static_cast<int>(_thresh.size());}

  /**
   * @return the bias
   */
  inline double getBias(void) const {return _bias;}

  /**
   * @return the obs value
   */
  inline double getObs(void) const {return _obsValue;}

  /**
   * @return the forecast value
   */
  inline double getFcst(void) const {return _fcstValue;}

  /**
   * @return 'yes' if this is the mothertile, otherwise return 'no'
   * @param[in] yes Value to return
   * @param[in] no Value to return
   */
  inline double getIsMother(double yes, double no) const
  {
    if (_motherTile)
    {
      return yes;
    }
    else
    {
      return no;
    }
  }

  /**
   * return the FieldThresh for a particular field
   *
   * @param[in] fieldName  
   * @param[out] item  Has the actual threshold and the name
   *
   * @return true if fieldName was found locally and item was set
   */
  bool get(const std::string &fieldName, FieldThresh &item) const;

  /**
   * Set obs value to input
   * @param[in] obsValue
   */
  inline void setObs(double obsValue) { _obsValue = obsValue; }

  /**
   * Value of XML tag
   */ 
  static const std::string _tag;

protected:
private:  

  bool _ok;                           /**< True if object is set */
  std::vector<FieldThresh2> _thresh;  /**< fieldname/thresholds  */
  double _bias;                       /**< Bias value */
  bool _coldstart;                    /**< True if the thresholds are set
				       *   to a coldstart value */
  bool _motherTile;                    /**< True if the thresholds are set
					*   to those from the 'mother tile' */
  time_t _generatingTime;             /**< The time of the data that was used
				       *   to generate thresholds */
  double _obsValue;                   /**< Value from observations data */
  double _fcstValue;                  /**< Value from forecast data */

  bool _replaceValue(const std::string &fieldName, const MultiThresh &filtMap);

};

# endif
