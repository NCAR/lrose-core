/**
 * @file MathDataSimple.hh 
 * @brief Simple MathData derived class, empty everything
 * @class MathDataSimple
 * @brief Simple MathData derived class, empty everything
 *
 * A placeholder for when you want a simple class that does nothing
 * with virtual methods instantiated in the most simple way
 */

#ifndef MATH_DATA_SIMPLE_HH
#define MATH_DATA_SIMPLE_HH
#include <rapmath/MathData.hh>

//------------------------------------------------------------------
class MathDataSimple : public MathData
{
public:

  /**
   * Empty 
   */
  MathDataSimple(void);

  /**
   * Destructor
   */
  virtual ~MathDataSimple(void);

  #include <rapmath/MathDataVirtualMethods.hh>
  
protected:
private:

};

#endif
