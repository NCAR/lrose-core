/**
 * @file FloatUserData.hh 
 * @brief Simple derived class, base class is MathUserData, a single double
 * @class FloatUserData
 * @brief Simple derived class, base class is MathUserData, a single double
 */

#ifndef FLOAT_SPECIAL_DATA_H
#define FLOAT_SPECIAL_DATA_H
#include <rapmath/MathUserData.hh>

//------------------------------------------------------------------
class FloatUserData : public MathUserData
{
public:

  /**
   * Construct and set value to input
   * @param[in] v
   */
  inline FloatUserData(double v) : _value(v) {}

  /**
   * Destructor
   */
  inline virtual ~FloatUserData(void) {}

  #include <rapmath/MathUserDataVirtualMethods.hh>

protected:
private:

  double _value;   /**< The single value */
};

#endif
