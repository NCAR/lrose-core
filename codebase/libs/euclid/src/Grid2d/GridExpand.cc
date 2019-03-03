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
 * @file GridExpand.cc
 */
#include <euclid/GridExpand.hh>

//------------------------------------------------------------------
GridExpand::GridExpand() : Grid2d()
{
   _expandNy = 0;
}

//------------------------------------------------------------------
GridExpand::GridExpand(const Grid2d &g, const int ny) :
  Grid2d("expand", g.getNx(), g.getNy() + 2*ny, g.getMissing())
{
  _expandNy = ny;
}

//------------------------------------------------------------------
GridExpand::~GridExpand()
{
}

//------------------------------------------------------------------
void GridExpand::fill(const Grid2d &gdata, const bool doesWraparound)
{
  setAllMissing();

  int gy, y;
  int grid_nx, grid_ny;
  grid_nx = gdata.getNx();
  grid_ny = gdata.getNy();
  if (doesWraparound)
  {
    // fill bottom y with repeat of top of input
    for (gy=grid_ny-_expandNy,y=0; gy<grid_ny; ++gy,++y)
    {
      for (int x=0; x<grid_nx; ++x)
      {
	double v = gdata.getValue(x, gy);
	setValue(x, y, v);
      }
    }
  }
    
  // fill middle part with input
  for (gy=0,y=_expandNy; gy<grid_ny; ++gy,++y)
  {
    for (int x=0; x<grid_nx; ++x)
    {
      double v = gdata.getValue(x, gy);
      setValue(x, y, v);
    }
  }

  if (doesWraparound)
  {
    // fill top y with repeat of bottom of input
    for (gy=0, y=grid_ny+_expandNy; gy<_expandNy; ++gy, ++y)
    {
      for (int x=0; x<grid_nx; ++x)
      {
	double v = gdata.getValue(x, gy);
	setValue(x, y, v);
      }
    }
  }
}

//------------------------------------------------------------------
bool GridExpand::xyIsInExpandRange(const int x, const int y) const
{
  return inRange(x, y+_expandNy);
}


//------------------------------------------------------------------
bool GridExpand::xyGetValue(const int x, const int y, double &v) const
{
  return getValue(x, y+_expandNy, v);
}

//------------------------------------------------------------------
bool GridExpand::yIsInExpandRange(const int y) const
{
  return (y+_expandNy >= 0 && y+_expandNy < _ny);
}

//------------------------------------------------------------------
bool GridExpand::xIsInExpandRange(const int x) const
{
  return (x >= 0 && x < _nx);
}
