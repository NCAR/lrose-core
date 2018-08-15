/**
 * @file FunctionDef.hh
 * @brief A description of a function which is a name and comments
 * @class FunctionDef
 * @brief A description of a function which is a name and comments
 */
#ifndef FUNCTION_DEF_H
#define FUNCTION_DEF_H

#include <string>

class FunctionDef
{
public:
  /**
   * @param[in] name  The function name
   * @param[in] descrs Description
   */
  inline FunctionDef(const std::string &name, const std::string &descr) :
    _name(name), _description(descr)  {  }

  /**
   * Destructor
   */
  inline virtual ~FunctionDef(void) {}

  std::string _name;
  std::string _description;

private:

};

#endif
