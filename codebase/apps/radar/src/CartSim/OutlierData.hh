// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  OutlierData.hh
 * @brief  Outlier data values that are distinct from surroundings
 * @class  OutlierData
 * @brief  Outlier data values that are distinct from surroundings
 */

# ifndef    OUTLIER_DATA_H
# define    OUTLIER_DATA_H

#include "Xyz.hh"
class Data;

//----------------------------------------------------------------
class OutlierData
{
public:

  /**
   * Default constructor
   */
  OutlierData(void);
  
  /**
   * Default constructor
   * @param[in] xvel_knots  velocity (knots)
   * @param[in] yvel_knots velocity (knots)
   * @param[in] zvel_knots velocity (knots)
   * @param[in] noise   
   * @param[in] dbz
   * @param[in] clutter
   */
  OutlierData(double xvel_knots, double yvel_knots, double zvel_knots,
	      double noise, double dbz, double clutter);
  
  /**
   * Destructor
   */
  virtual ~OutlierData(void);

  /**
   * Replace the velocity and reflectivity values with outlier values
   * scaled by noise and intensity.
   *
   * @param[in] intensity   Scale factor
   * @param[in,out] data
   */
  void setData(double intensity, Data &data) const;

protected:
private:  

  Xyz _vel;        /**< Velocity (m/s) */
  double  _noise;  /**< noise */
  double _dbz;     /**< reflecivity */
  double _clutter;
};

# endif 
