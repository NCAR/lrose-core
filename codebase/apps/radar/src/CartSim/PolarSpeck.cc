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
 * @file PolarSpeck.cc
 */

#include "PolarSpeck.hh"
#include "SimMath.hh"
#include "Data.hh"
#include <cmath>

//----------------------------------------------------------------
PolarSpeck::PolarSpeck(const Params &P, const Params::PolarSpeck_t &mP) :
  Thing(P, -1, -1, mP.start_minutes, mP.lifetime_minutes),
  _theta(mP.theta_degrees),
  _phi(mP.phi_degrees),
  _r(mP.r_km*KM_TO_METERS),
  _dtheta(mP.dtheta_degrees),
  _dphi(mP.dphi_degrees),
  _dr(mP.dr_km*KM_TO_METERS),
  _outlier(mP.xvel_knots, mP.yvel_knots, mP.zvel_knots, mP.noise, mP.dbz, 
	   mP.clutter)
{
  while (_theta < 0)
  {
    _theta += 360;
  }
  while (_theta >= 360)
  {
    _theta -= 360;
  }
}

//----------------------------------------------------------------
PolarSpeck::~PolarSpeck()
{
}

//----------------------------------------------------------------
void PolarSpeck::_addToData(const Xyz &loc,  Data &data) const
{
  double theta = loc.xyAngleDegrees0to360();
  double phi = loc.zAngleDegrees0to360();
  double r = loc.magnitude();
  if (theta >= _theta - _dtheta/2.0 && theta <= _theta + _dtheta/2.0 &&
      phi >= _phi - _dphi/2.0 && phi <= _phi + _dphi/2.0 &&
      r >= _r - _dr/2.0 && r <= _r + _dr/2.0)
  {
    _outlier.setData(_intensity, data);
  }
}
