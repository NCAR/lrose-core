/**
 * @file MathParser.hh
 * @brief  Parse math equations and do what they say
 * @class MathParser
 * @brief  Parse math equations and do what they say
 */
#ifndef MATH_PARSER
#define MATH_PARSER

#include <rapmath/Node.hh>
#include <rapmath/FunctionDef.hh>
#include <string>
#include <vector>

class ProcessingNode;
class VolumeData;
class Filter;
class TaThreadQue;

class MathParser
{
public:

  /**
   * Types of filtering:
   * LOOP2D = looping within a volume, 2d slices, returns a 2d slice
   * LOOP2D_USER = looping within a volume, 2d slices, returns user defined data
   * LOOP1D = looping within a volume, 1d rays
   * VOLUME_BEFORE = processing the entire volume, before LOOP2d and LOOP1D
   * VOLUME_AFTER = processing the entire volume, after LOOP2d and LOOP1D
   */
  typedef enum {LOOP2D, LOOP2D_USER, LOOP1D, VOLUME_BEFORE,
		VOLUME_AFTER} Filter_t;

  /**
   * Constructor
   */
  MathParser(void);

  /**
   * Destructor
   */
  ~MathParser(void);

  /**
   * Evaluate the input string, and add a Filter object based on the
   * parsed results to the local state.
   *
   * @param[in] s  String to parse
   * @param[in] filterType  Indicator as to where to put result in state
   *
   * @return true if successful
   */
  bool parse(const std::string &s, Filter_t filterType,
	     const std::vector<std::string> &fixedConstants,
	     const std::vector<std::string> &userDataNames);

  /**
   * Store a user defined binary operator
   * @param[in] name
   * @param[in] descr  
   */
  inline void addUserBinaryOperator(const std::string &name,
				    const std::string &descr)
  {
    _userBinaryOperators.push_back(FunctionDef(name, descr));
  }    

  /**
   * Store a user defined unary operator
   * @param[in] name
   * @param[in] descr  
   */
  inline void addUserUnaryOperator(const std::string &name,
				   const std::string &descr)
  {
    _userUnaryOperators.push_back(FunctionDef(name, descr));
  }

  /**
   * @return number of 2d loop filters in the state
   */ 
  inline int numLoop2DFilters(void) const {return (int)_filters2d.size();}

  /**
   * @return number of 1d loop filters in the state
   */
  inline int numLoop1DFilters(void) const {return (int)_filters1d.size();}

  /**
   * @return reference to a 2d loop filter by index
   * @param[in] i  Index
   */   
  inline const Filter &loopFilter2dRef(int i) const {return _filters2d[i];}

  /**
   * @return reference to a 1d loop filter by index
   * @param[in] i  Index
   */   
  inline const Filter &loopFilter1dRef(int i) const {return _filters1d[i];}

  /**
   * @return the strings that are the input variables, based on state
   */
  std::vector<std::string> identifyInputs(void) const;

  /**
   * @return the strings that are the output variables, based on state
   */
  std::vector<std::string> identifyOutputs(void) const;
  
  /**
   * Clean out all the pointers owned by the object
   */
  void cleanup(void);

  /**
   * Process an entire volume of input data, before doing loop stuff, by
   * going through the VOLUME_BEFORE filters in sequential order
   *
   * @param[in,out]  data  Pointer to the volume, added to by this method
   */
  void processVolume(VolumeData *data) const;

  /**
   * Process 2 dimensional input data, as defined by the input data itself
   * by going through the LOOP2D/LOOP2D_USER filters in sequential order.
   *
   * @param[in,out] rdata Pointer to the volume, assumed to have 2d subsets,
   *                      added to by this method
   * @param[in] ii  Index into the 2d data
   */
  void processOneItem2d(VolumeData *rdata, int ii) const;
  void processOneItem2d(VolumeData *rdata, int ii, TaThreadQue *thread) const;

