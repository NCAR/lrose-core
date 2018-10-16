/**
 * @file NamePair.hh
 * @brief Internal/External name pairs
 *
 * @class NamePair
 * @brief Internal/External name pairs
 *
 * Internal name is what is used within the algorithm.
 * External name is what is written externally or read externally.
 *
 * Because the inputs can come from more than one source, the external names
 * can be redundant.  The internal names must be unique.
 *
 * Struct-like class with public members
 */

#ifndef NAME_PAIR_H
#define NAME_PAIR_H

#include <string>

class NamePair
{
public:
  /**
   * Empty constructor
   */
  inline NamePair(void): _internal("NONE"), _external("NONE") {}

  /**
   * Constructor
   * @param[in] internal
   * @param[in] external
   */
  inline NamePair(const std::string &internal, const std::string &external) :
    _internal(internal),  _external(external) {}

  /**
   * Destructor
   */
  inline ~NamePair(void) {}

  /**
   * @return true if input matches internal name
   * @param[in] name
   */
  inline bool isInternalName(const std::string &name) const
  {
    return name == _internal;
  }

  /**
   * @return true if input matches external name
   * @param[in] name
   */
  inline bool isExternalName(const std::string &name) const
  {
    return name == _external;
  }

  std::string _internal; /**< Internal (algorithms) name, public */
  std::string _external; /**< External (data files) name, public */

protected:
private:
};

#endif
