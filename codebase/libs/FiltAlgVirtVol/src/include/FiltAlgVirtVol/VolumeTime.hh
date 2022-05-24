/**
 * @file VolumeTime.hh 
 * @brief lookup tables for each gate index
 *        
 * @class VolumeTime
 * @brief lookup tables for each gate index
 *
 * THis is MathUserData so it can be defined as a parameter
 * to be used in various filters
 */

#ifndef VolumeTime_h
#define VolumeTime_h

#include <rapmath/MathUserData.hh>
#include <ctime>

//------------------------------------------------------------------
class VolumeTime : public MathUserData
{
public:

  /**
   * Constructor with time passed in
   * @param[in] time
   */
  VolumeTime(const time_t &time);

  /**
   * Destructor
   */
  inline virtual ~VolumeTime (void) {}

  #include <rapmath/MathUserDataVirtualMethods.hh>

protected:
private:

  time_t _time;
};

#endif
