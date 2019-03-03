/**
 * @file AzGradientStateSpecialData.hh 
 * @brief State data created by a RayData volume filter
 * @class AzGradientStateSpecialData
 * @brief State data created by a RayData volume filter
 */

#ifndef AZ_GRADIENT_STATE_SPECIAL_DATA_H
#define AZ_GRADIENT_STATE_SPECIAL_DATA_H
#include "AzGradientState.hh"
#include <rapmath/MathUserData.hh>
#include <vector>

class RadxVol;
class RadxSweep;

//------------------------------------------------------------------
class AzGradientStateSpecialData : public MathUserData
{
public:

  /**
   * Construct from the volume input
   * @param[in] vol
   */
  AzGradientStateSpecialData(const RadxVol &vol);

  /**
   * Destructor
   */
  inline virtual ~AzGradientStateSpecialData(void) {}

#include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return number of AzGradientState objects
   */
  inline int nstate(void) const {return (int)_state.size();}

  /**
   * @return size of _state vector
   */
  inline size_t size(void) const {return _state.size();}

  /**
   * @return reference to i'th AzGradientState
   * @param[in] i
   */
  inline AzGradientState & operator[](size_t i) {return _state[i];}

  /**
   * @return reference to i'th AzGradientState
   * @param[in] i
   */
  inline const AzGradientState & operator[](size_t i) const {return _state[i];}

protected:
private:

  /**
   * One state object per sweep
   */
  std::vector<AzGradientState> _state;

  void _setState(const RadxVol &vol, const RadxSweep &s);
  double _deltaAngleDegrees(const RadxVol &vol,const RadxSweep &s,
			    double &angle0, double &angle1) const;
};

#endif
