/**
 * @file ProcessingNode.hh
 * @brief  One node which can have sub nodes (recursively)
 * @class ProcessingNode
 * @brief  One node which can have sub nodes (recursively)
 */

#ifndef PROCESSING_NODE_H
#define PROCESSING_NODE_H

#include <rapmath/Node.hh>
#include <rapmath/MathFindSimple.hh>


class FunctionDef;
class Find;
class BinaryArgs;
class MathLoopData;
class LogicalArgs;

class ProcessingNode
{
public:

  /**
   * Types of nodes
   * LEAF  = bottom node with variables and values
   * BINARY = node which is a binary operation on two subnodes
   * UNARY = node which is a unary operation, with any number of 
   *         subnodes (depends on operation)
   * LOGICAL = node which has a left subnode test for true false 
   *           and a right subnode assignment
   * ASSIGNMENT = node where left side is a variable and right is
   *              a node that returns a value
   */
  typedef enum {LEAF, BINARY, UNARY, LOGICAL, ASSIGNMENT, NOT_SET} Type_t;

  /**
   * Binary operations hardwired
   * Should be self evident
   */
  typedef enum {ADD, SUB, MULT, DIV, POW, USER, BAD} Operator_t;

  /**
   * Unary operations hardwired
   * See comments in source code for meaning of some of these
   */
  typedef enum {ABS, SQRT, LOG10, EXP, SMOOTH, SMOOTHDBZ, STDDEV, FUZZY,
		AVERAGE, WEIGHTED_AVERAGE, MASK, MASK_MISSING_TO_MISSING,
		TRAPEZOID, S_REMAP, UUSER, MAX, MEDIAN, WEIGHTED_ANGLE_AVERAGE,
		MAX_EXPAND, EXPAND_ANGLES_LATERALLY, CLUMP,
		UBAD} UnaryOperator_t;

  /**
   * Empty
   */
  ProcessingNode(void);

  /**
   * Constructor, Leaf variable
   * @param[in] s  Name of a variable
   */
  ProcessingNode(const std::string &s);

  /**
   * Constructor, Leaf value
   * @param[in] s  String for value
   * @param[in] value  The value
   */
  ProcessingNode(const std::string &s, double value);

  /**
   * Constructor, Binary operation node
   * @param[in] input  The full string
   * @param[in] left  Pointer to left half of binary operation 
   * @param[in] right Pointer to right half of binary operation
   * @param[in] op  The binary operation
   */
  ProcessingNode(const std::string &input, ProcessingNode *left,
		 ProcessingNode *right, Operator_t op);

  /**
   * Constructor, User defined binary operation node
   * @param[in] input  The full string
   * @param[in] left  Pointer to left half of binary operation 
   * @param[in] right Pointer to right half of binary operation
   * @param[in] key  User name for this binary operation
   */
  ProcessingNode(const std::string &input, ProcessingNode *left,
		 ProcessingNode *right, const std::string &key);

  /**
   * Constructor, Unary operation node
   * @param[in] input  The full string
   * @param[in] op  The unary operation
   * @param[in] args  The arguments for this unary operation, in order
   */
  ProcessingNode(const std::string &input, UnaryOperator_t op,
		 std::vector<ProcessingNode *> &args);

  /**
   * Constructor, User defined Unary operation node
   * @param[in] input  The full string
   * @param[in] args  The arguments for this unary operation, in order
   * @param[in] key  User name for this unary operation
   */
  ProcessingNode(const std::string &input, std::vector<ProcessingNode *> &args,
		 const std::string &key);

  /**
   * Constructor, Logical 'if then' node
   *
   * @param[in] input  Full string
   * @param[in] find  The logical stuff
   * @param[in] value  The sub node to execute when find returns true
   */
  ProcessingNode(const std::string &input, const Find &find,
		 ProcessingNode *value);

  /**
   * Constructor, Assignment node
   * @param[in] input  Full string
   * @param[in] var  The variable to assign to
   * @param[in] value  The value to pull from
   */
  ProcessingNode(const std::string &input,
		 const LeafContent &var, ProcessingNode *value);

  /**
   * Destructor
   */
  ~ProcessingNode(void);

  /**
   * @return the node's pattern type
   */
  inline Node::Pattern_t pattern(void) const {return _pattern;}

  /**
   * Cleanup sub steps, and delete pointers
   */
  void cleanup(void);
  
  /**
   * Simple debug printing of the string for this node
   * @return string representation
   */
  std::string sprint(void) const;

  /**
   * Simple debug printing to stdout of the string for this node
   */
  void print(void) const;

  /**
   * Simple debug printing of the parsed node
   */
  void printParsed(void) const;

