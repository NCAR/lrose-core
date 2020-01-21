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
 * @file  VelCircle.hh
 * @brief  Sphere with outlier velocity values
 * @class  VelCircle
 * @brief  Sphere with outlier velocity values
 */

# ifndef    VEL_CIRCLE_H
# define    VEL_CIRCLE_H

#include "Params.hh"
#include "Thing.hh"
#include "Xyz.hh"
class Data;

//----------------------------------------------------------------
class VelCircle : public Thing
{
public:

  /**
   * Default constructor
   */
  VelCircle(const Params &P, const Params::VelCircle_t &mP);
  
  /**
   * Destructor
   */
  virtual ~VelCircle(void);

protected:

  virtual void _addToData(const Xyz &loc, Data &data) const;

private:  

  double _noise;    /**< Noise in the object */
  double _radius;   /**< radius meters of the object */
  Xyz _vel;         /**< Velocity within the object */
  Xyz _loc;         /**< Centerpoint of object */
  Xyz _motion;      /**< Motion vector (m/s) of object */
};

# endif 
