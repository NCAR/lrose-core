/**
 * @file FieldThresh2.hh
 * @brief Simple class to store a field name and two thresholds
 * @class FieldThresh2
 * @brief Simple class to store a field name and two thresholds
 */

# ifndef    FieldThresh2_hh
# define    FieldThresh2_hh

#include <string>
#include <rapformats/FieldThresh.hh>

//----------------------------------------------------------------
class FieldThresh2 : public FieldThresh
{
public:

  /**
   * Empty constructor
   */
  inline FieldThresh2(void) : FieldThresh(), _thresh2(0.0) { }

  /**
   * Constructor from base class FieldThresh, where _thresh2 is set to
   * the same value as the input threshold
   *
   * @param[in] f  FieldThresh to use
   */
  inline FieldThresh2(const FieldThresh &f) : FieldThresh(f), 
					      _thresh2(f.getThresh()) { }

  /**
   * Constructor, Members set from inputs
   *
   * @param[in] field
   * @param[in] thresh
   * @param[in] thresh2
   */
  inline FieldThresh2(const std::string &field, double thresh,
		      double thresh2) : FieldThresh(field, thresh),
					_thresh2(thresh2) {}

  /**
   * Constructor, Name set from input, with thresholds set to a bad value
   *
   * @param[in] field
   */
  inline FieldThresh2(const std::string &field) : FieldThresh(field),
						  _thresh2(-999.99)
  {}

  /**
   * Constructor from XML string, with corraborating fieldName
   *
   * @param[in] xml  This gets parsed to set state
   * @param[in] fieldName  Expect this to match the parsed data
   *
   * _ok is set to false if fieldName is not the same as parsed XML value
   */
  FieldThresh2(const std::string &xml, const std::string &fieldName);

  /**
   * Destructor
   */
  inline ~FieldThresh2(void) {}

  /**
   * @return XML representation
   *
   * @param[in] indent  number of tabs to indent the XML
   */
  std::string toXml2(int indent=0) const;

  /**
   * @return a debug representation
   */
  std::string sprint2(void) const;

  /**
   * Print to stdout
   */
  void print2(void) const;

  /**
   * Print to stdout, no \n
   */
  void printNoNewline2(void) const;

  /**
   * Set threshold to input
   *
   * @param[in] thresh
   */
  inline void setThresh2(double thresh) {_thresh2=thresh;}

  /**
   * Set threshold to input objects threshold
   *
   * @param[in] f  Object with threshold to use
   */
  inline void setThresh2FromInput(const FieldThresh2 &f)
  {
    _thresh2= f._thresh2;
  }

  /**
   * Get threshold
   */
  inline double getThresh2(void) const {return _thresh2;}

  /**
   * @return field name associated with this data with restrictions
   * specified.  Example if _fieldName="XYZDATA" and _thresh2=12.3456, if
   * nameChars=3 and precision=2 the returned value would be:
   * "XYZ12.34"
   *
   * @param[in] nameChars  Number of leading chars of field name to use
   * @param[in] precision  Number of digits precision in threshold
   */
  std::string dataFieldName2(int nameChars=2, int precision=2) const;

  /**
   * Value of XML tag
   */ 
  static const std::string _tag2;

protected:
private:  

  double _thresh2;          /**< Threshold number 2 */

};

# endif
