/**
 * @file BinaryArgs.hh
 * @brief Handler of arguments for binary operations
 */
#ifndef BINARY_ARGS_H
#define BINARY_ARGS_H

#include <rapmath/ProcessingNode.hh>
#include <vector>
#include <string>

class MathLoopData;

/**
 * @class BinaryArg
 * @brief One argument for a binary operation
 */
class BinaryArg
{
public:

  /**
   * Constructor for a named variable
   */
  BinaryArg(const std::string &name);

  /*
   * Constructor for value or missing value
   */
  BinaryArg(double value, bool missing);

  inline ~BinaryArg(void) {}

  /**
   * @return true if able to retrieve a value at a point
   * @param[in] index  Into data
   * @param[out] v  Value
   */
  bool value(int index, double &v) const;

protected:
private:
  bool _isVariable;            /**< True if the arg is a variable */
  std::string _variableName;   /**< The name of the variable */
  double _value;               /**< The value when !_isVariable */
  bool _valueIsMissing;        /**< Special missing value indicator */
  MathLoopData *_data;         /**< place to store pointer to use */


};

/**
 * @class BinaryArgs
 * @brief All the arguments, and the operators between them
 *
 * _args[0] _ops[0] _args[1] _ops[1] ... _ops[n] _args[n+1] 
 */
class BinaryArgs
{
public:

  inline BinaryArgs(void) {}
  inline ~BinaryArgs(void) {}

  inline void appendArg(const BinaryArg &b) {_args.push_back(b);}
  inline void appendOp(const ProcessingNode::Operator_t &o) {_ops.push_back(o);}

protected:
private:
  /**
   * The arguments (should  be one more than _ops.size())
   *
   */
  std::vector<BinaryArg> _args;

  /**
   * The operators (should  be one less than _ops.size())
   */
  std::vector<ProcessingNode::Operator_t> _ops;

};

#endif
