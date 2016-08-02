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
 * @file Grid2dEdgeBuilder.cc
 */

#include <euclid/Grid2dEdgeBuilder.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

#define BAD -99.0
#define GOOD 1.0

//----------------------------------------------------------------
Grid2dEdgeBuilder::Grid2dEdgeBuilder(const Grid2d &g)
{
  _edge = g;
  _edge.setName("edge");
  _edge.changeMissing(BAD);
  _edge.setAllMissing();
  _x0=-1;
  _last_x = -1;
  _last_y = -1;
}

//----------------------------------------------------------------
Grid2dEdgeBuilder::~Grid2dEdgeBuilder()
{
}

//----------------------------------------------------------------
void Grid2dEdgeBuilder::addVertex(const int x, const int y)
{
  if (_edge.inRange(x, y))
  {
    _edge(x, y) = GOOD;
    if (_x0 == -1)
    {
      _x0 = _x1 = x;
      _y0 = _y1 = y;
    }
    else
    {
      if (x < _x0) _x0 = x;
      if (y < _y0) _y0 = y;
      if (x > _x1) _x1 = x;
      if (y > _y1) _y1 = y;
    }
    _fillGaps(x, y);
    _last_x = x;
    _last_y = y;
  }
  else
  {
    LOG(ERROR) << "Out of range " << x << " " << y;
  }
}


//----------------------------------------------------------------
bool Grid2dEdgeBuilder::bad(void) const
{
  return (_x0 < 0);
}
  
//----------------------------------------------------------------
void Grid2dEdgeBuilder::_fillGaps(const int x, const int y)
{
  if (_last_x == -1)
  {
    return;
  }

  // make a line equation
  bool vertical = _last_x  == x;
  double slope = 0.0, intercept = 0.0;
  if (!vertical)
  {
    slope = static_cast<double>(y-_last_y)/static_cast<double>(x-_last_x);
    intercept = static_cast<double>(y) - slope*static_cast<double>(x);
  }
  bool more_vert = (vertical || (!vertical && fabs(slope) > 1.0));
  if (more_vert)
  {
    int y0, y1;
    if (y > _last_y)
    {
      y0 = _last_y+1;
      y1 = y-1;
    }
    else
    {
      y0 = y+1;
      y1 = _last_y-1;
    }
    for (int iy=y0; iy<=y1; ++iy)
    {
      int ix;
      if (vertical)
      {
	ix = x;
      }
      else
      {
	double v = (static_cast<double>(iy)-intercept)/slope;
	ix = static_cast<int>(rint(v));
      }
      _edge(ix, iy) = GOOD;
    }
  }
  else
  {
    int x0, x1;
    if (x > _last_x)
    {
      x0 = _last_x+1;
      x1 = x-1;
    }
    else
    {
      x0 = x+1;
      x1 = _last_x-1;
    }
    for (int ix=x0; ix<=x1; ++ix)
    {
      int iy;
      double v;
      v = static_cast<double>(ix)*slope+intercept;
      iy = static_cast<int>(rint(v));
      _edge(ix, iy) = GOOD;
    }
  }
}

