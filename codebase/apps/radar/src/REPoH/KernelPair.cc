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
#include <Mdv/MdvxProj.hh>
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>

/*----------------------------------------------------------------*/
KernelPair::KernelPair(double vlevel, const Grid2d &mask_far,
		       const Grid2d &mask_near, const Grid2d &omask,
		       const CloudGap &g,  const RepohParams &params,
		       const MdvxProj &gp) :
  _near(vlevel, false, mask_near, omask, g, params, gp),
  _far(vlevel, true, mask_far, omask, g, params, gp)
{
}

/*----------------------------------------------------------------*/
KernelPair::~KernelPair()
{
}

/*----------------------------------------------------------------*/
void KernelPair::print(void) const
{
  _near.printStatus();
  _far.printStatus();
}

/*----------------------------------------------------------------*/
void KernelPair::centerpointToGrid(Grid2d &kcp) const
{
  _near.centerpointToGrid(kcp);
  _far.centerpointToGrid(kcp);
}

/*----------------------------------------------------------------*/
void KernelPair::finishProcessing(const time_t &time, double vlevel,
				  const KernelGrids &grids,
				  const RepohParams &P, double kmPerGate,
				  int nearId, int farId)
{
  _near.finishProcessing(time, nearId, vlevel, grids, P, kmPerGate);
  _far.finishProcessing(time, farId, vlevel, grids, P, kmPerGate);
}

/*----------------------------------------------------------------*/
void KernelPair::computeAttenuation(double dx, Grid2d &att) const
{
  // compute the attenuation, y, and range of x
  double x0, x1, y;
  double a = attenuation(dx, x0, x1, y);

  // Store to output grid for that sub beam
  for (int i=(int)x0; i<=(int)x1; ++i)
  {
    att.setValue(i, (int)y, a);
  }
}

/*----------------------------------------------------------------*/
void KernelPair::computeHumidity(double dx, Grid2d &hum) const
{
  // compute the attenuation, y, and range of x
  double x0, x1, y;
  double a = attenuation(dx, x0, x1, y);

  // Derive humidity from that
  double h = Kernel::humidityFromAttenuation(a);

  // Store to output grid for that sub beam
  for (int i=(int)x0; i<=(int)x1; ++i)
  {
    hum.setValue(i, (int)y, h);
  }
}

/*----------------------------------------------------------------*/
string KernelPair::asciiOutput(double vlevel, const MdvxProj &gp) const
{
  Mdvx::coord_t coord = gp.getCoord();

  // compute attenuation, y, and range of x
  double x0, x1, y;

  double att = attenuation(coord.dx, x0, x1, y);

  // derive humidity from that
  double h = Kernel::humidityFromAttenuation(att);

  // convert the azimuth associated with y into a number between 0 and 359
  double az = y*coord.dy + coord.miny;
  while (az >=360.0)
    az -= 360.0;
  while (az < 0.0)
    az += 360.0;

  // build the string, which does have \n
  char buf[1000];
  sprintf(buf, "%10.6lf %7.2lf %8.5lf %8.5lf %10.6lf %10.6lf\n",
	  vlevel, az, x1*coord.dx + coord.minx, x0*coord.dx + coord.minx,
	  att, h);
  string ret = buf;
  return ret;
}

/*----------------------------------------------------------------*/
double KernelPair::attenuation(double dx, double &x0, double &x1,
			       double &y) const
{
  double y1;
  double a0, a1;

  // Get attenuation (and center x,y) for near and far kernels
  _near.getAttenuation(x0, y, a0);
  _far.getAttenuation(x1, y1, a1);
  if (y != y1)
  {
    LOG(ERROR) << "Logic error";
    return 0.0;
  }

  // Derive the attenuation along the path (one way, so divide by 2)
  double path_length = (x1 - x0)*dx;
  double attenuation_along_path = (a1 - a0)/(path_length*2.0);
  return attenuation_along_path;
}

/*----------------------------------------------------------------*/
bool KernelPair::writeGenpoly(const time_t &t, bool outside,
			      const MdvxProj &proj, DsSpdb &D) const
{
  bool stat = true;
  if (!_near.writeGenpoly(t, outside, proj, D))
    stat = false;
  if (!_far.writeGenpoly(t, outside, proj, D))
    stat = false;
  return stat;
}
