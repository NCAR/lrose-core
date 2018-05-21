/**
 * @file FieldThresh.hh
 * @brief Simple class to store a field name and threshold
 * @class FieldThresh
 * @brief Simple class to store a field name and threshold
 */

# ifndef    FieldThresh_hh
# define    FieldThresh_hh

#include <string>
#include <vector>

//----------------------------------------------------------------
class FieldThresh
{
public:

  /**
   * Empty
   */
  inline FieldThresh(void) :
    _ok(false),
    _fieldName("Unknown"),
    _thresh(0.0)
  {
  }

  /**
   * Members set from inputs
   * @param[in] field
   * @param[in] thresh
   */
  inline FieldThresh(const std::string &field, double thresh) :
    _ok(true),
    _fieldName(field),
    _thresh(thresh)
  {}

  /**
   * Name set from input, with threshold set to a bad value
   * @param[in] field
   */
  inline FieldThresh(const std::string &field) :
    _ok(true),
    _fieldName(field),
    _thresh(-999.99)
  {}

  /**
   * Constructor from XML string, with corraborating fieldName
   *
   * @param[in] xml  This gets parsed to set state
   * @param[in] fieldName
   *
   * _ok is set to false if fieldName is not the same as parsed XML value
   */
  FieldThresh(const std::string &xml, const std::string &fieldName);

  /**
   * Destructor
   */
  inline ~FieldThresh(void) {}

  /**
   * @return XML representation
   * @param[in] indent  Number of tabs to indent XML
   */
  std::string toXml(int indent=0) const;

  /**
   * @return XML representation without enclosing tag
   * @param[in] indent  Number of tabs to indent XML
   */
  std::string toXmlContent(int indent=0) const;

  /**
   * @return a debug representation
   */
  std::string sprint(void) const;

  /**
   * Print to stdout
   */
  void print(void) const;

  /**
   * Print to stdout, no \n
   */
  void printNoNewline(void) const;

  /**
   * Set threshold to input
   * @param[in] thresh
   */
  inline void setThresh(double thresh) {_thresh=thresh;}
  
  /**
   * Set threshold to input objects threshold
   *
   * @param[in] f  Object with threshold to use
   */
  inline void setThreshFromInput(const FieldThresh &f)
  {
    _thresh= f._thresh;
  }

  /**
   * Get threshold
   */
  inline double getThresh(void) const {return _thresh;}

  /**
   * Get the field name
   */
  inline std::string getField(void) const {return _fieldName;}

  /**
   * @return true if fieldnames match
   *
   * @param[in] f  Object from which to check field name
   */
  inline bool fieldMatch(const FieldThresh &f) const
  {
    return _fieldName == f._fieldName;
  }

  /**
   * @return true if names match
   *
   * @param[in] fieldName  
   */
  inline bool nameMatch(const std::string &fieldName) const
  {
    return _fieldName == fieldName;
  }

  /**
   * If the _fieldName matches one of the the input strings,
   * set _thresh to corresponding input double.
   * If no matches, do nothing.
   *
   * @param[in] nameThresh  pairs of fieldnames/thresholds
   */
  void update(const std::vector<std::pair<std::string,double> > &nameThresh);

  /**
   * @return true if object is set
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Value of XML tag
   */ 
  static const std::string _tag;

protected:

  bool _ok;                /**< True if set */
  std::string _fieldName;  /**< Name */
  double _thresh;          /**< Threshold */

private:  

};

# endif
