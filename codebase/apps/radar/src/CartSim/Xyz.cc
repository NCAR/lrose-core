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
 * @file Xyz.cc
 */

#include "Xyz.hh"
#include "SimMath.hh"
#include <cmath>
#include <cstdio>
#include <cstdlib>

//----------------------------------------------------------------
Xyz::Xyz(const Xyz &v, double c, const Xyz &u, const bool zpositive)
{
  _x = v._x + c*u._x;
  _y = v._y + c*u._y;
  _z = v._z + c*u._z;
  if (_z < 0 && zpositive)
  {
    _z = 0;
  }
}

//----------------------------------------------------------------
Xyz::Xyz(const Xyz &v, const double scale)
{
  _x = v._x*scale;
  _y = v._y*scale;
  _z = v._z*scale;
}

//----------------------------------------------------------------
double Xyz::magnitude(void) const
{
  return sqrt(_x*_x + _y*_y + _z*_z);
}

//----------------------------------------------------------------
void Xyz::scale(const double s)
{
  _x *= s;
  _y *= s;
  _z *= s;
}

//----------------------------------------------------------------
void Xyz::scale(const double sx, const double sy, const double sz)
{
  _x *= sx;
  _y *= sy;
  _z *= sz;
}

//----------------------------------------------------------------
double Xyz::dotProduct(const Xyz &s) const
{
  return _x*s._x + _y*s._y + _z*s._z;
}

//----------------------------------------------------------------
double Xyz::xyAngleDegrees0to360(void) const
{
  double theta = atan2(_y, _x)*180.0/LOC_PI;
  while (theta < 0.0)
  {
    theta +=360.0;
  }
  while (theta >= 360.0)
  {
    theta -= 360.0;
  }
  return theta;
}

//----------------------------------------------------------------
double Xyz::zAngleDegrees0to360(void) const
{
  double phi = atan2(_z, sqrt(_x*_x + _y*_y))*180./LOC_PI;
  while (phi < 0.0)
  {
    phi +=360.0;
  }
  while (phi >= 360.0)
  {
    phi -= 360.0;
  }
  return phi;
}

//----------------------------------------------------------------
bool Xyz::operator==(const Xyz &x) const
{
  return x._x == _x && x._y == _y && x._z == _z;
}

//----------------------------------------------------------------
bool Xyz::inBox(const Xyz &center, const Xyz &radius) const
{
  if (_x < center._x - radius._x || _x > center._x + radius._x)
  {
    return false;
  }
  if (_y < center._y - radius._y || _y > center._y + radius._y)
  {
    return false;
  }
  if (_z < center._z - radius._z || _z > center._z + radius._z)
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------
bool Xyz::vectorBetween(const Xyz &a, const Xyz &r, const Xyz &b)
{
  double thetaa, thetab, theta;
  
  thetaa = atan2(a._y, a._x);
  thetab = atan2(b._y, b._x);
  thetaa = RADIANS2DEGREES(thetaa);
  thetab = RADIANS2DEGREES(thetab);
  theta = atan2(r._y, r._x);
  theta = RADIANS2DEGREES(theta);

  if ((thetaa >= 0.0 && thetab >= 0.0) || (thetaa < 0.0 && thetab < 0.0))
  {
    if (thetaa >= thetab)
      return (CART_SIM_BETWEEN(thetab, theta, thetaa));
    else
      return (CART_SIM_BETWEEN(thetaa, theta, thetab));
  }
  else if (thetaa >= 0.0 && thetab < 0.0)
    return vectorBetweenMixed(thetaa, theta, thetab);
  else if (thetaa < 0.0 && thetab >= 0.0)
    return vectorBetweenMixed(thetab, theta, thetaa);
  else
  {
    printf("What did I forget?\n");
    exit(-1);
  }
}


bool Xyz::vectorBetweenMixed(double a, double x, double b)
{
  if (a - b < 180.0)
  {
    if ((x < 0.0 && x <= b) || (x >= 0.0 && x <= a))
      return true;
    else
      return false;
  }
  else
  {
    if ((x >= 0.0 && x >= a) || (x < 0.0 && x <= b))
      return true;
    else
      return false;
  }
}


