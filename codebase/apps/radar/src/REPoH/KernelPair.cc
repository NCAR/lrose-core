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
 * @file KernelPair.cc
 */
#include "KernelPair.hh"
#include <FiltAlg/GridProj.hh>
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>

/*----------------------------------------------------------------*/
KernelPair::KernelPair(const double vlevel, const Grid2d &mask_far,
		       const Grid2d &mask_near, const Grid2d &omask,
		       const CloudGap &g, const Grid2d &clumps,
		       const Params &params, const GridProj &gp) :
  _near(vlevel, false, mask_near, omask, g, clumps, params, gp),
  _far(vlevel, true, mask_far, omask, g, clumps, params, gp)
{
}

/*----------------------------------------------------------------*/
KernelPair::~KernelPair()
{
}

/*----------------------------------------------------------------*/
void KernelPair::print(void) const
{
  _near.print_status();
  _far.print_status();
}

/*----------------------------------------------------------------*/
void KernelPair::cp_to_grid(Grid2d &kcp) const
{
  _near.cp_to_grid(kcp);
  _far.cp_to_grid(kcp);
}

/*----------------------------------------------------------------*/
void KernelPair::finish_processing(const time_t &time, const double vlevel,
				   const KernelGrids &grids, const Params &P,
				   const double km_per_gate, const int &id,
				   Grid2d &kcp)
{
  _near.finish_processing(time, id, vlevel, grids, P, km_per_gate);
  _far.finish_processing(time, id+1, vlevel, grids, P, km_per_gate);
  cp_to_grid(kcp);
}

/*----------------------------------------------------------------*/
string KernelPair::humidity_estimate(const double vlevel, const GridProj &gp,
				     Grid2d &att, Grid2d &hum) const
{
  int x0, x1, y0, y1;
  double a0, a1;

  // Get attenuation (and center x,y) for near and far kernels
  _near.get_attenuation(x0, y0, a0);
  _far.get_attenuation(x1, y1, a1);
  if (y0 != y1)
  {
    LOG(ERROR) << "Logic error";
    return "";
  }

  // Derive the attenuation along the path (one way, so divide by 2)
  double path_length = (double)(x1 - x0)*gp._dx;
  double attenuation_along_path = (a1 - a0)/(path_length*2.0);

  // Derive humidity from that
  double h = Kernel::humidity_from_attenuation(attenuation_along_path);

  // Store to grids
  for (int i=x0; i<=x1; ++i)
  {
    hum.setValue(i, y0, h);
    att.setValue(i, y0, attenuation_along_path);
  }

  // convert the azimuth into a number between 0 and 359
  double az = (double)y0*gp._dy + gp._y0;
  while (az >=360.0)
    az -= 360.0;
  while (az < 0.0)
    az += 360.0;

  // build the string 
  char buf[1000];
  sprintf(buf, "%10.6lf %7.2lf %8.5lf %8.5lf %10.6lf %10.6lf\n",
	  vlevel, az, (double)x1*gp._dx + gp._x0,
	  (double)x0*gp._dx + gp._x0, attenuation_along_path, h);
  string ret = buf;
  return ret;
}

/*----------------------------------------------------------------*/
bool KernelPair::write_genpoly(const time_t &t, const int nx, const int ny,
			       const bool outside, DsSpdb &D) const
{
  bool stat = true;
  if (!_near.write_genpoly(t, nx, ny, outside, D))
    stat = false;
  if (!_far.write_genpoly(t, nx, ny, outside, D))
    stat = false;
  return stat;
}
