/**
 * @file PolarCircularTemplate.hh 
 * @brief lookup tables for each gate index
 *        
 * @class PolarCircularTemplate
 * @brief lookup tables for each gate index
 *
 * THis is MathUserData so it can be defined as a parameter
 * to be used in various filters
 */

#ifndef PolarCircularTemlate_h
#define PolarCircularTemlate_h
#include <FiltAlgVirtVol/LookupOffsets.hh>
#include <rapmath/MathUserData.hh>
#include <vector>

class MdvxProj;
class Grid2d;

//------------------------------------------------------------------
class PolarCircularTemplate : public MathUserData
{
public:

  /**
   * Constructor for a box of some radius
   * @param[in] radiusKm   The radius of the box (km)
   * @param[in] proj  The projection, which is used to figure out
   *                  which gridpoints go into the template at each point
   * @param[in] minRadiusKm  Minimum distance from radar to create a
   *                         template.  To prevent computational blowup
   *                         near the radar
   */
  PolarCircularTemplate(double radiusKm, const MdvxProj &proj,
			double minRadiusKm);

  /**
   * Destructor
   */
  inline virtual ~PolarCircularTemplate (void) {}

  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return number of gates in the lookup
   */
  inline int nGates(void) const {return _ngates;}

  /**
   * @return reference to indexed indivdual lookup
   * @param[in] i
   */
  inline const LookupOffsets & operator[](const std::size_t i) const
  {
    return _lookup[i];
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
   * @return true if input radius matches local one
   * @param[in] radiusKm
   */
  inline bool match(double radiusKm) const
  {
    return radiusKm == _radiusKm;
  }

  /**
   * At a given point, return the data values within the template at that
   * point
   * @param[in] x  The index to the point
   * @param[in] y  The index to the point
   * @param[in] data  The data grid
   * @return The values
   */
  std::vector<double> dataInsideCircle(int x, int y,
				       const Grid2d &data) const;

protected:
private:

  double _radiusKm;       /**< Circle radius, km */
  int _ngates;            /**< Number of gates */
  double _startRangeKm;   /**< Range to starting gate */
  double _deltaGateKm;    /**< Gate spacing km */
  double _deltaAzDeg;     /**< Azimuth spacing degrees */
  bool _circular;
  
  /**
   * One lookup object per gate index
   */
  std::vector<LookupOffsets> _lookup;

  std::vector<double> _dataInsideCircle(const LookupOffsets &lx, int y,
					const Grid2d &data) const;
};

#endif
