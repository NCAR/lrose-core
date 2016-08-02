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
 * @file Grid2dInside.cc
 */

#include <euclid/Grid2dInside.hh>
#include <euclid/Grid2dEdgeBuilder.hh>

//----------------------------------------------------------------
Grid2dInside::Grid2dInside(const Grid2dEdgeBuilder &e)
{
  // range comes from the input object
  e.getRange(_x0, _x1, _y0, _y1);

  // point to the grid itself, with non-missing values at edge points
  const Grid2d *g = e.getEdgePtr();

  double out = 0.0;   /**< Point not inside */
  double in = 1.0;    /**< Point for sure inside */
  double maybe = 2.0; /**< Point at which not sure */

  // create the subset grid
  _nx = _x1-_x0+1;
  _ny = _y1-_y0+1;
  _g = Grid2d("inside", _nx, _ny, -1.0);

  // start with inside everywhere
  _g.setAllToValue(1.0);

  // create some workspace grids
  Grid2d dir_x("dir_x", _nx, _ny, -1.0);
  Grid2d dir_y("dir_y", _nx, _ny, -1.0);

  // erode away in all 4 search directions.
  for (int iy=0; iy<_ny; ++iy)
  {
    bool is_out = true;
    for (int ix=0; ix<_nx; ++ix)
    {
      if (g->isMissing(ix+_x0, iy+_y0))
      {
	if (is_out)
	{
	  dir_x(ix, iy) = out;
	}
	else
	{
	  dir_x(ix, iy) = maybe;
	}
      }
      else
      {
	is_out = false;
	dir_x(ix, iy) = in;
      }
    }
    is_out = true;
    for (int ix=_nx-1; ix>=0; --ix)
    {
      if (g->isMissing(ix+_x0, iy+_y0))
      {
	if (is_out)
	{
	  dir_x(ix, iy) = out;
	}
      }
      else
      {
	is_out = false;
      }
    }
  }

  for (int ix=0; ix<_nx; ++ix)
  {
    bool is_out = true;
    for (int iy=0; iy<_ny; ++iy)
    {
      if (g->isMissing(ix+_x0, iy+_y0))
      {
	if (is_out)
	{
	  dir_y(ix, iy) = out;
	}
	else
	{
	  dir_y(ix, iy) = maybe;
	}
      }
      else
      {
	is_out = false;
	dir_y(ix, iy) = in;
      }
    }
    is_out = true;
    for (int iy=_ny-1; iy>=0; --iy)
    {
      if (g->isMissing(ix+_x0, iy+_y0))
      {
	if (is_out)
	{
	  dir_y(ix, iy) = out;
	}
      }
      else
      {
	is_out = false;
      }
    }
  }
  for (int i=0; i<_nx*_ny; ++i)
  {
    if (dir_y[i] == in || dir_x[i] == in)
    {
      _g[i] = 1.0;
    }
    else if (dir_y[i] == out || dir_x[i] == out)
    {
      _g.setMissing(i);
    }
    else
    {
      // at these 'ambiguous' points we leave things inside i.e. true
      //       printf("HERE IS THE AMBIGUOUS REGION\n");
    }
  }

}

//----------------------------------------------------------------
Grid2dInside::~Grid2dInside()
{
}

//----------------------------------------------------------------
bool Grid2dInside::inside(const int x, const int y, int &xi, int &yi) const
{
  xi = x + _x0;
  yi = y + _y0;
  return !_g.isMissing(x, y);
}
