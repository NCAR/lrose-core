/**
 * @file TemplateLookupMgr.hh 
 * @brief All the lookup tables for each gate index
 *        
 * @class TemplateLookupMgr
 * @brief All the lookup tables for each gate index
 */

#ifndef TEMPLATE_HANDLER_H
#define TEMPLATE_HANDLER_H
#include "TemplateLookup.hh"
#include <rapmath/MathUserData.hh>
#include <vector>

class Volume;

//------------------------------------------------------------------
class TemplateLookupMgr : public MathUserData
{
public:

  /**
   * Constructor
   * @param[in] x  x width of box, km
   * @param[in] y  y width of box, km
   * @param[in] yoff  offset to the box in y (km)
   * @param[in] v  Volume to pull info out of
   */
  TemplateLookupMgr(double x, double y, double yoff, 
		    const Volume &v);

  /**
   * Destructor
   */
  inline virtual ~TemplateLookupMgr(void) {}

  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return number of gates in the lookup
   */
  inline int nGates(void) const {return _ngates;}

  /**
   * @return reference to indexed indivdual lookup
   * @param[in] i
   */
  inline const TemplateLookup & operator[](const std::size_t i) const
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

  /**
   * @return true if the inputs are the same as the template local values
   * @param[in] x  Box length x (km)
   * @param[in] y  Box length y (km)
   * @param[in] yoffset  Offset to the boxes in y
   */
  inline bool match(double x, double y, double yoffset) const
  {
    return x == _x && y == _y && yoffset == _yoff;
  }

protected:
private:

  double _x;             /**< Km box size x */
  double _y;             /**< Km box size y */
  double _yoff;          /**< Offset to box km (y) */

  int _ngates;            /**< Number of gates */
  double _startRangeKm;   /**< Range to starting gate */
  double _deltaGateKm;    /**< Gate spacing km */
  double _deltaAzDeg;     /**< Azimuth spacing degrees */

  /**
   * One lookup object per gate index
   */
  std::vector<TemplateLookup> _state;

};

#endif
