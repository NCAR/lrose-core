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

#include "OutputGeometry.hh"
using namespace std;


  int _nx, 
      _ny;

  float _dx,
        _dy,
        _minx,
        _miny;

  bool _isGeometrySet;

OutputGeometry::OutputGeometry() :
  _nx(0),
  _ny(0),
  _dx(0.0),
  _dy(0.0),
  _minx(0.0),
  _miny(0.0),
  _isGeometrySet(false)

{

}

OutputGeometry::OutputGeometry(const int nx, const int ny,
                               const float dx, const float dy,
                               const float minx, const float miny,
                               const bool debug, const bool verbose) :
  _nx(nx),
  _ny(ny),
  _dx(dx),
  _dy(dy),
  _minx(minx),
  _miny(miny),
  _isGeometrySet(true)

{

}


void OutputGeometry::setGeometry(const int nx, const int ny,
                                 const float dx, const float dy,
                                 const float minx, const float miny)

{
  _nx = nx;
  _ny = ny;
  _dx = dx;
  _dy = dy;
  _minx = minx;
  _miny = miny;
  _isGeometrySet = true;
}

bool OutputGeometry::isEqual(const int nx, const int ny,
                             const float dx, const float dy,
                             const float minx, const float miny) const

{
  if (_nx == nx && _ny == ny &&
      _dx == dx && _dy == dy &&
      _minx == minx && _miny == miny) {

    return true;
  }
  else {
    return false;
  }
}


