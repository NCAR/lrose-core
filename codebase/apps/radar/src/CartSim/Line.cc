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
 * @file Line.cc
 */

#include "Line.hh"
#include "GustFront.hh"
#include <toolsa/LogMsg.hh>
#include <cmath>

//----------------------------------------------------------------
Line::Line() :
  _width(0),
  _orientation(NONE)
{
}

//----------------------------------------------------------------
Line::~Line()
{
}

/*----------------------------------------------------------------*/
void Line::setLocs(const Xyz &loc0, const Xyz &loc1)
{
  _loc0 = loc0;
  _loc1 = loc1;
  _width = 0;
  _orientation = NONE;
  _front_dir = Xyz();
  
  // The whole thing is projected onto a 2d plane 
  bool yoriented = false;
  bool xoriented = false;
  double alpha;  // linear to take on value 1 at p0 and 0 at _loc1 

  alpha = (_loc1._x - _loc0._x);
  if (fabs(alpha) < 1.0e-6)
  {
    yoriented = true;
  }
  alpha = (_loc0._y - _loc1._y);
  if (fabs(alpha) < 1.0e-6)
  {
    xoriented = true;
  }
  if (xoriented && yoriented)
  {
    LOG(LogMsg::WARNING, "NULL GUST FRONT (line length0)");
    _orientation = NONE;
  }
  else if (yoriented && !xoriented)
  {
    _orientation = Y_LINE;
    if (_loc0._y < _loc1._y)
    {
      _width = _loc1._y - _loc0._y;
      _front_dir._x = 1.0;
      _front_dir._y = 0.0;
    }
    else
    {
      _width = _loc0._y - _loc1._y;
      _front_dir._x = -1.0;
      _front_dir._y = 0.0;
    }
  }
  else if ((!yoriented) && xoriented)
  {
    _orientation = X_LINE;
    if (_loc0._x > _loc1._x)
    {
      _width = _loc0._x - _loc1._x;
      _front_dir._x = 0.0;
      _front_dir._y = 1.0;
    }
    else
    {
      _width = _loc1._x - _loc0._x;
      _front_dir._x = 0.0;
      _front_dir._y = -1.0;
    }
  }
  else
  {
    _orientation = SLOPED;

    double yy, xx;
    yy = _loc0._x - _loc1._x;
    xx = _loc1._y - _loc0._y;
    if (xx*xx + yy*yy < 1.0e-6)
    {
      LOG(LogMsg::ERROR, "logic..unstable due to very short gf line?");
      _orientation = NONE;
    }
    else
    {
      alpha = atan2(yy, xx);
      _front_dir._x = cos(alpha);
      _front_dir._y = sin(alpha);
      _width = sqrt((_loc0._x - _loc1._x)*(_loc0._x - _loc1._x) +
		    (_loc0._y - _loc1._y)*(_loc0._y - _loc1._y));
    }
  }
}

/*----------------------------------------------------------------*/
bool Line::pointBetweenAdjacentSegments(const Line &previous_segment,
					const Xyz &loc, double &x,
					double &z) const
{
  double mag;
  x = z = 0;

  // check for common point, expect 0th point of this, and 1th point of prev
  if (!(previous_segment._loc1 == _loc0))
  {
    LOG(LogMsg::ERROR, "unexpected input locations");
    return false;
  }

  // Compute r = loc - loc0 = vector relative to loc0.
  // If its very small, its effectively right where we want it
  // (between the segments) or at least close enough.
  // 
  // If its not small, figure out where this direction (r) is relative
  // to the two front directions 
  Xyz r(loc, -1, _loc0, false);
  r._z = 0.0;
  mag = r.magnitude();
  if (mag > 1.0e-6)
  {
    if (!Xyz::vectorBetween(previous_segment._front_dir, r, _front_dir))
    {
      return false;
    }
  }

  // At this point, the data lies in the arc between the 2 input directions.
  x = r.magnitude();
  z = _loc0._z;
  return true;

}

//----------------------------------------------------------------
Xyz Line::normalVector(void) const
{
  return Xyz(_loc1._y - _loc0._y, _loc0._x - _loc1._x, 0);
}

