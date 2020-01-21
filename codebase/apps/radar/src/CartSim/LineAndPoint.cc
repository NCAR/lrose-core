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
 * @file LineAndPoint.cc
 */

#include "LineAndPoint.hh"
#include "GustFront.hh"
#include "Data.hh"
#include <toolsa/LogMsg.hh>
#include <cmath>

//----------------------------------------------------------------
LineAndPoint::LineAndPoint() : 
  Line(),
  _distance_in_front(0),
  _distance_beyond_side(0),
  _beyond_side(INSIDE),
  _distance_inside(0)
{
}


//----------------------------------------------------------------
LineAndPoint::~LineAndPoint()
{
}

//----------------------------------------------------------------
bool LineAndPoint::pointLocation(const Xyz &p)
{
  double slope;
  double alpha;
  double num, denom;

  switch (_orientation)
  {
  case Y_LINE:
    if (_loc0._y < _loc1._y)
    {
      _distance_in_front = p._x - _loc0._x;
    }
    else
    {
      _distance_in_front = _loc0._x - p._x;
    }
    alpha = (p._y - _loc1._y)/(_loc0._y - _loc1._y);
    break;
  case X_LINE:
    if (_loc0._x > _loc1._x)
    {
      _distance_in_front = p._y - _loc0._y;
    }
    else
    {
      _distance_in_front = _loc0._y - p._y;
    }
    alpha = (p._x - _loc1._x)/(_loc0._x - _loc1._x);
    break;
  case SLOPED:
    _orientation = SLOPED;
    slope = (_loc1._y - _loc0._y)/(_loc1._x - _loc0._x);
    denom = _loc0._y - _loc1._y + (_loc0._x - _loc1._x)/slope;
    num = p._y - _loc1._y + (p._x - _loc1._x)/slope;
    alpha = num/denom;
    _sloped(p, slope);
    break;
  default:
    return false;
  }

  if (alpha > 1.0)
  {
    _distance_beyond_side = (alpha - 1.0)*_width;
    _beyond_side = OUTSIDE_0;
    _distance_inside = 0.0;
  }
  else if (alpha < 0)
  {
    _distance_beyond_side = -1.0*alpha*_width;
    _beyond_side = OUTSIDE_1;
    _distance_inside = 0.0;
  }
  else
  {
    _beyond_side = INSIDE;
    _distance_beyond_side = 0.0;
    if (alpha <= 0.5)
    {
      _distance_inside = _width*alpha;
    }
    else
    {
      _distance_inside = _width*(1.0-alpha);
    }
  }

  return true;
}

/*----------------------------------------------------------------*/
void LineAndPoint::minimizeDistanceBeyondSide(int index, double &a,
					      int &imin) const
{
  if (imin < 0)
  {
    imin = index;
    a = _distance_beyond_side;
  }
  else
  {
    if (_distance_beyond_side < a)
    {
      a = _distance_beyond_side;
      imin = index;
    }
  }
}

/*----------------------------------------------------------------*/
void LineAndPoint::_sloped(const Xyz &p, const double slope)
{
  double parametric, b;

  // To determine behind or ahead, if direction of normal vector
  // is such that y decreases, then if (x,y) is greater than (x,line_y)
  // then we are behind, and conversely.
  // The normal vector is in direction (y[1]-y[0], -(x[1]-x[0]))
  parametric = p._y - _loc0._y - slope*(p._x - _loc0._x);
  if (_loc0._x > _loc1._x)
  {
    // Normal is y increases, so reverse the check (check for y < line).
    parametric = -parametric;
  }

  // Y = slope*x + b i.e.   b = y - slope*x
  // distance = | slope*x - y + b| / sqrt(slope**2 + 1)
  b = _loc0._y - slope*_loc0._x;

  // Set up return values
  _distance_in_front = fabs(slope*p._x - p._y + b)/sqrt(slope*slope + 1.0);
  if (parametric >= 0)
  {
    // Behind the front
    _distance_in_front = -1.0* (_distance_in_front);
  }
}

/*----------------------------------------------------------------*/
void LineAndPoint::_rotateVector(Xyz &dir) const
{
  double xx, yy, alpha, temp;

  // Base things on orientation
  switch (_orientation)
  {
  case X_LINE:
    if (_loc0._x < _loc1._x)
    {
      // +x goes to -y, +y goes to +x
      temp = dir._x;
      dir._x = dir._y;
      dir._y = -1.0*temp;
    }
    else
    {
      // +x goes to +y, +y goes to -x
      temp = dir._x;
      dir._x = -1.0*dir._y;
      dir._y = temp;
    }   
    break;
  case Y_LINE:
    if (_loc0._y < _loc1._y)
    {
      // +x goes to +x, +y goes to +y, no rotation
    }
    else
    {
      // +x goes to -x, +y goes to -y
      dir._x = -1.0*dir._x;
      dir._y = -1.0*dir._y;
    }   
    break;
    case SLOPED:
      // have to think a bit and compute dir and front_dir by rotation
      yy = _loc0._x - _loc1._x;
      xx = _loc1._y - _loc0._y;
      alpha = atan2(yy, xx);
      xx = dir._x*cos(alpha) - dir._y*sin(alpha);
      yy = dir._x*sin(alpha) + dir._y*cos(alpha);
      dir._x = xx;
      dir._y = yy;
      break;
  case NONE:
  default:
    dir._x = dir._y = 0.0;
    break;
  }	
}

