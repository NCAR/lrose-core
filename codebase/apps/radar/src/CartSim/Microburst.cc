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
 * @file Microburst.cc
 */

#include "Microburst.hh"
#include "LineAndPoint.hh"
#include "SimMath.hh"
#include "Data.hh"
#include <cmath>


//----------------------------------------------------------------
Microburst::Microburst(const Params &P, const Params::Microburst_t &mP) :
  Thing(P, mP.minutes_to_intensity_fuzzy_index, mP.minutes_to_size_fuzzy_index,
	mP.start_minutes, mP.lifetime_minutes)
{
  // Divide by two to convert from peak outflow to expected
  // loss estimate, i.e. peak outflow of mag/2 leads to a loss
  // of mag.  The loss magnitude is what is in config file.
  _mag = mP.magnitude_knots*KNOTS_TO_MS/2.0;

  // Divide radius of mb by 2 to store the radius to peak outflow,
  // which is defined as 1/2 the microburst "radius" (hazard region)
  _r = mP.radius_km*KM_TO_METERS/2.0;
  _z = mP.zmax_km*KM_TO_METERS;
  _core_dbz = mP.dbz_core;

  if (!Thing::endpoints(P, mP.endpoint_index, _loc))
  {
    exit(1);
  }
  switch (_loc.size())
  {
  case 1:
    _line = false;
    break;
  case 2:
    _line = true;
    break;
  default:
    printf("ERROR %d points, want 1 or 2", (int)_loc.size());
    exit(1);
  }
  _gain = mP.gain;
  _motion = Xyz(mP.xmotion_knots, mP.ymotion_knots, 0);
  _motion.scale(KNOTS_TO_MS);

  _setFuzzy(P.utilda_n, P._utilda, _Utilda);
  _setFuzzy(P.stilda_n, P._stilda, _Stilda);
  _setFuzzy(P.wtilda_n, P._wtilda, _Wtilda);
  _setFuzzy(P.wrmax_n, P._wrmax, _Wrmax);
}

//----------------------------------------------------------------
Microburst::~Microburst()
{
}

//----------------------------------------------------------------
void Microburst::_addToData(const Xyz &loc, Data &data) const
{
  if (_line)
  {
    _mbLine(loc, data);
  }
  else
  {
    _mbPoint(loc, data);
  }
}

//----------------------------------------------------------------
// add to data for microburst line
void Microburst::_mbLine(const Xyz &loc, Data &data) const
{
  double r, z;
  Xyz wind;
  
  if (_windLine(loc, r, z, wind))
  {
    _add(wind, r, z, data);
  }
}

//----------------------------------------------------------------
// add to data for microburst point
void Microburst::_mbPoint(const Xyz &loc, Data &data) const
{
  double r, z;
  Xyz wind;
  if (_windPoint(loc, r, z, wind))
  {
    _add(wind, r, z, data);
  }
}

//----------------------------------------------------------------
// get r, z, and wind for microbust line
bool Microburst::_windLine(const Xyz &loc, double &r, double &z, 
			   Xyz &wind) const
{
  LineAndPoint info;
  // set locations into info
  info.setLocs(Xyz(_loc[0], _elapsedSeconds, _motion, true),
	       Xyz(_loc[1], _elapsedSeconds, _motion, true));
  // set point into info
  if (!info.pointLocation(loc))
  {
    return false;
  }
  if (info.getDistanceBeyondSide() > 0.0)
  {
    // if the point is off to the side, do nothing
    return false;
  }

  r = fabs(info.getDistanceInFront());
  z = loc._z;

  double vert, horiz;
  if (!_windMag(r, z, vert, horiz))
  {
    // no winds
    return false;
  }
    
  if (!_lineWindDirection(info, wind))
  {
    return false;
  }
       
  // Set the horizontal and vertical winds.
  wind.scale(horiz, horiz, vert);
  return true;
}

//----------------------------------------------------------------
// get r, z, and wind for microbust point
bool Microburst::_windPoint(const Xyz &loc, double &r, double &z,
			    Xyz &wind) const
{
  Xyz current0(_loc[0], _elapsedSeconds, _motion, true);
  Xyz v = Xyz(loc, -1.0, current0, false);
  r = sqrt(v._x*v._x + v._y*v._y);
  z = v._z;
  
  double vert, horiz;
  if (!_windMag(r, z, vert, horiz))
  {
    return false;
  }
  wind = _wind(v, r, vert, horiz);
  return true;
}  

