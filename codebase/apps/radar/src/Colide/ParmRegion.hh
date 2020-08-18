/**
 * @file ParmRegion.hh 
 * @brief Parms for Region filter
 * @class ParmRegion
 * @brief Parms for Region filter
 */

#ifndef PARM_REGION_H
#define PARM_REGION_H

//------------------------------------------------------------------
class ParmRegion
{
public:

  /**
   * Constructor
   */
  inline ParmRegion(void) {}

  /**
   * Destructor
   */
  inline virtual ~ParmRegion(void) {}

  double _min_mean_length;   /**< Min mean length of a region (pixels) */
  double _min_max_length;    /**< Minimum max length of a region (pixels) */
  double _min_min_length;    /**< Minimum min length of a region (pixels) */
  double _min_area;          /**< Minimum area of a region (pixels^2) */
  double _min_hot_area;      /**< Minimum hotspot area of a region (pixels^2) */
  double _hole_max_fill_area; /**< holes up to this size are filled in (pix^2)*/
  double _min_percent_for_circle;  /**< Percent test to see if reg is circle*/

protected:
private:

};

#endif
