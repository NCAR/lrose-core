/**
 * @file RadxAppCircularLookupHandler.hh 
 * @brief All the lookup tables for each gate index
 */

#ifndef RADXAPP_CIRCULARLOOKUP_HANDLER_H
#define RADXAPP_CIRCULARLOOKUP_HANDLER_H

#include <vector>

class RadxVol;
class RadxSweep;
/*
 * @class CircularLookup
 * @brief One lookup table giving range and azimuth offsets
 */
class RadxAppCircularLookup
{
public:
  /**
   * Empty lookup for gates too close to radar
   * @param[in] centerIndex  Index to this gate
   */
  RadxAppCircularLookup(int centerIndex);

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
  RadxAppCircularLookup(double circleRadius, double centerR, int centerIndex,
			int ngates, double startRangeKm, double deltaGateKm,
			double deltaAzDeg);

  /**
   * @destructor
   */
  virtual ~RadxAppCircularLookup(void);

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

//------------------------------------------------------------------
/**
 * @class RadxAppCircularLookupHandler
 * @brief All the lookup tables for each gate index
 */
class RadxAppCircularLookupHandler
{
public:

  /**
   * Constructor
   * @param[in] r  Radius (km) of lookup region
   * @param[in] vol  Data
   */
  RadxAppCircularLookupHandler(double r, const RadxVol &vol);

  /**
   * Destructor
   */
  inline virtual ~RadxAppCircularLookupHandler(void) {}

  /**
   * @return number of gates in the lookup
   */
  inline int nGates(void) const {return _ngates;}

  /**
   * @return reference to indexed indivdual lookup
   * @param[in] i
   */
  inline const RadxAppCircularLookup & operator[](const std::size_t i) const
  {
    return _state[i];
  }

  /**
   * Debug
   */
  void print(void) const;

  /**
   * Debug
   */
  void printLookups(void) const;

protected:
private:

  int _ngates;            /**< Number of gates */
  double _startRangeKm;   /**< Range to starting gate */
  double _deltaGateKm;    /**< Gate spacing km */
  double _deltaAzDeg;     /**< Azimuth spacing degrees */

  /**
   * One lookup object per gate index
   */
  std::vector<RadxAppCircularLookup> _state;

  void _setState(const RadxVol &vol, const RadxSweep &s, int &ngates,
		 double &da, bool first);
  double _deltaAngleDegrees(const RadxVol &vol, const RadxSweep &s,
			    int &ngates) const;
};


#endif
