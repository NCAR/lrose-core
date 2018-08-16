/**
 * @file  RayUnaryFilter.hh
 * @brief 
 * @class RayUnaryFilter
 * @brief 
 */
#ifndef RAYUNARYFILTER_H
#define RAYUNARYFILTER_H

#include <string>
#include <vector>

class RayLoopData;
class RayunaryStateSpecialData;
class RadxRay;
class MathData;
class RadxFuzzyF;

class RayUnaryFilter
{
public:
  /**
   * Constructor, no members
   */
  RayUnaryFilter(void);

  /**
   * Destructor
   */
  ~RayUnaryFilter(void);
  
  /**
   * Peform smoothing filter on inputs, output into last arg
   * @param[in] name  Field name
   * @param[in] nx
   * @param[in] ny
   * @param[in] ray  Active ray
   * @param[in] data  The data that has been derived so far
   * @param[out] output  Pointer to output that is updated 
   * @return true if successful
   */
  bool smooth(const std::string &name, int nx, int ny, const RadxRay *ray,
	      const std::vector<RayLoopData> &data, MathData *output);

  bool fuzzy(const std::string &name, const RadxFuzzyF &f, const RadxRay *ray,
	     const std::vector<RayLoopData> &data, MathData *output);
  
private:
};

#endif
