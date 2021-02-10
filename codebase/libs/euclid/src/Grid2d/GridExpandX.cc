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
 * @file GridExpandX.cc
 */
#include <euclid/GridExpandX.hh>

//------------------------------------------------------------------
GridExpandX::GridExpandX() : GridAlgs()
{
   _expandNx = 0;
}

//------------------------------------------------------------------
GridExpandX::GridExpandX(const Grid2d &g, const int nx) :
  GridAlgs(g.getName(), g.getNx() + 2*nx, g.getNy(), g.getMissing())
{
  _expandNx = nx;
}

//------------------------------------------------------------------
GridExpandX::~GridExpandX()
{
}

//------------------------------------------------------------------
void GridExpandX::fill(const Grid2d &gdata, const bool doesWraparound)
{
  setAllMissing();

  int gx, x;
  int grid_nx, grid_ny;
  grid_nx = gdata.getNx();
  grid_ny = gdata.getNy();
  if (doesWraparound)
  {
    // fill lowest x with repeat of rightmost
    for (gx=grid_nx-_expandNx,x=0; gx<grid_nx; ++gx,++x)
    {
      for (int y=0; y<grid_ny; ++y)
      {
	double v = gdata.getValue(gx, y);
	setValue(x, y, v);
      }
    }
  }
    
  // fill middle part with input
  for (gx=0,x=_expandNx; gx<grid_nx; ++gx,++x)
  {
    for (int y=0; y<grid_ny; ++y)
    {
      double v = gdata.getValue(gx, y);
      setValue(x, y, v);
    }
  }

  if (doesWraparound)
  {
    // fill rightmost x with repeat of leftmost input
    for (gx=0, x=grid_nx+_expandNx; gx<_expandNx; ++gx, ++x)
    {
      for (int y=0; y<grid_ny; ++y)
      {
	double v = gdata.getValue(gx, y);
	setValue(x, y, v);
      }
    }
  }
}

//------------------------------------------------------------------
bool GridExpandX::xyIsInExpandRange(const int x, const int y) const
{
  return inRange(x+_expandNx, y);
}


//------------------------------------------------------------------
bool GridExpandX::xyGetValue(const int x, const int y, double &v) const
{
  return getValue(x+_expandNx, y, v);
}

//------------------------------------------------------------------
bool GridExpandX::xIsInExpandRange(const int x) const
{
  return (x+_expandNx >= 0 && x+_expandNx < _nx);
}

//------------------------------------------------------------------
bool GridExpandX::yIsInExpandRange(const int y) const
{
  return (y >= 0 && y < _ny);
}


//------------------------------------------------------------------
GridAlgs GridExpandX::unexpand(void) const
{
  GridAlgs ret(getName(), getNx() - 2*_expandNx, getNy(), getMissing());
  for (int y=0; y<getNy(); ++y)
  {
    int x, locx;
    for (locx=_expandNx, x=0; locx < getNx()-_expandNx; ++locx,++x)
    {
      if (x < ret.getNx())
      {
	if (locx < getNx())
	{
	  double v;
	  if (getValue(locx, y, v))
	  {
	    ret.setValue(x, y, v);
	  }
	}
      }
    }
  }
  return ret;
}


