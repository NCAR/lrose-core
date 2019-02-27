/**
 * @file UnaryNode.hh
 * @brief A Node that is a unary operation
 * @class UnaryNode
 * @brief A Node that is a unary operation
 */
#ifndef UNARY_NODE_H
#define UNARY_NODE_H

#include <rapmath/Node.hh>
#include <rapmath/ProcessingNode.hh>

class UnaryNode : public Node
{
public:
  /**
   * A hardwired unary operation
   *
   * @param[in] op  The operation
   * @param[in] args  The arguments for this operation, which are now
   *                  owned by this object
   */
  UnaryNode(ProcessingNode::UnaryOperator_t op,
	    std::vector<ProcessingNode *> &args);


  /**
   * A user defined unary operation
   *
   * @param[in] args  The arguments for this operation, which are now
   *                  owned by this object
   * @param[in] key  The keyword name of the user defined operation
   */
  UnaryNode(std::vector<ProcessingNode *> &args, const std::string &key);

  /**
   * Destructor
   */
  virtual ~UnaryNode(void);

  #include <rapmath/NodeVirtualMethods.hh>

  /**
   * If this is a user unary node, return it's keyword in the arg
   *
   * @param[out] keyword
   * @param[in] warn  True to warn if not a user unary node, or there are
   *  problems, false to be silent
   *
   * @return true if it was a user unary node
   */
  bool getUserUnaryKeyword(std::string &keyword, bool warn=false) const;
  
  /**
   * @return true if this is user unary node, false otherwise
   */
  bool isUserFunction(void) const;

  /**
   * @return true if it is a fixed unary op, with multiple arguments
   */
  bool isMultiArgFunction(void) const;
  
  /**
   * @return the actual strings that are the unary args
   */
  std::vector<std::string> getUnaryNodeArgStrings(void) const;

  /**
   * @return a pointer to the local argument pointers
   */
  inline std::vector<ProcessingNode *> *unaryOpArgs(void)
  {
    return &_value;
  }

  /**
   * @return a pointer to the local argument pointers
   */
  inline const std::vector<ProcessingNode *> *constUnaryOpArgs(void) const
  {
    return &_value;
  }

  /**
   * @return the operator
   */
  inline ProcessingNode::UnaryOperator_t getOp(void) const {return _uop;}

private:

  std::string _userUopKey;               /**< user op keyword, or empty */
  ProcessingNode::UnaryOperator_t _uop;  /**< the operation */
  std::vector<ProcessingNode *> _value;  /**< Any number of arguments */
};

#endif
