/**
 * @file CircularLookupHandler.hh 
 * @brief All the lookup tables for each gate index
 *        
 * @class CircularLookupHandler
 * @brief All the lookup tables for each gate index
 */

#ifndef CIRCULARLOOKUP_HANDLER_H
#define CIRCULARLOOKUP_HANDLER_H
#include "CircularLookup.hh"
#include <vector>
#include <Radx/RadxVol.hh>

//------------------------------------------------------------------
class CircularLookupHandler
{
public:

  /**
   * Constructor
   * @param[in] r  Radius (km) of lookup region
   * @param[in] vol  Data
   */
  CircularLookupHandler(double r, const RadxVol &vol);

  /**
   * Destructor
   */
  inline virtual ~CircularLookupHandler(void) {}

  /**
   * @return number of gates in the lookup
   */
  inline int nGates(void) const {return _ngates;}

  /**
   * @return reference to indexed indivdual lookup
   * @param[in] i
   */
  inline const CircularLookup & operator[](const std::size_t i) const
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
  std::vector<CircularLookup> _state;

  void _setState(const RadxVol &vol, const RadxSweep &s, int &ngates,
		 double &da, bool first);
  double _deltaAngleDegrees(const RadxVol &vol, const RadxSweep &s,
			    int &ngates) const;
};

#endif
