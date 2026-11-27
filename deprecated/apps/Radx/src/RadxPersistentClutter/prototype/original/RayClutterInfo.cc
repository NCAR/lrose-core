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
 * @file RayClutterInfo.cc
 */
#include <cstdio>
#include "RayClutterInfo.hh"
#include "FrequencyCount.hh"
#include <Radx/RayxData.hh>
#include <toolsa/LogMsg.hh>

//------------------------------------------------------------------
RayClutterInfo::RayClutterInfo(void) :
  _x0(0),
  _dx(0),
  _nx(0),
  _az(0),
  _elev(0)
{
  _counts = RayxData();
  _clutter = RayxData();
}

//------------------------------------------------------------------
RayClutterInfo::RayClutterInfo(const double az, const double elev,
			       const double x0, const double dx, const int nx) :
  _x0(x0),
  _dx(dx),
  _nx(nx),
  _az(az),
  _elev(elev)
{
  _counts = RayxData("counts", "none", _nx, -1.0, _az, _elev, _dx, _x0);
  _counts.setAllToValue(0.0);

  _clutter = RayxData("clutter", "none", _nx, -1.0, _az, _elev, _dx, _x0);
  _clutter.setAllToValue(0.0);
}

//------------------------------------------------------------------
RayClutterInfo::~RayClutterInfo()
{
}

//------------------------------------------------------------------
bool RayClutterInfo::update(const RayxData &r, const double threshold)
{
  int n = _initNpt(r);
  if (n >0)
  {
    for (int i=0; i<n; ++i)
    {
      double v;
      if (r.getV(i, v))
      {
	if (v >= threshold)
	{
	  LOGF(LogMsg::DEBUG_VERBOSE, "beam[%d]=%lf YES", i, v);
	  _counts.incAtPoint(i, 1.0);
	}
	else
	{
	  LOGF(LogMsg::DEBUG_VERBOSE, "beam[%d]=%lf", i, v);
	}
      }
      else
      {
	// missing is assumed non-clutter for sure
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
bool RayClutterInfo::equalOrExceed(RayxData &r, const int k) const
{
  int n = _initNpt(r);
  if (n > 0)
  {
    for (int i=0; i<n; ++i)
    {
      double c;
      if (_counts.getV(i, c))
      {
	if (c >= static_cast<double>(k))
	{
	  r.setV(i, 1.0);
	}
	else
	{
	  r.setV(i, 0.0);
	}
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
bool RayClutterInfo::loadNormalizedFrequency(RayxData &r, const int num) const
{
  int n = _initNpt(r);
  if (n > 0)
  {
    for (int i=0; i<n; ++i)
    {
      double c;
      if (_counts.getV(i, c))
      {
	double value = c/static_cast<double>(num);
	r.setV(i, value);
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
void RayClutterInfo::print(void) const
{
  printf("az:%lf  elev:%lf  x0:%lf   dx:%lf  nx:%d\n",
	 _az, _elev, _x0, _dx, _nx);
}

//------------------------------------------------------------------
double RayClutterInfo::numWithMatchingCount(const int number) const
{
  double dn = static_cast<double>(number);
  double ret = 0.0;
  for (int i=0; i<_nx; ++i)
  {
    double v;
    if (_counts.getV(i, v))
    {
      if (v == dn)
      {
	ret += 1.0;
      }
    }
  }  
  return ret;
}

//------------------------------------------------------------------
int RayClutterInfo::updateClutter(const int number, int &nclutter,
				  FrequencyCount &F)
{
  double dn = static_cast<double>(number);
  int ret = 0;
  for (int i=0; i<_nx; ++i)
  {
    double v;

    bool isClutter = false;
    bool wasClutter = false;
    if (_counts.getV(i, v))
    {
      if (v >= dn)
      {
	isClutter = true;
      }
      F.update(v);
    }
    if (_clutter.getV(i, v))
    {
      if (v == 1.0)
      {
	wasClutter = true;
      }
    }
    if (isClutter)
    {
      ++nclutter;
    }
    if (isClutter && !wasClutter)
    {
      ++ret;
      _clutter.setV(i, 1.0);
    }
    else if ((!isClutter) && wasClutter)
    {
      ++ret;
      _clutter.setV(i, 0.0);
    }
  }  
  return ret;
}

//------------------------------------------------------------------
int RayClutterInfo::_initNpt(const RayxData &r) const
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
    return n;
  }
  else
  {
    LOG(LogMsg::WARNING, "No beam match for a ray");
    return -1;
  }
}
