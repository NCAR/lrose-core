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
 * @file CloudGap.cc
 */
#include "CloudGap.hh"
#include "CloudEdge.hh"
#include <euclid/Grid2d.hh>
#include <cstdio>

/*----------------------------------------------------------------*/
CloudGap::CloudGap(const CloudEdge &e)
{
  _y = e.get_y();

  _x0[0] = _x0[1] = 0;
  _xout0[0] = _xout0[1] = -1;
  _v0 = 0.0;

  _x1[0] = e.get_x0();
  _x1[1] = e.get_x1();
  _xout1[0] = e.get_xout0();
  _xout1[1] = e.get_xout1();
  _v1 = e.get_color();
}

/*----------------------------------------------------------------*/
CloudGap::CloudGap(const CloudEdge &e0, const CloudEdge &e1)
{
  _y = e0.get_y();
  if (e1.get_y() != _y)
    LOG(ERROR) << _y << " " << e1.get_y() << " values";

  _x0[0] = e0.get_x0();
  _x0[1] = e0.get_x1();
  _xout0[0] = e0.get_xout0();
  _xout0[1] = e0.get_xout1();
  _v0 = e0.get_color();

  _x1[0] = e1.get_x0();
  _x1[1] = e1.get_x1();
  _xout1[0] = e1.get_xout0();
  _xout1[1] = e1.get_xout1();
  _v1 = e1.get_color();
}

/*----------------------------------------------------------------*/
CloudGap::~CloudGap()
{
}

/*----------------------------------------------------------------*/
void CloudGap::to_grid(Grid2d &data) const
{
  for (int x=_x0[0]; x<=_x0[1]; ++x)
  {
    if (x >= 0)
      data.setValue(x, _y, _v0);
  }
  for (int x=_x1[0]; x<=_x1[1]; ++x)
  {
    if (x >= 0)
      data.setValue(x, _y, _v1);
  }
}

/*----------------------------------------------------------------*/
void CloudGap::to_outside_grid(Grid2d &outside) const
{
  for (int x=_xout0[0]; x<=_xout0[1]; ++x)
  {
    if (x >= 0)
      outside.setValue(x, _y, _v0);
  }
  for (int x=_xout1[0]; x<=_xout1[1]; ++x)
  {
    if (x >= 0)
      outside.setValue(x, _y, _v1);
  }
}

/*----------------------------------------------------------------*/
int CloudGap::npt_between(void) const	
{
  return _x1[0] - _x0[1];
}

/*----------------------------------------------------------------*/
int CloudGap::get_x(const bool is_far) const
{
  if (is_far)
    // first x is it
    return _x1[0];
  else
    // last x is it
    return _x0[1];
}

/*----------------------------------------------------------------*/
void CloudGap::print(void) const
{
  printf("x0:[%d,%d]  x0_out:[%d,%d]   x1:[%d,%d]  x1_out:[%d,%d]  v0:%lf v1:%lf\n",
	 _x0[0], _x0[1], _xout0[0], _xout0[1], 
	 _x1[0], _x1[1], _xout1[0], _xout1[1], _v0, _v1);
}


