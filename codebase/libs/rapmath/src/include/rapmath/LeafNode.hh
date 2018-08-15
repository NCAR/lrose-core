/**
 * @file LeafNode.hh
 * @brief  A Node that is a leaf (value or variable)
 * @class LeafNode
 * @brief  A Node that is a leaf (value or variable)
 */
#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include <rapmath/Node.hh>
#include <rapmath/LeafContent.hh>

class LeafNode : public Node
{
public:
  /**
   * LeafNode variable
   * @param[in] s  Name of the variable, or PI,pi,missing
   */
  LeafNode(const std::string &s);

  /**
   * LeafNode value
   * @param[in] s  Name (string) for the value
   * @param[in] value  The value
   */
  LeafNode(const std::string &s, double value);

  /**
   * Destructor
   */
  virtual ~LeafNode();
  
  #include <rapmath/NodeVirtualMethods.hh>
  
  /**
   * @return true if object is a variable
   */
  bool isVariable(void) const;

  /**
   * @return true if leaf is a variable, and set arg to the variable name
   * @param[out] s  Name
   */
  bool getLeafVariableName(std::string &s) const;

  /**
   * @return true if leaf is not a variable, and set return args
   * @param[out] v  Value
   * @param[out] isMissing  set to true if the value is 'missing data'
   */
  bool getLeafNumberOrMissing(double &v, bool &isMissing) const;

  /**
   * @return the name from the local content
   */
  inline std::string getName(void) const {return _leafContent.getName();}

  /**
   * @return a copy of the local leaf content
   */
  inline LeafContent getLeafContent(void) const {return _leafContent;}

  /**
   * @return true if able to set fixed value arg at a point
   * @param[out] v  Value
   *
   * If not a variable, return the fixed value (unless missing, then return
   * false)
   *
   * If is a variable, return false
   * 
   */
  inline bool getValue(double &v) const {return _leafContent.getValue(v);}

private:

  LeafContent _leafContent;  /**< The contents at this leaf */
};

#endif