//----------------------------------------------------------------
// add winds and thinline for particular wind, r, z
void Microburst::_add(const Xyz &wind, double r, double z, Data &data) const
{
  // Add to current values, scale this down by intensity..
  data.setVel(Xyz(data.getVel(), _intensity, wind, false));

  // Compute the reflectivity at the location, scaled down by intensity
  // and a scale factor
  double rscale2 = _radialCoreScale(r, z, 5.0, _Wrmax);
  double dbz = rscale2*_core_dbz*_intensity;

  // Add in as a new maximum if appropriate.
  data.setMaxDbz(dbz);
}

//----------------------------------------------------------------
// compute horizontal and vertical wind components using mb model
// r = distance out, z = distance up
bool Microburst::_windMag(double r, double z, double &vert, double &horiz) const
{
  double rscale = _radialCoreScale(r, z, 1.0, _Wrmax);
  double size_scale = _seconds2size.apply((double)_elapsedSeconds);
  horiz = _windU( r, z, size_scale);
  vert = _windZ(r, z, rscale, size_scale);
  if (_gain)
  {
    horiz = -horiz;
    vert = -vert;
  }
  return (fabs(vert) > 1.0e-6 && fabs(horiz) > 1.0e-6);
}

//----------------------------------------------------------------
double Microburst::_radialCoreScale(double r, double z, double scale,
				    const FuzzyF &mapping) const
{
  double rtilda, ztilda, b, c, alpha;

  // It's defined as per the notes, in terms of an exponential
  // drop off outside 1/2 the maximum (core) radius
  if (fabs(r) < 1.0e-6)
  {
    // Near the center...you get the maximum reflectivity at that height
    c = 1.0;
  }
  else
  {
    rtilda = r/_r;
    ztilda = z/_z;
    if (rtilda > 0.5)
    {
      // Outside the descending core..it drops off exponentially
      // Such that it is 0.1 maximum at a distance defined by
      // b (in terms of the core, its b - 0.5 away from the core).
      // Determine radius b at which exponential drops to 0.1
      // (depends on height)
      b = mapping.apply(ztilda);
      b = b*scale;
      if (b > 0.5 + 1.0e-6)
      {
	// At a height where we do the exponential decay
	alpha = log(0.1)/(b-0.5);
	c = exp(alpha*(rtilda-0.5));
      }
      else
      {
	//Height is such that it decays immediately to 0
	c = 0.0;
      }
    }
    else
    {
      // Inside the core, no decay, its a full blast (for that height)
      c = 1.0;
    }
  }
  return c;
}


//----------------------------------------------------------------
// set wind values by res-scaling a vector to the proper magnitude
// v = vector
// r = magnitude of v
// vert = wanted vertical component
// horiz = wanted horizontal component
Xyz Microburst::_wind(const Xyz &v, double r, double vert, double horiz) const
{
  Xyz w;
  
  // The vector v/r is a unit vector in the direction from the mb
  // center to the point...the x and y components are what we want, to
  // compute the x and y winds.
  if (fabs(r) < 1.0e-6)
  {
    w._x = w._y = 0.0;
  }
  else
  {
    w._x = horiz*v._x/r;
    w._y = horiz*v._y/r;
  }
  w._z = vert;
  return w;
}

//----------------------------------------------------------------
double Microburst::_windZ(double r, double z, double rscale,
			  double size_scale) const
{
  double ztilda, a, z0;
  // double rtilda, r0;
  
  // r0 = _r*rscale;
  z0 = _z*rscale;

  // It's defined as per the notes, in terms of wtilda and an exponential
  // drop off outside 1/2 the maximum radius
  // rtilda = r/r0;
  ztilda = z/z0;
  a = _Wtilda.apply(ztilda);

  // Give the maximum intensity 2* the max outflow possibly
  return (-1.0*rscale*a*_mag*2.0);

}

//----------------------------------------------------------------
double Microburst::_windU(double r, double z, double size_scale) const
{
  double a, b, f, r0;

  r0 = _r*size_scale;

  a = _Utilda.apply(r/r0);
  b = _Stilda.apply(z/(_z*size_scale));
  f = _mag*a*b;
  return (f);
}

//----------------------------------------------------------------
bool Microburst::_lineWindDirection(const LineAndPoint &info,
				    Xyz &wind) const
{
  double mag;
  
  // Project into the correct direction horizontally, which is either
  // of the normal vectors of the line segment.
  wind = info.normalVector();
  mag = wind.magnitude();
  if (mag < 1.0e-6)
  {
    return false;
  }
    
  // Normalize
  wind.scale(1.0/mag);

  // Under some circumstances reverse the direction
  double d = info.getDistanceInFront();
  if ((d < 0.0 && !_gain) ||(d >= 0.0 && _gain))
  {
    wind.scale(-1.0);
  }
  return true;
}
