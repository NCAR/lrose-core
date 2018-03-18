// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
#include <euclid/MotionVector.hh>
#include <rapmath/Math.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <cstdio>

#define BAD -99.99

/*----------------------------------------------------------------*/
MotionVector::MotionVector()
{
  _vx = 0.0;
  _vy = 0.0;
}

/*----------------------------------------------------------------*/
MotionVector::MotionVector(double ivx, double ivy)
{
  _vx = ivx;
  _vy = ivy;
}

/*----------------------------------------------------------------*/
MotionVector::~MotionVector()
{
}

/*----------------------------------------------------------------*/
double MotionVector::getVx(void) const
{
  return _vx;
}

/*----------------------------------------------------------------*/
double MotionVector::getVy(void) const
{
  return _vy;
}

/*----------------------------------------------------------------*/
double MotionVector::getSpeed(void) const
{
  return sqrt(_vx*_vx + _vy*_vy);
}

/*----------------------------------------------------------------*/
void MotionVector::average(const MotionVector &v)
{
  if (_vx == BAD)
    _vx = v._vx;
  else
  {
    if (v._vx != BAD)
      _vx = (_vx + v._vx)/2.0;
  }
    
  if (_vy == BAD)
    _vy = v._vy;
  else
  {
    if (v._vy != BAD)
      _vy = (_vy + v._vy)/2.0;
  }
}

/*----------------------------------------------------------------*/
void MotionVector::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
void MotionVector::print(FILE *fp) const
{
  fprintf(fp, "%s", sprint().c_str());
}

/*----------------------------------------------------------------*/
std::string MotionVector::sprint(void) const
{
  char buf[100];
  sprintf(buf, "v=(%.2f,%.2f)", _vx, _vy);
  std::string ret = buf;
  return ret;
}

/*----------------------------------------------------------------*/
void MotionVector::adjustDirection(double angle)
{
  double mag;

  mag = getSpeed();
  if (fabs(mag) < 1.0e-10)
  {
    // Zero velocity, effectively..make it so.
    _vx = _vy = 0.0;
    return;
  }

  // rotate the Vel
  rotate(angle, false);
  if (fabs(_vx) < 1.0e-10)
  {
    // Its already set correctly ..just use original values.
  }
  else
  {
    if (fabs(_vy) < 1.0e-10)
    {
      // Vel is perp. to wanted direction, don't know what to do,
      // so set it to 0
      LOG(ERROR) << "adjusting line vel direction..perpendicular set to 0";
      _vx = _vy = 0.0;
      return;
    }	
    else
    {
      // preserve mag and orient perpendicular one or other way.
      _vx = 0.0;
      if (_vy > 0.0)
	_vy = mag;
      else
	_vy = -mag;
    }
  }
    
  // Rotate back to original orientation
  rotate(-angle, false);
}

/*----------------------------------------------------------------*/
double MotionVector::angleBetween(const MotionVector &v) const
{
  return Math::angleBetweenVectors(_vx, _vy, v._vx, v._vy);
}

/*----------------------------------------------------------------*/
double MotionVector::getAngle(void) const
{
  double t0;
  
  t0 = atan2(_vy, _vx)*180.0/3.14159;
  if (t0 < 0.0)
    t0 += 360.0;
  return t0;
}

/*----------------------------------------------------------------*/
double MotionVector::getAnglePlusMinus180(void) const
{
  double t0;
  
  t0 = atan2(_vy, _vx)*180.0/3.14159;
  if (t0 < -180.0)
    t0 = -180.0;
  if (t0 > 180.0)
    t0 = 180.0;
  return t0;
}

/*----------------------------------------------------------------*/
void MotionVector::add(const MotionVector &v)
{
  _vx += v._vx;
  _vy += v._vy;
}

/*----------------------------------------------------------------*/
void MotionVector::mult(const double v)
{
  _vx *= v;
  _vy *= v;
}

/*----------------------------------------------------------------*/
void MotionVector::rotate(double angle, bool change_endpts)
{
  Math::rotatePoint(_vx, _vy, angle);
}

/*----------------------------------------------------------------*/
void MotionVector::bias(double b)
{
  _vx += b;
  _vy += b;
}

/*----------------------------------------------------------------*/
void MotionVector::reverseHandedness(void)
{
  if (_vx != BAD && _vy != BAD)
  {
    _vx = -_vx;
    _vy = -_vy;
  }
}

/*----------------------------------------------------------------*/
bool MotionVector::velMissing(void) const
{
  return (_vx == BAD || _vy == BAD);
}

/*----------------------------------------------------------------*/
void MotionVector::scale(double s)
{
  _vx *= s;
  _vy *= s;
}

/*----------------------------------------------------------------*/
// set speed to this value, which is a rescaling.
void MotionVector::set(double v)
{
  double mag = getSpeed();
  if (fabs(mag) < 1.0e-10)
    return;  // don't know direction
  scale(v/mag);
}
