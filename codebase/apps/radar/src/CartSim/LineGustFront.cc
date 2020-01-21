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
 * @file LineGustFront.cc
 */

#include "LineGustFront.hh"
#include "GustFront.hh"
#include "Data.hh"
#include <toolsa/LogMsg.hh>
#include <cmath>

//----------------------------------------------------------------
LineGustFront::LineGustFront() : 
  LineAndPoint(),
  _scale(0),
  _dbz(0),
  _is_dbz(false)
{
}


//----------------------------------------------------------------
LineGustFront::~LineGustFront()
{
}

/*----------------------------------------------------------------*/
bool LineGustFront::segment(const GustFront &gf, const Xyz &loc,
			   bool &is_inside)
{
  double mag;
  Xyz dir;
    
  // Determine the loc of the point relative to the front, and some front
  // characteristics
  if (!pointLocation(loc))
  {
    return false;
  }
  if (_distance_beyond_side > 0.0)
  {
    _scale = exp(_distance_beyond_side*log(0.1)/gf.getSideDecay());
  }
  else
  {
    _scale = 1.0;
  }
  if (_distance_inside > 0.0)
  {
    is_inside = true;
  }

  // Apply the gust front model to get the wind magnitude, and indicate
  // random direction or not
  _model(gf, loc._z, mag, dir);
    
  // Rotate the direction vector properly for 3 dimensions
  _rotateVector(dir);

  // Rotate the wind properly into the direction
  _vel = Xyz(dir, mag);

  return true;
}

/*----------------------------------------------------------------*/
void LineGustFront::setDbz(double scale, Data &data) const
{
  if (_is_dbz)
  {
    data.setMaxDbz(scale*_scale*_dbz);
  }
}

/*----------------------------------------------------------------*/
void LineGustFront::accumlateInsideWind(Xyz &wind, double &denom) const
{
  wind = Xyz(wind, _distance_inside*_scale, _vel, false);
  denom += _distance_inside;
}

/*----------------------------------------------------------------*/
Xyz LineGustFront::getScaledWind(void) const
{
  return Xyz(_vel, _scale);
}

/*----------------------------------------------------------------*/
bool LineGustFront::fullInsideDbz(double dbz) const
{
  return (_dbz == dbz && _distance_beyond_side == 0.0);
}

/*----------------------------------------------------------------*/
bool LineGustFront::dbzOutside(bool zeroth, const GustFront &gf,
			      double z, double &ref) const
{
  if (((zeroth && _beyond_side==OUTSIDE_0) || 
       (!zeroth && _beyond_side == OUTSIDE_1)) && _distance_beyond_side > 0)
  {
    if (gf.insideThinLine(_distance_in_front, z))
    {
      // note the scale factor is computed from distance_beyond_side
      ref = gf.getDbzThinLine()*_scale;
      return true;
    }
  }
  return false;
}

/*----------------------------------------------------------------*/
void LineGustFront::_model(const GustFront &gf, const double z,
			  double &mag, Xyz &dir)
{
  double x = _distance_in_front;

  mag = 0.0;
  dir = Xyz();
    
  // Deal with the thin line
  _setDbz(gf, x, z);

  // deal with the mag and dir
  gf.model(_distance_in_front, z, mag, dir);
}

/*----------------------------------------------------------------*/
void LineGustFront::_setDbz(const GustFront &gf, double x, double z)
{
  _is_dbz = gf.insideThinLine(x, z);
  if (_is_dbz)
  {
    _dbz = gf.getDbzThinLine();
  }
  else
  {
    _dbz = 0;
  }
}

