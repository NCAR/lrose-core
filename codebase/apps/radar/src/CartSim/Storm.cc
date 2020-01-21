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
 * @file Storm.cc
 */

#include "Storm.hh"
#include "Data.hh"
#include "SimMath.hh"
#include "LineAndPoint.hh"
#include <cmath>

//----------------------------------------------------------------
Storm::Storm(const Params &P, const Params::Storm_t &mP) :
  Thing(P, mP.minutes_to_intensity_fuzzy_index,
	mP.minutes_to_size_fuzzy_index, mP.start_minutes, mP.lifetime_minutes)
{
  _override = mP.override;
  _noise = mP.noise;
  _r = mP.radius_km*KM_TO_METERS;
  _min_z = mP.min_z_km*KM_TO_METERS;
  _motion = Xyz(mP.xmotion_knots, mP.ymotion_knots, mP.zmotion_knots);
  _motion.scale(KNOTS_TO_MS);

  if (!Thing::endpoints(P, mP.endpoint_index, _loc))
  {
    exit(1);
  }
  _setFuzzyKmToInterest(P, mP.km_from_center_to_dbz_fuzzy_index, _dist2dbz);
}

//----------------------------------------------------------------
Storm::~Storm()
{
}

//----------------------------------------------------------------
void Storm::_addToData(const Xyz &loc,  Data &data) const
{
  // IF its below the minimum storm height, no storm
  if (loc._z < _min_z)
  {
    return;
  }

  if (_loc.size() == 1)
  {
    _addPoint(loc, data);
  }
  else
  {
    _addLines(loc, data);
  }
}

//----------------------------------------------------------------
void Storm::_addPoint(const Xyz &loc, Data &data) const
{
  double value = _singlePointValue(loc, _loc[0]);
  _add(value, data);
}

//----------------------------------------------------------------
void Storm::_addLines(const Xyz &loc, Data &data) const
{
  vector<LineAndPoint> info;
  // maximize reflectivity
  for (size_t i=0; i<_loc.size()-1; ++i)
  {
    LineAndPoint p;
    p.setLocs(Xyz(_loc[i], _elapsedSeconds, _motion, true),
	      Xyz(_loc[i+1], _elapsedSeconds, _motion, true));
    if (!p.pointLocation(loc))
    {
      return;
    }
    info.push_back(p);
  }

  bool first = true;
  double value = 0;
  double storm_r = _r*_seconds2size.apply((double)_elapsedSeconds);
  for (size_t i=0; i<info.size(); ++i)
  {
    if (info[i].getDistanceBeyondSide() > 0)
    {
      continue;
    }
    double d = fabs(info[i].getDistanceInFront());
    double dist = fabs(d - storm_r);
    double valuei = _dist2dbz.apply(dist);
    if (first)
    {
      value = valuei;
      first = false;
    }
    else
    {
      if (valuei > value)
      {
	value = valuei;
      }
    }
  }


  // do all centerpoints as spheres to get that stuff off edge
  for (size_t i=0; i<_loc.size(); ++i)
  {
    double valuei = _singlePointValue(loc, _loc[i]);
    if (valuei > value)
    {
      value = valuei;
    }
  }
  _add(value, data);
}

//----------------------------------------------------------------
double Storm::_singlePointValue(const Xyz &loc, const Xyz &loc0) const
{
  // determine current center and vector from there to
  // current loc, plus the radius.
  Xyz current0(loc0, _elapsedSeconds, _motion, true);
  Xyz v(loc, -1.0, current0, false);
  double r = v.magnitude();
  
  // Adjust the radius of the storm
  double value = _seconds2size.apply((double)_elapsedSeconds);
  double storm_r = _r*value;
    
  // Now determine distance from the center point.
  double dist = r - storm_r;
  if (dist <= 0)
  {
    dist = 0.0;
  }
  
  // Apply the mapping to get out a dbz value
  return _dist2dbz.apply(dist);
}


//----------------------------------------------------------------
void Storm::_add(double value, Data &data) const
{
  if (_noise > 0.0)
  {
    value = RANDOMF(_noise)*value*_intensity;
  }
  else
  {
    value = value*_intensity;
  }

  if (_override)
  {
    data.setDbz(value);
  }
  else
  {
    data.setMaxDbz(value);
  }
}
