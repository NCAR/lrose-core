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
 * @file OutlierData.cc
 */

#include "OutlierData.hh"
#include "SimMath.hh"
#include "Data.hh"

//----------------------------------------------------------------
OutlierData::OutlierData(void) : _vel(0,0,0), _noise(0), _dbz(0), _clutter(0)
{
}

//----------------------------------------------------------------
OutlierData::OutlierData(double xvel_knots, double yvel_knots,
			 double zvel_knots, double noise, double dbz,
			 double clutter) :
  _vel(xvel_knots, yvel_knots, zvel_knots), _noise(noise), _dbz(dbz),
  _clutter(clutter)
{
  _vel.scale(KNOTS_TO_MS);
}

//----------------------------------------------------------------
OutlierData::~OutlierData()
{
}

//----------------------------------------------------------------
void OutlierData::setData(double intensity, Data &data) const
{
  double n;
  if (_noise > 0.0)
  {
    n = RANDOMF(_noise);
  }
  else
  {
    n = 1.0;
  }

  Xyz v(_vel, intensity*n);
  data.setVel(v);
  data.setDbz(_dbz*intensity*n);
  data.setClutter(_clutter);
}
