/**
 * @file TemplateLookup.hh
 * @brief One lookup table giving range and azimuth offsets
 * @class TemplateLookup
 * @brief One lookup table giving range and azimuth offsets
 */
#ifndef TEMPLATE_LOOKUP_H
#define TEMPLATE_LOOKUP_H
#include "TemplateLookup1.hh"
#include <vector>
#include <string>

class TemplateLookup
{
public:
  /**
   * Empty lookup for gates too close to radar
   * @param[in] centerIndex  Index to this gate
   */
  TemplateLookup(int centerIndex);

  /**
   * Nonempty lookup for gates away from radar
   * @param[in] x             length (x, km) of template
   * @param[in] y             width (y, km, need to convert from degrees)
   * @param[in] yOff          distance (y, km to template on each side)
   * @param[in] centerR       Range (km) to the centerpoint
   * @param[in] centerIndex   Index to this gate
   * @param[in] ngates        Total number of gates
   * @param[in] startRangeKm  Km to first gate
   * @param[in] deltaGateKm   Km per gate
   * @param[in] deltaAzDeg    Degrees difference between azimuths
   */
 TemplateLookup(double x, double y, double yOff, double centerR,
		 int centerIndex,
		 int ngates, double startRangeKm, double deltaGateKm,
		 double deltaAzDeg);

  /**
   * @destructor
   */
  virtual ~TemplateLookup(void);

  /**
   * @return number of points in lookup, one side
   */
  inline int num1(void) const {return _points.num1();}

  /**
   * @return number of points in lookup, other side
   */
  inline int num2(void) const {return _points.num2();}

  /**
   * @return gate index of i'th point in lookup
   */
  inline int ithIndex1R(const int i) const {return _points.ithIndex1R(i);}

  /**
   * @return azimuth offset index of i'th point in lookup
   */
  inline int ithIndex1A(const int i) const {return _points.ithIndex1A(i);}

  /**
   * @return gate index of i'th point in lookup
   */
  inline int ithIndex2R(const int i) const {return _points.ithIndex2R(i);}

  /**
   * @return azimuth offset index of i'th point in lookup
   */
  inline int ithIndex2A(const int i) const {return _points.ithIndex2A(i);}

  /**
   * Debug
   */
  void print(void) const;
  
protected:
private:

  int _centerIndex;  /**< Center point gate index */
  int _centerIndexA;  /**< Center point azimuth index (always 0) */

  TemplateLookup1 _points;
};

#endif
