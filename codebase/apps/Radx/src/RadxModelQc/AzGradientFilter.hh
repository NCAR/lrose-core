/**
 * @file  AzGradientFilter.hh
 * @brief Azimuth gradient filter
 * @class AzGradientFilter
 * @brief Azimuth gradient filter
 */
#ifndef AZGRADIENTFILTER_H
#define AZGRADIENTFILTER_H

#include <string>
#include <vector>

class RayLoopData;
class AzGradientStateSpecialData;
class RadxRay;

class AzGradientFilter
{
public:
  /**
   * Constructor, no members
   */
  AzGradientFilter(void);

  /**
   * Destructor
   */
  ~AzGradientFilter(void);
  
  /**
   * Peform filter on inputs, output into last arg
   * @param[in] state  The state object
   * @param[in] v  Value to add to input data at each point prior to filtering
   * @param[in] name  Field name
   * @param[in] ray0  Pointer to ray before active ray
   * @param[in] ray1  Pointer to ray after active ray
   * @param[in] ray  Active ray
   * @param[in] data  The data that has been derived so far
   * @param[out] output  Pointer to output that is updated 
   * @return true if successful
   */
  bool filter(const AzGradientStateSpecialData &state, double v,
	      const std::string &name, const RadxRay *ray0,
	      const RadxRay *ray1, const RadxRay *ray,
	      const std::vector<RayLoopData> &data, RayLoopData *output);
    
private:
  const AzGradientStateSpecialData *_state;

  bool _set2Rays(const RadxRay *ray0, const RadxRay *ray1,
		 const RadxRay &ray, const RadxRay **lray0,
		 const RadxRay **lray1) const;
  bool _setPreviousRayWhenNewSweep(int oldSweep, int sweep,
				   const RadxRay **lray0,
				   const RadxRay **lray1) const;
  bool _setNextRayWhenEndSweep(int sweep, int nextSweep,
			       const RadxRay **lray0,
			       const RadxRay **lray1) const;
  bool _veryFirstRay(const RadxRay &ray, const RadxRay *ray1,
		     const RadxRay **lray0,
		     const RadxRay **lray1) const;
  bool _veryLastRay(const RadxRay *ray0, const RadxRay &ray,
		    const RadxRay **lray0,
		    const RadxRay **lray1) const;
};

#endif
