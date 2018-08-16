/**
 * @file MathUserData.hh 
 * @brief Container for abitrarily defined user data
 *        
 * @class MathUserData
 * @brief Container for abitrarily defined user data
 *        
 * Pure virtual, not much going on with this yet.
 */

#ifndef MATH_USER_DATA_H
#define MATH_USER_DATA_H

class MathUserData
{
public:
  /**
   * Empty constructor
   */
  inline MathUserData(void) {}

  /**
   * Destructor
   */
  inline virtual ~MathUserData(void) {}

#define MATH_USER_DATA_BASE
  #include <rapmath/MathUserDataVirtualMethods.hh>
#undef MATH_USER_DATA_BASE

protected:
private:

};

#endif
