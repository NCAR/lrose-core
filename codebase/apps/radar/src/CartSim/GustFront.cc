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
 * @file GustFront.cc
 */

#include "GustFront.hh"
#include "LineGustFront.hh"
#include "SimMath.hh"
#include "Data.hh"
#include <cmath>
using std::vector;

//----------------------------------------------------------------
GustFront::GustFront(const Params &P, const Params::GustFront_t &mP) : 
  Thing(P, mP.minutes_to_intensity_fuzzy_index, -1, mP.start_minutes,
	mP.lifetime_minutes),
  _mag(mP.wbehind_knots*KNOTS_TO_MS),
  _motion(mP.xmotion_knots, mP.ymotion_knots, 0),
  _zt(mP.thin_line_z_km*KM_TO_METERS),
  _xt(mP.thin_line_x_km*KM_TO_METERS),
  _rt(mP.thin_line_r_km*KM_TO_METERS),
  _dbzt(mP.thin_line_dbz),
  _z1(mP.z1_km*KM_TO_METERS),
  _z2(mP.z1_km*KM_TO_METERS),
  _z3(mP.z1_km*KM_TO_METERS),
  _x1(mP.x1_km*KM_TO_METERS),
  _x2(mP.x2_km*KM_TO_METERS),
  _xb(mP.xb_km*KM_TO_METERS),
  _side_decay(mP.side_decay),
  _is_wave(mP.wave)
{
  if (_is_wave)
  {
    printf("Wave mode not currently implemented\n");
    exit(1);
  }

  _motion.scale(KNOTS_TO_MS);
  if (!Thing::endpoints(P, mP.endpts_index, _loc))
  {
    exit(1);
  }
  if (_loc.size() < 2)
  {
    printf("ERROR need at least 2 points in a gust front\n");
    exit(1);
  }
}

//----------------------------------------------------------------
GustFront::~GustFront()
{
}

//----------------------------------------------------------------
void GustFront::_addToData(const Xyz &loc, Data &data) const
{
  vector<LineGustFront> w;
  int nloc;

  // Figure out the actual points to use, and move them where they
  // belong (based on time)
  if (!_setup(loc, nloc, w))
  {
    return;
  }

  // For each pair of points that make up a line segment
  // build the wind model for the segment.
  bool is_inside = false;
  bool status = true;
  for (int i=0; i<nloc; ++i)
  {
    if (!w[i].segment(*this, loc, is_inside))
    {
      status = false;
    }
  }
  if (!status)
  {
    return;
  }


  if (nloc == 1)
  {
    // add scaled wind to current data winds
    Xyz swind = w[0].getScaledWind();
    data.setVel(Xyz(data.getVel(), _intensity, swind, false));

    // modify the reflecitity in the data if in the thin line
    w[0].setDbz(_intensity, data);
  }
  else
  {
    // look at all segments to build up winds, then add to current winds
    Xyz wind;
    if (_buildSegmentWinds(is_inside, nloc, w, wind))
    {
      data.setVel(Xyz(data.getVel(), _intensity, wind, false));
    }
    else
    {
      return;
    }

    // look at all segments to detemine reflectivity, then modify data if
    // should do so
    double ref;
    if (_buildSegmentThinLine(w, loc, nloc, ref))
    {
      data.setMaxDbz(ref*_intensity);
    }
  }

}

/*----------------------------------------------------------------*/
bool GustFront::insideThinLine(double x, double z) const
{
  double r;

  // Compute radius from cylinder center
  r = sqrt((z - _zt)*(z - _zt) + (x - _xt)*(x - _xt));

  return (r <= _rt);
}


/*----------------------------------------------------------------*/
void GustFront::model(const double x, const double z, double &mag,
		      Xyz &dir) const
{
  mag = 0.0;
  dir = Xyz();
    
  if (x > 0.0)
  {
    // In front, no winds.
    return;
  }

  if (z > _z3)
  {
    // Above the top, no winds.
    return;
  }

  if (x <= 0.0 && x > -1.0*_x2)
  {
    // circular roll region
    _region3(x, z, mag, dir);
  }
  else if (x <= -1.0*_x2 && x > (-1.0*(_x1 + _x2)))
  {
    // Turb at top, normal flow below
    _region2(x, z, mag, dir);
  }
  else if (x <= (-1.0*(_x1 + _x2)) && x > (-1.0*(_x1 + _x2 + _xb)))
  {
    // Behind the front
    _region1(x, z, mag, dir);
  }
  else
  {
    // Way behind..no winds.
    return;
  }
}

/*----------------------------------------------------------------*/
void GustFront::_region1(double x, double z, double &mag, Xyz &dir) const
{
  if (z >= _z1)
  {
    // too high
    return;
  }
  mag = _mag*(x + _x2 + _x1 + _xb)/_xb;
  dir = Xyz(1, 0, 0);
}

