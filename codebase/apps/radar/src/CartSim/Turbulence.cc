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
 * @file Turbulence.cc
 */

#include "Turbulence.hh"
#include "SimMath.hh"
#include "Data.hh"
#include <cmath>

//----------------------------------------------------------------
Turbulence::Turbulence(const Params &P, const Params::Turbulence_t &mP) :
  Thing(P, mP.minutes_to_intensity_fuzzy_index, mP.minutes_to_size_fuzzy_index,
	mP.start_minutes, mP.lifetime_minutes)
{
  _r = mP.radius_km*KM_TO_METERS;
  _loc = Xyz(mP.x_km, mP.y_km, mP.z_km);
  _loc.scale(KM_TO_METERS);
  _noise = mP.noise;
  _sw = mP.sw;
  _motion = Xyz(mP.xmotion_knots, mP.ymotion_knots, mP.zmotion_knots);
  _motion.scale(KNOTS_TO_MS);
}

//----------------------------------------------------------------
Turbulence::~Turbulence()
{
}

//----------------------------------------------------------------
void Turbulence::_addToData(const Xyz &loc, Data &data) const
{
  // get current center and vector from there to current loc, plus radius
  Xyz current0(_loc, _elapsedSeconds, _motion, true);
  Xyz v(loc, -1.0, current0, false);
  double r = v.magnitude();

  // adjust radius of region
  double value = _seconds2size.apply((double)_elapsedSeconds);
  double turb_r = _r*value;
  if (r <= turb_r)
  {
    double sw = _sw*_intensity;

    // make a max spectrum width
    data.setMaxSpectrumWidth(sw);

    double noise = _noise*_intensity;

    // add in noise
    data.addTurbNoise(noise);
  }  
}
