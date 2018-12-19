/**
 * @file BinaryNode.hh
 * @brief  Node that is of form A <op> B
 * @class BinaryNode
 * @brief  Node that is of form A <op> B
 */

#ifndef BINARY_NODE_H
#define BINARY_NODE_H

#include <rapmath/Node.hh>
#include <rapmath/ProcessingNode.hh>

class BinaryNode : public Node
{
public:
  /**
   * Constructor for hardwired op
   * @param[in] left  Pointer to the left side, now owned
   *                  by this object
   * @param[in] right  Pointer to the right side, now owned
   *                  by this object
   * @param[in] op  The operation
   */
  BinaryNode(ProcessingNode *left, ProcessingNode *right,
	 ProcessingNode::Operator_t op);
  
  /**
   * Constructor for user binary op
   * @param[in] left  Pointer to the left side, now owned
   *                  by this object
   * @param[in] right  Pointer to the right side, now owned
   *                  by this object
   * @param[in] key  The keyword for this user binary op
   */
  BinaryNode(ProcessingNode *left, ProcessingNode *right,
	     const std::string &key);

  /**
   * Destructor
   */
  virtual ~BinaryNode();
  
  #include <rapmath/NodeVirtualMethods.hh>

  /**
   * @return true if this node has left and right subnodes
   * either leafs or binary nodes that satisfy isSimple()
   */
  bool isSimple(void) const;

  /**
   * Return the arguments for a simple binary args situation
   * @param[out] args
   * @return true if successful
   * NOTE this needs work for operator precidence
   */
  bool getSimpleArgs(BinaryArgs &args) const;

private:

  ProcessingNode *_left;   /**< Pinter ot left side */
  ProcessingNode *_right;  /**< Pointer to right side */
  std::string _userOpKey;  /**< Set if a user operation */
  ProcessingNode::Operator_t _op;  /**< The binary operator */

};

#endif
