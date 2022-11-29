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
///////////////////////////////////////////////////////////////
// RayMapping.cc
//
// RayMapping object
//
///////////////////////////////////////////////////////////////

#include "RayMapping.hh"
#include <Radx/RadxRay.hh>
#include <algorithm>
#include <cmath>
#include <vector>
using std::vector;

//------------------------------------------------------------------
RayMapping::RayMapping(void) :
        _az_tolerance_degrees(-1),
        _elev_tolerance_degrees(-1)
{
}

//------------------------------------------------------------------
RayMapping::RayMapping(const int nelev, const double *elev,
                         const double az_tolerance_degrees,
                         const double elev_tolerance_degrees) :
        _az_tolerance_degrees(az_tolerance_degrees),
        _elev_tolerance_degrees(elev_tolerance_degrees)
{
  for (int i=0; i<nelev; ++i)
  {
    _elev.push_back(elev[i]);
  }
  return;
}

//------------------------------------------------------------------
RayMapping::~RayMapping()

{
}

//------------------------------------------------------------------
bool RayMapping::add(const RadxRay &ray)
{
  double az = ray.getAzimuthDeg();
  double elev = ray.getElevationDeg();
  if (find(_az.begin(), _az.end(), az) == _az.end())
  {
    _az.push_back(az);
  }

  // map the elev to a mapped to one
  double elev_match;
  if (_match(elev, _elev, _elev_tolerance_degrees, elev_match))
  {
    RayAzElev ae(az, elev_match);
    if (find(_azelev.begin(), _azelev.end(), ae) != _azelev.end())
    {
      _azelevMulti.push_back(ae);
    }
    else
    {
      _azelev.push_back(ae);
    }
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool RayMapping::isMulti(const double az, const double elev) const
{
  double az_match, elev_match;
  if (_match(az, _az, _az_tolerance_degrees, az_match) &&
      _match(elev, _elev, _elev_tolerance_degrees, elev_match))
  {
    RayAzElev ae(az_match, elev_match);
    return find(_azelevMulti.begin(), _azelevMulti.end(), ae) != 
      _azelevMulti.end();
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
bool RayMapping::isMulti(const RayAzElev &ae) const
{
  return find(_azelevMulti.begin(), _azelevMulti.end(), ae) != 
    _azelevMulti.end();
}

//------------------------------------------------------------------
RayAzElev RayMapping::match(const double az, const double elev) const
{
  double az_match, elev_match;
  if (_match(az, _az, _az_tolerance_degrees, az_match) &&
      _match(elev, _elev, _elev_tolerance_degrees, elev_match))
  {
    return RayAzElev(az_match, elev_match);
  }
  else
  {
    return RayAzElev();
  }
}

//------------------------------------------------------------------
bool RayMapping::_match(const double a, const vector<double> &v, 
                         const double tolerance, double &amatch)
{
  double mindiff = 0.0;
  bool first = true;
  for (int i=0; i<(int)v.size(); ++i)
  {
    if (first)
    {
      mindiff = fabs(v[i] - a);
      amatch = v[i];
      first = false;
    }
    else
    {
      double d = fabs(v[i] - a);
      if (d < mindiff)
      {
	mindiff = d;
	amatch = v[i];
      }
    }
  }
  if (first)
  {
    return false;
  }
  else
  {
    return (mindiff < tolerance);
  }
}