  /**
   * Process 1 dimensional input data, as defined by the input data itself
   * by going through the LOOP1D filters in sequential order.
   * @param[in,out] rdata Pointer to the volume, assumed to have 1d subsets,
   *                      added to by this method
   * @param[in] ii  Index into the 1d data
   */
  void processOneItem1d(VolumeData *rdata, int ii) const;
  
  /**
   * Process an entire volume of input data, after doing loop stuff, by going
   * through the VOLUME_AFTER filters in sequential order
   *
   * @param[in,out]  data  Pointer to the volume, added to by this method
   */
  void processVolumeAfter(VolumeData *data) const;

  /**
   * Trim leading and trailing whitespace from a string
   * @param[in,out]  s  String to trim
   */
  static void trim(std::string &s);

  /**
   * check for input string starting with a '(' and ending with ')'.
   * If so, remove those two edge chars from input string.
   * Also removes leading/training white space
   *
   * @param[in,out] s  String to modify
   */
  static bool parenRemove(std::string &s);

  /**
   * Detailed check for floating point values
   * @param[in] s  String to evaluate and see if it is a float
   */
  static bool isFloat(const std::string &s);

  /**
   * @return true if string is 'True', 'TRUE', 'true'
   *
   * @param[in] s  String to evaluate
   */
  static bool isTrue(const std::string &s);

  /**
   * @return true if string is 'False', 'FALSE', 'false'
   *
   * @param[in] s  String to evaluate
   */
  static bool isFalse(const std::string &s);

  std::string sprintUnaryOperators(void) const;
  std::string sprintBinaryOperators(void) const;

private:

  /**
   * library defined binary operator strings, such as "+"
   */
  std::vector<FunctionDef> _binaryOperators;

  /**
   * library defined unary operator strings, such as "exp"
   */
  std::vector<FunctionDef> _unaryOperators;

  /**
   * User define binary operator strings
   */
  std::vector<FunctionDef> _userBinaryOperators;

  /**
   * User define unary operator strings
   */
  std::vector<FunctionDef> _userUnaryOperators;

  /**
   * The filters that act on the entire volume at once, before the _filters2d
   * and _filters1d
   */
  std::vector<Filter> _volFilters;

  /**
   * The filters that are done in a loop of 2d data, with the loop defined by
   * the data class
   */
  std::vector<Filter> _filters2d;
  
  /**
   * The filters that are done in a loop of 1d data, with the loop defined by
   * the data class
   */
  std::vector<Filter> _filters1d;
  
  /**
   * The filters that act on the entire volume at once, after the _filters
   */
  std::vector<Filter> _volFiltersAfter;

  bool _insideParens(const std::string &s);
  ProcessingNode *_parse(const std::string &s);
  ProcessingNode *_parseIfThen(std::string &s2, std::size_t p);
  ProcessingNode *_parseAssignment(std::string &s2, std::size_t p);
  ProcessingNode *
  _parseBinaryOperator(std::string &s2, const std::string &op, int iop);
  ProcessingNode *
  _parseUnary(const std::string &s2, std::string &fname, std::string &part2);
  std::vector<ProcessingNode *>
  _commaSeparatedArgNodes(std::string &part2, bool &bad);
  ProcessingNode *_val(const std::string &s);
  void _processLoop(const Filter &filter, MathData *rdata) const;
  void _processV(const Filter &filter, VolumeData *rdata) const;
};
  
  
/**
 * @class Filter
 * @brief Simple class holding information about an individual Processing node
 *        at the top level.
 */
class Filter
  {
  public:
    inline Filter(void) : _filter(NULL) {}
    inline ~Filter(void) {}

    ProcessingNode *_filter;                 /**< Pointer to top node */
    Node::Pattern_t _pattern;                /**< Pattern, if any */
    std::vector<std::string> _inputs;        /**< The input variables */
    std::string _output;                     /**< The output variable */
    MathParser::Filter_t _dataType;          /**< The type of filter */

  protected:
  private:
  };
  

#endif
