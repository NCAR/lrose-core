// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 * @file Find.hh 
 * @brief Mimics matlab 'find', logical tests on any number of inputs
 * @class Find
 * @brief Mimics matlab 'find', logical tests on any number of inputs
 * 
 */

#ifndef FIND_H
#define FIND_H
#include <FiltAlg/FindSimple.hh>
#include <FiltAlg/Comb.hh>
#include <string>
#include <vector>

//------------------------------------------------------------------
class Find 
{
public:

  /**
   * @enum Logical_t
   * @brief The logical and/or 
   */
  typedef enum
  {
    OR = 0,  /**< Or */
    AND = 1,  /**< And */
    NONE = 2  /**< Terminator (not a comparison operator)*/
  } Logical_t;

  /**
   * Empty constructor
   */
  Find(void);

  /**
   * Constructor from a parsed string
   *
   * @param[in] s  String to parse  "(A <= 1.0 & C < 3) | D >= 7"
   * @param[in] vlevel_tolerance   Allowed error in vlevel values
   */
  Find(const std::string &s, const double vlevel_tolerance);


  /**
   * Constructor from token strings
   *
   * @param[in] token  tokens
   * @param[in] ind0  Index to first token
   * @param[in] ind1  Index to last token
   * @param[in] vlevel_tolerance   Allowed error in vlevel values
   */
  Find(const std::vector<std::string> &token, const int ind0, const int ind1,
       const double vlevel_tolerance);

  /**
   * Constructor for simple case
   * @param[in] name   Name of data
   * @param[in] comp   String with comparison 
   * @param[in] value  String with a numerical value
   * @param[in] vlevel_tolerance   Allowed error in vlevel values
   */
  Find(const std::string &name, const std::string &comp,
       const std::string &value, const double vlevel_tolerance);

  /**
   * Destructor
   */
  virtual ~Find(void);

  /**
   * @return true if well formed (parsed string success, non-empty)
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Debug print to stdout
   */
  void print(void) const;

  /**
   * Debug print to stdout, from top level
   */
  void printTop(void) const;

  /**
   * @return true if input named things are found in 'comb' input
   * @param[in] comb  Input to check against
   */
  bool isConsistent(const Comb &comb) const;

  /**
   * Set pointers to the data using input Comb object
   *
   * @param[in] comb   Object with pointers to Data set
   *
   * @return true if successful
   */
  bool setPointers(const Comb &comb);

  /**
   * @return true if conditions are satisfied at the input point
   * @param[in] ipt  Index into the grids
   * @param[in] vlevel  Vertical level
   */
  bool satisfiesConditions(const int ipt, const double vlevel) const;

  /**
   * @return true if local state is for a single test
   */
  inline bool is_single(void) const {return _is_single;}

  /**
   * @return reference to the single test (makes sense only when is_single()
   */
  inline FindSimple &get_single(void) {return _single;}

  /**
   * Parse a string for an assumed logical and or 
   *
   * @param[in] s  String containing (hopefully) AND or OR
   *
   * @return the enumerated result, NONE if error.
   */
  static Logical_t parseLogical(const std::string &s);

  /**
   * Convert a Logical_t to a string 
   *
   * @param[in] l  Enumerated value to convert
   */
  static std::string logicalString(const Logical_t &l);

protected:
private:

  bool _ok;           /**< True for well formed */
  double _vlevel_tolerance; /**< Allowed error in vertical levels */
  bool _is_single;        /**< True for a single test "A <= 3" */
  /**< The logical tests when multiple tests */
  std::vector<std::pair<Find, Logical_t> > _multiple; 
  FindSimple _single;     /**< The single test when not multiple tests */

  std::string::size_type _nextToken(const std::string &s,
				    const std::string::size_type i,
				    std::vector<std::string> &tokens);

  /**
   * Parse the tokens over a range of indices, updating the local state
   * @param[in] tokens  A set of tokens
   * @param[in] ind0  Lowest token index to look at
   * @param[in] ind1  Highest token index to look at
   */
  void _tokenParse(const std::vector<std::string> &tokens, const int ind0,
		   const int ind1);

  /**
   * Parse the tokens after a left paren, up to the matching right paren,
   * updating the local state.
   *
   * @param[in] tokens  A set of tokens
   * @param[in] leftParen  Index to the "("
   * @param[in] ind1  Highest token index to look at
   *
   * @return Index to the token past the matching right paren.
   */
  int _parenParse(const std::vector<std::string> &tokens, const int leftParen,
		  const int ind1);

  /**
   * Parse the tokens when expect name test value, followed by either nothing
   * else, or a logical and/or operator.
   *
   * @param[in] tokens  A set of tokens
   * @param[in] k  Index where name is
   * @param[in] ind1  Highest token index to look at
   *
   * @return Index to the token past the ones parsed.
   */
  int _comparisonParse(const std::vector<std::string> &tokens, const int k, 
		       const int ind1);

  /**
   * Finish a particular parse, where the Find object F has been created.
   * @param[in] F  Object to put to state
   * @param[in] tokens  A set of tokens
   * @param[in] k2  Pointer to just past tokens used to build F
   * @param[in] ind1  Highest token index to look at
   */
  int _finishOne(const Find &F, const std::vector<std::string> &tokens, 
		 const int k2, const int ind1);
};

#endif
