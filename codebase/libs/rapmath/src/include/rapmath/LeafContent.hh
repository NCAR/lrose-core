/**
 * @file LeafContent.hh
 * @brief LeafContent is either a numerical value or the name of a variable
 *        or a special missing data flag
 * 
 * @class LeafContent
 * @brief LeafContent is either a numerical value or the name of a variable
 *        or a special missing data flag
 */

#ifndef LEAF_CONTENT_H
#define LEAF_CONTENT_H

#include <vector>
#include <string>
#include <cstdio>

class VolumeData;
class MathData;

class LeafContent
{
public:
  /**
   * Empty constructor
   */
  LeafContent(void);

  /**
   * Constructor for a variable
   * @param[in] s  Name of the variable
   *
   * If s == PI or pi, constructor is for the value 3.14159
   * If s == missing constructor is for the missing value
   */
  LeafContent(const std::string &s);

  /**
   * Constructor for a value
   * @param[in] s  Name (string) for the value
   * @param[in] v  The value
   */
  LeafContent(const std::string &s, double v);

  /**
   * Destructor
   */
  ~LeafContent(void);

  /**
   * Debug print, no \n
   */
  void print(void) const;

  /**
   * @return true if object is a variable
   */
  inline bool isVariable(void) const {return _isVariable;}

  /**
   * @return the name 
   */
  inline std::string getName(void) const {return _name;}

  /**
   * @return true if able to set value arg at a point
   * @param[in] data 
   * @param[in] ipt  Index into data
   * @param[out] v  Value
   *
   * If not a variable, return the fixed value (unless missing, then return
   * false)
   *
   * If is a variable, query data for the value at ipt for _name
   */
  bool getValue(const MathData *data, int ipt, double &v) const;

  /**
   * @return true if able to set fixed value arg at a point
   * @param[out] v  Value
   *
   * If not a variable, return the fixed value (unless missing, then return
   * false)
   *
   * If is a variable, return false with a logged error
   */
  bool getValue(double &v) const;

private:

  bool _isVariable;  /**< True if content is a variable, referenced by _name*/
  double _value;     /**< The fixed value when not _isVariable */
  std::string _name; /**< The variable name, or value string */
  bool _missingData; /**< Special case of a non variable 'missing data' value */
};

#endif
