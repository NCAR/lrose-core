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
/**
 * @file GridProj.cc
 */
#include <cmath>
#include <FiltAlg/GridProj.hh>

//------------------------------------------------------------------
GridProj::GridProj()
{
  _nx = 0;
  _ny = 0;
  _dx = 0;
  _dy = 0;
  _x0 = _y0 = 0;
  _lat = _lon = 0;
  _proj_type = 0;
}

//------------------------------------------------------------------
GridProj::GridProj(const Mdvx::field_header_t &hdr)
{
  _nx = hdr.nx;
  _ny = hdr.ny;
  _dx = hdr.grid_dx;
  _dy = hdr.grid_dy;
  _x0 = hdr.grid_minx;
  _y0 = hdr.grid_miny;
  _lat = hdr.proj_origin_lat;
  _lon = hdr.proj_origin_lon;
  _proj_type = hdr.proj_type;
}

//------------------------------------------------------------------
GridProj::~GridProj()
{
}

//------------------------------------------------------------------
bool GridProj::operator==(const GridProj &g) const
{
  return (_nx == g._nx && _ny == g._ny &&
	  _dx == g._dx && _dy == g._dy &&
	  _x0 == g._x0 && _y0 == g._y0 &&
	  _lat == g._lat && _lon == g._lon &&
	  _proj_type == g._proj_type);
}

//------------------------------------------------------------------
void GridProj::print(void) const
{
  printf("nx,ny=%d,%d dx,dy=%lf,%lf  x0,y0=%lf,%lf proj(lat,long):%lf,%lf  type;%d\n",
	 _nx, _ny, _dx, _dy, _x0, _y0, _lat, _lon, _proj_type);
}

//------------------------------------------------------------------
bool GridProj::isCircle(void) const
{
  if (_proj_type == Mdvx::PROJ_POLAR_RADAR ||
      _proj_type == Mdvx::PROJ_RADIAL)
  {
    double delta;
    double y1 = static_cast<double>(_ny-1)*_dy + _y0;
    delta = fabs(y1 - _y0);
    while (delta >= 360.0)
    {
      delta -= 360.0;
    }
    return (delta == fabs(_dy));
  }
  else
  {
    return false;
  }
}
