/**
 * @file LogicalArgs.hh
 * @brief Handler of arguments for Logical operations
 */

#ifndef LOGICAL_ARGS_H
#define LOGICAL_ARGS_H

#include <vector>
#include <string>
#include <rapmath/Find.hh>

class MathLoopData;
class MathData;

/**
 * @class LogicalArg
 * @brief One logical comparison
 */
class LogicalArg
{
public:
  /**
   * Constructor, arguments map directly to the members
   */
  LogicalArg(const std::string &name, double value, bool missingValue,
	     const MathFindSimple::Compare_t &test);

  /**
   * Empty constructor
   */
  inline LogicalArg(void) {}

  /**
   * Destructor
   */
  inline ~LogicalArg(void) {}

  /**
   * @return true if able to retrieve a value at a point, and the
   * the value satisfies the condition
   * @param[in] index  Into data
   */
  bool satisfiesCondition(int index) const;


  /**
   * Set the _data (and _value members when _valueIsMissing) using inputs
   * @param[in] rdata  The data from which to get what is needed
   * @return true if succcessful
   */
  bool synch(MathData *rdata);

protected:
private:
  std::string _variableName;   /**< The name of the variable */
  double _value;               /**< The value to compare to */
  bool _valueIsMissing;        /**< Special missing value
				*  comparison indicator */
  MathFindSimple::Compare_t _op;  /**< The test */
  MathLoopData *_data;         /**< place to store pointer to use */

};

/**
 * @class LogicalArgs
 * @brief All the logical test arguments, and the and/or operators between them
 *
 *
 * _args[0] _ops[0] _args[1] _ops[1] ... _ops[n] _args[n+1] 
 */
class LogicalArgs
{
public:

  /**
   * Empty constructor
   */
  inline LogicalArgs(void) {}

  /**
   * Destructor
   */
  inline ~LogicalArgs(void) {}

  inline void appendArg(const LogicalArg &a) { _args.push_back(a); }
  inline void appendOp(const Find::Logical_t &o) {_ops.push_back(o);}
  inline size_t numArgs(void) const {return _args.size(); }
  inline LogicalArg & operator[](size_t i) {return _args[i];}
  inline const LogicalArg & operator[](size_t i) const {return _args[i];}

  /**
   * Update status at opIndex'th position 
   * @param[in] bi  Status up to the previous position true or false
   * @param[in] opIndex  Index to the operators
   * @param[out] ret the result of combining bi with opIndex and next arg
   */
  void updateStatus(bool bi, int opIndex, bool &ret) const;

protected:
private:
  /**
   * The arguments (should  be one more than _ops.size())
   *
   */
  std::vector<LogicalArg> _args;

  /**
   * The operators between args
   */
  std::vector<Find::Logical_t> _ops;


};

#endif
