/**
 * @file LookupOffsets.hh
 * @brief One lookup table giving range and azimuth offsets
 * @class LookupOffsets
 * @brief One lookup table giving range and azimuth offsets
 *
 * Used for polar radar data circular template
 */
#ifndef LookupOffsets_h
#define LookupOffsets_h
#include <vector>
#include <string>

class LookupOffsets
{
public:
  /**
   * Empty constructor
   */
  inline LookupOffsets(void) {}

  /**
   * Main constructor
   * @param[in] xCenterIndex  Center index of lookup in X (Range)
   * @param[in] nx  Number of x (range) values in grid
   * @param[in] x0Km  Km distance from polar center point to 0th index 
   * @param[in] dxKm  Km distance between x gridpoints (range) 
   * @param[in] dyDeg  Degrees between azimumths in grid
   * @param[in] templateCircleRadiusKm  Radius of circular template (km)
   */
  LookupOffsets(int xCenterIndex, int nx, double x0Km, double dxKm,
		double dyDeg, double templateCircleRadiusKm);

  /**
   * Destructor
   */
  inline ~LookupOffsets(void) {}

  /**
   * Debug print of the index'th offsets
   * @param[in] index
   */
  void print(int index) const;

  /**
   * @return number of points in lookup
   */
  inline int num(void) const {return (int)(_offsets.size());}

  /**
   * @return gate index of i'th point in lookup
   * @param[in] i
   */
  inline int ithIndexR(const int i) const {return _offsets[i].first;}

  /**
   * @return azimuth offset index of i'th point in lookup
   * @param[in] i
   */
  inline int ithIndexA(const int i) const {return _offsets[i].second;}

private:
  /**
   * The lookup points, first = actual gate index, second = azimuth offset index
   */
  std::vector<std::pair<int,int> >  _offsets; 

  void _add(int x, double x0Km, double dxKm, double dyDeg,
	    double templateCircleRadiusKm);
};

#endif
