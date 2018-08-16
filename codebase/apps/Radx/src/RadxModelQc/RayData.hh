/**
 * @file RayData.hh 
 * @brief Volume data handler for Radx data, contains entire volume
 *
 * @class RayData
 * @brief Volume data handler for Radx data, contains entire volume
 */

#ifndef RAY_DATA_H
#define RAY_DATA_H
#include "RayLoopData.hh"
#include <rapmath/VolumeData.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RayxData.hh>
#include <vector>
#include <string>

class MathUserData;
class CircularLookupHandler;
class RadxSweep;

//------------------------------------------------------------------
class RayData : public VolumeData
{
  friend class RayData1;
  friend class RayData2;

public:

  /**
   * Constructor, store inputs into local state
   *
   * @param[in] vol Pointer to a volume, pointer is stored internally,
   *            remains owned by caller
   *
   * @param[in] lookup  Lookup pointer, also owned by caller
   */
  RayData(const RadxVol *vol, const CircularLookupHandler *lookup);


  /**
   * Destructor
   */
  virtual ~RayData(void);

  /**
   * Virtual methods from base class
   */
  #include <rapmath/VolumeDataVirtualMethods.hh>


  /**
   * @return number of 2d looping items within the volume
   */
  int numSweeps(void) const;

  /**
   * @return number of 2d looping items within the volume
   */
  int numRays(void) const;


  static bool retrieveRay(const std::string &name, const RadxRay &ray,
                          const std::vector<RayLoopData> &data, RayxData &r,
			  bool showError=false);
  

  void trim(const std::vector<std::string> &outputKeep, bool outputAll);

protected:
private:

  const CircularLookupHandler *_lookup; /**< Lookup thing for 2dvar */
  const RadxVol *_vol;                  /**< Pointer to the radx volume */
  const std::vector<RadxRay *> &_rays;  /**< Pointer to each ray in vol */
  const vector<RadxSweep *> &_sweeps;   /**< Pointer to each sweep */
  RadxRay *_ray;                       /**< Pointer to current ray */

  /**
   * Names of derived fields
   */
  std::vector<std::string> _rayDataNames;

  /**
   * The names/pointers to special data fields, these are not in same
   * format as the RayData itself.
   */
  std::vector<std::string> _specialName;

  /**
   * Pointers to special data to go with each such field
   */
  std::vector<MathUserData *> _specialValue;

  bool _needToSynch(const std::string &userKey) const;
  MathUserData *_volumeAverage(const std::string &name) const;
  MathUserData *_volumeAzGradientState(void) const;
  bool _hasData(const std::string &userKey, const std::string &name,
		bool suppressWarn=false);
};

#endif
