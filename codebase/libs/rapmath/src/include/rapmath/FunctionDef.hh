/**
 * @file FunctionDef.hh
 * @brief A description of a function which is a name and comments
 * @class FunctionDef
 * @brief A description of a function which is a name and comments
 */
#ifndef FUNCTION_DEF_H
#define FUNCTION_DEF_H

#include <string>
#include <algorithm>

class FunctionDef
{
public:
  /**
   * @param[in] name  The function name
   * @param[in] descr Description
   */
  inline FunctionDef(const std::string &name, const std::string &descr) :
    _name(name), _description(splitInLines(descr, 70))  {  }

  /**
   * construct from pieces
   *   _description = 'leftHandSide=name(args), comments'
   *   _name = name
   */
  inline FunctionDef(const std::string &name,
		     const std::string &leftHandSide,
		     const std::string &args,
		     const std::string &comments) :
    _name(name)
  {
    _description = leftHandSide + "=";
    _description += name;
    _description += "(";
    _description += args;
    _description += "), ";
    _description += comments;
    _description = splitInLines(_description, 70);
  }

  /**
   * Destructor
   */
  inline virtual ~FunctionDef(void) {}

  inline static std::string splitInLines(std::string source, std::size_t width,
					 std::string whitespace = " \t\r")
  {
    std::size_t  currIndex = width - 1;
    std::size_t  sizeToElim;
    while ( currIndex < source.length() )
    {
      
      currIndex = source.find_last_of(whitespace,currIndex + 1);
      if (currIndex == std::string::npos)
	break;
      currIndex = source.find_last_not_of(whitespace,currIndex);
      if (currIndex == std::string::npos)
	break;
      sizeToElim = source.find_first_not_of(whitespace,currIndex + 1) - currIndex - 1;
      source.replace( currIndex + 1, sizeToElim , "\n");
      currIndex += (width + 1); //due to the recently inserted "\n"
    }
    return source;
  }


  inline static std::string capitalizeString(const std::string &s)
  {
    std::string ret = s;
    transform(ret.begin(), ret.end(), ret.begin(),
	      [](unsigned char c){ return std::toupper(c); });
    return ret;
  }

  std::string _name;            /**< Name of the function */
  std::string _description;     /**< Description of the function */


  bool operator<(const FunctionDef &f) const
  {
    return capitalizeString(_name) <= capitalizeString(f._name);
  }

  bool operator<=(const FunctionDef &f) const
  {
    return capitalizeString(_name) <= capitalizeString(f._name);
  }

private:

};

#endif
