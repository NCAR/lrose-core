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
 * @file VelCircle.cc
 */

#include "VelCircle.hh"
#include "SimMath.hh"
#include "Data.hh"
#include <cmath>

//----------------------------------------------------------------
VelCircle::VelCircle(const Params &P, const Params::VelCircle_t &mP) :
  Thing(P, -1, -1, mP.start_minutes, mP.lifetime_minutes)
{
  _noise = mP.noise;
  _radius = mP.radius_km*KM_TO_METERS;

  _vel = Xyz(mP.xvel_knots, mP.yvel_knots, mP.zvel_knots);
  _vel.scale(KNOTS_TO_MS);

  _loc = Xyz(mP.x_km, mP.y_km, mP.z_km);
  _loc.scale(KM_TO_METERS);

  _motion = Xyz(mP.xmotion_knots, mP.ymotion_knots, mP.zmotion_knots);
  _motion.scale(KNOTS_TO_MS);
}

//----------------------------------------------------------------
VelCircle::~VelCircle()
{
}

//----------------------------------------------------------------
void VelCircle::_addToData(const Xyz &loc, Data &data) const
{
  // determine current center and vector from there to
  // current loc, plus the radius.
  Xyz current0(_loc, _elapsedSeconds, _motion, true);
  Xyz v(loc, -1, current0, false);
  double r = v.magnitude();

  // Is the distance small enough?
  if (r <= _radius)
  {
    double n;
    if (_noise > 0.0)
      n = RANDOMF(_noise);
    else
      n = 1.0;

    // Replace the velocity with the value*intensity*noise
    data.setVel(Xyz(_vel, _intensity*n));
  }
}
