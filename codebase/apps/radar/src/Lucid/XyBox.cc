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
// XyBox.cc
//
// Zoom limits object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2024
//
///////////////////////////////////////////////////////////////

#include "XyBox.hh"
#include <cmath>

using namespace std;

const double XyBox::defaultLimit = 999999.0;

//////////////
// Constructor

XyBox::XyBox()
  
{
  _init();
}

XyBox::XyBox(double minY, double maxY,
             double minX, double maxX) :
        _minY(minY),
        _maxY(maxY),
        _minX(minX),
        _maxX(maxX)
{

}

/////////////////////////////
// Copy constructor
//

XyBox::XyBox(const XyBox &rhs)
{
  _copy(rhs);
}

/////////////
// destructor

XyBox::~XyBox()
{
}

/////////////////////////////
// Assignment
//

XyBox &XyBox::operator= (const XyBox &rhs)
{
  return _copy(rhs);
}

/////////////////////////////
// Check for equality
//

bool XyBox::operator== (const XyBox &rhs)
{
  if (fabs(_minY - rhs._minY) > 1.0e-5) {
    return false;
  }
  if (fabs(_maxY - rhs._maxY) > 1.0e-5) {
    return false;
  }
  if (fabs(_minX - rhs._minX) > 1.0e-5) {
    return false;
  }
  if (fabs(_maxX - rhs._maxX) > 1.0e-5) {
    return false;
  }
  return true;
}

bool XyBox::operator!= (const XyBox &rhs)
{
  return !operator==(rhs);
}

/////////////////////////////////////////////////////////
// initialize data members

void XyBox::_init()
  
{
  _minY = -defaultLimit;
  _maxY = defaultLimit;
  _minX = -defaultLimit;
  _maxX = defaultLimit;
}

/////////////////////////////////////////////////////////
// clear the limits

void XyBox::clearLimits()
{
  _init();
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

XyBox &XyBox::_copy(const XyBox &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }
  
  _minY = rhs._minY;
  _maxY = rhs._maxY;

  _minX = rhs._minX;
  _maxX = rhs._maxX;

  return *this;
  
}

/////////////////////////////////////////////////////////
// set limits

void XyBox::setLimits(double minY,
                      double maxY,
                      double minX,
                      double maxX)
{
  _minY = minY;
  _maxY = maxY;
  _minX = minX;
  _maxX = maxX;
}

/////////////////////////////////////////////////////////
// print

void XyBox::print(ostream &out) const
{
  out << "  XyBox:" << endl;
  out << "    minY: " << _minY << endl;
  out << "    maxY: " << _maxY << endl;
  out << "    minX: " << _minX << endl;
  out << "    maxX: " << _maxX << endl;
}

