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
 * @file Data.cc
 */

#include "Data.hh"
#include "SimMath.hh"
#include <toolsa/LogMsg.hh>
#include <cmath>

//----------------------------------------------------------------
Data::Data(void) : _vel(), _ref(0), _snr(0), _sw(0), _clutter(0),
		   _noise(0), _turb_noise(0)
{
}

//----------------------------------------------------------------
Data::Data(const Params &p) :
  _vel(p._ambient_vel_knots[0], p._ambient_vel_knots[1],
       p._ambient_vel_knots[2]),
  _ref(p.ambient_dbz),
  _snr(p.ambient_snr),
  _sw(p.ambient_sw),
  _clutter(p.ambient_clutter),
  _noise(p.ambient_noise),
  _turb_noise(0)
{
  _vel.scale(KNOTS_TO_MS);
}

//----------------------------------------------------------------
Data::~Data()
{
}

//----------------------------------------------------------------
double Data::value(Params::Field_t f) const
{
  double s;
  switch (f)
  {
  case Params::RADIAL_VEL:
    LOG(LogMsg::ERROR, "RADIAL_VEL not available");
    s = 0.0;
    break;
  case Params::VX:
    s = _vel._x;
    break;
  case Params::VY:
    s = _vel._y;
    break;
  case Params::VZ:
    s = _vel._z;
    break;
  case Params::DBZ:
    s = _ref;
    break;
  case Params::SNR:
    s = _snr;
    break;
  case Params::SW:
    s = _sw;
    break;
  case Params::CLUTTER:
    s = _clutter;
    break;
  default:
    LOGF(LogMsg::ERROR, "Bad input %d", (int)f);
    s = 0.0;
    break;
  }
  return s;
}

//----------------------------------------------------------------
void Data::addNoise(void)
{
  // Velocity contaminated with noise due to background and turbulence
  // where change in each component is to be random in the interval
  // [|v|(-noise-turb_noise), |v|*(noise+turb_noise)]
  double m = windSpeed();

  double n = RANDOMF3(_noise + _turb_noise)*m;
  _vel._x += n;

  n = RANDOMF3(_noise + _turb_noise)*m;
  _vel._y += + n;

  n = RANDOMF3(_noise + _turb_noise)*m;
  _vel._z += + n;


  // Other fields get background noise only
  n = RANDOMF(_noise);
  _ref *= n;

  n = RANDOMF(_noise);
  _snr *= n;
  if (_snr < 0.0)
  {
    _snr = 0.0;
  }


  // except for sw, which gets the turbulent noise too...
  n = RANDOMF(_noise + _turb_noise);
  _sw *= n;
  if (_sw < 0.0)
  {
    _sw = 0.0;
  }
}


