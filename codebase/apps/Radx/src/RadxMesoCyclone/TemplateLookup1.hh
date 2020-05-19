/**
 * @file TemplateLookup1.hh
 * @brief One lookup table giving range and azimuth offsets
 * @class TemplateLookup1
 * @brief One lookup table giving range and azimuth offsets
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

  TemplateLookup1(int xCenterIndex, double x, int ngates,
		  double startRangeKm, double deltaGateKm,  double deltaAzDeg,
		  double yOff, double y);

  /**
   * @destructor
   */
  inline virtual ~TemplateLookup1 (void) {}

  // /**
  //  * @return number of points in lookup
  //  */
  // inline int num(void) const {return (int)_offsets1.size();}

  // /**
  //  * @return gate index of i'th point in lookup
  //  */
  // inline int ithIndex1R(const int i) const {return _offsets1[i].first;}

  // /**
  //  * @return azimuth offset index of i'th point in lookup
  //  */
  // inline int ithIndex1A(const int i) const {return _offsets1[i].second;}

  // /**
  //  * @return gate index of i'th point in lookup
  //  */
  // inline int ithIndex2R(const int i) const {return _offsets2[i].first;}

  // /**
  //  * @return azimuth offset index of i'th point in lookup
  //  */
  // inline int ithIndex2A(const int i) const {return _offsets2[i].second;}

  /**
   * Debug
   */
  void print(void) const;
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
   */
  inline int ithIndex1R(const int i) const {return _offsets1[i].first;}

  /**
   * @return azimuth offset index of i'th point in lookup
   */
  inline int ithIndex1A(const int i) const {return _offsets1[i].second;}

  /**
   * @return gate index of i'th point in lookup
   */
  inline int ithIndex2R(const int i) const {return _offsets2[i].first;}

  /**
   * @return azimuth offset index of i'th point in lookup
   */
  inline int ithIndex2A(const int i) const {return _offsets2[i].second;}


protected:
private:

  // int _centerIndexRNear;  /**< Center point gate index */
  // int _centerIndexRFar;  /**< Center point gate index */
  // int _centerIndexA;  /**< Center point azimuth index (always 0) */

  /**
   * The offsets, first = gate index, second = azimuth offset index
   */
  std::vector<std::pair<int,int> >  _offsets1;  // positive angles
  std::vector<std::pair<int,int> >  _offsets2;  // negative angles


  void _addToOffsets(int r, int ngates, double startRangeKm,
		     double deltaGateKm, double deltaAzDeg,
		     double yOff, double y, bool negative,
		     std::vector<std::pair<int,int> > &offsets);
};

#endif
