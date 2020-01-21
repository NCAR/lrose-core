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
 * @file CartesianSpeck.cc
 */

#include "CartesianSpeck.hh"
#include "SimMath.hh"
#include "Data.hh"
#include <cmath>

//----------------------------------------------------------------
CartesianSpeck::CartesianSpeck(const Params &P,
			       const Params::CartesianSpeck_t &mP) :
  Thing(P, -1, -1, mP.start_minutes, mP.lifetime_minutes),
  _center(mP.x_km, mP.y_km, mP.z_km),
  _len(mP.dx_km, mP.dy_km, mP.dz_km),
  _outlier(mP.xvel_knots, mP.yvel_knots, mP.zvel_knots, mP.noise, mP.dbz,
	   mP.clutter)
{
  
  _center.scale(KM_TO_METERS);
  _len.scale(KM_TO_METERS*0.5);  // 0.5 because of radius from length
}

//----------------------------------------------------------------
CartesianSpeck::~CartesianSpeck()
{
}

//----------------------------------------------------------------
void CartesianSpeck::_addToData(const Xyz &loc, Data &data) const
{
  // See if current x,y,z is in the speck.
  if (loc.inBox(_center, _len))
  {
    _outlier.setData(_intensity, data);
  }
}
