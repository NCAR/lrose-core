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
/////////////////////////////////////////////////////////////
// WayPts.cc
//
// Zoom limits object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#include "WayPts.hh"
#include <cmath>
#include <cassert>

using namespace std;

//////////////
// Constructor

WayPts::WayPts()
{
}

/////////////////////////////
// Copy constructor
//

WayPts::WayPts(const WayPts &rhs)
{
  _copy(rhs);
}

/////////////
// destructor

WayPts::~WayPts()
{
  clear();
}

/////////////////////////////
// Assignment
//

WayPts &WayPts::operator=(const WayPts &rhs)
{
  return _copy(rhs);
}

/////////////////////////////
// Check for equality
//

bool WayPts::operator==(const WayPts &rhs)
{
  if (_lat.size() != rhs._lat.size()) {
    return false;
  }
  if (_lon.size() != rhs._lon.size()) {
    return false;
  }
  for (size_t ii = 0; ii < _lat.size(); ii++) {
    if (fabs(_lat[ii] - rhs._lat[ii]) > 1.0e-5) {
      return false;
    }
  }
  for (size_t ii = 0; ii < _lon.size(); ii++) {
    if (fabs(_lon[ii] - rhs._lon[ii]) > 1.0e-5) {
      return false;
    }
  }
  return true;
}

/////////////////////////////////////////////////////////
// clear the way points

void WayPts::clear()
{
  _lat.clear();
  _lon.clear();
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

WayPts &WayPts::_copy(const WayPts &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }

  _lat = rhs._lat;
  _lon = rhs._lon;
  
  return *this;
  
}

/////////////////////////////////////////////////////////
// add a point

void WayPts::addPoint(double lat, double lon)
{
  _lat.push_back(lat);
  _lon.push_back(lon);
}

/////////////////////////////////////////////////////////
// print

void WayPts::print(ostream &out) const
{
  assert(_lat.size() == _lon.size());
  out << "N wayPts:" << _lat.size() << endl;
  for (size_t ii = 0; ii < _lat.size(); ii++) {
    out << "  index, lat, lon: "
        << ii << ", "
        << _lat[ii] << ", "
        << _lon[ii] << endl;  
  }
}

/////////////////////////////////////////////////////////
// get a way point

void WayPts::getPoint(size_t index, double &lat, double &lon) const
{
  if (index >= _lat.size()) {
    lat = 0.0;
    lon = 0.0;
    return;
  }
  lat = _lat[index];
  lon = _lon[index];
}

