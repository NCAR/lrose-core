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
 * @file DeviantRay.cc
 */

#include "DeviantRay.hh"
#include "SimMath.hh"
#include "Data.hh"
#include <cmath>

//----------------------------------------------------------------
DeviantRay::DeviantRay(const Params &P, const Params::DeviantRay_t &mP) :
  Thing(P, -1, -1, mP.start_minutes, mP.lifetime_minutes),
  _angle0(mP.angle0_degrees),
  _angle1(mP.angle1_degrees),
  _outlier(mP.xvel_knots, mP.yvel_knots, mP.zvel_knots, mP.noise, mP.dbz,
	   mP.clutter)
{
}

//----------------------------------------------------------------
DeviantRay::~DeviantRay()
{
}

//----------------------------------------------------------------
void DeviantRay::_addToData(const Xyz &loc, Data &data) const
{
  double theta = loc.xyAngleDegrees0to360();
  if (theta >= _angle0 && theta <= _angle1)
  {
    _outlier.setData(_intensity, data);
  }
}