  /**
   * Simple debug printing of the parsed node, with a \n
   */
  void printParsedCr(void) const;

  /**
   * @return string for an operator
   * @param[in] op
   */
  static std::string sprintOp(const Operator_t &op);

  /**
   * @return string for an operators description
   * @param[in] op
   */
  static std::string sprintOpDescr(const Operator_t &op);

  /**
   * @return string for a unary operator
   * @param[in] op
   */
  static std::string sprintUOp(const UnaryOperator_t &op);

  /**
   * @return string for a unary operator's description
   * @param[in] op
   */
  static std::string sprintUOpDescr(const UnaryOperator_t &op);

  /**
   * @return Binary operator associated with an index
   * @param[in] index  Index
   */
  static Operator_t binaryOperatorValue(int index);

  /**
   * @return Unary operator associated with an index
   * @param[in] index  Index
   */
  static UnaryOperator_t unaryOperatorValue(int index);

  /**
   * @return info for all fixed unary operators
   */
  static std::vector<FunctionDef> unaryOperators(void);

  /**
   * @return info for all fixed binary operators
   */
  static std::vector<FunctionDef> binaryOperators(void);

  /**
   * Process the input, and update the data state.
   *
   * @param[in,out] data  Data to read/write
   *
   * @return true if able to do everything
   */
  bool process(MathData *data) const;

  /**
   * Process the input, update data state, return pointer to user defined data
   *
   * @param[in,out] data  Data to read/write
   *
   * @return Pointer to user defined data created by the filter
   */
  MathUserData *processToUserDefined(MathData *data) const;

  /**
   * Process the input, assumed for the entire volume, update data state
   * @param[in,out] data  Data to read/write
   *
   * @return Pointer to user defined data created by the filter
   */
  MathUserData *processVol(VolumeData *data) const;

  /**
   * perform computation at this node to compute a value
   *
   * @param[in] data  Pointer to the data
   * @param[in] ind  Index into data
   * @param[out] v  Computed value
   *
   * @return true for v was set, false for could not compute
   */
  bool compute(const MathData *data, int ind, double &v) const;

  /**
   * Append all variable fields not on left hand side of an
   * assignment to the input vector, these are the inputs for this node
   *
   * @param[in,out] names
   */
  void inputFields(std::vector<std::string> &names) const;


  /**
   * Append all variable fields that are on left hand side of an
   * assignment to the input vector, these are the outputs for this node
   *
   * @param[in,out] names
   */
  void outputFields(std::vector<std::string> &names) const;
  
  /**
   * @return True if this node is a user defined unary operator
   */
  bool isUserUnaryFunction(void) const;

  /**
   * @return true if this node is a multi arg fixed unary operator
   */
  bool isMultiArgUnaryFunction(void) const;
  
  /**
   * @return True if this node is an assignment node
   */
  inline bool isAssignment(void) const {return _type == ASSIGNMENT;}

  /**
   * @return true if this node is a leaf variable
   */
  bool isVariable(void) const;

  /**
   * @return a copy of the leaf content at this node, assumed a leaf
   * node, if not a leaf node return an empty object
   */
  LeafContent getLeafContent(void) const;

  /**
   * @return true if this node is a user defined unary operation
   * on the right hand side, being assigned to the left hand side
   *
   * @param[out] keyword  The user keyword when true
   * @param[in] warn True to warn if not a user unary op, false to stay silent
   */
  bool isUserAssignmentWithUnaryOp(std::string &keyword,
				   bool warn=false) const;

  /**
   * @return true if this node is a user defined unary operation
   *
   * @param[out] keyword  The user's keyword when true
   * @param[in] warn True to warn if not a user unary op, false to stay silent
   */
  bool isUserUnaryOp(std::string &keyword,  bool warn=false) const;

  /**
   * @return pointers the arguments to a unary operation (zero or more)
   * If not a unary operation, return an empty vector
   */
  std::vector<ProcessingNode *> *unaryOpArgs(void);

  /**
   * If the node is a leaf with fixed value, return that value in the arg
   *
   * @param[out] v
   * @return true if v was set
   */
  bool getValue(double &v) const;

  /**
   * @return the name of the leaf it is a leaf node, or empty string if not
   */
  std::string leafName(void) const;

  /**
   * @return true if the node is a leaf node with a variable
   *
   * @param[out] s Name of variable when true
   */
  bool getLeafVariableName(std::string &s) const;

  /**
   * @return true if the node is a leaf node with a number or a leaf node
   * for missing
   *
   * @param[out] v  The number when true and not missing
   * @param[out] isMissing  True if the node is the missing data value
   */
  bool getLeafNumberOrMissing(double &v, bool &isMissing) const;

