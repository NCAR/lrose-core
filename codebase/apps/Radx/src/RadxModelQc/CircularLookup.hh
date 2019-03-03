/**
 * @file CircularLookup.hh
 * @brief One lookup table giving range and azimuth offsets
 * @class CircularLookup
 * @brief One lookup table giving range and azimuth offsets
 */
#ifndef CIRCULAR_LOOKUP_H
#define CIRCULAR_LOOKUP_H
#include <vector>
#include <string>

class CircularLookup
{
public:
  /**
   * Empty lookup for gates too close to radar
   * @param[in] centerIndex  Index to this gate
   */
  CircularLookup(int centerIndex);

  /**
   * Nonempty lookup for gates away from radar
   * @param[in] circleRadius  Km radius of region
   * @param[in] centerR       Range (km) to the centerpoint
   * @param[in] centerIndex  Index to this gate
   * @param[in] ngates       Total number of gates
   * @param[in] startRangeKm  Km to first gate
   * @param[in] deltaGateKm   Km per gate
   * @param[in] deltaAzDeg    Degrees difference between azimuths
   * @param[in] 
   */
  CircularLookup(double circleRadius, double centerR, int centerIndex,
		 int ngates, double startRangeKm, double deltaGateKm,
		 double deltaAzDeg);

  /**
   * @destructor
   */
  virtual ~CircularLookup(void);

  /**
   * @return number of points in lookup
   */
  inline int num(void) const {return (int)_offsets.size();}

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

  /**
   * Debug
   */
  void print(void) const;
  
protected:
private:

  int _centerIndexR;  /**< Center point gate index */
  int _centerIndexA;  /**< Center point azimuth index (always 0) */

  /**
   * The offsets, first = gate index, second = azimuth offset index
   */
  std::vector<std::pair<int,int> >  _offsets;
};

#endif
