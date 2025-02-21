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
// LatLonBox.cc
//
// Zoom limits object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#include "LatLonBox.hh"
#include <cmath>

using namespace std;

//////////////
// Constructor

LatLonBox::LatLonBox()
  
{
  _init();
}

LatLonBox::LatLonBox(double minLat, double maxLat,
                     double minLon, double maxLon) :
        _minLat(minLat),
        _maxLat(maxLat),
        _minLon(minLon),
        _maxLon(maxLon)
{

}

/////////////////////////////
// Copy constructor
//

LatLonBox::LatLonBox(const LatLonBox &rhs)
{
  _copy(rhs);
}

/////////////
// destructor

LatLonBox::~LatLonBox()
{
}

/////////////////////////////
// Assignment
//

LatLonBox &LatLonBox::operator=(const LatLonBox &rhs)
{
  return _copy(rhs);
}

/////////////////////////////
// Check for equality
//

bool LatLonBox::operator==(const LatLonBox &rhs)
{
  if (fabs(_minLat - rhs._minLat) > 1.0e-5) {
    return false;
  }
  if (fabs(_maxLat - rhs._maxLat) > 1.0e-5) {
    return false;
  }
  if (fabs(_minLon - rhs._minLon) > 1.0e-5) {
    return false;
  }
  if (fabs(_maxLon - rhs._maxLon) > 1.0e-5) {
    return false;
  }
  return true;
}

bool LatLonBox::operator!=(const LatLonBox &rhs)
{
  return !operator==(rhs);
}

/////////////////////////////////////////////////////////
// initialize data members

void LatLonBox::_init()
  
{
  _minLat = -90.0;
  _maxLat = 90.0;
  _minLon = -360.0;
  _maxLon = 360.0;
}

/////////////////////////////////////////////////////////
// clear the limits

void LatLonBox::clearLimits()
{
  _init();
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

LatLonBox &LatLonBox::_copy(const LatLonBox &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }
  
  _minLat = rhs._minLat;
  _maxLat = rhs._maxLat;

  _minLon = rhs._minLon;
  _maxLon = rhs._maxLon;

  return *this;
  
}

/////////////////////////////////////////////////////////
// set limits

void LatLonBox::setLimits(double minLat,
                          double maxLat,
                          double minLon,
                          double maxLon)
{
  _minLat = minLat;
  _maxLat = maxLat;
  _minLon = minLon;
  _maxLon = maxLon;
}

/////////////////////////////////////////////////////////
// print

void LatLonBox::print(ostream &out) const
{
  out << "  LatLonBox:" << endl;
  out << "    minLat: " << _minLat << endl;
  out << "    maxLat: " << _maxLat << endl;
  out << "    minLon: " << _minLon << endl;
  out << "    maxLon: " << _maxLon << endl;
}

