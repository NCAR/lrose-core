/**
 * @file LogicalNode.hh
 * @brief A Node at which there is a logical test and action
 * @class LogicalNode
 * @brief A Node at which there is a logical test and action
 *
 * Each such node is 'if (test) then action'
 */
#ifndef LOGICAL_NODE_H
#define LOGICAL_NODE_H

#include <rapmath/Node.hh>
#include <rapmath/Find.hh>

class LogicalArgs;

class LogicalNode : public Node
{
public:
  /**
   * Constructor
   * @param[in] find  The test, local copy made here
   * @param[in] action  The pointer to the action, now owned by this object
   *
   * @note  action must be an assignment 'A = x'
   */
  LogicalNode(const Find &find, ProcessingNode *action);

  /**
   * Destructor
   */
  virtual ~LogicalNode();

  #include <rapmath/NodeVirtualMethods.hh>
  
  /**
   * @return true if this is a simple comparison to a number that if true 
   * causes a simple assignment from a number or missing to a variable,
   * and if so set the args
   * @param[out]  compareName  What is being compared to
   * @param[out]  compareV     Value being compared to
   * @param[out]  compareMissing True if compareName is tested for missing
   * @param[out]  c  The comparison
   * @param[out]  assignName  Variable being assigned to when true
   * @param[out]  assignV    Value assigned to variable when true
   * @param[out]  assignMissing  True when Variable is set missing when true
   */
  bool getSimpleCompare(std::string &compareName, double &compareV,
			bool &compareMissing, MathFindSimple::Compare_t &c,
			std::string &assignName, double &assignV,
			bool &assignMissing) const;

  /**
   * @return true if this is a simple comparison to a number that if true 
   * causes a simple assignment from one variable to another,
   * and if so set the args
   * @param[out]  compareName  What is being compared to
   * @param[out]  compareV     Value being compared to
   * @param[out]  compareMissing True if compareName is tested for missing
   * @param[out]  c  The comparison
   * @param[out]  assignToName  Variable being assigned to when true
   * @param[out]  assignFromName  Variable being assigned from when true
   */
  bool getSimpleCompare(std::string &compareName, double &compareV,
			bool &compareMissing, MathFindSimple::Compare_t &c,
			std::string &assignToName,
			std::string &assignFromName) const;

  /**
   * @return true if this is a sequence of simple comparisons to make up a 
   * logical expression to a number that if true 
   * causes a simple assignmentment of a number to a variable.
   * and if so set the args
   *
   * @param[out]  args  The list of arguments/operations of the logical
   * @param[out]  assignName  Variable being assigned to when true
   * @param[out]  assignV Value being assigned to variable
   * @param[out]  assignMissing  TRue if missing value is being assigned to 
   *                             variable
   */
  bool getMultiCompare(LogicalArgs &args,
		       std::string &assignName,
		       double &assignV,
		       bool &assignMissing) const;

  /**
   * @return true if this is a sequence of simple comparisons to make up a 
   * logical expression to a number that if true 
   * causes a simple assignment of one variable to another
   * and if so set the args
   *
   * @param[out]  args  The list of arguments/operations of the logical
   * @param[out]  assignToName  Variable being assigned to when true
   * @param[out]  assignFromName Value being assigned from when true
   */
  bool getMultiCompare(LogicalArgs &args,
		       std::string &assignToName,
		       std::string &assignFromName) const;
private:

  Find _find;               /**< The test object */
  ProcessingNode *_action;  /**< Pointer to the assignment node */
};

#endif