  /**
   * @return true if the node is a binary with both left and right sides
   * simple leaf nodes
   */
  bool isSimpleBinary(void) const;

  /**
   * @return true if the node is an assignment of form name = number
   *  or name = missing
   *
   * @param[out] name    Name
   * @param[out] number  Number (if not missing)
   * @param[out] missing  true if missing
   */
  bool getSimpleAssign(std::string &name, double &number, bool &missing) const;

  /**
   * @return true if the node is an assignment of form to = from
   *
   * @param[out] from  Name of 'from' variable
   * @param[out] to    Name of 'to' variable
   */
  bool getSimpleAssign(std::string &from, std::string &to) const;

  /**
   * @return true if the node is an 'if then' node with a simple
   * test and a simple assignment to a variable when true.
   * Simple test is: name <compare> value  or name <compare> missing.
   * Simple assign is name=value or name=missing
   *
   * Args set when true is returned, for 'if'
   * If compareMissing
   *    compareName == missing
   * else
   *    compareName c compareV
   *
   * Args set when true is returned for action
   * if assignMissing
   *    assignName = missing
   * else
   *    assignName = assignV
   *
   * @param[out] compareName 
   * @param[out] compareV
   * @param[out] compareMissing
   * @param[out] c
   * @param[out] assignName
   * @param[out] assignV
   * @param[out] assignMissing
   */
  bool getSimpleCompare(std::string &compareName, double &compareV,
			bool &compareMissing, MathFindSimple::Compare_t &c,
			std::string &assignName, double &assignV,
			bool &assignMissing) const;

  /**
   * @return true if the node is an 'if then' node with a simple
   * test and a simple assignment from one var to another when true.
   * Simple test is: name <compare> value  or name <compare> missing.
   * Simple assign is variable1 = variable2
   *
   * Args set when true is returned, for 'if'
   * If compareMissing
   *    compareName == missing
   * else
   *    compareName c compareV
   *
   * Args set when true, for the assignment
   *    assignToName = assignFromName
   *
   * @param[out] compareName 
   * @param[out] compareV
   * @param[out] compareMissing
   * @param[out] c
   * @param[out] assignToName
   * @param[out] assignFromName
   */
  bool getSimpleCompare(std::string &compareName, double &compareV,
			bool &compareMissing, 	MathFindSimple::Compare_t &c,
			std::string &assignToName,
			std::string &assignFromName) const;


  /**
   * @return true if the node is an 'if then' node with a sequence
   * of simple comparisons with and/or's (no parens), and 
   * 
   * a simple assignment to a value when true.
   *
   * @param[out] args  Description of the logicals
   * @param[out] assignName Name to assign to 
   * @param[out] assignV  Value to assign
   * @param[out] assignMissing  true to set assignName to missing
   */
  bool getMultiCompare(LogicalArgs &args, std::string &assignName,
		       double &assignV, bool &assignMissing) const;

  /**
   * @return true if the node is an 'if then' node with a sequence
   * of simple comparisons with and/or's (no parens), and 
   * 
   * a simple assignment to a value when true.
   *
   * @param[out] args  Description of the logicals
   * @param[out] assignToName  variable to assign to
   * @param[out] assignFromName variable to get from
   */
  bool getMultiCompare(LogicalArgs &args, std::string &assignToName,
		       std::string &assignFromName) const;
  
  /**
   * @return true if the node is an assignment
   *
   * @param[out] name Name of assignment variable (when return is true)
   */
  bool getAssignName(std::string &name) const;

  /**
   * @return true if the node is an assignment, with right hand side
   * any number of simple binary operations
   *
   * @param[out] args  The binary operations specification
   *
   * @note this method does not quite work, just goes left to right,
   * no handling of operator order, and parens
   */
  bool getAssignSimpleBinaryArgs(BinaryArgs &args) const;
  
  /**
   * @return the original full string for this node
   */
  inline std::string getInput(void) const {return _input;}

  /**
   * @return the type of node
   */
  inline Type_t getType(void) const {return _type;}

  /**
   * @return a pointer to the Node associated with this object
   */
  inline const Node *nodePtr(void) const {return _content;}

  /**
   * @return a pointer to the Node associated with this object
   */
  inline Node *nodePtr(void) {return _content;}

  /**
   * @return the unary operator for this node, or UBAD
   */
  UnaryOperator_t getUnaryOperator(void) const;
  
private:

  Type_t _type;             /**< Type of node */
  Node::Pattern_t _pattern; /**< Pattern detected at this node */
  Node *_content;           /**< Pointer to the Node that goes with the _type */
  std::string _input;       /**< Input string parsed to create the node */
};

#endif
