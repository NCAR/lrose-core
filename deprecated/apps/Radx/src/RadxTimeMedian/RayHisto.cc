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
 * @file RayHisto.cc
 */
#include <cstdio>
#include "RayHisto.hh"
#include <Radx/RayxData.hh>
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
RayHisto::RayHisto(void) :
  _az(0),
  _elev(0),
  _x0(0),
  _dx(0),
  _nx(0)
{
}

//------------------------------------------------------------------
RayHisto::RayHisto(const double az, const double elev, const double x0,
		   const double dx, const int nx, const double minBin,
		   const double deltaBin, const double maxBin):
  _az(az),
  _elev(elev),
  _x0(x0),
  _dx(dx),
  _nx(nx)
{
  _histograms.resize(_nx);
  for (int i=0; i<_nx; ++i)
  {
    _histograms[i] = Histo(minBin, deltaBin, maxBin);
  }
}

//------------------------------------------------------------------
RayHisto::~RayHisto()
{
}

//------------------------------------------------------------------
bool RayHisto::update(const RayxData &r)
{
  int n;

  if (_nx > r.getNpoints())
  {
    n = r.getNpoints();
  }
  else
  {
    n = _nx;
  }
  if (r.matchBeam(_x0, _dx))
  {
    for (int i=0; i<n; ++i)
    {
      double v;
      if (r.getV(i, v))
      {
	_histograms[i].update(v);
      }
      else
      {
	_histograms[i].updateMissing();
      }
    }
    return true;
  }
  else
  {
    LOG(LogMsg::WARNING, "No beam match for a ray");
    return false;
  }
}

//------------------------------------------------------------------
bool RayHisto::computeMedian(RayxData &r) const
{
  if (r.matchBeam(_x0, _dx))
  {
    for (int i=0; i<_nx; ++i)
    {
      double m;
      if (_histograms[i].computeMedian(m))
      {
	r.setV(i, m);
      }
      else
      {
	r.setV(i, r.getMissing());
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

//------------------------------------------------------------------
void RayHisto::print(void) const
{
  printf("az:%lf  elev:%lf  x0:%lf   dx:%lf  nx:%d\n",
	 _az, _elev, _x0, _dx, _nx);
  for (size_t i=0; i<_histograms.size(); ++i)
  {
    _histograms[i].print(_x0 + _dx*static_cast<double>(i));
  }
}
