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
#include <toolsa/copyright.h>
#include <euclid/Box.hh>
#include <cstdio>
using std::string;

/*----------------------------------------------------------------*/
Box::Box(void)
{
  _ok = false;
}

/*----------------------------------------------------------------*/
Box::Box(double x0, double y0, double x1, double y1)
{
  _ok = true;
  _setBoxValues(x0, y0, x1, y1);
}

/*----------------------------------------------------------------*/
Box::~Box()
{
}

/*----------------------------------------------------------------*/
void Box::print(void) const
{
  print(stdout);
}

/*----------------------------------------------------------------*/
void Box::print(FILE *fp) const
{
  if (emptyBox())
    fprintf(fp, "Box [empty]\n");
  else
    fprintf(fp, "Box [%.1f,%.1f] to [%.1f,%.1f]\n",
	    _minx, _miny, _maxx, _maxy);
}

/*----------------------------------------------------------------*/
string Box::sprint(void) const
{
  char buf[1000];
  if (emptyBox())
    sprintf(buf, "Box [empty]");
  else
    sprintf(buf, "Box [%.1f,%.1f] to [%.1f,%.1f]",
	    _minx, _miny, _maxx, _maxy);
  string ret = buf;
  return ret;
}

/*----------------------------------------------------------------*/
void Box::expand(double distance, bool no_truncate, int nx, int ny)
{
  if (emptyBox())
    return;

  /*
   * Expand each thing.
   */
  _minx -= distance;
  _maxx += distance;
  _miny -= distance;
  _maxy += distance;

  /*
   * Truncate at image edges.
   */
  if (!no_truncate)
    _truncateAtEdges(nx, ny);
}

/*----------------------------------------------------------------*/
void Box::expand(const Box &b)
{
  if (b.emptyBox())
    return;
  if (emptyBox())
  {
    *this = b;
    return;
  }
  if (b._minx < _minx)
    _minx = b._minx;
  if (b._miny < _miny)
    _miny = b._miny;
  if (b._maxx > _maxx)
    _maxx = b._maxx;
  if (b._maxy > _maxy)
    _maxy = b._maxy;
}

/*----------------------------------------------------------------*/
void Box::_truncateAtEdges(int nx, int ny)
{
  double x, y;
  if (emptyBox())
    return;
    
  x = nx;
  y = ny;
  if (_minx < 0) _minx = 0.0;
  if (_miny < 0) _miny = 0.0;
  if (_minx >= x) _minx = x;
  if (_miny >= y) _miny = y;
  if (_maxx < 0) _maxx = 0.0;
  if (_maxy < 0) _maxy = 0.0;
  if (_maxx >= x) _maxx = x;
  if (_maxy >= y) _maxy = y;
}

