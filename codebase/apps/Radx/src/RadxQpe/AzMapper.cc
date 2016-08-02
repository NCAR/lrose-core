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
 * @file AzMapper.cc
 */
#include "AzMapper.hh"
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <cmath>

//----------------------------------------------------------------
AzMapper::AzMapper(void)
{
}

//----------------------------------------------------------------
AzMapper::AzMapper(const RadxSweep &sweep, const std::vector<RadxRay *> &rays)
{
  for (int ir = sweep.getStartRayIndex(); 
       ir <= static_cast<int>(sweep.getEndRayIndex()); ++ir)
  {
    const RadxRay *ray = rays[ir];
    double a = ray->getAzimuthDeg();
    _map.push_back(AzMapper1(ir, a));
  }
}

//----------------------------------------------------------------
AzMapper::~AzMapper(void)
{
}

//----------------------------------------------------------------
int AzMapper::closestRayIndex(double a, double maxDelta) const
{
  if (_map.empty())
  {
    return -1;
  }
  
  double min = fabs(_map[0]._azimuthDeg - a);
  int ret = _map[0]._rayIndex;
  for (size_t i=1; i<_map.size(); ++i)
  {
    double diff = fabs(_map[i]._azimuthDeg - a);
    if (diff < min)
    {
      min = diff;
      ret = _map[i]._rayIndex;
    }
  }
  if (min > maxDelta)
  {
    return -1;
  }
  else
  {
    return ret;
  }
}


