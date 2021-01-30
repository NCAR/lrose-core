/**
 * @file Find.hh 
 * @brief Mimics matlab 'find', logical tests on any number of inputs
 * @class Find
 * @brief Mimics matlab 'find', logical tests on any number of inputs
 */

#ifndef FIND_H
#define FIND_H
#include <rapmath/MathFindSimple.hh>
#include <rapmath/VolumeData.hh>
#include <rapmath/MathLoopData.hh>
#include <string>
#include <vector>

class LogicalArgs;

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
   * @num Pattern_t
   * @brief The types of comparison patterns
   */
  typedef enum
  {
    SIMPLE_COMPARE_TO_NUMBER,   /**< Simple A <op> <number> */
    SIMPLE_MULTIPLES,           /**< Any number of simple comparisions 
				 *   with ands/ors between */
    COMPLEX,                    /**< More than one test */
    UNKNOWN                     /**< don't know */
  } Pattern_t;

  /**
   * Empty constructor
   */
  Find(void);

  /**
   * Constructor from a parsed string
   *
   * @param[in] s  String to parse  "(A <= 1.0 & C < 3) | D >= 7"
   */
  Find(const std::string &s);

  /**
   * Constructor from token strings
   *
   * @param[in] token  tokens, each of which was parsed as whitespace separated
   *                   substrings of some original string
   *
   * @param[in] ind0  Index to first token to use in building the object
   * @param[in] ind1  Index to last token to use in building the object
   */
  Find(const std::vector<std::string> &token, const int ind0, const int ind1);

  /**
   * Constructor for the simple case of  A <op> <number>
   *
   * @param[in] name   Name of data
   * @param[in] comp   String with comparison 
   * @param[in] value  String with a numerical value
   *
   * An example: 'X < 7'
   *  name = X
   *  comp = <
   *  value = 7
   */
  Find(const std::string &name, const std::string &comp,
       const std::string &value);

  /**
   * Destructor
   */
  virtual ~Find(void);

  /**
   * @return true if object is well formed (parsed string success, non-empty)
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Debug print to stdout, recursive, no '\n'
   */
  void print(void) const;

  /**
   * Debug print to stdout, from top level, which does the recursive print()
   * followed by '\n'
   */
  void printTop(void) const;

  /**
   * @return true if conditions are satisfied at the input point
   *
   * @param[in] data  The data
   * @param[in] ipt  Index into the data
   *
   * This is called after synch()
   */
  bool satisfiesConditions(const MathData *data, int ipt) const;

  /**
   * Append the names used in the Find to the input vector
   * @param[in,out] names
   */
  void fields(std::vector<std::string> &names) const;

  /**
   * @return true if local state indicates it is a simple test
   */
  inline bool isSimple(void) const {return _isSimple;}

  /**
   * @return reference to the Simple test (makes sense only when isSimple()
   */
  inline MathFindSimple &getSimple(void) {return _simple;}

  /**
   * Parse a string for an assumed logical and/or 
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

  /**
   * @return the pattern value for this object
   */
  inline Pattern_t pattern(void) const {return _pattern;}

  /**
   * @return true if this is a simple comparison and if so, set the return args
   * @param[out] compareName  Name of data
   * @param[out] c  Comparison
   * @param[out] compareV  The value compared to
   * @param[out] compareMissing True if checking data for missing
   */
  bool getSimpleCompare(std::string &compareName, MathFindSimple::Compare_t &c,
			double &compareV, bool &compareMissing) const;

  /**
   * Construct and return the LogicalArgs representation of the object.
   * @param[out] args  Set if possible
   * @return true if able to set the args output
   */
  bool getMultiCompare(LogicalArgs &args) const;

protected:
private:

  bool _ok;           /**< True for well formed */
  bool _isSimple;     /**< True for a simple test "A <= 3" */
  Pattern_t _pattern; /**< The patten for this comparison */
  
  /**
   *  The logical tests when multiple tests (_isSimple=false)
   */
  std::vector<std::pair<Find, Logical_t> > _multiple; 

  /**
   * The simple test when not multiple tests (_isSimple=true)
   */
  MathFindSimple _simple;

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
