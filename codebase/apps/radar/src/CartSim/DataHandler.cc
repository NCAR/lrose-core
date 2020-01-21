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
 * @file DataHandler.cc
 */

#include "DataHandler.hh"
#include "Xyz.hh"
#include "Data.hh"
#include <cmath>

//----------------------------------------------------------------
DataHandler::DataHandler(const Params &parms) :
  _parms(parms), _polar(false), _cx(0), _cy(0), _cz(0), _first(true)
{
}

//----------------------------------------------------------------
DataHandler::~DataHandler()
{
}

//----------------------------------------------------------------
bool DataHandler::nextPoint(Xyz &loc)
{
  if (_first)
  {
    _initIndexing();
    _first = false;
  }
  else
  {
    if (!_increment())
    {
      return false;
    }
  }

  loc = _computeXyzMeters();
  return true;
}

//----------------------------------------------------------------
void DataHandler::_reset(void)
{
  _cx = _cy = _cz = 0;
  _first = true;
  _initIndexing();
}

//----------------------------------------------------------------
double DataHandler::_radialVel(const Data &data) const
{
  // get x,y,z motion
  Xyz motion = data.getVel();

  // radial unit vector
  Xyz r;

  if (_polar)
  {
    // y,z are radians
    r = Xyz(cos(_cy)*cos(_cz), sin(_cy)*cos(_cz), sin(_cz));
  }
  else
  {
    double m = sqrt(_cx*_cx + _cy*_cy + _cz*_cz);
    r = Xyz(_cx, _cy, _cz);
    r.scale(1.0/m);
  }

  return motion.dotProduct(r);
}

