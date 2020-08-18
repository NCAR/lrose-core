/**
 * @file TemplateLookup1.hh
 * @brief One lookup table giving range and azimuth offsets for two sided offset boxes
 * @class TemplateLookup1
 * @brief One lookup table giving range and azimuth offsets for two sided offset boxes
 */
#ifndef TEMPLATE_LOOKUP_1_H
#define TEMPLATE_LOOKUP_1_H
#include <vector>
#include <string>

class TemplateLookup1
{
public:
  /**
   * Empty lookup for gates too close to radar
   */
  TemplateLookup1(void);

  /**
   * Constructor
   * @param[in] x  Template box size x (km)
   * @param[in] y  Template box size y (km)
   * @param[in] yOff  Offset from centerpoint to box y (km)
   * @param[in] xCenterIndex  center point index x (gate)
   * @param[in] ngates  Number of gates total
   * @param[in] startRangeKm  km to first gate
   * @param[in] deltaGateKm km between gates
   * @param[in] deltaAzDeg  degrees between azimuths
   */
  TemplateLookup1(double x, double y, double yOff,
		  int xCenterIndex, int ngates,
		  double startRangeKm, double deltaGateKm,  double deltaAzDeg);


  /**
   * @destructor
   */
  inline virtual ~TemplateLookup1 (void) {}

  /**
   * Debug print
   */
  void print(void) const;

  /**
   * @return debug print string
   */
  std::string sprint(void) const;
  
  /**
   * @return number of points in lookup, one side
   */
  inline int num1(void) const {return (int)(_offsets1.size());}

  /**
   * @return number of points in lookup, other side
   */
  inline int num2(void) const {return (int)(_offsets2.size());}

  /**
   * @return gate index of i'th point in lookup
   * @param[in] i
   */
  inline int ithIndex1R(const int i) const {return _offsets1[i].first;}

  /**
   * @return azimuth offset index of i'th point in lookup
   * @param[in] i
   */
  inline int ithIndex1A(const int i) const {return _offsets1[i].second;}

  /**
   * @return gate index of i'th point in lookup
   * @param[in] i
   */
  inline int ithIndex2R(const int i) const {return _offsets2[i].first;}

  /**
   * @return azimuth offset index of i'th point in lookup
   * @param[in] i
   */
  inline int ithIndex2A(const int i) const {return _offsets2[i].second;}


protected:
private:

  /**
   * The offsets, first = gate index, second = azimuth offset index, positive angles
   */
  std::vector<std::pair<int,int> >  _offsets1;

  /**
   * The offsets, first = gate index, second = azimuth offset index, negative angles
   */
  std::vector<std::pair<int,int> >  _offsets2;


  void _addToOffsets(int r, int ngates, double startRangeKm,
		     double deltaGateKm, double deltaAzDeg,
		     double yOff, double y, bool negative,
		     std::vector<std::pair<int,int> > &offsets);
};

#endif
