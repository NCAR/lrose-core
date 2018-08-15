/**
 * @file MathLoopData.hh 
 * @brief Container for one named data field within a MathData object
 * @class MathLoopData
 * @brief Container for one named data field within a MathData object
 *
 * Pure virtual base class, derived by the app
 */

#ifndef MATH_LOOP_DATA_H
#define MATH_LOOP_DATA_H
#include <vector>
#include <string>

class ProcessingNode;

//------------------------------------------------------------------
class MathLoopData 
{
public:

  /**
   * Empty constructor
   */
  inline MathLoopData(void) {}

  /**
   * Destructor
   */
  inline virtual ~MathLoopData(void) {}

#define MATH_LOOP_DATA_BASE
  #include <rapmath/MathLoopDataVirtualMethods.hh>
#undef MATH_LOOP_DATA_BASE

protected:
private:

};

#endif
