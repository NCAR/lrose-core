/**
 * @file StatusUserData.hh 
 * @brief Simple derived class, base class is MathUserData, a single boolean 
 *        status
 *          
 * @class StatusUserData
 * @brief Simple derived class, base class is MathUserData, a single boolean
 *        status
 */

#ifndef STATUS_SPECIAL_DATA_H
#define STATUS_SPECIAL_DATA_H
#include <rapmath/MathUserData.hh>

//------------------------------------------------------------------
class StatusUserData : public MathUserData
{
public:

  /**
   * Constructor
   * @param[in] status to set
   */
  inline StatusUserData(bool status) : _status(status) {}

  /**
   * Destructor
   */
  inline virtual ~StatusUserData(void) {}

  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return true if _status is true
   */
  inline bool isTrue(void) const {return _status;}

protected:
private:

  bool _status;  /**< The status */
};

#endif