/*----------------------------------------------------------------*/
void GustFront::_region2(double x, double z, double &mag, Xyz &dir) const
{
  double line;
  double line2;

  line = ((_z2 - _z1)*x + (_x2 + _x1)*_z2 - _x2*_z1)/_x1;
  if (z < line)
  {
    mag = _mag*(x + _x2 + _x1 + _xb)/_xb;
    dir = Xyz(1, 0, 0);
  }
  else
  {
    line2 = ((_z3 - _z1)*x + (_x2 + _x1)*_z3 - _x2*_z1)/_x1;
    if (z <= line2)
    {
      _turbulentRegion(_mag, mag, dir);
    }
    else
    {	    
      return;
    }
  }
}

/*----------------------------------------------------------------*/
void GustFront::_region3(double x, double z, double &mag, Xyz &dir) const
{
  double xbar;
  double centerx;
  double centerz;
  double r0, r;

  // Rescale so its a square
  xbar = x*_z3/_x2;

  r0 = _z3/2.0;
  centerx = -1.0*r0;
  centerz = r0;

  // Get the radius from the center to the point.
  r = sqrt((xbar-centerx)*(xbar-centerx) + (z-centerz)*(z-centerz));

  if (r >= r0)
  {
    // Outside the big circle
    _regionOutside3(xbar, centerx, z, centerz, mag, dir);
  }
  else
  {
    // Inside the big circle
    mag = _mag*r/r0;
    if (r < 1.0e-6)
    {
      dir = Xyz(1, 0, 0);
    }
    else
    {
      dir = Xyz((centerz - z)/r, 0, (xbar - centerx)/r);
    }
    // This is in the xbar,z coordinate system, transfer back to
    // xz  (but not yet!)
  }
}

/*----------------------------------------------------------------*/
void GustFront::_turbulentRegion(double max, double &mag, Xyz &dir) const
{
  double a;

  if (max <1.0e-6)
  {
    mag = 0.0;
    dir = Xyz(1, 0, 0);
    return;
  }

  // mag = Random [0,max]
  mag = RANDOMF2(max);

  // direction is random 
  dir = Xyz(RANDOMF2(1.0), RANDOMF2(1.0), RANDOMF2(1.0));
  a = sqrt(dir._x*dir._x + dir._y*dir._y + dir._z*dir._z);
  if (a < 1.0e-6)
  {
    printf("WARNING...direction singularity..should be rare\n");
    dir = Xyz(1, 0, 0);
    a = 1.0;
  }
  dir.scale(1.0/a);
}

/*----------------------------------------------------------------*/
void GustFront::_regionOutside3(double x, double centerx, double z,
				double centerz, double &mag, Xyz &dir) const
{
  // No winds, unless one of the cases below
  if (x < centerx)
  {
    // To the side of the circle that is more behind the front
    if (z < centerz)
    {
      // Bottom has nice winds
      mag = _mag;
      dir = Xyz(1, 0, 0);
    }
    else
    {
      // Top has turbulent winds
      _turbulentRegion(_mag, mag, dir);
    }
  }
}

