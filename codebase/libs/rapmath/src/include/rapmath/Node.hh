/**
 * @file Node.hh
 * @brief  base class for all the MathParser nodes, sets the node pattern
 * @class Node
 * @brief  base class for all the MathParser nodes, sets the node pattern
 */

#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>

class LeafContent;
class VolumeData;
class MathData;
class MathUserData;
class MathLoopData;

class Node
{
public:

  typedef enum
  {
    SIMPLE_ASSIGN_MISSING_TO_VAR,
    SIMPLE_ASSIGN_NUMBER_TO_VAR,
    SIMPLE_ASSIGN_SIMPLE_BINARY_TO_VAR,
    SIMPLE_ASSIGN_VAR_TO_VAR,
    LOGICAL_SIMPLE_ASSIGN_NUMBER_TO_VAR,
    LOGICAL_SIMPLE_ASSIGN_VAR_TO_VAR,
    LOGICAL_MULTIPLE_SIMPLE_ASSIGN_NUMBER_TO_VAR,
    LOGICAL_MULTIPLE_SIMPLE_ASSIGN_VAR_TO_VAR,
    DO_IT_THE_HARD_WAY
  } Pattern_t;


  /**
   * Constructor
   */
  inline Node(void): _pattern(DO_IT_THE_HARD_WAY) {}

  /**
   * Destructor
   */
  inline virtual ~Node(void) {}

  /**
   * @return the pattern
   */
  inline Pattern_t pattern(void) const {return _pattern;}

#define NODE_BASE
   #include <rapmath/NodeVirtualMethods.hh>
#undef NODE_BASE

protected:
  
  Pattern_t _pattern;  /**< The pattern */

private:

};

#endif
