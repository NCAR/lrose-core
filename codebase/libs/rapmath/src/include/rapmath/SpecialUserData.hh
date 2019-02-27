/**
 * @file SpecialUserData.hh
 * @brief Contains map from name to pointer to MathUserData
 * @class SpecialUserData
 * @brief Contains map from name to pointer to MathUserData
 */

#ifndef SPECIAL_USER_DATA_HH
#define SPECIAL_USER_DATA_HH

#include <rapmath/MathUserData.hh>
#include <string>
#include <vector>
#include <map>

//------------------------------------------------------------------
class SpecialUserData 
{
public:

  /**
   * Constructor
   * @param[in] ownsPointers True to take ownership of local pointers and
   * delete them in the destructor, false to not delete them in the destructor
   */
  SpecialUserData(bool ownsPointers=true);

  /**
   * Destructor
   */
  virtual ~SpecialUserData(void);

  /**
   * Set the ownership state to the input value
   * @param[in] status
   */
  inline void setOwnership(bool status) {_ownsPointers = status;}

  /**
   * @return true if input name is one of the named user data pointers
   * @param[in] name
   */
  bool hasName(const std::string &name) const;

  /**
   * Try to add a map entry from name to input pointer
   * @param[in] name
   * @param[in] v  Pointer
   * @return true if added, false if already there
   */
  bool store(const std::string &name, MathUserData *v);

  /**
   * @return mapping of input name to a pointer, unless name is not
   * in the map, in which case return NULL
   * @param[in] name
   */
  MathUserData *matchingDataPtr(const std::string &name);

  /**
   * @return mapping of input name to a pointer, unless name is not
   * in the map, in which case return NULL
   * @param[in] name
   */
  const MathUserData *matchingDataPtrConst(const std::string &name) const;

  /**
   * @return number of named user data items in the state
   */
  inline size_t size(void) const {return _special.size();}

protected:
private:

  /**
   * Useful typedef
   */
  typedef std::map<std::string, MathUserData *> Special_t;

  /**
   * Useful typedef
   */
  typedef Special_t::iterator Special_iterator_t;

  /**
   * Useful typedef
   */
  typedef Special_t::const_iterator Special_const_iterator_t;

  /**
   * Map from names of special data fields to pointer to special values
   * This gets filled in by the filtering
   */
  Special_t _special;

  bool _ownsPointers;  /**< Pointer ownership flag */
};

#endif