/*----------------------------------------------------------------*/
bool GustFront::_setup(const Xyz &loc, int &nloc,
		       std::vector<LineGustFront> &w) const
{
  if (_elapsedSeconds > 0 && _is_wave)
  {
#ifdef WAVE
    nloc = 2*_loc.size();
    for (int i=0; i<nloc; ++i)
    {
      w.push_back(GustFrontInfo());
    }
    _setupWave(_elapsedSeconds, nloc, w);
#endif
  }
  else
  {
    // one segment per pair of points
    nloc = _loc.size()-1;
    for (int i=0; i<nloc; ++i)
    {
      w.push_back(LineGustFront());
    }

    // Move the endpoints forward together.
    for (int i=0; i<nloc; ++i)
    {
      w[i].setLocs(Xyz(_loc[i], _elapsedSeconds, _motion, true),
		   Xyz(_loc[i+1], _elapsedSeconds, _motion, true));
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
bool GustFront::_buildSegmentWinds(bool is_inside, int nloc,
				   std::vector<LineGustFront> &w,
				   Xyz &wind) const
{
  if (is_inside)
  {
    double denom = 0.0;
    wind = Xyz();
    for (int i=0; i<nloc; ++i)
    {
      w[i].accumlateInsideWind(wind, denom);
    }
    if (denom > 0.0)
    {
      wind.scale(1.0/denom);
    }
    else
    {
      printf("ERROR...unexpected logical situation\n");
      return false;
    }
  }
  else
  {
    double a=0;
    int imin = -1;
    for (int i=0; i<nloc; ++i)
    {
      w[i].minimizeDistanceBeyondSide(i, a, imin);
    }
    if (imin >= 0)
    {
      wind = w[imin].getScaledWind();
    }
    else
    {
      printf("No winds set\n");
      return false;
    }
  }
  return true;
}

/*----------------------------------------------------------------*/
bool GustFront::_buildSegmentThinLine(vector<LineGustFront> &w,
				      const Xyz &loc, int nloc,
				      double &ref) const
{
  int i;
  bool is_between;

  for (i=0; i<nloc; ++i)
  {
    if (w[i].fullInsideDbz(_dbzt))
    {
      // Thats it..full intensity for this time here..
      ref = _dbzt;
      return true;
    }
  }

  // Never found a thin line this point was inside of..
  // Do the between cylinder checks.  (2 less than # of verticies (the
  // 2 endpoints don't have this check)).
  for (is_between=false, i=0; i<nloc-1; ++i)
  {
    double x, z;
    if (w[i+1].pointBetweenAdjacentSegments(w[i], loc, x, z))
    {
      is_between = true;
      if (insideThinLine(x, z))
      {
	ref = _dbzt;
	return true;
      }
    }
  }
  if (is_between)
  {
    return false;
  }

  // Not between any of the cylinders, and not right in there either.
  // Could be off the side of the 0th or last part..
  if (w[0].dbzOutside(true, *this, loc._z, ref))
  {
    return true;
  }
  if (w[nloc-2].dbzOutside(false, *this, loc._z, ref))
  {
    return true;
  }
  return false;
}

#ifdef WAVE
/*----------------------------------------------------------------*/
void GustFront::_setupWave(int elapsedt, int nloc,
			   std::vector<LineGustFront> &w) const
{

  double meters_move;

  // Add extra points to the thing.  How far do points move?
  meters_move = _motion.magnitude()*elapsedt;

  // move the interior endpoints forward.
  for (int i=0; i<(int)_loc.size()-1; ++i)
  {
    Xyz l0, l1;
    _waveInteriorPoint(_loc[i], _loc[i+1], meters_move, l0, l1);

    w[2*i+1].set_loc(l0);
    w[2*i+2].set_loc(l1);
  }

  // Create the 0th point and last point
  Xyz loc;
  _waveEndpoint(w[3].get_loc(), w[2].get_loc(), _loc[0], _loc[1], meters_move,
		 true, loc);
  w[0].set_loc(loc);
  _waveEndpoint(w[nloc-3].get_loc(), w[nloc-4].get_loc(), _loc[_loc.size()-1],
		_loc[_loc.size()-2], meters_move, false, loc);
  w[nloc-1].set_loc(loc);
}

/*----------------------------------------------------------------*/
void GustFront::_waveInteriorPoint(const Xyz &loc0, const Xyz &loc1, 
				   double meters_move, Xyz &outloc0,
				   Xyz &outloc1) const
{
  double mag;
  Xyz segment;

  // Get the direction of this segment (segment from loc0 to loc1)
  segment = Xyz(loc1._y - loc0._y, -1.0*(loc1._x - loc0._x), 0);

  mag = segment.magnitude();
  if (mag <= 1.0e-6)
  {
    printf("ERROR very small segment\n");
    exit(-1);
  }

  segment.scale(1.0/mag);

  // Move the points foward in this direction into the proper indicies
  outloc0 = Xyz(loc0, meters_move, segment, true);
  outloc1 = Xyz(loc1, meters_move, segment, true);
}

/*----------------------------------------------------------------*/
void GustFront::_waveEndpoint(const Xyz &outloc0,  const Xyz &outloc1, 
			      const Xyz &inloc0, const Xyz &inloc1,  
			      double meters_move, bool is_ok, Xyz &outloc)
{
  double len, mag;
  Xyz segment;

  // build the vector between the two output locs, and get its length
  segment = Xyz(outloc0, -1.0, outloc1, false);
  len = segment.magnitude();

  // Build the unit vector from 1 to 0 input points
  segment = Xyz(inloc0._x - inloc1._x, inloc0._y - inloc1._y, 0);
  mag = segment.magnitude();
  if (mag <= 1.0e-6)
  {
    printf("ERROR very small segment\n");
    exit(-1);
  }
  segment.scale(1.0/mag);

  // Move the 0th input point over by len in this unit direction
  outloc = Xyz(inloc0, len, segment, true);

  // Now determine a unit motion vector perpendicular to the 0,1 segment.
  if (is_ok)
  {
    segment = Xyz(inloc1._y - inloc0._y, -1.0*(inloc1._x - inloc0._x), 0);
  }
  else
  {
    segment = Xyz(inloc0._y - inloc1._y, -1.0*(inloc0._x - inloc1._x), 0);
  }
  // segment._z = 0.0;
  mag = segment.magnitude();
  if (mag <= 1.0e-6)
  {
    printf("ERROR very small segment\n");
    exit(-1);
  }
  segment.scale(1.0/mag);

  // Move the segment by the proper meters out along this motion direction
  outloc = Xyz(outloc, meters_move, segment, true);
}

#endif

