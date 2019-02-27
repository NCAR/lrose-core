/**
 * @file AssignmentNode.hh
 * @brief  Node that is of form A = b
 * @class AssignmentNode
 * @brief  Node that is of form A = b
 */

#ifndef ASSIGNMENT_NODE_HH
#define ASSIGNMENT_NODE_HH

#include <rapmath/Node.hh>
#include <rapmath/LeafContent.hh>

class ProcessingNode;
class BinaryArgs;

class AssignmentNode : public Node
{
public:
  /**
   * Constructor
   * @param[in] var  Variable being assigned to
   * @param[in] value  Pointer to value being assigned,
   *                   to be owned by this objet.
   */
  AssignmentNode(const LeafContent &var, ProcessingNode *value);

  /**
   * Destructor
   */
  virtual ~AssignmentNode(void);

  #include <rapmath/NodeVirtualMethods.hh>

  /**
   * @return true if the value computation is a user unary operation
   * @param[out] keyword  Keyword for user unary op when return is true
   * @param[in] warn  True to warn if false is returned, false to remain silent
   */
  bool isUnaryUserOpRightHandSide(std::string &keyword, bool warn=false) const;

  /**
   * @return true if this is a simple assignment Variable=number or 
   *         variable = missing
   * @param[out] name  Name of variable when true
   * @param[out] number Number when true and missing = false
   * @param[out] missing  True if variable is set to missing
   */
  bool getSimpleAssign(std::string &name, double &number, bool &missing) const;

  /**
   * @return true if this is a simple assignment Variable=variable2
   *
   * @param[out] from  Name of variable2 when true
   * @param[out] to Name of Variable when true
   */
  bool getSimpleAssign(std::string &from, std::string &to) const;

  /**
   * Set returned arg to the name of what is getting assigned to
   *
   * @param[out] name  
   * @return true if successful
   */
  bool getAssignedName(std::string &name) const;

  /**
   * Set returned args to the arguments when the assiged value is a
   * left to right simple binary arguments with each sub node a simple
   * variable or value (leaf node).
   *
   * NOTE: This needs work for precedence of operators
   *
   * @param[out] args
   * @return true if it is a simple binary arg situation
   */
  bool getSimpleBinaryArgs(BinaryArgs &args) const;
  
private:

  LeafContent _variable;                     /**< Variable assigned to */
  ProcessingNode *_assignedValue;            /**< Value assigned to variable */

  bool _processUserFunction(MathData *data) const;
  bool _processMultiArgUnaryFunction(MathLoopData *out, MathData *data) const;

};

#endif
