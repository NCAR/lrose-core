/**
 * @file MathFindSimple.hh 
 * @brief Mimics matlab 'find', logical test on one input
 * @class MathFindSimple
 * @brief Mimics matlab 'find', logical tests on one input
 *
 * A simple comparison is a variable tested relative to a value, or to missing
 * 
 */

#ifndef FIND_SIMPLE_H
#define FIND_SIMPLE_H

// #include <rapmath/VolumeData.hh>
#include <rapmath/MathData.hh>
#include <rapmath/LeafContent.hh>
#include <string>

class LogicalArg;

//------------------------------------------------------------------
class MathFindSimple 
{
public:

  /**
   * @enum Compare_t
   * @brief The logical comparisions that are supported
   */
  typedef enum
  {
    GT = 0,  /**< Greater than */
    GE = 1,  /**< Greater than or equal */
    EQ = 2,  /**< Equal */
    LE = 3,  /**< Less than or equal */
    LT = 4,  /**< Less than */
  } Compare_t;

  /**
   * Empty constructor
   */
  MathFindSimple(void);

  /**
   * Constructor with tokens
   *
   * @param[in] name   Name of data
   * @param[in] comp   String with comparison 
   * @param[in] value  String with a numerical value
   *
   * "A < 17"  name="A",  comp="<",  value="17"
   */
  MathFindSimple(const std::string &name, const std::string &comp, 
	     const std::string &value);

  /**
   * Destructor
   */
  virtual ~MathFindSimple(void);

  /**
   * @return true if well formed (parsed string success, non-empty)
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Debug print to stdout, no "\n"
   */
  void print(void) const;

  /**
   * @return true if condition is satisfied at the input point
   * @param[in] data  The data
   * @param[in] ipt  Index into the data
   */
  bool satisfiesCondition(const MathData *data, int ipt) const;

  /**
   * @return name of what is being tested
   */
  inline std::string getName(void) const { return _data.getName();}

  /**
   * @return the string for a particular comparision enum
   * @param[in] c  Compare type to make a string for
   */
  static std::string comparisonString(const Compare_t &c);

  /**
   * @return true if this comparison is the 'simple' Name op Value
   */
  inline bool isSimpleCompareToNumber(void) const
  {
    return (_missingValue == false && _data.isVariable());
  }

  /**
   * @return true if this comparison is the 'simple' Name = missing
   */
  inline bool isSimpleCompareToMissing(void) const
  {
    return (_missingValue == true && _data.isVariable());
  }

  /**
   * @return true if this is a simple comparison and if so, set the return args
   * @param[out] compareName  Name of data
   * @param[out] c  Comparison
   * @param[out] compareV  The value compared to
   * @param[out] compareMissing True if checking data for missing
   */
  bool getSimpleCompare(std::string &compareName,
			MathFindSimple::Compare_t &c,
			double &compareV,
			bool &compareMissing) const;


  /**
   * @return the LogicalArg representation of this object
   */
  LogicalArg getLogicalArg(void) const;

protected:
private:


  bool _ok;                   /**< True for well formed */
  Compare_t _test;            /**< The comparison to make */
  bool _missingValue;         /**< True if comparison is to missing data value*/
  double _value;              /**< Value to compare to if not _missingValue */
  LeafContent _data;          /**< Data to compare to value */
};

#endif
