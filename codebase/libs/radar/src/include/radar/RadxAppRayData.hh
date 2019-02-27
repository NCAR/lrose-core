/**
 * @file RadxAppRayData.hh 
 * @brief Container for the processing of one ray. Has pointer to a ray, 
 *        all fields.
 * @class RadxAppRayData
 * @brief Container for the processing of one ray. Has pointer to a ray,
 *        all fields.
 */

#ifndef RADX_APP_RAY_DATA_H
#define RADX_APP_RAY_DATA_H

#include <radar/RadxAppRayLoopData.hh>
#include <rapmath/MathData.hh>
#include <rapmath/SpecialUserData.hh>
#include <vector>
#include <string>

class RadxAppVolume;
class RadxRay;

//------------------------------------------------------------------
class RadxAppRayData : public MathData
{
public:

  /**
   * Empty constructor
   */
  RadxAppRayData(void);

  /**
   * Set up for index'th ray in the RadxVol
   *
   * @param[in] r The volume object
   * @param[in] s  The special data from the volume
   * @param[in] index  Ray index
   */
  RadxAppRayData(const RadxAppVolume &r, const SpecialUserData &s,
		 int index);

  /**
   * Destructor
   */
  virtual ~RadxAppRayData(void);

  #include <rapmath/MathDataVirtualMethods.hh>
  
  #define RADXAPP_MATH_DATA_BASE
  #include <radar/RadxAppMathDataVirtualMethods.hh>
  #undef RADXAPP_MATH_DATA_BASE
  
  /**
   * @return pointer to the center ray
   */
  inline const RadxRay *rPtr(void) const {return _ray;}

  /**
   * @return a reference to the loop data derived fields for this ray
   */
  inline const
  std::vector<RadxAppRayLoopData> & dataRef(void) const {return _data;}

protected:
  RadxRay *_ray;             /**< Pointer to Center ray (main one ) */
  SpecialUserData _special;  /**< Special data, passed in from volume */

  /**
   * Each filter sets to produce pointer to its particular output
   */
  RadxAppRayLoopData *_outputRay;

private:

  /**
   * Derived data as filters are processed gets put here
   */
  std::vector<RadxAppRayLoopData> _data;

  /**
   * Each filter sets to produce pointers to its particular inputs
   */
  std::vector<RadxAppRayLoopData *> _inps;

  /**
   * Names of user derived inputs to a filter
   */
  SpecialUserData _specialInp;

  /**
   * pure virtual method, app implements this,
   * @return true if need to synchronize data for this operator
   * @param[in] userKey  The operator string
   */
  // virtual bool _needToSynch(const std::string &userKey) const = 0;


  RadxAppRayLoopData *_retrieveRay(const std::string &name,
				   bool showError) const;
  RadxAppRayLoopData *_refToData(const std::string &name,
				 bool suppressWarn=false);
  RadxAppRayLoopData *_exampleData(const std::string &name);
  void _updateRay(void);
  RadxAppRayLoopData *_match(const std::string &n);
  const RadxAppRayLoopData *_matchConst(const std::string &n) const;
  bool _synchInput(const std::string &name);
  void _setLocalInput(const std::string &input);


};

#endif
